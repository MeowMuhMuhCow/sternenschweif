/* OVL031 - Overlay O031 (Segment 0x1fa1 in SCHWEIF.EXE): Ereignisorte 5..28 der belagerten Stadt Lowangen - Klasse
 * Ev (s_class.h, 8 Bytes: Texte LOWANGEN.LTX = Datei 0x21c, 0x78 Stueck), die O030 beim Betreten eines Ereignisortes
 * (Ortsart 17, Nummer g_loc_id) anlegt; O186 benutzt offering_bowl/bridge_toll. Die Ereignisroutinen aus O184
 * (tribute, story_13, talk_90, count_sneak_fails, wait_noon) werden mit this aufgerufen. Gespraeche ueber
 * talk(&g_talk, Datei, Partner, 0): LOW47..LOW68.TLK (0x2b0..0x2c4), LOW48.TLK = 0x90, LOW47CD.TLK = 0x4d8.
 * market_hall (5): Markthalle - Bettler (tribute) bzw. nach dem Verscheuchen meist leer. magistrate (6): Aushaenge
 *   (story_13), Gespraech im Magistrat. eolan (7): Zustand g_eolan_state (Gespraech LOW48, talk_90, Rueckkehr loc_return_redraw;
 *   mit Salamanderstein -> g_low_arrival_due). house_low49/academy/smithy (8..10): Gespraeche, Blickrichtung
 *   g_exit_dir.
 * showhouse (11): Einbruch ins Schauhaus - tagsueber (g_daytime im Fenster 0x93a8..0x190c8) ertappt, sonst
 *   Schloesser-Probe +10, Fragen mit Schleichen-Proben (count_sneak_fails 0/+4) bis 5 Fehlpunkte, Brosche;
 *   Ergebnis g_low_burglary_result mit Tagebuch txt[81..83].
 * guardhouse (12): Waffen (item_desc.weapon) im Rucksack zaehlen - unter 10: bei mehr Proviantpaketen als 2 je Held
 *   Offizierin (Portraet 0xb0) verlangt die Abgabe, Gespraech 0x47 (conv); sonst LOW53A (Waffen abliefern).
 * city_gate1/city_gate2 (13/14): LOW54 an den Stadttoren, neue Position. fortress_gate (15): Torhaus der Feste -
 *   Kampf 0x9a (verloren -> Kerker group_imprison(2)) oder zurueck. trade_office (16): geschlossen.
 * altrogolan/ailian/vindaria/jandora (17..20): Gespraeche je Zustand (ailian: Kampf 0xa4, Ablage 0x37; vindaria:
 *   Ablagen 0/1). dragan (21): Auftraege Dragans (NSC 8 abgeben, Brosche, Paeckchen) -> Gespraech 0x48 bzw. txt[116].
 * talk_49/zaskia/talk_4a (22..24): Gespraeche 0x49/LOW64/0x4a bzw. Gavron (g_low_gavron_house) -> wait_noon.
 * peddler_redhead/falkert (25/26): Haendler (lokales Peddler-Objekt, run, g_peddler loeschen). deringorn (27):
 *   Paeckchen abgeben. offering_bowl (28): Opferschale (Dukaten x100 / Silber x10 / Heller -> g_god_favor_tsa),
 *   Ablehnen -10. bridge_toll: 1 Silberstueck zahlen oder umdrehen ((dir + 2) & 3, Zeile g_low_bridge_y).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL031.CPP   -> 5179/5179 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 *   (-rd ebenso GLEICH; ohne -Og 2398, mit -Oi 3039 Abweichungen - strcat/memset sind echte Aufrufe).
 * - C++-Klasse mit Konstruktor (this == 0 -> new(8), toter Epilog) und Destruktor (flags in SI).
 * - guardhouse: "id = h->inv[i].id; if (id)" getrennt (mov di,es:[..] / or di,di); eingebettet "(id = ..) != 0"
 *   ergibt mov ax / mov di,ax / or ax,ax. -Og haelt &h->inv[i] in [bp-0ah] (Segment per mov es,[bp-2]).
 * - dragan: "group_set_slot(g, group_hero_pos(g, h), 0x63)" - die 0x63 wird vor dem inneren Aufruf gepusht.
 * - ailian: case 0 und 2 teilen den Rumpf (Sprungtabelle), "Stash far *l" auf Funktionsebene fuer beide new.
 * Originalfehler/Auffaelliges: eolan - in case 1 wird "g_eolan_state == 2" geprueft (dort nie wahr), group_leave_place(1)
 *   ist unerreichbar. offering_bowl: zweimal identisch "g_god_favor_tsa -= 10" (else-Zweige).
 */
