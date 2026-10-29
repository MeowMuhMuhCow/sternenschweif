/* OVL165 - Overlay O165 (Segment 0x229b in SCHWEIF.EXE): Wegereignisse im Gebirge (Reiseobjekt o = Travel, Texte
 * g_tev_txt = ALL_SECT.LTX und g_tev_loc_txt = Ortstexte des Weges; Bilder per nvfile_load(g_nvf_heads, 0x197/0x198,
 * ..) in lokale Bmp-Objekte 64x64).
 * group_damage(o, n, add, lim, _, msg): Schaden W(n) + add fuer alle Helden (check_hero3), hoechstens bis LE (als
 *   signed char) - lim; bei msg Meldung g_tev_loc_txt[43] (Wegtexte, z. B. SECTION1.LTX[43] "%s verliert %d Punkte
 *   Lebensenergie."), sub_hero_le (O117 Baerin, avalanche).
 * valley_ambush(o) (O118/O119, ALL_SECT.LTX[6..8]): Tal mit Stellen fuer einen Hinterhalt - einmal (g_valley_asked,
 *   nicht bei g_slow5_on) wahlweise vorsichtig sichern -> o->speed = travel_slow5_on; danach immer [21].
 * mountain_rope(o) (O118/O119, ALL_SECT.LTX[9..12]): Gebirge mit gefaehrlichen Stellen - einmal (g_mountain_asked,
 *   nicht bei g_slow4_on) anseilen: mit Seil oder Stabzauber 3 -> o->speed = travel_slow4_on, sonst [12] "Und womit,
 *   bitte sehr?"; danach immer [22].
 * rockslide_dig(o) (O118/O121, SECTION2.LTX[45..50]): Steinlawine versperrt den Weg - Texte SECTION2.LTX (Datei 0x141)
 *   neu laden; umdrehen (o->action) oder freiraeumen: 2 Stunden, dann je Stunde bis 14 (mit Schaufel und Hacke) bzw.
 *   25 Stunden; ab 19 Uhr Nachtlager (Hunger/Durst 90, ALL_SECT.LTX[133], bis 8 Uhr) mit Rueckfrage [49]. Ergebnis 0
 *   = Weg frei, 1 = noch versperrt.
 * avalanche(o) (O121): traegt jemand einen der acht besonderen Gegenstaende (group_has_special_item >= 0) und beim
 *   ersten Mal (g_tev_avalanche_done) Lawine - Wegtexte [51]/[52] (vermutet SECTION3.LTX: Stille, Deckung hinter einem Felsen), Klang 0x71, Schaden W3
 *   bis LE 4; beim zweiten Mal Kampf 0x99 (Hoehlenspinnen, SECTION3.LTX[48]) per travel_leave/start_fight/fight_result.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL165.CPP   -> 2400/2400 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - C++ wegen der lokalen Bmp-Objekte (Bmp(w, h) = gb_new_alloc, automatischer Destruktor gb_delete(.., 2) am
 *   Blockende, rueckwaerts; Objekte werden auf gerade Adressen gelegt). In rockslide_dig steht b1 hinter Anweisungen,
 *   b2/b3 in inneren Bloecken; "return res" mit Objekt im Rumpf -> Zwischenwert [bp-14h] vor dem Destruktor.
 * - -Og noetig: SI/DI als Zwischenwerte fuer le - lim und (short)dmg in group_damage, Epilogkopien.
 * - inline-Getter hero::get_le() { return le; } -> mov ax,es:[bx+72h] / mov [bp-5],al (direkte Zuweisung ergibt
 *   mov al).
 * - "g_valley_asked == 0" -> cmp byte; char-Zaehler "hours = hours - 1"; "ans++" mit char -> inc byte ptr.
 * - switch ohne default (Faelle 1, 2 -> jmp near ans Ende).
 * - group_has_special_item hat keinen Parameter, avalanche uebergibt aber o -> Aufruf ueber gecasteten Zeiger.
 * Auffaellig: rockslide_dig laedt die Wegtexte als SECTION2.LTX neu und stellt die alten nicht wieder her
 *   (avalanche liest danach [51]/[52] aus SECTION2 - vermutet Originalfehler, wenn der Weg andere Texte hat).
 */
#include <stdio.h>
#include "schweif.h"

#define DATE		((Date far *)g_date)	/* Tageszeit als g_date + 1 (nicht g_daytime: anderes Fixup-Ziel) */
#define FILE_SECTION2	0x141		/* SECTION2.LTX */
#define FIGHT_SPIDERS	0x99		/* Kampf nach der zweiten Lawine (Hoehlenspinnen) */

