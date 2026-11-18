#!/usr/bin/env python3
"""
vergleich.py (W5) - Bild-fuer-Bild-Referenzvergleich zwischen Original (DOSBox)
und dem nativen Windows-Port (SCHWEIF_PORT.EXE).

Verwendung:
    python3 Port/tools/vergleich.py <bild_original.bmp> <bild_port.bmp> [diff_ausgabe.png]

Berechnet:
- Exakte Pixel-Uebereinstimmung (%)
- Toleranz-Uebereinstimmung (|dR|+|dG|+|dB| <= 12, z. B. bei 6-Bit->8-Bit DAC-Rundung)
- Mittlere absolute RGB-Abweichung pro Kanal
- Erzeugt ein visuelles Differenzbild (PNG oder BMP), in dem Abweichungen rot/gelb hervorgehoben sind.
"""
import os
import sys
import struct
import zlib


def load_png_rgb(data, path):
    pos = 8
    w = h = bit_depth = color_type = 0
    pal = []
    idat_chunks = []
    while pos + 8 <= len(data):
        length = struct.unpack_from(">I", data, pos)[0]
        tag = data[pos + 4 : pos + 8]
        body = data[pos + 8 : pos + 8 + length]
        pos += 12 + length
        if tag == b"IHDR":
            w, h, bit_depth, color_type = struct.unpack_from(">IIBB", body, 0)
        elif tag == b"PLTE":
            for i in range(0, len(body), 3):
                pal.append((body[i], body[i + 1], body[i + 2]))
        elif tag == b"IDAT":
            idat_chunks.append(body)
        elif tag == b"IEND":
            break
    raw = zlib.decompress(b"".join(idat_chunks))
    bpp_bytes = 1 if color_type == 3 else (3 if color_type == 2 else 4)
    stride = 1 + w * bpp_bytes
    pixels = []
    prev_row = bytearray(w * bpp_bytes)
    for y in range(h):
        f_type = raw[y * stride]
        row = bytearray(raw[y * stride + 1 : (y + 1) * stride])
        for i in range(len(row)):
            a = row[i - bpp_bytes] if i >= bpp_bytes else 0
            b = prev_row[i]
            c = prev_row[i - bpp_bytes] if i >= bpp_bytes else 0
            if f_type == 1:
                row[i] = (row[i] + a) & 0xFF
            elif f_type == 2:
                row[i] = (row[i] + b) & 0xFF
            elif f_type == 3:
                row[i] = (row[i] + ((a + b) >> 1)) & 0xFF
            elif f_type == 4:
                p = a + b - c
                pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                pr = a if (pa <= pb and pa <= pc) else (b if pb <= pc else c)
                row[i] = (row[i] + pr) & 0xFF
        prev_row = row
        if color_type == 3:
            for x in range(w):
                idx = row[x]
                pixels.append(pal[idx] if idx < len(pal) else (0, 0, 0))
        else:
            for x in range(w):
                pixels.append((row[x * bpp_bytes], row[x * bpp_bytes + 1], row[x * bpp_bytes + 2]))
    return w, h, pixels


def load_bmp_rgb(path):
    with open(path, "rb") as f:
        data = f.read()
    if data[:8] == b"\x89PNG\r\n\x1a\n":
        return load_png_rgb(data, path)
    if data[:2] != b"BM":
        raise ValueError(f"Keine BMP/PNG-Datei: {path}")
    bfOffBits = struct.unpack_from("<I", data, 10)[0]
    biSize, w, h = struct.unpack_from("<Iii", data, 14)
    bpp = struct.unpack_from("<H", data, 28)[0]
    flip = h > 0
    h = abs(h)
    pixels = []
    if bpp == 24:
        stride = ((w * 3 + 3) // 4) * 4
        for y in range(h):
            sy = (h - 1 - y) if flip else y
            row = data[bfOffBits + sy * stride : bfOffBits + sy * stride + w * 3]
            for x in range(w):
                b, g, r = row[x * 3], row[x * 3 + 1], row[x * 3 + 2]
                pixels.append((r, g, b))
    elif bpp == 8:
        pal_off = 14 + biSize
        pal = []
        for i in range(256):
            b, g, r, _ = struct.unpack_from("<BBBB", data, pal_off + i * 4)
            pal.append((r, g, b))
        stride = ((w + 3) // 4) * 4
        for y in range(h):
            sy = (h - 1 - y) if flip else y
            row = data[bfOffBits + sy * stride : bfOffBits + sy * stride + w]
            for x in range(w):
                pixels.append(pal[row[x]])
    else:
        raise ValueError(f"Nicht unterstuetzte Farbtiefe {bpp} bpp in {path}")
    return w, h, pixels


def save_png_rgb(path, w, h, pixels):
    rows = []
    for y in range(h):
        row = bytearray(1 + w * 3)
        for x in range(w):
            r, g, b = pixels[y * w + x]
            row[1 + x * 3 + 0] = r
            row[1 + x * 3 + 1] = g
            row[1 + x * 3 + 2] = b
        rows.append(bytes(row))
    raw = b"".join(rows)

    def chunk(tag, body):
        return struct.pack(">I", len(body)) + tag + body + struct.pack(">I", zlib.crc32(tag + body) & 0xFFFFFFFF)

    ihdr = struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0)
    png = b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", ihdr) + chunk(b"IDAT", zlib.compress(raw)) + chunk(b"IEND", b"")
    with open(path, "wb") as f:
        f.write(png)


def compare_images(path_a, path_b, diff_out=None):
    wa, ha, pa = load_bmp_rgb(path_a)
    wb, hb, pb = load_bmp_rgb(path_b)
    if (wa, ha) != (wb, hb):
        raise ValueError(f"Bildgroessen unterschiedlich: {wa}x{ha} vs {wb}x{hb}")

    n = wa * ha
    exact = 0
    within_tol = 0
    sum_diff = 0
    diff_pixels = []

    for (ra, ga, ba), (rb, gb, bb) in zip(pa, pb):
        dr = abs(ra - rb)
        dg = abs(ga - gb)
        db = abs(ba - bb)
        d = dr + dg + db
        sum_diff += d
        if d == 0:
            exact += 1
            within_tol += 1
            diff_pixels.append((ra // 3, ga // 3, ba // 3))
        elif d <= 12:
            within_tol += 1
            diff_pixels.append((min(255, ra // 2 + 80), min(255, ga // 2 + 80), 0))
        else:
            diff_pixels.append((255, min(255, d), 0))

    exact_pct = 100.0 * exact / n
    tol_pct = 100.0 * within_tol / n
    diff_pct = 100.0 - tol_pct
    mean_rgb = sum_diff / (n * 3.0)

    print(f"=== W5 Referenzvergleich ===")
    print(f"Bild A: {path_a} ({wa}x{ha})")
    print(f"Bild B: {path_b} ({wb}x{hb})")
    print(f"Exakt gleich:     {exact:6d} / {n} ({exact_pct:.2f} %)")
    print(f"Toleranz (<=12):  {within_tol:6d} / {n} ({tol_pct:.2f} %)")
    print(f"Abweichung:       {diff_pct:.2f} % (mittlere Kanal-Differenz: {mean_rgb:.2f} / 255)")

    if diff_out:
        save_png_rgb(diff_out, wa, ha, diff_pixels)
        print(f"Differenzbild:    {diff_out}")

    return diff_pct


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Verwendung: python3 Port/tools/vergleich.py <bild_a.bmp> <bild_b.bmp> [diff.png]")
        sys.exit(1)
    out = sys.argv[3] if len(sys.argv) >= 4 else None
    compare_images(sys.argv[1], sys.argv[2], out)
