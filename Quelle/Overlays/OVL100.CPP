/* OVL100 - Overlay O100 (Segment 0x2144 in SCHWEIF.EXE): Lagerhaus - hinterlegte Gegenstaende abholen (store_fetch,
 * Aufruf aus Store::run O012; Objekt class Store (s_class.h) mit Lagerplatz s->depot (Wert value, Anzahl nr_items,
 * Zeitstempel stored), Bestaenden je Lagerhaus LOC_ITEMS(s) = items[g_loc_id - 1] (27-Byte-inv_slot), Warenliste
 * s->wares (7 Bytes struct ware {id, price, unit, slot}), Auswahl s->take_qty (256 Bytes, -1 = verloren)).
 * store_fetch(s): Pruefung store_fetch_check, Warenliste store_fetch_fill, Hintergrund retten (Bmp 0xd0 x 0x87,
 *   get), Gesicht (Bmp 24x24 aus s->pic_buf), Menueschleife (Leiste draw_loc_icons mit 2 bzw. 4 Punkten je nach
 *   Warenzahl > 15, Eingabe check_time/grid_input, Menue dlg_choose): 0x90/0x1c0d Ware an-/abwaehlen (bei
 *   stapelbaren Waren Menge per dlg_input), 0x81 abholen (store_fetch_commit), 0x82/0x83 blaettern (+-15), 0x80 +
 *   Punkte bzw. ESC 0x11b Ende. store_fetch_commit(s): gewaehlte Waren per group_give_item an die Gruppe, Rest
 *   zusammenschieben (Puffer s->rest), Wert des Rests s->value = Summe price * unit * Menge, depot->value -= value,
 *   save(1); leeres Lager -> Lagerschein ITEM_DEPOT_TICKET_1..3 (Ort 1/2/3) per group_drop_item entfernen.
 *   store_fetch_check(s, &n): Lagerschein vorhanden (group_find_item), Gruppengeld >= 100, Alter in Tagen =
 *   date_since(stored) / TICKS_PER_DAY; > 465 Tage alles verloren, > 365 Tage Verlust nach W100 > alter - 365, sonst
 *   Verderb mit Wahrscheinlichkeit 1 / (3000 / alter); Meldungen txt[20..24]. store_fetch_draw_grid(s): 3 x 5
 *   Warenfelder zeichnen (Bild aus g_nvf_heads Nr. 0x0b, Stueckzahl und Auswahl als Zahl mit Schatten).
 *   store_fetch_fill(s): Warenliste aus dem Bestand.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL100.CPP   -> 4186/4186 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - -Og: (int)w.slot als Zwischenwert in SI, Offsets slot * 27 in [bp-14h] bzw. DI, i * 7 in [bp-6]; "nr_icons"
 *   OHNE register (-> DI, mit register tauschen SI/DI); die beiden Bmp in einem inneren Block (liegen hinter dem
 *   -Og-Zwischenwert [bp-14h], Destruktoren am Blockende rueckwaerts vor dem Freigeben von s->take_qty).
 * - "s->rest[n++] = LOC_ITEMS(s)[i];" (27-Byte-Struktur) -> Zieladresse doppelt gepusht, nach SCOPY@ pop ax /
 *   pop dx, Adresse als off + n * 27 (push/pop um imul); getrennt "s->rest[n] = ..; n++;" rechnet n * 27 + off und
 *   pusht das Ziel nur einmal (291 Abweichungen).
 * - LOC_ITEMS: das Original indiziert ab +0x5c mit g_loc_id (1..3), also (&new_items)[g_loc_id] statt
 *   items[g_loc_id - 1] (gleiche Adresse, aber dec ax).
 * - "w = s->wares[s->sel + s->base]" (7 Bytes) -> SCOPY@ cx=7; "s->wares[s->base + 15].id" -> imul 7 / +69h.
 * - "(signed long)price * unit * n" -> movsx/imul eax,edx / push eax / movsx / pop edx / imul edx,eax.
 * - group_find_item(..) == -1 nach Aufruf -> cmp ax,0ffffh; globaler far-Zeiger "g_mouse_areas2 = 0" -> mov dword.
 * - Funktionen weiter unten (store_fetch_commit, _check, _draw_grid, _fill) -> far call aus store_fetch;
 *   store_fetch_check "return ok;" am Ende -> toter Epilog. font_print(.., x - font_text_width(..), y): y wird vor
 *   dem inneren Aufruf gepusht.
 * - screen_textbar mit int-Farbe (push word) ueber gecasteten Funktionszeiger LOC_TEXTBAR.
 * Originalfehler/Auffaelliges: store_fetch_check Verlust bei 365 < alter <= 465 mit "W100 > alter - 365" - je
 *   juenger, desto mehr geht verloren (vermutlich <= gemeint).
 */
