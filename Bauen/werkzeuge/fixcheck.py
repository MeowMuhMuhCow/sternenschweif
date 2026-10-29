#!/usr/bin/env python3
import os as _swos; _SW = _swos.environ.get('SW_ROOT', '/root/sternenschweif')  # Bau-Paket
"""fixcheck.py NEU.EXE [ORIG.EXE] - statische Pruefung einer Fix-EXE (FIX_BUGS > 0) gegen das Original.

Die Fix-EXE kann nicht bytegleich sein. Geprueft wird, was fuer ein lauffaehiges Programm gleich bleiben muss:
  1. Residente Segmente: gleiche Anzahl, gleiche Lage (Paragraph) und Laenge -> DGROUP, Stubs und alle residenten
     Adressen unveraendert (geaenderte residente C-Module werden in build.sh auf ihre Originallaenge aufgefuellt).
  2. Datensegment (initialisierte Daten): Byte fuer Byte gleich dem Original.
  3. Overlays: gleiche Anzahl, je Overlay gleich viele Stub-Einsprungpunkte; jeder Einsprung zeigt in der Fix-EXE
     auf einen Funktionsanfang mit demselben Prolog wie im Original (Funktionszeiger/Stubs stimmen).
  4. MZ-Kopf: Zahl der Relocations, Stack, Einsprung CS:IP.
Ausgabe: Abweichungen und eine Zusammenfassung; Exit 1 bei Fehlern."""
import sys, struct
sys.path.insert(0, _SW + '/q004/zerl'); import schweif
new = sys.argv[1]; orig = sys.argv[2] if len(sys.argv) > 2 else _SW + '/orig/SCHWEIF.EXE'
DG = 0x24EB
err = []; warn = []
A = schweif.load(orig); B = schweif.load(new)
ra = [s for s in A if s['kind'] == 'R']; rb = [s for s in B if s['kind'] == 'R']
oa = [s for s in A if s['kind'] == 'O']; ob = [s for s in B if s['kind'] == 'O']
# 1. residente Segmente
if len(ra) != len(rb): err.append(f'residente Segmente: {len(ra)} -> {len(rb)}')
nres = 0
for x, y in zip(ra, rb):
    if (x['para'], len(x['code'])) != (y['para'], len(y['code'])):
        err.append(f"resident {x['id']}: Lage/Laenge {x['para']:#x}/{len(x['code']):#x} -> {y['para']:#x}/{len(y['code']):#x}")
    elif x['code'] != y['code']: nres += 1
# 2. Datensegment
da = open(orig, 'rb').read(); db = open(new, 'rb').read()
ha = struct.unpack_from('<H', da, 8)[0] * 16; hb = struct.unpack_from('<H', db, 8)[0] * 16
L = 0x86b6
if da[ha + DG * 16:ha + DG * 16 + L] != db[hb + DG * 16:hb + DG * 16 + L]:
    bad = [i for i in range(L) if da[ha + DG * 16 + i] != db[hb + DG * 16 + i]]
    err.append(f'Datensegment: {len(bad)} Bytes anders, erstes ds:{bad[0]:#x}')
# 3. Overlays und Einsprungpunkte
if len(oa) != len(ob): err.append(f'Overlays: {len(oa)} -> {len(ob)}')
nov = 0; nent = 0; prolog_anders = []
for x, y in zip(oa, ob):
    if len(x['entries']) != len(y['entries']):
        err.append(f"{x['id']}: Einsprungpunkte {len(x['entries'])} -> {len(y['entries'])}"); continue
    if x['code'] != y['code']: nov += 1
    for k, (ea, eb) in enumerate(zip(x['entries'], y['entries'])):
        nent += 1
        if eb >= len(y['code']): err.append(f"{x['id']} Einsprung {k}: {eb:#x} hinter dem Codeende"); continue
        pa = x['code'][ea:ea + 4]; pb = y['code'][eb:eb + 4]
        if pa[:1] != pb[:1] or (pa[0] == 0xc8 and pa[3:4] != pb[3:4]) or (pa[0] == 0x55 and pa[:3] != pb[:3]):
            prolog_anders.append(f"{x['id']} Einsprung {k}: {ea:#x} {pa.hex()} -> {eb:#x} {pb.hex()}")
        elif pa != pb:
            warn.append(f"{x['id']} Einsprung {k}: Prolog {pa.hex()} -> {pb.hex()} (andere Stackgroesse)")
for p in prolog_anders: err.append('Prolog anders: ' + p)
# 4. MZ-Kopf
for nm, o in (('Relocations', 6), ('SS', 0x0e), ('SP', 0x10), ('IP', 0x14), ('CS', 0x16)):
    va = struct.unpack_from('<H', da, o)[0]; vb = struct.unpack_from('<H', db, o)[0]
    if va != vb: (err if nm != 'Relocations' else warn).append(f'MZ {nm}: {va:#x} -> {vb:#x}')
for w in warn: print('HINWEIS', w)
for e in err: print('FEHLER', e)
print(f'fixcheck {new}: {len(rb)} residente Segmente (Lage gleich, {nres} mit geaendertem Code), Datensegment '
      f'{"gleich" if not any(e.startswith("Datensegment") for e in err) else "ANDERS"}, {len(ob)} Overlays '
      f'({nov} geaendert), {nent} Einsprungpunkte geprueft, Groesse {len(da)} -> {len(db)} Bytes: '
      f'{"IN ORDNUNG" if not err else f"{len(err)} FEHLER"}')
sys.exit(1 if err else 0)
