# Sternenschweif – der Quelltext

**Das Schwarze Auge 2: Sternenschweif** (Attic Entertainment Software, 1994), Quelltext rekonstruiert aus der
deutschen CD-Fassung C1.02. Borland C++ 3.1 baut daraus eine SCHWEIF.EXE, die **Byte für Byte dem Original gleicht**
(MD5 `da3b59ce86a82e4b8bb14d36ed37d4d0`).

Rekonstruiert im Oktober 2026 von MooMoo mit Claude (Anthropic). Vorbild war das Projekt Bright-Eyes
(Schicksalsklinge). Nur für den privaten Gebrauch und für die Rechteinhaber, nicht zur Veröffentlichung.

## Erste Schritte

| Ich will … | dann … |
|---|---|
| **das Spiel aus dem Quelltext starten** | GOG-Sternenschweif installieren und `SCHWEIF_STARTEN.bat` doppelklicken |
| **einen Überblick** | `Analyse/Quelltextkarte.html` im Browser öffnen: alle Module nach Spielbereich, mit Funktionen und Fehlern |
| **wissen, was gemacht wurde** | `ABSCHLUSSBERICHT.md` |
| **selbst bauen** | Python 3 installieren, dann `Bauen/BAUEN.bat` doppelklicken (Anleitung `Bauen/BAUEN.md`, ca. 10 min, am Ende „BYTEGLEICH mit dem Original“) |
| **die Fehler im Original** | `Analyse/Originalfehler.md` (243 Einträge) |
| **verstehen, wie die 3D-Ansicht funktioniert** | `Analyse/Bericht_3D_Engine.md` |

## Den Code lesen: wo anfangen?

| Reihenfolge | Datei | Was dort steht |
|---|---|---|
| 1 | `Quelle/inc/schweif.h` | der gemeinsame Header: Konstanten, Strukturen (Held, Gruppe, Gegenstand …), Klassen, Daten, Prototypen |
| 2 | `Quelle/Overlays/OVL032.CPP` | `main()`: Programmstart, globale Objekte |
| 3 | `Quelle/SEG001.CPP` | `main_loop`: die Hauptschleife (Stadt, Dungeon, Reise, Kampf, Tod, Spielende) |
| 4 | `Quelle/Overlays/OVL128.CPP` | eine Kampfrunde |
| 5 | `Quelle/SEG024.ASM` | die 3D-Engine (Assembler, kommentiert) |

Jede Datei beginnt mit einem Kopfkommentar: was das Modul tut, wer es aufruft, Besonderheiten und Fehler im Original.

## Aufbau

| Ordner | Inhalt |
|---|---|
| `Quelle/` | 46 residente Module: 32 in C/C++, 14 in Attics Assembler |
| `Quelle/Overlays/` | 271 nachladbare Module: 269 in C/C++, 2 in Assembler |
| `Quelle/inc/` | Header |
| `Bauen/` | Bau-Paket: `BAUEN.bat` baut die EXE auf Windows aus dem Quelltext |
| `Build/` | die gebaute SCHWEIF.EXE |
| `Werkzeuge/` | Build und Prüfwerkzeuge |
| `Analyse/` | Berichte, Fehlerliste, Namen, Spieltexte, Leitfaden zu den Compiler-Mustern |
| `Referenz/` | Prüfmaterial (Assembler-Fassungen, altes Datensegment), kein Quelltext |
| `Original/` | Original-EXE als Vergleich |
| `Compiler/` | Borland C++ 3.1. **Nicht weitergeben**, die Rechte liegen bei Embarcadero |

## Namen und Begriffe

| Begriff | Bedeutung |
|---|---|
| `SEGnnn` | residentes Segment Nr. nnn (immer im Speicher) |
| `OVLnnn` / `Onnn` | Overlay Nr. nnn (wird bei Bedarf nachgeladen) |
| `Rnn` | anderer Name für das residente Segment SEG0nn |
| `g_…` | globale Variable |
| `F_Rnn_xxxx`, `F_Onnn_xxxx` | ursprüngliche Adresse einer Funktion (Linkname). Der sprechende Name steht daneben |
| `vermutet` | Deutung nicht sicher belegt |
| bytegleich | der gebaute Code ist Byte für Byte das Original |

Die Namen stammen nicht von Attic, deren Bezeichner sind beim Kompilieren verloren gegangen. Vergeben haben wir sie
im Stil von Bright-Eyes, nach dem, was der Code tut, und nach den Spieltexten, die er ausgibt.
