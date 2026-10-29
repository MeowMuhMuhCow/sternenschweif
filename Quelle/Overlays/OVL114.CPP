/* OVL114 - Overlay O114 (Segment 0x218d in SCHWEIF.EXE): Reiseereignisse der Wege 9..36 (tevent_run O211; Reiseobjekt
 * o mit Ort o->from, Abbruch o->action/ev_half/ev_start/ev_end; Merker route.event in g_routes; Texte g_tev_loc_txt =
 * SECTION.LTX (Wege 9..24) bzw. SECTION0.LTX (ab 25), g_tev_txt = ALL_SECT.LTX; Bilder 64x64 aus Datei 0x197).
 * tevent_009/019/023/034: Fuehrer-Angebote guide_offer je Ort (bei g_guide_won Ereignis aus; tevent_019 sonst
 *   ford_crossing). tevent_012/014/017: npc6_encounter ab Ort 0x17/0x2a/0x2c, tevent_013: show_once_anim, tevent_024
 *   leer.
 * tevent_010 (Orte 0x27/0x28, Zustand route.event[10]): Faehre ohne Faehrmann - warten (bis dreimal je 1 Stunde),
 *   dann Ueberfahrt fuer 30 Heller je Held (Bild 1, Gespraech 0x84 mit dem Faehrmann), umkehren, oder selbst
 *   uebersetzen (Fahrzeuge-Proben zweier Helden, misslungen: Proviantpakete und Gegenstaende verloren, Faehre
 *   gesunken: g_ferry_robbed = 1; danach Mann am Ufer txt[48] und Abbruch). Zustand 2: ferry_crossing(o, 1, 4).
 * tevent_016/tevent_036: Pflanzenkunde-Probe mit wachsender Erschwernis (route.event * 3 + 5) -> W3 Wirselkraut
 *   (und W2 Belmart). tevent_020/tevent_035: Zufallsbegegnung random_encounter (einmalig W6 + 4, sonst bei W10 <= 5
 *   W6 + 2). tevent_022: Kampf 0x2f/0x30 je Richtung. tevent_030: Ueberfall 0x82 mit wachsender Staerke (Ort 0x31,
 *   travel_stage == 2). tevent_025: Wasserfall (Bildfolgen 0x1d6/0x1d7) - spenden: halbes Gruppengeld, Tsa heilt alle
 *   (group_heal_full), Tagebuch ALL_SECT[118]. Die Indizes passen zu SECTION6.LTX[0..4]; Weg 25 laedt nach O211
 *   aber SECTION0.LTX (dort Warzennashorn) - vermutlich Fehlnummerierung im Original oder unerreichbar.
 * tevent_031: Warzennashorn (Bild 3) - Angriff/Flucht auf einen Baum; Schaden max(3W6 + 2 - rs_bonus, 5) am
 *   schlechtesten Akrobaten. tevent_032: Steppenrinder (Bild 4) ausserhalb des Winters im Zyklus 1..4.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL114.CPP   -> 5037/5037 Bytes GLEICH (18.10.2026), cobjfix laeuft.
 *   Mit -Oi ebenfalls GLEICH, ohne -Og 3 Bytes zu kurz (Epilogkopien).
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH; die Wegtabelle als g_routes[k].event statt der
 *   Bytesicht c7d0[10 * k + 8])
 * - Erster Entwurf bis auf eine Stelle GLEICH (Aufbau wie OVL122/OVL123): tevent_025 "c = g_group->get_money()" mit
 *   inline group::get_money() -> mov eax,es:[bx+2fh] / mov [bp-2],al; direkt mov al.
 * - tevent_010: zwei Bildobjekte per new/delete (Bmp-Konstruktor (0, ..) / Bmp_dtor(p, 3)) in getrennten
 *   Zeigervariablen
 *   b1/b2; switch 1..3 als Vergleichskette, jedes break mit -Og als Epilogkopie; cost in SI, (signed long)cost
 *   als -Og-Zwischenwert [bp-20h]; "tries += 1" (char) -> mov al / inc al / mov.
 * - tevent_031: Schaden als ausgeschriebenes max-Makro - der Vergleich rechnet 16-bittig (dx), der Wert 8-bittig
 *   (dl).
 * - tevent_020/tevent_035: "if (..) { ..; if/else; return; }" -> jmp short auf eigene Epilogkopie.
 * - tevent_036: unbenutztes "signed char b" (Kopie von tevent_016) noetig fuer sub sp,8.
 * Originalfehler: tevent_025 nimmt das halbe Gruppengeld ueber ein signed-char-Local (c = money, nur das untere Byte,
 *   vorzeichenbehaftet) - abgezogen werden -64..63 statt der Haelfte (bei negativem Wert gibt es sogar Geld dazu).
 *   tevent_031: das max-Makro wuerfelt fuer den Wert neu (3W6 zweimal ausgewertet). tevent_036 begrenzt den Zaehler
 *   auf 41,
 *   41 * 3 + 5 = 128 laeuft im signed-char-Parameter von test_skill auf -128 ueber; tevent_016 begrenzt gar nicht.
 */
