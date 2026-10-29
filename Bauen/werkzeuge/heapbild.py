#!/usr/bin/env python3
import os as _swos; _SW = _swos.environ.get('SW_ROOT', '/root/sternenschweif')  # Bau-Paket
"""heapbild.py SPEC LOAD.BIN - Speicherbild fuer einen frischen BCC-Lauf (tools/frisch.txt, Vorlauf "@...").
SPEC = "@KKKK:ooo=ww,ooo=ww,..." (hex): 1 KB ab XMS-KB KKKK, alles 00 ausser den genannten Bytes (Versatz im KB).
Schreibt LOAD.BIN (1024 Bytes) und gibt die DOS-Zeile fuer tools/fill/FILL.COM aus ("FILL.COM L KKKK 0001").

Hintergrund (OVL030): BCC 3.1 liest bei der switch-Registerwahl nicht initialisierten Heap (im frischen DOSBox-Lauf
XMS-KB 277 = 0x115: Flag +0x2E8 Bit 0, Typcode +0x2F8; 0x17/0x8F loesen die lange Form aus). Frueher stellte der
Vorlauf OVL198 diesen Zustand her - das haengt aber von den Kopfdateien ab, die OVL198 einliest (jede Aenderung an
names/inc verschiebt den Rest). Das Speicherbild ist davon unabhaengig (Unterlagen: x030/t/Z0.BIN)."""
import sys
spec, out = sys.argv[1], sys.argv[2]
kb, _, rest = spec.lstrip('@').partition(':')
img = bytearray(1024)
for p in filter(None, rest.split(',')):
    o, v = p.split('=')
    img[int(o, 16)] = int(v, 16)
open(out, 'wb').write(img)
print('FILL.COM L %04X 0001\r' % int(kb, 16))
