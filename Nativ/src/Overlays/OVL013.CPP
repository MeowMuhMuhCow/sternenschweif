/* OVL013 - Overlay O013 (Segment 0x1f42 in SCHWEIF.EXE): Dialog-Sonderfunktionen (struct dlg wie O101; setzen
 * meist den Folgezustand d->next, Texte aus d->ptr[]). f_0000/f_012d: Geldsumme der Gruppe (O077_072A) bzw.
 * Gegenstand 0x2d (Anzahl +1aa, beim ersten Stapel 2 behalten, O085_1158) -> next 10/6/7/8 je nach Menge und
 * Besitz von 0x13f. f_025d(d, k): Belohnung O202_00A6 (Gruppenwert O201_0127 * 20/2, 100, 10/1; Flags
 * ds:279a/2799), k == 3: Gegenstand 0x13f, Text 13, mit ds:3998 Zwischensequenz (Musik, O034/O024, f_12e5 mit
 * Tastenrueckruf f_11ac). f_0470/f_09ff/f_0a65/f_0b3e: Bedingungen -> next. f_049d(d, s): Name des ersten
 * Helden mit +33 == 6 nach s (strcpy). f_0527/f_0571/f_098e: Gegenstand 0x133, Texte 65/66/69, ds:25c6.
 * f_05b2/f_06bb: Heldenwahl f_0761 (O190_01C6, Namen in Stackkopien der DS-Vorlagen ds:0638/0698) bzw. f_08f9
 * (O202_01BC/O074_01CA, O029_0339/074A, O202_0200) nach ds:281b/282b, Text 67/68 per O103_04E1. f_0a85: ab 100
 * Geld (+2f) O088_03DC mit O233-Objekt und O235_0E1F. f_0b6f(d, w): Gegenstaende mit Bit 1 (Tabelle *ds:b84a,
 * 14 Bytes) mit Wahrscheinlichkeit w verlieren (mindestens einer). f_0ca0/f_0f87: O018_0C4D-Ergebnis, Bilder
 * 0xb1/0xb2/0xb4. f_0d26: Gegenstand 0x14b/0x14c tauschen (O083_02EA, Texte 23..26), Rueckgabe 0/1/2.
 * f_0fec(d, k): Preisstufe ds:260c (3..5 nach Geld / -2) und Vergleich mit dem Geld. f_1067(d, k): Werte
 * ds:c812..c822 je Stufe 1..3. f_10fb(d, n): Text d->ptr[n] mit eigenen Textfarben/-fenster (R20, ds:37e4).
 * f_11ac(key, limit): Tastenrueckruf (Strg-Q, Taste 6/0x3a -> Sprache 0x413/0x414 und Text 16/17). f_12e5(n, f,
 * d): O024_0010(n, -1, f_11ac, 1), Strg-Q beendet das Spiel (R03_028C, exit).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL013.CPP   -> 4934/4934 Bytes GLEICH (18.10.2026), cobjfix laeuft
 * - Ohne -Og 1981 Abweichungen (Epilogkopien in den else-if-Ketten). Kein -Oi: strcpy/memset sind Aufrufe.
 * - C++ wegen inline hname() (f_0d26: Parameterkopien [bp-0ah]/[bp-0eh]), new/delete fuer O008 (Tmp) und Bmp,
 *   lokales O233-Objekt im Block (Destruktor am Blockende).
 * - Funktionszeiger f_11ac in dasselbe Overlay: Original "push seg OVL013_TEXT / push 0ach" = Stub-Thunk
 *   0x20 + 5 * 28 (f_11ac ist das 29. Public); im C einfach f_11ac.
 * - f_0761: initialisierte lokale Arrays (char[6][16], 6 Heldenzeiger) als Strukturkopien "struct names6 names =
 *   g_0638;" (rep movsw, 0x30 bzw. 0x0c Worte).
 * - f_012d: id und first als Block-Locals in der do-Schleife ([bp-0ch]/[bp-0eh], hinter dem -Og-Offset [bp-0ah]).
 * - f_0fec: r und m OHNE register -> r in CX, m in SI (mit register m tauscht BCC). "g_260c -= 2" -> add al,0feh.
 * - f_1067: "default: break;" hinter dem letzten case (mit break) -> default springt auf den echten Epilog.
 * - f_0ca0: "case 2: d->done = 1; default: break;" (eigener jmp short vor dem Ende).
 * Auffaellig (Original): f_025d gibt den hohen Bonus (* 20 bzw. * 10) nur, wenn ds:279a/ds:2799 schon gesetzt
 *   ist, und setzt das Flag dann (erneut) auf 1 - vermutlich war "if (!flag)" gemeint; kein anderes Overlay
 *   schreibt ds:279a/2799, der Bonus haengt also nur vom Anfangswert/Spielstand ab. f_049d/f_08f9/f_0b6f/f_0fec:
 *   Parameter d unbenutzt.
 * (Namensfassung mit schweif.h, struct talk_desc in s_struct.h: 19.10.2026 GLEICH, LINKGLEICH) Inv/Cast als Klassen
 *   aus s_class.h (new Inv -> hero_sheet_loop), hname(h) -> hero::get_name(); Vorlagen ds:0638/ds:0698 als
 *   g_talk_tpl.names/.heroes (kein eigenes Public).
 */
