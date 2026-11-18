/* OVL119 - Overlay O119 (Segment 0x21af in SCHWEIF.EXE): Reiseereignisse der Wege 100..119 (tevent_run O211;
 * Reiseobjekt o, Ort o->from, Abbruch o->action/ev_half/ev_end; Texte g_tev_loc_txt = SECTION3.LTX; Merker
 * route.event/route.known in g_routes). tevent_101/104/106/116 -> valley_ambush, tevent_112/113 -> mountain_rope
 *   (O165).
 * tevent_100/tevent_119: Kampf 0x37/0x38 bzw. 0x3b/0x3c je Richtung zwischen travel_leave/travel_enter, gewonnen ->
 *   route.event[100]/[119] = 0. tevent_103/tevent_105: random_encounter - beim ersten Mal (route.event == 1) W6 + 4,
 *   sonst zu 60 % W6 + 2. tevent_108: npc6_encounter ab Ort 0x64. tevent_109: txt[0].
 * tevent_110: Ort 0xc0 -> tevent_111; route.event[110] == 60 -> Hoehlenspinnen (txt[48], Kampf 9, danach 70); == 50
 *   -> Bild 0x197/0x10: der beste in Sinnesschaerfe entdeckt einen alten Zwergenstollen (zwei Fragen, Bild 0x27),
 *   der schlechteste in Koerperbeherrschung loest den Einsturz aus, Freilegen 1 Stunde (Schaufel und Hacke) bzw. 3.
 * tevent_111: Ort 0x69/0xc0 bei travel_stage == 2 -> Bilder 0x198/0x1b/0x1c: Schlucht; beim ersten Mal
 *   (g_tev111_seen) Gefahrensinn +2 des ersten Helden - misslungen: Steinlawine, LE bis auf 1, txt[12..16]; danach
 *   verschuettet (Umkehr o->action = 1).
 * tevent_114: Steinbock auf schmalem Pfad (Bild 0x197/0x23, txt[17..20]): umdrehen; Nase geben (W6 + 4 Schaden);
 *   Futter hinwerfen - Gegenstand eines Helden waehlen (get_active_hero/select_item): Lakritze/Bonbons -> Bildfolge
 *   0x1d0 (new Anim), sonst txt[25], wenn der Gegenstand nicht unablegbar ist. tevent_117: armed_hero_fight(o, 4/3).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y -E..\bc\BIN\TASM.EXE OVL119.CPP   -> 3386/3386 Bytes GLEICH (18.10.2026),
 *   cobjfix laeuft. Ohne -Og 1 Byte zu lang (Epilogkopien in tevent_103/tevent_105/tevent_111/tevent_117).
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Bild-Objekte als Block-Locals (Konstruktor an der Deklarationsstelle, auch mitten im Block; Destruktoren
 *   (.., 2) rueckwaerts am Blockende); geschwisterliche Bloecke teilen sich die Plaetze [bp-12h]/[bp-1ch].
 * - tevent_114: "done == 0" (cmp byte) statt "!done" (mov al / cbw / or ax,ax); switch "case 1: case 2: done = 1;
 *   case 3: break;" (Durchfall in case 3, kein default); new Anim(0x1d0, 0, 0) -> Anim_ctor(0, ..), delete ->
 *   Anim_dtor(p, 3).
 * - random_schweif liefert int ("add ax,4" ohne cbw).
 * - Originalfehler/Auffaelliges: tevent_114 liest h->inv[i].item_id erst NACH drop_item(h, i, 1) (Gegenstand schon
 *   verbraucht - war es der letzte, ist der Platz vermutlich leer und 0x59/0x5a wird nie erkannt);
 *   tevent_103/tevent_105 initialisieren r = 0 unnoetig; tevent_110 ruft tevent_111 auf, das seine Bedingung selbst
 *   prueft.
 */
#include <stdio.h>

#include "schweif.h"

void tevent_100(Travel far *o)
{
	signed char r;

	travel_leave(o);
	g_fight_flag = 1;
	if (o->from == 0x62)
		r = start_fight(0x37, 0);
	else
		r = start_fight(0x38, 0);
	r = fight_result(o, r);
	travel_enter(o);
	if (r == 0)
		g_routes[100].event = 0;
}

void tevent_101(Travel far *o)
{
	valley_ambush(o);
}

void tevent_103(Travel far *o)
{
	signed char r = 0;

	if (g_routes[103].event == 1) {
		g_routes[103].event++;
		r = random_schweif(6) + 4;
		if (o->from == 0x64)
			random_encounter(o, r, 4);
		else
			random_encounter(o, r, 3);
	} else {
		r = random_schweif(10);
		if (r <= 5) {
			r = random_schweif(6) + 2;
			if (o->from == 0x64)
				random_encounter(o, r, 4);
			else
				random_encounter(o, r, 3);
		}
	}
}

