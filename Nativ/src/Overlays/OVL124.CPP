/* OVL124 - Overlay O124 (Segment 0x21d8 in SCHWEIF.EXE): Reiseereignisse der Wege 225..259 (tevent_run O211;
 * Reiseobjekt o, Ort o->from; Texte g_tev_loc_txt = SECTION6.LTX (Datei 0x143), g_tev_txt = ALL_SECT.LTX; Merker
 * route.event in g_routes; Bilder 64x64 per nvfile_load aus g_nvf_heads, Datei 0x197). Geschwister O121..O123.
 * tevent_225: die Verfolger lauern hinter der naechsten Biegung (SECTION6[28..30]) - nur wenn die Gruppe "nach vorne
 *   sichert" (g_slow5_on, FEATURE.LTX[18]; Bild 0x2d, Spaeher = erster handlungsfaehiger Held): ausweichen und
 *   umdrehen (o->action = 1) oder kaempfen; sonst gleich Kampf 0x5a/0x5c.
 * tevent_228/tevent_239/tevent_251: Fuehrer-Angebot guide_offer (Begleitung 3/4 bzw. 1/2 je Ausgangsort); g_guide_won
 *   -> Ereignis aus.
 * tevent_229/tevent_253: Zufallsbegegnung random_encounter (erstes Mal W6 + 4 Gegner, sonst bei W10 <= 5 W6 + 2).
 * tevent_230/tevent_244/tevent_247/tevent_254: Kampf (0x39/0x3a bzw. 0x37/0x38) zwischen travel_leave/travel_enter
 *   mit g_fight_max_enemies = 2 bzw. 1; Ereignis aus.
 * tevent_234: Beutelchen mit getrockneten Kraeutern (SECTION6[32]): 3 Trockenwirselkraut, 2 trockene Vierblaettrige
 *   Einbeeren; Ereignis aus.
 * tevent_238: jedes vierte Mal (Merker zyklisch 1..4) die Rehjagd deer_hunt (O163).
 * tevent_242/tevent_258: zu Beginn des Weges (travel_stage == 0) die Faehre ferry_crossing(o, 1, 4 bzw. 2) (O163).
 * tevent_245: im Winter (date_season == 0) beim Merkerstand 2 Kampf 0x56/0x57 (g_fight_max_enemies = 8).
 * tevent_246: Orklager (vom Ort 0xb6 aus; Bild 0x17, SECTION6[33..36]): umdrehen (o->action = 1) oder leise
 *   vorbeigehen (dem Schlechtesten im Schleichen knackt ein Zweig, niemand bemerkt Euch).
 * tevent_248: fahle Pflanzen und seltsamer Geruch (Bild 0x18; vom Ort 0xbc und bei travel_stage == 2,
 *   SECTION6[46..48]): umdrehen (o->action = 1) oder weitergehen - Paralyse mit 2 % fuer jeden Helden
 *   (group_get_ill).
 * tevent_250: Ork-Hinterhalt: ohne Sichern nach vorne (SECTION6[37]) beschiessen vier Orks die Gruppe (erster und bis
 *   zu drei weitere Helden W6 + 4 - RS-Bonus LE), Kampf 0x58/0x59 mit Gegnervorteil (1), sonst mit Heldenvorteil (2);
 *   Ereignis aus, wenn fight_result 0 liefert.
 * tevent_257: Absturz im Gebirge (Bild 0x19): der Schlechteste im Klettern (Probe W20 - Merker * 5) verliert den Halt
 *   (SECTION6[39]); angeseilt (g_slow4_on, FEATURE.LTX[20]) rettet ihn das Seil, aber der Rucksack oeffnet sich
 *   (SECTION6[40]: Gegenstaende der linken Hand, zu 40 % der rechten Hand, Wasserschlauch und zwei Rucksackplaetze
 *   gehen verloren; Tagebuch ALL_SECT[123]), sonst stuerzt er zu Tode (SECTION6[41], Tagebuch ALL_SECT[134], aus der
 *   Gruppe entfernt: group_set_slot(.., -1)); Merker bis 40 erhoehen.
 * tevent_259: Furt ford_crossing (O163).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL124.CPP -> 3524/3524 Bytes GLEICH (18.10.2026; -Ov statt -Og ebenso), cobjfix
 *   laeuft (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Ohne -Og/-Ov 10 Bytes anders: Epilogkopien am Ende der case-Zweige (tevent_225) und der Spruenge ans
 *   Funktionsende.
 * - Aufbau 1:1 wie O121 (gleiche Routinen-Typen): new/delete Bmp in tevent_225, lokales Bmp im if-Block (tevent_246)
 *   bzw. auf Funktionsebene (tevent_248/tevent_257); "break;" am Ende des letzten case -> toter Epilog.
 * - "g_slow5_on == 0" bzw. "if (g_slow5_on)" -> cmp byte; travel_stage liefert char ("== 0" -> or al,al), date_season
 *   int.
 * - Schaden "random_schweif(6) + 4 - h->rs_bonus" (push ax / cbw / pop dx / sub dx,ax); Probenwert als
 *   signed-char-Parameter "random_schweif(20) - g_routes[257].event * 5" -> sub dl,al; Heldentest "h != 0" -> cmp
 *   dword.
 * Moeglicher Originalfehler: tevent_250 ruft get_next_hero(g, h) weiter auf, auch nachdem es 0 geliefert hat (h = 0).
 */
