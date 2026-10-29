import struct
import sys; sys.path.insert(0,'/root/sternenschweif/q004/zerl'); sys.path.insert(0,'/root/sternenschweif/tools')
import schweif, omfcmp
exec(open('/root/sternenschweif/tools/funcmatch.py').read().split('\nexe = open')[0].split('import omfcmp\n',1)[1])
R=[s for s in schweif.load('/root/sternenschweif/orig/SCHWEIF.EXE') if s['id']==sys.argv[2]][0]['code']
base=int(sys.argv[3],16) if len(sys.argv)>3 else 5
code=[c for c in omfcmp.parse_obj(sys.argv[1]) if c[1]=='CODE'][0]; _,_,data,mask=code
P=publics(sys.argv[1]); tot=eqt=0
for i,(off,name) in enumerate(P):
    end=P[i+1][0] if i+1<len(P) else len(data)
    fx=sum(mask[off:end]); eq=sum(1 for t in range(off,end) if mask[t] and base+t<len(R) and R[base+t]==data[t])
    tot+=fx; eqt+=eq
    print(f'{name:28} +{base+off:#06x} {end-off:4} Bytes  feste gleich {eq}/{fx}' + ('  GLEICH' if eq==fx else ''))
print(f'Segment: OBJ {len(data)} Bytes, Original {len(R)-base} Bytes, feste gleich {eqt}/{tot}')
