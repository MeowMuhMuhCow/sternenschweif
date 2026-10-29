/* OVL120 - Overlay O120 (Segment 0x21b6 in SCHWEIF.EXE): Reiseereignisse der Wege 121..138 (tevent_run O211;
 *   Reiseobjekt
 * o, Ort o->from; Texte g_tev_loc_txt = SECTION3.LTX, fuer Weg 136 (0x88) SECTION2.LTX, g_tev_txt = ALL_SECT.LTX;
 *   Merker
 * route.event in g_routes). tevent_121/125/131 -> valley_ambush, tevent_132 -> spring_event (mit o, s. u.).
 * tevent_122: mountain_rope, dann tevent_123, wenn die Gruppe den Salamanderstein hat und g_salamander_reported == 0.
 * tevent_123: unheimliche Gestalt auf dem Felsen (Bild 0x197/0x11 in lokales Bmp, txt[27]), route.event[123] = 0,
 *   Tagebuch ALL_SECT[122]. tevent_126/128/129/133: Kampf 0x3d/0x3e, 0x42/0x41, 0x3f/0x40, 0x41/0x42 je Richtung
 *   zwischen travel_leave/travel_enter (g_fight_flag = 1, teils g_fight_max_enemies = 3), Ereignis aus.
 * tevent_127: Pflanzenkunde-Probe (route.event[3] * 3 + 5) > 0 -> 3 Wirselkraut (ALL_SECT[3]/[5]); Zaehler [127]
 *   bis 41. tevent_134/tevent_137: Fuehrer-Angebot (Orte 0x77/0x26, travel_stage == 2) solange route.event != 60;
 *   g_guide_won -> 60 (tevent_134 danach show_once_anim).
 * tevent_135: Ort 0x79 -> guide_offer(o, 4); bei route.event[135] == 50 gebeugte Gestalt (Bilder 0x13/0x12, new Bmp),
 *   Gespraech 0x89 (30 Minuten), Merker = 60, Angebot txt[10] (31..33): 2 -> Zweihaender fuer 1500 Heller, 3 -> fuer
 *   10 Heller als NSC 7 anheuern (add_npc), sonst GLOBLTXT[4] (zu wenig Geld).
 * tevent_136: bei route.event[136] == 2 Steppenrinder (Bild 4, txt[13..16]): schiessen - mit Kurz-/Langbogen und
 *   Pfeilen oder Armbrust und Bolzen txt[18], sonst Rueckfrage txt[17]; anpirschen -> Herde rennt die Gruppe um
 *   (group_sub_le_dice(W20), 30 AP); Merker zyklisch 1..5.
 * tevent_138: Ort 0x7a -> guide_offer(o, 2); bei travel_stage == 0 einmalig (Merker [138] == 50 -> 51)
 *   random_encounter(o, W6 + 4, 1/2), sonst zu 50 % random_encounter(o, W6 + 2, 1/2).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y -E..\bc\BIN\TASM.EXE OVL120.CPP   -> 2959/2959 Bytes GLEICH (18.10.2026),
 *   cobjfix laeuft. Ohne -Og 1 Byte zu lang (Epilogkopien in tevent_135/tevent_136/tevent_138).
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH; spring_event hat keine Parameter, das Original
 *   uebergibt o - Aufruf ueber gecasteten Funktionszeiger)
 * - C++ wegen der R42-Bildklasse (lokales Bmp mit Destruktor (.., 2); new Bmp / delete (.., 3)) und
 *   nvfile_load (g_nvf_heads).
 * - Merker als g_routes[n].event (+4d6 = [123].event usw.); "f8 = f8 + 1" bzw. "f8 += 1" -> mov al / inc al / les /
 *   mov (Zeiger neu geladen), "f8++" -> inc byte.
 * - tevent_138: "travel_stage(o) == 0" (char-Rueckgabe) -> or al,al; "!travel_stage(o)" ergaebe cbw / or ax,ax.
 * - tevent_136: "ans = r = -1" (r zuerst), Bedingung "((A || B) && C) || (D && E)", case 1 faellt in case 2 durch
 *   (kein break), "case 3: break;" ohne default; "default: break;" ergaebe einen zusaetzlichen Sprung.
 * - tevent_135: switch ohne default -> Epilogkopie nach den Vergleichen; Konstanten 1500L/10L/30L als long.
 * - Originalfehler/Auffaelliges: tevent_135 "sprintf(g_text_output_buf, g_tev_loc_txt[10])" ohne Wirkung (die Frage
 *   benutzt den Text
 *   direkt); tevent_136 case 1 ohne break (gewollt oder nicht, so im Original); tevent_123 prueft die Bedingung aus
 *   tevent_122 nochmals; tevent_138 initialisiert r = 0 unnoetig.
 */
#include <stdio.h>

#include "schweif.h"

void tevent_121(Travel far *o)
{
	valley_ambush(o);
}