#include <stdio.h>
#include <string.h>
#include <mem.h>

#include "schweif.h"

Ev::Ev()
{
	txt_load_list(g_font, 0x21c, 0x78, &txt_buf, &txt);
}

Ev::~Ev()
{
	mem_free(txt_buf);
	mem_free(txt);
}

void Ev::market_hall()
{
	register signed short r = 0;

	if (g_low_beggars_scared)
		r = random_schweif(5) - 1;
	if (!r) {
		tribute();
		return;
	}
	/* LOWANGEN.LTX[12]: "Nichts ruehrt sich, als Ihr die Markthalle betretet. Von den Bettlern, die neulich noch hier
	 * waren, ist nichts zu sehen..." */
	dlg_message(g_dialog, txt[12]);
}

void Ev::magistrate()
{
	register signed short r = 1;

	if (!g_low_notices_read) {
		r = story_13();
		if (!g_low_notices_read)
			g_exit_dir = 3;
	}
	if (r) {
		if (g_speech_ext_avail)
			talk(&g_talk, 0x4d8, 0, 0);
		else
			talk(&g_talk, 0x2b0, 0, 0);
		g_exit_dir = 3;
	}
}

void Ev::eolan()
{
	switch (g_eolan_state) {
	case 1:
		talk(&g_talk, 0x90, 1, 0);
		if (g_eolan_state == 2)
			group_leave_place(1);
		return;
	case 2:
		talk_90();
		return;
	case 3:
	case 4:
		if (g_low_tunnel_arrival) {
			g_low_tunnel_arrival = 0;
			loc_return_redraw();
			if (group_find_item(g_group, ITEM_SALAMANDER_STONE) != -1 && !g_low_stone_delivered)
				g_low_arrival_due = 1;
		} else
			group_leave_place(1);
		return;
	case 5:
		talk(&g_talk, 0x90, 5, 0);
		return;
	default:
		talk(&g_talk, 0x90, 0, 0);
		return;
	}
}

void Ev::house_low49()
{
	talk(&g_talk, 0x2b1, 0, 0);
	g_exit_dir = 2;
}

void Ev::academy()
{
	talk(&g_talk, 0x2b2, 0, 0);
	g_exit_dir = 3;
}

void Ev::smithy()
{
	talk(&g_talk, 0x2b3, 0, 0);
}

