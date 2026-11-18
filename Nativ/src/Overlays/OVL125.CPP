/* OVL125 - Overlay O125 (Segment 0x21e1 in SCHWEIF.EXE): Ortsereignisse der Orte 103..198 (place_event O210 ruft je
 * Ort mit dem Reiseobjekt p; Texte g_tev_loc_txt = POINT1.LTX, fuer Ort 187 POINT.LTX, g_tev_txt = ALL_SECT.LTX;
 *   Bilder
 * aus Datei 0x198/0x197/0x196 in ein Bmp 64x64; neue Route p->route[0..2] mit p->action = 4; Ergebnis
 *   g_travel_state).
 * place_event_103: Burgruine auf der Waldlichtung (Bild 0x0b) - bei g_low_tunnel_known Frage, Geheimgang mit dem Lowangen-
 *   Schluessel -> p->rest = 1, g_travel_state = -5, sonst txt[59]; einmaliger Tagebucheintrag ALL_SECT[113]
 *   (g_tev_lowangen_key_noted). place_event_106: Hoehle - nach dem Betreten (g_finsterkopp_entered) eingestuerzt:
 *   Bild 0x0d, route_clear und Route 0x6a/0xc1; sonst Bild 0x0c und Frage -> g_finsterkopp_entered = 1,
 *   g_travel_state = -6. place_event_114/131/142: Bild + Text (0x0e; 0x11 Marschen -> Route 0x83/0x35; 0x1c
 *   Felsmassiv
 *   -> Route 0x8e/0x87). place_event_170/171: Hoehlen (Bild 0x13/0x14) und Frage (g_travel_state = -4 bzw. Route
 *   0xaa/0xa4 + p->f109; -3 mit p->rest). place_event_183: Waldsee mit Huette (Bild 0x15), Gespraech 0x81.
 * place_event_187: Orkheer (Bild 0x196/8, Groesse nvfile_info, und Bild 0x17) - Hauptmann Ugorzzih, ergeben oder
 *   kaempfen (case 1 faellt in case 2 durch), Verschleppung, g_travel_state = -2, Tagebuch ALL_SECT[108].
 * place_event_189: verdorrte Einoede (Bild 0x18): LE-Verlust und Krankheit (group_sub_le_range/group_get_ill), der
 *   schlechteste Zecher nimmt 2 LE Schaden; weitergehen -> Basilisk (Fenster mit Bildfolge 0x1d5, Klang 0xae), alle
 *   sterben (group_sub_le bis keiner lebt), g_travel_state = 1; sonst Umkehr (Route 0xbd/0xbc), Tagebuch
 *   ALL_SECT[114].
 * place_event_196/197 (nur wenn !g_place197_done): Schiff auf dem Svellt - Reparaturhalt (Bild 0x19, 2 Stunden) bzw.
 *   Raeuberbande (Kampf 0x5e), der Schiffer flieht, Gruppe an Ort 0x30 (group.set_route_from/set_route_to, inline,
 *   place_reveal_dirs). place_event_104/117/128/185/198 leer.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL125.CPP   -> 3846/3846 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Aufbau wie OVL124/OVL126 (wovs); -Og noetig (Epilogkopie am Ende des then-Zweigs mit lokalem Bmp in
 *   place_event_106).
 * - Lokale Bmp-Objekte: auf Funktionsebene (Konstruktor am Anfang, Destruktor (&b, 2) am Ende), in if-/else-
 *   Bloecken (place_event_106 beide auf [bp-0eh], place_event_189 b2 im else-Block nach h = get_worst_hero_skill),
 *   nach Anweisungen deklariert
 *   (place_event_187 b1 nach nvfile_info, b2 mitten in der Funktion); new/delete in place_event_183/197.
 * - "!g_place197_done"/"!g_tev_lowangen_key_noted" -> mov al/cbw/or; fight_result liefert hier int (or ax,ax);
 *   "group_find_key(..) != -1" -> 3D FF FF.
 * - place_event_197: g_group->set_route_from(p->from)/set_route_to(p->from) -> Kopien [bp-9]/[bp-0ah].
 * Auffaellig: place_event_187 zeigt in case 2 als ersten Text g_tev_loc_txt[1] statt der fortlaufenden Nummer 19
 *   (danach 20..22);
 *   place_event_189 laesst g_tev_loc_txt[17] in der ersten Auswahl aus und zeigt ihn spaeter im Fenster.
 */
#include <stdio.h>

#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:6318..631a, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */

