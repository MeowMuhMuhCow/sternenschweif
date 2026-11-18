/* OVL069 - Overlay O069 (Segment 0x2093 in SCHWEIF.EXE): fahrender Haendler - class Peddler (s_class.h, 0x3a Bytes;
 * lokales Objekt in O031 nach dem Gespraech, das g_peddler (Haendler 1..3) und dessen Ladendaten g_peddler_stock ..
 * g_peddler_female setzt - O013). Peddler(): Texte txt_load_list(g_font, 0x7d, 0x46), Warenliste g_nr_items * 7 Bytes
 * (wares) und Puffer 0x240 (pic_buf), Raster grid_x/grid_y; ~Peddler() gibt frei. run: Menueschleife, Bild nach
 * Ladenart g_peddler_type/Haendlerin g_peddler_female (0x82..0x87), Name txt[0x3d + g_peddler], 0x81 Handel
 * (peddler_buy_screen O068, + 30 Minuten, Merker g_peddler_away (Haendler 1/2) bzw. g_peddler3_away (3) = 3 nach
 * Ausverkauf, sonst 1), 0x82/ESC Ende. build_wares: Warenliste aus g_items (Stufe f0b >= g_peddler_lvl) je nach
 * Ladenart SHOP_WEAPONS (Waffen ab 0, Ruestungen ab 70) / SHOP_HERBS / SHOP_GENERAL, plus drei Zusatzwaren
 * g_peddler_extra; qsort mit shop_compar (O229), bei Waffen zweiter Block ab Index 70 hinten angehaengt, Rest bis
 * 100 geleert. add_ware(id, idx): Eintrag mit Preis price + price * g_peddler_markup_food (food) bzw.
 * g_peddler_markup / 100 (mindestens 1). trade(preis): Kauf - Zahl der gewaehlten Posten (buy_qty), Feilschen
 * bargain (-99 = Hausverbot g_peddler_banned/g_peddler3_banned), Rabatt clamp_long(.., 0, 30) %, Waren per
 * group_give_item verteilen (Nachfragezaehler g_item_demand, ab 10 items_change_price), bezahlen group_sub_money, Texte
 * 0x11/0x12/0x13. grid_input: Auswahlraster wie shop_grid_input (O229).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL069.CPP   -> 2879/2879 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - qsort-Vergleich shop_compar liegt in einem anderen Overlay: Original "push seg F_O229_0000 / push 20h" =
 *   Stub-Thunk. C schreibt einfach shop_compar; TLINK /o loest den Offset-Fixup auf den Thunk auf.
 * - -Og: [bp-6]/[bp-3] in build_wares (Offset von &g_items[i], Byte +2), SI in add_ware, DI fuer
 *   g_item_demand[i] in trade, Epilogkopien in build_wares (if/else mit qsort am Ende).
 * - mem_alloc(g_nr_items * sizeof(struct ware)) -> movzx eax (mit "* 7" movsx).
 * - add_ware: "(signed long)price * g_peddler_markup_food / 100" -> movsx/movsx/imul/idiv; Rabatt
 *   "price -= price * pct / 100".
 * - Verschachtelter ?: fuer die Bildnummer (mov ax/jmp short je Zweig).
 * - screen_textbar mit int-Farbe (push word) ueber gecasteten Funktionszeiger TEXTBAR.
 */
#include <stdio.h>
#include <stdlib.h>
#include "schweif.h"

/* Phase B: eigenes BSS (ds:c812..c824) in Adressreihenfolge */
signed short g_peddler;
signed short g_peddler_stock;
signed short g_peddler_markup;
signed short g_peddler_markup_food;
signed char g_peddler_type;
signed char g_peddler_lvl;
signed short g_peddler_extra[3];
signed char g_peddler_female;

/* Name in die Textleiste: Farbe 0xff als int wie im Original */
#define TEXTBAR(t) ((void (*)(void far *, char far *, signed short))screen_textbar)(g_screen, t, 0xff)

Peddler::Peddler()
{
	txt_load_list(g_font, 0x7d, 0x46, &txt_buf, &txt);
	wares = (struct ware far *)mem_alloc(g_nr_items * sizeof(struct ware));
	pic_buf = mem_alloc(0x240);
	grid_x[0] = 0x1e;
	grid_x[1] = 0x5f;
	grid_x[2] = 0xa0;
	grid_y[0] = 0x23;
	grid_y[1] = 0x37;
	grid_y[2] = 0x4b;
	grid_y[3] = 0x5f;
	grid_y[4] = 0x73;
	sold = 0;
	g_refresh = 1;
	done = 0;
	sold_out = 0;
}

