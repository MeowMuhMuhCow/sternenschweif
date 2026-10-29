import re,subprocess
R='/root/sternenschweif/'
body=open(R+'wovat/gen/b206.txt').read().splitlines()
# ??-Bloecke (zusammenhaengend) der Reihe nach ersetzen; ein ?? -Block kann direkt folgende generierte Zeilen mitnehmen
fix=[
 # 1: Kopf Name/Typ/Geschlecht (ersetzt ?? bis einschliesslich des sprintf-??)
 ['\tsprintf(g_b674, h->txt[4], h->name, g_b67c[(h->f34 ? 0x3e : 0x32) + h->f33],\n\t\t!h->f34 ? h->txt[116] : h->txt[117]);'],
 # 2: g_b67c[h->f38 + 4] (die folgende generierte sprintf-Zeile wird ersetzt)
 ['\tsprintf(g_b674, h->txt[5], h->u35, h->f36, g_b67c[h->f38 + 4]);', 1],
 # 3: avg; O006_0000(h, 7) + Rest
 ['\tavg = ((signed char)(h->f4a + h->f4b) + (signed char)(h->f53 + h->f54) + (signed char)(h->f59 + h->f5a)) / 4;'],
 ['\tsprintf(g_b674, h->txt[16], (signed char)(h->f78 + h->fbb), h->f42, O006_0000(h, 7) + avg);', 'pre1'],
 ['\tsprintf(g_b674, h->txt[17], h->f72 + (signed char)(h->f59 + h->f5a), O077_0000(h), O006_0000(h, 8) + avg);', 'pre1'],
 ['\tsprintf(g_b674, h->txt[18], (signed char)(h->f59 + h->f5a) * 100, h->f4ed);'],
]
out=[];k=0;i=0
while i<len(body):
    l=body[i]
    if '/* ??' in l:
        j=i
        while j<len(body) and '/* ??' in body[j]: j+=1
        f=fix[k]; k+=1
        if len(f)>1 and f[1]=='pre1':
            out.pop()   # vorausgehende generierte Zeile (O006_0000-Aufruf) verwerfen
        out.append(f[0])
        i=j
        if len(f)>1 and f[1]==1: i+=1
        continue
    out.append(l); i+=1
assert k==len(fix),k
body='\n'.join(out)
fields=dict(re.findall(r'h->([fu])([0-9a-f]+)',body)[::-1])
F={}
for t,x in re.findall(r'h->([fu])([0-9a-f]+)',body): F[int(x,16)]=t
# Typen aus dem Generatorlauf
typ={}
for e in subprocess.run(['python3',R+'wovat/gen/g206.py','0203','1257'],capture_output=True,text=True).stderr.split()[1:]:
    x,t=e.split(':'); typ[int(x,16)]=t
typ[0x72]='w'; typ[0x4ed]='w'; typ[0x38]='c'; typ[0x78]='c'; typ[0xbb]='c'
for x in (0x10,0x12,0x2): typ.pop(x,None)
for x,t in F.items():
    if x not in typ: typ[x]='c' if t=='f' else 'u'
typ[0x35]='u'; typ[0x14e]='u'
size={'c':1,'u':1,'w':2,'l':4}
ctype={'c':'signed char','u':'unsigned char','w':'signed short','l':'signed long'}
fixed={0x22:('char','name[0x11]',0x11),0x1a8:('struct slot','inv[23]',23*0x1b),0x5e7:('void far *','f5e7',4),0x5eb:('char far * far *','txt',4)}
items=sorted([(x,(ctype[t],('u' if t=='u' else 'f')+'%x'%x,size[t])) for x,t in typ.items()]+list(fixed.items()))
s='struct hero {\t\t\t\t/* 0x5ef Bytes */\n';pos=0
for x,(ct,nm,sz) in items:
    if x>pos: s+='\tchar pad%x[0x%x - 0x%x];\n'%(pos,x,pos) if pos else '\tchar pad0[0x%x];\n'%x
    s+='\t%s %s;%s/* +0x%x */\n'%(ct,nm,'\t'*(3-len(ct+nm)//8 if len(ct+nm)<24 else 1),x); pos=x+sz
s+=('\tchar pad%x[0x5ef - 0x%x];\n'%(pos,pos) if pos<0x5ef else '')+'\n\tsigned short mage() { return f33 >= 7; }\n};\n'
tpl=open(R+'wovat/OVL206.TPL').read()
open(R+'wovat/OVL206.CPP','w').write(tpl.replace('@HERO@',s).replace('@BODY@','\n'.join('\t\t'+l for l in body.split('\n'))))
