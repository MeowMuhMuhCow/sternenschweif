# Abschlussbericht: Sternenschweif aus dem Quellcode

Stand 19.10.2026, abends. MooMoo mit Claude.

## Ergebnis

Aus dem rekonstruierten Quelltext baut Borland C++ 3.1 eine SCHWEIF.EXE, die **Byte für Byte dem Original gleicht**.
Das Original ist die deutsche Fassung C1.02 (GOG), 1.387.552 Bytes, MD5 `da3b59ce86a82e4b8bb14d36ed37d4d0`.
Die EXE wird nicht nachbearbeitet. Sie entsteht allein durch Compiler, Assembler und den Linker der Entwicklungsumgebung.

| Teil | Endstand |
|---|---|
| Module | 317: 46 residente Segmente, 271 Overlays |
| in C/C++ | **301** (237 C++, 64 C), rund 119.000 Zeilen. Das ist alles, was Attic in C geschrieben hat |
| in Assembler | 16, so wie Attic sie geschrieben bzw. zugekauft hat. Alle sind benannt und kommentiert, zusammen rund 8.700 Zeilen |
| Daten | Jedes C-Modul definiert seine Daten selbst in C (145 Module mit Daten). Assembler-Daten gibt es nur noch für die Assembler-Module |
| Funktionen | **alle 3.215 benannt** |
| Datenobjekte | **alle 1.941 benannt**. 163 tragen einen Zusatz, der doppelte Texte oder Kartenpositionen unterscheidet. Bei 116 weiteren steht der Text direkt im Code, ein eigener Name ist dort nicht nötig |
| Header | `schweif.h` mit 694 Konstanten, 208 Strukturen, 97 Klassen, Daten und Prototypen |
| Fehler im Original | **243** dokumentiert (36 schwer, 113 mittel, 94 leicht), bewusst so nachgebaut |
| Spieltexte | 9.639 Texte (114 LTX-Dateien) und 3.483 Dialogzeilen (82 TLK-Dateien) |
| Übersicht | `Analyse/Quelltextkarte.html`: alle Module nach Spielbereich, mit Funktionen und Fehlern |
| Starten | `SCHWEIF_STARTEN.bat` startet die gebaute `Build/SCHWEIF.EXE` mit den GOG-Daten. Spielstände landen getrennt in `Build/Testspielstaende` |

**Grundsatz:** Was Attic in C hatte, steht in C. Was Attic in Assembler hatte, bleibt Assembler. Jede Tabelle, die das Spiel
benutzt, steht lesbar im Modul, zu dem sie gehört. Der bytegleiche Build ist der Schiedsrichter: Wer etwas anders
aufschreibt, als Attic es konnte, bekommt eine andere MD5. Die Namen stammen von uns und folgen dem Stil von Bright-Eyes.
Attics eigene Bezeichner stehen nicht im EXE.

## Zeitleiste

| Datum | Schritt |
|---|---|
| 17.10. | Werkbank und Compiler-Eichung an DSA 1 (Bright-Eyes, bytegleich). Zerlegung von SCHWEIF.EXE, Compiler-Optionen je Modul |
| 18.10. | Komplettlink bytegleich, erst ganz aus Assembler, dann mit fast allen Modulen aus C/C++ |
| 18.–19.10. | 24 Namenswellen, Header `schweif.h` statt Adressnamen, Spieltexte, Fehlerliste |
| 19.10. | O030 gelöst (Compilerfehler). Die angeblichen Kopierschutz-Segmente in C. Datensegment je Modul in C. Abschlusswelle: alle Namen, die 16 Assembler-Module kommentiert, 3D-Engine verstanden |

## Was wo liegt

| Ordner | Inhalt |
|---|---|
| `Quelle/` | 32 residente Module in C/C++ und 14 in Attics Assembler (`SEGnnn.*`) |
| `Quelle/Overlays/` | 269 Overlays in C/C++, OVL191 und OVL270 in Assembler |
| `Quelle/inc/` | `schweif.h` mit den Teil-Headern |
| `Quelle/werkbank_stand.tar.gz` | kompletter Arbeitsstand für den Neubau |
| `Build/` | die gebaute `SCHWEIF.EXE` (MD5 wie das Original) |
| `Werkzeuge/` | Build, Modulliste `c_module.tsv`, Nachbearbeitung, Datensegment-Werkzeuge, Kartengenerator |
| `Analyse/` | Fehlerliste, Leitfaden, Steckbriefe (`module.json`), Quelltextkarte, Berichte der Wellen, Namen, Spieltexte |
| `Referenz/` | Prüfmaterial: Assembler-Fassungen der C-Module, das alte Datensegment, Borland-Bibliotheken, Archiv |

