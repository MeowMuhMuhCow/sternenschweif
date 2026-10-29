/* OVL122 - Overlay O122 (Segment 0x21c4 in SCHWEIF.EXE): Reiseereignisse der Wege 157..190 (Fortsetzung von O121;
 * tevent_run O211; Reiseobjekt o, Ort o->from; Texte g_tev_loc_txt = SECTION3.LTX fuer die Wege bis 174, SECTION6.LTX
 * ab 175, g_tev_txt = ALL_SECT.LTX; Merker route.event in g_routes).
 * tevent_157/162/172/183: Zyklus 1..4, bei 1 deer_hunt. unused_tevent_o122: leer (keinem Weg zugeordnet).
 * tevent_158: Ort 0x35 -> swamp_path(o, 4), sonst Sumpf auf alten Fussstapfen durchquert (txt[57]).
 * tevent_159/tevent_160: Sumpfrantze auf dem Weg (Bildfolge 0x1d2, txt[58]), Merker 0. tevent_161: show_once_anim.
 * tevent_163/166/168/179: Kampf je Richtung (0x4e/0x4f, 0x47/0x48, 0x37/0x38, 0x84/0x85), 163/166 nur gewonnen
 *   Merker 0. tevent_164: place_event_nop (leer).
 * tevent_165: Pflanzenkunde-Probe (route.event[153] * 3 + 5) -> 2 Wirselkraut, 3 Einbeeren, 1 Alraune (txt[59]);
 *   Zaehler [165] bis 41.
 * tevent_167: alter Schaefer (Bild 0x14, txt[60..63]), Gespraech 0x8a (30 Minuten); hat die Gruppe ein Otterfell und
 *   g_music_played == 0: Tausch gegen einen starken Heiltrank; Merker 0.
 * tevent_170/177/190: random_encounter (erstes Mal W6 + 4, sonst zu 60 % W6 + 2). tevent_171: ohne Eilmarsch
 *   "Ihr fuehlt Euch grossartig" (txt[64]) -> o->speed = travel_speed_hurry(o, speed, 0).
 * tevent_174: guide_offer. tevent_175: spring_event. tevent_178: wie tevent_025.
 * tevent_181: nur im Winter (date_season == 0): Schneewehen (SECTION6[5], Bild 0x15) - ohne Schneeschuhe an allen
 *   Fuessen (group_all_have_item(.., ITEM_SNOWSHOES, INV_SLOT_FEET) == -1) umkehren.
 * tevent_182: Luchs lauert auf Bergziegen (Bildfolge 0x1d4, SECTION6[6..10]): abwarten bzw. weitergehen, Merker 0.
 * tevent_185: Pflanzenkunde -> 2 Wirselkraut (ALL_SECT[3]/[5]); Zaehler bis 41.
 * tevent_186/tevent_187: im Winter: Merker == 2 -> Kampf 0x49/0x4a je Richtung; Zyklus 1..4.
 * tevent_188: swamp_path(o, 3).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL122.CPP   -> 3655/3655 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH; spring_event hat keine Parameter, das Original
 *   uebergibt o - Aufruf ueber gecasteten Funktionszeiger)
 * - Aufbau wie OVL121.CPP; -Og noetig (sonst 6 Bytes in tevent_170/tevent_177/tevent_190: Epilogkopien nach return).
 * - new Bmp/delete b (ohne Nullpruefung) in tevent_167, lokales Bmp im if-Block von tevent_181 ("jz short / jmp near"
 *   = if (!date_season(..)) { ... }).
 * - "if (!g_music_played && group_find_item(g_group, ITEM_OTTER_PELT) != -1)" -> mov al/cbw/or bzw. cmp ax,-1 (3D FF
 *   FF).
 * - travel_speed_hurry liefert unsigned short (movzx eax beim Speichern in den long o->speed).
 * Originalfehler: tevent_165 liest die Pflanzenkunde-Stufe aus route.event[153] (Weg 153, O121), zaehlt aber den
 *   eigenen Merker [165] hoch (tevent_185 benutzt korrekt [185] fuer beides). Auffaellig (Kopie?): tevent_159 und
 *   tevent_160 zeigen denselben Text txt[58]; tevent_186 und tevent_187 sind bis auf den Merker identisch.
 */
