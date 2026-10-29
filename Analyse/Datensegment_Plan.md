# Daten zurück in die Module – Analyse und Umbauplan (dsana, 19.10.2026)

Nur Analyse. Build, csrc, tools und lnk sind unverändert. Kein Build gestartet. BCC lief nur für Testdateien und den Piloten in `dsana/t1` und `dsana/pilot`.

## 1. Kernbefunde

| Befund | Beleg |
|---|---|
| **Original-Linkreihenfolge: Residente und Overlays verzahnt.** Die Overlays O000..O270 und die Residenten R01..R46 stehen jeweils in Nummernfolge, sind aber ineinander geschoben (Projektliste der IDE, Overlay-Bit je Knoten). Das DS ist die Folge der `_DATA`-Beiträge in genau dieser Reihenfolge. | `ds_linkorder.tsv`. Prüfung: Code-Relocations der Residenten, DGROUP-Relocations und `_INIT_`/`_EXIT_`-Einträge in dieser Reihenfolge simuliert ergeben die MZ-Tabelle des Originals **identisch** (417/417 Token). |
| **BCC schreibt `_DATA` in LEDATA-Blöcken zu 1024 Bytes ab Modulanfang.** Ein Block wird nur gekürzt, wenn eine Fixup die Grenze schneiden würde. Fixups stehen je Block absteigend, Code-Relocations vor den Daten-Relocations, danach `_INIT_` und `_EXIT_`. | Test `t1/T3.C`, `t1/T4.C` (BCC 3.1). Damit verraten die absteigenden Läufe in der MZ-Tabelle die Modulanfänge modulo 0x400. |
| **O032 (Hauptmodul, `main`) definiert die globalen Daten.** DATA 0x1966..0x3c5e (8.952 Bytes) und BSS 0x8b96..0xc776 (15.328 Bytes). | Alle 7 Relocationläufe von Gruppe 1 ab 0x1bd8 passen exakt auf Blöcke ab 0x1966. Die Grenze 0x1d64..0x1d68 lässt nur diesen Anfang zu. Die Tabellen 0x32cb und 0x3587 zeigen auf O032-Literale. `_stklen`/`_ovrbuffer` liegen bei 0x39a0. `_INIT_`/`_EXIT_` von O032 folgen direkt danach. Im BSS stehen davor O002..O024, R01, O030 und dahinter O034, O036, O038 … in Linkreihenfolge. |
| **Die asm-strcat-Literale sind echte Modul-Literale.** | O238 " " bei 0x75c6, O111 bei 0x62e2 (je 2 Bytes, eigener Modulblock). O106 0x613e..0x618a und O068 0x5818..0x5826 sind reine Literalpools. **Pilot O238 aus C mit eigenem Literal: Code 2492/2492 Bytes GLEICH, `_DATA` = `20 00`.** |
| Initialisierte lokale Aggregate haben ihre Vorlage im eigenen `_DATA`. | O236 `struct t9 tab = g_75b4` → 0x75b4..0x75c6 gehört O236. |
| Die globalen C++-Objekte der Startup-Module liegen in deren Daten. | O224 `Tm g_cb2e` = BSS 0xcb2e..0xcb34 (6 Bytes). Das passt zur Notiz in OVL224.CPP. |
| Zwischen Modulen sind Zeiger nur auf globale Daten erlaubt. | 165 DGROUP-Fernzeiger zeigen ins eigene Modul. Die übrigen 21 zeigen auf O032-Globale oder BSS, nie auf fremde Literale. |

## 2. Zuordnungsgrad (`ds_owner.tsv`)

| Bereich | Bytes | sicher | wahrscheinlich | unsicher |
|---|---|---|---|---|
| DATA 0x0094..0x86b6 | 34.338 | 25.311 (73,7 %) | 4.788 (13,9 %) | 4.239 (12,3 %) |
| davon ohne O032 | 25.386 | 16.359 (64,4 %) | 4.788 (18,9 %) | 4.239 (16,7 %) |
| BSS 0x8b4e..0xcb76 | 16.424 | 311 | 15.337 (O032-Block) | 776 |

