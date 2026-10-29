/* OVL126 - Overlay O126 (Segment 0x21e9 in SCHWEIF.EXE): Flusshaefen am Svellt - drei Orte der Reisekarte mit
 * Herberge und Bootsfahrt (Ortsereignisse aus place_event O210; Reiseobjekt p = Travel, Texte g_tev_txt = ALL_SECT.LTX
 * und g_tev_loc_txt = POINT2.LTX, Bilder nvfile_load(g_nvf_heads, 0x196/0x198, ..)). Gemeinsamer Ablauf: Ort setzen
 * (g_group->town, alter Ort nach old_town), beim ersten Betreten (g_travel_state == 0) Tagebuch ALL_SECT.LTX[112]
 * "Wir haben ein kleines Staedtchen namens %s erreicht." mit Ortsname GLOBLTXT.LTX[355 + Ort], dann Menueschleife mit
 * Bild 0x198 (new Bmp 64x64) und dlg_box_bmp: 1 = Herberge (set_loctype(LOC_VILLAGE_INN), g_travel_state = 4),
 * 2 = Bootsfahrt, 3 = weiter zu Fuss (g_travel_state = 0). Die Bootsfahrt traegt die Orte der Fahrt als Route in
 * p->route[] ein (Ende ROUTE_END, vorher route_clear), bezahlt mit group_sub_money; Erfolg g_on_boat = 1 und Tagebuch
 * ALL_SECT.LTX[116] "In %s haben wir uns entschlossen auf dem Wasserwege weiterzureisen.", zu wenig Geld
 * POINT2.LTX[55] und kein zweiter Versuch (state 2, g_on_boat = -1). Am Ende g_travel_state 4 (Boot/Herberge) bzw. 6
 * (Ort verlassen: town = 0, g_on_boat = 0).
 * place_event_028 (Svellmia, POINT2.LTX[46..56]): Schiffer legt in vier Stunden ab (timewarp_hours 4): Bild 0x196/4,
 *   Tiefhusen 9 Dukaten (Route 0x1c, 0xc4, 0xc5, 0x10), Hilvalla 12 (+ 0x1e), Norhus 15 (+ 0x1f) - die Faelle fallen
 *   von Norhus ueber Hilvalla nach Tiefhusen durch und fuellen die Route von hinten.
 * place_event_030 (Hilvalla, POINT2.LTX[61..69]): Faehrstation, Bild 0x198/1: Norhus 5 Dukaten (Route 0x1e, 0x1f),
 *   Tiefhusen 3 Dukaten (Route 0x1e, 0x10).
 * place_event_031 (Norhus, POINT2.LTX[70..77]): Boot flussaufwaerts, Bild 0x196/5 mit Titel " ": Tiefhusen fuer
 *   6 Dukaten (Route 0x1f, 0x1e, 0x10; der Schiffer darf in Hilvalla nicht anlegen).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL126.CPP   -> 3586/3586 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - C++ wegen group::set_loctype (inline wie O183: -Og-Zwischenwert [bp-1bh]/[bp-17h], loses cbw); -Og noetig
 *   (Zwischenwert dir, Adresse &p->route[i] in SI fuer den Abschluss ROUTE_END).
 * - Ortsname als g_ttx[TTX_TOWN_NAMES + town] (char far * far *): mov es,[b67e] / add ax,[b67c] / [bx+58ch].
 * - place_event_028: old OHNE register -> DI (SI braucht -Og fuer &p->route[i]); in den beiden anderen register old
 *   -> SI.
 * - Innerer switch mit 4 Faellen -> Sprungtabelle mit Rumpfreihenfolge case 3, 2, 1, 4 (Durchfallen), mit 3
 *   Faellen (place_event_030) Vergleichskette. Konstante 4L -> o32 push byte, 600L -> o32 push dword.
 * - Lokalreihenfolge je Funktion verschieden (place_event_031: money zuerst; state = 3 vor done = 0).
 * Auffaellig: place_event_030 setzt bei zu wenig Geld g_on_boat nicht auf -1 (anders als die beiden anderen); die
 *   Zeit (vier Stunden bis zur Abfahrt) wird nur in Svellmia weitergestellt.
 */
#include <stdio.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:631a..631c, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */

