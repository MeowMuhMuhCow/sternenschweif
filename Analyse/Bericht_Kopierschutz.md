# Welle KS – Kopierschutz-Segmente als C (19.10.2026)

**Ergebnis:** Komplettbuild in `/root/ks_baum` (Stand nach den DS-Patches Phase A/B, dazu diese Abgabe):
Stufe 1 BYTEGLEICH. In Stufe 2 kommen 301 Module aus C (vorher 298, neu SEG001/SEG003/SEG023). SEG005 bleibt D-Modul,
F_R05_0012 ist jetzt echtes C. **MD5 da3b59ce86a82e4b8bb14d36ed37d4d0, „BYTEGLEICH mit dem Original“.**
Die Prüfungen bleiben unverändert wirksam. Es gibt keine Umgehung und keinen Schalter.

## Was die Segmente wirklich sind

| Segment | bisherige Einordnung | tatsächlich |
|---|---|---|
| SEG001 | Kopierschutz | **Spielschleife**: Reisemodus, Hauptschleife, Leerlaufschritt, Speichern des markierten Spielstands. Kein CD-Audio (das ist in DSA 1 seg001, hier O004). Enthält zwei *markierte* Aufrufe in den Kopierschutz. |
| SEG003 | Kopierschutz | **Handbuchabfrage** als C++-Klasse `CopyProt` (Seite/Zeile/Wort aus PROTECT.LTX, verschlüsselte Antworten in PASSWORD.DAT, 3 Versuche), dazu `cleanup_game` (BE-Name) und `cpu_v86_active`. In der CD-Fassung wird die Abfrage `query` **nirgends aufgerufen**. Wirksam ist nur `copyprot_check` aus game_idle, und die besteht immer, weil der Konstruktor `ok` auf 3..21 setzt. |
| SEG023 | Kopierschutz | **Zeitgeber und Maus**: INT-1Ch-ISR, AIL-Rückrufe der 3D-Ansicht und des Kampfs, Maus-ISR (BE mouse_isr), long_sqrt. Der Kampf-Zeitgeber enthält die **Integritätsprüfung**: Prüfsumme über den Code von SEG003 sowie die Suche nach den markierten Aufrufen in SEG001. Ein Fehlbefund zählt `g_tamper_count` hoch, ab > 50 verlieren die Helden LE (R10, stille Strafe). |
| R05_0012 | Byteblock | **CD-Prüfung `cd_check`**: Laufwerk aus PATH.CFG, MSCDEX-Laufwerk, Datenträgername DSA2_CD1/DSA2_CD2, sonst Aufforderung (Esc bricht ab). Der Abbruch springt auf PSP:0000 (INT 20h). |

Markierte Aufrufe: `EB 04 87 23 31 56` (jmp short über das Muster 0x56312387), direkt danach `call far`. In C steht
dafür `COPYPROT_MARK(); copyprot_check();`. Der Prüfbereich wird durch leere Marken-Funktionen in Originalreihenfolge
begrenzt.

## Funktionen (neue Namen)

| Link | Name | | Link | Name |
|---|---|---|---|---|
| R01_000D | do_travel_mode (BE) | | R03_0278 | copyprot_code_end |
| R01_009D | main_loop | | R03_027D | exit_game |
| R01_0366 | copyprot_idle_begin | | R03_028C | cleanup_game (BE; früher sys_exit) |
| R01_036B | game_idle | | R03_0306 | CopyProt::CopyProt (früher music_init) |
| R01_04D4 | copyprot_idle_end | | R03_0347/036D | CopyProt::load_texts / finish |
| R01_04D9 | copyprot_save_begin | | R03_03D0/03E2 | CopyProt::rnd / rnd_interval |
| R01_04DE | savemenu_save (früher savegame_op) | | R03_03F7 | copyprot_empty |
| R01_06E0 | copyprot_save_end | | R03_03FC | CopyProt::~CopyProt (früher music_done) |
| R03_0006 | copyprot_code_begin | | R03_0421..0558 | seek_record, clear_answer, ask, take_answer, open_file, read_record, set_f8 |
| R03_000B | copyprot_stub | | R03_0591 | cpu_v86_active (früher ems_present) |
| R03_0010 | copyprot_check | | R05_0012 | cd_check |
| R03_0041 | CopyProt::query | | R23_000F/0043 | timer_init (früher r23_000f) / timer_restore |
| R03_0128/018C | CopyProt::encrypt / compare | | R23_005A | timer_isr (BE) |
| R03_01F5/0256 | CopyProt::get_ok / get_ok2 | | R23_01B0/0264 | view_timer_cb / fight_timer_cb |
| R03_0203/0264 | CopyProt::init / fail | | R23_0437/068F | mouse_isr (BE) / long_sqrt |

