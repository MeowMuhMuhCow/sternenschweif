# Namen und gemeinsamer Header – Konventionen

Ziel: alle C/C++-Module benutzen `#include "schweif.h"` statt eigener Strukturkopien und sprechende Namen statt
Adressnamen. Namen kosten keine Bytes (lokale Namen und Felder stehen nicht in der OBJ, externe Namen biegt
cobjfix über die Fixup-Stellen um). **Typen kosten dagegen Bytes.** Jede Umstellung wird geprüft:
ccmp GLEICH, cobjfix läuft, und `fixvgl.py` meldet gegenüber der alten Fassung LINKGLEICH.

## Namensstil

| Was | Stil | Beispiel |
|---|---|---|
| Funktionen | englisch, snake_case, Bright-Eyes-Name wenn vorhanden | `sub_hero_le`, `test_skill`, `dice_roll` |
| „Methoden“ einer Objektklasse als freie Funktion | Präfix der Klasse | `dlg_message`, `txt_load_list`, `file_open`, `group_add_money`, `gb_blit` |
| Konstruktor/Destruktor | `Klasse_ctor` / `Klasse_dtor` | `Group_ctor`, `Pic_dtor` |
| globale Daten | `g_` + snake_case | `g_group`, `g_heroes`, `g_text_output_buf` |
| Strukturen | klein, englisch | `struct hero`, `struct group`, `struct inv_slot` |
| C++-Klassen | CamelCase | `Bmp` |
| Konstanten/enums | GROSS mit BE-Präfix | `ATTRIB_KK`, `TA_SCHLEICHEN`, `POISON_ARAX`, `HERO_TYPE_MAGE` |
| Inline-Elementfunktionen | `set_<feld>`, `add_<feld>` (klemmend bei 0), `clr_`, `inc_`, `dec_`, `is_` | `set_poi_state`, `add_ill_kk` |
| Locals | sprechend, kurz | `poi`, `ill`, `was_unconscious`, `txt_buf` |
| Kommentare | deutsch, kurz und sachlich | |

Kollisionen mit der Borland-Laufzeitbibliothek (`random`, `open`, `delay` …) vermeiden; `namen.py` prüft
gegen `bc/INCLUDE` und hängt sonst `_` an. Doppelte automatische Namen bekommen das Segment angehängt
(`list_ctor_o153`).

## Unsichere Namen

- `namen.tsv`, Spalte Sicherheit: **hoch** (BE-Name bzw. eindeutig belegt), **mittel** (aus Kopfkommentar/C-Namen
  abgeleitet), **niedrig** (Vermutung, gern umbenennen), **offen** (kein Vorschlag – Name bleibt der Adressname in
  Kleinschreibung, z. B. `o038_0197`, `g_c94e`).
- Quelle: BE (Bright Eyes, DSA 1/2), KK (Kopfkommentar bzw. C-Name eines fertigen Moduls), TXT (Spieltext), AIL (Miles-Quelle AIL.ASM 2.14, SEG022),
  AB (abgeleitet aus dem Code).
- Unbekannte Strukturfelder heißen `f<offset>` (`f4f0`) und behalten ihren Typ, Füllbytes `_r<offset>`,
  unbekannte Bits `<byte><bit>` (`bd4`). Vermutete DSA-1-Bedeutung als Kommentar „(BE: …)“, nicht als Name.
- Ein Name wird erst „hoch“, wenn mindestens ein DSA-2-Modul die Bedeutung bestätigt.

## Header-Aufteilung (`names/inc`)

| Datei | Inhalt | Pflege |
|---|---|---|
| `schweif.h` | bindet alles ein, sonst nichts | – |
| `s_const.h` | enums/Konstanten | von Hand |
| `s_struct.h` | `hero` (0x5ef), `hero_attrib`, `hero_ill`, `inv_slot` (0x1b), `hero_effect`, `item_desc` (0x0e), `group` (0x49), `enemy` (0x49), Tabellenstrukturen; C++-Inline-Setter | von Hand |
| `s_class.h` | C++-Klassen (bisher `Bmp`), `BMP_PTR` für Prototypen | von Hand |
| `s_data.h` | extern-Deklarationen | **erzeugt** aus `daten/manuell_data.tsv` |
| `s_proto.h` | Prototypen aller benannten Funktionen | **erzeugt** aus `namen.tsv` + Deklarationen der Module; Ausnahmen in `daten/proto_fest.tsv` |

Erzeugen: `python3 -I names/tools/evidenz.py && python3 -I names/tools/namen.py` (vorher `cnamen.py` auf einem
fertigen Build-Ordner, siehe Werkzeuge). Standard-Header (`stdio.h`, `string.h` …) bindet jedes Modul selbst
**vor** `schweif.h` ein – sie entscheiden über Intrinsics und inline-Varianten. Der Header erzeugt selbst weder
Code noch Daten noch EXTDEFs (geprüft).

## Umstellung eines Moduls (Ablauf)

