#!/usr/bin/env python3
"""namen.py - schlaegt Namen fuer SCHWEIF-Funktionen vor (Befehlsgeruest, 4er-Folgen, Jaccard).
Quellen: (a) Bright-Eyes-OBJ mit -O -3 -Oe -Ot, (b) SCHICKM.EXE mit Namen aus BLADEM.MAP."""
import sys, re, glob, os, collections, struct
sys.path.insert(0, '/root/sternenschweif/q004/zerl'); sys.path.insert(0, '/root/sternenschweif/tools')
import schweif, omfcmp, eich_dsa1
src = open('/root/sternenschweif/tools/aehnlichkeit.py').read()
sys.argv, keep = ['x', '/root/sternenschweif/orig/SCHWEIF.EXE', '.', '/dev/null'], sys.argv
exec(src.split('K=4')[0])            # norm, canon, dis, publics
sys.argv = keep
K = 4
sh = lambda v: set(tuple(v[i:i + K]) for i in range(len(v) - K + 1))
def funcs_exe(path, names=None):
    segs = schweif.load(path); F = schweif.functions(segs); S = {s['id']: s for s in segs}; out = {}
    for sid, L in F.items():
        c = S[sid]['code']
        for i, (o, q) in enumerate(L):
            end = L[i + 1][0] if i + 1 < len(L) else len(c)
            nm = (names or {}).get((sid, o))
            out[(sid, o)] = (nm, dis(c[o:end]), end - o)
    return out, segs
# Quelle b: DSA 1 EXE mit MAP-Namen
segs1 = schweif.load('/root/sternenschweif/orig/SCHICKM.EXE')
pubs, _ = eich_dsa1.truth(segs1, '/root/sternenschweif/dsa1/BLADEM.MAP')
nm1 = {}
for sid, P in pubs.items():
    for o, n in sorted(P, key=lambda x: x[1]): nm1.setdefault((sid, o), n)
B, _ = funcs_exe('/root/sternenschweif/orig/SCHICKM.EXE', nm1)
ref = []
for k, (n, v, sz) in B.items():
    if n and len(v) >= 6: ref.append(('exe', n, v))
# Quelle a: optimierte OBJ
for obj in sorted(glob.glob('/root/sternenschweif/v_fA/o/SEG*.OBJ')):
    code = [c for c in omfcmp.parse_obj(obj) if c[1] == 'CODE']
    if not code: continue
    data = code[0][2]; P = publics(obj)
    for i, (off, name) in enumerate(P):
        end = P[i + 1][0] if i + 1 < len(P) else len(data)
        v = dis(bytes(data[off:end]))
        if len(v) >= 6: ref.append(('obj', name, v))
RS = [sh(v) for _, _, v in ref]
inv = collections.defaultdict(list)
for j, s in enumerate(RS):
    for g in s: inv[g].append(j)
A, segs2 = funcs_exe('/root/sternenschweif/orig/SCHWEIF.EXE')
rows = []
for (sid, o), (_, v, sz) in sorted(A.items()):
    best = (0, None)
    if len(v) >= 6:
        s = sh(v); cnt = collections.Counter()
        for g in s:
            l = inv.get(g, ())
            if len(l) > 300: continue
            for j in l: cnt[j] += 1
        for j, c in cnt.most_common(8):
            jac = c / len(s | RS[j])
            if jac > best[0]: best = (jac, j)
    jac, j = best
    rows.append((sid, o, sz, len(v), round(jac, 2), ref[j][1] if j is not None else '', ref[j][0] if j is not None else ''))
with open('/root/sternenschweif/q004/zerl/namen.tsv', 'w') as f:
    f.write('segment\tstart\tgroesse\tbefehle\taehnlichkeit\tname_dsa1\tquelle\n')
    for r in rows: f.write(f'{r[0]}\t{r[1]:#06x}\t{r[2]}\t{r[3]}\t{r[4]}\t{r[5]}\t{r[6]}\n')
bins = collections.Counter(); byb = collections.Counter()
for r in rows:
    b = 1.0 if r[4] >= 0.999 else 0.9 if r[4] >= 0.9 else 0.7 if r[4] >= 0.7 else 0.5 if r[4] >= 0.5 else 0
    bins[b] += 1; byb[(b, r[0][0], r[6])] += 1
print('Referenzfunktionen', len(ref), '(exe', sum(1 for x in ref if x[0] == 'exe'), 'obj', sum(1 for x in ref if x[0] == 'obj'), ')')
for b in (1.0, 0.9, 0.7, 0.5, 0):
    print(f'Aehnlichkeit >= {b}: {bins[b]:5d}   ', {f'{k[1]}/{k[2]}': n for k, n in byb.items() if k[0] == b})
