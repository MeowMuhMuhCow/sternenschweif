/* OVL068 - Overlay O068 (Segment 0x208e in SCHWEIF.EXE): Einkauf beim fahrenden Haendler (class Peddler aus O069,
 * von Peddler::run gerufen) - Gegenstueck zu O228 shop_buy_screen.
 * STUFE D1 (wovd1, 19.10.2026): asm-strcat durch echtes strcat(.., "..") ersetzt - Code GLEICH, das Literal
 *   im eigenen _DATA bildet wovd1/cobjfix_d.py auf ds:5818 ab (Bytes geprueft).
 * peddler_buy_screen: Warenkorb s->buy_qty (je Gegenstand eine Anzahl, g_nr_items Eintraege) anlegen, Warenliste
 * build_wares, Raster 3 x 5 (bei mehr als 15 Waren mit Blaettern, Leiste draw_loc_icons 4 statt 2 Knoepfe),
 * Schleife: Neuzeichnen (Bilder, Summe peddler_buy_draw_money, Gewicht _draw_weight, Plaetze _draw_slots, Raster
 * _draw_grid) bzw. nur ein Feld (_draw_item), Auswahl grid_input, Rahmen peddler_grid_mark; Maus/Taste 0x4900 ->
 * Menue dlg_choose (txt[6..9]); '+'/Enter: Ware in den Korb (stapelbare Ware - item_desc.stackable - Menge per
 * dlg_input, Preis price * unit * Menge, zu wenig Geld txt[15], Vorrat des Haendlers erschoepft txt[60] (sold +
 * Plaetze >= g_peddler_stock), zu schwer/zu viele Plaetze txt[16]); '-'/Backspace: herausnehmen; 0x81 kaufen
 * trade (Summe 0 -> txt[21]), sold += Plaetze (ab g_peddler_stock: sold_out, Ende), Korb leeren; 0x82/0x83
 * blaettern, cols + 0x80 Ende. peddler_buy_draw_grid: Raster mit Preisen (Format txt[10]/[11]/[12] je nach Einheit
 * 1/10/sonst) und Gewicht txt[56]. peddler_buy_draw_item: Bild der Ware (Datei 0x0b) und Anzahl im Korb
 * (zweifarbig, rechtsbuendig). peddler_buy_sum: belegte Plaetze (stapelbar: n / 100 + 1) und Gewicht (weight * n)
 * des Korbs. peddler_buy_draw_money: Korbsumme und Geld der Gruppe (ab 100000 als "D.H" per peddler_format_money,
 * sonst money_to_string). peddler_buy_draw_weight/_draw_slots: Gewicht "a^/^b" und Plaetze
 * (g_peddler_fmt_slots_free/_used). peddler_format_money: sprintf(txt[61], v / 100, v % 100). peddler_grid_mark:
 * Auswahlrahmen versetzen (do_border), Name der Ware in der Statuszeile (screen_textbar, Farbe 0xcd wenn
 * group_mark_can_use == 0).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL068.CPP   -> 4983/4983 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - -Oi: memset als Intrinsic (rep stosw / adc cx,cx / rep stosb). Mit unsigned Laenge
 *   ("sizeof(struct ware) * g_nr_items", "g_nr_items * sizeof(signed short)") wird ZUERST das Ziel geladen und mit
 *   push es/di/ax ... pop cx/di/es gerettet; mit signed Laenge ("g_nr_items * 7") kommt die Laenge zuerst.
 * - -Og: s->sel als Zwischenwert [bp-3ah], Offset (sel + base) * 7 in [bp-3ch], 99 - cnt in [bp-3eh] (doppeltes
 *   Argument); in _draw_grid/_draw_item/_sum entsprechend (&g_items[i] in DI, Segment je Zugriff nachgeladen).
 * - Bmp-Objekte: b1/b2 mitten in peddler_buy_screen deklariert (10 Bytes, bleiben in Reihenfolge vor den
 *   Zwischenwerten), b3 im else-if-Block hinter den Zwischenwerten; new/delete Bmp ueber gb_new_alloc(0, ..)/
 *   gb_delete(p, 3).
 * - _draw_item: row vor col deklariert, damit row DI bekommt (col auf dem Stack); _draw_grid col ohne register auf
 *   dem Stack.
 * - Aeussere Argumente vor dem inneren Aufruf gepusht: font_print(.., x + 17 - font_text_width(..), y) - y zuerst.
 * - strcat(g_text_output_buf, "^/^") in _draw_weight mit bekannter Laenge 4 (repne scasb / dec di / mov cx,4 /
 *   rep movsb, Intrinsic) wie OVL111/OVL238 - geht in C nur mit eigenem Literal (ds:5818).
 * - screen_textbar mit int-Farbe (push word) ueber gecasteten Funktionszeiger TEXTBAR_COL.
 * Originalfehler: peddler_buy_screen '-'/Backspace bei einer stapelbaren Ware mit Anzahl 1: qty wird nicht gesetzt
 *   (Wert der vorigen Aktion bzw. uninitialisiertes SI). '+': die Warnung txt[16] vergleicht die bisherige
 *   Anzahl dieser Ware (cnt) mit den freien Plaetzen statt der belegten Plaetze (nslots).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mem.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:5818..5826, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */

