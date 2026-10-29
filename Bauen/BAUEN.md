# SCHWEIF.EXE selbst bauen

Dieser Ordner baut aus dem rekonstruierten Quelltext die Spiel-EXE von *Sternenschweif* (DSA 2, Attic 1994)
und vergleicht sie Byte für Byte mit dem Original. Am Ende steht im Fenster:

> **BYTEGLEICH mit dem Original**

## Einmal vorbereiten

| Schritt | Was |
|---|---|
| 1 | Diesen Ordner als `Bauen` in den Projektordner legen: z. B. `C:\Sternenschweif\Bauen\` |
| 2 | **Python für Windows** installieren: [python.org/downloads](https://www.python.org/downloads/), beim Installieren **„Add python.exe to PATH“** ankreuzen. Sonst nichts. |

DOSBox ist schon da, nämlich das aus dem GOG-Sternenschweif. Den Compiler nimmt der Bau aus `Compiler\` im Projektordner.

**Ohne Installation (Alternative zu Schritt 2):** das „Windows embeddable package (64-bit)“ von python.org
herunterladen und nach `Bauen\python\` entpacken, sodass `Bauen\python\python.exe` existiert. BAUEN.bat nimmt es dann von selbst.

## Bauen

Doppelklick auf **`BAUEN.bat`**. Dauer etwa 10 Minuten. Zwischendurch meldet der Bau neun Schritte `[0/9] … [9/9]`.

| Ergebnis | Wo |
|---|---|
| gebaute EXE | `Q 004 …\Build\SCHWEIF.EXE` |
| Protokoll | `Q 004 …\Build\bau.log` |
| Arbeitsordner (wird bei jedem Bau geleert) | `%TEMP%\sternenschweif_bau` |

## Fix-Fassungen (Fehlerbehebungen)

`BAUEN.bat wichtig` bzw. `BAUEN.bat alles` baut nach dem normalen Bau zusätzlich eine EXE mit eingeschalteten
Fehlerbehebungen (Schalter `FIX_nnnn` in `Quelle\inc\s_fix.h`, Liste in `FIXES.md`). Dauer etwa 5 Minuten mehr.

| Aufruf | Ergebnis | Inhalt |
|---|---|---|
| `BAUEN.bat wichtig` | `Build\SCHWEIF_FIX_WICHTIG.EXE` | Behebungen der Kategorien *kritisch* und *mittel* (`FIX_BUGS=1`) |
| `BAUEN.bat alles` | `Build\SCHWEIF_FIX_ALLES.EXE` | zusätzlich alle *egal*-Fehler (`FIX_BUGS=2`) |

Gestartet werden sie mit `SCHWEIF_FIX_WICHTIG_STARTEN.bat` bzw. `SCHWEIF_FIX_ALLES_STARTEN.bat` im Projektordner
(neben `SCHWEIF_STARTEN.bat`). Die Startdateien kopieren die Fix-EXE als `SCHWEIF.EXE` nach `Build\Fix_Wichtig` bzw.
`Build\Fix_Alles` (DOS kennt nur 8.3-Namen) und legen Spielstände in eigene Ordner
`Build\Testspielstaende_Fix_Wichtig` bzw. `Build\Testspielstaende_Fix_Alles`.

Der normale Bau läuft im Fix-Modus immer mit: er muss weiterhin **BYTEGLEICH** melden und liefert die
Vergleichsfassung, über die der Fix-Bau die Linknamen zuordnet. Die Fix-EXE kann nicht bytegleich sein; geprüft wird
stattdessen (`fixcheck`): residente Segmente an gleicher Stelle, Datensegment unverändert, alle Overlay-Einsprünge
vorhanden. Die Spielstände bleiben mit dem Original austauschbar (keine Behebung ändert Strukturen oder das
Spielstandformat).

Weitere Optionen: `BAUEN.bat alles --exploits-lassen` (Fehler, die dem Spieler nützen, bleiben drin, `FIX_EXPLOITS=0`),
`BAUEN.bat alles --fixflags "-DFIX_0020=0"` (einzelne Schalter übersteuern).

## Was der Bau benutzt

| Teil | Woher |
|---|---|
| C-Quellen (301 Module) | `Quelle\SEG*.C/CPP`, `Quelle\Overlays\OVL*.C/CPP` |
| Kopfdateien | `Quelle\inc\` |
| Assembler-Fassungen, Datensegment, Link-Vorlagen | `Bauen\baustand.tar.gz` |
| Borland C++ 3.1 | `Compiler\` (bleibt unverändert, der Bau arbeitet mit einer Kopie) |
| Original zum Vergleich | `Original\SCHWEIF.EXE` |
| DOSBox 0.74 | GOG: `Realms of Arkania 2\DOSBOX\DOSBox.exe`, oder `Bauen\DOSBox\DOSBox.exe` |

Wer eine Quelldatei in `Quelle\` ändert, baut mit seiner Änderung. Das Protokoll nennt dann die Dateien, die vom
Referenzstand abweichen, und die EXE ist natürlich nicht mehr bytegleich.

## Meldungen

| Meldung | Bedeutung |
|---|---|
| `BYTEGLEICH mit dem Original` | alles richtig |
| `Stufe 1 (alles Assembler): BYTEGLEICH` | Zwischenstand: die reine Assembler-Fassung stimmt |
| `GEAENDERT gegenueber Referenzstand: …` | Quelle\ weicht vom geprüften Stand ab (gewollt, wenn man selbst ändert) |
| `ABWEICHUNG vom Original: erste Differenz bei Byte …` | gebaut, aber nicht gleich. Das Protokoll zeigt, ab welcher Stufe |
| `Python 3 wurde nicht gefunden` | Schritt 2 oben fehlt |
| `DOSBox nicht gefunden` | GOG-Pfad anders: DOSBox-Ordner nach `Bauen\DOSBox\` kopieren |

## Optionen (für Fortgeschrittene)

`BAUEN.bat` reicht Optionen an `bauen.py` weiter, z. B. `BAUEN.bat --sichtbar`.

| Option | Wirkung |
|---|---|
| `wichtig` / `alles` | zusätzlich die Fix-EXE bauen (siehe oben) |
| `--sichtbar` | DOSBox-Fenster zeigen (sonst unsichtbar, falls möglich) |
| `--nur-asm` | nur Stufe 1, schneller Test (wenige Minuten) |
| `--nur-paket` | C-Quellen aus dem Paket statt aus `Quelle\` |
| `--dosbox PFAD` | anderes DOSBox 0.74 |
| `--baudir ORDNER` | anderer Arbeitsordner |