void Ev::showhouse()
{
	signed short old;
	struct hero far *h;
	signed short n;
	register signed short r;
	register signed short cnt;

	old = dlg_set16(g_dialog, 6);
	r = 0;
	cnt = 0;
	if (g_low_brooch_state > 0 && !g_low_brooch_taken) {
		/* LOWANGEN.LTX[27]: "<F1>Schauhaus der handwerklichen Kuenste<F0>@@Die Tuer des Gebaeudes ist verschlossen, aber
		 * Ihr werdet auf einen Aushang aufmerksam:@>Das Schauhaus bleibt bis auf weiteres geschlossen.>" */
		sprintf(g_text_output_buf, txt[27]);
		/* LOWANGEN.LTX[28]: "@Wollt Ihr" */
		strcat(g_text_output_buf, txt[28]);
		/* LOWANGEN.LTX[29]: "in das Gebaeude einsteigen?" */
		/* LOWANGEN.LTX[30]: "oder das Unternehmen abbrechen?" */
		r = dlg_choose(g_dialog, g_text_output_buf, 2, txt[29], txt[30]);
	} else
		dlg_message(g_dialog, txt[27]);
	if (r == 1) {
		if (g_daytime > 0x93a8 && g_daytime <= 0x190c8) {
			g_low_burglary_result = 1;
			/* LOWANGEN.LTX[81]: "Wir wurden beim Versuch, in das Schauhaus einzubrechen, ertappt. Tagsueber laufen einfach
			 * zuviele Patrouillen durch Lowangens Strassen." */
			diary_add(g_diary, txt[81], 0);
		} else {
			h = get_first_active_hero(g_group);
			if (test_skill(h, TA_SCHLOESSER, 10) > 0) {
				/* LOWANGEN.LTX[31]: "Geschickt und vor allem lautlos oeffnet %s die Fensterlaeden." */
				sprintf(g_text_output_buf, txt[31], h->name);
				dlg_message(g_dialog, g_text_output_buf);
				r = 1;
			} else {
				/* LOWANGEN.LTX[32]: "Es knarrt und quietscht, als %s die Fensterlaeden oeffnet.@Wollt Ihr" */
				sprintf(g_text_output_buf, txt[32], h->name);
				r = dlg_choose(g_dialog, g_text_output_buf, 2, txt[29], txt[30]);
				cnt++;
			}
			if (r == 1) {
				/* LOWANGEN.LTX[33]: "Das Gebaeude besteht fast nur aus einem einzigen Raum. Darin stehen mehrere leere Sockel,
				 * die offensichtlich zur Ausstellung von Kunstwerken dienen sollen.@Wollt Ihr" */
				/* LOWANGEN.LTX[34]: "Euch weiter umsehen?" */
				/* LOWANGEN.LTX[35]: "oder lieber abhauen?" */
				r = dlg_choose(g_dialog, txt[33], 2, txt[34], txt[35]);
				if (r == 1) {
					n = count_sneak_fails( 0);
					if (n) {
						/* LOWANGEN.LTX[36]: "Ihr stapft nicht gerade leise durch die Ausstellungshalle." */
						dlg_message(g_dialog, txt[36]);
						cnt += n;
					}
					if (cnt < 5) {
						/* LOWANGEN.LTX[37]: "In einem kleinen Raum neben der Halle fuehrt eine Holztreppe nach oben, ansonsten
						 * entdeckt Ihr nichts Bemerkenswertes.@Wollt Ihr" */
						/* LOWANGEN.LTX[38]: "Euch oben umsehen?" */
						/* LOWANGEN.LTX[39]: "oder doch lieber abhauen?" */
						r = dlg_choose(g_dialog, txt[37], 2, txt[38], txt[39]);
						if (r == 1) {
							n = count_sneak_fails( 4);
							if (n) {
								/* LOWANGEN.LTX[40]: "Das Holz der Treppe knarrt und quietscht gewaltig." */
								dlg_message(g_dialog, txt[40]);
								cnt += n;
							}
							if (cnt < 5) {
								/* LOWANGEN.LTX[41]: "Oben gibt es mehrere kleine Raeume.@Wollt Ihr" */
								/* LOWANGEN.LTX[42]: "die Raeume durchsuchen?" */
								/* LOWANGEN.LTX[43]: "oder nun doch endlich sehen, dass Ihr wegkommt?" */
								r = dlg_choose(g_dialog, txt[41], 2, txt[42], txt[43]);
								if (r == 1) {
									/* LOWANGEN.LTX[44]: "Im groessten der Raeume stehen mehrere Truhen, in denen die ehemaligen
									 * Ausstellungsstuecke verstaut sind. Darunter befindet sich auch eine Brosche mit dem Bild einer
									 * Esche.@Wollt Ihr" */
									/* LOWANGEN.LTX[45]: "nur die Brosche nehmen?" */
									/* LOWANGEN.LTX[46]: "oder den restlichen Krempel auch pruefen?" */
									r = dlg_choose(g_dialog, txt[44], 2, txt[45], txt[46]);
									if (r == 2) {
										/* LOWANGEN.LTX[47]: "Das verursacht unglaublichen Laerm." */
										dlg_message(g_dialog, txt[47]);
										cnt = 5;
									} else {
										group_give_item(g_group, ITEM_BROOCH, 1);
										g_low_brooch_taken = 1;
										/* LOWANGEN.LTX[48]: "Ihr nehmt die Brosche und seht zu, dass Ihr weg kommt." */
										dlg_message(g_dialog, txt[48]);
									}
								}
							}
						}
					}
				}
			}
			if (cnt >= 5) {
				g_low_burglary_result = 2;
				/* LOWANGEN.LTX[82]: "Wir sind in das Schauhaus der handwerklichen Kuenste eingebrochen. Einbruch zaehlt nicht
				 * gerade zu unseren Staerken : eine Partrouille wurde von dem Laerm, den wir verursachten, auf uns
				 * aufmerksam." */
				diary_add(g_diary, txt[82], 0);
			} else {
				g_low_burglary_result = 3;
				/* LOWANGEN.LTX[83]: "Einige von uns erwaegen inzwischen eine Karriere als Dieb: unser Einbruch in das
				 * Schauhaus der handwerklichen Kuenste war ein voller Erfolg!" */
				diary_add(g_diary, txt[83], 0);
			}
		}
	}
	dlg_set16(g_dialog, old);
}

