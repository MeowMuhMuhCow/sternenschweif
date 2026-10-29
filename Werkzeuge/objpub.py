#!/usr/bin/env python3
"""objpub.py OBJ [NAME,NAME,...] - zeigt PUBDEF-Reihenfolge; mit Namensliste: schreibt PUBDEFs in dieser Reihenfolge neu"""
import sys, struct
def recs(d):
    p=0
    while p<len(d):
        t=d[p]; n=struct.unpack_from('<H',d,p+1)[0]; yield t,d[p:p+3+n]; p+=3+n
def idx(b,p):
    v=b[p]
    return (((v&0x7f)<<8)|b[p+1],p+2) if v&0x80 else (v,p+1)
def pubentries(r):
    t=r[0]; b=r[3:-1]; q=0; g,q=idx(b,q); s,q=idx(b,q); hdr=b[:q]
    if s==0: hdr=b[:q+2]; q+=2
    out=[]
    while q<len(b):
        st=q; l=b[q]; name=b[q+1:q+1+l].decode('latin1'); q+=1+l+(2 if t==0x90 else 4); _,q=idx(b,q)
        out.append((name,hdr,b[st:q]))
    return out
def mk(t,body):
    r=bytes([t])+struct.pack('<H',len(body)+1)+body
    return r+bytes([(-sum(r))&0xff])
d=open(sys.argv[1],'rb').read()
allp=[]; first=None; rest=[]
for t,r in recs(d):
    if t in (0x90,0x91):
        if first is None: first=len(rest); rest.append(None)
        allp+= [(n,h,e,t) for n,h,e in pubentries(r)]
    else: rest.append(r)
if len(sys.argv)<3:
    print([n for n,_,_,_ in allp]); sys.exit()
order=sys.argv[2].split(',')
rank={n:i for i,n in enumerate(order)}
allp.sort(key=lambda x:rank.get(x[0],len(order)+1))
newp=b''.join(mk(t,h+e) for n,h,e,t in allp)
out=b''.join(newp if r is None else r for r in rest)
open(sys.argv[1],'wb').write(out); print('PUBDEF neu geordnet:',[n for n,_,_,_ in allp])
