#!/usr/bin/env python3
"""objsym.py - Symbolnamen in Borland-OBJ-Dateien (OMF) anzeigen und umbenennen.

  objsym.py list DATEI.OBJ            EXTDEF/PUBDEF-Namen auflisten
  objsym.py rename DATEI.OBJ ALT NEU   externen Namen ALT in NEU umbenennen

Wozu: Bright Eyes deklariert einige Funktionen beim Aufrufer absichtlich mit
anderem Prototyp als bei der Definition, damit der erzeugte Code bytegenau
stimmt. In C++ ergibt das andere (gemangelte) Namen, und TLINK findet das
Ziel nicht. Das Umbenennen in der OBJ aendert keinen einzigen Code-Byte,
nur den Namen, den der Linker aufloest.
"""
import sys, struct


def records(d):
    p = 0
    while p < len(d):
        typ = d[p]; ln = struct.unpack_from('<H', d, p + 1)[0]
        yield typ, d[p + 3:p + 3 + ln - 1]
        p += 3 + ln


def names_in(typ, body):
    out = []
    if typ in (0x8C,):  # EXTDEF
        q = 0
        while q < len(body):
            n = body[q]; out.append(body[q + 1:q + 1 + n].decode('latin1')); q += 1 + n
            t = body[q]; q += 1 if t < 0x80 else 2
    elif typ in (0x90, 0x91):  # PUBDEF
        q = 0
        gi = body[q]; q += 1 if gi < 0x80 else 2
        si = body[q]; q += 1 if si < 0x80 else 2
        if gi == 0 and si == 0:
            q += 2
        w = 2 if typ == 0x90 else 4
        while q < len(body):
            n = body[q]; out.append(body[q + 1:q + 1 + n].decode('latin1')); q += 1 + n + w
            t = body[q]; q += 1 if t < 0x80 else 2
    return out


def make(typ, body):
    rec = bytes([typ]) + struct.pack('<H', len(body) + 1) + body
    return rec + bytes([(-sum(rec)) & 0xFF])


def rename(path, old, new):
    d = open(path, 'rb').read()
    out = bytearray(); hit = 0
    for typ, body in records(d):
        if typ == 0x8C:
            nb = bytearray(); q = 0
            while q < len(body):
                n = body[q]; nm = body[q + 1:q + 1 + n]; q += 1 + n
                t0 = q; t = body[q]; q += 1 if t < 0x80 else 2
                if nm.decode('latin1') == old:
                    nm = new.encode('latin1'); hit += 1
                nb += bytes([len(nm)]) + nm + body[t0:q]
            body = bytes(nb)
        out += make(typ, body)
    open(path, 'wb').write(out)
    return hit


if __name__ == '__main__':
    if sys.argv[1] == 'list':
        d = open(sys.argv[2], 'rb').read()
        for typ, body in records(d):
            for n in names_in(typ, body):
                print('EXT' if typ == 0x8C else 'PUB', n)
    elif sys.argv[1] == 'rename':
        h = rename(sys.argv[2], sys.argv[3], sys.argv[4])
        print(f'{sys.argv[2]}: {h}x {sys.argv[3]} -> {sys.argv[4]}')
        if h == 0:
            sys.exit(1)


def autofix(objs):
    """Externe Namen ohne passende Definition auf die einzige Definition
    gleichen Grundnamens umbiegen (nur der Teil vor '$' zaehlt)."""
    pub = {}; ext = {}
    for o in objs:
        d = open(o, 'rb').read()
        for typ, body in records(d):
            for n in names_in(typ, body):
                (ext if typ == 0x8C else pub).setdefault(n, set()).add(o)
    base = {}
    for n in pub:
        base.setdefault(n.split('$')[0], []).append(n)
    done = []
    for n, os_ in sorted(ext.items()):
        if n in pub:
            continue
        cands = base.get(n.split('$')[0], [])
        if len(cands) == 1:
            for o in sorted(os_):
                rename(o, n, cands[0]); done.append((o, n, cands[0]))
    return done


if __name__ == '__main__' and sys.argv[1] == 'autofix':
    for o, a, b in autofix(sys.argv[2:]):
        print(f'{o}: {a} -> {b}')
