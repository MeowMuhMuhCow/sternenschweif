/* OVL017 - Overlay O017 (Segment 0x1f5d in SCHWEIF.EXE): Ortsereignisse der Reisekarte (place_event O210 je Ort
 * o->from; Reiseobjekt o; Texte g_tev_loc_txt = POINT2.LTX fuer Ort 7, POINT1.LTX fuer die Orte 17/18, sonst
 * POINT.LTX, g_tev_txt = ALL_SECT.LTX; Bilder per nvfile_load(g_nvf_heads, ..) aus den Dateien 0x196/0x198).
 * place_event_007 (Orte 7/9): orkisches Heerlager am Waldrand (Bild 0x196/2, POINT2[0..15]): umdrehen (Route zurueck,
 *   Tagebuch ALL_SECT[105]); ein NSC (nicht npc_id 5/6) kann noch abraten (txt[87]) - sonst verlaesst er die Gruppe
 *   (g_npc_months = 99); weitergehen -> rennen/umdrehen, die vier Krieger (Bild 3): ergeben -> orc_camp_luggage
 *   (Gefangenschaft, rest = 1, ALL_SECT[106]), kaempfen = Tod der Gruppe (g_travel_state = 1), umdrehen.
 * place_event_017/place_event_018: Rorkvell von den Orks besetzt (Bild 0x198/0x20, POINT1[56]), Tagebuch [107].
 * place_event_039 (Orte 39/40): leer.
 * place_event_041: ohne Salamanderstein (echt/falsch): NSC mit Portraet 0x28 raet zum Weg nach Suedosten (POINT[3]);
 *   sonst bei g_low_tunnel_known, g_low_stone_delivered und noch nicht g_blackmage_done die Schwarzmagier-Szene blackmage_scene (O007): > 0
 *   Kampf
 *   0x29 (gewonnen: Steine weg, g_blackmage_done; Flucht: Route 0x29 -> 0x67), -1 Faelschung erkannt (POINT[60]),
 *   0 die Magier nehmen den Stein mit (POINT[59], g_blackmage_done).
 * place_event_048: nur bei g_on_boat == 0: Bootssteg am Svelltufer (Bild 5, POINT[6]), Route 0x30 -> 0xb1.
 * place_event_065: Bewegung in der Ferne (Bild 7, POINT[8]).
 * place_event_066: Orkheer - keine Chance zu entkommen (Bilder 0x196/8 und 0x198/8, POINT[9..22]): ergeben oder
 *   kaempfen, beides fuehrt in die Gefangenschaft (Augenbinden, Marsch durch die Berge); g_travel_state = -2,
 *   Tagebuch [108].
 * place_event_083: Bogenschuetzen im Blaetterdach (POINT[23..34]) - bis zu 8 Stufen (Pfeile, Schaden, zuletzt Regen
 *   aus Pfeilen und Speeren), je Stufe Warten (g_random_seed2 + 0x50) mit Menue bei Bild-hoch-Taste bzw. rechter
 *   Maustaste: angreifen (ab Stufe 3 Kampf 0x80/0x81 nach AP je Held, g_place083_attacked), zurueckgehen (Route
 *   0x53 -> 0x51), abwarten.
 * place_event_087: Lichtung mit Artherions Huette (POINT[35..56]): Begegnung mit dem Elfen Artherion (Fragen mit
 *   Wiederholung bei -1, Geschichte txt[49..53], g_sites_night_met, Tagebuch [109]/[110]), spaeter leere Huette bzw.
 *   Aschehaufen, nach dem Abendkampf (g_sites_evening == -1) Dank: Gespraech 0x80, Schwert und Bogen des Artherion
 *   (g_artherion_done, Tagebuch [111]).
 * place_event_100: Schwarzmagier-Szene wie place_event_041 (Flucht: Route 0x64 -> 0x67; gewonnen ohne
 *   g_blackmage_done).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL017.CPP   -> 6496/6496 Bytes GLEICH (18.10.2026), cobjfix laeuft.
 *   Ohne -Og 4 Bytes zu kurz.
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Lokale Bmp (Bmp(w, h) / Destruktor (.., 2)) an der Deklarationsstelle nach nvfile_info, zweites Bild im
 *   default-Zweig als eigener Block "default: { .. } break;" (das break erzeugt den jmp short hinter dem Dtor);
 *   new/delete ueber Konstruktor mit this == 0 und Dtor-Flag 3.
 * - switch-Rumpfreihenfolge aus den Vergleichsketten: place_event_007 "case 3" vor "case 2", innen 1, 3, 2;
 *   place_event_083 Menue "case 2" vor "case 1" und "case 1: ..; default: break;" (case 1 ohne break -> der default-
 *   Sprung landet auf dem break-jmp von case 1).
 * - place_event_083: "step = attack = retreated = sum = next = 0" (Stores rechts nach links), register t =
 *   g_random_seed2 in SI, while-Bedingung mit "attack == 0" (cmp byte; mit ! cbw / or ax,ax); "group_sum_ap(g) /
 *   g->nr_heroes" -> cbw / movsx ebx / cdq / idiv ebx.
 * Auffaelliges im Original: place_event_017 und place_event_018 sind identisch, place_event_039 ist leer;
 *   place_event_041 und place_event_100 sind Kopien (nur Fluchtroute und das Setzen von g_blackmage_done nach dem
 *   gewonnenen Kampf unterscheiden sich); place_event_007 prueft dieselbe NSC-Bedingung zweimal; place_event_083
 *   Stufe 3 benutzt hh aus Stufe 2.
 */
