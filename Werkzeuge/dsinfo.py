#!/usr/bin/env python3
"""dsinfo.py - Datenbereiche je Originalmodul (Phase A/B, dsana/PLAN.md) fuer cobjfix_d.py --eigen und dscheck.py.

ranges()        {modul: {'DATA': [a,e], 'BSS': [a,e]}}   aus dsana/zuordnung.json (e exklusiv, inkl. Fuellbyte)
rest()          {modul: [(kind, a, e)]}                    aus dsana/dm_rest.tsv (Reststuecke im ASM)
own(modul)      {'DATA': [a,e] | None, 'BSS': [a,e] | None} Bereich, den die C-OBJ selbst liefern muss
mzorder()       {modul: [(seg, ds-adresse)]}               DGROUP-Relocations des Originals in MZ-Reihenfolge
modname(seg)    'OVL068_TEXT'/'SEG005_TEXT' -> 'O068'/'R05'
Aufruf: dsinfo.py MODUL  -> Bereiche ausgeben"""
import sys, json, os, struct, collections
R = '/root/sternenschweif'; D = R + '/dsana/'
DGROUP_PARA = 0x24EB
INIT_BASE, EXIT_BASE = 0x8b06, 0x8b42
INIT_IDX = {'O008': 0, 'O032': 1, 'O224': 2}; EXIT_IDX = {'O032': 0}
INIT_FN = {'O008': 'F_O008_0DFC', 'O032': 'F_O032_0C8C', 'O224': 'F_O224_032B'}   # Satz 0,1,2 in _INIT_ (Prioritaet 32)
EXIT_FN = {'O032': 'F_O032_0F94'}                                                  # Satz 0 in _EXIT_
def own_initexit():
    """Module mit Spalte DI in c_module.tsv: die C-OBJ liefert ihre _INIT_/_EXIT_-Eintraege selbst"""
    s = set()
    for l in open(R + '/c_module.tsv'):
        if l.startswith('#') or not l.strip(): continue
        f = l.rstrip('\r\n').split('\t')
        if len(f) >= 3 and f[2].strip() == 'DI': s.add(f[0] if f[0].startswith('O') else 'R%02d' % int(f[0]))
    return s
_r = None
def korrekturen():
    out = []
    if os.path.exists(D + 'korrekturen.tsv'):
        for l in open(D + 'korrekturen.tsv'):
            if l.startswith('#') or not l.strip(): continue
            a, e, m, k = l.split('\t')[:4]; out.append((int(a, 16), int(e, 16), m, k))
    return out
def zuordnung():
    """zuordnung.json mit dsana/korrekturen.tsv: {'DATA': [[a,e,m]...], 'BSS': [...]}"""
    Z = json.load(open(D + 'zuordnung.json'))
    for a, e, m, k in korrekturen():
        neu = []
        for x, y, mm in Z[k]:
            if y <= a or x >= e: neu.append([x, y, mm]); continue
            if x < a: neu.append([x, a, mm])
            if y > e: neu.append([e, y, mm])
        neu.append([a, e, m]); Z[k] = sorted(neu)
    return Z
def ranges():
    global _r
    if _r is None:
        Z = zuordnung(); _r = collections.defaultdict(dict)
        for kind in ('DATA', 'BSS'):
            for a, e, m in Z[kind]:
                r = _r[m].get(kind)
                if r is not None and r[1] != a: raise SystemExit(f'{m} {kind}: Bereich nicht zusammenhaengend ({r[1]:#x}/{a:#x}) - korrekturen.tsv pruefen')
                _r[m][kind] = [a, e] if r is None else [r[0], e]
    return _r
def rest():
    out = collections.defaultdict(list)
    if os.path.exists(D + 'dm_rest.tsv'):
        for l in open(D + 'dm_rest.tsv'):
            if l.startswith('#') or not l.strip(): continue
            m, k, a, b = l.split()[:4]; out[m].append((k, int(a, 16), int(b, 16)))
    return out
def own(m):
    r = ranges().get(m, {}); out = {}
    for k in ('DATA', 'BSS'):
        if k not in r: out[k] = None; continue
        a, e = r[k]
        for kk, x, y in rest().get(m, []):
            if kk != k: continue
            if x == a: a = y            # Rest vorn
            elif y == e: e = x          # Rest hinten
            else: raise SystemExit(f'{m}: Reststueck {k} {x:#x}..{y:#x} weder vorn noch hinten')
        out[k] = [a, e] if a < e else None
    return out
def owner(p, kind='DATA'):
    for m, r in ranges().items():
        if kind in r and r[kind][0] <= p < r[kind][1]: return m
    return None
def mzorder():
    e = open(R + '/orig/SCHWEIF.EXE', 'rb').read()
    nr, ro = struct.unpack_from('<H', e, 6)[0], struct.unpack_from('<H', e, 0x18)[0]
    out = collections.defaultdict(list)
    for i in range(nr):
        o, s = struct.unpack_from('<HH', e, ro + 4 * i)
        if s != DGROUP_PARA: continue
        if 0x94 <= o < 0x86b6: out[owner(o)].append(('_DATA', o))
        elif INIT_BASE <= o < EXIT_BASE:
            m = next((x for x, v in INIT_IDX.items() if v == (o - INIT_BASE - 4) // 6), None)
            if m: out[m].append(('_INIT_', o))
        elif EXIT_BASE <= o < 0x8b4e:
            m = next((x for x, v in EXIT_IDX.items() if v == (o - EXIT_BASE - 4) // 6), None)
            if m: out[m].append(('_EXIT_', o))
    return out
def modname(cseg):
    import re
    x = re.match(r'(OVL|SEG)(\d+)_TEXT$', cseg)
    if not x: return None
    return ('O%03d' if x.group(1) == 'OVL' else 'R%02d') % int(x.group(2))
if __name__ == '__main__':
    for m in sys.argv[1:]:
        print(m, 'Bereich', {k: [hex(x) for x in v] for k, v in ranges().get(m, {}).items()},
              'eigen', {k: v and [hex(x) for x in v] for k, v in own(m).items()},
              'Rest', rest().get(m, []), 'MZ', [(s, hex(p)) for s, p in mzorder().get(m, [])])
