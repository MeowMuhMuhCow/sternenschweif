#!/usr/bin/env python3
"""fixvgl_adr.py A.FIX B.FIX [MAP] - wie names/tools/fixvgl.py, aber Datenverweise auf DGROUP-Publics werden zur
DS-Adresse aufgeloest (Public-Offset + Inline-Wert + disp). Damit gelten `_g_5a03+8` und `_g_5a09+2` als gleich
(beide ds:5a0b). ADRESSGLEICH = gleiches Linkergebnis trotz anderer Hilfsnamen."""
import sys, struct
sys.path.insert(0, '/root/sternenschweif/tools'); import objfix, objbytes
sys.path.insert(0, '/root/sternenschweif/wovd1'); import cobjfix_d
DG = 0x24EB
mapf = sys.argv[3] if len(sys.argv) > 3 else '/root/sternenschweif/ref_obj/SCHW_CD.MAP'
alln = cobjfix_d.mapnames(mapf)
def key(p):
    o = objfix.parse(p); sb = {s: bytearray(b) for s, b in objbytes.seg_bytes(p).items()}
    fx = []; dnames = set()
    for f in o['fix']:
        t = f['tgt']
        if t[0] == 'EXT' and t[1] in alln and alln[t[1]][0] == DG and f['typ'] in (1, 3, 5, 9):
            b = sb[f['seg']]; inl = struct.unpack_from('<H', b, f['loc'])[0]
            ad = (alln[t[1]][1] + inl + f['disp']) & 0xffff; struct.pack_into('<H', b, f['loc'], 0)
            fx.append((f['seg'], f['loc'], f['typ'], ('DS', ad), 0)); dnames.add(t[1])
        else: fx.append((f['seg'], f['loc'], f['typ'], t, f['disp']))
    ext = sorted(set(o['ext']) - dnames)
    return {s: bytes(b) for s, b in sb.items()}, sorted(fx), sorted(o['pubs']), ext
a, b = key(sys.argv[1]), key(sys.argv[2])
names = ['Segmentbytes', 'Fixups', 'Publics', 'Externs']
diff = [n for n, x, y in zip(names, a, b) if x != y]
print(sys.argv[2].split('/')[-1], 'ADRESSGLEICH' if not diff else 'ABWEICHUNG: ' + ', '.join(diff))
sys.exit(0 if not diff else 1)