#define NVF_HEADS ((Nvf far *)g_nvf_heads)
/* Text in die Textleiste mit int-Farbe (push word) wie im Original */
#define TEXTBAR_COL(t, c) ((void (*)(void far *, char far *, signed short))screen_textbar)(g_screen, t, c)

void peddler_buy_screen(Peddler far *s)
{
	signed short ans;
	signed short slots;
	signed short cnt;
	signed short nslots;
	signed short cap;
	signed short nweight;
	signed short id;
	signed short cols;
	signed long price;
	signed long total = 0;
	signed short redraw = 1;
	signed short redraw2 = 0;
	signed short keep = 0;
	signed short done = 0;
	Bmp far *tmp;
	signed short qty;

	memset(s->wares, 0, sizeof(struct ware) * g_nr_items);
	s->buy_qty = (signed short far *)mem_alloc(g_nr_items * sizeof(signed short));
	s->build_wares();
	nslots = nweight = 0;
	g_portrait_drag = 0;
	s->old_sel = -1;
	s->sel = 0;
	s->base = 0;
	s->mx = g_mouse_posx;
	s->my = g_mouse_posy;
	g_pp20_index = -1;
	screen_redraw(g_screen);
	mouse_hide(0);
	Bmp b1(0x17, 0x1c);
	b1.get(g_vga_buf, 0xc8, 0x6d);
	mouse_show(0);
	read_record(s->pic_buf, 0x16);
	Bmp b2(0x18, 0x18, s->pic_buf);
	font_set_colors(g_font, 0xff, 0);
	g_refresh = 1;
	while (done == 0) {
		if (redraw != 0 || g_refresh != 0) {
			mouse_hide(0);
			if (g_refresh != 0) {
				slots = group_count_free_knapsack(g_group);
				cap = group_free_capacity(g_group);
				if (s->count > 15) {
					cols = 4;
					draw_loc_icons(g_screen, 4, 0x17, 0x1a, 0x1b, 8);
				} else {
					cols = 2;
					draw_loc_icons(g_screen, 2, 0x17, 8);
				}
				screen_redraw(g_screen);
				snd_play(g_snd_music, 0x18);
				font_set_colors(g_font, 0xff, -1);
				g_refresh = 0;
			}
			tmp = new Bmp(0x97, 0x1f);
			tmp->put(g_vga_buf, 0x2f, 2);
			delete tmp;
			tmp = new Bmp(0xcd, 0x68);
			tmp->put(g_vga_buf, 0x12, 0x21);
			delete tmp;
			b1.put(g_vga_buf, 0xc8, 0x6d);
			do_v_line(g_vga_buf, 0x57, 0x23, 0x60, 0xff);
			do_v_line(g_vga_buf, 0x98, 0x23, 0x60, 0xff);
			b2.put(g_vga_buf, 0x30, 5);
			peddler_buy_draw_money(s, total);
			peddler_buy_draw_weight(s, cap, nweight);
			peddler_buy_draw_slots(s, slots, nslots);
			peddler_buy_draw_grid(s);
			mouse_show(0);
			s->old_sel = -1;
			if (keep == 0)
				s->sel = 0;
			else
				keep = 0;
			redraw = 0;
		} else if (redraw2 != 0) {
			mouse_hide(0);
			Bmp b3(0x7c, 0x17);
			b3.put(g_vga_buf, 0x4b, 6);
			peddler_buy_draw_money(s, total);
			peddler_buy_draw_weight(s, cap, nweight);
			peddler_buy_draw_slots(s, slots, nslots);
			peddler_buy_draw_item(s, s->sel + s->base);
			mouse_show(0);
			redraw2 = 0;
		}
		g_mouse_areas2 = g_action_table_merchant;
		handle_input(g_screen);
		g_mouse_areas2 = 0;
		keep = s->grid_input(&redraw);
		if (g_refresh == 0 && redraw == 0)
			peddler_grid_mark(s, 1);
		if (g_mouse_rclick != 0 && get_click_action(g_action_table_merchant) != 0) {
			g_key_ascii = 8;
			g_action = 0;
			g_mouse_rclick = 0;
		} else if (g_action == 0x90) {
			g_key = 0x1c0d;
			g_action = 0;
		} else if (g_mouse_rclick != 0 || g_action == 0x4900) {
			if (cols == 4)
				ans = dlg_choose(g_dialog, 0, 4, s->txt[6], s->txt[7], s->txt[8], s->txt[9]) - 1;
			else
				ans = dlg_choose(g_dialog, 0, 2, s->txt[6], s->txt[9]) - 1;
			if (ans != -2)
				g_action = ans + 0x81;
		}
		if (g_key_ascii == 0x2b || g_key == 0x1c0d) {
			if (s->sold + nslots >= g_peddler_stock) {
				dlg_message(g_dialog, s->txt[60]);
			} else {
				id = s->wares[s->sel + s->base].id;
				cnt = s->buy_qty[id];
				if (cnt < 99) {
					if (s->sel == s->old_sel)
						redraw2 = 1;
					else
						redraw = 1;
					if (g_items[id].stackable) {
						sprintf(g_text_output_buf, s->txt[14], grammar_article(4, id, 0));
						qty = dlg_input(g_dialog, 2, g_text_output_buf, 0, 99 - cnt, 99 - cnt);
					} else
						qty = 1;
					if (qty > 0) {
						price = (signed long)s->wares[s->sel + s->base].price * s->wares[s->sel + s->base].unit * qty;
						if (total + price > g_group->money) {
							dlg_message(g_dialog, s->txt[15]);
						} else {
							total += price;
							s->buy_qty[id] += qty;
						}
						peddler_buy_sum(s, &nslots, &nweight);
#if FIX_0128
						/* Fix #128: Warnung mit den neu berechneten belegten Plaetzen nslots */
						if (nslots > slots || nweight >= cap)
							dlg_message(g_dialog, s->txt[16]);
#else
						if (cnt >= slots || nweight >= cap)
							dlg_message(g_dialog, s->txt[16]);
#endif
					}
				}
			}
		} else if (g_key_ascii == 0x2d || g_key_ascii == 8) {
			if (s->sel == s->old_sel)
				redraw2 = 1;
			else
				redraw = 1;
			id = s->wares[s->sel + s->base].id;
			cnt = s->buy_qty[id];
			if (cnt != 0) {
				if (g_items[id].stackable) {
#if FIX_0128
					/* Fix #128: Menge 1 auch bei stapelbarer Ware setzen */
					if (cnt > 1) {
						sprintf(g_text_output_buf, s->txt[13], grammar_article(4, id, 0));
						qty = dlg_input(g_dialog, 2, g_text_output_buf, 0, 99, cnt);
					} else
						qty = 1;
#else
					if (cnt > 1) {
						sprintf(g_text_output_buf, s->txt[13], grammar_article(4, id, 0));
						qty = dlg_input(g_dialog, 2, g_text_output_buf, 0, 99, cnt);
					}
#endif
				} else
					qty = 1;
				if (qty > 0) {
					if (qty > cnt)
						qty = cnt;
					price = (signed long)s->wares[s->sel + s->base].price * s->wares[s->sel + s->base].unit * qty;
					total -= price;
					s->buy_qty[id] -= qty;
				}
				peddler_buy_sum(s, &nslots, &nweight);
			}
		} else if (g_action == 0x81) {
			if (total != 0) {
				s->sold += nslots;
				if (s->sold >= g_peddler_stock)
					s->sold_out = 1;
				s->done = done = s->trade(total);
				if (s->sold_out != 0)
					done = 1;
				memset(s->buy_qty, 0, g_nr_items * sizeof(signed short));
				redraw = 1;
				slots = group_count_free_knapsack(g_group);
				cap = group_free_capacity(g_group);
				total = nslots = nweight = 0;
			} else
				dlg_message(g_dialog, s->txt[21]);
		} else if (g_action == 0x82 && s->wares[s->base + 15].id != 0 && cols == 4) {
			redraw = 1;
			s->base += 15;
		} else if (g_action == 0x83 && cols == 4) {
			if (s->base != 0) {
				s->base -= 15;
				redraw = 1;
			} else {
				s->sel -= 10;
				if (s->sel < 0)
					s->sel = 0;
			}
		} else if (cols + 0x80 == g_action)
			done = 1;
	}
	mem_free(s->buy_qty);
	font_restore_colors(g_font);
	g_refresh = 1;
	g_portrait_drag = 1;
	g_pp20_index = -1;
}

