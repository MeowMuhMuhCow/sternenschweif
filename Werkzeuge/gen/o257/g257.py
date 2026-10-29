# Generator fuer den switch in O257 f_05c8 (aus der ASM; Sprungtabelle aus der Referenz-OBJ)
import re,sys,collections
sys.path.insert(0,'/root/sternenschweif/tools'); import objbytes
R='/root/sternenschweif/'
L=[l.split(';')[0].rstrip() for l in open(R+'o257/OVL257.ASM')]
O=[l for l in open(R+'o257/OVL257.ASM')]
i=[k for k,l in enumerate(L) if 'jmp	cs:[bx+0b8dh]' in l][0]+1
end=[k for k,l in enumerate(L) if l.startswith('L_0b6a:')][0]
def off(k): return int(O[k].split(';@')[1].strip(),16) if ';@' in O[k] else None
sb=objbytes.seg_bytes(R+'ref_obj/OVL257.OBJ'); b=sb[[k for k in sb if k.endswith('_TEXT')][0]]
tab=[b[0xb8d+2*k]|b[0xb8d+2*k+1]<<8 for k in range(19)]
by=collections.defaultdict(list)
for v,t in enumerate(tab): by[t].append(v+1)
# Bloecke trennen an 'jmp near ptr L_0b6a' bzw. Marke L_0ab8
blocks=[];cur=[];start=None
for k in range(i,end):
    l=L[k].strip()
    if not l: continue
    if not l.endswith(':') and off(k) in by and cur and start!=off(k):
        blocks.append((start,cur+['/*fall*/']));cur=[];start=None
    if start is None and not l.endswith(':'): start=off(k)
    if l=='L_0ab8:':
        if cur: blocks.append((start,cur))
        cur=[];start=None;continue
    cur.append(l)
    if l in ('jmp	near ptr L_0b6a','jmp	short L_0b6a'):
        blocks.append((start,cur));cur=[];start=None
if cur: blocks.append((start,cur))
def F(x): return 'f370[0x%02x]'%(x-0x370)
def G(x): return 'g_%04x'%x
def conv(ins):
    out=[];k=0
    while k<len(ins):
        l=ins[k]
        if l.startswith('jmp') : k+=1; continue
        if l=='les	bx,[bp+6]': k+=1; continue
        if l=='mov	al,1':
            # Kette
            st=[];k+=1
            while k<len(ins):
                l2=ins[k]
                m=re.match(r'mov\tes:\[bx\+0?([0-9a-f]+)h\],al',l2); m2=re.match(r'mov\tds:\[0?([0-9a-f]+)h\],al',l2)
                if m: st.append(F(int(m.group(1),16))); k+=1
                elif m2: st.append(G(int(m2.group(1),16))); k+=1
                elif l2=='les	bx,[bp+6]': k+=1
                else: break
            out.append('\t\t'+' = '.join(reversed(st))+' = 1;'); continue
        m=re.match(r'mov\tbyte ptr es:\[bx\+0?([0-9a-f]+)h\],1',l)
        if m: out.append('\t\t%s = 1;'%F(int(m.group(1),16))); k+=1; continue
        m=re.match(r'mov\tbyte ptr ds:\[0?([0-9a-f]+)h\],1',l)
        if m: out.append('\t\t%s = 1;'%G(int(m.group(1),16))); k+=1; continue
        m=re.match(r'cmp\tbyte ptr ds:\[0?([0-9a-f]+)h\],0',l)
        if m:
            # Bedingung bis zum Ziel sammeln
            conds=['%s != 0'%G(int(m.group(1),16))]; k+=2
            while ins[k].startswith('mov	al,ds:'):
                g=int(re.search(r'\[0?([0-9a-f]+)h\]',ins[k]).group(1),16); conds.append('!%s'%G(g)); k+=4
            # Rumpf: les + mov byte
            k+=1; m3=re.match(r'mov\tbyte ptr es:\[bx\+0?([0-9a-f]+)h\],1',ins[k]); k+=1
            s='\t\tif (%s)\n\t\t\t%s = 1;'%(' && '.join(conds),F(int(m3.group(1),16)))
            if k<len(ins) and ins[k].startswith('jmp	short'):
                # else if
                k+=1; lab=ins[k]; k+=1
                m4=re.match(r'cmp\tbyte ptr ds:\[0?([0-9a-f]+)h\],0',ins[k]); k+=3
                m5=re.match(r'mov\tbyte ptr es:\[bx\+0?([0-9a-f]+)h\],1',ins[k]); k+=1
                s+='\n\t\telse if (%s != 0)\n\t\t\t%s = 1;'%(G(int(m4.group(1),16)),F(int(m5.group(1),16)))
            out.append(s)
            if k<len(ins) and ins[k].startswith('L_'): k+=1
            continue
        if l.startswith('L_') or l=='/*fall*/': k+=1; continue
        raise SystemExit('?? '+l)
    return out
res=[]
for st,ins in blocks:
    vals=by.get(st,[])
    lab=''.join('\tcase %d:\n'%v for v in vals) if vals else '\tdefault:\n'
    res.append(lab+'\n'.join(conv(ins))+('' if ins[-1]=='/*fall*/' else '\n\t\tbreak;'))
print('\n'.join(res))
print('Tabelle:',[hex(t) for t in tab],file=sys.stderr)
print('Bloecke:',[hex(s) for s,_ in blocks],file=sys.stderr)
