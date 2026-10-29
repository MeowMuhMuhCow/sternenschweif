#!/usr/bin/env python3
"""mkiso.py AUSGABE.ISO [LABEL] - minimales, leeres ISO-9660-Abbild (Primaerdeskriptor, leeres Wurzelverzeichnis).
Fuer tools/exerun.sh: CD-Attrappe mit dem Label der Spiel-CD (DSA2_CD1), damit cd_check besteht."""
import sys, struct
out = sys.argv[1]; label = (sys.argv[2] if len(sys.argv) > 2 else 'DSA2_CD1').encode()
S = 2048; NSEC = 21; ROOT = 20


def both16(v): return struct.pack('<H', v) + struct.pack('>H', v)
def both32(v): return struct.pack('<I', v) + struct.pack('>I', v)


def dirrec(name, extent, size):
    r = bytes([0, 0]) + both32(extent) + both32(size) + bytes([94, 1, 1, 0, 0, 0, 0]) + bytes([2, 0, 0]) + both16(1) + bytes([1]) + name
    r = bytes([len(r) + (len(r) & 1)]) + r[1:]
    return r + (b'\0' if len(r) & 1 else b'')


img = bytearray(S * NSEC)
date = b'1994000000000000\0'
pvd = bytearray(S)
pvd[0:7] = b'\x01CD001\x01'
pvd[8:40] = b' ' * 32
pvd[40:72] = label.ljust(32, b' ')
pvd[80:88] = both32(NSEC)
pvd[120:124] = both16(1); pvd[124:128] = both16(1); pvd[128:132] = both16(S)
pvd[132:140] = both32(10)
pvd[140:144] = struct.pack('<I', 18); pvd[148:152] = struct.pack('>I', 19)
pvd[156:190] = dirrec(b'\0', ROOT, S)
for a, n in ((190, 128), (318, 128), (446, 128), (574, 128), (702, 37), (739, 37), (776, 37)):
    pvd[a:a + n] = b' ' * n
for k in range(4): pvd[813 + 17 * k:830 + 17 * k] = date
pvd[881] = 1
img[16 * S:17 * S] = pvd
img[17 * S:17 * S + 7] = b'\xffCD001\x01'
img[18 * S:18 * S + 10] = bytes([1, 0]) + struct.pack('<I', ROOT) + struct.pack('<H', 1) + b'\0\0'
img[19 * S:19 * S + 10] = bytes([1, 0]) + struct.pack('>I', ROOT) + struct.pack('>H', 1) + b'\0\0'
d = dirrec(b'\0', ROOT, S) + dirrec(b'\1', ROOT, S)
img[ROOT * S:ROOT * S + len(d)] = d
open(out, 'wb').write(img)
