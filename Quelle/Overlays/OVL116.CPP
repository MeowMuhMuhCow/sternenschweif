/* OVL116 - Overlay O116 (Segment 0x219b in SCHWEIF.EXE): Reiseereignisse der Wege 49..64 (tevent_run O211;
 *   Reiseobjekt
 * o, Ort o->from; Texte g_tev_loc_txt = SECTION0.LTX fuer Weg 49, sonst SECTION1.LTX, g_tev_txt = ALL_SECT.LTX;
 *   Merker
 * route.event in g_routes; Bilder per nvfile_load(g_nvf_heads, ..) in ein Bmp 64x64). Geschwister von O121.
 * tevent_049: verlassenes Holzfaellerlager - Datei 0xaa (FEAT049.DAT, 6 Eintraege struct loot_item nach
 *   g_tev049_loot) einlesen, Bild + Frage txt[44]; bei Ja Fund melden (txt[45..49]) und Schleife: Menue aus bis zu 7
 *   Zeilen (Puffer 0xd2 = 7 x 30 Bytes; Decken, Aexte, Trockennahrung, Stiefel, Schuhe, Seil txt[50..58], letzte
 *   Zeile
 *   txt[67] nichts mitnehmen), gewaehlten Eintrag suchen, bei Anzahl > 1 Menge per dlg_input erfragen (txt[60]/[61]
 *   mit
 *   Namen txt[63..66]), group_give_item; am Ende Datei 0xaa zurueckschreiben (ofile_create/ofile_write/ofile_close).
 * tevent_050: Steinbock - route.event[50] 2 -> Bildfolge 0x1c8 mit txt[0], 1 -> Erbrochenes im Gebuesch (Bild 0x20,
 *   txt[1]). tevent_051: Ort 0x41 - Rauch am Horizont (txt[2]). tevent_052: show_once_anim. tevent_053:
 *   random_encounter (erstes
 *   Mal W6 + 4, sonst zu 60 % W6 + 2). tevent_054/059/061: guide_offer je Richtung. tevent_055: verhungerter
 *   Steinbock
 *   auf dem Weg (txt[3]), Zaehler -1.
 * tevent_062: versteckter Bogenschuetze hinter dem Felsen - dreistufiger Wortwechsel (txt[4..24], new Bmp), bei
 *   frechen Antworten kommt er heraus (txt[25]) und es folgt das Gespraech 0x88 (PHEXNITE.TLK, 30 Minuten).
 * tevent_063: Pflanzenkunde-Probe (route.event[3] * 3 + 5) > 0 -> 1 Wirselkraut (ALL_SECT[3]); Zaehler [63] bis 41.
 * tevent_064: Zyklus 1..3, bei 1 deer_hunt.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL116.CPP   -> 4167/4167 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - -Og: Zwischenwerte (int)i, i*2 und Offset [bp-20h]/[bp-22h]/[bp-24h] fuer g_tev049_loot[i] (werden ueber
 *   Anweisungen hinweg wiederverwendet: "g_tev049_loot[i].n -= amount" nimmt den Offset aus [bp-24h], das switch den
 *   Index aus [bp-20h]), (int)g_tev049_loot[i].n doppelt als Argument -> [bp-26h]; -Oi: strcat.
 * - Lokale Variablen: Bmp b (10 Bytes) bleibt in Deklarationsreihenfolge, das Feld opt[7] (28 Bytes) liegt hinten;
 *   "Bmp b(64, 64);" mitten in der Funktion nach der Schleife (Destruktor am Funktionsende).
 * - Sprungtabelle verraet die Faelle 0, 1, 2, 4 (nicht 3!) + default; ccmp prueft die Tabelle.
 * - tevent_062: switch-Rumpfreihenfolge mit Durchfall ("case 3: flag = 1; case 1: break;", aeusseres "case 1:" ganz
 *   am Ende hinter case 3); new Bmp -> Bmp(0, ..), delete -> Bmp-Dtor(b, 3).
 * - "cnt = cnt + 1" / "i -= 1" (mov/inc bzw. dec/mov), "i++" -> inc byte.
 * Originalfehler: tevent_049 - Eintrag 3 (Stiefel) hat keinen case (der Text txt[66] haengt an "case 4"), Eintraege
 *   3 und 5 mit Anzahl > 1 landen im default (Debug-sprintf g_fmt_dbg_k_i "k=%d, i=%d" und Meldung) und die
 *   Mengenabfrage nutzt diesen Text; g_tev049_loot wird bei jedem Aufruf neu angelegt und nie freigegeben (nur hier
 *   benutzt -> Speicherleck).
 */
