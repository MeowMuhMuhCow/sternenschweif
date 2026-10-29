#!/usr/bin/env python3
import os as _swos; _SW = _swos.environ.get('SW_ROOT', '/root/sternenschweif')  # Bau-Paket
"""dscheck.py FIX_OBJ [MAP] - Phase B (dsana/PLAN.md): prueft eine linkfertige C-OBJ, die ihre Daten selbst definiert,
unabhaengig von cobjfix_d.py --eigen gegen das Original:
  1. eigene Segmente nur _DATA/_BSS (und _INIT_/_EXIT_ mit dem Originaleintrag bei Spalte DI), wortausgerichtet, Lage/Laenge = Modulbereich (tools/dsinfo.own, ggf. ohne Fuellbyte)
  2. _DATA-Bytes (Fixups aufgeloest) = Original-DS ab Modulbasis
  3. DGROUP-Relocations aus den eigenen Daten in derselben Reihenfolge wie in der MZ-Tabelle des Originals
  4. alle DGROUP-Publics der Stufe-1-MAP im eigenen Bereich werden von der OBJ an derselben Stelle exportiert
Ausgabe: 'dscheck MODUL: GLEICH ...' (Exit 0) oder 'dscheck MODUL: FEHLER ...' (Exit 1)."""
import sys, struct
sys.path.insert(0, _SW + '/tools')
import objfix, objbytes, dsinfo, cobjfix_d as CD
fixobj = sys.argv[1]; mapf = sys.argv[2] if len(sys.argv) > 2 else _SW + '/lnk/asm1/SCHW_CD.MAP'
def idx(b, q):
    v = b[q]; return (((v & 0x7f) << 8) | b[q + 1], q + 2) if v & 0x80 else (v, q + 1)
O = objfix.parse(fixobj); sb = objbytes.seg_bytes(fixobj)
cseg = [s for s in O['segs'] if s.endswith('_TEXT')][0]; mod = dsinfo.modname(cseg)
fehler = []
def F(t): fehler.append(t)
# Segmentdefinitionen (Laenge, Ausrichtung)
segdef = {}; ln = ['']
for t, b in CD.records(open(fixobj, 'rb').read()):
    if t == 0x96:
        q = 0
        while q < len(b): l = b[q]; ln.append(b[q + 1:q + 1 + l].decode('latin1')); q += 1 + l
    elif t == 0x98:
        a = b[0]; q = 1 + (3 if a >> 5 == 0 else 0); L = struct.unpack_from('<H', b, q)[0]; ni, _ = idx(b, q + 2)
        segdef[ln[ni]] = (a >> 5, L)
exp = dsinfo.own(mod); base = {}; initexit = []
for s, (al, L) in segdef.items():
    if s.endswith('_TEXT') or L == 0: continue
    if s in ('_INIT_', '_EXIT_'):             # eigener Eintrag (globales C++-Objekt, #pragma startup/exit)
        idx = (dsinfo.INIT_IDX if s == '_INIT_' else dsinfo.EXIT_IDX).get(mod)
        if idx is None: F(f'{s}: Modul hat im Original keinen Eintrag'); continue
        orig = CD.ds_bytes((dsinfo.INIT_BASE if s == '_INIT_' else dsinfo.EXIT_BASE) + 6 * idx, 6)
        if L != 6 or sb.get(s, b'')[:2] != orig[:2]: F(f'{s}: {sb.get(s, b"").hex()} passt nicht zu {orig.hex()}')
        initexit.append(s); continue
    k = {'_DATA': 'DATA', '_BSS': 'BSS'}.get(s)
    if k is None: F(f'Segment {s} ({L} Bytes) nicht unterstuetzt'); continue
    if al != 2: F(f'{s} nicht wortausgerichtet (align {al})')
    e = exp.get(k)
    if not e: F(f'{s}: {L} Bytes, Modul hat laut dsana kein {k}'); continue
    if not (e[0] + L == e[1] or (e[0] + L + 1 == e[1] and L % 2 == 1)):
        F(f'{s}: Laenge {L}, Bereich {e[0]:#x}..{e[1]:#x} ({e[1] - e[0]} Bytes)')
    base[s] = e[0]