#include <stdio.h>

#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:0f2a..0f34, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */

void place_event_007(Travel far *o)
{
	signed char r;
	signed char done = 0;
	signed short w;
	signed short h;
	signed short dummy;
	struct hero far *hh;

	nvfile_info(g_nvf_heads, 0x196, 2, &w, &h);
	Bmp b1(w, h);
	nvfile_load(g_nvf_heads, 0x196, b1.data(), 0, 2, 0, &dummy, &dummy, 0x140);
	g_dlg_box_flag = 0;
	/* POINT2.LTX[0]: "Als Ihr den Wald erreicht, steht Ihr unvermittelt vor einem grossen orkischen Heerlager.
	 * Hunderte der Schwarzpelze lungern herum, spielen mit Wuerfeln oder polieren Ihre Waffen. Das ganze aehnelt so
	 * sehr einem kaiserlichen Garderegiment, dass Ihr zweimal hinsehen muesst, um auch wirklich sicher zu gehen. Von
	 * Euch allerdings, obwohl gelegentlich mal einer heruebersieht, scheinen sie sonst keinerlei Notiz zu nehmen...
	 * Wollt Ihr" */
	/* POINT2.LTX[1]: "Umdrehen oder" */
	/* POINT2.LTX[2]: "weitergehen?" */
	r = dlg_box_bmp(g_dialog, &b1, " ", g_tev_loc_txt[0], 2, g_tev_loc_txt[1], g_tev_loc_txt[2]);
	hh = group_get_npc(g_group);
	if (r != 1 && hh != 0 && check_hero(hh) && hh->npc_id != 6 && hh->npc_id != 5) {
		/* POINT2.LTX[87]: "<Mir wird es hier zu heiss>, meldet sich %s zu Wort, <ich drehe um!>@Wollt Ihr %s Beispiel
		 * folgen und auch umdrehen?" */
		sprintf(g_text_output_buf, g_tev_loc_txt[87], hh->name, hero_name_gen(hh, 1));
		/* ALL_SECT.LTX[1]: "Ja" */
		/* ALL_SECT.LTX[2]: "Nein" */
		r = dlg_box_bmp(g_dialog, &b1, " ", g_text_output_buf, 2, g_tev_txt[1], g_tev_txt[2]);
	}
	if (r == 1) {
		done = 1;
		/* ALL_SECT.LTX[105]: "Wir sind fast in ein orkisches Heerlager gestolpert, konnten aber noch umkehren, bevor wir
		 * bemerkt wurden.@Mann, war das knapp!" */
		diary_add(g_diary, g_tev_txt[105], 0);
	} else {
		if (hh != 0 && check_hero(hh) && hh->npc_id != 6 && hh->npc_id != 5) {
			g_npc_months = 99;
			groups_check_npc();
		}
		g_dlg_box_flag = 0;
		/* POINT2.LTX[3]: "Langsam und betont unauffaellig geht Ihr weiter, aber niemand scheint sich an Eurer
		 * Anwesenheit zu stoeren. Erst als Ihr mitten im Lager steht, naehern sich Euch gemaechlich vier Orks. Wollt
		 * Ihr" */
		/* POINT2.LTX[4]: "dennoch weitergehen?" */
		/* POINT2.LTX[5]: "zu rennen beginnen" */
		/* POINT2.LTX[6]: "oder umdrehen?" */
		r = dlg_box_bmp(g_dialog, &b1, " ", g_tev_loc_txt[3], 3, g_tev_loc_txt[4], g_tev_loc_txt[5], g_tev_loc_txt[6]);
		switch (r) {
		case 3:
			done = 1;
			diary_add(g_diary, g_tev_txt[105], 0);
			break;
		case 2:
			/* POINT2.LTX[7]: "Bereits nach wenigen Schritten bleibt Ihr wieder stehen, denn mit Eurem  Gerenne habt Ihr
			 * unvermittelt die Aufmerksamkeit weniger Dutzend Orks in  Eurer unmittelbaren Naehe auf Euch gezogen. Und vor
			 * einigen Hundert kann man nun mal nicht weglaufen, wenn man mitten unter Ihnen steht..." */
			dlg_message(g_dialog, g_tev_loc_txt[7]);
		default:
			{
				nvfile_info(g_nvf_heads, 0x196, 3, &w, &h);
				Bmp b2(w, h);
				nvfile_load(g_nvf_heads, 0x196, b2.data(), 0, 3, 0, &dummy, &dummy, 0x140);
				g_dlg_box_flag = 0;
				/* POINT2.LTX[8]: "Ohne groessere Feindseligkeit bauen sich die vier Krieger vor Euch auf und mustern Euch. In
				 * sehr hartem, aber gut verstaendlichem Garethi spricht Euch einer an. <Ihr wollt nach Lowangen, richtig? Kein
				 * Problem, herein lassen wir alle. Ist gut fuer uns. Gepaeck und Waffen her, Ruestungen natuerlich auch, und
				 * keiner hat Schmerzen.>" */
				/* POINT2.LTX[9]: "<Wir haben wohl keine andere Wahl...>" */
				/* POINT2.LTX[10]: "<Und wenn wir wieder umdrehen?>" */
				r = dlg_box_bmp(g_dialog, &b2, " ", g_tev_loc_txt[8], 2, g_tev_loc_txt[9], g_tev_loc_txt[10]);
				switch (r) {
				case 2:
					g_dlg_box_flag = 0;
					/* POINT2.LTX[11]: "<Natuerlich. Gepaeck her, Waffen und Ruestungen.>" */
					/* POINT2.LTX[12]: "kaempfen" */
					/* POINT2.LTX[13]: "Umdrehen" */
					/* POINT2.LTX[14]: "oder weiter nach Lowangen?" */
					r = dlg_box_bmp(g_dialog, &b2, " ", g_tev_loc_txt[11], 3, g_tev_loc_txt[12], g_tev_loc_txt[13], g_tev_loc_txt[14]);
					switch (r) {
					case 1:
						/* POINT2.LTX[15]: "Eine wirklich gute Idee, einen Kampf eins zu hundert zu beginnen..." */
						dlg_message(g_dialog, g_tev_loc_txt[15]);
						while (group_count_alive(g_group))
							group_sub_le(g_group, 0x32);
						g_travel_state = 1;
						group_set_on_route(g_group, 0);
						break;
					case 3:
						orc_camp_luggage(o);
						o->rest = 1;
						/* ALL_SECT.LTX[106]: "Wir sind vor Lowangen in ein orkisches Heerlager geraten, mussten saemtliche Waffen,
						 * Ruestungen und unser gesamtes Gepaeck abgeben. Danach liessen sie uns laufen." */
						diary_add(g_diary, g_tev_txt[106], 0);
						break;
					case 2:
						done = 1;
						orc_camp_luggage(o);
						diary_add(g_diary, g_tev_txt[105], 0);
						break;
					}
					break;
				default:
					orc_camp_luggage(o);
					o->rest = 1;
					diary_add(g_diary, g_tev_txt[106], 0);
					break;
				}
			}
			break;
		}
	}
	if (done == 1) {
		o->route[0] = o->from;
		o->route[1] = o->prev_place;
		o->route[2] = 0xff;
		o->action = 4;
		o->f109 = 1;
	}
}

