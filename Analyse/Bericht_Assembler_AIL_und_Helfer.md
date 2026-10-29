# Abschlusswelle B – Assembler-Module lesbar (Bericht)

Stand 19.10.2026, Arbeitsbaum `/root/zw_b`. Bereich nach der Aufteilung: SEG022 (AIL) und die zwölf kleinen
Module SEG009, 026, 027, 031, 033, 034, 035, 036, 039, 043, 046, O191, O270. SEG024/SEG025 hat Auftrag C
übernommen; dort wurde nichts geändert.

## Ergebnis

| Prüfung | Ergebnis |
|---|---|
| Je Modul: OBJ-Vergleich (Segmentbytes, Fixups, Publics, Externs) gegen die Ausgangsfassung | 14 von 14 GLEICH |
| Stufe 1 (`NUR_ASM=1 bash tools/build.sh`) | BYTEGLEICH |
| Voller Build (`bash tools/build.sh`, einmal am Ende) | **BYTEGLEICH mit dem Original** (MD5 da3b59ce86a82e4b8bb14d36ed37d4d0; Stufe 2: 301 C-Module, 145 mit eigenen Daten dscheck GLEICH, Linker-Meldungen 0) |
| Funktionen ohne Namen | **50 benannt**, alle aus meinem Bereich (R22 36, R39 7, R36 3, R43 3, R31 1). In namen.tsv standen 60 als „offen“ (der Abschlussbericht zählt 63). Offen bleiben 10: 5 in SEG024/025 (Auftrag C) und 5 in den C-Modulen O030 (2), O097, O118, O122 (Auftrag A). Zusätzlich nachgetragen: O191 (hatte keinen Eintrag) und R43 get_color (fehlte in namen.tsv) |

## Vorgehen (bytegleich und lesbar)

- **Linknamen bleiben.** `F_Rnn_xxxx` steht in allen anderen ASM-Modulen, den Datenstücken, `ovlpub.py` und in den
  C-Fixups über cobjfix. Der sprechende Name steht als nahe Marke direkt hinter `F_… label far` (Kommentar
  „Linkname von …“). Kein neues Public, keine Änderung an OBJ oder MAP.
- **Kopfkommentar je Modul**: Inhalt, Aufrufer, Aufrufkonvention, Register, Daten, Herkunft. **Kommentarblock je
  Prozedur**: Ein/Aus, Ablauf, Locals, Originalfehler.
- **Sprungmarken** `L_xxxx` wurden zu `@@…` (mit `locals`), Kommentare an den wichtigen Stellen. Die Adressmarker
  `;@nnnn` bleiben als Verweis auf das Binärbild.
- **Daten über `equ`.** Datenadressen (`ds:[g_xms_entry]`, `cs:[timer_status]`) und unmittelbare Code-Offsets
  (`API_timer_ofs`) sind Konstanten. Marken ergäben Fixups, die das Original nicht hat.
- **Datenbereiche**, die asm2tasm als Befehle zerlegt hatte (AIL-Daten 0008–03C1 und 0D84–0E81, GDT in SEG035), stehen
  jetzt als `db/dw/dd`. Nullfelder über das Makro `ZEROS n` (rept), nicht über `dup`: `dup` ergibt LIDATA-Sätze, an
  denen `tools/objorder.py` abbricht („LIDATA nicht unterstuetzt“). Stufe 1 war zwar auch damit bytegleich, die Fassung
  ohne LIDATA hält den Ablauf aber unverändert.
- **Makros statt Wiederholung**: SEG033 `GET_BIT` (20 Bitlesefolgen, 553 → 295 Zeilen Code-Anteil), SEG022 `POP_F`
  (21×), `PUSH_C` (19×), `ADD_PTR` (10×), `FAR_TO_HUGE` (1×). Die letzten vier sind dieselben Makros wie in Miles'
  AIL.MAC.
- Befehle mit `[reg+Offset < 80h]` bleiben `db`, weil das Original 16-Bit-Verschiebungen codiert. Der symbolische
  Befehl steht im Kommentar.
- Werkzeuge in `abgabe/werkzeug/`: `annot.py` (Spec → Kopf, Marken, Kommentare, Ersetzungen), `pp_ail.py` (AIL-Makros,
  symbolische CS-Adressen), `pp_macro.py` (GET_BIT), `spec/*.spec` je Modul. Sie arbeiten auf der Ausgangsfassung, ein
  Modul lässt sich damit jederzeit neu erzeugen. Die Regeln stehen kurz in `LEITFADEN_C.md` („Handgeschriebenes TASM“).