void place_event_028(Travel far *p)
{
	signed short dummy;
	signed short w;
	signed short h;
	signed char sel;
	signed char i;
	signed char state;
	signed char done;
	signed long money;
	signed long cost;
	Bmp far *b;
	Bmp far *b2;
	signed short old;

	done = 0;
	state = 3;
	cost = 200;
	old = dlg_set16(g_dialog, 6);
	g_group->old_town = g_group->town;
	g_group->town = TOWN_SVELLMIA;
	if (g_travel_state == 0) {
		/* ALL_SECT.LTX[112]: "Wir haben ein kleines Staedtchen namens %s erreicht." */
		sprintf(g_text_output_buf, g_tev_txt[112], g_ttx[TTX_TOWN_NAMES + g_group->town]);
		diary_add(g_diary, g_text_output_buf, 0);
	}
	while (!done) {
		if (g_travel_state == 4 || g_on_boat == -1)
			/* POINT2.LTX[56]: "Ihr seid wieder im Ortskern. Wollt Ihr" */
			sprintf(g_text_output_buf, g_tev_loc_txt[56]);
		else
			/* POINT2.LTX[46]: "Der Ort Svellmia, der am Zusammenfluss von Svellt und Svall liegt, ist nichts weiter als ein
			 * kleiner Flusshafen. Eine kleine Herberge bietet sich zur Rast an. Wollt Ihr" */
			sprintf(g_text_output_buf, g_tev_loc_txt[46]);
		b = new Bmp(0x40, 0x40);
		nvfile_load(g_nvf_heads, 0x198, b->data(), (void far *)0, 0, 0, &dummy, &dummy, 0x140);
		if (g_on_boat == -1) {
			/* POINT2.LTX[47]: "die Herberge unsicher machen" */
			/* POINT2.LTX[49]: "oder Euren Weg zu Fuss fortsetzen?" */
			sel = dlg_box_bmp(g_dialog, b, (char far *)0, g_text_output_buf, 2, g_tev_loc_txt[47],
				g_tev_loc_txt[49]);
			if (sel == 2)
				sel = 3;
		} else {
			/* POINT2.LTX[48]: "am Flusshafen schauen, ob die naechsten Tage ein Boot abfaehrt" */
			sel = dlg_box_bmp(g_dialog, b, (char far *)0, g_text_output_buf, 3, g_tev_loc_txt[47],
				g_tev_loc_txt[48], g_tev_loc_txt[49]);
		}
		dlg_set16(g_dialog, old);
		delete b;
		switch (sel) {
		case 1:
			g_group->set_loctype(LOC_VILLAGE_INN);
			g_travel_state = 4;
			done = 1;
			break;
		case 2:
			if (state == 3) {
				money = g_group->money;
				nvfile_info(g_nvf_heads, 0x196, 4, &w, &h);
				b2 = new Bmp(w, h);
				nvfile_load(g_nvf_heads, 0x196, b2->data(), (void far *)0, 4, 0, &dummy, &dummy,
					0x140);
				g_dlg_box_flag = 0;
				/* POINT2.LTX[50]: "Als Ihr die Anlegestelle erreicht seht Ihr gerade einen Schiffer, der sein Boot belaedt. Er
				 * will in vier Stunden ablegen und wuerde Euch fuer folgende Preise mitnehmen. Allerdings muesstet Ihr sofort
				 * aufs Boot und im Voraus bezahlen" */
				/* POINT2.LTX[51]: "Tiefhusen fuer 9 Dukaten" */
				/* POINT2.LTX[52]: "Hilvalla fuer 12 Dukaten" */
				/* POINT2.LTX[53]: "Norhus fuer 15 Dukaten" */
				/* POINT2.LTX[54]: "Natuerlich koennt Ihr auch wieder zurueck ins Dorf gehen." */
				sel = dlg_box_bmp(g_dialog, b2, (char far *)0, g_tev_loc_txt[50], 4,
					g_tev_loc_txt[51], g_tev_loc_txt[52], g_tev_loc_txt[53],
					g_tev_loc_txt[54]);
				delete b2;
				if (sel != 4)
					route_clear(p);
				/* Faelle fallen durch: Norhus -> Hilvalla -> Tiefhusen */
				switch (sel) {
				case 3:
					if (money >= 1500) {
						p->route[5] = 0x1f;
						cost = 1500;
					} else {
						cost = 0;
					}
				case 2:
					if (money >= 1200 && cost != 0) {
						p->route[4] = 0x1e;
						if (cost != 1500)
							cost = 1200;
					} else {
						cost = 0;
					}
				case 1:
					if (money >= 900 && cost != 0) {
						p->route[0] = 0x1c;
						p->route[1] = 0xc4;
						p->route[2] = 0xc5;
						p->route[3] = 0x10;
						if (cost != 1500 && cost != 1200)
							cost = 900;
						for (i = 0; p->route[i] != 0; i++)
							;
						p->route[i] = ROUTE_END;
					} else {
						cost = 0;
					}
					if (cost != 0) {
						g_on_boat = 1;
						group_sub_money(g_group, cost);
						timewarp_hours(g_date, 4L);
					} else {
						/* POINT2.LTX[55]: "Fuer eine solche Bootsfahrt fehlt Euch leider das noetige Geld!" */
						dlg_message(g_dialog, g_tev_loc_txt[55]);
						state = 2;
						g_on_boat = -1;
					}
					break;
				case 4:
					state = 2;
					g_on_boat = -1;
					break;
				}
			}
			if (g_on_boat == 1) {
				done = 1;
				/* ALL_SECT.LTX[116]: "In %s haben wir uns entschlossen auf dem Wasserwege weiterzureisen." */
				sprintf(g_text_output_buf, g_tev_txt[116], g_ttx[TTX_TOWN_NAMES + g_group->town]);
				diary_add(g_diary, g_text_output_buf, 0);
			}
			break;
		case 3:
			done = 1;
			g_travel_state = 0;
			break;
		}
	}
	if (g_on_boat == 1)
		g_travel_state = 4;
	if (done == 1 && g_travel_state != 4)
		g_travel_state = 6;
	if (g_travel_state == 6) {
		g_group->old_town = g_group->town;
		g_group->town = 0;
		g_on_boat = 0;
	}
}

