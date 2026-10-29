# Bau-Paket für Windows – Bericht (19.10.2026)

## Ergebnis

| Frage | Antwort |
|---|---|
| Bytegleich mit Standard-DOSBox 0.74? | **Ja.** `bauen.py` mit ungepatchtem DOSBox **0.74-2** (der GOG-Version) und **0.74-3**, beide aus den SVN-Tags gebaut: `BYTEGLEICH mit dem Original`, MD5 da3b59ce86a82e4b8bb14d36ed37d4d0, je 15 DOSBox-Läufe, ca. 9,5 min |
| Gegenprobe | ohne Datum setzen (`--ohne-datum --nur-asm`): ABWEICHUNG in genau 3 Bytes bei 0x20EBC (`_EXEINFO_`-Datum 0F 02 CB 07 = 15.02.1995 gegen heute). Der Test erkennt das Datum also |
| Was MooMoo installieren muss | nur **Python 3** (python.org, „Add to PATH“). Alternative ohne Installation: embeddable-Zip nach `Bauen\python\` |
| Paket | `/root/zw_d/baupaket/` → kommt nach `Q 004 DSA Sternenschweif\Bauen\` |

## 1. Analyse: was der Bau braucht

### Was build.sh tut

| Schritt | Werkzeug | DOSBox-Lauf |
|---|---|---|
| Overlay-Einsprünge, Aliase, `F_O032_0000` → `_main` (in place, idempotent) | ovlpub, aliases, sed | – |
| l*/o* nach lnk, Datenstücke DM*.ASM, FULL.RES/FULL2.RES, dm.json | dmgen (+ dsinfo, items) | – |
| 474 × TASM | ASMALL.BAT | 1 |
| OBJ-Nachbearbeitung | ovlobjfix (→ objpub), objchunk ×271, objorder ×200 | – |
| Link Stufe 1 (BC.EXE /b, IDE-Projekt aus mkprj/VORLAGE.PRJ) | LINK.BAT | 1 (Datum!) |
| FILL.COM (TASM+TLINK) | FILL.BAT | 1 |
| frische Läufe: OVL199, OVL030 mit Heapbild `@0115:…` | F_*.BAT (heapbild) | 2 |
| 301 × BCC in Blöcken zu 40 | CB00–CB07.BAT | 8 |
| C-OBJ anpassen, Daten prüfen | cobjfix_d (→ cobjfix), dscheck, cvsasm, pubdup | – |
| Link Stufe 2 | LINK.BAT | 1 (Datum!) |

Eingaben (per strace aus einem vollen Bau ermittelt): `l01–l46/SEG0nn.ASM`, `o000–o270/OVLnnn.ASM`, `dsg/DS00–05.ASM, DSBSS.ASM`,
`dsana/{dm_rest, ds_linkorder, ds_module, ds_owner, korrekturen}.tsv, zuordnung.json, items.py`, `lnk/INIT.ASM, FARDATA.ASM`,
`lnk_ranges.json`, `aliases.tsv`, `c_module.tsv`, `tools/{frisch.txt, fill/FILL.ASM, VORLAGE.PRJ}`, `q004/zerl/schweif.py`, `csrc/`, `names/inc/`,
`orig/SCHWEIF.EXE`, 22 Python-Werkzeuge, aus `bc/` nur BC/BCC/TASM/TLINK/DPMI*, 18 Header, C0L.OBJ, CL.LIB, OVERLAY.LIB.

### Was das gepatchte DOSBox beiträgt

| Funktion | gebraucht? | Ersatz im Standard-DOSBox |
|---|---|---|
| `BE_DATE` (INT 21h/2Ah liefert festes Datum) | **ja**, BC.EXE schreibt das Datum in `_EXEINFO_` | 0.74 hat **keinen** DATE-Befehl. Aber INT 21h/2Bh setzt `dos.date`, und das bleibt in 0.74 fest (kein Tageswechsel). `bauen.py` schreibt ein 15-Byte-`DATUM.COM` (`mov ah,2Bh / cx=1995 / dx=020Fh / int 21h`) und ruft es am Anfang jedes Laufs auf |
| `timeout` (in dosrun.sh) | ja | `subprocess.run(timeout=…)` |
| übrige BE_*-Fühler, Bright-Eyes-Hooks | nein | – |
| Speicherbild für BCC (frisch.txt) | ja | geht auch im Standard-DOSBox: frischer Lauf + FILL.COM (XMS-KB 0x115) ergibt dieselbe Registerwahl. Getestet mit 0.74-2 und 0.74-3 |

Gefundene Nebenbefunde:
- `X.BAT > DOSOUT.TXT` erfasst in DOSBox die Ausgabe der BAT-Zeilen **nicht**, denn die Umleitung endet, bevor die BAT läuft. Deshalb war `full_out.txt` im alten build.sh immer leer („Linker-Meldungen: 0“). Der Bau hängt davon nicht ab.
- Eine BAT, die aus dem autoexec ohne CALL gestartet wird, kehrt nicht zurück. Deshalb endet jede BAT mit `exit`.

## 2. Portierung

| build.sh / dosrun.sh | bauen.py |
|---|---|
| cp, sed, split, grep, cmp, md5sum | shutil, re, Listen-Slicing, Bytevergleich, hashlib |
| dosrun.sh + Bright-Eyes-DOSBox | erzeugte `.conf` (memsize 16, core normal, cycles max, kein Sound, `keyboardlayout=none`), `mount c "<Bauordner>"`, `C:\DATUM.COM`, `-noconsole -exit`, cwd = `_dosbox\` (dort landen stdout.txt/stderr.txt) |
| `SDL_VIDEODRIVER=dummy` | Probe-Lauf: erst ohne Fenster (dummy), wenn das nicht geht, mit Fenster |
| feste Pfade `/root/sternenschweif` | Werkzeuge im Paket umgeschrieben auf `SW_ROOT` (Umgebungsvariable) |
| `python3 -I` | `sys.executable -I -X utf8` (gleiche Kodierung wie unter Linux, Windows hätte sonst cp1252) |

Änderungen an den Werkzeugen erzeugt `tools/mk_baupaket.py` automatisch (Quelle bleibt unverändert):
1. `'/root/sternenschweif…'` → `_SW + '…'` (28 Stellen)
2. `ovlobjfix.py`: hart codiertes `'python3'` → `sys.executable`. **Wichtig für Windows:** dort wäre `python3` der Store-Platzhalter, und ovlobjfix schluckt Fehler (`capture_output`, kein Check). Der Bau liefe dann still falsch
3. Unterprozesse mit `-X utf8`
4. `cobjfix_d.py`: `shutil.move` → `os.replace` (Windows: Ziel darf existieren)

`bauen.py` selbst behandelt außerdem: schreibgeschützte Compilerdateien (chmod vor Überschreiben/Löschen), CRLF für alle BAT,
`\r` in Werkzeugausgaben, Groß-/Kleinschreibung der Dateinamen in `Quelle\`, Umlaute im Projektpfad (nur Python sieht ihn),
Bauordner mit Leerzeichen (getestet). Wenn `%TEMP%` nicht-ASCII ist, wird `C:\sternenschweif_bau` genommen.

## 3. Paketaufbau

Entscheidung: **Bau aus dem Paket plus Quelle\.** Der Baustand (ASM, Daten, Vorlagen) kommt aus `baustand.tar.gz`
und wird nach `%TEMP%\sternenschweif_bau` entpackt. Die C-Quellen und Kopfdateien kommen aus `Q004\Quelle\`. Sie sind
identisch mit csrc/ und names/inc (geprüft), und eigene Änderungen wirken so. Abweichungen vom Referenzstand meldet der Bau
mit Namen. Fehlt in Quelle\ ein Modul, ergänzt der Bau es aus dem Paket. Ergebnis nach `Q004\Build\SCHWEIF.EXE` + `bau.log`.
`werkbank_stand.tar.gz` (13 MB, ganze Cloud-Werkbank) wird **nicht** gebraucht. `baustand.tar.gz` ist der schlanke Auszug (3,6 MB, 645 Dateien).
Compiler: aus `Q004\Compiler\` (nur BIN, INCLUDE, LIB, als Kopie im Bauordner, TURBOC.CFG/TLINK.CFG dort auf `C:\BC\…`). Nicht im Paket.

### Dateiliste `baupaket/` (→ `Q 004 DSA Sternenschweif\Bauen\`)

| Datei | Größe | Zweck |
|---|---|---|
| `BAUEN.bat` | 2 KB | Doppelklick: findet Python (`python\python.exe`, `py -3`, `python`) und DOSBox (`DOSBox\DOSBox.exe`, GOG-Pfade), ruft `bauen.py` |
| `BAUEN.md` | 3 KB | Anleitung für MooMoo |
| `bauen.py` | 26 KB | der Bau (Portierung von build.sh + dosrun.sh) |
| `baustand.tar.gz` | 3,6 MB | ASM-Fassungen l*/o*, dsg, dsana-Tabellen, lnk-Vorlagen, lnk_ranges.json, aliases.tsv, c_module.tsv, frisch.txt, FILL.ASM, VORLAGE.PRJ, Referenz-csrc + names/inc |
| `werkzeuge/` | 22 .py | aliases, cobjfix, cobjfix_d, cvsasm, dmgen, dscheck, dsinfo, heapbild, items, items_ds, mkprj, objbytes, objchunk, objfix, objorder, objpub, ovlobjfix, ovlpub, prjlib, pubdup, schweif, selfoff |

Neu erzeugen nach Änderungen am Baum: `python3 -I tools/mk_baupaket.py` (schreibt `baupaket/werkzeuge` und `baustand.tar.gz`).

## 4. Test unter Linux

Nachgebauter Projektordner `…/Q - --- Spiele und Interaktives/Q 004 DSA Sternenschweif Ü/` mit Compiler und Original aus MooMoos
Upload (Compiler schreibgeschützt), Quelle aus dem aktuellen Stand, Paket als `Bauen/`, Bauordner unter `tmp mit leer/`:

```
cd ".../Bauen" && TMPDIR=".../tmp mit leer" LD_LIBRARY_PATH=/root/zw_d/sdlinst/lib \
  python3 bauen.py --dosbox /root/zw_d/dosbox074/dosbox-0.74-2