- 147 Module haben eigenes DATA, 42 eigenes BSS, 4.511 Bytes sind erkennbare Literalpools. 69 von 146 DATA-Modulgrenzen sind beidseitig sicher.
- **„unsicher“** heißt: der Bereich gehört einem der beiden Nachbarmodule. Entweder fehlt ein Beleg (Grenzzone), oder die Varianten der Zuordnung widersprechen sich (9 Läufe mit unterschiedlicher Gewichtung, `var/`). Größte offene Zonen: 0x62e4..0x684b (O116..O140), 0x1222..0x153c (O024/O025), 0x70b6..0x7204 (O212..O216), 0x1542..0x163d (O026), Rand 0x3dc6..0x3e6e (O035/O036).
- **Verwaiste Daten:** keine. Jedes Byte des Spiel-DS liegt in einem Modulblock. Nicht von C aus definierbar sind nur die Daten der **ASM-Module**: R24 3.338 Bytes, O030 232, R27 50, R46 16, R01 6+12 BSS, R23 8, R39 6, R26 4, R36 4, R03 2+2. Die gehören später ins jeweilige ASM-Modul. Das RTL-DS (0x0000..0x0093, ab 0x86b6, BSS ab 0xcb76) bleibt unberührt.

Methode (`zuordnung.py`): dynamische Programmierung über die Verzahnung von R und O. Punkte gibt es für Verweise des Eigentümers: exakte Verweise der C-Module aus den linkfertigen OBJ (`refs_fix.tsv`, Snapshot 19.10. 00:44), für die ASM-Module die Disassembly. Harte Bedingungen:
- gerade Modulanfänge
- kein Wechsel mitten in einem Text
- DGROUP-Gruppen zwischen den Residenten
- `_INIT_`-Lage von O008/O032/O224
- ein BCC-Block gehört einem Modul
- `festlegungen.tsv` (Handprüfung mit Begründung)

BSS nutzt die Reihenfolge-Schranken aus DATA. `blocksim.py` prüft das Ergebnis gegen alle 381 DGROUP-Fixups: Reihenfolge identisch.

## 3. Machbarkeit

| Frage | Ergebnis |
|---|---|
| Reihenfolge | Passt. Jedes Modul hat genau einen zusammenhängenden Block, und die Blöcke liegen in einer Reihenfolge, die beide Nummernfolgen wahrt. **Voraussetzung ist die verzahnte Linkreihenfolge.** Solange alle Overlays hinter allen Residenten gelinkt werden (heutiges `FULL.RES`), landet eigenes Overlay-`_DATA` am DS-Ende. Ein einzelnes Overlay umzustellen ändert die Overlay-Nummerierung. |
| Lücken/Füllbytes | Keine Lücken. Alle Blöcke beginnen gerade. Ein ungerades Modulende wird durch ein 00 aufgefüllt, das in der Tabelle beim Vorgänger steht. TLINK erzeugt es selbst, wenn das C-Modul sein `_DATA` wortausgerichtet liefert. |
| Gemeinsam genutzte Daten | Fast alles liegt in O032. Andere Module greifen per `extern` darauf zu (wie heute). In den übrigen Modulen sind fremd benutzte Globale die Ausnahme. |
| `#pragma startup`/`_INIT_` | O008, O032 und O224 schreiben ihren `_INIT_`-Eintrag (O032 auch `_EXIT_`) direkt hinter die eigenen Daten-Relocations. Das passt zur BCC-Satzfolge, wenn das Datenstück bzw. die C-OBJ direkt hinter dem Modul liegt. Echte globale C++-Objekte (statt nachgebildeter Initialisierer) sind möglich, müssen aber die Initialisiererfunktion bytegleich erzeugen. Bis dahin trägt das Datenstück den `_INIT_`-Eintrag. |
| Overlays mit Daten in DGROUP | Normal für VROOMM. Ihre Daten-Relocations stehen in der MZ-Tabelle an der Linkposition des Overlays. Genau das zeigt das Original. |
| Reihenfolge innerhalb eines Moduls | BCC: initialisierte Globale und Statics in Definitionsreihenfolge, danach der Literalpool in Reihenfolge des Auftretens. Gleiche Literale werden nicht zusammengelegt (kein `-d`). BSS in Definitionsreihenfolge, kein COMDEF. Byteausrichtung (`-a-`), deshalb Wörter auf ungeraden Adressen (O032-BSS). |
| Externe Code-Zeiger in Daten | Tabellen mit `dd F_Oxxx_yyyy` (O032, O048, O078, O108, O258, R12, R39). In C heißen die Ziele `_F_...` bzw. C++-dekoriert. cobjfix muss Fixups im `_DATA` genauso umbenennen wie im Code. |

