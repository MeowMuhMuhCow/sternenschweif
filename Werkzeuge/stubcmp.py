#!/usr/bin/env python3
"""stubcmp.py - vergleicht die Overlay-Einsprungtabellen (Stub-Segmente) zweier EXEs.
Jeder Eintrag: CD 3F <Offset der Funktion im Overlay> 00. Die Reihenfolge der
Eintraege bestimmt, welche Adresse eine Funktion von aussen hat."""
import sys, struct, re
def ld(f):
    d=open(f,'rb').read(); h=struct.unpack_from('<H',d,8)[0]*16; l,p=struct.unpack_from('<HH',d,2); e=(p-1)*512+l
    return d[h:e]
a=ld(sys.argv[1]); b=ld(sys.argv[2])
stubs=[]
for l in open(sys.argv[3],encoding='latin1'):
    m=re.match(r'\s*([0-9A-F]{5})H ([0-9A-F]{5})H ([0-9A-F]{5})H (\S+)\s+STUBSEG',l)
    if m and int(m[3],16)>0: stubs.append((m[4],int(m[1],16),int(m[3],16)))
def entries(img,s,ln):
    return [struct.unpack_from('<H',img,s+i+2)[0] for i in range(0x20,ln,5)]
bad=0; tot=0
for name,s,ln in stubs:
    ea,eb=entries(a,s,ln),entries(b,s,ln); tot+=len(ea)
    if ea!=eb:
        bad+=1
        perm=[eb.index(x) if x in eb else '?' for x in ea]
        print(f'{name:12} {len(ea):3} Eintraege  neu->orig-Position: {perm}')
print(f'{bad} von {len(stubs)} Stub-Segmenten in anderer Reihenfolge, {tot} Eintraege gesamt')
