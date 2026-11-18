/* OVL231 - Overlay O231 (Segment 0x2404 in SCHWEIF.EXE): Reparaturbildschirm der Schmiede (BE seg058 repair_screen,
 * add_item_to_smith) fuer das Objekt aus O230 (class Smith, 0x56 Bytes; hier freie Funktionen mit Smith far *o;
 * Aufruf Smith::run). smith_select_hero(o) (O231_11E0): Helden mit reparierbaren Gegenstaenden sammeln (o->heroes[7],
 * Namen 7 x 16 lokal, dlg_choose txt[7]), Rueckgabe Held oder 0. smith_repair(o, h) (O231_0000): Hauptschleife wie
 * O227/O228 - Raster (smith_draw_grid), Liste smith_list_items, Symbolfelder draw_loc_icons (2..5 Spalten), Geld
 * money_to_string, Auswahl Smith::grid_input, Menue txt[13..17]; Rasterklick/ACTION_ICON_1/Enter: Angebot txt[9]
 * (Preis * Einheit), Feilschen bargain (-99 -> Hausverbot g_smith_banned[g_loc_id], o->done), Rabatt
 * clamp_long(r, 0, 30) %, Geldpruefung, Gegenstand merken (o->repair->item_id, date_now), Abschlusstext je
 * Tageszeit (nach 14 Uhr) + strcat, drop_item abgeben, group_sub_money bezahlen; ACTION_ICON_2/_3 Seite,
 * ACTION_ICON_4/_2 Heldenwechsel, ACTION_PORTRAIT.. Held direkt (group_has_hero). smith_draw_cursor: Rahmen
 * (do_border) und Name (g_item_names[id], grammar_name). smith_mark_hero: Namensleiste screen_draw_name_normal/
 * screen_draw_name. smith_draw_grid: 3 x 5 Raster mit Bild (Nvf g_nvf_heads, Datei 0xb) und Preis txt[21..23] je
 * Einheit 1/10/sonst. smith_list_items(o, h): Liste aus den 31 Inventarplaetzen - reparierbar, wenn (Waffe, nicht
 * stapelbar und (zerbrochen oder Bruchfaktor > Startwert g_weapons_bf[Waffe])) oder (Ruestung mit rs_lost); Preis
 * (Grundpreis + Grundpreis * o->price_mod / 100) / 2, bei beschaedigt nochmals halbiert, mindestens 1.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL231.CPP   -> 5216/5216 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 *   Ohne -Og 179 Bytes zu lang, ohne -Oi 59 Bytes zu kurz (memset/strcpy/strcat als Aufrufe).
 * - smith_repair im ersten Entwurf GLEICH: Bmp b1/b2 mitten in der Funktion deklariert (Ktor an der Stelle), -Og-
 *   Zwischenwert [bp-32h] = (sel + page) * 7, Block-Locals n [bp-34h], i SI, found DI.
 * - smith_list_items/smith_select_hero: Vergleich zweier signed char als int "(signed short)inv[i].bf >
 *   (signed short)g_weapons_bf[..]" -> mov al/cbw/push ax ... cbw/pop dx/cmp dx,ax; Wahrheitstest "!!rs_lost" auch
 *   in der ||-Kette (cbw/or ax,ax). g_weapons_bf ist ein eigenes Public (Schrittweite 8: Index * 8).
 * - Preis: "(price + (signed short)((signed long)price * (signed long)price_mod / 100)) >> 1" -> 16-Bit-add + sar;
 *   ohne den short-Cast legt -Og den long-Wert in [bp-0eh] ab (add edx / sar edx).
 * - smith_select_hero: Initialisierer mit Aufrufen in Deklarationsreihenfolge (old, sel = -1, h); zwei tote
 *   Epiloge.
 * Originalfehler/Auffaelligkeiten: smith_repair prueft "if (g_refresh)" direkt nach "g_refresh = 0" ein zweites
 *   Mal - der Block (screen_redraw/snd_play/font_set_colors) ist toter Code.
 */
#include <stdio.h>
#include <string.h>
#include "schweif.h"

#define NVF_HEADS	((Nvf far *)g_nvf_heads)	/* NVF-Dateiobjekt (O098) */
#define DATE		((struct game_date far *)g_date)
#define ITEM_PICS	0x0b		/* NVF-Datei der Gegenstandsbilder */
#define ACTION_GRID	0x90		/* Klick in das Auswahlraster */

