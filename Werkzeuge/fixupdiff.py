#!/usr/bin/env python3
"""fixupdiff.py - erklaert abweichende Fixup-Ziele (falsche Aufrufe/Verweise).

Fuer ein Modul: jede Fixup-Stelle, deren gelinkter Wert vom Original abweicht.
Zeigt das Ziel im Nachbau (EXTDEF-Name) und rekonstruiert das Ziel im Original
ueber die MAP (Symbol an derselben Adresse).
Aufruf: fixupdiff.py BLADEM.EXE ORIGINAL.EXE BLADEM.MAP segmente.txt SEGnnn [...]
"""
import sys, re, struct
sys.path.insert(0, __file__.rsplit('/', 1)[0])
import omfcmp

exe_n, exe_o, mapf, segf = sys.argv[1:5]
a = open(exe_n, 'rb').read(); b = open(exe_o, 'rb').read()
ha = struct.unpack_from('<H', a, 8)[0] * 16
# Publics nach Wert (seg:off) aus der MAP
pubs = {}
inpub = False
for l in open(mapf, encoding='latin1'):
    if 'Publics by Value' in l:
        inpub = True; continue
    if inpub:
        m = re.match(r'\s*([0-9A-F]{4}):([0-9A-F]{4})\s+(?:idle\s+)?(.+?)\s*$', l)
        if m:
            pubs.setdefault((int(m[1], 16), int(m[2], 16)), m[3])
segpos = {}
for l in open(segf):
    m = re.match(r'(SEG\d+)\.OBJ\s+\S+\s+(\d+).*@0x([0-9a-f]+)', l)
    if m:
        segpos[m[1]] = int(m[3], 16)


def records(d):
    p = 0
    while p < len(d):
        t = d[p]; ln = struct.unpack_from('<H', d, p + 1)[0]
        yield t, d[p + 3:p + 3 + ln - 1]
        p += 3 + ln


def idx(b_, q):
    v = b_[q]
    return (((v & 0x7F) << 8) | b_[q + 1], q + 2) if v & 0x80 else (v, q + 1)


SIZE = {0: 1, 1: 2, 2: 2, 3: 4, 4: 1, 5: 2}
for mod in sys.argv[5:]:
    d = open(mod + '.OBJ', 'rb').read()
    exts = ['']; lnames = ['']; segs = [None]; last = None; fix = []
    for t, body in records(d):
        if t == 0x96:
            q = 0
            while q < len(body):
                n = body[q]; lnames.append(body[q + 1:q + 1 + n].decode('latin1')); q += 1 + n
        elif t == 0x98:
            acbp = body[0]; q = 1 + (3 if (acbp >> 5) == 0 else 0) + 2
            ni, q = idx(body, q); ci, q = idx(body, q); segs.append((lnames[ni], lnames[ci]))
        elif t == 0x8C:
            q = 0
            while q < len(body):
                n = body[q]; exts.append(body[q + 1:q + 1 + n].decode('latin1')); q += 1 + n; _, q = idx(body, q)
        elif t == 0xA0:
            si, q = idx(body, 0); last = (si, struct.unpack_from('<H', body, q)[0])
        elif t == 0x9C:
            q = 0
            while q < len(body):
                x = body[q]
                if x & 0x80:
                    locat = (x << 8) | body[q + 1]; q += 2
                    loc = (locat >> 10) & 0xF; drec = locat & 0x3FF
                    fd = body[q]; q += 1
                    F = fd & 0x80; fr = (fd >> 4) & 7; T = fd & 8; P = fd & 4; tm = fd & 3
                    if not F and fr < 3:
                        _, q = idx(body, q)
                    tgt = None
                    if not T:
                        ti, q = idx(body, q)
                        tgt = exts[ti] if (tm == 2 and ti < len(exts)) else (f'SEG#{ti}' if tm == 0 else f'?{tm}:{ti}')
                    if not P:
                        q += 2
                    if segs[last[0]] and segs[last[0]][1] == 'CODE':
                        fix.append((last[1] + drec, SIZE.get(loc, 2), loc, tgt))
                else:
                    D = x & 0x40; meth = (x >> 2) & 7; q += 1
                    if (D and meth < 3) or (not D and meth < 4):
                        _, q = idx(body, q)
    code = [c for c in omfcmp.parse_obj(mod + '.OBJ') if c[1] == 'CODE'][0]
    pn, _ = omfcmp.find(a, code[2], code[3]); po = segpos[mod]
    print(f'== {mod}')
    for off, sz, loc, tgt in fix:
        va = a[pn + off:pn + off + sz]; vb = b[po + off:po + off + sz]
        if va != vb:
            ow = struct.unpack_from('<H', b, po + off)[0]
            if loc == 3:
                o_off, o_seg_raw = struct.unpack_from('<HH', b, po + off)
                # Segment im Original ist unreloziert = relativ zum Image
                guess = pubs.get((o_seg_raw, o_off), '?')
                print(f'  +{off:04X} Zeiger  neu -> {tgt}   orig -> {o_seg_raw:04X}:{o_off:04X} {guess}')
            else:
                print(f'  +{off:04X} loc{loc}  neu -> {tgt} ({va.hex()})   orig Wert {vb.hex()} (Offset {ow:04X})')