void tevent_104(Travel far *o)
{
	valley_ambush(o);
}

void tevent_105(Travel far *o)
{
	signed char r = 0;

	if (g_routes[105].event == 1) {
		g_routes[105].event++;
		r = random_schweif(6) + 4;
		if (o->from == 0x63)
			random_encounter(o, r, 4);
		else
			random_encounter(o, r, 3);
	} else {
		r = random_schweif(10);
		if (r <= 5) {
			r = random_schweif(6) + 2;
			if (o->from == 0x63)
				random_encounter(o, r, 4);
			else
				random_encounter(o, r, 3);
		}
	}
}

void tevent_106(Travel far *o)
{
	valley_ambush(o);
}

void tevent_108(Travel far *o)
{
	if (o->from == 0x64)
		npc6_encounter(o);
}

void tevent_109(Travel far *o)
{
	/* SECTION3.LTX[0]: "Tunnel" */
	dlg_message(g_dialog, g_tev_loc_txt[0]);
}

void tevent_110(Travel far *o)
{
	signed short w;
	signed char r;
	signed char k;
	struct hero far *h;

	if (o->from == 0xc0 && travel_stage(o) == 2) {
		tevent_111(o);
		g_routes[110].known = 1;
	}
	if (travel_stage(o) == 0 && g_routes[110].event == 60) {
		/* SECTION3.LTX[48]: "Bei dem Einsturz des Zwergenstollens wurden einige Hoehlenspinnen aufgeschreckt, die sich
		 * jetzt fuer die Zerstoerung ihres Heimes raechen wollen." */
		dlg_message(g_dialog, g_tev_loc_txt[48]);
		travel_leave(o);
		g_fight_flag = 1;
		k = start_fight(9, 0);
		fight_result(o, k);
		travel_enter(o);
		g_routes[110].event = 70;
	}
	if (travel_stage(o) == 0 && g_routes[110].event == 50) {
		Bmp b(0x40, 0x40);
		nvfile_load(g_nvf_heads,0x197, b.data(), 0, 0x10, 0, &w, &w, 0x140);
		h = get_best_hero_skill(g_group, TA_SINNESSCHAERFE);
		/* SECTION3.LTX[1]: "In der Bergwand am Wegesrand entdeckt %s einen Eingang. Wollt Ihr ihn" */
		sprintf(g_text_output_buf, g_tev_loc_txt[1], h->name);
		/* SECTION3.LTX[2]: "naeher inspizieren oder" */
		/* SECTION3.LTX[3]: "links (da liegt er naemlich) liegen lassen?" */
		r = dlg_box_bmp(g_dialog, &b, (char far *)0, g_text_output_buf, 2, g_tev_loc_txt[2], g_tev_loc_txt[3]);
		if (r == 1) {
			Bmp b2(0x40, 0x40);
			nvfile_load(g_nvf_heads,0x197, b2.data(), 0, 0x27, 0, &w, &w, 0x140);
			/* SECTION3.LTX[4]: "Es handelt sich um den Eingang zu einem alten Stollen, dem Zustand der Stempel nach schon
			 * seit geraumer Zeit aufgegeben. Wollt Ihr" */
			/* SECTION3.LTX[5]: "hinein- oder" */
			/* SECTION3.LTX[6]: "weitergehen?" */
			r = dlg_box_bmp(g_dialog, &b2, (char far *)0, g_tev_loc_txt[4], 2, g_tev_loc_txt[5], g_tev_loc_txt[6]);
			if (r == 1) {
				h = get_worst_hero_skill(g_group, TA_KOERPERBEHERRSCHUNG);
				/* SECTION3.LTX[7]: "Einige Schritt innen, tritt %s irgendwogegen, wo es besser nicht geschehen waere: Hinter
				 * Euch stuerzt der Gang mit lautem Getoese ein. Auf der anderen Seite hat er das schon laengst getan, so dass
				 * Ihr Euch den Weg nach draussen wohl freibuddeln muesst. Zum Glueck sind es ja nur ein paar Schritt..." */
				sprintf(g_text_output_buf, g_tev_loc_txt[7], h->name);
				dlg_message(g_dialog, g_text_output_buf);
				if (group_find_item(g_group, ITEM_SHOVEL) >= 0 && group_find_item(g_group, ITEM_PICKAXE) >= 0)
					timewarp_hours(g_date, 1L);
				else
					timewarp_hours(g_date, 3L);
				g_routes[110].event = 60;
			}
		}
	}
}