#include <stdio.h>

#include "schweif.h"

void tevent_009(Travel far *o)
{
	if (o->from == 3)
		guide_offer(o, 1);
	if (o->from == 0x27)
		guide_offer(o, 2);
	if (g_guide_won != 0)
		g_routes[9].event = 0;
}

void tevent_010(Travel far *o)
{
	signed short d;
	signed char ans;
	signed char r;
	signed char n;
	signed char slot;
	Bmp far *b1;
	Bmp far *b2;
	struct hero far *h1;
	struct hero far *h2;
	struct hero far *h3;
	signed char tries = 0;
	signed short cost;

	if (o->from == 0x27 || o->from == 0x28) {
		if (g_ferry_robbed == 1) {
			b1 = new Bmp(0x40, 0x40);
			nvfile_load(g_nvf_heads, 0x197, b1->data(), 0, 2, 0, &d, &d, 0x140);
			/* SECTION.LTX[48]: "Am Ufer sitzt ein Mann, der hingebungsvoll an einem Knochen schnitzt. <Die Faehre ist
			 * abgesoffen.> teilt er Euch gleichmuetig mit, <Da muesst Ihr Euch wohl einen anderen Weg suchen, wenn Ihr
			 * nicht schwimmen wollt. Allerdings werdet Ihr mit dem Gepaeck nicht weit kommen...>" */
			dlg_box_bmp(g_dialog, b1, (char far *)0, g_tev_loc_txt[48], 0);
			o->action = 1;
			o->ev_half = 0;
			o->ev_start = 0;
			o->ev_end = 0;
			delete b1;
		}
		if (g_routes[10].event == 2 && g_ferry_robbed == 0)
			ferry_crossing(o, 1, 4);
		if (g_routes[10].event == 1) {
			b2 = new Bmp(0x40, 0x40);
			nvfile_load(g_nvf_heads, 0x197, b2->data(), 0, 0, 0, &d, &d, 0x140);
			/* SECTION.LTX[29]: "Da, wo die Strasse im Fluss zu verschwinden scheint, liegt eine Faehre am Steg; jedoch ist
			 * weit und breit niemand zu sehen, der Euch herueberfahren koennte. Wollt Ihr" */
			/* SECTION.LTX[30]: "Warten?" */
			/* SECTION.LTX[31]: "einen anderen Weg waehlen?" */
			/* SECTION.LTX[32]: "oder die Sache selber in die Hand nehmen?" */
			ans = dlg_box_bmp(g_dialog, b2, (char far *)0, g_tev_loc_txt[29], 3, g_tev_loc_txt[30], g_tev_loc_txt[31], g_tev_loc_txt[32]);
			while (ans == 1 && tries < 3) {
				timewarp_hours(g_date, 1L);
				/* SECTION.LTX[33]: "Es ist eine Stunde vergangen und immer noch ist niemand zu sehen. Wollt Ihr" */
				ans = dlg_box_bmp(g_dialog, b2, (char far *)0, g_tev_loc_txt[33], 3, g_tev_loc_txt[30], g_tev_loc_txt[31],
					g_tev_loc_txt[32]);
				tries += 1;
			}
			delete b2;
			if (ans == -1)
				ans = 2;
			switch (ans) {
			case 1:
				g_routes[10].event = 2;
				b1 = new Bmp(0x40, 0x40);
				nvfile_load(g_nvf_heads, 0x197, b1->data(), 0, 1, 0, &d, &d, 0x140);
				/* SECTION.LTX[34]: "Aus dem Waeldchen an der anderen Seite des Flusses seht Ihr eine Gestalt kommen, die ein
				 * groesseres Buendel ueber der Schulter traegt. Sie besteigt ein Boot und rudert herueber; dabei entpuppt sich
				 * das Buendel als ein kleines Reh. Auf Eurer Seite angekommen, nickt Euch der Mann mit einem knappen Gruss zu.
				 * <Wollt rueber, hm? Macht 3 Silber jeweils.>" */
				dlg_box_bmp(g_dialog, b1, (char far *)0, g_tev_loc_txt[34], 0);
				delete b1;
				cost = g_group->nr_heroes * 30;
				if ((signed long)cost > g_group->money) {
					/* SECTION.LTX[35]: "Wenn Ihr nicht genug Geld habt, muesst Ihr leider umdrehen." */
					dlg_message(g_dialog, g_tev_loc_txt[35]);
					o->action = 1;
					o->ev_half = 0;
					o->ev_start = 0;
					o->ev_end = 0;
				} else {
					group_sub_money(g_group, (signed long)cost);
					/* SECTION.LTX[36]: "Ihr gebt dem Mann %d Silbertaler und besteigt das Boot. Der Faehrmann legt ab, und
					 * steuert auf das andere Ufer zu. Wollt Ihr" */
					sprintf(g_text_output_buf, g_tev_loc_txt[36], cost / 10);
					/* SECTION.LTX[37]: "den Mann in ein Gespraech verwickeln oder" */
					/* SECTION.LTX[38]: "ihn bei seiner schweren Arbeit nicht stoeren?" */
					ans = dlg_choose(g_dialog, g_text_output_buf, 2, g_tev_loc_txt[37], g_tev_loc_txt[38]);
					if (ans == 1) {
						travel_leave(o);
						if (conv_run(g_dialog2, 0x84))
							timewarp_minutes(g_date, 30L);
						mouse_hide(0);
						travel_enter(o);
						mouse_show(0);
						travelmap_draw(o);
						travel_redraw(o);
					}
					/* SECTION.LTX[39]: "Ihr erreicht das andere Ufer und der Faehrmann macht sich nach einem knappen Nicken auf
					 * den Rueckweg." */
					dlg_message(g_dialog, g_tev_loc_txt[39]);
				}
				break;
			case 2:
				o->action = 1;
				o->ev_half = 0;
				o->ev_start = 0;
				o->ev_end = 0;
				break;
			case 3:
				n = group_count_check3(g_group);
				if (n == 1) {
					h1 = get_first_active_hero(g_group);
					/* SECTION.LTX[40]: "Allein ist %s nicht in der Lage das Boot zu steuern." */
					sprintf(g_text_output_buf, g_tev_loc_txt[40], h1->name);
					dlg_message(g_dialog, g_text_output_buf);
				} else {
					g_routes[10].event = 2;
					h1 = get_best_hero_skill(g_group, TA_FAHRZEUGE);
					do {
						h2 = get_random_hero(g_group);
					} while (h1 == h2);
					r = test_skill(h1, TA_FAHRZEUGE, 0);
					if (r > 0) {
						r = test_skill(h2, TA_FAHRZEUGE, 0);
						if (r > 0)
							/* SECTION.LTX[41]: "Wie erfahrene Seeleute steuert Ihr die Faehre an das andere Flussufer." */
							dlg_message(g_dialog, g_tev_loc_txt[41]);
					}
					if (r <= 0) {
						/* SECTION.LTX[42]: "Mitten auf dem Fluss reisst die Stroemung %s das Ruder aus der Hand. Die Faehre beginnt,
						 * sich um sich selbst drehend abzutreiben; bei den schlingernden Bewegungen rutscht Euer Gepaeck bedenklich
						 * nahe an den Rand des Bootes." */
						sprintf(g_text_output_buf, g_tev_loc_txt[42], h2->name);
						dlg_message(g_dialog, g_text_output_buf);
						while ((slot = get_item_pos(h2, ITEM_FOOD_PACKAGE)) >= 0)
							drop_item(h2, slot, 1);
						drop_item(h1, random_schweif(6) + 15, 1);
						drop_item(h2, random_schweif(6) + 20, 1);
						r = test_skill(h1, TA_FAHRZEUGE, 3);
						if (r > 0) {
							r = test_skill(h2, TA_FAHRZEUGE, 3);
							if (r > 0)
								/* SECTION.LTX[43]: "Mit einer gemeinsamen Anstrengung bekommt Ihr die Faehre wieder auf Kurs, gerade noch
								 * rechtzeitig, um einer kleinen Insel mitten im Fluss auszuweichen. Wenig spaeter erreicht Ihr
								 * Wohlbehalten das andere Ufer." */
								dlg_message(g_dialog, g_tev_loc_txt[43]);
						}
					}
					if (r <= 0) {
						if (n >= 3) {
							do {
								h3 = get_random_hero(g_group);
							} while (h3 == h1 || h3 == h2);
							/* SECTION.LTX[44]: "%s, %s und %s" */
							sprintf(g_text_buf2, g_tev_loc_txt[44], h1->name, h2->name, h3->name);
							/* SECTION.LTX[46]: "Eure hektischen Versuche, das Boot zu stabilisieren, machen alles nur noch schlimmer.
							 * Als dann %s %s" */
							/* SECTION.LTX[47]: "gleichzeitig an derselben Seite sich mit aller Kraft in die Riemen legen, beginnt Euer
							 * Fahrzeug gefaehrlich zu schwanken. Eine kleine Welle tut ihr uebriges, und die Faehre schlaegt um. Zum
							 * Glueck seid Ihr schon fast auf der anderen Seite, und so koennt Ihr sogar den groessten Teil Eurer
							 * Ausruestung retten. Allerdings ist Euer gesamter Proviant nass und ungeniessbar geworden." */
							sprintf(g_text_output_buf, g_tev_loc_txt[46], g_text_buf2, g_tev_loc_txt[47]);
							dlg_message(g_dialog, g_text_output_buf);
						} else {
							/* SECTION.LTX[45]: "%s und %s" */
							sprintf(g_text_buf2, g_tev_loc_txt[45], h1->name, h2->name);
							sprintf(g_text_output_buf, g_tev_loc_txt[46], g_text_buf2, g_tev_loc_txt[47]);
							dlg_message(g_dialog, g_text_output_buf);
						}
						while (group_find_item(g_group, ITEM_FOOD_PACKAGE) != -1)
							group_drop_item(g_group, ITEM_FOOD_PACKAGE);
						drop_item(h2, random_schweif(6) + 15, 1);
						drop_item(h1, random_schweif(6) + 20, 1);
						g_ferry_robbed = 1;
					}
				}
				break;
			}
		}
	}
}

