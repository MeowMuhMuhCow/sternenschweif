/* OVL099 - Overlay O099 (Segment 0x213f in SCHWEIF.EXE): Lagerhaus - Gegenstaende einlagern (store_deposit, Aufruf
 * aus Store::run O012, Gegenstueck O100 store_fetch "abholen"; Objekt class Store (s_class.h): Lagerplatz s->depot
 * (Wert value, Anzahl nr_items, Zeitstempel stored), Warenliste s->wares (7 Bytes struct ware {id, price, unit,
 * slot}), Held s->hero (0..6, 6 = NSC group_get_npc) und je Held eine Auswahl s->sel_qty[7] (je 31 Bytes =
 * Inventarplaetze)).
 * store_deposit(s): Auswahl anlegen, Pruefung store_deposit_check (Gruppengeld >= 100, sonst txt[8]), value =
 *   depot->value, Hintergrund retten (Bmp 0xd0 x 0x87), Gesicht (read_record(pic_buf, 0x30), Bmp 24x24), Schleife:
 *   Heldenwahl get_active_hero(txt[9]) bzw. Tasten 0xf1..0xf7 (group_has_hero), Leiste draw_loc_icons mit 3 bzw. 5
 *   Punkten (count > 15), Anzeige Gruppengeld (money_to_string) und Betrag value / 300 (aufgerundet, kleines Bmp
 *   0x46 x 0x17 bei Aenderung), Felder store_deposit_draw_grid; Eingabe check_time/grid_input, Menue dlg_choose:
 *   0x90/0x1c0d Gegenstand an-/abwaehlen (stapelbar: Menge per dlg_input, Text txt[11]; value += price * unit *
 *   Menge; hoechstens 256 Eintraege, sonst txt[18]), 0x81 einlagern (store_deposit_commit, dann Auswahl leeren),
 *   0x82/0x83 blaettern, 0x84 (5 Punkte) bzw. 0x82 (3 Punkte) Held waehlen, 0x80 + Punkte bzw. ESC 0x11b Ende;
 *   Auswahl freigeben.
 * store_deposit_commit(s): s->new_items (0x16e3 = 217 x 27 Bytes) aus den gewaehlten Heldengegenstaenden (drop_item
 *   nimmt sie dem Helden), group_sub_money(g, 100), depot->nr_items += nr_sel, depot->value += value, Zeitstempel
 *   date_now(stored), save(0), Lagerschein ITEM_DEPOT_TICKET_1..3 (Ort g_loc_id) per group_give_item, falls nicht
 *   vorhanden; Meldung txt[19].
 * store_deposit_draw_grid(s, h): 3 x 5 Felder per store_deposit_draw_item(s, idx, h): Feld zeichnen (Bild 0x0b aus
 *   g_nvf_heads, Stueckzahl beim Helden, Auswahlzahl mit Schatten). store_deposit_fill(s, h): Warenliste aus dem
 *   Heldeninventar (ohne undropable/food/herb_potion und ohne Lagerscheine, Preis mindestens 1), Rest der 256
 *   Eintraege nullen. store_deposit_mark_hero(s): Heldenmarke in der Leiste umsetzen (screen_draw_name_normal/
 *   screen_draw_name).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y -E..\bc\BIN\TASM.EXE OVL099.CPP -> 5292/5292 Bytes GLEICH (18.10.2026), cobjfix
 *   laeuft, kein eigenes _DATA. Ohne -Oi 84 Bytes zu lang, ohne -Og falsche Laenge.
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Vorlage OVL100 (Bmp-Methoden, Menue-/Blaetterlogik).
 * - store_deposit: die register-Variable n (Gruppengroesse) nur im Block des Zweigs 0xf1..0xf7 -> SI; auf
 *   Funktionsebene landet sie auf dem Stack (memset-Intrinsic). Die drei Bmp-Objekte: back/face mitten im if-Block,
 *   das kleine im else-if-Block ([bp-4eh]).
 * - "(signed long)w.price * w.unit * ((signed long)amount - (signed long)s->sel_qty[..][..])" -> movsx eax,byte und
 *   Subtraktion in 32 Bit (ohne den zweiten Cast cbw / movsx eax,ax).
 * - store_deposit_commit: "s->new_items[n] = g_group->heroes[i]->get_inv(j)" (inline "return inv[k];") ->
 *   Zielzeiger als Local [bp-8]/[bp-6] und Parameterkopie j [bp-0ah] vor SCOPY@ cx=1bh; die direkte
 *   Strukturzuweisung ergibt push bx/es ohne diese Locals.
 * - Menue-Zweig prueft s->count > 15, nicht nr_icons.
 * - screen_textbar mit int-Farbe (push word) ueber gecasteten Funktionszeiger LOC_TEXTBAR.
 * Auffaelligkeiten (Original): store_deposit loescht mit memset(s->wares, 0, 0x100) nur 256 Bytes der 7-Byte-Tabelle
 *   (harmlos, store_deposit_fill nullt den Rest). store_deposit_draw_item zeichnet die Stueckzahl beim Helden
 *   zweimal (x + 17 / x + 16) nach font_set_colors(0xfe) ohne den Farbwechsel auf 0xf9, den die Auswahlzahl
 *   darunter hat.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "schweif.h"

/* Ortsname in die Textleiste: Farbe 0xff als int wie im Original */
#define LOC_TEXTBAR(t) ((void (*)(void far *, char far *, signed short))screen_textbar)(g_screen, t, 0xff)

