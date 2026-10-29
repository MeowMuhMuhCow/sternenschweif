#!/usr/bin/env python3
import os as _swos; _SW = _swos.environ.get('SW_ROOT', '/root/sternenschweif')  # Bau-Paket
"""cobjfix_d.py C_OBJ ASM_OBJ MAP OUT_OBJ - cobjfix.py mit Annahme eigener Daten (Stufe D1, dsana/PLAN.md).

Ablauf:
  1. tools/cobjfix.py (unveraendert) auf die C-OBJ -> Namen, Inline-Werte, Publics wie bisher.
     Hat die C-OBJ keine eigenen Daten, ist das schon das Ergebnis (bytegleich zu cobjfix.py).
  2. Eigene Daten (_DATA/_BSS/CONST ... in DGROUP mit Inhalt oder Laenge > 0):
     a) Lage im zentralen DS: jede Code-Fixup auf SEG <eigenes Segment> +v liegt in der ASM-Fassung an
        derselben Stelle als feste Zahl (bzw. als Fixup auf ein DGROUP-Public + Zahl) -> DS-Adresse A,
        delta = A - v. Gleiches delta -> ein Stueck; verschiedene delta (z. B. nur einzelne Literale eines
        Pools echt) -> Stuecke ab dem jeweils ersten verwiesenen v. Segment ohne Code-Verweis: Suche des
        Byte-Musters im Modulbereich aus dsana/ds_module.tsv (eindeutig, sonst Abbruch).
     b) Pruefung: Bytes jedes Stuecks = Original-DS (orig/SCHWEIF.EXE, DGROUP 0x24EB). Fixups innerhalb
        des eigenen _DATA: Offset = abgebildete Adresse, Segmentwort = DGROUP mit MZ-Relocation; Verweise
        auf Externe werden nur auf MZ-Relocation geprueft (Bytes maskiert, Meldung). _BSS: nur Lage.
     c) Jede Fixup auf das eigene Segment -> EXTDEF auf das DGROUP-Public an A (vorhandener EXTDEF-Name
        bevorzugt, sonst Regel wie cobjfix), notfalls naechstniedrigeres Public mit angepasstem Inline-Wert.
        Reine Segment-Fixups (typ 2) -> Name des ersten Stuecks.
     d) Eigene LEDATA/LIDATA und ihre FIXUPP entfallen, SEGDEF Laenge 0 und byteausgerichtet, eigene
        PUBDEF-Eintraege in diesen Segmenten entfallen (die zentralen Publics gelten). EXTDEFs, die nur noch
        aus dem entfernten _DATA verwiesen wurden, entfallen (Indizes werden neu nummeriert).
Ergebnis: dieselbe linkfertige OBJ, als haette das Modul die Daten per extern benutzt.

Als Modul: analyse(c_obj, asm_obj, mapf=None) -> dict (Stuecke, Fixups, Meldungen); wirft Fehler (D1Fehler).

Fix-Modus (names/inc/s_fix.h): zusaetzlich `--ref REF_OBJ` (dieselbe Quelle mit FIX_BUGS=0, bytegleich).
  Positionsabhaengige Schritte (Namen, Datenlage, Codeverweise auf eigene Daten, Publics) laufen an REF_OBJ,
  uebertragen wird ueber Namen (cobjfix.py --ref). Eigene Daten der Fix-Fassung muessen Byte fuer Byte und
  Fixup fuer Fixup gleich REF sein (eine Fehlerbehebung darf das Datensegment nicht veraendern).
"""
import sys, os, struct, re, bisect, subprocess, shutil
ROOT = _SW + ''
sys.path.insert(0, ROOT + '/tools'); import objfix, objbytes
DG = 0x24EB
EXE = ROOT + '/orig/SCHWEIF.EXE'
DSMOD = ROOT + '/dsana/ds_module.tsv'
DSOWN = ROOT + '/dsana/ds_owner.tsv'
W = {0: 1, 1: 2, 2: 2, 3: 4, 4: 1, 5: 2, 9: 2, 11: 4, 13: 4}


class D1Fehler(Exception):
    pass


def idx(b, q):
    v = b[q]; return (((v & 0x7f) << 8) | b[q + 1], q + 2) if v & 0x80 else (v, q + 1)


def enc_idx(i):
    return bytes([i]) if i < 0x80 else bytes([0x80 | (i >> 8), i & 0xff])


def rec(t, body):
    r = bytes([t]) + struct.pack('<H', len(body) + 1) + body; return r + bytes([(-sum(r)) & 0xff])


def records(d):
    p = 0
    while p < len(d):
        t = d[p]; n = struct.unpack_from('<H', d, p + 1)[0]; yield t, d[p + 3:p + 2 + n]; p += 3 + n


# ---------------------------------------------------------------- Original-DS
_exe = None
def exe():
    global _exe
    if _exe is None:
        e = open(EXE, 'rb').read(); hdr = struct.unpack_from('<H', e, 8)[0] * 16
        nrel, trel = struct.unpack_from('<H', e, 6)[0], struct.unpack_from('<H', e, 0x18)[0]
        rel = set()
        for k in range(nrel):
            o, s = struct.unpack_from('<HH', e, trel + 4 * k); rel.add(s * 16 + o)
        _exe = (e, hdr, rel)
    return _exe


def ds_bytes(a, n):
    e, hdr, _ = exe(); q = hdr + DG * 16 + a; return e[q:q + n]


def ds_reloc(a):
    return DG * 16 + a in exe()[2]


def ds_module(mod):
    for l in open(DSMOD, encoding='latin1').read().splitlines()[1:]:
        f = l.split('\t')
        if f[0] == mod:
            h = lambda x: int(x, 16) if x else None
            return dict(data=(h(f[2]), h(f[3])), bss=(h(f[6]), h(f[7])))
    return None


_own = None
def ds_owner(a):
    global _own
    if _own is None:
        _own = []
        for l in open(DSOWN, encoding='latin1').read().splitlines()[1:]:
            f = l.split('\t'); _own.append((int(f[0], 16), int(f[1], 16), f[2], f[4]))
    for s, e, m, sich in _own:
        if s <= a < e: return m, sich
    return None, None


def modname(cseg):
    m = re.match(r'(OVL|SEG)(\d+)_TEXT$', cseg)
    if not m: return None
    return ('O%03d' if m.group(1) == 'OVL' else 'R%02d') % int(m.group(2))


# ---------------------------------------------------------------- MAP
def mapnames(mapf):
    m = open(mapf, encoding='latin1').read()
    a, b = m.index('Publics by Name'), m.index('Publics by Value')
    alln = {}
    for l in m[a:b].splitlines()[2:]:
        f = l.split()
        if len(f) >= 2 and re.match(r'^[0-9A-F]{4}:[0-9A-F]{4}$', f[0]):
            sg, of = (int(x, 16) for x in f[0].split(':')); alln[f[-1]] = (sg, of)
    return alln


