#!/usr/bin/env python3
"""dsconv.py MODUL [--write] [--globals] - Phase B Umsetzungshilfe: schreibt die Daten eines Moduls in seinen C-Quelltext.

Grundlage: cbau/<m>.OBJ (C-Namen, Stand letzter build.sh; CBAU=... fuer anderen Ordner), cbau/<m>.FIX,
lnk/asm1/SCHW_CD.MAP, tools/dsinfo.own (Modulbereich), Original-DS.
Vorgehen:
  1. Jeder extern-Name des Moduls, der in den eigenen Bereich zeigt, bekommt seine Basisadresse (Verweise
     der C-OBJ: Inline-Wert C gegen Adresse der FIX). Luecken ohne Namen werden zu g_<adr>.
  2. Hinten liegende Strings, die genau einmal (Versatz 0) benutzt werden, werden zu Literalen an der
     Verwendungsstelle (BCC: Literalpool hinter den initialisierten Daten, Reihenfolge des Auftretens).
     Mit --globals entfaellt das (alles wird Definition).
  3. Alles davor wird in Adressreihenfolge als initialisierte Definition geschrieben (Typ aus der
     extern-Deklaration des Moduls bzw. aus names/inc/s_data.h; char-Arrays als String, wenn moeglich).
     Vorhandene extern-Zeilen im Modul werden durch den Definitionsblock ersetzt.
Ohne --write nur Vorschau. Abbruch mit Meldung, wenn ein Typ nicht automatisch geht (struct, Zeiger) - dann von Hand."""
import sys, os, re, struct, glob, collections
sys.path.insert(0, '/root/sternenschweif/tools')
import objfix, objbytes, dsinfo, items_ds, cobjfix_d as CD
R = '/root/sternenschweif'; CB = os.environ.get('CBAU', R + '/cbau'); OWN = os.environ.get('OWNDIR', CB); MAP = R + '/lnk/asm1/SCHW_CD.MAP'
mod = sys.argv[1]; WRITE = '--write' in sys.argv; GLOB = '--globals' in sys.argv
fn = f'OVL{mod[1:]}' if mod[0] == 'O' else f'SEG{int(mod[1:]):03d}'
src = [p for p in glob.glob(f'{R}/csrc/{fn}.C*')]; src = sorted(src, key=lambda p: not p.endswith('.CPP'))[0]
S = open(src, newline='', encoding='latin1').read(); NL = '\r\n' if '\r\n' in S else '\n'
own = dsinfo.own(mod)
alln = CD.mapnames(MAP)
# 1. C-Namen -> Basisadresse (eigenes Modul und, fuer Luecken, die Namen anderer C-Module)
def bases(fname_, lo, hi):
    """{C-Name: Basis}, uses, offuse fuer Verweise der C-OBJ fname_ in [lo, hi)"""
    d_ = OWN if fname_ == fn else CB
    C = objfix.parse(f'{d_}/{fname_}.OBJ'); F = objfix.parse(f'{d_}/{fname_}.FIX')
    cseg = [s_ for s_ in C['segs'] if s_.endswith('_TEXT')][0]
    cb = objbytes.seg_bytes(f'{d_}/{fname_}.OBJ').get(cseg, b''); fb = objbytes.seg_bytes(f'{d_}/{fname_}.FIX').get(cseg, b'')
    ff = {f['loc']: f for f in F['fix'] if f['seg'] == cseg}
    base = {}; uses = collections.Counter(); offuse = collections.Counter(); bad = set()
    for f in C['fix']:
        if f['seg'] != cseg or f['tgt'][0] != 'EXT' or f['typ'] not in (1, 3, 5, 9): continue
        g = ff.get(f['loc'])
        if not g or g['tgt'][0] != 'EXT' or g['tgt'][1] not in alln or alln[g['tgt'][1]][0] != CD.DG: continue
        ad = (alln[g['tgt'][1]][1] + struct.unpack_from('<H', fb, f['loc'])[0] + g['disp']) & 0xffff
        vc = (struct.unpack_from('<H', cb, f['loc'])[0] + f['disp']) & 0xffff
        bb = (ad - vc) & 0xffff; n = f['tgt'][1]
        if not (lo <= ad < hi): continue
        if n in base and base[n] != bb: bad.add(n)
        base[n] = bb; uses[n] += 1
        if vc: offuse[n] += 1
    return base, uses, offuse, bad
RANGES = [(k, own[k]) for k in ('DATA', 'BSS') if own[k]]
base = {}; uses = collections.Counter(); offuse = collections.Counter()
for k, (lo, hi) in RANGES:
    bb, uu, oo, bad = bases(fn, lo, hi)
    if bad: raise SystemExit(f'widerspruechliche Basis fuer {sorted(bad)} - von Hand')
    base.update(bb); uses.update(uu); offuse.update(oo)
for n, b in base.items():
    if not any(lo <= b < hi for k, (lo, hi) in RANGES): raise SystemExit(f'{n}: Basis {b:#x} ausserhalb des Modulbereichs (Name zeigt nur mit Versatz hinein) - von Hand')
fremd = {}                                     # Adresse -> Name (andere C-Module, nur exakte Basen im Bereich)
for p in sorted(glob.glob(CB + '/*.FIX')):
    f_ = os.path.basename(p)[:-4]
    if f_ == fn or not os.path.exists(f'{CB}/{f_}.OBJ'): continue
    for k, (lo, hi) in RANGES:
        try: bb, _, _, bad = bases(f_, lo, hi)
        except Exception: continue
        for n, b in bb.items():
            if n in bad or not (lo <= b < hi): continue
            fremd.setdefault(b, []).append(n[1:] if n.startswith('_') else n)
# existierende Literale (eigenes _DATA der C-OBJ, D1) -> Lage
lit_have = []
try:
    tmp = CD.analyse(f'{OWN}/{fn}.OBJ', f'{R}/lnk/asm1/{fn}.OBJ', MAP)
    for p in tmp['pieces']:
        if not p['bss'] and own['DATA'] and own['DATA'][0] <= p['adr'] < own['DATA'][1]: lit_have.append((p['adr'], p['adr'] + p['bis'] - p['von']))
