/* OVL123 - Overlay O123 (Segment 0x21d0 in SCHWEIF.EXE): Reiseereignisse der Wege 191..221 (tevent_run O211;
 * Reiseobjekt o, Ort o->from; Texte g_tev_loc_txt = SECTION6.LTX (Datei 0x143, Wege ab 175), g_tev_txt =
 * ALL_SECT.LTX; Merker route.event in g_routes; Bilder 64x64 per nvfile_load aus g_nvf_heads, Datei 0x197).
 * Fortsetzung von O122, Geschwister O124.
 * tevent_191/tevent_218: Fuehrer-Angebot guide_offer(o, 3/4 bzw. 1/2 je Ausgangsort 0x48 bzw. 0x1d); g_guide_won ->
 *   Ereignis aus. tevent_192/tevent_214: jedes vierte Mal (Merker zyklisch 1..4) die Rehjagd deer_hunt (O163;
 *   ALL_SECT[80..85]).
 * tevent_193: Gelaechter rauher Kehlen (SECTION6[11..17]): anschleichen (Schleichen-Probe des gewaehlten Helden mit
 *   RS-Bonus * 2 + 5; misslungen -> entdeckt), weitergehen oder hinmarschieren; beim Anschleichen Bild 0x2e und
 *   Lauschen (Bewaffnete im Auftrag Sudran Alatzers suchen Euch) -> Ueberfall (Kampf 0x4d, Helden beginnen) oder
 *   Abstand gewinnen; entdeckt bzw. hinmarschiert -> Kampf 0x4d, Gegner beginnen.
 * tevent_195/tevent_205/tevent_211: Zufallsbegegnung random_encounter (beim ersten Mal W6 + 4 Gegner, sonst bei W10
 *   <= 5 W6 + 2; Begegnungsart 1/2 bzw. 3/4 je Ausgangsort).
 * tevent_198: Borkenbaer im Tal (Bild 7, SECTION6[18..25]): stellen (Koerperbeherrschung-Schlechtester bekommt den
 *   Schlag, LE-abhaengiger Schaden), fliehen (Koerperbeherrschung-Schlechtester stolpert: 4 LE) oder Suessigkeiten
 *   hinwerfen (mit Lakritze/Bonbons gelingt die Flucht und alle Suessigkeiten sind weg, sonst bekommt der Held mit
 *   der kleinsten KL die Pruegel); beim Stellen schreit der Held mit dem kleinsten MU.
 * tevent_200/tevent_202/tevent_213/tevent_221: Kampf (0x54/0x55, 0x37/0x38, 0x56/0x57 je Richtung) zwischen
 *   travel_leave/travel_enter (fight_result), teils mit g_fight_max_enemies = 1; Ereignis aus.
 * tevent_201: Wirselkraut am Wegesrand (Pflanzenkunde-Probe, ALL_SECT[3]/[4]); Zaehler bis 41.
 * tevent_204: Frau in gruenem Leder mit Bogen (Bild 0x16, SECTION6[42..45]): zu den Waffen greifen -> Schuss in den
 *   Bauch (Held mit kleinstem RS-Bonus, W6 + 4 LE, 1 Stunde), antworten -> sie laesst Euch ziehen (Dialog-Skript
 *   0x8b, ggf. 30 Minuten; Karte neu).
 * tevent_209: verschneites Gebirge (nur im Winter, date_season == 0; Bild 0x15, SECTION6[5]): Held waehlen, ohne
 *   Schneeschuhe fuer alle Helden umkehren (o->action = 1).
 * tevent_217: im Winter Kampf 0x56/0x57 (g_fight_max_enemies = 5) beim Merkerstand 2 (zyklisch 1..4).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL123.CPP -> 3792/3792 Bytes GLEICH (18.10.2026), cobjfix laeuft (Namensfassung mit
 *   schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Aufbau wie OVL121/OVL122 (wovt); -Og noetig (Epilogkopien, h->name als -Og-Zwischenwert: in tevent_198 in SI, in
 *   tevent_204 ohne register-Variablen auf dem Stack [bp-0eh], (int)rnd in DI, (int)le in [bp-1eh]).
 * - LE +0x72 (int) in signed-char-Local: nur ueber den inline-Getter hero::get_le() "mov ax,es:[bx+72h] / mov
 *   [bp-8],al"; direkt "le = h->le" ergibt mov al.
 * - tevent_193: switch-Ruempfe in der Reihenfolge case 3 (state = 1, faellt durch), case 2 (break), case 1.
 * - tevent_198: lokales Bmp b(w, h) erst nach nvfile_info deklariert (Konstruktor an der Deklarationsstelle,
 *   Destruktor (&b, 2) am Ende); switch-Ruempfe case 2, case 1, case 3.
 * Originalfehler: tevent_201 liest die Probenstufe aus g_routes[3].event (Merker eines fremden Weges, wie tevent_127
 *   in O120), zaehlt aber den eigenen Merker [201] hoch. Auffaellig: tevent_193 setzt r nur fuer state 1/2 (sonst
 *   nicht benutzt), tevent_213 prueft wie tevent_211 auf Ort 0xa9 (Kopie?).
 */
