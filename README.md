# Sternenschweif – rekonstruierter Quelltext

**Das Schwarze Auge 2: Sternenschweif** (Attic, 1994), deutsche CD-Fassung C1.02, vollständig als C/C++-Quelltext
rekonstruiert. Borland C++ 3.1 baut daraus eine `SCHWEIF.EXE`, die **Byte für Byte dem Original gleicht**
(MD5 `da3b59ce86a82e4b8bb14d36ed37d4d0`).

*English: Complete source reconstruction of the 1994 DOS RPG "Realms of Arkania: Star Trail" (German CD version 1.02).
Rebuilds a byte-identical executable with the original Borland C++ 3.1 compiler. Includes an unofficial bug-fix build (200+ original bugs fixed,
savegame compatible) behind compile-time switches. Documentation is in German.*

| | |
|---|---|
| Code | 301 Module in C/C++ (rund 119.000 Zeilen), 16 Module in Attics eigenem Assembler, alle kommentiert |
| Namen | alle 3.215 Funktionen und 1.941 Datenobjekte benannt (unsere Namen, Attics sind nicht erhalten) |
| Daten | jedes Modul trägt seine Daten selbst, so wie bei Attic |
| Fehler im Original | 268 dokumentiert, 18 kritisch, 22 Exploits ([Fehlerliste](Analyse/Fehlerliste_Sternenschweif.pdf)) |
| Fehlerbehebung | per Schalter `FIX_BUGS` 1 (wichtig) / 2 (alles), `FIX_EXPLOITS`, jeder Fehler einzeln. Ohne Schalter bytegleich ([FIXES.md](Bauen/FIXES.md)) |

## Bugfix-Fassung: Sternenschweif ohne die Fehler von 1994

Neben dem originalgetreuen Bau gibt es eine **inoffizielle Fehlerbehebung**: über 200 Fehler aus dem Original
lassen sich per Schalter beheben, von Abstürzen über verlorene Ausrüstung bis zu falschen Händlerpreisen.

| | |
|---|---|
| Stufe „wichtig“ | kritische und mittlere Fehler (u. a. Lagerhaus mit explodierendem Warenwert, Orks werfen Ausrüstung weg, Ware bezahlt aber nicht erhalten, Kräuterwirkungen verfallen nicht, Horriphobus-Kampf endet nie, Absturz am Schlüsselbund) |
| Stufe „alles“ | zusätzlich viele kleine Fehler ohne große Spielwirkung |
| Exploits | standardmäßig mit behoben (z. B. Geld aus der Spende, endlose Truhe). Wer sie behalten will: `FIX_EXPLOITS 0` |
| Spielstände | **kompatibel**, das Speicherformat ist unverändert. Alte Spielstände lassen sich weiterspielen |
| Bauen | `Bauen\BAUEN.bat wichtig` oder `Bauen\BAUEN.bat alles` → `Build\SCHWEIF_FIX_WICHTIG.EXE` bzw. `SCHWEIF_FIX_ALLES.EXE` |
| Spielen | `SCHWEIF_FIX_WICHTIG_STARTEN.bat` bzw. `SCHWEIF_FIX_ALLES_STARTEN.bat` (eigene Spielstand-Ordner) |
| Stand | statisch vollständig geprüft, im Spiel angetestet (Start, Herberge, Speichern/Laden). **Tester willkommen** – Prüfliste in [Analyse/Bericht_Fehler_Patches.md](Analyse/Bericht_Fehler_Patches.md) |

Welcher Fehler wie behoben ist und was bewusst nicht geändert wurde (Spielstandformat, Absicht unklar): [Bauen/FIXES.md](Bauen/FIXES.md).
Alle Fehler verständlich beschrieben, mit Cheat-Liste: [Fehlerliste](Analyse/Fehlerliste_Sternenschweif.pdf).

## Bauen (Windows)

Kurzfassung, ausführlich in [START_HIER.md](START_HIER.md):

1. Python 3 installieren.
2. Borland C++ 3.1 besorgen (z. B. https://archive.org/details/bcpp31) und nach `Compiler\` kopieren (`BIN`, `INCLUDE`, `LIB`).
3. Die eigene `SCHWEIF.EXE` aus der GOG-Installation nach `Original\` kopieren (nur zum Vergleich).
4. `Bauen\BAUEN.bat` starten → „BYTEGLEICH mit dem Original“. Mit `BAUEN.bat wichtig` bzw. `alles` zusätzlich die Fassung mit Fehlerbehebungen.
5. `SCHWEIF_STARTEN.bat` bzw. `SCHWEIF_FIX_WICHTIG_STARTEN.bat` startet das Spiel mit den GOG-Daten.

## Aufbau

| Ordner | Inhalt |
|---|---|
| `Quelle/` | der Quelltext: residente Module, `Overlays/`, Header in `inc/` (`schweif.h`) |
| `Bauen/` | Bau-Paket für Windows (`BAUEN.bat`, `bauen.py`) |
| `Analyse/` | Fehlerliste, Quelltextkarte (`Quelltextkarte.html`), Berichte (3D-Engine, Audio, Kopierschutz, Datensegment) |
| `Werkzeuge/` | die Werkzeuge der Rekonstruktion |

Lesereihenfolge und Begriffe: [LIESMICH.md](LIESMICH.md). Gesamtbericht: [ABSCHLUSSBERICHT.md](ABSCHLUSSBERICHT.md).

## Wie es gemacht wurde

Das Originalprogramm wurde in Module zerlegt und als Assembler wieder bytegleich zusammengebaut. Danach wurde jede
Funktion in C/C++ neu geschrieben und mit dem Original-Compiler gegen den Assembler geprüft, bis das Ergebnis
übereinstimmte. Der Compiler ist dabei der Schiedsrichter: gleich oder nicht gleich.

Dank an das Projekt **Bright-Eyes** (Henne, siebenstreich) für die Pionierarbeit an der Schicksalsklinge und an die
Spieler im Crystals-DSA-Forum, deren Fehlermeldungen seit 2006 in die Fehlerliste eingeflossen sind.

## Rechte

Kein Originalprogramm, keine Spieldaten, kein Compiler in diesem Repository. Rechte am Spiel bei den Rechteinhabern.
Details: [RECHTE.md](RECHTE.md).

MooMoo mit Claude, Oktober 2026.
