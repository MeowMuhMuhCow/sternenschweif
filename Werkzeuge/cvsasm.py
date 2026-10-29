#!/usr/bin/env python3
"""cvsasm.py NNN... - vergleicht Code-Bytes cmod/SEGNNN.OBJ (C) mit lnk/SEGNNN.OBJ (ASM-Linkfassung),
Fixup-Felder beider Seiten ausgeblendet"""
import sys, os; sys.path.insert(0,'/root/sternenschweif/tools'); import objbytes, objfix, selfoff
CD=os.environ.get('CDIR','/root/sternenschweif/cmod')
W={0:1,1:2,2:2,3:4,4:1,5:2,9:2,11:4,13:4}
def masked(path,seg):
    b=bytearray(objbytes.seg_bytes(path)[seg]); m=set()
    for f in objfix.parse(path)['fix']:
        if f['seg']==seg: m.update(range(f['loc'],f['loc']+W.get(f['typ'],2)))
    return b,m
for n in sys.argv[1:]:
    nm=f'OVL{n[1:]}' if n[0]=='O' else f'SEG{n}'; seg=nm+'_TEXT'
    c,mc=masked(f'{CD}/{nm}.OBJ',seg); a,ma=masked(f'/root/sternenschweif/lnk/{nm}.OBJ',seg)
    d=[i for i in range(min(len(a),len(c))) if a[i]!=c[i] and i not in mc and i not in ma]
    d+=selfoff.check(f'/root/sternenschweif/lnk/{nm}.OBJ',f'{CD}/{nm}.OBJ')
    x={s_:len(b_) for s_,b_ in objbytes.seg_bytes(f'{CD}/{nm}.OBJ').items() if not s_.endswith('_TEXT') and len(b_)}
    print(f'{nm}: C {len(c)} ASM {len(a)}  Abweichungen {len(d)}', [hex(x) for x in d[:8]] if d else (('GLEICH (+eigene Daten '+str(x)+', cobjfix_d prueft)') if (x and len(a)==len(c)) else ('GLEICH' if len(a)==len(c) else 'LAENGE')))