#include <stdio.h>
#include <string.h>
#include <mem.h>
#include <bios.h>
#include <stdlib.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:0630..06b4, dsana) - initialisierte Daten in Adressreihenfolge */
signed short g_cutscene_pics[3] = { -15341, -15340, -1 };
struct talk_tpl g_talk_tpl = { 0, { { "", "", "", "", "", "" } }, { { 0, 0, 0, 0, 0, 0 } } };
signed short g_cutscene_sub1 = 0;
signed short g_cutscene_sub2 = 0;
/* Phase B: eigenes BSS (ds:8b56..8b5c) in Adressreihenfolge */
signed short g_cutscene_key;
struct talk_desc far *g_cutscene_talk;


void talk_hand_over_weapons(struct talk_desc far *d)
{
	struct hero far *h;
	signed long sum;
	signed short has;
	register signed short i;
	register signed short id;

	h = get_first_hero(g_group);
	sum = 0;
	has = 0;
	do {
		sum += hero_take_weapons(h);
		h = get_next_hero(g_group, h);
	} while (get_first_hero(g_group) != h);
	do {
		for (i = 0; i < NR_INV_SLOTS; i++) {
			id = h->inv[i].item_id;
			if (id == ITEM_MAGISTRATE_DOC)
				has = 1;
		}
		h = get_next_hero(g_group, h);
	} while (get_first_hero(g_group) != h);
	if (sum == 0)
		d->next = 10;
	else if (sum < 2000)
		d->next = 6;
	else if (sum < 10000 || has != 0)
		d->next = 7;
	else
		d->next = 8;
}

void talk_hand_over_items(struct talk_desc far *d)
{
	struct hero far *h;
	signed short total;
	signed short has;
	register signed short n;
	register signed short i;

	h = get_first_hero(g_group);
	total = 0;
	has = 0;
	do {
		signed short first = 0;
		signed short id;

		for (i = INV_SLOT_KNAPSACK; i < NR_INV_SLOTS; i++) {
			id = h->inv[i].item_id;
			if (id == ITEM_MAGISTRATE_DOC)
				has = 1;
			if (id == ITEM_FOOD_PACKAGE) {
				n = h->inv[i].quantity;
				if (first == 0) {
					first = 1;
					if (n > 2) {
						n -= 2;
						drop_item(h, i, n);
						total += n;
					}
				} else {
					drop_item(h, i, n);
					total += n;
				}
			}
		}
		h = get_next_hero(g_group, h);
	} while (get_first_hero(g_group) != h);
	if (total == 0)
		d->next = 10;
	else if (total < 50)
		d->next = 6;
	else if (total < 200 || has != 0)
		d->next = 7;
	else
		d->next = 8;
}

