#!/usr/bin/env python3
"""objfix.py - OMF-OBJ einlesen: Namen, Segmente, Gruppen, EXTDEF/PUBDEF und alle Fixups (auch THREADs).

fixups(path) -> Liste von dict(seg, loc, typ, M, ftgt=(art,name), tgt=(art,name), disp)
   art: 'SEG','GRP','EXT','FRAME','TARGET' ; loc = Offset im Segment (LEDATA-Basis + Lage)
Aufruf als Skript: objfix.py OBJ  -> Fixups ausgeben"""
import sys, struct

def idx(b, q):
    v = b[q]; return (((v & 0x7f) << 8) | b[q + 1], q + 2) if v & 0x80 else (v, q + 1)

def parse(path):
    d = open(path, 'rb').read(); p = 0
    ln = ['']; segs = ['']; grps = ['']; ext = ['']; pubs = []; fix = []
    last = None; fthr = [None] * 4; tthr = [None] * 4
    def ref(meth, i):
        if meth == 0: return ('SEG', segs[i])
        if meth == 1: return ('GRP', grps[i])
        if meth == 2: return ('EXT', ext[i])
        if meth == 4: return ('FRAME', 'loc')
        if meth == 5: return ('TARGET', '')
        return ('?', meth)
    while p < len(d):
        t = d[p]; n = struct.unpack_from('<H', d, p + 1)[0]; b = d[p + 3:p + 2 + n]; p += 3 + n
        if t == 0x96:
            q = 0
            while q < len(b): l = b[q]; ln.append(b[q + 1:q + 1 + l].decode('latin1')); q += 1 + l
        elif t == 0x98:
            a = b[0]; q = 1 + (3 if a >> 5 == 0 else 0) + 2; ni, q = idx(b, q); segs.append(ln[ni])
        elif t == 0x9a:
            ni, q = idx(b, 0); grps.append(ln[ni])
        elif t in (0x8c, 0xb4):
            q = 0
            while q < len(b): l = b[q]; ext.append(b[q + 1:q + 1 + l].decode('latin1')); q += 1 + l; _, q = idx(b, q)
        elif t in (0x90, 0x91):
            gi, q = idx(b, 0); si, q = idx(b, q)
            if si == 0: q += 2
            while q < len(b):
                l = b[q]; nm = b[q + 1:q + 1 + l].decode('latin1'); q += 1 + l
                off = struct.unpack_from('<H', b, q)[0]; q += 2; _, q = idx(b, q)
                pubs.append((segs[si] if si else '', off, nm))
        elif t in (0xa0, 0xa2):
            si, q = idx(b, 0); last = (segs[si], struct.unpack_from('<H', b, q)[0], t)
        elif t == 0x9c:
            q = 0
            while q < len(b):
                if not b[q] & 0x80:                     # THREAD
                    x = b[q]; q += 1; D = (x >> 6) & 1; meth = (x >> 2) & 7; th = x & 3
                    i = 0
                    if meth < 3: i, q = idx(b, q)
                    (fthr if D else tthr)[th] = (meth, i)
                    continue
                lo = (b[q] << 8) | b[q + 1]; q += 2; fd = b[q]; q += 1
                M = (lo >> 14) & 1; typ = (lo >> 10) & 15; loc = last[1] + (lo & 0x3ff)
                if fd & 0x80: fm, fi = fthr[(fd >> 4) & 3]
                else:
                    fm = (fd >> 4) & 7; fi = 0
                    if fm < 3: fi, q = idx(b, q)
                if fd & 8: tm, ti = tthr[fd & 3]; tm &= 3
                else:
                    tm = fd & 3; ti, q = idx(b, q)
                P = (fd >> 2) & 1
                disp = 0
                if not P: disp = struct.unpack_from('<H', b, q)[0]; q += 2
                fix.append(dict(seg=last[0], loc=loc, typ=typ, M=M, ftgt=ref(fm, fi), tgt=ref(tm, ti), disp=disp))
    return dict(segs=segs[1:], grps=grps[1:], ext=ext[1:], pubs=pubs, fix=fix)

if __name__ == '__main__':
    o = parse(sys.argv[1])
    for f in o['fix']:
        print(f"{f['seg']}:{f['loc']:04x} typ{f['typ']} M{f['M']} F={f['ftgt']} T={f['tgt']} +{f['disp']:#x}")
