#!/usr/bin/env python3
"""ccmp_d.py ASM_OBJ C_OBJ... - Vorschlag fuer tools/ccmp.py (Stufe D1): wie ccmp.py, aber eigene Daten der C-OBJ
sind kein Fehler mehr, wenn cobjfix_d sie aufloesen kann (Lage aus den Codeverweisen, Bytes = Original-DS).
Meldung dann `GLEICH (+n Bytes eigene Daten geprueft)`. Kann cobjfix_d sie nicht aufloesen (Bytes anders, Lage
widerspruechlich, kein Codeverweis und Muster nicht eindeutig ...): `EIGENE DATEN NICHT AUFLOESBAR: <Grund>`.
Umgebung MAP=... (Standard ref_obj/SCHW_CD.MAP) fuer ASM-Fixups auf DGROUP-Publics."""
import sys, os
sys.path.insert(0, '/root/sternenschweif/tools'); import objbytes, objfix, selfoff
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__))); import cobjfix_d
MAP = os.environ.get('MAP', '/root/sternenschweif/ref_obj/SCHW_CD.MAP')
W = {0: 1, 1: 2, 2: 2, 3: 4, 4: 1, 5: 2, 9: 2, 11: 4, 13: 4}
def masked(path):
    sb = objbytes.seg_bytes(path); seg = [s for s in sb if s.endswith('_TEXT')][0]
    b = sb[seg]; m = set()
    for f in objfix.parse(path)['fix']:
        if f['seg'] == seg: m.update(range(f['loc'], f['loc'] + W.get(f['typ'], 2)))
    return b, m
a, ma = masked(sys.argv[1])
for p in sys.argv[2:]:
    try: c, mc = masked(p)
    except Exception as e: print(p, 'FEHLT'); continue
    d = [i for i in range(min(len(a), len(c))) if a[i] != c[i] and i not in mc and i not in ma]
    try: d += selfoff.check(sys.argv[1], p)
    except Exception as e: print('  selfoff:', e)
    own = {s: len(b) for s, b in objbytes.seg_bytes(p).items() if not s.endswith('_TEXT') and len(b)}
    own.update(cobjfix_d.own_segments(p))            # auch _BSS (Laenge ohne LEDATA)
    ok_own = True; extra = ''
    if own:
        try:
            r = cobjfix_d.analyse(p, sys.argv[1], MAP)
            extra = f' (+{r["checked"]} Bytes eigene Daten geprueft' + \
                    (f', {r["masked"]} Verweise maskiert' if r['masked'] else '') + ')'
            for x in r['pieces']:
                print(f"  eigene Daten {x['seg']}+{x['von']:#x}..{x['bis']:#x} -> ds:{x['adr']:#x}"
                      f"{' (BSS)' if x['bss'] else ''}, {x['dsana']}")
        except cobjfix_d.D1Fehler as e:
            ok_own = False; print(f'  EIGENE DATEN NICHT AUFLOESBAR: {own}: {e}')
    same = not d and len(a) == len(c)
    print(f'{p.split("/")[-1]}: {len(c)}/{len(a)} Bytes, {len(d)} anders', [hex(x) for x in d[:10]],
          ('GLEICH' + extra) if same and ok_own else '')