#include <stdio.h>

#include "schweif.h"

void tevent_191(Travel far *o)
{
	if (o->from == 0x48)
		guide_offer(o, 3);
	else
		guide_offer(o, 4);
	if (g_guide_won != 0)
		g_routes[191].event = 0;
}

void tevent_192(Travel far *o)
{
	if (g_routes[192].event == 1) {
		deer_hunt(o);
		g_routes[192].event = g_routes[192].event + 1;
		return;
	}
	if (g_routes[192].event >= 4) {
		g_routes[192].event = 1;
		return;
	}
	g_routes[192].event = g_routes[192].event + 1;
}

void tevent_193(Travel far *o)
{
	signed char sel;
	signed char r;
	signed char state = 0;
	signed short dummy;
	signed short res;
	struct hero far *h;
	Bmp far *b;

	/* SECTION6.LTX[11]: "Ploetzlich hoert Ihr irgendwo rechts von Euch lautes Gelaechter aus zahlreichen, rauhen
	 * Kehlen. Wollt Ihr" */
	/* SECTION6.LTX[12]: "Euch anschleichen und nachsehen?" */
	/* SECTION6.LTX[13]: "Weitergehen oder" */
	/* SECTION6.LTX[14]: "einfach hinmarschieren?" */
	sel = dlg_choose(g_dialog, g_tev_loc_txt[11], 3, g_tev_loc_txt[12], g_tev_loc_txt[13], g_tev_loc_txt[14]);
	if (sel == -1)
		sel = 3;
	switch (sel) {
	case 3:
		state = 1;
	case 2:
		break;
	case 1:
		preselect_best_skill(g_group, TA_SCHLEICHEN);
		/* ALL_SECT.LTX[84]: "Wer will es versuchen?" */
		h = get_active_hero(g_group, g_tev_txt[84]);
		res = test_skill(h, TA_SCHLEICHEN, h->rs_bonus * 2 + 5);
		if (res < 0)
			state = 1;
		else {
			b = new Bmp(0x40, 0x40);
			nvfile_load(g_nvf_heads, 0x197, b->data(), 0, 0x2e, 0, &dummy, &dummy, 0x140);
			/* SECTION6.LTX[15]: "Ein Haufen Bewaffneter sitzt im Kreis und unterhaelt sich lautstark ueber die taktischen
			 * Probleme, die man hat, wenn man Jemanden durch ein Gebirge verfolgen muss. In ihrem Gespraech wird auch
			 * mehrmals der Name Sudran Alatzer erwaehnt; als Auftraggeber.@Aber zweierlei duerfte Euch besonders
			 * interessieren:@Sie koennen nicht das ganze Gebirge abriegeln und sie suchen Euch.@@Wollt Ihr" */
			/* SECTION6.LTX[16]: "einen Ueberraschungsangriff unternehmen" */
			/* SECTION6.LTX[17]: "oder die Gelegenheit nutzen, um Abstand zwischen Euch und die Verfolger zu bringen?" */
			sel = dlg_box_bmp(g_dialog, b, (char far *)0, g_tev_loc_txt[15], 2, g_tev_loc_txt[16],
				g_tev_loc_txt[17]);
			delete b;
			if (sel == 1)
				state = 2;
		}
		break;
	}
	if (state > 0) {
		travel_leave(o);
		g_fight_flag = 1;
		if (state == 2)
			r = start_fight(0x4d, 2);
		if (state == 1)
			r = start_fight(0x4d, 0);
		fight_result(o, r);
		travel_enter(o);
	}
	g_routes[193].event = 0;
}