## Module

| Modul | Inhalt | benannte Prozeduren (davon neu) | wichtigste Funktionen | Besonderheiten |
|---|---|---|---|---|
| SEG009 | huge-Zeiger | 1 (0) | huge_ptr_diff | Eine zweite, unbenutzte Kopie der Routine vor dem Einsprung (`huge_ptr_diff_copy`) |
| SEG022 | **Miles AIL 2.14** | 108: 76 mit Linknamen (36 neu) + 7 interne + 25 Treiberstubs ohne Linknamen | find_proc, call_driver, API_timer, program_timers, AIL_register_timer, AIL_register_driver, AIL_init_driver, DIGPAK-Schale DP_* | Version 2.14 vom 24.09.1992: `DP_handle` in register_driver und das Zurücksetzen von timer_value beim ersten Zeitgeber sind enthalten, `current_rev` = 211 wie in der Quelle. USE_INT8, DIGPAK an, DIGPAK_SHARE aus, 8086-Code. Alle Namen wie in Miles' AIL.ASM/AIL.INC, Funktionsnummern der Stubs symbolisch (AIL_INDEX_VOC_BLK …), DIGPAK-Funktionstabelle mit Zielen. Fehler #300 |
| SEG026 | Schriftzeichen | 2 + 3 tote (0) | glyph_blit, glyph_blit_bg | Toter Debug-Code: Wächter für den Overlay-Lader (int 3Fh) hält das Spiel an, wenn ein Overlay während einer ISR geladen würde (ovl_trap_isr/on/off). Fehler #303 |
| SEG027 | Reisekarte | 1 (0) | tmap_blit_rows | Ruft EMS_norm_ptr (R36) near über den gemeinsamen Codeabschnitt (als `db`). Daten g_tmap_* |
| SEG031 | Tastatur | 2 (1) | clear_kbd, **wait_key_clear** | – |
| SEG033 | PowerPacker | 1 (0) | ppDecrunch | Makro GET_BIT; Bitstrom-Format im Kopf beschrieben |
| SEG034 | Zeilen/Spalten | 8 (0) | fill_line, fill_v_line, vga_save/restore_row/col | setzt DF = 0 voraus |
| SEG035 | Flat Real Mode | 3 (0) | flat_enter, flat_leave, flat_copy | GDT und Arbeitszellen im Codesegment, selbstmodifizierende far-Sprünge. Fehler #306, #307 |
| SEG036 | EMS | 10 (3 + EMS_installed) | EMS_norm_ptr, **EMS_get_frame_ptr**, **EMS_save/restore_page_map** | Namen wie Bright-Eyes (DSA 1 seg010). Bis auf EMS_norm_ptr und EMS_get_num_pages_unalloced im Spiel unbenutzt (wohl DSA-1-Erbe) |
| SEG039 | XMS | 14 (7) | xms_present, xms_alloc_kb, xms_move, xms_lock, **xms_local_a20_on/off**, **xms_no_driver**, **xms_umb_alloc/free**, **xms_largest_kb**, **xms_a20_state** | Rückgabe als long: AX = Wert, DH = Fehlercode. Fehler #304, #305 |
| SEG043 | Video | 9 (3) | set_video_mode, **save_display_stat** (BE-Name), restore_video_mode, get_palette, **vga_set_color/vga_set_palette** (unbenutzt) | get_color fehlte in namen.tsv (stand nur in manuell_res.tsv), nachgetragen |
| SEG046 | Bmp-Zeilen | 3 (0) | blit_lines, grab_lines, scale_lines | scale_lines erzeugt lodsb/stosb-Code und ruft ihn far auf. Fehler #301, #302 |
| O191 | Maskenblit | 1 (1) | **blit_masked_screen** | Overlay ohne Aufrufer, hatte keinen Eintrag in namen.tsv |
| O270 | FLI | 8 (0) | fli_lc, fli_brun, fli_color_dac … | fli_black/fli_copy zählen Worte, nicht Bytes (Beschreibung korrigiert) |

Fett = neu benannt. Aufrufer aus dem Spiel gibt es für 25 der AIL-Stubs nicht (kein Linkname im EXE).

## Namen und Header

