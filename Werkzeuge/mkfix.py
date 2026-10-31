#!/usr/bin/env python3
"""mkfix.py - erzeugt die Schalter fuer die Fehlerbehebungen aus den Tabellen des Projekts.

Quellen (alle unter /root/sternenschweif):
  abschluss/fehler_raster.tsv   Nr, Sicherheit, Auswirkung (kritisch/mittel/egal) ...
  abschluss/fehler_liste.tsv    (falls vorhanden) Spalte exploit = ja -> Fehler nuetzt dem Spieler
  abschluss/fehler_netz_geprueft.tsv  am Code bestaetigte Spielermeldungen mit neuer Nummer (neue_nr 352..),
                                Spalten sicherheit, auswirkung, exploit
  abschluss/fix_titel.tsv       Nr <TAB> Kurztitel <TAB> umgesetzt (ja/teilweise/nein) <TAB> Grund/Hinweis
  csrc/*.C*, l*/SEG0*.ASM       wo ein Schalter tatsaechlich benutzt wird (#if FIX_nnnn / IF FIX_nnnn)
Ausgabe:
  names/inc/s_fix.h             C-Header (in schweif.h eingebunden): Tabelle als Kommentar, Vorgaben je Schalter
  names/inc/s_fix.inc           dasselbe fuer TASM (von den ASM-Modulen mit Fehlerbehebung eingebunden)
  abschluss/fix_schalter.tsv    maschinenlesbare Gesamttabelle (fuer FIXES.md, Bau-Paket)

Stufen: FIX_BUGS 0 = Original (bytegleich), 1 = wichtig (Auswirkung kritisch oder mittel), 2 = alles.
        FIX_EXPLOITS 1 = auch Fehler beheben, die dem Spieler nuetzen (Vorgabe 1, sobald FIX_BUGS > 0).
Jeder Schalter FIX_nnnn laesst sich einzeln uebersteuern (BCC -DFIX_0020=0, TASM /dFIX_0020=0).
Kein Fehler / gewollt (kein Schalter): 30, 40, 70, 96, 137, 156, 223, 228, 229, 303. Nr. 6 = Nr. 212 (FIX_0006 folgt FIX_0212).
"""
import glob, os, re, sys
R = os.environ.get('SSROOT', '/root/sternenschweif')
KEIN = {30: 'gewollt: bewusstes Durchfallen (case 1 -> case 2)', 40: 'kein Fehler: hero_mr_target liefert den Verteidiger',
        70: 'gewollt: Diagnose im Fragetext genannt', 96: 'kein Fehler: Dummy-Ziel (+0x1a1 waere old_dlg_mode)',
        137: 'kein Fehler: Zustand wird im Dialog gesetzt', 156: 'gewollt: -1 passt zum anderen Menue',
        223: 'kein Fehler: get_next_hero liefert nie 0', 228: 'gewollt: Schutz ueber cd_check, Rest der Disketten-Fassung',
        229: 'ohne Wirkung: doppelter Test, Absicht nicht belegbar', 303: 'toter Debug-Code (Waechter nie eingehaengt)'}
ALIAS = {6: 212}


def ascii_(t):
    """Kommentartexte in den DOS-Quellen nur ASCII (Umlaute umschreiben, typografische Zeichen ersetzen)."""
    for a, b in (('ä', 'ae'), ('ö', 'oe'), ('ü', 'ue'), ('Ä', 'Ae'), ('Ö', 'Oe'), ('Ü', 'Ue'), ('ß', 'ss'),
                 ('„', '"'), ('“', '"'), ('”', '"'), ('‚', "'"), ('‘', "'"), ('’', "'"), ('–', '-'), ('—', '-'),
                 ('…', '...'), ('×', 'x'), ('→', '->'), ('²', '^2')):
        t = t.replace(a, b)
    return t.encode('ascii', 'replace').decode('ascii').replace('*/', '* /')


def tsv(p):
    if not os.path.exists(p): return []
    rows = [l.rstrip('\n').split('\t') for l in open(p, encoding='utf-8', errors='replace')]
    return rows


