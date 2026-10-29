#!/usr/bin/env python3
"""dgscan.py - sammelt alle Zugriffe auf DGROUP in allen Funktionen von SCHWEIF.EXE.
Arten: D = direkter Zugriff [addr] (Breite b/w/d), I = indiziert [reg+addr], A = Adressuebergabe (push ds; push addr)
bzw. mov reg,addr direkt nach push ds. Nur Zugriffe ohne Segmentpraefix oder mit ds:."""
import sys, re, csv, collections
sys.path.insert(0,'/root/sternenschweif/q004/zerl'); import schweif
segs={s['id']:s for s in schweif.load('/root/sternenschweif/orig/SCHWEIF.EXE')}
F=collections.defaultdict(list)
for r in csv.DictReader(open('/root/sternenschweif/q004/zerl/funktionen.tsv'),delimiter='\t'):
    F[r['segment']].append((int(r['start'],16),int(r['groesse'])))
out=[]
mem=re.compile(r'(?:(byte|word|dword)\s+)?(?:(\w\w):)?\[([^\]]+)\]')
for sid,L in F.items():
    if sid in ('R00','R47'): continue   # Bibliothek / Assembler
    code=segs[sid]['code']
    for st,n in L:
        ins=schweif.disasm(code[st:st+n]); prev=''; fremd=False
        for o,ln,t in ins:
            # DS-Zustand verfolgen: nach 'mov ds,X' (X nicht DGROUP) bis 'pop ds' kein DGROUP-Zugriff
            if t.startswith('mov ds,'):
                fremd=not re.match(r'mov (\w+),0x24eb$',prev); prev=t; continue
            if t=='pop ds': fremd=False; prev=t; continue
            if fremd: prev=t; continue
            for m in mem.finditer(t):
                width,sego,expr=m.group(1),m.group(2),m.group(3)
                if sego and sego!='ds': continue
                if 'bp' in expr or 'sp' in expr: continue
                if re.match(r'(es|cs|ss|fs|gs):',expr): continue
                if expr.startswith('ds:'): expr=expr[3:]
                hx=re.findall(r'0x([0-9a-f]+)',expr)
                regs=re.findall(r'\b(bx|si|di)\b',expr)
                if not hx: continue
                a=int(hx[-1],16)
                if '-' in expr: continue
                if not width:
                    op=t.split()[0]; w=None
                    mm=re.match(r'\w+\s+(\w+),\[',t) or re.match(r'\w+\s+\[[^\]]+\],(\w+)',t)
                    if mm:
                        r=mm.group(1)
                        w='b' if r in ('al','ah','bl','bh','cl','ch','dl','dh') else 'd' if r.startswith('e') and len(r)==3 else 'w'
                    width={'b':'byte','w':'word','d':'dword'}.get(w,'?') if w else '?'
                kind='I' if regs else 'D'
                if kind=='I' and a<0x100: continue
                out.append((a,kind,width,sid,st+o,t))
            m=re.match(r'push word 0x([0-9a-f]+)$',t) or re.match(r'push 0x([0-9a-f]+)$',t)
            if m and prev=='push ds':
                out.append((int(m.group(1),16),'A','-',sid,st+o,t))
            prev=t
with open('/root/sternenschweif/ds/zugriffe.tsv','w') as f:
    for r in out: f.write(f'{r[0]:#06x}\t{r[1]}\t{r[2]}\t{r[3]}\t{r[4]:#06x}\t{r[5]}\n')
c=collections.Counter(r[1] for r in out)
print('Zugriffe gesamt',len(out),dict(c),' verschiedene Adressen',len(set(r[0] for r in out)))