void smith_repair(Smith far *o, struct hero far *h)
{
	signed short ans;
	signed short r;
	signed short id;
	signed short cnt;
	signed short cols;
	signed long price;
	struct hero far *save;
	signed short redraw = 1;
	signed short redraw2 = 1;
	signed short pick = 0;
	signed short done = 0;
	signed short keep = 0;

	g_portrait_drag = 0;
	o->old_sel = -1;
	o->sel = 0;
	o->old_cur = 0;
	o->page = 0;
	o->mouse_x = g_mouse_posx;
	o->mouse_y = g_mouse_posy;
	ani_halt(g_screen_pic);
	g_pp20_index = -1;
	screen_redraw(g_screen);
	mouse_hide(0);
	Bmp b1(0xd0, 0x87);
	b1.get(g_vga_buf, 0x10, 2);
	mouse_show(0);
	read_record(o->icon_buf, 0x12);
	Bmp b2(0x18, 0x18, o->icon_buf);
	font_set_colors(g_font, 0xff, 0);
	g_refresh = 1;
	if (group_get_npc(g_group) == h)
		o->cur = 6;
	else
		o->cur = group_hero_pos(g_group, h);
	smith_mark_hero(o);

	while (!done) {
		if (redraw != 0 || pick != 0 || redraw2 != 0 || g_refresh != 0) {
			if (g_refresh != 0) {
				screen_redraw(g_screen);
				snd_play(g_snd_music, 4);
				font_set_colors(g_font, 0xff, 0);
				smith_mark_hero(o);
				redraw2 = 1;
			}
			if (pick != 0) {
				save = h;
				h = smith_select_hero(o);
				if (h != 0) {
					if (group_get_npc(g_group) == h)
						o->cur = 6;
					else
						o->cur = group_hero_pos(g_group, h);
					smith_mark_hero(o);
					o->old_sel = -1;
					o->sel = 0;
					redraw2 = 1;
					pick = 0;
				} else if (save != 0) {
					h = save;
					o->cur = o->old_cur;
					pick = 0;
				} else
					done = 1;
			}
			if (!done) {
				if (redraw2 != 0) {
					memset(o->items, 0, 0xd9);
					cnt = smith_list_items(o, h);
					redraw2 = 0;
					o->old_sel = -1;
					o->sel = o->page = 0;
				}
				if (g_refresh != 0) {
					if (o->count > 1) {
						if (cnt > 15) {
							cols = 5;
							draw_loc_icons(g_screen, 5, 0x17, 0x1a, 0x1b, 0x1c, 8);
						} else {
							cols = 3;
							draw_loc_icons(g_screen, 3, 0x17, 0x1c, 8);
						}
					} else {
						if (cnt > 15) {
							cols = 4;
							draw_loc_icons(g_screen, 4, 0x17, 0x1a, 0x1b, 8);
						} else {
							cols = 2;
							draw_loc_icons(g_screen, 2, 0x17, 8);
						}
					}
					g_refresh = 0;
				}
				if (g_refresh != 0) {
					screen_redraw(g_screen);
					snd_play(g_snd_music, 4);
					font_set_colors(g_font, 0xff, 0);
				}
				mouse_hide(0);
				b1.put(g_vga_buf, 0x10, 2);
				do_v_line(g_vga_buf, 0x57, 0x23, 0x60, 0xff);
				do_v_line(g_vga_buf, 0x98, 0x23, 0x60, 0xff);
				b2.put(g_vga_buf, 0x6c, 5);
				money_to_string(g_text_output_buf, g_group->money);
				font_print(g_font, g_text_output_buf, 0x68 - font_text_width(g_font, g_text_output_buf), 0xd);
				smith_draw_grid(o);
				mouse_show(0);
				o->old_sel = -1;
				if (keep == 0)
					o->sel = 0;
				else
					keep = 0;
				redraw = 0;
			}
		}
		g_mouse_areas2 = g_action_table_merchant;
		handle_input(g_screen);
		g_mouse_areas2 = 0;
		keep = o->grid_input(&redraw);
		smith_draw_cursor(o);
		if (g_mouse_rclick != 0 || g_action == KEY_PGUP) {
			if (cols == 5)
				ans = dlg_choose(g_dialog, (char far *)0, 5, o->txt[13], o->txt[14], o->txt[15], o->txt[16], o->txt[17]) - 1;
			else if (cols == 4)
				ans = dlg_choose(g_dialog, (char far *)0, 4, o->txt[13], o->txt[14], o->txt[15], o->txt[17]) - 1;
			else if (cols == 3)
				ans = dlg_choose(g_dialog, (char far *)0, 3, o->txt[13], o->txt[16], o->txt[17]) - 1;
			else
				ans = dlg_choose(g_dialog, (char far *)0, 2, o->txt[13], o->txt[17]) - 1;
			if (ans != -2)
				g_action = ans + ACTION_ICON_1;
		}
		if (g_action == ACTION_GRID || g_action == ACTION_ICON_1 || g_action == KEY_RETURN) {
			id = o->items[o->sel + o->page].item_id;
			price = (signed long)o->items[o->sel + o->page].price * (signed long)o->items[o->sel + o->page].unit;
			money_to_string(g_text_buf2, price);
			sprintf(g_text_output_buf, o->txt[9], grammar_article(0x8002, id, 0), g_text_buf2,
				o->female ? o->txt[39] : o->txt[38], grammar_pronoun(o->female, 0, 0));
			save = h;
			preselect_best_skill(g_group, 0x15);
			h = get_active_hero_forced(g_group, g_text_output_buf);
			r = bargain(h, 1, price, 0, 0);
			if (r == -99) {
				dlg_message(g_dialog, o->txt[12]);
				g_smith_banned[g_loc_id] = 1;
				done = o->done = 1;
			} else {
				r = clamp_long(r, 0, 30);
				price -= price * r / 100;
				if (g_group->money < price) {
					sprintf(g_text_output_buf, o->txt[18], o->female ? o->txt[39] : o->txt[38]);
					dlg_message(g_dialog, g_text_output_buf);
				} else {
					money_to_string(g_text_buf2, price);
					if (r == 0)
						sprintf(g_text_output_buf, o->txt[20], h->name);
					else
						sprintf(g_text_output_buf, o->txt[19], h->name, r, g_text_buf2);
					dlg_message(g_dialog, g_text_output_buf);
					o->repair->item_id = id;
					date_now((struct timestamp far *)o->repair->since);
					if (g_group->town == 4)
						sprintf(g_text_output_buf, o->txt[37]);
					else
						sprintf(g_text_output_buf, g_str_empty_731e);
					if (DATE->ticks > (14 * TICKS_PER_HOUR))
						sprintf(g_text_buf2, o->txt[10], o->female ? o->txt[39] : o->txt[38], grammar_pronoun(o->female, 1, 1));
					else
						sprintf(g_text_buf2, o->txt[11], o->female ? o->txt[41] : o->txt[40]);
					strcat(g_text_output_buf, g_text_buf2);
					dlg_message(g_dialog, g_text_output_buf);
					h = save;
					drop_item(h, o->items[o->sel + o->page].slot, 1);
					group_sub_money(g_group, price);
					done = 1;
				}
			}
		} else if (g_action == ACTION_ICON_2 && o->items[o->page + 15].item_id != 0 && cols >= 4) {
			redraw = 1;
			o->page += 15;
		} else if (g_action == ACTION_ICON_3 && o->page != 0 && cols >= 4) {
			redraw = 1;
			o->page -= 15;
		} else if ((g_action == ACTION_ICON_4 && cols == 5) || (g_action == ACTION_ICON_2 && cols == 3)) {
			pick = 1;
		} else if (cols + ACTION_ICON_1 - 1 == g_action || g_key == KEY_ESC) {
			done = 1;
		} else if (g_action >= ACTION_PORTRAIT && g_action <= ACTION_PORTRAIT + 6) {
			signed short n;
			register signed short i;
			register signed short found;

			n = g_group->nr_heroes;
			found = 0;
			save = h;
			h = 0;
			o->cur = g_action - ACTION_PORTRAIT;
			if (group_get_npc(g_group))
				n--;
			if (o->cur == 6)
				h = group_get_npc(g_group);
			else if (o->cur < n)
				h = g_group->heroes[o->cur];
			if (h != 0) {
				if (h != save) {
					for (i = 0; i < 7; i++)
						if (o->heroes[i] == h)
							found = 1;
					if (o->count > 1 && found) {
						if (group_has_hero(g_group, h)) {
							smith_mark_hero(o);
							redraw2 = 1;
							redraw = 1;
						} else {
							h = save;
							o->cur = o->old_cur;
						}
					} else {
						sprintf(g_text_output_buf, o->txt[43], hero_name_gen(h, 1));
						dlg_message(g_dialog, g_text_output_buf);
						h = save;
						o->cur = o->old_cur;
					}
				}
			}
		}
	}
	font_restore_colors(g_font);
	g_refresh = 1;
	g_portrait_drag = 1;
	g_pp20_index = -1;
}

