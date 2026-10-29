/* OVL117 - Overlay O117 (Segment 0x21a1 in SCHWEIF.EXE): Reiseereignisse der Wege 66..78 (tevent_run O211;
 *   Reiseobjekt
 * o, Ort o->from; Texte g_tev_loc_txt = SECTION1.LTX, fuer Weg 76 und 78 SECTION2.LTX, g_tev_txt = ALL_SECT.LTX;
 * Merker route.event in g_routes). Geschwister von O116/O118..O124.
 * tevent_066 (auch Weg 69 ueber tevent_069): Baerenjunges auf der Blumenwiese (route.event[66] == 2, Bild 0x1d,
 *   txt[26]):
 *   mit der Faust erlegen (staerkster Held, 8 Proviantpakete, g_tev066_cub_killed), einfangen (schlechtester in
 *   Tierkunde wird gebissen: Schaden 2, Tollwut-/Wundfieber-Probe, Tierkunde +2) oder spielen lassen; beide Merker
 *   -1.
 *   Sonst die Baerin (Bild 0x196/6 in passender Groesse): der Held mit der hoechsten Ruestungsbehinderung - allein
 *   reisend tot (txt[34]); mit getoetetem Jungen fliehen/kaempfen mit Opfer (txt[35..52], group_damage-Kampfschaden),
 *   sonst
 *   laesst die Baerin ab (txt[50..54]); AP 40/20, Tagebuch ALL_SECT[121], Merker [66]/[69] = 0.
 * tevent_067: spring_event. tevent_070/tevent_071: Frau auf dem Elch (Merker [70]/[71] 3: Bild 0x22, txt[55]; 2: Bild
 *   0x26, txt[56]), beide Merker -1.
 * tevent_072: Kampf 0x2f/0x30 je Richtung, gewonnen -> Merker 0.
 * tevent_073: Ort 0x4f, travel_stage == 1: Erdrutsch (Bild 0x0c, txt[57]): umkehren (action = 1, ev_half = 0) bzw.
 *   GE-Probe der Gruppe group_test_attrib_msg (txt[58]/[67], W3 Schaden), 30 Minuten.
 * tevent_075: Ort 0x51, travel_stage == 2: Gefuehl, beobachtet zu werden (txt[59..65]) - ggf. umkehren.
 * tevent_076: Bach, nasse Fuesse (SECTION2[0], Bild 0x0d): Dumpfschaedel-Probe 2 % fuer die Gruppe.
 * tevent_077: Merker == 50, travel_stage == 1, nicht krank: schaerferes Tempo (txt[66]) -> g_rest_mod += 2, hurry =
 *   1.
 * tevent_078: Teich mit Bremsen (Bildfolgen 0x1c9..0x1cb, SECTION2[1..8]): nachsehen, Abwehr (select_hero_bar),
 *   mit brennender Fackel 1..3 Schaden, sonst CH -2 fuer zwei Tage (group_add_effect) und 2..6 Schaden.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL117.CPP   -> 4974/4974 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH; spring_event hat keine Parameter, das Original
 *   uebergibt o - Aufruf ueber gecasteten Funktionszeiger)
 * - -Og: Epilogkopien (Ende des ersten if-Zweigs in tevent_066, tevent_070/tevent_071, tevent_078), h->name-Offset
 *   als
 *   Zwischenwert in SI (mov ax,[bp-0ch] / add ax,22h / mov si,ax, bei doppeltem Namen push si).
 * - Bmp als Block-Local: tevent_066 zwei Bloecke teilen [bp-16h] (zweites erst nach nvfile_info mitten im Block),
 *   Destruktor am Blockende (vor diary_add); tevent_073 Bmp nach dem char-Local -> Luecke [bp-4] (gerade Adresse).
 * - tevent_073: Rumpf von case 2 steht vor case 1 (Sprungziel-Reihenfolge), Vergleiche trotzdem 1, 2.
 * - tevent_078: switch OHNE default als letzte Anweisung, letzter "case 2: return;" -> Nicht-Treffer springt aufs
 *   Funktionsende, -Og macht daraus eine Epilogkopie direkt hinter den Vergleichen (jnz short / jmp near case 2);
 *   case 2 -> Epilogkopie am Ende + toter echter Epilog. "default: return;" (vorn oder hinten) erzeugt immer
 *   einen zusaetzlichen Sprung.
 * - (signed short)g_group->nr_heroes == 1 -> cbw / cmp ax,1. Merker "x = x - 1" (mov/dec/mov mit zwei les).
 */