except Exception as e:
    lit_have = []
a0, a1 = own['DATA'] if own['DATA'] else (0, 0)
ds = CD.ds_bytes(a0, a1 - a0) if own['DATA'] else b''
def byt(a, b): return ds[a - a0:b - a0]
# Handbloecke: DSCONV_HAND="0x07be-0x0cef:datei;..." - der Bereich wird nicht automatisch definiert, sondern durch den
# Text der Datei ersetzt (Tabellen mit Zeigern auf eigene Daten, Codezeigern, Feldern mit Basis vor dem ersten Element ...)
HAND = []
for _h in filter(None, os.environ.get('DSCONV_HAND', '').split(';')):
    _r, _f = _h.split(':', 1); _a, _e = (int(x, 16) for x in _r.split('-'))
    HAND.append((_a, _e, open(_f, encoding='latin1').read().rstrip('\n').split('\n')))
def objlist(lo, hi, lits):
    out = objlist0(lo, hi, lits)
    for a_, e_, txt in HAND:
        if not (lo <= a_ < hi): continue
        inner = [o for o in out if a_ <= o['a'] < e_]
        if not inner or inner[0]['a'] != a_ or inner[-1]['e'] != e_: raise SystemExit(f'Handblock {a_:#x}..{e_:#x} passt nicht auf Objektgrenzen')
        k_ = out.index(inner[0]); out[k_:k_ + len(inner)] = [dict(a=a_, e=e_, kind='hand', name=f'hand_{a_:04x}', text=txt,
                                                                  names=[o['name'] for o in inner if o['kind'] == 'name'])]
    return out
def objlist0(lo, hi, lits):
    nb = {b: (n[1:] if n.startswith('_') else n) for n, b in base.items() if lo <= b < hi}
    lit_at = {a: e for a, e in lits}
    ptg = [o for p_, o in items_ds.dptr().items() if a0 <= p_ < a1 and lo <= o < hi]
    marks = sorted(set([lo, hi] + list(nb) + [a for a in fremd if lo <= a < hi] + list(lit_at) + list(lit_at.values()) + ptg))
    out = []
    for i in range(len(marks) - 1):
        a, e = marks[i], marks[i + 1]
        if a in lit_at: out.append(dict(a=a, e=lit_at[a], kind='lit_alt')); continue
        if any(x <= a < y for x, y in lits): continue
        if a in nb: out.append(dict(a=a, e=e, kind='name', name=alias(nb[a])))
        elif a in fremd:
            ns = [alias(n) for n in fremd[a]]; hn = [n for n in ns if decl(n) and decl(n)[2] == 'header']
            out.append(dict(a=a, e=e, kind='fremd', name=(hn or sorted(ns))[0]))
        elif decl(alias(f'g_{a:04x}')) or decl_raw(alias(f'g_{a:04x}')):   # Adressname ist schon deklariert (s_data.h)
            out.append(dict(a=a, e=e, kind='fremd', name=alias(f'g_{a:04x}')))
        else: out.append(dict(a=a, e=e, kind='luecke', name=f'g_{a:04x}'))
    return out
# Deklarationen finden
H = open(R + '/names/inc/s_data.h', encoding='latin1').read()
# Kompatibilitaet alter Namen (#define alt neu in s_proto.h/s_data.h, z. B. Welle KS: g_c6c6 -> g_tamper_count)
ALIAS = {}
for f_ in ('s_proto.h', 's_data.h'):
    for m_ in re.finditer(r'^#define\s+(\w+)\s+([A-Za-z_]\w*)\s*$', open(R + '/names/inc/' + f_, encoding='latin1').read(), re.M):
        ALIAS[m_.group(1)] = m_.group(2)
def alias(n):
    for _ in range(5): n = ALIAS.get(n, n)
    return n
H += ''.join(l for f in ('s_class.h', 's_struct.h') for l in open(R + '/names/inc/' + f, encoding='latin1') if l.startswith('extern'))
EXT_RE = re.compile(r'^[ \t]*extern\s+([^;(){}]*?);[^\n]*\n?', re.M)
def split_decl(stmt):
    """'char far *a[], b[3]' -> (Typ, [(name, ptr, dims), ...])"""
    m = re.match(r'\s*((?:(?:const|volatile|signed|unsigned|struct|union|enum|far|near|huge)\s+)*\w+)\s*(.*)$', stmt, re.S)
    if not m: return None, []
    typ, rest = m.group(1), m.group(2); out = []
    for d in rest.split(','):
        x = re.match(r'\s*((?:(?:far|near|huge)\s+)?(?:\*\s*(?:(?:far|near|huge)\s*)?)*)(\w+)\s*((?:\[[^\]]*\])*)\s*$', d)
        if not x: return None, []
        pt = re.sub(r'\s+', ' ', x.group(1)).strip()
        out.append((x.group(2), pt if '*' in pt else '', x.group(3)))
    return typ, out
def decl(n):
    for txt, wo in ((S, 'modul'), (H, 'header')):
        for m in EXT_RE.finditer(txt):
            typ, ds_ = split_decl(m.group(1))
            for nm, ptr, dims in ds_:
                if nm == n:
                    t = (typ + (' ' + ptr if ptr else '')).strip()
                    return t, dims, wo, m
    return None
RAW_RE = re.compile(r'^[ \t]*extern\s+([^;{}]*\(\s*(?:far\s+|near\s+)?\*\s*(\w+)\s*\)[^;{}]*);[^\n]*\n?', re.M)
def decl_raw(n):
    """extern-Anweisung mit genau einem geklammerten Deklarator (void (far *n)(short), T (far *n)[25]) ->
    (Text ohne extern, 'modul'/'header', Match)"""
    isc = src.upper().endswith('.C')
    for txt, wo in ((S, 'modul'), (H, 'header')):
        hits = [m for m in RAW_RE.finditer(txt) if m.group(2) == n and ',' not in m.group(1).split('(')[0]]
        # #ifdef __cplusplus / #else: im C-Modus die Fassung ohne "(...)"-Parameterliste
        if isc: hits = sorted(hits, key=lambda m: '(...)' in m.group(1))
        if hits: return re.sub(r'\s+', ' ', hits[0].group(1)).strip(), wo, hits[0]
    return None
