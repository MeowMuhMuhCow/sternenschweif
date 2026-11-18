/* OVL228 - Overlay O228 (Segment 0x23f6 in SCHWEIF.EXE): Laden - Kaufen (class Shop aus O226, hier freie Funktionen
 * mit Shop far *s; Aufruf Shop::run, Menuepunkt 0x82). BE: buy_screen.
 * STUFE D1 (wovd1, 19.10.2026): asm-strcat durch echtes strcat(.., "..") ersetzt - Code GLEICH, das Literal
 *   im eigenen _DATA bildet wovd1/cobjfix_d.py auf ds:72d0 ab (Bytes geprueft).
 * shop_buy_screen(s): Warenliste (s->wares, shop_build_wares), Mengen s->buy_qty (mem_alloc(g_nr_items * 2)),
 * Hauptschleife: Neuzeichnen (Rahmen, Geld shop_buy_draw_money, Last shop_buy_draw_weight, Plaetze shop_buy_draw_slots,
 * Raster shop_buy_draw_grid), Auswahl shop_grid_input/shop_grid_mark, Menue dlg_choose (txt[6..9]); '+'/Enter: Ware
 * dazu (stapelbar -> Menge dlg_input bis 99), Preis * Einheit * Menge, Geldpruefung (txt[15]), Warnung txt[16];
 * '-'/Backspace: Ware zurueck; 0x81: bezahlen shop_trade(s, summe, 1) (-> s->done), 0x82/0x83 Seite vor/zurueck
 * (15 Waren), Ende. shop_buy_draw_grid: 3x5 Raster mit Preis (txt[10..12] je Einheit 1/10/sonst) und Gewicht
 * (txt[56]). shop_buy_draw_item(s, idx): Bild (NVF 0x0b) und Menge (zweifarbig). shop_buy_sum(s, &plaetze, &last):
 * Summen ueber s->buy_qty (stapelbar: Menge/100 + 1). shop_buy_draw_money(s, summe): Summe und Gruppengeld (ab 100000
 * shop_format_money, sonst money_to_string). shop_buy_draw_weight/shop_buy_draw_slots: Last/Plaetze (frei, gekauft).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL228.CPP   -> 4438/4438 Bytes GLEICH (18.10.2026), cobjfix laeuft.
 *   Ohne -Og 37 Bytes zu lang; -Oi fuer memset (rep stosw) und das strcat in shop_buy_draw_weight.
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - shop_buy_screen im ersten Entwurf GLEICH: -Og-Zwischenwerte [bp-3ah] (s->sel), [bp-3ch] (Index * 7), [bp-3eh]
 *   (99 - cnt im ?:), r (shop_trade) im eigenen Block [bp-40h] teilt sich den Platz mit Bmp b3 [bp-48h].
 * - Bmp-Locals b1/b2 (10 Bytes) bleiben in Deklarationsreihenfolge, Konstruktor an der Deklarationsstelle.
 * - shop_buy_draw_item: ohne register landet die ZUERST deklarierte Variable in DI -> row vor Bmp/w/col/rel
 *   deklariert; "row = (rel = idx - s->base) % 5" eingebettet (sonst mov ax,[bp-10h] nachgeladen).
 * - shop_buy_draw_weight: strcat(g_text_output_buf, "^/^") mit konstanter Laenge 4 (Literal im eigenen
 *   Datensegment, ds:72d0).
 * - shop_buy_sum: i in CX (keine Aufrufe), &g_items[i] als -Og-Zwischenwert in DI (Segment aus ds:b84c nachgeladen).
 * - "total = nslots = nweight = 0" -> xor ax / zwei Wort-Stores / movsx eax.
 * Originalfehler: shop_buy_screen (Kaufen) warnt mit "cnt >= slots" - cnt ist die Menge dieser Ware VOR dem Kauf statt
 *   der eben berechneten Platzzahl nslots (die shop_buy_draw_slots anzeigt). Beim Zurueckgeben einer stapelbaren Ware
 *   mit Menge 1 bleibt n (SI) uninitialisiert (Restwert). Geld wird vorzeichenbehaftet verglichen (O200 rechnet
 *   unsigned).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:72cc..72de, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
SHOP_PTR g_shop = 0;

#define NVF_HEADS ((Nvf far *)g_nvf_heads)

void shop_buy_screen(Shop far *s)
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
	register signed short qty;

	memset(s->wares, 0, g_nr_items * sizeof(struct ware));
	s->buy_qty = (signed short far *)mem_alloc(g_nr_items * sizeof(signed short));
	shop_build_wares(s);
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
			shop_buy_draw_money(s, total);
			shop_buy_draw_weight(s, cap, nweight);
			shop_buy_draw_slots(s, slots, nslots);
			shop_buy_draw_grid(s);
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
			shop_buy_draw_money(s, total);
			shop_buy_draw_weight(s, cap, nweight);
			shop_buy_draw_slots(s, slots, nslots);
			shop_buy_draw_item(s, s->sel + s->base);
			mouse_show(0);
			redraw2 = 0;
		}

		g_mouse_areas2 = g_action_table_merchant;
		handle_input(g_screen);
		g_mouse_areas2 = 0;
		keep = shop_grid_input(s, &redraw);
		if (g_refresh == 0 && redraw == 0)
			shop_grid_mark(s, 1);

		if (g_mouse_rclick != 0 && get_click_action(g_action_table_merchant)) {
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
			id = s->wares[s->sel + s->base].id;
			cnt = s->buy_qty[id];
			if (cnt < 99) {
				if (s->sel == s->old_sel)
					redraw2 = 1;
				else
					redraw = 1;
				if (g_items[id].stackable) {
					sprintf(g_text_output_buf, s->txt[14], grammar_article(4, id, 0));
					qty = dlg_input(g_dialog, 2, g_text_output_buf, 0, 99 - cnt, 99 - cnt > 10 ? 10 : 99 - cnt);
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
					shop_buy_sum(s, &nslots, &nweight);
#if FIX_0094
					/* Fix #94: Warnung nach der neu berechneten Platzzahl */
					if (nslots > slots || nweight >= cap)