#include <stdio.h>

#include "schweif.h"

void tevent_066(Travel far *o)
{
	signed short dummy;
	signed short w;
	signed short hh;
	signed char r = -1;
	signed char c;
	struct hero far *h;
	signed short hp;

	if (g_routes[66].event == 2) {
		g_routes[66].event = g_routes[66].event - 1;
		g_routes[69].event = g_routes[69].event - 1;
		Bmp b(0x40, 0x40);
		nvfile_load(g_nvf_heads, 0x197, b.data(), 0, 0x1d, 0, &dummy, &dummy, 0x140);
		while (r == -1)
			/* SECTION1.LTX[26]: "Ihr seht auf einer Blumenwiese ein Baerenjunges herumtollen. Von seiner Mutter ist weit
			 * und breit nichts zu sehen. Wollt Ihr" */
			/* SECTION1.LTX[27]: "Eure Lebensmittelvorraete durch frisches Baerenfleisch bereichern," */
			/* SECTION1.LTX[28]: "das Baerenbaby einfangen und in der naechsten Stadt an einen Gaukler verkaufen," */
			/* SECTION1.LTX[29]: "den Baeren weiterspielen lassen, und Eurer Wege gehen?" */
			r = dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[26], 3, g_tev_loc_txt[27], g_tev_loc_txt[28], g_tev_loc_txt[29]);
		switch (r) {
		case 1:
			h = get_hero_attrib_max(g_group, ATTRIB_KK);
			/* SECTION1.LTX[30]: "Mit einem Schlag mit der blossen Faust gelingt es %s das Tier niederzustrecken." */
			sprintf(g_text_output_buf, g_tev_loc_txt[30], h->name);
			dlg_choose(g_dialog, g_text_output_buf, 0);
			timewarp_minutes(g_date, 30L);
			group_give_item(g_group, ITEM_FOOD_PACKAGE, 8);
			g_tev066_cub_killed = 1;
			break;
		case 2:
			h = get_worst_hero_skill(g_group, TA_TIERKUNDE);
			/* SECTION1.LTX[31]: "Tatsaechlich gelingt es %s nach einer Weile das Tier zu schnappen, doch da %s nicht weiss,
			 * wie man sich dabei anstellen sollte, beisst %s der Baer in die Hand, und ergreift dann die Flucht." */
			sprintf(g_text_output_buf, g_tev_loc_txt[31], h->name, grammar_pronoun(h->sex, 0, 0), grammar_pronoun(h->sex, 2, 0));
			dlg_choose(g_dialog, g_text_output_buf, 0);
			sub_hero_le(h, 2);
			hero_disease_test(h, ILLNESS_TOLLWUT, 5);
			hero_disease_test(h, ILLNESS_WUNDFIEBER, 0x2d);
			add_skill_value(h, TA_TIERKUNDE, 2);
		case 3:
			break;
		}
	} else {
		h = get_hero_rs_be_max(g_group);
		hp = h->le;
		c = h->sex;
		if ((signed short)g_group->nr_heroes == 1) {
			/* SECTION1.LTX[34]: "Allein zu reisen ist eben zu gefaehrlich." */
			dlg_message(g_dialog, g_tev_loc_txt[34]);
			sub_hero_le(h, hp);
		} else {
			nvfile_info(g_nvf_heads, 0x196, 6, &w, &hh);
			Bmp b(w, hh);
			nvfile_load(g_nvf_heads, 0x196, b.data(), 0, 6, 0, &dummy, &dummy, 0x140);
			if (g_tev066_cub_killed != 0) {
				while (r == -1) {
					g_dlg_box_flag = 0;
					/* SECTION1.LTX[35]: "Ploetzlich steht eine riesige Baerin vor Euch. Mit wutentbrannten rotleuchtenden Augen
					 * hat sie sich auf den Hinterpfoten aufgebaut. Mit knapp drei Schritt Hoehe handelt es sich um ein besonders
					 * grosses Exemplar. Wollt Ihr" */
					/* SECTION1.LTX[36]: "die Flucht ergreifen," */
					/* SECTION1.LTX[37]: "oder Euch dem Tier zum Angriff stellen?" */
					r = dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[35], 2, g_tev_loc_txt[36], g_tev_loc_txt[37]);
				}
				switch (r) {
				case 1:
					r = -1;
					/* SECTION1.LTX[38]: "Ihr rennt so schnell Ihr koennt, doch die Baerin kommt immer naeher. %s geht langsam die
					 * Luft aus, %s Ruestung wird %s wohl bremsen, und %s faellt langsam zurueck, so dass die Baerin %s wohl bald
					 * eingeholt hat. Wollt Ihr" */
					sprintf(g_text_output_buf, g_tev_loc_txt[38], h->name, grammar_pronoun(c, 1, 1), grammar_pronoun(c, 2, 0),
						grammar_pronoun(c, 0, 0), grammar_pronoun(c, 2, 0));
					while (r == -1)
						/* SECTION1.LTX[39]: "trotzdem weiterrennen," */
						/* SECTION1.LTX[40]: "oder Euch noch zum Kampf entschliessen." */
						r = dlg_choose(g_dialog, g_text_output_buf, 2, g_tev_loc_txt[39], g_tev_loc_txt[40]);
					switch (r) {
					case 1:
						/* SECTION1.LTX[41]: "Ihr rennt weiter so schnell Ihr koennt. Da hoert Ihr hinter Euch die grauenvollen
						 * Todesschreie von %s.@Ihr rennt und rennt...@bis zur totalen Erschoepfung.@Gluecklicherweise hat sich die
						 * Baerin mit einem Opfer zufrieden gegeben. Ihr koennt sogar noch am Kampfplatz die zerfetzte Leiche von %s
						 * finden." */
						sprintf(g_text_output_buf, g_tev_loc_txt[41], h->name, h->name);
						dlg_message(g_dialog, g_text_output_buf);
						sub_hero_le(h, hp);
						break;
					case 2:
						/* SECTION1.LTX[52]: "Als Ihr umdreht und Euch der Baerin zuwendet, seht Ihr, wie sie gerade %s mit einem
						 * einzigen Tatzenhieb von den Beinen holt, und %s bewegungslos am Boden liegen bleibt. Es gibt kein zurueck
						 * mehr und ein heftiger Kampf entbrennt." */
						sprintf(g_text_output_buf, g_tev_loc_txt[52], h->name, grammar_pronoun(c, 0, 0));
						dlg_message(g_dialog, g_text_output_buf);
						sub_hero_le(h, hp - 1);
						group_damage(o, 0x14, -5, 5, 3, 1);
						if (group_count_check3(g_group) >= 1)
							/* SECTION1.LTX[44]: "Doch die Goetter stehen Euch bei und Ihr koennt das Tier niederstrecken." */
							dlg_message(g_dialog, g_tev_loc_txt[44]);
						else
							g_travel_state = 1;
						break;
					}
					break;
				case 2:
					r = -1;
					/* SECTION1.LTX[45]: "Ihr habt Euch noch nicht richtig postiert, da hat die Baerin %s bereits mit einem
					 * einzigen Prankenhieb zu Boden gestreckt. Wollt Ihr jetzt" */
					sprintf(g_text_output_buf, g_tev_loc_txt[45], h->name);
					while (r == -1)
						/* SECTION1.LTX[46]: "doch lieber fluechten," */
						/* SECTION1.LTX[47]: "oder den Kampf austragen." */
						r = dlg_choose(g_dialog, g_text_output_buf, 2, g_tev_loc_txt[46], g_tev_loc_txt[47]);
					sub_hero_le(h, hp - 4);
					switch (r) {
					case 1:
						hp = h->le;
						/* SECTION1.LTX[48]: "Ihr rennt davon so schnell Ihr koennt, aber die Baerin verfolgt Euch gar nicht.
						 * Offenbar hat sie mit einem Opfer genug.@%s ist allerdings nicht mehr zu helfen." */
						sprintf(g_text_output_buf, g_tev_loc_txt[48], h->name);
						dlg_message(g_dialog, g_text_output_buf);
						sub_hero_le(h, hp);
						break;
					case 2:
						/* SECTION1.LTX[49]: "Durch Eure Attacken wird die Baerin von %s abgelenkt und ein harter Kampf entsteht." */
						sprintf(g_text_output_buf, g_tev_loc_txt[49], h->name);
						dlg_message(g_dialog, g_text_output_buf);
						group_damage(o, 0x14, -5, 5, 3, 1);
						if (group_count_check3(g_group) >= 1)
							dlg_message(g_dialog, g_tev_loc_txt[44]);
						else
							g_travel_state = 1;
						break;
					}
					break;
				}
				group_add_ap(g_group, 0x28);
			} else {
				while (r == -1) {
					g_dlg_box_flag = 0;
					r = dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[35], 2, g_tev_loc_txt[36], g_tev_loc_txt[37]);
				}
				switch (r) {
				case 1:
					r = -1;
					sprintf(g_text_output_buf, g_tev_loc_txt[38], h->name, grammar_pronoun(c, 1, 1), grammar_pronoun(c, 2, 0),
						grammar_pronoun(c, 0, 0), grammar_pronoun(c, 2, 0));
					while (r == -1)
						r = dlg_choose(g_dialog, g_text_output_buf, 2, g_tev_loc_txt[39], g_tev_loc_txt[40]);
					switch (r) {
					case 1:
						/* SECTION1.LTX[50]: "%s spuert schon den heissen Atem der Baerin im Nacken, als die Schreie des Baerenjungen
						 * zu hoeren sind.@Sofort bleibt die Baerin stehen, lauscht nach der Richtung der Schreie und hetzt in dieser
						 * Richtung davon." */
						sprintf(g_text_output_buf, g_tev_loc_txt[50], h->name);
						dlg_message(g_dialog, g_text_output_buf);
						break;
					case 2:
						/* SECTION1.LTX[51]: "Gleichzeitig mit der Baerin erreicht Ihr %s und ein harter Kampf entbrennt." */
						sprintf(g_text_output_buf, g_tev_loc_txt[51], h->name);
						dlg_message(g_dialog, g_text_output_buf);
						group_damage(o, 0x0c, -4, 7, 3, 1);
						if (group_count_check3(g_group) >= 1)
							/* SECTION1.LTX[54]: "Da ist mitten im Kampfgeschehen ploetzlich das aengstliche Schreien des Baerenjungen
							 * zu hoeren, und sofort wendet sich die besorgte Mutter von Euch ab." */
							dlg_message(g_dialog, g_tev_loc_txt[54]);
						else
							g_travel_state = 1;
						break;
					}
					break;
				case 2:
					g_dlg_box_flag = 0;
					/* SECTION1.LTX[53]: "Ein harter Kampf entbrennt..." */
					dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[53], 0);
					group_damage(o, 0x0c, -4, 7, 3, 1);
					if (group_count_check3(g_group) >= 1)
						dlg_message(g_dialog, g_tev_loc_txt[54]);
					else
						g_travel_state = 1;
					break;
				}
				group_add_ap(g_group, 0x14);
			}
		}
		/* ALL_SECT.LTX[121]: "Wir wurden unterwegs von einer aufgebrachten Baerenmutter angegriffen." */
		diary_add(g_diary, g_tev_txt[121], 0);
		g_routes[66].event = 0;
		g_routes[69].event = 0;
	}
}

