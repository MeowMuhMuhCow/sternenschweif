#!/usr/bin/env python3
"""omfinfo.py OBJ... - Segmente (Name, Klasse, Laenge), PUBDEFs (je Segment), EXTDEFs einer OMF-OBJ"""
import sys,struct
def idx(b,q):
    v=b[q]; return (((v&0x7f)<<8)|b[q+1],q+2) if v&0x80 else (v,q+1)
def info(path):
    d=open(path,'rb').read(); p=0; ln=['']; segs=[None]; pubs=[]; ext=['']; grp=['']
    while p<len(d):
        t=d[p]; n=struct.unpack_from('<H',d,p+1)[0]; b=d[p+3:p+2+n]; p+=3+n
        if t==0x96:
            q=0
            while q<len(b): l=b[q]; ln.append(b[q+1:q+1+l].decode('latin1')); q+=1+l
        elif t==0x98:
            a=b[0]; q=1+(3 if a>>5==0 else 0); L=struct.unpack_from('<H',b,q)[0]; q+=2
            if a&2: L=0x10000
            ni,q=idx(b,q); ci,q=idx(b,q); segs.append((ln[ni],ln[ci],L,a>>5))
        elif t==0x9a:
            ni,q=idx(b,0); grp.append(ln[ni])
        elif t in (0x90,0x91):
            gi,q=idx(b,0); si,q=idx(b,q)
            if si==0: q+=2
            while q<len(b):
                l=b[q]; nm=b[q+1:q+1+l].decode('latin1'); q+=1+l; off=struct.unpack_from('<H',b,q)[0]; q+=2; _,q=idx(b,q)
                pubs.append((segs[si][0] if si else '-',off,nm))
        elif t in (0x8c,0xb4):
            q=0
            while q<len(b): l=b[q]; ext.append(b[q+1:q+1+l].decode('latin1')); q+=1+l; _,q=idx(b,q)
    return segs[1:],pubs,ext[1:]
if __name__=='__main__':
    for f in sys.argv[1:]:
        s,p,e=info(f)
        print('==',f); print('  Segmente:',[(a,c,hex(l)) for a,c,l,_ in s if l])
        print('  Publics:',len(p),' Externs:',len(e))
