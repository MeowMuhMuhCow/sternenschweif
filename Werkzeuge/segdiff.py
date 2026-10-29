#!/usr/bin/env python3
"""segdiff.py - Abweichungen je Code-Segment zwischen gelinkter EXE und Original."""
import re,struct,sys
sys.path.insert(0,__file__.rsplit('/',1)[0]); import omfcmp
a=open(sys.argv[1],'rb').read(); b=open(sys.argv[2],'rb').read()
res=[]
for l in open(sys.argv[3]):
    m=re.match(r'(SEG\d+)\.OBJ\s+\S+\s+(\d+).*@0x([0-9a-f]+)',l)
    if not m: continue
    seg,n,po=m[1],int(m[2]),int(m[3],16)
    for name,cls,data,mask in omfcmp.parse_obj(seg+'.OBJ'):
        if cls!='CODE': continue
        pn,_=omfcmp.find(a,data,mask)
        if pn is None: res.append((-1,seg,'nicht gefunden')); continue
        d=[i for i in range(n) if a[pn+i]!=b[po+i]]
        if d: res.append((len(d),seg,' '.join(f'+{i:x}:{a[pn+i]:02x}/{b[po+i]:02x}' for i in d[:8])))
res.sort(reverse=True)
for r in res: print(f'{r[0]:4} {r[1]} {r[2]}')
print('Segmente mit Abweichung:',len(res),' Summe',sum(max(r[0],0) for r in res))