#include <stdio.h>
#include <stdlib.h>
#include "schweif.h"

/* Ortsname in die Textleiste: Farbe 0xff als int wie im Original */
#define LOC_TEXTBAR(t) ((void (*)(void far *, char far *, signed short))screen_textbar)(g_screen, t, 0xff)
/* Gegenstaende des aktuellen Lagerhauses (Index ab +0x5c mit g_loc_id 1..3, siehe Kopf) */
#define LOC_ITEMS(s) ((&(s)->new_items)[g_loc_id])

void store_fetch(Store far *s)
{
	signed short amount;
	signed short choice;
	struct ware w;
	signed short redraw = 1;
	signed short keep = 0;
	signed short done = 0;
	signed short nr_icons;

	s->take_qty = (signed char far *)mem_alloc(0x100L);
	if (store_fetch_check(s, &s->depot->nr_items)) {
		store_fetch_fill(s);
		g_portrait_drag = 0;
		s->base = 0;
		s->old_sel = -1;
		s->sel = 0;
		s->nr_sel = 0;
		s->mx = g_mouse_posx;
		s->my = g_mouse_posy;
		g_pp20_index = -1;
		screen_redraw(g_screen);
		mouse_hide(0);
		Bmp back(0xd0, 0x87);
		back.get(g_vga_buf, 0x10, 2);
		mouse_show(0);
		read_record(s->pic_buf, 0x31);
		Bmp face(0x18, 0x18, s->pic_buf);
		font_set_colors(g_font, 0xff, 0);
		g_refresh = 1;
		while (done == 0) {
			if (redraw != 0 || g_refresh != 0) {
				mouse_hide(0);
				if (g_refresh != 0) {
					if (s->count > 15) {
						nr_icons = 4;
						draw_loc_icons(g_screen, 4, 0x17, 0x1a, 0x1b, 8);
					} else {
						nr_icons = 2;
						draw_loc_icons(g_screen, 2, 0x17, 8);
					}
					screen_redraw(g_screen);
					LOC_TEXTBAR(s->txt[g_loc_name]);
					snd_play(g_snd_music, 7);
					font_set_colors(g_font, 0xff, -1);
					g_refresh = 0;
				}
				back.put(g_vga_buf, 0x10, 2);
				do_v_line(g_vga_buf, 0x57, 0x23, 0x60, 0xff);
				do_v_line(g_vga_buf, 0x98, 0x23, 0x60, 0xff);
				face.put(g_vga_buf, 0x6c, 5);
				store_fetch_draw_grid(s);
				mouse_show(0);
				s->old_sel = -1;
				if (keep == 0)
					s->sel = 0;
				else
					keep = 0;
				redraw = 0;
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
				if (nr_icons == 4)
					choice = dlg_choose(g_dialog, 0, 4, s->txt[36], s->txt[13], s->txt[14], s->txt[16]) - 1;
				else
					choice = dlg_choose(g_dialog, 0, 2, s->txt[36], s->txt[16]) - 1;
				if (choice != -2)
					g_action = choice + 0x81;
			}
			if (g_mouse_rclick != 0 && get_click_action(g_action_table_merchant))
				g_action = 0x90;
			if (g_action == 0x90 || g_action == 0x1c0d) {
				redraw = 1;
				w = s->wares[s->sel + s->base];
				if (s->take_qty[w.slot] > 0) {
					if (g_items[w.id].stackable && LOC_ITEMS(s)[w.slot].quantity > 1) {
						s->take_qty[w.slot] = 0;
						sprintf(g_text_output_buf, s->txt[12], grammar_article(4, w.id, 0));
						amount = dlg_input(g_dialog, 2, g_text_output_buf, 0, LOC_ITEMS(s)[w.slot].quantity,
							LOC_ITEMS(s)[w.slot].quantity);
						if (amount != -1) {
							s->take_qty[w.slot] = s->take_qty[w.slot] + amount;
							if (amount == 0)
								s->nr_sel--;
						}
					} else {
						s->take_qty[w.slot] = s->take_qty[w.slot] - 1;
						s->nr_sel--;
					}
				} else if (s->take_qty[w.slot] != -1) {
					if (g_items[w.id].stackable && LOC_ITEMS(s)[w.slot].quantity > 1) {
						sprintf(g_text_output_buf, s->txt[12], grammar_article(4, w.id, 0));
						amount = dlg_input(g_dialog, 2, g_text_output_buf, 0, LOC_ITEMS(s)[w.slot].quantity,
							LOC_ITEMS(s)[w.slot].quantity);
					} else
						amount = 1;
					if (amount != -1) {
						s->take_qty[w.slot] = s->take_qty[w.slot] + amount;
						s->nr_sel++;
					}
				}
			} else if (g_action == 0x81) {
				if (s->nr_sel != 0)
					store_fetch_commit(s);
				s->nr_sel = 0;
				done = 1;
				store_fetch_fill(s);
			} else if (g_action == 0x82 && s->wares[s->base + 15].id != 0 && nr_icons == 4) {
				redraw = 1;
				s->base += 15;
			} else if (g_action == 0x83 && nr_icons == 4) {
				if (s->base != 0) {
					s->base -= 15;
					redraw = 1;
				} else {
					s->sel -= 10;
					if (s->sel < 0)
						s->sel = 0;
				}
			} else if (nr_icons + 0x80 == g_action || g_action == 0x11b) {
				done = 1;
			}
		}
		font_restore_colors(g_font);
		g_refresh = 1;
		g_portrait_drag = 1;
		g_pp20_index = -1;
	}
	s->take_qty = (signed char far *)mem_free(s->take_qty);
}

