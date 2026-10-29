#!/usr/bin/env python3
"""relocfix.py - fehlende Relocations im Datensegment aus dem Original uebernehmen.

Bright Eyes legt das Datensegment als festen Byteblock an. Darin stehen
Fern-Zeiger (Segment:Offset), deren Segmentteil DOS beim Laden an die
Ladeadresse anpassen muss. Im Original stehen diese Stellen in der
Relocation-Tabelle, im Nachbau nicht -> die Zeiger zeigen ins Leere.
Dasselbe gilt fuer fest eingetragene Segmentwerte im Code, z. B.
RealMake(0xb2a + reloc_game, 0x244) fuer den Timer-Interrupt (seg004).

Das Werkzeug uebernimmt jede Relocation des Originals, die
  - im Nachbau fehlt und
  - an deren Stelle im Nachbau dieselben 2 Bytes stehen wie im Original,
und schreibt eine neue EXE mit erweiterter Tabelle.

Aufruf: relocfix.py ORIGINAL.EXE NACHBAU.EXE AUSGABE.EXE
"""
import sys, struct


def parse(d):
    n = struct.unpack_from('<H', d, 6)[0]
    hdrpar = struct.unpack_from('<H', d, 8)[0]
    lfa = struct.unpack_from('<H', d, 0x18)[0]
    rel = [struct.unpack_from('<HH', d, lfa + 4 * i) for i in range(n)]
    last, pages = struct.unpack_from('<HH', d, 2)
    img_end = (pages - 1) * 512 + last if last else pages * 512
    return hdrpar * 16, lfa, rel, img_end


def ds_start(img):
    k = img.find(b'Borland C++ - Copyright')
    return k - 4


def main(orig, new, out):
    a = open(new, 'rb').read(); b = open(orig, 'rb').read()
    ha, lfa_a, ra, enda = parse(a); hb, _, rb, _ = parse(b)
    ia, ib = a[ha:enda], b[hb:]
    ds_a, ds_b = ds_start(ia), ds_start(ib)
    if ds_a != ds_b:
        raise SystemExit(f'Datensegment liegt unterschiedlich: {ds_a:#x} / {ds_b:#x}')
    have = {s * 16 + o for o, s in ra}
    add = []
    for o, s in rb:
        lin = s * 16 + o
        if lin not in have and lin + 2 <= len(ia) \
           and ia[lin:lin + 2] == ib[lin:lin + 2]:
            add.append((o, s)); have.add(lin)
    rel = ra + add
    # neuen Header bauen: Relocation-Tabelle direkt nach dem 0x1C-Basisheader
    lfa = 0x1C
    hdr_len = lfa + 4 * len(rel)
    hdr_par = (hdr_len + 15) // 16
    hdr_par = (hdr_par + 31) // 32 * 32          # wie TLINK: auf 512 Bytes runden
    hdr = bytearray(a[:0x1C])
    struct.pack_into('<H', hdr, 6, len(rel))
    struct.pack_into('<H', hdr, 8, hdr_par)
    struct.pack_into('<H', hdr, 0x18, lfa)
    for o, s in rel:
        hdr += struct.pack('<HH', o, s)
    hdr += b'\0' * (hdr_par * 16 - len(hdr))
    body = a[ha:]                                  # Image + Overlays unveraendert
    total_img = hdr_par * 16 + len(ia)
    struct.pack_into('<H', hdr, 2, total_img % 512)
    struct.pack_into('<H', hdr, 4, (total_img + 511) // 512)
    open(out, 'wb').write(bytes(hdr) + body)
    print(f'{len(add)} Relocations ergaenzt ({len(ra)} -> {len(rel)}), Original hat {len(rb)}')
    print(f'Header {ha} -> {hdr_par * 16} Bytes, Datei {len(a)} -> {len(hdr) + len(body)} Bytes')


if __name__ == '__main__':
    main(*sys.argv[1:4])
