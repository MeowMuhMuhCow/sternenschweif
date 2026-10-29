#!/usr/bin/env python3
"""funcdiff.py ASM_OBJ STARTS C_OBJ... - je Funktion (Starts modulrelativ, hex, Komma) Laenge/Diffs.
Funktionen werden in beiden OBJs ueber die Reihenfolge zugeordnet (C-Starts aus PUBDEF/Funktionsbeginn 55 8B EC)."""
import sys; sys.path.insert(0,'/root/sternenschweif/tools'); import objbytes, objfix, omfinfo
W={0:1,1:2,2:2,3:4,4:1,5:2,9:2,11:4,13:4}
def masked(path):
    sb=objbytes.seg_bytes(path); seg=[s for s in sb if s.endswith('_TEXT')][0]
    m=set()
    for f in objfix.parse(path)['fix']:
        if f['seg']==seg: m.update(range(f['loc'],f['loc']+W.get(f['typ'],2)))
    return sb[seg],m
def pubstarts(path):
    s,p,e=omfinfo.info(path); return sorted(set(o for _,o,_ in p))
a,ma=masked(sys.argv[1]); st=[int(x,16) for x in sys.argv[2].split(',')]+[len(a)]
res=[]
for p in sys.argv[3:]:
    c,mc=masked(p); cs=pubstarts(p)
    # Funktionen ohne public (static) fehlen in cs -> nur zuordnen, wenn Anzahl passt
    if len(cs)!=len(st)-1: cs=None
    out=[]
    for k in range(len(st)-1):
        a0,a1=st[k],st[k+1]
        if cs:
            c0=cs[k]; c1=cs[k+1] if k+1<len(cs) else len(c)
        else:
            c0,c1=a0,a1
        n=min(a1-a0,c1-c0)
        d=sum(1 for i in range(n) if a[a0+i]!=c[c0+i] and (a0+i) not in ma and (c0+i) not in mc)+abs((a1-a0)-(c1-c0))
        out.append(d)
    res.append((sum(out),p,out))
for t,p,out in sorted(res)[:int(__import__('os').environ.get('TOP','10'))]: print(t,p.split('/')[-1],out)