void tevent_012(Travel far *o)
{
	if (o->from == 0x17)
		npc6_encounter(o);
}

void tevent_013(Travel far *o)
{
	show_once_anim(o);
}

void tevent_014(Travel far *o)
{
	if (o->from == 0x2a)
		npc6_encounter(o);
}

void tevent_016(Travel far *o)
{
	struct hero far *h;
	signed short r;
	signed char a;
	signed char b;

	h = get_best_hero_skill(g_group, TA_PFLANZENKUNDE);
	r = test_skill(h, TA_PFLANZENKUNDE, g_routes[16].event * 3 + 5);
	if (r > 0) {
		a = random_schweif(3);
		b = random_schweif(2);
		/* SECTION.LTX[49]: "%s entdeckt am Wegesrand %d Portion%s Wirselkraut und %d Portion%s Belmart." */
		/* ALL_SECT.LTX[5]: "en" */
		/* ALL_SECT.LTX[4]: "" */
		sprintf(g_text_output_buf, g_tev_loc_txt[49], h->name, a, a > 1 ? g_tev_txt[5] : g_tev_txt[4],
			b, b > 1 ? g_tev_txt[5] : g_tev_txt[4]);
		dlg_message(g_dialog, g_text_output_buf);
		group_give_item(g_group, ITEM_WHIRLWEED, a);
		group_give_item(g_group, ITEM_BELMART, b);
	}
#if FIX_0119
	/* Fix #119: Zaehler wie bei den Schwesterereignissen begrenzen */
	if (g_routes[16].event < 0x28)
		g_routes[16].event++;
	}