void store_fetch_commit(Store far *s)
{
	signed short n = 0;
	register signed short i = 0;
	signed short got;

	s->value = 0;
	s->rest = (struct inv_slot far *)mem_alloc(s->depot->nr_items * sizeof(struct inv_slot));
	for (; s->depot->nr_items > i; i++) {
#if FIX_0352
		/* Fix #352: als verkauft/zerbrochen gemeldete Plaetze (take_qty -1) wirklich entfernen */
		if (s->take_qty[i] == -1) {
			LOC_ITEMS(s)[i].quantity = 0;
		} else if (s->take_qty[i] != 0) {
			got = group_give_item(g_group, s->wares[i].id, s->take_qty[i]);
#else
		if (s->take_qty[i] != 0) {
			got = group_give_item(g_group, s->wares[i].id, s->take_qty[i]);
#endif
			s->take_qty[i] = s->take_qty[i] - got;
			LOC_ITEMS(s)[i].quantity -= got;
		}
		if (LOC_ITEMS(s)[i].quantity != 0) {
			s->rest[n++] = LOC_ITEMS(s)[i];
			s->value += (signed long)s->wares[i].price * s->wares[i].unit * LOC_ITEMS(s)[i].quantity;
		}
	}
	s->depot->nr_items = n;
#if FIX_0352
	/* Fix #352: Warenwert = Wert des Rests */
	s->depot->value = s->value;
#else
	s->depot->value -= s->value;
#endif
	date_now(&s->depot->stored);
	s->save(1);
	if (s->depot->nr_items == 0)
		group_drop_item(g_group, g_loc_id == 1 ? ITEM_DEPOT_TICKET_1 : g_loc_id == 2 ? ITEM_DEPOT_TICKET_2 :
			ITEM_DEPOT_TICKET_3);
	s->count = s->depot->nr_items;
	s->rest = (struct inv_slot far *)mem_free(s->rest);
}

signed short store_fetch_check(Store far *s, signed short far *cnt)
{
	signed short lim;
	signed short k;
	signed short days;
	signed short ok = 1;
	register signed short i;

	if (group_find_item(g_group, g_loc_id == 1 ? ITEM_DEPOT_TICKET_1 : g_loc_id == 2 ? ITEM_DEPOT_TICKET_2 :
		ITEM_DEPOT_TICKET_3) == -1) {
		ok = 0;
		dlg_message(g_dialog, s->txt[27]);
	}
	if (ok != 0 && g_group->money < 100) {
		dlg_message(g_dialog, s->txt[8]);
		ok = 0;
	}
	if (ok != 0) {
		days = date_since(&s->depot->stored) / TICKS_PER_DAY;
		if (days == 0)
			days = 1;
		if (days > 465) {
			dlg_message(g_dialog, s->txt[20]);
			ok = 0;
		}
	}
	if (ok != 0) {
		*cnt = s->depot->nr_items;
		if (days > 365) {
			k = 0;
			lim = days - 365;
			for (i = 0; s->depot->nr_items > i; i++) {
#if FIX_0078
				/* Fix #78: Verlust steigt mit dem Alter (<= statt >) */
				if (random_schweif(100) <= lim) {
#else
				if (random_schweif(100) > lim) {
#endif
					s->take_qty[i] = -1;
					k++;
				}
			}
			if (k > 0) {
				*cnt -= k;
				if (k == 1)
					dlg_message(g_dialog, s->txt[21]);
				else {
					sprintf(g_text_output_buf, s->txt[23], k);
					dlg_message(g_dialog, g_text_output_buf);
				}
			}
		}
		k = 0;
		lim = 3000 / days;
		for (i = 0; s->depot->nr_items > i; i++) {
			if (random_schweif(lim) == 1) {
				s->take_qty[i] = -1;
				k++;
			}
		}
		if (k > 0) {
			*cnt -= k;
			if (k == 1)
				dlg_message(g_dialog, s->txt[22]);
			else {
				sprintf(g_text_output_buf, s->txt[24], k);
				dlg_message(g_dialog, g_text_output_buf);
			}
		}
	}
	s->count = *cnt;
	return ok;
}

void store_fetch_draw_grid(Store far *s)
{
	Bmp b(0x10, 0x10);
	signed short col;
	signed short idx;
	signed short dummy;
	signed short n;
	register signed short row;

	for (col = 0; col < 3; col++) {
		for (row = 0; row < 5; row++) {
			idx = col * 5 + row + s->base;
			if (s->wares[idx].id != 0) {
				if (s->take_qty[s->wares[idx].slot] != -1) {
					nvfile_load(g_nvf_heads, 0x0b, b.data(), 0, g_items[s->wares[idx].id].gfx, 0, &dummy,
						&dummy, 0x140);
					b.put(g_vga_buf, s->grid_x[col], s->grid_y[row]);
					if (g_items[s->wares[idx].id].stackable &&
					    (n = LOC_ITEMS(s)[s->wares[idx].slot].quantity) > 1) {
						itoa(n, g_text_output_buf, 10);
						font_print(g_font, g_text_output_buf,
							s->grid_x[col] + 16 - font_text_width(g_font, g_text_output_buf), s->grid_y[row] + 9);
					}
					if (s->take_qty[s->wares[idx].slot] != 0) {
						itoa(s->take_qty[s->wares[idx].slot], g_text_output_buf, 10);
						font_set_colors(g_font, 0xfe, -1);
						font_print(g_font, g_text_output_buf,
							s->grid_x[col] + 17 - font_text_width(g_font, g_text_output_buf), s->grid_y[row]);
						font_restore_colors(g_font);
						font_set_colors(g_font, 0xf9, -1);
						font_print(g_font, g_text_output_buf,
							s->grid_x[col] + 16 - font_text_width(g_font, g_text_output_buf), s->grid_y[row]);
						font_restore_colors(g_font);
					}
				}
			}
		}
	}
}

void store_fetch_fill(Store far *s)
{
	signed short id;
	signed short i;

	for (i = 0; i < s->count; i++) {
		id = LOC_ITEMS(s)[i].item_id;
		if (id != 0) {
			s->wares[i].id = id;
			s->wares[i].slot = i;
			s->wares[i].price = g_items[id].price;
			s->wares[i].unit = g_items[id].price_unit;
		}
	}
	for (i = s->count; i < 0x100; i++)
		s->wares[i].id = 0;
}
