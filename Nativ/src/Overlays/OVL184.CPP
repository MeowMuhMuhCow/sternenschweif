/* OVL184 - Overlay O184 (Segment 0x22f3 in SCHWEIF.EXE): Dialog-/Ereignisfunktionen auf einem Textobjekt o
 * (+0 Puffer, +4 Textzeiger txt[], geladen per O248_02E3(ds:37e4, 0x21c, 0x78, &o->m0, &o->txt)), Antworten ueber
 * O190_01C6(ds:b59a, Frage, n, ...). f_0000: Fragen txt[1..9]; bei Antwort 1 bis zu dreimal Gegenstand 0x2d
 * (R02_0C99/R02_043D/O085_1158), fuer fehlende (3 - i) * 10 per O200_016A abziehen; danach ggf. txt[10], Texte neu
 * laden nach O018_0C4D(0xa0, 0) (ds:395f = 2), txt[11]; ds:25c4 = 1. f_0230: Frage txt[13..15] -> Texte 16..19,
 * ds:25c5 = 1, Ergebnis Frage txt[20..22] == 1; sonst Bild (lokales R42-Objekt 0x20x0x20 aus ds:c4fb) mit
 * O190_037F-Texten 23..26. f_03ff: O101_0000(ds:c918, 0x90, 2/3/4, 0) je nach aktivem Helden (+9b == 5, Bit
 * +bc.0), ds:2794 = 0. f_0479: Bild 0x197 per O098_00D1 in ein neues R42-Objekt (new/delete), Texte 86/87, Klang
 * ds:38b6 0x69. f_0560(o, v): Probe O076_0A65(h, 13, v) fuer alle Helden (Ring O088_01DD/09FF): Anzahl Nullen,
 * 99 bei -1. f_05f7/f_0778: Fragen txt[49..52] bzw. txt[56..58], sonst Texte neu laden nach O018_0C4D(0xa1/0xa2)
 * und switch (0 / 2 / default) mit O034_0AE4/O034_080D. f_08c3: Frage txt[78..80] -> R21_0001(ds:b856, 43200),
 * O202_070B(Gruppe, 0), ds:3994 = ds:25d7 = 0.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL184.CPP   -> 2369/2369 Bytes GLEICH (18.10.2026), cobjfix laeuft
 * - C++ wegen der R42-Bildklasse Bmp: lokales Objekt mit Konstruktor R42_004B(w, h, p) und automatischem
 *   Destruktor R42_03B3(.., 2); "new Bmp(0x40, 0x40)" -> R42_00D8(0, 0x40, 0x40), "delete b" -> R42_03B3(b, 3);
 *   b->data() = R42_072D. Bitfeld +bc.0 als unsigned char b:1 (and ax,1 / or al,al).
 * - -Og noetig: Epilogkopien statt jmp zum Funktionsende (f_03ff, f_05f7, f_0778, switch-break).
 * - Locals in Deklarationsreihenfolge von [bp-2] abwaerts (f_0000: old, r, i, h).
 * - O085_1158(h, R02_043D(h, ITEM_2D), 1): die 1 wird vor dem inneren Aufruf gepusht.
 * - "while (i++ < 3)" + "if (--i < 3)"; "(signed long)(3 - i) * 10" -> movsx eax / imul eax,eax,0ah.
 * - "g_resting = g_25d7 = 0" (Byte zuerst, dann cbw).
 * (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH) Die Funktionen sind Methoden der Klasse Ev (O031,
 *   s_class.h): tribute, story_13, talk_90, show_pic_197, count_sneak_fails, choice_49, choice_56, wait_noon.
 *   Bit +0xbc.0 = hero.dead ((unsigned char)-Test wie mit dem unsigned-char-Bitfeld).
 */
#include "schweif.h"

#define TXT_EV		0x21c		/* Textdatei der Ereignisorte */
#define TXT_EV_N	0x78

void Ev::tribute()
{
	signed short old;
	signed short r;
	signed short i;
	struct hero far *h;
	register signed short k;
	register signed short ans;

	old = dlg_set16(g_dialog, 6);
	ans = dlg_choose(g_dialog, txt[1], 2, txt[2], txt[3]);
	r = 0;
	if (ans == 2) {
		ans = dlg_choose(g_dialog, txt[4], 2, txt[5], txt[6]);
		if (ans == 1) {
			i = 0;
			while (i++ < 3) {
				k = group_find_item(g_group, ITEM_FOOD_PACKAGE);
				if (k != -1) {
					h = g_group->heroes[k];
					drop_item(h, get_item_pos(h, ITEM_FOOD_PACKAGE), 1);
				} else
					break;
			}
			if (--i < 3)
				group_sub_money(g_group, (signed long)(3 - i) * 10);
			ans = dlg_choose(g_dialog, txt[7], 2, txt[8], txt[9]);
		}
		if (ans == 2) {
			dlg_message(g_dialog, txt[10]);
			mem_free(txt_buf);
			mem_free(txt);
			g_fight_end = 2;
#if FIX_0374
			/* Fix #374: Merker nach dem Kampf zuruecksetzen (bei leerem Kampf laeuft fight_finish nicht und der naechste Kampf endet sonst mit Bettlerflucht) */
			r = start_fight(0xa0, 0);
			g_fight_end = 0;
#else
			r = start_fight(0xa0, 0);
#endif
			txt_load_list(g_font, TXT_EV, TXT_EV_N, &txt_buf, &txt);
			if (!r)
				dlg_message(g_dialog, txt[11]);
		}
	}
	g_low_beggars_scared = 1;
	dlg_set16(g_dialog, old);
}