# ---------------------------------------------------------------- Analyse
def own_segments(path):
    """eigene Datensegmente: alle SEGDEFs ausser *_TEXT mit Laenge > 0 -> {name: laenge}"""
    out = {}
    ln = ['']
    for t, b in records(open(path, 'rb').read()):
        if t == 0x96:
            q = 0
            while q < len(b): l = b[q]; ln.append(b[q + 1:q + 1 + l].decode('latin1')); q += 1 + l
        elif t == 0x98:
            a = b[0]; q = 1 + (3 if a >> 5 == 0 else 0); L = struct.unpack_from('<H', b, q)[0]
            ni, _ = idx(b, q + 2); nm = ln[ni]
            if a & 2: L = 0x10000
            if not nm.endswith('_TEXT') and L: out[nm] = L
    return out


def analyse(cobj, aobj, mapf=None):
    own = own_segments(cobj)
    res = dict(own=own, pieces=[], fix=[], notes=[], checked=0, masked=0)
    if not own: return res
    C = objfix.parse(cobj); A = objfix.parse(aobj)
    sb = objbytes.seg_bytes(cobj)
    cseg = [s for s in C['segs'] if s.endswith('_TEXT')][0]
    mod = modname(cseg); res['mod'] = mod
    cb = sb.get(cseg, b''); ab = objbytes.seg_bytes(aobj)[cseg]
    am = {f['loc']: f for f in A['fix'] if f['seg'] == cseg}
    alln = mapnames(mapf) if mapf else {}
    rng = ds_module(mod) if mod else None
    # Fixups auf eigene Segmente sammeln
    refs = {s: [] for s in own}           # seg -> [(v, delta|None, fixup)]
    for f in C['fix']:
        tk, tn = f['tgt']
        if tk == 'GRP' and tn == 'DGROUP' and f['typ'] not in (2,) and f['seg'] == cseg:
            raise D1Fehler(f"{cseg}:{f['loc']:#x}: Offset-Fixup auf GRP DGROUP - mit eigenen Daten nicht eindeutig")
        if tk != 'SEG' or tn not in own: continue
        if f['seg'] in own:               # Verweis aus eigenen Daten (Tabelle) -> nur pruefen
            continue
        if f['seg'] != cseg: raise D1Fehler(f"Fixup auf {tn} aus Segment {f['seg']} nicht unterstuetzt")
        if f['typ'] == 2:
            refs[tn].append((None, None, f)); continue
        if f['typ'] not in (1, 3, 5, 9): raise D1Fehler(f"{cseg}:{f['loc']:#x}: Fixup-Typ {f['typ']} auf {tn}")
        v = (struct.unpack_from('<H', cb, f['loc'])[0] + f['disp']) & 0xffff
        w = struct.unpack_from('<H', ab, f['loc'])[0]
        g = am.get(f['loc'])
        if g is None: ad = w
        elif g['tgt'][0] == 'EXT':
            n = g['tgt'][1]
            if n not in alln or alln[n][0] != DG:
                raise D1Fehler(f"{cseg}:{f['loc']:#x}: ASM-Fixup auf {n} ist kein DGROUP-Public")
            ad = (alln[n][1] + w + g['disp']) & 0xffff
        elif g['tgt'] in (('GRP', 'DGROUP'), ('SEG', '_DATA')):
            ad = (w + g['disp']) & 0xffff
        else:
            raise D1Fehler(f"{cseg}:{f['loc']:#x}: ASM-Fixup {g['tgt']} - Adresse nicht bestimmbar")
        refs[tn].append((v, (ad - v) & 0xffff, f))
    # Stuecke bilden
    for s, L in own.items():
        data = sb.get(s, b'')
        isbss = not data
        rs = sorted((v, d) for v, d, _ in refs[s] if v is not None)
        cuts = []
        for v, d in rs:
            if v > L: raise D1Fehler(f'{s}+{v:#x} hinter Segmentende {L:#x}')
            if cuts and cuts[-1][1] == d: continue
            if cuts and cuts[-1][0] == v: raise D1Fehler(f'{s}+{v:#x}: widerspruechliche Adressen')
            cuts.append((v, d))
        if not cuts:
            if isbss: raise D1Fehler(f'{s}: kein Codeverweis, Lage nicht bestimmbar')
            # Mustersuche im Modulbereich
            if not rng or rng['data'][0] is None: raise D1Fehler(f'{s}: kein Codeverweis und kein dsana-Modulbereich fuer {mod}')
            lo, hi = rng['data']; mask = mask_of(C, s)
            hits = [a for a in range(lo, hi - L + 1) if match(ds_bytes(a, L), data, mask)]
            if len(hits) != 1: raise D1Fehler(f'{s}: Muster im Modulbereich {lo:#x}..{hi:#x} {len(hits)}-mal gefunden')
            cuts = [(0, (hits[0]) & 0xffff)]
            res['notes'].append(f'{s}: Lage per Mustersuche {hits[0]:#x}')
        for k, (v, d) in enumerate(cuts):
            st = 0 if k == 0 else v; en = cuts[k + 1][0] if k + 1 < len(cuts) else L
            res['pieces'].append(dict(seg=s, von=st, bis=en, delta=d, bss=isbss))
    # Pruefung gegen das Original-DS
    mask_ext = {}
    for f in C['fix']:
        if f['seg'] in own: mask_ext.setdefault(f['seg'], []).append(f)
    def mapaddr(s, v):
        for p in res['pieces']:
            if p['seg'] == s and (p['von'] <= v < p['bis'] or v == p['bis'] == own[s]):
                return (v + p['delta']) & 0xffff
        raise D1Fehler(f'{s}+{v:#x}: keinem Stueck zugeordnet')
    for p in res['pieces']:
        s = p['seg']; a0 = (p['von'] + p['delta']) & 0xffff; n = p['bis'] - p['von']
        p['adr'] = a0
        if p['bss']:
            if a0 < 0x86b6: raise D1Fehler(f'{s}: BSS-Stueck bei {a0:#x} liegt im initialisierten DS')
            continue
        own_b = bytearray(sb[s][p['von']:p['bis']].ljust(n, b'\0')); orig = ds_bytes(a0, n)
        skip = set()
        for f in mask_ext.get(s, []):
            if not (p['von'] <= f['loc'] < p['bis']): continue
            o = f['loc'] - p['von']; tk, tn = f['tgt']
            if f['typ'] not in (1, 3, 5, 9, 2): raise D1Fehler(f"{s}:{f['loc']:#x}: Fixup-Typ {f['typ']} in eigenen Daten")
            if f['typ'] in (1, 3, 5, 9) and tk == 'SEG' and tn in own:
                v = (struct.unpack_from('<H', own_b, o)[0] + f['disp']) & 0xffff
                struct.pack_into('<H', own_b, o, mapaddr(tn, v))
            elif f['typ'] in (1, 3, 5, 9):
                skip.update(range(o, o + 2)); res['masked'] += 1
                res['notes'].append(f"{s}:{f['loc']:#x} Verweis auf {tn}: Offset maskiert")
            so = o + 2 if f['typ'] == 3 else (o if f['typ'] == 2 else None)
            if so is not None:
                if not ds_reloc(a0 + so): raise D1Fehler(f'{s}:{f["loc"]:#x}: keine MZ-Relocation bei ds:{a0 + so:#x}')
                if tk == 'SEG' and tn in own or f['tgt'] == ('GRP', 'DGROUP'):
                    struct.pack_into('<H', own_b, so, DG)
                else: skip.update(range(so, so + 2))
        bad = [i for i in range(n) if i not in skip and own_b[i] != orig[i]]
        if bad:
            i = bad[0]
            raise D1Fehler(f'{s}+{p["von"] + i:#x} (ds:{a0 + i:#x}): eigene Daten {bytes(own_b[i:i+8]).hex()} '
                           f'!= Original {orig[i:i+8].hex()} ({len(bad)} Bytes anders)')
        res['checked'] += n - len(skip)
    # Lage gegen dsana
    for p in res['pieces']:
        a0, a1 = p['adr'], p['adr'] + p['bis'] - p['von']
        r = rng and (rng['bss'] if p['bss'] else rng['data'])
        if r and r[0] is not None and r[0] <= a0 and a1 <= r[1]: p['dsana'] = 'im Modulbereich'
        else:
            ow = []
            for a in range(a0, max(a1, a0 + 1)):
                o = '%s/%s' % ds_owner(a)
                if not ow or ow[-1][1] != o: ow.append((a, o))
            txt = ', '.join(f'{a:#x} {o}' for a, o in ow)
            r_txt = f'{r[0]:#x}..{r[1]:#x}' if r and r[0] is not None else 'keiner'
            p['dsana'] = f'ausserhalb dsana-Bereich {mod} ({r_txt}); ds_owner: {txt}'
            res['notes'].append(f"HINWEIS dsana: ds:{a0:#x}..{a1:#x} gehoert nach Codebeleg zu {mod}")
    # abgebildete Fixups
    for s in own:
        for v, d, f in refs[s]:
            res['fix'].append(dict(f=f, seg=s, v=v, adr=None if v is None else (v + d) & 0xffff))
    return res