void peddler_buy_draw_grid(Peddler far *s)
{
	signed short col;
	register signed short row;
	register signed short idx;

	for (col = 0; col < 3; col++) {
		for (row = 0; row < 5; row++) {
			idx = col * 5 + row + s->base;
			if (s->wares[idx].id != 0) {
				peddler_buy_draw_item(s, idx);
				sprintf(g_text_output_buf, s->wares[idx].unit == 1 ? s->txt[10] :
					s->wares[idx].unit == 10 ? s->txt[11] : s->txt[12], s->wares[idx].price);
				font_print(g_font, g_text_output_buf, s->grid_x[col] + 20, s->grid_y[row]);
				sprintf(g_text_output_buf, s->txt[56], g_items[s->wares[idx].id].weight);
				font_print(g_font, g_text_output_buf, s->grid_x[col] + 20, s->grid_y[row] + 9);
			}
		}
	}
}

void peddler_buy_draw_item(Peddler far *s, register signed short idx)
{
	Bmp b(0x10, 0x10);
	signed short dummy;
	signed short row;
	signed short col;

	row = (idx - s->base) % 5;
	col = (idx - s->base) / 5;
	NVF_HEADS->load(0x0b, b.data(), 0, g_items[s->wares[idx].id].gfx, 0, &dummy, &dummy, 0x140);
	b.put(g_vga_buf, s->grid_x[col], s->grid_y[row]);
	if (s->buy_qty[s->wares[idx].id] != 0) {
		itoa(s->buy_qty[s->wares[idx].id], g_text_output_buf, 10);
		font_set_colors(g_font, 0xfe, -1);
		font_print(g_font, g_text_output_buf, s->grid_x[col] + 17 - font_text_width(g_font, g_text_output_buf),
			s->grid_y[row]);
		font_restore_colors(g_font);
		font_set_colors(g_font, 0xf9, -1);
		font_print(g_font, g_text_output_buf, s->grid_x[col] + 16 - font_text_width(g_font, g_text_output_buf),
			s->grid_y[row]);
		font_restore_colors(g_font);
	}
}

