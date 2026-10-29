"""namen_code.py - uebertraegt DSA-1-Variablennamen ueber den Code: aehnliche Funktionspaare (namen.tsv)
werden Befehl fuer Befehl ausgerichtet, DGROUP-Adressen an gleichen Stellen ergeben Stimmen DSA2->DSA1."""
import sys, re, csv, collections, difflib, bisect
sys.path.insert(0,'/root/sternenschweif/q004/zerl'); sys.path.insert(0,'/root/sternenschweif/ds')
import schweif, eich_dsa1, dsa1_vars
MINSIM=float(sys.argv[1]) if len(sys.argv)>1 else 0.4
segs1=schweif.load('/root/sternenschweif/orig/SCHICKM.EXE'); S1={s['id']:s for s in segs1}; F1=schweif.functions(segs1)
pubs,_=eich_dsa1.truth(segs1,'/root/sternenschweif/dsa1/BLADEM.MAP')
byname={}
for sid,P in pubs.items():
    for o,n in P: byname.setdefault(n,(sid,o))
def fend(F,S,sid,o):
    L=[x[0] for x in F.get(sid,[])]; i=bisect.bisect_right(L,o)
    return L[i] if i<len(L) else len(S[sid]['code'])
segs2=schweif.load('/root/sternenschweif/orig/SCHWEIF.EXE'); S2={s['id']:s for s in segs2}
mem=re.compile(r'(?:(byte|word|dword)\s+)?\[([^\]]+)\]')
def items(code):
    raw=[t for o,ln,t in schweif.disasm(code)]; out=[]; i=0
    while i<len(raw):
        t=raw[i]
        if t=='nop' and i+2<len(raw) and raw[i+1]=='push cs' and raw[i+2].startswith('call'): out.append(('CALL',raw[i+2])); i+=3; continue
        if t=='push cs' and i+1<len(raw) and raw[i+1].startswith('call'): out.append(('CALL',raw[i+1])); i+=2; continue
        if t.startswith('call 0x') and ':' in t: out.append(('CALL',t)); i+=1; continue
        out.append((re.sub(r'0x[0-9a-f]+|\b\d+\b','N',t),t)); i+=1
    return out
def dsaddr(t,prev):
    r=[]
    for m in mem.finditer(t):
        e=m.group(2)
        if re.match(r'(es|cs|ss):',e) or 'bp' in e or 'sp' in e: continue
        e=e.replace('ds:','')
        hx=re.findall(r'0x([0-9a-f]+)',e)
        if hx and '-' not in e: r.append(int(hx[-1],16))
    m=re.match(r'push (?:word )?0x([0-9a-f]+)$',t)
    if m and prev=='push ds': r.append(int(m.group(1),16))
    return r
votes=collections.defaultdict(collections.Counter); pairs=0
for row in csv.DictReader(open('/root/sternenschweif/q004/zerl/namen.tsv'),delimiter='\t'):
    if float(row['aehnlichkeit'])<MINSIM or row['name_dsa1'] not in byname: continue
    sid1,o1=byname[row['name_dsa1']]; e1=fend(F1,S1,sid1,o1)
    sid2,o2=row['segment'],int(row['start'],16); e2=o2+int(row['groesse'])
    A=items(S2[sid2]['code'][o2:e2]); B=items(S1[sid1]['code'][o1:e1]); pairs+=1
    sm=difflib.SequenceMatcher(None,[x[0] for x in A],[x[0] for x in B],autojunk=False)
    for tag,i1,i2,j1,j2 in sm.get_opcodes():
        if tag=='equal' or (tag=='replace' and i2-i1==j2-j1):
            for k in range(i2-i1):
                ta=A[i1+k]; tb=B[j1+k]
                if tag=='replace' and re.sub(r'\[[^\]]*\]','M',ta[0])!=re.sub(r'\[[^\]]*\]','M',tb[0]): continue
                pa=A[i1+k-1][1] if i1+k else ''; pb=B[j1+k-1][1] if j1+k else ''
                xa=dsaddr(ta[1],pa); xb=dsaddr(tb[1],pb)
                if len(xa)==1 and len(xb)==1: votes[xa[0]][(xb[0],row['name_dsa1'])]+=1
V=dsa1_vars.load(); VA=[v['addr'] for v in V]
def vname(a):
    i=bisect.bisect_right(VA,a)-1
    if i<0: return None
    v=V[i]; off=a-v['addr']
    if v['size'] and off>=v['size']: return None
    return v['name'] if off==0 else f"{v['name']}+{off}"
res=[]
for a2,c in votes.items():
    tot=collections.Counter()
    for (a1,fn),n in c.items(): tot[a1]+=n
    a1,n=tot.most_common(1)[0]; allv=sum(tot.values())
    res.append((a2,a1,vname(a1),n,allv,len({fn for (x,fn) in c if x==a1})))
res.sort()
with open('/root/sternenschweif/ds/namen_code.tsv','w') as f:
    f.write('adresse_dsa2\tadresse_dsa1\tname_dsa1\tstimmen\tstimmen_gesamt\tfunktionen\n')
    for r in res: f.write(f'{r[0]:#06x}\t{r[1]:#06x}\t{r[2]}\t{r[3]}\t{r[4]}\t{r[5]}\n')
good=[r for r in res if r[3]==r[4] and r[2]]
print('Paare',pairs,'DSA2-Adressen mit Stimmen',len(res),'eindeutig+benannt',len(good),'davon >=2 Stimmen',sum(1 for r in good if r[3]>=2))
for r in good[::max(1,len(good)//40)]: print(hex(r[0]),hex(r[1]),r[2],r[3],r[5])
