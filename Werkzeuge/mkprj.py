#!/usr/bin/env python3
"""mkprj.py - erzeugt lnk/SCHW_CD.PRJ (Borland C++ 3.1 IDE-Projekt) aus der Objektliste in lnk/FULL.RES.
Attic hat SCHWEIF in der IDE gelinkt: deren eingebaute Linkerin schreibt den EXE-Namen in _EXEINFO_
in Grossbuchstaben (TLINK 5.1 immer klein). Vorlage: TCALC.PRJ aus den BC-3.1-Beispielen.
Optionen (Record-IDs per PRJ2MAK ermittelt): 0xf6 Overlays, 0xfc Modell large, 0xce ohne graphics.lib,
0x100 ohne Gleitkomma (kein emu/mathl), 0xcd ohne Debuginfo, 0xf1 ausfuehrliche MAP.
Knoten: 108 Bytes, Byte 0 Bit 3 = Overlay."""
import sys, re
sys.path.insert(0, '/root/sternenschweif/tools'); import prjlib
R = '/root/sternenschweif'
hdr, recs, tail = prjlib.load(R + '/tools/VORLAGE.PRJ')
for rid, v in ((0xf6, 1), (0xfc, 4), (0xce, 0), (0x100, 0), (0xcd, 2), (0xf1, 3)):
    prjlib.setopt(recs, rid, v)
prjlib.setstr(recs, 0x12e, '..\\bc\\INCLUDE'); prjlib.setstr(recs, 0x12f, '..\\bc\\LIB')
res = open(R + '/lnk/' + (sys.argv[1] if len(sys.argv) > 1 else 'FULL.RES')).read().split('\n')
objs = []
for l in res:
    l = l.strip()
    if l.upper().startswith('SCHW_CD'): break
    objs += (l[:-1] if l.endswith('+') else l).split()   # nur EIN '+' abschneiden ('/o++' am Zeilenende)
ovl = False; nodes = bytearray()
for o in objs:
    if o == '/o+': ovl = True; continue
    if o == '/o-': ovl = False; continue
    if o.upper().endswith('C0L'): continue          # Startcode setzt die IDE selbst davor
    nodes += prjlib.node(o + '.OBJ', ovl)
for r in recs:
    if r[0] == 0x35: r[1] = nodes + b'\x00\x80'
prjlib.save(R + '/lnk/SCHW_CD.PRJ', hdr, recs, tail)
print('Knoten', len(nodes) // 108)
