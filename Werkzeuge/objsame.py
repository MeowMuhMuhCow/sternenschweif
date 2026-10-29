#!/usr/bin/env python3
"""objsame.py A.OBJ B.OBJ - Exit 0, wenn beide OBJ denselben Inhalt haben: gleiche Segmentbytes UND gleiche Fixups
(Lage, Art, Ziel). Fix-Modus (tools/build.sh, bauen.py): eine Fix-Fassung, die sich nur in einem Fixup-Ziel
unterscheidet (z. B. g_map_y0 statt g_map_x0), hat dieselben Bytes - der reine Bytevergleich reicht nicht."""
import sys
sys.path.insert(0, '/root/sternenschweif/tools'); import objfix, objbytes


def inhalt(p):
    fx = sorted((f['seg'], f['loc'], f['typ'], f['M'], f['tgt'], f['ftgt'], f.get('disp')) for f in objfix.parse(p)['fix'])
    return objbytes.seg_bytes(p), fx


sys.exit(0 if inhalt(sys.argv[1]) == inhalt(sys.argv[2]) else 1)