void smith_draw_cursor(Smith far *o)
{
	char far *name;
	signed short x0;
	signed short x1;
	signed short y0;
	signed short y1;
	register signed short old;

	if ((old = o->old_sel) != o->sel) {
		x0 = o->cell_x[old / 5];
		x1 = o->cell_x[o->sel / 5];
		y0 = o->cell_y[old % 5];
		y1 = o->cell_y[o->sel % 5];
		mouse_hide(0);
		if (o->old_sel != -1)
			do_border(g_vga_buf, x0 - 1, y0 - 1, 0x12, 0x12, 0);
		do_border(g_vga_buf, x1 - 1, y1 - 1, 0x12, 0x12, 0xff);
		screen_clear_textbar(g_screen);
		if ((name = g_item_names[o->items[o->sel + o->page].item_id]) != 0)
			screen_textbar(g_screen, grammar_name(name), 0xff);
		o->old_sel = o->sel;
		mouse_show(0);
	}
}

void smith_mark_hero(Smith far *o)
{
	screen_draw_name_normal(g_screen, o->old_cur);
	screen_draw_name(g_screen, o->cur, 0xfb);
	o->old_cur = o->cur;
}

void smith_draw_grid(Smith far *o)
{
	signed short id;
	signed short wh;
	Bmp b(0x10, 0x10);
	struct repair_item far *e;
	register signed short col;
	register signed short row;

	for (row = 0; row < 3; row++)
		for (col = 0; col < 5; col++) {
			e = &o->items[row * 5 + col + o->page];
			id = e->item_id;
			if (id != 0) {
				NVF_HEADS->load(ITEM_PICS, b.data(), 0, g_items[id].gfx, 0, &wh, &wh, 0x140);
				b.put(g_vga_buf, o->cell_x[row], o->cell_y[col]);
				sprintf(g_text_output_buf, e->unit == 1 ? o->txt[21] : e->unit == 10 ? o->txt[22] : o->txt[23], e->price);
				font_print(g_font, g_text_output_buf, o->cell_x[row] + 20, o->cell_y[col] + 5);
			}
		}
}

