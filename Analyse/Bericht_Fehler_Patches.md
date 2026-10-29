# Bericht: Fehlerbehebungen per Schalter (Fix-Bau)

Stand 19.10.2026, Arbeitsbaum `/root/zw_f`. Der Hauptbaum blieb unverändert.

## Ergebnis in Kürze

| Punkt | Ergebnis |
|---|---|
| Standardbau (`FIX_BUGS=0`, Vorgabe) | **BYTEGLEICH mit dem Original** (MD5 da3b59ce86a82e4b8bb14d36ed37d4d0), Vollbau mit allen Schaltern im Quelltext, Stufe 1 und Stufe 2 |
| `SCHWEIF_FIX_WICHTIG.EXE` (`FIX_BUGS=1`) | 1 389 584 Bytes (Original 1 387 552), MD5 83e8f53d9d46442121624fdc2423202a, fixcheck IN ORDNUNG |
| `SCHWEIF_FIX_ALLES.EXE` (`FIX_BUGS=2`) | 1 390 848 Bytes, MD5 784c89e572898752068b3f610f56f5f2, fixcheck IN ORDNUNG, Neubau ergibt dieselbe EXE |
| Nr. 20 und Nr. 84 | Kategorie *kritisch*, darum **in WICHTIG enthalten** (`FIX_0020`/`FIX_0084` = Stufe 1) |
| Windows-Paket | `BAUEN.bat wichtig` / `BAUEN.bat alles`; der Fix-Teil von `bauen.py` lief unter Linux und ergab dieselbe WICHTIG-EXE (gleiche MD5) |

## Umfang

**Ursprüngliche Liste (243 Nummern):** 10 sind kein Fehler oder gewollt, 233 haben einen Schalter.

| Stufe | ja | teilweise | nein |
|---|---|---|---|
| wichtig | 80 | 15 | 3 (117, 177, 233) |
| alles | 101 | 13 | 21 |

**Neue Nummern 352–376** (bestätigte Spielermeldungen): Sie kamen während der Arbeit dazu und waren schon eingebaut,
als die Anweisung kam, sie offen zu lassen. Darum sind sie in beiden EXEs enthalten: 14 ja, 2 teilweise (352, 356), 9 nein.
Kritisch: 352 Lagerhaus (teilweise), 361 Asthenilring/Wasserschlauch, 362 Schlüsselbund, 363 leere Gruppe, 364 Horriphobus
(unsicher: ohne Fluchtweg endet die Angst), 369 Kräuterwirkungen, 375 Lowangen-Schlüssel (unsicher). Mit `-DFIX_0352=0` usw.
lassen sie sich einzeln abschalten. Der Nebenbefund O099 (NSC über `heroes[6]`) steckt in `FIX_0352`. Die Erweiterung von
Nr. 180 ist nicht umgesetzt: `g_blackmage_done = 1` bedeutet „die Magier haben den Stein“ (O264 lässt dann die Leiche mit dem
Stein finden). Den Merker nach einem Sieg zu setzen, ergäbe einen zweiten Stein. Nr. 360 ist offen.

Die vollständige Tabelle mit Nummer, Titel, Kategorie, Exploit, Stufe, umgesetzt und Grund steht in `FIXES.md`.

## Was nicht umgesetzt ist und warum (Auswahl)

- **Kein Platz im residenten Segment:** Residente Segmente dürfen nicht wachsen, weil sie dicht vor DGROUP liegen und O011
  mit festen Offsets über SEG003 prüft. Betroffen sind Nr. 3 (SEG012, +2 Bytes), Nr. 210 (SEG005, +21 Bytes), Nr. 217
  (SEG041), Nr. 302 und 305 (ASM) sowie Nr. 350 (SEG024: der abgerollte Pixelschritt würde um Hunderte Bytes wachsen).
  Für Nr. 209, 230, 231 (SEG005) und 369 (SEG002) wurde unter demselben Schalter toter Code entfernt, ohne das Verhalten zu
  ändern: in SEG005 eine doppelte Prüfung und ein überschriebenes `size = 8`, in SEG002 die nie aufgerufene Funktion
  `check_palette`.