void Ev::guardhouse()
{
	struct hero far *h;
	signed short bad;
	signed short sum;
	register signed short i;
	register signed short id;

	h = get_first_hero(g_group);
	bad = 0;
	sum = 0;
	g_low_guard_talk = 0;
	do {
		for (i = 15; i < 31; i++) {
			id = h->inv[i].item_id;
			if (id) {
				if (g_items[id].weapon)
					bad++;
				else if (id == ITEM_FOOD_PACKAGE)
					sum += h->inv[i].quantity;
			}
		}
		h = get_next_hero(g_group, h);
	} while (get_first_hero(g_group) != h);
	if (bad < 10) {
		if (g_group->nr_heroes * 2 < sum) {
			Bmp b(0x20, 0x20, g_portrait_buf);

			load_portrait_buf(0xb0);
			/* LOWANGEN.LTX[62]: "Gardehaus an der Eydaler Bruecke" */
			/* LOWANGEN.LTX[63]: "Der diensthabende Offizier, eine Frau, mustert Euch kurz und sagt dann: <Ihr habt eine
			 * Menge Nahrung dabei - liefert das umgehend im Magistrat ab, sonst muss ich es beschlagnahmen! Klar?!>" */
			dlg_box_bmp(g_dialog, &b, txt[62], txt[63], 0);
		}
		conv_clr_counter(g_dialog2, 0x47);
		g_low_guard_talk = 1;
	} else {
		conv_clr_counter(g_dialog2, 0x47);
		talk(&g_talk, 0x2b5, 0, 0);
	}
	if (g_low_guard_talk) {
		g_low_guard_talk = 0;
		conv_topic_on(g_dialog2, 0x17);
		if (conv_run(g_dialog2, 0x47))
			timewarp_minutes(g_date, 30);
		conv_set_topic(g_dialog2, TOPIC_WAFFEN, 0);
	}
	g_exit_dir = 3;
}

void Ev::city_gate1()
{
	g_low_gate1_open = 1;
	talk(&g_talk, 0x2b6, 0, 0);
	g_low_gate1_open = 0;
	group_set_screen_pos(6, 5);
	g_group->dir = 0;
}

void Ev::city_gate2()
{
	g_low_gate2_open = 1;
	talk(&g_talk, 0x2b6, 0, 0);
	g_low_gate2_open = 0;
	group_set_screen_pos(0x0c, 0x2a);
	g_group->dir = 2;
}

void Ev::fortress_gate()
{
	signed short old;
	signed short r;
	register signed short f;

	old = dlg_set16(g_dialog, 6);
	f = 0;
	/* LOWANGEN.LTX[65]: "<F1>Torhaus der Feste Lowangen<F0>@@Als Ihr durch das Tor in den Burghof der Feste Lowangen
	 * wollt, treten Euch sofort vier Gardisten mit gekreuzten Hellebarden in den Weg: <Halt! Hier hat niemand
	 * Zutritt!>@Wollt Ihr" */
	/* LOWANGEN.LTX[66]: "Euch den Zutritt durch Waffengewalt verschaffen?" */
	/* LOWANGEN.LTX[67]: "oder die Befehle der Gardisten befolgen?" */
	r = dlg_choose(g_dialog, txt[65], 2, txt[66], txt[67]);
	if (r == 1) {
		mem_free(txt_buf);
		mem_free(txt);
		f = start_fight(0x9a, 0);
		txt_load_list(g_font, 0x21c, 0x78, &txt_buf, &txt);
		if (!f) {
			screen_redraw(g_screen);
			/* LOWANGEN.LTX[68]: "Aus allen Richtungen rennen Bewaffnete auf Euch zu: Es ist zwecklos. Ihr muesst Euch
			 * ergeben!" */
			dlg_message(g_dialog, txt[68]);
			/* LOWANGEN.LTX[69]: "<Das kostet Euch zwei Tage Kerker!> bruellt der Hauptmann der Torgarde, wutentbrannt winkt
			 * er ein paar seiner Leute heran: <Ab mit dem Gesindel, dass es schnell aus meinen Augen kommt!>@Man legt Euch
			 * Fesseln an und fuehrt Euch ueber die belebten Strassen zum Kerker." */
			dlg_message(g_dialog, txt[69]);
			group_imprison(2);
		} else {
			group_set_screen_pos(5, 6);
			g_group->dir = 3;
		}
	} else {
		group_set_screen_pos(5, 6);
		g_group->dir = 3;
	}
	dlg_set16(g_dialog, old);
}

