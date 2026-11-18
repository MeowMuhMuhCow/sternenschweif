/* OVL118 - Overlay O118 (Segment 0x21a7 in SCHWEIF.EXE): Reiseereignisse der Wege 81..99 (tevent_run O211;
 *   Reiseobjekt
 * o, Ort o->from; Texte g_tev_loc_txt = SECTION2.LTX, g_tev_txt = ALL_SECT.LTX; Merker route.event in g_routes) wie
 * O119/O120. unused_tevent_o118: leer (keinem Weg zugeordnet).
 * tevent_081: Ort 0x56 und g_artherion_heard/g_artherion_hut_known/g_artherion_hut_visited -> Bild 0x198/0x1a: Gashok von aussen (txt[9]), Merker [81] = 0.
 * tevent_082: Ort 0x56 ohne g_artherion_hut_visited: Pfad in den Wald (Bild 0x197/0x32, txt[10..12]); weiterstolpern und mind. 2
 *   Helden, einmalig (g_tev082_thicket_done): Orientierung +6 des ersten Helden - misslungen txt[51] (der Beste in
 *   Orientierung warnt vergeblich), 12 Stunden im Kreis (txt[52]), g_travel_state = -7; gelungen txt[51] mit dem
 *   Schlechtesten; umkehren -> action = 1, ev_half = 0.
 * tevent_083: Steppenrinder (Merker [83] == 2, Bild 4, txt[13..20]): schiessen (Kurz-/Langbogen und Pfeile oder
 *   Armbrust und Bolzen, bester Schuetze; sonst Rueckfrage txt[17]) bzw. anpirschen -> Herde rennt die Gruppe um
 *   (group_sub_le_dice(W20), 30 AP); Merker zyklisch 1..5. Wie O120 tevent_136.
 * tevent_084: Pflanzenkunde-Probe (route.event[3] * 3 + 5) > 0 -> 3 Wirselkraut (ALL_SECT[3]/[5]); Zaehler bis 41.
 * tevent_085/086/098 -> valley_ambush bzw. mountain_rope. tevent_087: Zyklus 1..4, bei 1 deer_hunt. tevent_088:
 *   guide_offer.
 * tevent_089: armed_hero_fight(o, 1/2). tevent_092: txt[21] (nichts passiert). tevent_093: Schlagloch (txt[22]),
 *   Merker -1.
 * tevent_094: Otter in der Wildfalle (Bildfolge 0x1ce, txt[23..26]): befreien -> Tsa +50, 15 Minuten; haeuten ->
 *   Otterfell, 1 Stunde, 3 Proviantpakete; Merker [94] = 0.
 * tevent_096: show_once_anim.
 * tevent_097: Bauer mit gebrochener Wagenachse (Bild 0x2c, txt[27..37]): helfen -> Merker 0, alle Helden legen die
 *   Gegenstaende der Plaetze 3 und 12 in freie Rucksackplaetze (move_item), Text je nach Zahl der Helfer, Hinterhalt
 *   der fuenf Gestalten (Kampf 0x2a); weitergehen -> txt[30].
 * tevent_099: Steinschlag der Wegelagerer: liegt die Lawine noch (g_tev099_rockslide), rockslide_dig (freiraeumen;
 *   fertig
 *   -> Merker 0); sonst bei gedrosseltem Tempo (g_slow5_on) Vorhut sieht die Falle (txt[38..40], umgehen = umkehren);
 *   Steinschlag (Bild 0x198/0x1b, txt[41..43]): kaempfen (Kampf 0x36/0x35 je Richtung) bzw. fliehen (Bild 0x1c,
 *   txt[44], umkehren); g_tev099_rockslide = 1.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL118.CPP   -> 4364/4364 Bytes GLEICH (18.10.2026), cobjfix laeuft.
 *   Ohne -Og 2 Bytes zu lang (Epilogkopien), -Oi wegen strcat in tevent_097.
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Klassen/Strukturen wie OVL119/OVL120 (Bmp, Nvf, Anim, g_routes); Bmp als Block-Local, new Bmp/new Anim.
 * - tevent_083: wie in O120 faellt case 2 ohne break in "case 3: break;" (mit break entsteht ein zweites jmp short).
 * - tevent_097: "cnt = s1 = s2 = 0" (s2 zuerst), "g_routes[97].event = n = i = 0" (les zuerst), heroes[i] = 0 -> zwei
 *   Wort-Stores (Segment zuerst); -Og-Zwischenwerte [bp-16h] (&heroes[i]) und [bp-18h] ((int)n).
 * - tevent_094: ans vor dem Zeiger deklariert ([bp-1]); switch ohne default.
 * - tevent_099: geschwisterliche Bloecke teilen den Bmp-Platz [bp-10h], case-2-Block eigenes Bmp [bp-1ah].
 * Originalfehler: tevent_097 laeuft ueber alle 7 Eintraege von heroes[], auch ueber die leeren (NULL) - liest dann
 *   bei 0000:01xx (Interruptvektoren) und kann move_item mit NULL-Held aufrufen; bei 4..7 Helden nennt der Text
 *   (txt[33]) nur die ersten drei. tevent_083 wie O120 (case 1 ohne break). tevent_082: "else if (r >= 0)"
 *   ueberfluessig.
 */
