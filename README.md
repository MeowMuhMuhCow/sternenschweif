# Sternenschweif – rekonstruierter Quelltext

**Das Schwarze Auge 2: Sternenschweif** (Attic, 1994), deutsche CD-Fassung C1.02, vollständig als C/C++-Quelltext
rekonstruiert. Borland C++ 3.1 baut daraus eine `SCHWEIF.EXE`, die **Byte für Byte dem Original gleicht**
(MD5 `da3b59ce86a82e4b8bb14d36ed37d4d0`).

> **Passt zur GOG-Fassung:** Grundlage ist die deutsche `SCHWEIF.EXE` (CD-Version C1.02), wie sie in **„Realms of Arkania 2 - Star Trail Classic“ bei GOG** enthalten ist (MD5 `da3b59ce86a82e4b8bb14d36ed37d4d0`). Andere Fassungen (Diskette, englisch, andere Patchstände) ergeben eine andere EXE. Vor dem Bauen die eigene EXE mit dieser Prüfsumme vergleichen.

*English: Complete source reconstruction of the 1994 DOS RPG "Realms of Arkania: Star Trail" (German CD version 1.02, as sold by GOG).
Rebuilds a byte-identical executable with the original Borland C++ 3.1 compiler. Includes an unofficial bug-fix build (200+ original bugs fixed,
savegame compatible) behind compile-time switches. Documentation is in German.*

| | |
|---|---|
| Code | 301 Module in C/C++ (rund 119.000 Zeilen), 16 Module in Attics eigenem Assembler, alle kommentiert |
| Namen | alle 3.215 Funktionen und 1.941 Datenobjekte benannt (unsere Namen, Attics sind nicht erhalten) |
| Daten | jedes Modul trägt seine Daten selbst, so wie bei Attic |
| Fehler im Original | 268 dokumentiert, 18 kritisch, 22 Exploits ([Fehlerliste](Analyse/Fehlerliste_Sternenschweif.pdf)) |
| Fehlerbehebung | per Schalter `FIX_BUGS` 1 (wichtig) / 2 (alles), `FIX_EXPLOITS`, jeder Fehler einzeln. Ohne Schalter bytegleich ([FIXES.md](Bauen/FIXES.md)) |

## Danke

Dieses Projekt (und schon sein Vorgehen) steht auf den Schultern von Menschen, die sich jahrelang, unbezahlt und aus reiner Liebe zu diesen
Spielen durch Maschinencode gearbeitet haben. **Ohne sie gäbe es das hier nicht.**

| | |
|---|---|
| **Henne** | Er hat im Projekt **Bright-Eyes** die Pionierarbeit für die Nordlandtrilogie geleistet: Die Schicksalsklinge Funktion für Funktion aus dem Maschinencode in binäräquivalenten Code zurückgewonnen, dazu die Werkzeuge und den Weg, den historischen Borland-Compiler in DOSBox anzusteuern. Über viele Jahre, größtenteils allein, mit einem Stundenaufwand, der sich kaum ermessen lässt. Er hat den Rohdiamanten ausgegraben. |
| **siebenstreich** | hat Bright-Eyes weitergeführt, den C-Code geschliffen und das Projekt am Leben gehalten (github.com/siebenstreich/BrightEyes), und hat unsere Fragen geduldig und offen beantwortet. |
| **cmfrydos** | entschlüsselt mit dem **Riva HD Viewer** die Datenformate der Nordlandtrilogie (Dungeons, Ereigniszonen, Dialoge, Kampfsystem) und teilt seine Ergebnisse offen im Forum und im Wiki. |
| **Das Crystals-DSA-Forum** | Seit 2006 sammeln die Spielerinnen und Spieler dort Fehler, Beobachtungen und Rätsel der Nordlandtrilogie, im Thread „Bugs in Sternenschweif“, in den Truhenpatch-Threads, in Tipps-Sammlungen und in der Technischen Werkstatt. Viele Einträge unserer Fehlerlisten gehen auf ihre Meldungen zurück und sind dort zuerst beschrieben worden. Danke an alle, die dort über die Jahre geschrieben, gemessen und nachgespielt haben, und an die Forenbetreiber, die das alles erhalten. |
| **Alle Beteiligten an Bright-Eyes, am BrightEyes-Wiki und an den Reverse-Engineering-Projekten der Nordlandtrilogie** | für Karten, Datenformate, Dokumentation und den langen Atem. |
| **Attic Entertainment Software** | für diese Spiele. 30 Jahre später lohnt es sich immer noch, jeden Winkel davon zu verstehen. |

Fehler in diesem Repository sind unsere eigenen. Alles, was gut daran ist, beruht auf ihrer Vorarbeit.

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

## Rechte

Kein Originalprogramm, keine Spieldaten, kein Compiler in diesem Repository. Rechte am Spiel bei den Rechteinhabern.
Details: [RECHTE.md](RECHTE.md).

MooMoo mit Claude, Oktober 2026.