#include <stdio.h>

#include "schweif.h"

void tevent_157(Travel far *o)
{
	if (g_routes[157].event == 1) {
		deer_hunt(o);
		g_routes[157].event = g_routes[157].event + 1;
		return;
	}
	if (g_routes[157].event >= 4) {
		g_routes[157].event = 1;
		return;
	}
	g_routes[157].event = g_routes[157].event + 1;
}

void tevent_158(Travel far *o)
{
	if (o->from == 0x35) {
		swamp_path(o, 4);
		return;
	}
	/* SECTION3.LTX[57]: "Es gelingt Euch problemlos, den alten Fussstapfen folgend, durch den Sumpf zu gelangen." */
	dlg_message(g_dialog, g_tev_loc_txt[57]);
}

void tevent_159(Travel far *o)
{
	void far *win;

	win = Anim_ctor(0, 0x1d2, 0, 0);
	/* SECTION3.LTX[58]: "Ein eigenartiges, affenaehnliches Tier, jedoch mit scharfen Krallen und gar spitzigen
	 * Zaehnen sitzt auf dem Weg und beobachtet Euch. Das Biest ist unschwer als eine der gefuerchteten Sumpfrantzen
	 * zu erkennen; jedoch sagt man ihnen nach, dass sie nur im Rudel auftreten, dann aber sehr aggressiv seien.
	 * Dieses Exemplar hier beobachtet jedoch nur, um dann wieder im Gebuesch zu verschwinden." */
	dlg_box_anim(g_dialog, win, (char far *)0, g_tev_loc_txt[58], 0);
	Anim_dtor(win, 3);
	g_routes[159].event = 0;
}

void tevent_160(Travel far *o)
{
	void far *win;

	win = Anim_ctor(0, 0x1d2, 0, 0);
	/* SECTION3.LTX[58]: "Ein eigenartiges, affenaehnliches Tier, jedoch mit scharfen Krallen und gar spitzigen
	 * Zaehnen sitzt auf dem Weg und beobachtet Euch. Das Biest ist unschwer als eine der gefuerchteten Sumpfrantzen
	 * zu erkennen; jedoch sagt man ihnen nach, dass sie nur im Rudel auftreten, dann aber sehr aggressiv seien.
	 * Dieses Exemplar hier beobachtet jedoch nur, um dann wieder im Gebuesch zu verschwinden." */
	dlg_box_anim(g_dialog, win, (char far *)0, g_tev_loc_txt[58], 0);
	Anim_dtor(win, 3);
	g_routes[160].event = 0;
}

void tevent_161(Travel far *o)
{
	show_once_anim(o);
}

void tevent_162(Travel far *o)
{
	if (g_routes[162].event == 1) {
		deer_hunt(o);
		g_routes[162].event = g_routes[162].event + 1;
		return;
	}
	if (g_routes[162].event >= 4) {
		g_routes[162].event = 1;
		return;
	}
	g_routes[162].event = g_routes[162].event + 1;
}

void tevent_163(Travel far *o)
{
	signed char r;

	travel_leave(o);
	g_fight_flag = 1;
	if (o->from == 0x87)
		r = start_fight(0x4e, 0);
	else
		r = start_fight(0x4f, 0);
	fight_result(o, r);
	travel_enter(o);
	if (r == 0)
		g_routes[163].event = 0;
}

void tevent_164(Travel far *o)
{
	place_event_nop(o);
}