def drop_decl(T, m, names):
    """extern-Anweisung m (aus S) ohne die Deklaratoren names; leer -> Zeile entfaellt"""
    full = m.group(0); typ, ds_ = split_decl(m.group(1))
    rest = [d for d in ds_ if d[0] not in names]
    if not rest: return T.replace(full, '', 1)
    nl = '\n' if full.endswith('\n') else ''
    ind = re.match(r'[ \t]*', full).group(0)
    neu = ind + 'extern ' + typ + ' ' + ', '.join(p + n + d for n, p, d in rest) + ';' + full[full.index(';') + 1:]
    return T.replace(full, neu, 1)
def isstr(b):
    return b and b[-1] == 0 and all(32 <= c < 127 or c in (9, 10, 13) or c >= 0x80 for c in b[:-1]) and 0 not in b[:-1]
def cstr(b):
    out = ''
    for c in b:
        if c == 10: out += '\\n'
        elif c == 13: out += '\\r'
        elif c == 9: out += '\\t'
        elif c == 34: out += '\\"'
        elif c == 92: out += '\\\\'
        elif 32 <= c < 127: out += chr(c)
        else: out += '\\x%02x' % c
    return '"' + re.sub(r'(\\x[0-9a-f]{2})(?=[0-9a-fA-F])', r'\1""', out) + '"'
def uses_in_src(n):
    body = re.sub(r'/\*.*?\*/', lambda m: ' ' * len(m.group(0)), S, flags=re.S)
    body = re.sub(r'//[^\n]*', '', body)
    return [m.start() for m in re.finditer(r'\b' + re.escape(n) + r'\b', body) if not re.match(r'[^\n]*extern', body[body.rfind('\n', 0, m.start()) + 1:m.start()])]
# --- Strukturen (names/inc + Modul): Felder, Groessen, Initialisierer aus Bytes ---
class NichtAuto(Exception): pass
def _strip(t): return re.sub(r'//[^\n]*', '', re.sub(r'/\*.*?\*/', ' ', t, flags=re.S))
_STX = None
SCHWEIF = re.search(r'#include\s+"schweif.h"', S) is not None
SRC_STRUCTS = set(); HDR_TXT = {}; NEED_STRUCT = []          # Strukturen aus names/inc, die ein Modul ohne schweif.h braucht
def _scan(txt, into, raw=None):
    for m in re.finditer(r'\bstruct\s+(\w+)\s*\{', txt):
        d = 0; j = m.end() - 1
        while True:
            if txt[j] == '{': d += 1
            elif txt[j] == '}':
                d -= 1
                if d == 0: break
            j += 1
        into[m.group(1)] = txt[m.end():j]
        if raw is not None:
            k = txt.index(';', j); raw[m.group(1)] = txt[m.start():k + 1]
def structs():
    global _STX
    if _STX is None:
        _STX = {}; src = {}
        _scan(''.join(_strip(open(f, encoding='latin1').read()) for f in sorted(glob.glob(R + '/names/inc/s_*.h'))), _STX, HDR_TXT)
        _scan(_strip(S), src); SRC_STRUCTS.update(src); _STX.update(src)
    return _STX
def need(name):
    """Modul ohne schweif.h: Struktur aus names/inc wird gebraucht -> Definition in den Datenblock uebernehmen"""
    if SCHWEIF or name in SRC_STRUCTS or name in NEED_STRUCT: return
    if re.search(r'\bstruct\s+' + name + r'\b', _strip(S).replace('struct ' + name + ' ', 'struct ' + name + ' ')) and False: return
    for ft, fp, fn_, fd in fields(structs()[name]):
        if not fp and isinstance(ft, str):
            mm = re.match(r'struct (\w+)$', re.sub(r'\s+', ' ', ft).strip())
            if mm: need(mm.group(1))
    NEED_STRUCT.append(name)
SC = {'char': (1, False), 'signed char': (1, True), 'unsigned char': (1, False), 'short': (2, True), 'signed short': (2, True),
      'unsigned short': (2, False), 'int': (2, True), 'signed int': (2, True), 'unsigned int': (2, False), 'unsigned': (2, False),
      'signed': (2, True), 'long': (4, True), 'signed long': (4, True), 'unsigned long': (4, False)}
def _nomethods(body):
    """Strukturrumpf ohne Praeprozessorzeilen, inline-Rumpfe von Elementfunktionen und Elementfunktionen"""
    body = re.sub(r'^[ \t]*#[^\n]*$', '', body, flags=re.M)
    while True:                                  # "... ) { ... }" (inline-Rumpf) -> ");"
        m = re.search(r'\)\s*(?:const\s*)?\{', body)
        if not m: break
        d = 0; j = m.end() - 1
        while True:
            if body[j] == '{': d += 1
            elif body[j] == '}':
                d -= 1
                if d == 0: break
            j += 1
        body = body[:m.start() + 1] + ';' + body[j + 1:]
    out = []
    for st in body.split(';'):
        if '(' in st and not re.search(r'\(\s*(?:far\s+|near\s+)?\*', st): continue   # Elementfunktion
        if re.match(r'\s*(static|typedef|public|private|protected)\b', st): continue
        out.append(st)
    return ';'.join(out)
