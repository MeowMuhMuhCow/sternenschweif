# Spieltexte Sternenschweif (deutsche Fassung, aus einer GOG-Installation)

Quelle: DATA/STAR.DAT (Archiv, 1242 Dateien), nur gelesen. SCHWEIF.EXE der Installation hat dieselbe MD5 wie unser Original.

- `dateinummern.tsv` – Dateinummer → Dateiname. Die Nummer ist der Index im Archiv, genau die Zahl, die der Code
  an die Dateifunktionen übergibt (z. B. 0x8e = FIGHT.LST, 0x8f = SCENARIO.LST, 0x48 = MONSTER.DAT, 0xfa/0xfb = CHARS1/2.DAT).
- `texte/*.txt` – alle 114 LTX-Textdateien, je Zeile `[Index] Text` (Index ab 0, `@` = Zeilenumbruch, Zeichensatz CP437 → UTF-8).
  Der Index entspricht dem Zugriff im Code (z. B. `txt[23]` auf die geladene Textliste).
- `dialoge/*.txt` – lesbare Texte aus 82 der 198 Dialogdateien (TLK), Reihenfolge wie in der Datei; die übrigen TLK sind
  nicht im Klartext gespeichert.
- `star/` – entpackte Archivdateien (Arbeitskopie, wird nicht gesichert).

Archivformat: u32 Anzahl, dann je Eintrag 20 Bytes (Name[14], u32 Offset, u16 Flag); Daten ab 2 + Anzahl*20.