#include <stdio.h>
#include <string.h>

#include "schweif.h"

void unused_tevent_o118(void)
{
}

void tevent_081(Travel far *o)
{
	signed short w;

	if (o->from == 0x56 && (g_artherion_heard != 0 || g_artherion_hut_known != 0 || g_artherion_hut_visited != 0)) {
		Bmp b(0x40, 0x40);
		nvfile_load(g_nvf_heads, 0x198, b.data(), 0, 0x1a, 0, &w, &w, 0x140);
		/* SECTION2.LTX[9]: "Von hier draussen aus besehen, sieht dieses kleine Staedtchen Gashok richtiggehend
		 * romantisch aus. Aber was von diesem Eindruck zu halten ist?" */
		dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[9], 0);
		g_routes[81].event = 0;
	}
}

void tevent_082(Travel far *o)
{
	signed short w;
	signed short ans;
	struct hero far *h;
	struct hero far *h2;
	struct hero far *h3;
	register signed short r;

	if (!g_artherion_hut_visited && o->from == 0x56) {
		Bmp b(0x40, 0x40);
		nvfile_load(g_nvf_heads, 0x197, b.data(), 0, 0x32, 0, &w, &w, 0x140);
		/* SECTION2.LTX[10]: "Der Pfad windet sich weiter und weiter in den Wald hinein. Waere nicht gelegentlich eine
		 * Stelle, an der das Gras nur spaerlich und erst seit kurzem wieder waechst, haettet Ihr laengst das Gefuehl,
		 * den Weg verloren zu haben. Jetzt aber scheint es soweit zu sein: Ihr steht im Wald, und in keiner Richtung
		 * ausser der, aus der Ihr gekommen seid, ist irgendetwas zu erkennen. Wollt Ihr" */
		/* SECTION2.LTX[11]: "dennoch und auf gut Glueck weiterstolpern oder" */
		/* SECTION2.LTX[12]: "umkehren und den richtigen Weg suchen?" */
		ans = dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[10], 2, g_tev_loc_txt[11], g_tev_loc_txt[12]);
		if (ans == 1) {
			if (group_count_check1(g_group) >= 2 && !g_tev082_thicket_done) {
				h = get_first_active_hero(g_group);
				h2 = get_best_hero_skill(g_group, TA_ORIENTIERUNG);
				h3 = get_worst_hero_skill(g_group, TA_ORIENTIERUNG);
				while (h2 == h)
					h2 = get_random_hero(g_group);
				while (h3 == h)
					h3 = get_random_hero(g_group);
				r = test_skill(h, TA_ORIENTIERUNG, 6);
				if (r < 0) {
					/* SECTION2.LTX[51]: "Obwohl %s meint, dass Ihr nicht mehr in der gleichen Richtung marschiert, haltet Ihr
					 * Euch dicht hinter %s um Euch in diesem Dickicht nicht aus den Augen zu verlieren." */
					sprintf(g_text_output_buf, g_tev_loc_txt[51], h2->name, h->name);
					dlg_box_bmp(g_dialog, &b, (char far *)0, g_text_output_buf, 0);
					timewarp_hours(g_date, 12L);
					/* SECTION2.LTX[52]: "Diese Kreuzung kennt Ihr doch! Da seid Ihr ja wohl ganz schoen im Kreis herumgewandert." */
					dlg_message(g_dialog, g_tev_loc_txt[52]);
					g_travel_state = -7;
				} else if (r >= 0) {
					sprintf(g_text_output_buf, g_tev_loc_txt[51], h3->name, h->name);
					dlg_box_bmp(g_dialog, &b, (char far *)0, g_text_output_buf, 0);
				}
			}
			g_tev082_thicket_done = 1;
		} else {
			o->action = 1;
			o->ev_half = 0;
		}
	}
}

