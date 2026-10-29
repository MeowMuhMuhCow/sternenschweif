# sdf.py NNN STARTS k [obj] - Seite-an-Seite fuer Funktion k (Ref-Starts STARTS, eigene Starts aus PUBDEF)
import sys,subprocess,itertools,tempfile
sys.path.insert(0,'/root/sternenschweif/tools'); import objbytes, objfix, omfinfo
W={0:1,1:2,2:2,3:4,4:1,5:2,9:2,11:4,13:4}
n=sys.argv[1]; st=[int(x,16) for x in sys.argv[2].split(',')]; k=int(sys.argv[3])
o=sys.argv[4] if len(sys.argv)>4 else __import__('os').environ.get('WOV','.')+f'/OVL{n}.OBJ'
r=f'/root/sternenschweif/ref_obj/OVL{n}.OBJ'
def get(p):
    sb=objbytes.seg_bytes(p); seg=[x for x in sb if x.endswith('_TEXT')][0]; b=bytearray(sb[seg])
    for f in objfix.parse(p)['fix']:
        if f['seg']==seg:
            for i in range(f['loc'],f['loc']+W.get(f['typ'],2)):
                if i<len(b): b[i]=0
    return b
def dis(b,a0,a1):
    f=tempfile.NamedTemporaryFile(delete=False); f.write(bytes(b[a0:a1])); f.close()
    out=subprocess.run(['ndisasm','-b16','-o',str(a0),f.name],capture_output=True,text=True).stdout.splitlines()
    return [(l[:8],l[28:].strip()) for l in out if l[:8].strip()]
A=get(r); B=get(o)
s,p,e=omfinfo.info(o); cs=sorted(set(x for _,x,_ in p))
a0=st[k]; a1=st[k+1] if k+1<len(st) else len(A)
c0=cs[k]; c1=cs[k+1] if k+1<len(cs) else len(B)
X=dis(A,a0,a1); Y=dis(B,c0,c1)
for a,b in itertools.zip_longest(X,Y,fillvalue=('','')):
    print(('  ' if a[1]==b[1] else '* ')+f'{a[0][4:]} {a[1]:34s}| {b[0][4:]} {b[1]}')
