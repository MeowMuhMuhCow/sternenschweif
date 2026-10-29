"""tabellen2.py - leitet Tabellen (Felder mit Satzgroesse) ab:
 (1) indizierte Zugriffe mit erkannter Schrittweite, Basen gleicher Schrittweite innerhalb eines Satzes = Felder,
 (2) Folgen von Fernzeigern (Relokationen im Abstand 4 oder fester Satzgroesse).
Ausdehnung: Saetze so lange, bis eine Marke auftaucht, die zu keinem Feld passt."""
import collections, csv, sys
X='/root/sternenschweif/'
exec(open(X+'ds/datseg_gen.py').read().split('# Zugriffe')[0])   # DG, rel, D0..B1
acc=set(); W=collections.defaultdict(set); K=collections.defaultdict(set)
for l in open(X+'ds/zugriffe.tsv'):
    a,k,w,*_=l.split('\t'); a=int(a,16); acc.add(a); K[a].add(k)
    if w in ('byte','word','dword'): W[a].add(w)
import re as _re
strs=set(m.start()+D0-1 for m in _re.finditer(rb'(?<=\x00)[\x20-\x7e\x81\x84\x8e\x94\x99\x9a\xe1]{3,}\x00',DG[D0-1:D1]))
idx=[l.rstrip('\n').split('\t') for l in open(X+'ds/indiziert.tsv')]
by=collections.defaultdict(collections.Counter); FW=collections.defaultdict(set)
for b,s,w,sid,o,t in idx:
    if s!='None' and s!='1': by[int(s)][int(b,16)]+=1; FW[(int(s),int(b,16))].add(w)
ibases=set(int(r[0],16) for r in idx)
cands=[]
for S,c in by.items():
    bases=sorted(c)
    i=0
    while i<len(bases):
        j=i
        while j+1<len(bases) and bases[j+1]-bases[i]<S: j+=1
        grp=bases[i:j+1]; cands.append((grp[0],S,{x-grp[0]:FW[(S,x)] for x in grp},sum(c[x] for x in grp),'indiziert'))
        i=j+1
# Fernzeiger-Folgen
R=sorted(p for p in rel if D0<=p<D1)
i=0
while i<len(R):
    j=i
    while j+1<len(R) and R[j+1]-R[j]==R[i+1]-R[i] if i+1<len(R) else False: j+=1
    if j-i>=2:
        S=R[i+1]-R[i]; cands.append((R[i],S,{0:{'dword'},2:{'word'}},j-i+1,'zeiger'))
        i=j+1
    else: i+=1
labels=sorted(acc|set(rel)|set(r+2 for r in rel))
import bisect
out=[]
for start,S,fields,n,kind in sorted(cands,key=lambda r:(r[0],r[1])):
    fs={f%S:w for f,w in fields.items()}
    def ok(a):
        if a>=start+S and a in ibases: return False
        f=(a-start)%S
        if kind=='zeiger': return a in rel or a-2 in rel
        if f not in fs or a in strs or a in rel: return False
        if W[a] and fs[f] and not W[a]<=fs[f]: return False
        return True
    # nach vorn erweitern: Saetze, deren Marken alle Felder sind
    k=0; last=start
    while True:
        lo=start+k*S; hi=lo+S
        if hi>(D1 if start<D1 else B1): break
        L=[a for a in labels[bisect.bisect_left(labels,lo):bisect.bisect_left(labels,hi)]]
        if k>0 and not all(ok(a) for a in L): break
        if L: last=hi
        k+=1
        if k>4096: break
    cnt=(last-start)//S
    if cnt<2: continue
    out.append((start,S,cnt,sorted(fs),n,kind))
# ueberlappende zusammenfassen: groessere Abdeckung gewinnt
out.sort(key=lambda r:(r[0],-(r[1]*r[2])))
final=[]; end=-1
for r in out:
    if r[0]<end: continue
    final.append(r); end=r[0]+r[1]*r[2]
with open(X+'ds/tabellen.tsv','w') as f:
    f.write('anfang\tsatzgroesse\tsaetze\tbytes\tfelder\tbelege\tquelle\n')
    for s,S,cnt,fs,n,kind in final: f.write(f'{s:#06x}\t{S}\t{cnt}\t{S*cnt}\t{",".join(map(str,fs))}\t{n}\t{kind}\n')
print('Tabellen',len(final),'Bytes',sum(r[1]*r[2] for r in final))
cov=collections.Counter(r[5] for r in final); print(cov)
for r in final[:60]: print(f'{r[0]:#06x} S={r[1]} n={r[2]} felder={r[3]} belege={r[4]} {r[5]}')
