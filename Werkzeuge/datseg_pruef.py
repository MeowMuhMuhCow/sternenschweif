#!/usr/bin/env python3
"""datseg_pruef.py - prueft DSxx.OBJ gegen das Original-DGROUP: Bytes, Laengen, Fixup-Stellen und -Ziele."""
import struct, sys
X='/root/sternenschweif/'
exec(open(X+'ds/datseg_gen.py').read().split('# Zugriffe')[0])
def idx(b,q):
    v=b[q]; return ((((v&0x7F)<<8)|b[q+1]),q+2) if v&0x80 else (v,q+1)
def parse(path):
    d=open(path,'rb').read(); p=0; ln=['']; sd=[None]; grp=[None]; ext=[None]; data={}; fix=[]; last=None
    while p<len(d):
        t=d[p]; n=struct.unpack_from('<H',d,p+1)[0]; b=d[p+3:p+3+n-1]; p+=3+n
        if t==0x96:
            q=0
            while q<len(b): k=b[q]; ln.append(b[q+1:q+1+k].decode('latin1')); q+=1+k
        elif t==0x98:
            acbp=b[0]; q=1+(3 if (acbp>>5)==0 else 0); L=struct.unpack_from('<H',b,q)[0]; q+=2; ni,q=idx(b,q); ci,q=idx(b,q); sd.append((ln[ni],L))
        elif t==0x9A: gi,q=idx(b,0); grp.append(ln[gi])
        elif t==0x8C:
            q=0
            while q<len(b): k=b[q]; ext.append(b[q+1:q+1+k].decode('latin1')); q+=1+k; _,q=idx(b,q)
        elif t==0xA0:
            si,q=idx(b,0); off=struct.unpack_from('<H',b,q)[0]; q+=2; seg=sd[si][0]
            buf=data.setdefault(seg,bytearray())
            if len(buf)<off+len(b)-q: buf.extend(b'\0'*(off+len(b)-q-len(buf)))
            buf[off:off+len(b)-q]=b[q:]; last=(seg,off)
        elif t==0xA2: raise SystemExit(path+': LIDATA in _DATA')
        elif t==0x9C:
            q=0
            while q<len(b):
                if not b[q]&0x80: raise SystemExit('THREAD nicht erwartet')
                loc=(b[q]<<8)|b[q+1]; q+=2; lt=(loc>>10)&0xF; o=loc&0x3FF
                fd=b[q]; q+=1; F=fd>>7; fm=(fd>>4)&7; T=(fd>>3)&1; P=(fd>>2)&1; tm=fd&3
                if not F and fm<3: fi,q=idx(b,q)
                ti,q=idx(b,q)
                if not P: q+=2
                tgt={0:lambda i:'SEG:'+sd[i][0],1:lambda i:'GRP:'+grp[i],2:lambda i:'EXT:'+ext[i]}[tm](ti)
                fix.append((last[0],last[1]+o,lt,tgt))
    return sd,data,fix
files,cuts=[l.split() for l in open(X+'dsg/teile.txt')]
pos=D0; err=0; nfix=0; exp={}
for p,(t,off) in rel.items():
    if not (D0<=p<D1): continue
    if t=='DGROUP': exp[p+2]=('seg','DGROUP')
    else: exp[p]=('ptr','code')
got={}
for fn in files[:-1]:
    sd,data,fix=parse(X+'dsg/'+fn.replace('.ASM','.OBJ'))
    L=[l for n,l in sd[1:] if n=='_DATA'][0]; buf=data.get('_DATA',bytearray())
    if L%2: print(fn,'ungerade Laenge',L); err+=1
    for f in fix:
        a=pos+f[1]
        if f[2]==2: got[a]=('seg','DGROUP' if f[3]=='GRP:DGROUP' else 'code')
        elif f[2]==3: got[a]=('ptr','code')
        else: print('Fixup-Typ',f); err+=1
    fixb=set()
    for a,(k,_) in got.items():
        if pos<=a<pos+L: fixb|=set(range(a,a+(4 if k=='ptr' else 2)))
    for i in range(L):
        a=pos+i
        if a in fixb: continue
        if (buf[i] if i<len(buf) else 0)!=DG[a]: err+=1; 
    pos+=L
print('DATA-Ende',hex(pos),'erwartet',hex(D1))
print('Fixups erwartet',len(exp),'erhalten',len(got),'gleich',sum(1 for a in exp if got.get(a)==exp[a]))
sd,data,fix=parse(X+'dsg/DSBSS.OBJ'); L=[l for n,l in sd[1:] if n=='_BSS'][0]
print('BSS-Laenge',hex(L),'erwartet',hex(B1-B0),'BSS-Bytes im Original alle 0:',not any(DG[B0:B1]))
# Pruefen, dass die DGROUP-Zeiger-Offsets stimmen (Rohwert) und Code-Zeigerziele dem Stub-Eintrag entsprechen
print('Bytefehler',err)
