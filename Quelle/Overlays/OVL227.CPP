/* OVL227 - Overlay O227 (Segment 0x23f2 in SCHWEIF.EXE): Laden - Verkaufen (class Shop aus O226, hier freie Funktionen
 * mit Shop far *s; Aufruf Shop::run, Menuepunkt 0x83). Gegenstueck zu O228 (Kaufen). BE: sell_screen.
 * shop_sell_screen(s): Warenliste leeren, Verkaufsmengen s->sell_qty = mem_alloc(7 * 31 * 2) (je Held [s->hero 0..6]
 * und Inventarplatz), Hauptschleife: Held waehlen (get_active_hero mit txt[23], NSC group_get_npc -> Index 6, sonst
 * group_hero_pos; Abbruch ohne vorherigen Helden -> Ende), Liste aus dem Heldengepaeck (shop_sell_fill), Leiste
 * draw_loc_icons mit 5 bzw. 3 Punkten (> 15 Waren), Neuzeichnen (Rahmen, Gruppengeld, Erloes, Raster
 * shop_sell_draw_grid), Teilneuzeichnen (Bmp b3, shop_sell_draw_item), Menue dlg_choose (txt[6..9], txt[24]);
 * 0x90/0x1c0d Ware an-/abwaehlen (stapelbar mit Menge > 1 -> dlg_input, Text txt[25] mit Warenname und Heldenname),
 * 0xf1..0xf7 Held direkt waehlen (group_has_hero prueft), 0x81 verkaufen shop_trade(s, erloes, 2) -> s->done,
 * 0x82/0x83 blaettern (15), 0x84 bzw. 0x82 (3 Punkte) Held wechseln, Ende.
 * shop_sell_draw_grid(s, h): 3x5 Raster mit Preis (txt[10..12] je Einheit). shop_sell_draw_item(s, idx, h): Bild
 * (NVF 0x0b), Stueckzahl im Gepaeck (stapelbar, > 1) und Verkaufsmenge. shop_sell_fill(s, h): Warenliste aus den 31
 * Inventarplaetzen nach Ladenart (Waffen/Ruestungen -> SHOP_WEAPONS, Kraeuter -> SHOP_HERBS, sonst SHOP_GENERAL;
 * undropable unverkaeuflich), Preis = halber Ladenpreis (mind. 1), beschaedigt (broken bzw. rs_lost) -> Preis 1.
 * shop_mark_hero(s): Heldenmarkierung screen_draw_name_normal/screen_draw_name.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL227.CPP   -> 4998/4998 Bytes GLEICH (18.10.2026), cobjfix laeuft.
 *   Ohne -Oi kein memset-Intrinsic (12 Bytes kuerzer, alles verschoben), ohne -Og 202 Bytes zu lang.
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Erster Entwurf nach dem Muster von OVL228 bis auf eine Stelle GLEICH: shop_sell_fill "h->inv[i].broken ||
 *   !!h->inv[i].rs_lost" (signed char in ||-Kette: nur mit !! mov al / cbw / or ax,ax, sonst cmp byte).
 * - -Og-Zwischenwerte in shop_sell_screen [bp-40h] (int)w.slot, [bp-42h] slot * 2, [bp-44h] s->hero, [bp-46h]
 *   hero * 62, [bp-48h] Offset &h->inv[slot]; entstehen alle von selbst aus s->sell_qty[s->hero][w.slot] (Typ short
 *   (far *)[31]).
 * - "(signed long)w.price * w.unit * ((signed long)n - s->sell_qty[..][..])" -> Subtraktion 32-bittig (movsx /
 *   sub edx,eax).
 * - "memset(s->sell_qty, 0, 7 * 31 * 2)" mit -Oi -> nur mov cx,0d9h / rep stosw; mem_alloc(0x1b2L) -> push dword.
 * - Ketten "s->sel = s->base = 0", "redraw = refill = 1", "s->done = done = r" (rechts zuerst gespeichert).
 * Originalfehler/Auffaelliges: shop_sell_draw_item zeichnet die Gepaeck-Stueckzahl zweimal (Schatten 0xfe bei x+17,
 *   dann x+16), setzt aber vor dem zweiten Mal die Farbe 0xf9 nicht und ruft danach kein font_restore_colors (anders
 *   als beim zweiten Zahlenblock). shop_sell_screen: wird die erste Heldenwahl abgebrochen (done = 1), laeuft die
 *   Eingabe (handle_input, shop_grid_input, Menue) trotzdem noch einmal durch, bevor die Schleife endet.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "schweif.h"

#define NVF_HEADS ((Nvf far *)g_nvf_heads)

void shop_sell_screen(Shop far *s)
{
	signed short qty;
	signed short ans;
	struct ware ware;
	Bmp far *tmp;
	struct hero far *old_h;
	struct hero far *h = 0;
	signed short cols = 0;
	signed long total = 0;
	signed short refill = 1;
	signed short choose = 1;
	signed short redraw = 1;
	signed short redraw2 = 0;
	signed short keep = 0;
	signed short done = 0;

	memset(s->wares, 0, g_nr_items * sizeof(struct ware));
	s->sell_qty = (signed short (far *)[31])mem_alloc(0x1b2L);
	g_portrait_drag = 0;
	s->old_sel = -1;
	s->old_hero = -1;
	s->sel = 0;
	s->base = 0;
	s->hero = 0;
	s->mx = g_mouse_posx;
	s->my = g_mouse_posy;
	g_pp20_index = -1;
	screen_redraw(g_screen);
	mouse_hide(0);
	Bmp b1(0x17, 0x1c);
	b1.get(g_vga_buf, 0xc8, 0x6d);
	mouse_show(0);
	read_record(s->pic_buf, 0x18);
	Bmp b2(0x18, 0x18, s->pic_buf);
	font_set_colors(g_font, 0xff, 0);
	g_refresh = 1;

	while (done == 0) {
		if (redraw != 0 || choose != 0 || refill != 0 || g_refresh != 0) {
			if (g_refresh != 0) {
				screen_redraw(g_screen);
				snd_play(g_snd_music, 0x18);
				font_set_colors(g_font, 0xff, 0);
				shop_mark_hero(s);
				refill = 1;
			}
			if (choose != 0) {
				old_h = h;
				h = get_active_hero(g_group, s->txt[23]);
				if (h != 0) {
					if (group_get_npc(g_group) == h)
						s->hero = 6;
					else
						s->hero = group_hero_pos(g_group, h);
					shop_mark_hero(s);
					s->old_sel = -1;
					s->sel = 0;
					shop_grid_mark(s, 0);
					refill = 1;
				} else if (old_h != 0) {
					h = old_h;
					s->hero = s->old_hero;
				} else
					done = 1;
				choose = 0;
			}
			if (done == 0) {
				mouse_hide(0);
				if (refill != 0) {
					signed short oc = cols;
					shop_sell_fill(s, h);
					refill = 0;
					s->old_sel = -1;
					s->sel = s->base = 0;
					if (s->count > 15 && oc != 5 || s->count <= 15 && oc != 3)
						g_refresh = 1;
				}
				if (g_refresh != 0) {
					if (s->count > 15) {
						cols = 5;
						draw_loc_icons(g_screen, 5, 0x17, 0x1a, 0x1b, 0x1c, 8);
					} else {
						cols = 3;
						draw_loc_icons(g_screen, 3, 0x17, 0x1c, 8);
					}
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
				b2.put(g_vga_buf, 0x6c, 5);
				money_to_string(g_text_output_buf, g_group->money);
				font_print(g_font, g_text_output_buf, 0x68 - font_text_width(g_font, g_text_output_buf), 0xd);
				money_to_string(g_text_output_buf, total);
				font_set_colors(g_font, 0xf9, 0);
				font_print(g_font, g_text_output_buf, 0xca - font_text_width(g_font, g_text_output_buf), 0xd);
				font_restore_colors(g_font);
				shop_sell_draw_grid(s, h);
				mouse_show(0);
				s->old_sel = -1;
				if (keep == 0)
					s->sel = 0;
				else
					keep = 0;
				redraw = 0;
			}
		} else if (redraw2 != 0) {
			mouse_hide(0);
			Bmp b3(0x46, 0x17);
			b3.put(g_vga_buf, 0x84, 6);
			money_to_string(g_text_output_buf, total);
			font_set_colors(g_font, 0xf9, 0);
			font_print(g_font, g_text_output_buf, 0xca - font_text_width(g_font, g_text_output_buf), 0xd);
			font_restore_colors(g_font);
			shop_sell_draw_item(s, s->sel + s->base, h);
			mouse_show(0);
			redraw2 = 0;
		}

		g_mouse_areas2 = g_action_table_merchant;
		handle_input(g_screen);
		g_mouse_areas2 = 0;
		keep = shop_grid_input(s, &redraw);
		if (redraw == 0 && g_refresh == 0)
			shop_grid_mark(s, 0);

		if (g_mouse_rclick != 0 || g_action == 0x4900) {
			if (s->count > 15)
				ans = dlg_choose(g_dialog, 0, 5, s->txt[6], s->txt[7], s->txt[8], s->txt[24], s->txt[9]) - 1;
			else
				ans = dlg_choose(g_dialog, 0, 3, s->txt[6], s->txt[24], s->txt[9]) - 1;
			if (ans != -2)
				g_action = ans + 0x81;
		}

		if (g_action == 0x90 || g_action == 0x1c0d) {
			ware = s->wares[s->sel + s->base];
			if (ware.id != 0) {
				redraw2 = 1;
				qty = 1;
				if (s->sell_qty[s->hero][ware.slot] != 0) {
					if (g_items[ware.id].stackable && h->inv[ware.slot].quantity > 1) {
						sprintf(g_text_output_buf, s->txt[25], grammar_article(4, ware.id, 0), h->name);
						qty = dlg_input(g_dialog, 2, g_text_output_buf, 0, h->inv[ware.slot].quantity, h->inv[ware.slot].quantity);
						if (qty != -1) {
							total += (signed long)ware.price * ware.unit * ((signed long)qty - s->sell_qty[s->hero][ware.slot]);
							s->sell_qty[s->hero][ware.slot] = qty;
						}
					} else {
						s->sell_qty[s->hero][ware.slot] = 0;
						total -= (signed long)ware.price * ware.unit * qty;
					}
				} else {
					if (g_items[ware.id].stackable && h->inv[ware.slot].quantity > 1) {
						sprintf(g_text_output_buf, s->txt[25], grammar_article(4, ware.id, 0), h->name);
						qty = dlg_input(g_dialog, 2, g_text_output_buf, 0, h->inv[ware.slot].quantity, h->inv[ware.slot].quantity);
					} else
						qty = 1;
					if (qty != -1) {
						total += (signed long)ware.price * ware.unit * ((signed long)qty - s->sell_qty[s->hero][ware.slot]);
						s->sell_qty[s->hero][ware.slot] = qty;
					}
				}
			}
		} else if (g_action >= 0xf1 && g_action <= 0xf7) {
			signed short cnt = g_group->nr_heroes;
			old_h = h;
			h = 0;
			s->hero = g_action - 0xf1;
			if (group_get_npc(g_group) != 0)
				cnt--;
			if (s->hero == 6)
				h = group_get_npc(g_group);
			else if (s->hero < cnt)
				h = g_group->heroes[s->hero];
			if (group_has_hero(g_group, h) == 1) {
				shop_mark_hero(s);
				refill = 1;
				redraw = 1;
			} else {
				h = old_h;
				s->hero = s->old_hero;
			}
		} else if (g_action == 0x81) {
			if (total != 0) {
				signed short r;
				r = shop_trade(s, total, 2);
				if (r != -1) {
					s->done = done = r;
					redraw = refill = 1;
					total = 0;
					memset(s->sell_qty, 0, 7 * 31 * sizeof(signed short));
				}
			} else
				dlg_message(g_dialog, s->txt[21]);
		} else if (g_action == 0x82 && s->wares[s->base + 15].id != 0 && cols == 5) {
			redraw = 1;
			s->base += 15;
		} else if (g_action == 0x83 && cols == 5) {
			if (s->base != 0) {
				s->base -= 15;
				redraw = 1;
			} else {
				s->sel -= 10;
				if (s->sel < 0)
					s->sel = 0;
			}
		} else if (g_action == 0x84 && cols == 5 || g_action == 0x82 && cols == 3) {
			choose = 1;
		} else if (cols + 0x80 == g_action || g_key == 0x11b) {
			done = 1;
		}
	}

	mem_free(s->sell_qty);
	font_restore_colors(g_font);
	g_refresh = 1;
	g_portrait_drag = 1;
	g_pp20_index = -1;
}

void shop_sell_draw_grid(Shop far *s, struct hero far *h)
{
	signed short col;
	register signed short row;
	register signed short idx;

	for (col = 0; col < 3; col++) {
		for (row = 0; row < 5; row++) {
			idx = col * 5 + row + s->base;
			if (s->wares[idx].id != 0) {
				shop_sell_draw_item(s, idx, h);
				sprintf(g_text_output_buf, s->wares[idx].unit == 1 ? s->txt[10] :
					s->wares[idx].unit == 10 ? s->txt[11] : s->txt[12], s->wares[idx].price);
				font_print(g_font, g_text_output_buf, s->grid_x[col] + 20, s->grid_y[row] + 5);
			}
		}
	}
}

void shop_sell_draw_item(Shop far *s, signed short idx, struct hero far *h)
{
	signed short row;
	Bmp b(0x10, 0x10);
	signed short w;
	signed short n;
	signed short col;
	signed short rel;

	row = (rel = idx - s->base) % 5;
	col = rel / 5;
	NVF_HEADS->load(0xb, b.data(), 0, g_items[s->wares[idx].id].gfx, 0, &w, &w, 0x140);
	b.put(g_vga_buf, s->grid_x[col], s->grid_y[row]);
	if (g_items[s->wares[idx].id].stackable && (n = h->inv[s->wares[idx].slot].quantity) > 1) {
		itoa(n, g_text_output_buf, 10);
		font_set_colors(g_font, 0xfe, -1);
		font_print(g_font, g_text_output_buf, s->grid_x[col] + 17 - font_text_width(g_font, g_text_output_buf), s->grid_y[row] + 9);
		font_restore_colors(g_font);
		font_print(g_font, g_text_output_buf, s->grid_x[col] + 16 - font_text_width(g_font, g_text_output_buf), s->grid_y[row] + 9);
	}
	if (s->sell_qty[s->hero][s->wares[idx].slot] != 0) {
		itoa(s->sell_qty[s->hero][s->wares[idx].slot], g_text_output_buf, 10);
		font_set_colors(g_font, 0xfe, -1);
		font_print(g_font, g_text_output_buf, s->grid_x[col] + 17 - font_text_width(g_font, g_text_output_buf), s->grid_y[row]);
		font_restore_colors(g_font);
		font_set_colors(g_font, 0xf9, -1);
		font_print(g_font, g_text_output_buf, s->grid_x[col] + 16 - font_text_width(g_font, g_text_output_buf), s->grid_y[row]);
		font_restore_colors(g_font);
	}
}

void shop_sell_fill(Shop far *s, struct hero far *h)
{
	signed short i;
	signed short id;
	signed short count = 0;
	signed short sellable;

	for (i = 0; i < 31; i++) {
		sellable = 0;
		id = h->inv[i].item_id;
		if (id != 0 && !g_items[id].undropable) {
			if (g_items[id].armor || g_items[id].weapon) {
				if (s->info.type == SHOP_WEAPONS)
					sellable = 1;
			} else if (g_items[id].herb_potion) {
				if (s->info.type == SHOP_HERBS)
					sellable = 1;
			} else if (s->info.type == SHOP_GENERAL)
				sellable = 1;
			if (sellable) {
				s->wares[count].id = id;
				s->wares[count].slot = i;
				if (h->inv[i].broken || !!h->inv[i].rs_lost) {
					s->wares[count].unit = 1;
					s->wares[count].price = 1;
				} else {
					s->wares[count].unit = g_items[id].price_unit;
					if (g_items[id].food)
						s->wares[count].price = g_items[id].price +
							(signed long)g_items[id].price * s->info.markup_food / 100;
					else
						s->wares[count].price = g_items[id].price +
							(signed long)g_items[id].price * s->info.markup / 100;
					s->wares[count].price >>= 1;
					if (s->wares[count].price == 0)
						s->wares[count].price = 1;
				}
				count++;
			}
		}
	}
	s->count = count;
	for (i = count; i < g_nr_items; i++)
		s->wares[i].id = 0;
}

void shop_mark_hero(Shop far *s)
{
	if (s->old_hero != -1)
		screen_draw_name_normal(g_screen, s->old_hero);
	screen_draw_name(g_screen, s->hero, 0xfb);
	s->old_hero = s->hero;
}
