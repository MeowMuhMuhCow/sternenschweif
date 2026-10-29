#!/usr/bin/env python3
"""varaddr.py OBJ SEGID BASE - liest fuer jeden Verweis der OBJ auf eine externe Variable
die echte DGROUP-Adresse aus dem Original-Code an derselben Stelle.
Adresse = Originalwort - Verschiebung im OBJ (z. B. Feld +2)."""
import sys, struct, collections
sys.path.insert(0,'/root/sternenschweif/q004/zerl'); import schweif
def idx(b,q):
    v=b[q]; return ((((v&0x7F)<<8)|b[q+1]),q+2) if v&0x80 else (v,q+1)
def refs(path):
    d=open(path,'rb').read(); p=0; lnames=['']; segs=[None]; exts=['']; grps=['']; last=None; data=collections.defaultdict(bytearray); out=[]
    while p<len(d):
        t=d[p]; n=struct.unpack_from('<H',d,p+1)[0]; b=d[p+3:p+3+n-1]; p+=3+n
        if t==0x96:
            q=0
            while q<len(b): k=b[q]; lnames.append(b[q+1:q+1+k].decode('latin1')); q+=1+k
        elif t==0x98:
            acbp=b[0]; q=1+(3 if (acbp>>5)==0 else 0)+2; ni,q=idx(b,q); ci,q=idx(b,q); segs.append((lnames[ni],lnames[ci]))
        elif t==0x8C:
            q=0
            while q<len(b): k=b[q]; exts.append(b[q+1:q+1+k].decode('latin1')); q+=1+k; _,q=idx(b,q)
        elif t==0xA0:
            si,q=idx(b,0); off=struct.unpack_from('<H',b,q)[0]; last=(si,off); blk=b[q+2:]
            buf=data[si]
            if len(buf)<off+len(blk): buf.extend(b'\0'*(off+len(blk)-len(buf)))
            buf[off:off+len(blk)]=blk
        elif t==0x9C:
            q=0
            while q<len(b):
                x=b[q]
                if x&0x80:
                    locat=(x<<8)|b[q+1]; q+=2; loc=(locat>>10)&0xF; drec=locat&0x3FF; fd=b[q]; q+=1
                    F=fd&0x80; fr=(fd>>4)&7; T=fd&8; P=fd&4; tm=fd&3
                    if not F and fr<3: _,q=idx(b,q)
                    tg=None
                    if not T:
                        ti,q=idx(b,q)
                        if tm==2: tg=exts[ti]
                    disp=0
                    if not P: disp=struct.unpack_from('<H',b,q)[0]; q+=2
                    if segs[last[0]][1]=='CODE' and tg and loc==1:
                        out.append((last[1]+drec,tg,disp))
                else:
                    D=x&0x40; m=(x>>2)&7; q+=1
                    if (D and m<3) or (not D and m<4): _,q=idx(b,q)
    code=[v for k,v in data.items() if segs[k][1]=='CODE'][0]
    return out, code
if __name__=='__main__':
    obj,sid,base=sys.argv[1],sys.argv[2],int(sys.argv[3],16)
    R=[s for s in schweif.load('/root/sternenschweif/orig/SCHWEIF.EXE') if s['id']==sid][0]['code']
    out,code=refs(obj)
    for off,name,disp in out:
        o=struct.unpack_from('<H',R,base+off)[0]; inobj=struct.unpack_from('<H',code,off)[0]
        print(f'{name}\t{(o-inobj-disp)&0xffff:#06x}\t{sid}+{base+off:#06x}\t{inobj+disp}')
