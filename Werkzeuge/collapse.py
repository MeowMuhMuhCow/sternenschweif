import re,sys
L=[l.split(';@')[0].rstrip()+(('  ;'+l.split(';@')[1]) if ';@' in l else '') for l in open(sys.argv[1]).read().splitlines()]
a=int(sys.argv[2],16); b=int(sys.argv[3],16)
def off(l):
    m=re.search(r';([0-9a-f]{4})$',l); return int(m.group(1),16) if m else None
i=0; out=[]
sel=[l for l in L if (off(l) is None and l.startswith('L_')) or (off(l) is not None and a<=off(l)<b)]
i=0
while i<len(sel):
    l=sel[i]
    if 'mov\tax,[bp-014h]' in l and i+4<len(sel) and 'add\tword ptr [bp-016h],2' in sel[i+4]:
        # find end: 'add\tdx,ax' after 'pop\tdx'
        j=i
        while j<len(sel) and not ('pop\tdx' in sel[j] and 'add\tdx,ax' in sel[j+1]): j+=1
        slots=sorted(set(re.findall(r'\[bp-(0?[0-9a-f]+h?)\]',' '.join(sel[i:j+2]))))
        out.append(f'   READW {off(sel[i]):04x}-{off(sel[j+1]):04x} slots {slots}')
        i=j+2; continue
    out.append(l); i+=1
print('\n'.join(out))
