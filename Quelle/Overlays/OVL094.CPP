/* OVL094 - Overlay O094 (Segment 0x2126 in SCHWEIF.EXE): Heldenbogen - Seiten zeichnen (Elementfunktionen der
 * Klasse Inv aus O008/O009, Daten struct inv_sheet: Texte txt, Held hero, Nvf-Objekt nvf, Bmp-Zeiger screen..b13a,
 * Puffer buf). draw_page1: Seite 1 - Gottheit g_ttx[god + 4] (txt[18]), 14 Eigenschaften (Wert/Modifikator,
 *   txt[19..21] bzw. g_str_empty_5cfa/5cfb ohne Modifikator) in g_text_buf2 + i * 10, Tabelle txt[22], calc_bp
 *   (Formate g_fmt_load_hl/g_fmt_bp_hl bei BP 1, sonst g_fmt_load/g_fmt_bp), LE/AE-Zeile txt[23] (bei g_game_mode == GAME_MODE_ADVANCED)
 *   bzw. txt[24] mit hero_at/hero_pa.
 * draw_page2: Seite 2 - Kampfwerte txt[26] (g_ttx[75..83], hero_at_weapon/hero_pa_weapon je Waffentalent 0..6,
 *   get_skill_value(7/8) + Mittel aus KL/GE/KK / 4). draw_page3: Seite 3 = inv_draw_skills. draw_page4/draw_page5:
 *   Zauberseiten - acht bzw. vier Ueberschriften g_ttx[219..230] (font_center_x) und Zauberklassen
 *   g_spells_index[8]/g_spells_index2[4] ueber inv_draw_spell mit Spaltentabelle g_spell_cols1/2 (lokale Kopie).
 * draw_inventory: Gepaeck auf den Bildschirm (Plaetze 0..30, 0..14 nur bei g_inv_show_body == 1; Hintergrund status_bg fuer
 *   Plaetze aus g_inv_status_slots, leere Plaetze item_bmp->fill(7 ? 0 : 0xe8), Anzahl bei stapelbaren Gegenstaenden).
 * draw_body_slots: Plaetze 0..14 in den Puffer (Hintergrund Bild 0x10 in body). knapsack_open/knapsack_close:
 *   Rucksack auf-/zuklappen (Animation in 2er-Schritten ueber lokale Bmp-Objekte, nur bei g_anims_on), Auswahl
 *   ggf. auf Platz 15 bzw. reselect.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL094.CPP   -> 5644/5644 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (ohne -Og 3394 Abweichungen: &f0f in DI/[bp-6], i * 6 [bp-6], Epilogkopie in draw_page1).
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * - C++: Elementfunktionen von Inv (class Inv : struct inv_sheet in s_class.h), lokale Bmp-Objekte mit Dtor
 *   (&o, 2) am Block- bzw. Funktionsende, Bmp-Methoden: put gb_blit, get gb_save, put(.., mode) gb_blit_m,
 *   fill gb_fill, set_base gb_set_base, over gb_put; Nvf::load ueber "((Nvf far *)nvf)->load" (eingebettetes
 *   Objekt als Bytefeld, gleiche Bytes).
 * - draw_page4/draw_page5: "struct col_pos8 tab = g_spell_cols1;" (rep movsw ins Local, blockiert SI/DI -> i/k auf
 *   dem Stack). Ueberschriften per Makro: das y des aeusseren font_print wird vor dem inneren font_center_x
 *   gepusht.
 * - knapsack_open: "&p1[(0xa7 - i) * 0x67 + 0x135]" (add ax,[p1] / add ax,135h); "p1 + (..)" ergibt mov dx,[p1] /
 *   add dx,ax (so in knapsack_close).
 * - draw_page1: "hero->attrib[i].normal != v ? txt[19] : g_str_empty_5cfa" je Argument einzeln (zweimal getestet).
 * Originalfehler: keine gefunden.
 */