#include <stdio.h>
#include <string.h>

#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:62e4..6318, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
signed char g_anim_slots[30] = "";
signed char g_anim_slots_extra[10] = "";

void tevent_049(Travel far *o)
{
	signed short dummy;
	signed long len;
	signed char r;
	signed char sel;
	signed char sum;
	signed char amount;
	signed char cnt;
	signed char i;
	signed char k;
	char far *buf;
	signed char done = 0;
	char far *opt[7];

	g_tev049_loot = (struct loot_item far *)mem_alloc(ofile_length_id(g_ofile, 0xaa));
	ofile_load(g_ofile, 0xaa, g_tev049_loot);
	buf = (char far *)mem_alloc(0xd2L);
	for (i = 0; i <= 6; i++)
		opt[i] = buf + i * 30;
	Bmp b(0x40, 0x40);

	nvfile_load(g_nvf_heads, 0x197, b.data(), 0, 7, 0, &dummy, &dummy, 0x140);
	/* SECTION0.LTX[44]: "Ihr entdeckt einige Schritt in den Wald hinein ein verlassenes Lager. Die herumliegenden
	 * Werkzeuge, vornehmlich Aexte, und die schon seit laengerer Zeit verlassenen Zelte lassen auf einen hektischen
	 * Aufbruch der Bewohner schliessen. Wollt Ihr das Lager durchsuchen?" */
	/* ALL_SECT.LTX[1]: "Ja" */
	/* ALL_SECT.LTX[2]: "Nein" */
	r = dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[44], 2, g_tev_txt[1], g_tev_txt[2]);
	if (r == 1) {
		sum = g_tev049_loot[0].n + g_tev049_loot[1].n + g_tev049_loot[2].n + g_tev049_loot[3].n + g_tev049_loot[4].n + g_tev049_loot[5].n;
		/* SECTION0.LTX[45]: "Beim Dursuchen des Lagers entdeckt Ihr %s" */
		/* SECTION0.LTX[46]: "gar nichts." */
		/* SECTION0.LTX[47]: "einen noch brauchbaren Gegenstand, den" */
		/* SECTION0.LTX[48]: "einige noch brauchbare Gegenstaende, die" */
		sprintf(g_text_output_buf, g_tev_loc_txt[45], sum == 0 ? g_tev_loc_txt[46] : sum == 1 ? g_tev_loc_txt[47] : g_tev_loc_txt[48]);
		if (sum != 0)
			/* SECTION0.LTX[49]: "die Holzfaeller in ihrer Hektik offenbar uebersehen haben." */
			strcat(g_text_output_buf, g_tev_loc_txt[49]);
		dlg_message(g_dialog, g_text_output_buf);
		while (done == 0) {
			cnt = 0;
			k = 0;
			for (i = 0; i < 6; i++) {
				if (g_tev049_loot[i].n > 0)
					cnt = cnt + 1;
			}
			if (cnt == 0) {
				done = 1;
				continue;
			}
			if (g_tev049_loot[0].n > 0) {
				/* SECTION0.LTX[50]: "%d Decke%s" */
				/* SECTION0.LTX[51]: "n" */
				/* ALL_SECT.LTX[4]: "" */
				sprintf(opt[k], g_tev_loc_txt[50], g_tev049_loot[0].n, g_tev049_loot[0].n > 1 ? g_tev_loc_txt[51] : g_tev_txt[4]);
				k++;
			}
			if (g_tev049_loot[1].n > 0) {
				/* SECTION0.LTX[52]: "%d Aexte" */
				/* SECTION0.LTX[53]: "%d Axt" */
				sprintf(opt[k], g_tev049_loot[1].n > 1 ? g_tev_loc_txt[52] : g_tev_loc_txt[53], g_tev049_loot[1].n);
				k++;
			}
			if (g_tev049_loot[2].n > 0) {
				/* SECTION0.LTX[54]: "%d Portion%s Trockennahrung" */
				/* SECTION0.LTX[55]: "en" */
				sprintf(opt[k], g_tev_loc_txt[54], g_tev049_loot[2].n, g_tev049_loot[2].n > 1 ? g_tev_loc_txt[55] : g_tev_txt[4]);
				k++;
			}
			if (g_tev049_loot[3].n > 0) {
				/* SECTION0.LTX[56]: "1 Paar Stiefel" */
				sprintf(opt[k], g_tev_loc_txt[56], g_tev049_loot[3].n);
				k++;
			}
			if (g_tev049_loot[4].n > 0) {
				/* SECTION0.LTX[57]: "%d Paar Schuhe" */
				sprintf(opt[k], g_tev_loc_txt[57], g_tev049_loot[4].n);
				k++;
			}
			if (g_tev049_loot[5].n > 0) {
				/* SECTION0.LTX[58]: "1 Seil von 10 Fuss Laenge" */
				sprintf(opt[k], g_tev_loc_txt[58], g_tev049_loot[5].n);
				k++;
			}
			/* SECTION0.LTX[67]: "nichts mitnehmen" */
			sprintf(opt[k], g_tev_loc_txt[67]);
			cnt = cnt + 1;
			/* SECTION0.LTX[59]: "Welchen Gegenstand wollt Ihr mitnehmen?" */
			sel = dlg_choose(g_dialog, g_tev_loc_txt[59], cnt, opt[0], opt[1], opt[2], opt[3], opt[4], opt[5], opt[6]);
			if (sel == cnt || sel == -1) {
				done = 1;
				continue;
			}
			for (i = 0, k = 1; k <= sel; k++, i++) {
				while (g_tev049_loot[i].n <= 0)
					i++;
			}
			i -= 1;
#if FIX_0077
			/* Fix #77: Stiefel (3) und Seil (5) haben keinen Mengentext - immer 1 Stueck statt Debugmeldung */
			amount = 1;
			if (g_tev049_loot[i].n > 1 && i != 3 && i != 5) {
#else
			amount = 1;
			if (g_tev049_loot[i].n > 1) {
#endif
				switch (i) {
				case 0:
					/* SECTION0.LTX[60]: "Wieviele %s wollt Ihr mitnehmen?" */
					/* SECTION0.LTX[63]: "Decken" */
					sprintf(g_text_output_buf, g_tev_loc_txt[60], g_tev_loc_txt[63]);
					/* SECTION0.LTX[61]: "Soviele %s habt Ihr gar nicht gefunden." */
					sprintf(g_text_buf2, g_tev_loc_txt[61], g_tev_loc_txt[63]);
					break;
				case 1:
					/* SECTION0.LTX[64]: "Aexte" */
					sprintf(g_text_output_buf, g_tev_loc_txt[60], g_tev_loc_txt[64]);
					sprintf(g_text_buf2, g_tev_loc_txt[61], g_tev_loc_txt[64]);
					break;
				case 2:
					/* SECTION0.LTX[65]: "Proviant-Rationen" */
					sprintf(g_text_output_buf, g_tev_loc_txt[60], g_tev_loc_txt[65]);
					sprintf(g_text_buf2, g_tev_loc_txt[61], g_tev_loc_txt[65]);
					break;
				case 4:
					/* SECTION0.LTX[66]: "Paar Schuhe" */
					sprintf(g_text_output_buf, g_tev_loc_txt[60], g_tev_loc_txt[66]);
					sprintf(g_text_buf2, g_tev_loc_txt[61], g_tev_loc_txt[66]);
					break;
				default:
					sprintf(g_text_output_buf, "k=%d, i=%d", k, i);
					dlg_message(g_dialog, g_text_output_buf);
					break;
				}
				amount = dlg_input(g_dialog, 2, g_text_output_buf, 0, g_tev049_loot[i].n, g_tev049_loot[i].n);
			}
			if (g_tev049_loot[i].n < amount) {
				dlg_message(g_dialog, g_text_buf2);
#if FIX_0353
			/* Fix #353: Abbruch (-1) aendert nichts; nur abziehen, was wirklich gegeben wurde */
			} else if (amount > 0) {
				g_tev049_loot[i].n -= group_give_item(g_group, g_tev049_loot[i].id, amount);
			}
#else
			} else {
				g_tev049_loot[i].n -= amount;
				group_give_item(g_group, g_tev049_loot[i].id, amount);
			}
#endif
		}
	}
	len = ofile_length_id(g_ofile, 0xaa);
	ofile_create(g_ofile, 0xaa, 4);
	ofile_write(g_ofile, g_tev049_loot, len);