void tevent_122(Travel far *o)
{
	mountain_rope(o);
	if (group_find_item(g_group, ITEM_SALAMANDER_STONE) >= 0 && g_salamander_reported == 0)
		tevent_123(o);
}

void tevent_123(Travel far *o)
{
	signed short w;

	if (group_find_item(g_group, ITEM_SALAMANDER_STONE) >= 0 && g_salamander_reported == 0) {
		Bmp b(0x40, 0x40);
		nvfile_load(g_nvf_heads,0x197, b.data(), 0, 0x11, 0, &w, &w, 0x140);
		/* SECTION3.LTX[27]: "Eine dunkle Silhouette hebt sich ueber Euch gegen den Himmel ab: Ein Mann steht dort und
		 * starrt auf Euch herunter. <Nach Lowangen> ruft er herunter, <bringt es nach Lowangen, und Ihr werdet reich
		 * belohnt werden!> dann verschwindet er." */
		dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[27], 0);
		g_routes[123].event = 0;
		/* ALL_SECT.LTX[122]: "Wir sind heute einer etwas unheimlichen Gestalt begegnet. Es schien ein Mann zu sein, mehr
		 * konnten wir nicht erkennen. Die Gestalt forderte uns auf, <es> nach Lowangen zu bringen. Was damit gemeint
		 * war, koennen wir hoechstens erraten." */
		diary_add(g_diary, g_tev_txt[122], 0);
	}
}

void tevent_125(Travel far *o)
{
	valley_ambush(o);
}

void tevent_126(Travel far *o)
{
	signed char r;

	travel_leave(o);
	g_fight_flag = 1;
	if (o->from == 0x5a)
		r = start_fight(0x3d, 0);
	else
		r = start_fight(0x3e, 0);
	fight_result(o, r);
	travel_enter(o);
	g_routes[126].event = 0;
}

void tevent_127(Travel far *o)
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
	if (g_routes[127].event <= 40)
		g_routes[127].event++;
}

void tevent_128(Travel far *o)
{
	signed char r;

	g_fight_max_enemies = 3;
	travel_leave(o);
	g_fight_flag = 1;
	if (o->from == 0x74)
		r = start_fight(0x42, 0);
	else
		r = start_fight(0x41, 0);
	fight_result(o, r);
	travel_enter(o);
	g_routes[128].event = 0;
}

void tevent_129(Travel far *o)
{
	signed char r;

	g_fight_max_enemies = 3;
	travel_leave(o);
	g_fight_flag = 1;
	if (o->from == 0x75)
		r = start_fight(0x3f, 0);
	else
		r = start_fight(0x40, 0);
	fight_result(o, r);
	travel_enter(o);
	g_routes[129].event = 0;
}

void tevent_131(Travel far *o)
{
	valley_ambush(o);
}

void tevent_132(Travel far *o)
{
	((void (*)(Travel far *))spring_event)(o);	/* Original uebergibt o, spring_event hat keine Parameter */
}

void tevent_133(Travel far *o)
{
	signed char r;

	g_fight_max_enemies = 3;
	travel_leave(o);
	g_fight_flag = 1;
	if (o->from == 0x76)
		r = start_fight(0x41, 0);
	else
		r = start_fight(0x42, 0);
	fight_result(o, r);
	travel_enter(o);
	g_routes[133].event = 0;
}

void tevent_134(Travel far *o)
{
	if (o->from == 0x77 && travel_stage(o) == 2 && g_routes[134].event != 0x3c)
		guide_offer(o, 3);
	if (g_guide_won != 0)
		g_routes[134].event = 0x3c;
	show_once_anim(o);
}

