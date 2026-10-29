#!/usr/bin/env python3
"""odis.py OBJ SEGID BASIS - Disassembly von neu (OBJ) und Original nebeneinander bis zur ersten Abweichung + Kontext."""
import sys
sys.path.insert(0,'/root/sternenschweif/q004/zerl'); sys.path.insert(0,'/root/sternenschweif/tools')
import schweif, omfcmp
obj,sid,base=sys.argv[1],sys.argv[2],int(sys.argv[3],16)
lo=int(sys.argv[4],16) if len(sys.argv)>4 else 0
R=[s for s in schweif.load('/root/sternenschweif/orig/SCHWEIF.EXE') if s['id']==sid][0]['code'][base:]
d=bytes([c for c in omfcmp.parse_obj(obj) if c[1]=='CODE'][0][2])
A=schweif.disasm(d); B=schweif.disasm(R)
import itertools
for a,b in itertools.zip_longest(A,B,fillvalue=(0,0,'')):
    if a[0]<lo and b[0]<lo: continue
    m=' ' if a[2]==b[2] else '*'
    print(f'{m} {a[0]:04x} {a[2]:34s} | {b[0]:04x} {b[2]}')