def fields(body):
    """[(typ, ptr, name, dims)]; typ 'struct x' oder Skalar oder ('anon', body)"""
    out = []; i = 0; body = _nomethods(body).strip()
    while i < len(body):
        while i < len(body) and body[i] in ' \t\r\n': i += 1
        if i >= len(body): break
        m = re.match(r'struct\s*\{', body[i:])
        if m:
            d = 0; j = i + m.end() - 1
            while True:
                if body[j] == '{': d += 1
                elif body[j] == '}':
                    d -= 1
                    if d == 0: break
                j += 1
            inner = body[i + m.end():j]; k = body.index(';', j)
            for dcl in body[j + 1:k].split(','):
                x = re.match(r'\s*(\w+)\s*((?:\[[^\]]*\])*)\s*$', dcl)
                if not x: raise NichtAuto('anonyme Struktur ' + dcl)
                out.append((('anon', inner), '', x.group(1), x.group(2)))
            i = k + 1; continue
        k = body.index(';', i) if ';' in body[i:] else len(body); st = body[i:k]; i = k + 1
        if not st.strip(): continue
        st = re.sub(r'^\s*(public|private|protected)\s*:', '', st)
        if ':' in st or '(' in st or 'union' in st: raise NichtAuto('Bitfeld/Union/Funktion: ' + st.strip()[:60])
        typ, ds_ = split_decl(st)
        if typ is None: raise NichtAuto('Feld nicht lesbar: ' + st.strip()[:60])
        for nm, ptr, dims in ds_: out.append((typ, ptr, nm, dims))
    return out
_CONSTS = None
def consts():
    """Zahlkonstanten (#define/enum) aus names/inc und dem Modul fuer Feldgroessen wie [NR_SKILLS]"""
    global _CONSTS
    if _CONSTS is None:
        _CONSTS = {}
        for t in [open(f, encoding='latin1').read() for f in sorted(glob.glob(R + '/names/inc/s_*.h'))] + [S]:
            for m in re.finditer(r'^#define\s+(\w+)\s+\(?(-?(?:0x[0-9a-fA-F]+|\d+))\)?\s*(?:/\*.*)?$', t, re.M):
                _CONSTS.setdefault(m.group(1), int(m.group(2), 0))
            for m in re.finditer(r'\b([A-Z][A-Z0-9_]+)\s*=\s*(-?(?:0x[0-9a-fA-F]+|\d+))\b', t):
                _CONSTS.setdefault(m.group(1), int(m.group(2), 0))
    return _CONSTS
def dimlist(dims):
    out = []
    for x in re.findall(r'\[([^\]]*)\]', dims):
        try: out.append(int(eval(x, {'__builtins__': {}}, consts())))
        except Exception: raise NichtAuto('Arraygroesse ' + x)
    return out
def tsize(typ, ptr=''):
    if ptr: return 4 if 'near' not in ptr else 2
    if isinstance(typ, tuple): return sum(tsize(t, p) * prod(dimlist(d)) for t, p, n, d in fields(typ[1]))
    t = re.sub(r'\s+', ' ', typ.replace('far', '').replace('const', '')).strip()
    if t in SC: return SC[t][0]
    if t.startswith('enum'): return 2
    m = re.match(r'struct (\w+)$', t)
    if m and m.group(1) in structs():
        need(m.group(1))
        return sum(tsize(ft, fp) * prod(dimlist(fd)) for ft, fp, fn_, fd in fields(structs()[m.group(1)]))
    raise NichtAuto('Typ ' + t)
def prod(l):
    r = 1
    for x in l: r *= x
    return r
def typed(n):
    """(t, ptr, dims-Liste) aus der Deklaration von n oder None"""
    d = decl(n)
    if d is None: return None
    t = re.sub(r'\s+', ' ', d[0]).strip(); ptr = ''
    mm = re.match(r'(.*?)\s*((?:far\s*)?\*.*)$', t)
    if mm and '*' in t: t, ptr = mm.group(1).strip(), mm.group(2).strip()
    if re.fullmatch(r'\w+_PTR', t): return None
    dl = re.findall(r'\[([^\]]*)\]', d[1])
    if any(not x.strip() for x in dl): return None
    return t, ptr, dimlist(d[1])
INNEN = []                                    # (Objekt, Innen-Public, Versatz, Art) - Namen, die jetzt Teil eines Objekts sind
_CLS = None
def has_ctor(cn):
    """Klasse/Struktur cn (names/inc oder Modul) hat einen Konstruktor oder Destruktor"""
    global _CLS
    if _CLS is None:
        _CLS = set()
        for t_ in [open(f, encoding='latin1').read() for f in sorted(glob.glob(R + '/names/inc/s_*.h'))] + [S]:
            names = set(re.findall(r'\b(?:class|struct)\s+(\w+)\s*(?::[^{;]*)?\{', t_))
            for m_ in re.finditer(r'^[ \t]*~?(\w+)\s*\([^;{]*\)\s*[;{]', t_, re.M):
                if m_.group(1) in names: _CLS.add(m_.group(1))
    return cn in _CLS
def merge_typed(items):
    """Objekt mit bekanntem Typ, dessen Groesse ueber folgende namenlose Luecken reicht (Zeigerziele auf seine Felder,
    z. B. &g_blaster_desc.io), mit diesen zusammenfassen"""
    i = 0
    while i < len(items):
        o = items[i]
        if o['kind'] in ('name', 'fremd'):
            try:
                ty = typed(o['name'])
                size = tsize(ty[0], ty[1]) * prod(ty[2]) if ty else 0
            except (NichtAuto, KeyError, AttributeError, ValueError): size = 0
            if size > o['e'] - o['a']:
                j = i + 1
                while j < len(items) and items[j]['kind'] in ('luecke', 'name', 'fremd') and items[j]['e'] <= o['a'] + size: j += 1
                if j < len(items) and items[j]['kind'] in ('luecke', 'name', 'fremd') and items[j]['a'] < o['a'] + size < items[j]['e']:
                    # Folgeobjekt nur teilweise ueberdeckt (Zeigerziel/Innen-Public am Ende): Rest als namenlose Luecke
                    x = items[j]; rest = dict(a=o['a'] + size, e=x['e'], kind='luecke', name=f"g_{o['a'] + size:04x}")
                    items[j] = dict(x, e=o['a'] + size); items.insert(j + 1, rest); j += 1
                if j > i + 1 and items[j - 1]['e'] == o['a'] + size:
                    inner = [x for x in items[i + 1:j] if x['kind'] in ('name', 'fremd')]
                    if inner: INNEN.extend((o['name'], x['name'], x['a'] - o['a'], x['kind']) for x in inner)
                    o['e'] = items[j - 1]['e']; del items[i + 1:j]
        i += 1
    return items
