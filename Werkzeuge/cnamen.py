#!/usr/bin/env python3
"""cnamen.py CDIR ASMDIR > cnamen.tsv - Namenstabelle C-Name -> Linkname fuer den Fix-Modus (cobjfix.py --ref).

Je C-Modul aus c_module.tsv: Public der BCC-OBJ (CDIR/<modul>.OBJ, FIX_BUGS=0) an Offset o <-> Public der
ASM-Fassung (ASMDIR/<modul>.OBJ) an demselben Offset. Gebraucht nur, wenn eine Fehlerbehebung eine Funktion
eines anderen Moduls aufruft, die das Modul vorher nicht benutzt hat (Overlays exportieren nur Linknamen).
Ausgabe: C-Name <TAB> Linkname <TAB> Modul (nur Namen, die nicht schon Linknamen sind)."""
import sys
sys.path.insert(0, '/root/sternenschweif/tools'); import objfix
cdir, adir = sys.argv[1:3]
R = '/root/sternenschweif'
for l in open(R + '/c_module.tsv'):
    if l.startswith('#') or not l.strip(): continue
    n = l.split('\t')[0]; m = ('OVL' + n[1:]) if n.startswith('O') else 'SEG' + n
    try:
        A = objfix.parse(f'{adir}/{m}.OBJ'); C = objfix.parse(f'{cdir}/{m}.OBJ')
    except FileNotFoundError:
        continue
    cs = [s for s in C['segs'] if s.endswith('_TEXT')][0]
    aby = {}
    for s, o, nm in A['pubs']:
        if s == cs: aby.setdefault(o, nm)
    for s, o, nm in C['pubs']:
        if s == cs and o in aby and aby[o] != nm:
            print(f'{nm}\t{aby[o]}\t{m}')
# zweite Quelle: Externe, die irgendein Modul schon benutzt - Fixups der BCC-OBJ (CDIR/<m>.OBJ, C-Namen) und ihrer
# linkfertigen Fassung (CDIR/<m>.FIX, Linknamen) an derselben Stelle. Nur Zuordnungen ohne Inline-Versatz
# (gleiche Bytes im Fixup-Feld), damit z. B. "_g_x -> _g_ca7e+2" nicht als reiner Namenstausch erscheint.
sys.path.insert(0, R + '/tools'); import objbytes
seen = set()
for l in open(R + '/c_module.tsv'):
    if l.startswith('#') or not l.strip(): continue
    n = l.split('\t')[0]; m = ('OVL' + n[1:]) if n.startswith('O') else 'SEG' + n
    try:
        C = objfix.parse(f'{cdir}/{m}.OBJ'); X = objfix.parse(f'{cdir}/{m}.FIX')
        cb = objbytes.seg_bytes(f'{cdir}/{m}.OBJ'); xb = objbytes.seg_bytes(f'{cdir}/{m}.FIX')
    except (FileNotFoundError, KeyError, IndexError):
        continue
    xf = {(f['seg'], f['loc']): f for f in X['fix']}
    for f in C['fix']:
        if f['tgt'][0] != 'EXT': continue
        g = xf.get((f['seg'], f['loc']))
        if not g or g['tgt'][0] != 'EXT' or g['tgt'][1] == f['tgt'][1]: continue
        if f['seg'] not in cb or f['seg'] not in xb: continue
        if cb[f['seg']][f['loc']:f['loc'] + 2] != xb[f['seg']][f['loc']:f['loc'] + 2]: continue
        k = (f['tgt'][1], g['tgt'][1])
        if k in seen: continue
        seen.add(k); print(f'{k[0]}\t{k[1]}\t{m}')