void place_event_103(Travel far *p)
{
	signed short dummy;
	signed char sel;
	Bmp b(0x40, 0x40);

	nvfile_load(g_nvf_heads, 0x198, b.data(), 0, 0x0b, 0, &dummy, &dummy, 0x140);
	if (g_low_tunnel_known != 0) {
		/* POINT1.LTX[52]: "Ihr erreicht wieder die kleine Lichtung im Wald, an deren Ostrand die bemoosten und
		 * ueberwucherten Reste einer kleinen, zerfallenen Burg stehen. Wollt Ihr" */
		/* POINT1.LTX[53]: "in das Gemaeuer eindringen und wieder den Geheimgang benutzen, oder" */
		/* POINT1.LTX[54]: "Euren Weg fortsetzen." */
		sel = dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[52], 2, g_tev_loc_txt[53], g_tev_loc_txt[54]);
		if (sel == 1) {
			if (group_find_key(g_group, ITEM_LOWANGEN_KEY) >= 0) {
				p->rest = 1;
				g_travel_state = -5;
			} else
				/* POINT1.LTX[59]: "Die Tuere zum Geheimgang ist verschlossen und Ihr habt den Schluessel nicht dabei." */
				dlg_message(g_dialog, g_tev_loc_txt[59]);
		}
	} else
		/* POINT1.LTX[57]: "Ihr erreicht eine kleine Lichtung im Wald, an deren Ostrand die bemoosten und ueberwucherten
		 * Reste einer kleinen, zerfallenen Burg stehen." */
		dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[57], 0);
	if (!g_tev_lowangen_key_noted && group_find_key(g_group, ITEM_LOWANGEN_KEY) != -1) {
		/* ALL_SECT.LTX[113]: "Am Ostrand einer kleinen Lichtung im Wald steht die Ruine einer kleinen, zerfallenen Burg.
		 * Hier endet der Geheimgang, der in der Ordensburg der grauen Staebe in Lowangen beginnt." */
		diary_add(g_diary, g_tev_txt[113], 0);
		g_tev_lowangen_key_noted = 1;
	}
}

void place_event_104(Travel far *p)
{
}

void place_event_106(Travel far *p)
{
	signed short dummy;
	signed char sel;

	if (g_finsterkopp_entered != 0) {
		Bmp b(0x40, 0x40);

		nvfile_load(g_nvf_heads, 0x198, b.data(), 0, 0x0d, 0, &dummy, &dummy, 0x140);
		/* POINT1.LTX[51]: "Ihr steht wieder vor dem Eingang zu der inzwischen eingestuertzten Hoehle. Hier ist kein
		 * Weiterkommen moeglich." */
		dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[51], 0);
		route_clear(p);
		p->route[0] = 0x6a;
		p->route[1] = 0xc1;
		p->route[2] = 0xff;
		p->action = 4;
	} else {
		Bmp b(0x40, 0x40);

		nvfile_load(g_nvf_heads, 0x198, b.data(), 0, 0x0c, 0, &dummy, &dummy, 0x140);
		/* POINT1.LTX[58]: "Der Pfad endet vor dem Eingang zu einer natuerlichen Hoehle. Das Loch hat einen Durchmesser
		 * von etwa zwei Schritt. Wollt Ihr" */
		/* POINT1.LTX[49]: "in das Loch hineingehen, oder" */
		/* POINT1.LTX[50]: "wieder umkehren?" */
		sel = dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[58], 2, g_tev_loc_txt[49], g_tev_loc_txt[50]);
		if (sel == 1) {
			g_finsterkopp_entered = 1;
			g_travel_state = -6;
		}
	}
}

void place_event_114(Travel far *p)
{
	signed short dummy;
	Bmp b(0x40, 0x40);

	nvfile_load(g_nvf_heads, 0x198, b.data(), 0, 0x0e, 0, &dummy, &dummy, 0x140);
	/* POINT1.LTX[0]: "Ein steiler Pfad fuehrt in westlicher Richtung den Berg hinauf; oben erkennt Ihr ein paar
	 * Hoehlen." */
	dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[0], 0);
}

void place_event_117(Travel far *p)
{
}

void place_event_128(Travel far *p)
{
}

