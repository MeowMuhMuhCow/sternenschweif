#!/usr/bin/env python3
"""ovlstruct.py EXE - findet die VROOMM-Stub-Segmente im Ladebild und ihre Overlays.
Stub-Kopf (Borland): +0 CD 3F, +4 dd Dateiposition des Overlays (relativ zum Overlay-Block),
+8 dw Codegroesse, +0A dw Fixup-Bytes, +0C dw Anzahl Einsprünge, danach ab +20 Eintraege CD 3F dw off 00.
Ausgabe: Tabelle stub_para, ovl_dateipos, codegroesse, fixupbytes, einsprünge."""
import sys, struct
d = open(sys.argv[1], 'rb').read()
h = struct.unpack_from('<H', d, 8)[0] * 16
l, p = struct.unpack_from('<HH', d, 2); e = (p - 1) * 512 + l if l else p * 512
img = d[h:e]
stubs = []
for para in range(0, len(img) - 0x25, 16):
    if img[para:para + 2] != b'\xcd\x3f': continue
    fo, cs, rs, ne = struct.unpack_from('<IHHH', img, para + 4)
    if ne == 0 or ne > 2000: continue
    ok = all(img[para + 0x20 + 5 * i:para + 0x22 + 5 * i] == b'\xcd\x3f' and img[para + 0x24 + 5 * i] == 0 for i in range(ne))
    if not ok: continue
    offs = [struct.unpack_from('<H', img, para + 0x22 + 5 * i)[0] for i in range(ne)]
    stubs.append((para, fo, cs, rs, ne, offs))
if __name__ == '__main__':
    print(f'Kopf {h} Bytes, Ladebild {len(img)} Bytes, Overlay-Block {len(d)-e} Bytes ab Datei {e:#x}')
    print(f'{len(stubs)} Stub-Segmente, {sum(s[4] for s in stubs)} Einsprünge, Code {sum(s[2] for s in stubs)} Bytes, Fixups {sum(s[3] for s in stubs)} Bytes')
    for s in stubs[:5]: print(f'  stub {s[0]:#07x} ovl {s[1]:#08x} code {s[2]:6} fix {s[3]:5} ein {s[4]:4} offs {s[5][:4]}')
    print('  erster Stub im Ladebild bei', hex(stubs[0][0]), ' letzter', hex(stubs[-1][0]))
