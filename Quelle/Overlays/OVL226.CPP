/* OVL226 - Overlay O226 (Segment 0x23ec in SCHWEIF.EXE): Haendler/Laden - class Shop (s_class.h, 0x47 Bytes, *g_shop).
 * Felder (struct shop): count Anzahl Waren, done Ende, txt_buf/txt Ladentexte 0x7d (txt_load_list), info Ladendaten
 * (13 Bytes struct shop_info: Aufschlag markup/markup_food in %, Art type SHOP_WEAPONS/HERBS/GENERAL, Stufe lvl, drei
 * Zusatzwaren extra, Haendlerin female), wares Warenliste (7 Bytes je Ware, mem_alloc(g_nr_items * 7)), sell_qty/
 * buy_qty Mengen fuer Verkauf/Kauf (O227/O228), pic_buf Puffer 0x240, grid_x/grid_y Raster.
 * Shop()/~Shop(): Konstruktor (Ladendaten aus g_market_shop beim Marktstand SHOP_MARKET, sonst Datensatz g_loc_id
 * aus Datei 0x1f3 per arc_*) und Destruktor (mem_free, g_shop = 0). leave: Ende. is_open: geschlossen (g_daytime vor
 * 8 bzw. nach 19 Uhr) oder Hausverbot g_shop_banned[Ort] -> Meldung, Rueckgabe 0. run: Hauptschleife - Bild/Name
 * (screen_redraw, anim_load Bild 0x82..0x87), Uhrzeit check_time, Menue dlg_choose (txt[1..5], txt[0x41]) -> g_action 0x81
 * reden talk, 0x82/0x83 shop_buy_screen/shop_sell_screen (+30 Minuten timewarp_minutes), 0x84 steal, 0x85/Esc Ende.
 * check_time(t): Ladenschluss -> Meldung txt[0x37], g_action = g_mouse_rclick = 0. talk: Gespraech - Bild je Ort
 * (conv_topic_on/conv_set_topic, conv_run). build_wares: Warenliste aufbauen (g_items-Eintraege ab Stufe info.lvl nach
 * Art; Waffen (weapon) ab Platz 0, Ruestungen ab 70, beide Teile per qsort(shop_compar) sortiert und
 * zusammengeschoben, Rest bis 100 leeren; Nahrung beim Kraemer 8..12 nur mit g_shop_sells_food). add_ware(id, platz,
 * slot): Ware eintragen, Preis = Grundpreis + Grundpreis * Aufschlag / 100 (mind. 1). format_money(buf, geld):
 * "%d.%02d"-Ausgabe txt[0x3d]. trade(preis, art): Handel abschliessen - Feilschen bargain (-99 = Hausverbot), Rabatt
 * clamp_long(r, 0, 30) %; Kauf: Waren per group_give_item verteilen, Nachfragezaehler g_item_demand (ab 10 ->
 *   items_change_price
 * und build_wares), Geld group_sub_money; Verkauf: Gegenstaende drop_item abgeben, Zaehler bis -10, Geld
 * group_add_money; Meldungen txt[0x11..0x1b]. steal: Diebstahl use_skill (-2 -> Hausverbot).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL226.CPP   -> 4248/4248 Bytes GLEICH (18.10.2026), cobjfix laeuft.
 *   Mit zusaetzlich -Oi ebenfalls GLEICH, ohne -Og 143 Bytes zu lang (Epilogkopien, Zwischenwerte).
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Erster Entwurf sofort GLEICH: -Og-Zwischenwerte erklaeren die Stack-Slots ohne Variable: build_wares [bp-6]
 *   (Offset &g_items[i], Segment je Zugriff nachgeladen) und [bp-3] (Flag-Byte fuer die Bittests), trade
 *   [bp-10h]/[bp-12h] (&g_item_demand[i] bzw. [id]), add_ware CX (idx * 7) und SI (&g_items[id]).
 * - "g_shop = 0" (far-Zeiger auf Klasse) -> mov dword; Zuweisung "= this" -> mov eax.
 * - "mem_alloc(g_nr_items * sizeof(struct ware))" -> imul / movzx eax (unsigned); "(signed long)g_loc_id * 13" ->
 *   movsx / imul eax,eax,0dh; konstante long-Argumente -> push dword (66 6a 0d).
 * - Funktionszeiger qsort(.., shop_compar) -> push seg / push 20h (Stub-Thunk).
 * - Verschachtelter ?: mit "female == 0 ? 0x86 : 0x87" (cmp byte / jnz); "female ? a : b" -> cmp / jz.
 * - "price + (signed long)price * pct / 100" mit int-Ziel -> nur 16-Bit-add des Quotienten.
 * - Textleiste: screen_textbar mit ?: zweier Texte (char far *, sonst push ds) und int-Farbe (push word) ueber
 *   gecasteten Funktionszeiger LOC_TEXTBAR.
 * Originalfehler/Auffaelliges: trade (Kauf) bricht ab, wenn nicht alle Waren Platz finden ("more"), zieht aber
 *   trotzdem den vollen Preis ab. build_wares: kommen mehr als 70 Waren ohne weapon zusammen, ueberschreibt der erste
 *   Teil den zweiten (ab Platz 70).
 */