void tevent_195(Travel far *o)
{
	signed char n = 0;

	if (g_routes[195].event == 1) {
		g_routes[195].event++;
		n = random_schweif(6) + 4;
		if (o->from == 0x2e)
			random_encounter(o, n, 1);
		else
			random_encounter(o, n, 2);
		return;
	}
	n = random_schweif(10);
	if (n <= 5) {
		n = random_schweif(6) + 2;
		if (o->from == 0x2e)
			random_encounter(o, n, 1);
		else
			random_encounter(o, n, 2);
	}
}

void tevent_198(Travel far *o)
{
	signed short w;
	signed short hh;
	signed short dummy;
	signed char sel = -1;
	signed char le;
	signed char rnd;
	struct hero far *h;
	struct hero far *h2;

	nvfile_info(g_nvf_heads, 0x196, 7, &w, &hh);
	Bmp b(w, hh);
	nvfile_load(g_nvf_heads, 0x196, b.data(), 0, 7, 0, &dummy, &dummy, 0x140);
	g_dlg_box_flag = 0;
	while (sel == -1)
		/* SECTION6.LTX[18]: "Als Ihr ein kleines Tal durchschreitet, kommt ein riesenhaftes Geschoepf auf Euch zu. Erst
		 * als es nur wenige Schritt vor Euch stehen bleibt, koennt Ihr erkennen, dass es sich hier um einen
		 * ausgewachsenen, Borkenbaeren handelt. Was wollt Ihr tun?" */
		/* SECTION6.LTX[19]: "Euch dem Kampf stellen," */
		/* SECTION6.LTX[20]: "laufen, was der Koerper hergibt, oder" */
		/* SECTION6.LTX[21]: "dem Baeren Suessigkeiten hinwerfen und dann versuchen unbemerkt zu entkommen." */
		sel = dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[18], 3, g_tev_loc_txt[19],
			g_tev_loc_txt[20], g_tev_loc_txt[21]);
	switch (sel) {
	case 2:
		h = get_worst_hero_skill(g_group, TA_KOERPERBEHERRSCHUNG);
		/* SECTION6.LTX[23]: "Die Flucht gelingt Euch gerade eben, %s stolpert jedoch noch und zieht sich einige Kratzer
		 * zu." */
		sprintf(g_text_output_buf, g_tev_loc_txt[23], h->name);
		dlg_message(g_dialog, g_text_output_buf);
		sub_hero_le(h, 4);
		break;
	case 1:
		h = get_worst_hero_skill(g_group, TA_KOERPERBEHERRSCHUNG);
		h2 = get_hero_attrib_min(g_group, ATTRIB_MU);
		g_dlg_box_flag = 0;
		/* SECTION6.LTX[22]: "Ihr seid zwar gut geruestet fuer einen Kampf, aber der Baer hat enorme Vorteile durch seine
		 * Groesse und so verpasst er %s einen maechtigen Schlag. Erst als %s einen Mark und Bein erschuetternden Schrei
		 * loslaesst, ergreift der Baer verwirrt die Flucht." */
		sprintf(g_text_output_buf, g_tev_loc_txt[22], h->name, h2->name);
		dlg_box_bmp(g_dialog, &b, (char far *)0, g_text_output_buf, 0);
		le = h->get_le();
		rnd = random_schweif(10) + 6;
		if (rnd >= le + 5)
			sub_hero_le(h, le - 4);
		else
			sub_hero_le(h, rnd);
		break;
	case 3:
		if (group_find_item(g_group, ITEM_LIQUORICE) == -1 && group_find_item(g_group, ITEM_SWEETS) == -1) {
			h = get_hero_attrib_min(g_group, ATTRIB_KL);
			g_dlg_box_flag = 0;
			/* SECTION6.LTX[25]: "Ihr haettet wissen sollen, dass Ihr ueber keine Suessigkeiten verfuegt und so habt Ihr
			 * wertvolle Zeit damit verbracht in Euren Taschen zu suchen. Der Baer drischt nun mittlerweile auf %s ein und
			 * hoert auch erst damit auf, als er merkt, dass er von den anderen Helden angegriffen wird. Als er seine
			 * Siegeschancen schwinden sieht, gibt er %s noch einen enormen Hieb um sich freie Bahn fuer seine Flucht zu
			 * schaffen und trottet davon." */
			sprintf(g_text_output_buf, g_tev_loc_txt[25], h->name, h->name);
			dlg_box_bmp(g_dialog, &b, (char far *)0, g_text_output_buf, 0);
			le = h->get_le();
			rnd = random_schweif(20) + 8;
			if (rnd >= le + 2)
				sub_hero_le(h, rnd);
			else
				sub_hero_le(h, le - 2);
		} else {
			/* SECTION6.LTX[24]: "Ihr koennt den Baeren fuer einige Sekunden ablenken und dadurch die Flucht ergreifen." */
			dlg_message(g_dialog, g_tev_loc_txt[24]);
			while (group_find_item(g_group, ITEM_LIQUORICE) != -1)
				group_drop_item(g_group, ITEM_LIQUORICE);
			while (group_find_item(g_group, ITEM_SWEETS) != -1)
				group_drop_item(g_group, ITEM_SWEETS);
		}
		break;
	}
	g_routes[198].event = 0;
}

