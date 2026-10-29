#!/usr/bin/env python3
import os as _swos; _SW = _swos.environ.get('SW_ROOT', '/root/sternenschweif')  # Bau-Paket
"""schweif.py - laedt eine Borland-VROOMM-EXE in Segmente (resident + Overlay) und findet Funktionen.
Ergebnis von load(): Liste von Segmenten {id, kind, para, code, entries, calls_far:[(off,(segid,off))]}
Funktionsanfaenge: Overlay-Einsprünge, Fernaufruf-Ziele, Nahaufruf-Ziele, Prologe nach Ruecksprung."""
import struct, subprocess, tempfile, os, re, collections

def load(path):
    d = open(path, 'rb').read()
    h = struct.unpack_from('<H', d, 8)[0] * 16
    l, p = struct.unpack_from('<HH', d, 2); e = (p - 1) * 512 + l if l else p * 512
    img = d[h:e]
    size, toff, cnt = struct.unpack_from('<III', d, e + 4)
    tab = [struct.unpack_from('<HHHH', img, toff - h + 8 * i) for i in range(cnt)]
    stubs = {}
    for para in range(0, len(img) - 0x25, 16):
        if img[para:para + 2] != b'\xcd\x3f': continue
        fo, cs, rs, ne = struct.unpack_from('<IHHH', img, para + 4)
        if not 0 < ne <= 2000: continue
        if all(img[para + 0x20 + 5 * i:para + 0x22 + 5 * i] == b'\xcd\x3f' and img[para + 0x24 + 5 * i] == 0 for i in range(ne)):
            stubs[para // 16] = (fo, cs, rs, [struct.unpack_from('<H', img, para + 0x22 + 5 * i)[0] for i in range(ne)])
    segs = []; byp = {}
    res = sorted({(x[0], x[1]) for x in tab if x[2] == 1 and x[0] not in stubs})
    for i, (sp, ln) in enumerate(res):
        s = dict(id=f'R{i:02d}', kind='R', para=sp, code=img[sp * 16:sp * 16 + ln], entries=[], fix=[]); segs.append(s); byp[sp] = s
    for i, sp in enumerate(sorted(stubs)):
        fo, cs, rs, ent = stubs[sp]; ov = d[e + 16 + fo:e + 16 + fo + cs + rs]
        fx = [struct.unpack_from('<H', ov, cs + 2 * j)[0] for j in range(rs // 2)]
        s = dict(id=f'O{i:03d}', kind='O', para=sp, code=ov[:cs], entries=ent, fix=fx); segs.append(s); byp[sp] = s
    # MZ-Relocs: Segmentwoerter im residenten Code
    nrel, relo = struct.unpack_from('<H', d, 6)[0], struct.unpack_from('<H', d, 0x18)[0]
    rel = [struct.unpack_from('<HH', d, relo + 4 * i) for i in range(nrel)]
    rpos = collections.defaultdict(list)
    for o, s in rel:
        a = s * 16 + o
        for r in res:
            if r[0] * 16 <= a < r[0] * 16 + r[1]: rpos[r[0]].append(a - r[0] * 16); break
    def target(segval, off):
        t = byp.get(segval)
        if t is None: return None
        if t['kind'] == 'O':
            if off >= 0x20 and (off - 0x20) % 5 == 0 and (off - 0x20) // 5 < len(t['entries']):
                return (t['id'], t['entries'][(off - 0x20) // 5])
            return None
        return (t['id'], off) if off < len(t['code']) else None
    for s in segs:
        c = s['code']; s['far'] = []
        locs = s['fix'] if s['kind'] == 'O' else rpos.get(s['para'], [])
        for o in locs:
            if o < 3 or o + 2 > len(c) or c[o - 3] != 0x9A: continue
            v = struct.unpack_from('<H', c, o)[0]; off = struct.unpack_from('<H', c, o - 2)[0]
            sv = tab[v // 8][0] if (s['kind'] == 'O' and v % 8 == 0 and v // 8 < cnt) else (v if s['kind'] == 'R' else None)
            if sv is None: continue
            t = target(sv, off)
            if t: s['far'].append((o - 3, t))
    return segs

def disasm(code):
    f = tempfile.NamedTemporaryFile(delete=False); f.write(code); f.close()
    out = subprocess.run(['ndisasm', '-b16', f.name], capture_output=True, text=True).stdout; os.unlink(f.name)
    ins = []
    for l in out.splitlines():
        m = re.match(r'([0-9A-F]{8})\s+([0-9A-F]+)\s+(.*)', l)
        if m: ins.append((int(m[1], 16), len(m[2]) // 2, m[3]))
        elif ins and l.strip().startswith('-'): pass
    return ins

def functions(segs):
    """liefert {segid: sorted [(start, quellen)]}"""
    starts = collections.defaultdict(lambda: collections.defaultdict(set))
    for s in segs:
        for o in s['entries']: starts[s['id']][o].add('E')
        for _, (t, o) in s['far']: starts[t][o].add('F')
    for s in segs:
        c = s['code']; ins = disasm(c); bound = {o for o, _, _ in ins}; s['ins'] = ins
        for o, n, t in ins:
            m = re.match(r'call (0x[0-9a-f]+)$', t)
            if m:
                x = int(m[1], 16)
                if x < len(c) and x in bound: starts[s['id']][x].add('N')
        prev = None
        for o, n, t in ins:
            if prev and prev.startswith(('ret', 'jmp')) and (c[o:o + 3] == b'\x55\x8b\xec' or (c[o] == 0xC8 and c[o + 3:o + 4] == b'\x00')):
                starts[s['id']][o].add('P')
            prev = t
        starts[s['id']][0].add('A')  # Segmentanfang
    return {k: sorted(v.items()) for k, v in starts.items()}
