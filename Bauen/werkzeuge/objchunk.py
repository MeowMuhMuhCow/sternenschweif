#!/usr/bin/env python3
import os as _swos; _SW = _swos.environ.get('SW_ROOT', '/root/sternenschweif')  # Bau-Paket
"""objchunk.py OBJ SEGID - baut LEDATA/FIXUPP einer Overlay-OBJ so um, dass TLINK die Relocations in derselben
Reihenfolge schreibt wie im Original (BCC: Bloecke <=1024 Bytes, Fixups im Block absteigend)"""
import sys,struct
sys.path.insert(0,_SW + '/q004/zerl'); import schweif
obj,sid=sys.argv[1],sys.argv[2]
if sid.startswith('R'):
    # residentes Modul: Reihenfolge aus der MZ-Relocationtabelle, modulrelativ
    import json
    L=json.load(open(_SW + '/lnk_ranges.json'))
    st=L['start'][sid]; en=st+L['mods'][sid][1]
    e=open(_SW + '/orig/SCHWEIF.EXE','rb').read()
    nr,ro=struct.unpack_from('<H',e,6)[0],struct.unpack_from('<H',e,0x18)[0]
    orig=[]
    for i in range(nr):
        o,sg=struct.unpack_from('<HH',e,ro+4*i); x=sg*16+o
        if st<=x<en: orig.append(x-st)
else:
    orig=[s for s in schweif.load(_SW + '/orig/SCHWEIF.EXE') if s['id']==sid][0]['fix']
def idx(b,q):
    v=b[q]; return (((v&0x7f)<<8)|b[q+1],q+2) if v&0x80 else (v,q+1)
def rec(t,body):
    r=bytes([t])+struct.pack('<H',len(body)+1)+body; return r+bytes([(-sum(r))&0xff])
d=open(obj,'rb').read(); p=0; recs=[]
while p<len(d):
    t=d[p]; n=struct.unpack_from('<H',d,p+1)[0]; recs.append((t,d[p:p+3+n])); p+=3+n
data={}; segidx=None; fix=[]; last=None; first_le=None; out_before=[]; out_after=[]
for i,(t,r) in enumerate(recs):
    b=r[3:-1]
    if t==0xa0:
        if first_le is None: first_le=i
        s,q=idx(b,0); off=struct.unpack_from('<H',b,q)[0]; segidx=s; last=off
        for k,x in enumerate(b[q+2:]): data[off+k]=x
    elif t==0x9c:
        q=0
        while q<len(b):
            if not b[q]&0x80: raise SystemExit('THREAD-Fixup nicht unterstuetzt')
            lo=(b[q]<<8)|b[q+1]; st=q; q+=2; fd=b[q]; q+=1
            F=(fd>>4)&7; T=fd&3; P=(fd>>2)&1
            if not fd&0x80 and F<3: _,q=idx(b,q)
            if not fd&8: _,q=idx(b,q)
            if not P: q+=2
            loc=last+(lo&0x3ff); typ=(lo>>10)&15
            fix.append([loc,typ,lo&0xfc00,b[st+2:q]])
    elif t==0xa2: raise SystemExit(obj+': LIDATA nicht unterstuetzt')
    elif first_le is None: out_before.append(r)
    elif t not in (0xa0,0x9c): out_after.append(r)
n=max(data)+1
code=bytes(data.get(i,0) for i in range(n))
# Reloc-Position je Fixup (Segmentwort)
for f in fix: f.append(f[0]+2 if f[1]==3 else f[0])
# Laeufe im Original
runs=[[orig[0]]] if orig else []
for x in orig[1:]:
    if x<runs[-1][-1]: runs[-1].append(x)
    else: runs.append([x])
pos2run={x:k for k,r in enumerate(runs) for x in r}
starts=[0]
for k in range(1,len(runs)):
    locs=[f[0] for f in fix if pos2run.get(f[4])==k]
    starts.append(min(locs) if locs else min(runs[k])-2)
starts=sorted(set(starts))+[n]
blocks=[]
for k,(a,b) in enumerate(zip(starts,starts[1:])):
    fs=[f for f in fix if a<=f[0]<b]
    rf=[f for f in fs if f[4] in pos2run]
    hi=max([f[0]+(4 if f[1]==3 else 2) for f in rf],default=a)
    if hi-a>1024: raise SystemExit(f'Block zu gross {a:#x}-{hi:#x}')
    # erster Block: alle Relocation-Fixups dieses Laufs; danach Stuecke <=1024 (nur Fixups ohne Listeneintrag)
    end1=min(b,a+1024)
    # Grenze so legen, dass kein Fixup-Feld zerschnitten wird
    def cut(e):
        for f in fs:
            w=4 if f[1]==3 else 2
            if f[0]<e<f[0]+w: return f[0]
        return e
    end1=cut(end1)
    blocks.append((a,end1,[f for f in fs if f[0]<end1]))
    q=end1
    while q<b:
        e=cut(min(b,q+1024)); blocks.append((q,e,[f for f in fs if q<=f[0]<e])); q=e
out=list(out_before)
for a,b,fs in blocks:
    out.append(rec(0xa0,bytes([segidx])+struct.pack('<H',a)+code[a:b]))
    if fs:
        body=b''
        for loc,typ,hib,rest,_ in sorted(fs,key=lambda f:-f[0]):
            v=hib|(loc-a); body+=bytes([v>>8,v&0xff])+rest
        out.append(rec(0x9c,body))
out+=out_after
open(obj,'wb').write(b''.join(out))
print(sid,'Bloecke',len(blocks),'Fixups',len(fix))
