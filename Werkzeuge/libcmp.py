#!/usr/bin/env python3
"""libcmp.py - vergleicht Lage der Bibliotheksmodule in _TEXT (Map) mit der Original-Belegung"""
import re
m=open('/root/sternenschweif/lnk/SCHWEIF.MAP',errors='replace').read()
neu=[(int(a,16),int(b,16),c) for a,b,c in re.findall(r'\s0000:([0-9A-F]{4}) ([0-9A-F]{4}) C=CODE\s+S=_TEXT\s+G=\S+\s+M=(\S+)',m)]
orig={}
for f in ('schweif_cl.tsv','schweif_overlay.tsv'):
    for l in open(f'/root/sternenschweif/lib/{f}'):
        a,b,c,d,e=l.rstrip('\n').split('\t')
        if a=='R00': orig[d.upper()]=(int(b,16),int(c))
ok=0; bad=[]
for o,ln,name in neu:
    k=name.upper()
    if k in orig:
        if orig[k][0]==o: ok+=1
        else: bad.append((name,o,orig[k][0]))
print('Module an Originaladresse:',ok,'falsch:',len(bad),' _TEXT-Ende neu',hex(max(o+l for o,l,_ in neu)),'orig 0x4ccd')
for b in bad[:15]: print('  %-10s neu %#06x orig %#06x  (%+d)'%(b[0],b[1],b[2],b[1]-b[2]))