## 4. Umbauplan

### Stufe D1 – eigene Daten im C-Quelltext, Link bleibt zentral (sofort möglich, kleiner Eingriff)
- `cobjfix.py`: Hat die C-OBJ eigenes `_DATA`/`_BSS`, holt es die Modulbasis aus `dsana/ds_module.tsv`. Dann prüft es die LEDATA Byte für Byte gegen das Original-DS an dieser Basis (Fixups im `_DATA` aufgelöst). Fixups auf `SEG _DATA`/`_BSS` +v werden auf das DGROUP-Public an Basis+v umgeschrieben (wie heute für feste Zahlen: nächstniedrigeres Public mit Inline-Korrektur). Danach werden `_DATA`/`_BSS` geleert und byteausgerichtet, eigene PUBDEFs entfallen zugunsten der zentralen.
- `ccmp.py`/`cvsasm.py`: „EIGENE DATEN“ ist kein Fehler mehr, wenn die Bytes zur Modulbasis passen. Meldung `GLEICH (+n Bytes eigene Daten geprüft)`.
- Gewinn: Die asm-strcat-Stellen (O238, O111, O260, O106, O068) werden zu normalem C, ebenso Vorlagen und Statics. Das DS bleibt zentral, das Risiko ist minimal.

### Phase A – verzahnte Linkreihenfolge mit Datenstücken je Modul (ohne C-Änderung, muss bytegleich sein)
- Neues Werkzeug nach Vorbild `dsana/dsmod.py` (Prototyp, erzeugt `phaseA/`): je Modul `DM<modul>.ASM` mit `_DATA` + `_BSS` (+ `_INIT_`/`_EXIT_`). Dazu `dm.json` mit der BCC-Blockreihenfolge für `objorder.py` und `FULL.RES` in Originalreihenfolge (`/o+`/`/o-` je Modul, Stück direkt hinter dem Modul).
- `build.sh`: `dssplit.py` und `DSBSS` entfallen. Die DM-Stücke werden assembliert (`/ml /m2`) und mit `objorder.py … dm.json DM<modul>` geordnet. `mkprj.py` übernimmt die Verzahnung aus `FULL.RES` (kann `/o+`/`/o-` schon).
- `INIT.ASM` und `FARDATA.ASM` bleiben vorerst hinter R46. Später kommen die Far-Variablen in O024/O036.
- Erwartung: bytegleich. Damit ist die Linkreihenfolge bestätigt. Danach gilt: ein Stück darf nur noch schrumpfen.

### Phase B – Modul für Modul „Daten zurück“
- `c_module.tsv`: neue dritte Spalte `D`, wenn das Modul seine Daten selbst definiert. `build.sh` lässt dann `DM<modul>` weg (bzw. erzeugt ein Reststück, falls das Modul nur einen Teil definiert, z. B. vorn liegende Globale aus unsicherer Zone). cobjfix behält eigenes `_DATA`/`_BSS` (wortausgerichtet), mappt nur EXTDEFs.
- Prüfung pro Modul (neues `dscheck.py`):
  - `_DATA`-Bytes der C-OBJ = Original-DS ab Modulbasis
  - Länge = Blocklänge (ggf. ohne Füllbyte)
  - `_BSS`-Länge
  - simulierte Relocation-Reihenfolge = Original (`blocksim.simulate`)
  - Gesamtlink bytegleich
