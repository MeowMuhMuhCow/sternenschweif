#!/usr/bin/env python3
"""ccmp.py ASM_OBJ C_OBJ... - Codevergleich (erstes Codesegment), Fixup-Felder beider Seiten ausgeblendet"""
import sys; sys.path.insert(0,'/root/sternenschweif/tools'); import objbytes, objfix, selfoff
W={0:1,1:2,2:2,3:4,4:1,5:2,9:2,11:4,13:4}
def masked(path):
    sb=objbytes.seg_bytes(path); seg=[s for s in sb if s.endswith('_TEXT')][0]
    b=sb[seg]; m=set()
    for f in objfix.parse(path)['fix']:
        if f['seg']==seg: m.update(range(f['loc'],f['loc']+W.get(f['typ'],2)))
    return b,m
a,ma=masked(sys.argv[1])
for p in sys.argv[2:]:
    try: c,mc=masked(p)
    except Exception as e: print(p,'FEHLT'); continue
    d=[i for i in range(min(len(a),len(c))) if a[i]!=c[i] and i not in mc and i not in ma]
    try: d+=selfoff.check(sys.argv[1],p)
    except Exception as e: print('  selfoff:', e)
    # eigene Daten (_DATA, _BSS ...) in der C-OBJ: das Original hat alle Daten im zentralen DS -> Fehler
    x={s:len(b) for s,b in objbytes.seg_bytes(p).items() if not s.endswith('_TEXT') and len(b)}
    if x: print(f'  EIGENE DATEN in der C-OBJ: {x} (z. B. String-Literal) - so nicht linkbar')
    print(f'{p.split("/")[-1]}: {len(c)}/{len(a)} Bytes, {len(d)} anders', [hex(x) for x in d[:10]], 'GLEICH' if not d and len(a)==len(c) and not x else '')
