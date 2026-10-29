# fsd.py NNN STARTS k [obj] - Seite-an-Seite einer Funktion k (Ref-Starts), C-Start aus Publics
import sys,subprocess,itertools,tempfile,difflib
sys.path.insert(0,'/root/sternenschweif/tools'); import objbytes,omfinfo
n=sys.argv[1]; st=[int(x,16) for x in sys.argv[2].split(',')]; k=int(sys.argv[3])
o=sys.argv[4] if len(sys.argv)>4 else __import__('os').environ.get('WOV','.')+f'/OVL{n}.OBJ'
def seg(p):
    sb=objbytes.seg_bytes(p); return bytes([s for kk,s in sb.items() if kk.endswith('_TEXT')][0])
def dis(b,base):
    f=tempfile.NamedTemporaryFile(delete=False); f.write(b); f.close()
    r=subprocess.run(['ndisasm','-b16','-o',str(base),f.name],capture_output=True,text=True).stdout.splitlines()
    return [(l[4:8],l[28:].strip()) for l in r]
A=seg(f'/root/sternenschweif/ref_obj/OVL{n}.OBJ'); B=seg(o)
s,p,e=omfinfo.info(o); cs=sorted(set(x for _,x,_ in p))
a0=st[k]; a1=st[k+1] if k+1<len(st) else len(A)
if len(cs)==len(st): c0=cs[k]; c1=cs[k+1] if k+1<len(cs) else len(B)
else: c0,c1=a0,a1
DA=dis(A[a0:a1],a0); DB=dis(B[c0:c1],c0)
sm=difflib.SequenceMatcher(None,[x[1] for x in DA],[x[1] for x in DB],autojunk=False)
for tag,i1,i2,j1,j2 in sm.get_opcodes():
    if tag=='equal':
        if i2-i1>4:
            print('  ',DA[i1][0],DA[i1][1]); print('   ...'); print('  ',DA[i2-1][0],DA[i2-1][1])
        else:
            for i in range(i1,i2): print('  ',DA[i][0],DA[i][1])
    else:
        for a,b in itertools.zip_longest(DA[i1:i2],DB[j1:j2],fillvalue=('','')):
            print('* ',f'{a[0]} {a[1]:36s}| {b[0]} {b[1]}')