void store_deposit(Store far *s)
{
	signed short i;
	signed short nr_icons;
	signed short choice;
	signed short amount;
	struct ware w;
	signed short done = 0;
	signed short keep = 0;
	signed short redraw_value = 1;
	signed short redraw = 1;
	signed short choose_hero = 1;
	signed short refill = 1;
	struct hero far *h = 0;
	struct hero far *old;

	for (i = 0; i < 7; i++)
		s->sel_qty[i] = (signed char far *)mem_alloc(0x1fL);
	if (store_deposit_check(s)) {
		g_portrait_drag = 0;
		s->old_sel = -1;
		s->sel = 0;
		s->base = 0;
		s->old_hero = 0;
		s->mx = g_mouse_posx;
		s->my = g_mouse_posy;
		s->value = s->depot->value;
		s->nr_sel = 0;
		memset(s->wares, 0, 0x100);
		g_pp20_index = -1;
		screen_redraw(g_screen);
		mouse_hide(0);
		Bmp back(0xd0, 0x87);
		back.get(g_vga_buf, 0x10, 2);
		mouse_show(0);
		read_record(s->pic_buf, 0x30);
		Bmp face(0x18, 0x18, s->pic_buf);
		font_set_colors(g_font, 0xff, 0);
		g_refresh = 1;
		while (done == 0) {
			if (redraw != 0 || choose_hero != 0 || refill != 0 || g_refresh != 0) {
				if (g_refresh != 0) {
					screen_redraw(g_screen);
					LOC_TEXTBAR(s->txt[g_loc_name]);
					snd_play(g_snd_music, 7);
					font_set_colors(g_font, 0xff, 0);
					refill = 1;
				}
				if (choose_hero != 0) {
					old = h;
					h = get_active_hero(g_group, s->txt[9]);
					if (h != 0) {
						if (group_get_npc(g_group) == h)
							s->hero = 6;
						else
							s->hero = group_hero_pos(g_group, h);
						store_deposit_mark_hero(s);
						s->old_sel = -1;
						s->sel = 0;
						s->grid_mark();
						refill = 1;
					} else if (old != 0) {
						h = old;
						s->hero = s->old_hero;
					} else {
						done = 1;
					}
					choose_hero = 0;
				}
				if (done == 0) {
					mouse_hide(0);
					if (refill != 0) {
						store_deposit_fill(s, h);
						refill = 0;
						s->old_sel = -1;
						s->sel = s->base = 0;
					}
					if (g_refresh != 0) {
						if (s->count > 15) {
							nr_icons = 5;
							draw_loc_icons(g_screen, 5, 0x17, 0x1a, 0x1b, 0x1c, 8);
						} else {
							nr_icons = 3;
							draw_loc_icons(g_screen, 3, 0x17, 0x1c, 8);
						}
						g_refresh = 0;
					}
					back.put(g_vga_buf, 0x10, 2);
					do_v_line(g_vga_buf, 0x57, 0x23, 0x60, 0xff);
					do_v_line(g_vga_buf, 0x98, 0x23, 0x60, 0xff);
					face.put(g_vga_buf, 0x6c, 5);
					money_to_string(g_text_output_buf, g_group->money);
					font_print(g_font, g_text_output_buf, 0x68 - font_text_width(g_font, g_text_output_buf), 0x0d);
					amount = s->value / 300;
					if (s->value % 300 > 0)
						amount++;
					money_to_string(g_text_output_buf, amount);
					font_set_colors(g_font, 0xf9, 0);
					font_print(g_font, g_text_output_buf, 0xca - font_text_width(g_font, g_text_output_buf), 0x0d);
					font_restore_colors(g_font);
					store_deposit_draw_grid(s, h);
					mouse_show(0);
					s->old_sel = -1;
					if (keep == 0)
						s->sel = 0;
					else
						keep = 0;
					redraw = 0;
				}
			} else if (redraw_value != 0) {
				mouse_hide(0);
				Bmp b(0x46, 0x17);
				b.put(g_vga_buf, 0x84, 6);
				amount = s->value / 300;
				if (s->value % 300 > 0)
					amount++;
				money_to_string(g_text_output_buf, amount);
				font_set_colors(g_font, 0xf9, 0);
				font_print(g_font, g_text_output_buf, 0xca - font_text_width(g_font, g_text_output_buf), 0x0d);
				font_restore_colors(g_font);
				store_deposit_draw_item(s, s->sel + s->base, h);
				mouse_show(0);
				redraw_value = 0;
			}
			if (s->check_time(g_daytime))
				done = 1;
			if (done != 0)
				continue;
			g_mouse_areas2 = (void far *)g_action_table_merchant;
			handle_input(g_screen);
			g_mouse_areas2 = 0;
			keep = s->grid_input(&redraw);
			if (redraw == 0)
				s->grid_mark();
			if (g_mouse_rclick != 0 || g_action == 0x4900) {
				if (s->count > 15)
					choice = dlg_choose(g_dialog, 0, 5, s->txt[36], s->txt[13], s->txt[14], s->txt[15], s->txt[16]) - 1;
				else
					choice = dlg_choose(g_dialog, 0, 3, s->txt[36], s->txt[15], s->txt[16]) - 1;
				if (choice != -2)
					g_action = choice + 0x81;
			}
			if (g_mouse_rclick != 0 && get_click_action(g_action_table_merchant))
				g_action = 0x90;
			if (g_action == 0x90 || g_action == 0x1c0d) {
				redraw_value = 1;
				amount = 1;
				w = s->wares[s->sel + s->base];
				if (s->sel_qty[s->hero][w.slot] != 0) {
					if (g_items[w.id].stackable && h->inv[w.slot].quantity > 1) {
						sprintf(g_text_output_buf, s->txt[11], grammar_article(4, w.id, 0), h->name);
						amount = dlg_input(g_dialog, 2, g_text_output_buf, 0, h->inv[w.slot].quantity,
							h->inv[w.slot].quantity);
						if (amount != -1) {
							s->value += (signed long)w.price * w.unit *
								((signed long)amount - (signed long)s->sel_qty[s->hero][w.slot]);
							s->sel_qty[s->hero][w.slot] = amount;
							if (amount == 0)
								s->nr_sel--;
						}
					} else {
						s->sel_qty[s->hero][w.slot] = 0;
						s->value -= (signed long)w.price * w.unit * amount;
						s->nr_sel--;
					}
				} else if (s->depot->nr_items + s->nr_sel < 0x100) {
					if (g_items[w.id].stackable && h->inv[w.slot].quantity > 1) {
						sprintf(g_text_output_buf, s->txt[11], grammar_article(4, w.id, 0), h->name);
						amount = dlg_input(g_dialog, 2, g_text_output_buf, 0, h->inv[w.slot].quantity,
							h->inv[w.slot].quantity);
						if (amount != -1) {
							s->value += (signed long)w.price * w.unit * amount;
							s->sel_qty[s->hero][w.slot] = s->sel_qty[s->hero][w.slot] + amount;
						}
					} else {
						s->sel_qty[s->hero][w.slot] = 1;
						s->value += (signed long)w.price * w.unit * amount;
					}
					s->nr_sel++;
				} else {
					dlg_message(g_dialog, s->txt[18]);
				}
			} else if (g_action >= 0xf1 && g_action <= 0xf7) {
				register signed short n;
				n = g_group->nr_heroes;
				old = h;
				h = 0;
				s->hero = g_action - 0xf1;
				if (group_get_npc(g_group))
					n--;
				if (s->hero == 6)
					h = group_get_npc(g_group);
				else if (s->hero < n)
					h = g_group->heroes[s->hero];
				if (group_has_hero(g_group, h) == 1) {
					store_deposit_mark_hero(s);
					refill = 1;
					redraw = 1;
				} else {
					h = old;
				}
			} else if (g_action == 0x81 && s->value != 0) {
				if (s->nr_sel != 0)
					store_deposit_commit(s);
				done = 1;
				s->value = s->depot->value;
				s->nr_sel = 0;
				for (i = 0; i < 7; i++)
					memset(s->sel_qty[i], 0, 0x1f);
			} else if (g_action == 0x82 && s->wares[s->base + 15].id != 0 && nr_icons == 5) {
				redraw = 1;
				s->base += 15;
			} else if (g_action == 0x83 && nr_icons == 5) {
				if (s->base != 0) {
					s->base -= 15;
					redraw = 1;
				} else {
					s->sel -= 10;
					if (s->sel < 0)
						s->sel = 0;
				}
			} else if (g_action == 0x84 && nr_icons == 5 || g_action == 0x82 && nr_icons == 3) {
				choose_hero = 1;
			} else if (nr_icons + 0x80 == g_action || g_action == 0x11b) {
				done = 1;
			}
		}
		font_restore_colors(g_font);
		g_refresh = 1;
		g_portrait_drag = 1;
		g_pp20_index = -1;
	}
	for (i = 0; i < 7; i++)
		s->sel_qty[i] = (signed char far *)mem_free(s->sel_qty[i]);
}