#else
	g_routes[16].event++;
}
#endif

void tevent_017(Travel far *o)
{
	if (o->from == 0x2c)
		npc6_encounter(o);
}

void tevent_019(Travel far *o)
{
	if (g_routes[19].event != 0x14) {
		if (o->from == 0x1c)
			guide_offer(o, 3);
		if (o->from == 0x2e)
			guide_offer(o, 4);
	}
	if (g_guide_won != 0)
		g_routes[19].event = 0x14;
	if (o->action != 1)
		ford_crossing(o);
}

void tevent_020(Travel far *o)
{
	signed char n = 0;

	if (g_routes[20].event == 1) {
		g_routes[20].event = g_routes[20].event + 1;
		n = random_schweif(6) + 4;
		if (o->from == 0x2e)
			random_encounter(o, n, 3);
		else
			random_encounter(o, n, 4);
		return;
	}
	n = random_schweif(10);
	if (n <= 5) {
		n = random_schweif(6) + 2;
		if (o->from == 0x2e)
			random_encounter(o, n, 3);
		else
			random_encounter(o, n, 4);
	}
}

void tevent_022(Travel far *o)
{
	signed char r;

	travel_leave(o);
	g_fight_flag = 1;
	if (o->from == 0x0d)
		r = start_fight(0x2f, 0);
	else
		r = start_fight(0x30, 0);
	r = fight_result(o, r);
	travel_enter(o);
	if (r == 0)
		g_routes[22].event = 0;
}

