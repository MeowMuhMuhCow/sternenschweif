import subprocess
out=subprocess.run(['python3','calls.py','../o170/OVL170.ASM'],capture_output=True,text=True).stdout
M={}
for l in out.splitlines():
    off,fn,a=l.split(' ',2)
    if fn!='F_O190_01C6': continue
    a=[x for x in eval(a) if x not in('0b59ah','ds')]
    if 'bp-' in a[1]: continue
    t=a[0]; n=int(a[1][:-1],16) if a[1].endswith('h') else int(a[1])
    rest=', '.join('g_b67c[%s]'%x[5:-1] for x in a[2:])
    M[off]='O190_01c6(g_b59a, g->%s, %d, %s)'%(t,n,rest)
def sub(off,add,ind,call='f_00ca(g, sel);'):
    s=ind+'sel = %s - 1;\n'%M[off]
    if add: s+=ind+'if (sel != -2) {\n'+ind+'\tsel += %s;\n'%add+ind+'\t'+call+'\n'+ind+'}\n'
    else: s+=ind+'if (sel != -2)\n'+ind+'\t'+call+'\n'
    return s
def top(case,off,subs,call=None):
    s='\t\tcase %d:\n\t\t\tr = %s;\n\t\t\tif (r != -1) {\n\t\t\t\tswitch (r) {\n'%(case,M[off])
    for k,(o,a,c) in enumerate(subs):
        s+='\t\t\t\tcase %d:\n'%(k+1)+sub(o,a,'\t\t\t\t\t',c)+'\t\t\t\t\tbreak;\n'
    s+='\t\t\t\t}\n\t\t\t}\n\t\t\tbreak;\n'
    return s
F='f_00ca(g, sel);'
A='''void f_0396(struct game far *g)
{
	signed short r;
	signed short sel;

	do {
		if (!g->h->f1a5) {
			O190_00e8(g_b59a, g->txt[37]);
			return;
		} else
		switch (g->f0f) {
'''
A+=top(5,'042f',[('04b8','1',F),('0537','0x21',F),('05c6','6',F)])
A+=top(6,'062d',[('06c4','0x0c',F),('0753','0x12',F),('07b7','0x18',F)])
A+=top(7,'081e',[('08b5','0x1b',F),('0952','0x26',F),('09a8','0x2d',F)])
A+=top(8,'0a0f',[('0a6b','0x2f',F),('0b24','0x31',F),('0b7a','0x3a',F)])
for c,o,a in ((9,'0c97','0x3c'),(10,'0d63','0x4c')):
    A+='\t\tcase %d:\n'%c+'\t\t\tsel = %s - 1;\n'%M[o]+'\t\t\tif (sel != -2) {\n\t\t\t\tsel += %s;\n\t\t\t\tf_00ca(g, sel);\n\t\t\t} else\n\t\t\t\tr = -1;\n\t\t\tbreak;\n'%a
A+='''		}
	} while (r != -1);
}

void f_0da3(struct game far *g)
{
	signed short r;
	signed short sel;

	do {
		if (!g->h->f14e) {
			O190_00e8(g_b59a, g->txt[37]);
			return;
		} else
		switch (g->f0f) {
'''
c=lambda k,t:'O169_0f99(g, sel, %d, g->txt[%d]);'%(k,t)
A+=top(1,'0e32',[('0eeb',None,c(1,39)),('0fc3','9',c(2,40))])
A+=top(2,'1030',[('10cd','0x13',c(2,40)),('119a','0x20',c(3,41))])
A+=top(3,'1215',[('12d6','0x29',c(2,40)),('1379','0x1a',c(2,40)),('13e3','0x32',c(1,39))])
A+='''		}
	} while (r != -1);
}
'''
open('gen170.out','w').write(A)