void group_damage(Travel far *o, signed char n, signed char add, signed char lim, signed short x, signed short msg)
{
	struct hero far *h;
	signed char le;
	signed char dmg;

	h = get_first_hero(g_group);
	do {
		if (h && check_hero3(h)) {
			le = h->get_le();
			dmg = random_schweif(n) + add;
			if (dmg > 0 && le > lim) {
				if (le - lim <= dmg) {
					if (msg) {
						/* SECTION1.LTX[43]: "%s verliert %d Punkte Lebensenergie." */
						sprintf(g_text_output_buf, g_tev_loc_txt[43], h->name, le - lim);
						dlg_message(g_dialog, g_text_output_buf);
					}
					sub_hero_le(h, le - lim);
				} else {
					if (msg) {
						sprintf(g_text_output_buf, g_tev_loc_txt[43], h->name, dmg);
						dlg_message(g_dialog, g_text_output_buf);
					}
					sub_hero_le(h, dmg);
				}
			}
		}
		h = get_next_hero(g_group, h);
	} while (get_first_hero(g_group) != h);
}

void valley_ambush(Travel far *o)
{
	signed short w;
	signed char ans;
	Bmp b(0x40, 0x40);

	nvfile_load(g_nvf_heads, 0x197, b.data(), (void far *)0, 0x0e, 0, &w, &w, 0x140);
	if (g_valley_asked == 0 && g_slow5_on == 0) {
		/* ALL_SECT.LTX[6]: "Der Weg folgt einem Tal, das sich tiefer in die Berge windet und dessen Waende immer steiler
		 * werden. Ein um das andere Mal ergeben sich Stellen, die ideal fuer einen Hinterhalt scheinen. Wollt Ihr" */
		/* ALL_SECT.LTX[7]: "wie gewohnt weitergehen oder" */
		/* ALL_SECT.LTX[8]: "vorsichtig nach vorne sichern?" */
		ans = dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_txt[6], 2, g_tev_txt[7], g_tev_txt[8]);
		if (ans == 2)
			o->speed = travel_slow5_on(o, o->speed);
		g_valley_asked = 1;
	} else
		/* ALL_SECT.LTX[21]: "Ihr habt hinter der naechsten Biegung eine Bewegung bemerkt, aber als Ihr dort genauer
		 * nachseht koennt Ihr nichts entdecken." */
		dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_txt[21], 0);
}

void mountain_rope(Travel far *o)
{
	signed short w;
	signed char ans;
	signed char r;
	Bmp b(0x40, 0x40);

	nvfile_load(g_nvf_heads, 0x197, b.data(), (void far *)0, 0x0f, 0, &w, &w, 0x140);
	if (g_mountain_asked == 0 && g_slow4_on == 0) {
		/* ALL_SECT.LTX[9]: "Ihr befindet Euch nun im Gebirge, und es gibt einige Stellen, an denen ein unbedachter
		 * Fehltritt das Ende bedeuten kann. Wollt Ihr" */
		/* ALL_SECT.LTX[10]: "einfach weitergehen oder" */
		/* ALL_SECT.LTX[11]: "Euch anseilen?" */
		ans = dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_txt[9], 2, g_tev_txt[10], g_tev_txt[11]);
		if (ans == 2) {
			r = group_find_item(g_group, ITEM_ROPE);
			if (r >= 0 || get_hero_staffspell(g_group, 3))
				o->speed = travel_slow4_on(o, o->speed);
			else
				/* ALL_SECT.LTX[12]: "Und womit, bitte sehr?" */
				dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_txt[12], 0);
		}
		g_mountain_asked = 1;
	} else
		/* ALL_SECT.LTX[22]: "Der Weg wird immer schmaler und der Abgrund immer tiefer." */
		dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_txt[22], 0);
}