void place_event_131(Travel far *p)
{
	signed short dummy;
	Bmp b(0x40, 0x40);

	nvfile_load(g_nvf_heads, 0x198, b.data(), 0, 0x11, 0, &dummy, &dummy, 0x140);
	/* POINT1.LTX[3]: "Der Weg versumpft zusehends und endet schliesslich im Morast der Brinasker Marschen.@Ihr muesst
	 * umkehren." */
	dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[3], 0);
	p->route[0] = 0x83;
	p->route[1] = 0x35;
	p->route[2] = 0xff;
	p->action = 4;
}

void place_event_142(Travel far *p)
{
	signed short dummy;
	Bmp b(0x40, 0x40);

	nvfile_load(g_nvf_heads, 0x198, b.data(), 0, 0x1c, 0, &dummy, &dummy, 0x140);
	/* POINT1.LTX[4]: "Der Weg endet vor einem Felsmassiv. Hier bleibt nur der Weg zurueck." */
	dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[4], 0);
	p->route[0] = 0x8e;
	p->route[1] = 0x87;
	p->route[2] = 0xff;
	p->action = 4;
}

void place_event_170(Travel far *p)
{
	signed short dummy;
	signed char sel;
	Bmp b(0x40, 0x40);

	nvfile_load(g_nvf_heads, 0x198, b.data(), 0, 0x13, 0, &dummy, &dummy, 0x140);
	/* POINT1.LTX[5]: "Der Pfad laeuft auf eine Steilwand zu und fuehrt in einer Hoehle tiefer in den Berg hinein.
	 * Wollt Ihr" */
	/* POINT1.LTX[43]: "die Hoehle betreten, oder" */
	/* POINT1.LTX[44]: "lieber umdrehen?" */
	sel = dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[5], 2, g_tev_loc_txt[43], g_tev_loc_txt[44]);
	route_clear(p);
	if (sel == 1)
		g_travel_state = -4;
	else {
		p->route[0] = 0xaa;
		p->route[1] = 0xa4;
		p->route[2] = 0xff;
		p->action = 4;
		p->f109 = 1;
	}
}

void place_event_171(Travel far *p)
{
	signed short dummy;
	signed char sel;
	Bmp b(0x40, 0x40);

	nvfile_load(g_nvf_heads, 0x198, b.data(), 0, 0x14, 0, &dummy, &dummy, 0x140);
	/* POINT1.LTX[6]: "Im Osten fuehrt ein schmaler Pfad den Berg hinauf und endet dort vor einem Hoehleneingang.
	 * Wollt Ihr" */
	/* POINT1.LTX[45]: "den Pfad hinaufsteigen und die Hoehle betreten, oder" */
	/* POINT1.LTX[46]: "Euch die Muehe sparen und weitergehen?" */
	sel = dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[6], 2, g_tev_loc_txt[45], g_tev_loc_txt[46]);
	if (sel == 1) {
		route_clear(p);
		p->rest = 1;
		g_travel_state = -3;
	}
}

void place_event_183(Travel far *p)
{
	signed short dummy;
	signed char ans;
	Bmp far *b;

	b = new Bmp(0x40, 0x40);
	nvfile_load(g_nvf_heads, 0x198, b->data(), 0, 0x15, 0, &dummy, &dummy, 0x140);
	/* POINT1.LTX[7]: "Ihr erreicht einen grossen Waldsee. In einiger Entfernung entdeckt Ihr am Ufer eine Holzhuette.
	 * Wollt Ihr" */
	/* POINT1.LTX[8]: "anklopfen und die Huette betreten, oder" */
	/* POINT1.LTX[9]: "einfach weitergehen?" */
	ans = dlg_box_bmp(g_dialog, b, (char far *)0, g_tev_loc_txt[7], 2, g_tev_loc_txt[8], g_tev_loc_txt[9]);
	delete b;
	if (ans == 1) {
		travel_leave(p);
		if (conv_run(g_dialog2, 0x81))
			timewarp_minutes(g_date, 30L);
		mouse_hide(0);
		travel_enter(p);
		mouse_show(0);
		travelmap_draw(p);
		travel_redraw(p);
	}
}

void place_event_185(Travel far *p)
{
}

