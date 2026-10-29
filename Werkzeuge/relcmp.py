#!/usr/bin/env python3
"""relcmp.py NEU.EXE ORIG.EXE - vergleicht EXE-Header und Relocation-Tabellen."""
import sys,struct
def ld(f):
    d=open(f,'rb').read(); n=struct.unpack_from('<H',d,6)[0]; lf=struct.unpack_from('<H',d,0x18)[0]
    return [struct.unpack_from('<HH',d,lf+4*i) for i in range(n)], struct.unpack_from('<13H',d,2)
ra,ha=ld(sys.argv[1]); rb,hb=ld(sys.argv[2])
names=['last','pages','nrel','hdrpar','minalloc','maxalloc','SS','SP','csum','IP','CS','lfarlc','ovl']
print('Header-Felder anders:',[(n,hex(x),hex(y)) for n,x,y in zip(names,ha,hb) if x!=y] or 'keine')
sa,sb=set(ra),set(rb)
print(f'Relocations: neu {len(ra)}, orig {len(rb)}, nur orig {len(sb-sa)}, nur neu {len(sa-sb)}, Reihenfolge der gemeinsamen gleich: {[x for x in rb if x in sa]==[x for x in ra if x in sb]}')
for o,s in sorted(sa-sb): print(f'  nur neu {s:04X}:{o:04X}')
