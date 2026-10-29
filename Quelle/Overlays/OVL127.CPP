/* OVL127 - Overlay O127 (Segment 0x21ec in SCHWEIF.EXE): neun Doerfer der Reisekarte (Ortsereignisse aus place_event
 * O210 wie O126; Reiseobjekt p = Travel, Texte g_tev_txt = ALL_SECT.LTX und g_tev_loc_txt = POINT2.LTX). Ablauf je Ort
 * (Makros PIC/SETLOC/ENTER/LEAVE): Bild 0x198 (new Bmp 64x64, nvfile_load mit Teilbild 0, bei Aumond 0x1d, Sibra
 * 0x1f, Hiltorp 0x1e), Ort g_group->town setzen (alter nach old_town), beim ersten Betreten (g_travel_state == 0)
 * Tagebuch ALL_SECT.LTX[112] "Wir haben ein kleines Staedtchen namens %s erreicht." mit dem Ortsnamen
 * GLOBLTXT.LTX[355 + Ort], dann dlg_box_bmp mit der Ortsbeschreibung (nach Rueckkehr aus einem Gebaeude,
 * g_travel_state == 4, ein kuerzerer Text) und 2, 3 bzw. 4 Antworten: Gebaeude betreten -> set_loctype(LOC_VILLAGE_INN/
 * _TEMPLE/_SMITH), g_travel_state = 4; letzte Antwort/Abbruch -> weiterreisen, g_travel_state = 6 und Ort 0.
 * place_event_021 (Aumond, Elfendorf in den Salamandersteinen, POINT2.LTX[16..18]): Antwort 1 = Elfin ansprechen:
 *   travel_leave, Gespraech conv_run(g_dialog2, 0x6e) - wenn gefuehrt, 30 Minuten -, travel_enter, Karte neu.
 * place_event_022 (Hillhaus, [19..22]): Herberge. place_event_023 (Neulowangen, [23..27]): Praiostempel, Herberge.
 * place_event_024 (Reichsend, [28..32]): Praiostempel, Herberge <Kaiserstolz>; sichert die Fensterbreite (dlg_set16 6).
 * place_event_025 (Nordhag, [33..36]): Herberge <Greifenfurter>. place_event_026 (Yrramis am Svellt, [37..41]):
 *   Tsatempel, Herberge <Junger Svellt>. place_event_027 (Arsingen am Svall, [42..45]): Herberge im verlassenen Ort.
 * place_event_029 (Sibra, Tsatempel im Wald, [57..60]): nur der Tempel. place_event_032 (Hiltorp, Zwergenstaedtchen,
 *   [78..83]): Ingerimmtempel, Herberge, Schmiede.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL127.CPP   -> 4290/4290 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - C++ wegen group::set_loctype (inline wie O126: -Og-Zwischenwert [bp-9], "else ;" -> toter jmp short, loses
 *   cbw); ohne -Og 26 Bytes zu lang.
 * - Gemeinsame Teile als Makros PIC/SETLOC/ENTER/LEAVE (Bytes identisch zu ausgeschriebenem Code). In
 *   place_event_024 steht dlg_set16 zwischen Bild und Ortswechsel.
 * - 2 Antworten: "switch (sel) { case 1: ..; break; default: g_travel_state = 6; break; }" (cmp ax,1 / jz / jmp
 *   short default, toter jmp short am Ende); 3/4 Antworten: if-Kette mit "sel == 3 || sel == -1" (cmp byte).
 * - place_event_021: case 1 faellt in ein leeres "case 2: break;" durch - nur so springt case 2 auf den ECHTEN
 *   Epilog (mit "break;" in case 1 liegt danach der tote Epilog, und case 2 springt auf diesen).
 * Auffaellig: p ist ausser in place_event_021 unbenutzt.
 */
#include <stdio.h>
#include "schweif.h"

/* Bild 0x198/pic in ein neues Bmp 64 x 64 */
#define PIC(pic) \
	b = new Bmp(0x40, 0x40); \
	nvfile_load(g_nvf_heads, 0x198, b->data(), (void far *)0, pic, 0, &dummy, &dummy, 0x140);

#define ENTER(loc, pic) \
	PIC(pic) \
	SETLOC(loc)

/* Ort setzen, beim ersten Betreten Tagebucheintrag */
#define SETLOC(loc) \
	g_group->old_town = g_group->town; \
	g_group->town = loc; \
	if (g_travel_state == 0) {	/* ALL_SECT.LTX[112]: "Wir haben ein kleines Staedtchen namens %s erreicht." */ \
		sprintf(g_text_output_buf, g_tev_txt[112], g_ttx[TTX_TOWN_NAMES + g_group->town]); \
		diary_add(g_diary, g_text_output_buf, 0); \
	}

