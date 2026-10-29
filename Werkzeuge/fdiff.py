import sys, struct, difflib, re
sys.path.insert(0,'/root/sternenschweif/q004/zerl'); sys.path.insert(0,'/root/sternenschweif/tools')
import schweif, omfcmp
exec(open('/root/sternenschweif/tools/funcmatch.py').read().split('\nexe = open')[0].split('import omfcmp\n',1)[1])
obj, sid, fname, ostart, olen = sys.argv[1], sys.argv[2], sys.argv[3], int(sys.argv[4],16), int(sys.argv[5],16)
R=[s for s in schweif.load('/root/sternenschweif/orig/SCHWEIF.EXE') if s['id']==sid][0]['code']
_,_,data,mask=[c for c in omfcmp.parse_obj(obj) if c[1]=='CODE'][0]
P=publics(obj); i=[k for k,(o,n) in enumerate(P) if n==fname][0]; a=P[i][0]; b=P[i+1][0] if i+1<len(P) else len(data)
def ins(code):
    return [re.sub(r'0x[0-9a-f]+','N',t) for _,_,t in schweif.disasm(bytes(code))]
A=ins(data[a:b]); B=ins(R[ostart:ostart+olen])
for l in difflib.unified_diff(B,A,'original','neu',lineterm='',n=2): print(l)
