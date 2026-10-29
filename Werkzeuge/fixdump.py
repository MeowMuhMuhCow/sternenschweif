#!/usr/bin/env python3
"""fixdump.py OBJ - zeigt FIXUPP-Eintraege (Lage, Methode, Frame, Target)"""
import sys,struct
d=open(sys.argv[1],'rb').read(); p=0; names=['']; ext=['']; segs=['']; last=None
def idx(b,q):
    v=b[q]; return (((v&0x7f)<<8)|b[q+1],q+2) if v&0x80 else (v,q+1)
while p<len(d):
    t=d[p]; n=struct.unpack_from('<H',d,p+1)[0]; b=d[p+3:p+3+n-1]; p+=3+n
    if t==0x96:
        q=0
        while q<len(b): l=b[q]; names.append(b[q+1:q+1+l].decode('latin1')); q+=1+l
    elif t==0x98: segs.append(names[b[3]])
    elif t in (0x8c,0xb4):
        q=0
        while q<len(b): l=b[q]; ext.append(b[q+1:q+1+l].decode('latin1')); q+=1+l; _,q=idx(b,q)
    elif t==0xa0: last=struct.unpack_from('<H',b,1)[0]
    elif t==0x9c:
        q=0
        while q<len(b):
            if b[q]&0x80:
                lo=(b[q]<<8)|b[q+1]; q+=2; m=(lo>>14)&1; loc=(lo>>10)&15; off=lo&0x3ff
                fd=b[q]; q+=1
                F=(fd>>4)&7; T=fd&3; P=(fd>>2)&1; fth=fd&0x80; tth=fd&8
                fr=None; tg=None
                if not fth and F<3: fr,q=idx(b,q)
                if not tth: tg,q=idx(b,q)
                disp=None
                if not P: disp=struct.unpack_from('<H',b,q)[0]; q+=2
                tname={0:lambda x:'SEG '+segs[x],1:lambda x:'GRP%d'%x,2:lambda x:'EXT '+ext[x]}.get(T,lambda x:str(x))(tg) if tg is not None else '-'
                print(f'@{last+off:04x} {"seg" if m else "self"} loc={loc} F{F}{"" if fr is None else "("+str(fr)+")"} T{T} {tname} disp={disp}')
            else:
                q+=1
                # THREAD-Subrecord (selten)