def fieldpath(t, ptr, dims, off):
    """'.feld[k]...' fuer den Versatz off in einem Objekt vom Typ (t, ptr, dims) oder None"""
    if dims:
        w = tsize(t, ptr) * prod(dims[1:]); k = off // w
        r = fieldpath(t, ptr, dims[1:], off - k * w)
        return None if r is None else f'[{k}]' + r
    if off == 0: return ''
    if ptr: return None
    if isinstance(t, tuple): fl = fields(t[1])
    else:
        mm = re.match(r'struct (\w+)$', re.sub(r'\s+', ' ', t.replace('far', '')).strip())
        if not mm or mm.group(1) not in structs(): return None
        fl = fields(structs()[mm.group(1)])
    q = 0
    for ft, fp, fn_, fd in fl:
        n = tsize(ft, fp) * prod(dimlist(fd))
        if q <= off < q + n:
            r = fieldpath(ft, fp, dimlist(fd), off - q)
            return None if r is None else '.' + fn_ + r
        q += n
    return None
a0, a1 = own['DATA'] if own['DATA'] else (0, 0)
lst = objlist(a0, a1, lit_have) if own['DATA'] else []
blst = objlist(*own['BSS'], []) if own['BSS'] else []
merge_typed(lst); merge_typed(blst)
# Pad am Bereichsende: ungerades Ende -> Fuellbyte gehoert TLINK
if len(lst) > 1 and lst[-1]['kind'] == 'luecke' and lst[-1]['e'] - lst[-1]['a'] == 1 and lst[-1]['a'] % 2 == 1 and byt(lst[-1]['a'], lst[-1]['e']) == b'\0':
    lst.pop()
elif lst and lst[-1]['kind'] != 'lit_alt':
    l = lst[-1]
    if (l['e'] - l['a']) > 1 and byt(l['e'] - 1, l['e']) == b'\0' and byt(l['e'] - 2, l['e'] - 1) == b'\0': l['pad'] = 1
# 2. Literal-Schwanz
_dp = items_ds.dptr()
ptcount = collections.Counter(o for p_, o in _dp.items() if a0 <= p_ < a1)
for l in lst:
    if l['kind'] == 'luecke' and ptcount.get(l['a']) == 1:
        b = byt(l['a'], l['e'] - l.get('pad', 0))
        if isstr(b) and not any(l['a'] <= p_ < l['e'] for p_ in _dp): l['ptrlit'] = True; l['lit_ok'] = True
    if l['kind'] != 'name': continue
    b = byt(l['a'], l['e'] - l.get('pad', 0))
    d = decl(l['name']); l['decl'] = d
    u = uses_in_src(l['name'])
    l['lit_ok'] = (not GLOB and d is not None and re.fullmatch(r'(signed |unsigned )?char', d[0]) is not None and
                   isstr(b) and len(u) == 1 and uses['_' + l['name']] == 1 and offuse['_' + l['name']] == 0
                   and l['a'] not in ptcount)
    l['use'] = u[0] if len(u) == 1 else None
k = len(lst)
while k > 0 and (lst[k - 1]['kind'] == 'lit_alt' or lst[k - 1].get('lit_ok')): k -= 1
# Reihenfolge des Auftretens im Schwanz pruefen (nur umgesetzte Namen)
tail = [l for l in lst[k:] if l['kind'] == 'name']
pos = [l['use'] for l in tail]
if pos != sorted(pos):
    print('HINWEIS: Literal-Reihenfolge im Quelltext passt nicht zur Adressreihenfolge -> nur Definitionen'); k = len(lst)
    while k > 0 and lst[k - 1]['kind'] == 'lit_alt': k -= 1
    tail = []
def init(typ, ptr, dims, b, adr=None):
    """Initialisierer fuer Objekt (typ, ptr, dims-Liste) aus Bytes b ab DS-Adresse adr"""
    if dims:
        n = dims[0]; w = len(b) // n
        if not ptr and not isinstance(typ, tuple) and re.sub(r'\s+', ' ', typ).strip() in ('char', 'unsigned char', 'signed char') and len(dims) == 1:
            if 0 in b and isstr(b[:b.index(0) + 1]) and not any(b[b.index(0):]): return cstr(b[:b.index(0)])
        el = [init(typ, ptr, dims[1:], b[i * w:(i + 1) * w], None if adr is None else adr + i * w) for i in range(n)]
        while el and len(el) > 1 and el[-1] in ('0', '{ 0 }') and False: el.pop()
        return '{ ' + ', '.join(el) + ' }'
    if ptr:
        if not any(b): return '0'
        if adr is not None and adr in DPTR and len(b) == 4: return ptr_expr(DPTR[adr], typ, ptr)
        raise NichtAuto('Zeiger mit Wert ' + b.hex() + (f' bei ds:{adr:04x}' if adr is not None else ''))
    if isinstance(typ, tuple): fl = fields(typ[1])
    else:
        t = re.sub(r'\s+', ' ', typ.replace('far', '').replace('const', '')).strip()
        if t in SC or t.startswith('enum'):
            w, sg = SC.get(t, (2, True)); return str(int.from_bytes(b, 'little', signed=sg)) if sg else '0x%0*x' % (2 * w, int.from_bytes(b, 'little'))
        fl = fields(structs()[re.match(r'struct (\w+)$', t).group(1)])
    out = []; q = 0
    for ft, fp, fn_, fd in fl:
        dl = dimlist(fd); n = tsize(ft, fp) * prod(dl)
        out.append(init(ft, fp, dl, b[q:q + n], None if adr is None else adr + q)); q += n
    if q != len(b): raise NichtAuto(f'Strukturgroesse {q} != {len(b)}')
    return '{ ' + ', '.join(out) + ' }'
