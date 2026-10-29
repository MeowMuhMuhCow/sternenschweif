#!/usr/bin/env python3
"""fcmp.py OBJ SEGID START1,START2,... - vergleicht die Funktionen der OBJ (in Reihenfolge)
mit den Originalfunktionen an den angegebenen Startadressen (volle Originallaenge)."""
import sys, struct, csv, collections
sys.path.insert(0,'/root/sternenschweif/q004/zerl'); sys.path.insert(0,'/root/sternenschweif/tools')
import schweif, omfcmp
exec(open('/root/sternenschweif/tools/funcmatch.py').read().split('\nexe = open')[0].split('import omfcmp\n',1)[1])
obj, sid, starts = sys.argv[1], sys.argv[2], [int(x,16) for x in sys.argv[3].split(',')]
R=[s for s in schweif.load('/root/sternenschweif/orig/SCHWEIF.EXE') if s['id']==sid][0]['code']
L={int(r['start'],16):int(r['groesse']) for r in csv.DictReader(open('/root/sternenschweif/q004/zerl/funktionen.tsv'),delimiter='\t') if r['segment']==sid}
_,_,data,mask=[c for c in omfcmp.parse_obj(obj) if c[1]=='CODE'][0]
P=publics(obj); ok=0
# NEARSKIP: Sprungweite von push cs/call near ausblenden (Segment noch nicht vollstaendig)
import os
mask=bytearray(mask)
if os.environ.get('NEARSKIP'):
    for i in range(len(data)-3):
        if data[i]==0x0E and data[i+1]==0xE8: mask[i+2]=0; mask[i+3]=0
for i,(off,name) in enumerate(P):
    end=P[i+1][0] if i+1<len(P) else len(data); st=starts[i]; n=L.get(st,end-off)
    eq=sum(1 for t in range(end-off) if mask[off+t] and t<n and (R[st+t]==data[off+t] or (data[off+t]==0x9A and R[st+t]==0x90))); fx=sum(mask[off:end])
    same = eq==fx and end-off==n; ok+=same
    print(f'{name:28} +{st:#06x} neu {end-off:4} / orig {n:4} Bytes  feste gleich {eq}/{fx}' + ('  GLEICH' if same else ''))
print(f'{ok} von {len(P)} Funktionen gleich')