void place_event_017(Travel far *o)
{
	signed short dummy;
	Bmp b(64, 64);

	nvfile_load(g_nvf_heads, 0x198, b.data(), 0, 0x20, 0, &dummy, &dummy, 0x140);
	/* POINT1.LTX[56]: "Als Ihr das Zwergenstaedtchen Rorkvell erreicht, muesst Ihr zu Eurem Entsetzen feststellen,
	 * dass hier die Orks bereits das Kommando uebernommen haben. Euch bleibt nur der Weg zurueck." */
	dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[56], 0);
	/* ALL_SECT.LTX[107]: "Wir haben Rorkvell erreicht; die Stadt ist jedoch von den Orks besetzt." */
	diary_add(g_diary, g_tev_txt[107], 0);
}

void place_event_018(Travel far *o)
{
	signed short dummy;
	Bmp b(64, 64);

	nvfile_load(g_nvf_heads, 0x198, b.data(), 0, 0x20, 0, &dummy, &dummy, 0x140);
	/* POINT1.LTX[56]: "Als Ihr das Zwergenstaedtchen Rorkvell erreicht, muesst Ihr zu Eurem Entsetzen feststellen,
	 * dass hier die Orks bereits das Kommando uebernommen haben. Euch bleibt nur der Weg zurueck." */
	dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[56], 0);
	/* ALL_SECT.LTX[107]: "Wir haben Rorkvell erreicht; die Stadt ist jedoch von den Orks besetzt." */
	diary_add(g_diary, g_tev_txt[107], 0);
}

void place_event_039(Travel far *o)
{
}

void place_event_041(Travel far *o)
{
	signed short dummy;
	signed char r;
	struct hero far *hh;
	register signed short n;

	hh = group_get_npc(g_group);
	if (group_find_item(g_group, ITEM_SALAMANDER_STONE) == -1 && group_find_item(g_group, ITEM_SALAMANDER_STONE_FAKE) == -1) {
		if (hh != 0 && hh->portrait == 0x28) {
			Bmp b(64, 64);

			nvfile_load(g_nvf_heads, 0x198, b.data(), 0, 2, 0, &dummy, &dummy, 0x140);
			/* POINT.LTX[3]: "<Wir sollten den Weg nach Suedosten nehmen. Das ist zwar ein Umweg, aber am Svelltufer laufen
			 * wir dem Orkenheer geradewegs in die Arme,> meldet sich Antharon zu Wort, als Ihr Euch nach dem richtigen Weg
			 * umschaut." */
			dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[3], 0);
		}
	} else if (g_low_tunnel_known != 0 && !g_blackmage_done && g_low_stone_delivered != 0 &&
		(group_find_item(g_group, ITEM_SALAMANDER_STONE) >= 0 || group_find_item(g_group, ITEM_SALAMANDER_STONE_FAKE) >= 0)) {
		travel_leave(o);
		n = blackmage_scene();
		travel_enter(o);
		mem_free(g_tev_loc_buf);
		mem_free(g_tev_loc_txt);
		txt_load_list(g_font, 0xac, 0x41, &g_tev_loc_buf, &g_tev_loc_txt);
		if (n > 0) {
			travel_leave(o);
			g_fight_flag = 1;
			r = start_fight(0x29, 1);
			r = fight_result(o, r);
			travel_enter(o);
			if (r == 2) {
				group_remove_item(g_group, ITEM_SALAMANDER_STONE);
				group_remove_item(g_group, ITEM_SALAMANDER_STONE_FAKE);
				g_blackmage_done = 1;
			}
			if (r == 1) {
				route_clear(o);
				o->route[0] = 0x29;
				o->route[1] = 0x67;
				o->route[2] = 0xff;
				o->action = 4;
			}
		} else if (n == -1) {
			group_remove_item(g_group, ITEM_SALAMANDER_STONE_FAKE);
			/* POINT.LTX[60]: "<Das ist eine Faelschung!>@knurrt der Magier nach einer kurzen Untersuchung. Er starrt Euch
			 * durchdringend an, dann dreht er sich zu seinen Gefaehrten um. @<Der echte Stein muss noch in Lowangen
			 * sein.>@Ohne Euch weiter zu beachten, verschwinden die Magier in Richtung Westen." */
			dlg_message(g_dialog, g_tev_loc_txt[60]);
		} else {
			group_remove_item(g_group, ITEM_SALAMANDER_STONE);
			group_remove_item(g_group, ITEM_SALAMANDER_STONE_FAKE);
			g_blackmage_done = 1;
			/* POINT.LTX[59]: "Die Magier verschwinden mit dem Salamanderstein in Richtung Westen." */
			dlg_message(g_dialog, g_tev_loc_txt[59]);
		}
	}
}