def mask_of(C, s):
    m = set()
    for f in C['fix']:
        if f['seg'] == s: m.update(range(f['loc'], f['loc'] + W.get(f['typ'], 2)))
    return m


def match(orig, data, mask):
    return all(i in mask or orig[i] == data[i] for i in range(len(data)))


# ---------------------------------------------------------------- Umschreiben
def rewrite(fixed, res, alln, out):
    d = open(fixed, 'rb').read()
    # Struktur der fertigen OBJ
    ln = ['']; segs = ['']; exts = ['']
    for t, b in records(d):
        if t == 0x96:
            q = 0
            while q < len(b): l = b[q]; ln.append(b[q + 1:q + 1 + l].decode('latin1')); q += 1 + l
        elif t == 0x98:
            a = b[0]; q = 1 + (3 if a >> 5 == 0 else 0) + 2; ni, _ = idx(b, q); segs.append(ln[ni])
        elif t in (0x8c, 0xb4):
            q = 0
            while q < len(b): l = b[q]; exts.append(b[q + 1:q + 1 + l].decode('latin1')); q += 1 + l; _, q = idx(b, q)
    own = set(res['own']); ownidx = {i for i, s in enumerate(segs) if s in own}
    dpub = sorted((of, n) for n, (sg, of) in alln.items() if sg == DG); dkeys = [x[0] for x in dpub]
    extset = {n: i for i, n in enumerate(exts) if i}
    newext = []
    def extfor(ad):
        """(Name, Basisadresse) fuer DS-Adresse ad"""
        exact = [n for o, n in dpub if o == ad]
        if exact:
            for n in exact:
                if n in extset: return n, ad
            return min(exact), ad                       # cobjfix: daddr.setdefault ueber sortierte Liste
        k = bisect.bisect_right(dkeys, ad) - 1
        if k < 0: raise D1Fehler(f'ds:{ad:#x}: kein Public darunter')
        base = dkeys[k]; cand = [n for o, n in dpub if o == base]
        for n in cand:
            if n in extset: return n, base
        return dpub[k][1], base                         # cobjfix: naechstniedrigeres = letzter Eintrag
    def ext_index(n):
        if n not in extset:
            newext.append(n); extset[n] = len(exts) + len(newext) - 1
        return extset[n]
    # Ziel je Fixup (Segment, Stelle)
    plan = {}
    first = {}
    for x in res['fix']:
        if x['adr'] is not None and x['seg'] not in first: first[x['seg']] = x['adr']
    for x in res['fix']:
        f = x['f']; ad = x['adr']
        if ad is None:
            ad = first.get(x['seg'], dpub[0][0]); n, base = extfor(ad); plan[(f['seg'], f['loc'])] = (ext_index(n), None)
            continue
        n, base = extfor(ad)
        plan[(f['seg'], f['loc'])] = (ext_index(n), (ad - base) & 0xffff)
        if ad != base: res['notes'].append(f"{x['seg']}+{x['v']:#x} -> {n}+{ad - base:#x}")
    # EXT-Verweise ausserhalb eigener Daten zaehlen (fuer das Entfernen verwaister EXTDEFs)
    used = set(); used_own = set()
    last = None
    for t, b in records(d):
        if t in (0xa0, 0xa2):
            si, _ = idx(b, 0); last = si
        elif t == 0x9c:
            for sub in parse_fixupp(b):
                if sub['kind'] != 'fix': continue
                for meth, i in ((sub['fm'], sub['fi']), (sub['tm'], sub['ti'])):
                    if meth == 2 and i is not None: (used_own if last in ownidx else used).add(i)
                if sub['thread_t'] or sub['thread_f']:
                    raise D1Fehler('FIXUPP mit THREAD-Bezug - nicht unterstuetzt')
        elif t in (0x8c, 0xb4, 0x90, 0x91, 0x98, 0x96, 0x9a, 0x88, 0x80, 0x8a, 0x8b):
            pass
    drop_ext = sorted(used_own - used)
    for i in drop_ext: res['notes'].append(f'EXTDEF {exts[i]} entfaellt (nur aus eigenem _DATA verwiesen)')
    # Neunummerierung
    remap = {}; k = 0
    for i in range(1, len(exts)):
        if i in drop_ext: continue
        k += 1; remap[i] = k
    for j, n in enumerate(newext): remap[len(exts) + j] = k + 1 + j
    # Ausgabe
    outr = []; last = None; last_led = None; ext_done = False; seen_ext = False
    recs = list(records(d))
    last_ext_pos = max([i for i, (t, _) in enumerate(recs) if t in (0x8c, 0xb4)], default=None)
    grp_pos = max([i for i, (t, _) in enumerate(recs) if t in (0x9a, 0x98)], default=0)
    ins_after = last_ext_pos if last_ext_pos is not None else grp_pos
    segi = 0
    for ri, (t, body) in enumerate(recs):
        r = rec(t, body)
        if t == 0x98:
            segi += 1
            if segi in ownidx:
                a = body[0]; q = 1 + (3 if a >> 5 == 0 else 0)
                nb = bytearray(body); nb[0] = (a & 0x1d) | 0x20; struct.pack_into('<H', nb, q, 0)
                r = rec(t, bytes(nb))
        elif t in (0x8c, 0xb4):
            q = 0; nb = b''
            # Index des ersten Eintrags dieses Records
            i = 1 + sum(count_ext(bb) for tt, bb in recs[:ri] if tt in (0x8c, 0xb4))
            while q < len(body):
                l = body[q]; q2 = q + 1 + l; _, q3 = idx(body, q2)
                if i not in drop_ext: nb += body[q:q3]
                q = q3; i += 1
            r = rec(t, nb) if nb else None
        elif t in (0x90, 0x91):
            gi, q = idx(body, 0); si, q = idx(body, q)
            if si in ownidx:
                res['notes'].append(f'PUBDEF in {segs[si]} entfaellt'); r = None
        elif t in (0xa0, 0xa2):
            si, _ = idx(body, 0); last = si
            if si in ownidx: r = None
            else: last_led = len(outr)
        elif t == 0x9c:
            if last in ownidx: r = None
            else:
                subs = parse_fixupp(body); nb = b''
                led = bytearray(outr[last_led][3:-1]); lt = outr[last_led][0]
                si, q0 = idx(led, 0); base_off = struct.unpack_from('<H', led, q0)[0]; dq = q0 + 2
                for sub in subs:
                    if sub['kind'] != 'fix': nb += sub['raw']; continue
                    key = (segs[last], base_off + sub['off'])
                    fm, fi, tm, ti, P, disp = sub['fm'], sub['fi'], sub['tm'], sub['ti'], sub['P'], sub['disp']
                    if key in plan:
                        ti_new, val = plan[key]; tm = 2; ti = ti_new
                        if val is not None:
                            if P: struct.pack_into('<H', led, dq + sub['off'], val)
                            else: disp = (val - struct.unpack_from('<H', led, dq + sub['off'])[0]) & 0xffff
                    if fm == 2: fi = remap[fi]
                    if tm == 2 and ti is not None: ti = remap[ti]
                    nb += enc_fix(sub, fm, fi, tm, ti, P, disp)
                outr[last_led] = rec(lt, bytes(led))
                r = rec(t, nb)
        if r is not None: outr.append(r)
        if ri == ins_after and newext:
            eb = b''.join(bytes([len(n)]) + n.encode('latin1') + b'\0' for n in newext)
            outr.append(rec(0x8c, eb))
    open(out, 'wb').write(b''.join(outr))
    return newext, drop_ext


