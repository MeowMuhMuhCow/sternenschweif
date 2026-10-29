#!/usr/bin/env python3
"""execmp.py NEU.EXE [ORIG.EXE] - vergleicht zwei VROOMM-EXEs bereichsweise"""
import sys, struct
a=open(sys.argv[1],'rb').read(); b=open(sys.argv[2] if len(sys.argv)>2 else '/root/sternenschweif/orig/SCHWEIF.EXE','rb').read()
def info(d):
    l,p,nr,hp=struct.unpack_from('<HHHH',d,2); e=(p-1)*512+l if l else p*512; h=hp*16; ro=struct.unpack_from('<H',d,0x18)[0]
    rel=sorted(struct.unpack_from('<HH',d,ro+4*i) for i in range(nr))
    return dict(end=e,hdr=h,img=d[h:e],rel=rel,fbov=d[e:],ss=struct.unpack_from('<HH',d,0xe),csip=struct.unpack_from('<HH',d,0x14),minmax=struct.unpack_from('<HH',d,0xa))
A,B=info(a),info(b)
rows=[('Dateigroesse',len(a),len(b)),('Header',A['hdr'],B['hdr']),('Lademodul',len(A['img']),len(B['img'])),('Relocations',len(A['rel']),len(B['rel'])),
      ('SS:SP',A['ss'],B['ss']),('IP:CS',A['csip'],B['csip']),('min/max',A['minmax'],B['minmax']),('FBOV-Teil',len(A['fbov']),len(B['fbov']))]
for r in rows: print(f'{r[0]:14} neu {str(r[1]):>16}  orig {str(r[2]):>16}  {"gleich" if r[1]==r[2] else "ANDERS"}')
ra,rb=set(A['rel']),set(B['rel']); print('Relocations gemeinsam',len(ra&rb),'nur neu',len(ra-rb),'nur orig',len(rb-ra))
def regions(x,y,name,step=4096):
    n=min(len(x),len(y)); diff=sum(1 for i in range(n) if x[i]!=y[i])+abs(len(x)-len(y))
    first=next((i for i in range(n) if x[i]!=y[i]),None)
    print(f'{name}: {n} Bytes verglichen, {diff} anders, erste Abweichung {hex(first) if first is not None else "-"}')
    return first
regions(A['img'][:0x24eb*16],B['img'][:0x24eb*16],'Code+Stubs (bis DGROUP)')
regions(A['img'][:118758],B['img'][:118758],'residenter Code')
regions(A['img'][0x24eb*16:],B['img'][0x24eb*16:],'DGROUP+Stack')
regions(A['fbov'],B['fbov'],'FBOV')
