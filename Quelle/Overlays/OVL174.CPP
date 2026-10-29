/* OVL174 - Overlay O174 (Segment 0x22c0 in SCHWEIF.EXE): Heiler LOC_HEALER - class Healer (s_class.h, 0x0e Bytes).
 * Healer(): Texte 0x19 (0x23 Stueck) per txt_load_list nach txt_buf/txt, Datensatz g_loc_id (5 Bytes struct
 * healer_info: Preis-% markup, Koennen skill, Art kind, Versteinerung stone) aus Datei 0x1ff (g_file_3, R06) nach
 * info. ~Healer(), is_open liefert 1. run: Menue (draw_loc_icons mit 5 bzw. 6 Eintraegen, Bild 0x28/0x29, Klang
 * 2) - beim ersten Mal ausserhalb 0x7e90..0x1baf8 Ticks (g_daytime, Nacht) Hinweis und doppelter Preis (state 2);
 * Auswahl ueber dlg_choose, 0x81 Gespraech talk (conv_run je g_loc_id, 30 Minuten), 0x82 cure_wounds (LE
 * auffuellen, Preis 50 je le_mod und 5 je fehlendem LE-Punkt), 0x83 cure_disease und 0x84 cure_poison (Krankheit/
 * Gift per hero_is_diseased/hero_is_poisoned, Preis g_disease_prices * 10 bzw. g_poison_prices * 20, Erfolg W100
 * <= 120 - skill * 10 (* 5) + g_disease_delays/g_poison_delays, sonst halber Preis), 0x85 cure_stone
 * (Versteinerung petrified loesen, 5000, Erfolg info.stone < W20). price: + markup %, nachts doppelt.
 * Versteinerte/tote Helden werden abgewiesen (txt[31]/txt[15]).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL174.CPP   -> 4090/4090 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - -Og: Zwischenwerte [bp-9] (Byte +bc) und [bp-0ch] (Bit petrified als int) fuer die Statustests; Epilogkopie
 *   nach dem "return;" am Ende des Nicht-versteinert-Zweigs (der versteinert-Zweig steht danach).
 * - Heldenstatus +bc als unsigned short-Bitfeld; Test "!(signed char)h->petrified" -> shr/and/cbw/or ax,ax;
 *   "(signed char)h->petrified" in cure_stone -> or al,al; inline h->is_healable() { return dead || petrified ? 0 :
 *   1; } ergibt die Reihenfolge jnz/jz -> xor ax,ax / mov ax,1 (mit !(dead || ..) kommt mov ax,1 zuerst).
 * - set_ill_diagnosed(k)/set_poi_diagnosed(k) als inline-Elementfunktion (Parameterkopie [bp-0ah]).
 * - cure_wounds: "cost = (signed long)((k = h->le_mod) * 50)" (Zuweisung eingebettet, sonst mov ax,si
 *   nachgeladen); diff im innersten Block (liegt hinter den -Og-Zwischenwerten bei [bp-0eh]).
 * - price: "cost * info.markup / 100" -> movsx/push eax/mov eax,cost/pop edx/imul; mit "(long)pct * cost"
 *   imul eax,mem.
 * - draw_loc_icons und dlg_choose sind variadisch (Menueeintraege), sprintf-Argumente in Textreihenfolge.
 * - screen_textbar mit int-Farbe (push word) ueber gecasteten Funktionszeiger LOC_TEXTBAR.
 * Auffaelliges: cure_disease/cure_poison setzen das Merkbit diagnosed schon vor der Rueckfrage und Bezahlung; die
 *   Versteinerungsmeldung nutzt in cure_wounds grammar_pronoun(sex), in cure_disease/cure_poison g_ttx[0xfd/0xfe].
 *   Die alten Namen poison()/disease() waren vertauscht (Krankheit: illness, g_disease_prices, hero_cure_disease).
 */
#include <stdio.h>
#include "schweif.h"

/* Ortsname in die Textleiste: Farbe 0xff als int wie im Original */
#define LOC_TEXTBAR(t) ((void (*)(void far *, char far *, signed short))screen_textbar)(g_screen, t, 0xff)

Healer::Healer()
{
	txt_load_list(g_font, 0x19, 0x23, &txt_buf, &txt);
	g_refresh = 1;
	state = 0;
	arc_open(g_file_3, 0x1ff);
	arc_seek(g_file_3, (signed long)g_loc_id * 5, 0);
	arc_read(g_file_3, &info, 5L);
	arc_close(g_file_3);
}

Healer::~Healer()
{
	mem_free(txt_buf);
	mem_free(txt);
}

signed short Healer::is_open()
{
	return 1;
}