void peddler_buy_sum(Peddler far *s, signed short far *slots, signed short far *weight)
{
	signed short i;

	*slots = *weight = 0;
	for (i = 0; i < g_nr_items; i++) {
		if (s->buy_qty[i] != 0) {
			if (g_items[i].stackable)
				*slots += s->buy_qty[i] / 100 + 1;
			else
				*slots += s->buy_qty[i];
			*weight += g_items[i].weight * s->buy_qty[i];
		}
	}
}

void peddler_buy_draw_money(Peddler far *s, signed long sum)
{
	if (sum >= 100000L)
		peddler_format_money(s, g_text_output_buf, sum);
	else
		money_to_string(g_text_output_buf, sum);
	font_print(g_font, g_text_output_buf, 0x4b, 5);
	font_set_colors(g_font, 0xf9, -1);
	if (g_group->money >= 100000L)
		peddler_format_money(s, g_text_output_buf, g_group->money);
	else
		money_to_string(g_text_output_buf, g_group->money);
	font_print(g_font, g_text_output_buf, 200 - font_text_width(g_font, g_text_output_buf), 5);
	font_restore_colors(g_font);
}

void peddler_buy_draw_weight(Peddler far *s, signed short cap, signed short weight)
{
	signed short w;

	font_print(g_font, s->txt[58], 0x4b, 0x0d);
	sprintf(g_text_output_buf, s->txt[59], cap);
	w = font_text_width(g_font, g_text_output_buf);
	font_set_colors(g_font, 0xf9, -1);
	font_print(g_font, g_text_output_buf, 200 - w, 0x0d);
	font_restore_colors(g_font);
	sprintf(g_text_output_buf, s->txt[59], weight);
	strcat(g_text_output_buf, "^/^");
	w += font_text_width(g_font, g_text_output_buf);
	font_print(g_font, g_text_output_buf, 200 - w, 0x0d);
}