void tevent_165(Travel far *o)
{
	struct hero far *h;
	signed short r;

	h = get_best_hero_skill(g_group, TA_PFLANZENKUNDE);
#if FIX_0045
	/* Fix #45: Probe mit dem eigenen Zaehler von Weg 165 */
	r = test_skill(h, TA_PFLANZENKUNDE, g_routes[165].event * 3 + 5);
#else
	r = test_skill(h, TA_PFLANZENKUNDE, g_routes[153].event * 3 + 5);
#endif
	if (r > 0) {
		/* SECTION3.LTX[59]: "%s entdeckt am Wegesrand 2 Portionen Wirselkraut, 3 Einbeeren und 1 Alraune!" */
		sprintf(g_text_output_buf, g_tev_loc_txt[59], h->name);
		dlg_message(g_dialog, g_text_output_buf);
		group_give_item(g_group, ITEM_WHIRLWEED, 2);
		group_give_item(g_group, ITEM_EINBEERE, 3);
		group_give_item(g_group, ITEM_MANDRAKE, 1);
	}
#if FIX_0045
	/* Fix #45: Zaehler bis 40 (41 * 3 + 5 = 128 liefe im signed char ueber) */
	if (g_routes[165].event < 40)
		g_routes[165].event++;
#else
	if (g_routes[165].event <= 40)
		g_routes[165].event++;
#endif
}

void tevent_166(Travel far *o)
{
	signed char r;

	travel_leave(o);
	g_fight_flag = 1;
	if (o->from == 0x8a)
		r = start_fight(0x47, 0);
	else
		r = start_fight(0x48, 0);
	fight_result(o, r);
	travel_enter(o);
	if (r == 0)
		g_routes[166].event = 0;
}

void tevent_167(Travel far *o)
{
	signed short dummy;
	signed char ans;
	Bmp far *b;

	b = new Bmp(0x40, 0x40);
	nvfile_load(g_nvf_heads, 0x197, b->data(), 0, 0x14, 0, &dummy, &dummy, 0x140);
	/* SECTION3.LTX[60]: "Ein alter Schaefer sitzt am Wegesrand und strickt mit einem stillen Laecheln an einem
	 * riesigen Strumpf. <Peraine mit Euch!> ruft er Euch gutgelaunt entgegen." */
	dlg_box_bmp(g_dialog, b, (char far *)0, g_tev_loc_txt[60], 0);
	delete b;
	travel_leave(o);
	if (conv_run(g_dialog2, 0x8a))
		timewarp_minutes(g_date, 30L);
	mouse_hide(0);
	travel_enter(o);
	mouse_show(0);
	travelmap_draw(o);
	travel_redraw(o);
	b = new Bmp(0x40, 0x40);
	nvfile_load(g_nvf_heads, 0x197, b->data(), 0, 0x14, 0, &dummy, &dummy, 0x140);
	/* SECTION3.LTX[61]: "<Oh, meine Schafe wollen weiter, und am besten laesst man ihnen ihren Willen. Ich muss dann
	 * wohl mal.>" */
	dlg_box_bmp(g_dialog, b, (char far *)0, g_tev_loc_txt[61], 0);
	if (!g_music_played && group_find_item(g_group, ITEM_OTTER_PELT) != -1) {
		/* SECTION3.LTX[62]: "<Halt, Ihr habt nicht zufaellig ein Otterfell gegen mein Zipperlein dabei? Ich bin naemlich
		 * zu alt, um selber welche zu jagen, und meines ist letzte Tage entzweigegangen.>" */
		/* ALL_SECT.LTX[1]: "Ja" */
		/* ALL_SECT.LTX[2]: "Nein" */
		ans = dlg_box_bmp(g_dialog, b, (char far *)0, g_tev_loc_txt[62], 2, g_tev_txt[1], g_tev_txt[2]);
		if (ans == 1) {
			group_drop_item(g_group, ITEM_OTTER_PELT);
			/* SECTION3.LTX[63]: "<Das ist ja wundervoll! Habt meinen Dank. Hier, nehmt das; ein kraeftiger Kraeutertrunk.
			 * Den habe ich selber gemacht; ist gut gegen allerlei.>" */
			dlg_box_bmp(g_dialog, b, (char far *)0, g_tev_loc_txt[63], 0);
			group_give_item(g_group, ITEM_STRONG_LE_POTION, 1);
		}
	}
	delete b;
	g_routes[167].event = 0;
}

