# Sternenschweif unter Linux

Linux-Gegenstück zu `PATCHEN.bat` und den `*_STARTEN.bat`. Gleiche Patches, gleiches Ergebnis
(die Prüfsummen werden beim Patchen kontrolliert).

## Was man braucht

| Was | Woher |
|---|---|
| die deutsche GOG-Fassung von Sternenschweif | GOG-Installer, unter Linux z. B. mit `innoextract` entpackt, oder über Heroic/Lutris installiert. Gebraucht wird der Ordner mit `SCHWEIF.EXE` |
| Python 3 | ist meist schon da (`python3 --version`) |
| DOSBox | DOSBox Staging (empfohlen), DOSBox oder DOSBox-X, aus der Paketverwaltung oder als Flatpak (`io.github.dosbox-staging`) |

## So geht's

```bash
cd sternenschweif
bash Linux/patchen.sh            # erzeugt Build/SCHWEIF_FIX_WICHTIG.EXE und Build/SCHWEIF_FIX_ALLES.EXE
bash Linux/starten.sh wichtig    # kritische + mittlere Fehler behoben
bash Linux/starten.sh alles      # alle behebbaren Fehler
```

Beim ersten Mal sucht `patchen.sh` den Spielordner (unter `~/GOG Games`, `~/Games`, `~/Spiele`, Wine und Steam) oder
fragt danach. Der Pfad wird in `Linux/SPIELPFAD.txt` gemerkt, zum Neu-Wählen die Datei löschen. Direkt angeben geht
auch: `SPIEL=~/Spiele/Sternenschweif bash Linux/patchen.sh`.

## Was beim Start passiert

| | |
|---|---|
| Spielordner | wird nicht angefasst. Beim ersten Start kopiert `starten.sh` ihn (ohne CD-Abbild und DOSBox) nach `Build/Linux_C_<Fassung>`. Dort liegen auch die Spielstände dieser Fassung. Spätere Starts überschreiben dort nichts außer der EXE |
| Laufwerke in DOSBox | C: = die Kopie, D: = CD-Abbild von GOG (`gamea.ins` + `gameb.ins`, für die CD-Prüfung), E: = die Fix-EXE als `SCHWEIF.EXE` |
| Einstellungen | `dosbox_realms2.conf` aus dem Spielordner (Grafik, Sound) wird mitgeladen, falls vorhanden. Ohne: `OHNE_GOGCONF=1 bash Linux/starten.sh wichtig` |
| andere DOSBox | `DOSBOX=/pfad/zu/dosbox bash Linux/starten.sh wichtig` |

Selbst bauen (bytegleich aus dem Quelltext) braucht Borland C++ 3.1 in DOSBox, siehe `Bauen/BAUEN.md`. Die gebaute
EXE startet `bash Linux/starten.sh quelltext`.

Alle Behebungen sind im Spiel noch ungetestet. Rückmeldungen sind willkommen.
