/* OVL239 - Overlay O239 (Segment 0x243c in SCHWEIF.EXE): Zufallsereignisse in der Stadt (gerufen aus O046
 * town_random_event je nach Stadt/Tageszeit; bis N13 town_rnd_<adresse>, Namen nach dem Ablauf - ohne die Texte 0x36d
 * vermutet). Rahmen jedes Ereignisses: town_rnd_open(bild, groesse) = swap_out_screen, g_trnd_pic = new Bmp(n, n),
 * Texte 0x36d (0xc8 Stueck) nach g_trnd_txt_buf/g_trnd_txt, Bild 0x36c/bild per nvfile_load, Fensterbreite 6 setzen
 * (alte in g_trnd_old_width); dann Texte/Fragen ueber dlg_box_bmp bzw. dlg_message; zum Schluss town_rnd_close =
 * Fensterbreite zurueck, Texte freigeben, delete g_trnd_pic.
 * Ereignisse: tallest_hero groesster Held (height) verliert 1 LE; magic_user/mage_arrest zauberkundiger Held
 * (get_hero_of_type in den Reihenfolgen Magier, Druide, Hexe, Wald-, Firn-, Auelf bzw. Wald-, Au-, Firnelf,
 * Magier, Hexe, Druide; mage_arrest in Stadt 4: group_imprison); best_weapon wertvollste Waffe der Gruppe (Preis
 * * Einheit, ohne 0x85/0x8c, nicht stapelbar); item83_arrest Frage mit 3 Antworten (alle Gegenstaende 0x83
 * abgeben, group_imprison); dwarf_test Zwerg, Mutproben, zwei Ungeschickteste verlieren LE und group_scatter;
 * gift_7e Gegenstand 0x7e; extortion verschachtelte Fragen (1000 Silber zahlen, sonst LE des Charismaschwaechsten
 * auf 4); lured_away Held mit dem hoechsten Aberglauben geht verloren (group_set_slot); donation/
 * donation_amount/paid_service/offer_1000/trader_800 Bezahl-Ereignisse (g_god_favor[1..14] -= 3 / ++,
 * group_sub_money, Betrag per dlg_input); twice_0c zweistufig ueber g_trnd_twice_state; once_01/once_02 einmalig
 * ueber g_trnd_once1_done/g_trnd_once2_done; find_money group_add_money(10); find_item_0e/trader_800 Gegenstand
 * per group_give_item; note_NN nur Text (NN = Bild).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y -E..\bc\BIN\TASM.EXE OVL239.CPP -> 10008/10008 Bytes GLEICH (18.10.2026),
 * cobjfix laeuft (ohne -Og 10022 Bytes).
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Aufruf-/sprintf-Folgen per Stapelmaschine aus der ASM erzeugt (wovbc/g239.py), Kontrollfluss von Hand.
 * - C++ wegen new Bmp(n, n) (gb_new_alloc(0, n, n)) und delete g_trnd_pic (gb_delete(p, 3) ohne Nulltest).
 * - dlg_box_bmp ohne Titel: (char far *)0 (frueher als zwei int-Nullen im variablen Teil, gleiche Bytes).
 * - h->name mehrfach in einem Ausdruck -> -Og-Zwischenwert (mov [bp-x],ax bzw. mov si,ax in mage_arrest),
 *   entsteht von selbst. In dwarf_test liegen alle Locals in "if (nr_heroes >= 3) {" (Zwischenwert [bp-2]
 *   davor), Cast (signed short)g_group->nr_heroes noetig (cbw / cmp ax,3).
 * - best_weapon: "id = h->inv[i].item_id; if (id != 0)" getrennt (mov si / or si,si); Flagbyte [bp-15h] und
 *   &g_items[id] in DI sind -Og-Zwischenwerte; "(signed long)price * price_unit" -> movsx / push eax / cbw /
 *   movsx / pop edx / imul.
 * - donation: k = random_schweif(3) - 1 muss als LETZTE Variable deklariert sein (sonst bekommt k DI statt r);
 *   "case 2: default:" teilen sich einen Rumpf; g_trnd_txt[k + 19] rechnet Index zuerst (mov es / add bx).
 * - paid_service: Rueckgabe r einmal benutzt -> [bp-2]; "v == 0 ? 7 : 8" -> or si,si / jnz.
 * Auffaelliges im Original: donation zieht die 100 Silber (group_sub_money) auch ab, wenn die Gruppe weniger hat
 *   (nur die Ansehenserhoehung prueft money >= 100). dwarf_test: pa/pb bleiben 0, wenn weniger als zwei andere
 *   Helden einsatzfaehig sind (sprintf/sub_hero_le mit Nullzeiger). best_weapon rechnet val nicht je Gegenstand
 *   neu (veralteter Wert bleibt stehen, wirkt sich aber nicht aus).
 */
