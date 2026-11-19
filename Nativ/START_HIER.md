# Das Schwarze Auge: Sternenschweif – Nativer Windows-Port

Nativer 32-Bit-Windows-Port der deutschen CD-Version C1.02 von **DSA 2: Sternenschweif** – läuft direkt unter Windows 10/11 (und unter Linux mit Wine) ohne DOSBox. Die Original-Spieldaten bleiben in deiner vorhandenen Installation (z. B. GOG *Realms of Arkania 2*).

## Schnellstart (Windows)

Je nach gewünschter Bugfix-Stufe eine der Startdateien doppelklicken:

| Startdatei | EXE-Variante | Beschreibung |
|---|---|---|
| `Sternenschweif_starten_ALLE_FIXES.bat` | `SCHWEIF_PORT.EXE` (`FIX_BUGS=2`) | **Empfohlen:** Alle Fehlerkorrekturen aus `s_fix.h` aktiv |
| `Sternenschweif_starten_ALLE_FIXES_MIT_PROBENLOG.bat` | `SCHWEIF_PORT.EXE` (`FIX_BUGS=2`) | Alle Fixes + **Live-Probenlog-Fenster** neben dem Spiel (`PROBEN.CSV`) |
| `Sternenschweif_starten_WICHTIGE_FIXES.bat` | `SCHWEIF_PORT_WICHTIG.EXE` (`FIX_BUGS=1`) | Nur sichere Fehlerkorrekturen (Abstürze, falsche Offsets, kaputte Zauber/Items) |
| `Sternenschweif_starten_WICHTIGE_FIXES_MIT_PROBENLOG.bat` | `SCHWEIF_PORT_WICHTIG.EXE` (`FIX_BUGS=1`) | Wichtige Fixes + **Live-Probenlog-Fenster** |
| `Sternenschweif_starten_ORIGINAL.bat` | `SCHWEIF_PORT_ORIG.EXE` (`FIX_BUGS=0`) | Exaktes Verhalten der Original-EXE C1.02 (inkl. Original-Bugs) |

### Automatische Spielordner-Suche (`SPIELPFAD.txt`)
1. Beim ersten Start sucht das Skript automatisch nach deiner Installation (u. a. unter `C:\GOG Games\Realms of Arkania 2` und `C:\Program Files (x86)\GOG Galaxy\Games\Realms of Arkania 2`).
2. Wird der Ordner nicht automatisch gefunden, fragt das Skript einmalig nach dem Pfad und speichert ihn in `SPIELPFAD.txt`.
3. **Schreibschutz-Absicherung:** Ist der Spielordner schreibgeschützt (z. B. unter `C:\Program Files (x86)\...`), legt der Port automatisch den Ordner `Spielstaende\` neben der Programmdatei an, leitet Spielstände (`GAMES\`, `TEMP\`) sowie `PROBEN.CSV` dorthin um.

## Vorspann (Intro)
- Der originale Sternenschweif-Vorspann (FLI-Animationen + 2D/Wellen-Szenen synchron zur CD-Musik `Track09.ogg`) läuft beim Spielstart automatisch.
- Abbrechen jederzeit mit **ESC**, **Leertaste** oder **Mausklick**.
- Mit dem Startparameter `--kein-intro` (oder `--no-intro`) wird der Vorspann komplett übersprungen.
- Übergibt man einen Spielstandnamen (z. B. `SCHWEIF_PORT.EXE START`), wird direkt dieser Spielstand geladen.

## Ton & Steuerung
- **CD-Musik:** Spielt die Original-CD-Spuren `MUSIC\Track02.ogg` bis `Track22.ogg` exakt wie die GOG-Fassung ab.
- **Digitale Effekte & Sprache:** VOC-Klänge und Sprachausgabe (`SPEECH.ACD`) werden direkt gemischt.
- **Tasten:** `F10` = Pause (wie im Original), `F11` / `F12` = Musiklautstärke leiser/lauter, `Strg+F11` / `Strg+F12` = Effektlautstärke, `Alt+Enter` = Vollbild/Fenster.

## Probenlog (`Probenlog\`)
Startet man über `*_MIT_PROBENLOG.bat`, öffnet sich neben dem Spiel ein Live-Fenster (`Probenlog\probenlog_fenster.ps1`), das jede Eigenschafts-, Talent-, Zauber-, Fernkampf-, Attacke/Parade- und Stufenanstiegsprobe im Klartext anzeigt. Im Browser lässt sich `PROBEN.CSV` außerdem mit `Probenlog\Probenlog_Sternenschweif.html` auswerten (Details in `Probenlog\LIESMICH.md`).
