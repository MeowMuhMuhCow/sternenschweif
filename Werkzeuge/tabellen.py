"""tabellen.py - Schrittweiten (Satzgroessen) der indizierten DGROUP-Zugriffe bestimmen und Tabellen ableiten."""
import sys, re, csv, collections
sys.path.insert(0,'/root/sternenschweif/q004/zerl'); import schweif
segs={s['id']:s for s in schweif.load('/root/sternenschweif/orig/SCHWEIF.EXE')}
F=collections.defaultdict(list)
for r in csv.DictReader(open('/root/sternenschweif/q004/zerl/funktionen.tsv'),delimiter='\t'):
    F[r['segment']].append((int(r['start'],16),int(r['groesse'])))
memI=re.compile(r'\[(bx|si|di)\+0x([0-9a-f]+)\]')
def stride(ins,k,reg):
    """Rueckwaerts: wie wurde reg berechnet?  Liefert Faktor oder None."""
    f=1; r=reg; j=k-1; steps=0
    while j>=0 and steps<12:
        t=ins[j][2]; steps+=1
        m=re.match(rf'shl {r},(?:byte )?(1|cl|0x[0-9a-f]+)$',t)
        if m:
            if m.group(1)=='1': f*=2
            elif m.group(1).startswith('0x'): f*=1<<int(m.group(1),16)
            else:
                mm=re.match(r'mov cl,0x([0-9a-f]+)$',ins[j-1][2]) if j else None
                if not mm: return None
                f*=1<<int(mm.group(1),16); j-=1
            j-=1; continue
        if t==f'add {r},{r}': f*=2; j-=1; continue
        m=re.match(rf'imul {r},{r},(?:byte |word )?\+?0x([0-9a-f]+)$',t)
        if m: return f*int(m.group(1),16)
        m=re.match(rf'mov {r},(ax|dx|cx|si|di|bx)$',t)
        if m:
            src=m.group(1)
            if src=='ax':
                # ax = ? * K
                for jj in range(j-1,max(-1,j-8),-1):
                    u=ins[jj][2]
                    mm=re.match(r'imul ax,ax,(?:byte |word )?\+?0x([0-9a-f]+)$',u)
                    if mm: return f*int(mm.group(1),16)
                    mm=re.match(r'(?:i?mul) (dx|cx|bx|word \[.*\])$',u)
                    if mm:
                        reg2=mm.group(1)
                        for j3 in range(jj-1,max(-1,jj-5),-1):
                            m3=re.match(rf'mov {reg2},0x([0-9a-f]+)$',ins[j3][2])
                            if m3: return f*int(m3.group(1),16)
                        return None
                    mm=re.match(r'shl ax,(?:byte )?(1|cl|0x[0-9a-f]+)$',u)
                    if mm:
                        if mm.group(1)=='1': f*=2; continue
                        if mm.group(1).startswith('0x'): f*=1<<int(mm.group(1),16); continue
                        m3=re.match(r'mov cl,0x([0-9a-f]+)$',ins[jj-1][2])
                        if m3: f*=1<<int(m3.group(1),16); continue
                        return None
                    if u=='add ax,ax': f*=2; continue
                    if re.match(r'(mov|cbw|xor ah|mov ah)',u) and ('ax' in u or u=='cbw'): return f
                return f
            r=src; j-=1; continue
        if re.match(rf'(add|sub|inc|dec) {r}\b',t): j-=1; continue
        if re.match(rf'mov {r},(?:word )?\[',t) or re.match(rf'pop {r}$',t): return None if f==1 else f
        if re.match(rf'(mov|les|lds|lea|xor|and|or) {r}\b',t):
            return f
        j-=1
    return f
rows=[]
for sid,L in F.items():
    if sid in ('R00','R47'): continue
    code=segs[sid]['code']
    for st,n in L:
        ins=schweif.disasm(code[st:st+n])
        fremd=False
        for k,(o,ln,t) in enumerate(ins):
            if t.startswith('mov ds,'):
                fremd=not (k and re.match(r'mov (\w+),0x24eb$',ins[k-1][2])); continue
            if t=='pop ds': fremd=False; continue
            if fremd: continue
            if re.search(r'\b(es|cs|ss):',t) or 'bp' in t: continue
            m=memI.search(t)
            if not m: continue
            base=int(m.group(2),16)
            if base<0x94 or base>=0xcc0e: continue
            w='byte' if re.search(r'\b(al|ah|bl|bh|cl|ch|dl|dh)\b|byte',t) else 'dword' if t.startswith(('les','lds')) or 'dword' in t else 'word'
            s=stride(ins,k,m.group(1))
            rows.append((base,s,w,sid,st+o,t))
with open('/root/sternenschweif/ds/indiziert.tsv','w') as f:
    for r in rows: f.write(f'{r[0]:#06x}\t{r[1]}\t{r[2]}\t{r[3]}\t{r[4]:#06x}\t{r[5]}\n')
c=collections.Counter(r[1] for r in rows)
print('indizierte Zugriffe',len(rows),'Schrittweiten',c.most_common(20))