void Healer::run()
{
	signed short done = 0;
	signed short redraw = 1;
	register signed short n;
	register signed short sel;

	while (!done) {
		if (g_refresh) {
			n = info.stone ? 6 : 5;
			if (n == 5)
				draw_loc_icons(g_screen, n, 0x15, 0x1e, 0x1f, 0x20, 8);
			else
				draw_loc_icons(g_screen, n, 0x15, 0x1e, 0x1f, 0x20, 0x3b, 8);
			screen_redraw(g_screen);
			anim_load(g_screen_pic, info.kind == 1 ? 0x28 : 0x29, 0);
			snd_play(g_snd_music, 2);
			LOC_TEXTBAR(txt[g_loc_name]);
			g_refresh = redraw = 0;
			if (!state) {
				state = 1;
				if (g_daytime > 0x1baf8L || g_daytime < 0x7e90L) {	/* nach 21 bzw. vor 6 Uhr */
					dlg_message(g_dialog, txt[0]);
					state = 2;
				}
			}
		}
		if (redraw) {
			LOC_TEXTBAR(txt[g_loc_name]);
			redraw = 0;
		}
		handle_input(g_screen);
		if (g_mouse_rclick || g_key == 0x4900) {
			if (n == 5)
				sel = dlg_choose(g_dialog, txt[5], n, txt[24], txt[1], txt[2], txt[3], txt[4]) - 1;
			else
				sel = dlg_choose(g_dialog, txt[5], n, txt[24], txt[1], txt[2], txt[3], txt[25], txt[4]) - 1;
			if (sel != -2)
				g_action = sel + 0x81;
		}
		if (g_action == 0x81)
			talk();
		else if (g_action == 0x82)
			cure_wounds();
		else if (g_action == 0x83)
			cure_disease();
		else if (g_action == 0x84)
			cure_poison();
		else if (g_action == 0x85 && n == 6)
			cure_stone();
		else if (n + 0x80 == g_action || g_key == 0x11b)
			done = 1;
		if (g_leave_location)
			done = 1;
	}
}

void Healer::cure_wounds()
{
	struct hero far *h;
	signed long cost;
	register signed short k;

	h = get_any_hero(g_group, txt[6]);
	if (h != 0) {
		if (!(signed char)h->petrified) {
			if (h->is_healable()) {
				if (h->le >= h->le_max && (k = h->le_mod) == 0) {
					sprintf(g_text_output_buf, txt[7], h->name);
					dlg_message(g_dialog, g_text_output_buf);
				} else {
					cost = (signed long)((k = h->le_mod) * 50);
					cost += (signed long)((h->le_max + k - h->le) * 5);
					cost = price(cost);
					sprintf(g_text_output_buf, txt[10], hero_name_gen(h, 1), cost);
					if (dlg_yes_no(g_dialog, g_text_output_buf) == 1) {
						cost *= 10;
						if (g_group->money < cost) {
							dlg_message(g_dialog, g_ttx[4]);
						} else {
							signed short diff = h->le_max - h->le;
							group_sub_money(g_group, cost);
							hero_restore_le_mod(h);
							add_hero_le(h, h->le_max);
							g_refresh_portraits = 1;
							sprintf(g_text_output_buf, txt[30], hero_name_gen(h, 1));
							dlg_message(g_dialog, g_text_output_buf);
							timewarp_minutes(g_date, (signed long)diff);
						}
					}
				}
			} else {
				dlg_message(g_dialog, txt[15]);
			}
			return;
		}
		sprintf(g_text_output_buf, txt[31], h->name, grammar_pronoun(h->sex, 0, 0));
		dlg_message(g_dialog, g_text_output_buf);
	}
}

void Healer::cure_disease()
{
	struct hero far *h;
	signed long cost;
	register signed short k;

	h = get_any_hero(g_group, txt[6]);
	if (h != 0) {
		if (!(signed char)h->petrified) {
			if (h->is_healable()) {
				k = hero_is_diseased(h);
				if (k == 0) {
					sprintf(g_text_output_buf, txt[8], h->name);
					dlg_message(g_dialog, g_text_output_buf);
				} else {
					h->set_ill_diagnosed(k);
					cost = (signed long)(g_disease_prices[k] * 10);
					cost = price(cost);
					sprintf(g_text_output_buf, txt[11], h->name, g_ttx[k + 43], cost);
					if (dlg_yes_no(g_dialog, g_text_output_buf) == 1) {
						cost *= 10;
						if (g_group->money < cost) {
							dlg_message(g_dialog, g_ttx[4]);
						} else {
							if (random_schweif(100) <= 120 - info.skill * 10 + g_disease_delays[k]) {
								hero_cure_disease(h);
								g_refresh_portraits = 1;
								sprintf(g_text_output_buf, txt[13], h->name);
								dlg_message(g_dialog, g_text_output_buf);
								timewarp_hours(g_date, 1L);
							} else {
								dlg_message(g_dialog, txt[14]);
								timewarp_hours(g_date, 1L);
								cost >>= 1;
							}
							group_sub_money(g_group, cost);
						}
					}
				}
			} else {
				dlg_message(g_dialog, txt[15]);
			}
			return;
		}
		sprintf(g_text_output_buf, txt[31], h->name, h->sex ? g_ttx[0xfe] : g_ttx[0xfd]);
		dlg_message(g_dialog, g_text_output_buf);
	}
}