signed short smith_list_items(Smith far *o, struct hero far *h)
{
	signed short id;
	signed short cnt = 0;
	struct repair_item far *e = o->items;
	register signed short i;

	for (i = 0; i < 31; i++) {
		id = h->inv[i].item_id;
		if ((g_items[id].weapon && !g_items[id].stackable &&
		     (h->inv[i].broken || (signed short)h->inv[i].bf > (signed short)g_weapons_bf[g_items[id].table_index * 8])) ||
		    (g_items[id].armor && !!h->inv[i].rs_lost)) {
			e->item_id = id;
			e->price = (g_items[id].price + (signed short)((signed long)g_items[id].price * (signed long)o->price_mod / 100)) >> 1;
			if ((signed short)h->inv[i].bf > (signed short)g_weapons_bf[g_items[id].table_index * 8] || !!h->inv[i].rs_lost)
				e->price >>= 1;
			if (e->price == 0)
				e->price = 1;
			e->unit = g_items[id].price_unit;
			e->slot = i;
			cnt++;
			e = &o->items[cnt];
		}
	}
	return cnt;
}

struct hero far *smith_select_hero(Smith far *o)
{
	signed short i;
	signed short id;
	signed short old = dlg_set16(g_dialog, 3);
	signed short sel = -1;
	struct hero far *h = get_first_hero(g_group);
	char names[7][16];

	memset(o->heroes, 0, sizeof(o->heroes));
	o->count = 0;
	do {
		if (h != 0 && check_hero(h)) {
			for (i = 0; i < 31; i++) {
				id = h->inv[i].item_id;
				if (id != 0) {
					if ((g_items[id].weapon && !g_items[id].stackable &&
					     (h->inv[i].broken || (signed short)h->inv[i].bf > (signed short)g_weapons_bf[g_items[id].table_index * 8])) ||
					    (g_items[id].armor && !!h->inv[i].rs_lost)) {
						o->heroes[o->count] = h;
						strcpy(names[o->count], h->name);
						o->count++;
						break;
					}
				}
			}
		}
		h = get_next_hero(g_group, h);
	} while (get_first_hero(g_group) != h);

	if (o->count == 0)
		dlg_message(g_dialog, o->txt[8]);
	else if (o->count == 1)
		sel = 0;
	else
		sel = dlg_choose(g_dialog, o->txt[7], o->count, names[0], names[1], names[2], names[3], names[4],
			names[5], names[6]) - 1;
	dlg_set16(g_dialog, old);
	if (sel >= 0)
		return o->heroes[sel];
	else
		return 0;
}
