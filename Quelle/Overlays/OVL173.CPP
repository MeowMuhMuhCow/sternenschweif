/* OVL173 - Overlay O173 (Segment 0x22bc in SCHWEIF.EXE): Wohnhaus LOC_HOUSE (class House in s_class.h, 8 Bytes: Texte
 * GETOFF.LTX = Datei 0x19d, 0x46 Eintraege per txt_load_list) - das Haus, an dessen Tuer die Gruppe in einer Stadt
 * klopft (O183 town_enter mit Ortsart LOC_HOUSE, Vorgabe fuer alle Tueren ohne eigenen Ort).
 * run(): ausserhalb von Finsterkoppen eine abweisende Antwort je Hauslage (((x & 3) ^ (y & 3)) * 4 + W4 - 1 ->
 *   GETOFF.LTX[0..15], z. B. [0] "Gesindel! Macht, dass Ihr weg kommt, Ihr Tagediebe!") nach g_text_buf2 kopieren
 *   und - sofern hint() keinen Bewohner mit Rat findet - in der Textleiste zeigen (Tueranimation 0x18b + Lage, 700
 *   Strahlruecklaeufe bzw. bis Taste/Maus). In Finsterkoppen (Zwerge): ohne Frevel (g_sacrilege) Tueranimation 0x18d,
 *   Abweisung [68] (g_finster_refuse_msg, einmal) bzw. Frage [62] nach einem Nachtlager: je Hauslage ((x + y) & 8)
 *   Schuppen [63] (Rast in der Stadt: alle Helden rest_bonus = 2, Camp run(1)) oder kein Platz [64], Nein [65];
 *   nach einem Frevel 70 % [66] (Stimme von innen), sonst Animation 0x1cc und der bewaffnete Zwerg [67], Zorn der
 *   Zwerge g_finster_anger beginnt (1). Ende: Textleiste loeschen (screen_textbar ""), Animation aus.
 * hint(): Bewohner mit Rat - Tabelle g_house_hints (Stadt, x, y; Ende -1; Kvirasim und Gashok je 8 Haeuser) nach der
 *   Gruppenposition durchsuchen, Portraet 0xc7 + i (NVF 10, 32 x 32) mit Name GETOFF.LTX[2i + 17] als Titel und Rat
 *   [2i + 16] (beim ersten Besuch, g_house_hint_seen[i]) bzw. [49] "Ich kann Euch nicht mehr weiterhelfen."; 1 =
 *   gefunden.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL173.CPP   -> 1451/1451 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Gewoehnlicher C++-Konstruktor (this==0 -> new(8), toter Epilog), Destruktor mit flags&1 -> delete.
 * - -Og noetig: Epilogkopien bei den return-Zweigen von run() und return 1 / else return 0 in hint().
 * - -Oi: strcpy inline (deshalb SI/DI-Sicherung in run()).
 * - hint() steht unter run() -> far call; gb_new_alloc(0, 0x20, 0x20) = new, gb_delete(p, 3) = delete.
 * - push 0ffh ist hier 68 ff 00 (Farbe 0xff, nicht -1).
 */
#include <string.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:6c8c..6d18, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
struct xy14 g_chargen_attrib_pos = {
	{ { 205, 73 },
	{ 205, 85 },
	{ 205, 97 },
	{ 205, 109 },
	{ 205, 121 },
	{ 205, 133 },
	{ 205, 145 },
	{ 273, 73 },
	{ 273, 85 },
	{ 273, 97 },
	{ 273, 109 },
	{ 273, 121 },
	{ 273, 133 },
	{ 273, 145 } }
};
struct range8x9 g_chargen_school_ranges = {
	{ { 1, 5 },
	{ 6, 17 },
	{ 18, 26 },
	{ 27, 32 },
	{ 33, 37 },
	{ 38, 44 },
	{ 49, 57 },
	{ 60, 75 },
	{ 76, 85 } }
};
struct type_list13 g_chargen_type_list_init = { "" };
char g_unused_6ce3[1] = { 0 };   /* ds:6ce3, unbenutzt */
struct house_hint g_house_hints[17] = {
	{ 1, 15, 14 },
	{ 1, 9, 10 },
	{ 1, 6, 3 },
	{ 1, 5, 5 },
	{ 1, 19, 5 },
	{ 1, 10, 4 },
	{ 1, 18, 13 },
	{ 1, 4, 5 },
	{ 2, 16, 3 },
	{ 2, 6, 7 },
	{ 2, 6, 17 },
	{ 2, 12, 25 },
	{ 2, 15, 8 },
	{ 2, 9, 3 },
	{ 2, 17, 24 },
	{ 2, 8, 24 },
	{ -1, -1, -1 }
};

