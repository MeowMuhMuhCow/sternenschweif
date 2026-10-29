#!/usr/bin/env python3
"""be_fixes.py - Korrekturen an Bright-Eyes-Quellcode (DSA 1, Revision df74edde).

Alle Stellen hier erzeugen bei Bright Eyes BYTEGLEICHEN Code, rufen aber eine
andere Funktion auf als das Original. Der Segmentvergleich blendet Aufrufziele
aus und konnte sie deshalb nie finden. Gefunden am 17.10.2026 mit
fixupdiff.py / callidx.py / dis.py durch Vergleich der fertig gelinkten EXE
mit SCHICKM.EXE (GOG, CD 3.02 de).

Jede Korrektur: (Datei, Funktion, alter Text, neuer Text, n-tes Vorkommen in
der Funktion, Begruendung). Angewendet nur auf die Arbeitskopie.
"""
import sys, os, re

FIXES = [
    ('seg002.cpp', 'cruft_1', 'AIL_start_sequence(', 'AIL_stop_sequence(', 1,
     'Original stoppt die Musiksequenz, Bright Eyes startete sie'),
    ('seg002.cpp', 'read_sound_cfg', 'bc_close(handle)', 'bc__close(handle)', 1,
     'Original ruft _close (DOS-Ebene), nicht close; bc__close gibt es fuer DOSBox schon'),
    ('seg002.cpp', 'read_music_driver', 'bc_close(handle)', 'bc__close(handle)', 1,
     'Original ruft _close, nicht close'),
    ('seg002.cpp', 'read_digi_driver', 'bc_close(handle)', 'bc__close(handle)', 1,
     'Original ruft _close, nicht close'),
    ('seg002.cpp', 'do_timers', 'ds_readd(DNG07_POISON_TIMER) % MINUTES(15)',
     'ds_readds(DNG07_POISON_TIMER) % MINUTES(15)', 1,
     'Modulo mit Vorzeichen (F_LMOD@ statt F_LUMOD@)'),
    ('seg002.cpp', 'timewarp', 'hour_new = (signed short)(ds_readd(DAY_TIMER) / HOURS(1));',
     'hour_new = (signed short)(ds_readds(DAY_TIMER) / HOURS(1));', 1,
     'Division mit Vorzeichen (F_LDIV@ statt F_LUDIV@)'),
    ('seg004.cpp', 'update_wallclock', '((d % 771) != ds_readws(WALLCLOCK_POS))',
     '((d / 771) != ds_readws(WALLCLOCK_POS))', 1,
     'Original vergleicht die Uhrposition d/771, nicht d%771 (Logikfehler)'),
    ('seg005.cpp', 'draw_fight_screen', 'bc_open((RealPt)ds_readd(STR_TEMP_XX_PTR), 0)',
     'bc__open((RealPt)ds_readd(STR_TEMP_XX_PTR), 0)', 1,
     'Original ruft _open, nicht open'),
    ('seg030.cpp', 'prepare_date_str', '(signed short)(ds_readd(DAY_TIMER) / HOURS(1))',
     '(signed short)(ds_readds(DAY_TIMER) / HOURS(1))', 1,
     'Division mit Vorzeichen'),
    ('seg030.cpp', 'do_talk', 'GUI_get_ptr(', 'GUI_get_ptr2(', 4,
     'Original nimmt hier GUI_get_ptr2 (n-tes Vorkommen im Quelltext; Borland wertet Argumente von rechts aus)'),
    ('seg064.cpp', 'get_passage_travel_hours', 'Bit32u tmp;', 'Bit32s tmp;', 1,
     'tmp ist im Original vorzeichenbehaftet (F_LDIV@ statt F_LUDIV@)'),
    ('seg084.cpp', 'DNG09_handler', 'ds_readds(DAY_TIMER) / 90 * 20', 'ds_readds(DAY_TIMER) % 90 * 20', 1,
     'Original rechnet Rest (F_LMOD@), nicht Division'),
    ('seg097.cpp', 'GUI_1c2', 'GUI_write_char_to_screen(v3, 8, v2);',
     '((void (*)(RealPt, signed short, signed short))GUI_write_char_to_screen_xy)(v3, 8, v2);', 1,
     'Original ruft GUI_write_char_to_screen_xy (gleiche Argumente auf dem Stack)'),
    ('seg110.cpp', 'tevent_043', 'TRV_found_replenish_place(0)', 'TRV_found_camp_place(0)', 1,
     'Original: Lagerplatz, nicht Auffuellplatz'),
    ('seg117.cpp', 'TLK_way_to_ruin', 'test_skill(Real2Host(ds_readd(RUIN_HERO)), TA_AEXTE, 2)',
     'test_attrib(Real2Host(ds_readd(RUIN_HERO)), ATTRIB_GE, 2)', 1,
     'Original prueft die Eigenschaft GE (4), nicht das Talent Aexte (4)'),
    ('seg118.cpp', 'tevent_037', 'GUI_get_ptr(', 'GUI_get_ptr2(', 2,
     'Original nimmt hier GUI_get_ptr2 (n-tes Vorkommen im Quelltext; Borland wertet Argumente von rechts aus)'),
    ('seg058.cpp', 'repair_screen', 'select_hero_ok(', 'select_hero_ok_forced(', 1,
     'Original nimmt hier select_hero_ok_forced'),
]


