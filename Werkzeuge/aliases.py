#!/usr/bin/env python3
"""aliases.py - setzt zusaetzliche oeffentliche Namen (C++-Namen) an Funktionen laut aliases.tsv"""
import re
ROOT='/root/sternenschweif'
for l in open(ROOT+'/aliases.tsv'):
    sid,off,name=l.rstrip('\n').split('\t'); num=int(sid[1:])
    f=f'{ROOT}/l{num:02d}/SEG{num:03d}.ASM'; s=open(f,newline='').read()
    lab=f'F_{sid}_{off}\tlabel\tfar'
    if lab not in s: print('FEHLT',lab); continue
    if f'\n{name}\tlabel\tfar' in s: continue
    s=s.replace(lab+'\r\n',lab+'\r\n'+f'{name}\tlabel\tfar\r\n',1)
    s=re.sub(r'(\tpublic\t[^\r\n]+\r\n)',r'\1'+f'\tpublic\t{name}\r\n'.replace('$','\\$') if False else r'\1',s,count=1)
    i=s.index('\tpublic\t'); s=s[:i]+f'\tpublic\t{name}\r\n'+s[i:]
    open(f,'w',newline='').write(s); print('Alias',name,'->',sid,off)