def count_ext(b):
    q = 0; n = 0
    while q < len(b): l = b[q]; q += 1 + l; _, q = idx(b, q); n += 1
    return n


def parse_fixupp(b):
    out = []; q = 0
    while q < len(b):
        q0 = q
        if not b[q] & 0x80:
            x = b[q]; q += 1; meth = (x >> 2) & 7
            if meth < 3: _, q = idx(b, q)
            out.append(dict(kind='thread', raw=b[q0:q])); continue
        lo = (b[q] << 8) | b[q + 1]; q += 2; fd = b[q]; q += 1
        tf = bool(fd & 0x80); tt = bool(fd & 8)
        fm = (fd >> 4) & 7; fi = None
        if not tf and fm < 3: fi, q = idx(b, q)
        tm = fd & 3; ti = None; P = (fd >> 2) & 1
        if not tt: ti, q = idx(b, q)
        disp = 0
        if not P: disp = struct.unpack_from('<H', b, q)[0]; q += 2
        out.append(dict(kind='fix', raw=b[q0:q], lo=lo, off=lo & 0x3ff, fd=fd, fm=fm, fi=fi, tm=tm, ti=ti, P=P,
                        disp=disp, thread_f=tf, thread_t=tt))
    return out


def enc_fix(sub, fm, fi, tm, ti, P, disp):
    fd = (sub['fd'] & 0x80) | (fm << 4 if not sub['thread_f'] else sub['fd'] & 0x70) | (P << 2) | (tm & 3)
    r = bytes([sub['lo'] >> 8, sub['lo'] & 0xff, fd])
    if not sub['thread_f'] and fm < 3: r += enc_idx(fi)
    r += enc_idx(ti)
    if not P: r += struct.pack('<H', disp)
    return r


# ---------------------------------------------------------------- Phase B: eigene Daten behalten (--eigen)
def strip_data_pubs(cobj, out):
    """PUBDEF-Eintraege ausserhalb *_TEXT entfernen (cobjfix kennt nur Code-Publics) -> [(seg, off, name)]"""
    d = open(cobj, 'rb').read(); ln = ['']; segs = ['']; outr = []; removed = []
    for t, b in records(d):
        if t == 0x96:
            q = 0
            while q < len(b): l = b[q]; ln.append(b[q + 1:q + 1 + l].decode('latin1')); q += 1 + l
        elif t == 0x98:
            a = b[0]; q = 1 + (3 if a >> 5 == 0 else 0) + 2; ni, _ = idx(b, q); segs.append(ln[ni])
        if t in (0x90, 0x91):
            gi, q = idx(b, 0); si, q = idx(b, q)
            if si and not segs[si].endswith('_TEXT'):
                while q < len(b):
                    l = b[q]; nm = b[q + 1:q + 1 + l].decode('latin1'); q += 1 + l
                    off = struct.unpack_from('<H', b, q)[0]; q += 2; _, q = idx(b, q)
                    removed.append((segs[si], off, nm))
                continue
        outr.append(rec(t, b))
    open(out, 'wb').write(b''.join(outr))
    return removed