#include <stdio.h>
#include <stdlib.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:5cb2..5d0c, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
struct col_pos8 g_spell_cols1 = {
	{ { 5, 104, 50 },
	{ 5, 104, 107 },
	{ 112, 209, 50 },
	{ 112, 209, 169 },
	{ 112, 209, 107 },
	{ 217, 315, 50 },
	{ 217, 315, 107 },
	{ 217, 315, 169 } }
};
struct col_pos4 g_spell_cols2 = { { { 5, 104, 50 }, { 5, 104, 130 }, { 112, 209, 50 }, { 217, 315, 50 } } };

void Inv::draw_page1()
{
	signed short r;
	signed short i;
	signed short v;

	font_set_tab(g_font, 0x10e, 0);
	sprintf(g_text_output_buf, txt[18], g_ttx[hero->god + 4]);
	font_print(g_font, g_text_output_buf, 0xd7, 0x2d);
	font_set_tab(g_font, 0xeb, 0);
	font_set_tab(g_font, 0x10e, 1);
	font_set_tab(g_font, 0x122, 2);
	for (i = 0; i <= 13; i++) {
		v = (signed char)(hero->attrib[i].cur + hero->attrib[i].mod);
		sprintf(g_text_buf2 + i * 10, txt[21], hero->attrib[i].normal != v ? txt[19] : "", v,
			hero->attrib[i].normal != v ? txt[20] : "", hero->attrib[i].normal);
	}
	sprintf(g_text_output_buf, txt[22], g_text_buf2, g_text_buf2 + 70, g_text_buf2 + 10, g_text_buf2 + 80, g_text_buf2 + 20, g_text_buf2 + 90,
		g_text_buf2 + 30, g_text_buf2 + 100, g_text_buf2 + 40, g_text_buf2 + 110, g_text_buf2 + 50, g_text_buf2 + 120, g_text_buf2 + 60,
		g_text_buf2 + 130);
	font_print(g_font, g_text_output_buf, 0xd7, 0x3b);
	r = calc_bp(hero);
	if (r == 1) {
		sprintf(g_text_buf2, "\xf1%d\xf0", hero->load);
		sprintf(g_text_buf2 + 50, "\xf1%d\xf0", r);
	} else {
		sprintf(g_text_buf2, "%d", hero->load);
		sprintf(g_text_buf2 + 50, "%d", r);
	}
	if (g_game_mode == GAME_MODE_ADVANCED) {
		sprintf(g_text_output_buf, txt[23], hero->le, hero->le_max, hero->ae, hero->ae_max,
			(signed char)(hero->mr + hero->mr_mod), hero->rs_bonus + hero->rs_bonus2,
			hero->le + (signed char)(hero->attrib[ATTRIB_KK].cur + hero->attrib[ATTRIB_KK].mod), g_text_buf2, g_text_buf2 + 50);
		font_print(g_font, g_text_output_buf, 0xd7, 0x7a);
	} else {
		sprintf(g_text_output_buf, txt[24], hero->le, hero->le_max, hero->ae, hero->ae_max, hero_at(hero), hero_pa(hero),
			(signed char)(hero->mr + hero->mr_mod), hero->rs_bonus + hero->rs_bonus2,
			hero->le + (signed char)(hero->attrib[ATTRIB_KK].cur + hero->attrib[ATTRIB_KK].mod), g_text_buf2, g_text_buf2 + 50);
		font_print(g_font, g_text_output_buf, 0xd7, 0x7a);
	}
}