#include <stdio.h>

#include "schweif.h"

void tevent_225(Travel far *o)
{
	signed char sel;
	signed char r;
	signed short dummy;
	struct hero far *h;
	Bmp far *b;

	h = get_first_active_hero(g_group);
	if (g_slow5_on) {
		b = new Bmp(0x40, 0x40);
		nvfile_load(g_nvf_heads, 0x197, b->data(), 0, 0x2d, 0, &dummy, &dummy, 0x140);
		/* SECTION6.LTX[28]: "%s kommt zurueck und meldet, dass hinter der naechsten Biegung Eure Verfolger lauern! Es
		 * scheint, als koenne man diesen Mistkerlen nie entkommen... Wollt Ihr" */
		sprintf(g_text_output_buf, g_tev_loc_txt[28], h->name);
		/* SECTION6.LTX[29]: "Ihnen ausweichen und umdrehen oder" */
		/* SECTION6.LTX[30]: "den Kampf aufnehmen?" */
		sel = dlg_box_bmp(g_dialog, b, (char far *)0, g_text_output_buf, 2, g_tev_loc_txt[29],
			g_tev_loc_txt[30]);
		delete b;
		if (sel == -1)
			sel = 2;
	} else {
		sel = 2;
	}
	switch (sel) {
	case 1:
		o->action = 1;
		o->ev_half = 0;
		break;
	case 2:
		travel_leave(o);
		g_fight_flag = 1;
		if (o->from == 0xae)
			r = start_fight(0x5a, 0);
		else
			r = start_fight(0x5c, 0);
		fight_result(o, r);
		travel_enter(o);
		break;
	}
}

void tevent_228(Travel far *o)
{
	if (o->from == 0x1d)
		guide_offer(o, 3);
	else
		guide_offer(o, 4);
	if (g_guide_won)
		g_routes[228].event = 0;
}

void tevent_229(Travel far *o)
{
	signed char n = 0;

	if (g_routes[229].event == 1) {
		g_routes[229].event++;
		n = random_schweif(6) + 4;
		if (o->from == 0xb1)
			random_encounter(o, n, 3);
		else
			random_encounter(o, n, 4);
		return;
	}
	n = random_schweif(10);
	if (n <= 5) {
		n = random_schweif(6) + 2;
		if (o->from == 0xb1)
			random_encounter(o, n, 3);
		else
			random_encounter(o, n, 4);
	}
}

void tevent_230(Travel far *o)
{
	signed char r;

	g_fight_max_enemies = 2;
	travel_leave(o);
	g_fight_flag = 1;
	if (o->from == 0xb2)
		r = start_fight(0x39, 0);
	else
		r = start_fight(0x3a, 0);
	fight_result(o, r);
	travel_enter(o);
	g_routes[230].event = 0;
}

void tevent_234(Travel far *o)
{
	/* SECTION6.LTX[32]: "Ihr findet ein Beutelchen mit getrockneten Kraeutern!" */
	dlg_message(g_dialog, g_tev_loc_txt[32]);
	group_give_item(g_group, ITEM_DRIED_WHIRLWEED, 3);
	group_give_item(g_group, ITEM_EINBEERE_DRIED, 2);
	g_routes[234].event = 0;
}

void tevent_238(Travel far *o)
{
	if (g_routes[238].event == 1) {
		deer_hunt(o);
		g_routes[238].event = g_routes[238].event + 1;
		return;
	}
	if (g_routes[238].event >= 4) {
		g_routes[238].event = 1;
		return;
	}
	g_routes[238].event = g_routes[238].event + 1;
}

void tevent_239(Travel far *o)
{
	if (o->from == 0xc6)
		guide_offer(o, 1);
	else
		guide_offer(o, 2);
	if (g_guide_won)
		g_routes[239].event = 0;
}

void tevent_242(Travel far *o)
{
	if (travel_stage(o) == 0)
		ferry_crossing(o, 1, 4);
}

