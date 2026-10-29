#!/usr/bin/env python3
"""mapcheck.py - prueft, ob jede Variable aus datseg.cpp an ihrer Originaladresse liegt.

Liest die Kommentare '// ds:0xNNNN' aus datseg.cpp und die Adressen aus der
TLINK-MAP (Publics). Meldet jede Variable, die woanders gelandet ist.
Aufruf: mapcheck.py datseg.cpp BLADEM.MAP
"""
import re, sys
want = {}
for l in open(sys.argv[1], encoding='latin1'):
    m = re.search(r'\b(g_\w+|_stklen)\s*(\[[^\]]*\])?\s*(=.*)?;\s*// ds:0x([0-9a-fA-F]+)', l)
    if m and not l.lstrip().startswith('/*'):
        want[m[1].upper()] = int(m[4], 16)
have = {}
dgroup = None
for l in open(sys.argv[2], encoding='latin1'):
    m = re.match(r'\s*([0-9A-F]{4}):([0-9A-F]{4})\s+(?:idle\s+)?(\S+)\s*$', l)
    if m:
        have[m[3].upper()] = (int(m[1], 16), int(m[2], 16))
    m = re.match(r'\s*([0-9A-F]{4}):0000\s+[0-9A-F]{4}\s+C=DATA\s+S=_DATA\s+G=DGROUP', l)
    if m and dgroup is None:
        dgroup = int(m[1], 16)
ok = bad = missing = 0
wrong = []
for name, off in sorted(want.items(), key=lambda x: x[1]):
    sym = '_' + name
    if sym not in have:
        missing += 1; continue
    seg, o = have[sym]
    lin = (seg - dgroup) * 16 + o
    if lin == off:
        ok += 1
    else:
        bad += 1; wrong.append((name, off, lin))
print(f'{ok} Variablen an Originaladresse, {bad} verschoben, {missing} nicht in der MAP')
for n, w, h in wrong[:25]:
    print(f'  {n:32} soll {w:04X} ist {h:04X} ({h - w:+X})')
sys.exit(1 if bad else 0)
