#!/usr/bin/env python3
"""zeige.py SEGID [...] - Disassembly eines Segments mit Funktionsgrenzen und Aufrufzielen."""
import sys,csv,collections,re
sys.path.insert(0,'/root/sternenschweif/q004/zerl'); import schweif
segs={s['id']:s for s in schweif.load('/root/sternenschweif/orig/SCHWEIF.EXE')}
F=collections.defaultdict(list)
for r in csv.DictReader(open('/root/sternenschweif/q004/zerl/funktionen.tsv'),delimiter='\t'):
    F[r['segment']].append(int(r['start'],16))
P={int(r['para']):r['segment'] for r in csv.DictReader(open('/root/sternenschweif/q004/zerl/segmente.tsv'),delimiter='\t')}
lib={}
for f in ('schweif_cl.tsv','schweif_overlay.tsv'):
    for l in open('/root/sternenschweif/lib/'+f):
        s,o,n,m,_=l.rstrip('\n').split('\t'); lib[(s,int(o,16))]=m
def ann(t):
    m=re.match(r'call 0x([0-9a-f]+):0x([0-9a-f]+)',t)
    if m:
        sg=P.get(int(m.group(1),16)); o=int(m.group(2),16)
        if sg and sg[0]=='O':
            e=segs[sg]['entries']; i=(o-0x20)//5
            return t+f'   ; {sg} Eintrag {i} -> +{e[i]:#x}' if 0<=i<len(e) else t+f'   ; {sg}'
        return t+f'   ; {sg} {lib.get((sg,o),"")}'
    return t
for sid in sys.argv[1:]:
    c=segs[sid]['code']; fs=set(F[sid]); print('=====',sid,len(c),[hex(x) for x in sorted(fs)])
    for o,ln,t in schweif.disasm(c):
        print(('>' if o in fs else ' ')+f'{o:04x} {c[o:o+ln].hex():16s} {ann(t)}')
