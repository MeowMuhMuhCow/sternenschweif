#!/usr/bin/env python3
"""exediff.py - Abweichungen zwischen Nachbau-EXE und Original nach Bereichen (per MAP)."""
import sys, struct, re
from collections import Counter
def ld(f):
    d=open(f,'rb').read(); h=struct.unpack_from('<H',d,8)[0]*16; l,p=struct.unpack_from('<HH',d,2); e=(p-1)*512+l
    return d,h,e
a,ha,ea=ld(sys.argv[1]); b,hb,eb=ld(sys.argv[2])
segs=[]
for l in open(sys.argv[3],encoding='latin1'):
    m=re.match(r'\s*([0-9A-F]{5})H ([0-9A-F]{5})H ([0-9A-F]{5})H (\S+)\s+(\S+)',l)
    if m: segs.append((int(m[1],16),int(m[3],16),m[4],m[5]))
ia,ib=a[ha:ea],b[hb:eb]
print('Header: neu',ha,'orig',hb,' Image neu',len(ia),'orig',len(ib),' Overlays neu',len(a)-ea,'orig',len(b)-eb)
c=Counter()
for i in range(min(len(ia),len(ib))):
    if ia[i]!=ib[i]:
        name='?'
        for s,ln,n,cl in segs:
            if s<=i<s+max(ln,1) and cl!=':OVY': name=n+'/'+cl; break
        c[name]+=1
print('Image-Abweichungen:',sum(c.values()))
for k,v in c.most_common(30): print(f'  {v:6}  {k}')
oa,ob=a[ea:],b[eb:]
print('Overlay-Abweichungen:',sum(1 for x,y in zip(oa,ob) if x!=y))
