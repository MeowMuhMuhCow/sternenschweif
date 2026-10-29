#!/usr/bin/env python3
"""datseg_gen.py - erzeugt DATSEG.ASM: Spiel-_DATA (0x0094..0x86b3) und Spiel-_BSS (0x8b4e..0xcb75)
von SCHWEIF.EXE als TASM-Quelle mit Marken an jeder genutzten Adresse.
Byte-Gleichheit per Konstruktion: alle Bytes kommen aus dem Original; Fernzeiger mit EXE-Relokation
werden als Fixups geschrieben (DGROUP-Zeiger: dw off,DGROUP; Code-Zeiger: dd extrn)."""
import struct, collections, csv, re, sys
sys.path.insert(0,'/root/sternenschweif/q004/zerl'); import schweif
X='/root/sternenschweif/'
D0,D1,B0,B1=0x94,0x86b4,0x8b4e,0xcb76
d=open(X+'orig/SCHWEIF.EXE','rb').read()
nrel,hdr=struct.unpack_from('<HH',d,6); relo=struct.unpack_from('<H',d,0x18)[0]; h=hdr*16
DGP=0x24eb; DG=d[h+DGP*16:h+DGP*16+52238]
segpara={int(r['para']):r['segment'] for r in csv.DictReader(open(X+'q004/zerl/segmente.tsv'),delimiter='\t')}
segs={s['id']:s for s in schweif.load(X+'orig/SCHWEIF.EXE')}
# Relokationen im DGROUP
rel={}
for i in range(nrel):
    o,s=struct.unpack_from('<HH',d,relo+4*i); a=s*16+o-DGP*16
    if 0<=a<52238:
        seg=struct.unpack_from('<H',DG,a)[0]; off=struct.unpack_from('<H',DG,a-2)[0]
        rel[a-2]=(segpara.get(seg,'DGROUP' if seg==DGP else None),off)
# Zugriffe
acc=collections.defaultdict(collections.Counter); users=collections.defaultdict(set); kinds=collections.defaultdict(set)
for l in open(X+'ds/zugriffe.tsv'):
    a,k,w,s,o,t=l.rstrip('\n').split('\t',5); a=int(a,16)
    acc[a][w]+=1; users[a].add(s); kinds[a].add(k)
names={}
for r in csv.DictReader(open(X+'ds/variablen.tsv'),delimiter='\t'):
    names[int(r['adresse'],16)]=r['name']
quelle={}
for r in csv.DictReader(open(X+'ds/namen_gesamt.tsv'),delimiter='\t'):
    names[int(r['adresse'],16)]=r['name']; quelle[int(r['adresse'],16)]=r['quelle']
sprechend=set(a for a,n in names.items() if not re.fullmatch(r'g_(str_)?[0-9a-f]{4}',n))
def inr(a): return D0<=a<D1 or B0<=a<B1
labels=set(a for a in acc if inr(a))|set(a for a in rel if inr(a))|set(a for a in names if inr(a))
# Zeiger-Ziele in DGROUP als Marken
for p,(t,off) in rel.items():
    if t=='DGROUP' and inr(off): labels.add(off)
# Strings: Anfang eines druckbaren, nullterminierten Laufs, der an einer Marke beginnt oder auf eine Marke folgt
PR=set(range(0x20,0x7f))|{0x81,0x84,0x8e,0x94,0x99,0x9a,0xe1,0x0a,0x0d}
def strlen(a,lim):
    n=0
    while a+n<lim and DG[a+n] in PR: n+=1
    return n if a+n<lim and DG[a+n]==0 else -1
strs={}
for m in re.finditer(rb'(?<=\x00)[\x20-\x7e\x81\x84\x8e\x94\x99\x9a\xe1\x0a\x0d]{3,}\x00',DG[D0-1:D1]):
    a=m.start()+D0-1; strs[a]=m.end()-m.start()
for a in list(labels):
    if D0<=a<D1:
        n=strlen(a,D1)
        if n>=1: strs[a]=n+1
labels|=set(strs)
# Hochwort eines Fernzeigers nicht als eigene Marke
dw_=set(a for a in labels if acc[a].most_common(1) and acc[a].most_common(1)[0][0]=='dword')|set(rel)
alias=set(a+2 for a in dw_ if a+2 in labels and a+2 not in names and a+2 not in rel and a+2 not in strs)
for p in rel:
    for k in (1,2,3):
        if p+k in labels and p+k not in names: alias.add(p+k)