def struct_def(t, name, dims, b, span, last):
    """Definition fuer Strukturtyp t ('struct x') mit dims ('', '[]', '[n]', '[n][m]') -> (text, rest-bytes)"""
    w = tsize(t); dl_raw = re.findall(r'\[([^\]]*)\]', dims)
    if not dl_raw: cnt = None; size = w
    else:
        inner = dimlist(''.join('[' + x + ']' for x in dl_raw[1:]))
        if dl_raw[0].strip(): cnt = dimlist('[' + dl_raw[0] + ']')[0]
        else:
            per = w * prod(inner); cnt = span // per
            if span % per and not (span % per == 1 and last): raise NichtAuto(f'{span} Bytes kein Vielfaches von {per}')
        size = w * cnt * prod(inner)
    if size > span: raise NichtAuto(f'Typgroesse {size} > Bereich {span}')
    vb = b[:size]
    dl = ([cnt] + dimlist(''.join('[' + x + ']' for x in dl_raw[1:]))) if dl_raw else []
    txt = init(t, '', dl, vb)
    dimtxt = ''.join('[' + x + ']' for x in dl_raw) if dl_raw and dl_raw[0].strip() else (f'[{cnt}]' + ''.join('[' + x + ']' for x in dl_raw[1:]) if dl_raw else '')
    if len(txt) > 100: txt = txt.replace('}, {', '},' + NL + '\t{').replace('{ {', '{' + NL + '\t{', 1)
    if txt.endswith('} }') and NL in txt: txt = txt[:-2] + NL + '}'
    return f'{t} {name}{dimtxt} = {txt};', b[size:]
DPTR = items_ds.dptr()
HADR = {}                                     # ds-Adresse -> Name aus s_data.h-Kommentaren
for m_ in re.finditer(r'^extern\s+[^;]*?\b(\w+)\s*(?:\[[^\]]*\])*\s*;\s*/\*\s*ds:([0-9a-f]{4})\b', H, re.M): HADR.setdefault(int(m_.group(2), 16), m_.group(1))
OBJT = {}                                     # eigene Objekte: Name -> (a, e) (fuer Zeiger auf eigene Daten)
PTRLIT = {}                                   # Adresse -> Literalbytes (Zeigerziel im Literalpool)
EXTRA_EXT = []                                # fremde Zeigerziele ohne Deklaration im Modul
FWD = []                                      # eigene Objekte, auf die vor ihrer Definition gezeigt wird
DONE = set()                                  # schon definierte eigene Objekte
def ptr_expr(o, typ, ptr):
    r = ptr_expr0(o, typ, ptr)
    for n_, (x, y) in OBJT.items():
        if x <= o < y and n_ not in DONE and n_ not in FWD and not decl(n_) and not decl_raw(n_): FWD.append(n_)
    return r
def ptr_expr0(o, typ, ptr):
    ft = (typ if isinstance(typ, str) else 'void') + ' ' + ptr
    ft = re.sub(r'\s+', ' ', ft).strip()
    if o in PTRLIT: return cstr(PTRLIT[o])
    for n_, (x, y) in OBJT.items():
        if x <= o < y:
            try: ty = typed(n_)
            except (NichtAuto, KeyError, AttributeError, ValueError): ty = None
            fp = fieldpath(*ty, o - x) if ty else None
            if fp: return f'({ft})&{n_}{fp}'
            return f'({ft})&{n_}' if o == x else f'({ft})((char far *)&{n_} + {o - x})'
    if a0 <= o < a1: raise NichtAuto(f'Zeiger auf ds:{o:04x} (eigener Bereich, kein Objekt/Literal)')
    if o in HADR: n_ = HADR[o]; x = o
    else:
        mp = sorted(n_[1:] for n_, v_ in alln.items() if v_ == (CD.DG, o) and n_.startswith('_'))
        if mp: n_ = mp[0]; x = o
        else:
            ks = [x_ for x_ in HADR if x_ <= o]
            if not ks: raise NichtAuto(f'Zeiger auf ds:{o:04x}: kein Name')
            x = max(ks); n_ = HADR[x]
    if not re.search(r'\b' + re.escape(n_) + r'\b', _strip(S)) and (not SCHWEIF or n_ not in HADR.values()):
        if n_ not in EXTRA_EXT: EXTRA_EXT.append(n_)
    return f'({ft})&{n_}' if o == x else f'({ft})((char far *)&{n_} + {o - x})'
# 3. Definitionen
def flat(t, name, dimtxt, vals_txt):
    return f"{t} {name}{dimtxt} = {vals_txt};"
def fmt_init(txt):
    if len(txt) <= 110 or txt.startswith('"'): return txt
    if txt.startswith('{ {'):
        txt = txt.replace('}, {', '},' + NL + '\t{').replace('{ {', '{' + NL + '\t{', 1)
        return txt[:-2] + NL + '}' if txt.endswith('} }') else txt
    parts = txt[2:-2].split(', ')
    rows = [', '.join(parts[i:i + 16]) for i in range(0, len(parts), 16)]
    return '{' + NL + '\t' + (',' + NL + '\t').join(rows) + NL + '}'
