#!/usr/bin/env python3
"""objfixgen.py - macht aus allen fest eingetragenen Segmentwerten echte Verweise.

Bright Eyes schreibt an vielen Stellen Fern-Zeiger und Segmentwerte als feste
Zahlen hin (Datenblock in datseg.cpp, Segmentkonstanten in Code/Assembler).
Im Original waren das Verweise, die der Linker aufloest und als Relocation in
den EXE-Header schreibt. Dieses Werkzeug findet jede Relocation des Originals,
die TLINK beim Nachbau NICHT erzeugt, bestimmt Modul, Stelle und Ziel und traegt
in der OBJ-Datei einen passenden Fixup ein. Danach erzeugt TLINK alle
Relocations selbst (in seiner eigenen Reihenfolge, mit richtiger Headergroesse).

Ziele:
  Segment = DGROUP          -> DATASEG@ + Offset
  Segment = Stub-Segment    -> die Overlay-Funktion hinter dem Einsprung
  Segment = anderes Segment -> ein oeffentliches Symbol in diesem Segment (+ Abstand)

Aufruf: objfixgen.py ORIGINAL.EXE BLADEM.EXE BLADEM.MAP <OBJ-Verzeichnis>
        (BLADEM.EXE/MAP = Ergebnis eines Linklaufs OHNE diese Fixups)
"""
import sys, os, re, struct
from collections import defaultdict

orig, newexe, mapf, objdir = sys.argv[1:5]


def ld(f):
    d = open(f, 'rb').read()
    h = struct.unpack_from('<H', d, 8)[0] * 16
    n = struct.unpack_from('<H', d, 6)[0]
    lf = struct.unpack_from('<H', d, 0x18)[0]
    rel = {s * 16 + o for o, s in (struct.unpack_from('<HH', d, lf + 4 * i) for i in range(n))}
    return d[h:], rel


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


def enc_idx(i):
    return bytes([i]) if i < 0x80 else bytes([0x80 | (i >> 8), i & 0xFF])


def split_fixups(body):
    """FIXUPP-Inhalt in einzelne Subrecords zerlegen -> [(drec, bytes)] oder None bei THREADs"""
    q = 0; res = []
    while q < len(body):
        x = body[q]; st = q
        if not x & 0x80:
            return None
        locat = (x << 8) | body[q + 1]; q += 2
        fd = body[q]; q += 1
        F = fd & 0x80; fr = (fd >> 4) & 7; T = fd & 8; P = fd & 4
        if not F and fr < 3:
            _, q = idx(body, q)
        if not T:
            _, q = idx(body, q)
        if not P:
            q += 2
        res.append((locat & 0x3FF, body[st:q]))
    return res


def merge_desc(old, new):
    """wie Borland: Fixups eines Satzes absteigend nach Position"""
    a = split_fixups(old); b = split_fixups(new)
    if a is None:
        return old + new
    allf = sorted(a + b, key=lambda x: -x[0])
    return b''.join(x[1] for x in allf)


def objinfo(path):
    d = open(path, 'rb').read()
    lnames = ['']; segs = [None]; pubs = []; nmext = 0
    for t, b in records(d):
        if t == 0x96:
            q = 0
            while q < len(b):
                n = b[q]; lnames.append(b[q + 1:q + 1 + n].decode('latin1')); q += 1 + n
        elif t == 0x98:
            acbp = b[0]; q = 1 + (3 if (acbp >> 5) == 0 else 0) + 2
            ni, q = idx(b, q); ci, q = idx(b, q); segs.append((lnames[ni], lnames[ci]))
        elif t == 0x8C:
            q = 0
            while q < len(b):
                n = b[q]; q += 1 + n; _, q = idx(b, q); nmext += 1
        elif t == 0x90:
            q = 0; gi, q = idx(b, q); si, q = idx(b, q)
            if gi == 0 and si == 0:
                q += 2
            while q < len(b):
                n = b[q]; nm = b[q + 1:q + 1 + n].decode('latin1'); q += 1 + n
                o = struct.unpack_from('<H', b, q)[0]; q += 2; _, q = idx(b, q)
                pubs.append((si, segs[si][0] if segs[si] else None, o, nm))
    return segs, pubs, nmext