void tevent_067(Travel far *o)
{
	((void (*)(Travel far *))spring_event)(o);	/* Original uebergibt o, spring_event hat keine Parameter */
}

void tevent_069(Travel far *o)
{
	tevent_066(o);
}

void tevent_070(Travel far *o)
{
	signed short dummy;

	if (g_routes[70].event == 3) {
		Bmp b(0x40, 0x40);
		nvfile_load(g_nvf_heads, 0x197, b.data(), 0, 0x22, 0, &dummy, &dummy, 0x140);
		/* SECTION1.LTX[55]: "Eine zierliche Frau auf einem Elch kommt Euch entgegen und nickt Euch freundlich zu, ohne
		 * jedoch deshalb anzuhalten." */
		dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[55], 0);
		g_routes[70].event = g_routes[70].event - 1;
		g_routes[71].event = g_routes[71].event - 1;
	} else if (g_routes[70].event == 2) {
		Bmp b(0x40, 0x40);
		nvfile_load(g_nvf_heads, 0x197, b.data(), 0, 0x26, 0, &dummy, &dummy, 0x140);
		/* SECTION1.LTX[56]: "<Holla, Ihr dort!> hoert Ihr eine weibliche Stimme von hinten rufen. Als Ihr Euch umdreht,
		 * seht Ihr die Frau auf dem Elch eilig naeherpreschen. <Ich wollte Euch nur sagen, dass man wirklich auf diesen
		 * Viechern reiten kann, aber ausser mir macht das niemand, glaube ich.> Damit wendet sie ihr feuriges Reittier
		 * und entschwindet." */
		dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[56], 0);
		g_routes[70].event = g_routes[70].event - 1;
		g_routes[71].event = g_routes[71].event - 1;
	}
}

