#!/usr/bin/env python3
"""librename.py ASM... - ersetzt Kunstnamen fuer Bibliotheksaufrufe (F_R00_xxxx, MODUL_xxxx) durch die echten
oeffentlichen Namen aus lib/lib_syms.tsv. Nur Namen, keine Bytes."""
import sys, re
ROOT='/root/sternenschweif'
L={}
for l in open(ROOT+'/lib/lib_syms.tsv'):
    a,b,c=l.rstrip('\n').split('\t'); L.setdefault((a,int(b,16)),c)
tot=0; unk=set()
for f in sys.argv[1:]:
    s=open(f,newline='').read()
    names=set(re.findall(r'\textrn\t([^:\s]+):far',s)); rep={}
    for n in names:
        m=re.match(r'F_(R00|R47)_([0-9A-F]{4})$',n)
        if m:
            k=(m.group(1),int(m.group(2),16))
            if k in L: rep[n]=L[k]
            else: unk.add(n)
            continue
        m=re.match(r'([A-Z0-9_]+)_([0-9A-F]{4})$',n)
        if m and not n.startswith('F_'):
            off=int(m.group(2),16)
            k=('R00',off) if ('R00',off) in L else ('R47',off)
            if k in L: rep[n]=L[k]
            else: unk.add(n)
    if rep:
        # Doppelte Zielnamen bei extrn vermeiden
        for a,b in rep.items(): s=re.sub(r'(?<![\w@$?])'+re.escape(a)+r'(?![\w@$?])',b,s)
        lines=s.split('\r\n'); seen=set(); out=[]
        for ln in lines:
            m=re.match(r'\textrn\t([^:\s]+):far$',ln)
            if m:
                if m.group(1) in seen: continue
                seen.add(m.group(1))
            out.append(ln)
        open(f,'w',newline='').write('\r\n'.join(out)); tot+=len(rep)
print('umbenannt',tot,'; unbekannt',sorted(unk)[:20],len(unk))