void Inv::draw_page2()
{
	signed short v;

	font_set_tab(g_font, 0x118, 0);
	font_set_tab(g_font, 0x12c, 1);
	v = ((signed char)(hero->attrib[ATTRIB_KL].cur + hero->attrib[ATTRIB_KL].mod) + (signed char)(hero->attrib[ATTRIB_GE].cur + hero->attrib[ATTRIB_GE].mod) +
		(signed char)(hero->attrib[ATTRIB_KK].cur + hero->attrib[ATTRIB_KK].mod)) / 4;
	sprintf(g_text_output_buf, txt[26], hero->atpa_base,
		g_ttx[75], hero_at_weapon(hero, 0), hero_pa_weapon(hero, 0),
		g_ttx[76], hero_at_weapon(hero, 1), hero_pa_weapon(hero, 1),
		g_ttx[77], hero_at_weapon(hero, 2), hero_pa_weapon(hero, 2),
		g_ttx[78], hero_at_weapon(hero, 3), hero_pa_weapon(hero, 3),
		g_ttx[79], hero_at_weapon(hero, 4), hero_pa_weapon(hero, 4),
		g_ttx[80], hero_at_weapon(hero, 5), hero_pa_weapon(hero, 5),
		g_ttx[81], hero_at_weapon(hero, 6), hero_pa_weapon(hero, 6),
		hero_at(hero), hero_pa(hero),
		g_ttx[82], get_skill_value(hero, 7) + v,
		g_ttx[83], get_skill_value(hero, 8) + v);
	font_print(g_font, g_text_output_buf, 0xd7, 0x32);
}

void Inv::draw_page3()
{
	inv_draw_skills(this);
}

/* Ueberschrift g_ttx[n] zentriert in 100 Pixeln ab x */
#define HEAD(n, x, y) font_print(g_font, g_ttx[n], font_center_x(g_font, g_ttx[n], x, 0x64), y)

void Inv::draw_page4()
{
	signed short i;
	signed short k;
	struct col_pos8 tab = g_spell_cols1;

	font_set_colors(g_font, 0xf9, -1);
	HEAD(219, 5, 0x28);
	HEAD(221, 0x6e, 0x28);
	HEAD(224, 0xd7, 0x28);
	HEAD(220, 5, 0x61);
	HEAD(223, 0x6e, 0x61);
	HEAD(225, 0xd7, 0x61);
	HEAD(222, 0x6e, 0x9f);
	HEAD(226, 0xd7, 0x9f);
	font_set_colors(g_font, 0xff, -1);
	for (i = 0; i < 8; i++)
		for (k = g_spells_index[i].first; g_spells_index[i].first + g_spells_index[i].count > k; k++)
			inv_draw_spell(this, k, g_spells_index[i].first, tab.c[i].x, tab.c[i].x_val, tab.c[i].y);
}

void Inv::draw_page5()
{
	signed short i;
	signed short k;
	struct col_pos4 tab = g_spell_cols2;

	font_set_colors(g_font, 0xf9, -1);
	HEAD(227, 5, 0x28);
	HEAD(228, 5, 0x78);
	HEAD(229, 0x6e, 0x28);
	HEAD(230, 0xd7, 0x28);
	font_set_colors(g_font, 0xff, -1);
	for (i = 0; i < 4; i++)
		for (k = g_spells_index2[i].first; g_spells_index2[i].first + g_spells_index2[i].count > k; k++)
			inv_draw_spell(this, k, g_spells_index2[i].first, tab.c[i].x, tab.c[i].x_val, tab.c[i].y);
}

#define COUNT(dst) \
	if (g_items[hero->inv[i].item_id].stackable) { \
		font_set_colors(g_font, 0xff, 0); \
		itoa(hero->inv[i].quantity, g_text_output_buf, 10); \
		font_print(g_font, g_text_output_buf, g_inv_slot_xy[i].x + 15 - font_text_width0(g_font, g_text_output_buf), g_inv_slot_xy[i].y + 8); \
		font_set_colors(g_font, 0xff, -1); \
	}