# Korrekturen ausserhalb von Funktionen: (Datei, alter Text, neuer Text, Begruendung)
GLOBAL_FIXES = [
    ('datseg.cpp', 'signed short g_attack_items[3] = {\n\tITEM_MIASTHMATICUM,\n\tITEM_HYLAILIC_FIRE,',
     'signed short g_attack_items[3] = {\n\tITEM_HYLAILIC_FIRE, /* be_fixes: Reihenfolge wie im Original */\n\tITEM_MIASTHMATICUM,',
     'Im Original steht HYLAILIC_FIRE (0xef) vor MIASTHMATICUM (0xee).'),
    ('seg001.cpp',
     'cd_buf1[252] = 0x0c;',
     'cd_buf1[0] = 0x0c; /* be_fixes */',
     'CD_unused1: Original schreibt das IOCTL-Kommando 0x0c ins erste Byte von cd_buf1 (es:0xfc), nicht nach cd_buf1[252].'),
    ('seg001.cpp',
     '\tCD_driver_request((driver_request*)RealMake(reloc_game + CDA_DATASEG, 0x38));\n\tasm {nop}',
     '\tCD_driver_request(&req[2]); /* be_fixes */',
     'CD_0432: Original uebergibt &req[2] (push offset mit Verweis, 3 Bytes) - kein Konstanten-Push + nop.'),
    ('seg001.cpp',
     '\t\tCD_driver_request((driver_request*)RealMake(reloc_game + CDA_DATASEG, 0x38));\n\t\tasm{nop}',
     '\t\tCD_driver_request(&req[2]); /* be_fixes */',
     'wie oben, in der Schleife.'),
    ('seg001.cpp',
     '/* TODO: write this code in C */\n\tasm {\n\n\t\tmov bx, si\n\t\tshl bx, 3\n\t\tmov ax, CDA_DATASEG\n\t\tmov es, ax\n\t\tmov al, [es:bx+0x10b]\n\t\tmov ah, 0\n\t\tshl ax, 8\n\n\t\tmov bx, si\n\t\tshl bx, 3\n\t\tmov dx, CDA_DATASEG\n\t\tmov es, dx\n\t\tmov dl, [es:bx+0x10a]\n\t\tmov dh, 0\n\n\t\tadd ax,dx\n\n\t\tmov bx, si\n\t\tshl bx, 3\n\t\tmov dx, CDA_DATASEG\n\t\tmov es, dx\n\t\tmov dl, [es:bx+0x10c]\n\t\tmov dh, 0\n\n\t\tmov bx, CDA_DATASEG\n\t\tmov es, bx\n\t\tmov [es:0x9c], dx\n\t\tmov [es:0x9a], ax\n\n\t}\n\t',
     '/* TODO: write this code in C */\n\tasm {\n\n\t\tdb 0x8b, 0xde\t/* mov bx, si (Kodierung wie im Original) */\n\t\tshl bx, 3\n\t\tmov ax, CDA_DATASEG\n\t\tmov es, ax\n\t\tmov al, [es:bx+0x10b]\n\t\tmov ah, 0\n\t\tshl ax, 8\n\n\t\tdb 0x8b, 0xde\t/* mov bx, si (Kodierung wie im Original) */\n\t\tshl bx, 3\n\t\tmov dx, CDA_DATASEG\n\t\tmov es, dx\n\t\tmov dl, [es:bx+0x10a]\n\t\tmov dh, 0\n\n\t\tdb 0x03, 0xc2\t/* add ax, dx (Kodierung wie im Original) */\n\n\t\tdb 0x8b, 0xde\t/* mov bx, si (Kodierung wie im Original) */\n\t\tshl bx, 3\n\t\tmov dx, CDA_DATASEG\n\t\tmov es, dx\n\t\tmov dl, [es:bx+0x10c]\n\t\tmov dh, 0\n\n\t\tmov bx, CDA_DATASEG\n\t\tmov es, bx\n\t\tmov [es:0x9c], dx\n\t\tmov [es:0x9a], ax\n\n\t}\n\t',
     'seg001_00c1: der Inline-Assembler (BASM) kodiert mov bx,si / add ax,dx als 89 F3 / 01 D0, das Original hat 8B DE / 03 C2 -> Bytes direkt.'),
    ('seg048.cpp',
     '\t\t\t\t\t\tbreak;\n\t\t\t\t\t}\n\t\t\t\t\tcase 5: {\n\t\t\t\t\t\t/* TODO: different code is generated here */\n\t\t\t\t\t\tds_writew(STATUS_PAGE_MODE, 1);\n\t\t\t\t\t\tds_writew(REQUEST_REFRESH, 1);\n\t\t\t\t\t\tbreak;\n\t\t\t\t\t}\n\t\t\t\t\tcase 6: {\n\t\t\t\t\t\treset_item_selector();\n\t\t\t\t\t\tds_writew(STATUS_PAGE_MODE, 3);\n\t\t\t\t\t\tds_writew(REQUEST_REFRESH, 1);\n\t\t\t\t\t\tbreak;\n\t\t\t\t\t}\n\t\t\t\t\tcase 7: {\n\t\t\t\t\t\tif (host_readbs(hero2 + HERO_TYPE) < HERO_TYPE_WITCH) {',
     '\t\t\t\t\t\tbreak;\n\t\t\t\t\t}\n\t\t\t\t\tcase 5:\n\t\t\t\t\t\tds_writew(STATUS_PAGE_MODE, 1);\n\t\t\t\t\t\tif (0) {\n\t\t\t\t\tcase 6:\n\t\t\t\t\t\treset_item_selector();\n\t\t\t\t\t\tds_writew(STATUS_PAGE_MODE, 3);\n\t\t\t\t\t\t}\n\t\t\t\t\t\tds_writew(REQUEST_REFRESH, 1);\n\t\t\t\t\t\tbreak;\n\t\t\t\t\tcase 7: {\n\t\t\t\t\t\tif (host_readbs(hero2 + HERO_TYPE) < HERO_TYPE_WITCH) {',
     'status_menu, Inventarseite Fall 5/6: Original springt aus den frueheren Faellen VORWAERTS in das gemeinsame Ende (REQUEST_REFRESH=1). Attic kompilierte C (vermutlich goto); C++ erzeugt bei goto einen toten Zusatzsprung, deshalb if (0) { case ...: } - gleiche Bytes.'),
    ('seg048.cpp',
     '\t\t\t\t\t\tbreak;\n\t\t\t\t\t}\n\t\t\t\t\tcase 2: {\n\t\t\t\t\t\tds_writew(STATUS_PAGE_MODE, 1);\n\t\t\t\t\t\tds_writew(REQUEST_REFRESH, 1);\n\t\t\t\t\t\tbreak;\n\t\t\t\t\t}\n\t\t\t\t\tcase 3: {\n\t\t\t\t\t\tds_writew(STATUS_PAGE_MODE, 2);\n\t\t\t\t\t\tds_writew(REQUEST_REFRESH, 1);\n\t\t\t\t\t\tbreak;\n\t\t\t\t\t}\n\t\t\t\t\tcase 4: {\n\t\t\t\t\t\tif (host_readbs(hero2 + HERO_TYPE) < HERO_TYPE_WITCH) {',
     '\t\t\t\t\t\tbreak;\n\t\t\t\t\t}\n\t\t\t\t\tcase 2:\n\t\t\t\t\t\tds_writew(STATUS_PAGE_MODE, 1);\n\t\t\t\t\t\tif (0) {\n\t\t\t\t\tcase 3:\n\t\t\t\t\t\tds_writew(STATUS_PAGE_MODE, 2);\n\t\t\t\t\t\t}\n\t\t\t\t\t\tds_writew(REQUEST_REFRESH, 1);\n\t\t\t\t\t\tbreak;\n\t\t\t\t\tcase 4: {\n\t\t\t\t\t\tif (host_readbs(hero2 + HERO_TYPE) < HERO_TYPE_WITCH) {',
     'status_menu, Talentseite Fall 2/3: Original springt aus den frueheren Faellen VORWAERTS in das gemeinsame Ende (REQUEST_REFRESH=1). Attic kompilierte C (vermutlich goto); C++ erzeugt bei goto einen toten Zusatzsprung, deshalb if (0) { case ...: } - gleiche Bytes.'),
    ('seg048.cpp',
     '\t\t\t\t\t\tbreak;\n\t\t\t\t\t}\n\t\t\t\t\tcase 2: {\n\t\t\t\t\t\tds_writew(STATUS_PAGE_MODE, 1);\n\t\t\t\t\t\tds_writew(REQUEST_REFRESH, 1);\n\t\t\t\t\t\tbreak;\n\t\t\t\t\t}\n\t\t\t\t\tcase 3: {\n\t\t\t\t\t\tds_writew(STATUS_PAGE_MODE, 2);\n\t\t\t\t\t\tds_writew(REQUEST_REFRESH, 1);\n\t\t\t\t\t\tbreak;\n\t\t\t\t\t}\n\t\t\t\t\tcase 4: {\n\t\t\t\t\t\tds_writew(STATUS_PAGE_MODE, 3);\n\t\t\t\t\t\tds_writew(REQUEST_REFRESH, 1);\n\t\t\t\t\t\tbreak;\n\t\t\t\t\t}\n\t\t\t\t\tcase 5: {\n\t\t\t\t\t\tif (ds_readws(STATUS_PAGE_MODE) == 4)\n\t\t\t\t\t\t\tds_writew(STATUS_PAGE_MODE, 5);\n\t\t\t\t\t\telse\n\t\t\t\t\t\t\tds_writew(STATUS_PAGE_MODE, 4);\n\t\t\t\t\t\tds_writew(REQUEST_REFRESH, 1);\n\t\t\t\t\t\tbreak;\n\t\t\t\t\t}\n\t\t\t\t\tcase 6: {\n\t\t\t\t\t\tflag1 = 1;',
     '\t\t\t\t\t\tbreak;\n\t\t\t\t\t}\n\t\t\t\t\tcase 2:\n\t\t\t\t\t\tds_writew(STATUS_PAGE_MODE, 1);\n\t\t\t\t\t\tif (0) {\n\t\t\t\t\tcase 3:\n\t\t\t\t\t\tds_writew(STATUS_PAGE_MODE, 2);\n\t\t\t\t\t\tif (0) {\n\t\t\t\t\tcase 4:\n\t\t\t\t\t\tds_writew(STATUS_PAGE_MODE, 3);\n\t\t\t\t\t\tif (0) {\n\t\t\t\t\tcase 5:\n\t\t\t\t\t\tif (ds_readws(STATUS_PAGE_MODE) == 4)\n\t\t\t\t\t\t\tds_writew(STATUS_PAGE_MODE, 5);\n\t\t\t\t\t\telse\n\t\t\t\t\t\t\tds_writew(STATUS_PAGE_MODE, 4);\n\t\t\t\t\t\t}\n\t\t\t\t\t\t}\n\t\t\t\t\t\t}\n\t\t\t\t\t\tds_writew(REQUEST_REFRESH, 1);\n\t\t\t\t\t\tbreak;\n\t\t\t\t\tcase 6: {\n\t\t\t\t\t\tflag1 = 1;',
     'status_menu, Zauberseite Fall 2-5: Original springt aus den frueheren Faellen VORWAERTS in das gemeinsame Ende (REQUEST_REFRESH=1). Attic kompilierte C (vermutlich goto); C++ erzeugt bei goto einen toten Zusatzsprung, deshalb if (0) { case ...: } - gleiche Bytes.'),
    ('seg106.cpp',
     '#if defined(__BORLANDC__)\n\t/* this assembler code here is only for comparization the disassemblies */\n\t\tasm { db 0x9a, 0xff, 0xff, 0x00, 0x00 }\n\t\tasm { db 0x9a, 0xff, 0xff, 0x00, 0x00 }\n#endif\n',
     '/* be_fixes: Bright-Eyes-Platzhalter (zwei unechte far calls 0000:FFFF) entfernt */\n',
     'pass_item: Bright Eyes fuellte hier 10 Bytes mit "call 0000:FFFF" auf, nur fuer den Disassembly-Vergleich. '
     'Im gebauten Spiel springt "if (pos2 < 7)" bei Ruecksack-Plaetzen genau dorthin -> Absturz beim Weitergeben von Gegenstaenden.'),
    ('seg106.cpp',
     '#if defined(__BORLANDC__)\n\t\t\tdesc1_5 = desc1_5;\n#endif\n',
     '',
     'pass_item: Fuellbefehl desc1_5 = desc1_5 (6 Bytes) gehoerte zu den Platzhaltern oben.'),
    ('seg106.cpp',
     '\t\t\t\t(char*)Real2Host(GUI_names_grammar(2, item1, 0)));\n\n\n\t\t\tGUI_output(Real2Host(ds_readd(DTP2)));',
     '\t\t\t\t(char*)Real2Host(GUI_names_grammar(2, item1, 0)));\n\npass_item_msg1: /* be_fixes */\n\t\t\tGUI_output(Real2Host(ds_readd(DTP2)));',
     'pass_item: unbenutzte Sprungmarke vor GUI_output. Borland legt gleiche Befehlsenden zusammen (Cross-Jumping), eine Marke unterbricht das. Ohne sie springt der Nachbau aus dieser Meldung in eine andere gleich endende (47 Bytes kuerzer). Mit beiden Marken ist SEG106 bytegleich.'),
    ('seg106.cpp',
     '\t\t\t\t(char*)(hero2 + HERO_NAME2));\n\n\t\t\tGUI_output(Real2Host(ds_readd(DTP2)));',
     '\t\t\t\t(char*)(hero2 + HERO_NAME2));\n\npass_item_msg2: /* be_fixes */\n\t\t\tGUI_output(Real2Host(ds_readd(DTP2)));',
     'pass_item: unbenutzte Sprungmarke vor GUI_output. Borland legt gleiche Befehlsenden zusammen (Cross-Jumping), eine Marke unterbricht das. Ohne sie springt der Nachbau aus dieser Meldung in eine andere gleich endende (47 Bytes kuerzer). Mit beiden Marken ist SEG106 bytegleich.'),
    ('seg000.h', '#define bc_close close\n', '#define bc_close close\n#define bc__close _close\t/* be_fixes */\n',
     'Borland-Seite: bc__close ist fuer DOSBox schon deklariert, hier fehlte die Zuordnung zu _close.'),
    ('seg000.h', '#define bc_open open\n', '#define bc_open open\n#define bc__open _open\t/* be_fixes */\n',
     'Borland-Seite: neuer Wrapper bc__open fuer _open.'),
    ('seg000.h', 'Bit16s bc_open(RealPt, Bit16u);\n', 'Bit16s bc_open(RealPt, Bit16u);\nBit16s bc__open(RealPt, Bit16u);\t/* be_fixes */\n',
     'DOSBox-Seite: Deklaration bc__open.'),
    ('seg000.cpp', 'void bc_qsort(RealPt base, Bit16u nmemb, Bit16u size, RealPt fcmp)\n',
     '/* be_fixes: _open der C-Bibliothek, im Original bei seg000:0x3636 */\nBit16s bc__open(RealPt fname, Bit16u attrib)\n{\n\tCPU_Push16(attrib);\n\tCPU_Push32(fname);\n\tCALLBACK_RunRealFar(reloc_game + 0, 0x3636);\n\tCPU_Pop32();\n\tCPU_Pop16();\n\n\treturn reg_ax;\n}\n\nvoid bc_qsort(RealPt base, Bit16u nmemb, Bit16u size, RealPt fcmp)\n',
     'DOSBox-Seite: bc__open ruft _open im Originalprogramm auf (wie bc_open bei 0x34c7 und bc__close bei 0x2eda).'),
    ('seg092.cpp', '\nvoid (*func)(RealPt);\n', '\n/* be_fixes: unbenutzte Variable func entfernt */\n',
     'func wird nirgends benutzt; im Original gibt es sie nicht. Sie belegte 4 Bytes BSS '
     'und verschob alle BSS-Variablen der C-Bibliothek um 4 Bytes.'),
]


