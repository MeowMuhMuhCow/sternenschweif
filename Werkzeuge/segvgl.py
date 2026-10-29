#!/usr/bin/env python3
"""segvgl.py OBJ SEGID BASIS - vergleicht das ganze Code-Segment der OBJ mit dem Original ab BASIS.
9A (call far) gegen 90 0E E8 (vom Linker nah gemacht) gilt als gleich, Fixup-Stellen sind ausgeblendet."""
import sys, os
sys.path.insert(0,'/root/sternenschweif/q004/zerl'); sys.path.insert(0,'/root/sternenschweif/tools')
import schweif, omfcmp
obj,sid,base=sys.argv[1],sys.argv[2],int(sys.argv[3],16)
R=[s for s in schweif.load('/root/sternenschweif/orig/SCHWEIF.EXE') if s['id']==sid][0]['code']
_,_,data,mask=[c for c in omfcmp.parse_obj(obj) if c[1]=='CODE'][0]
n=len(data); orig=R[base:base+n]; bad=[]
i=0
while i<n:
    if not mask[i] or (i<len(orig) and orig[i]==data[i]): i+=1; continue
    if data[i]==0x9A and orig[i:i+3]==b'\x90\x0e\xe8': i+=5; continue
    bad.append(i); i+=1
print(f'{sid}: neu {n} Bytes, Original ab {base:#x} noch {len(R)-base} Bytes, Abweichungen {len(bad)}', 'GLEICH' if not bad and n==len(R)-base else '')
if bad:
    b=bad[0]; lo=max(0,b-12)
    print('erste Abweichung bei +%#x (Original +%#x)'%(b,b+base))
    print(' neu :',bytes(data[lo:b+12]).hex(' ')); print(' orig:',orig[lo:b+12].hex(' '))