- `names/namen.tsv`: 98 Funktionszeilen aktualisiert bzw. neu (alle AIL-Namen jetzt Quelle `AIL`, Sicherheit hoch),
  18 Datennamen neu (g_old_int3f, g_tmap_*, g_ems_frame_ptr, g_xms_present, g_xms_entry, g_scale_*).
  Handliste: `names/daten/manuell_zwb.tsv`. Neue Quelle `AIL` in `KONVENTIONEN.md`.
- `names/inc/s_proto.h`: vier Prototypen umbenannt (wait_key_clear, xms_local_a20_on, xms_local_a20_off,
  save_display_stat), Block `/* Abschlusswelle B … */` vor dem `#endif` mit den vier Kompatibilitäts-#defines.
- `s_data.h`: unverändert, kein C-Modul benutzt die neuen Datennamen.
- Kollision vermieden: `g_ems_frame` gibt es schon (ds:b16c), die R36-Kopie heißt `g_ems_frame_ptr`.

## Originalfehler (#300–#307, in ORIGINALFEHLER.md)

| # | Wo | Kurz | Schwere |
|---|---|---|---|
| 300 | R22 DP_format_sb (AIL 2.14) | `add sp,4` statt 2, DS kommt falsch zurück. Nur mit DIGPAK-Treiber | niedrig |
| 301 | R46 blit_lines BLIT_OR | `loop` springt in die Kopierschleife | niedrig (unbenutzt) |
| 302 | R46 blit_lines rückwärts | SI/DI eins zu weit, DF bleibt gesetzt | niedrig (unbenutzt) |
| 303 | R26 Overlay-Wächter | Farbe je Sperre geladen, immer Rot ausgegeben | niedrig (tot) |
| 304 | R39 xms_no_driver | fällt in xms_present und meldet Erfolg | niedrig |
| 305 | R39 xms_move | liest g_xms_entry über das Segment des Auftrags | niedrig |
| 306 | R35 + Aufrufer | flat_enter-Ergebnis ignoriert, flat_leave ohne V86-Prüfung | mittel (vermutet) |
| 307 | R35 flat_copy | ungerades letztes Byte fehlt | niedrig |

## Offen / Hinweise

- **Datenzuordnung:** "EMMXXXX0" (ds:8692..8699) gehört inhaltlich zu R36 (EMS_installed), liegt aber im Stück DMR27
  (Ende von `_g_8678`). Für Stufe 1 ist das egal. Für eine spätere Datenfassung von R36 wäre die Grenze 8692 statt 869a zu
  prüfen (`dsana`).
- **g_78d8** (ds:78d8, R23/O199) ist die Wiedereintrittssperre der Maus-ISR. Vorschlag `g_mouse_isr_lock`. Nicht
  umbenannt, weil SEG023.C und OVL199 Auftrag A gehören. SEG026 benutzt den Namen nur als Konstante.
- `FLI_SCREEN_SIZE` (s_const.h) ist mit „320 x 200 Bytes“ kommentiert, gemeint sind 32000 Worte. Nicht geändert.
- Nicht gemacht: Stackparameter als Textmakros (`[bp+6]` steht weiter im Code, die Bedeutung im Prozedurkopf).

## Abgabe

| Datei | Inhalt |
|---|---|
| `asm.patch` | die 14 ASM-Dateien (`l09 l22 l26 l27 l31 l33 l34 l35 l36 l39 l43 l46 o191 o270`) |
| `names.patch` | `names/namen.tsv`, `names/daten/manuell_zwb.tsv` (neu), `names/inc/s_proto.h`, `names/KONVENTIONEN.md` |
| `andere.patch` | `ORIGINALFEHLER.md`, `LEITFADEN_C.md` |
| `../ausgang.tgz` | Ausgangsstand (l*/ o191 o270 names tools ORIGINALFEHLER.md LEITFADEN_C.md) |

Alle Patches mit `-p1`, geprüft mit `patch --dry-run` gegen eine frische Entpackung von `ausgang.tgz`. Danach angewendet:
der Stand gleicht `/root/zw_b` in allen Dateien des Ausgangsumfangs.

Quellen für die AIL: Miles Design, AIL 2.x SDK (AIL.ASM, AIL.INC, AIL.MAC, AIL.H), freie Quelle:
https://github.com/Tronix286/AIL2. Bright-Eyes `src/custom/schick/rewrite_m302de/seg010.cpp` (EMS-Namen).