void tevent_083(Travel far *o)
{
	signed short w;
	signed char ans;
	signed char r;
	struct hero far *h;

	ans = r = -1;
	if (g_routes[83].event == 2) {
		Bmp b(0x40, 0x40);
		nvfile_load(g_nvf_heads, 0x197, b.data(), 0, 4, 0, &w, &w, 0x140);
		h = get_best_hero_skill(g_group, TA_SCHUSSWAFFEN);
		while (ans == -1)
			/* SECTION2.LTX[13]: "Im Schatten einiger Baeume grast eine kleine Herde Steppenrinder. Wollt Ihr" */
			/* SECTION2.LTX[14]: "aus sicherer Entfernung eines der Tiere schiessen" */
			/* SECTION2.LTX[15]: "Euch anpirschen und die <Kuehe> niedermetzeln" */
			/* SECTION2.LTX[16]: "oder unter den Augen der misstrauischen Tiere weitergehen?" */
			ans = dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[13], 3, g_tev_loc_txt[14], g_tev_loc_txt[15], g_tev_loc_txt[16]);
		if (ans == 2)
			r = 1;
		switch (ans) {
		case 1:
			if ((group_find_item(g_group, ITEM_SHORTBOW) || group_find_item(g_group, ITEM_LONGBOW)) && group_find_item(g_group, ITEM_ARROW) ||
				group_find_item(g_group, ITEM_CROSSBOW) && group_find_item(g_group, ITEM_BOLT)) {
				/* SECTION2.LTX[18]: "Obwohl %s ein Treffer gelingt, ist das Tier nicht gefaehrlich verletzt. Es schaut grimmig
				 * in Eure Richtung, bevor sich die Herde aus dem Staub macht." */
				sprintf(g_text_output_buf, g_tev_loc_txt[18], h->name);
				dlg_message(g_dialog, g_text_output_buf);
			} else {
				while (r == -1)
					/* SECTION2.LTX[17]: "Da Ihr keine geeigneten Fernkampfwaffen besitzt muesst Ihr Euch schon etwas anderes
					 * einfallen lassen." */
					r = dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[17], 2, g_tev_loc_txt[15], g_tev_loc_txt[16]);
			}
		case 2:
			if (r == 1) {
				/* SECTION2.LTX[19]: "Alles klappt nach Plan. Ihr habt Euch bis auf zwanzig Fuss an die Herde herangemacht und
				 * lauft jetzt schreiend und mit gezueckten Waffen auf die Herde zu. Doch die Rinder ergreifen nicht wie
				 * erwartet die Flucht, sondern setzen zum Gegenangriff an." */
				dlg_message(g_dialog, g_tev_loc_txt[19]);
				group_sub_le_dice(g_group, 0x14, 0, 0);
				group_add_ap(g_group, 0x1e);
				/* SECTION2.LTX[20]: "Mit Ihren massigen Koerpern rennen sie Euch um, als ob Ihr kein Hindernis seid.
				 * Dementsprechend schmerzhaft ist dieses Ereignis auch. Zum Glueck setzt die Herde den eingeschlagenen Weg
				 * fort." */
				dlg_message(g_dialog, g_tev_loc_txt[20]);
			}
		case 3:
			break;
		}
		g_routes[83].event = g_routes[83].event + 1;
	} else if (g_routes[83].event >= 5)
		g_routes[83].event = 1;
	else
		g_routes[83].event = g_routes[83].event + 1;
}