#include <stdio.h>
#include <stdlib.h>
#include "schweif.h"

/* Ortsname in die Textleiste: Text als char far * (?: zweier Felder) und Farbe 0xff als int wie im Original */
#define LOC_TEXTBAR(t) ((void (*)(void far *, char far *, signed short))screen_textbar)(g_screen, t, 0xff)

Shop::Shop()
{
	g_shop = this;
	txt_load_list(g_font, 0x7d, 0x46, &txt_buf, &txt);
	if (g_loc_id == SHOP_MARKET)
		info = g_market_shop;
	else {
		arc_open(g_file_3, 0x1f3);
		arc_seek(g_file_3, (signed long)g_loc_id * 13, 0);
		arc_read(g_file_3, &info, 13L);
		arc_close(g_file_3);
	}
	wares = (struct ware far *)mem_alloc(g_nr_items * sizeof(struct ware));
	pic_buf = mem_alloc(0x240L);
	grid_x[0] = 0x1e;
	grid_x[1] = 0x5f;
	grid_x[2] = 0xa0;
	grid_y[0] = 0x23;
	grid_y[1] = 0x37;
	grid_y[2] = 0x4b;
	grid_y[3] = 0x5f;
	grid_y[4] = 0x73;
	g_refresh = 1;
	done = 0;
}

Shop::~Shop()
{
	mem_free(pic_buf);
	mem_free(wares);
	mem_free(txt_buf);
	mem_free(txt);
	g_shop = 0;
}

void Shop::leave()
{
	done = 1;
}

signed short Shop::is_open()
{
	register signed short ok;

	ok = 1;
	if (g_daytime < 0xa8c0L || g_daytime > 0x190c8L) {
		LOC_TEXTBAR(g_loc_id == SHOP_MARKET ? txt[0x33] : txt[g_loc_name]);
		dlg_message(g_dialog, txt[0]);
		ok = 0;
	}
	if (g_shop_banned[g_loc_id]) {
		LOC_TEXTBAR(g_loc_id == SHOP_MARKET ? txt[0x33] : txt[g_loc_name]);
		sprintf(g_text_output_buf, txt[0x34], info.female ? txt[0x36] : txt[0x35]);
		dlg_message(g_dialog, g_text_output_buf);
		ok = 0;
	}
	return ok;
}

void Shop::run()
{
	register signed short sel;

	while (done == 0) {
		if (g_refresh) {
			draw_loc_icons(g_screen, 5, 0x15, 0x16, 0x18, 0x19, 8);
			screen_redraw(g_screen);
			anim_load(g_screen_pic, info.type == SHOP_WEAPONS ? (info.female == 0 ? 0x86 : 0x87) :
				info.type == SHOP_HERBS ? (info.female == 0 ? 0x82 : 0x83) : (info.female == 0 ? 0x84 : 0x85), 0);
			LOC_TEXTBAR(g_loc_id == SHOP_MARKET ? txt[0x33] : txt[g_loc_name]);
			snd_play(g_snd_music, 0x18);
			g_refresh = 0;
		}
		check_time(g_daytime);
		handle_input(g_screen);
		if (g_mouse_rclick || g_key == 0x4900) {
			sel = dlg_choose(g_dialog, txt[1], 5, txt[4], txt[2], txt[3], txt[0x41], txt[5]) - 1;
			if (sel != -2)
				g_action = sel + 0x81;
		}
		if (g_action == 0x81)
			talk();
		else if (g_action == 0x82) {
			ani_stop(g_screen_pic);
			shop_buy_screen(this);
			timewarp_minutes(g_date, 30L);
		} else if (g_action == 0x83) {
			ani_stop(g_screen_pic);
			shop_sell_screen(this);
			timewarp_minutes(g_date, 30L);
		} else if (g_action == 0x84)
			steal();
		else if (g_action == 0x85 || g_key == 0x11b)
			done = 1;
		if (g_leave_location)
			done = 1;
	}
}

