#!/usr/bin/env python3
import os as _swos; _SW = _swos.environ.get('SW_ROOT', '/root/sternenschweif')  # Bau-Paket
"""objorder.py OBJ JSON NAME - ordnet LEDATA/FIXUPP einer TASM-OBJ (mehrere Segmente) so um,
dass TLINK die Relocations genau in der vorgegebenen Reihenfolge schreibt.

Vorgabe (aus dsp.json, Eintrag NAME): order = [[segname, pos], ...], pos = Position des
Segmentworts relativ zum Segmentbeitrag dieser OBJ. Aufeinanderfolgende, absteigende Eintraege
im selben Segment bilden einen Block (eine LEDATA <=1024 Bytes + FIXUPP, Fixups absteigend) -
so wie BCC seine Bloecke schreibt. Alle uebrigen Bytes kommen vorher in Fuellbloecken."""
import sys, struct, json
obj, js, name = sys.argv[1:4]
if js == '-':
    # residentes Codemodul NAME=R##: Reihenfolge aus der MZ-Tabelle des Originals (modulrelativ)
    L = json.load(open(_SW + '/lnk_ranges.json'))
    st = L['start'][name]; en = st + L['mods'][name][1]
    en = min([en] + [v for v in L['start'].values() if v > st])
    e = open(_SW + '/orig/SCHWEIF.EXE', 'rb').read()
    nr, ro = struct.unpack_from('<H', e, 6)[0], struct.unpack_from('<H', e, 0x18)[0]
    order = []
    for i in range(nr):
        o, sg = struct.unpack_from('<HH', e, ro + 4 * i); x = sg * 16 + o
        if st <= x < en: order.append((None, x - st))
else:
    order = [tuple(x) for x in next(s for s in json.load(open(js))['stuecke'] if s['name'] == name)['order']]

def idx(b, q):
    v = b[q]; return (((v & 0x7f) << 8) | b[q + 1], q + 2) if v & 0x80 else (v, q + 1)
def enc(i): return bytes([i]) if i < 0x80 else bytes([0x80 | (i >> 8), i & 0xff])
def rec(t, body):
    r = bytes([t]) + struct.pack('<H', len(body) + 1) + body; return r + bytes([(-sum(r)) & 0xff])

d = open(obj, 'rb').read(); p = 0; recs = []
while p < len(d):
    t = d[p]; n = struct.unpack_from('<H', d, p + 1)[0]; recs.append((t, d[p:p + 3 + n])); p += 3 + n
lnames = ['']; segname = [None]
data = {}; fix = {}; last = None; first_le = None; before = []; after = []
for i, (t, r) in enumerate(recs):
    b = r[3:-1]
    if t == 0x96:
        q = 0
        while q < len(b): l = b[q]; lnames.append(b[q + 1:q + 1 + l].decode()); q += 1 + l
    elif t == 0x98:
        acbp = b[0]; q = 1
        if (acbp >> 5) == 0: q += 3
        q += 2
        ni, q = idx(b, q); segname.append(lnames[ni])
    if t == 0xa0:
        if first_le is None: first_le = i
        s, q = idx(b, 0); off = struct.unpack_from('<H', b, q)[0]; last = (s, off)
        for k, x in enumerate(b[q + 2:]): data.setdefault(s, {})[off + k] = x
    elif t == 0x9c:
        q = 0
        while q < len(b):
            if not b[q] & 0x80: raise SystemExit('THREAD-Fixup nicht unterstuetzt')
            lo = (b[q] << 8) | b[q + 1]; st = q; q += 2; fd = b[q]; q += 1
            F = (fd >> 4) & 7; P = (fd >> 2) & 1
            if not fd & 0x80 and F < 3: _, q = idx(b, q)
            if not fd & 8: _, q = idx(b, q)
            if not P: q += 2
            loc = last[1] + (lo & 0x3ff); typ = (lo >> 10) & 15
            w = 4 if typ == 3 else 2 if typ in (1, 2, 5) else 1
            fix.setdefault(last[0], []).append(dict(loc=loc, typ=typ, hib=lo & 0xfc00, rest=b[st + 2:q], w=w,
                                                    pos=loc + 2 if typ == 3 else loc))
    elif t == 0xa2:
        raise SystemExit('LIDATA nicht unterstuetzt')
    elif first_le is None: before.append(r)
    else: after.append(r)