void tevent_071(Travel far *o)
{
	signed short dummy;

	if (g_routes[71].event == 3) {
		Bmp b(0x40, 0x40);
		nvfile_load(g_nvf_heads, 0x197, b.data(), 0, 0x22, 0, &dummy, &dummy, 0x140);
		/* SECTION1.LTX[55]: "Eine zierliche Frau auf einem Elch kommt Euch entgegen und nickt Euch freundlich zu, ohne
		 * jedoch deshalb anzuhalten." */
		dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[55], 0);
		g_routes[70].event = g_routes[70].event - 1;
		g_routes[71].event = g_routes[71].event - 1;
	} else if (g_routes[71].event == 2) {
		Bmp b(0x40, 0x40);
		nvfile_load(g_nvf_heads, 0x197, b.data(), 0, 0x26, 0, &dummy, &dummy, 0x140);
		/* SECTION1.LTX[56]: "<Holla, Ihr dort!> hoert Ihr eine weibliche Stimme von hinten rufen. Als Ihr Euch umdreht,
		 * seht Ihr die Frau auf dem Elch eilig naeherpreschen. <Ich wollte Euch nur sagen, dass man wirklich auf diesen
		 * Viechern reiten kann, aber ausser mir macht das niemand, glaube ich.> Damit wendet sie ihr feuriges Reittier
		 * und entschwindet." */
		dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[56], 0);
		g_routes[70].event = g_routes[70].event - 1;
		g_routes[71].event = g_routes[71].event - 1;
	}
}