def main():
    raster = {}
    for f in tsv(R + '/abschluss/fehler_raster.tsv')[1:]:
        if f and f[0].isdigit(): raster[int(f[0])] = dict(sich=f[1], ausw=f[2])
    expl = set()
    # bestaetigte Spielermeldungen (Foren), neue Nummern 352.. (Spalten nach Namen)
    netz = tsv(R + '/abschluss/fehler_netz_geprueft.tsv')
    if netz:
        h = [x.strip().lower() for x in netz[0]]
        for f in netz[1:]:
            f = f + [''] * (len(h) - len(f)); d = dict(zip(h, f))
            if d.get('neue_nr', '').strip().isdigit():
                n = int(d['neue_nr'])
                raster[n] = dict(sich=d.get('sicherheit', ''), ausw=d.get('auswirkung', ''))
                if d.get('exploit', '').strip().lower() == 'ja': expl.add(n)
    rows = tsv(R + '/abschluss/fehler_liste.tsv')
    if rows:
        h = [x.strip().lower() for x in rows[0]]
        ci = h.index('exploit') if 'exploit' in h else None
        ni = h.index('nr') if 'nr' in h else 0
        for f in rows[1:]:
            if ci is not None and len(f) > ci and f[ni].strip().isdigit() and f[ci].strip().lower() == 'ja':
                expl.add(int(f[ni]))
    titel = {}
    for f in tsv(R + '/abschluss/fix_titel.tsv'):
        if f and f[0].isdigit(): titel[int(f[0])] = f[1:] + [''] * (3 - len(f[1:]))
    used = {}
    for p in glob.glob(R + '/csrc/*.C*') + glob.glob(R + '/l[0-9][0-9]/SEG0*.ASM'):
        for m in re.finditer(r'^\s*#?if\s+FIX_(\d{4})\b', open(p, encoding='latin1').read(), re.M | re.I):
            used.setdefault(int(m.group(1)), set()).add(os.path.basename(p))
    out = []
    for nr in sorted(raster):
        a = raster[nr]['ausw']
        t = titel.get(nr, ['', '', ''])
        if nr in KEIN:
            out.append(dict(nr=nr, titel=t[0], kat=a, expl=nr in expl, stufe='-', um='kein Fehler', grund=KEIN[nr], dat=''))
            continue
        stufe = 'wichtig' if a in ('kritisch', 'mittel') else 'alles'
        src = used.get(ALIAS.get(nr, nr), set())
        um = t[1] or ('ja' if src else 'nein')
        if not src and um != 'nein': um = 'nein'
        out.append(dict(nr=nr, titel=t[0], kat=a, expl=nr in expl or (ALIAS.get(nr) in expl), stufe=stufe, um=um,
                        grund=t[2], dat=' '.join(sorted(src))))
    # TSV
    with open(R + '/abschluss/fix_schalter.tsv', 'w') as f:
        f.write('nr\tschalter\ttitel\tkategorie\texploit\tstufe\tumgesetzt\tgrund\tdateien\n')
        for o in out:
            sw = '-' if o['stufe'] == '-' else 'FIX_%04d' % o['nr']
            f.write(f"{o['nr']}\t{sw}\t{o['titel']}\t{o['kat']}\t{'ja' if o['expl'] else 'nein'}\t{o['stufe']}\t{o['um']}\t{o['grund']}\t{o['dat']}\n")
    # Header
    L = ['/* s_fix.h - Fehlerbehebungen per Schalter (Projekt Sternenschweif, erzeugt von tools/mkfix.py - nicht von Hand aendern)',
         ' *',
         ' * Stufenschalter (BCC -D..., Vorgabe: Original):',
         ' *   FIX_BUGS 0      Original, bytegleich mit SCHWEIF.EXE C1.02 (Standard)',
         ' *   FIX_BUGS 1      "wichtig": alle Fehler mit Auswirkung kritisch oder mittel',
         ' *   FIX_BUGS 2      "alles": dazu alle Fehler mit Auswirkung egal',
         ' *   FIX_EXPLOITS 1  auch Fehler beheben, die dem Spieler nuetzen (Vorgabe 1, sobald FIX_BUGS > 0); 0 = drinlassen',
         ' * Einzelschalter: jeder FIX_nnnn laesst sich uebersteuern, z. B. -DFIX_0020=0 (aus) oder -DFIX_0050=1 (an).',
         ' * Keine Behebung aendert Strukturen, das Spielstandformat oder das Datensegment.',
         ' *',
         ' * Schalter   Stufe    Kategorie Exploit umgesetzt  Titel',
         ' * ---------  -------  --------- ------- ---------  -----']
    for o in out:
        sw = 'FIX_%04d' % o['nr'] if o['stufe'] != '-' else '(%d)' % o['nr']
        L.append(ascii_(f" * {sw:<10} {o['stufe']:<8} {o['kat']:<9} {'ja' if o['expl'] else '-':<7} {o['um']:<10} {o['titel']}"
                 + (f' [{o["grund"]}]' if o['stufe'] == '-' else '')))
    L += [' */', '#ifndef S_FIX_H', '#define S_FIX_H', '',
          '#ifndef FIX_BUGS', '#define FIX_BUGS 0', '#endif',
          '#ifndef FIX_EXPLOITS', '#if FIX_BUGS > 0', '#define FIX_EXPLOITS 1', '#else', '#define FIX_EXPLOITS 0', '#endif', '#endif',
          '', '/* Vorgaben je Stufe (X = Exploit: nur mit FIX_EXPLOITS) */',
          '#define FIX_STUFE_W  (FIX_BUGS >= 1)', '#define FIX_STUFE_A  (FIX_BUGS >= 2)',
          '#define FIX_STUFE_WX (FIX_BUGS >= 1 && FIX_EXPLOITS)', '#define FIX_STUFE_AX (FIX_BUGS >= 2 && FIX_EXPLOITS)', '',
          '/* Zusatzfunktion, keine Fehlerbehebung: Probenlog. Jede Probe wird als Zahlenzeile an PROBEN.CSV im Spielordner',
          ' * angehaengt, aber nur, wenn diese Datei schon da ist (Datei anlegen = Log an). Spiel und Spielstand bleiben gleich.',
          ' * Umsetzung: s_plog.h (static je Overlay), Aufrufe in O076, O091, O158..O161. */',
          '#ifndef FIX_PROBENLOG', '#define FIX_PROBENLOG FIX_STUFE_W', '#endif', '']
    A = ['; s_fix.inc - Fehlerbehebungen per Schalter fuer TASM (erzeugt von tools/mkfix.py, siehe names/inc/s_fix.h)',
         'ifndef FIX_BUGS', 'FIX_BUGS = 0', 'endif', 'ifndef FIX_EXPLOITS', 'if FIX_BUGS gt 0', 'FIX_EXPLOITS = 1', 'else',
         'FIX_EXPLOITS = 0', 'endif', 'endif']
    for o in out:
        if o['stufe'] == '-': continue
        n = o['nr']; sw = 'FIX_%04d' % n
        if n in ALIAS:
            val = 'FIX_%04d' % ALIAS[n]
            L += [f'#ifndef {sw}', f'#define {sw} {val}\t/* dieselbe Stelle */', '#endif']
            continue
        st = ('W' if o['stufe'] == 'wichtig' else 'A') + ('X' if o['expl'] else '')
        L += [f'#ifndef {sw}', f'#define {sw} FIX_STUFE_{st}', '#endif']
        if n >= 300:
            cond = ('FIX_BUGS ge 1' if st[0] == 'W' else 'FIX_BUGS ge 2') + (' and FIX_EXPLOITS ne 0' if 'X' in st else '')
            A += [f'ifndef {sw}', f'if {cond}', f'{sw} = 1', 'else', f'{sw} = 0', 'endif', 'endif']
    L += ['', '#endif', '']
    open(R + '/names/inc/s_fix.h', 'w', newline='\n').write('\n'.join(L))
    open(R + '/names/inc/s_fix.inc', 'w', newline='\r\n').write('\n'.join(A) + '\n')
    n_um = {k: sum(1 for o in out if o['um'] in ('ja', 'teilweise') and o['stufe'] == k) for k in ('wichtig', 'alles')}
    print(f"mkfix: {len(out)} Nummern, {sum(1 for o in out if o['stufe'] != '-')} Schalter, umgesetzt wichtig {n_um['wichtig']}, "
          f"alles {n_um['alles']}, Exploits {len(expl)}")


if __name__ == '__main__':
    main()
