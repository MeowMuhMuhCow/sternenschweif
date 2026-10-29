#!/usr/bin/env python3
import os as _swos; _SW = _swos.environ.get('SW_ROOT', '/root/sternenschweif')  # Bau-Paket
"""cobjfix.py C_OBJ ASM_OBJ MAP OUT_OBJ - macht eine BCC-OBJ (echter C-Code) linkfertig fuer den Komplettlink.

Die C-Datei benutzt sprechende Namen, der Rest des Programms die Linknamen (F_R02_018F ...).
Weil der Code der C-OBJ bytegleich zur ASM-Linkfassung ist, liegen alle Fixups an denselben Stellen:
  1. EXTDEF: Ziel einer C-Fixup = Ziel der ASM-Fixup an derselben Stelle -> Name uebernehmen.
  2. Datenverweise (in der ASM-Fassung feste Zahlen ohne Fixup): Adresse = Originalwort - Inline-Wert der
     C-OBJ. Steht dort in DGROUP ein Public mit genau dem Namen -> bleibt. Sonst Umbenennung auf das Public
     an dieser Adresse, notfalls auf das naechstniedrigere Public mit angepasstem Inline-Wert.
  3. PUBDEF: die Linknamen der ASM-Fassung (gleiche Offsets) kommen dazu, die C-Namen bleiben, wenn sie
     nirgends sonst vorkommen.
  4. Leere SEGDEFs (_DATA/_BSS ohne Inhalt) werden byte-ausgerichtet, damit sie im Datensegment kein
     Fuellbyte erzeugen.

Fix-Modus (Fehlerbehebungen per Schalter, names/inc/s_fix.h): cobjfix.py C_OBJ ASM_OBJ MAP OUT_OBJ --ref REF_OBJ
  REF_OBJ = dieselbe Quelle mit FIX_BUGS=0 uebersetzt (bytegleich zur ASM-Fassung). Alle Zuordnungen (Namen,
  Inline-Korrekturen, Publics) werden wie oben an REF_OBJ ueber die Positionen bestimmt und dann ueber die NAMEN
  auf C_OBJ (geaenderter Code, verschobene Positionen) uebertragen:
  - EXTDEF-Umbenennung je Name; Inline-Korrektur eines Datenverweises (naechstniedrigeres Public) als Versatz je
    Name auf alle Fixups dieses Namens in C_OBJ
  - Publics: ASM-Linkname an REF-Offset o -> C-Name an o in REF_OBJ -> dessen Offset in C_OBJ (Overlays: Reihenfolge
    der ASM-Fassung bleibt, also auch die Stub-Einsprungnummern)
  - neue Externe (nur in C_OBJ) ueber die Namenstabelle $CNAMEN (tools/cnamen.py), sonst bleibt der C-Name
  - GRP-Fixup-Reihenfolge (nur fuer Bytegleichheit der MZ-Tabelle) entfaellt
"""
import sys, struct, re, bisect
sys.path.insert(0, _SW + '/tools'); import objfix, objbytes
args = sys.argv[1:]; REF = None
if '--ref' in args:
    i_ = args.index('--ref'); REF = args[i_ + 1]; del args[i_:i_ + 2]
cobj, aobj, mapf, out = args[:4]
ANA = REF or cobj                                 # Positionsanalyse immer an der bytegleichen Fassung
DG = 0x24EB

def idx(b, q):
    v = b[q]; return (((v & 0x7f) << 8) | b[q + 1], q + 2) if v & 0x80 else (v, q + 1)
def rec(t, body):
    r = bytes([t]) + struct.pack('<H', len(body) + 1) + body; return r + bytes([(-sum(r)) & 0xff])

# --- Linksymbole aus der MAP ---
m = open(mapf, encoding='latin1').read()
a, b = m.index('Publics by Name'), m.index('Publics by Value')
allnames = {}
for l in m[a:b].splitlines()[2:]:
    f = l.split()
    if len(f) >= 2 and re.match(r'^[0-9A-F]{4}:[0-9A-F]{4}$', f[0]):
        sg, of = (int(x, 16) for x in f[0].split(':')); allnames[f[-1]] = (sg, of)
dpub = sorted((of, n) for n, (sg, of) in allnames.items() if sg == DG)
daddr = {}
for of, n in dpub: daddr.setdefault(of, n)
dkeys = [x[0] for x in dpub]

