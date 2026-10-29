# Sternenschweif – Quelltext · START HIER

**Das Schwarze Auge 2: Sternenschweif** (Attic, 1994), aus der deutschen CD-Fassung C1.02 komplett als Quelltext
rekonstruiert. Borland C++ 3.1 baut daraus eine SCHWEIF.EXE, die **Byte für Byte dem Original gleicht**
(MD5 `da3b59ce86a82e4b8bb14d36ed37d4d0`). MooMoo mit Claude, 17.–19.10.2026.

Stand dieses Pakets: **19.10.2026**. Rechte am Spiel bei den Rechteinhabern, siehe `RECHTE.md`.

## Was du tun musst (einmalig)

| # | Schritt |
|---|---|
| 1 | Dieses Zip irgendwohin entpacken, z. B. `C:\Sternenschweif\` |
| 2 | **Python 3** installieren (python.org → Downloads), beim Installieren **„Add python.exe to PATH“** ankreuzen |
| 3 | **Borland C++ 3.1** besorgen, zu finden auf archive.org: https://archive.org/details/bcpp31. Dann nach `Compiler\` kopieren, also so, dass dort `BIN\`, `INCLUDE\` und `LIB\` liegen. Er ist nicht im Paket, weil die Rechte bei Embarcadero liegen. |
| 4 | Aus deiner Sternenschweif-Installation (GOG) die Datei **`SCHWEIF.EXE`** nach `Original\` kopieren. Sie dient nur zum Vergleich. |
| 5 | Sternenschweif von **GOG** installiert haben. Von dort kommen DOSBox und die Spieldaten. Liegt es nicht am üblichen Ort, fragt das Paket mit einem Auswahlfenster nach dem Ordner. |

## Dann

| Ich will … | dann … | Ergebnis |
|---|---|---|
| **die EXE aus dem Quelltext bauen** | `Bauen\BAUEN.bat` doppelklicken | nach etwa 6–10 Minuten **„BYTEGLEICH mit dem Original“**. Die EXE liegt dann in `Build\SCHWEIF.EXE`, das Protokoll in `Build\bau.log`. |
| **das Spiel aus dem Quelltext spielen** | danach `SCHWEIF_STARTEN.bat` doppelklicken | Das Spiel startet mit der selbst gebauten EXE und den GOG-Daten. Spielstände landen getrennt in `Build\Testspielstaende`. Das Original wird nicht angefasst. |
| **eine Fassung mit behobenen Fehlern bauen** | `Bauen\BAUEN.bat wichtig` oder `Bauen\BAUEN.bat alles` | erst der normale bytegleiche Bau, dann zusätzlich `Build\SCHWEIF_FIX_WICHTIG.EXE` (kritische und mittlere Fehler) bzw. `Build\SCHWEIF_FIX_ALLES.EXE` (alle behebbaren). Etwa 5 Minuten mehr. Welcher Fehler wie behoben ist: `Bauen\FIXES.md` |
| **die Fassung mit behobenen Fehlern spielen** | `SCHWEIF_FIX_WICHTIG_STARTEN.bat` oder `SCHWEIF_FIX_ALLES_STARTEN.bat` | wie oben, mit eigenen Spielstand-Ordnern `Build\Testspielstaende_Fix_*`. **Noch ungetestet im Spiel**, Prüfliste in `Analyse\Bericht_Fehler_Patches.md` |
| **verstehen, wie der Code aufgebaut ist** | `LIESMICH.md` lesen | Aufbau, Lesereihenfolge (`schweif.h` → `main()` → Hauptschleife → Kampf → 3D-Engine), Begriffe |
| **einen Überblick zum Durchklicken** | `Analyse\Quelltextkarte.html` im Browser öffnen | alle 317 Module nach Spielbereich, dazu die Fehler in 6 Feldern |
| **wissen, was gemacht und gefunden wurde** | `ABSCHLUSSBERICHT.md` | Ergebnis, Funde über Attic und den Compiler, 3D-Engine, Kopierschutz-Geschichte |
| **die Fehler im Original, zum Lesen** | `Analyse\Fehlerliste_Sternenschweif.pdf` (oder `.docx`) | 268 Fehler, verständlich beschrieben, nach Spielbereich, Cheat-Liste mit 22 Exploits zum Nachstellen, Spalte „im Netz bekannt“ |
| **die Fehler im Original, technisch** | `Analyse\Originalfehler.md`, `Analyse\Fehler_Raster.md` (Tabelle: `Fehler_Raster.tsv`) | Codestelle, Einordnung sicher/unsicher × kritisch/mittel/egal, Begründung, Fix-Vorschlag. Die 45 geprüften Meldungen aus Foren: `Fehler_aus_dem_Netz_geprueft.tsv` |

## Was drin ist

| Ordner / Datei | Inhalt |
|---|---|
| `START_HIER.md` | diese Datei |
| `LIESMICH.md` | Aufbau des Quelltexts, Lesereihenfolge, Begriffe |
| `ABSCHLUSSBERICHT.md` | Gesamtbericht |
| `Quelle\` | **der Quelltext**: 32 residente Module in C/C++, 14 in Attics Assembler, `Overlays\` mit 269 Modulen in C/C++ und 2 in Assembler, `inc\` mit den Headern (`schweif.h`) |
| `Bauen\` | Bau-Paket für Windows: `BAUEN.bat`, `bauen.py`, Anleitung `BAUEN.md`, Liste der Fehlerbehebungen `FIXES.md` |
| `SCHWEIF_STARTEN.bat`, `SCHWEIF_FIX_WICHTIG_STARTEN.bat`, `SCHWEIF_FIX_ALLES_STARTEN.bat`, `SPIELPFAD_SUCHEN.cmd` | Spiel mit der gebauten EXE starten. Der Spielordner wird gesucht und in `SPIELPFAD.txt` gemerkt. |
| `Analyse\` | Fehlerliste und Raster, Quelltextkarte, Berichte (3D-Engine, Audio-Bibliothek, Kopierschutz, Datensegment, Bau-Paket), Leitfaden der Compiler-Muster, Namensliste, Spieltexte, Steckbriefe aller Module |
| `Werkzeuge\` | die Linux-Werkzeuge der Rekonstruktion (Build, Nachbearbeitung, Prüfung), zum Nachvollziehen |
| `Compiler\`, `Original\` | **leer**, hier kommen Schritt 3 und 4 hinein |

## Kurz zum Ergebnis

| | |
|---|---|
| Code | 301 Module in C/C++ (rund 119.000 Zeilen), 16 Module in Attics eigenem Assembler, alle kommentiert |
| Namen | alle 3.215 Funktionen und alle 1.941 Datenobjekte benannt. Die Namen sind unsere, Attics sind nicht erhalten. |
| Daten | jedes C-Modul trägt seine Daten selbst, so wie bei Attic |
| Fehler im Original | 268, davon 18 kritisch (14 sicher falsch, 4 unsicher), 22 Exploits. 34 davon waren im Netz schon beschrieben, 228 sind neu |
| Geprüft | bytegleich in der Cloud-Werkbank **und auf einem echten Windows-Rechner** (GOG-DOSBox 0.74-2, 5,7 Minuten) |

## Fehler beheben per Schalter

| | |
|---|---|
| Prinzip | Jede Behebung steht im Quelltext hinter `#if FIX_nnnn`. Ohne Schalter (`FIX_BUGS=0`) ergibt der Bau das bytegleiche Original |
| Stufen | `FIX_BUGS 1` = wichtig (kritisch + mittel), `2` = alles. `FIX_EXPLOITS 0` lässt die Exploits im Spiel. Jeder Fehler einzeln: `FIX_0020` usw. in `Quelle\inc\s_fix.h` |
| Umfang | wichtig: 92 Fehler ganz, 17 teilweise behoben. alles: zusätzlich 103 ganz, 13 teilweise. Was nicht geht und warum (Spielstandformat, kein Platz im residenten Teil, Absicht unklar): `Bauen\FIXES.md` |
| Geprüft | Standardbau bytegleich, beide Fix-EXEs statisch geprüft (Segmente, Datensegment, alle 271 Overlays) und in DOSBox gestartet. **Im Spiel noch nicht getestet** |