void tevent_023(Travel far *o)
{
	if (o->from == 0x1e)
		guide_offer(o, 3);
	if (o->from == 0x1f)
		guide_offer(o, 4);
	if (g_guide_won != 0)
		g_routes[23].event = 0;
}

void tevent_024(Travel far *o)
{
}

void tevent_025(Travel far *o)
{
	signed char ans;
	signed char c;
	void far *p;

	p = Anim_ctor(0, 0x1d6, 0, 0);
	/* SECTION6.LTX[0]: "Der Fluss stuerzt vor Euch in einem majestaetischen Wasserfall aus grosser Hoehe in sein
	 * Bett. Die Luft ist feucht, und in den Tropfen ist ein Regenbogen zu sehen. Angesichts dieses erhabenen Anblicks
	 * kann einen schon die Ehrfurcht packen..." */
	/* SECTION6.LTX[3]: "Spenden oder" */
	/* SECTION6.LTX[4]: "weitergehen" */
	ans = dlg_box_anim(g_dialog, p, (char far *)0, g_tev_loc_txt[0], 2, g_tev_loc_txt[3], g_tev_loc_txt[4]);
	Anim_dtor(p, 3);
	if (ans == 1) {
		p = Anim_ctor(0, 0x1d7, 0, 0);
		/* SECTION6.LTX[1]: "Als Ihr die Haelfte Eurer Muenzen mit einer Bitte um den Segen der Goetter in den Fluss
		 * werft, faellt Euch eine Eidechse auf einem Felsen im Wasser auf. Ihre gruenen Schuppen scheinen den Regenbogen
		 * ueber Euch zu reflektieren, und Euch durchstroemt ploetzlich ein koerperliches Wohlbefinden sondergleichen." */
		dlg_box_anim(g_dialog, p, (char far *)0, g_tev_loc_txt[1], 0);
#if FIX_0118
		/* Fix #118: halbes Gruppengeld als long (nicht nur das untere Byte) */
		group_sub_money(g_group, g_group->get_money() / 2);
#else
		c = g_group->get_money();
		group_sub_money(g_group, c / 2);
#endif
		group_heal_full(g_group);
		Anim_dtor(p, 3);
		/* ALL_SECT.LTX[118]: "Wir haben an einem Wasserfall den Goettern geopfert. Die Goettin Tsa, in Gestalt einer
		 * Eidechse, verhalf uns allen zu neuen Kraeften." */
		diary_add(g_diary, g_tev_txt[118], 0);
	}
}

