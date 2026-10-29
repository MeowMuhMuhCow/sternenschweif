#!/usr/bin/env python3
"""exemod_stack.py - setzt den Start-Stackzeiger im MZ-Kopf (wie Borlands EXEMOD /STACK).

Das Original SCHICKM.EXE hat SP = 0x1000, TLINK schreibt bei unserem Aufbau 0x80
(Groesse des _STACK-Segments aus C0). SS ist bereits gleich. Attic hat den Wert
vermutlich mit EXEMOD nachgetragen. Danach ist der MZ-Kopf identisch.
Aufruf: exemod_stack.py BLADEM.EXE [sp_hex]
"""
import sys, struct
p = sys.argv[1]; sp = int(sys.argv[2], 16) if len(sys.argv) > 2 else 0x1000
d = bytearray(open(p, 'rb').read())
old = struct.unpack_from('<H', d, 0x10)[0]
struct.pack_into('<H', d, 0x10, sp)
open(p, 'wb').write(d)
print(f'SP {old:#x} -> {sp:#x}')
