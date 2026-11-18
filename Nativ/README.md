# Das Schwarze Auge: Sternenschweif – Nativer 32-Bit-Windows-Port

Nativer 32-Bit-Windows-Port der deutschen CD-Version C1.02 von **Das Schwarze Auge: Sternenschweif** (*Realms of Arkania Vol. 2: Star Trail*), basierend auf dem vollständig rekonstruierten und byte-verifizierten C/C++-Quellcode (301 Module: Resident-Segmente `SEG002`–`SEG048` und Overlays `OVL001`–`OVL271`).

> **Wichtig:** Dieses Repository/Paket enthält **keine Original-Spieldaten** (`STAR.DAT`, `SPEECH.ACD`, `MUSIC\*.ogg`, `gamea.gog`/`gameb.gog`). Zum Spielen wird eine eigene Installation von *DSA 2: Sternenschweif* (z. B. von GOG.com) benötigt.

## Eigenschaften des Ports

- **Nativer 32-Bit-Lauf ohne DOSBox:** Alle 301 C/C++-Module und die 16 ehemaligen Assembler-Module laufen nativ unter Windows 10/11 (und Linux/Wine) mit SDL2-Fenster-/Vollbildausgabe (`320×200` VGA, 3× skaliert auf `960×600`).
- **Drei Varianten im Release:**
  - `SCHWEIF_PORT.EXE` (`FIX_BUGS=2`, `ALLES`): Alle Fehlerbehebungen und Verbesserungen aus `s_fix.h` (Standard).
  - `SCHWEIF_PORT_WICHTIG.EXE` (`FIX_BUGS=1`, `WICHTIG`): Nur sichere Fehlerbehebungen.
  - `SCHWEIF_PORT_ORIG.EXE` (`FIX_BUGS=0`, `ORIG`): Originalverhalten der DOS-EXE C1.02.
- **Original-Vorspann & Abspann:** Kompletter Vorspann (FLI + 2D-Szenen synchron zur CD-Musik, abbrechbar per `ESC`, `Leertaste` oder Mausklick; überspringbar mit `--kein-intro`) sowie vollständiger Abspann nach dem Endkampf.
- **Originalgetreuer Ton:** CD-Musik (`MUSIC\Track02.ogg`..`Track22.ogg`) über `stb_vorbis` + `waveOut`, gemischt mit digitalen `.VOC`-Kampf-/Umgebungsklängen und Sprachausgabe (`SPEECH.ACD`).
- **Live-Probenlog (`Probenlog\`):** In den Stufen `ALLES` und `WICHTIG` protokolliert das Spiel auf Wunsch jede gewürfelte Probe live in `PROBEN.CSV` und zeigt sie in einem Begleitfenster (`probenlog_fenster.ps1` / `probenlog.py`) oder im Browser (`Probenlog_Sternenschweif.html`) an.
- **Schreibschutz-Absicherung:** Ist das Spielverzeichnis nicht beschreibbar, werden Spielstände (`GAMES\`, `TEMP\`) und `PROBEN.CSV` automatisch nach `Spielstaende\` neben der EXE umgeleitet.

## Starten

Siehe **[START_HIER.md](START_HIER.md)** für die fünf `.bat`-Starter unter Windows bzw. `bash start_sternenschweif.sh` unter Linux/WSL.

## Selbst bauen (MinGW-w64 / TDM-GCC 32-Bit)

Aus dem Projektstamm:
```bash
python3 Port/bauen_port.py alle_stufen
```
Erzeugt `Port/build/SCHWEIF_PORT.EXE`, `SCHWEIF_PORT_ALLES.EXE`, `SCHWEIF_PORT_WICHTIG.EXE` und `SCHWEIF_PORT_ORIG.EXE`. Details stehen in [BAUEN.md](BAUEN.md) und [ABWEICHUNGEN.md](ABWEICHUNGEN.md).

## Lizenz

Veröffentlicht unter der **GNU General Public License v3.0 (GPLv3)** – siehe [COPYING](COPYING) und [LIZENZ.md](LIZENZ.md).