signed short store_deposit_check(Store far *s)
{
	register signed short ok = 1;

	if (g_group->money < 100) {
		dlg_message(g_dialog, s->txt[8]);
		ok = 0;
	}
	return ok;
}

void store_deposit_commit(Store far *s)
{
	signed short ticket;
	signed short n = 0;
	register signed short j;
	register signed short i;

	s->new_items = (struct inv_slot far *)mem_alloc(0x16e3L);
	group_sub_money(g_group, 100L);
	for (i = 0; i < 7; i++) {
		for (j = 0; j < NR_INV_SLOTS; j++) {
#if FIX_0352
			/* Fix #352: Platz 6 = NSC ueber group_get_npc (heroes[6] ist bei kleiner Gruppe leer) */
			struct hero far *h = i == 6 ? group_get_npc(g_group) : g_group->heroes[i];
			if (s->sel_qty[i][j] != 0 && h != 0) {
				s->new_items[n] = h->get_inv(j);
				drop_item(h, j, s->sel_qty[i][j]);
				s->new_items[n].quantity = s->sel_qty[i][j];
				n++;
			}
#else
			if (s->sel_qty[i][j] != 0) {
				s->new_items[n] = g_group->heroes[i]->get_inv(j);
				drop_item(g_group->heroes[i], j, s->sel_qty[i][j]);
				s->new_items[n].quantity = s->sel_qty[i][j];
				n++;
			}
#endif
		}
	}
	s->depot->nr_items += s->nr_sel;
#if FIX_0352
	/* Fix #352: s->value enthaelt schon den alten Warenwert (store_deposit beginnt mit depot->value) - nicht verdoppeln */
	s->depot->value = s->value;
#else
	s->depot->value += s->value;
#endif
	date_now(&s->depot->stored);
	s->save(0);
	ticket = g_loc_id == 1 ? ITEM_DEPOT_TICKET_1 : g_loc_id == 2 ? ITEM_DEPOT_TICKET_2 : ITEM_DEPOT_TICKET_3;
	if (group_find_item(g_group, ticket) == -1)
		group_give_item(g_group, ticket, 1);
	dlg_message(g_dialog, s->txt[19]);
	s->new_items = (struct inv_slot far *)mem_free(s->new_items);
}

