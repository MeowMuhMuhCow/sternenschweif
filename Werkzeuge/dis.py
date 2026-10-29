#!/usr/bin/env python3
"""dis.py MOD VON BIS [EXE ORIG segmente.txt] - Disassembly eines Modulbereichs, Nachbau vs. Original."""
import sys,struct,re,subprocess,tempfile
sys.path.insert(0,__file__.rsplit('/',1)[0]); import omfcmp
mod=sys.argv[1]; v=int(sys.argv[2],16); bi=int(sys.argv[3],16)
exe=sys.argv[4] if len(sys.argv)>4 else 'BLADEM_FIX.EXE'
orig=sys.argv[5] if len(sys.argv)>5 else '../g1/SCHICKM.EXE'
segf=sys.argv[6] if len(sys.argv)>6 else '../dsa1/segmente.txt'
a=open(exe,'rb').read(); b=open(orig,'rb').read()
po=[int(m[1],16) for m in [re.search(mod+r'\.OBJ.*@0x([0-9a-f]+)',l) for l in open(segf)] if m][0]
code=[c for c in omfcmp.parse_obj(mod+'.OBJ') if c[1]=='CODE'][0]
pn,_=omfcmp.find(a,code[2],code[3])
outs=[]
for buf,base in [(a,pn),(b,po)]:
    with tempfile.NamedTemporaryFile(delete=False) as f: f.write(buf[base+v:base+bi]); fn=f.name
    outs.append(subprocess.run(['ndisasm','-b16','-o',hex(v),fn],capture_output=True,text=True).stdout.splitlines())
w=max(len(x) for x in outs[0]) if outs[0] else 40
from itertools import zip_longest
print(f'{"Bright Eyes":{w}} | Original')
for x,y in zip_longest(*outs,fillvalue=''):
    mark=' ' if x.split()[1:]==y.split()[1:] else '*'
    print(f'{x:{w}} {mark} {y}')