void Healer::cure_poison()
{
	struct hero far *h;
	signed long cost;
	register signed short k;

	h = get_any_hero(g_group, txt[6]);
	if (h != 0) {
		if (!(signed char)h->petrified) {
			if (h->is_healable()) {
				k = hero_is_poisoned(h);
				if (k == 0) {
					sprintf(g_text_output_buf, txt[9], h->name);
					dlg_message(g_dialog, g_text_output_buf);
				} else {
					h->set_poi_diagnosed(k);
					cost = (signed long)(g_poison_prices[k] * 20);
					cost = price(cost);
					sprintf(g_text_output_buf, txt[12], cost, h->name);
					if (dlg_yes_no(g_dialog, g_text_output_buf) == 1) {
						cost *= 10;
						if (g_group->money < cost) {
							dlg_message(g_dialog, g_ttx[4]);
						} else {
							if (random_schweif(100) <= 120 - info.skill * 5 + g_poison_delays[k]) {
								hero_cure_poison(h);
								g_refresh_portraits = 1;
								sprintf(g_text_output_buf, txt[28], h->name);
								dlg_message(g_dialog, g_text_output_buf);
								timewarp_hours(g_date, 1L);
							} else {
								dlg_message(g_dialog, txt[14]);
								timewarp_hours(g_date, 1L);
								cost >>= 1;
							}
							group_sub_money(g_group, cost);
						}
					}
				}
			} else {
				dlg_message(g_dialog, txt[15]);
			}
			return;
		}
		sprintf(g_text_output_buf, txt[31], h->name, h->sex ? g_ttx[0xfe] : g_ttx[0xfd]);
		dlg_message(g_dialog, g_text_output_buf);
	}
}

signed long Healer::price(signed long cost)
{
	cost += cost * info.markup / 100;
	if (state == 2)
		cost *= 2;
	return cost;
}

void Healer::cure_stone()
{
	struct hero far *h;
	signed long cost;

	h = get_any_hero(g_group, txt[6]);
	if (h != 0) {
		cost = 5000;
		cost = price(cost);
		if (!(signed char)h->dead) {
			if ((signed char)h->petrified) {
				sprintf(g_text_output_buf, txt[26], cost / 10, h->name);
				if (dlg_yes_no(g_dialog, g_text_output_buf) == 1) {
					if (g_group->money < cost) {
						dlg_message(g_dialog, g_ttx[4]);
					} else {
						if (info.stone < random_schweif(20)) {
							h->petrified = 0;
							sprintf(g_text_output_buf, txt[29], h->name);
							dlg_message(g_dialog, g_text_output_buf);
							timewarp_hours(g_date, 1L);
						} else {
							dlg_message(g_dialog, txt[14]);
							timewarp_hours(g_date, 1L);
							cost >>= 1;
						}
						group_sub_money(g_group, cost);
					}
				}
			} else {
				sprintf(g_text_output_buf, txt[27], h->name);
				dlg_message(g_dialog, g_text_output_buf);
			}
			return;
		}
		dlg_message(g_dialog, txt[15]);
	}
}

void Healer::talk()
{
	register signed short script = -1;

	conv_topic_on(g_dialog2, 0x16);
	conv_topic_on(g_dialog2, 0x18);
	conv_topic_on(g_dialog2, 0x19);
	if (g_loc_id == 1)
		script = 4;
	else if (g_loc_id == 2)
		script = 0x10;
	else if (g_loc_id == 3)
		script = 0x1b;
	else if (g_loc_id >= 4 && g_loc_id <= 6)
		script = g_loc_id + 0x37;
	else if (g_loc_id == 7)
		script = 0x59;
	else if (g_loc_id == 8)
		script = 0x69;
	if (script != -1 && conv_run(g_dialog2, script))
		timewarp_minutes(g_date, 0x1eL);
	conv_set_topic(g_dialog2, TOPIC_KRAEUTER, 0);
	conv_set_topic(g_dialog2, TOPIC_KRANKHEITEN, 0);
	conv_set_topic(g_dialog2, TOPIC_GIFTE, 0);
}
