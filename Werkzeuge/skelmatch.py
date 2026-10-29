#!/usr/bin/env python3
"""skelmatch.py - vergleicht Funktionen zweier EXEs am Befehlsgeruest.

Teilt jede EXE an den Borland-Funktionsanfaengen (55 8B EC = push bp / mov bp,sp),
disassembliert mit ndisasm und ersetzt alle Zahlen (Adressen, Variablen-Offsets,
Sprungziele, Konstanten) durch N. Gleiches Geruest = gleiche Funktion, nur mit
anderen Adressen.  Aufruf: skelmatch.py A.EXE B.EXE  -> Statistik + skel_A.tsv
"""
import sys, re, subprocess, hashlib, collections


def funcs(path):
    d = open(path, 'rb').read()
    hdr = int.from_bytes(d[8:10], 'little') * 16
    starts = []; p = hdr
    while True:
        p = d.find(b'\x55\x8b\xec', p)
        if p < 0:
            break
        starts.append(p); p += 1
    args = ['ndisasm', '-b16']
    for s in starts:
        args += ['-s', str(s)]
    out = subprocess.run(args + [path], capture_output=True, text=True).stdout
    ins = []
    for l in out.splitlines():
        m = re.match(r'([0-9A-F]{8})\s+([0-9A-F]+)\s+(.*)', l)
        if m:
            ins.append((int(m[1], 16), m[3]))
    res = []; si = set(starts); cur = None
    for off, txt in ins:
        if off in si:
            if cur:
                res.append(cur)
            cur = [off, []]
        if cur is not None:
            t = re.sub(r'0x[0-9a-f]+|\b\d+\b', 'N', txt)
            cur[1].append(t)
            if txt.startswith(('retf', 'ret')) and len(cur[1]) > 3:
                res.append(cur); cur = None
    if cur:
        res.append(cur)
    out = {}
    for off, sk in res:
        if len(sk) >= 6:
            out[off] = (hashlib.md5('\n'.join(sk).encode()).hexdigest(), len(sk))
    return out


a = funcs(sys.argv[1]); b = funcs(sys.argv[2])
hb = collections.Counter(h for h, _ in b.values())
hit = [(o, n) for o, (h, n) in a.items() if hb[h]]
print(f'{sys.argv[1]}: {len(a)} Funktionen, davon {len(hit)} mit gleichem Geruest in {sys.argv[2]} '
      f'({100*len(hit)/len(a):.0f} %), Befehle: {sum(n for _,n in hit)} von {sum(n for _,n in a.values())}')
with open('skel_' + sys.argv[1].split('/')[-1] + '.tsv', 'w') as f:
    for o, (h, n) in sorted(a.items()):
        f.write(f'{o:#x}\t{n}\t{h}\t{"JA" if hb[h] else "-"}\n')
