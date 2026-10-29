#!/usr/bin/env python3
"""calib.py - vergleicht Optionsvarianten an fast unveraendert uebernommenen DSA-1-Funktionen.
Fuer jede Kandidatenfunktion (Name -> Startadresse in SCHWEIF.EXE, aus aehnlichkeit.py) wird der
Code der Variante an diese Adresse gelegt und gezaehlt: feste Bytes gleich (positionsgenau) und
Befehlsfolge-Aehnlichkeit (difflib auf dem Befehlsgeruest).
Aufruf: calib.py SCHWEIF.EXE kandidaten.tsv VARIANTE...   (VARIANTE = ~/sternenschweif/v_NAME/o)
"""
import sys, os, glob, difflib
sys.path.insert(0, '/root/sternenschweif/tools')
import omfcmp
src = open('/root/sternenschweif/tools/aehnlichkeit.py').read()
pre = src.split('K=4')[0]
sys_argv = sys.argv; sys.argv = ['x', sys_argv[1], '.', '/dev/null']
exec(pre)
sys.argv = sys_argv
exe = open(sys.argv[1], 'rb').read()
EF = exe_funcs(sys.argv[1])
cand = [l.rstrip('\n').split('\t') for l in open(sys.argv[2])]
cand = [(c[0], c[1], int(c[4], 16)) for c in cand]
def funcs_of(odir):
    out = {}
    for obj in glob.glob(odir + '/SEG*.OBJ'):
        code = [c for c in omfcmp.parse_obj(obj) if c[1] == 'CODE']
        if not code: continue
        _, _, data, mask = code[0]; pubs = publics(obj)
        for i, (off, name) in enumerate(pubs):
            end = pubs[i + 1][0] if i + 1 < len(pubs) else len(data)
            out[(os.path.basename(obj), name)] = (bytes(data[off:end]), bytes(mask[off:end]))
    return out
print('Variante\t' + '\t'.join(n.split('$')[0].lstrip('@')[:14] for _, n, _ in cand) + '\tBytes\tGeruest')
for v in sys.argv[3:]:
    F = funcs_of(v); row = []; tb = tf = 0; ts = 0.0
    for o, n, base in cand:
        if (o, n) not in F: row.append('-'); continue
        d, m = F[(o, n)]
        fixed = sum(m); eq = sum(1 for t in range(len(d)) if m[t] and base + t < len(exe) and exe[base + t] == d[t])
        s = difflib.SequenceMatcher(None, dis(d), EF.get(base, [])).ratio() if base in EF else 0
        tb += eq; tf += fixed; ts += s
        row.append(f'{eq}/{fixed}' + ('*' if eq == fixed else ''))
    print(os.path.basename(os.path.dirname(v)) + '\t' + '\t'.join(row) + f'\t{100*tb/tf:.1f}%\t{ts/len(cand):.3f}')