#if FIX_0077
	/* Fix #77: Beuteliste nach dem Zurueckschreiben freigeben (Speicherleck) */
	ofile_close(g_ofile);
	g_tev049_loot = (struct loot_item far *)mem_free(g_tev049_loot);
	buf = (char far *)mem_free(buf);
#else
	ofile_close(g_ofile);
	buf = (char far *)mem_free(buf);
#endif
}

void tevent_050(Travel far *o)
{
	signed short dummy;
	void far *win;

	if (g_routes[50].event == 2) {
		win = Anim_ctor(0, 0x1c8, 0, 0);
		/* SECTION1.LTX[0]: "Mit einem gewaltigen Satz landet ein riesiger, grauer Steinbock vor Euren Fuessen. Waehrend
		 * er Euch intensiv zu mustern scheint, beginnt er ungeruehrt wiederzukaeuen, um sich dann vor Euren Fuessen zu
		 * erbrechen. Mit einem noch gewaltigeren Satz entschwindet er sodann im Unterholz." */
		dlg_box_anim(g_dialog, win, (char far *)0, g_tev_loc_txt[0], 0);
		Anim_dtor(win, 3);
		g_routes[50].event = 1;
		return;
	}
	if (g_routes[50].event == 1) {
		Bmp b(0x40, 0x40);

		nvfile_load(g_nvf_heads, 0x197, b.data(), 0, 0x20, 0, &dummy, &dummy, 0x140);
		/* SECTION1.LTX[1]: "Ihr hoert ein seltsames, wuergendes Geraeusch aus einem Gebuesch neben dem Weg. Als Ihr
		 * nachsehen wollt, findet Ihr dort nichts ausser einem Haufen Erbrochenes, offensichtlich das eines
		 * Pflanzenfressers." */
		dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[1], 0);
		g_routes[50].event = 0;
	}
}