void peddler_buy_draw_slots(Peddler far *s, signed short nfree, signed short nused)
{
	signed short w;

	font_print(g_font, s->txt[57], 0x4b, 0x15);
	sprintf(g_text_output_buf, "/^%d", nfree);
	w = font_text_width(g_font, g_text_output_buf);
	font_set_colors(g_font, 0xf9, -1);
	font_print(g_font, g_text_output_buf, 200 - w, 0x15);
	font_restore_colors(g_font);
	sprintf(g_text_output_buf, "%d^", nused);
	w += font_text_width(g_font, g_text_output_buf);
	font_print(g_font, g_text_output_buf, 200 - w, 0x15);
}

void peddler_format_money(Peddler far *s, char far *buf, signed long m)
{
	sprintf(buf, s->txt[61], m / 100, m % 100);
}

void peddler_grid_mark(Peddler far *s, signed short show_usable)
{
	register signed short old;

	if ((old = s->old_sel) != s->sel) {
		void far *name;
		signed short usable;
		signed short x1;
		signed short y1;
		signed short x0;
		signed short y0;
		register signed short col;

		col = 0xff;
		usable = 0;
		x1 = s->grid_x[s->sel / 5];
		y1 = s->grid_y[s->sel % 5];
		x0 = s->grid_x[old / 5];
		y0 = s->grid_y[old % 5];
		mouse_hide(0);
		if (s->old_sel != -1)
			do_border(g_vga_buf, x0 - 1, y0 - 1, 0x12, 0x12, 0);
		do_border(g_vga_buf, x1 - 1, y1 - 1, 0x12, 0x12, 0xff);
		screen_clear_textbar(g_screen);
		if (show_usable != 0) {
			usable = group_mark_can_use(g_group, s->wares[s->sel + s->base].id);
			if (usable != 0)
				col = 0xff;
			else
				col = 0xcd;
		}
		if ((name = g_item_names[s->wares[s->sel + s->base].id]) != 0)
			TEXTBAR_COL(grammar_name((char far *)name), col);
		s->old_sel = s->sel;
		mouse_show(0);
	}
}
