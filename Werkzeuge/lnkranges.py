#!/usr/bin/env python3
"""lnkranges.py - exakte Byte-Bereiche der residenten Module (rahmenrelativ) fuer den Komplett-Link"""
import csv,json
S={r['segment']:r for r in csv.DictReader(open('q004/zerl/segmente.tsv'),delimiter='\t')}
F={}
for r in csv.DictReader(open('q004/zerl/funktionen.tsv'),delimiter='\t'):
    F.setdefault(r['segment'],[]).append(r)
R=[f'R{i:02d}' for i in range(48)]
def funcs(s): return [f for f in F[s] if f['belege']!='A']
P={s:int(S[s]['para'])*16 for s in R}
lastend={s:max(int(f['start'],16)+int(f['groesse']) for f in funcs(s))+P[s] for s in R}
first={s:min(int(f['start'],16) for f in funcs(s))+P[s] for s in R}
start={'R00':0}
for i,s in enumerate(R[1:],1):
    st=min(max(P[s],lastend[R[i-1]]),first[s])
    if not (P[s]<=st<=P[s]+15 and st<=first[s]): print('PROBLEM',s,P[s],st,first[s],lastend[R[i-1]])
    start[s]=st+(11 if s=="R04" else 0)  # snd_pmode gehoert zu seg003 (Aufruf ueber R03-Rahmen)
mods={s:(start[s]-P[s], start[R[i+1]]-P[s]) for i,s in enumerate(R[1:47],1)}
json.dump({'mods':mods,'start':start},open('lnk_ranges.json','w'))
for s in ['R01','R03','R04','R08','R09','R21','R22','R24','R25','R46']: print(s,mods.get(s))
print('R47 start',start['R47'])