Peddler::~Peddler()
{
	mem_free(pic_buf);
	mem_free(wares);
	mem_free(txt_buf);
	mem_free(txt);
}

void Peddler::run()
{
	register signed short sel;

	while (done == 0) {
		if (g_refresh) {
			draw_loc_icons(g_screen, 2, 0x16, 8);
			screen_redraw(g_screen);
			anim_load(g_screen_pic, g_peddler_type == SHOP_WEAPONS ? (g_peddler_female == 0 ? 0x86 : 0x87) :
				(g_peddler_type == SHOP_HERBS ? (g_peddler_female == 0 ? 0x82 : 0x83) :
				(g_peddler_female == 0 ? 0x84 : 0x85)), 0);
			TEXTBAR(txt[g_peddler + 0x3d]);
			snd_play(g_snd_music, 0x18);
			g_refresh = 0;
		}
		handle_input(g_screen);
		if (g_mouse_rclick != 0 || g_key == 0x4900) {
			sel = dlg_choose(g_dialog, txt[1], 2, txt[2], txt[5]) - 1;
			if (sel != -2)
				g_action = sel + 0x81;
		}
		if (g_action == 0x81) {
			ani_halt(g_screen_pic);
			anim_free_all(g_screen_pic);
			peddler_buy_screen(this);
			timewarp_minutes(g_date, 30);
			if (sold_out) {
				done = 1;
				if (g_peddler == 1 || g_peddler == 2)
					g_peddler_away = 3;
				else if (g_peddler == 3)
					g_peddler3_away = 3;
			} else {
				if (g_peddler == 1 || g_peddler == 2)
					g_peddler_away = 1;
				else if (g_peddler == 3)
					g_peddler3_away = 1;
			}
		} else if (g_action == 0x82 || g_key == 0x11b)
			done = 1;
	}
}

void Peddler::build_wares()
{
	register signed short i = 1;
	register signed short n = 0;
	signed short k = 70;

	count = 0;
	while (g_items[i].gfx != -1) {
		if (g_items[i].f0b >= g_peddler_lvl) {
			if (g_items[i].armor || g_items[i].weapon) {
				if (g_peddler_type == SHOP_WEAPONS) {
#if FIX_0021
					/* Fix #21: Waffen nur bis Platz 69 (ab 70 liegen die Ruestungen) */
					if (g_items[i].weapon) {
						if (n < 70)
							add_ware(i, n++);
					}
#else
					if (g_items[i].weapon)
						add_ware(i, n++);
#endif
					else
						add_ware(i, k++);
				}
			} else if (g_items[i].herb_potion) {
				if (g_peddler_type == SHOP_HERBS)
					add_ware(i, n++);
			} else if (g_peddler_type == SHOP_GENERAL)
				add_ware(i, n++);
		}
		i++;
	}
	for (i = 0; i < 3; i++)
		if (g_peddler_extra[i])
			add_ware(g_peddler_extra[i], n++);
	if (g_peddler_type == SHOP_WEAPONS) {
		qsort(wares, n, sizeof(struct ware), shop_compar);
		qsort(wares + 70, k - 70, sizeof(struct ware), shop_compar);
		for (i = 0; i < k - 70; i++)
			wares[n + i] = wares[70 + i];
		for (i = n + k - 70; i < 100; i++)
			wares[i].id = 0;
	} else
		qsort(wares, n, sizeof(struct ware), shop_compar);
}

void Peddler::add_ware(signed short id, signed short idx)
{
	wares[idx].id = id;
	wares[idx].unit = g_items[id].price_unit;
	if (g_items[id].food)
		wares[idx].price = g_items[id].price + (signed long)g_items[id].price * g_peddler_markup_food / 100;
	else
		wares[idx].price = g_items[id].price + (signed long)g_items[id].price * g_peddler_markup / 100;
	if (wares[idx].price == 0)
		wares[idx].price = 1;
	count++;
}