def eigen_check(cobj, aobj, mapf):
    """Prueft eine (von cobjfix.py benannte) C-OBJ mit eigenen Daten gegen den Modulbereich (dsinfo.own):
    Codeverweise -> Basis + v, Bytes = Original-DS, Laenge = Bereich (ggf. ohne Fuellbyte).
    -> dict(mod, segs={seg: dict(kind, base, len)}, extren={ext: name}, notes)"""
    import dsinfo, items_ds
    own = own_segments(cobj)
    C = objfix.parse(cobj); A = objfix.parse(aobj)
    sb = objbytes.seg_bytes(cobj)
    cseg = [s for s in C['segs'] if s.endswith('_TEXT')][0]
    mod = modname(cseg)
    exp = dsinfo.own(mod)
    res = dict(mod=mod, segs={}, extren={}, notes=[], checked=0)
    KIND = {'_DATA': 'DATA', '_BSS': 'BSS'}
    for s in own:
        if s in ('_INIT_', '_EXIT_'):
            res['segs'][s] = dict(kind=s, base=None, len=own[s]); continue
        if s not in KIND: raise D1Fehler(f'eigenes Segment {s} wird (noch) nicht unterstuetzt')
        k = KIND[s]; e = exp.get(k)
        if e is None: raise D1Fehler(f'{mod} hat laut dsana kein {k} (C-OBJ: {s} {own[s]} Bytes)')
        L = own[s]; a, z = e
        if not (a + L == z or (a + L + 1 == z and (a + L) % 2 == 1)):
            raise D1Fehler(f'{s}: {L} Bytes ab ds:{a:#x} enden bei {a + L:#x}, Modulbereich endet bei {z:#x} '
                           f'(Rest {z - a - L:+d} Bytes -> dsana/dm_rest.tsv oder Quelltext)')
        res['segs'][s] = dict(kind=k, base=a, len=L)
    initexit_check(mod, cseg, C, sb, res)
    for k in ('DATA', 'BSS'):
        if exp.get(k) and not any(v['kind'] == k for v in res['segs'].values()):
            raise D1Fehler(f'{mod}: Modulbereich {k} {exp[k][0]:#x}..{exp[k][1]:#x} wird von der C-OBJ nicht geliefert')
    alln = mapnames(mapf)
    cb = sb.get(cseg, b''); ab = objbytes.seg_bytes(aobj)[cseg]
    am = {f['loc']: f for f in A['fix'] if f['seg'] == cseg}
    # 1. Codeverweise auf eigene Segmente: Adresse laut ASM-Fassung = Basis + v
    for f in C['fix']:
        tk, tn = f['tgt']
        if f['seg'] != cseg: continue
        if tk == 'GRP' and tn == 'DGROUP' and f['typ'] != 2:
            raise D1Fehler(f"{cseg}:{f['loc']:#x}: Offset-Fixup auf GRP DGROUP - mit eigenen Daten nicht eindeutig")
        if tk != 'SEG' or tn not in res['segs'] or f['typ'] == 2: continue
        if f['typ'] not in (1, 3, 5, 9): raise D1Fehler(f"{cseg}:{f['loc']:#x}: Fixup-Typ {f['typ']} auf {tn}")
        v = (struct.unpack_from('<H', cb, f['loc'])[0] + f['disp']) & 0xffff
        w = struct.unpack_from('<H', ab, f['loc'])[0]; g = am.get(f['loc'])
        if g is None: ad = w
        elif g['tgt'][0] == 'EXT':
            n = g['tgt'][1]
            if n not in alln or alln[n][0] != DG: raise D1Fehler(f"{cseg}:{f['loc']:#x}: ASM-Fixup auf {n} ist kein DGROUP-Public")
            ad = (alln[n][1] + w + g['disp']) & 0xffff
        elif g['tgt'] in (('GRP', 'DGROUP'), ('SEG', '_DATA')): ad = (w + g['disp']) & 0xffff
        else: raise D1Fehler(f"{cseg}:{f['loc']:#x}: ASM-Fixup {g['tgt']} - Adresse nicht bestimmbar")
        want = (res['segs'][tn]['base'] + v) & 0xffff
        if ad != want:
            raise D1Fehler(f"{cseg}:{f['loc']:#x}: {tn}+{v:#x} liegt bei ds:{want:#x}, ASM-Fassung verweist auf ds:{ad:#x}")
    # 2. Bytes der eigenen Daten
    cptr = items_ds.cptr(); dptr = items_ds.dptr()
    for s, info in res['segs'].items():
        if info['kind'] in ('_INIT_', '_EXIT_'): continue
        if info['kind'] == 'BSS':
            if s in sb and any(sb[s]): raise D1Fehler(f'{s}: BSS mit Inhalt')
            continue
        a0, n = info['base'], info['len']
        own_b = bytearray(sb.get(s, b'').ljust(n, b'\0')); orig = ds_bytes(a0, n); skip = set()
        for f in C['fix']:
            if f['seg'] != s: continue
            o = f['loc']; tk, tn = f['tgt']
            if f['typ'] not in (1, 2, 3, 5, 9): raise D1Fehler(f"{s}:{o:#x}: Fixup-Typ {f['typ']} in eigenen Daten")
            if f['typ'] in (1, 3, 5, 9):
                if tk == 'SEG' and tn in res['segs']:
                    v = (struct.unpack_from('<H', own_b, o)[0] + f['disp']) & 0xffff
                    struct.pack_into('<H', own_b, o, (res['segs'][tn]['base'] + v) & 0xffff)
                elif tk == 'EXT' and f['typ'] == 3 and (a0 + o) in dptr:
                    # Fernzeiger auf fremde DGROUP-Daten: Ziel = Original-Offset; Name ggf. auf das MAP-Public dort umbenennen
                    inl = (struct.unpack_from('<H', own_b, o)[0] + f['disp']) & 0xffff
                    ziel = (dptr[a0 + o] - inl) & 0xffff
                    if not (tn in alln and alln[tn] == (DG, ziel)):
                        cand = sorted(n_ for n_, v_ in alln.items() if v_ == (DG, ziel))
                        if not cand: raise D1Fehler(f'{s}:{o:#x}: Fernzeiger auf {tn}+{inl:#x}, ds:{ziel:#x} hat kein Public')
                        want = cand[0]
                        if res['extren'].get(tn, want) != want: raise D1Fehler(f'{tn}: widerspruechlich {res["extren"][tn]} / {want}')
                        res['extren'][tn] = want
                    struct.pack_into('<H', own_b, o, dptr[a0 + o]); struct.pack_into('<H', own_b, o + 2, DG)
                elif tk == 'SEG' and tn == cseg and f['typ'] == 3:
                    # Fernzeiger auf eigenen Code (BCC: Funktions-/Methodenzeiger in Tabellen, Ziel = Codesegment + Versatz).
                    # Das Original-DS muss dort einen Codezeiger auf genau diese Funktion haben; die Fixup bleibt
                    # segmentrelativ (TLINK setzt bei Overlays wie im Original den Einsprung des Stubs ein).
                    want = cptr.get(a0 + o); v = (struct.unpack_from('<H', own_b, o)[0] + f['disp']) & 0xffff
                    mm = re.match(r'F_([OR])(\d+)_([0-9A-F]{4})$', want or '')
                    if not mm or modname(cseg) != (mm.group(1) + (mm.group(2) if mm.group(1) == 'O' else '%02d' % int(mm.group(2)))) \
                            or int(mm.group(3), 16) != v:
                        raise D1Fehler(f'{s}:{o:#x}: Codezeiger {cseg}+{v:#x}, im Original-DS steht dort {want}')
                    skip.update(range(o, o + 2))
                elif tk == 'EXT' and f['typ'] == 3:
                    want = cptr.get(a0 + o)
                    if want is None: raise D1Fehler(f'{s}:{o:#x}: Fernzeiger auf {tn}, im Original-DS steht dort kein Codezeiger')
                    if tn != want:
                        if res['extren'].get(tn, want) != want: raise D1Fehler(f'{tn}: widerspruechlich {res["extren"][tn]} / {want}')
                        res['extren'][tn] = want
                    skip.update(range(o, o + 2))
                else:
                    raise D1Fehler(f"{s}:{o:#x}: Verweis auf {f['tgt']} (Typ {f['typ']}) in eigenen Daten nicht unterstuetzt")
            so = o + 2 if f['typ'] == 3 else (o if f['typ'] == 2 else None)
            if so is not None:
                if not ds_reloc(a0 + so): raise D1Fehler(f'{s}:{o:#x}: keine MZ-Relocation bei ds:{a0 + so:#x}')
                if (tk == 'SEG' and tn in res['segs']) or f['tgt'] == ('GRP', 'DGROUP') or (tk == 'EXT' and f['typ'] == 3 and (a0 + o) in dptr):
                    struct.pack_into('<H', own_b, so, DG)
                else: skip.update(range(so, so + 2))
        bad = [i for i in range(n) if i not in skip and own_b[i] != orig[i]]
        if bad:
            i = bad[0]
            raise D1Fehler(f'{s}+{i:#x} (ds:{a0 + i:#x}): eigene Daten {bytes(own_b[i:i+8]).hex()} != Original '
                           f'{orig[i:i+8].hex()} ({len(bad)} Bytes anders)')
        res['checked'] += n - len(skip)
        info['zeiger'] = len([f for f in C['fix'] if f['seg'] == s])
    return res


