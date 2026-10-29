# erzeugt den Rumpf von O206 f_0000 (Bereich a0..a1) aus OVL206.ASM - Stapelmaschine
import re,sys
L=[l.rstrip('\n') for l in open('/root/sternenschweif/o206/OVL206.ASM')]
def off(l):
    m=re.search(r';@([0-9a-f]{4})$',l); return int(m.group(1),16) if m else None
ins=[(off(l),re.sub(r'\s+',' ',l.split(';')[0].strip())) for l in L if off(l) is not None]
a0,a1=int(sys.argv[1],16),int(sys.argv[2],16)
ins=[(o,t) for o,t in ins if a0<=o<a1]
def num(s):
    s=s.strip()
    neg=s.startswith('-'); s=s.lstrip('-')
    v=int(s[:-1],16) if s.endswith('h') else int(s)
    return -v if neg else v
fields={}
def F(x,t): fields.setdefault(x,set()).add(t); return f'h->f{x:x}'
st=[]; out=[]; acc=None; bx=None; i=0
def T(k): return ins[k][1] if k<len(ins) else ''
calls={'F_O006_0000':'O006_0000','F_O077_04AB':'O077_04ab','F_O077_0471':'O077_0471','F_O077_0000':'O077_0000','_sprintf':'sprintf','F_O207_10A9':'O207_10a9'}
while i<len(ins):
    o,t=ins[i]
    try:
        if t=='les bx,[bp+6]': bx='h'
        elif t=='les bx,es:[bx+05ebh]' and bx=='h': bx='txt'
        elif t.startswith('push word ptr es:[bx+') and bx=='txt':
            x=num(re.search(r'\+(\w+)\]',t).group(1)); x2=num(re.search(r'\+(\w+)\]',T(i+1)).group(1)); assert x2==x-2
            st.append((f'h->txt[{x2//4}]',2)); i+=1
        elif t.startswith('push word ptr es:[bx+') and bx=='h':
            x=num(re.search(r'\+(\w+)\]',t).group(1)); st.append((F(x,'w'),1))
        elif t.startswith('push dword ptr es:[bx+') and bx=='h':
            x=num(re.search(r'\+(\w+)\]',t).group(1)); st.append((F(x,'l'),2))
        elif t.startswith('mov al,es:[bx+') and bx=='h':
            x=num(re.search(r'\+(\w+)\]',t).group(1)); acc=('c',F(x,'c'))
        elif t.startswith('add al,es:[bx+') and bx=='h':
            x=num(re.search(r'\+(\w+)\]',t).group(1)); acc=('c',f'(signed char)({acc[1]} + {F(x,"c")})')
        elif t=='cbw': acc=('i',acc[1])
        elif t=='mov ah,0': acc=('u',acc[1].replace('h->f','h->u'))
        elif t=='push ax': st.append((acc[1],1))
        elif t=='push dx' and T(i+1)=='push ax': st.append((acc[1],2)); i+=1
        elif re.match(r'push -?[0-9a-f]+h?$',t): st.append((str(num(t.split()[1])) if num(t.split()[1])<10 else hex(num(t.split()[1])),1))
        elif t=='push ds':
            x=num(T(i+1).split()[1]); st.append((f'g_{x:04x}',2)); i+=1
        elif t=='push word ptr ds:[0b676h]' and T(i+1)=='push word ptr ds:[0b674h]': st.append(('g_b674',2)); i+=1
        elif t=='push word ptr [bp+8]' and T(i+1)=='push word ptr [bp+6]': st.append(('h',2)); i+=1
        elif t.startswith('call far ptr') or t.startswith('db 09ah'):
            if t.startswith('db'):
                nm='O206_1580'; i+=1
            else:
                nm=calls[t.split()[-1]]
            n=num(T(i+1).split(',')[1])//2; i+=1
            args=[];w=0
            while w<n: e,k=st.pop(); args.append(e); w+=k
            assert w==n,(hex(o),nm)
            ex=f'{nm}({", ".join(args)})'
            # Rueckgabe benutzt?
            if T(i+1)=='cbw' and T(i+2)=='push ax': st.append((ex,1)); i+=2
            elif T(i+1)=='push ax': st.append((ex,1)); i+=1
            elif T(i+1)=='push dx' and T(i+2)=='push ax': st.append((ex,2)); i+=2
            else:
                assert not st,(hex(o),st); out.append('\t'+ex+';')
        else: raise ValueError
    except Exception as e:
        out.append(f'\t/* ?? {o:04x} {t} */'); st=[]; acc=None
    i+=1
print('\n'.join(out))
print('FIELDS',' '.join(f'{k:x}:{"".join(sorted(v))}' for k,v in sorted(fields.items())),file=sys.stderr)