void tevent_051(Travel far *o)
{
	signed short dummy;

	if (o->from == 0x41) {
		Bmp b(0x40, 0x40);

		nvfile_load(g_nvf_heads, 0x197, b.data(), 0, 8, 0, &dummy, &dummy, 0x140);
		/* SECTION1.LTX[2]: "Rauch! Rauch am Horizont! Hinter der naechsten Biegung, dem naechsten Huegel muss es
		 * brennen! Jedoch ist es kein Waldbrand, denn die Rauchwolken scheinen duenn und faserig zu sein, wie wenn
		 * tausend Feuer brennen. Und nun traegt der Wind Euch einen Geruch in die Nase, ein Geruch von Feuer, von
		 * Gebratenem und irgendwie von feuchtem Pelz." */
		dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[2], 0);
	}
}

void tevent_052(Travel far *o)
{
	show_once_anim(o);
}

void tevent_053(Travel far *o)
{
	signed char n = 0;

	if (g_routes[53].event == 1) {
		g_routes[53].event++;
		n = random_schweif(6) + 4;
		if (o->from == 0x43)
			random_encounter(o, n, 3);
		else
			random_encounter(o, n, 4);
		return;
	}
	n = random_schweif(10);
	if (n <= 5) {
		n = random_schweif(6) + 2;
		if (o->from == 0x35)
			random_encounter(o, n, 3);
		else
			random_encounter(o, n, 4);
	}
}