- Reihenfolge der Arbeit:
  1. Module mit reinen Literalpools und sicherer Zuordnung (O068, O106, O111, O238, O260, O062, O071 …)
  2. Module mit Tabellen/Fernzeigern
  3. unsichere Zonen: hier entscheidet der Link, welches Nachbarmodul die Bytes liefert
  4. O032 mit generierter Definitionsliste (~1.000 DATA- + ~800 BSS-Objekte in Adressreihenfolge, Typen für die von O032 selbst benutzten)
  5. ASM-Module bekommen ihr Stück als eigenes `_DATA` in `SEGnnn.ASM`

## 5. Pilot: O238

| Teil | Stand |
|---|---|
| Quelle | `dsana/pilot/OVL238.CPP`: 12 Zeilen Inline-Assembler ersetzt durch `strcat(s + i * 40, " ");`, `extern g_75b0` entfernt. Original daneben als `OVL238.ORIG.CPP`. |
| Ergebnis | `BCC -mlarge -3 -Ot -Oi -Og -c -Y`: Code **2492/2492 Bytes, 0 anders** gegen die linkfertige OBJ. `_DATA` = 2 Bytes `20 00`. Fixup bei 0x04e3 = DGROUP/`SEG _DATA` +0 (vorher `_g_75b0`+0x16). |
| Aus dem zentralen DS zu entfernen | 0x75c6..0x75c8 (`20 00`), Ende von Item `_g_75b0` in `dsg/DS05.ASM` (heute in `DSP09`), siehe `pilot/O238_ds_entfernen.tsv`. Kein Public betroffen. |
| Linken | D1: cobjfix bildet das Literal auf `_g_75b0+16h` ab. Phase A/B: `DMO238` (genau diese 2 Bytes) entfällt, O238 liegt in `phaseA/FULL.RES` zwischen `DMO236`/`OVL237` und `OVL239`. |
| Nächste Kandidaten | O111 (0x62e2, gleiches Muster), O068 (Pool 0x5818..0x5826, `"^/^"` cx=4), O106 (Pool 0x613e..0x618a, `".GAM"` cx=5), O260 (Tabelle + Pool 0x784a..0x78d2, Fernzeiger → `dm.json`-Blockreihenfolge nötig). |

## 6. Risiken

| Risiko | Gegenmittel |
|---|---|
| Die IDE-Linkerin bzw. TLINK ordnet bei verzahnter Liste Segmente oder Overlay-Stubs anders an (Klassenreihenfolge, `_1STUB_`). | Phase A ist genau dieser Test, ohne C-Änderung. Abweichung sofort sichtbar (`exediff.py`). |
| Unsichere Zonen (12 % DATA, BSS der kleinen Module) falsch zugeordnet. | In Phase A unschädlich, weil nur die Reihenfolge der Stücke zählt. In Phase B fällt es auf, wenn ein Modul seine Bytes nicht erzeugt. Dann wandert die Zone zum Nachbarn (Reststück), `festlegungen.tsv` nachtragen und neu rechnen. |
| Verborgene Module ohne Code (reine Datendateien) zwischen zwei Blöcken. | Bisher kein Hinweis (O032 erklärt den großen Block, die Blocksimulation passt überall). Falls doch: Reststück an der Stelle. |
| Near-Zeiger im `_DATA` (`dw offset`) haben eine OMF-Fixup ohne MZ-Relocation und können einen BCC-Block kürzen, ohne dass man es sieht. | Bei Tabellenmodulen die Blockfolge mit `blocksim` gegen die eigene OBJ prüfen. |
| `-a`/Ausrichtung je Modul anders. | Bisher passt überall Byteausrichtung (ungerade Wortadressen in O032). Bei Abweichung `-a` testen. |
| O032 ist groß (24 KB Daten), Typen der Globalen beeinflussen den Code anderer Module nicht, wohl aber O032 selbst. | O032 zuletzt, Definitionen generieren (Byte-Arrays) und nur die von O032 benutzten typisieren. |
| Paralleles Arbeiten: Datenänderungen in csrc verschieben nichts, solange D1/Phase B die Byteprüfung erzwingen. | `dscheck.py` fest in `build.sh`. |