void talk_reward(struct talk_desc far *d, signed short k)
{
	signed short old;

	switch (k) {
	case 2:
#if FIX_0126
		/* Fix #126: hoher Bonus nur beim ersten Mal */
		if (!g_reward2_given) {
#else
		if (g_reward2_given) {
#endif
			group_add_ap(g_group, group_count_alive(g_group) * 20);
			g_reward2_given = 1;
		} else
			group_add_ap(g_group, group_count_alive(g_group) * 2);
		group_add_money(g_group, 25000L);
		break;
	case 3:
		group_add_ap(g_group, group_count_alive(g_group) * 100);
		group_give_item_each(g_group, ITEM_MAGISTRATE_DOC, 1);
		diary_add(g_diary, d->txt[13], 0);
		if (g_speech_ext_avail != 0) {
			old = g_music_id_req & 0x3fff;
			if (g_cd_music != 0)
				snd_obj_stop(g_snd_music);
			if (g_cd_speech)
				load_pic_list(g_cd_cache, g_cutscene_pics, &g_talk_tpl.pics_end, 1, 0);
			g_pp20_index = -1;
			mouse_hide(0);
			pal_blend(0, 0x19);
			fli_play_frames(0x0d, 1, 0);
			pal_blend_from_dark(g_fli_pal, 0x19);
			if (!talk_cutscene(0x0d, 1, d)) {
				fli_play_file(0x0e, 0);
				pal_blend(g_fli_pal, 0x19);
			}
			load_palette_arc3();
			mouse_show(0);
			g_refresh = 1;
			snd_play(g_snd_music, old);
		}
		break;
	default:
#if FIX_0126
		/* Fix #126: hoher Bonus nur beim ersten Mal */
		if (!g_reward_given) {
#else
		if (g_reward_given) {
#endif
			group_add_ap(g_group, group_count_alive(g_group) * 10);
			g_reward_given = 1;
		} else
			group_add_ap(g_group, group_count_alive(g_group));
		break;
	}
}

void talk_check_dwarf(struct talk_desc far *d)
{
	if (group_has_dwarf(g_group))
		d->next = 0x0d;
	else
		d->next = 0x10;
}

void talk_get_dwarf_name(struct talk_desc far *d, char far *s)
{
	struct hero far *h;

	h = get_first_hero(g_group);
	do {
		if (check_hero3(h) && h->type == HERO_TYPE_DWARF)
			break;
		h = get_next_hero(g_group, h);
	} while (get_first_hero(g_group) != h);
	strcpy(s, h->name);
}

void talk_give_lowangen_key(struct talk_desc far *d)
{
#if FIX_0375
	/* Fix #375: Schluessel bei vollem Rucksack ueber die Gruppe geben (fragt nach Ablegen); Zustand nur bei Erfolg, sonst bietet Eolan ihn erneut an */
	if (give_hero_new_item(get_first_active_hero(g_group), ITEM_LOWANGEN_KEY, 1, 1) > 0 ||
	    group_give_item(g_group, ITEM_LOWANGEN_KEY, 1) > 0) {
		g_eolan_state = 2;
		diary_add(g_diary, d->txt[65], 0);
	}
#else
	give_hero_new_item(get_first_active_hero(g_group), ITEM_LOWANGEN_KEY, 1, 1);
	g_eolan_state = 2;
	diary_add(g_diary, d->txt[65], 0);
#endif
}

void talk_rejoin_leave(struct talk_desc far *d)
{
	talk_rejoin_heroes(d);
	diary_add(g_diary, d->txt[66], 0);
	g_eolan_state = 3;
	group_leave_place(0);
}

void talk_take_heroes(struct talk_desc far *d)
{
	Inv far *o;

	if (dlg_yes_no(g_dialog, d->txt[72]) == 1) {
		o = new Inv;
		hero_sheet_loop(o, get_first_hero(g_group));
		delete o;
		g_pp20_index = -1;
		g_refresh = 1;
		screen_redraw(g_screen);
	}
	talk_choose_hero(d, 0, g_absent_hero1);
	talk_choose_hero(d, 1, g_absent_hero2);
	if (g_pp20_index == 4)
		screen_redraw(g_screen);
	load_portrait_buf(0xac);
	sprintf(g_text_output_buf, d->txt[67], g_absent_hero1, g_absent_hero2);
	diary_add(g_diary, g_text_output_buf, 0);
}

void talk_rejoin_heroes(struct talk_desc far *d)
{
	talk_rejoin_hero(d, g_absent_hero1);
	talk_rejoin_hero(d, g_absent_hero2);
	if (g_pp20_index == 4)
		screen_redraw(g_screen);
	load_portrait_buf(0xac);
	sprintf(g_text_output_buf, d->txt[68], g_absent_hero1, g_absent_hero2);
	diary_add(g_diary, g_text_output_buf, 0);
	memset(g_absent_hero1, 0, 16);
	memset(g_absent_hero2, 0, 16);
}

void talk_choose_hero(struct talk_desc far *d, signed short mode, char far *dst)
{
	signed short sel;
	signed short n = 0;
	struct hero far *h;
	struct names6 names = g_talk_tpl.names;
	struct heroes6 hs = g_talk_tpl.heroes;

	h = get_first_hero(g_group);
	do {
		if (check_hero3(h) && !h->npc_id) {
			strcpy(names.n[n], h->name);
			hs.h[n] = h;
			n++;
		}
		h = get_next_hero(g_group, h);
	} while (get_first_hero(g_group) != h);
	do {
		sel = dlg_choose(g_dialog, mode == 0 ? d->txt[38] : d->txt[39], n, names.n[0], names.n[1], names.n[2],
			names.n[3], names.n[4], names.n[5]) - 1;
	} while (sel < 0);
	strcpy(dst, hs.h[sel]->file_name);
	sel = group_hero_pos(g_group, hs.h[sel]);
	group_set_slot(g_group, sel, 0x63);
}

void talk_rejoin_hero(struct talk_desc far *d, char far *s)
{
	signed short pos;
	struct hero far *h;
	register signed short b;
	register signed short a;

	a = group_free_slot(g_group);
	b = get_free_hero_slot();
	if (a != -1 && b != -1 && b != 6) {
		h = g_heroes + b;
		herofile_find(s, &pos, 0xfb);
		if (pos != -1 && hero_load_chr(0xfb, pos, b, g_group_no))
			group_add_hero(g_group, h, a);
	}
}

void talk_npc_leaves(struct talk_desc far *d)
{
	g_npc_months = 0x7f;
	groups_check_npc();
	diary_add(g_diary, d->txt[69], 0);
	talk_rejoin_heroes(d);
	group_add_ap(g_group, group_count_alive(g_group) * 120);
}

void talk_game_won(struct talk_desc far *d)
{
	g_game_state = 1;
}

void talk_nop(void)
{
}

void talk_check_group_size(struct talk_desc far *d)
{
	struct hero far *h;
	register signed short n;

	n = group_count_check3(g_group);
	h = group_get_npc(g_group);
	if (h != 0 && check_hero3(h))
		n--;
	if (n < 3)
		d->next = 0x19;
	else
		d->next = 0x1a;
}

void talk_check_quest_anim(struct talk_desc far *d)
{
	if (g_quest_anim_seen)
		d->next = 4;
	else
		d->next = 6;
}

void talk_cast_analues(struct talk_desc far *d)
{
	struct hero far *h;

	if (g_group->money >= 100) {
		h = get_active_mage(g_group, d->txt[10]);
		if (h != 0) {
			g_spell_by_item = 1;
			{
				Cast o(h);

				spell_analues(&o);
				if (*g_text_output_buf == 0)
					d->next = 2;
			}
		} else
			d->next = 2;
	} else {
		dlg_message(g_dialog, g_ttx[4]);
		d->next = 2;
	}
}

void talk_check_parcel(struct talk_desc far *d)
{
	if (group_find_item(g_group, ITEM_PARCEL) != -1)
		d->next = 4;
	else
		d->next = 1;
}

void talk_lose_items(struct talk_desc far *d, signed short chance)
{
	struct hero far *h;
	signed short first;
	register signed short i;
	register signed short id;

	h = get_first_hero(g_group);
	first = 1;
	if (chance == 0) {
		do {
			hero_take_weapons(h);
			h = get_next_hero(g_group, h);
		} while (get_first_hero(g_group) != h);
	} else {
		do {
			for (i = INV_SLOT_KNAPSACK; i < NR_INV_SLOTS; i++) {
				id = h->inv[i].item_id;
				if (id != 0) {
					if (g_items[id].weapon) {
						if (random_schweif(100) <= chance || first != 0) {
							if (!g_items[id].undropable) {
								drop_item(h, i, 1);
								first = 0;
							}
						}
					}
				}
			}
			h = get_next_hero(g_group, h);
		} while (get_first_hero(g_group) != h);
	}
}

void talk_fight_95(struct talk_desc far *d)
{
	register signed short r = 0;

	r = start_fight(g_low_gate1_open ? 0x95 : 0x96, 0);
	switch (r) {
	case 0:
	case 3:
		dlg_message(g_dialog, d->txt[13]);
		break;
	case 2:
		d->done = 1;
	default:
		break;
	}
	if (g_low_gate1_open)
		load_portrait_buf(0xb1);
	else if (g_low_gate2_open)
		load_portrait_buf(0xb2);
}

signed short talk_hand_over_document(struct talk_desc far *d)
{
	struct hero far *h;
	signed short slot;
	register signed short r = 0;
	register signed short id;

	if (g_group->money >= 100) {
		h = get_active_hero(g_group, d->txt[22]);
		if (h != 0) {
			slot = select_item(h, g_ttx[240], 1);
			if (slot != -1) {
				id = h->inv[slot].item_id;
				if (id == ITEM_DOCUMENT_A) {
					sprintf(g_text_output_buf, d->txt[25], get_first_active_hero(g_group)->get_name(), d->txt[23]);
					dlg_message(g_dialog, g_text_output_buf);
					sprintf(g_text_output_buf, d->txt[26], d->txt[23]);
					diary_add(g_diary, g_text_output_buf, 0);
					r = 1;
					group_sub_money(g_group, 100L);
				} else if (id == ITEM_DOCUMENT_B) {
					sprintf(g_text_output_buf, d->txt[25], get_first_active_hero(g_group)->get_name(), d->txt[24]);
					dlg_message(g_dialog, g_text_output_buf);
					sprintf(g_text_output_buf, d->txt[26], d->txt[24]);
					diary_add(g_diary, g_text_output_buf, 0);
					r = 1;
					group_sub_money(g_group, 100L);
				} else if (g_items[id].table_index != 3) {
					r = 2;
					group_sub_money(g_group, 100L);
				}
			}
		}
	} else
		dlg_message(g_dialog, g_ttx[4]);
	return r;
}

signed short talk_fight_97(struct talk_desc far *d, signed short v)
{
	register signed short r = 0;

	if (v == 0)
		r = start_fight(0x98, 2);
	else
		r = start_fight(0x97, 1);
	load_portrait_buf(0xb4);
	diary_add(g_diary, d->txt[14], 0);
	return r == 0 ? 1 : 0;
}

signed short talk_price(struct talk_desc far *d, signed short k)
{
	signed short r = 0;
	signed short m;

	switch (k) {
	case 0:
		m = g_group->money / 100;
		g_price_level = m >= 500 ? 5 : m >= 400 ? 4 : 3;
		break;
	case 1:
		g_price_level -= 2;
		break;
	case 2:
		break;
	}
	if ((signed long)(signed short)g_price_level * 10000 <= g_group->money)
		r = 1;
	return r;
}

void talk_set_peddler(struct talk_desc far *d, register signed short k)
{
	g_peddler = k;
	switch (k) {
	case 1:
		g_peddler_stock = 15;
		g_peddler_markup = 200;
		g_peddler_markup_food = 900;
		g_peddler_type = 3;
		g_peddler_lvl = 5;
		g_peddler_female = 1;
		break;
	case 2:
		g_peddler_stock = 10;
		g_peddler_markup = 300;
		g_peddler_markup_food = 1400;
		g_peddler_type = 3;
		g_peddler_lvl = 8;
		g_peddler_female = 1;
		break;
	case 3:
		g_peddler_stock = 10;
		g_peddler_markup = 150;
		g_peddler_markup_food = 0;
		g_peddler_type = 1;
		g_peddler_lvl = 7;
		g_peddler_female = 0;
		break;
	default:
		break;
	}
}

void talk_nop2(struct talk_desc far *d)
{
}

void talk_nop3(struct talk_desc far *d)
{
}

void talk_subtitle(struct talk_desc far *d, signed short idx)
{
	signed short x0;
	signed short y0;
	signed short x1;
	signed short y1;
	signed short fg;
	signed short bg;

	font_get_box(g_font, &x0, &y0, &x1, &y1);
	font_get_colors(g_font, &fg, &bg);
	font_set_box(g_font, 5, 0x136, 0xaf, 1);
	font_set_colors(g_font, 0x81, -1);
	font_print_wrapped(g_font, d->txt[idx]);
	font_set_box(g_font, x0, y0, x1, y1);
	((void (*)(void far *, signed short, signed short))font_set_colors)(g_font, fg, bg);
}

signed short cutscene_key(register signed short key, unsigned short limit)
{
	Bmp far *b;

	if (bioskey(1)) {
		g_cutscene_key = bioskey(0);
		if (g_cutscene_key == KEY_CTRL_Q)
			return 1;
	}
	if (key == 6 && g_cutscene_sub1 == 0) {
		g_cutscene_sub1 = 1;
		if (g_speech_on == 1 && speech_active(g_snd_speech))
			snd_speech(g_snd_speech, 0x413);
		if (g_subtitles)
			talk_subtitle(g_cutscene_talk, 0x10);
	} else if (key == 0x3a && g_cutscene_sub2 == 0) {
		g_cutscene_sub2 = 1;
		b = new Bmp(0x140, 0x1c, (char far *)g_vga_buf + 0xd700);
		b->clear();
		delete b;
		if (g_speech_on == 1 && speech_active(g_snd_speech))
			snd_speech(g_snd_speech, 0x414);
		if (g_subtitles)
			talk_subtitle(g_cutscene_talk, 0x11);
	} else if ((unsigned short)key >= limit) {
		g_cutscene_sub1 = g_cutscene_sub2 = 0;
		return 1;
	}
	return 0;
}

signed short talk_cutscene(signed short n, signed short f, struct talk_desc far *d)
{
	register signed short r;

	g_cutscene_talk = d;
	g_cutscene_key = 0;
	r = fli_play(n, -1, (fli_cb)cutscene_key, 1);
	if (f)
		fli_clear_screen();
	if (g_cutscene_key == KEY_CTRL_Q) {
		sys_exit(0);
		exit(0);
	}
	return r != 0 ? r : g_cutscene_key;
}