def initexit_check(mod, cseg, C, sb, res):
    """_INIT_/_EXIT_ der C-OBJ (statischer Initialisierer eines globalen C++-Objekts bzw. #pragma startup/exit):
    je genau ein Eintrag {1 = fern, Prioritaet, Fernzeiger} wie im Original (Prioritaet aus dem Original-DS,
    Ziel = eigener Code am Versatz des Originaleinsprungs dsinfo.INIT_FN/EXIT_FN)"""
    import dsinfo
    for s, fn, base, idx in (('_INIT_', dsinfo.INIT_FN, dsinfo.INIT_BASE, dsinfo.INIT_IDX), ('_EXIT_', dsinfo.EXIT_FN, dsinfo.EXIT_BASE, dsinfo.EXIT_IDX)):
        if s not in res['segs']: continue
        if mod not in fn: raise D1Fehler(f'{s}: Modul {mod} hat im Original keinen Eintrag')
        b = sb.get(s, b'')
        if len(b) != 6: raise D1Fehler(f'{s}: {len(b)} Bytes, erwartet ein Eintrag (6 Bytes)')
        orig = ds_bytes(base + 6 * idx[mod], 6)
        if b[:2] != orig[:2]: raise D1Fehler(f'{s}: Art/Prioritaet {b[:2].hex()} != Original {orig[:2].hex()}')
        fx = [f for f in C['fix'] if f['seg'] == s]
        want = int(fn[mod][-4:], 16)
        if len(fx) != 1 or fx[0]['typ'] != 3 or fx[0]['loc'] != 2 or fx[0]['tgt'] != ('SEG', cseg) or \
                (struct.unpack_from('<H', b, 2)[0] + fx[0]['disp']) & 0xffff != want:
            raise D1Fehler(f'{s}: Ziel {fx} passt nicht zu {fn[mod]} ({cseg}+{want:#x})')
        res['notes'].append(f'{s}: Eintrag -> {fn[mod]} wie im Original')


def eigen_rewrite(fixed, res, alln, removed, out, offmap=None):
    """OBJ mit eigenen Daten linkfertig machen: EXTDEF-Namen aus Datenzeigern, eigene Segmente wortausgerichtet,
    PUBDEFs fuer alle DGROUP-Publics im eigenen Bereich (Namen der Stufe-1-MAP) + eigene C-Namen."""
    d = open(fixed, 'rb').read(); ln = ['']; segs = ['']; grps = ['']; outr = []
    recs = list(records(d))
    for t, b in recs:
        if t == 0x96:
            q = 0
            while q < len(b): l = b[q]; ln.append(b[q + 1:q + 1 + l].decode('latin1')); q += 1 + l
        elif t == 0x98:
            a = b[0]; q = 1 + (3 if a >> 5 == 0 else 0) + 2; ni, _ = idx(b, q); segs.append(ln[ni])
        elif t == 0x9a:
            ni, _ = idx(b, 0); grps.append(ln[ni])
    if 'DGROUP' not in grps: raise D1Fehler('keine GRPDEF DGROUP')
    gi = grps.index('DGROUP')
    pubs = []                                   # (segidx, off, name)
    names = set()
    for s, info in res['segs'].items():
        if info['base'] is None: continue
        si = segs.index(s); a0, n = info['base'], info['len']
        for nm, (sg, of) in sorted(alln.items(), key=lambda x: (x[1][1], x[0])):
            if sg == DG and a0 <= of < a0 + max(n, 1) and nm not in names:
                pubs.append((si, of - a0, nm)); names.add(nm)
    for s, off, nm in removed:
        if s not in res['segs']: raise D1Fehler(f'Public {nm} in Segment {s} nicht unterstuetzt')
        ad = res['segs'][s]['base'] + off
        if nm in alln and alln[nm] != (DG, ad):
            raise D1Fehler(f'eigenes Public {nm} bei ds:{ad:#x}, Stufe-1-MAP hat es bei {alln[nm][0]:04X}:{alln[nm][1]:04X}')
        if nm not in names: pubs.append((segs.index(s), off, nm)); names.add(nm)
    # Lokales Public des eigenen _INIT_/_EXIT_-Ziels (BCCs statischer Initialisierer @_STCON_$qv eines globalen
    # Objekts, LPUBDEF): TLINK legt dafuer einen eigenen Overlay-Einsprung an. Das Public der ASM-Fassung an derselben
    # Stelle (F_O224_032B) faellt weg, sonst gaebe es zwei Einspruenge fuer denselben Versatz (Original: nur das lokale).
    cseg_i = next(i for i, n in enumerate(segs) if n.endswith('_TEXT'))
    import dsinfo
    ie_off = {int(dsinfo.INIT_FN[res['mod']][-4:], 16) for s_ in ('_INIT_',) if s_ in res['segs']} | \
             {int(dsinfo.EXIT_FN[res['mod']][-4:], 16) for s_ in ('_EXIT_',) if s_ in res['segs']}
    if offmap: ie_off = {offmap(x) for x in ie_off}
    loc_off = set()
    for t, b in (recs if ie_off else []):
        if t in (0xb6, 0xb7):
            gi_, q = idx(b, 0); si_, q = idx(b, q)
            if si_ != cseg_i: continue
            while q < len(b):
                l = b[q]; q += 1 + l; v_ = struct.unpack_from('<H', b, q)[0]; q += 2; _, q = idx(b, q)
                if v_ in ie_off: loc_off.add(v_)
    if loc_off:
        newrecs = []
        for t, b in recs:
            if t == 0x90:
                gi_, q = idx(b, 0); si_, q = idx(b, q)
                if si_ == cseg_i:
                    hdr = b[:q]; body = b''
                    while q < len(b):
                        l = b[q]; e0 = q; q += 1 + l; off = struct.unpack_from('<H', b, q)[0]; q += 2; _, q = idx(b, q)
                        nm = b[e0 + 1:e0 + 1 + l].decode('latin1')
                        if off in loc_off: res['notes'].append(f'Public {nm} entfaellt (lokales Public an {off:#x})'); continue
                        body += b[e0:q]
                    if not body: continue
                    b = hdr + body
            newrecs.append((t, b))
        recs = newrecs
    last_pub = max([i for i, (t, _) in enumerate(recs) if t in (0x90, 0x91)], default=None)
    anchor = last_pub if last_pub is not None else max(i for i, (t, _) in enumerate(recs) if t in (0x8c, 0xb4, 0x98, 0x9a))
    segi = 0
    for ri, (t, body) in enumerate(recs):
        r = rec(t, body)
        if t == 0x98:
            segi += 1
            if segs[segi] in res['segs'] and res['segs'][segs[segi]]['base'] is not None:
                nb = bytearray(body); nb[0] = (nb[0] & 0x1f) | 0x40; r = rec(t, bytes(nb))   # wortausgerichtet
        elif t in (0x8c, 0xb4) and res['extren']:
            q = 0; nb = b''
            while q < len(body):
                l = body[q]; nm = body[q + 1:q + 1 + l].decode('latin1'); q += 1 + l; _, q2 = idx(body, q)
                tb = body[q:q2]; q = q2; nm = res['extren'].get(nm, nm); nb += bytes([len(nm)]) + nm.encode('latin1') + tb
            r = rec(t, nb)
        outr.append(r)
        if ri == anchor and pubs:
            for si in sorted({p[0] for p in pubs}):
                cur = b''; hdr = enc_idx(gi) + enc_idx(si)
                for s_, off, nm in pubs:
                    if s_ != si: continue
                    e = bytes([len(nm)]) + nm.encode('latin1') + struct.pack('<H', off) + b'\0'
                    if len(cur) + len(e) > 1000: outr.append(rec(0x90, hdr + cur)); cur = b''
                    cur += e
                if cur: outr.append(rec(0x90, hdr + cur))
    open(out, 'wb').write(b''.join(outr))
    return pubs