- **Absicht nicht belegbar:** Nr. 23, 28, 113, 177, 221, 233, 100, 66, 367.
- **Kein Fehler oder ohne Wirkung:** Nr. 175 (Pronomen richtig), 205 (Text ohne `%s`), 19, 64, 69, 117 (wie in DSA 1),
  225, 226, 306.
- **Spielstandformat oder Datensegment müsste sich ändern:** Nr. 355, 358/359 (Lager und Schmied geben neue Gegenstände
  aus), 365 und 376 (Spieldaten).
- **Teilweise:** Bei 27 Nummern ist nur der eindeutige Teil behoben. Die Gründe stehen in `FIXES.md`.

## Technik

- `names/inc/s_fix.h` und `s_fix.inc` (TASM) erzeugt `tools/mkfix.py` aus dem Raster, der Exploit-Liste, der Netzliste
  und `abschluss/fix_titel.tsv`. Stufen: `FIX_BUGS 0/1/2`, `FIX_EXPLOITS` (Vorgabe 1 ab Stufe 1). Jeder Schalter
  `FIX_nnnn` lässt sich übersteuern. Die Header-Tabelle ist ASCII.
- Im Code steht `#if FIX_nnnn` mit Kommentar `/* Fix #n: … */`, dann die neue Fassung, `#else` und das Original.
  Strukturen, Spielstandformat und Datensegment bleiben unverändert: keine neuen Literale oder Globalen. dscheck prüft
  alle D-Module, fixcheck das ganze Datensegment.
- ASM: `IF FIX_0300/0301/0304/0307/0351 … ELSE … ENDIF`, jeweils gleich lang oder ausgeglichen.
- Bau: `FIX=1 FIXFLAGS="-DFIX_BUGS=2" bash tools/build.sh`. Der Standardbau läuft unverändert und muss bytegleich sein.
  Danach folgt `tools/build_fix.sh`, das auch allein nach einem Standardbau läuft:
  1. Nur die Quellen mit Schaltern übersetzen.
  2. `cobjfix --ref` (Namen aus der FIX_BUGS=0-Fassung, neue Externe über `tools/cnamen.py`, jetzt auch aus den
     Fixups aller Module).
  3. `tools/segpad.py` füllt residente Module auf die Originallänge auf.
  4. TASM mit `/dFIX_BUGS=…`.
  5. Link in `lnkfix/` und `fixcheck`.
  Neu sind `tools/objsame.py` (Vergleich von Bytes und Fixup-Zielen) und `tools/segpad.py`.
- Geänderte Funktionen laut Disassembler-Diff (Fix gegen FIX_BUGS=0, ohne Fixup-Felder):
  - WICHTIG: 187 Funktionen in 97 C-Modulen, kein ASM.
  - ALLES: 370 Funktionen in 181 C-Modulen, dazu 6 ASM-Prozeduren in 5 Modulen.
  - Die Zahlen enthalten auch Funktionen, die sich nur durch verschobene Sprungziele unterscheiden.
  - Einzeln aufgelistet in `funktionen_wichtig.tsv` und `funktionen_alles.tsv`.
- fixcheck für beide EXEs: Die 48 residenten Segmente liegen an derselben Stelle, das Datensegment ist gleich, alle 271
  Overlays und alle 2775 Einsprünge sind vorhanden und haben passende Prologe.

## Test in DOSBox (Cloud, ohne Spieldaten)

`tools/exerun.sh` startet die EXE mit PATH.CFG und einem leeren ISO mit dem Datenträgernamen DSA2_CD1. Gemessen wurden
die Dateizugriffe, die IP-Spur, Sprünge ins Leere `[FAR]` und Division durch 0 `[DIV0]`.

- Original, WICHTIG und ALLES laufen denselben Weg: cd_check, Overlay-Manager mit XMS, `star.dat`, `filename.ltx`,
  `fx.dat` … bis zur fehlenden Spieldatei.