void Ev::trade_office()
{
	signed short old;

	old = dlg_set16(g_dialog, 6);
	/* LOWANGEN.LTX[64]: "<F1>Markt- und Handelssekretariat<F0>@@Die Tuer des Gebaeudes ist verschlossen, aber Ihr
	 * werdet auf einen Aushang aufmerksam:@@<Verfuegung gemaess des Notstandserlasses:@Die Dienste des Markt- und
	 * Handelssekretariats ruhen bis auf weiteres. Die Rechtsangelegenheiten des Handels werden weiterhin direkt von
	 * der Stadtgarde geregelt, entsprechende Begehren muessen im Gardehaus an der Eydaler Bruecke vorgetragen
	 * werden." */
	dlg_message(g_dialog, txt[64]);
	dlg_set16(g_dialog, old);
}

void Ev::altrogolan()
{
	if (g_altrogolan_absent) {
		/* LOWANGEN.LTX[70]: "Niemand reagiert auf Euer Klopfen." */
		dlg_message(g_dialog, txt[70]);
		return;
	}
	talk(&g_talk, 0x2ba, 0, 0);
}

void Ev::ailian()
{
	Stash far *l;

	switch (g_low_ailian_state) {
	case -1:
		l = new Stash(0x37, 0);
		delete l;
		g_gavron_chase = 0xfc;
		break;
	case 0:
	case 2:
		/* LOWANGEN.LTX[70]: "Niemand reagiert auf Euer Klopfen." */
		dlg_message(g_dialog, txt[70]);
		break;
	case 1:
		if (g_topic_siebenquell) {
			talk(&g_talk, 0x2bb, 0, 0);
			/* LOWANGEN.LTX[84]: "Wir haben Ailian Siebenquell besucht. Der Elf betrug sich ziemlich seltsam." */
			diary_add(g_diary, txt[84], 0);
			g_low_ailian_state = 2;
		} else
			dlg_message(g_dialog, txt[70]);
		break;
	case 3:
		g_low_ailian_state = 4;
		talk(&g_talk, 0x2bb, 1, 0);
		break;
	case 4:
		g_low_ailian_state = 5;
		talk(&g_talk, 0x2bb, 2, 0);
		break;
	case 5:
		/* LOWANGEN.LTX[0]: "Glaubt Ihr dem Mann sowenig, dass Ihr in sein Haus einbrechen wollt?" */
		if (dlg_yes_no(g_dialog, txt[0]) == 1) {
			mem_free(txt_buf);
			mem_free(txt);
			start_fight(0xa4, 0);
			txt_load_list(g_font, 0x21c, 0x78, &txt_buf, &txt);
		}
		g_topic_siebenquell = -1;
		break;
	}
	if (group_find_item(g_group, ITEM_SALAMANDER_STONE) != -1)
		g_low_stone_days = 0;
}

void Ev::vindaria()
{
	Stash far *l;

	if (!g_low_vindaria_known) {
		/* LOWANGEN.LTX[70]: "Niemand reagiert auf Euer Klopfen." */
		dlg_message(g_dialog, txt[70]);
		return;
	}
	switch (g_vindaria_state) {
	case -2:
		l = new Stash(1, 0);
		delete l;
		return;
	case -1:
		l = new Stash(0, 0);
		delete l;
		return;
	case 1:
		talk(&g_talk, 0x2bc, 0, 0);
		return;
	default:
		dlg_message(g_dialog, txt[70]);
		return;
	}
}