def pop_ref(argv):
    a = list(argv)
    if '--ref' in a:
        i = a.index('--ref'); r = a[i + 1]; del a[i:i + 2]; return a, r
    return a, None


def run_cobjfix(cobj, aobj, mapf, out, ref=None):
    cmd = [sys.executable, '-I', '-X', 'utf8', ROOT + '/tools/cobjfix.py', cobj, aobj, mapf, out] + (['--ref', ref] if ref else [])
    r = subprocess.run(cmd, capture_output=True, text=True, env=os.environ)
    sys.stdout.write(r.stdout); sys.stderr.write(r.stderr)
    return r.returncode


def own_signature(path):
    """eigene Datensegmente einer OBJ: Laengen, Bytes und Fixups aus diesen Segmenten (fuer den Fix-Modus)"""
    own = own_segments(path); sb = objbytes.seg_bytes(path); O = objfix.parse(path)
    fx = sorted((f['seg'], f['loc'], f['typ'], f['tgt'], f['disp']) for f in O['fix'] if f['seg'] in own)
    return own, {s: sb.get(s, b'') for s in own}, fx


def own_compare(fix, ref, offmap=None):
    """eigene Daten der Fix-Fassung gegen REF: gleiche Segmente/Laengen, gleiche Fixups (Lage, Typ, Ziel), gleiche Bytes
    ausser an Codezeigern auf das eigene Codesegment - deren Versatz muss der verschobenen Funktion entsprechen
    (offmap: REF-Versatz -> Fix-Versatz ueber die C-Namen). -> Fehlertext oder None"""
    of, bf, xf = own_signature(fix); orr, br, xr = own_signature(ref)
    if of != orr: return f'eigene Segmente {of} != REF {orr}'
    if xf != xr: return 'Fixups in den eigenen Daten anders als in REF'
    cseg = [x for x in objfix.parse(fix)['segs'] if x.endswith('_TEXT')][0]
    for sg in of:
        a, b = bytearray(bf[sg]), bytearray(br[sg])
        if len(a) != len(b): return f'{sg}: Laenge {len(a)} != REF {len(b)}'
        for (s_, loc, typ, tgt, disp) in xf:
            if s_ != sg or tgt != ('SEG', cseg) or typ not in (1, 3): continue
            va = struct.unpack_from('<H', a, loc)[0]; vb = struct.unpack_from('<H', b, loc)[0]
            if va != vb:
                if offmap is None: return f'{sg}+{loc:#x}: Codezeiger {vb:#x} -> {va:#x} ohne Abbildung'
                try: want = offmap(vb)
                except D1Fehler as e: return f'{sg}+{loc:#x}: {e}'
                if want != va: return f'{sg}+{loc:#x}: Codezeiger {vb:#x} -> {va:#x}, erwartet {want:#x}'
            a[loc:loc + 2] = b[loc:loc + 2]
        if a != b:
            i = next(k for k in range(len(a)) if a[k] != b[k])
            return f'{sg}+{i:#x}: Bytes anders als REF'
    return None


def code_pubs(path):
    """PUBDEF/LPUBDEF im Codesegment -> {name: offset}"""
    d = open(path, 'rb').read(); ln = ['']; segs = ['']; res = {}
    for t, b in records(d):
        if t == 0x96:
            q = 0
            while q < len(b): l = b[q]; ln.append(b[q + 1:q + 1 + l].decode('latin1')); q += 1 + l
        elif t == 0x98:
            a = b[0]; q = 1 + (3 if a >> 5 == 0 else 0) + 2; ni, _ = idx(b, q); segs.append(ln[ni])
        elif t in (0x90, 0x91, 0xb6, 0xb7):
            gi, q = idx(b, 0); si, q = idx(b, q)
            if si == 0: q += 2
            while q < len(b):
                l = b[q]; nm = b[q + 1:q + 1 + l].decode('latin1'); q += 1 + l
                o = struct.unpack_from('<H', b, q)[0]; q += 2; _, q = idx(b, q)
                if si and segs[si].endswith('_TEXT'): res.setdefault(nm, o)
    return res


def main_eigen(argv):
    argv, ref = pop_ref(argv)
    if ref: return main_eigen_ref(argv, ref)
    cobj, aobj, mapf, out = argv[1:5]
    pre = out + '.pre'; tmp = out + '.d1tmp'
    removed = strip_data_pubs(cobj, pre)
    r = subprocess.run([sys.executable, '-I', '-X', 'utf8', ROOT + '/tools/cobjfix.py', pre, aobj, mapf, tmp], capture_output=True, text=True)
    sys.stdout.write(r.stdout); sys.stderr.write(r.stderr)
    os.remove(pre)
    if r.returncode != 0:
        if os.path.exists(tmp): os.remove(tmp)
        return r.returncode
    try:
        if not own_segments(tmp): raise D1Fehler('Spalte D, aber keine eigenen Daten in der C-OBJ')
        res = eigen_check(tmp, aobj, mapf)
        pubs = eigen_rewrite(tmp, res, mapnames(mapf), removed, out)
    except D1Fehler as e:
        print(f'{cobj}: EIGENE DATEN (Phase B) NICHT IN ORDNUNG: {e}')
        if os.path.exists(out): os.remove(out)
        return 2
    finally:
        if os.path.exists(tmp): os.remove(tmp)
    print(f"{cobj}: Phase B, Modul {res['mod']} definiert seine Daten selbst, {res['checked']} Bytes gegen Original geprueft, "
          f"{len(pubs)} Publics im eigenen Bereich")
    for s, i in res['segs'].items():
        if i['base'] is None: print(f"  {s}: {i['len']} Bytes (eigener Eintrag)"); continue
        print(f"  {s}: ds:{i['base']:#06x}..{i['base'] + i['len']:#06x} ({i['len']} Bytes){' BSS' if i['kind'] == 'BSS' else ' GLEICH'}")
    for a, b in res['extren'].items(): print(f'  EXTDEF {a} -> {b} (Zeiger in eigenen Daten)')
    return 0