void Shop::check_time(signed long t)
{
	if (t > 0x190c8L || t < 0xa8c0L) {
		done = 1;
		sprintf(g_text_output_buf, txt[0x37], info.female ? txt[0x36] : txt[0x35]);
		dlg_message(g_dialog, g_text_output_buf);
		g_mouse_rclick = g_action = 0;
	}
}

void Shop::talk()
{
	register signed short pic;

	pic = -1;
	if (info.type == SHOP_WEAPONS)
		conv_topic_on(g_dialog2, 0x17);
	else if (info.type == SHOP_HERBS)
		conv_topic_on(g_dialog2, 0x16);
	else
		conv_topic_on(g_dialog2, 0x15);
	if (g_loc_id >= 1 && g_loc_id <= 2)
		pic = g_loc_id + 4;
	else if (g_loc_id >= 3 && g_loc_id <= 5)
		pic = g_loc_id + 0x0e;
	else if (g_loc_id == 6)
		pic = 0xa4;
	else if (g_loc_id == 7)
		pic = g_loc_id + 0x15;
	else if (g_loc_id >= 8 && g_loc_id <= 0x10)
		pic = g_loc_id + 0x36;
	else if (g_loc_id >= 0x11 && g_loc_id <= 0x14)
		pic = g_loc_id + 0x49;
	else if (g_loc_id >= 0x15 && g_loc_id <= 0x17)
		pic = g_loc_id + 0x55;
	else if (g_loc_id == SHOP_MARKET) {
		if (info.type == SHOP_WEAPONS) {
			if (info.female)
				pic = 0x98;
			else
				pic = 0x97;
		} else if (info.type == SHOP_HERBS) {
			if (info.female)
				pic = 0x9c;
			else
				pic = 0x9b;
		} else {
			if (info.female)
				pic = 0x9a;
			else
				pic = 0x99;
		}
	}
	if (pic != -1 && conv_run(g_dialog2, pic))
		timewarp_minutes(g_date, 30L);
	if (info.type == SHOP_WEAPONS)
		conv_set_topic(g_dialog2, TOPIC_WAFFEN, 0);
	else if (info.type == SHOP_HERBS)
		conv_set_topic(g_dialog2, TOPIC_KRAEUTER, 0);
	else
		conv_set_topic(g_dialog2, TOPIC_HANDEL, 0);
}

void Shop::build_wares()
{
	signed short idx2;
	register signed short i;
	register signed short idx;

	i = 1;
	idx = 0;
	idx2 = 0x46;
	count = 0;
	for (; g_items[i].gfx != -1; i++) {
		if (info.lvl <= g_items[i].f0b) {
			if (g_items[i].armor || g_items[i].weapon) {
				if (info.type == SHOP_WEAPONS) {
#if FIX_0085
					/* Fix #85: Waffen nur bis Platz 69 (ab 70 liegen die Ruestungen) */
					if (g_items[i].weapon) {
						if (idx < 70)
							add_ware(i, idx++, -1);
					}
#else
					if (g_items[i].weapon)
						add_ware(i, idx++, -1);
#endif
					else
						add_ware(i, idx2++, -1);
				}
			} else if (g_items[i].herb_potion) {
				if (info.type == SHOP_HERBS)
					add_ware(i, idx++, -1);
			} else if (info.type == SHOP_GENERAL) {
				if (g_loc_id >= 8 && g_loc_id <= 12 && g_items[i].food) {
					if (g_shop_sells_food[g_loc_id])
						add_ware(i, idx++, -1);
				} else
					add_ware(i, idx++, -1);
			}
		}
	}
	for (i = 0; i < 3; i++) {
		if (info.extra[i])
			add_ware(info.extra[i], idx++, -1);
	}
	if (info.type == SHOP_WEAPONS) {
		qsort(wares, idx, 7, shop_compar);
		qsort(wares + 70, idx2 - 70, 7, shop_compar);
		for (i = 0; i < idx2 - 70; i++)
			wares[idx + i] = wares[i + 70];
		for (i = idx + idx2 - 70; i < 100; i++)
			wares[i].id = 0;
	} else
		qsort(wares, idx, 7, shop_compar);
}

void Shop::add_ware(register signed short id, signed short idx, signed char f)
{
	wares[idx].id = id;
	wares[idx].slot = f;
	wares[idx].unit = g_items[id].price_unit;
	if (g_items[id].food)
		wares[idx].price = g_items[id].price + (signed long)g_items[id].price * info.markup_food / 100;
	else
		wares[idx].price = g_items[id].price + (signed long)g_items[id].price * info.markup / 100;
	if (wares[idx].price == 0)
		wares[idx].price = 1;
	count++;
}

