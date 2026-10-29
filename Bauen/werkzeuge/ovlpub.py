#!/usr/bin/env python3
import os as _swos; _SW = _swos.environ.get('SW_ROOT', '/root/sternenschweif')  # Bau-Paket
"""ovlpub.py OVL-ASM... - exportiert in Overlay-Modulen nur die Einsprünge, in der Reihenfolge der Original-Stubs"""
import sys, re
sys.path.insert(0,_SW + '/q004/zerl'); import schweif
S={s['id']:s for s in schweif.load(_SW + '/orig/SCHWEIF.EXE')}
for f in sys.argv[1:]:
    num=int(re.search(r'OVL(\d+)',f).group(1)); sid=f'O{num:03d}'
    s=open(f,newline='').read(); L=s.split('\r\n')
    have=set(re.findall(r'^(F_'+sid+r'_[0-9A-F]{4})\tlabel\tfar',s,re.M))
    ALIAS={'F_O032_0000':'_main'}
    for a_,b_ in ALIAS.items():
        if a_.startswith(f'F_{sid}_'): s=re.sub(r'(?<![\w@$?])'+a_+r'(?![\w@$?])',b_,s); L=s.split('\r\n'); have=set(re.findall(r'^(F_'+sid+r'_[0-9A-F]{4}|_main)\tlabel\tfar',s,re.M))
    want=[ALIAS.get(f'F_{sid}_{e:04X}',f'F_{sid}_{e:04X}') for e in S[sid]['entries']]
    miss=[w for w in want if w not in have]
    out=[]; ins=None
    for ln in L:
        if re.match(r'\tpublic\t',ln):
            if ins is None: ins=len(out)
            continue
        out.append(ln)
    if ins is None: ins=[i for i,l in enumerate(out) if '_TEXT\tsegment' in l][0]+2
    out[ins:ins]=[f'\tpublic\t{w}' for w in want if w in have]
    open(f,'w',newline='').write('\r\n'.join(out))
    if miss: print(sid,'fehlende Einsprungmarken:',miss)
print('fertig')