## 7. Dateien in dsana

| Datei | Inhalt |
|---|---|
| `ds_owner.tsv` | je Item/Teilstück: start, ende, modul, art (Literal/initialisiert/BSS), sicherheit, bemerkung (Name, Typ, Verweiser, Grund) |
| `ds_module.tsv` | je Modul: C/ASM, DATA- und BSS-Bereich, Literal-Bytes, sicher/unsicher |
| `ds_linkorder.tsv` | vollständige verzahnte Linkreihenfolge (317 Module) mit Daten je Modul |
| `festlegungen.tsv` | handgeprüfte feste Zuordnungen mit Begründung (O030, O032, O258/O260, O236, O238, O111, O032-BSS, O224-BSS) |
| `zuordnung.py`, `zuordnung.json`, `var/` | Zuordnung (DP) und 9 Parametervarianten für die Sicherheit |
| `blocksim.py`, `runs.py`, `relgroups.py`, `relseq.py` | BCC-Blocksimulation, Zerlegung der MZ-Relocations |
| `bericht.py` | erzeugt die drei Tabellen, Statistik und die Prüfung der Linkreihenfolge |
| `items.py`, `refs.py`, `fixrefs.py`, `refs*.tsv` | DS-Items aus dsg (bytegleich zum EXE geprüft), Verweise (OBJ-Fixups, Disassembly) |
| `dsmod.py`, `phaseA/` | Prototyp Phase A: 150 Datenstücke `DM*.ASM`, `dm.json`, `FULL.RES` (nicht gebaut) |
| `pilot/` | `OVL238.CPP` mit eigenem Literal, OBJ, `O238_ds_entfernen.tsv` |
| `snap/` | Snapshot der linkfertigen C-OBJ (`*.FIX`) und der MAP vom Build 19.10. 00:44 |
| `t1/` | BCC-Tests zur Blockbildung und Datenreihenfolge |

Neu erzeugen: `python3 refs.py && python3 zuordnung.py && python3 blocksim.py && python3 bericht.py && python3 dsmod.py` (Varianten siehe Kopf von `zuordnung.py`, Umgebungsvariablen PEN/WSTR/WEX/WSH/OUT).

## 8. Stand Phase A/B (19.10.2026, Arbeitskopie /root/sternenschweif)

