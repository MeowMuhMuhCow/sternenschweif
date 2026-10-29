#!/usr/bin/env python3
import os as _swos; _SW = _swos.environ.get('SW_ROOT', '/root/sternenschweif')  # Bau-Paket
"""pubdup.py OBJ...|ANTWORTDATEI.RES - meldet PUBDEF-Namen, die in mehreren OBJ definiert sind (Phase B: eigene C-Namen in _DATA/_BSS
duerfen nicht mit Namen anderer Module kollidieren; die IDE-Linkerin BC.EXE /b bricht dann ohne Meldung ab).
Exit 1 bei Doppelten."""
import sys, os, collections
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import objfix
def objs(args):
    for a in args:
        if a.upper().endswith('.RES'):
            d = os.path.dirname(a); txt = open(a).read().replace('+', ' ').split('\n')
            for l in txt:
                for t in l.split():
                    if t.startswith('/') or '\\' in t or '.' in t: continue
                    yield os.path.join(d, t + '.OBJ')
        else: yield a
seen = collections.defaultdict(list)
for p in objs(sys.argv[1:]):
    try: O = objfix.parse(p)
    except Exception: continue
    for s, o, n in O['pubs']: seen[n].append(os.path.basename(p))
dup = {n: v for n, v in seen.items() if len(v) > 1}
for n, v in sorted(dup.items()): print(f'Public {n} mehrfach definiert: {", ".join(v)}')
sys.exit(1 if dup else 0)
