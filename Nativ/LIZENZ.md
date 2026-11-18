# Lizenzen

- Die Spiellogik in `Port/src` stammt aus der Quelle des Besitzers und wird über `Port/tools/prepare_sources.py` eingesetzt. Das Spiel selbst ist kein Teil dieser Freigabe. Spieldaten und die Original-EXE gehören nicht in diesen Ordner.
- `Port/third_party/stb_vorbis/stb_vorbis.c` ist Public Domain, Sean Barrett, Vorbis-Dekoder. Dieselbe Datei wie im Schicksalsklinge-Port. Die Datei wird hier nicht verändert.
- `Port/third_party/SDL2` steht unter der zlib-Lizenz. Dieser Port nutzt das Fenster und die Eingabe, keine SDL-Audiothek.
- DOSBox-OPL ist nicht enthalten. Für den Ton gibt es deshalb keinen zusätzlichen GPL-Baustein aus DOSBox.
- Die Plattformdateien unter `Port/platform` sind der Ersatz für das 16-Bit-DOS-Umfeld dieses Ports.