#include <stdio.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:75c8..75d2, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
signed short g_trnd_old_width = 0;
/* Phase B: eigenes BSS (ds:cb4e..cb5a) in Adressreihenfolge */
void far *g_trnd_txt_buf;
char far * far *g_trnd_txt;
BMP_PTR g_trnd_pic;

#define TRND_TXT	0x36d		/* Texte der Zufallsereignisse (0xc8 Stueck) */
#define TRND_TXT_N	0xc8
#define TRND_PICS	0x36c		/* NVF-Datei der Ereignisbilder */


void town_rnd_tallest_hero(void)
{
	struct hero far *h;
	struct hero far *best;
	signed short max;
	signed short lvl;

	h = get_first_hero(g_group);
	best = get_first_hero(g_group);
	max = 0;
	town_rnd_open(0x15, 0x40);
	do {
		if (check_hero3(h) && (lvl = h->height) > max) {
			max = lvl;
			best = h;
		}
		h = get_next_hero(g_group, h);
	} while (get_first_hero(g_group) != h);
	sprintf(g_text_output_buf, g_trnd_txt[128], best->name);
	dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_text_output_buf, 0);
	sprintf(g_text_output_buf, g_trnd_txt[129], hero_name_gen(best, 1), best->name, grammar_pronoun(best->sex, 0, 0), hero_name_gen(best, 1));
	dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_text_output_buf, 0);
	sub_hero_le(best, 1);
	sprintf(g_text_output_buf, g_trnd_txt[130], best->name, grammar_pronoun(best->sex, 0, 0), grammar_pronoun(best->sex, 0, 0), hero_name_gen(best, 1));
	dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_text_output_buf, 0);
	sprintf(g_text_output_buf, g_trnd_txt[131], best->name, grammar_pronoun(best->sex, 0, 0));
	dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_text_output_buf, 0);
	town_rnd_close();
}

void town_rnd_magic_user(void)
{
	struct hero far *h;

	h = get_hero_of_type(g_group, HERO_TYPE_MAGE);
	if (h == 0)
		h = get_hero_of_type(g_group, HERO_TYPE_DRUID);
	if (h == 0)
		h = get_hero_of_type(g_group, HERO_TYPE_WITCH);
	if (h == 0)
		h = get_hero_of_type(g_group, HERO_TYPE_SYLVAN_ELF);
	if (h == 0)
		h = get_hero_of_type(g_group, HERO_TYPE_ICE_ELF);
	if (h == 0)
		h = get_hero_of_type(g_group, HERO_TYPE_GREEN_ELF);
	if (h != 0) {
		town_rnd_open(0x19, 0x40);
		sprintf(g_text_output_buf, g_trnd_txt[132], h->name);
		dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_text_output_buf, 0);
		sprintf(g_text_output_buf, g_trnd_txt[133], h->name, grammar_pronoun(h->sex, 0, 0), grammar_pronoun(h->sex, 0, 0));
		dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_text_output_buf, 0);
		town_rnd_close();
	}
}