void tevent_244(Travel far *o)
{
	signed char r;

	g_fight_max_enemies = 1;
	travel_leave(o);
	g_fight_flag = 1;
	if (o->from == 0xb8)
		r = start_fight(0x37, 0);
	else
		r = start_fight(0x38, 0);
	fight_result(o, r);
	travel_enter(o);
	g_routes[244].event = 0;
}

void tevent_245(Travel far *o)
{
	signed char r;

	if (date_season(g_date) == 0) {
		if (g_routes[245].event == 2) {
			g_fight_max_enemies = 8;
			travel_leave(o);
			g_fight_flag = 1;
			if (o->from == 0xba)
				r = start_fight(0x56, 0);
			else
				r = start_fight(0x57, 0);
			fight_result(o, r);
			travel_enter(o);
			return;
		}
		if (g_routes[245].event >= 4) {
			g_routes[245].event = 1;
			return;
		}
		g_routes[245].event = g_routes[245].event + 1;
	}
}

void tevent_246(Travel far *o)
{
	signed short dummy;
	signed char sel;
	struct hero far *h;

	if (o->from == 0xb6) {
		Bmp b(0x40, 0x40);

		nvfile_load(g_nvf_heads, 0x197, b.data(), 0, 0x17, 0, &dummy, &dummy, 0x140);
		h = get_worst_hero_skill(g_group, TA_SCHLEICHEN);
		/* SECTION6.LTX[33]: "Schon seit geraumer Zeit mehren sich die Anzeichen einer groesseren Gruppe in der
		 * Naehe:@Ausgeweidete Tierkadaver, gefaellte Baeume und eine Spur der Verwuestung durch das Unterholz. Dann
		 * riecht Ihr den Geruch von Lagerfeuern, und kurz danach koennt Ihr ein grosses Lager der Orks entdecken. Wollt
		 * Ihr" */
		/* SECTION6.LTX[34]: "umdrehen oder" */
		/* SECTION6.LTX[35]: "leise vorbeigehen?" */
		sel = dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[33], 2, g_tev_loc_txt[34],
			g_tev_loc_txt[35]);
		if (sel == 1) {
			o->action = 1;
			o->ev_half = 0;
		} else {
			/* SECTION6.LTX[36]: "Einmal knackt ein Zweig unter den Fuessen von %s, und Ihr bleibt wie erstarrt stehen;
			 * jedoch bemerkt Euch niemand und Ihr kommt unbemerkt vorbei." */
			sprintf(g_text_output_buf, g_tev_loc_txt[36], h->name);
			dlg_message(g_dialog, g_text_output_buf);
		}
	}
}

void tevent_247(Travel far *o)
{
	signed char r;

	g_fight_max_enemies = 1;
	travel_leave(o);
	g_fight_flag = 1;
	if (o->from == 0xba)
		r = start_fight(0x39, 0);
	else
		r = start_fight(0x3a, 0);
	fight_result(o, r);
	travel_enter(o);
	g_routes[247].event = 0;
}

void tevent_248(Travel far *o)
{
	signed short dummy;
	signed char sel;
	Bmp b(0x40, 0x40);

	nvfile_load(g_nvf_heads, 0x197, b.data(), 0, 0x18, 0, &dummy, &dummy, 0x140);
	if (o->from == 0xbc && travel_stage(o) == 2) {
		/* SECTION6.LTX[46]: "Ein seltsamer Geruch liegt in der Luft, und ganz unvermittelt ist kein Geraeusch mehr zu
		 * hoeren. Das Gruen der Pflanzen um Euch herum hat eine seltsam fahle Farbe, und nicht einmal die
		 * allgegenwaertigen Insekten sind zu sehen. Wollt Ihr" */
		/* SECTION6.LTX[47]: "dennoch weitergehen oder" */
		/* SECTION6.LTX[48]: "lieber umdrehen?" */
		sel = dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[46], 2, g_tev_loc_txt[47],
			g_tev_loc_txt[48]);
		if (sel == 2)
			o->action = 1;
		else
			group_get_ill(g_group, ILLNESS_PARALYSE, 2);
	}
}

void tevent_250(Travel far *o)
{
	struct hero far *h;
	signed char i;
	signed char r;

	if (g_slow5_on == 0) {
		/* SECTION6.LTX[37]: "Hinter den Felsen tauchen ploetzlich vier Orks auf, die Euch mit ihren Kurzboegen
		 * beschiessen. Auf in den Kampf!" */
		dlg_message(g_dialog, g_tev_loc_txt[37]);
		h = get_first_active_hero(g_group);
		sub_hero_le(h, random_schweif(6) + 4 - h->rs_bonus);
		for (i = 0; i < 3; i++) {
			h = get_next_hero(g_group, h);
			if (h != 0)
				sub_hero_le(h, random_schweif(6) + 4 - h->rs_bonus);
		}
		travel_leave(o);
		g_fight_flag = 1;
		if (o->from == 0x9f)
			r = start_fight(0x58, 1);
		else
			r = start_fight(0x59, 1);
		r = fight_result(o, r);
		travel_enter(o);
	} else {
		travel_leave(o);
		g_fight_flag = 1;
		if (o->from == 0x9f)
			r = start_fight(0x58, 2);
		else
			r = start_fight(0x59, 2);
		r = fight_result(o, r);
		travel_enter(o);
	}
	if (r == 0)
		g_routes[250].event = 0;
}