void tevent_054(Travel far *o)
{
	if (o->from == 0x44)
		guide_offer(o, 3);
	else
		guide_offer(o, 4);
	if (g_guide_won != 0)
		g_routes[54].event = 0;
}

void tevent_055(Travel far *o)
{
	signed short dummy;
	Bmp b(0x40, 0x40);

	nvfile_load(g_nvf_heads, 0x197, b.data(), 0, 0x21, 0, &dummy, &dummy, 0x140);
	/* SECTION1.LTX[3]: "Mitten auf dem Weg liegt die Leiche eines riesigen, grauen Steinbocks. Das arme Tier ist
	 * offensichtlich verhungert..." */
	dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[3], 0);
	g_routes[55].event = g_routes[55].event - 1;
}

void tevent_059(Travel far *o)
{
	if (o->from == 0x48)
		guide_offer(o, 3);
	else
		guide_offer(o, 4);
	if (g_guide_won != 0)
		g_routes[59].event = 0;
}

void tevent_061(Travel far *o)
{
	if (o->from == 0x49)
		guide_offer(o, 4);
	else
		guide_offer(o, 3);
	if (g_guide_won != 0)
		g_routes[61].event = 0;
}

void tevent_062(Travel far *o)
{
	signed short dummy;
	signed char sel;
	struct hero far *h;
	Bmp far *b;
	signed char flag = 0;

	b = new Bmp(0x40, 0x40);
	nvfile_load(g_nvf_heads, 0x197, b->data(), 0, 9, 0, &dummy, &dummy, 0x140);
	h = get_first_active_hero(g_group);
	/* SECTION1.LTX[4]: "Vor %s faehrt aus heiterem Himmel ein Pfeil in den Boden! Als Ihr Eure Waffen zieht und Euch
	 * umseht, ertoent oberhalb von Euch hinter einem Felsblock hervor eine zittrige Stimme: <Stehenbleiben, oder Ihr
	 * seid alle tot!>" */
	sprintf(g_text_output_buf, g_tev_loc_txt[4], h->name);
	/* SECTION1.LTX[5]: "<Versuch es doch!>" */
	/* SECTION1.LTX[6]: "<Aber wieso denn?>" */
	/* SECTION1.LTX[7]: "<Schon gut, wir tun ja nichts.>" */
	sel = dlg_box_bmp(g_dialog, b, (char far *)0, g_text_output_buf, 3, g_tev_loc_txt[5], g_tev_loc_txt[6], g_tev_loc_txt[7]);
	switch (sel) {
	case 2:
		/* SECTION1.LTX[8]: "<Weil ich es sage!>" */
		/* SECTION1.LTX[10]: "<Na und?>" */
		/* SECTION1.LTX[11]: "<Das ist natuerlich ein Grund.>" */
		/* SECTION1.LTX[12]: "<Aber nur, wenn Ihr dann auch herauskommt!>" */
		sel = dlg_box_bmp(g_dialog, b, (char far *)0, g_tev_loc_txt[8], 3, g_tev_loc_txt[10], g_tev_loc_txt[11], g_tev_loc_txt[12]);
		switch (sel) {
		case 3:
			flag = 1;
		case 1:
			break;
		case 2:
			/* SECTION1.LTX[16]: "<Und ob das einer ist!>" */
			/* SECTION1.LTX[19]: "<Wenn das so ist... Seht Ihr, wir stehen. Wie angewurzelt!>" */
			/* SECTION1.LTX[20]: "<Kann ich mir nicht vorstellen.>" */
			/* SECTION1.LTX[21]: "<Komm da herunter, oder wir holen Dich!>" */
			sel = dlg_box_bmp(g_dialog, b, (char far *)0, g_tev_loc_txt[16], 3, g_tev_loc_txt[19], g_tev_loc_txt[20], g_tev_loc_txt[21]);
			switch (sel) {
			case 1:
				flag = 1;
				break;
			case 2:
			case 3:
				break;
			}
			break;
		}
		break;
	case 3:
		/* SECTION1.LTX[9]: "<Wirklich nicht?>" */
		/* SECTION1.LTX[13]: "<Ach du liebe Guete.>" */
		/* SECTION1.LTX[14]: "<Mal sehen...>" */
		/* SECTION1.LTX[15]: "<Nein, ganz bestimmt nicht.>" */
		sel = dlg_box_bmp(g_dialog, b, (char far *)0, g_tev_loc_txt[9], 3, g_tev_loc_txt[13], g_tev_loc_txt[14], g_tev_loc_txt[15]);
		switch (sel) {
		case 1:
			/* SECTION1.LTX[18]: "<Was soll das denn heissen?>" */
			/* SECTION1.LTX[22]: "<Was wohl, Hanswurst?>" */
			/* SECTION1.LTX[23]: "<Och, nichts.>" */
			/* SECTION1.LTX[24]: "<Nur ein Zeichen unserer Angst, hoher Herr oder Dame.>" */
			sel = dlg_box_bmp(g_dialog, b, (char far *)0, g_tev_loc_txt[18], 3, g_tev_loc_txt[22], g_tev_loc_txt[23], g_tev_loc_txt[24]);
			switch (sel) {
			case 2:
			case 3:
				flag = 1;
			case 1:
				break;
			}
		case 2:
			break;
		case 3:
			flag = 1;
			break;
		}
	case 1:
		break;
	}
	if (flag == 1) {
		/* SECTION1.LTX[25]: "Hinter dem Felsen erhebt sich ein Gebilde, ganz in ungegerbte Felle gekleidet, in der einen
		 * Hand einen Kurzbogen und in der anderen ein Schwert. Mit wiegendem Schritt geht die Gestalt den geroelligen
		 * Abhang hinunter und auf Euch zu. Dabei koennt Ihr erkennen, dass das Schwert in der Rechten des Mannes kein
		 * gewoehnliches Schwert ist, sondern ein Zweihaender, den er sich kurzerhand handlicher gemacht hat, indem er
		 * die vorderen drei Spann angebrochen hat." */
		dlg_box_bmp(g_dialog, b, (char far *)0, g_tev_loc_txt[25], 0);
		delete b;
		travel_leave(o);
		if (conv_run(g_dialog2, 0x88))
			timewarp_minutes(g_date, 0x1eL);
		mouse_hide(0);
		travel_enter(o);
		mouse_show(0);
		travelmap_draw(o);
		travel_redraw(o);
	} else {
		delete b;
	}
	g_routes[62].event = 0;
}

void tevent_063(Travel far *o)
{
	struct hero far *h;
	signed short r;

	h = get_best_hero_skill(g_group, TA_PFLANZENKUNDE);
	r = test_skill(h, TA_PFLANZENKUNDE, g_routes[3].event * 3 + 5);
	if (r > 0) {
		/* ALL_SECT.LTX[3]: "%s entdeckt am Wegesrand %d Portion%s Wirselkraut!" */
		/* ALL_SECT.LTX[4]: "" */
		sprintf(g_text_output_buf, g_tev_txt[3], h->name, 1, g_tev_txt[4]);
		dlg_message(g_dialog, g_text_output_buf);
		group_give_item(g_group, ITEM_WHIRLWEED, 1);
	}
	if (g_routes[63].event <= 40)
		g_routes[63].event++;
}

void tevent_064(Travel far *o)
{
	if (g_routes[64].event == 1) {
		deer_hunt(o);
		g_routes[64].event = g_routes[64].event + 1;
		return;
	}
	if (g_routes[64].event >= 3) {
		g_routes[64].event = 1;
		return;
	}
	g_routes[64].event = g_routes[64].event + 1;
}