1. Kopie in einen Arbeitsordner (z. B. `wovnm`), Rohfassung mit
   `python3 -I names/tools/umbenennen.py Onnn --apply wovxx/OVLnnn.CPP` (externe Namen nach `namen.tsv`, Felder
   lokaler Kopien nach Offset auf die Fassung in `s_struct.h`; Regeln mit „kurzer Name“ prüfen).
2. Lokale Strukturkopien, `extern`- und Prototyp-Zeilen löschen, `#include "schweif.h"` nach den Standard-Headern.
   Eigene Funktionen nach `namen.tsv` benennen, Locals sprechend, Zahlen durch Konstanten (`ATTRIB_KK`,
   `POISON_ARAX`, `ILL_STATE_SICK`) ersetzen – Konstanten ändern keinen Code.
3. Übersetzen und prüfen: `bash names/tools/pilot.sh wovxx` (Optionen aus `c_module.tsv`, `-I..\names\inc`,
   ccmp + cobjfix). Danach `fixvgl.py ALT.FIX NEU.FIX` gegen die cobjfix-Fassung des alten Moduls.
4. Kopfkommentar behalten, Namen darin nachziehen, Zeile „Namensfassung mit schweif.h: Datum GLEICH“ ergänzen.
5. Übernahme nach `csrc/` macht die Hauptsitzung; `tools/build.sh` braucht dafür `-I..\names\inc` in der
   BCC-Zeile (Stufe 2).

### Codewirksame Fallen (aus dem Pilot und den Konflikten)

- **Bitfelder:** verbindlich `unsigned short b:1`. Module, die mit `unsigned char`-Bitfeldern gebaut wurden,
  brauchen im Wahrheitstest `(signed char)`: `if ((signed char)h->poison[i].effect)` ergibt `and ax,1 / or al,al`
  (ohne Cast `or ax,ax`). `!(signed char)h->dead`, `(unsigned char)h->dead` wie bisher.
- **Parametertypen:** `signed char`-Parameter erzeugen beim Aufrufer `mov al / push ax`, `short` `push word`
  bzw. `cbw`. Bei Abweichung gilt der Prototyp in `s_proto.h` (Regel: char vor short); ein Modul, das anders
  übergibt, bekommt einen Cast im Aufruf – nie einen eigenen Prototyp. Liste: `konflikte.tsv`.
- **C++-Überladung:** eine Definition im Modul muss exakt dem Prototyp in `s_proto.h` entsprechen, sonst ist es
  in C++ eine zweite Funktion (Aufrufe innerhalb des Moduls gingen dann far nach außen). Abweichung → Prototyp
  über `daten/proto_fest.tsv` korrigieren und neu erzeugen.
- **Zeiger:** Objektzeiger im Header als `void far *` (alles konvertiert), Objekte in `s_data.h` als `char[]`;
  Mehrfachzeiger exakt (`void far * far *` für `&txt_buf`, `char far * far * far *` für `&txt`).
- **Vorzeichen von Feldern** (z. B. `group.money` signed, O200 rechnet unsigned): im Modul casten.
- **Inline-Funktionen:** Rumpf einer vorhandenen inline-Funktion nie ändern (andere Module hängen an ihren
  Parameterkopien) – neue Variante mit eigenem Namen anlegen.
- **Eigenschaften:** `attrib[k]` ab +0x46 `{normal, cur, mod}`; alte Sicht `a[k]` ab +0x47 mit `n/m` entspricht
  `attrib[k].cur/.mod` (Konstanten falten, OVL076 GLEICH).

## Werkzeuge (`names/tools`)

| Werkzeug | Zweck |
|---|---|
| `cnamen.py [CDIR]` | C-Namen je Modul ↔ Linkname (OBJ gegen FIX eines fertigen Builds, Standard `cbau`) → `daten/cnamen.tsv` |
| `evidenz.py` | je Linkname: Modul, C-Namen, Signaturen, Rückgabetypen, Kopfkommentar, BE-/DSA1-Namen → `daten/evidenz.tsv` |
| `namen.py` | `namen.tsv`, `konflikte.tsv`, `inc/s_proto.h`, `inc/s_data.h` |
| `structs.py NAME [GRÖSSE]`, `agg.py`, `feldkonflikte.py` | Strukturinventar aus csrc (`daten/*_felder.tsv`, `*_agg.txt`) |
| `umbenennen.py Onnn [--apply]` | Umbenennungsregeln/Rohfassung für ein Modul |
| `pilot.sh [WOV]` | Module im Arbeitsordner übersetzen, ccmp + cobjfix |
| `fixvgl.py A B` | zwei cobjfix-Fassungen inhaltlich vergleichen (LINKGLEICH) |

Handlisten: `daten/manuell_res.tsv` (residente Funktionen), `daten/manuell_ovl*.tsv` (Overlays),
`daten/manuell_data.tsv` (Daten mit C-Deklaration), `daten/konflikte_hand.tsv`, `daten/proto_fest.tsv`.