void tevent_251(Travel far *o)
{
	if (o->from == 0xbf)
		guide_offer(o, 3);
	else
		guide_offer(o, 4);
	if (g_guide_won)
		g_routes[251].event = 0;
}

void tevent_253(Travel far *o)
{
	signed char n = 0;

	if (g_routes[253].event == 1) {
		g_routes[253].event++;
		n = random_schweif(6) + 4;
		if (o->from == 0xc2)
			random_encounter(o, n, 1);
		else
			random_encounter(o, n, 2);
		return;
	}
	n = random_schweif(10);
	if (n <= 5) {
		n = random_schweif(6) + 2;
		if (o->from == 0xc2)
			random_encounter(o, n, 1);
		else
			random_encounter(o, n, 2);
	}
}

void tevent_254(Travel far *o)
{
	signed char r;

	g_fight_max_enemies = 1;
	travel_leave(o);
	g_fight_flag = 1;
	if (o->from == 0xc3)
		r = start_fight(0x37, 0);
	else
		r = start_fight(0x38, 0);
	fight_result(o, r);
	travel_enter(o);
	g_routes[254].event = 0;
}

void tevent_257(Travel far *o)
{
	signed short dummy;
	struct hero far *h;
	signed short slot;
	register signed short r;
	Bmp b(0x40, 0x40);

	nvfile_load(g_nvf_heads, 0x197, b.data(), 0, 0x19, 0, &dummy, &dummy, 0x140);
	h = get_worst_hero_skill(g_group, TA_KLETTERN);
	r = test_skill(h, TA_KLETTERN, random_schweif(20) - g_routes[257].event * 5);
	if (r <= 0) {
		/* SECTION6.LTX[39]: "%s verliert den Halt!" */
		sprintf(g_text_output_buf, g_tev_loc_txt[39], h->name);
		dlg_message(g_dialog, g_text_output_buf);
		if (g_slow4_on == 1) {
			/* SECTION6.LTX[40]: "Nach kurzem Sturz bremst das Seil den Sturz und rettet %s vor dem sicheren Tod, aber der
			 * Rucksack hat sich geoeffnet." */
			sprintf(g_text_output_buf, g_tev_loc_txt[40], h->name);
			dlg_box_bmp(g_dialog, &b, (char far *)0, g_text_output_buf, 0);
			drop_item_at(h, INV_SLOT_LEFT_HAND);
			if (random_schweif(100) < 40)
				drop_item_at(h, INV_SLOT_RIGHT_HAND);
			slot = get_item_pos(h, ITEM_WATERSKIN);
			drop_item_at(h, slot);
			drop_item_at(h, random_schweif(16) + 15);
			drop_item_at(h, random_schweif(16) + 15);
			/* ALL_SECT.LTX[123]: "%s ist bei einem Absturz nur knapp dem Tode entgangen." */
			sprintf(g_text_output_buf, g_tev_txt[123], h->name);
			diary_add(g_diary, g_text_output_buf, 0);
		} else {
			/* SECTION6.LTX[41]: "Der Sturz wird durch nichts aufgehalten, und waehrend die Schreie langsam verklingen,
			 * koennt Ihr %s immer tiefer stuerzen sehen. Da ist wohl nichts mehr zu machen..." */
			sprintf(g_text_output_buf, g_tev_loc_txt[41], h->name);
			dlg_message(g_dialog, g_text_output_buf);
			/* ALL_SECT.LTX[134]: "%s ist im Gebirge zu Tode gestuerzt. Die Leiche war nirgends zu finden." */
			sprintf(g_text_output_buf, g_tev_txt[134], h->name);
			diary_add(g_diary, g_text_output_buf, 0);
			r = group_hero_pos(g_group, h);
			group_set_slot(g_group, r, -1);
		}
		if (g_routes[257].event < 40)
			g_routes[257].event = g_routes[257].event + 1;
	}
}

void tevent_258(Travel far *o)
{
	if (travel_stage(o) == 0)
		ferry_crossing(o, 1, 2);
}

void tevent_259(Travel far *o)
{
	ford_crossing(o);
}