for k in ('DATA', 'BSS'):
    if exp.get(k) and {'DATA': '_DATA', 'BSS': '_BSS'}[k] not in base: F(f'{k}-Bereich {exp[k][0]:#x}..{exp[k][1]:#x} nicht geliefert')
# Bytes
nb = 0
if '_DATA' in base:
    a0 = base['_DATA']; L = segdef['_DATA'][1]
    b = bytearray(sb.get('_DATA', b'').ljust(L, b'\0')); skip = set()
    for f in O['fix']:
        if f['seg'] != '_DATA': continue
        o = f['loc']; tk, tn = f['tgt']
        if f['typ'] in (1, 3, 5, 9):
            if tk == 'SEG' and tn in base:
                struct.pack_into('<H', b, o, (base[tn] + struct.unpack_from('<H', b, o)[0] + f['disp']) & 0xffff)
            else: skip.update(range(o, o + 2))
        so = o + 2 if f['typ'] == 3 else (o if f['typ'] == 2 else None)
        if so is not None:
            if (tk == 'SEG' and tn in base) or f['tgt'] == ('GRP', 'DGROUP'): struct.pack_into('<H', b, so, CD.DG)
            else: skip.update(range(so, so + 2))
    orig = CD.ds_bytes(a0, L)
    bad = [i for i in range(L) if i not in skip and b[i] != orig[i]]
    if bad: F(f'_DATA+{bad[0]:#x} (ds:{a0 + bad[0]:#x}): {bytes(b[bad[0]:bad[0]+8]).hex()} != {orig[bad[0]:bad[0]+8].hex()} ({len(bad)} Bytes)')
    if L % 2 and CD.ds_bytes(a0 + L, 1) != b'\0': F(f'Fuellbyte ds:{a0 + L:#x} ist nicht 00')
    nb = L - len(skip)
# Relocation-Reihenfolge der eigenen Daten (TLINK uebernimmt die Satzfolge der OBJ)
seq = []
for f in O['fix']:
    if f['seg'] in base:
        if f['typ'] == 3: seq.append(base[f['seg']] + f['loc'] + 2)
        elif f['typ'] == 2: seq.append(base[f['seg']] + f['loc'])
    elif f['seg'] in initexit and f['typ'] == 3: seq.append(f['seg'])
want = [p if sg == '_DATA' else sg for sg, p in dsinfo.mzorder().get(mod, [])
        if (sg == '_DATA' and '_DATA' in base and base['_DATA'] <= p < base['_DATA'] + segdef['_DATA'][1]) or sg in initexit]
if seq != want:
    i = next((k for k, (x, y) in enumerate(zip(seq, want)) if x != y), min(len(seq), len(want)))
    hx = lambda l: [x if isinstance(x, str) else hex(x) for x in l]
    F(f'Relocation-Reihenfolge weicht ab bei Nr. {i}: OBJ {hx(seq[i:i+4])} Original {hx(want[i:i+4])}')
# Publics
alln = CD.mapnames(mapf); pubs = {n: (s, o) for s, o, n in O['pubs']}
np = 0
for n, (sg, of) in alln.items():
    if sg != CD.DG: continue
    for s, a0 in base.items():
        L = segdef[s][1]
        if a0 <= of < a0 + max(L, 1):
            np += 1
            if pubs.get(n) != (s, of - a0): F(f'Public {n} (ds:{of:#x}) fehlt/falsch: {pubs.get(n)}')
for n, (s, o) in pubs.items():
    if s in base and n in alln and alln[n] != (CD.DG, base[s] + o): F(f'Public {n}: OBJ ds:{base[s] + o:#x}, MAP {alln[n]}')
txt = ', '.join([f"{s} ds:{a:#x}..{a + segdef[s][1]:#x}" for s, a in base.items()] + [f'{s} eigener Eintrag' for s in initexit])
if fehler:
    print(f'dscheck {mod}: FEHLER ({txt})'); [print('  ' + x) for x in fehler]; sys.exit(1)
print(f'dscheck {mod}: GLEICH ({txt}; {nb} Bytes = Original, {len(seq)} Relocations in Originalreihenfolge, {np} Publics)')