void tevent_030(Travel far *o)
{
	signed char r;

	if (o->from == 0x31 && travel_stage(o) == 2) {
		g_routes[4].event = g_routes[4].event + 1;
		g_routes[5].event = g_routes[5].event + 1;
		g_routes[30].event = g_routes[30].event + 1;
		g_fight_max_enemies = g_routes[30].event >= 0x38 ? 12 : g_routes[30].event * 2 - 100;
		travel_leave(o);
		g_fight_flag = 1;
		r = start_fight(0x82, 0);
		fight_result(o, r);
		travel_enter(o);
	}
}

void tevent_031(Travel far *o)
{
	signed short d;
	signed char ans;
	struct hero far *h;
	signed char dmg;
	Bmp b(0x40, 0x40);

	nvfile_load(g_nvf_heads, 0x197, b.data(), 0, 3, 0, &d, &d, 0x140);
	h = get_worst_hero_skill(g_group, TA_AKROBATIK);
#if FIX_0119
	/* Fix #119: 3W6 einmal wuerfeln (Vergleich und Schaden aus demselben Wurf) */
	{
		signed short w_ = random_schweif(6) + random_schweif(6) + random_schweif(6) + 2 - h->rs_bonus;

		dmg = w_ > 5 ? w_ : 5;
	}
#else
	dmg = random_schweif(6) + random_schweif(6) + random_schweif(6) + 2 - h->rs_bonus > 5 ?
		random_schweif(6) + random_schweif(6) + random_schweif(6) + 2 - h->rs_bonus : 5;
#endif
	/* SECTION0.LTX[0]: "Ihr seht etwa hundert Schritt abseits der Strecke ein Warzennashorn grasen. Wollt Ihr" */
	/* SECTION0.LTX[1]: "Angreifen, weil Ihr dringend Proviant fuer die naechsten Monate braucht" */
	/* SECTION0.LTX[2]: "So unauffaellig wie moeglich Euren Weg fortsetzen, oder" */
	/* SECTION0.LTX[3]: "das Tier durch Schreien und Winken auf Euch aufmerksam machen." */
	ans = dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[0], 3, g_tev_loc_txt[1], g_tev_loc_txt[2], g_tev_loc_txt[3]);
	if (ans == -1)
		ans = 3;
	switch (ans) {
	case 1:
		/* SECTION0.LTX[4]: "Zwar ist das Tier etwas verunsichert durch Euer Verhalten, aber dennoch flieht es erst,
		 * nachdem es %s umgerannt hat." */
		sprintf(g_text_output_buf, g_tev_loc_txt[4], h->name);
		dlg_choose(g_dialog, g_text_output_buf, 0);
		sub_hero_le(h, dmg);
		break;
	case 2:
		break;
	case 3:
		/* SECTION0.LTX[5]: "Das Nashorn fuehlt sich durch Eure Anwesenheit belaestigt, und stuermt auf Euch zu. Wollt
		 * Ihr" */
		/* SECTION0.LTX[6]: "Stehenbleiben, oder" */
		/* SECTION0.LTX[7]: "den naechsten Baum erklimmen." */
		ans = dlg_choose(g_dialog, g_tev_loc_txt[5], 2, g_tev_loc_txt[6], g_tev_loc_txt[7]);
		if (ans == -1)
			ans = 1;
		if (ans == 1) {
			sprintf(g_text_output_buf, g_tev_loc_txt[4], h->name);
			dlg_choose(g_dialog, g_text_output_buf, 0);
			sub_hero_le(h, dmg);
		} else
			/* SECTION0.LTX[8]: "Das Tier ist durch Eure Flucht befriedigt und trollt sich von dannen, so dass Ihr die Reise
			 * unbeschadet fortsetzen koennt." */
			dlg_message(g_dialog, g_tev_loc_txt[8]);
		break;
	}
	g_routes[31].event = 0;
}