C = objfix.parse(ANA); A = objfix.parse(aobj)
cseg = [s for s in C['segs'] if s.endswith('_TEXT')][0]
cb = bytearray(objbytes.seg_bytes(ANA)[cseg]); ab = objbytes.seg_bytes(aobj)[cseg]
am = {f['loc']: f for f in A['fix'] if f['seg'] == cseg}
ren = {}; patch = {}; notes = []; edelta = {}
def setren(e, n):
    if e in ren and ren[e] != n: raise SystemExit(f'{e}: widerspruechlich {ren[e]} / {n}')
    ren[e] = n
pending = {}
cand = {}                                         # e -> {(ASM-Name, nur Segment?)}
for f in C['fix']:
    if f['seg'] != cseg or f['tgt'][0] != 'EXT': continue
    e = f['tgt'][1]; g = am.get(f['loc'])
    if g and g['tgt'][0] == 'EXT':
        cand.setdefault(e, set()).add((g['tgt'][1], f['typ'] == 2)); continue
    if g and g['tgt'][0] in ('SEG', 'GRP'):
        notes.append(f"{e}@{f['loc']:#x}: ASM {g['tgt']}"); pending.setdefault(e, []).append(f); continue
    pending.setdefault(e, []).append(f)
# Kandidaten aufloesen: Segment-Fixups (typ 2) koennen ueber irgendein Symbol desselben Segments laufen
# (ASM `push seg F_R42_0008 / push 03b3h`) - bei Widerspruch entscheiden die Offset-/Zeiger-Fixups.
for e, cs in cand.items():
    names = {n for n, sg in cs}
    if len(names) == 1: setren(e, names.pop()); continue
    full = {n for n, sg in cs if not sg}
    if len(full) == 1: setren(e, full.pop()); notes.append(f'{e}: Segment-Fixups ueber {sorted(names - full)}'); continue
    raise SystemExit(f'{e}: widerspruechlich {sorted(names)}')
# Pruefung: Offset-Fixups ohne ASM-Fixup auf Externs, die ueber eine Segment-Fixup umbenannt wurden
# (ASM: `push seg F_X / push 055dh` - Segment ueber irgendein Symbol des Moduls, Offset als Zahl).
# Der Offset muss zum Namen passen, sonst das Public an dieser Stelle desselben Segments nehmen.
byname = {}
for n_, (sg_, of_) in allnames.items(): byname.setdefault((sg_, of_), n_)
# Overlay-Stubsegmente (Klasse STUBSEG): Funktionszeiger zeigen auf einen Thunk (0x20 + 5*n: int 3Fh, dw Offset).
# Die Zielfunktion steht im Thunk der Original-EXE (Wort nach CD 3F = Offset im Overlay-Code).
import os
stub = {}
for l in m.splitlines():
    f_ = l.split()
    if len(f_) >= 5 and f_[4] == 'STUBSEG' and f_[3].startswith('OVL'): stub[int(f_[0][:-1], 16) >> 4] = f_[3][:6]
_exe = open(_SW + '/orig/SCHWEIF.EXE', 'rb').read(); _hdr = struct.unpack_from('<H', _exe, 8)[0] * 16
def stubname(sg_, want):
    if want < 0x20 or (want - 0x20) % 5: return None
    q_ = _hdr + sg_ * 16 + want
    if _exe[q_:q_ + 2] != b'\xcd\x3f': return None
    return byname.get((sg_, struct.unpack_from('<H', _exe, q_ + 2)[0]))
for f in C['fix']:
    if f['seg'] != cseg or f['tgt'][0] != 'EXT' or f['typ'] not in (1, 5, 9) or f['loc'] in am: continue
    e = f['tgt'][1]
    if e not in ren or ren[e] not in allnames or allnames[ren[e]][0] == DG: continue
    inl = struct.unpack_from('<H', cb, f['loc'])[0]; orig = struct.unpack_from('<H', ab, f['loc'])[0]
    sg_, of_ = allnames[ren[e]]; want = (orig - inl) & 0xffff
    if of_ == want: continue
    if sg_ in stub:
        n2 = stubname(sg_, want)
        if not n2: raise SystemExit(f'{e}: Thunk-Offset {want:#x} im Stub {stub[sg_]} nicht aufloesbar')
        if n2 != ren[e]: notes.append(f'{e}: {ren[e]} -> {n2} (Thunk {want:#x})'); ren[e] = n2
        continue
    n2 = byname.get((sg_, want))
    if not n2: raise SystemExit(f'{e}: Offset {want:#x} passt nicht zu {ren[e]} ({sg_:04X}:{of_:04X}), kein Public dort')
    notes.append(f'{e}: {ren[e]} -> {n2} (Offset {want:#x})'); ren[e] = n2