void tevent_072(Travel far *o)
{
	signed char r = 0;

	travel_leave(o);
	g_fight_flag = 1;
	if (o->from == 0x4f)
		r = start_fight(0x2f, 0);
	else
		r = start_fight(0x30, 0);
	r = fight_result(o, r);
	travel_enter(o);
	if (r == 0)
		g_routes[72].event = 0;
}

void tevent_073(Travel far *o)
{
	signed short dummy;
	signed char r = -1;
	Bmp b(0x40, 0x40);

	nvfile_load(g_nvf_heads, 0x197, b.data(), 0, 0x0c, 0, &dummy, &dummy, 0x140);
	if (o->from == 0x4f && travel_stage(o) == 1) {
		while (r == -1)
			/* SECTION1.LTX[57]: "Der Weg verliert sich in einem grau-braunen Feld aus getrocknetem Schlamm und Geroell, so
			 * als haetten heftige Regenfaelle einen Erdrutsch im Gebirge ausgeloest und die Lawine die Strassen unter sich
			 * begraben. Wollt Ihr trotzdem weitergehen?" */
			/* ALL_SECT.LTX[1]: "Ja" */
			/* ALL_SECT.LTX[2]: "Nein" */
			r = dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[57], 2, g_tev_txt[1], g_tev_txt[2]);
		switch (r) {
		case 2:
			o->action = 1;
			o->ev_half = 0;
			break;
		case 1:
			/* SECTION1.LTX[67]: "%s gelangt heil ueber die gefaehrliche Stelle." */
			/* SECTION1.LTX[58]: "%s rutscht aus und kullert einige Schritt den Hang hinunter." */
			group_test_attrib_msg(g_group, ATTRIB_GE, 0, g_tev_loc_txt[67], g_tev_loc_txt[58], random_schweif(3));
			timewarp_minutes(g_date, 30L);
			break;
		}
	}
}

