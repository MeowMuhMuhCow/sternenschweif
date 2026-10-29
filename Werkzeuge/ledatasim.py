#!/usr/bin/env python3
"""ledatasim.py - prueft Regeln fuer Borlands LEDATA-Aufteilung gegen die Reloc-Reihenfolge des Originals."""
import struct,re,sys
def ld(f):
    d=open(f,'rb').read(); n=struct.unpack_from('<H',d,6)[0]; lf=struct.unpack_from('<H',d,0x18)[0]
    return [struct.unpack_from('<HH',d,lf+4*i) for i in range(n)]
rb=ld(sys.argv[1]); dat=sys.argv[2]
ds=[o-2 for o,s in rb if s==0x14fc and o-2<0xB4E0]   # Zeigerbeginn, nur Spiel-Initialdaten
vars_=[]
for l in open(dat,encoding='latin1'):
    if l.lstrip().startswith('/*'): continue
    m=re.search(r'// ds:0x([0-9a-fA-F]+)',l)
    if m and '=' in l.split('//')[0]: vars_.append(int(m[1],16))
vars_=sorted(set(v for v in vars_ if 0x94<=v<0xB4E0))+[0xB4E0]
def sim(maxlen, split_big):
    recs=[]; start=0x94; cur=0x94
    for a,b in zip(vars_,vars_[1:]):
        size=b-a
        if a-start+size>maxlen and a>start:
            recs.append((start,a)); start=a
        while split_big and b-start>maxlen:
            recs.append((start,start+maxlen)); start+=maxlen
    recs.append((start,0xB4E0))
    return recs
def check(recs):
    import bisect
    starts=[r[0] for r in recs]
    rid=lambda x: bisect.bisect_right(starts,x)-1
    bad=0; order=[]
    for x in ds:
        r=rid(x)
        if x+4>recs[r][1]: bad+=1     # Zeiger ueber Satzgrenze
        order.append((r,-x))
    pred=[x for _,x in sorted((r,-negx) for r,negx in [(rid(x),-x) for x in ds])]
    # erwartete TLINK-Reihenfolge: Satz aufsteigend, innerhalb absteigend
    exp=sorted(ds,key=lambda x:(rid(x),-x))
    return bad, sum(1 for p,q in zip(exp,ds) if p!=q)
for ml in (1024,1020,1016,1000,512):
    for sb in (True,False):
        r=sim(ml,sb); print(ml,sb,'Saetze',len(r),'Grenzverletzungen/abweichende Positionen:',check(r))
