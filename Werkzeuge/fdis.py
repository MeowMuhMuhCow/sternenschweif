#!/usr/bin/env python3
"""fdis.py OBJ OBJOFF SEGID ORIGOFF LAENGE - zwei Codeabschnitte nebeneinander disassemblieren."""
import sys, itertools
sys.path.insert(0,'/root/sternenschweif/q004/zerl'); sys.path.insert(0,'/root/sternenschweif/tools')
import schweif, omfcmp
obj,oo,sid,ro,n=sys.argv[1],int(sys.argv[2],16),sys.argv[3],int(sys.argv[4],16),int(sys.argv[5],16)
R=[s for s in schweif.load('/root/sternenschweif/orig/SCHWEIF.EXE') if s['id']==sid][0]['code'][ro:ro+n]
d=bytearray([c for c in omfcmp.parse_obj(obj) if c[1]=='CODE'][0][2])[oo:oo+n]
i=0
while i<len(d)-4:
    if d[i]==0x9a and R[i:i+3]==b'\x90\x0e\xe8': d[i:i+5]=R[i:i+5]
    i+=1
d=bytes(d)
for a,b in itertools.zip_longest(schweif.disasm(d),schweif.disasm(R),fillvalue=(0,0,'')):
    print(('  ' if a[2]==b[2] else '* ')+f'{a[0]:04x} {a[2]:36s}| {b[0]:04x} {b[2]}')
