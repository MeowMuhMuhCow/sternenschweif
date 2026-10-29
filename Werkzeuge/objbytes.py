#!/usr/bin/env python3
"""objbytes.py - Bytes eines Segments aus LEDATA (ohne Fixup-Anwendung)"""
import struct
def idx(b,q):
    v=b[q]; return (((v&0x7f)<<8)|b[q+1],q+2) if v&0x80 else (v,q+1)
def seg_bytes(path, segname=None):
    d=open(path,'rb').read(); p=0; ln=['']; segs=['']; data={}
    while p<len(d):
        t=d[p]; n=struct.unpack_from('<H',d,p+1)[0]; b=d[p+3:p+2+n]; p+=3+n
        if t==0x96:
            q=0
            while q<len(b): l=b[q]; ln.append(b[q+1:q+1+l].decode('latin1')); q+=1+l
        elif t==0x98:
            a=b[0]; q=1+(3 if a>>5==0 else 0)+2; ni,q=idx(b,q); segs.append(ln[ni])
        elif t==0xa0:
            si,q=idx(b,0); off=struct.unpack_from('<H',b,q)[0]
            s=segs[si]
            if segname is None or s==segname:
                dd=data.setdefault(s,{})
                for k,x in enumerate(b[q+2:]): dd[off+k]=x
    out={}
    for s,dd in data.items():
        n=max(dd)+1; out[s]=bytes(dd.get(i,0) for i in range(n))
    return out