void town_rnd_best_weapon(void)
{
	struct hero far *h;
	struct hero far *best;
	signed long max;
	signed long val;
	signed short bestid;
	signed short i;
	signed short id;

	h = get_first_hero(g_group);
	best = 0;
	max = 0;
	val = 0;
	do {
		for (i = 0; i < 31; i++) {
			id = h->inv[i].item_id;
			if (id != 0) {
				if (g_items[id].weapon && id != 0x85 && id != 0x8c && !g_items[id].stackable)
					val = (signed long)g_items[id].price * g_items[id].price_unit;
			}
			if (val > max) {
				max = val;
				best = h;
				bestid = id;
			}
		}
		h = get_next_hero(g_group, h);
	} while (get_first_hero(g_group) != h);
	if (best != 0) {
		town_rnd_open(0x18, 0x40);
		sprintf(g_text_output_buf, g_trnd_txt[134], best->name, grammar_article(0x8002, bestid, 0));
		dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_text_output_buf, 0);
		town_rnd_close();
	}
}

void town_rnd_note_17(void)
{
	town_rnd_open(0x17, 0x40);
	dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_trnd_txt[135], 0);
	town_rnd_close();
}

void town_rnd_item83_arrest(void)
{
	signed short r;
	signed short f;

	f = 0;
	town_rnd_open(0x1b, 0x40);
	do {
		r = dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_trnd_txt[106], 3, g_trnd_txt[107], g_trnd_txt[108], g_trnd_txt[109]);
	} while (r == -1);
	switch (r) {
	case 1:
		dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_trnd_txt[110], 0);
		break;
	case 2:
		dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_trnd_txt[111], 0);
		f = 1;
		break;
	case 3:
		dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_trnd_txt[112], 0);
		f = 1;
		break;
	}
	if (f) {
		while (group_find_item(g_group, 0x83) != -1)
			group_drop_item(g_group, 0x83);
		group_imprison(1);
	}
	town_rnd_close();
}

void town_rnd_dwarf_test(void)
{
	signed short cnt;
	signed short v;

	if ((signed short)g_group->nr_heroes >= 3) {
		struct hero far *sel;
		struct hero far *h;
		signed short a;
		signed short b;
		struct hero far *pa;
		struct hero far *pb;

		sel = 0;
		h = get_first_hero(g_group);
		do {
			if (check_hero3(h) && h->type == HERO_TYPE_DWARF && h->sex == 0) {
				sel = h;
				break;
			}
			h = get_next_hero(g_group, h);
		} while (get_first_hero(g_group) != h);
		if (sel != 0) {
			cnt = 0;
			town_rnd_open(0x10, 0x40);
			sprintf(g_text_output_buf, g_trnd_txt[69], sel->name);
			dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_text_output_buf, 0);
			h = get_first_hero(g_group);
			do {
				if (check_hero3(h)) {
					if (test_attrib(h, ATTRIB_MU, 0) > 0)
						cnt++;
					else
						cnt--;
				}
				h = get_next_hero(g_group, h);
			} while (get_first_hero(g_group) != h);
			if (cnt <= 0) {
				a = 99;
				b = 99;
				pa = 0;
				pb = 0;
				h = get_first_hero(g_group);
				do {
					if (h != sel && check_hero3(h)) {
						v = test_attrib(h, ATTRIB_GE, 3);
						if (v < a) {
							if (v < b) {
								a = b;
								pa = pb;
								b = v;
								pb = h;
							} else {
								a = v;
								pa = h;
							}
						}
					}
					h = get_next_hero(g_group, h);
				} while (get_first_hero(g_group) != h);
#if FIX_0206
				/* Fix #206: nur vorhandene Helden treffen (kein Schreiben ueber den Nullzeiger) */
				if (pa != 0 && pb != 0) {
					sprintf(g_text_output_buf, g_trnd_txt[70], sel->name, pa->name, pb->name);
					dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_text_output_buf, 0);
					sub_hero_le(pa, random_schweif(4) + 1);
					sub_hero_le(pb, random_schweif(4) + 1);
				} else if (pb != 0)
					sub_hero_le(pb, random_schweif(4) + 1);
#else
				sprintf(g_text_output_buf, g_trnd_txt[70], sel->name, pa->name, pb->name);
				dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_text_output_buf, 0);
				sub_hero_le(pa, random_schweif(4) + 1);
				sub_hero_le(pb, random_schweif(4) + 1);
#endif
				group_scatter(1, 0);
			} else {
				h = get_hero_attrib_max(g_group, ATTRIB_IN);
				if (test_attrib(h, ATTRIB_IN, 2) <= 0) {
					sprintf(g_text_output_buf, g_trnd_txt[71], sel->name, sel->name);
					dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_text_output_buf, 0);
					group_scatter(3, 0);
				} else {
					sprintf(g_text_output_buf, g_trnd_txt[72], h->name);
					dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_text_output_buf, 0);
					sprintf(g_text_output_buf, g_trnd_txt[73], h->name, sel->name, grammar_pronoun(h->sex, 2, 0));
					dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_text_output_buf, 0);
				}
			}
			town_rnd_close();
		}
	}
}

