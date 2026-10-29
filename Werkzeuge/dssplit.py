#!/usr/bin/env python3
"""dssplit.py - teilt das Spiel-_DATA (dsg/DS00..DS05) in Stuecke je Originalmodul.

Im Original traegt jedes C-Modul seine eigenen Daten; TLINK schreibt die DGROUP-Relocations
deshalb verschraenkt mit dem Code der Module. Aus der MZ-Relocationtabelle des Originals
ergeben sich Gruppen von DGROUP-Relocations (jeweils zwischen zwei residenten Codemodulen).
Fuer jede Gruppe entsteht ein Stueck lnk/DSPnn.ASM, das vor dem naechsten residenten Modul
gelinkt wird. Die #pragma-startup/exit-Eintraege (_INIT_/_EXIT_) wandern in das Stueck,
dessen Gruppe sie enthaelt.

Ausgabe: lnk/DSPnn.ASM, lnk/dsp.json  {stuecke:[{name,vor,order:[[seg,pos],...]}]}
"""
import re, struct, json
R = '/root/sternenschweif'
DGROUP_PARA = 0x24EB
INIT_BASE, EXIT_BASE = 0x8b06, 0x8b42           # Beginn _INIT_ / _EXIT_ in DGROUP
INIT_RECS = ['F_O008_0DFC', 'F_O032_0C8C', 'F_O224_032B']
EXIT_RECS = ['F_O032_0F94']
L = json.load(open(R + '/lnk_ranges.json'))
start = L['start']; mods = L['mods']

# --- Original-Relocations in Laeufe nach Segment ---
e = open(R + '/orig/SCHWEIF.EXE', 'rb').read()
nr, ro = struct.unpack_from('<H', e, 6)[0], struct.unpack_from('<H', e, 0x18)[0]
rel = [struct.unpack_from('<HH', e, ro + 4 * i) for i in range(nr)]
def modof(lin):
    for m in sorted(start, key=lambda k: start[k]):
        if m in mods and start[m] <= lin < start[m] + mods[m][1]:
            return m
    return None
seq = []                                          # [(art, wert, [pos...])]
for o, s in rel:
    if s == DGROUP_PARA:
        k = ('D', None)
    else:
        k = ('C', modof(s * 16 + o))
    if seq and seq[-1][0] == k:
        seq[-1][1].append(o)
    else:
        seq.append((k, [o]))
groups = []                                       # (pos-liste, naechstes residentes Modul)
for i, ((art, m), pl) in enumerate(seq):
    if art != 'D':
        continue
    nxt = None
    for (a2, m2), _ in seq[i + 1:]:
        if a2 == 'C' and m2 and m2 not in ('R00', 'R47'):
            nxt = m2; break
    if nxt is None:
        break                                     # Bibliotheksteil am Ende: bleibt wie er ist
    groups.append((pl, nxt))

# --- DS-Quellen einlesen ---
items = []; extrns = {}
for n in range(6):
    t = open(f'{R}/dsg/DS0{n}.ASM').read().split('\n')
    a = next(i for i, l in enumerate(t) if l.startswith('_DATA\tsegment'))
    b = next(i for i, l in enumerate(t) if l.startswith('_DATA\tends'))
    for l in t[:a]:
        m = re.match(r'\textrn\t(\S+):(\S+)', l)
        if m: extrns[m.group(1)] = l
    for l in t[a + 1:b]:
        m = re.match(r'(\S+)\tlabel\t\S+\t; 0x([0-9a-f]+)', l)
        if m:
            items.append([int(m.group(2), 16), [l]])
        else:
            items[-1][1].append(l)
offs = [it[0] for it in items]
END = 0x86b6

# --- Grenzen: erstes Label hinter der letzten Relocation der vorigen Gruppe ---
def dpos(pl): return [p for p in pl if p < INIT_BASE]
bounds = [offs[0]]
for k in range(1, len(groups)):
    lo = max(dpos(groups[k - 1][0])) + 2
    hi = min(dpos(groups[k][0]))
    c = [o for o in offs if lo <= o <= hi]
    if not c: raise SystemExit(f'keine Grenze zwischen Gruppe {k-1} und {k}')
    bounds.append(c[0])
bounds.append(END)

out = []
for k, (pl, nxt) in enumerate(groups):
    b0, b1 = bounds[k], bounds[k + 1]
    body = [l for o, ls in items if b0 <= o < b1 for l in ls]
    txt = '\n'.join(body)
    used = [v for s, v in extrns.items() if re.search(r'\b' + re.escape(s) + r'\b', txt)]
    inits = [i for i in range(len(INIT_RECS)) if INIT_BASE + 6 * i + 4 in pl]
    exits = [i for i in range(len(EXIT_RECS)) if EXIT_BASE + 6 * i + 4 in pl]
    order = []
    for p in pl:
        if p >= EXIT_BASE:
            i = (p - EXIT_BASE - 4) // 6; order.append(['_EXIT_', 6 * exits.index(i) + 4])
        elif p >= INIT_BASE:
            i = (p - INIT_BASE - 4) // 6; order.append(['_INIT_', 6 * inits.index(i) + 4])
        else:
            order.append(['_DATA', p - b0])
    segs = ['_DATA', '_BSS'] + (['_INIT_'] if inits else []) + (['_EXIT_'] if exits else [])
    name = f'DSP{k:02d}'
    h = [f'; {name}.ASM - Spiel-_DATA 0x{b0:04x}..0x{b1-1:04x} (Daten des Moduls vor {nxt}), erzeugt von dssplit.py',
         '\tifndef ??version', '?debug\tmacro', '\tendm', '\tendif',
         'DGROUP\tgroup\t' + ','.join(segs), '\tassume\tds:DGROUP']
    h += [extrns[s] for s in extrns if extrns[s] in used]
    for s in INIT_RECS + EXIT_RECS:
        if (s in [INIT_RECS[i] for i in inits] + [EXIT_RECS[i] for i in exits]) and not any(f'\t{s}:' in u for u in used):
            h.append(f'\textrn\t{s}:far')
    h += ['_BSS\tsegment word public \'BSS\'', '_BSS\tends', '_DATA\tsegment ' + ('word' if k == 0 else 'byte') + ' public \'DATA\'']
    h += body + ['_DATA\tends']
    if inits:
        h.append("_INIT_\tsegment word public 'INITDATA'")
        for i in inits: h += ['\tdb\t1,32', f'\tdd\t{INIT_RECS[i]}']
        h.append('_INIT_\tends')
    if exits:
        h.append("_EXIT_\tsegment word public 'EXITDATA'")
        for i in exits: h += ['\tdb\t1,32', f'\tdd\t{EXIT_RECS[i]}']
        h.append('_EXIT_\tends')
    h.append('\tend')
    open(f'{R}/lnk/{name}.ASM', 'w', newline='').write('\r\n'.join(h) + '\r\n')
    out.append({'name': name, 'vor': nxt, 'von': b0, 'bis': b1, 'order': order})
    print(f'{name} 0x{b0:04x}-0x{b1:04x} vor {nxt}: {len(pl)} Relocs, init {inits} exit {exits}')
json.dump({'stuecke': out}, open(R + '/lnk/dsp.json', 'w'), indent=1)