void tevent_084(Travel far *o)
{
	struct hero far *h;
	signed short r;

	h = get_best_hero_skill(g_group, TA_PFLANZENKUNDE);
	r = test_skill(h, TA_PFLANZENKUNDE, g_routes[3].event * 3 + 5);
	if (r > 0) {
		/* ALL_SECT.LTX[3]: "%s entdeckt am Wegesrand %d Portion%s Wirselkraut!" */
		/* ALL_SECT.LTX[5]: "en" */
		sprintf(g_text_output_buf, g_tev_txt[3], h->name, 3, g_tev_txt[5]);
		dlg_message(g_dialog, g_text_output_buf);
		group_give_item(g_group, ITEM_WHIRLWEED, 3);
	}
	if (g_routes[84].event <= 40)
		g_routes[84].event++;
}

void tevent_085(Travel far *o)
{
	valley_ambush(o);
}

void tevent_086(Travel far *o)
{
	mountain_rope(o);
}

void tevent_087(Travel far *o)
{
	if (g_routes[87].event == 1) {
		deer_hunt(o);
		g_routes[87].event = g_routes[87].event + 1;
	} else if (g_routes[87].event >= 4)
		g_routes[87].event = 1;
	else
		g_routes[87].event = g_routes[87].event + 1;
}

void tevent_088(Travel far *o)
{
	if (o->from == 0x5b)
		guide_offer(o, 3);
	else
		guide_offer(o, 4);
	if (g_guide_won != 0)
		g_routes[88].event = 0;
}

void tevent_089(Travel far *o)
{
	if (o->from == 0x58)
		armed_hero_fight(o, 1);
	else
		armed_hero_fight(o, 2);
}

void tevent_092(Travel far *o)
{
	/* SECTION2.LTX[21]: "Wundert es Euch nicht, dass schon so lange nichts mehr passiert ist?" */
	dlg_message(g_dialog, g_tev_loc_txt[21]);
}

void tevent_093(Travel far *o)
{
	struct hero far *h;

	h = get_first_active_hero(g_group);
	/* SECTION2.LTX[22]: "Unvermittelt taucht vor Euch ein Schlagloch auf, dass %s in den Magen boxt (oder was glaubt
	 * Ihr, warum die Dinger so heissen?)." */
	sprintf(g_text_output_buf, g_tev_loc_txt[22], h->name);
	dlg_message(g_dialog, g_text_output_buf);
	g_routes[93].event = g_routes[93].event - 1;
}

void tevent_094(Travel far *o)
{
	signed char ans;
	Anim far *a;

	a = new Anim(0x1ce, 0, 0);
	/* SECTION2.LTX[23]: "Ein jaemmerliches Quietschen erweckt Eure Aufmerksamkeit: Am Wegesrand hat sich ein Otter in
	 * einer Wildfalle gefangen und schaut Euch aus braunen, flehenden Augen an. Sein Gewinsel ist wirklich
	 * steinerweichend. Wollt Ihr" */
	/* SECTION2.LTX[24]: "weitergehen?" */
	/* SECTION2.LTX[25]: "das Tier befreien oder" */
	/* SECTION2.LTX[26]: "es haeuten und verspeisen?" */
	ans = dlg_box_anim(g_dialog, a, (char far *)0, g_tev_loc_txt[23], 3, g_tev_loc_txt[24], g_tev_loc_txt[25], g_tev_loc_txt[26]);
	delete a;
	switch (ans) {
	case 2:
		g_god_favor_tsa += 50;
		timewarp_minutes(g_date, 15L);
		break;
	case 3:
		group_give_item(g_group, ITEM_OTTER_PELT, 1);
		timewarp_hours(g_date, 1L);
		group_give_item(g_group, ITEM_FOOD_PACKAGE, 3);
		break;
	}
	g_routes[94].event = 0;
}