void town_rnd_gift_7e(void)
{
	struct hero far *h;

	town_rnd_open(0x13, 0x40);
	if (dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_trnd_txt[84], 2, g_trnd_txt[85], g_trnd_txt[86]) == 1) {
		h = get_first_active_hero(g_group);
		sprintf(g_text_output_buf, g_trnd_txt[87], h->name, grammar_pronoun(h->sex, 2, 0));
		dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_text_output_buf, 0);
		timewarp_hours(g_date, 1L);
		dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_trnd_txt[88], 0);
		sprintf(g_text_output_buf, g_trnd_txt[89], h->name);
		dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_text_output_buf, 0);
		group_give_item(g_group, 0x7e, 1);
	}
	town_rnd_close();
}

void town_rnd_extortion(void)
{
	signed short r;
	signed short f;
	struct hero far *h;

	f = 0;
	town_rnd_open(0x16, 0x40);
	do {
		r = dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_trnd_txt[90], 2, g_trnd_txt[91], g_trnd_txt[92]);
	} while (r == -1);
	switch (r) {
	case 1:
		do {
			r = dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_trnd_txt[93], 3, g_trnd_txt[94], g_trnd_txt[95], g_trnd_txt[96]);
		} while (r == -1);
		switch (r) {
		case 1:
			f = 1;
			break;
		case 2:
			do {
				r = dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_trnd_txt[98], 2, g_trnd_txt[99], g_trnd_txt[100]);
			} while (r == -1);
			if (r == 1) {
				if (g_group->money >= 1000) {
					dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_trnd_txt[102], 0);
					group_sub_money(g_group, 1000L);
				} else {
					dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_trnd_txt[103], 0);
					group_sub_money(g_group, g_group->money);
				}
			} else
				f = 1;
			break;
		case 3:
			do {
				r = dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_trnd_txt[101], 2, g_trnd_txt[99], g_trnd_txt[100]);
			} while (r == -1);
			if (r == 1) {
				if (g_group->money >= 1000) {
					dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_trnd_txt[102], 0);
					group_sub_money(g_group, 1000L);
				} else {
					dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_trnd_txt[103], 0);
					group_sub_money(g_group, g_group->money);
				}
			} else
				f = 1;
			break;
		}
		break;
	case 2:
		h = get_hero_attrib_min(g_group, ATTRIB_CH);
		sprintf(g_text_output_buf, g_trnd_txt[104], h->name, h->name, grammar_pronoun(h->sex, 2, 0), grammar_pronoun(h->sex, 0, 0));
		dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_text_output_buf, 0);
		sub_hero_le(h, h->le - 4);
		break;
	}
	if (f) {
		h = get_hero_attrib_min(g_group, ATTRIB_CH);
		sprintf(g_text_output_buf, g_trnd_txt[97], h->name, grammar_pronoun(h->sex, 2, 0), grammar_pronoun(h->sex, 0, 0));
		dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_text_output_buf, 0);
		sub_hero_le(h, h->le - 4);
	}
	town_rnd_close();
}

void town_rnd_note_1a(void)
{
	town_rnd_open(0x1a, 0x40);
	dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_trnd_txt[105], 0);
	town_rnd_close();
}