signed short Ev::story_13()
{
	signed short old;
	register signed short r;

	old = dlg_set16(g_dialog, 6);
	r = 0;
	if (dlg_choose(g_dialog, txt[13], 2, txt[14], txt[15]) == 1) {
		dlg_message(g_dialog, txt[16]);
		dlg_message(g_dialog, txt[17]);
		dlg_message(g_dialog, txt[18]);
		dlg_message(g_dialog, txt[19]);
		g_low_notices_read = 1;
		if (dlg_choose(g_dialog, txt[20], 2, txt[21], txt[22]) == 1)
			r = 1;
	} else {
		Bmp b(0x20, 0x20, g_portrait_buf);
		load_portrait_buf(0xa9);
		dlg_box_bmp(g_dialog, &b, txt[23], txt[24], 1, txt[25]);
		dlg_box_bmp(g_dialog, &b, txt[23], txt[26], 0);
	}
	dlg_set16(g_dialog, old);
	return r;
}

void Ev::talk_90()
{
	struct hero far *h;

	h = group_get_npc(g_group);
	if (h->npc_id == 5) {
		if ((unsigned char)h->dead)
			talk(&g_talk, 0x90, 3, 0);
		else
			talk(&g_talk, 0x90, 4, 0);
		g_low_tunnel_arrival = 0;
	} else
		talk(&g_talk, 0x90, 2, 0);
}

void Ev::show_pic_197()
{
	signed short w;
	signed short old;
	Bmp far *b;

	b = new Bmp(0x40, 0x40);
	nvfile_load(g_nvf_heads, 0x197, b->data(), 0, 0x27, 0, &w, &w, SCREEN_W);
	g_low_tunnel_known = 1;
	old = dlg_set16(g_dialog, 5);
	snd_play(g_snd_fx, 0x69);
	dlg_box_bmp(g_dialog, b, (char far *)0, txt[86], 0);
	dlg_box_bmp(g_dialog, b, (char far *)0, txt[87], 0);
	dlg_set16(g_dialog, old);
	delete b;
}

signed short Ev::count_sneak_fails(signed char v)
{
	struct hero far *h;
	register signed short n;
	register signed short r;

	n = 0;
	h = get_first_hero(g_group);
	do {
		if (check_hero3(h)) {
			r = test_skill(h, TA_SCHLEICHEN, v);
			if (r == -1)
				n = 99;
			else if (!r)
				n++;
		}
		h = get_next_hero(g_group, h);
	} while (get_first_hero(g_group) != h);
	return n;
}

void Ev::choice_49()
{
	signed short r;
	register signed short ans;

	ans = dlg_choose(g_dialog, txt[49], 3, txt[50], txt[51], txt[52]);
	if (ans == 1) {
		dlg_message(g_dialog, txt[55]);
		group_imprison(3);
	} else if (ans == 2) {
		if (!group_scatter(0, 1))
			group_imprison(4);
	} else {
		g_fight_flag = 1;
		mem_free(txt_buf);
		mem_free(txt);
		r = start_fight(0xa1, 0);
		txt_load_list(g_font, TXT_EV, TXT_EV_N, &txt_buf, &txt);
		switch (r) {
		case 0:
			dlg_message(g_dialog, txt[54]);
			group_scatter(3, 1);
			break;
		case 2:
			break;
		default:
			dlg_message(g_dialog, txt[53]);
			if (!group_scatter(0, 1))
				group_imprison(5);
			break;
		}
	}
}

void Ev::choice_56()
{
	signed short ans;
	signed short r;

	ans = dlg_choose(g_dialog, txt[56], 2, txt[57], txt[58]);
	if (ans == 1) {
		dlg_message(g_dialog, txt[59]);
		group_imprison(5);
	} else {
		g_fight_flag = 1;
		mem_free(txt_buf);
		mem_free(txt);
		r = start_fight(0xa2, 0);
		txt_load_list(g_font, TXT_EV, TXT_EV_N, &txt_buf, &txt);
		switch (r) {
		case 0:
			dlg_message(g_dialog, txt[61]);
			group_scatter(3, 1);
			break;
		case 2:
			break;
		default:
			dlg_message(g_dialog, txt[60]);
			if (group_scatter(0, 1) != 2)
				group_imprison(5);
			break;
		}
	}
}

void Ev::wait_noon()
{
	if (dlg_choose(g_dialog, txt[78], 2, txt[79], txt[80]) == 1) {
		g_resting = 1;
		timewarp_until(g_date, 43200L);
		group_check_drunk(g_group, 0);
		g_resting = g_low_gavron_house = 0;
	}
}