void place_event_030(Travel far *p)
{
	signed short dummy;
	signed char sel;
	signed char state;
	signed char done;
	signed long money;
	signed long cost;
	Bmp far *b;
	Bmp far *b2;
	register signed short old;

	done = 0;
	state = 3;
	cost = 200;
	old = dlg_set16(g_dialog, 6);
	g_group->old_town = g_group->town;
	g_group->town = TOWN_HILVALLA;
	if (g_travel_state == 0) {
		/* ALL_SECT.LTX[112]: "Wir haben ein kleines Staedtchen namens %s erreicht." */
		sprintf(g_text_output_buf, g_tev_txt[112], g_ttx[TTX_TOWN_NAMES + g_group->town]);
		diary_add(g_diary, g_text_output_buf, 0);
	}
	while (!done) {
		if (g_travel_state == 4 || g_on_boat == -1)
			/* POINT2.LTX[69]: "Was habt Ihr als naechstes vor. Wollt Ihr" */
			sprintf(g_text_output_buf, g_tev_loc_txt[69]);
		else
			/* POINT2.LTX[61]: "Hilvalla als Dorf zu bezeichnen waere uebertrieben. Ausser der Faehrstation und der Herberge
			 * hat Euch der Ort nichts zu bieten. Wollt Ihr" */
			sprintf(g_text_output_buf, g_tev_loc_txt[61]);
		b = new Bmp(0x40, 0x40);
		nvfile_load(g_nvf_heads, 0x198, b->data(), (void far *)0, 0, 0, &dummy, &dummy, 0x140);
		/* POINT2.LTX[62]: "in die Herberge" */
		/* POINT2.LTX[63]: "zur Faehrstation" */
		/* POINT2.LTX[64]: "oder wie die anderen den Ort so schnell wie moeglich verlassen?" */
		sel = dlg_box_bmp(g_dialog, b, (char far *)0, g_text_output_buf, 3, g_tev_loc_txt[62],
			g_tev_loc_txt[63], g_tev_loc_txt[64]);
		dlg_set16(g_dialog, old);
		delete b;
		switch (sel) {
		case 1:
			g_group->set_loctype(LOC_VILLAGE_INN);
			g_travel_state = 4;
			done = 1;
			break;
		case 2:
			if (state == 3) {
				money = g_group->money;
				b2 = new Bmp(0x40, 0x40);
				nvfile_load(g_nvf_heads, 0x198, b2->data(), (void far *)0, 1, 0, &dummy, &dummy,
					0x140);
				/* POINT2.LTX[65]: "Die Faehrstation ist zur Zeit nicht besetzt. Aber Ihr findet folgende Bekanntmachung:@<Der
				 * Faehrverkehr wird aufrecht erhalten; zum Uebersetzen muss man den Ort in westlicher Richtung verlassen.
				 * Ausserdem wird bei Nachfrage auf einem der Boote eine Fahrt in beide Richtungen angeboten.> Wollt Ihr" */
				/* POINT2.LTX[66]: "fuer 5^Dukaten nach Norhus schippern" */
				/* POINT2.LTX[67]: "fuer 3^Dukaten nach Tiefhusen fahren" */
				/* POINT2.LTX[68]: "oder in dem Ortskern eine andere Wahl treffen?" */
				sel = dlg_box_bmp(g_dialog, b2, (char far *)0, g_tev_loc_txt[65], 3,
					g_tev_loc_txt[66], g_tev_loc_txt[67], g_tev_loc_txt[68]);
				delete b2;
				if (sel != 3)
					route_clear(p);
				switch (sel) {
				case 1:
					if (money >= 500) {
						p->route[0] = 0x1e;
						p->route[1] = 0x1f;
						cost = 500;
					} else {
						cost = 0;
					}
				case 2:
					if (cost == 200) {
						if (money >= 300) {
							p->route[0] = 0x1e;
							p->route[1] = 0x10;
							cost = 300;
						} else {
							cost = 0;
						}
					}
					if (cost != 0) {
						g_on_boat = 1;
						group_sub_money(g_group, cost);
						p->route[2] = ROUTE_END;
						done = 1;
						/* ALL_SECT.LTX[116]: "In %s haben wir uns entschlossen auf dem Wasserwege weiterzureisen." */
						sprintf(g_text_output_buf, g_tev_txt[116],
							g_ttx[TTX_TOWN_NAMES + g_group->town]);
						diary_add(g_diary, g_text_output_buf, 0);
					} else {
						/* POINT2.LTX[55]: "Fuer eine solche Bootsfahrt fehlt Euch leider das noetige Geld!" */
#if FIX_0055
						/* Fix #55: bei zu wenig Geld g_on_boat = -1 wie in den Parallelorten */
						dlg_message(g_dialog, g_tev_loc_txt[55]);
						state = 2;
						g_on_boat = -1;
					}
					break;
				case 3:
#else
						dlg_message(g_dialog, g_tev_loc_txt[55]);
						state = 2;
					}
					break;
				case 3:
#endif
					state = 2;
					g_on_boat = -1;
					break;
				}
			}
			break;
		case 3:
			done = 1;
			g_travel_state = 0;
			break;
		}
	}
	if (g_on_boat == 1)
		g_travel_state = 4;
	if (done == 1 && g_travel_state != 4)
		g_travel_state = 6;
	if (g_travel_state == 6) {
		g_group->old_town = g_group->town;
		g_group->town = 0;
		g_on_boat = 0;
	}
}

