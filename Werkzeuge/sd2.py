# sd2.py NNN start end [obj] - difflib-Vergleich, jede OBJ mit eigener Fixup-Maske, Sprungziele normalisiert
import sys,subprocess,itertools,tempfile,difflib,re
sys.path.insert(0,'/root/sternenschweif/tools'); import objbytes, objfix
W={0:1,1:2,2:2,3:4,4:1,5:2,9:2,11:4,13:4}
n=sys.argv[1]; s0=int(sys.argv[2],16); s1=int(sys.argv[3],16)
o=sys.argv[4] if len(sys.argv)>4 else __import__('os').environ.get('WOV','.')+f'/OVL{n}.OBJ'
def dis(p,a,b):
    sb=objbytes.seg_bytes(p); seg=[k for k in sb if k.endswith('_TEXT')][0]; by=bytearray(sb[seg])
    for f in objfix.parse(p)['fix']:
        if f['seg']==seg:
            for i in range(f['loc'],f['loc']+W.get(f['typ'],2)): by[i]=0
    f=tempfile.NamedTemporaryFile(delete=False); f.write(bytes(by)); f.close()
    r=subprocess.run(['ndisasm','-b16',f.name],capture_output=True,text=True).stdout.splitlines()
    return [(l[:8],l[28:].strip()) for l in r if l[:8].strip() and a<=int(l[:8],16)<b]
import omfinfo
def starts(p):
    s,pp,e=omfinfo.info(p); return sorted(set(x for _,x,_ in pp))
ra=starts(f'/root/sternenschweif/ref_obj/OVL{n}.OBJ'); rb=starts(o)
if len(ra)==len(rb) and s0 in ra:
    k=ra.index(s0); b0=rb[k]; b1=rb[k+1] if k+1<len(rb) else 1<<30
    if s1 in ra: b1=rb[ra.index(s1)]
else: b0,b1=s0,s1
A=dis(f'/root/sternenschweif/ref_obj/OVL{n}.OBJ',s0,s1); B=dis(o,b0,b1)
norm=lambda x: re.sub(r'push word 0x[0-9a-f]+','push word N',re.sub(r'\[0x[0-9a-f]+\]','[N]',re.sub(r'\b(j\w+|call|loop\w*)( short| near)? 0x[0-9a-f]+',r'\1 J',x)))
sm=difflib.SequenceMatcher(None,[norm(x[1]) for x in A],[norm(x[1]) for x in B],autojunk=False)
for op,i1,i2,j1,j2 in sm.get_opcodes():
    if op=='equal': continue
    for a,b in itertools.zip_longest(A[i1:i2],B[j1:j2],fillvalue=('','')):
        print(f'* {a[0][4:]} {a[1]:34s}| {b[0][4:]} {b[1]}')
    print('-')