#else
					if (cnt >= slots || nweight >= cap)
#endif
						dlg_message(g_dialog, s->txt[16]);
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
#if FIX_0094
					/* Fix #94: Menge vorbelegen (stapelbare Ware mit Menge 1) */
					qty = 1;
					if (cnt > 1) {
#else
					if (cnt > 1) {
#endif
						sprintf(g_text_output_buf, s->txt[13], grammar_article(4, id, 0));
						qty = dlg_input(g_dialog, 2, g_text_output_buf, 0, cnt, cnt);
					}
				} else
					qty = 1;
				if (qty > 0) {
					if (qty > cnt)
						qty = cnt;
					price = (signed long)s->wares[s->sel + s->base].price * s->wares[s->sel + s->base].unit * qty;
					total -= price;
					s->buy_qty[id] -= qty;
				}
				shop_buy_sum(s, &nslots, &nweight);
			}
		} else if (g_action == 0x81) {
			if (total != 0) {
				signed short r;
				r = shop_trade(s, total, 1);
				if (r != -1) {
					s->done = done = r;
					memset(s->buy_qty, 0, g_nr_items * sizeof(signed short));
					redraw = 1;
					slots = group_count_free_knapsack(g_group);
					cap = group_free_capacity(g_group);
					total = nslots = nweight = 0;
				}
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
		} else if (cols + 0x80 == g_action || g_key == 0x11b) {
			done = 1;
		}
	}

	mem_free(s->buy_qty);
	font_restore_colors(g_font);
	g_refresh = 1;
	g_portrait_drag = 1;
	g_pp20_index = -1;
}

void shop_buy_draw_grid(Shop far *s)
{
	signed short col;
	register signed short row;
	register signed short idx;

	for (col = 0; col < 3; col++) {
		for (row = 0; row < 5; row++) {
			idx = col * 5 + row + s->base;
			if (s->wares[idx].id != 0) {
				shop_buy_draw_item(s, idx);
				sprintf(g_text_output_buf, s->wares[idx].unit == 1 ? s->txt[10] :
					s->wares[idx].unit == 10 ? s->txt[11] : s->txt[12], s->wares[idx].price);
				font_print(g_font, g_text_output_buf, s->grid_x[col] + 20, s->grid_y[row]);
				sprintf(g_text_output_buf, s->txt[56], g_items[s->wares[idx].id].weight);
				font_print(g_font, g_text_output_buf, s->grid_x[col] + 20, s->grid_y[row] + 9);
			}
		}
	}
}

void shop_buy_draw_item(Shop far *s, register signed short idx)
{
	signed short row;
	Bmp b(0x10, 0x10);
	signed short w;
	signed short col;
	signed short rel;

	row = (rel = idx - s->base) % 5;
	col = rel / 5;
	NVF_HEADS->load(0xb, b.data(), 0, g_items[s->wares[idx].id].gfx, 0, &w, &w, 0x140);
	b.put(g_vga_buf, s->grid_x[col], s->grid_y[row]);
	if (s->buy_qty[s->wares[idx].id] != 0) {
		itoa(s->buy_qty[s->wares[idx].id], g_text_output_buf, 10);
		font_set_colors(g_font, 0xfe, -1);
		font_print(g_font, g_text_output_buf, s->grid_x[col] + 17 - font_text_width(g_font, g_text_output_buf), s->grid_y[row]);
		font_restore_colors(g_font);
		font_set_colors(g_font, 0xf9, -1);
		font_print(g_font, g_text_output_buf, s->grid_x[col] + 16 - font_text_width(g_font, g_text_output_buf), s->grid_y[row]);
		font_restore_colors(g_font);
	}
}

void shop_buy_sum(Shop far *s, signed short far *slots, signed short far *weight)
{
	register signed short i;

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

void shop_buy_draw_money(Shop far *s, signed long sum)
{
	if (sum >= 100000L)
		shop_format_money(s, g_text_output_buf, sum);
	else
		money_to_string(g_text_output_buf, sum);
	font_print(g_font, g_text_output_buf, 0x4b, 5);
	font_set_colors(g_font, 0xf9, -1);
	if (g_group->money >= 100000L)
		shop_format_money(s, g_text_output_buf, g_group->money);
	else
		money_to_string(g_text_output_buf, g_group->money);
	font_print(g_font, g_text_output_buf, 200 - font_text_width(g_font, g_text_output_buf), 5);
	font_restore_colors(g_font);
}

void shop_buy_draw_weight(Shop far *s, signed short cap, signed short weight)
{
	signed short w;

	font_print(g_font, s->txt[58], 0x4b, 0xd);
	sprintf(g_text_output_buf, s->txt[59], cap);
	w = font_text_width(g_font, g_text_output_buf);
	font_set_colors(g_font, 0xf9, -1);
	font_print(g_font, g_text_output_buf, 200 - w, 0xd);
	font_restore_colors(g_font);
	sprintf(g_text_output_buf, s->txt[59], weight);
	strcat(g_text_output_buf, "^/^");
	w += font_text_width(g_font, g_text_output_buf);
	font_print(g_font, g_text_output_buf, 200 - w, 0xd);
}

void shop_buy_draw_slots(Shop far *s, signed short nfree, signed short nused)
{
	register signed short w;

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