void town_rnd_choice_11(void)
{
	town_rnd_open(0x11, 0x40);
	dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_trnd_txt[74], 0);
	if (dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_trnd_txt[75], 2, g_trnd_txt[76], g_trnd_txt[77]) == 1) {
		dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_trnd_txt[78], 0);
		dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_trnd_txt[79], 0);
	} else {
		dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_trnd_txt[80], 0);
		dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_trnd_txt[81], 0);
	}
	town_rnd_close();
}

void town_rnd_find_item_0e(void)
{
	struct hero far *h;

	h = get_random_hero(g_group);
	town_rnd_open(0x12, 0x40);
	sprintf(g_text_output_buf, g_trnd_txt[82], h->name, grammar_pronoun(h->sex, 3, 0), h->name, grammar_pronoun(h->sex, 0, 0));
	dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_text_output_buf, 0);
	dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_trnd_txt[83], 0);
	group_give_item(g_group, 0xe, 1);
	town_rnd_close();
}

void town_rnd_note_00(void)
{
	town_rnd_open(0, 0x40);
	dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_trnd_txt[0], 0);
	town_rnd_close();
}

void town_rnd_once_01(void)
{
	if (!g_trnd_once1_done) {
		town_rnd_open(1, 0x40);
		dlg_box_bmp(g_dialog, g_trnd_pic, g_trnd_txt[54], g_trnd_txt[1], 0);
		town_rnd_close();
		g_trnd_once1_done = 1;
	}
}

void town_rnd_once_02(void)
{
	signed short r;
	struct hero far *h;

	if (!g_trnd_once2_done) {
		town_rnd_open(2, 0x40);
		r = dlg_box_bmp(g_dialog, g_trnd_pic, g_trnd_txt[55], g_trnd_txt[2], 2, g_trnd_txt[3], g_trnd_txt[4]);
		if (r == 2) {
			r = dlg_box_bmp(g_dialog, g_trnd_pic, g_trnd_txt[55], g_trnd_txt[8], 2, g_trnd_txt[9], g_trnd_txt[10]);
			if (r == 2) {
				dlg_box_bmp(g_dialog, g_trnd_pic, g_trnd_txt[55], g_trnd_txt[12], 0);
				g_trnd_once2_done = 1;
			} else
				dlg_box_bmp(g_dialog, g_trnd_pic, g_trnd_txt[55], g_trnd_txt[11], 0);
		} else {
			h = get_random_hero(g_group);
			sprintf(g_text_output_buf, g_trnd_txt[5], h->name, h->sex == 0 ? g_trnd_txt[6] : g_trnd_txt[7]);
			dlg_box_bmp(g_dialog, g_trnd_pic, g_trnd_txt[55], g_text_output_buf, 0);
		}
		town_rnd_close();
	}
}