void place_event_048(Travel far *o)
{
	signed short dummy;

	if (g_on_boat == 0) {
		Bmp b(64, 64);

		nvfile_load(g_nvf_heads, 0x198, b.data(), 0, 5, 0, &dummy, &dummy, 0x140);
		/* POINT.LTX[6]: "Ihr erreicht das Svelltufer, wo ein morscher Bootssteg, in den Fluten steht. Richtung Norden
		 * verliert sich der Weg im Ufermorast, und Ihr koennt dort unmoeglich weitergehen." */
		dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[6], 0);
		route_clear(o);
		o->route[0] = 0x30;
		o->route[1] = 0xb1;
		o->route[2] = 0xff;
		o->action = 4;
	}
}

void place_event_065(Travel far *o)
{
	signed short dummy;
	Bmp b(64, 64);

	nvfile_load(g_nvf_heads, 0x198, b.data(), 0, 7, 0, &dummy, &dummy, 0x140);
	/* POINT.LTX[8]: "Irgendetwas bewegt sich in der Ferne auf dem Weg. Vielleicht ist es eine Rinderherde, vielleicht
	 * auch eine grosse Gruppe Reisender." */
	dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[8], 0);
}

void place_event_066(Travel far *o)
{
	signed char r;
	signed short dummy;
	signed short w;
	signed short h;

	nvfile_info(g_nvf_heads, 0x196, 8, &w, &h);
	Bmp b1(w, h);
	nvfile_load(g_nvf_heads, 0x196, b1.data(), 0, 8, 0, &dummy, &dummy, 0x140);
	/* POINT.LTX[9]: "Ihr habt keine Chance zu entkommen. Von vorn kommt Euch ein kleines Heer Orks entgegen, hinter
	 * Euch stuermen ploetzlich zwanzig weitere Schwarzpelze aus den Bergen. Ihr seid umzingelt." */
	dlg_box_bmp(g_dialog, &b1, (char far *)0, g_tev_loc_txt[9], 0);
	Bmp b2(64, 64);
	nvfile_load(g_nvf_heads, 0x198, b2.data(), 0, 8, 0, &dummy, &dummy, 0x140);
	/* POINT.LTX[10]: "Einer der Orks, offensichtlich ihr Hauptmann, spricht Euch an:@<Du Gefangene von %s, Du ergeben
	 * oder erledigt!>" */
	/* POINT.LTX[57]: "Orzatai" */
	sprintf(g_text_output_buf, g_tev_loc_txt[10], g_tev_loc_txt[57]);
	dlg_box_bmp(g_dialog, &b2, (char far *)0, g_text_output_buf, 0);
	/* POINT.LTX[11]: "Wollt Ihr" */
	/* POINT.LTX[12]: "Euch ergeben?" */
	/* POINT.LTX[13]: "oder kaempfen?" */
	r = dlg_box_bmp(g_dialog, &b2, (char far *)0, g_tev_loc_txt[11], 2, g_tev_loc_txt[12], g_tev_loc_txt[13]);
	if (r == -1)
		r = 1;
	if (r == 2)
		/* POINT.LTX[14]: "Als Ihr aufwacht, stellt Ihr fest, dass man Euch die Augen verbunden hat. Ausser einigen
		 * Kleidungsfetzen und Gegenstaenden, von denen die Orks vermutlich glaubten, dass Ihr sie fuers marschieren
		 * benoetigt, ist Euer gesamtes Habe gestohlen. - Ihr koennt Euch gar nicht richtig an den Kampf erinnern.
		 * Irgendwann hat Euch wohl einer dieser Lumpen am Kopf getroffen. - Ihr werdet hart in die Seite geknufft. Die
		 * Orks setzen sich in Bewegung, und Ihr werdet von ihnen mit Tritten und Griffen auf den richtigen Weg
		 * dirigiert." */
		dlg_message(g_dialog, g_tev_loc_txt[14]);
	switch (r) {
	case 1:
		/* POINT.LTX[15]: "Der Hauptmann befiehlt ein paar anderen Orks, Euch Fesseln anzulegen. Danach  stuerzen sich
		 * die Orks auf Euer Gepaeck, und machen nicht einmal vor den  Kleidern am Leib halt. Erst als Ihr den Hauptmann
		 * darauf aufmerksam macht, dass  Ihr Euch durch Eure Nacktheit gedemuetigt fuehlt, laesst er Euch wieder ein
		 * paar  alte Fetzen ueberstreifen, und Gegenstaende zurueckgeben, von denen er glaubt,  dass Ihr damit besser
		 * marschieren koennt. Anschliessend laesst er Euch die Augen  verbinden." */
		dlg_box_bmp(g_dialog, &b2, (char far *)0, g_tev_loc_txt[15], 0);
		/* POINT.LTX[16]: "Durch die Augenbinden koennt Ihr nichts sehen. Die Orks reden wirr durcheinander und scheinen
		 * sich zu beraten. Ploetzlich werdet Ihr in die Seite geknufft:@<Mitkommen da!>" */
		dlg_message(g_dialog, g_tev_loc_txt[16]);
		/* POINT.LTX[17]: "Stundenlang marschieren die Orks mit Euch stumm durch die Berge. Abends legen die Schwarzpelze
		 * eine Rast ein - Ihr werdet mit Wasser und Brot gefuettert.@Ihr seid sehr muede und schlaft ein." */
		dlg_message(g_dialog, g_tev_loc_txt[17]);
		/* POINT.LTX[18]: "Ihr werdet ploetzlich wach, weil Euch jemand hart in die Seite knufft. Offensichtlich setzen
		 * sich die Orks wieder in Bewegung, und Ihr werdet von ihnen mit Tritten und Griffen auf den richtigen Weg
		 * dirigiert." */
		dlg_message(g_dialog, g_tev_loc_txt[18]);
	case 2:
		/* POINT.LTX[19]: "Mittags vernehmt Ihr das Rauschen von Wasser. Einige Stunden lang verlaeuft die unfreiwillige
		 * Reise an einem Bach entlang, dann geht es durch das wadentiefe Wasser und Ihr entfernt Euch vom Bach." */
		dlg_message(g_dialog, g_tev_loc_txt[19]);
		/* POINT.LTX[20]: "Am Abend legen die Orks eine Rast ein. Die letzten Meilen waren sehr anstrengend, denn Ihr
		 * seid wieder in den Bergen. Nachdem Euch etwas Brot und Wasser gegeben wurde, schlaft Ihr ein." */
		dlg_message(g_dialog, g_tev_loc_txt[20]);
		/* POINT.LTX[21]: "Am naechsten Tag geht es weiter durch die Berge. Schliesslich erreicht Ihr eine Hoehle.
		 * Nachdem Ihr Euch in den verwinkelten Gaengen mehr als einmal gestossen habt, werden Euch die Augenbinden
		 * abgenommen. Ihr hoert, wie eine Tuer zugeworfen und das Schloss verriegelt wird." */
		dlg_message(g_dialog, g_tev_loc_txt[21]);
		/* POINT.LTX[22]: "Langsam gewoehnen sich Eure Augen an den Fackelschein, der aus einem Gang in Euer Gefaengnis
		 * faellt..." */
		dlg_message(g_dialog, g_tev_loc_txt[22]);
		break;
	}
	g_travel_state = -2;
	/* ALL_SECT.LTX[108]: "Wir wurden von einem Trupp Orks ueberrascht, die uns gefangen nahmen und verschleppten.
	 * Nach zwei Tagen Wanderung, waehrend der uns die Augen verbunden waren, haben wir eine Hoehle - offensichtlich
	 * das Ziel der Reise - erreicht." */
	diary_add(g_diary, g_tev_txt[108], 0);
}