| Teil | Stand |
|---|---|
| Phase A | `tools/dmgen.py` (Nachfolger von dssplit.py/DSBSS) erzeugt je Modul `lnk/DM<modul>.ASM` und die verzahnte `FULL.RES`; Reihenfolge der DGROUP-Relocations direkt aus der MZ-Tabelle (`dm.json`). **Bytegleich beim ersten Lauf** (Stufe 1 und 2). Linkreihenfolge damit bestaetigt. |
| Phase B | `c_module.tsv` Spalte 3 `D`. `tools/dsconv.py` schreibt die Daten in den Quelltext (Definitionen in Adressreihenfolge, Literale am Verwendungsort, Strukturen/Zeiger auf DGROUP automatisch), `tools/cobjfix_d.py --eigen` behaelt `_DATA`/`_BSS` (wortausgerichtet, PUBDEFs fuer alle Namen im Bereich), `tools/dscheck.py` prueft Bytes, Laenge, Relocation-Reihenfolge, Publics. `FULL2.RES` (Stufe 2) laesst die Stuecke der D-Module weg. |
| Werkzeuge | `dsinfo.py` (Bereiche inkl. `korrekturen.tsv`/`dm_rest.tsv`), `dsrefs.py` (wer benutzt was), `dsb.sh` (Einzeltest + Link in Sekunden), `dsprep.sh`, `dsall.sh` (alles neu anwenden nach Namenswelle), `dmark.sh`. |
| Zuordnung | Korrekturen: O260 endet bei 0x78c6 (0x78c6.. O263, 0x78cc.. O267), O091 0x5ca0..0x5cb2 (struct select_spell_tmpl 18 Bytes), BSS-Zone 0xc7ec..0xc80a (O046/R07/O052/O056 statt erfundener 2-Byte-Stuecke). `zuordnung.py`: BSS-Schranken nur noch fuer platzierte Bloecke (vorher zu streng). `korrekturen.tsv`: 0x56ec..0x56fc gehoert O056 (Zeigerliterale), R10 hat keine Daten (DP kann Residente ohne Daten nicht abbilden). |
| BCC-Erkenntnisse | Initialisierte Daten in Definitionsreihenfolge, Literalpool dahinter (auch Literale aus Initialisierern globaler Zeigertabellen, Reihenfolge des Auftretens). **C-Modus (.C): BSS in umgekehrter Reihenfolge der ERSTEN Deklaration** (auch der in s_data.h) - dsconv setzt dafuer extern-Zeilen vor die #include. C++ (.CPP): BSS in Definitionsreihenfolge. |
| Erledigt (Phase B, 19.10. abends) | Alle 19 offenen Module (O014, O025, O026, O032, O034, O041, O046, O048, O053, O078, O084, O086, O103, O108, O140, O224, O232, O258, R12) und dazu R01, R03, R23 definieren ihre Daten selbst (145 D-Module, Komplettbuild BYTEGLEICH). Loesungen: Auto-Aggregat-Vorlagen (O041, O053, O084, O103), 1-basierte Tabellen `tab[i - 1]` (O014, O046, O026, O053, O086, O232, R12, O032, O140 `[id - 0xab]`), Methoden-/Funktionszeigertabellen (O048, O078, O108, O258, O032), `_INIT_` im Modul (Spalte `DI`: O008, O224), Konstruktor-Objekte in O032 als Speicher unter dem Adressnamen. Leitfaden: Abschnitt „Phase B … schwere Faelle“. |
| Zuordnung (Phase B) | korrekturen.tsv: 0x5a72..0x5a80 O084 (Ende der Vorlage by_type), 0x5a0c..0x5a0e O078 (Fuellbyte), BSS 0xcb5a..0xcb6c O241 (g_tavern_crowd, g_cb5c), 0xcb6c..0xcb6e O262; R23-BSS beginnt bei 0xcb6e. R23-DATA 0x78d2..0x78da benutzt R23 selbst, R01-BSS 0x8b8e..0x8b96 bleibt bei R01 (Nachbarn O024/O032 bestaetigt). CopyProt-Merker und g_copyprot_sum/g_tamper_count liegen im Bereich von O032 (nicht R03/R23). |
| Offen (erledigt, s. u.) | BSS 0xc948..0xcaba (57 Kampf-Globale, Reststueck DRO131): die Hauptnutzer liegen ausserhalb des Fensters O104..O139 (O141 105, O144 63, O018 45, O142 42 Verweise), im Fenster nur wenige (O128 17, O134 16, O137 10, O112 8, O131 4); allein von O131 benutzt wird nur ds:cab8. Jedes Modul zwischen DMO103 und DMO140 ohne BSS (O104..O108, R18, O111, O116, O125, O126, O131, O132, O139, auch datenlose wie O112/O128/O134/O137) kommt in Frage - ueber die Zugriffe nicht entscheidbar, bleibt Reststueck. |
| Erledigt (Abschlusswelle A, 19.10. spaet) | **DRO131 entfaellt.** BSS 0xc948..0xcab8 (Kampf-Globale, 57 MAP-Publics) definiert jetzt O128 (Spalte `D`, nur BSS), 0xcab8..0xcaba (g_dng03_last_pos, nur von O131 benutzt) O131. **Zuordnung zu O128 vermutet: Besitzer ueber Linkreihenfolge und Zugriffe nicht entscheidbar (Fenster O104..O139), bytegleich bei jedem Kandidaten.** O128 (Kampfrunde) hat im Fenster die meisten Zugriffe (17); Hauptnutzer bleiben O141/O144/O018/O142 ausserhalb des Fensters. Eintraege in korrekturen.tsv, dm_rest.tsv leer. dscheck O128/O131 GLEICH, dsb-Link BYTEGLEICH. |