void tevent_200(Travel far *o)
{
	signed char r;

	travel_leave(o);
	g_fight_flag = 1;
	if (o->from == 0x9f)
		r = start_fight(0x55, 1);
	else
		r = start_fight(0x54, 1);
	fight_result(o, r);
	travel_enter(o);
	g_routes[200].event = 0;
}

void tevent_201(Travel far *o)
{
	struct hero far *h;
	signed short r;

	h = get_best_hero_skill(g_group, TA_PFLANZENKUNDE);
#if FIX_0063
	/* Fix #63: Probe mit dem eigenen Zaehler von Weg 201 */
	r = test_skill(h, TA_PFLANZENKUNDE, g_routes[201].event * 3 + 5);
#else
	r = test_skill(h, TA_PFLANZENKUNDE, g_routes[3].event * 3 + 5);
#endif
	if (r > 0) {
		/* ALL_SECT.LTX[3]: "%s entdeckt am Wegesrand %d Portion%s Wirselkraut!" */
		/* ALL_SECT.LTX[4]: "" */
		sprintf(g_text_output_buf, g_tev_txt[3], h->name, 1, g_tev_txt[4]);
		dlg_message(g_dialog, g_text_output_buf);
		group_give_item(g_group, ITEM_WHIRLWEED, 1);
	}
#if FIX_0063
	/* Fix #63: Zaehler bis 40 (41 * 3 + 5 = 128 liefe im signed char ueber) */
	if (g_routes[201].event < 40)
		g_routes[201].event++;
#else
	if (g_routes[201].event <= 40)
		g_routes[201].event++;
#endif
}

void tevent_202(Travel far *o)
{
	signed char r;

	g_fight_max_enemies = 1;
	travel_leave(o);
	g_fight_flag = 1;
	if (o->from == 0x9e)
		r = start_fight(0x37, 0);
	else
		r = start_fight(0x38, 0);
	fight_result(o, r);
	travel_enter(o);
	g_routes[202].event = 0;
}

void tevent_204(Travel far *o)
{
	signed short dummy;
	signed char ans;
	struct hero far *h;
	Bmp far *b;

	b = new Bmp(0x40, 0x40);
	nvfile_load(g_nvf_heads, 0x197, b->data(), 0, 0x16, 0, &dummy, &dummy, 0x140);
	/* SECTION6.LTX[42]: "<Keine Bewegung, oder ich schiesse Euch in den Bauch. Man sagt, es soll lange dauern, bis
	 * man stirbt, und erst ganz am Schluss soll man aufhoeren. Zu schreien, meine Ich. Was wollt Ihr hier?> Die Frau,
	 * ganz in gruenem Leder, steht halb von einem Baum verdeckt und beobachtet Euch scharf. Wollt Ihr" */
	/* SECTION6.LTX[43]: "vorsichtig zu Euren Waffen greifen, oder" */
	/* SECTION6.LTX[44]: "Ihr antworten: <Wir sind nur harmlose Wanderer>?" */
	ans = dlg_box_bmp(g_dialog, b, (char far *)0, g_tev_loc_txt[42], 2, g_tev_loc_txt[43], g_tev_loc_txt[44]);
	delete b;
	if (ans == 2) {
		travel_leave(o);
		if (conv_run(g_dialog2, 0x8b))
			timewarp_minutes(g_date, 30L);
		mouse_hide(0);
		travel_enter(o);
		mouse_show(0);
		travelmap_draw(o);
		travel_redraw(o);
	} else {
		h = get_hero_rs_bonus_min(g_group);
		/* SECTION6.LTX[45]: "Mit einem gekonnten Schuss hat die Frau ihre Warnung wahr gemacht, und %s in den Bauch
		 * geschossen.@Dann verschwindet sie mit einem leisen Rascheln im Dickicht.@%s hat es zum Glueck nicht zu schwer
		 * erwischt und nach kurzer Behandlung koennt Ihr die Reise fortsetzen." */
		sprintf(g_text_output_buf, g_tev_loc_txt[45], h->name, h->name);
		dlg_message(g_dialog, g_text_output_buf);
		sub_hero_le(h, random_schweif(6) + 4);
		timewarp_hours(g_date, 1L);
	}
	g_routes[204].event = 0;
}

