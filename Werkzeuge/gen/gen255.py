import re,sys
t=open('o255.tpl').read()
def z(m):
    a=m.group(1).split()
    return ' = '.join('g_'+x for x in reversed(a))+' = 0;'
t=re.sub(r'Z\(([^)]*)\)',z,t)
def ix(m):
    a=int(m.group(1),16)
    if 0xb4a6<=a<=0xb4f6: b,base='g_b4a6',0xb4a6
    else: b,base='g_b4f7',0xb4f7
    i=a-base; return '%s[%d*9+%d]'%(b,i//9,i%9)
t=re.sub(r'g_b([0-9a-f]{3})',lambda m:ix(re.match('(.*)','b'+m.group(1))),t)
hdr=open('o255.hdr').read()
decl='extern unsigned char g_b4a6[81];\t/* Objekt-Nr. + 1 der 9x9 Nachbarzellen (0 = nicht zeichnen) */\nextern unsigned char g_b4f7[81];\t/* Verdeckung der 9x9 Nachbarzellen (*ds:b4a2[Nr.]) */\n'
open('OVL255.C','w').write(hdr+decl+'\n'+t)