signed short Peddler::trade(signed long price)
{
	signed short ret = 0;
	signed short more = 0;
	signed short nr_sel = 0;
	signed short pct;
	signed short got;
#if FIX_0020
	/* Fix #20: Hilfsvariablen fuer die Abrechnung der gelieferten Ware */
	struct hero far *h;
	register signed short i;
	signed short j;
	signed long paid = 0;

	for (i = 0; i < g_nr_items; i++)
		if (buy_qty[i])
			nr_sel++;
#else
	struct hero far *h;
	register signed short i;

	for (i = 0; i < g_nr_items; i++)
		if (buy_qty[i])
			nr_sel++;
#endif
	if (nr_sel) {
		preselect_best_skill(g_group, 0x15);
		h = get_active_hero_forced(g_group, txt[0x14]);
		pct = bargain(h, nr_sel, price, 0, 0);
		if (pct == -99) {
			sprintf(g_text_output_buf, txt[0x16], h->name, g_peddler_female ? txt[0x36] : txt[0x35]);
			dlg_message(g_dialog, g_text_output_buf);
			if (g_peddler == 1 || g_peddler == 2)
				g_peddler_banned = 1;
			else
				g_peddler3_banned = 1;
			ret = 1;
		} else {
			pct = clamp_long(pct, 0, 30);
			for (i = 0; i < g_nr_items; i++) {
				if (buy_qty[i]) {
					got = group_give_item(g_group, i, buy_qty[i]);
					g_item_demand[i] += got;
					if (g_item_demand[i] >= 10) {
						g_item_demand[i] = 0;
						items_change_price(&g_items, i, 10);
					}
#if FIX_0020
					/* Fix #20: weiter verteilen und nur die gegebenen Stueck berechnen */
					if (buy_qty[i] > got)
						more = 1;
					for (j = 0; j < 100; j++)
						if (wares[j].id == i) {
							paid += (signed long)wares[j].price * wares[j].unit * got;
							break;
						}
#else
					if (buy_qty[i] > got) {
						more = 1;
						break;
					}
#endif
				}
			}
#if FIX_0020
			/* Fix #20: bei Uebergepaeck nur die gelieferte Ware bezahlen */
			if (more)
				price = paid;
			price -= price * pct / 100;
			group_sub_money(g_group, price);
#else
			price -= price * pct / 100;
			group_sub_money(g_group, price);
#endif
			money_to_string(g_text_buf2, price);
			if (pct > 0)
				sprintf(g_text_output_buf, txt[0x12], h->name, pct, g_text_buf2);
			else
				sprintf(g_text_output_buf, txt[0x11], h->name, g_text_buf2);
			dlg_message(g_dialog, g_text_output_buf);
			if (more)
				dlg_message(g_dialog, txt[0x13]);
		}
	}
	return ret;
}

signed short Peddler::grid_input(signed short far *redraw)
{
	register signed short ret;
	register signed short i;

	ret = 0;
	if (g_mouse_locked == 0 && (mx != g_mouse_posx || my != g_mouse_posy)) {
		mx = g_mouse_posx;
		my = g_mouse_posy;
		for (i = 0; i < 15; i++) {
			if (g_merchant_items_posx[i] <= g_mouse_posx && g_merchant_items_posx[i] + 50 >= g_mouse_posx &&
			    g_merchant_items_posy[i] <= g_mouse_posy && g_merchant_items_posy[i] + 17 >= g_mouse_posy &&
			    wares[base + i].id != 0) {
				sel = i;
				break;
			}
		}
	} else if (g_action == 0x4800) {
		if (sel > 0)
			sel--;
		else {
			sel = 14;
			while (wares[base + sel].id == 0 && sel != 0)
				sel--;
		}
	} else if (g_action == 0x5000) {
		if (sel < 14) {
			sel++;
			if (wares[base + sel].id == 0)
				sel = 0;
		} else
			sel = 0;
	} else if (g_action == 0x4d00) {
		if (sel < 10) {
			if (wares[base + sel + 5].id != 0)
				sel += 5;
		} else {
			if (wares[base + 15].id != 0) {
				*redraw = 1;
				base += 15;
			}
		}
	} else if (g_action == 0x4b00) {
		if (sel > 4)
			sel -= 5;
		else {
			ret = 1;
			if (base != 0) {
				base -= 15;
				*redraw = 1;
#if FIX_0024
			/* Fix #24: erste Spalte der ersten Seite: Cursor bleibt stehen (sel -= 10 ergab immer 0) */
			}
#else
			} else {
				sel -= 10;
				if (sel < 0)
					sel = 0;
			}
#endif
		}
	}
	return ret;
}