labels-=alias
# Tabellen: innere Marken werden zu Feldern (Name der Tabelle + Abstand)
tabs={}; feld={}
for r in csv.DictReader(open(X+'ds/tabellen.tsv'),delimiter='\t'):
    st,S,n=int(r['anfang'],16),int(r['satzgroesse']),int(r['saetze'])
    end=st+S*n
    if st in strs: continue
    cut=[a for a in sprechend|set(strs) if st<a<end]
    if cut: end=st+((min(cut)-st)//S)*S; n=(end-st)//S
    if n<2: continue
    tabs[st]=(S,n); labels.add(st)
    for a in [x for x in labels if st<x<end and x not in sprechend]:
        feld[a]=st; labels.discard(a)
L=sorted(labels)
def nm(a):
    if a in names: return '_'+names[a] if not names[a].startswith('_') else names[a]
    return ('_s_%04x' if a in strs else '_g_%04x')%a
def ftarget(seg,off):
    s=segs[seg]
    if s['kind']=='O':
        i=(off-0x20)//5; return 'F_%s_%04X'%(seg,s['entries'][i])
    return 'F_%s_%04X'%(seg,off)
extrn=sorted({ftarget(t,o) for p,(t,o) in rel.items() if t not in ('DGROUP',None)})

def typ(a):
    if a in tabs: return f'tabelle[{tabs[a][1]}x{tabs[a][0]}]'
    if a in rel: return 'fptr'
    if a in strs: return 'string'
    c=acc[a]
    if not c: return '?'
    return c.most_common(1)[0][0] if c.most_common(1)[0][0] in ('byte','word','dword') else '?'
def ltype(t): return "dword" if t in ("fptr","dword") else "word" if t=="word" else "byte"
def emit(W,lo,hi,ext):
    p=lo
    while p<hi:
        if p in rel:
            t,off=rel[p]
            if t=='DGROUP': W(f'\tdw\t0{off:04x}h,DGROUP')
            else: f=ftarget(t,off); ext.add(f); W(f'\tdd\t{f}')
            p+=4; continue
        q=p+1
        while q<hi and q-p<16 and q not in rel: q+=1
        W('\tdb\t'+','.join(f'0{b:02x}h' for b in DG[p:q])); p=q
def emits(W,a,n):
    parts=[]; cur=''
    for b in DG[a:a+n]:
        if 0x20<=b<0x7f and b!=0x27: cur+=chr(b)
        else:
            if cur: parts.append("'"+cur+"'"); cur=''
            parts.append(f'0{b:02x}h')
    if cur: parts.append("'"+cur+"'")
    line=''
    for x in parts:
        if len(line)+len(x)>100: W('\tdb\t'+line.rstrip(',')); line=''
        line+=x+','
    W('\tdb\t'+line.rstrip(','))
tab=[]
HDR=['\tifndef ??version','?debug\tmacro','\tendm','\tendif','DGROUP\tgroup\t_DATA,_BSS','\tassume\tds:DGROUP']
def part(fname,lo,hi,LL,bss):
    body=[]; W=body.append; ext=set()
    W(f'_{"BSS" if bss else "DATA"}\tsegment word public \'{"BSS" if bss else "DATA"}\'')
    p=lo
    LL=[a for a in LL if lo<=a<hi]
    for i,a in enumerate(LL+[hi]):
        if a>p:
            if bss: W(f'\tdb\t{a-p} dup (?)')
            else: emit(W,p,a,ext)
        if a==hi: break
        nxt=(LL+[hi])[i+1]; n=nxt-a; t=typ(a)
        W(f'{nm(a)}\tlabel\t{ltype(t)}\t; {a:#06x} {t} {n}')
        W(f'\tpublic\t{nm(a)}')
        tab.append((a,nm(a).lstrip('_'),t+(' (BSS)' if bss else ''),n,sum(acc[a].values()),' '.join(sorted(users[a])[:8]),'' if bss else DG[a:a+min(n,16)].hex(),fname))
        if not bss and t=='string' and strs[a]<=n: emits(W,a,strs[a]); p=a+strs[a]
        else: p=a
    W(f'_{"BSS" if bss else "DATA"}\tends')
    if not bss: body.insert(0,'_BSS\tsegment word public \'BSS\'\n_BSS\tends')
    else: body.insert(0,'_DATA\tsegment word public \'DATA\'\n_DATA\tends')
    txt=[f'; {fname} - SCHWEIF.EXE DGROUP {lo:#06x}..{hi-1:#06x} ({"BSS" if bss else "DATA"}), erzeugt von datseg_gen.py']+HDR+[f'\textrn\t{e}:far' for e in sorted(ext)]+body+['\tend']
    open(X+'dsg/'+fname,'w',newline='\r\n').write('\n'.join(txt)+'\n')
    return len(LL)
DL=[a for a in L if D0<=a<D1]; BL=[a for a in L if B0<=a<B1]
# Teilung in Stuecke zu ~450 Marken, Grenzen gerade
cuts=[D0]
for k in range(450,len(DL),450):
    c=DL[k]
    if c%2: c-=1
    # nicht in Fernzeiger schneiden
    while any(p<c<p+4 for p in rel): c-=2
    cuts.append(c)
cuts.append(D1)
files=[]
for i in range(len(cuts)-1):
    fn=f'DS{i:02d}.ASM'; files.append(fn); part(fn,cuts[i],cuts[i+1],DL,False)
part('DSBSS.ASM',B0,B1,BL,True); files.append('DSBSS.ASM')
with open(X+'ds/datseg_marken.tsv','w') as f:
    f.write('adresse\tname\ttyp\tgroesse\tzugriffe\tsegmente\tanfang_hex\tdatei\n')
    for r in tab: f.write(f'{r[0]:#06x}\t'+'\t'.join(map(str,r[1:]))+'\n')
open(X+'dsg/teile.txt','w').write(' '.join(files)+'\n'+' '.join(hex(c) for c in cuts)+'\n')
c=collections.Counter(r[2] for r in tab)
print('Marken',len(tab),'DATA',len(DL),'BSS',len(BL),'Relokationen',len(rel),'Aliase',len(alias),'Dateien',files)
print(dict(c))
with open(X+'ds/tabellen_felder.tsv','w') as f:
    f.write('adresse\ttabelle\tsatz\tabstand_im_satz\tzugriffe\n')
    for a in sorted(feld):
        st=feld[a]; S=tabs[st][0] if st in tabs else 1
        f.write(f'{a:#06x}\t{nm(st).lstrip("_")}\t{(a-st)//S}\t{(a-st)%S}\t{sum(acc[a].values())}\n')
print('Tabellen',len(tabs),'Felder (aufgeloeste Marken)',len(feld))