/* Ort verlassen (g_travel_state 6): kein Ort mehr */
#define LEAVE() \
	if (g_travel_state == 6) { \
		g_group->old_town = g_group->town; \
		g_group->town = 0; \
	}

void place_event_021(Travel far *p)
{
	signed short dummy;
	signed char sel;
	Bmp far *b;

	ENTER(TOWN_AUMOND, 0x1d)
	/* POINT2.LTX[16]: "Ihr erreicht ein kleines Elfendorf in den bewaldeten Haengen der Salamandersteine. Wollt Ihr" */
	/* POINT2.LTX[17]: "eine Elfin ansprechen, die gerade aus ihrer Huette schaut, oder" */
	/* POINT2.LTX[18]: "das Dorf gleich wieder verlassen?" */
	sel = dlg_box_bmp(g_dialog, b, (char far *)0, g_tev_loc_txt[16], 2, g_tev_loc_txt[17], g_tev_loc_txt[18]);
	delete b;
	switch (sel) {
	case 1:
		travel_leave(p);
		if (conv_run(g_dialog2, 0x6e))
			timewarp_minutes(g_date, 30L);
		mouse_hide(0);
		travel_enter(p);
		mouse_show(0);
		travelmap_draw(p);
		travel_redraw(p);
	case 2:
		break;
	}
}

void place_event_022(Travel far *p)
{
	signed short dummy;
	signed char sel;
	Bmp far *b;

	ENTER(TOWN_HILLHAUS, 0)
	if (g_travel_state == 4)
		/* POINT2.LTX[22]: "Ihr steht wieder auf der Strasse, an deren Seiten sich die Haeuser von Hillhaus aufreihen.
		 * Wollt Ihr" */
		sprintf(g_text_output_buf, g_tev_loc_txt[22]);
	else
		/* POINT2.LTX[19]: "Ihr erreicht das Doerfchen Hillhaus. In der Herberge direkt am Wegesrand koenntet Ihr eine
		 * Rast einlegen. Wollt Ihr" */
		sprintf(g_text_output_buf, g_tev_loc_txt[19]);
	/* POINT2.LTX[20]: "die Herberge betreten, oder" */
	/* POINT2.LTX[21]: "dem Ort den Ruecken kehren?" */
	sel = dlg_box_bmp(g_dialog, b, (char far *)0, g_text_output_buf, 2, g_tev_loc_txt[20], g_tev_loc_txt[21]);
	delete b;
	switch (sel) {
	case 1:
		g_group->set_loctype(LOC_VILLAGE_INN);
		g_travel_state = 4;
		break;
	default:
		g_travel_state = 6;
		break;
	}
	LEAVE()
}

void place_event_023(Travel far *p)
{
	signed short dummy;
	signed char sel;
	Bmp far *b;

	ENTER(TOWN_NEULOWANGEN, 0)
	if (g_travel_state == 4)
		/* POINT2.LTX[27]: "Wieder auf dem Platz muesst Ihr Euch erneut entscheiden. Wollt Ihr" */
		sprintf(g_text_output_buf, g_tev_loc_txt[27]);
	else
		/* POINT2.LTX[23]: "Ihr erreicht Neulowangen. Mit Lowangen hat das Kaff wohl nur den Namen gemein: Die Strasse
		 * weitet sich auf einen Platz, an dem ein paar Haeuschen stehen. Mitten auf dem Platz steht ein Praiostempel,
		 * eine Herberge findet sich am Suedende des Platzes. Wollt Ihr" */
		sprintf(g_text_output_buf, g_tev_loc_txt[23]);
	/* POINT2.LTX[24]: "den Tempel betreten" */
	/* POINT2.LTX[25]: "der Herberge einen Besuch abstatten" */
	/* POINT2.LTX[26]: "oder weiterreisen?" */
	sel = dlg_box_bmp(g_dialog, b, (char far *)0, g_text_output_buf, 3, g_tev_loc_txt[24], g_tev_loc_txt[25],
		g_tev_loc_txt[26]);
	delete b;
	if (sel == 3 || sel == -1)
		g_travel_state = 6;
	else {
		if (sel == 1)
			g_group->set_loctype(LOC_VILLAGE_TEMPLE);
		if (sel == 2)
			g_group->set_loctype(LOC_VILLAGE_INN);
		g_travel_state = 4;
	}
	LEAVE()
}