## Wellenplan (Vorschlag)

Reihenfolge nach Abhängigkeit: zuerst die Module, die die Kernstrukturen am stärksten benutzen und deren
Namen die übrigen festlegen; Klassen erst, wenn ihre Methoden benannt sind.

1. **Heldenwerte** (Pilot fortsetzen): O074, O077, O080, O082, O006, O088, O090, O091, O093–O095, O005 –
   dabei die Bitfeld-Module O074/O009 und die Krankheitstabellen O090/O093/O174 klären.
2. **Gruppe und Inventar:** O200–O203 (Methoden als C++-Elementfunktionen an `struct group`), O075, O196,
   O083–O085, O008, O047.
3. **Residente C-Module:** SEG002 (hero-Kopie mit unsigned-char-Bits), SEG010, SEG018, SEG020, SEG042,
   übrige SEG – f_-Namen durch `namen.tsv`-Namen ersetzen.
4. **Objektklassen:** O190 Dialog, O248 Schrift/Text, O037/O243 Dateien, O098 NVF-Datei, O269 Bild, O238/O153
   Listen, O188 Bildschirm → in `s_class.h`, danach Aufrufer auf Methoden umstellen (oder freie Namen lassen).
5. **Kampf:** O018–O022, O128–O162, O192 (`struct enemy` vervollständigen, Kampfstruktur 0xfb1 inventarisieren).
6. **Reise, Karte, Orte, Ereignisse:** O034, O038, O046, O052/O053, O129, O164, O183/O249, O209–O223.
7. **Rest:** Intro/Menüs/Optionen, Klang, Händler (O099/O100/O226 ff.), Grafik.

Je Welle 30–50 Module; nach jeder Welle `namen.tsv`/Handlisten ergänzen und `namen.py` neu laufen lassen,
damit die nächste Welle die neuen Namen schon im Header findet.

## Erkenntnisse aus den Wellen N1–N4 (Stand 19.10.2026)

- Die Header in `inc/` sind die maßgebliche, handgepflegte Quelle. `namen.py` reproduziert `s_proto.h`/`s_data.h`
  nicht mehr vollständig – nicht neu erzeugen. Neue Deklarationen direkt in `inc/` eintragen und in `namen.tsv`
  bzw. `daten/manuell_<welle>.tsv` / `daten/proto_fest.tsv` vermerken (`namen.py` liest alle `manuell_*.tsv`).
- Prototyp weicht vom Aufruf ab (char ↔ short, `void far *` ↔ `char far *`): Aufruf über gecasteten
  Funktionszeiger ergibt denselben far call.
- `?:` zweier Felder an einen `void far *`-Parameter pusht ds vorher – dort eine `char far *`-Sicht verwenden.
- Eigene Publics mitten in Tabellen brauchen eigene Symbole (gleiche Bytes, aber anderes Fixup-Ziel).
- Teilsichten auf Objekte (`((struct dlg_view far *)g_dialog)->sel`) ergeben dieselben Bytes wie der direkte Zugriff.
- `struct date` heißt `struct game_date` (Kollision mit dos.h).
- Falsche Namen werden korrigiert, wenn nachgewiesen – mit allen Aufrufern (csrc-Patch) und Gegenprobe über alle
  umgestellten Module. Beispiele: hero_cure_disease/poison waren vertauscht, O082 ist rabies_effect,
  sound_mem → swap_mem_init, g_mouse_cursor → g_cursor_nr_prev.

## Abschlusswelle A (19.10.2026)
- Echt unbenutzte Bytes: `g_unused_<adr>` (nur im Quelltext des Besitzermoduls; ohne MAP-Public kein Eintrag in namen.tsv).
- Texte, die als Literal im Code stehen: Name entfällt, die Zeile in namen.tsv bleibt mit Quelle **LIT** (Beschreibung
  nennt Text und Modul), die extern-Deklaration in s_data.h ist gestrichen.
- Durchgezählte Einmal-Kämpfe: `g_dngNN_<gegner>_<feld>` (Gegner = erster Gegnertyp, MONNAMES.LTX; Feld = Kartenposition
  x << 8 | y, kein Adresszusatz), z. B. `g_dng03_skeleton_warriors_171d`.
- Gesprächsthemen `TOPIC_*` (s_const.h, Frage laut XDIALOG.DAT/STDTALKS.LTX bzw. TALKS_nn.LTX), Merker „Thema frei/bekannt“
  `g_topic_<thema>`. Kalender `MONTH_*` (1 = Praios), `HOLIDAY_*` (date.special).
- Merker, die nur geschrieben oder nie gesetzt werden, bekommen trotzdem einen Bedeutungsnamen; die Beschreibung sagt
  „wird nie gelesen“ bzw. „nie gesetzt (immer 0)“, unsichere Deutungen „vermutet“.
- Alte Namen: Block `/* Abschlusswelle A */` am Ende von s_proto.h (`#define alt neu`).