void tevent_075(Travel far *o)
{
	signed char r;
	struct hero far *h;
	signed char done = 0;

	if (o->from == 0x51 && travel_stage(o) == 2) {
		/* SECTION1.LTX[59]: "Schon seit geraumer Zeit verstaerkt sich bei Euch der Eindruck, beobachtet zu werden, aber
		 * Ihr koennt einfach niemanden entdecken. Wollt Ihr" */
		/* SECTION1.LTX[60]: "Euch verstecken und Eurerseits beobachten?" */
		/* SECTION1.LTX[61]: "laut in den Wald hineinrufen?" */
		/* SECTION1.LTX[62]: "unbeteiligt weitergehen?" */
		r = dlg_choose(g_dialog, g_tev_loc_txt[59], 3, g_tev_loc_txt[60], g_tev_loc_txt[61], g_tev_loc_txt[62]);
		switch (r) {
		case 1:
			h = get_random_hero(g_group);
			/* SECTION1.LTX[63]: "Einmal meint %s eine Bewegung gesehen zu haben, aber das war wohl eine Taeuschung, denn
			 * alle anderen sehen nichts, so sehr Ihr Euch auch anstrengt. Als Ihr jedoch aufgeben wollt und auf den Weg
			 * zurueckkehrt, findet jeder von Euch einen langen Pfeil in seinem Rucksack stecken. Merkwuerdigerweise haben
			 * diese Pfeile keinerlei Spitze... Wollt Ihr jetzt" */
			sprintf(g_text_output_buf, g_tev_loc_txt[63], h->name);
			/* SECTION1.LTX[64]: "umdrehen?" */
			r = dlg_choose(g_dialog, g_text_output_buf, 3, g_tev_loc_txt[61], g_tev_loc_txt[64], g_tev_loc_txt[62]);
			switch (r) {
			case 1:
				/* SECTION1.LTX[65]: "Ihr wisst doch: Wie man in den Wald hineinruft... Sonst geschieht nichts. Wollt Ihr
				 * jetzt" */
				r = dlg_choose(g_dialog, g_tev_loc_txt[65], 2, g_tev_loc_txt[64], g_tev_loc_txt[62]);
				switch (r) {
				case 1:
					done = 1;
					break;
				case 2:
					break;
				}
				break;
			case 2:
				done = 1;
				break;
			case 3:
				break;
			}
			break;
		case 2:
			r = dlg_choose(g_dialog, g_tev_loc_txt[65], 2, g_tev_loc_txt[64], g_tev_loc_txt[62]);
			switch (r) {
			case 1:
				done = 1;
				break;
			case 2:
				break;
			}
			break;
		case 3:
			break;
		}
		if (done == 1) {
			o->action = 1;
			o->ev_half = 0;
			o->ev_start = 0;
			o->ev_end = 0;
		}
	}
	g_routes[75].event = 0;
}

void tevent_076(Travel far *o)
{
	signed short dummy;
	Bmp b(0x40, 0x40);

	nvfile_load(g_nvf_heads, 0x197, b.data(), 0, 0x0d, 0, &dummy, &dummy, 0x140);
	/* SECTION2.LTX[0]: "Bei der Ueberquerung eines Baches holt Ihr Euch alle nasse Fuesse. Wenn das mal keine
	 * Erkaeltung gibt..." */
	dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[0], 0);
	group_get_ill(g_group, ILLNESS_DUMPFSCHAEDEL, 2);
}