PTRS = None
def define(l, isbss, last):
    """-> [Definitionstexte]; wirft NichtAuto"""
    global PTRS
    a, e = l['a'], l['e']; span = e - a; b = None if isbss else byt(a, e)
    if l['kind'] == 'hand':
        for n_ in l['names']:
            d_ = decl(n_) or decl_raw(n_)
            if d_ and d_[-2] == 'modul': remove.append((d_[-1], n_))
        return list(l['text'])
    if PTRS is None: PTRS = sorted(items_ds.cptr())
    if not isbss and any(a <= p_ < e for p_ in PTRS):
        raise NichtAuto('Codezeiger im Bereich - Zeigertabelle von Hand')
    if not isbss and l['kind'] == 'luecke' and any(a <= p_ < e for p_ in DPTR):
        raise NichtAuto('DGROUP-Zeiger in unbenannter Luecke')
    name = l['name']; d = decl(name) if l['kind'] != 'luecke' else None
    if d is None and isbss and l['kind'] != 'luecke':
        # Deklaratoren mit Klammern (Funktionszeiger, Zeiger auf Array): BSS braucht keinen Initialisierer ->
        # extern-Zeile aus Modul bzw. s_data.h ohne "extern" uebernehmen (Groesse muss zum Bereich passen)
        r = decl_raw(name)
        if r is not None:
            txt, wo, m = r
            if wo == 'modul': remove.append((m, name))
            if wo == 'header' and not SCHWEIF: return [f"char {name}[{span}];   /* ds:{a:04x}, Typ in s_data.h: {txt} */"]
            out_ = [f"{txt};"]
            if re.search(r'\(\s*far\s*\*\s*\w+\s*\)', txt) and span > 4:      # Fernzeiger, dahinter ungenutzter Rest
                out_.append(f"char g_{a + 4:04x}[{span - 4}];   /* ds:{a + 4:04x}, unbenutzt */")
            return out_
    if d is None:
        if l['kind'] == 'name': raise NichtAuto('keine extern-Deklaration')
        if isbss: return [f"char {name}[{span}];   /* ds:{a:04x}, {'unbenutzt' if l['kind'] == 'luecke' else 'nur ' + l['kind']} */"]
        wer = 'Zeigerziel' if a in TARGETS else 'unbenutzt'
        if isstr(b) or (len(b) > 1 and b[-1] == 0 and isstr(b[:-1]) and last):
            z = b if isstr(b) else b[:-1]
            return [f"char {name}[{span}] = {cstr(z[:-1])};   /* ds:{a:04x}, {wer} */"]
        return [f"char {name}[{span}] = {fmt_init('{ ' + ', '.join('0x%02x' % c for c in b) + ' }')};   /* ds:{a:04x}, {wer} */"]
    typ, dims, wo, m = d
    if wo == 'modul': remove.append((m, name))
    if wo == 'header' and not SCHWEIF:
        # Modul ohne schweif.h: Name aus s_data.h, Typ dort - hier als Bytes (Publics/Verweise bleiben gleich)
        tt = re.sub(r'\s+', ' ', typ).strip() + dims
        if isbss: return [f"char {name}[{span}];   /* ds:{a:04x}, Typ in s_data.h: {tt} */"]
        if any(a <= p_ < e for p_ in DPTR): raise NichtAuto('Zeiger in Objekt ohne Typ (Modul ohne schweif.h)')
        return [f"char {name}[{span}] = {fmt_init('{ ' + ', '.join('0x%02x' % c for c in b) + ' }')};   /* ds:{a:04x}, Typ in s_data.h: {tt} */"]
    t = re.sub(r'\s+', ' ', typ).strip(); ptr = ''
    mm = re.match(r'(.*?)\s*((?:far\s*)?\*.*)$', t)
    if mm and '*' in t: t, ptr = mm.group(1).strip(), mm.group(2).strip()
    cm = re.fullmatch(r'(?:struct |class )?(\w+)', t)
    if not ptr and cm and has_ctor(cm.group(1)):
        # Objekt einer Klasse mit Konstruktor: als Objekt definiert, erzeugte BCC einen statischen Initialisierer
        # (Konstruktoraufruf) - hier nur der Speicher; konstruiert wird es von Hand (globals_ctor)
        # Speicher unter dem Adressnamen; Zugriff ueber die typisierte extern-Deklaration (MAP-Public)
        tt = t + dims; an = f'g_{a:04x}'
        if wo == 'modul': raise NichtAuto(f'Objekt mit Konstruktor {name} im Modul deklariert - von Hand')
        if isbss: return [f"char {an}[{span}];   /* ds:{a:04x}, Speicher von {name} ({tt}, Konstruktor in globals_ctor) */"]
        return [f"char {an}[{span}] = {fmt_init('{ ' + ', '.join('0x%02x' % c for c in b) + ' }')};   /* ds:{a:04x}, Speicher von {name} ({tt}, Konstruktor in globals_ctor) */"]
    if re.fullmatch(r'\w+_PTR', t): ptr = '*'; tdisp = t
    else: tdisp = None
    try: w = tsize(t, ptr)
    except NichtAuto as x_:
        if not isbss: raise
        return [f"{(tdisp or (t + ' ' + ptr)).strip()}{'' if ptr.endswith('*') else ' '}{name}{dims};   /* ds:{a:04x}, {span} Bytes (Groesse von dscheck geprueft) */"]
    dl_raw = re.findall(r'\[([^\]]*)\]', dims)
    inner = dimlist(''.join('[' + x + ']' for x in dl_raw[1:])) if dl_raw else []
    if dl_raw and dl_raw[0].strip(): cnt = dimlist('[' + dl_raw[0] + ']')[0]
    elif dl_raw:
        per = w * prod(inner); cnt = span // per
        if cnt == 0: raise NichtAuto(f'Elementgroesse {per} > Bereich {span}')
    else: cnt = None
    size = w * (cnt if cnt is not None else 1) * prod(inner)
    if size > span: raise NichtAuto(f'Typgroesse {size} > Bereich {span}')
    dimtxt = (f'[{dl_raw[0].strip() or cnt}]' + ''.join('[' + x + ']' for x in dl_raw[1:])) if dl_raw else ''
    full = tdisp or (t + ' ' + ptr).strip()
    sep = '' if full.endswith('*') else ' '
    if isbss: out = [f"{full}{sep}{name}{dimtxt};"]
    else:
        dl = ([cnt] + inner) if dl_raw else []
        out = [f"{full}{sep}{name}{dimtxt} = {fmt_init(init(t, ptr, dl, b[:size], a))};"]
    rest = span - size
    if rest and not isbss and any(e - rest <= p_ < e for p_ in DPTR):
        raise NichtAuto(f'Typgroesse {size} < Bereich {span} und im Rest liegen DGROUP-Zeiger (Arraygroesse in der Deklaration pruefen)')
    if rest and not (rest == 1 and last and (isbss or b[-1] == 0)):
        ra = e - rest; rn = alias(f'g_{ra:04x}'); d2 = decl(rn)
        if d2 and not d2[1]:                      # Rest hat schon einen Namen/Typ (s_data.h)
            try:
                t2 = re.sub(r'\s+', ' ', d2[0]).strip()
                if tsize(t2) == rest:
                    out.append(f"{t2} {rn};" if isbss else f"{t2} {rn} = {init(t2, '', [], b[size:], ra)};")
                    return out
            except (NichtAuto, KeyError, AttributeError, ValueError): pass
        out.append(f"char g_{ra:04x}[{rest}];   /* ds:{ra:04x}, unbenutzt */" if isbss else
                   f"char g_{ra:04x}[{rest}] = {{ {', '.join('0x%02x' % c for c in b[size:])} }};   /* ds:{ra:04x}, unbenutzt */")
    return out