void place_event_083(Travel far *o)
{
	signed char r;
	signed char k;
	signed char sum;
	signed char x;
	signed char step;
	signed char attack;
	signed char retreated;
	signed char next;
	struct hero far *hh;
	register signed short t = g_random_seed2;

	step = attack = retreated = sum = next = 0;
	g_mouse_rclick = g_key = 0;
	if (g_place083_attacked != 0) {
		next = 99;
		attack = 1;
	} else
		while (attack == 0 && retreated == 0 && step <= 7 && group_count_check1(g_group)) {
			next = 0;
			t += 0x50;
			if (t < 0)
				t = 0;
			switch (step) {
			case 0:
				/* POINT.LTX[23]: "Ein Pfeil faehrt etwa zehn Schritt vor Euch in den Boden!" */
				dlg_message(g_dialog, g_tev_loc_txt[23]);
				break;
			case 1:
				/* POINT.LTX[24]: "Erneut ein Pfeil, diesmal etwa fuenf Schritt von Euch entfernt!" */
				dlg_message(g_dialog, g_tev_loc_txt[24]);
				break;
			case 2:
				hh = get_hero_attrib_min(g_group, ATTRIB_CH);
				/* POINT.LTX[25]: "Der naechste Pfeil faehrt unmittelbar vor %s in den Boden; diesmal kann man erkennen, das er
				 * von unmittelbar ueber Euch gekommen ist." */
				sprintf(g_text_output_buf, g_tev_loc_txt[25], hh->name);
				dlg_message(g_dialog, g_text_output_buf);
				break;
			case 3:
				/* POINT.LTX[26]: "Im Blaetterdach sieht man einen Elfen mit gespanntem Bogen sitzen, der Euch regungslos
				 * ansieht. So ist es denn auch nicht sein Pfeil, der %s trifft, sondern wohl der eines anderen, die erst jetzt
				 * in zahlreichen Baeumen vor Euch zu sehen sind. Einer von ihnen deutet mit seiner Waffe in die Richtung, aus
				 * der Ihr gekommen seid." */
				sprintf(g_text_output_buf, g_tev_loc_txt[26], hh->name);
				dlg_message(g_dialog, g_text_output_buf);
				sub_hero_le(hh, random_schweif(6) + 5 - hh->rs_bonus);
				break;
			case 4:
				for (k = 0; k < 6; k++)
					sum += random_schweif(6);
				if (!check_hero(hh))
					hh = get_random_hero(g_group);
				/* POINT.LTX[27]: "Der naechste Pfeil trifft %s in den Hals!" */
				sprintf(g_text_output_buf, g_tev_loc_txt[27], hh->name);
				dlg_message(g_dialog, g_text_output_buf);
				sub_hero_le(hh, sum);
				break;
			case 5:
				hh = get_worst_hero_skill(g_group, TA_VERSTECKEN);
				if (!check_hero(hh))
					hh = get_random_hero(g_group);
				/* POINT.LTX[28]: "Diesmal ist %s das Ziel, und dieser Pfeil verfehlt nur um Haaresbreite die Kehle." */
				sprintf(g_text_output_buf, g_tev_loc_txt[28], hh->name);
				dlg_message(g_dialog, g_text_output_buf);
				sub_hero_le(hh, dice_roll(2, 6, 0));
				break;
			case 6:
				/* POINT.LTX[29]: "Und nun geht ein wahrer Regen von Pfeilen und Speeren auf Euch nieder!" */
				dlg_message(g_dialog, g_tev_loc_txt[29]);
				group_sub_le_range(g_group, 5, 0x0f);
				break;
			case 7:
				/* POINT.LTX[30]: "Wie lange habt Ihr eigentlich geglaubt, das aushalten zu koennen?" */
				dlg_message(g_dialog, g_tev_loc_txt[30]);
				while (group_count_alive(g_group))
					group_sub_le(g_group, 0x1e);
				g_travel_state = 1;
				break;
			}
			while (next == 0) {
				travel_redraw(o);
				if (g_random_seed2 > t) {
					step++;
					next = 1;
				}
				if (g_key == 0x4900 || g_mouse_rclick != 0) {
					/* POINT.LTX[11]: "Wollt Ihr" */
					/* POINT.LTX[31]: "Die Bogenschuetzen angreifen?" */
					/* POINT.LTX[32]: "Aus der Reichweite der Bogen fluechten, und zurueckgehen?" */
					/* POINT.LTX[33]: "Oder abwarten was passiert?" */
					r = dlg_choose(g_dialog, g_tev_loc_txt[11], 3, g_tev_loc_txt[31], g_tev_loc_txt[32], g_tev_loc_txt[33]);
					switch (r) {
					case 2:
						route_clear(o);
						o->route[0] = 0x53;
						o->route[1] = 0x51;
						o->route[2] = 0xff;
						o->action = 4;
						retreated = 1;
						break;
					case 1:
						if (step >= 3) {
							attack = 1;
							next = 1;
							g_place083_attacked = 1;
						} else
							/* POINT.LTX[34]: "Mit wem wollt Ihr den kaempfen?" */
							dlg_message(g_dialog, g_tev_loc_txt[34]);
					default:
						break;
					}
					next = 1;
					step += 1;
				}
			}
		}
	if (attack == 1) {
		travel_leave(o);
		g_fight_flag = 1;
		if (group_sum_ap(g_group) / g_group->nr_heroes <= 600)
			x = start_fight(0x80, 1);
		else
			x = start_fight(0x81, 1);
		fight_result(o, x);
		travel_enter(o);
	}
	if (group_count_check1(g_group) == 0)
		g_travel_state = 1;
	if (group_count_check1(g_group) == 1 && group_get_npc(g_group) && check_hero(group_get_npc(g_group)))
		g_travel_state = 1;
}