void place_event_187(Travel far *p)
{
	signed char sel;
	signed short dummy;
	signed short w;
	signed short h;

	nvfile_info(g_nvf_heads, 0x196, 8, &w, &h);
	Bmp b1(w, h);
	nvfile_load(g_nvf_heads, 0x196, b1.data(), 0, 8, 0, &dummy, &dummy, 0x140);
	/* POINT.LTX[9]: "Ihr habt keine Chance zu entkommen. Von vorn kommt Euch ein kleines Heer Orks entgegen, hinter
	 * Euch stuermen ploetzlich zwanzig weitere Schwarzpelze aus den Bergen. Ihr seid umzingelt." */
	dlg_box_bmp(g_dialog, &b1, (char far *)0, g_tev_loc_txt[9], 0);
	Bmp b2(0x40, 0x40);
	nvfile_load(g_nvf_heads, 0x198, b2.data(), 0, 0x17, 0, &dummy, &dummy, 0x140);
	/* POINT.LTX[10]: "Einer der Orks, offensichtlich ihr Hauptmann, spricht Euch an:@<Du Gefangene von %s, Du ergeben
	 * oder erledigt!>" */
	/* POINT.LTX[58]: "Ugorzzih" */
	sprintf(g_text_output_buf, g_tev_loc_txt[10], g_tev_loc_txt[58]);
	dlg_box_bmp(g_dialog, &b2, (char far *)0, g_text_output_buf, 0);
	/* POINT.LTX[11]: "Wollt Ihr" */
	/* POINT.LTX[12]: "Euch ergeben?" */
	/* POINT.LTX[13]: "oder kaempfen?" */
	sel = dlg_box_bmp(g_dialog, &b2, (char far *)0, g_tev_loc_txt[11], 2, g_tev_loc_txt[12], g_tev_loc_txt[13]);
	if (sel == -1)
		sel = 1;
	if (sel == 2)
		/* POINT.LTX[14]: "Als Ihr aufwacht, stellt Ihr fest, dass man Euch die Augen verbunden hat. Ausser einigen
		 * Kleidungsfetzen und Gegenstaenden, von denen die Orks vermutlich glaubten, dass Ihr sie fuers marschieren
		 * benoetigt, ist Euer gesamtes Habe gestohlen. - Ihr koennt Euch gar nicht richtig an den Kampf erinnern.
		 * Irgendwann hat Euch wohl einer dieser Lumpen am Kopf getroffen. - Ihr werdet hart in die Seite geknufft. Die
		 * Orks setzen sich in Bewegung, und Ihr werdet von ihnen mit Tritten und Griffen auf den richtigen Weg
		 * dirigiert." */
		dlg_message(g_dialog, g_tev_loc_txt[14]);
	switch (sel) {
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
		/* POINT.LTX[1]: "Eure unfreiwillige Reise fuehrt durchs Gebirge. Da Ihr den Weg nicht sehen koennt, stolpert Ihr
		 * oft und fallt. Immer wieder bringen Euch die Orks mit Tritten und festen Griffen auf die Beine." */
		dlg_message(g_dialog, g_tev_loc_txt[1]);
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

void place_event_189(Travel far *p)
{
	signed short dummy;
	signed char sel;
	signed char old;
	struct hero far *h;
	void far *win;
	Bmp b1(0x40, 0x40);

	nvfile_load(g_nvf_heads, 0x197, b1.data(), 0, 0x18, 0, &dummy, &dummy, 0x140);
	/* POINT1.LTX[15]: "Ueberall um Euch herum sind die Baeume und Straeucher verdorrt, nicht das kleinste bisschen
	 * Gruen ist geblieben. Der Geruch ist unertraeglich geworden und bereitet Euch Uebelkeit.@Wollt Ihr" */
	/* POINT1.LTX[16]: "weitergehen?" */
	/* POINT1.LTX[18]: "umkehren?" */
	sel = dlg_box_bmp(g_dialog, &b1, (char far *)0, g_tev_loc_txt[15], 2, g_tev_loc_txt[16], g_tev_loc_txt[18]);
	group_sub_le_range(g_group, 1, 3);
	group_get_ill(g_group, 4, 5);
	if (sel == 2) {
		p->route[0] = 0xbd;
		p->route[1] = 0xbc;
		p->route[2] = 0xff;
		p->action = 4;
	} else {
		h = get_worst_hero_skill(g_group, TA_ZECHEN);
		Bmp b2(0x40, 0x40);
		nvfile_load(g_nvf_heads, 0x198, b2.data(), 0, 0x18, 0, &dummy, &dummy, 0x140);
		/* POINT1.LTX[19]: "Ihr erreicht eine kleine Anhoehe. Die Landschaft, auf die Ihr von hier blicken koennt,
		 * gleicht einer Wueste, und dennoch ist Wueste ein schmeichelhaftes Wort fuer das, was Ihr seht. Der beissende
		 * Faeulnisgeruch brennt in Euren Nasen. %s ist es davon so uebel geworden, dass %s sich uebergeben muss.
		 * Inmitten der veroedeten Landschaft entdeckt Ihr einen schwarzen Fleck, der sich zu bewegen scheint.@Wollt Ihr" */
		sprintf(g_text_output_buf, g_tev_loc_txt[19], h->name, grammar_pronoun(h->sex, 0, 0));
		sub_hero_le(h, 2);
		group_sub_le_range(g_group, 3, 5);
		group_get_ill(g_group, 4, 10);
		/* POINT1.LTX[20]: "weitergehen?" */
		/* POINT1.LTX[21]: "umkehren?" */
		sel = dlg_box_bmp(g_dialog, &b2, (char far *)0, g_text_output_buf, 2, g_tev_loc_txt[20], g_tev_loc_txt[21]);
		if (sel == 2) {
			p->route[0] = 0xbd;
			p->route[1] = 0xbc;
			p->route[2] = 0xff;
			p->action = 4;
			/* ALL_SECT.LTX[114]: "Wir haben eine seltsame Einoede erreicht: <Wueste> ist noch eine recht schmeichelhafte
			 * Bezeichnung dafuer. Das schlimmste ist dieser unertraegliche Gestank, der ueber der ganzen Gegend liegt." */
			diary_add(g_diary, g_tev_txt[114], 0);
		} else {
			old = dlg_set16(g_dialog, 4);
			win = Anim_ctor(0, 0x1d5, 0, 0);
			snd_play(g_snd_fx, 0xae);
			/* POINT1.LTX[17]: "Den Anblick eines Basilisken hat bis jetzt noch niemand ueberlebt, und daran wird sich auch
			 * nichts aendern." */
			dlg_box_anim(g_dialog, win, " ", g_tev_loc_txt[17], 0);
			while (group_count_alive(g_group) > 0)
				group_sub_le(g_group, 20);
			Anim_dtor(win, 3);
			dlg_set16(g_dialog, old);
			g_travel_state = 1;
			diary_add(g_diary, g_tev_txt[114], 0);
		}
	}
}

void place_event_196(Travel far *p)
{
	signed short dummy;

	if (!g_place197_done) {
		Bmp b(0x40, 0x40);

		nvfile_load(g_nvf_heads, 0x198, b.data(), 0, 0x19, 0, &dummy, &dummy, 0x140);
		/* POINT1.LTX[22]: "Ein Ruck geht durch Euer Schiff. Der Schiffer steuert das linke Svelltufer an und betrachtet
		 * den Schaden. <Wir werden wohl ein bis zwei Stunden hier liegen bleiben muessen, damit ich den Schaden
		 * reparieren kann>" */
		dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[22], 0);
		timewarp_hours(g_date, 2L);
	}
}

void place_event_197(Travel far *p)
{
	signed short dummy;
	signed char r;
	Bmp far *b;

	if (!g_place197_done) {
		b = new Bmp(0x40, 0x40);
		nvfile_load(g_nvf_heads, 0x198, b->data(), 0, 0x19, 0, &dummy, &dummy, 0x140);
		/* POINT1.LTX[23]: "<Wir muessen noch einmal anlegen, damit ich die Reperaturarbeiten ueberpruefen kann>,
		 * erklaert der Schiffer, und steuert wieder das linke Svelltufer an." */
		dlg_box_bmp(g_dialog, b, (char far *)0, g_tev_loc_txt[23], 0);
		delete b;
		/* POINT1.LTX[26]: "Kaum habt Ihr das Boot verlassen, um Euch inzwischen die Fuesse zu vertreten, da springen
		 * fuenf dunkle Gestalten aus den Bueschen, die offensichtlich nur auf die Ankunft des Bootes gewartet haben." */
		dlg_message(g_dialog, g_tev_loc_txt[26]);
		travel_leave(p);
		g_fight_flag = 1;
		r = start_fight(0x5e, 1);
		travel_enter(p);
		if (!fight_result(p, r))
			/* POINT1.LTX[27]: "Der Schiffer hat sich angesichts der Raeuberbande mit dem Boot aus dem Staub gemacht, Ihr
			 * muesst Euren Weg wohl zu Fuss fortsetzen." */
			dlg_message(g_dialog, g_tev_loc_txt[27]);
		p->from = 0x30;
		route_clear(p);
		g_group->set_route_from(p->from);
		g_group->set_route_to(p->from);
		place_reveal_dirs(p, p->from);
		p->route[1] = 0xff;
	}
}

void place_event_198(Travel far *p)
{
}