void town_rnd_lured_away(void)
{
	struct hero far *h;
	struct hero far *h2;

	if (group_count_check2b(g_group) >= 2) {
		town_rnd_open(0x14, 0x40);
		if (dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_trnd_txt[117], 2, g_trnd_txt[118], g_trnd_txt[119]) == 1) {
			h = get_hero_attrib_max(g_group, ATTRIB_AG);
			timewarp_minutes(g_date, 5L);
			sprintf(g_text_output_buf, g_trnd_txt[120], h->name);
			sprintf(g_text_buf2, g_trnd_txt[122], h->name);
			if (dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_text_output_buf, 2, g_trnd_txt[121], g_text_buf2) == 1) {
				timewarp_minutes(g_date, 5L);
				sprintf(g_text_output_buf, g_trnd_txt[123], h->name);
				sprintf(g_text_buf2, g_trnd_txt[125], h->name);
				if (dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_text_output_buf, 2, g_trnd_txt[124], g_text_buf2) == 1) {
					h2 = get_hero_attrib_max(g_group, ATTRIB_IN);
					if (h2 == h)
						h2 = get_first_hero(g_group);
					if (h2 == h)
						h2 = get_second_active_hero(g_group);
					timewarp_minutes(g_date, 30L);
					sprintf(g_text_output_buf, g_trnd_txt[126], h->name, grammar_pronoun(h->sex, 0, 0));
					dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_text_output_buf, 0);
#if FIX_0363
					/* Fix #363: der letzte Held der Gruppe geht nicht verloren; ohne zweiten Helden keine Probe ueber den Nullzeiger */
					if (h2 != 0 && h2 != h && test_attrib(h2, ATTRIB_IN, 3) > 0) {
						sprintf(g_text_output_buf, g_trnd_txt[127], h2->name, h->name);
						dlg_message(g_dialog, g_text_output_buf);
					} else if ((signed char)g_group->nr_heroes - (group_get_npc(g_group) != 0) > 1)
						group_set_slot(g_group, group_hero_pos(g_group, h), -1);
#else
					if (test_attrib(h2, ATTRIB_IN, 3) > 0) {
						sprintf(g_text_output_buf, g_trnd_txt[127], h2->name, h->name);
						dlg_message(g_dialog, g_text_output_buf);
					} else
						group_set_slot(g_group, group_hero_pos(g_group, h), -1);
#endif
				}
			}
		}
		town_rnd_close();
	}
	g_refresh_portraits = 1;
}

void town_rnd_donation(signed short v)
{
	char far *title;
	char far *text;
	char far *what;
	signed long money;
	signed short i;
	signed short r;
	signed short k = random_schweif(3) - 1;

	switch (v) {
	case 0:
		town_rnd_open(3, 0x40);
		what = g_trnd_txt[22];
		title = g_trnd_txt[54];
		text = g_trnd_txt[13];
		break;
	case 1:
		town_rnd_open(4, 0x40);
		what = g_trnd_txt[23];
		title = g_trnd_txt[56];
		text = g_trnd_txt[24];
		break;
	case 2:
	default:
		town_rnd_open(5, 0x40);
		what = g_trnd_txt[22];
		title = g_trnd_txt[57];
		text = g_trnd_txt[25];
		break;
	}
	r = dlg_box_bmp(g_dialog, g_trnd_pic, title, text, 2, g_ttx[446], g_ttx[447]);
	if (r != 1 && k == 2)
		r = dlg_box_bmp(g_dialog, g_trnd_pic, title, g_trnd_txt[65], 2, g_ttx[446], g_ttx[447]);
	if (r != 1) {
		for (i = 1; i <= 14; i++)
			g_god_favor[i] -= 3;
		sprintf(g_text_output_buf, g_trnd_txt[k + 19], what);
		dlg_box_bmp(g_dialog, g_trnd_pic, title, g_text_output_buf, 0);
	} else {
		money = g_group->money;
#if FIX_0206
		/* Fix #206: Spende nur abziehen, wenn das Geld reicht */
		if (money >= 100) {
			for (i = 1; i <= 14; i++)
				g_god_favor[i]++;
			group_sub_money(g_group, 100L);
		}
#else
		if (money >= 100)
			for (i = 1; i <= 14; i++)
				g_god_favor[i]++;
		group_sub_money(g_group, 100L);
#endif
		dlg_box_bmp(g_dialog, g_trnd_pic, title, g_trnd_txt[k + 16], 0);
	}
	town_rnd_close();
}

void town_rnd_donation_amount(void)
{
	signed long n;
	signed short i;
	signed short r;

	town_rnd_open(6, 0x40);
	r = dlg_box_bmp(g_dialog, g_trnd_pic, g_trnd_txt[58], g_trnd_txt[26], 2, g_ttx[446], g_ttx[447]);
	if (r == 2 || r == -1) {
		for (i = 1; i <= 14; i++)
			g_god_favor[i] -= 3;
		dlg_box_bmp(g_dialog, g_trnd_pic, g_trnd_txt[58], g_trnd_txt[27], 0);
	} else {
		n = dlg_input(g_dialog, 2, g_trnd_txt[28], 0, 99, g_group->money < 100 ? g_group->money / 10 : 10);
		n *= 10;
		if (n != 0)
			for (i = 1; i <= 14; i++)
				g_god_favor[i]++;
		group_sub_money(g_group, n);
		if (n < 100)
			sprintf(g_text_output_buf, "%s%s", g_trnd_txt[29], g_trnd_txt[30]);
		else
			sprintf(g_text_output_buf, "%s", g_trnd_txt[30]);
		dlg_box_bmp(g_dialog, g_trnd_pic, g_trnd_txt[58], g_text_output_buf, 0);
	}
	town_rnd_close();
}