void place_event_024(Travel far *p)
{
	signed short dummy;
	signed char sel;
	signed char old;
	Bmp far *b;

	PIC(0)
	old = dlg_set16(g_dialog, 6);
	SETLOC(TOWN_REICHSEND)
	if (g_travel_state == 4)
		/* POINT2.LTX[32]: "Nachdem Ihr die Tuer hinter Euch geschlossen habt, koennt Ihr" */
		sprintf(g_text_output_buf, g_tev_loc_txt[32]);
	else
		/* POINT2.LTX[28]: "Ihr erreicht Reichsend. Gaebe es nicht einen praechtigen Praiostempel und die kleine Herberge
		 * <Kaiserstolz>, Ihr haettet ueberhaupt nicht bemerkt, dass die Ansammlung von Haeusern ein Dorf darstellt.
		 * Wollt Ihr" */
		sprintf(g_text_output_buf, g_tev_loc_txt[28]);
	/* POINT2.LTX[29]: "den herrlichen Tempel auch von innen bestaunen" */
	/* POINT2.LTX[30]: "in die Herberge gehen" */
	/* POINT2.LTX[31]: "oder den Ort wieder verlassen?" */
	sel = dlg_box_bmp(g_dialog, b, (char far *)0, g_text_output_buf, 3, g_tev_loc_txt[29], g_tev_loc_txt[30],
		g_tev_loc_txt[31]);
	dlg_set16(g_dialog, old);
	delete b;
	if (sel == 3 || sel == -1)
		g_travel_state = 6;
	else {
		if (sel == 1)
			g_group->set_loctype(LOC_VILLAGE_TEMPLE);
		if (sel == 2)
			g_group->set_loctype(LOC_VILLAGE_INN);
		g_travel_state = 4;
	}
	LEAVE()
}

void place_event_025(Travel far *p)
{
	signed short dummy;
	signed char sel;
	Bmp far *b;

	ENTER(TOWN_NORDHAG, 0)
	if (g_travel_state == 4)
		/* POINT2.LTX[36]: "Wieder an der frischen Luft stellt sich die Frage, wollt Ihr nochmal" */
		sprintf(g_text_output_buf, g_tev_loc_txt[36]);
	else
		/* POINT2.LTX[33]: "Das Dorf Nordhag ist idyllisch inmitten satter Wiesen gelegen. Mitten im Ort steht die
		 * Herberge <Greifenfurter>. Wollt Ihr" */
		sprintf(g_text_output_buf, g_tev_loc_txt[33]);
	/* POINT2.LTX[34]: "in der Herberge ein Paeuschen einlegen, oder" */
	/* POINT2.LTX[35]: "Euer Schuhwerk weiter strapazieren?" */
	sel = dlg_box_bmp(g_dialog, b, (char far *)0, g_text_output_buf, 2, g_tev_loc_txt[34], g_tev_loc_txt[35]);
	delete b;
	switch (sel) {
	case 1:
		g_group->set_loctype(LOC_VILLAGE_INN);
		g_travel_state = 4;
		break;
	default:
		g_travel_state = 6;
		break;
	}
	LEAVE()
}

void place_event_026(Travel far *p)
{
	signed short dummy;
	signed char sel;
	Bmp far *b;

	ENTER(TOWN_YRRAMIS, 0)
	if (g_travel_state == 4)
		/* POINT2.LTX[41]: "Ihr steht wieder in dem Dorf. Wollt Ihr" */
		sprintf(g_text_output_buf, g_tev_loc_txt[41]);
	else
		/* POINT2.LTX[37]: "Am Ufer des Svellt erreicht Ihr eine kleine Siedlung, deren Haeuser sich um den Tsatempel
		 * gruppieren. Die Herberge <Junger Svellt> bietet Reisenden die Gelegenheit, eine ordentliche Rast einzulegen.
		 * Wollt Ihr" */
		sprintf(g_text_output_buf, g_tev_loc_txt[37]);
	/* POINT2.LTX[38]: "in den Tempel" */
	/* POINT2.LTX[39]: "in die Herberge" */
	/* POINT2.LTX[40]: "oder weitergehen?" */
	sel = dlg_box_bmp(g_dialog, b, (char far *)0, g_text_output_buf, 3, g_tev_loc_txt[38], g_tev_loc_txt[39],
		g_tev_loc_txt[40]);
	delete b;
	if (sel == 3 || sel == -1)
		g_travel_state = 6;
	else {
		if (sel == 1)
			g_group->set_loctype(LOC_VILLAGE_TEMPLE);
		if (sel == 2)
			g_group->set_loctype(LOC_VILLAGE_INN);
		g_travel_state = 4;
	}
	LEAVE()
}

