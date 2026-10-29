#!/usr/bin/env python3
"""funcmatch.py - sucht DSA-1-Funktionen (aus den OBJ-Dateien) in einer anderen EXE.

Zerlegt den Code jedes OBJ an den Funktions-PUBDEFs, blendet Fixup-Stellen aus und
sucht jede Funktion in der Ziel-EXE: GLEICH (alle festen Bytes gleich), AEHNLICH
(Anker gefunden, >= 90 % gleich) oder FEHLT.
Aufruf: funcmatch.py ZIEL.EXE OBJ-Verzeichnis > ergebnis.tsv
"""
import sys, os, glob, struct
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import omfcmp


def idx(b, q):
    v = b[q]
    return (((v & 0x7F) << 8) | b[q + 1], q + 2) if v & 0x80 else (v, q + 1)


def publics(path):
    d = open(path, 'rb').read(); p = 0; ln_ = ['']; segs = [None]; out = []
    while p < len(d):
        t = d[p]; n = struct.unpack_from('<H', d, p + 1)[0]; b = d[p + 3:p + 3 + n - 1]; p += 3 + n
        if t == 0x96:
            q = 0
            while q < len(b):
                k = b[q]; ln_.append(b[q + 1:q + 1 + k].decode('latin1')); q += 1 + k
        elif t == 0x98:
            acbp = b[0]; q = 1 + (3 if (acbp >> 5) == 0 else 0) + 2
            ni, q = idx(b, q); ci, q = idx(b, q); segs.append((ln_[ni], ln_[ci]))
        elif t == 0x90:
            q = 0; gi, q = idx(b, q); si, q = idx(b, q)
            if gi == 0 and si == 0:
                q += 2
            while q < len(b):
                k = b[q]; name = b[q + 1:q + 1 + k].decode('latin1'); q += 1 + k
                off = struct.unpack_from('<H', b, q)[0]; q += 2; _, q = idx(b, q)
                if segs[si] and segs[si][1] == 'CODE':
                    out.append((off, name))
    return sorted(out)


exe = open(sys.argv[1], 'rb').read()
tot = {'GLEICH': [0, 0], 'AEHNLICH': [0, 0], 'FEHLT': [0, 0]}
for obj in sorted(glob.glob(os.path.join(sys.argv[2], 'SEG*.OBJ'))):
    code = [c for c in omfcmp.parse_obj(obj) if c[1] == 'CODE']
    if not code:
        continue
    _, _, data, mask = code[0]
    pubs = publics(obj)
    for i, (off, name) in enumerate(pubs):
        end = pubs[i + 1][0] if i + 1 < len(pubs) else len(data)
        fd, fm = data[off:end], mask[off:end]
        n = len(fd)
        if n < 8:
            continue
        # laengster fester Block als Anker
        best = (0, 0); j = 0
        while j < n:
            if fm[j]:
                k = j
                while k < n and fm[k]:
                    k += 1
                if k - j > best[1] - best[0]:
                    best = (j, k)
                j = k
            else:
                j += 1
        a, b = best
        status = 'FEHLT'; where = -1; same = 0
        if b - a >= 6:
            anchor = bytes(fd[a:min(b, a + 32)]); s = 0
            cands = []
            while len(cands) < 200:
                h = exe.find(anchor, s)
                if h < 0:
                    break
                cands.append(h - a); s = h + 1
            fixed = sum(fm)
            for base in cands:
                if base < 0 or base + n > len(exe):
                    continue
                eq = sum(1 for t in range(n) if fm[t] and exe[base + t] == fd[t])
                if eq > same:
                    same, where = eq, base
            if fixed and same == fixed:
                status = 'GLEICH'
            elif fixed and same >= 0.9 * fixed:
                status = 'AEHNLICH'
        tot[status][0] += 1; tot[status][1] += n
        print(f'{os.path.basename(obj)}\t{name}\t{n}\t{status}\t{where:#x}' if where >= 0 else f'{os.path.basename(obj)}\t{name}\t{n}\t{status}\t-')
alln = sum(v[1] for v in tot.values()); allc = sum(v[0] for v in tot.values())
for k, (c, n) in tot.items():
    print(f'# {k}: {c} Funktionen ({100*c/allc:.0f} %), {n} Bytes ({100*n/alln:.0f} %)', file=sys.stderr)
