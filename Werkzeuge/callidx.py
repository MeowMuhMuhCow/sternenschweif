#!/usr/bin/env python3
"""callidx.py MOD OFFSET - zeigt fuer die abweichende Fixup-Stelle, der wievielte
Verweis auf dasselbe Ziel innerhalb der Funktion es ist."""
import sys,struct,bisect
mod,off=sys.argv[1],int(sys.argv[2],16)
def records(d):
    p=0
    while p<len(d):
        t=d[p]; ln=struct.unpack_from('<H',d,p+1)[0]; yield t,d[p+3:p+3+ln-1]; p+=3+ln
def idx(b,q):
    v=b[q]; return ((((v&0x7F)<<8)|b[q+1]),q+2) if v&0x80 else (v,q+1)
d=open(mod+'.OBJ','rb').read()
lnames=['']; segs=[None]; exts=['']; pubs=[]; last=None; fx=[]
for t,b in records(d):
    if t==0x96:
        q=0
        while q<len(b): n=b[q]; lnames.append(b[q+1:q+1+n].decode()); q+=1+n
    elif t==0x98:
        acbp=b[0]; q=1+(3 if (acbp>>5)==0 else 0)+2; ni,q=idx(b,q); ci,q=idx(b,q); segs.append((lnames[ni],lnames[ci]))
    elif t==0x8C:
        q=0
        while q<len(b): n=b[q]; exts.append(b[q+1:q+1+n].decode()); q+=1+n; _,q=idx(b,q)
    elif t==0x90:
        q=0; gi,q=idx(b,q); si,q=idx(b,q)
        if gi==0 and si==0: q+=2
        while q<len(b):
            n=b[q]; nm=b[q+1:q+1+n].decode(); q+=1+n; o=struct.unpack_from('<H',b,q)[0]; q+=2; _,q=idx(b,q)
            if segs[si][1]=='CODE': pubs.append((o,nm))
    elif t==0xA0:
        si,q=idx(b,0); last=(si,struct.unpack_from('<H',b,q)[0])
    elif t==0x9C:
        q=0
        while q<len(b):
            x=b[q]
            if x&0x80:
                locat=(x<<8)|b[q+1]; q+=2; drec=locat&0x3FF; fd=b[q]; q+=1
                F=fd&0x80; fr=(fd>>4)&7; T=fd&8; P=fd&4; tm=fd&3
                if not F and fr<3: _,q=idx(b,q)
                tg=None
                if not T:
                    ti,q=idx(b,q); tg=exts[ti] if tm==2 else None
                if not P: q+=2
                if segs[last[0]][1]=='CODE' and tg: fx.append((last[1]+drec,tg))
            else:
                D=x&0x40; m=(x>>2)&7; q+=1
                if (D and m<3) or (not D and m<4): _,q=idx(b,q)
pubs.sort(); fx.sort()
i=bisect.bisect_right([p[0] for p in pubs],off)-1; fs,fn=pubs[i]; fe=pubs[i+1][0] if i+1<len(pubs) else 1<<20
tgt=[t for o,t in fx if o==off][0]
same=[o for o,t in fx if t==tgt and fs<=o<fe]
print(f'{mod}+{off:X}: {fn}, Ziel {tgt}, Verweis Nr. {same.index(off)+1} von {len(same)} in dieser Funktion')