void tevent_168(Travel far *o)
{
	signed char r;

	travel_leave(o);
	g_fight_flag = 1;
	if (o->from == 0x8b)
		r = start_fight(0x37, 0);
	else
		r = start_fight(0x38, 0);
	fight_result(o, r);
	travel_enter(o);
	g_routes[168].event = 0;
}

void tevent_170(Travel far *o)
{
	signed char n = 0;

	if (g_routes[170].event == 1) {
		g_routes[170].event++;
		n = random_schweif(6) + 4;
		if (o->from == 0x2b)
			random_encounter(o, n, 1);
		else
			random_encounter(o, n, 2);
		return;
	}
	n = random_schweif(10);
	if (n <= 5) {
		n = random_schweif(6) + 2;
		if (o->from == 0x2b)
			random_encounter(o, n, 1);
		else
			random_encounter(o, n, 2);
	}
}

void tevent_171(Travel far *o)
{
	if (o->hurry != 1) {
		/* SECTION3.LTX[64]: "Ihr fuehlt Euch einfach grossartig. All die widrigen Kleinigkeiten einer solchen Reise
		 * koennen Euch ueberhaupt nichts anhaben, so wie Ihr Euch fuehlt." */
		dlg_message(g_dialog, g_tev_loc_txt[64]);
		o->speed = travel_speed_hurry(o, o->speed, 0);
	}
}

void tevent_172(Travel far *o)
{
	if (g_routes[172].event == 1) {
		deer_hunt(o);
		g_routes[172].event = g_routes[172].event + 1;
		return;
	}
	if (g_routes[172].event >= 4) {
		g_routes[172].event = 1;
		return;
	}
	g_routes[172].event = g_routes[172].event + 1;
}

void unused_tevent_o122(Travel far *o)
{
}

void tevent_174(Travel far *o)
{
	if (o->from == 0x1b)
		guide_offer(o, 4);
	else
		guide_offer(o, 3);
	if (g_guide_won != 0)
		g_routes[174].event = 0;
}

void tevent_175(Travel far *o)
{
	((void (*)(Travel far *))spring_event)(o);	/* Original uebergibt o, spring_event hat keine Parameter */
}

void tevent_177(Travel far *o)
{
	signed char n = 0;

	if (g_routes[177].event == 1) {
		g_routes[177].event++;
		n = random_schweif(6) + 4;
		if (o->from == 0x91)
			random_encounter(o, n, 1);
		else
			random_encounter(o, n, 2);
		return;
	}
	n = random_schweif(10);
	if (n <= 5) {
		n = random_schweif(6) + 2;
		if (o->from == 0x91)
			random_encounter(o, n, 1);
		else
			random_encounter(o, n, 2);
	}
}

void tevent_178(Travel far *o)
{
	tevent_025(o);
}

void tevent_179(Travel far *o)
{
	signed char r;

	travel_leave(o);
	g_fight_flag = 1;
	if (o->from == 0x93)
		r = start_fight(0x84, 0);
	else
		r = start_fight(0x85, 0);
	fight_result(o, r);
	travel_enter(o);
	g_routes[179].event = 0;
}

void tevent_181(Travel far *o)
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
		c = group_all_have_item(g_group, ITEM_SNOWSHOES, INV_SLOT_FEET);
		if (c == -1) {
			o->action = 1;
			o->ev_half = 0;
		}
	}
}