void Ev::jandora()
{
	if (!g_price_level) {
		talk(&g_talk, 0x2bd, 0, 0);
		return;
	}
	talk(&g_talk, 0x2bd, 1, 0);
}

void Ev::dragan()
{
	switch (g_dragan_task) {
	case 0:
		{
			struct hero far *h = group_get_npc(g_group);

			if (h != 0 && h->npc_id == 8) {
				if (!check_hero3(h))
					/* LOWANGEN.LTX[88]: "<Also - ein wenig Muehe koenntet Ihr Euch sohon geben.> meint Dragan mit einem wuetenden
					 * Blick auf den Vinsalter. <Das war wohl nichts, eh?>" */
					dlg_message(g_dialog, txt[88]);
				else {
					/* LOWANGEN.LTX[71]: "<Ah! Da seid Ihr ja wieder. Ich sehe, dass Ihr Euch wirklich viel Muehe gebt - also:
					 * eine Hand waescht die andere. Was wollt Ihr wissen?>" */
					dlg_message(g_dialog, txt[71]);
					g_low_dragan_news = 1;
				}
				conv_clr_counter(g_dialog2, 0x48);
				group_set_slot(g_group, group_hero_pos(g_group, h), 0x63);
				g_dragan_task = 1;
			}
		}
		break;
	case 1:
		{
			signed short r = group_find_item(g_group, ITEM_BROOCH);

			if (r != -1) {
				dlg_message(g_dialog, txt[71]);
				conv_clr_counter(g_dialog2, 0x48);
				group_drop_item(g_group, ITEM_BROOCH);
				g_low_brooch_state = -1;
				g_dragan_task = 2;
				g_low_dragan_news = 1;
			}
		}
		break;
	case 2:
		if (g_low_deringorn_state == 2) {
			dlg_message(g_dialog, txt[71]);
			conv_clr_counter(g_dialog2, 0x48);
			g_low_deringorn_state = -1;
			g_dragan_task = 3;
			g_low_dragan_news = 1;
		}
		break;
	case 3:
		conv_clr_counter(g_dialog2, 0x48);
		break;
	default:
		break;
	}
	if (g_low_dragan_task1 > 0 && g_dragan_task == 0 || g_low_brooch_state > 0 && g_dragan_task == 1 || g_low_dragan_task3 > 0 && g_dragan_task == 2) {
		/* LOWANGEN.LTX[116]: "<Ich meinte, wir hatten einen Handel abgeschlossen! Erfuellt Euren Teil, dann werde ich
		 * meinen beisteuern!> Schon hat Dragan die Tuere wieder geschlossen." */
		dlg_message(g_dialog, txt[116]);
		return;
	}
	if (conv_run(g_dialog2, 0x48))
		timewarp_minutes(g_date, 30);
}

void Ev::talk_49()
{
	if (!g_low_gavron_house) {
		if (conv_run(g_dialog2, 0x49))
			timewarp_minutes(g_date, 30);
	} else if (g_low_gavron_house != 0x3f)
		/* LOWANGEN.LTX[70]: "Niemand reagiert auf Euer Klopfen." */
		dlg_message(g_dialog, txt[70]);
	screen_redraw(g_screen);
	if (g_low_gavron_house == 0x3f)
		wait_noon();
}

void Ev::zaskia()
{
	if (!g_low_gavron_house && g_low_kysira_ref)
		talk(&g_talk, 0x2c0, 0, 0);
	else if (g_low_gavron_house != 0x40)
		/* LOWANGEN.LTX[70]: "Niemand reagiert auf Euer Klopfen." */
		dlg_message(g_dialog, txt[70]);
	if (g_low_gavron_house == 0x40)
		wait_noon();
}

void Ev::talk_4a()
{
	if (!g_low_gavron_house) {
		if (conv_run(g_dialog2, 0x4a))
			timewarp_minutes(g_date, 30);
	} else if (g_low_gavron_house != 0x41)
		/* LOWANGEN.LTX[70]: "Niemand reagiert auf Euer Klopfen." */
		dlg_message(g_dialog, txt[70]);
	screen_redraw(g_screen);
	if (g_low_gavron_house == 0x41)
		wait_noon();
}