signed short rockslide_dig(Travel far *o)
{
	signed short w;
	signed char ans;
	signed char hours;
	struct hero far *h;
	register signed short done;
	register signed short res;

	res = 1;
	done = 0;
	mem_free(g_tev_loc_buf);
	mem_free(g_tev_loc_txt);
	txt_load_list(g_font, FILE_SECTION2, 0x37, &g_tev_loc_buf, &g_tev_loc_txt);
	Bmp b1(0x40, 0x40);
	nvfile_load(g_nvf_heads, 0x198, b1.data(), (void far *)0, 0x1c, 0, &w, &w, 0x140);
	/* SECTION2.LTX[46]: "Der Weg ist Euch durch eine riesige Steinlawine versperrt. Wollt Ihr" */
	/* SECTION2.LTX[47]: "umdrehen" */
	/* SECTION2.LTX[48]: "einen Weg freiraeumen" */
	ans = dlg_box_bmp(g_dialog, &b1, (char far *)0, g_tev_loc_txt[46], 2, g_tev_loc_txt[47], g_tev_loc_txt[48]);
	if (ans == -1)
		ans = 1;
	switch (ans) {
	case 1:
		o->action = TRAVEL_ACT_TURN;
		o->ev_half = 0;
		break;
	case 2: {
		Bmp b2(0x40, 0x40);
		nvfile_load(g_nvf_heads, 0x198, b2.data(), (void far *)0, 0x1c, 0, &w, &w, 0x140);
		timewarp_hours(g_date, 2L);
		/* SECTION2.LTX[45]: "Wenn das in dem Tempo weiter geht, werdet Ihr noch ein, zwei Tage brauchen. Wollt Ihr
		 * trotzdem weitergraben?" */
		/* ALL_SECT.LTX[1]: "Ja" */
		/* ALL_SECT.LTX[2]: "Nein" */
		ans = dlg_box_bmp(g_dialog, &b2, (char far *)0, g_tev_loc_txt[45], 2, g_tev_txt[1], g_tev_txt[2]);
		if (ans == 1) {
			if (group_find_item(g_group, ITEM_SHOVEL) >= 0 && group_find_item(g_group, ITEM_PICKAXE) >= 0)
				hours = 14;
			else
				hours = 25;
			while (!done) {
				if (g_travel_state == 3) {
					g_travel_state = 0;
					/* SECTION2.LTX[49]: "Wollt Ihr die Bergmannsarbeit fortsetzen?" */
					ans = dlg_box_bmp(g_dialog, &b2, (char far *)0, g_tev_loc_txt[49], 2,
						g_tev_txt[1],
						g_tev_txt[2]);
					if (ans == 2)
						done = 1;
				}
				if (DATE->ticks >= 0x190c8L) {	/* ab 19 Uhr: Nachtlager */
					g_travel_state = 3;
					for (ans = 0; ans < 7; ans++) {
						h = g_heroes + ans;
						if (h->type) {
							sub_hunger(h, 0x5a);
							sub_thirst(h, 0x5a);
						}
					}
					/* ALL_SECT.LTX[133]: "Als es zu daemmern beginnt, schlagt ihr ein notduerftiges Lager auf und legt Euch total
					 * erschoepft zur Ruhe." */
					dlg_message(g_dialog, g_tev_txt[133]);
					timewarp_until(g_date, 43200L);	/* 8 Uhr */
					for (ans = 0; ans < 7; ans++) {
						h = g_heroes + ans;
						if (h->type) {
							sub_hunger(h, 0x5a);
							sub_thirst(h, 0x5a);
						}
					}
				} else {
					timewarp_hours(g_date, 1L);
					hours = hours - 1;
					if (hours <= 0)
						done = 1;
				}
			}
			if (hours <= 0) {
				Bmp b3(0x40, 0x40);
				nvfile_load(g_nvf_heads, 0x198, b3.data(), (void far *)0, 0x1b, 0, &w, &w, 0x140);
				/* SECTION2.LTX[50]: "Ihr habt die Felsen so weit zur Seite geraeumt, dass Ihr Euren Weg fortsetzen koennt." */
				dlg_box_bmp(g_dialog, &b3, (char far *)0, g_tev_loc_txt[50], 0);
				res = 0;
			} else {
				o->action = TRAVEL_ACT_TURN;
				o->ev_half = 0;
			}
		} else {
			o->action = TRAVEL_ACT_TURN;
			o->ev_half = 0;
		}
#if FIX_0222
/* Fix #222: die Ereignistexte des Weges wiederherstellen (SECTION2 nur fuer den Steinschlag) */
		break;
	}
	}
	mem_free(g_tev_loc_buf);
	mem_free(g_tev_loc_txt);
	txt_load_list(g_font, o->evtxt_file, o->evtxt_n, &g_tev_loc_buf, &g_tev_loc_txt);
	return res;
}
#else
		break;
	}
	}
	return res;
}
#endif

void avalanche(Travel far *o)
{
	signed short w;
	signed char r;

	if (((signed char (*)(Travel far *))group_has_special_item)(o) >= 0 && g_tev_avalanche_done == 0) {
		Bmp b1(0x40, 0x40);
		nvfile_load(g_nvf_heads, 0x198, b1.data(), (void far *)0, 0x1b, 0, &w, &w, 0x140);
		/* SECTION3.LTX[51]: "Es ist immer stiller geworden in den letzten Minuten. Nur das Klirren der Ruestungen ist
		 * noch zu hoeren, ohrenbetaeubend laut kommt es Euch vor. Das findet der Berg offensichtlich auch, denn
		 * ploetzlich geht eine Lawine ab!" */
		dlg_box_bmp(g_dialog, &b1, (char far *)0, g_tev_loc_txt[51], 0);
		Bmp b2(0x40, 0x40);
		nvfile_load(g_nvf_heads, 0x198, b2.data(), (void far *)0, 0x1c, 0, &w, &w, 0x140);
		snd_play(g_snd_fx, 0x71);
		/* SECTION3.LTX[52]: "Ihr koennt alle hinter einem grossen Felsen Deckung finden, aber dennoch zieht Ihr Euch
		 * einige maechtige blaue Flecken zu.@Der Weg hinter Euch ist von der Lawine versperrt, aber da wollt Ihr ja auch
		 * gar nicht hin." */
		dlg_box_bmp(g_dialog, &b2, (char far *)0, g_tev_loc_txt[52], 0);
		group_damage(o, 3, 0, 4, 3, 0);
	}
	if (g_tev_avalanche_done == 1) {
		travel_leave(o);
		g_fight_flag = 1;
		r = start_fight(FIGHT_SPIDERS, 0);
		fight_result(o, r);
		travel_enter(o);
	}
}
