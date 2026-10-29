#!/usr/bin/env python3
"""libsyms.py - Adressen der oeffentlichen Bibliothekssymbole in R00/R47 (aus CL.LIB, OVERLAY.LIB, C0L.OBJ)
Ausgabe lib/lib_syms.tsv: segment, offset, name"""
import struct, sys
sys.path.insert(0,'/root/sternenschweif/lib'); import libsplit
ROOT='/root/sternenschweif'
def idx(b,p):
    v=b[p]
    if v&0x80: return ((v&0x7f)<<8)|b[p+1], p+2
    return v, p+1
def pubs(obj):
    p=0; lnames=['']; segs=[None]; out=[]
    while p<len(obj):
        t=obj[p]; n=struct.unpack_from('<H',obj,p+1)[0]; b=obj[p+3:p+3+n-1]; p+=3+n
        if t==0x96:
            q=0
            while q<len(b): l=b[q]; lnames.append(b[q+1:q+1+l].decode('latin1')); q+=1+l
        elif t in (0x98,0x99):
            q=1+(2 if t==0x98 else 4); nm,q=idx(b,q); cl,q=idx(b,q); segs.append((lnames[nm],lnames[cl]))
        elif t in (0x90,0x91):
            q=0; g,q=idx(b,q); s,q=idx(b,q)
            if s==0: q+=2
            while q<len(b):
                l=b[q]; name=b[q+1:q+1+l].decode('latin1'); q+=1+l
                off=struct.unpack_from('<H' if t==0x90 else '<I',b,q)[0]; q+=2 if t==0x90 else 4
                ti,q=idx(b,q); out.append((segs[s] if s else None,name,off))
        if t in (0x8a,0x8b): break
    return out
res=[]
mods={}
for lib in ('CL.LIB','OVERLAY.LIB'):
    for name,data in libsplit.split(f'{ROOT}/bc/LIB/{lib}'): mods[name.upper()]=data
place=[]
for f in ('schweif_cl.tsv','schweif_overlay.tsv'):
    for l in open(f'{ROOT}/lib/{f}'):
        sg,o,sz,m,segn=l.rstrip('\n').split('\t'); place.append((sg,int(o,16),m,segn))
# Doppelbelegungen (code-identische Module) nach alphabetischem Kontext aufloesen
place.sort(key=lambda x:(x[0],x[1]))
grp={}
for x in place: grp.setdefault((x[0],x[1]),[]).append(x)
keys=sorted(grp); chosen=[]
for i,k in enumerate(keys):
    c=grp[k]
    if len(c)==1: chosen.append(c[0]); continue
    prev=next((grp[keys[j]][0][2] for j in range(i-1,-1,-1) if len(grp[keys[j]])==1 and keys[j][0]==k[0]),'')
    nxt=next((grp[keys[j]][0][2] for j in range(i+1,len(keys)) if len(grp[keys[j]])==1 and keys[j][0]==k[0]),'~')
    fit=[x for x in c if prev.upper()<=x[2].upper()<=nxt.upper()]
    pick=fit[0] if len(fit)==1 else sorted(c,key=lambda x:x[2].startswith('F'))[0]
    print('doppelt',k,[x[2] for x in c],'Kontext',prev,nxt,'->',pick[2])
    chosen.append(pick)
place=chosen
miss=0
for sg,o,m,segn in place:
    key=[k for k in mods if k.split('.')[0]==m.upper() or k.upper().startswith(m.upper()+'.') or k.upper()==m.upper()]
    if not key: miss+=1; continue
    for seg,name,off in pubs(mods[key[0]]):
        if seg and seg[0]==segn: res.append((sg,o+off,name))
# C0L
for seg,name,off in pubs(open(f'{ROOT}/bc/LIB/C0L.OBJ','rb').read()):
    if seg and seg[0]=='_TEXT': res.append(('R00',off,name))
res=sorted(set(res))
open(f'{ROOT}/lib/lib_syms.tsv','w').write(''.join(f'{a}\t{b:#06x}\t{c}\n' for a,b,c in res))
print(len(res),'Symbole; Module ohne Treffer:',miss)
for r in res[:8]: print(r)