void Ev::peddler_redhead()
{
	if (!g_peddler_away) {
		talk(&g_talk, 0x2c2, 0, 0);
		if (g_peddler) {
			Peddler s;

			s.run();
			memset(&g_peddler, 0, 0x11);
		}
		return;
	}
	/* LOWANGEN.LTX[70]: "Niemand reagiert auf Euer Klopfen." */
	dlg_message(g_dialog, txt[70]);
}

void Ev::falkert()
{
	if (g_low_falkert_known && !g_peddler3_away) {
		talk(&g_talk, 0x2c3, 0, 0);
		if (g_peddler) {
			Peddler s;

			s.run();
			memset(&g_peddler, 0, 0x11);
		}
		return;
	}
	/* LOWANGEN.LTX[70]: "Niemand reagiert auf Euer Klopfen." */
	dlg_message(g_dialog, txt[70]);
}

void Ev::deringorn()
{
	if (g_low_deringorn_state == 1) {
		talk(&g_talk, 0x2c4, 0, 0);
		/* LOWANGEN.LTX[85]: "Wir haben besagtes Paeckchen bei Meister Deringorn abgegeben. Eines koennen wir mit
		 * Sicherheit sagen. Er und Dragan sind wohl alles andere als gut aufeinander zu sprechen." */
		diary_add(g_diary, txt[85], 0);
		return;
	}
	/* LOWANGEN.LTX[70]: "Niemand reagiert auf Euer Klopfen." */
	dlg_message(g_dialog, txt[70]);
}

void Ev::offering_bowl()
{
	register signed short n;
	register signed short sel;

	/* LOWANGEN.LTX[72]: "Eine Opferschale steht direkt neben der Bruecke. Es liegen einige Heller darin. Wollt Ihr
	 * auch etwas spenden?" */
	if (dlg_yes_no(g_dialog, txt[72]) == 1) {
		/* LOWANGEN.LTX[73]: "Was wollt Ihr opfern?" */
		/* LOWANGEN.LTX[74]: "Dukaten" */
		/* LOWANGEN.LTX[75]: "Silbertaler" */
		/* LOWANGEN.LTX[76]: "Heller" */
		sel = dlg_choose(g_dialog, txt[73], 3, txt[74], txt[75], txt[76]);
		if (sel != -1) {
			/* LOWANGEN.LTX[77]: "Wieviel wollt Ihr opfern?" */
			n = dlg_input(g_dialog, 2, txt[77], 0, 0x63, 10);
			if (n > 0) {
				switch (sel) {
				case 1:
					group_sub_money(g_group, (signed long)n * 100);
					break;
				case 2:
					group_sub_money(g_group, (signed long)n * 10);
					break;
				case 3:
					group_sub_money(g_group, n);
					g_god_favor_tsa += n * 10;
				default:
					break;
				}
			} else
				g_god_favor_tsa -= 10;
		}
	} else
		g_god_favor_tsa -= 10;
}

void Ev::bridge_toll()
{
	register signed short f = 1;

	/* LOWANGEN.LTX[111]: "<Brueckenzoll, 1^Silberstueck!> sagt der Wachhabende an der Bruecke.@Wollt Ihr" */
	/* LOWANGEN.LTX[112]: "umdrehen?" */
	/* LOWANGEN.LTX[113]: "oder zahlen?" */
	if (dlg_choose(g_dialog, txt[111], 2, txt[112], txt[113]) == 2) {
		if (g_group->money < 10)
			/* LOWANGEN.LTX[114]: "Soviel Geld habt Ihr doch gar nicht!" */
			dlg_message(g_dialog, txt[114]);
		else {
			group_sub_money(g_group, 10);
			/* LOWANGEN.LTX[115]: "Ihr bezahlt Euren Obulus und duerft passieren." */
			dlg_message(g_dialog, txt[115]);
			f = 0;
		}
	}
	if (f) {
		g_group->dir = (g_group->dir + 2) & 3;
		g_cam_angle = (g_group->dir << 8) + 0x100;
		group_set_screen_pos(0x0c, g_low_bridge_y);
		cam_set_from_group();
	}
}