#define FILE_GETOFF	0x19d		/* GETOFF.LTX */
#define ANIM_DOOR	0x18b		/* Tueranimation 0x18b..0x18e je Hauslage (x & 3) ^ (y & 3) */
#define ANIM_DWARF_DOOR	0x18d		/* Finsterkoppen: Zwerg oeffnet */
#define ANIM_DWARF_AXE	0x1cc		/* Finsterkoppen: Zwerg mit Axt (nach Frevel) */

House::House()
{
	txt_load_list(g_font, FILE_GETOFF, 0x46, &txt_buf, &txt);
}

House::~House()
{
	mem_free(txt_buf);
	mem_free(txt);
}

void House::run()
{
	signed short lage;
	signed short idx;
	signed short done;
	signed short cnt;
	void far *camp;
	struct hero far *h;

	if (g_group->town != TOWN_FINSTERKOPPEN) {
		lage = (g_group->x & 3) ^ (g_group->y & 3);
		idx = lage * 4 + random_schweif(4) - 1;
		done = 0;
		g_refresh = 1;
		do {
			if (g_refresh) {
				screen_redraw(g_screen);
				anim_load(g_screen_pic, lage + ANIM_DOOR, 0);
				g_refresh = 0;
			}
			/* GETOFF.LTX[0]: "Gesindel! Macht, dass Ihr weg kommt, Ihr Tagediebe!" */
			/* GETOFF.LTX[1]: "Ich hab zu tun und komme gut alleine klar!" */
			/* GETOFF.LTX[2]: "Ihr Streunerpack. Travia moege Euch vergeben." */
			/* GETOFF.LTX[3]: "Wenn Ihr Hausarbeit liebt, seid Ihr hier richtig..." */
			/* GETOFF.LTX[4]: "Nicht schlecht! Wuenschte ich haette Zeit fuer Euch." */
			/* GETOFF.LTX[5]: "Nicht eben Prembuttnasen. Besucht mich mal wieder." */
			/* GETOFF.LTX[6]: "Wer ist am Schoensten hier? Nicht Ihr!" */
			/* GETOFF.LTX[7]: "Sieh mal einer an. Wenn ich doch nur Zeit haette." */
			/* GETOFF.LTX[8]: "Wohl an Fremde. Seht Euch woanders um." */
			/* GETOFF.LTX[9]: "Habt Ihr nichts Besseres zu tun?" */
			/* GETOFF.LTX[10]: "Was ist los? Ihr wollt nicht zu mir." */
			/* GETOFF.LTX[11]: "Habt Ihr ein Problem oder wollt Ihr eines?" */
			/* GETOFF.LTX[12]: "Gesindel, Streuner. Schert Euch! Wird's bald?" */
			/* GETOFF.LTX[13]: "Kommt mir nicht zu nahe in dieser Zeit." */
			/* GETOFF.LTX[14]: "Besucht Tavernen und Herbergen, nicht mich!" */
			/* GETOFF.LTX[15]: "Viel Besuch, viel Ehr! Leider gilt's nicht mir!" */
			strcpy(g_text_buf2, txt[idx]);
			if (!hint()) {
				screen_textbar(g_screen, g_text_buf2, 0xff);
				cnt = 700;
				do {
					poll_input();
					wait_for_vsync();
					cnt--;
				} while (cnt != 0 && g_action != KEY_ESC && g_action != KEY_RETURN && g_action != KEY_DOWN &&
					 g_mouse_lclick == 0);
			}
			done = 1;
		} while (!done);
		mouse_clear_left();
		screen_clear_textbar(g_screen);
		return;
	}
	if (!g_sacrilege) {
		anim_load(g_screen_pic, ANIM_DWARF_DOOR, 0);
		if (g_finster_refuse_msg != 0) {
			/* GETOFF.LTX[68]: "<Tut mir leid, aber in dem unterirdischen Teil der Stadt gibt es keinen Platz fuer Gaeste
			 * wie Euch!>" */
			dlg_message(g_dialog, txt[68]);
			g_finster_refuse_msg = 0;
		/* GETOFF.LTX[62]: "Euch oeffnet ein Zwerg die Tuere, der Euch muerrisch anspricht: <Ihr wollt bestimmt bei mir
		 * uebernachten?>" */
		} else if (dlg_yes_no(g_dialog, txt[62]) == 1) {
			if ((g_group->x + g_group->y) & 8) {
				/* GETOFF.LTX[63]: "<Na meinetwegen; Ihr koennt im Schuppen schlafen; mein Haus ist nicht gross genug fuer
				 * Reisende, wie Ihr es seid. Aber Essen muesst Ihr selber besorgen. Es sind harte Zeiten; auch hier oben.>" */
				dlg_message(g_dialog, txt[63]);
				camp = Camp_ctor(0, 0);
				h = get_first_hero(g_group);
				do {
					h->rest_bonus = 2;
					h = get_next_hero(g_group, h);
				} while (get_first_hero(g_group) != h);
				camp_run((CAMP_PTR)camp, 1);
				Camp_dtor(camp, 3);
			} else
				/* GETOFF.LTX[64]: "<Tut mir leid, aber ich habe keinen Platz fuer Gaeste!>" */
				dlg_message(g_dialog, txt[64]);
		} else
			/* GETOFF.LTX[65]: "<Na dann eben nicht, dann braucht Ihr mich aber auch nicht zu stoeren!>.@Mit diesen Worten
			 * schlaegt der Zwerg die Tuere vor Eurer Nase zu." */
			dlg_message(g_dialog, txt[65]);
		ani_stop(g_screen_pic);
		return;
	}
	if (random_schweif(100) <= 70)
		/* GETOFF.LTX[66]: "Von innen toent eine tiefe Stimme.@<Macht bloss, dass Ihr wegkommt!>" */
		dlg_message(g_dialog, txt[66]);
	else {
		anim_load(g_screen_pic, ANIM_DWARF_AXE, 0);
		ani_play_until(g_screen_pic, 5, 99);
		/* GETOFF.LTX[67]: "Ein Zwerg oeffnet Euch die Tuere, geruestet und mit einer schweren Axt in der Hand.@>Macht,
		 * dass Ihr hier verschwindet. Mit Leuten, denen Ingra zuernt, wollen wir nichts zu tun haben!>" */
		dlg_message(g_dialog, txt[67]);
		ani_play_until(g_screen_pic, 5, 99);
		if (!g_finster_anger)
			g_finster_anger = 1;
	}
	screen_textbar(g_screen, "", 0xff);
	ani_stop(g_screen_pic);
}