void place_event_087(Travel far *o)
{
	signed char r;
	signed short dummy;
	Bmp far *b;
	signed char x = 0;

	if ((g_artherion_hut_visited == 1 && g_artherion_done != 1 && g_sites_evening == 0) || (g_artherion_hut_visited == 1 && g_artherion_done == 1 && g_sites_evening == -1)) {
		b = new Bmp(64, 64);
		nvfile_load(g_nvf_heads, 0x198, b->data(), 0, 10, 0, &dummy, &dummy, 0x140);
		/* POINT.LTX[54]: "Ihr erreicht wieder die Lichtung mit Artherions Huette, aber er ist nirgends zu finden." */
		dlg_box_bmp(g_dialog, b, (char far *)0, g_tev_loc_txt[54], 0);
		delete b;
	}
	if (g_artherion_hut_visited == 1 && g_artherion_done == 1 && g_sites_evening == 0)
		/* POINT.LTX[56]: "Die Huette ist verschwunden, nur ein grosser Aschehaufen kuendet noch von ihrer Existenz. Weit
		 * und breit ist niemand zu sehen oder zu hoeren." */
		dlg_message(g_dialog, g_tev_loc_txt[56]);
	if (g_artherion_hut_visited == 1 && g_sites_evening == -1 && g_artherion_done == 0) {
		b = new Bmp(64, 64);
		nvfile_load(g_nvf_heads, 0x198, b->data(), 0, 9, 0, &dummy, &dummy, 0x140);
		/* POINT.LTX[55]: "Als Ihr die Lichtung erreicht, kommt Euch Artherion entgegen. <Ich habe es schon gehoert. Ich
		 * moechte Euch vielmals danken, bitte kommt doch herein.>" */
		dlg_box_bmp(g_dialog, b, (char far *)0, g_tev_loc_txt[55], 0);
		delete b;
		travel_leave(o);
		if (conv_run(g_dialog2, 0x80))
			timewarp_minutes(g_date, 30);
		mouse_hide(0);
		travel_enter(o);
		mouse_show(0);
		travelmap_draw(o);
		travel_redraw(o);
		group_give_item(g_group, ITEM_ARTHERION_SWORD, 1);
		group_give_item(g_group, ITEM_ARTHERION_BOW, 1);
		g_artherion_done = 1;
		/* ALL_SECT.LTX[111]: "Heute waren wir wieder bei Artherion. Er hat schon von den Ereignissen in Gashok gehoert
		 * und sich sehr gefreut. Ausserdem hat er uns zwei Waffen, ein Schwert und einen Bogen geschenkt." */
		diary_add(g_diary, g_tev_txt[111], 0);
	}
	if (g_artherion_hut_known == 1 && g_artherion_hut_visited == 0) {
		b = new Bmp(64, 64);
		nvfile_load(g_nvf_heads, 0x198, b->data(), 0, 9, 0, &dummy, &dummy, 0x140);
		do {
			/* POINT.LTX[35]: "Ihr erreicht eine Lichtung, auf der eine kleine Huette steht, kaum zu sehen vor dem
			 * Hintergrund des dichten Waldes. Weiter kommt Ihr jedoch nicht, denn auf der Lichtumg steht ein Elf, der den
			 * in dieser Gegend so beliebten Langbogen gespannt und auf Euch gerichtet haelt.@<Keinen Schritt weiter, oder
			 * ich schiesse!>" */
			/* POINT.LTX[36]: "<Wir kommen aus Gashok!>" */
			/* POINT.LTX[37]: "<Gut Freund!>" */
			/* POINT.LTX[38]: "<Wir sind hier, um Euch zu raechen, Freund Artherion!>" */
			r = dlg_box_bmp(g_dialog, b, (char far *)0, g_tev_loc_txt[35], 3, g_tev_loc_txt[36], g_tev_loc_txt[37], g_tev_loc_txt[38]);
		} while (r == -1);
		switch (r) {
		case 1:
			/* POINT.LTX[39]: "Mit einem animalischen Schrei schiesst der Elf den Pfeil auf Euch ab; jedoch verfehlt er sein
			 * Ziel. Dann stuerzt er in seine Huette. Noch bevor Ihr die Lichtung ganz ueberquert habt, seht Ihr Flammen aus
			 * dem Dach schlagen; binnen kuerzester Zeit brennt sie lichterloh. Aus diesem Inferno ist nichts und niemand
			 * mehr zu retten." */
			dlg_message(g_dialog, g_tev_loc_txt[39]);
			g_artherion_hut_visited = g_artherion_done = 1;
			g_sites_evening = -1;
			break;
		case 2:
			r = -1;
			while (r == -1)
				/* POINT.LTX[40]: "<Ich habe keine Freunde, nur Feinde!>" */
				/* POINT.LTX[42]: "<Ihr irrt Euch!>" */
				/* POINT.LTX[43]: "<Nicht alle sind so wie die Leute in Gashok!>" */
				/* POINT.LTX[44]: "<Wenn das so ist...>" */
				r = dlg_box_bmp(g_dialog, b, (char far *)0, g_tev_loc_txt[40], 3, g_tev_loc_txt[42], g_tev_loc_txt[43], g_tev_loc_txt[44]);
			switch (r) {
			case 1:
				x = 1;
				break;
			case 2:
				x = 1;
				break;
			case 3:
				/* POINT.LTX[48]: "<Verschwindet, ich will niemanden sehen.> Mit diesen Worten dreht er sich um und laesst sich
				 * vom Wald verschlucken." */
				dlg_message(g_dialog, g_tev_loc_txt[48]);
				break;
			}
			break;
		case 3:
			r = -1;
			while (r == -1)
				/* POINT.LTX[41]: "<Was soll das heissen?>" */
				/* POINT.LTX[45]: "<Wir sind selber fast umgekommen in dieser Stadt.>" */
				/* POINT.LTX[46]: "<Wir wissen, was Ihr durchgemacht habt.>" */
				/* POINT.LTX[47]: "<Ratet mal.>" */
				r = dlg_box_bmp(g_dialog, b, (char far *)0, g_tev_loc_txt[41], 3, g_tev_loc_txt[45], g_tev_loc_txt[46], g_tev_loc_txt[47]);
			switch (r) {
			case 1:
				x = 1;
				break;
			case 2:
				x = 1;
				break;
			case 3:
				dlg_message(g_dialog, g_tev_loc_txt[48]);
				break;
			}
			break;
		}
		if (x == 1) {
			/* POINT.LTX[49]: "<Niemand kann ermessen, was es heisst, wenn mitten in der Nacht das eigene Haus brennt, und
			 * vor der Tuer stehen ein Dutzend braunvermummter Gestalten, und wenn man versucht, hinauszugehen, treiben sie
			 * einen wieder hinein, mitten in die Flammen. Ueberall haben welche gestanden, vorne und hinten. Und als ich
			 * versucht habe, trotz der Stoecke und Steine, die sie geworfen haben, an ihnen vorbeizukommen, haben sie
			 * Schwerter gezogen und sind auf mich losgegangen. Die wollten mich umbringen! Nicht einfach vertreiben oder so
			 * etwas, umbringen wollten sie mich. In den reinigenden Flammen wollten sie meine Seele laeutern, damit
			 * wenigstens einmal ein Elf in Borons Hallen kaeme, statt in den Limbus." */
			dlg_box_bmp(g_dialog, b, (char far *)0, g_tev_loc_txt[49], 0);
			/* POINT.LTX[50]: "Und eine Brut des Namenlosen seien wir. Jahrelang habe ich geschuftet wie ein Tier, sogar das
			 * Zaubern habe ich mir abgewoehnt, um sie nicht zu erschrecken. Aber da, da habe ich. So sehr, dass es mir
			 * selber wehgetan hat, aber nicht so sehr wie Ihrem Anfuehrer. Dann gelang es mir, mich durch einen Zauber in
			 * einen Adler zu verwandeln und wegzufliegen. Und wenn ich einen von denen hier sehen sollte, dann bringe ich
			 * ihn um, und wenn es das letzte ist, was ich tue." */
			dlg_box_bmp(g_dialog, b, (char far *)0, g_tev_loc_txt[50], 0);
			/* POINT.LTX[51]: "Die einzige aus der Stadt, die nicht so war, war die Kraeuterfrau auf dem Markt;
			 * wahrscheinlich, weil sie selber Angst vor denen hatte. Eine Hexe sei sie; das koennte sogar stimmen.
			 * Jedenfalls kennt sie fast alle Bewohner der Stadt." */
			dlg_box_bmp(g_dialog, b, (char far *)0, g_tev_loc_txt[51], 0);
			/* POINT.LTX[52]: "Das ganze Gehetze hat angefangen als zwei Leute nach Gashok gezogen sind. Einer von denen
			 * muss ihr Anfuehrer sein. Und wenn Ihr mir helfen wollt, dann verschwindet von hier und toetet diesen Kerl,
			 * damit alle Voelker wieder friedlich in Gashok leben koennen.>" */
			dlg_box_bmp(g_dialog, b, (char far *)0, g_tev_loc_txt[52], 0);
			/* POINT.LTX[53]: "Erschoepft haelt er inne. Dann blickt er Euch an, als kaeme er von einem weit entfernten Ort
			 * zurueck, und das Grauen in seinem Gesicht laesst Euch den Atem anhalten.@Ploetzlich dreht er sich um und
			 * verschwindet im Wald. Nach kurzer Zeit ist nichts weiter zu hoeren als das Singen der Voegel." */
			dlg_box_bmp(g_dialog, b, (char far *)0, g_tev_loc_txt[53], 0);
			g_sites_night_met = 1;
			/* ALL_SECT.LTX[109]: "Wir haben Artherion, den Elfen gefunden. Er hat uns von seinen Erlebnissen in Gashok
			 * erzaehlt." */
			diary_add(g_diary, g_tev_txt[109], 0);
		}
		if (g_artherion_done == 1)
			/* ALL_SECT.LTX[110]: "Wir haben Artherion, den Elfen gefunden. Irgendwas muessen wir falsch gemacht haben, denn
			 * er hat sich, bevor wir etwas unternehmen konnten, mitsamt seiner Huette verbrannt. Komischer Kauz!" */
			diary_add(g_diary, g_tev_txt[110], 0);
		g_artherion_hut_visited = 1;
		delete b;
	}
}