void tevent_111(Travel far *o)
{
	signed short w;
	signed char r;
	struct hero far *h;

	if (o->from == 0x69 && travel_stage(o) == 2) {
		g_routes[111].known = 1;
		place_reveal_dirs(o, 0x68);
	}
	if (travel_stage(o) == 2 && (o->from == 0xc0 || o->from == 0x69)) {
		if (g_tev111_seen == 1) {
			Bmp b(0x40, 0x40);
			nvfile_load(g_nvf_heads,0x198, b.data(), 0, 0x1b, 0, &w, &w, 0x140);
			/* SECTION3.LTX[8]: "Der Pfad fuehrt weiter in eine Schlucht, deren Waende recht steil ansteigen. Stellenweise
			 * habt Ihr Schwierigkeiten voranzukommen, weil von den Seiten Geroell und Steine auf den Weg gerutscht sind." */
			dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[8], 0);
			Bmp b2(0x40, 0x40);
			nvfile_load(g_nvf_heads,0x198, b2.data(), 0, 0x1c, 0, &w, &w, 0x140);
			/* SECTION3.LTX[9]: "Nach etwa zwei Meilen Fussmarsch muesst Ihr feststellen, dass die Schlucht vollstaendig
			 * verschuettet ist. Ihr seid gezwungen umzukehren." */
			dlg_box_bmp(g_dialog, &b2, (char far *)0, g_tev_loc_txt[9], 0);
			o->action = 1;
			o->ev_end = 0;
		} else {
			Bmp b(0x40, 0x40);
			nvfile_load(g_nvf_heads,0x198, b.data(), 0, 0x1b, 0, &w, &w, 0x140);
			dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[8], 0);
			g_tev111_seen = 1;
			Bmp b2(0x40, 0x40);
			nvfile_load(g_nvf_heads,0x198, b2.data(), 0, 0x1c, 0, &w, &w, 0x140);
			h = get_first_active_hero(g_group);
			r = test_skill(h, TA_GEFAHRENSINN, 2);
			snd_play(g_snd_fx, 0x71);
			if (r > 0) {
				/* SECTION3.LTX[11]: "<Schnell zurueck>, bruellt %s und macht auf dem Absatz kehrt. Reflexartig dreht Ihr auch
				 * um und rennt los. Hinter Euch stuerzen von oben gewaltige Stein- und Geroellmassen in die Schlucht und
				 * verschuetten den Weg. Der Lawine seid Ihr gluecklich entkommen, doch bleibt Euch nur noch der Weg zurueck." */
				sprintf(g_text_output_buf, g_tev_loc_txt[11], h->name);
				dlg_box_bmp(g_dialog, &b2, (char far *)0, g_text_output_buf, 0);
			} else {
				/* SECTION3.LTX[12]: "Ploetzlich stuerzen von oben gewaltige Stein- und Geroellmassen herab. %s ist durch die
				 * Wucht der Lawine zu Boden geworfen und von Steinen verschuettet worden." */
				sprintf(g_text_output_buf, g_tev_loc_txt[12], h->name);
				dlg_box_bmp(g_dialog, &b2, (char far *)0, g_text_output_buf, 0);
				sub_hero_le(h, h->le - 1);
				/* SECTION3.LTX[13]: "Rasch macht Ihr Euch daran, %s zu befreien. Es dauert eine ganze Weile, bis Ihr %s
				 * bewusstlos bergen koennt." */
				/* SECTION3.LTX[14]: "Euren Freund" */
				/* SECTION3.LTX[15]: "Eure Freundin" */
				sprintf(g_text_output_buf, g_tev_loc_txt[13], h->sex == 0 ? g_tev_loc_txt[14] : g_tev_loc_txt[15],
					grammar_pronoun(h->sex, 2, 0));
				dlg_message(g_dialog, g_text_output_buf);
				/* SECTION3.LTX[16]: "Die Schlucht ist durch die Lawine komplett verschuettet. Euch bleibt nur der Weg zurueck,
				 * %s muesst Ihr tragen." */
				sprintf(g_text_output_buf, g_tev_loc_txt[16], h->name);
				dlg_message(g_dialog, g_text_output_buf);
			}
			o->action = 1;
			o->ev_end = 0;
		}
	}
}

void tevent_112(Travel far *o)
{
	mountain_rope(o);
}

void tevent_113(Travel far *o)
{
	mountain_rope(o);
}