sid = {segname[i]: i for i in range(1, len(segname))}
if order and order[0][0] is None:
    cs = [x for x in data if data[x]]
    if len(cs) != 1: raise SystemExit('Codemodul mit mehreren Datensegmenten')
    order = [(segname[cs[0]], p) for _, p in order]

# Bloecke aus der Vorgabe
runs = []
fpos = {(segname[x], f['pos']): f for x in fix for f in fix[x]}
def rng(sg, ps):
    fs = [fpos[(sg, x)] for x in ps]
    return min(f['loc'] for f in fs), max(f['loc'] + f['w'] for f in fs)
for sg, ps in order:
    if (sg, ps) not in fpos: raise SystemExit(f'{name}: keine Fixup fuer Relocation {sg} {ps:#x}')
    if runs and runs[-1][0] == sg and ps < runs[-1][1][-1]:
        a, e = rng(sg, runs[-1][1] + [ps])
        frei = all(e <= a2 or e2 <= a for s2, p2 in runs[:-1] if s2 == sg for a2, e2 in [rng(s2, p2)])
        if frei and e - a <= 1024:
            runs[-1][1].append(ps); continue
    runs.append((sg, [ps]))
used = set(); blocks = []
for sg, ps in runs:
    s = sid[sg]; fl = {f['pos']: f for f in fix.get(s, [])}
    fs = [fl[x] for x in ps]
    a = min(f['loc'] for f in fs); e = max(f['loc'] + f['w'] for f in fs)
    if e - a > 1024: raise SystemExit(f'Block zu gross {sg} {a:#x}-{e:#x}')
    for f in fs: used.add((s, f['loc']))
    blocks.append([s, a, e, fs])
# nicht gelistete Fixups innerhalb eines Blocks dort mitnehmen (hinten anhaengen)
for bl in blocks:
    s, a, e, fs = bl
    for f in fix.get(s, []):
        if (s, f['loc']) not in used and a <= f['loc'] and f['loc'] + f['w'] <= e:
            fs.append(f); used.add((s, f['loc']))
# Fuellbloecke fuer alles andere
fill = []
for s in sorted(data):
    cov = set()
    for bs, a, e, _ in blocks:
        if bs == s: cov.update(range(a, e))
    rest = sorted(o for o in data[s] if o not in cov)
    # zusammenhaengende Bereiche
    rng = []
    for o in rest:
        if rng and o == rng[-1][1]: rng[-1][1] = o + 1
        else: rng.append([o, o + 1])
    for a, e in rng:
        q = a
        while q < e:
            z = min(e, q + 1024)
            for f in fix.get(s, []):
                if f['loc'] < z < f['loc'] + f['w']: z = f['loc']
            fs = [f for f in fix.get(s, []) if (s, f['loc']) not in used and q <= f['loc'] < z]
            for f in fs: used.add((s, f['loc']))
            fill.append([s, q, z, fs]); q = z
missing = [(s, f['loc']) for s in fix for f in fix[s] if (s, f['loc']) not in used]
if missing: raise SystemExit(f'Fixups ohne Block: {missing[:5]}')
out = list(before)
for s, a, e, fs in fill + blocks:
    out.append(rec(0xa0, enc(s) + struct.pack('<H', a) + bytes(data[s][o] for o in range(a, e))))
    if fs:
        body = b''
        for f in fs:
            v = f['hib'] | (f['loc'] - a); body += bytes([v >> 8, v & 0xff]) + f['rest']
        out.append(rec(0x9c, body))
out += after
open(obj, 'wb').write(b''.join(out))
print(name, 'Bloecke', len(blocks), 'Fuell', len(fill))
