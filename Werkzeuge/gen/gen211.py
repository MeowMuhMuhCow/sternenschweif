import re,sys,collections
sys.path.insert(0,'/root/sternenschweif/tools'); import objbytes
R='/root/sternenschweif/'
L=open(R+'o211/OVL211.ASM').read().splitlines()
i=[k for k,l in enumerate(L) if 'jmp	cs:[bx+0eb4h]' in l][0]+1
end=[k for k,l in enumerate(L) if l.startswith('L_0de7:')][0]
bodies=[];cur=None
for l in L[i:end]:
    if cur is None: cur=int(l.split(';@')[1],16)
    m=re.search(r'call\tfar ptr F_(\w+)',l)
    if m: fn=m.group(1)
    if 'L_0de7' in l: bodies.append((cur,fn)); cur=None
sb=objbytes.seg_bytes(R+'ref_obj/OVL211.OBJ'); b=sb[[k for k in sb if k.endswith('_TEXT')][0]]
tab=[b[0xeb4+2*k]|b[0xeb4+2*k+1]<<8 for k in range(0x104)]
by=collections.defaultdict(list)
for v,t in enumerate(tab): by[t].append(v)
out=[]
fns=[]
for off,fn in bodies:
    vals=by[off]
    for v in vals: out.append('\tcase 0x%02x:'%v)
    f=fn[:5]+fn[5:].lower()
    fns.append(f)
    out.append('\t\t%s(p);\n\t\tbreak;'%f)
# order check
seq=[v for off,fn in bodies for v in by[off]]
print('aufsteigend' if seq==sorted(seq) else 'NICHT aufsteigend', file=sys.stderr)
print('mehrfach:',[ (hex(o),by[o]) for o,f in bodies if len(by[o])!=1], file=sys.stderr)
print('default values', [hex(v) for v in by[0xde7]], file=sys.stderr)
open('o211_sw.txt','w').write('\n'.join(out)+'\n')
seen=[];
for f in fns:
    if f not in seen: seen.append(f)
open('o211_decl.txt','w').write(''.join('void %s(struct obj far *o);\n'%f for f in sorted(seen)))