def main_eigen_ref(argv, ref):
    """--eigen im Fix-Modus: Pruefung/Lage an REF, Umschreiben der Fix-Fassung"""
    cobj, aobj, mapf, out = argv[1:5]
    pre, preref = out + '.pre', out + '.preref'; tmp, tmpref = out + '.d1tmp', out + '.d1ref'
    removed = strip_data_pubs(cobj, pre); strip_data_pubs(ref, preref)
    try:
        rc = run_cobjfix(preref, aobj, mapf, tmpref)
        if rc: return rc
        rc = run_cobjfix(pre, aobj, mapf, tmp, ref=preref)
        if rc: return rc
        try:
            rp, fp = code_pubs(tmpref), code_pubs(tmp)
            rby = {}
            for n, o in rp.items(): rby.setdefault(o, []).append(n)
            def offmap(o):
                c = {fp[n] for n in rby.get(o, []) if n in fp}
                if len(c) != 1: raise D1Fehler(f'Einsprung {o:#x} in der Fix-Fassung nicht eindeutig: {sorted(c)}')
                return c.pop()
            diff = own_compare(tmp, tmpref, offmap)
            if diff: raise D1Fehler('Fix-Fassung veraendert die eigenen Daten: ' + diff)
            res = eigen_check(tmpref, aobj, mapf)
            pubs = eigen_rewrite(tmp, res, mapnames(mapf), removed, out, offmap)
        except D1Fehler as e:
            print(f'{cobj}: EIGENE DATEN (Phase B, Fix-Modus) NICHT IN ORDNUNG: {e}')
            if os.path.exists(out): os.remove(out)
            return 2
    finally:
        for f in (pre, preref, tmp, tmpref):
            if os.path.exists(f): os.remove(f)
    print(f"{cobj}: Phase B Fix-Modus, Modul {res['mod']}, eigene Daten gleich REF, {len(pubs)} Publics im eigenen Bereich")
    return 0


# ---------------------------------------------------------------- Hauptprogramm
def main(argv):
    argv, ref = pop_ref(argv)
    cobj, aobj, mapf, out = argv[1:5]
    if ref:
        tmp = out + '.d1tmp'
        rc = run_cobjfix(cobj, aobj, mapf, tmp, ref=ref)
        if rc:
            if os.path.exists(tmp): os.remove(tmp)
            return rc
        if not own_segments(tmp) and not own_segments(ref):
            os.replace(tmp, out); return 0
        # eigene Daten (Literale) ins zentrale DS: Lage an REF bestimmen, Verweise der Fix-Fassung ueber ihren Wert v
        tmpref = out + '.d1ref'
        try:
            rc = run_cobjfix(ref, aobj, mapf, tmpref)
            if rc: return rc
            diff = own_compare(tmp, tmpref)
            if diff: raise D1Fehler('Fix-Fassung veraendert die eigenen Daten: ' + diff)
            res = analyse(tmpref, aobj, mapf)
            C = objfix.parse(tmp); cseg = [x for x in C['segs'] if x.endswith('_TEXT')][0]
            cb = objbytes.seg_bytes(tmp).get(cseg, b'')
            def mapaddr(sg, v):
                for pc in res['pieces']:
                    if pc['seg'] == sg and (pc['von'] <= v < pc['bis'] or v == pc['bis'] == res['own'][sg]):
                        return (v + pc['delta']) & 0xffff
                raise D1Fehler(f'{sg}+{v:#x}: keinem Stueck zugeordnet')
            fx = []
            for f in C['fix']:
                tk, tn = f['tgt']
                if tk != 'SEG' or tn not in res['own'] or f['seg'] != cseg: continue
                if f['typ'] == 2: fx.append(dict(f=f, seg=tn, v=None, adr=None)); continue
                v = (struct.unpack_from('<H', cb, f['loc'])[0] + f['disp']) & 0xffff
                fx.append(dict(f=f, seg=tn, v=v, adr=mapaddr(tn, v)))
            res['fix'] = fx
            rewrite(tmp, res, mapnames(mapf), out)
        except D1Fehler as e:
            print(f'{cobj}: EIGENE DATEN (Fix-Modus) NICHT AUFLOESBAR: {e}')
            if os.path.exists(out): os.remove(out)
            return 2
        finally:
            for f in (tmp, tmpref):
                if os.path.exists(f): os.remove(f)
        print(f"{cobj}: Fix-Modus, eigene Daten {res['own']} -> zentrales DS wie REF, {len(res['fix'])} Fixups umgebogen")
        return 0
    tmp = out + '.d1tmp'
    r = subprocess.run([sys.executable, '-I', '-X', 'utf8', ROOT + '/tools/cobjfix.py', cobj, aobj, mapf, tmp],
                       capture_output=True, text=True)
    sys.stdout.write(r.stdout); sys.stderr.write(r.stderr)
    if r.returncode != 0:
        if os.path.exists(tmp): os.remove(tmp)
        return r.returncode
    try:
        res = analyse(tmp, aobj, mapf)
        if not res['own']:
            os.replace(tmp, out); return 0
        newext, drop = rewrite(tmp, res, mapnames(mapf), out)
    except D1Fehler as e:
        print(f'{cobj}: EIGENE DATEN NICHT AUFLOESBAR: {e}')
        if os.path.exists(out): os.remove(out)
        return 2
    finally:
        if os.path.exists(tmp): os.remove(tmp)
    nb = sum(p['bis'] - p['von'] for p in res['pieces'])
    print(f"{cobj}: eigene Daten {res['own']} -> zentrales DS, {len(res['pieces'])} Stueck(e), "
          f"{res['checked']} Bytes gegen Original geprueft, {len(res['fix'])} Fixups umgebogen")
    for p in res['pieces']:
        print(f"  {p['seg']}+{p['von']:#06x}..{p['bis']:#06x} -> ds:{p['adr']:#06x}  "
              f"{'BSS' if p['bss'] else 'Bytes GLEICH'}, {p['dsana']}")
    for n in res['notes'][:12]: print('  ' + n)
    return 0


if __name__ == '__main__':
    if len(sys.argv) > 1 and sys.argv[1] == '--eigen': sys.exit(main_eigen(sys.argv[1:]))
    sys.exit(main(sys.argv))