```

| Lauf | Ergebnis |
|---|---|
| DOSBox 0.74-3, voller Bau | Stufe 1 BYTEGLEICH, 301 C GLEICH, 146 dscheck GLEICH, **BYTEGLEICH mit dem Original**, 9,2 min |
| DOSBox 0.74-2, voller Bau | **BYTEGLEICH mit dem Original**, 9,4 min |
| 0.74-2, `--nur-asm --ohne-datum` | ABWEICHUNG, 3 Bytes Datum (erwartet) |
| Quelle-Abgleich (Einzeltest) | geänderte Datei gemeldet, fehlendes Modul ergänzt, `seg002.c` → `SEG002.C` |

Die Standard-DOSBox-Binaries liegen unter `/root/zw_d/dosbox074/` (0.74-2, 0.74-3), gebaut aus dem dosbox-staging-Spiegel (Tags `svn/RELEASE_0_74_2/3`), denn sourceforge ist gesperrt.

## 5. Unter Windows ungetestet / Risiken

| Risiko | Einschätzung, Abhilfe |
|---|---|
| `BAUEN.bat` nie gelaufen (kein Windows/Wine hier) | einfache goto-Logik ohne Klammerblöcke (wegen `%ProgramFiles(x86)%`), CRLF. Erster echter Test bei MooMoo |
| GOG-DOSBox.exe unter Windows: SDL-Dummytreiber | Probe-Lauf mit 60-s-Zeitlimit. Wenn er nicht geht, mit Fenster weiter (15 kurz aufblitzende DOSBox-Fenster, harmlos) |
| BCC-Speicherzustand (OVL030/OVL199) auf Windows-DOSBox | in Linux mit 0.74-2 und 0.74-3 bytegleich. Speicher wird bei Start genullt, `keyboardlayout=none` verhindert, dass ein deutsches KEYB-Layout DOS-Speicher belegt. Restrisiko klein. Falls nur O030/O199 abweichen, liegt es hier |
| Virenscanner sperrt frisch geschriebene Dateien (os.remove/replace) | selten. Dann Bau neu starten oder `%TEMP%\sternenschweif_bau` vom Scan ausnehmen |
| Laufzeit unter Windows | unbekannt, `cycles=max`. Die Zeitlimits (TASM 50 min, BCC-Block 10 min, Link 20 min) sind großzügig |
| GOG-Pfad anders als angenommen | Meldung „DOSBox nicht gefunden“. Dann DOSBox-Ordner nach `Bauen\DOSBox\` kopieren oder `--dosbox` angeben |
| Python aus dem Microsoft Store / Store-Platzhalter | BAT prüft Version ≥ 3.8 per Testaufruf. Der Platzhalter fällt dabei durch, die Werkzeuge rufen nie `python3` auf |
| Textmodus-Zeilenenden | alle BAT/RES/Binärdateien explizit, nur `dm.json`/`dm_liste.txt` werden CRLF, beide werden zeilenendenunabhängig gelesen |