void store_deposit_draw_grid(Store far *s, struct hero far *h)
{
	signed short col;
	register signed short row;
	register signed short idx;

	for (col = 0; col < 3; col++) {
		for (row = 0; row < 5; row++) {
			idx = col * 5 + row + s->base;
			if (s->wares[idx].id != 0)
				store_deposit_draw_item(s, idx, h);
		}
	}
}

void store_deposit_draw_item(Store far *s, signed short idx, struct hero far *h)
{
	Bmp b(0x10, 0x10);
	signed short dummy;
	signed short n;
	register signed short row;
	register signed short col;

	row = (idx - s->base) % 5;
	col = (idx - s->base) / 5;
	nvfile_load(g_nvf_heads, 0x0b, b.data(), 0, g_items[s->wares[idx].id].gfx, 0, &dummy, &dummy, 0x140);
	b.put(g_vga_buf, s->grid_x[col], s->grid_y[row]);
	if (g_items[s->wares[idx].id].stackable && (n = h->inv[s->wares[idx].slot].quantity) > 1) {
		itoa(n, g_text_output_buf, 10);
		font_set_colors(g_font, 0xfe, -1);
		font_print(g_font, g_text_output_buf, s->grid_x[col] + 17 - font_text_width(g_font, g_text_output_buf),
			s->grid_y[row] + 9);
		font_restore_colors(g_font);
		font_print(g_font, g_text_output_buf, s->grid_x[col] + 16 - font_text_width(g_font, g_text_output_buf),
			s->grid_y[row] + 9);
	}
	if (s->sel_qty[s->hero][s->wares[idx].slot] != 0) {
		itoa(s->sel_qty[s->hero][s->wares[idx].slot], g_text_output_buf, 10);
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

void store_deposit_fill(Store far *s, struct hero far *h)
{
	signed short n = 0;
	register signed short i;
	register signed short id;

	for (i = 0; i < NR_INV_SLOTS; i++) {
		id = h->inv[i].item_id;
		if (id != 0 && !g_items[id].undropable && !g_items[id].food && !g_items[id].herb_potion &&
		    id != ITEM_DEPOT_TICKET_1 && id != ITEM_DEPOT_TICKET_2 && id != ITEM_DEPOT_TICKET_3) {
			s->wares[n].id = id;
			s->wares[n].slot = i;
			s->wares[n].price = g_items[id].price;
			s->wares[n].unit = g_items[id].price_unit;
			if (s->wares[n].price == 0)
				s->wares[n].price = 1;
			n++;
		}
	}
	s->count = n;
	for (i = n; i < 0x100; i++)
		s->wares[i].id = 0;
}

void store_deposit_mark_hero(Store far *s)
{
	screen_draw_name_normal(g_screen, s->old_hero);
	screen_draw_name(g_screen, s->hero, 0xfb);
	s->old_hero = s->hero;
}