## Wie gebaut wird

`Werkzeuge/build.sh` arbeitet in zwei Stufen. Ein Lauf dauert knapp 10 Minuten.

| Stufe | Was passiert | Prüfung |
|---|---|---|
| 1 | Alles wird aus Assembler gebaut (TASM 3.1), dazu ein Datenstück je Modul. Gelinkt wird mit dem Linker der IDE (BC.EXE) | muss bytegleich sein |
| 2 | Die 301 C-Module werden mit BCC 3.1 übersetzt, mit den Optionen je Modul aus `c_module.tsv`. `cobjfix_d.py` ordnet Publics und Fixups wie im Original, die Module bringen ihre Daten selbst mit (`dscheck.py`). Danach wird neu gelinkt | „BYTEGLEICH mit dem Original“ |

## Was wir gefunden haben

### Über Attics Arbeitsweise

1. **Attic hat C++ benutzt**: Klassen, Konstruktoren, globale Objekte. 237 von 301 Modulen sind C++.
2. **Gelinkt wurde in der IDE** (Projekt `SCHW_CD.PRJ`, 15.02.1995).
3. **Die Compiler-Optionen unterscheiden sich je Modul.** Gemeinsam ist `-mlarge -3 -Ot`, dazu kommen je Datei `-Og`, `-Oe`, `-Oi` oder `-Ol`.
4. **Jedes C-Modul trug seine Daten selbst.** Das Datensegment ist im Original verzahnt, die Daten folgen der Link-Reihenfolge. Belegt ist das durch die Relocation-Tabelle.
5. **Der Inline-Assembler im C-Code ist original.** Er steht in 14 Dateien (311 Zeilen): Tastatur-Interrupt, MSCDEX, EMS, Entpacker. Belegt ist das durch die TASM-Kodierung der Befehle.
6. **Attic hatte im Assembler `JUMPS` eingeschaltet.** Das verrät ein `jecxz` mit fünf Leerbefehlen.
7. **Die globalen Objekte liegen im Hauptmodul O032**, darunter auch die Merker des Kopierschutzes, zusammen mit `main()`.

### Über den Compiler (neu)

1. **BCC 3.1 liest nicht initialisierten Speicher.** Bei der Registerwahl für `switch` greift der Compiler auf einen Heap-Bereich zu,
   den er nie beschreibt. Das Ergebnis hängt davon ab, welche Datei vorher übersetzt wurde. Bei Attic hing es an der Reihenfolge in
   der IDE. Abhilfe: Vor O030 wird ein Speicherbild mit genau diesen zwei Bytes geladen (`frisch.txt`, `heapbild.py`, `FILL.ASM`).
2. **Im C-Modus liegt BSS rückwärts**, in umgekehrter Reihenfolge der *ersten* Deklaration. C++ behält die Definitionsreihenfolge.
3. **TASM 3.1 setzt die Gruppen-Fixup im Interrupt-Prolog** anders als das Original. `cobjfix.py` ordnet sie um.
4. Alle Muster stehen in `LEITFADEN_C.md`, 66 Abschnitte.

### Die 3D-Engine (SEG024, SEG025)

Die 3D-Engine ist handgeschriebener 386-Assembler. Sie rechnet in 32-Bit-Registern mit Festkommazahlen, was Borland C++ 3.1
nicht konnte. **Wände werden spaltenweise gefüllt, Böden und Decken zeilenweise.** Der Grund: Entlang einer Wandspalte und
entlang einer Bodenzeile ist die Tiefe konstant, die Perspektive muss also nur einmal je Linie gerechnet werden.
Die Pixelschritte sind 134- bzw. 208-fach ausgeschrieben, der Einsprung läuft über eine Sprungtabelle. Für EMS schreibt sich
der Code selbst um. Im Quelltext stehen die Wiederholungen jetzt als `rept`-Makro (SEG024: 10.011 → rund 1.550 Zeilen).
Eine ausführliche Erklärung steht in `Analyse/Bericht_3D_Engine.md`. Einen Hinweis auf eine Fremdbibliothek gibt es nicht,
alles spricht für Attics eigene Arbeit.