- Unterschiede gibt es nur bei den Lesegrößen der Overlays (die Overlays sind größer) und bei der Zahl der wiederholten
  DIV0-Meldungen am Ende, die vom Takt abhängt. Diese DIV0-Schleife zeigt auch das Original ohne Spieldaten.
- Kein Hänger beim Start, kein Absturz beim Laden oder in den Overlays.
- Spiellogik konnte ohne die GOG-Daten nicht getestet werden.

## Bitte im Spiel prüfen (nach Wichtigkeit)

1. **Start, Laden, Speichern** mit beiden Fix-EXEs und einem alten Spielstand. Das Format ist unverändert.
2. **Orkfestung, Gefangennahme (212/6):** Ausrüstung muss in den Truhen 0x34/0x35/0x36 liegen.
3. **Händler und Laden (20, 84):** Mehr kaufen als ins Gepäck passt. Es darf nur die gelieferte Ware bezahlt werden.
4. **Geld (25, 356):** Nicht unter 0 fallen lassen (Spende, Gebühren). Gruppe teilen und vereinen mit NSC.
5. **Lagerhaus (352):** Einlagern, mehrere Tage warten, Gebühr wird abgebucht, Abholen. Der Warenwert darf sich nicht
   verdoppeln. Einlagern beim NSC in einer kleinen Gruppe.
6. **Schlüsselbund (362, 14)**, **Asthenilring/Wasserschlauch (361)** und **Lowangen-Schlüssel (375)**, jeweils mit vollem
   Rucksack.
7. **Kräuter (369):** Viele Wirkungen gleichzeitig, z. B. drei Atmonblüten, im Heldenbogen.
8. **Leere Gruppe (363):** Einzelheld beim Geldscheißer, im Binge-Schacht und beim Stadtereignis „weggelockt“.
9. **Horriphobus (364):** Der Kampf muss enden.
10. **Reise (5, 8, 196, 170, 124):** Ereignisse, Begegnungen, Treffen mit anderen Gruppen, Routenplanung.
11. **Bolzenfalle Phex-Tempel (192)**, **Orkgefängnis-Unrat (199/218)**, **Zwergenmutprobe (206)**.
12. **Zauber mit AE-Grenze (159, 166, 173):** Exploits, die mit `FIX_EXPLOITS=0` erhalten bleiben.

## Abgabe (`/root/zw_f/abgabe`)

- Patches `-p1` gegen den Ausgangsstand, getestet mit `patch` auf einer Kopie:
  - `01_csrc.patch` (181 Dateien)
  - `02_names_inc.patch` (s_fix.h, s_fix.inc, schweif.h)
  - `03_tools.patch` (build.sh, build_fix.sh, cobjfix*, cnamen, mkfix, segpad, objsame, fixcheck, exerun, mkiso, mk_baupaket)
  - `04_asm.patch` (l22, l25, l35, l39, l46)
  - `05_abschluss.patch` (fix_titel.tsv, fix_schalter.tsv)
- `SCHWEIF_FIX_WICHTIG.EXE`, `SCHWEIF_FIX_ALLES.EXE`, `FIXES.md`, `funktionen_*.tsv`
- `baupaket/`:
  - `bauen.py`, `BAUEN.bat` (wichtig/alles, mit SPIELPFAD_SUCHEN.cmd), `BAUEN.md`
  - `SCHWEIF_FIX_WICHTIG_STARTEN.bat`, `SCHWEIF_FIX_ALLES_STARTEN.bat`: kopieren die EXE als SCHWEIF.EXE nach
    `Build\Fix_*` und legen Spielstände in `Build\Testspielstaende_Fix_*`
  - `werkzeuge/`, `baustand.tar.gz`
  - Der Generator `tools/mk_baupaket.py` erzeugt jetzt BAUEN.bat (mit dem Aufruf von SPIELPFAD_SUCHEN.cmd) und die beiden
    Startdateien selbst.

Hinweis: Nach dem letzten Vollbau hat `mkfix.py` die Tabelle in `s_fix.h` noch einmal neu geschrieben. Geändert haben sich
nur Kommentare (Spalte „umgesetzt“ bei 180 und 360), die Schalterwerte nicht.
