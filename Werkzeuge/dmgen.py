#!/usr/bin/env python3
"""dmgen.py - Phase A: je Originalmodul ein Datenstueck lnk/DM<modul>.ASM (_DATA + _BSS [+ _INIT_/_EXIT_])
und die verzahnte Linkreihenfolge lnk/FULL.RES (Residente und Overlays wie in Attics IDE-Projekt,
jedes Stueck direkt hinter seinem Modul). Nachfolger von dssplit.py/DSBSS (Grundlage: dsana/dsmod.py).

Quellen: dsg/DS00..05 + DSBSS (Bytes, Labels, Zeiger), dsana/zuordnung.json (Modulbereiche),
dsana/ds_linkorder.tsv (Linkreihenfolge). Reihenfolge der DGROUP-Relocations je Stueck kommt direkt
aus der MZ-Tabelle des Originals (lnk/dm.json fuer tools/objorder.py).

Phase B: Module mit Spalte D in c_module.tsv definieren ihre Daten selbst (C-OBJ behaelt _DATA/_BSS), mit Spalte DI
auch ihre _INIT_/_EXIT_-Eintraege (echtes globales C++-Objekt bzw. #pragma startup/exit).
Fuer sie entsteht kein Stueck. Eine Datei dsana/dm_rest.tsv (modul<TAB>kind<TAB>von<TAB>bis) kann Teile
eines D-Moduls weiter im Stueck halten (Reststueck), z. B. vorn liegende Globale aus unsicherer Zone.

Aufruf: dmgen.py [--dmodule O068,O106,...]   (Default: Spalte D aus c_module.tsv)"""
import sys, json, os, re, struct, collections
R = '/root/sternenschweif'; L = R + '/lnk/'; D = R + '/dsana/'
sys.path.insert(0, D); import items as IT
DGROUP_PARA = 0x24EB
INIT_BASE, EXIT_BASE = 0x8b06, 0x8b42
INIT_IDX = {'O008': 0, 'O032': 1, 'O224': 2}; EXIT_IDX = {'O032': 0}
# Kopierschutzmodule R01/R03/R23: seit Welle KS C-Module, die Sperre "Daten bleiben im ASM-Stueck" ist aufgehoben
SCHUTZ = set()

def dmodule():
    if '--dmodule' in sys.argv:
        v = sys.argv[sys.argv.index('--dmodule') + 1]
        return set(x for x in v.split(',') if x)
    s = set()
    for l in open(R + '/c_module.tsv'):
        if l.startswith('#') or not l.strip(): continue
        f = l.rstrip('\r\n').split('\t')
        if len(f) >= 3 and f[2].strip() in ('D', 'DI'): s.add(f[0] if f[0].startswith('O') else 'R%02d' % int(f[0]))
    return s

ITS = IT.load()
sys.path.insert(0, R + '/tools'); import dsinfo
INIT, EXIT = dsinfo.INIT_FN, dsinfo.EXIT_FN               # Satz 0,1,2 in _INIT_, Satz 0 in _EXIT_
DI = dsinfo.own_initexit()                                 # Spalte DI: C-OBJ liefert _INIT_/_EXIT_ selbst
Z = dsinfo.zuordnung()                        # zuordnung.json + dsana/korrekturen.tsv
rng = collections.defaultdict(dict)
for kind in ('DATA', 'BSS'):
    for a, e, m in Z[kind]:
        r = rng[m].get(kind)
        if r is not None and r[1] != a: raise SystemExit(f'{m} {kind}: Bereich nicht zusammenhaengend')
        rng[m][kind] = [a, e] if r is None else [r[0], e]
DM = dmodule() - SCHUTZ
REST = collections.defaultdict(list)          # (modul) -> [(kind, von, bis)]
if os.path.exists(D + 'dm_rest.tsv'):
    for l in open(D + 'dm_rest.tsv'):
        if l.startswith('#') or not l.strip(): continue
        m, k, a, b = l.split()[:4]; REST[m].append((k, int(a, 16), int(b, 16)))

def owner_of(p):
    for m, r in rng.items():
        a, e = r.get('DATA', (0, 0))
        if a <= p < e: return m
    return None