# Datenverweise ohne ASM-Fixup
for e, fl in pending.items():
    if e in ren: continue
    addrs = set()
    for f in fl:
        if f['typ'] in (1, 5, 9):                 # Offset 16 Bit
            inl = struct.unpack_from('<H', cb, f['loc'])[0]; orig = struct.unpack_from('<H', ab, f['loc'])[0]
            addrs.add((orig - inl) & 0xffff)
        elif f['typ'] == 3:
            inl = struct.unpack_from('<H', cb, f['loc'])[0]; orig = struct.unpack_from('<H', ab, f['loc'])[0]
            addrs.add((orig - inl) & 0xffff)
    if not addrs:                                   # nur Segmentverweise: jedes DGROUP-Symbol passt
        if e in allnames: continue
        setren(e, dpub[0][1]); continue
    if len(addrs) != 1: raise SystemExit(f'{e}: mehrere Adressen {sorted(hex(x) for x in addrs)}')
    ad = addrs.pop()
    if e in allnames and allnames[e] == (DG, ad): continue
    if ad in daddr: setren(e, daddr[ad]); continue
    k = bisect.bisect_right(dkeys, ad) - 1; base, nm = dpub[k]
    setren(e, nm); edelta[e] = (ad - base) & 0xffff
    for f in fl:
        v = struct.unpack_from('<H', cb, f['loc'])[0]; patch[f['loc']] = (v + ad - base) & 0xffff
    notes.append(f'{e} -> {nm}+{ad-base:#x}')
# Reihenfolge der GRP-Fixups (Welle KS): TASM 3.1 stellt die Gruppen-Fixup eines Interrupt-Prologs
# (mov bp,DGROUP) an den Anfang des FIXUPP-Satzes, im Original (und in der ASM-Fassung) steht sie hinter
# anderen Fixups. TLINK uebernimmt die Satzreihenfolge in die MZ-Relocationtabelle -> GRP-Fixups hinter
# ihren Vorgaenger in der ASM-Fassung setzen (bei schon richtiger Lage ohne Wirkung).
def rkey(f): return f['loc'] + 2 if f['typ'] == 3 else f['loc']
a_order = [(rkey(f), f['tgt'][0] == 'GRP') for f in A['fix'] if f['seg'] == cseg]
def grp_reorder(body, base):
    ents = []; q = 0
    while q < len(body):
        if not body[q] & 0x80: return body             # THREAD-Satz: nicht anfassen
        s0 = q; lo = (body[q] << 8) | body[q + 1]; typ = (lo >> 10) & 15; fd = body[q + 2]; q += 3
        if not fd & 0x80 and (fd >> 4) & 7 < 3: _, q = idx(body, q)
        if not fd & 8: _, q = idx(body, q)
        tm = fd & 3 if not fd & 8 else -1
        if not (fd >> 2) & 1: q += 2
        loc = base + (lo & 0x3ff)
        ents.append((loc + 2 if typ == 3 else loc, tm == 1, body[s0:q]))
    grp = [e for e in ents if e[1]]
    if not grp: return body
    rest = [e for e in ents if not e[1]]
    for g in grp:
        k = [i for i, (kk, isg) in enumerate(a_order) if kk == g[0] and isg]
        pos = 0
        if k:
            keys = [e[0] for e in rest]
            for kk, isg in reversed(a_order[:k[0]]):
                if kk in keys: pos = keys.index(kk) + 1; break
        rest.insert(pos, g)
    return b''.join(e[2] for e in rest)

