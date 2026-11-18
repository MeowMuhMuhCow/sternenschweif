# Bauanleitung

Ein Aufruf baut das Programm. Viele einzelne Compilerfenster sind nicht nötig.

```
python3 Port/bauen_port.py
python3 Port/bauen_port.py alle_stufen
```

Arbeitsverzeichnis ist der Projektordner, also der Ordner über `Port`. Das Skript ist `Port/bauen_port.py`.

| Aufruf | Stufe | Ergebnis |
| :--- | :--- | :--- |
| `python3 Port/bauen_port.py` | alles | `Port/build/SCHWEIF_PORT.EXE` und `SCHWEIF_PORT_ALLES.EXE` |
| `python3 Port/bauen_port.py wichtig` | wichtig | `SCHWEIF_PORT_WICHTIG.EXE` |
| `python3 Port/bauen_port.py original` | original | `SCHWEIF_PORT_ORIG.EXE` |
| `python3 Port/bauen_port.py alle_stufen` | alle drei | die vier Dateien oben |

Einzelschalter: `python3 Port/bauen_port.py alles -DFIX_0020=0`. Siehe `ABWEICHUNGEN.md`.

## Windows

Compiler: TDM-GCC 32-Bit, bei diesem Rechner

`C:\Program Files (x86)\Embarcadero\Dev-Cpp\TDM-GCC-64\bin\g++.exe`

Das Skript setzt diesen Pfad selbst. Python 3 startet es. Unter WSL heißt der Aufruf ebenfalls `python3 Port/bauen_port.py`.

Wichtige Übersetzerflags: `-m32 -mabi=sysv -O2 -fno-toplevel-reorder -fno-zero-initialized-in-bss -fno-common -fpack-struct=1 -funsigned-char -mwindows`. `stb_vorbis.c` und `dsa_audio_cd.cpp` werden ohne `-fpack-struct=1` übersetzt.

`Port/src` ist schon vorbereitet. `--prepare` setzt die Quellen aus `Quelle/` neu auf und erzwingt einen vollen Neubau. `Quelle/` selbst wird dabei nicht verändert.

## Linux

Ein Linux-Cross-Compiler lag auf diesem Rechner nicht. Der Bau läuft über den Windows-Compiler oben, auch wenn das Skript aus WSL kommt. Liegt ein 32-Bit-`g++.exe` woanders, muss der Pfad in `bauen_port.py` (`MINGW_BIN_WSL`) auf ihn zeigen.

## Startabfrage

Im Bau gilt die Startabfrage als beantwortet, weil das Original vorhanden ist. Das steht so im Bauskript.