void tevent_114(Travel far *o)
{
	signed short w;
	struct hero far *h2;
	struct hero far *h;
	Anim far *a;
	signed char ans = -1;
	signed char done = 0;
	register signed short i;
	Bmp b(0x40, 0x40);

	nvfile_load(g_nvf_heads,0x197, b.data(), 0, 0x23, 0, &w, &w, 0x140);
	while (ans == -1)
		/* SECTION3.LTX[17]: "An einer besonders schmalen Stelle, wo es auch noch schwindelerregend steil zu einer Seite
		 * abfaellt, steht mitten auf dem Weg ein Steinbock, der Euch misstrauisch beaeugt und sich weder durch Zurufen
		 * noch durch heftiges Armwedeln beeindrucken laesst.@ Wollt Ihr" */
		/* SECTION3.LTX[18]: "umdrehen?" */
		/* SECTION3.LTX[19]: "ihm eins auf die Nase geben oder" */
		/* SECTION3.LTX[20]: "ihm etwas zu fressen hinwerfen?" */
		ans = dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[17], 3, g_tev_loc_txt[18], g_tev_loc_txt[19], g_tev_loc_txt[20]);
	if (ans == 3) {
		while (done == 0) {
			/* SECTION3.LTX[21]: "Wer soll etwas hinwerfen?" */
			h = get_active_hero(g_group, g_tev_loc_txt[21]);
			if (h) {
				/* SECTION3.LTX[22]: "Welchen Gegenstand soll %s hinwerfen?" */
				sprintf(g_text_output_buf, g_tev_loc_txt[22], h->name);
				i = select_item(h, g_text_output_buf, 0);
				if (i != -1) {
#if FIX_0043
					/* Fix #43: Kennung vor dem Verbrauch lesen */
					{
						signed short id_ = h->inv[i].item_id;

						drop_item(h, i, 1);
						i = id_;
					}
#else
					drop_item(h, i, 1);
					i = h->inv[i].item_id;
#endif
					if (i == ITEM_LIQUORICE || i == ITEM_SWEETS) {
						a = new Anim(0x1d0, 0, 0);
						/* SECTION3.LTX[24]: "Nicht minder misstrauisch als bisher beschnuppert das Tier Eure Gabe, um sie dann
						 * vorsichtig in das Maul zu nehmen. Und nun bietet sich Euch die einmalige Gelegenheit, einem Steinbock
						 * zuzusehen, der gerade seine verklebten Zaehne wieder zu loesen versucht. Er hingegen findet das ueberhaupt
						 * nicht amuesant, und nachdem es ihm endlich gelungen ist, trollt er sich empoert." */
						dlg_box_anim(g_dialog, a, (char far *)0, g_tev_loc_txt[24], 0);
						delete a;
						done = 1;
						g_routes[114].event = 0;
					} else if (!g_items[i].undropable) {
						/* SECTION3.LTX[25]: "Das Viech schnuppert daran und schleudert dann das in seinen Augen unnuetze Ding mit
						 * den Hoernern den Hang hinunter." */
						dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[25], 0);
					}
				}
			}
			if (done == 0) {
				ans = -1;
				while (ans == -1)
					/* SECTION3.LTX[26]: "Wollt Ihr jetzt" */
					ans = dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[26], 3, g_tev_loc_txt[18], g_tev_loc_txt[19], g_tev_loc_txt[20]);
				switch (ans) {
				case 1:
				case 2:
					done = 1;
				case 3:
					break;
				}
			}
		}
	}
	if (ans == 1) {
		o->action = 1;
		o->ev_half = 0;
	}
	if (ans == 2) {
		g_routes[114].event = 0;
		h2 = get_first_active_hero(g_group);
		sub_hero_le(h2, random_schweif(6) + 4);
		/* SECTION3.LTX[23]: "Der Steinbock holt %s mit einem Rammstoss von den Fuessen und erklimmt dann den steilen
		 * Berghang." */
		sprintf(g_text_output_buf, g_tev_loc_txt[23], h2->name);
		dlg_message(g_dialog, g_text_output_buf);
	}
}

void tevent_116(Travel far *o)
{
	valley_ambush(o);
}

void tevent_117(Travel far *o)
{
	if (o->from == 0x6e)
		armed_hero_fight(o, 4);
	else
		armed_hero_fight(o, 3);
}

void tevent_119(Travel far *o)
{
	signed char r;

	travel_leave(o);
	g_fight_flag = 1;
	if (o->from == 0x3a)
		r = start_fight(0x3b, 0);
	else
		r = start_fight(0x3c, 0);
	r = fight_result(o, r);
	travel_enter(o);
	if (r == 0)
		g_routes[119].event = 0;
}