# ---- Fix-Modus: Zuordnungen von REF auf C_OBJ uebertragen
def allpubs(path, seg):
    """PUBDEF und LPUBDEF (0x90/0x91/0xb6/0xb7) im Codesegment -> {name: offset}"""
    d_ = open(path, 'rb').read(); p_ = 0; ln_ = ['']; sg_ = ['']; res_ = {}
    while p_ < len(d_):
        t_ = d_[p_]; n_ = struct.unpack_from('<H', d_, p_ + 1)[0]; b_ = d_[p_ + 3:p_ + 2 + n_]; p_ += 3 + n_
        if t_ == 0x96:
            q_ = 0
            while q_ < len(b_): l_ = b_[q_]; ln_.append(b_[q_ + 1:q_ + 1 + l_].decode('latin1')); q_ += 1 + l_
        elif t_ == 0x98:
            a_ = b_[0]; q_ = 1 + (3 if a_ >> 5 == 0 else 0) + 2; ni_, _ = idx(b_, q_); sg_.append(ln_[ni_])
        elif t_ in (0x90, 0x91, 0xb6, 0xb7):
            gi_, q_ = idx(b_, 0); si_, q_ = idx(b_, q_)
            if si_ == 0: q_ += 2
            while q_ < len(b_):
                l_ = b_[q_]; nm_ = b_[q_ + 1:q_ + 1 + l_].decode('latin1'); q_ += 1 + l_
                o_ = struct.unpack_from('<H', b_, q_)[0]; q_ += 2; _, q_ = idx(b_, q_)
                if si_ and sg_[si_] == seg: res_.setdefault(nm_, o_)
    return res_
newoff = None
if REF:
    rp = allpubs(REF, cseg); fp = allpubs(cobj, cseg)
    rby = {}
    for n_, o_ in rp.items(): rby.setdefault(o_, []).append(n_)
    def newoff(o_):
        """Offset o_ der REF-Fassung -> Offset in C_OBJ ueber die C-Namen an dieser Stelle"""
        if o_ not in rby: raise SystemExit(f'--ref: an {cseg}:{o_:#x} steht in REF kein C-Public')
        cand_ = {fp[n_] for n_ in rby[o_] if n_ in fp}
        if len(cand_) != 1: raise SystemExit(f'--ref: {rby[o_]} an {o_:#x} in C_OBJ nicht eindeutig ({sorted(cand_)})')
        return cand_.pop()
    F = objfix.parse(cobj)
    if [s_ for s_ in F['segs'] if s_.endswith('_TEXT')][0] != cseg: raise SystemExit('--ref: anderes Codesegment')
    fcb = objbytes.seg_bytes(cobj)[cseg]
    patch = {}
    for f in F['fix']:
        if f['seg'] != cseg or f['tgt'][0] != 'EXT' or f['tgt'][1] not in edelta: continue
        v = struct.unpack_from('<H', fcb, f['loc'])[0]; patch[f['loc']] = (v + edelta[f['tgt'][1]]) & 0xffff
    # neue Externe (nur in C_OBJ): Namenstabelle C-Name -> Linkname
    cn = {}
    if os.environ.get('CNAMEN') and os.path.exists(os.environ['CNAMEN']):
        for l_ in open(os.environ['CNAMEN'], encoding='latin1'):
            f_ = l_.rstrip('\n').split('\t')
            if len(f_) >= 2: cn.setdefault(f_[0], f_[1])
    for e in F['ext']:
        if e in ren or e in allnames or e in C['ext']: continue
        m_ = re.match(r'@([A-Za-z_]\w*)\$q', e)          # C++-Aufruf einer C-Funktion: @name$q.. -> _name
        if e in cn: ren[e] = cn[e]; notes.append(f'neu: {e} -> {cn[e]} (Namenstabelle)')
        elif m_ and '_' + m_.group(1) in cn:
            ren[e] = cn['_' + m_.group(1)]; notes.append(f'neu: {e} -> {ren[e]} (Namenstabelle, C-Name _{m_.group(1)})')
        else: notes.append(f'WARNUNG neu: {e} ohne Zuordnung (bleibt C-Name)')
    for n_ in sorted(set(rp) - set(fp)): notes.append(f'WARNUNG: Public {n_} fehlt in C_OBJ')

# Publics
apub = {}
for s, o, n in A['pubs']: apub.setdefault(o, []).append(n)
renamed_targets = set(ren.values())