void tevent_096(Travel far *o)
{
	show_once_anim(o);
}

void tevent_097(Travel far *o)
{
	signed char i;
	signed char n;
	signed char cnt;
	signed char r;
	signed short w;
	signed short s1;
	signed short s2;
	struct hero far *h;
	Bmp far *b;
	signed char ans;
	struct hero far *heroes[7];

	ans = -1;
	for (i = 0; i < 7; i++)
		heroes[i] = 0;
	cnt = s1 = s2 = 0;
	b = new Bmp(0x40, 0x40);
	nvfile_load(g_nvf_heads, 0x197, b->data(), 0, 0x2c, 0, &w, &w, 0x140);
	n = 0;
	h = get_first_active_hero(g_group);
	do {
		if (h != 0 && check_hero3(h)) {
			cnt++;
			heroes[n] = h;
			n++;
		}
		h = get_next_hero(g_group, h);
	} while (get_first_active_hero(g_group) != h);

	while (ans == -1)
		/* SECTION2.LTX[27]: "Am Wegesrand sitzt ein Bauer mit haengendem Kopf neben seinem Wagen, der seltsam schief
		 * dasteht. Etwas naeher heran kann man auch den Grund dafuer erkennen: ein Rad ist gebrochen. Wollt Ihr" */
		/* SECTION2.LTX[28]: "einfach weitergehen oder" */
		/* SECTION2.LTX[29]: "dem Mann Eure Hilfe anbieten?" */
		ans = dlg_box_bmp(g_dialog, b, (char far *)0, g_tev_loc_txt[27], 2, g_tev_loc_txt[28], g_tev_loc_txt[29]);

	switch (ans) {
	case 1:
		/* SECTION2.LTX[30]: "<Das ist aber nicht sehr fein von Euch!>" */
		dlg_box_bmp(g_dialog, b, (char far *)0, g_tev_loc_txt[30], 0);
		delete b;
		break;
	case 2:
		g_routes[97].event = n = i = 0;
#if FIX_0093
		/* Fix #93: nur ueber die gesammelten Helden laufen (keine NULL-Eintraege) */
		delete b;
		for (i = 0; i < cnt; i++) {
			for (n = 15; n <= 30; n++) {
#else
		delete b;
		for (i = 0; i < 7; i++) {
			for (n = 15; n <= 30; n++) {
#endif
				if (heroes[i]->inv[n].item_id <= 0) {
					if (s1 == 0) {
						s1 = n;
					} else {
						s2 = n;
						break;
					}
				}
			}
			if (heroes[i]->inv[3].item_id != 0 && s1 <= 30 && s1 != 0)
				move_item(heroes[i], 3, s1);
			if (heroes[i]->inv[12].item_id != 0 && s2 <= 30 && s2 != 0)
				move_item(heroes[i], 12, s2);
			s1 = s2 = 0;
		}
		if (cnt == 1) {
			/* SECTION2.LTX[31]: "Als %s gerade den Wagen anhebt, wobei %s natuerlich %s Waffen wegstecken muss," */
			sprintf(g_text_output_buf, g_tev_loc_txt[31], heroes[0]->name, grammar_pronoun(heroes[0]->sex, 0, 0),
				grammar_pronoun(heroes[0]->sex, 1, 1));
		}
		if (cnt >= 2 && cnt <= 7) {
			if (cnt == 2 || cnt == 3) {
				if (cnt == 2)
					/* SECTION2.LTX[32]: "Als %s und %s" */
					sprintf(g_text_output_buf, g_tev_loc_txt[32], heroes[0]->name, heroes[1]->name);
				else
					/* SECTION2.LTX[33]: "Als %s, %s und %s" */
					sprintf(g_text_output_buf, g_tev_loc_txt[33], heroes[0]->name, heroes[1]->name, heroes[2]->name);
				/* SECTION2.LTX[34]: "gerade den Wagen anheben," */
				strcat(g_text_output_buf, g_tev_loc_txt[34]);
			} else {
				sprintf(g_text_output_buf, g_tev_loc_txt[33], heroes[0]->name, heroes[1]->name, heroes[2]->name);
				strcat(g_text_output_buf, g_tev_loc_txt[34]);
				/* SECTION2.LTX[35]: "und der Rest versucht, die Achse wieder zu richten" */
				strcat(g_text_output_buf, g_tev_loc_txt[35]);
			}
			/* SECTION2.LTX[36]: "wobei sie natuerlich die Waffen wegstecken muessen," */
			strcat(g_text_output_buf, g_tev_loc_txt[36]);
		}
		/* SECTION2.LTX[37]: "springen fuenf zerlumpte Gestalten hinter dem naechsten Busch hervor!" */
		strcat(g_text_output_buf, g_tev_loc_txt[37]);
		dlg_choose(g_dialog, g_text_output_buf, 0);
		travel_leave(o);
		g_fight_flag = 1;
		r = start_fight(0x2a, 1);
		fight_result(o, r);
		travel_enter(o);
		break;
	}
}

void tevent_098(Travel far *o)
{
	valley_ambush(o);
}

void tevent_099(Travel far *o)
{
	signed short w;
	signed char ans;
	signed char r;
	signed char flag = 0;

	if (g_tev099_rockslide != 0) {
		g_tev099_rockslide = rockslide_dig(o);
		if (!g_tev099_rockslide)
			g_routes[99].event = 0;
		return;
	}
	if (g_slow5_on != 0) {
		Bmp b(0x40, 0x40);
		nvfile_load(g_nvf_heads, 0x197, b.data(), 0, 0x2d, 0, &w, &w, 0x140);
		/* SECTION2.LTX[38]: "Eure Vorhut entdeckt hinter der naechsten Biegung einige verdaechtige Gestalten, die auf
		 * halber Hoehe hinter einigen Felsen hocken. Wollt Ihr" */
		/* SECTION2.LTX[39]: "die mutmassliche Falle umgehen oder" */
		/* SECTION2.LTX[40]: "es darauf ankommen lassen?" */
		ans = dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[38], 2, g_tev_loc_txt[39], g_tev_loc_txt[40]);
		if (ans == 1) {
			o->action = 1;
			o->ev_half = 0;
			flag = 1;
		}
	}
	if (flag != 1) {
		Bmp b(0x40, 0x40);
		nvfile_load(g_nvf_heads, 0x198, b.data(), 0, 0x1b, 0, &w, &w, 0x140);
		/* SECTION2.LTX[41]: "Steinschlag! Von oben kommen Wegelagerer und versuchen, Euch mit dem Geroell den Weg zu
		 * versperren!" */
		/* SECTION2.LTX[42]: "Kaempfen oder" */
		/* SECTION2.LTX[43]: "fliehen?" */
		ans = dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[41], 2, g_tev_loc_txt[42], g_tev_loc_txt[43]);
		if (ans == -1)
			ans = 1;
		switch (ans) {
		case 1:
			travel_leave(o);
			g_fight_flag = 1;
			if (o->from == 0xc1)
				r = start_fight(0x36, 1);
			else
				r = start_fight(0x35, 1);
			fight_result(o, r);
			travel_enter(o);
			break;
		case 2: {
			Bmp b2(0x40, 0x40);
			nvfile_load(g_nvf_heads, 0x198, b2.data(), 0, 0x1c, 0, &w, &w, 0x140);
			/* SECTION2.LTX[44]: "Mit einigen wirklich grossen Spruengen schafft Ihr es, vor den herunterkollernden
			 * Felsbrocken die Engstelle zu passieren. Hinter Euch hoert Ihr die Wutschreie der ach so pfiffigen Raeuber,
			 * die sich durch Ihren eigenen Trick von Euch abgeschnitten haben..." */
			dlg_box_bmp(g_dialog, &b2, (char far *)0, g_tev_loc_txt[44], 0);
			o->action = 1;
			o->ev_half = 0;
			}
			break;
		}
		g_tev099_rockslide = 1;
	}
}
