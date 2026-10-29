#!/usr/bin/env python3
"""datseg_patch.py - Build-Patch fuer Bright Eyes' datseg.cpp und seg013.asm.

Ziel: Das Datensegment soll genauso angeordnet sein wie in Attics Original.

  Original (SCHICKM.EXE, DS = DGROUP):
    0x0000-0x0093  Borland-Startcode (C0)
    0x0094-0xB4DF  initialisierte Spieldaten         -> datseg.cpp Teil 1
    0xB4E0-0xBC37  Daten der C-Bibliothek (ctype, FILES, ... , _INIT_, _EXIT_)
    0xBC38-0xE5E4  uninitialisierte Spielvariablen  -> datseg.cpp Teil 2 (BSS)
    danach         BSS der Bibliothek, Stack

Aenderungen (nur an der Arbeitskopie):
  1. char ds[1]            -> extern (ds wird per objsym auf DATASEG@ gelegt)
  2. g_unkn_029 @0x447e    -> unsigned _stklen = 0x2000  (Attics Stackgroesse;
                              sonst zieht TLINK STKLEN aus CL.LIB dazu)
  3. Kopie der Bibliotheksdaten 0xB4E0-0xBC37 entfernen (die echten Daten
     legt TLINK genau dorthin)
  4. ab 0xBC38 alle Initialisierungen (= 0) entfernen -> Variablen landen im BSS
  5. seg013.asm: dummy1_seg entfernen und dummy2_seg durch ein uninitialisiertes
     4-KB-Stacksegment ersetzen. Die Segmenttabelle der Overlay-Verwaltung
     (_EXEINFO_) zeigt: Das Original hat EIN leeres Datensegment an dieser Stelle
     und hinter Borlands _STACK ein zweites Stacksegment von 0x1000 Bytes
     (daher SP=0x1000 im Header und 4 KB mehr Image).
  7. TLINK.RES: SEG013 hinter SEG001 (CD-Daten gehoerten zum CD-Modul), Name BLADEM.EXE
  6. seg010.asm: EMS_norm_ptr ruft F_LXLSH@ (Borland-Long-Shift) auf. Bright Eyes
     hatte dort Platzhalter 'db 09ah,0,0,0,0' = call 0000:0000 -> Sprung an den
     Programmanfang. Jetzt echter Fernaufruf mit Fixup. SEG010 ist damit bytegenau.

Aufruf: datseg_patch.py <Arbeitsverzeichnis>
"""
import re, sys, os

W = sys.argv[1]
p = os.path.join(W, 'datseg.cpp')
lines = open(p, encoding='latin1').read().split('\n')
out = []
mode = 1          # 1 = Teil 1, 2 = Bibliothekskopie (weg), 3 = Teil 2 (BSS)
n_lib = n_bss = 0
for l in lines:
    m = re.search(r'// ds:0x([0-9a-fA-F]+)', l)
    off = int(m[1], 16) if m else None
    if l.strip() == 'char ds[1];':
        out.append('extern char ds[1];  /* Build-Patch 1 */'); continue
    if off == 0x447e and 'g_unkn_029' in l:
        out.append('unsigned _stklen = 0x2000; // ds:0x447e  /* Build-Patch 2: Attics Stackgroesse */'); continue
    if off is not None:
        if mode == 1 and off >= 0xb4e0:
            mode = 2
        if mode == 2 and off >= 0xbc38:
            mode = 3
    if mode == 2 and off is not None:
        n_lib += 1
        out.append('/* Build-Patch 3 (Bibliothekskopie entfernt): ' + l.split('=')[0].strip() + ' */')
        continue
    if mode == 3 and off is not None and '=' in l.split('//')[0]:
        code, comment = l.split('//', 1)
        code = re.sub(r'\s*=.*;\s*$', ';', code.rstrip())
        out.append(code + ' //' + comment)
        n_bss += 1
        continue
    out.append(l)
open(p, 'w', encoding='latin1').write('\n'.join(out))

a = os.path.join(W, 'seg013.asm')
s = open(a, encoding='latin1').read()
# Segmente behalten (die Overlay-Verwaltung zaehlt sie), aber ohne Inhalt
s2 = re.sub(r'dummy1_seg\s+segment[^\n]*\n\s*assume nothing\s*\n\s*db 0\s*\ndummy1_seg\s+ends\s*\n', '', s)
s2 = re.sub(r'dummy2_seg\s+segment[^\n]*\n\s*assume nothing\s*\n\s*db 0\s*\ndummy2_seg\s+ends\s*\n',
            "stack2_seg\tsegment para public 'STACK'\t; Build-Patch 5: 4 KB Stack wie im Original\n"
            "\t\tdb 1000h dup (?)\nstack2_seg\tends\n"
            "_STACK\tsegment para stack 'STACK'\t; Build-Patch 5: beide Stacksegmente gehoeren\n"
            "_STACK\tends\nDGROUP\tgroup _STACK, stack2_seg\t; im Original zu DGROUP\n", s2)
open(a, 'w', encoding='latin1').write(s2)
print(f'datseg.cpp: {n_lib} Bibliotheks-Kopien entfernt, {n_bss} Variablen ins BSS verschoben; '
      f'seg013.asm: Dummy-Segmente {"umgebaut (1 leer, 1 Stack)" if "stack2_seg" in s2 else "NICHT gefunden"}')

# 6. seg010.asm
a = os.path.join(W, 'seg010.asm')
s = open(a, encoding='latin1').read()
n6 = s.count('db 09ah, 0, 0, 0, 0')
s = s.replace("""\tdb 09ah, 0, 0, 0, 0
\t;call 0x0:0x0\t;F_LXULSH""", """\tdb 09ah\t\t\t; Build-Patch 6: far call ...
\tdd F_LXLSH@\t\t; ... auf F_LXLSH@ statt 0000:0000""")
s = s.replace('.model large\n.code', '.model large\nextrn F_LXLSH@:far\t; Build-Patch 6\n.code', 1)
open(a, 'w', encoding='latin1').write(s)
print(f'seg010.asm: {n6} Platzhalter-Aufrufe ersetzt')

# 7. TLINK.RES: SEG013 (CD-Daten) direkt hinter SEG001 linken - bei Attic gehoerten die
#    CD-Daten zum CD-Modul; sichtbar an der Reihenfolge der Relocations. Ausgabe als
#    BLADEM.EXE (TLINK speichert den Namen in der Overlay-Tabelle; Original: grossgeschrieben).
a = os.path.join(W, 'TLINK.RES')
s = open(a, encoding='latin1').read()
s2 = s.replace('SEG001 SEG002 SEG003 SEG004+', 'SEG001 SEG013 SEG002 SEG003 SEG004+', 1)
s2 = s2.replace('SEG009 SEG010 SEG011 SEG013+', 'SEG009 SEG010 SEG011+', 1)
lines = s2.split('\n')
for k, l in enumerate(lines):
    if l.strip() == 'BLADEM':
        lines[k] = l.replace('BLADEM', 'BLADEM.EXE'); break
s2 = '\n'.join(lines)
open(a, 'w', encoding='latin1').write(s2)
print('TLINK.RES:', 'SEG013 nach SEG001, Ausgabe BLADEM.EXE' if s2 != s else 'NICHT geaendert')