# --- Reihenfolge der DGROUP-Relocations aus der MZ-Tabelle ---
e = open(R + '/orig/SCHWEIF.EXE', 'rb').read()
nr, ro = struct.unpack_from('<H', e, 6)[0], struct.unpack_from('<H', e, 0x18)[0]
mzorder = collections.defaultdict(list)       # modul -> [(seg, pos relativ)]
for i in range(nr):
    o, s = struct.unpack_from('<HH', e, ro + 4 * i)
    if s != DGROUP_PARA: continue
    if 0x94 <= o < 0x86b6:
        m = owner_of(o); mzorder[m].append(('_DATA', o))
    elif INIT_BASE <= o < EXIT_BASE:
        k = (o - INIT_BASE - 4) // 6
        m = next((x for x, v in INIT_IDX.items() if v == k), None)
        if m: mzorder[m].append(('_INIT_', 4))
    elif EXIT_BASE <= o < 0x8b4e:
        k = (o - EXIT_BASE - 4) // 6
        m = next((x for x, v in EXIT_IDX.items() if v == k), None)
        if m: mzorder[m].append(('_EXIT_', 4))

def body(kind, a0, a1):
    out = []; ext = set()
    for x in ITS:
        if x['kind'] != kind: continue
        s, e = x['addr'], x['addr'] + len(x['data'])
        if e <= a0 or s >= a1: continue
        if s >= a0:
            out.append(f"{x['name']}\tlabel\tbyte\t; {s:#06x}")
            out.append(f"\tpublic\t{x['name']}")
        dp = {p: o for p, o in x['dptr']}; cp = {p: n for p, n in x['cptr']}
        p = max(s, a0); buf = []
        def flush():
            if buf: out.append('\tdb\t' + ','.join('0%02xh' % v for v in buf)); buf.clear()
        while p < min(e, a1):
            if kind == 'BSS':
                n = min(e, a1) - p; out.append(f'\tdb\t{n} dup (?)'); p += n; continue
            if p in dp:
                if p + 4 > a1: raise SystemExit(f'Fernzeiger {p:#x} ueber Stueckgrenze {a1:#x}')
                flush(); out.append(f'\tdw\t0{dp[p]:04x}h,DGROUP'); p += 4; continue
            if p in cp:
                if p + 4 > a1: raise SystemExit(f'Fernzeiger {p:#x} ueber Stueckgrenze {a1:#x}')
                flush(); out.append(f'\tdd\t{cp[p]}'); ext.add(cp[p]); p += 4; continue
            v = x['data'][p - s]
            if v is None: raise SystemExit(f'unbekanntes Byte {p:#x}')
            buf.append(v); p += 1
            if len(buf) == 16: flush()
        flush()
    return out, ext

def piece(m, parts_wanted, pref='DM'):
    """parts_wanted: {'DATA': [a,e] | None, 'BSS': [a,e] | None, 'INIT': bool, 'EXIT': bool}"""
    dr, br = parts_wanted.get('DATA'), parts_wanted.get('BSS')
    segs = ['_DATA', '_BSS'] + (['_INIT_'] if parts_wanted.get('INIT') else []) + (['_EXIT_'] if parts_wanted.get('EXIT') else [])
    h = [f'; {pref}{m}.ASM - Daten des Moduls {m} (DATA {"%#06x-%#06x" % tuple(dr) if dr else "-"}, '
         f'BSS {"%#06x-%#06x" % tuple(br) if br else "-"}){" - REST, Rest liefert die C-OBJ" if pref == "DR" else ""}, erzeugt von tools/dmgen.py',
         '\tifndef ??version', '?debug\tmacro', '\tendm', '\tendif', 'DGROUP\tgroup\t' + ','.join(segs), '\tassume\tds:DGROUP']
    ext = set(); parts = []
    parts.append("_DATA\tsegment word public 'DATA'")
    if dr:
        b, ex = body('DATA', *dr); ext |= ex; parts += b
    parts.append('_DATA\tends')
    parts.append("_BSS\tsegment word public 'BSS'")
    if br: parts += body('BSS', *br)[0]
    parts.append('_BSS\tends')
    for seg, tab, flag in (('_INIT_', INIT, 'INIT'), ('_EXIT_', EXIT, 'EXIT')):
        if parts_wanted.get(flag):
            cls = 'INITDATA' if seg == '_INIT_' else 'EXITDATA'
            parts += [f"{seg}\tsegment word public '{cls}'", '\tdb\t1,32', f'\tdd\t{tab[m]}', f'{seg}\tends']
            ext.add(tab[m])
    # Relocation-Reihenfolge wie in der MZ-Tabelle (_INIT_/_EXIT_ stehen bei BCC hinter den Datenrelocations)
    full = []
    for sg, p in mzorder.get(m, []):
        if sg == '_DATA':
            if dr and dr[0] <= p < dr[1]: full.append(['_DATA', p - dr[0]])
        elif sg == '_INIT_' and parts_wanted.get('INIT'): full.append(['_INIT_', 4])
        elif sg == '_EXIT_' and parts_wanted.get('EXIT'): full.append(['_EXIT_', 4])
    h += [f'\textrn\t{n}:far' for n in sorted(ext)]
    open(L + f'{pref}{m}.ASM', 'w', newline='').write('\r\n'.join(h + parts + ['\tend']) + '\r\n')
    return {'name': f'{pref}{m}', 'modul': m, 'order': full}