ovl = cseg.startswith('OVL'); ovl_done = False
A_pub_order = [(n_, o_ if newoff is None else newoff(o_)) for s_, o_, n_ in A['pubs']]
d = open(cobj, 'rb').read(); p = 0; outr = []; ln = ['']; segs = ['']; last = None
while p < len(d):
    t = d[p]; n = struct.unpack_from('<H', d, p + 1)[0]; r = d[p:p + 3 + n]; body = r[3:-1]; p += 3 + n
    if t == 0x96:
        q = 0
        while q < len(body): l = body[q]; ln.append(body[q + 1:q + 1 + l].decode('latin1')); q += 1 + l
    if t == 0x98:
        acbp = body[0]; q = 1 + (3 if acbp >> 5 == 0 else 0); L = struct.unpack_from('<H', body, q)[0]
        ni, _ = idx(body, q + 2); segs.append(ln[ni])
        if L == 0 and not acbp & 2 and acbp >> 5 not in (0, 1):
            body = bytes([(acbp & 0x1f) | 0x20]) + body[1:]; r = rec(t, body)
    elif t in (0x8c, 0xb4):
        q = 0; nb = b''
        while q < len(body):
            l = body[q]; nm = body[q + 1:q + 1 + l].decode('latin1'); q += 1 + l; ti, q2 = idx(body, q)
            tb = body[q:q2]; q = q2; nm = ren.get(nm, nm); nb += bytes([len(nm)]) + nm.encode() + tb
        r = rec(t, nb)
    elif t in (0x90, 0x91) and ovl:
        # Overlay: genau die Publics der ASM-Fassung in deren Reihenfolge (Einsprungnummern der Stubs!)
        if ovl_done: continue
        ovl_done = True
        gi, q = idx(body, 0); si, q = idx(body, q)
        if segs[si] != cseg: raise SystemExit('PUBDEF nicht im Codesegment')
        hdr = body[:q]; cur = b''
        for n_, o_ in A_pub_order:
            e = bytes([len(n_)]) + n_.encode() + struct.pack('<H', o_) + b'\x00'
            if len(cur) + len(e) > 1000: outr.append(rec(t, hdr + cur)); cur = b''
            cur += e
        r = rec(t, hdr + cur)
    elif t in (0x90, 0x91):
        gi, q = idx(body, 0); si, q = idx(body, q); hdr = body[:q]
        if si == 0: hdr = body[:q + 2]; q += 2
        ents = []
        while q < len(body):
            l = body[q]; nm = body[q + 1:q + 1 + l].decode('latin1'); q += 1 + l
            off = struct.unpack_from('<H', body, q)[0]; q += 2; ti, q2 = idx(body, q); tb = body[q:q2]; q = q2
            names = list(apub.get(off if newoff is None else rp.get(nm, -1), []))
            if nm not in names and nm not in allnames and nm not in renamed_targets: names.append(nm)
            if not names: names = [nm]
            for x in names: ents.append(bytes([len(x)]) + x.encode() + struct.pack('<H', off) + tb)
        # in Records < 1024 Bytes aufteilen
        cur = b''
        for e in ents:
            if len(cur) + len(e) > 1000: outr.append(rec(t, hdr + cur)); cur = b''
            cur += e
        r = rec(t, hdr + cur) if cur else b''
    elif t == 0x9c and last is not None and last[0] == cseg and not REF:
        nb = grp_reorder(body, last[1])
        if nb != body: r = rec(t, nb)
    elif t == 0xa2:
        last = None
    elif t == 0xa0:
        si, q = idx(body, 0); off = struct.unpack_from('<H', body, q)[0]
        last = (segs[si], off)
        if segs[si] == cseg and patch:
            nb = bytearray(body)
            for loc, v in patch.items():
                if off <= loc and loc + 2 <= off + len(body) - q - 2:
                    struct.pack_into('<H', nb, q + 2 + loc - off, v)
            r = rec(t, bytes(nb))
    outr.append(r)
open(out, 'wb').write(b''.join(outr))
print(f'{cobj}: {len(ren)} Namen angeglichen, {len(patch)} Inline-Werte angepasst' + (f' (Fix-Modus, Zuordnung aus {REF})' if REF else ''),
      *(notes if REF else notes[:6]), sep='\n  ')