void tevent_032(Travel far *o)
{
	signed short d;
	signed char ans;

	if (date_season(g_date)) {
		if (g_routes[32].event == 2) {
			Bmp b(0x40, 0x40);
			nvfile_load(g_nvf_heads, 0x197, b.data(), 0, 4, 0, &d, &d, 0x140);
			/* SECTION0.LTX[9]: "Mehrere Steppenrinder schreiten aesend durch die saftig gruene Steppe, deren
			 * hochaufgeschossenes Gras sich mit wogenden Halmen vor den massigen Leibern teilt. Wollt Ihr" */
			/* SECTION0.LTX[10]: "Ihren Speisezettel mit frisch gepflueckten Steppenblumen bereichern," */
			/* SECTION0.LTX[11]: "Einen Versuch unternehmen, sie ohne Zuhilfenahme eines Sattels zuzureiten," */
			/* SECTION0.LTX[12]: "unter Zuhilfenahme roter Decken und Kleidungsstuecke einen Schaukampf improvisieren, oder" */
			ans = dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[9], 4, g_tev_loc_txt[10], g_tev_loc_txt[11], g_tev_loc_txt[12],
				/* SECTION0.LTX[13]: "einfach weitergehen." */
				g_tev_loc_txt[13]);
			switch (ans) {
			case 1:
				/* SECTION0.LTX[14]: "Bevor Ihr nahe genug an die Tiere herankommt, nehmen diese reissaus." */
				dlg_message(g_dialog, g_tev_loc_txt[14]);
				break;
			case 2:
				dlg_message(g_dialog, g_tev_loc_txt[14]);
				break;
			case 3:
				/* SECTION0.LTX[15]: "Die Tiere scheinen nicht zu verstehen, was Ihr von ihnen wollt, jedenfalls trotten sie
				 * gemuetlich davon." */
				dlg_message(g_dialog, g_tev_loc_txt[15]);
				break;
			case 4:
				break;
			}
			g_routes[32].event = g_routes[32].event + 1;
			return;
		}
		if (g_routes[32].event >= 4) {
			g_routes[32].event = 1;
			return;
		}
		g_routes[32].event = g_routes[32].event + 1;
	}
}

void tevent_034(Travel far *o)
{
	if (o->from == 0x34)
		guide_offer(o, 3);
	if (o->from == 0x35)
		guide_offer(o, 4);
	if (g_guide_won != 0)
		g_routes[34].event = 0;
}

void tevent_035(Travel far *o)
{
	signed char n = 0;

	if (g_routes[35].event == 1) {
		g_routes[35].event = g_routes[35].event + 1;
		n = random_schweif(6) + 4;
		if (o->from == 0x35)
			random_encounter(o, n, 1);
		else
			random_encounter(o, n, 2);
		return;
	}
	n = random_schweif(10);
	if (n <= 5) {
		n = random_schweif(6) + 2;
		if (o->from == 0x35)
			random_encounter(o, n, 1);
		else
			random_encounter(o, n, 2);
	}
}

void tevent_036(Travel far *o)
{
	struct hero far *h;
	signed short r;
	signed char a;
	signed char b;

	h = get_best_hero_skill(g_group, TA_PFLANZENKUNDE);
	r = test_skill(h, TA_PFLANZENKUNDE, g_routes[36].event * 3 + 5);
	if (r > 0) {
		a = random_schweif(3);
		/* ALL_SECT.LTX[3]: "%s entdeckt am Wegesrand %d Portion%s Wirselkraut!" */
		/* ALL_SECT.LTX[5]: "en" */
		/* ALL_SECT.LTX[4]: "" */
		sprintf(g_text_output_buf, g_tev_txt[3], h->name, a, a > 1 ? g_tev_txt[5] : g_tev_txt[4]);
		dlg_message(g_dialog, g_text_output_buf);
		group_give_item(g_group, ITEM_WHIRLWEED, a);
	}
#if FIX_0119
	/* Fix #119: Zaehler bis 40 (41 * 3 + 5 = 128 liefe im signed char ueber) */
	if (g_routes[36].event < 0x28)
		g_routes[36].event++;
#else
	if (g_routes[36].event <= 0x28)
		g_routes[36].event++;
#endif
}
