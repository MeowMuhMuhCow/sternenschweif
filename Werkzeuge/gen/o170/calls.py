import re,sys
L=open(sys.argv[1]).read().splitlines()
stack=[];base=None
for l in L:
    m=re.match(r'\s+(\w+)\s+(.*?)\s*;@(\w+)',l)
    if not m:
        continue
    op,arg,off=m.groups()
    if op=='les':
        if 'ds:[0' in arg: base='b67c' if 'b67c' in arg else 'ds'+arg
        elif arg=='bx,es:[bx+4]': base='txt'
        elif arg=='bx,[bp+6]': base='g'
        else: base=arg
    elif op=='push':
        mm=re.match(r'word ptr es:\[bx\+(\w+)h?\]',arg)
        if mm:
            v=int(mm.group(1).rstrip('h'),16)
            if stack and isinstance(stack[-1],tuple) and len(stack[-1])==2 and stack[-1][1]==v+2 and stack[-1][0]==base:
                stack[-1]=('%s[%d]'%(base,v//4),)
            else: stack.append((base,v))
        else: stack.append(arg)
    elif op in ('retf','leave') or op.startswith('j'): stack=[]
    elif op=='call':
        print(off,arg.split()[-1],[ (s[0] if isinstance(s,tuple) and len(s)==1 else s) for s in reversed(stack)])
        stack=[]
    elif op in('add','jmp','mov','cmp') and 'sp' not in arg: pass
