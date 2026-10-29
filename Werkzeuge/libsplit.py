#!/usr/bin/env python3
"""libsplit.py LIB - zerlegt eine OMF-Bibliothek (Borland TLIB) in ihre Module.
Liefert [(modulname, bytes)]."""
import struct, sys
def split(path):
    d = open(path, 'rb').read()
    assert d[0] == 0xF0
    page = struct.unpack_from('<H', d, 1)[0] + 3
    mods = []; p = page
    while p < len(d):
        if d[p] == 0xF1: break           # Bibliotheksende / Verzeichnis
        if d[p] != 0x80: p += 1; continue
        start = p; name = d[p + 4:p + 4 + d[p + 3]].decode('latin1')
        while True:
            t = d[p]; n = struct.unpack_from('<H', d, p + 1)[0]; p += 3 + n
            if t in (0x8A, 0x8B): break
        mods.append((name, d[start:p]))
        p = (p + page - 1) // page * page
    return mods
if __name__ == '__main__':
    m = split(sys.argv[1]); print(len(m), 'Module;', [x[0] for x in m[:10]])