void town_rnd_paid_service(signed short v)
{
	signed short r;
	signed short cost;

	cost = group_count_check3(g_group);
	if ((signed long)cost * 100 < g_group->money) {
		town_rnd_open(v == 0 ? 7 : 8, 0x40);
		r = dlg_box_bmp(g_dialog, g_trnd_pic, v == 0 ? g_trnd_txt[59] : g_trnd_txt[60], g_trnd_txt[31], 2, g_ttx[446], g_ttx[447]);
		if (r == 1) {
			group_sub_money(g_group, (signed long)cost * 100);
			dlg_message(g_dialog, v == 0 ? g_trnd_txt[14] : g_trnd_txt[15]);
			timewarp_minutes(g_date, (signed long)cost * 30);
			if (random_schweif(10) <= 3)
				group_get_ill(g_group, 2, 0x1e);
		}
		town_rnd_close();
	}
}

void town_rnd_offer_1000(void)
{
	signed short r;

	town_rnd_open(9, 0x40);
	r = dlg_box_bmp(g_dialog, g_trnd_pic, g_trnd_txt[61], g_trnd_txt[32], 2, g_ttx[446], g_ttx[447]);
	if (r == 1) {
		r = dlg_box_bmp(g_dialog, g_trnd_pic, g_trnd_txt[61], g_trnd_txt[33], 2, g_ttx[446], g_ttx[447]);
		if (r == 1) {
			if (g_group->money >= 1000) {
				dlg_box_bmp(g_dialog, g_trnd_pic, g_trnd_txt[61], g_trnd_txt[66], 0);
				group_sub_money(g_group, 1000L);
			} else
				dlg_box_bmp(g_dialog, g_trnd_pic, g_trnd_txt[61], g_trnd_txt[67], 0);
		}
	}
	town_rnd_close();
}

void town_rnd_find_money(void)
{
	struct hero far *h;

	h = get_random_hero(g_group);
	town_rnd_open(0xa, 0x40);
	sprintf(g_text_output_buf, g_trnd_txt[34], h->name);
	dlg_box_bmp(g_dialog, g_trnd_pic, g_trnd_txt[62], g_text_output_buf, 0);
	group_add_money(g_group, 10L);
	town_rnd_close();
}

void town_rnd_meeting_0b(void)
{
	struct hero far *h;

	h = get_random_hero(g_group);
	town_rnd_open(0xb, 0x40);
	sprintf(g_text_output_buf, g_trnd_txt[35], h->name, h->sex == 0 ? g_trnd_txt[36] : g_trnd_txt[37]);
	dlg_box_bmp(g_dialog, g_trnd_pic, g_trnd_txt[63], g_text_output_buf, 0);
	dlg_box_bmp(g_dialog, g_trnd_pic, g_trnd_txt[63], g_trnd_txt[38], 0);
	town_rnd_close();
}

void town_rnd_twice_0c(void)
{
	struct hero far *h;

	town_rnd_open(0xc, 0x40);
	if (!g_trnd_twice_state) {
		h = get_random_hero(g_group);
		sprintf(g_text_output_buf, g_trnd_txt[39], h->name);
		dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_text_output_buf, 0);
		g_trnd_twice_state++;
	} else if (g_trnd_twice_state == 1) {
		dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_trnd_txt[40], 0);
		g_trnd_twice_state++;
	}
	town_rnd_close();
}