### Die Audio-Bibliothek (SEG022)

SEG022 ist die **Miles Audio Interface Library 2.14 vom 24.09.1992**, zugekauft. Die Namen folgen Miles' heute frei verfügbarer Quelle.
Bericht: `Analyse/Bericht_Assembler_AIL_und_Helfer.md`.

### Der „Kopierschutz“

| Segment | wirklich |
|---|---|
| SEG001 | Spielschleife (`main_loop`, Reisemodus, Leerlaufschritt, Speichern) |
| SEG003 | Handbuchabfrage (Klasse `CopyProt`) und Aufräumen beim Programmende |
| SEG023 | Zeitgeber und Maus |
| SEG005 `cd_check` | CD-Prüfung beim Start (PATH.CFG, MSCDEX, Datenträger DSA2_CD1/DSA2_CD2) |

- **Die Handbuchabfrage wird in der CD-Fassung nie aufgerufen.** Geschützt hat nur die CD-Abfrage.
- **Attics stille Strafe für Cracker:** Der Kampf-Zeitgeber bildet alle 250 Takte eine Prüfsumme über den Schutzcode und prüft die
  Bytemuster vor den Aufrufen. Bei Manipulation zählt ein Zähler hoch, ab über 50 verlieren die Helden im Kampf Lebenspunkte.

Die Prüfungen arbeiten im Nachbau unverändert. Einen Schalter zum Abschalten gibt es nicht.

### Fehler im Original

243 Einträge in `Analyse/Originalfehler.md`. Die Nummern 1–233 betreffen den C-Teil, 300–307 die Assembler-Helfer und die Audio-Bibliothek,
350–351 die 3D-Engine. Beispiele:

| # | Wo | Was |
|---|---|---|
| 1 | Kampf, Patzer | `else` am falschen `if`: Die Patzerfolgen 9–12 treffen den Angreifer nie |
| 20 | Händler | Passt ein Posten nicht ins Gepäck, zahlt man trotzdem den vollen Preis |
| 25 | Gruppengeld | Geld ist vorzeichenlos: Wer mehr abzieht, als vorhanden ist, ist plötzlich reich |
| 233 | Wegereignis Kriegerhüne | Ein Zusatztext erscheint nie, weil sein Merker nie gesetzt wird |
| 306 | Flat Real Mode | Ein Fehlschlag wird ignoriert, unter EMM386 droht vermutlich ein Absturz |
| 350 | 3D-Engine, Böden | Ohne EMS entstehen an einer 64-KB-Grenze einzelne falsche Bodenpixel |

### Korrigierte eigene Irrtümer

| Irrtum | Richtig |
|---|---|
| 3.047 Overlay-Einsprünge | 2.775 |
| SEG001/003/023 = Kopierschutz | Spielschleife, Handbuchabfrage, Zeitgeber |
| SEG001 = CD-Audio wie in DSA 1 | CD-Audio liegt in O004 |
| Kandidat für einen Lesefehler in O140 | in Wahrheit eine Tabelle mit festem Versatz (`g_fight_defs_ab[id - 0xab]`) |

## Bewusst offen

| Was | Warum |
|---|---|
| Besitzer der 57 Kampf-Variablen (ds:c948..cab8) | Lage und Zugriffe lassen keine Entscheidung zu. Zugeordnet ist O128 (Kampfrunde), im Quelltext als **vermutet** markiert, bytegleich bei jedem Kandidaten |
| OVL030 mit Rohnamen | Seine Übersetzung hängt vom Compiler-Heap ab. Die Daten sind in ihren Besitzermodulen benannt |
| Einige Namen mit niedriger Sicherheit | als „vermutet“ markiert |

## Nächste Schritte

1. **Bau-Paket für Windows** (in Arbeit): ein Doppelklick, und am Ende steht „BYTEGLEICH“.
2. **FIX_BUGS-Fassung:** `#ifdef FIX_BUGS` an den Fehlerstellen. Der Standard bleibt bytegleich, die Fassung muss angespielt werden.
3. **Schatten über Riva** ab 22.10. (`Q 006`, `HOWTO_Schatten_ueber_Riva.md`), danach DSA 1 auf denselben Stand.
4. **Übergabe an den Rechteinhaber**, vor jeder Veröffentlichung. Den Borland-Compiler nicht weitergeben.