void tevent_135(Travel far *o)
{
	signed short w;
	signed short old;
	signed char ans;
	Bmp far *b1;
	Bmp far *b2;

	if (o->from == 0x79 && travel_stage(o) == 2)
		guide_offer(o, 4);
	if (g_routes[135].event == 0x32) {
		b1 = new Bmp(0x40, 0x40);
		nvfile_load(g_nvf_heads,0x197, b1->data(), 0, 0x13, 0, &w, &w, 0x140);
		/* SECTION3.LTX[28]: "Vor Euch schleppt sich eine gebeugte Gestalt die Strasse entlang. Sie geht sehr langsam,
		 * und so habt Ihr sie recht schnell eingeholt. Das erste, was Ihr seht, ist ein grosser Rucksack und eine
		 * stattliche Waffensammlung; erst dann fallen Euch die zahlreichen kleineren Verbaende ins Auge." */
		dlg_box_bmp(g_dialog, b1, (char far *)0, g_tev_loc_txt[28], 0);
		b2 = new Bmp(0x40, 0x40);
		nvfile_load(g_nvf_heads,0x197, b2->data(), 0, 0x12, 0, &w, &w, 0x140);
		/* SECTION3.LTX[29]: "Sie hat Euch wohl kommen gehoert, denn nun dreht sich die Gestalt zu Euch um. <Kor sei mit
		 * Euch; oder Phex, wenn Ihr friedlicher Natur seid. Aber so seht Ihr mir eigentlich nicht aus.> Die Frau, die
		 * trotz ihrer Breite momentan schwach und nicht ganz sicher auf den Beinen wirkt, bringt trotz ihres Zustandes
		 * sogar ein kleines Grinsen zustande." */
		dlg_box_bmp(g_dialog, b2, (char far *)0, g_tev_loc_txt[29], 0);
		delete b1;
		delete b2;
		travel_leave(o);
		if (conv_run(g_dialog2, 0x89))
			timewarp_minutes(g_date, 30L);
		mouse_hide(0);
		travel_enter(o);
		mouse_show(0);
		travelmap_draw(o);
		travel_redraw(o);
		g_routes[135].event = 0x3c;
		b2 = new Bmp(0x40, 0x40);
		nvfile_load(g_nvf_heads,0x197, b2->data(), 0, 0x12, 0, &w, &w, 0x140);
		old = dlg_set16(g_dialog, 6);
		/* SECTION3.LTX[30]: "<Ich muss jetzt weiter; in meinem Zustand will ich nicht im Freien uebernachten. Ihr wollt
		 * nicht zufaellig einen Zweihaender kaufen? Soll auch nur 15 Dukaten kosten, ich bin momentan etwas knapp bei
		 * Kasse.>" */
		dlg_box_bmp(g_dialog, b2, (char far *)0, g_tev_loc_txt[30], 0);
		/* SECTION3.LTX[10]: "<Ihr koennt mich auch anheuern; wenn ich wieder fit bin, kann ich ganz schoen gemein
		 * werden. Lowangen, Trollzacken, Al'Anfa, Maraskan; habe ich alles schon gemacht. Fuer Euch sogar zum halben
		 * Kurs, das heisst normalem. Bin naemlich sonst eine Doppelte. Kunchomer Kontrakt. In Ordnung?>" */
		sprintf(g_text_output_buf, g_tev_loc_txt[10]);
		/* SECTION3.LTX[31]: "<Nein, danke fuer das Angebot, aber wir haben alles und alle.>" */
		/* SECTION3.LTX[32]: "<Den Zweihaender, den nehmen wir.>" */
		/* SECTION3.LTX[33]: "<Gerne, seid uns willkommen. Einen solchen Schwertarm kann man immer gebrauchen.>" */
		ans = dlg_box_bmp(g_dialog, b2, (char far *)0, g_tev_loc_txt[10], 3, g_tev_loc_txt[31], g_tev_loc_txt[32], g_tev_loc_txt[33]);
		dlg_set16(g_dialog, old);
		delete b2;
		switch (ans) {
		case 2:
			if (g_group->money >= 1500) {
				group_give_item(g_group, ITEM_TWO_HANDED_SWORD, 1);
				group_sub_money(g_group, 1500L);
			} else
				/* GLOBLTXT.LTX[4]: "Soviel Geld habt Ihr nicht bei Euch!" */
				dlg_message(g_dialog, g_ttx[4]);
			break;
		case 3:
			if (g_group->money >= 10) {
				group_sub_money(g_group, 10L);
				add_npc(7);
			} else
				dlg_message(g_dialog, g_ttx[4]);
			break;
		}
	}
}

void tevent_136(Travel far *o)
{
	signed short w;
	signed char ans;
	signed char r;
	struct hero far *h;

	ans = r = -1;
	if (g_routes[136].event == 2) {
		Bmp b(0x40, 0x40);
		nvfile_load(g_nvf_heads,0x197, b.data(), 0, 4, 0, &w, &w, 0x140);
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
			break;
		case 3:
			break;
		}
		g_routes[136].event = g_routes[136].event + 1;
	} else if (g_routes[136].event >= 5)
		g_routes[136].event = 1;
	else
		g_routes[136].event = g_routes[136].event + 1;
}

void tevent_137(Travel far *o)
{
	if (o->from == 0x26 && travel_stage(o) == 2 && g_routes[137].event != 0x3c)
		guide_offer(o, 1);
	if (g_guide_won != 0)
		g_routes[137].event = 0x3c;
}

void tevent_138(Travel far *o)
{
	signed char r;

	r = 0;
	if (o->from == 0x7a && travel_stage(o) == 2)
		guide_offer(o, 2);
	if (travel_stage(o) == 0) {
		if (g_routes[138].event == 0x32) {
			g_routes[138].event++;
			r = random_schweif(6) + 4;
			if (o->from == 0x78)
				random_encounter(o, r, 1);
			else
				random_encounter(o, r, 2);
		} else {
			r = random_schweif(10);
			if (r <= 5) {
				r = random_schweif(6) + 2;
				if (o->from == 0x78)
					random_encounter(o, r, 1);
				else
					random_encounter(o, r, 2);
			}
		}
	}
}