def apply(w):
    done = 0
    for fn, old, new, why in GLOBAL_FIXES:
        p = os.path.join(w, fn)
        s = open(p, encoding='latin1').read()
        if s.count(old) != 1:
            raise SystemExit(f'{fn}: Text fuer globale Korrektur nicht eindeutig gefunden')
        open(p, 'w', encoding='latin1').write(s.replace(old, new))
        done += 1
    for fn, func, old, new, nth, why in FIXES:
        p = os.path.join(w, fn)
        lines = open(p, encoding='latin1').read().split('\n')
        s = next(i for i, l in enumerate(lines)
                 if re.match(r'^[A-Za-z].*[\s\*]' + re.escape(func) + r'\s*\(', l) and not l.rstrip().endswith(';'))
        e = next(i for i in range(s + 1, len(lines)) if lines[i].startswith('}'))
        body = '\n'.join(lines[s:e + 1])
        k = -1
        for _ in range(nth):
            k = body.index(old, k + 1)
        body = body[:k] + new + body[k + len(old):]
        lines[s:e + 1] = body.split('\n')
        open(p, 'w', encoding='latin1').write('\n'.join(lines))
        done += 1
    print(f'{done} Korrekturen angewendet')


if __name__ == '__main__':
    apply(sys.argv[1])
