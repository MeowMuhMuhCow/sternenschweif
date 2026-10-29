#!/usr/bin/env python3
"""dsrefs.py MODUL... - Phase B Hilfe: welche Daten im Modulbereich (dsinfo.own) benutzt die C-Fassung heute per extern?
Grundlage: cbau/<m>.OBJ (C-Namen) und cbau/<m>.FIX (linkfertig, Stand letzter build.sh), lnk/asm1/SCHW_CD.MAP.
Ausgabe je Item des Modulbereichs: Adresse, dsg-Name, Inhalt, C-Namen der Verweise dieses Moduls (Anzahl) und
Verweise anderer C-Module (cbau/*.FIX) auf diese Adressen."""
import sys, os, struct, glob, collections
sys.path.insert(0, '/root/sternenschweif/tools')
import objfix, objbytes, dsinfo, items_ds, cobjfix_d as CD
R = '/root/sternenschweif'; CB = os.environ.get('CBAU', R + '/cbau'); MAP = R + '/lnk/asm1/SCHW_CD.MAP'
alln = CD.mapnames(MAP)
def fname(m): return f'OVL{m[1:]}' if m[0] == 'O' else f'SEG{int(m[1:]):03d}'
def refs(fix, cobj=None):
    """[(adresse, C-Name|None, loc)] aller Codeverweise auf DGROUP-Publics (Adresse inkl. Inline-Wert)"""
    F = objfix.parse(fix); cseg = [s for s in F['segs'] if s.endswith('_TEXT')][0]
    cb = objbytes.seg_bytes(fix).get(cseg, b'')
    cn = {}
    if cobj and os.path.exists(cobj):
        for f in objfix.parse(cobj)['fix']:
            if f['seg'] == cseg and f['tgt'][0] == 'EXT': cn[f['loc']] = f['tgt'][1]
    out = []
    for f in F['fix']:
        if f['seg'] != cseg or f['tgt'][0] != 'EXT' or f['typ'] == 2: continue
        n = f['tgt'][1]
        if n not in alln or alln[n][0] != CD.DG: continue
        inl = struct.unpack_from('<H', cb, f['loc'])[0] if f['typ'] in (1, 3, 5, 9) else 0
        out.append(((alln[n][1] + inl + f['disp']) & 0xffff, cn.get(f['loc']), f['loc']))
    return out
_all = None
def allrefs():
    global _all
    if _all is None:
        _all = collections.defaultdict(list)
        for p in glob.glob(CB + '/*.FIX'):
            m = os.path.basename(p)[:-4]
            for a, _, _ in refs(p): _all[a].append(m)
    return _all
its = items_ds.its()
for m in sys.argv[1:]:
    own = dsinfo.own(m); fn = fname(m)
    print(f'== {m} ({fn}) Bereich', {k: v and f'{v[0]:#x}..{v[1]:#x}' for k, v in own.items()})
    mine = collections.defaultdict(collections.Counter)
    for a, c, _ in refs(f'{CB}/{fn}.FIX', f'{CB}/{fn}.OBJ'): mine[a][c] += 1
    for k in ('DATA', 'BSS'):
        if not own[k]: continue
        a0, a1 = own[k]
        for x in its:
            if x['kind'] != k: continue
            s, e = x['addr'], x['addr'] + max(len(x['data']), 1)
            if e <= a0 or s >= a1: continue
            s2, e2 = max(s, a0), min(e, a1)
            here = {a: dict(c) for a, c in mine.items() if s2 <= a < e2}
            other = sorted({mm for a in range(s2, e2) for mm in allrefs().get(a, []) if mm != fn})
            d = bytes(v or 0 for v in x['data'][s2 - s:e2 - s]) if k == 'DATA' else b''
            print(f"  {s2:#06x}..{e2:#06x} {x['name']:<28} {x['typ']:<8} {d[:48]!r:<52} hier: "
                  f"{', '.join(f'{a:#x}:{c}' for a, c in here.items()) or '-'}  andere: {', '.join(other) or '-'}")
    # Verweise ausserhalb (Kontrolle: Daten anderer Module, die dieses Modul benutzt)
    fremd = sorted(a for a in mine if not any(own[k] and own[k][0] <= a < own[k][1] for k in ('DATA', 'BSS')))
    near = [a for a in fremd if any(own[k] and own[k][0] - 0x40 <= a < own[k][1] + 0x40 for k in ('DATA', 'BSS'))]
    if near: print('  Verweise knapp ausserhalb des Bereichs:', ', '.join(f'{a:#x} ({dsinfo.owner(a) or dsinfo.owner(a, "BSS")})' for a in near))