void place_event_027(Travel far *p)
{
	signed short dummy;
	signed char sel;
	Bmp far *b;

	ENTER(TOWN_ARSINGEN, 0)
	if (g_travel_state == 4)
		/* POINT2.LTX[45]: "Ihr steht wieder auf der leeren Strasse. Wollt Ihr" */
		sprintf(g_text_output_buf, g_tev_loc_txt[45]);
	else
		/* POINT2.LTX[42]: "Ihr erreicht ein kleines Staedtchen am Ufer des Svall, das wie ausgestorben wirkt. Sicherlich
		 * sind viele Bewohner nach Lowangen gefluechtet. Mitten im Ort gibt es aber eine Herberge, die geoeffnet zu sein
		 * scheint. Wollt Ihr" */
		sprintf(g_text_output_buf, g_tev_loc_txt[42]);
	/* POINT2.LTX[43]: "Euch in die Herberge begeben, oder" */
	/* POINT2.LTX[44]: "wie die meisten den Ort moeglichst schnell verlassen?" */
	sel = dlg_box_bmp(g_dialog, b, (char far *)0, g_text_output_buf, 2, g_tev_loc_txt[43], g_tev_loc_txt[44]);
	delete b;
	switch (sel) {
	case 1:
		g_group->set_loctype(LOC_VILLAGE_INN);
		g_travel_state = 4;
		break;
	default:
		g_travel_state = 6;
		break;
	}
	LEAVE()
}

void place_event_029(Travel far *p)
{
	signed short dummy;
	signed char sel;
	Bmp far *b;

	ENTER(TOWN_SIBRA, 0x1f)
	if (g_travel_state == 4)
		/* POINT2.LTX[60]: "Ihr steht wieder im Gruenen. Wollt Ihr" */
		sprintf(g_text_output_buf, g_tev_loc_txt[60]);
	else
		/* POINT2.LTX[57]: "Mitten im Wald auf einer kleinen Lichtung erreicht Ihr einen Tsatempel. Ein paar Holzhuetten
		 * stehn am Rande der Lichtung im Schutz der Baeume.@Wollt Ihr" */
		sprintf(g_text_output_buf, g_tev_loc_txt[57]);
	/* POINT2.LTX[58]: "den Tempel betreten, oder" */
	/* POINT2.LTX[59]: "Euren Weg fortsetzen?" */
	sel = dlg_box_bmp(g_dialog, b, (char far *)0, g_text_output_buf, 2, g_tev_loc_txt[58], g_tev_loc_txt[59]);
	delete b;
	switch (sel) {
	case 1:
		g_group->set_loctype(LOC_VILLAGE_INN);
		g_travel_state = 4;
		break;
	default:
		g_travel_state = 6;
		break;
	}
	LEAVE()
}

void place_event_032(Travel far *p)
{
	signed short dummy;
	signed char sel;
	Bmp far *b;

	ENTER(TOWN_HILTORP, 0x1e)
	if (g_travel_state == 4)
		/* POINT2.LTX[83]: "Ihr steht wieder zwischen den niedrigen, in den Fels geschlagenen Wohnungen. Wollt Ihr" */
		sprintf(g_text_output_buf, g_tev_loc_txt[83]);
	else
		/* POINT2.LTX[78]: "Ihr erreicht das an einem Steilhang gelegene Zwergenstaedtchen Hiltorp. Erst auf den zweiten
		 * Blick erkennt Ihr, dass im Hang Gebaeude sind, denn der groesste Teil der Zwergenbehausungen liegt unter der
		 * Erde, freistehende Bauten gibt es nicht.@Mitten im Hang steht ein Ingerimmtempel, direkt daneben entdeckt Ihr
		 * eine Schmiede, in der fleissig gearbeitet wird. Auf halbem Weg dorthin gibt es eine Herberge. Wollt Ihr" */
		sprintf(g_text_output_buf, g_tev_loc_txt[78]);
	/* POINT2.LTX[79]: "den Tempel des Zwergen- und Schmiedegottes betreten" */
	/* POINT2.LTX[80]: "in die Herberge gehen" */
	/* POINT2.LTX[81]: "den Schmied bei seiner Arbeit unterbrechen" */
	/* POINT2.LTX[82]: "oder die Leute vom kleinen Volk in ihrem Dorf zuruecklassen?" */
	sel = dlg_box_bmp(g_dialog, b, (char far *)0, g_text_output_buf, 4, g_tev_loc_txt[79], g_tev_loc_txt[80],
		g_tev_loc_txt[81], g_tev_loc_txt[82]);
	delete b;
	if (sel == 4 || sel == -1)
		g_travel_state = 6;
	else {
		if (sel == 1)
			g_group->set_loctype(LOC_VILLAGE_TEMPLE);
		if (sel == 2)
			g_group->set_loctype(LOC_VILLAGE_INN);
		if (sel == 3)
			g_group->set_loctype(LOC_VILLAGE_SMITH);
		g_travel_state = 4;
	}
	LEAVE()
}
