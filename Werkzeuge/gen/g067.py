# erzeugt den Rumpf von f_06ad aus OVL067.ASM
import re,sys
L=[l.rstrip('\n') for l in open('/root/sternenschweif/o067/OVL067.ASM')]
def off(l):
    m=re.search(r';@([0-9a-f]{4})$',l); return int(m.group(1),16) if m else None
ins=[(off(l),l.split(';@')[0].strip()) for l in L if off(l) is not None]
a0,a1=int(sys.argv[1],16),int(sys.argv[2],16)
ins=[(o,t) for o,t in ins if a0<=o<a1]
def num(s):
    s=s.strip()
    return int(s[:-1],16) if s.endswith('h') else int(s)
out=[];i=0
while i<len(ins):
    o,t=ins[i]
    m=re.match(r'mov\s+si,(\w+)$',t)
    if m:
        st=num(m.group(1))
        # Schleife: bis cmp si,M / jng
        j=i
        while not ins[j][1].startswith('cmp\tsi,') and not re.match(r'cmp\s+si,',ins[j][1]) or 'jng' not in ins[j+1][1]: j+=1
        en=num(ins[j][1].split(',')[1])
        out.append(f'\tfor (i = 0x{st:02x}; i <= 0x{en:02x}; i++) g_c7d0[i].f9 = 1;')
        i=j+2; continue
    m=re.match(r'les\s+bx,ds:\[0c7d0h\]',t)
    if m and 'byte ptr es:[bx+' in ins[i+1][1]:
        x=num(re.search(r'\[bx\+(\w+)\]',ins[i+1][1]).group(1)); assert x%10==9 and ins[i+1][1].endswith(',1'),ins[i+1]
        out.append(f'\tg_c7d0[0x{x//10:02x}].f9 = 1;'); i+=2; continue
    m=re.match(r'push\s+(\w+)$',t)
    if m and i+3<len(ins) and ins[i+1][1].replace(' ','')=='push\tds'.replace(' ','') and 'F_O223_141A' in ins[i+3][1]:
        out.append(f'\tO223_141a(g_c0fb, 0x{num(m.group(1)):02x});'); i+=5; continue
    if m and i+4<len(ins) and 'push' in ins[i+1][1] and '[bp+8]' in ins[i+2][1] and '[bp+6]' in ins[i+3][1]:
        k=num(m.group(1)); n=num(ins[i+1][1].split()[1])
        out.append(f'\tf_13c2(p, 0x{n:02x}, {k});'); i+=7; continue
    out.append('\t/* ?? %04x %s */'%(o,t)); i+=1
print('\n'.join(out))
