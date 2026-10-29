# sd.py NNN [obj] [start] [end] - Seite-an-Seite: ref_obj/OVLNNN.OBJ vs $WOV/OVLNNN.OBJ (Standard: aktueller Ordner) (ndisasm), Fixup-Bytes genullt
import sys,subprocess,itertools,tempfile
sys.path.insert(0,'/root/sternenschweif/tools'); import objbytes, objfix
W={0:1,1:2,2:2,3:4,4:1,5:2,9:2,11:4,13:4}
n=sys.argv[1]; o=sys.argv[2] if len(sys.argv)>2 and sys.argv[2]!='-' else __import__('os').environ.get('WOV','.')+f'/OVL{n}.OBJ'
s0=int(sys.argv[3],16) if len(sys.argv)>3 else 0; s1=int(sys.argv[4],16) if len(sys.argv)>4 else 1<<30
M=set()
def fx(p):
    sb=objbytes.seg_bytes(p); seg=[k for k in sb if k.endswith('_TEXT')][0]
    for f in objfix.parse(p)['fix']:
        if f['seg']==seg: M.update(range(f['loc'],f['loc']+W.get(f['typ'],2)))
def dis(p):
    sb=objbytes.seg_bytes(p); seg=[k for k in sb if k.endswith('_TEXT')][0]; b=bytearray(sb[seg])
    for i in M:
        if i<len(b): b[i]=0
    f=tempfile.NamedTemporaryFile(delete=False); f.write(bytes(b)); f.close()
    r=subprocess.run(['ndisasm','-b16',f.name],capture_output=True,text=True).stdout.splitlines()
    return [(l[:8],l[28:].strip()) for l in r if l[:8].strip() and s0<=int(l[:8],16)<s1]
fx(o); fx(f'/root/sternenschweif/ref_obj/OVL{n}.OBJ')
A=dis(f'/root/sternenschweif/ref_obj/OVL{n}.OBJ'); B=dis(o)
try:
  for a,b in itertools.zip_longest(A,B,fillvalue=('','')):
    print(('  ' if a[1]==b[1] else '* ')+f'{a[0][4:]} {a[1]:34s}| {b[0][4:]} {b[1]}')
except BrokenPipeError: pass