void place_event_100(Travel far *o)
{
	signed char r;
	register signed short n;

	if (g_low_tunnel_known != 0 && !g_blackmage_done && g_low_stone_delivered != 0 &&
		(group_find_item(g_group, ITEM_SALAMANDER_STONE) >= 0 || group_find_item(g_group, ITEM_SALAMANDER_STONE_FAKE) >= 0)) {
		travel_leave(o);
		n = blackmage_scene();
		travel_enter(o);
		mem_free(g_tev_loc_buf);
		mem_free(g_tev_loc_txt);
		txt_load_list(g_font, 0xac, 0x41, &g_tev_loc_buf, &g_tev_loc_txt);
		if (n > 0) {
			travel_leave(o);
			g_fight_flag = 1;
			r = start_fight(0x29, 1);
			r = fight_result(o, r);
			travel_enter(o);
			if (r == 2) {
				group_remove_item(g_group, ITEM_SALAMANDER_STONE);
#if FIX_0180
				/* Fix #180: Sieg merken wie in place_event_041 */
				group_remove_item(g_group, ITEM_SALAMANDER_STONE_FAKE);
				g_blackmage_done = 1;
			}
			if (r == 1) {
#else
				group_remove_item(g_group, ITEM_SALAMANDER_STONE_FAKE);
			}
			if (r == 1) {
#endif
				route_clear(o);
				o->route[0] = 0x64;
				o->route[1] = 0x67;
				o->route[2] = 0xff;
				o->action = 4;
			}
		} else if (n == -1) {
			group_remove_item(g_group, ITEM_SALAMANDER_STONE_FAKE);
			/* POINT.LTX[60]: "<Das ist eine Faelschung!>@knurrt der Magier nach einer kurzen Untersuchung. Er starrt Euch
			 * durchdringend an, dann dreht er sich zu seinen Gefaehrten um. @<Der echte Stein muss noch in Lowangen
			 * sein.>@Ohne Euch weiter zu beachten, verschwinden die Magier in Richtung Westen." */
			dlg_message(g_dialog, g_tev_loc_txt[60]);
		} else {
			group_remove_item(g_group, ITEM_SALAMANDER_STONE);
			group_remove_item(g_group, ITEM_SALAMANDER_STONE_FAKE);
			g_blackmage_done = 1;
			/* POINT.LTX[59]: "Die Magier verschwinden mit dem Salamanderstein in Richtung Westen." */
			dlg_message(g_dialog, g_tev_loc_txt[59]);
		}
	}
}
