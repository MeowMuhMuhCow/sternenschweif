#!/usr/bin/env python3
"""rawdb.py SEGID - erzeugt eine reine db-ASM (Originalbytes) fuer ein Segment; garantiert bytegleich."""
import sys, os
sys.path.insert(0,'/root/sternenschweif/q004/zerl'); import schweif
sid=sys.argv[1]; num=int(sid[1:]); ovl=sid[0]=='O'
name=f'OVL{num:03d}' if ovl else f'SEG{num:03d}'; wd=f'/root/sternenschweif/{"o%03d"%num if ovl else "a%02d"%num}'
code=[s for s in schweif.load('/root/sternenschweif/orig/SCHWEIF.EXE') if s['id']==sid][0]['code']
L=[f'; {name} - {sid} als reine Originalbytes (db), bytegleich','\t.386p',
   f'{name}_TEXT\tsegment byte public use16 \'CODE\'',f'\tpublic\tF_{sid}_0000',f'F_{sid}_0000\tlabel\tfar']
for i in range(0,len(code),16):
    L.append('\tdb\t'+','.join(f'0{b:02x}h' for b in code[i:i+16]))
L+=[f'{name}_TEXT\tends','\tend']
os.makedirs(wd,exist_ok=True)
open(f'{wd}/{name}.ASM','w',newline='\r\n').write('\n'.join(L)+'\n')
print('geschrieben',f'{wd}/{name}.ASM',len(code),'Bytes')