void tevent_077(Travel far *o)
{
	signed char r;

	if (g_routes[77].event == 0x32 && travel_stage(o) == 1 && travel_health_factor(o, 0) != 1) {
		/* SECTION1.LTX[66]: "Dieser Weg macht einen sehr angenehmen Eindruck; wollt Ihr das und Eure gute Verfassung
		 * ausnutzen und ein etwas schaerferes Tempo vorlegen?" */
		/* ALL_SECT.LTX[1]: "Ja" */
		/* ALL_SECT.LTX[2]: "Nein" */
		r = dlg_choose(g_dialog, g_tev_loc_txt[66], 2, g_tev_txt[1], g_tev_txt[2]);
		if (r == 1) {
			g_rest_mod += 2;
			o->hurry = 1;
		}
		g_routes[77].event = 0;
	}
}

void tevent_078(Travel far *o)
{
	signed char r;
	signed short v;
	Anim far *a1;
	Anim far *a2;
	Anim far *a3;

	a1 = new Anim(0x1c9, 0, 0);
	/* SECTION2.LTX[1]: "Etwas vor Euch seht Ihr die blaue Oberflaeche eines kleinen Teiches glitzern. Irgendetwas
	 * jedoch ist nicht in Ordnung, und nach wenigen Schritten wisst Ihr auch, was: Es summt. Von vorne hoert Ihr ein
	 * Summen und Brummen wie von Insekten, jedoch sind kaum welche zu sehen, so dass es laut und noch einiges
	 * entfernt sein muss. Wollt Ihr" */
	/* SECTION2.LTX[2]: "Nachsehen, was dort hinten vor sich geht oder" */
	/* SECTION2.LTX[3]: "versuchen, dem Geraeusch auszuweichen?" */
	r = dlg_box_anim(g_dialog, a1, (char far *)0, g_tev_loc_txt[1], 2, g_tev_loc_txt[2], g_tev_loc_txt[3]);
	delete a1;
	if (r == -1)
		r = 1;
	switch (r) {
	case 1:
		a2 = new Anim(0x1ca, 0, 0);
		/* SECTION2.LTX[4]: "Es sind Insekten! Bremsen von einer enormen Groesse und offensichtlich voellig ausgehungert
		 * fallen ueber Euch her, und es scheinen immer mehr zu kommen. Wollt Ihr" */
		/* SECTION2.LTX[5]: "Abwehrmassnahmen ergreifen, oder" */
		/* SECTION2.LTX[6]: "abwarten, was passiert?" */
		r = dlg_box_anim(g_dialog, a2, (char far *)0, g_tev_loc_txt[4], 2, g_tev_loc_txt[5], g_tev_loc_txt[6]);
		if (r == 1)
			select_hero_bar(o);
		delete a2;
		v = group_find_item(g_group, ITEM_TORCH_ON);
		a3 = new Anim(0x1cb, 0, 0);
		if (v >= 0) {
			/* SECTION2.LTX[7]: "Der Rauch vertreibt sie ein wenig, aber nicht ganz. Und So seid Ihr nicht nur etwas ausser
			 * Atem, sondern auch ordentlich zerstochen, als sie endlich die Verfolgung aufgeben und von Euch ablassen." */
			dlg_box_anim(g_dialog, a3, (char far *)0, g_tev_loc_txt[7], 0);
			group_sub_le_range(g_group, 1, 3);
		} else {
			/* SECTION2.LTX[8]: "Sie stechen durch Kleidung und kriechen unter die Ruestung, sie stechen in die Beine und in
			 * das Gesicht, und wenn Ihr nicht vorsichtig seid, sogar in die Zunge. Ihr seht aus wie die Streuselkuchen, als
			 * sie endlich von Euch ablassen." */
			dlg_box_anim(g_dialog, a3, (char far *)0, g_tev_loc_txt[8], 0);
			group_add_effect(g_group, 2 * TICKS_PER_DAY, ATTRIB_CH, -2, EFFECT_ATTRIB);
			group_sub_le_range(g_group, 2, 6);
		}
		delete a3;
	case 2:
		return;
	}
}