void tevent_205(Travel far *o)
{
	signed char n = 0;

	if (g_routes[205].event == 1) {
		g_routes[205].event++;
		n = random_schweif(6) + 4;
		if (o->from == 0xa4)
			random_encounter(o, n, 3);
		else
			random_encounter(o, n, 4);
		return;
	}
	n = random_schweif(10);
	if (n <= 5) {
		n = random_schweif(6) + 2;
		if (o->from == 0xa4)
			random_encounter(o, n, 3);
		else
			random_encounter(o, n, 4);
	}
}

void tevent_209(Travel far *o)
{
	signed short dummy;
	signed char c;

	if (!date_season(g_date)) {
		Bmp b(0x40, 0x40);

		nvfile_load(g_nvf_heads, 0x197, b.data(), 0, 0x15, 0, &dummy, &dummy, 0x140);
		/* SECTION6.LTX[5]: "Das Gebirge ist tief verschneit, und vor Euch hat der Wind einige Schneewehen aufgehaeuft,
		 * die ohne Schneeschuhe nicht zu passieren sind." */
		dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[5], 0);
		select_hero_bar(o);
		c = group_all_have_item(g_group, ITEM_SNOWSHOES, 0x0e);
		if (c == -1) {
			o->action = 1;
			o->ev_half = 0;
		}
	}
}

void tevent_211(Travel far *o)
{
	signed char n = 0;

	if (g_routes[211].event == 1) {
		g_routes[211].event++;
		n = random_schweif(6) + 4;
		if (o->from == 0xa9)
			random_encounter(o, n, 3);
		else
			random_encounter(o, n, 4);
		return;
	}
	n = random_schweif(10);
	if (n <= 5) {
		n = random_schweif(6) + 2;
		if (o->from == 0xa9)
			random_encounter(o, n, 3);
		else
			random_encounter(o, n, 4);
	}
}

void tevent_213(Travel far *o)
{
	signed char r;

	g_fight_max_enemies = 1;
	travel_leave(o);
	g_fight_flag = 1;
	if (o->from == 0xa9)
		r = start_fight(0x37, 0);
	else
		r = start_fight(0x38, 0);
	fight_result(o, r);
	travel_enter(o);
	g_routes[213].event = 0;
}

void tevent_214(Travel far *o)
{
	if (g_routes[214].event == 1) {
		deer_hunt(o);
		g_routes[214].event = g_routes[214].event + 1;
		return;
	}
	if (g_routes[214].event >= 4) {
		g_routes[214].event = 1;
		return;
	}
	g_routes[214].event = g_routes[214].event + 1;
}

void tevent_217(Travel far *o)
{
	signed char r;

	if (!date_season(g_date)) {
		if (g_routes[217].event == 2) {
			g_fight_max_enemies = 5;
			travel_leave(o);
			g_fight_flag = 1;
			if (o->from == 0xa2)
				r = start_fight(0x56, 0);
			else
				r = start_fight(0x57, 0);
			fight_result(o, r);
			travel_enter(o);
			return;
		}
		if (g_routes[217].event >= 4) {
			g_routes[217].event = 1;
			return;
		}
		g_routes[217].event = g_routes[217].event + 1;
	}
}

void tevent_218(Travel far *o)
{
	if (o->from == 0x1d)
		guide_offer(o, 1);
	else
		guide_offer(o, 2);
	if (g_guide_won != 0)
		g_routes[218].event = 0;
}

void tevent_221(Travel far *o)
{
	signed char r;

	g_fight_max_enemies = 1;
	travel_leave(o);
	g_fight_flag = 1;
	if (o->from == 0xad)
		r = start_fight(0x37, 0);
	else
		r = start_fight(0x38, 0);
	fight_result(o, r);
	travel_enter(o);
	g_routes[221].event = 0;
}