In namen.tsv stehen die Methoden als freie Namen (`copyprot_query`, `CopyProt_ctor` ...); die Beschreibung nennt
den C++-Namen. Weitere Namen: R43_0130 dos_get_indos_ptr, O030_0000 enter_location, O074_023C check_heroes_down (Prototypen
ergänzt). Daten: g_copyprot (ds:c6ca, früher g_music_ctl), g_tamper_count (c6c6), g_indos_flag (c4ff),
g_copyprot_sum (35c2), g_copyprot_passed (3e6e), g_save_count (3e70), g_old_timer_isr (cb70), Sperren und Zähler
der Zeitgeber, Debug-Rasterschalter 5f04/5f0c, Kartenpunkte c20b..c211, g_mouse_lclick_time (8ba7) u. a.
Neu in s_class.h: `class CopyProt` und `struct pw_rec`. Neu in s_const.h: FILE_PROTECT_LTX, FILE_PASSWORD_DAT,
COPYPROT_MARKER, TIMER_INT sowie GAME_STATE_*. Alle alten Namen bleiben über Kompatibilitäts-#defines am Ende von
s_proto.h gültig.

## Optionen und Besonderheiten

- SEG001.CPP `-3 -Ot` (mit -Og auch gleich). SEG003.CPP `-3 -Ot -Og`. SEG023.C `-3 -Ot` (`#pragma inline`).
  SEG005.CPP bleibt `-3 -Ot D`.
- SEG023: Die 32-Bit-Register werden per Inline-asm gesichert. Das Modul lief im Original über TASM (TASM-Kodierung in
  long_sqrt).
- **Relocation-Reihenfolge:** Code und Daten waren sofort gleich, die EXE wich aber in der MZ-Relocationtabelle ab.
  TASM 3.1 stellt die GRP-Fixup des Interrupt-Prologs an den Satzanfang, im Original steht sie weiter hinten. Deshalb
  setzt `tools/cobjfix.py` jetzt GRP-Fixups hinter ihren Vorgänger aus der ASM-Fassung (`tools.patch`). Für alle
  anderen Module ist das wirkungslos, der Komplettlink bleibt gleich.
- `l01/SEG001.ASM`: Die beiden markierten Aufrufe waren als Datenbytes plus `dw seg` zerlegt. Daran scheiterte cobjfix:
  Die Far-Fixup der C-Fassung wurde als Datenverweis auf DATASEG@ umgebogen, ccmp sah das wegen der Maskierung nicht.
  Die Aufrufe stehen jetzt als `call far ptr F_R03_0010/000B` in der ASM-Datei (`asm_seg001.patch`), Stufe 1 bleibt gleich.
- SEG005: Die Literale von cd_check bilden genau den Literalpool ds:445b..4537. Das bisherige Platzhalterfeld `g_445b[200]`
  entfällt.
- Daten von R01/R03/R23 bleiben wie vereinbart in den ASM-Stücken. Spalte D ist nicht gesetzt, weil die Zuordnung der
  BSS-Teile in dsana „unsicher“ ist. R03 (nur `g_copyprot_passed`) wäre ein einfacher Kandidat.

## Originalfehler

Neu in ORIGINALFEHLER.md: #228–#232 (Handbuchabfrage wirkungslos, doppelter Ortsart-Test im Zeitgeber, ungeprüftes
`_read` und Abbruch per PSP:0000 in cd_check, `_open > 0`). Dazu Kuriositäten: verschleierte Konstanten, set_f8, leeres
`if (mode == 0) ;`.

## Abgabedateien (`/root/ks_baum/abgabe/`)

Alle Patches wurden mit `patch -p1 --dry-run` gegen den aktuellen Hauptbaum geprüft und lassen sich anwenden.

- `inc.patch`: s_class.h, s_const.h, s_data.h, s_proto.h
- `names.patch`: namen.tsv (69 Einträge)
- `csrc.patch` und `csrc/`: SEG001.CPP, SEG003.CPP, SEG023.C neu; SEG005.CPP geändert
- `c_module.patch` und `c_module_zeilen.tsv`
- `tools.patch`: cobjfix.py, GRP-Fixup-Reihenfolge
- `asm_seg001.patch`: l01/SEG001.ASM, markierte Aufrufe als call far
- `leitfaden.patch`: LEITFADEN_C.md, Abschnitt „Welle KS“
- `originalfehler.patch`: ORIGINALFEHLER.md, #228–#232

Nicht mehr gebraucht: tools/genblk.py (Byteblock-Generator) und die Assemblerfassungen l03/l23, die nur noch für
Stufe 1 dienen.

## Vorfall

Um 15:20 hat mein `pkill -f tools/build.sh` auch einen laufenden build.sh im Hauptbaum beendet. Den verwaisten
DOSBox-Lauf habe ich danach per PID beendet. Den Koordinator habe ich sofort informiert. Seitdem baue ich nur noch im
privaten Mount-Namensraum und beende Prozesse nur noch per PID.