for l in lst[k:]:
    if l.get('ptrlit'): PTRLIT[l['a']] = byt(l['a'], l['e'] - l.get('pad', 0))[:-1]
for l in lst[:k] + blst:
    if l['kind'] != 'lit_alt': OBJT[l['name']] = (l['a'], l['e'])
defs = []; bdefs = []; fehler = []; remove = []
TARGETS = set(o for p_, o in items_ds.dptr().items())
for i, l in enumerate(lst[:k]):
    if l['kind'] == 'lit_alt': fehler.append(f"vorhandenes Literal {l['a']:#x} liegt vor Definitionen - Quelltext pruefen"); continue
    DONE.add(l['name'])
    try: defs += define(l, False, i == len(lst) - 1)
    except (NichtAuto, KeyError, AttributeError, ValueError) as x:
        fehler.append(f"{l['name']}: nicht automatisch: {x} ({l['e'] - l['a']} Bytes ds:{l['a']:04x}: {byt(l['a'], l['e'])[:16].hex()})")
for i, l in enumerate(blst):
    try: bdefs.append(define(l, True, i == len(blst) - 1))
    except (NichtAuto, KeyError, AttributeError, ValueError) as x:
        fehler.append(f"{l['name']}: BSS nicht automatisch: {x} ({l['e'] - l['a']} Bytes ds:{l['a']:04x})")
print(f'== {mod} {fn} DATA {own["DATA"] and [hex(x) for x in own["DATA"]]} BSS {own["BSS"] and [hex(x) for x in own["BSS"]]}  Definitionen: {len(lst[:k])}+{len(blst)} BSS  Literale: {len(tail)} (+{len([l for l in lst[k:] if l["kind"] == "lit_alt"])} vorhandene)')
for l in lst + blst:
    print(f"  {l['a']:#06x}..{l['e']:#06x} {l['kind']:<7} {l.get('name', ''):<28} {'LITERAL' if l in tail else ('ZEIGERLITERAL' if l.get('ptrlit') and l in lst[k:] else '')} {byt(l['a'], l['e'])[:24]!r}")
for o_, n_, v_, k_ in INNEN: print(f'  INNEN {n_} = {o_} + {v_:#x} ({k_})')
if fehler:
    print('NICHT AUTOMATISCH:'); [print('  ' + f) for f in fehler]; sys.exit(1)
blk = []
if NEED_STRUCT:
    blk += ['/* Phase B: Strukturen aus names/inc (Modul ohne schweif.h) */'] + [HDR_TXT[n_] for n_ in NEED_STRUCT]
if EXTRA_EXT:
    blk += ['extern char ' + ', '.join(n_ + '[]' for n_ in EXTRA_EXT) + ';   /* Zeigerziele in fremden Daten */']
if FWD:
    blk += ['/* Zeigerziele, die erst weiter unten definiert werden */',
            'extern char ' + ', '.join(n_ + '[]' for n_ in FWD) + ';']
if defs or tail or lst[k:]:
    blk += [f'/* Phase B: eigene Daten des Moduls (ds:{a0:04x}..{a1:04x}, dsana) - initialisierte Daten in Adressreihenfolge'
            + (', danach der Literalpool */' if tail or lst[k:] else ' */')] + defs
# BCC im C-Modus (.C) legt nicht initialisierte Globale in umgekehrter Reihenfolge ihrer ERSTEN Deklaration an
# (auch der in s_data.h) -> vor dem ersten #include extern-Deklarationen in absteigender Adressfolge
ISC = src.upper().endswith('.C')
pre = ['extern ' + x.split('   /*')[0] for g_ in reversed(bdefs) for x in reversed(g_)] if ISC and bdefs else []
bdefs = [x for g_ in bdefs for x in g_]
if bdefs:
    blk += [f'/* Phase B: eigenes BSS (ds:{own["BSS"][0]:04x}..{own["BSS"][1]:04x}) in Adressreihenfolge' + (' (Reihenfolge festgelegt durch die extern-Zeilen vor den #include)' if pre else '') + ' */'] + bdefs
print(NL.join(blk))
for l in tail: print(f"  Literal {cstr(byt(l['a'], l['e'] - l.get('pad', 0))[:-1])} statt {l['name']}")
if not WRITE: sys.exit(0)
T = S
for l in sorted(tail, key=lambda l: -l['use']):
    lit = cstr(byt(l['a'], l['e'] - l.get('pad', 0))[:-1]); p = l['use']
    assert T[p:p + len(l['name'])] == l['name']; T = T[:p] + lit + T[p + len(l['name']):]
    d = l.get('decl')
    if d and d[2] == 'modul': remove.append((d[3], l['name']))
# Definitionsblock: an die Stelle der ersten betroffenen extern-Zeile, sonst hinter das letzte #include
ins = None; grp = collections.OrderedDict()
for m, n in remove: grp.setdefault(m.group(0), (m, set()))[1].add(n)
for full, (m, ns) in grp.items():
    i = T.find(full)
    if i >= 0 and (ins is None or i < ins): ins = i
for full, (m, ns) in grp.items(): T = drop_decl(T, m, ns)
if blk:
    if ins is None:
        ms = list(re.finditer(r'^#include[^\n]*\n', T, re.M)); ins = ms[-1].end() if ms else 0
        T = T[:ins] + NL + NL.join(blk) + NL + T[ins:]
    else: T = T[:ins] + NL.join(blk) + NL + T[ins:]
if pre:
    i = T.index('#include')
    T = T[:i] + '/* Phase B: BCC (C-Modus) legt das BSS in umgekehrter Reihenfolge der ersten Deklaration an - Reihenfolge hier vor schweif.h */' + NL + NL.join(pre) + NL + T[i:]
open(src, 'w', newline='', encoding='latin1').write(T)
print('geschrieben:', src)