void town_rnd_meeting_0d(void)
{
	struct hero far *h;

	town_rnd_open(0xd, 0x40);
	h = get_random_hero(g_group);
	sprintf(g_text_output_buf, g_trnd_txt[41], h->name);
	dlg_box_bmp(g_dialog, g_trnd_pic, g_trnd_txt[64], g_text_output_buf, 0);
	town_rnd_close();
}

void town_rnd_note_0e(void)
{
	town_rnd_open(0xe, 0x40);
	dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_trnd_txt[42], 0);
	town_rnd_close();
}

void town_rnd_mage_arrest(void)
{
	struct hero far *h;
	signed short r;

	h = get_hero_of_type(g_group, HERO_TYPE_SYLVAN_ELF);
	if (h == 0)
		h = get_hero_of_type(g_group, HERO_TYPE_GREEN_ELF);
	if (h == 0)
		h = get_hero_of_type(g_group, HERO_TYPE_ICE_ELF);
	if (h == 0)
		h = get_hero_of_type(g_group, HERO_TYPE_MAGE);
	if (h == 0)
		h = get_hero_of_type(g_group, HERO_TYPE_WITCH);
	if (h == 0)
		h = get_hero_of_type(g_group, HERO_TYPE_DRUID);
	if (h != 0) {
		town_rnd_open(0xf, 0x50);
		g_dlg_box_flag = 0;
		sprintf(g_text_output_buf, g_trnd_txt[43], h->name, h->sex == 0 ? g_trnd_txt[44] : g_trnd_txt[45], h->name);
		r = dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_text_output_buf, 2, g_trnd_txt[46], g_trnd_txt[47]);
		if (r == 1) {
			sprintf(g_text_output_buf, g_trnd_txt[48], h->sex == 0 ? g_trnd_txt[49] : g_trnd_txt[50]);
			r = dlg_choose(g_dialog, g_text_output_buf, 2, g_trnd_txt[51], g_trnd_txt[52]);
			if (r == 2) {
				if (g_group->town != 4) {
					sprintf(g_text_output_buf, g_trnd_txt[53], h->name, h->name);
					dlg_message(g_dialog, g_text_output_buf);
				} else {
					sprintf(g_text_output_buf, g_trnd_txt[68], h->name);
					dlg_message(g_dialog, g_text_output_buf);
					group_imprison(1);
				}
			}
		}
		town_rnd_close();
	}
}

void town_rnd_trader_800(void)
{
	struct hero far *h;
	signed short r;

	h = get_hero_attrib_max(g_group, ATTRIB_AG);
	town_rnd_open(0x1c, 0x40);
	sprintf(g_text_output_buf, g_trnd_txt[113], h->name);
	do {
		r = dlg_box_bmp(g_dialog, g_trnd_pic, (char far *)0, g_text_output_buf, 2, g_ttx[446], g_ttx[447]);
	} while (r == -1);
	switch (r) {
	case 1:
		if (g_group->money >= 800) {
			group_sub_money(g_group, 800L);
			dlg_message(g_dialog, g_trnd_txt[115]);
			group_give_item(g_group, random_schweif(100) <= 50 ? 0xbc : 0xa3, 1);
		} else
			dlg_message(g_dialog, g_trnd_txt[116]);
		break;
	case 2:
		dlg_message(g_dialog, g_trnd_txt[114]);
		break;
	}
	town_rnd_close();
}

void town_rnd_open(signed short pic, signed short size)
{
	signed short dummy;

	swap_out_screen();
	g_trnd_pic = new Bmp(size, size);
	txt_load_list(g_font, TRND_TXT, TRND_TXT_N, &g_trnd_txt_buf, &g_trnd_txt);
	nvfile_load(g_nvf_heads, TRND_PICS, g_trnd_pic->data(), 0, pic, 0, &dummy, &dummy, 0x140);
	g_trnd_old_width = dlg_set16(g_dialog, 6);
}

void town_rnd_close(void)
{
	dlg_set16(g_dialog, g_trnd_old_width);
	mem_free(g_trnd_txt_buf);
	mem_free(g_trnd_txt);
	delete g_trnd_pic;
}