# Stufe 1: alle Stuecke DM<m> (reine ASM-Fassung, bytegleich). Stufe 2: D-Module ohne Stueck bzw. mit Reststueck DR<m>.
meta = []; st1 = {}; st2 = {}
for m, r in sorted(rng.items()):
    want = {'DATA': r.get('DATA'), 'BSS': r.get('BSS'), 'INIT': m in INIT, 'EXIT': m in EXIT}
    meta.append(piece(m, want)); st1[m] = f'DM{m}'
    if m in DM:
        # _INIT_/_EXIT_-Eintraege bleiben im Reststueck, ausser die C-OBJ liefert sie selbst (Spalte DI)
        rw = {'DATA': None, 'BSS': None, 'INIT': m in INIT and m not in DI, 'EXIT': m in EXIT and m not in DI}
        for k, a, b in REST.get(m, []): rw[k] = [a, b]
        if any(rw.values()): meta.append(piece(m, rw, 'DR')); st2[m] = f'DR{m}'
    else: st2[m] = f'DM{m}'
json.dump({'stuecke': meta}, open(L + 'dm.json', 'w'), indent=1)

# --- verzahnte Linkreihenfolge ---
order = [l.split('\t')[1] for l in open(D + 'ds_linkorder.tsv').read().split('\n')[1:] if l]
def fname(m): return f'SEG{int(m[1:]):03d}' if m[0] == 'R' else f'OVL{m[1:]}'
def res(stk, fn):
    objs = ['..\\bc\\LIB\\C0L']; ovl = False
    for m in order:
        want = m[0] == 'O'
        if want != ovl: objs.append('/o+' if want else '/o-'); ovl = want
        objs.append(fname(m))
        if m in stk: objs.append(stk[m])
        if m == 'R46': objs += (['/o-'] if ovl else []) + ['INIT', 'FARDATA'] + (['/o+'] if ovl else [])
    lines = []; cur = []
    for o in objs:                               # hoechstens 8 je Zeile, keine Zeile endet mit /o+ oder /o-
        cur.append(o)
        if len(cur) >= 8 and not o.startswith('/'): lines.append(' '.join(cur) + '+'); cur = []
    if cur: lines.append(' '.join(cur) + '+')
    lines[-1] = lines[-1][:-1]
    open(L + fn, 'w', newline='').write('\r\n'.join(lines) + '\r\nSCHW_CD.EXE\r\nSCHW_CD.MAP\r\n..\\bc\\LIB\\OVERLAY ..\\bc\\LIB\\CL\r\n')
    return len(objs)
n1 = res(st1, 'FULL.RES'); n2 = res(st2, 'FULL2.RES')
open(L + 'dm_liste.txt', 'w').write('\n'.join(x['name'] for x in meta) + '\n')
# Selbstpruefung: Stuecke der Stufe 1 ergeben DS lueckenlos
for kind, lo, hi in (('DATA', 0x94, 0x86b6), ('BSS', 0x8b4e, 0xcb76)):
    seq = [tuple(rng[m][kind]) for m in order if kind in rng[m]]
    ok = seq[0][0] == lo and seq[-1][1] == hi and all(a[1] == b[0] for a, b in zip(seq, seq[1:])) and all(a % 2 == 0 for a, _ in seq)
    print(kind, len(seq), 'Bereiche, lueckenlos/geordnet/gerade:', ok)
    if not ok: raise SystemExit('Zuordnung nicht lueckenlos')
nrel = sum(len(x['order']) for x in meta if x['name'].startswith('DM'))
print('Stuecke:', len(st1), ' D-Module:', ' '.join(sorted(DM & set(rng))) or '-', ' Reststuecke:', len([1 for v in st2.values() if v.startswith('DR')]),
      ' Relocations:', nrel, ' FULL.RES:', n1, ' FULL2.RES:', n2)