void tevent_182(Travel far *o)
{
	signed char sel;
	void far *win;

	win = Anim_ctor(0, 0x1d4, 0, 0);
	/* SECTION6.LTX[6]: "Ihr entdeckt einen Luchs, der einige hundert Schritt vor Euch auf der Lauer liegt. Von der
	 * anderen Seite allerdings naehern sich einige Bergziegen. Wollt Ihr" */
	/* SECTION6.LTX[7]: "abwarten, bis das Raubtier Beute gemacht hat oder" */
	/* SECTION6.LTX[8]: "weitergehen und es auf einen Kampf ankommen lassen?" */
	sel = dlg_box_anim(g_dialog, win, (char far *)0, g_tev_loc_txt[6], 2, g_tev_loc_txt[7], g_tev_loc_txt[8]);
	Anim_dtor(win, 3);
	switch (sel) {
	case 1:
		/* SECTION6.LTX[9]: "Nach einiger Zeit zerreisst ein schriller Todesschrei die Stille. Der Luchs hat sein Opfer
		 * gefunden..." */
		dlg_message(g_dialog, g_tev_loc_txt[9]);
		break;
	case 2:
		/* SECTION6.LTX[10]: "Der Luchs ist schlau genug um zu wissen, dass er gegen Euch keine Chance hat, und
		 * verschwindet mit ein paar Saetzen hinter den naechsten Felsen." */
		dlg_message(g_dialog, g_tev_loc_txt[10]);
		break;
	}
	g_routes[182].event = 0;
}

void tevent_183(Travel far *o)
{
	if (g_routes[183].event == 1) {
		deer_hunt(o);
		g_routes[183].event = g_routes[183].event + 1;
		return;
	}
	if (g_routes[183].event >= 4) {
		g_routes[183].event = 1;
		return;
	}
	g_routes[183].event = g_routes[183].event + 1;
}

void tevent_185(Travel far *o)
{
	struct hero far *h;
	signed short r;

	h = get_best_hero_skill(g_group, TA_PFLANZENKUNDE);
	r = test_skill(h, TA_PFLANZENKUNDE, g_routes[185].event * 3 + 5);
	if (r > 0) {
		/* ALL_SECT.LTX[3]: "%s entdeckt am Wegesrand %d Portion%s Wirselkraut!" */
		/* ALL_SECT.LTX[5]: "en" */
		sprintf(g_text_output_buf, g_tev_txt[3], h->name, 2, g_tev_txt[5]);
		dlg_message(g_dialog, g_text_output_buf);
		group_give_item(g_group, ITEM_WHIRLWEED, 2);
	}
	if (g_routes[185].event <= 40)
		g_routes[185].event++;
}

void tevent_186(Travel far *o)
{
	signed char r;

	if (!date_season(g_date)) {
		if (g_routes[186].event == 2) {
			travel_leave(o);
			g_fight_flag = 1;
			if (o->from == 0x8d)
				r = start_fight(0x49, 0);
			else
				r = start_fight(0x4a, 0);
			fight_result(o, r);
			travel_enter(o);
			return;
		}
		if (g_routes[186].event >= 4) {
			g_routes[186].event = 1;
			return;
		}
		g_routes[186].event = g_routes[186].event + 1;
	}
}

void tevent_187(Travel far *o)
{
	signed char r;

	if (!date_season(g_date)) {
		if (g_routes[187].event == 2) {
			travel_leave(o);
			g_fight_flag = 1;
			if (o->from == 0x8d)
				r = start_fight(0x49, 0);
			else
				r = start_fight(0x4a, 0);
			fight_result(o, r);
			travel_enter(o);
			return;
		}
		if (g_routes[187].event >= 4) {
			g_routes[187].event = 1;
			return;
		}
		g_routes[187].event = g_routes[187].event + 1;
	}
}

void tevent_188(Travel far *o)
{
	swamp_path(o, 3);
}

void tevent_190(Travel far *o)
{
	signed char n = 0;

	if (g_routes[190].event == 1) {
		g_routes[190].event++;
		n = random_schweif(6) + 4;
		if (o->from == 0x98)
			random_encounter(o, n, 1);
		else
			random_encounter(o, n, 2);
		return;
	}
	n = random_schweif(10);
	if (n <= 5) {
		n = random_schweif(6) + 2;
		if (o->from == 0x98)
			random_encounter(o, n, 1);
		else
			random_encounter(o, n, 2);
	}
}