void Shop::format_money(char far *buf, signed long m)
{
	sprintf(buf, txt[0x3d], m / 100, m % 100);
}

signed short Shop::trade(signed long price, signed char mode)
{
	signed short ret;
	signed short more;
	signed short nr_sel;
	signed short discount;
	signed short got;
	struct hero far *h;
	register signed short i;
	register signed short j;

	ret = 0;
	more = 0;
	nr_sel = 0;
	if (mode == 1) {
		for (i = 0; i < g_nr_items; i++) {
			if (buy_qty[i])
				nr_sel++;
		}
	} else {
		for (i = 0; i < 7; i++) {
			for (j = 0; j < 31; j++) {
				if (sell_qty[i][j])
					nr_sel++;
			}
		}
	}
	if (nr_sel != 0) {
		preselect_best_skill(g_group, 0x15);
		h = get_active_hero(g_group, txt[0x14]);
		if (h == 0) {
			ret = -1;
		} else {
			discount = bargain(h, nr_sel, price, 0, 0);
			if (discount == -99) {
				sprintf(g_text_output_buf, txt[0x16], h->name, info.female ? txt[0x36] : txt[0x35]);
				dlg_message(g_dialog, g_text_output_buf);
				g_shop_banned[g_loc_id] = 1;
				ret = 1;
			} else {
				discount = clamp_long(discount, 0L, 30L);
				if (mode == 1) {
					for (i = 0; i < g_nr_items; i++) {
						if (buy_qty[i]) {
#if FIX_0084
							/* Fix #84: Stueckpreis vor einer Preisaenderung (build_wares) merken */
							signed long unitp = 0;

							for (j = 0; j < g_nr_items && wares[j].id != i; j++)
								;
							if (j < g_nr_items)
								unitp = (signed long)wares[j].price * wares[j].unit;
							got = group_give_item(g_group, i, buy_qty[i]);
#else
							got = group_give_item(g_group, i, buy_qty[i]);
#endif
							g_item_demand[i] += got;
							if (g_item_demand[i] >= 10) {
								g_item_demand[i] = 0;
								items_change_price(&g_items, i, 10);
								build_wares();
							}
#if FIX_0084
							/* Fix #84: weiter verteilen, nicht gelieferte Ware nicht berechnen */
							if (buy_qty[i] > got) {
								more = 1;
								price -= unitp * (buy_qty[i] - got);
								if (price < 0)
									price = 0;
							}
#else
							if (buy_qty[i] > got) {
								more = 1;
								break;
							}
#endif
						}
					}
					price -= price * discount / 100;
					group_sub_money(g_group, price);
					money_to_string(g_text_buf2, price);
					if (discount > 0)
						sprintf(g_text_output_buf, txt[0x12], h->name, discount, g_text_buf2);
					else
						sprintf(g_text_output_buf, txt[0x11], h->name, g_text_buf2);
					dlg_message(g_dialog, g_text_output_buf);
					if (more)
						dlg_message(g_dialog, txt[0x13]);
				} else {
					for (i = 0; i < 7; i++) {
						for (j = 0; j < 31; j++) {
							signed short amount;
							struct hero far *p;
							signed short id;

							amount = sell_qty[i][j];
							if (amount != 0) {
								p = g_group->heroes[i];
								if (i == 6)
									p = group_get_npc(g_group);
								id = p->inv[j].item_id;
								g_item_demand[id] -= amount;
								if (g_item_demand[id] <= -10) {
									g_item_demand[id] = 0;
									items_change_price(&g_items, id, -10);
								}
								drop_item(p, j, amount);
							}
						}
					}
					price += price * discount / 100;
					group_add_money(g_group, price);
					money_to_string(g_text_buf2, price);
					if (discount > 0)
						sprintf(g_text_output_buf, txt[0x1b], h->name, discount, g_text_buf2);
					else
						sprintf(g_text_output_buf, txt[0x1a], h->name, g_text_buf2);
					dlg_message(g_dialog, g_text_output_buf);
				}
			}
		}
	}
	return ret;
}

void Shop::steal()
{
	struct hero far *h;

	h = get_active_hero(g_group, txt[0x42]);
	if (h != 0) {
		if (use_skill(h, 0, -1) == -2) {
			done = 1;
			g_shop_banned[g_loc_id] = 1;
		}
		timewarp_minutes(g_date, 30L);
	}
}
