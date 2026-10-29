# erzeugt den Rumpf von O207 f_0000 (ab 0x003a .. 0x104c) aus OVL207.ASM
import re,sys
L=[l.rstrip('\n') for l in open('/root/sternenschweif/o207/OVL207.ASM')]
def off(l):
    m=re.search(r';@([0-9a-f]{4})$',l); return int(m.group(1),16) if m else None
ins=[(off(l),re.sub(r'\s+',' ',l.split(';@')[0].strip())) for l in L if off(l) is not None]
a0,a1=int(sys.argv[1],16),int(sys.argv[2],16)
ins=[(o,t) for o,t in ins if a0<=o<a1]
def num(s):
    s=s.strip(); return int(s[:-1],16) if s.endswith('h') else int(s)
out=[];i=0;fields=set()
T=lambda k: ins[k][1] if k<len(ins) else ''
while i<len(ins):
    o,t=ins[i]
    if t=='push ds' and T(i+2)=='push word ptr [bp+8]' and 'F_O206_1580' in T(i+4):
        out.append(f'\tO206_1580(h, g_{num(T(i+1).split()[1]):04x});'); i+=6; continue
    if t=='les bx,[bp+6]' and T(i+1)=='les bx,es:[bx+05ebh]' and T(i+4)=='push word ptr [bp+8]' and 'F_O206_1580' in T(i+6):
        x=num(re.search(r'\[bx\+(\w+)\]',T(i+3)).group(1))
        out.append(f'\tO206_1580(h, h->txt[{x//4}]);'); i+=8; continue
    if t=='push word ptr ds:[0b676h]' and T(i+2)=='push word ptr [bp+8]' and 'F_O206_1580' in T(i+4):
        out.append('\tO206_1580(h, g_b674);'); i+=6; continue
    # sprintf-Block
    args=[];j=i
    while T(j)=='les bx,[bp+6]' and T(j+1).startswith('mov al,es:[bx+') and T(j+2)=='cbw' and T(j+3)=='push ax':
        x=num(re.search(r'\[bx\+(\w+)\]',T(j+1)).group(1)); fields.add(x); args.append(f'h->w[0x{x-0x150:02x}]'); j+=4
    if args and T(j)=='les bx,[bp+6]' and T(j+1)=='les bx,es:[bx+05ebh]' and 'F__sprintf' in T(j+6) or (args and '_sprintf' in T(j+6)):
        x=num(re.search(r'\[bx\+(\w+)\]',T(j+3)).group(1))
        out.append(f'\tsprintf(g_b674, h->txt[{x//4}], {", ".join(reversed(args))});'); i=j+8; continue
    out.append('\t/* ?? %04x %s */'%(o,t)); i+=1
print('\n'.join(out))
print('FIELDS',' '.join('%x'%f for f in sorted(fields)),file=sys.stderr)
