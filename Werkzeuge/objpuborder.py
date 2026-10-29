#!/usr/bin/env python3
"""objpuborder.py - bringt die Overlay-Einsprünge in Attics Reihenfolge.

Befund (17.10.2026): TLINK legt die Einsprünge eines Overlay-Moduls in der
UMGEKEHRTEN Reihenfolge der Funktions-PUBDEFs in dessen OBJ an (98 von 99
Modulen geprueft). Die PUBDEF-Reihenfolge bestimmt der Compiler: Attic hat C
kompiliert, Bright Eyes kompiliert C++ - die Reihenfolge weicht ab.

Weil Funktionszeiger-Tabellen im Datensegment feste Einsprung-Adressen
enthalten, zeigen sie bei falscher Reihenfolge auf falsche Funktionen.

Das Werkzeug liest die Einsprungtabellen des Originals (CD 3F <Offset> 00),
ermittelt so Attics Reihenfolge und schreibt die PUBDEFs der Overlay-OBJs
entsprechend um. Code und Daten bleiben unveraendert.

Aufruf: objpuborder.py ORIGINAL.EXE BLADEM.MAP <OBJ-Verzeichnis>
"""
import sys, os, re, struct

orig, mapf, objdir = sys.argv[1:4]


def records(d):
    p = 0
    while p < len(d):
        t = d[p]; ln = struct.unpack_from('<H', d, p + 1)[0]
        yield t, d[p + 3:p + 3 + ln - 1]
        p += 3 + ln


def make(t, body):
    r = bytes([t]) + struct.pack('<H', len(body) + 1) + body
    return r + bytes([(-sum(r)) & 0xFF])


def idx(b, q):
    v = b[q]
    return (((v & 0x7F) << 8) | b[q + 1], q + 2) if v & 0x80 else (v, q + 1)


d = open(orig, 'rb').read()
img = d[struct.unpack_from('<H', d, 8)[0] * 16:]
stub = {}
for l in open(mapf, encoding='latin1'):
    m = re.match(r'\s*([0-9A-F]{5})H ([0-9A-F]{5})H ([0-9A-F]{5})H (SEG\d+)_TEXT\s+STUBSEG', l)
    if m and int(m[3], 16) > 0:
        stub[m[4]] = (int(m[1], 16), int(m[3], 16))

changed = 0
for mod, (s, ln) in sorted(stub.items()):
    want = [struct.unpack_from('<H', img, s + i + 2)[0] for i in range(0x20, ln, 5)]
    path = os.path.join(objdir, mod + '.OBJ')
    recs = list(records(open(path, 'rb').read()))
    lnames = ['']; segs = [None]
    for t, b in recs:
        if t == 0x96:
            q = 0
            while q < len(b):
                n = b[q]; lnames.append(b[q + 1:q + 1 + n].decode('latin1')); q += 1 + n
        elif t == 0x98:
            acbp = b[0]; q = 1 + (3 if (acbp >> 5) == 0 else 0) + 2
            ni, q = idx(b, q); ci, q = idx(b, q)
            segs.append((lnames[ni], lnames[ci]))
    # alle Code-PUBDEF-Eintraege sammeln
    code_entries = []; hdr = None; first = None
    for k, (t, b) in enumerate(recs):
        if t == 0x90:
            q = 0; gi, q = idx(b, q); si, q = idx(b, q)
            h = b[:q]
            if gi == 0 and si == 0:
                q += 2; h = b[:q]
            if segs[si] and segs[si][1] == 'CODE':
                if first is None:
                    first = k
                hdr = h
                while q < len(b):
                    n = b[q]; e0 = q; q += 1 + n
                    off = struct.unpack_from('<H', b, q)[0]; q += 2
                    _, q = idx(b, q)
                    code_entries.append((off, b[e0:q]))
    if not code_entries:
        continue
    have = [o for o, _ in code_entries]
    if sorted(have) != sorted(want):
        if len(have) != len(want):
            print(f'{mod}: Funktionen passen nicht zum Original, uebersprungen'); continue
        # Funktionen gleich, aber verschoben (Modul anders lang): nach Rangfolge zuordnen
        rank = dict(zip(sorted(want), sorted(have)))
        want = [rank[o] for o in want]
        print(f'{mod}: Funktionsadressen verschoben, Zuordnung nach Reihenfolge')
    if list(reversed(have)) == want:
        continue
    byoff = {o: e for o, e in code_entries}
    ordered = [byoff[o] for o in reversed(want)]     # umgekehrt -> TLINK dreht wieder um
    out = bytearray(); body = bytearray(hdr)
    for k, (t, b) in enumerate(recs):
        if t == 0x90:
            q = 0; gi, q = idx(b, q); si, q = idx(b, q)
            if segs[si] and segs[si][1] == 'CODE':
                if k == first:
                    for e in ordered:
                        if len(body) + len(e) > 1000:
                            out += make(0x90, bytes(body)); body = bytearray(hdr)
                        body += e
                    out += make(0x90, bytes(body))
                continue
        out += make(t, b)
    open(path, 'wb').write(out)
    changed += 1
print(f'{changed} Overlay-Module umsortiert')