void Inv::draw_inventory()
{
	void far *p;
	void far *save;
	signed short i;

	p = item_bmp->data();
	mouse_hide(0);
	status_bg->put(g_vga_buf, 0, 0xc0);
	save = g_font_dst;
	font_set_screen(g_font, g_vga_buf);
	for (i = 0; i <= 30; i++) {
		if (i < 15 && g_inv_show_body == 1 || i >= 15) {
			if (hero->inv[i].item_id) {
				if (is_in_word_array(i, g_inv_status_slots) != -1)
					status_bg->put(g_vga_buf, 0, 0xc0);
				((Nvf far *)nvf)->load(0x0b, p, 0, 0, g_items[hero->inv[i].item_id].gfx, 0, &f0f, &f0f, 0x140);
				item_bmp->put(g_vga_buf, g_inv_slot_xy[i].x + 1, g_inv_slot_xy[i].y + 1);
				COUNT(g_vga_buf)
				if (is_in_word_array(i, g_inv_status_slots) != -1)
					status_bg->get(g_vga_buf, 0, 0xc0);
				draw_slot(g_vga_buf, i, 0);
			} else {
				if (is_in_word_array(i, g_inv_status_slots) != -1)
					status_bg->put(g_vga_buf, 0, 0xc0);
				item_bmp->fill(i == 7 ? 0 : 0xe8);
				item_bmp->put(g_vga_buf, g_inv_slot_xy[i].x + 1, g_inv_slot_xy[i].y + 1);
				if (is_in_word_array(i, g_inv_status_slots) != -1)
					status_bg->get(g_vga_buf, 0, 0xc0);
				draw_slot(g_vga_buf, i, 0);
			}
		}
	}
	font_set_screen(g_font, save);
	mouse_show(0);
}

void Inv::draw_body_slots()
{
	void far *p;
	signed short i;

	((Nvf far *)nvf)->load(0x10, body->data(), 0, 0, 0, 0, &f0f, &f0f, 0x140);
	body->put(buf, 0x50, 0x21, 0);
	p = item_bmp->data();
	for (i = 0; i < 15; i++) {
		if (hero->inv[i].item_id) {
			((Nvf far *)nvf)->load(0x0b, p, 0, 0, g_items[hero->inv[i].item_id].gfx, 0, &f0f, &f0f, 0x140);
			item_bmp->put(buf, g_inv_slot_xy[i].x + 1, g_inv_slot_xy[i].y + 1);
			COUNT(buf)
		}
		draw_slot(buf, i, 0);
	}
	body->get(buf, 0x50, 0x21);
}

void Inv::knapsack_open()
{
	char far *p1;
	char far *p2;
	signed short i;
	Bmp a(0x67, 2, 0);
	Bmp b(0x67, 0xa7, buf);

	if (g_anims_on) {
		Bmp c(0x67, 0xa7);

		c.get(buf, 0x50, 0x21);
		body->put(buf, 0x50, 0x21);
		knapsack->put(buf, 0x50, 0x32, 4);
		screen->put(g_vga_buf, 0, 0);
		if (swap == 0 && slot >= 15)
			draw_slot(g_vga_buf, slot, 0xfd);
		b.over(body, 0, 0, 0);
		p1 = (char far *)body->data();
		p2 = (char far *)c.data();
		for (i = 0xa5; i > 13; i -= 2) {
			Bmp d(0x67, i - 3, &p1[(0xa7 - i) * 0x67 + 0x135]);

			a.set_base(p2 + i * 0x67);
			b.over(&d, 0, 3, 0);
			b.over(&a, 0, i, 0);
			b.over(knapsack, 0, 0x11, 4);
			b.put(g_vga_buf, 0x50, 0x21);
		}
		c.put(g_vga_buf, 0x50, 0x21);
	} else
		screen->put(g_vga_buf, 0, 0);
	if (slot2 == -1 && slot < 15)
		reselect();
	else if (slot2 >= 0 && slot2 < 15)
		slot2 = 15;
}

void Inv::knapsack_close()
{
	char far *p;
	signed short i;
	Bmp a(0x67, 0xa7, buf);

	if (g_anims_on) {
		if (swap == 0 && slot >= 15)
			draw_slot(g_vga_buf, slot, 0xfd);
		a.get(g_vga_buf, 0x50, 0x21);
		p = (char far *)body->data();
		for (i = 13; i <= 0xa4; i += 2) {
			Bmp d(0x67, i, p + (0xa7 - i) * 0x67);

			a.over(&d, 0, 3, 0);
			a.over(knapsack, 0, 0x11, 4);
			a.put(g_vga_buf, 0x50, 0x21);
		}
	}
	body->put(g_vga_buf, 0x50, 0x21);
	knapsack->put(g_vga_buf, 0x50, 0x32, 4);
}
