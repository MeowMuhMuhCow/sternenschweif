/* OVL171 - Overlay O171 (Segment 0x22b5 in SCHWEIF.EXE): Heldenerschaffung (class CharGen) - Helden uebernehmen.
 * chargen_save_hero(g): fehlenden Typ/Namen ueber chargen_select_type/chargen_enter_name nachholen (sonst 0), Rueckfrage
 * txt[0], bei offenen Steigerungen Auswahl (txt[69]: automatisch / so lassen / zurueck; chargen_auto_inc), dann
 * Versionskennung (g_dsa_version_string, g_version_a..d) und money_share = 1 eintragen, Dateinamen nach
 * g_text_output_buf, Held in FILE_HEROES_ALL und - falls dort schon vorhanden - FILE_HEROES speichern
 * (herofile_find/herofile_save), gleichnamige Helden in g_heroes (6) per Strukturkopie ueberschreiben,
 * mouse_reset, g->saved = 1. chargen_draw_type_pic(g, dst, n): Palette n (0x60 Bytes) aus Datei 0x1ef nach
 * g->type_pal, Typbild n aus 0x1ee (nvfile_load) nach dst (0x10, 8); Maus aus/an, wenn dst = g_vga_buf.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL171.CPP   -> 1230/1230 Bytes GLEICH (18.10.2026), cobjfix laeuft
 * - C++ wegen inline-Memberfunktionen: set_ver0..3(v) erzeugen die Parameterkopien [bp-0fh]..[bp-12h]
 *   (je Aufruf eine neue Byte-Variable), is_mage() { return type >= 7; } materialisiert 1/0 + or ax,ax.
 * - -Og noetig fuer chargen_draw_type_pic: &g->dummy wird als Zwischenwert [bp-2] einmal berechnet und zweimal
 *   gepusht; chargen_save_hero waere auch ohne -Og gleich.
 * - -Oi: strcpy/strcmp inline; *q = *g->hero ueber SCOPY@ (0x5ef Bytes).
 * - !g->hero->type (cbw/or) gegenueber q->type != 0 (cmp byte).
 * - Referenz-OBJ fehlt in ref_obj: Vergleich gegen lnk/OVL171.OBJ.
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 */
#include <stdio.h>
#include <string.h>
#include "schweif.h"

signed short chargen_save_hero(CharGen far *g)
{
	signed short r;
	signed short slot;
	signed short ret;
	signed short keep;
	signed short old;
	struct hero far *q;

	ret = 0;
	keep = 0;
	if (!g->hero->type) {
		dlg_message(g_dialog, g->txt[22]);
		chargen_select_type(g);
		if (!g->hero->type)
			return 0;
	}
	if (!g->hero->name[0]) {
		dlg_message(g_dialog, g->txt[45]);
		chargen_enter_name(g);
		if (!g->hero->name[0])
			return 0;
	}
	if (dlg_yes_no(g_dialog, g->txt[0]) == 1) {
		if ((g->hero->spell_incs > 0 && g->hero->is_mage()) || g->hero->skill_incs > 0) {
			old = dlg_set16(g_dialog, 5);
			r = dlg_choose(g_dialog, g->txt[69], 3, g->txt[72], g->txt[73], g->txt[74]);
			dlg_set16(g_dialog, old);
			if (r == -1 || r == 3)
				keep = 1;
			else {
				if (r == 1)
					chargen_auto_inc(g);
				keep = 0;
			}
		}
		if (keep == 0) {
			strcpy(g->hero->version, g_dsa_version_string);
			g->hero->set_ver0(g_version_a);
			g->hero->set_ver1(g_version_c);
			g->hero->set_ver2(g_version_b);
			g->hero->set_ver3(g_version_d);
			g->hero->money_share = 1;
			strcpy(g_text_output_buf, g->hero->file_name);
			ret = 1;
			if (herofile_find(g_text_output_buf, &slot, FILE_HEROES_ALL) && dlg_yes_no(g_dialog, g->txt[55]) == 0)
				ret = 0;
			if (ret != 0) {
				mouse_set_cursor(5, 1);
				herofile_save(g->hero, FILE_HEROES_ALL);
				if (herofile_find(g_text_output_buf, &slot, FILE_HEROES))
					herofile_save(g->hero, FILE_HEROES);
				for (r = 0, q = g_heroes; r < 6; r++, q++)
					if (q->type != 0 && !strcmp(g->hero->file_name, q->file_name))
						*q = *g->hero;
				mouse_reset();
				g->saved = 1;
			}
		}
	}
	return ret;
}

void chargen_draw_type_pic(CharGen far *g, void far *dst, signed short n)
{
	file_open(g_file_main, 0x1ef);
	file_seek(g_file_main, (signed long)(n * 0x60), SEEK_SET);
	file_read(g_file_main, g->type_pal, 0x60L);
	file_close(g_file_main);
	g->nvf.load(0x1ee, gb_data(g->bmp_type), 0, 0, n, NVF_OUT_RLE_DENSE, &g->dummy, &g->dummy, SCREEN_W);
	if (dst == g_vga_buf)
		mouse_hide(0);
	set_palette(g->type_pal, 0, 0x20);
	gb_blit(g->bmp_type, dst, 0x10, 8);
	if (dst == g_vga_buf)
		mouse_show(0);
}