# --- MAP: Segmentbeitraege je Modul, Stub-Segmente
contrib = []      # (linear start, length, module, segname, class)
stubs = {}
for l in open(mapf, encoding='latin1'):
    m = re.match(r'\s*([0-9A-F]{4}):([0-9A-F]{4}) ([0-9A-F]{4}) C=(\S+)\s+S=(\S+)\s+G=\S+\s+M=(\S+)', l)
    if m:
        contrib.append((int(m[1], 16) * 16 + int(m[2], 16), int(m[3], 16), m[6].split('.')[0].upper(), m[5], m[4]))
    m = re.match(r'\s*([0-9A-F]{5})H ([0-9A-F]{5})H ([0-9A-F]{5})H (SEG\d+)_TEXT\s+STUBSEG', l)
    if m and int(m[3], 16) > 0:
        stubs[int(m[1], 16) // 16] = m[4]
DGROUP = [c for c in contrib if c[3] == '_DATA' and c[2] == 'C0'][0][0] // 16


def owner(lin):
    for s, ln, mod, sn, cl in contrib:
        if s <= lin < s + ln and cl not in ('STUBSEG', 'OVRINFO'):
            return s, mod, sn, cl
    return None


img, rel_o = ld(orig)
_, rel_n = ld(newexe)
info = {}


def oi(mod):
    if mod not in info:
        p = os.path.join(objdir, mod + '.OBJ')
        info[mod] = objinfo(p) if os.path.exists(p) else None
    return info[mod]


# Symbolsuche: Segment (Paragraph) -> (Modul, Symbol, Offset relativ zum Segmentanfang)
seg_syms = defaultdict(list)
for s, ln, mod, sn, cl in contrib:
    if cl in ('STUBSEG',) or not ln:
        continue
    inf = oi(mod)
    if not inf:
        continue
    for si, snm, o, nm in inf[1]:
        if snm == sn:
            seg_syms[s // 16].append((mod, nm, (s % 16) + o, s))

todo = defaultdict(list)   # mod -> [(segname, modrel_off, loc, target_name, disp)]
skipped = []
for L in sorted(rel_o - rel_n):
    ow = owner(L)
    if not ow:
        skipped.append((L, 'kein Modul')); continue
    cstart, mod, sn, cl = ow
    V = struct.unpack_from('<H', img, L)[0]
    pointer = (cl not in ('CODE',)) or (L >= 3 and img[L - 3] in (0x9A, 0xEA) and False)
    if cl == 'CODE' and img[L - 3] in (0x9A, 0xEA):
        pointer = True
    if pointer:
        fix_at, loc, off = L - 2, 3, struct.unpack_from('<H', img, L - 2)[0]
    else:
        fix_at, loc, off = L, 2, 0
    if V == DGROUP:
        tgt, disp = 'DATASEG@', off
    elif V in stubs:
        e = V * 16 + off
        foff = struct.unpack_from('<H', img, e + 2)[0]
        cand = [p for p in oi(stubs[V])[1] if p[2] == foff and p[1] and p[1].endswith('_TEXT')]
        if not cand:
            skipped.append((L, f'Stub {V:04X}:{off:04X} ohne Name')); continue
        tgt, disp = cand[0][3], 0
    else:
        cand = [c for c in seg_syms.get(V, []) if c[0] != mod] or seg_syms.get(V, [])
        if not cand:
            skipped.append((L, f'kein Symbol in Segment {V:04X}')); continue
        smod, nm, soff, _ = cand[0]
        tgt, disp = nm, (off - soff) & 0xFFFF
        if smod == mod:
            # Ziel im selben Modul: Verweis auf das eigene Segment (SEGDEF)
            ts = [c for c in contrib if c[0] // 16 == V and c[2] == mod]
            tsn = ts[0][3] if ts else None
            if not tsn:
                skipped.append((L, f'Segment {V:04X} in {mod} nicht gefunden')); continue
            seg_base = [c for c in contrib if c[2] == mod and c[3] == tsn][0][0]
            todo[mod].append((sn, fix_at - cstart, loc, ('SEG', tsn), (off - (seg_base - V * 16)) & 0xFFFF))
            continue
    todo[mod].append((sn, fix_at - cstart, loc, tgt, disp))

total = 0
for mod, items in sorted(todo.items()):
    path = os.path.join(objdir, mod + '.OBJ')
    recs = list(records(open(path, 'rb').read()))
    segs, pubs, nmext = objinfo(path)
    names = []
    for it in items:
        if not isinstance(it[3], tuple) and it[3] not in names:
            names.append(it[3])
    segidx = {sg[0]: k for k, sg in enumerate(segs) if sg}
    eidx = {nm: nmext + 1 + i for i, nm in enumerate(names)}
    want = defaultdict(dict)
    for sn, o, loc, tgt, disp in items:
        want[sn][o] = (loc, tgt, disp)
    # Ziel-Stellen, die ueber das Ende eines LEDATA-Satzes ragen: die Grenze verschieben.
    # Der ueberstehende Teil wandert in einen eigenen Satz zusammen mit dem Anfang
    # des Folgesatzes (beide Saetze duerfen keine eigenen Fixups haben).
    nrecs = []; k = 0
    while k < len(recs):
        t, b = recs[k]
        if t == 0xA0 and not (k + 1 < len(recs) and recs[k + 1][0] == 0x9C):
            si, q = idx(b, 0); base = struct.unpack_from('<H', b, q)[0]; q += 2
            data = b[q:]; w = want.get(segs[si][0], {}) if segs[si] else {}
            cut = [(o - base, 4 if loc == 3 else 2) for o, (loc, _, _) in w.items()
                   if 0 <= o - base < len(data) < o - base + (4 if loc == 3 else 2)]
            nx = recs[k + 1] if k + 1 < len(recs) else None
            if cut and nx and nx[0] == 0xA0 and not (k + 2 < len(recs) and recs[k + 2][0] == 0x9C):
                c, size = cut[0]
                si2, q2 = idx(nx[1], 0); base2 = struct.unpack_from('<H', nx[1], q2)[0]; q2 += 2
                data2 = nx[1][q2:]
                if si2 == si and base2 == base + len(data):
                    need = c + size - len(data)
                    hdr = b[:q - 2]
                    if c:
                        nrecs.append((0xA0, hdr + struct.pack('<H', base) + data[:c]))
                    nrecs.append((0xA0, hdr + struct.pack('<H', base + c) + data[c:] + data2[:need]))
                    if len(data2) > need:
                        nrecs.append((0xA0, hdr + struct.pack('<H', base2 + need) + data2[need:]))
                    k += 2; continue
        nrecs.append((t, b)); k += 1
    recs = nrecs
    out = bytearray(); ext_done = False; done = 0; i = 0
    while i < len(recs):
        t, b = recs[i]
        if not ext_done and t in (0x90, 0x91, 0xA0, 0xA1, 0x9C, 0x9D, 0xA2, 0xA3, 0xC2, 0xC3):
            body = bytearray()
            for nm in names:
                e = bytes([len(nm)]) + nm.encode('latin1') + b'\x00'
                if len(body) + len(e) > 1000:
                    out += make(0x8C, bytes(body)); body = bytearray()
                body += e
            if body:
                out += make(0x8C, bytes(body))
            ext_done = True
        if t == 0xA0:
            si, q = idx(b, 0)
            base = struct.unpack_from('<H', b, q)[0]; q += 2
            data = bytearray(b[q:]); subs = bytearray()
            w = want.get(segs[si][0], {}) if segs[si] else {}
            for o, (loc, tgt, disp) in sorted(w.items()):
                r = o - base
                size = 4 if loc == 3 else 2
                if 0 <= r and r + size <= len(data):
                    data[r:r + size] = b'\0' * size
                    locat = 0xC000 | (loc << 10) | r
                    if isinstance(tgt, tuple):
                        # F=0 Frame=5(Ziel), T=0, P=0, Methode 0 (SEGDEF) + Abstand
                        subs += struct.pack('>H', locat) + bytes([0x50]) + enc_idx(segidx[tgt[1]]) + struct.pack('<H', disp)
                    elif disp:
                        subs += struct.pack('>H', locat) + bytes([0x52]) + enc_idx(eidx[tgt]) + struct.pack('<H', disp)
                    else:
                        subs += struct.pack('>H', locat) + bytes([0x56]) + enc_idx(eidx[tgt])
                    done += 1
            out += make(0xA0, b[:q] + bytes(data))
            if subs:
                if i + 1 < len(recs) and recs[i + 1][0] == 0x9C:
                    merged = merge_desc(recs[i + 1][1], bytes(subs))
                    out += make(0x9C, merged); i += 2; continue
                out += make(0x9C, merge_desc(b'', bytes(subs)))
            i += 1; continue
        out += make(t, b); i += 1
    open(path, 'wb').write(out)
    total += done
    if done != len(items):
        print(f'  {mod}: {len(items) - done} Stellen nicht eingetragen (LEDATA-Grenze?)')
print(f'{total} Fixups in {len(todo)} Modulen ergaenzt, {len(skipped)} uebersprungen')
for s in skipped[:20]:
    print(f'  uebersprungen {s[0]:05X}: {s[1]}')