signed short House::hint()
{
	signed short town;
	signed short x;
	signed short y;
	void far *pic;
	char far *name;
	char far *text;
	signed short tmp;
	signed short old;
	register signed short i;
	register signed short found;

	i = 0;
	town = g_group->town;
	x = g_group->x;
	y = g_group->y;
	found = 0;
	do {
		if (g_house_hints[i].x == x && g_house_hints[i].y == y && g_house_hints[i].town == town)
			found = 1;
		else
			i++;
	} while (g_house_hints[i].town <= town && g_house_hints[i].town != -1 && !found);
	if (found) {
		if (g_house_hint_seen[i] == 0)
			/* GETOFF.LTX[16]: "Wenn Ihr in die Suempfe geht, solltet Ihr unbedingt ein Netz dabeihaben. Man kann so
			 * allerhand fangen, dort in den Suempfen." */
			/* GETOFF.LTX[17]: "Marje von Thorwal" */
			/* GETOFF.LTX[18]: "Es ist immer besser, in einer Herberge zu uebernachten, als unter freiem Himmel." */
			/* GETOFF.LTX[19]: "Ingram Sohn des Utzlesch" */
			/* GETOFF.LTX[20]: "Wer mit Rondra im Bunde steht, braucht auf lange Sicht keinen Ork zu fuerchten." */
			/* GETOFF.LTX[21]: "Eida Matjus" */
			/* GETOFF.LTX[22]: "Der Elfenkoenig steht uns bei. Doch reicht seine Macht aus um diesen Orks zu trotzen?" */
			/* GETOFF.LTX[23]: "Heralja Olafsen" */
			/* GETOFF.LTX[24]: "Wer weiss, vielleicht solltet auch Ihr Euch in die Berge zurueckziehen, um dem Unheil zu
			 * entfliehen." */
			/* GETOFF.LTX[25]: "Asgrim Kollberg" */
			/* GETOFF.LTX[26]: "Wenn Elf und Zwerg sich zanken, dann gibt's kein orkisch Wanken." */
			/* GETOFF.LTX[27]: "Fladim Petermann" */
			/* GETOFF.LTX[28]: "Letzte Nacht sah ich den Stern am Himmel. Er begann zu gluehen und zog einen noch nie
			 * gesehenen Schweif hinter sich her. Die Stunde scheint gekommen." */
			/* GETOFF.LTX[29]: "Hjore von Beilunk" */
			/* GETOFF.LTX[30]: "Wo Blitz und Donner walten, kann Krankheit sich entfalten. Wer sich nicht fuer schlechtes
			 * Wetter wappnet, den findet Boron schneller als ihm lieb ist." */
			/* GETOFF.LTX[31]: "Rumhild Rohalsdottir" */
			/* GETOFF.LTX[32]: "Wenn Elfenblut in Euren Adern fliesst, dann weg von hier. Sonst ergeht es Euch wie dem
			 * frueheren Mueller, ganz im Sueden der Stadt." */
			/* GETOFF.LTX[33]: "Urja Naloth" */
			/* GETOFF.LTX[34]: "Drum wisset, wenn Ihr sucht nach schoenen Dingen, nicht selten findet Ihr sie unweit schon." */
			/* GETOFF.LTX[35]: "Dietgel Fridgard" */
			/* GETOFF.LTX[36]: "Ich sag's Euch im Geheimen. Manche behaupten, das Kraeuterweib am Markt waere eine Hexe.@Ich
			 * glaube das nicht, wieso auch, nur weil sie soviel weiss?" */
			/* GETOFF.LTX[37]: "Tronde Ismanson" */
			/* GETOFF.LTX[38]: "Oh welch eine Ehre werte Reisende, Gaeste in meinem bescheidenen Heim.@Ach Ihr wollt gar
			 * nicht bei mir verweilen." */
			/* GETOFF.LTX[39]: "Rohel Gumbaldson" */
			/* GETOFF.LTX[40]: "Es geschehen gar seltsame Dinge in Gashok, wenn Praios sein Haupt verhuellt." */
			/* GETOFF.LTX[41]: "Grabosch Guidon" */
			/* GETOFF.LTX[42]: "Hoert die Mahnung eines Rates: Bleibt nachts den Strassen fern. Ihr trefft im besten Fall
			 * auf fragwuerdige Genossen; wer zuviel wissen will, der ... naja, Ihr wisst schon." */
			/* GETOFF.LTX[43]: "Moria, der Weise" */
			/* GETOFF.LTX[44]: "Frueher war Gashok ein ruhiges Staedtchen. Erst seit die vielen Fremden hier sind geschieht
			 * immer wieder etwas." */
			/* GETOFF.LTX[45]: "Heroja Inhar" */
			/* GETOFF.LTX[46]: "Es gibt merkwuerdige Leute hier in Gashok. Noch nicht lange. Man munkelt so einiges. Nehmt
			 * Euch in Acht." */
			/* GETOFF.LTX[47]: "Erhild Hesindel" */
			/* GETOFF.LTX[48]: "Wer Kraeuter sammelt, kann sich ein nettes Suemmchen nebenher verdienen." */
			text = txt[2 * i + 16];
		else
			/* GETOFF.LTX[49]: "Ich kann Euch nicht mehr weiterhelfen." */
			text = txt[49];
		name = txt[2 * i + 17];
		g_house_hint_seen[i] = 1;
		pic = gb_new_alloc(0, 0x20, 0x20);
		nvfile_load(g_nvf_heads, 10, gb_data(pic), (void far *)0, i + 0xc7, 0, &tmp, &tmp, 0x140);
		old = dlg_set16(g_dialog, 5);
		dlg_box_bmp(g_dialog, pic, name, text, 0);
		dlg_set16(g_dialog, old);
		gb_delete(pic, 3);
		return 1;
	} else
		return 0;
}