void place_event_031(Travel far *p)
{
	signed long money;
	signed short dummy;
	signed short w;
	signed short h;
	signed char sel;
	signed char state;
	signed char done;
	Bmp far *b;
	Bmp far *b2;
	register signed short old;

	state = 3;
	done = 0;
	g_group->old_town = g_group->town;
	g_group->town = TOWN_NORHUS;
	if (g_travel_state == 0) {
		/* ALL_SECT.LTX[112]: "Wir haben ein kleines Staedtchen namens %s erreicht." */
		sprintf(g_text_output_buf, g_tev_txt[112], g_ttx[TTX_TOWN_NAMES + g_group->town]);
		diary_add(g_diary, g_text_output_buf, 0);
	}
	while (!done) {
		if (g_travel_state == 4 || g_on_boat == -1)
			/* POINT2.LTX[77]: "Ihr erreicht wieder die Strassenkreuzung und steht vor der Herberge. Wollt Ihr" */
			sprintf(g_text_output_buf, g_tev_loc_txt[77]);
		else
			/* POINT2.LTX[70]: "Ihr erreicht Norhus, einen wichtigen Knotenpunkt im Svellttal. Die Svelltuferstrasse fuehrt
			 * durch den Ort und die Strasse nach Riva beginnt hier. Wen wundert es, dass in einem solchen Ort eine grosse
			 * Herberge steht. Wollt Ihr" */
			sprintf(g_text_output_buf, g_tev_loc_txt[70]);
		b = new Bmp(0x40, 0x40);
		nvfile_load(g_nvf_heads, 0x198, b->data(), (void far *)0, 0, 0, &dummy, &dummy, 0x140);
		if (g_on_boat == -1) {
			/* POINT2.LTX[71]: "diese Herberge aufsuchen" */
			/* POINT2.LTX[73]: "oder Eure Reise zu Fuss fortsetzen?" */
			sel = dlg_box_bmp(g_dialog, b, (char far *)0, g_text_output_buf, 2, g_tev_loc_txt[71],
				g_tev_loc_txt[73]);
			if (sel == 2)
				sel = 3;
		} else {
			/* POINT2.LTX[72]: "am Fluss nach einem Schiff Ausschau halten" */
			sel = dlg_box_bmp(g_dialog, b, (char far *)0, g_text_output_buf, 3, g_tev_loc_txt[71],
				g_tev_loc_txt[72], g_tev_loc_txt[73]);
		}
		delete b;
		switch (sel) {
		case 1:
			g_group->set_loctype(LOC_VILLAGE_INN);
			g_travel_state = 4;
			done = 1;
			break;
		case 2:
			old = dlg_set16(g_dialog, 6);
			money = g_group->money;
			if (state == 3) {
				nvfile_info(g_nvf_heads, 0x196, 5, &w, &h);
				b2 = new Bmp(w, h);
				nvfile_load(g_nvf_heads, 0x196, b2->data(), (void far *)0, 5, 0, &dummy, &dummy,
					0x140);
				g_dlg_box_flag = 0;
				/* POINT2.LTX[74]: "Als Ihr am Fluss ankommt seht Ihr gerade ein Boot, das flussaufwaerts faehrt. Als Ihr dem
				 * Schiffer zuruft, ob er Euch mitnimmt, entgegnet er: <Fuer 6^Dukaten bis Tiefhusen, allerdings darf ich in
				 * Hilvalla nicht anlegen, weil ich die Gebuehren dort nicht bezahlt habe.> Wollt Ihr" */
				/* POINT2.LTX[75]: "den Schiffer zum Anlegen veranlassen, oder" */
				/* POINT2.LTX[76]: "wieder in den Ort zurueck?" */
				sel = dlg_box_bmp(g_dialog, b2, " ", g_tev_loc_txt[74], 2,
					g_tev_loc_txt[75], g_tev_loc_txt[76]);
				delete b2;
				if (sel == 1) {
					if (money >= 600) {
						route_clear(p);
						p->route[0] = 0x1f;
						p->route[1] = 0x1e;
						p->route[2] = 0x10;
						p->route[3] = ROUTE_END;
						g_on_boat = 1;
						group_sub_money(g_group, 600L);
						done = 1;
						/* ALL_SECT.LTX[116]: "In %s haben wir uns entschlossen auf dem Wasserwege weiterzureisen." */
						sprintf(g_text_output_buf, g_tev_txt[116],
							g_ttx[TTX_TOWN_NAMES + g_group->town]);
						diary_add(g_diary, g_text_output_buf, 0);
					} else {
						/* POINT2.LTX[55]: "Fuer eine solche Bootsfahrt fehlt Euch leider das noetige Geld!" */
						dlg_message(g_dialog, g_tev_loc_txt[55]);
						state = 2;
						g_on_boat = -1;
					}
				} else {
					state = 2;
					g_on_boat = -1;
				}
			}
			dlg_set16(g_dialog, old);
			break;
		case 3:
			done = 1;
			g_travel_state = 0;
			break;
		}
	}
	if (g_on_boat == 1)
		g_travel_state = 4;
	if (done == 1 && g_travel_state != 4)
		g_travel_state = 6;
	if (g_travel_state == 6) {
		g_group->old_town = g_group->town;
		g_group->town = 0;
		g_on_boat = 0;
	}
}
