# pseudo.py ASM start end : kompakte Darstellung (Aufrufe mit Argumenten)
import re,sys
L=[]
for l in open(sys.argv[1]).read().splitlines():
    if ';@' in l:
        ins,off=l.split(';@'); L.append((int(off[:4],16),ins.strip().replace('\t',' ')))
    elif re.match(r'^L_[0-9a-f]+:',l.strip()):
        L.append((None,l.strip()))
a=int(sys.argv[2],16); b=int(sys.argv[3],16)
sel=[]
for i,(o,t) in enumerate(L):
    if o is None:
        # label: include if next offset in range
        nx=next((x for x,_ in L[i+1:] if x is not None),None)
        if nx is not None and a<=nx<b: sel.append((None,t))
    elif a<=o<b: sel.append((o,t))
out=[]; stack=[]
def flush():
    global stack
    for s in stack: out.append('   '+s[1])
    stack=[]
i=0
while i<len(sel):
    o,t=sel[i]
    if o is None: flush(); out.append(t); i+=1; continue
    if t.startswith('push '):
        stack.append((o,t[5:].replace('word ptr ','')))
        i+=1; continue
    m=re.match(r'(call far ptr|call near ptr) (\S+)',t)
    if m or t.startswith('db 09ah'):
        name=m.group(2) if m else sel[i+1][1].split()[-1]
        if not m: i+=1
        if stack and stack[-1][1]=='cs': stack.pop()
        # pair args
        args=[x[1] for x in stack]; stack=[]
        # look for add sp
        n=None
        if i+1<len(sel) and sel[i+1][1].startswith('add sp,'):
            n=sel[i+1][1][7:]; i+=1
        args=args[::-1]
        A=[];j=0
        while j<len(args):
            x=args[j]
            if j+1<len(args) and (args[j+1]=='ds' or args[j+1]=='ss' or re.match(r'.*\[.*\+?0?[0-9a-f]*h?\]$',args[j+1])) and (args[j+1] in('ds','ss') ):
                A.append(f'{args[j+1]}:{x}'); j+=2; continue
            A.append(x); j+=1
        out.append(f'{o:04x} CALL {name}({", ".join(A)})' + (f' sp+{n}' if n else ''))
        i+=1; continue
    flush()
    out.append(f'{o:04x} {t}'); i+=1
flush()
print('\n'.join(out))
