/* OVL259 - Overlay O259 (Segment 0x24aa in SCHWEIF.EXE): Szenen in Gespraechen (Gespraechsobjekt g_dialog2 = class Conv
 * aus O049/O257: Texte XDS.LTX = Datei 0x1d8 nach scene_buf/scene_txt, Bildschirm bmp/bg, Zustand done, Gespraech
 * id, Menuetexte menu_txt). Aufrufe aus den Gespraechs-Rueckrufen O258, O179 und O257.
 * conv_scene_redraw: Gespraechsbild neu (bmp nach g_vga_buf). conv_finish: Gespraech beenden (done = 2, level =
 *   Geduld des Skripts + 6). conv_scene_picture: Ja/Nein, dann das Bild LOWANGEN.AIF (0x1c7) bis Taste/Maus.
 * conv_scene_lore(a): Erzaehlung XDS.LTX[a - 1] (Zwerge, Sternenschweif, Tordol) mit den Folgeseiten [3..8] bis
 *   "Ende", g_lore_told = 1. conv_scene_brothel(a): Bordell [9]/[10] fuer 5 Dukaten je Kopf ([11] zu wenig Geld,
 *   [12] Nacht bis 8 Uhr, ggf. Schenke verlassen). conv_scene_iwain: Wutrede Iwain Basiliskentoeters [14..19].
 *   conv_scene_meal: Mahlzeit [13] (Hunger/Durst 0). conv_scene_music: mit einem Elfen in der Gruppe Bitte um Musik
 *   [20], Kraeutertrunk [21] (Gegenstand 0x92), g_music_played.
 * conv_bye_scene: Abschied je Gespraech (0x66 TJOL09 -> Bordell-Frage, 0x89/0x8a ohne Abschied), done = 2.
 * conv_scene_dragan_task: Dragans Auftraege je g_dragan_task - Vinsalter holen [22]/[26], Brosche aus dem Schauhaus
 *   [23]/[27], Paeckchen fuer Meister Deringorn [23]/[28] (Gegenstand 0x14a), Tagebuch [44..46]; quitt [24],
 *   Ablehnung [25]. conv_scene_shed: Schuppen fuer 5 Silberlinge je Kopf [29..33] (nur bei g_shed_free_today, Monatswurf),
 *   g_low_gavron_house = 0x3f. conv_scene_no_room: kein Platz frei [34..41], auf Dragans Empfehlung (g_dragan_room_today)
 *   g_low_gavron_house = 0x41. conv_scene_artherion: einmal (g_artherion_heard gesetzt, g_artherion_hut_known noch nicht) Auskunft ueber
 *   Artherion [42]/[43]. conv_scene_note(a): Tagebuch [47 + a] (Dragans Tipps). conv_scene_no_idea: Zufallsantwort
 *   [51..55].
 *
 * BCC -mlarge -3 -Ot -c -Y OVL259.CPP   -> 5244/5244 Bytes GLEICH (18.10.2026; mit -Og ebenso), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Erster Entwurf sofort GLEICH. C++ nur fuer conv_scene_picture (Pic 0xa6 Bytes zuerst deklariert -> [bp-0b0h],
 *   Bmp [bp-0ah]; Ktoren in Deklarationsreihenfolge, Dtoren umgekehrt).
 * - Die Funktionen nehmen das Gespraechsobjekt als void far * (Prototypen) und greifen ueber P = (Conv far
 *   *)conv zu (gleicher Code); level = (signed char)script->tolerance + 6 liest nur das niedere Byte.
 * - "P->scene_txt[a - 1]" / "P->scene_txt[r + 51]" (Index kein Register) -> Index zuerst, Zeiger als mov dx/add ax-Paar,
 *   Konstante in der Verschiebung ([bx-4], [bx+0cch]); "P->scene_txt[i]" mit register i -> les bx zuerst.
 * - "if ((cost = n) > g_group->money)" -> movsx eax / Store / cmp eax (eingebettet); "(signed long)n * 500" ->
 *   imul eax,eax,1f4h. switch conv_scene_dragan_task: case 3 leer mit break, default danach; conv_scene_note break
 *   + "default: break;".
 * Originalfehler: conv_scene_music liefert ohne Elfen einen uninitialisierten Wert (r wird nur im if-Zweig gesetzt).
 */
#include "schweif.h"

#define P		((Conv far *)conv)
#define FILE_XDS	0x1d8		/* XDS.LTX */
#define PIC_LOWANGEN	0x1c7		/* LOWANGEN.AIF */

void conv_scene_redraw(void far *conv)
{
	P->bmp.put(g_vga_buf, 0, 0);
}

void conv_finish(void far *conv)
{
	P->done = 2;
	P->level = (signed char)P->script->tolerance + 6;
}

void conv_scene_picture(void far *conv)
{
	Pic pic;
	Bmp b(320, 200, P->bg);

	txt_load_list(g_font, FILE_XDS, 0x3c, &P->scene_buf, &P->scene_txt);
	/* GLOBLTXT.LTX[446]: "Ja" */
	/* GLOBLTXT.LTX[447]: "Nein" */
	conv_set_menu(conv, 2, g_ttx[446], g_ttx[447]);
	if (conv_menu(conv, 2) == 1) {
		pic.loadf(PIC_LOWANGEN, P->bg);
		set_palette(pic.get_palette(), 0, 0x30);
		mouse_hide(0);
		b.put(g_vga_buf, 0, 0);
		mouse_show(0);
		do
			poll_input();
		while (g_key == 0 && g_mouse_lclick == 0 && g_mouse_rclick == 0);
		mouse_clear_left();
		mouse_clear_right();
	}
	P->scene_buf = mem_free(P->scene_buf);
	P->scene_txt = (char far * far *)mem_free(P->scene_txt);
}

void conv_scene_lore(void far *conv, signed short a)
{
	register signed short i;

	txt_load_list(g_font, FILE_XDS, 0x3c, &P->scene_buf, &P->scene_txt);
	conv_set_menu(conv, 2, P->menu_txt[7], P->menu_txt[4]);
	mouse_hide(0);
	/* XDS.LTX[0]: "<Ich bin ueberrascht. Also gut, wenn Ihr wirklich etwas ueber Zwerge wissen wollt, fangen wir am
	 * besten von vorne an:>" */
	/* XDS.LTX[1]: "<Ihr wollt die Geschichte von Sternenschweif hoeren - nun gut...>" */
	/* XDS.LTX[2]: "<Nun der groesste Schmied aller Zeiten war der Zwergenfuerst Tordol, Sohn des Thiodasch. Aber das
	 * ist eine lange Geschichte...>" */
	conv_print(conv, P->scene_txt[a - 1], 0);
	mouse_show(0);
	if (conv_menu(conv, 2) == 1) {
		for (i = 3; i < 9; i++) {
			mouse_hide(0);
			conv_scene_redraw(conv);
			font_set_colors(g_font, 0xff, -1);
			font_set_box(g_font, 10, 0xd2, 0x2e, -1);
			/* XDS.LTX[3]: "Lange bevor die uebrigen Voelker sich in diesem Gebiet breitmachten, gab es im Thasch einen
			 * Zwergenfuersten namens Tordol, Sohn des Thiondasch und er war der beste Schmied auf Dere.@Nur aus dem besten
			 * Stahl wollte er sein Meisterstueck anfertigen, deshalb zog er mit seinen Vertrauten los, um das passende Erz
			 * zu einer vollendeten Legierung zu suchen." */
			/* XDS.LTX[4]: "In den Grossen Zinnen hatten sich die Orks eingenistet, die dem Fuersten den schuldigen Tribut
			 * verweigerten. Schliesslich forderte Tordol diesen mit Blut ein. Es fand ein Gemetzel statt und Tordol
			 * unterwarf die Orkenbrut - die Grossen Zinnen wurden fortan nur noch die Blutzinnen geheissen.@Das Erz der
			 * Zinnen erwies sich jedoch als unrein, und so zog Tordol weiter nach Norden, doch auch die Erze des Rohrwed
			 * waren nicht von jener Art, die Tordol suchte." */
			/* XDS.LTX[5]: "Als Tordol wiederum weiterzog, erhielt er von Ingerimm ein Zeichen. Daraufhin fand er auf dem
			 * hoechsten Berg des Duesterkamms ein bislang unbekanntes Erz - die Finstererde. Aus der Finstererde schmiedete
			 * Tordol eine Vielzahl von Meisterstuecken. In jener Zeit entstand auch die Finsterkoppenbinge und Tordol
			 * weihte dem Ingerimm zum Dank eine grosse Tempelhoehle." */
			/* XDS.LTX[6]: "Thiondasch, Tordols Sohn, war bestrebt die Schmiedekunst seines Vaters zu uebertreffen. Im
			 * Glauben, dass das Erz fuer das Misslingen seiner Schmiedewerke verantwortlich sei, raubte er das gesamte
			 * Erzvorkommen der Elfen und kehrte damit nach Finsterkoppen zurueck.@Ingerimm konnte diesen Frevel nicht
			 * dulden und so bestrafte er Thiondasch, indem er dessen Seele in den Altarstein des Tempels bannte und eine
			 * neue Gestalt daraus formte. Es heisst, Thiondasch muesse dem Gott dort noch heute zu Diensten sein." */
			/* XDS.LTX[7]: "Nun wurde Ingerimm bei der Zurueckfuehrung des Erzes des Tagewerks ueberdruessig. Dies bemerkte
			 * der schlaue Phex und so schlug er Ingerimm einen Handel vor: Ein Teil des Erzes solle den Zwergen ueberlassen
			 * werden, das erste daraus gefertigte Schmiedewerk wuerde er dafuer erhalten. Ingerimm ging auf den Handel ein,
			 * doch zeigte es sich, dass das restliche Salamandererz nur fuer eine einzige Schmiedearbeit ausreichte.
			 * Ingerimm liess Thiondasch daraufhin Sternenschweif anfertigen, da er wusste, dass die Axt dem Fuchs keinen
			 * Nutzen bringen wuerde." */
			/* XDS.LTX[8]: "<... nun, die Geschichte ist natuerlich noch laengst nicht zu Ende, doch ich glaube, wenn ich
			 * jetzt nicht aufhoere, sitzt Ihr naechste Woche noch hier, also lassen wir es gut sein.>" */
			conv_print(conv, P->scene_txt[i], 0);
			mouse_show(0);
			if (conv_menu(conv, 2) == 2)
				break;
		}
	}
	P->scene_buf = mem_free(P->scene_buf);
	P->scene_txt = (char far * far *)mem_free(P->scene_txt);
	g_lore_told = 1;
}

void conv_scene_brothel(void far *conv, signed short a)
{
	signed long cost;

	txt_load_list(g_font, FILE_XDS, 0x3c, &P->scene_buf, &P->scene_txt);
	mouse_hide(0);
	font_set_colors(g_font, 0xff, -1);
	/* XDS.LTX[9]: "<Na also, ich dachte schon, Ihr waeret falsch hier. Das macht 5 Dukaten pro Kopf, inclusive
	 * Uebernachtung, kommt nur mit.>" */
	/* XDS.LTX[10]: "<Jetzt ist aber genug geredet; deshalb seid Ihr doch sicher nicht hier. Wie waere es denn mit ein
	 * wenig Entspannung?>" */
	conv_print(conv, P->scene_txt[a + 9], 0);
	mouse_show(0);
	/* GLOBLTXT.LTX[446]: "Ja" */
	/* GLOBLTXT.LTX[447]: "Nein" */
	conv_set_menu(conv, 2, g_ttx[446], g_ttx[447]);
	if (conv_menu(conv, 2) == 1) {
		cost = (signed long)group_count_check2b(g_group) * 500;
		font_set_colors(g_font, 0xff, -1);
		if (g_group->money < cost) {
			mouse_hide(0);
			/* XDS.LTX[11]: "<Soviel Geld habt Ihr doch gar nicht!>" */
			conv_print(conv, P->scene_txt[11], 1);
			mouse_show(0);
		} else {
			mouse_hide(0);
			/* XDS.LTX[12]: "Ihr verbringt eine angenehme Nacht im Bordell." */
			conv_print(conv, P->scene_txt[12], 1);
			mouse_show(0);
			g_dialog_depth--;
			timewarp_until(g_date, 0xa8c0L);
			g_dialog_depth++;
			P->done = 1;
			group_sub_money(g_group, cost);
			if (g_tavern != 0)
				tavern_leave(g_tavern);
		}
		/* GLOBLTXT.LTX[377]: "Weiter" */
		conv_set_menu(conv, 1, g_ttx[377]);
		conv_menu(conv, 1);
	}
	if (P->done == 2)
		P->done = 1;
	P->scene_buf = mem_free(P->scene_buf);
	P->scene_txt = (char far * far *)mem_free(P->scene_txt);
}

void conv_scene_iwain(void far *conv)
{
	register signed short i;

	txt_load_list(g_font, FILE_XDS, 0x3c, &P->scene_buf, &P->scene_txt);
	/* GLOBLTXT.LTX[377]: "Weiter" */
	conv_set_menu(conv, 1, g_ttx[377]);
	mouse_hide(0);
	font_set_colors(g_font, 0xff, -1);
	/* XDS.LTX[14]: "<Sternenschweif? Sagtet Ihr gerade wirklich Sternenschweif? Ihr muesst lebensmuede sein, dieses
	 * Wort in Gegenwart von Iwain Basiliskentoeter zu verwenden! Sagt das nie wieder, versteht Ihr! Bloss weil uns
	 * diese daemliche Kuh gesagt hat, da gaebe es diese Schaetze, sind wir ueberhaupt dahin!>" */
	conv_print(conv, P->scene_txt[14], 0);
	mouse_show(0);
	/* XDS.LTX[15]: "Der reinste Wahnsinn! Tausende von diesen Kreaturen rundherum, und Alara wollte unbedingt da
	 * rein. Na, das hat sie jetzt davon. Jetzt steht sie in der Gegend herum, und wenn ich das richtig verstanden
	 * habe, dann bleibt das auch so.>" */
	/* XDS.LTX[16]: "<Ich bin ja sowieso nur mitgekommen, weil ich die doch nicht alleine ins Verderben stolpern
	 * lassen konnte. Aber als ich das gesehen habe, habe ich mich aus dem Staub gemacht.>" */
	/* XDS.LTX[17]: "Was sagt Ihr da? Fuer die Goetter? Hoert mir bloss damit auf! Noch ein Wort von denen, und ich
	 * dreh Euch die Haelse um. Ihr habt leicht reden. Wart Ihr in Grangor dabei, damals? Nein, wart Ihr nicht. Sonst
	 * wuerdet Ihr jetzt nicht fuer diese Brut Eure Aersche riskieren. Ja, Brut habe ich gesagt. Alles uebelste
	 * Echsenmagie, wenn Ihr mich fragt. Wisst Ihr, was die gemacht haben? Eine ganze Stadt von Elfen haben die in
	 * Orks verwandelt! Naja, jedenfalls fast. Die spitzen Ohren haben sie noch, aber sonst sind es Orks. Sowas kann
	 * doch nur Echsenmagie sein." */
	/* XDS.LTX[18]: "Da muesst Ihr die Echsen halt mal fragen; hier etwas noerdlicher soll es ja welche geben, mitten
	 * im Sumpf. Aber wenn Ihr die fragen wollt, koennt Ihr Euch auch gleich in ein Schwert stuerzen, stehe zu
	 * Diensten. Ach, ist mir doch egal, ob Ihr Euch ins Verderben stuerzt. Wuensche viel Spass dabei!" */
	/* XDS.LTX[19]: "Sternenschweif! Ich habe Leute schon fuer ganz andere Dinge zu Boron geschickt! Und wenn es Ihnen
	 * da auch nur halb so schlimm geht, wie ich es vermute, dann...>" */
	for (i = 15; i < 20; i++) {
		conv_menu(conv, 1);
		mouse_hide(0);
		conv_scene_redraw(conv);
		font_set_colors(g_font, 0xff, -1);
		font_set_box(g_font, 10, 0xd2, 0x2e, -1);
		conv_print(conv, P->scene_txt[i], 0);
		mouse_show(0);
	}
	P->scene_buf = mem_free(P->scene_buf);
	P->scene_txt = (char far * far *)mem_free(P->scene_txt);
}

signed short conv_scene_meal(void far *conv)
{
	txt_load_list(g_font, FILE_XDS, 0x3c, &P->scene_buf, &P->scene_txt);
	group_clr_hunger_thirst(g_group);
	/* XDS.LTX[13]: "Ihr bekommt eine ordentliche Mahlzeit vorgesetzt, die sehr gut schmeckt." */
	conv_print(conv, P->scene_txt[13], 1);
	P->scene_buf = mem_free(P->scene_buf);
	P->scene_txt = (char far * far *)mem_free(P->scene_txt);
	return 0;
}

signed short conv_scene_music(void far *conv)
{
#if FIX_0132
	/* Fix #132: Rueckgabe vorbelegen (0 wie die anderen Rueckrufe) */
	signed short r = 0;

	if (group_has_elf(g_group)) {
#else
	signed short r;

	if (group_has_elf(g_group)) {
#endif
		txt_load_list(g_font, FILE_XDS, 0x3c, &P->scene_buf, &P->scene_txt);
		mouse_hide(0);
		font_set_colors(g_font, 0xff, -1);
		/* XDS.LTX[20]: "<Wollt Ihr mir nicht etwas vorspielen? Ihr wuerdet mich sehr gluecklich machen!>" */
		conv_print(conv, P->scene_txt[20], 0);
		mouse_show(0);
		/* GLOBLTXT.LTX[446]: "Ja" */
		/* GLOBLTXT.LTX[447]: "Nein" */
		conv_set_menu(conv, 2, g_ttx[446], g_ttx[447]);
		if (conv_menu(conv, 2) == 1) {
			font_set_colors(g_font, 0xff, -1);
			/* XDS.LTX[21]: "<Das ist ja wundervoll! Habt meinen Dank. Hier, nehmt das; ein kraeftiger Kraeutertrunk. Den
			 * habe ich selber gemacht; ist gut gegen allerlei.>" */
			conv_print(conv, P->scene_txt[21], 0);
			group_give_item(g_group, 0x92, 1);
			g_music_played = 1;
			do
				poll_input();
			while (g_key == 0 && g_mouse_lclick == 0 && g_mouse_rclick == 0);
			mouse_clear_left();
			mouse_clear_right();
		}
		P->scene_buf = mem_free(P->scene_buf);
		P->scene_txt = (char far * far *)mem_free(P->scene_txt);
		r = -1;
	}
	return r;
}

signed short conv_bye_scene(void far *conv)
{
	register signed short r = 0;

	if (P->id == 0x66)
		r = conv_cb_tjol09_brothel(1, 3);
	else if (P->id == 0x89 || P->id == 0x8a)
		r = 1;
	P->done = 2;
	return r;
}

signed short conv_scene_dragan_task(void far *conv)
{
	register signed short fail = 0;

	txt_load_list(g_font, FILE_XDS, 0x3c, &P->scene_buf, &P->scene_txt);
	/* GLOBLTXT.LTX[446]: "Ja" */
	/* GLOBLTXT.LTX[447]: "Nein" */
	conv_set_menu(conv, 2, g_ttx[446], g_ttx[447]);
	font_set_colors(g_font, 0xff, -1);
	font_set_tab(g_font, 0xcd, 0);
	switch (g_dragan_task) {
	case 0:
		mouse_hide(0);
		/* XDS.LTX[22]: "<Ich wuesste so einiges, was Euch weiterhelfen koennte, aber da muesstet Ihr mir schon eine
		 * kleine Gefaelligkeit erweisen.>" */
		conv_print(conv, P->scene_txt[22], 0);
		mouse_show(0);
		if (conv_menu(conv, 2) == 1) {
			mouse_hide(0);
			/* XDS.LTX[26]: "<Bringt mir den Vinsalter hierher, er muss mir was uebersetzen. Der Alte wohnt in Bunte Flucht,
			 * direkt am Ufer des Svellt.>" */
			conv_print(conv, P->scene_txt[26], 0);
			mouse_show(0);
			g_low_dragan_task1 = 1;
			/* XDS.LTX[44]: "Wir haben von Dragan den Auftrag erhalten, den Vinsalter herbeizuschaffen. Das scheint nicht
			 * allzu schwer zu sein. Als Gegenleistung hat er uns Informationen angeboten - etwas was wir dringend brauchen
			 * koennen." */
			diary_add(g_diary, P->scene_txt[44], 0);
		} else
			fail = 1;
		break;
	case 1:
		mouse_hide(0);
		/* XDS.LTX[23]: "<Eine Gefaelligkeit - eine Information, so laeuft das nun mal. Aber wenn Ihr mir noch einmal
		 * behilflich sein wollt, kann ich sicher in dieser Sache etwas fuer Euch herausfinden.>" */
		conv_print(conv, P->scene_txt[23], 0);
		mouse_show(0);
		if (conv_menu(conv, 2) == 1) {
			mouse_hide(0);
			/* XDS.LTX[27]: "<Im Schauhaus neben dem Tsatempel liegt eine Brosche von mir - ein Erbstueck, das mir die Stadt
			 * sozusagen gestohlen hat. Die Brosche hat die Form einer Esche. Bringt mir das gute Stueck, dann werden wir
			 * weitersehen.>" */
			conv_print(conv, P->scene_txt[27], 0);
			mouse_show(0);
			g_low_brooch_state = 1;
			/* XDS.LTX[45]: "Dragan rueckt pro erfuellten Auftrag nur eine Information heraus. Diesmal sollen wir einen
			 * Einbruch fuer ihn verueben. Wir koennten nicht behaupten, dass uns dabei wohl ist." */
			diary_add(g_diary, P->scene_txt[45], 0);
		} else
			fail = 1;
		break;
	case 2:
		mouse_hide(0);
		conv_print(conv, P->scene_txt[23], 0);
		mouse_show(0);
		if (conv_menu(conv, 2) == 1) {
			mouse_hide(0);
			/* XDS.LTX[28]: "<Ich haette hier ein Paeckchen, gebt es bei Meister Deringorn in der Halle der Macht ab und
			 * bestellt ihm einen schoenen Gruss von mir.>" */
			conv_print(conv, P->scene_txt[28], 0);
			mouse_show(0);
			g_low_dragan_task3 = 1;
			group_give_item(g_group, 0x14a, 1);
			/* XDS.LTX[46]: "Dragan hatte mal wieder eine Aufgabe fuer uns: Einfach ein Paeckchen abgeben... Irgendwie zu
			 * einfach. Meister Deringorn ist der Empfaenger und normalerweise ist dieser in der Halle der Macht zu finden.
			 * Wir fragen uns, wo der Haken bei der ganzen Angelegenheit ist." */
			diary_add(g_diary, P->scene_txt[46], 0);
		} else
			fail = 1;
		break;
	case 3:
		break;
	default:
		mouse_hide(0);
		/* XDS.LTX[24]: "<Was soll ich sagen - wir sind quitt! Ihr habt ein paar Kleinigkeiten fuer mich erledigt, und
		 * ich habe Euch ein paar wichtige Informationen zukommen lassen. Also macht das Beste daraus...>" */
		conv_print(conv, P->scene_txt[24], 0);
		mouse_show(0);
		break;
	}
	if (fail) {
		mouse_hide(0);
		/* XDS.LTX[25]: "<Ihr muesst es ja wissen... tja, also dann - bis zum naechsten Mal.>" */
		conv_print(conv, P->scene_txt[25], 0);
		mouse_show(0);
	}
	P->scene_buf = mem_free(P->scene_buf);
	P->scene_txt = (char far * far *)mem_free(P->scene_txt);
	conv_finish(conv);
	mouse_clear_left();
	mouse_clear_right();
	return -1;
}

void conv_scene_shed(void far *conv)
{
	signed long cost;
	signed short n;

	txt_load_list(g_font, FILE_XDS, 0x3c, &P->scene_buf, &P->scene_txt);
	if (g_shed_free_today != 0) {
		n = g_group->nr_heroes * 50;
		mouse_hide(0);
		font_set_colors(g_font, 0xff, -1);
		font_set_tab(g_font, 0xcd, 0);
		/* GLOBLTXT.LTX[446]: "Ja" */
		/* GLOBLTXT.LTX[447]: "Nein" */
		conv_set_menu(conv, 2, g_ttx[446], g_ttx[447]);
		/* XDS.LTX[29]: "<Gebt mir 5 Silberlinge pro Kopf, dann koennt Ihr es Euch im Schuppen bequem machen.>" */
		conv_print(conv, P->scene_txt[29], 0);
		mouse_show(0);
		if (conv_menu(conv, 2) == 1) {
			mouse_hide(0);
			if ((cost = n) > g_group->money) {
				font_set_colors(g_font, 0xf9, -1);
				/* XDS.LTX[31]: "<Soviel Geld haben wir nicht.>" */
				conv_print(conv, P->scene_txt[31], 0);
				font_set_colors(g_font, 0xff, -1);
				/* XDS.LTX[32]: "<Naja, dann haltet mich nicht laenger auf.>" */
				conv_print(conv, P->scene_txt[32], 0);
			} else {
				group_sub_money(g_group, cost);
				/* XDS.LTX[30]: "<Gut, dann kommt herein.>" */
				conv_print(conv, P->scene_txt[30], 0);
				g_low_gavron_house = 0x3f;
			}
			mouse_show(0);
		} else {
			mouse_hide(0);
			conv_print(conv, P->scene_txt[32], 0);
			mouse_show(0);
		}
	} else {
		mouse_hide(0);
		/* XDS.LTX[33]: "<Tut mir leid, ich habe keinen Platz frei.>" */
		conv_print(conv, P->scene_txt[33], 0);
		mouse_show(0);
	}
	mouse_clear_left();
	mouse_clear_right();
	P->scene_buf = mem_free(P->scene_buf);
	P->scene_txt = (char far * far *)mem_free(P->scene_txt);
	conv_finish(conv);
}

void conv_scene_no_room(void far *conv)
{
	txt_load_list(g_font, FILE_XDS, 0x3c, &P->scene_buf, &P->scene_txt);
	/* XDS.LTX[34]: "<Da kann ich Euch nicht weiterhelfen.>" */
	/* XDS.LTX[35]: "<Wirklich nicht?>" */
	/* XDS.LTX[36]: "<War ja nur eine Frage.>" */
	/* XDS.LTX[37]: "<Dragan hat uns zu Euch geschickt.>" */
	if (dlg_choose(g_dialog, P->scene_txt[34], 3, P->scene_txt[35], P->scene_txt[36], P->scene_txt[37]) == 3) {
		if (g_dragan_room_today != 0) {
			/* XDS.LTX[40]: "<Hmm, das ist was anderes. Mal sehen, wenn ich vielleicht...>" */
			dlg_message(g_dialog, P->scene_txt[40]);
			/* XDS.LTX[41]: "<Also gut, irgendwie wird es sich schon machen lassen.>" */
			dlg_message(g_dialog, P->scene_txt[41]);
			g_low_gavron_house = 0x41;
		} else
			/* XDS.LTX[39]: "<Ist mir egal, wer Euch schickt. Im Moment ist einfach nichts frei.>" */
			dlg_message(g_dialog, P->scene_txt[39]);
	} else
		/* XDS.LTX[38]: "<Hoert zu, wenn ich was sage, dann meine ich es auch so.>" */
		dlg_message(g_dialog, P->scene_txt[38]);
	mouse_clear_left();
	mouse_clear_right();
	P->scene_buf = mem_free(P->scene_buf);
	P->scene_txt = (char far * far *)mem_free(P->scene_txt);
	conv_finish(conv);
}

void conv_scene_artherion(void far *conv)
{
	if (g_artherion_heard != 0 && !g_artherion_hut_known) {
		g_artherion_hut_known = 1;
		txt_load_list(g_font, FILE_XDS, 0x3c, &P->scene_buf, &P->scene_txt);
		font_set_colors(g_font, 0xff, -1);
		font_set_tab(g_font, 0xcd, 0);
		mouse_hide(0);
		/* XDS.LTX[42]: "<Artherion? Der ist weg, nachdem seine Muehle abgebrannt ist. Er hat ziemliches Glueck gehabt,
		 * dass er da ueberhaupt wieder herausgekommen ist; jedenfalls ist er danach aus der Stadt verschwunden und hat
		 * sich nie wieder hier sehen lassen.>" */
		conv_print(conv, P->scene_txt[42], 0);
		mouse_show(0);
		/* GLOBLTXT.LTX[377]: "Weiter" */
		conv_set_menu(conv, 1, g_ttx[377]);
		conv_menu(conv, 1);
		mouse_hide(0);
		conv_scene_redraw(conv);
		font_set_colors(g_font, 0xff, -1);
		font_set_box(g_font, 10, 0xd2, 0x2e, -1);
		/* XDS.LTX[43]: "<Er soll jetzt in irgendwo noerdlich oder oestlich wohnen, mitten in der Einoede, aber das sind
		 * wahrscheinlich alles nur Geruechte. Woher soll man das auch wissen? Er redet jedenfalls mit niemandem mehr,
		 * heisst es.>" */
		conv_print(conv, P->scene_txt[43], 0);
		mouse_show(0);
		mouse_clear_left();
		mouse_clear_right();
		P->scene_buf = mem_free(P->scene_buf);
		P->scene_txt = (char far * far *)mem_free(P->scene_txt);
	}
	conv_finish(conv);
}

void conv_scene_note(void far *conv, signed short a)
{
	signed short x0;
	signed short y0;
	signed short x1;
	signed short y1;
	signed short old;

	font_get_box(g_font, &x0, &y0, &x1, &y1);
	old = font_get_y(g_font);
	txt_load_list(g_font, FILE_XDS, 0x3c, &P->scene_buf, &P->scene_txt);
	switch (a) {
	case 0:
		/* XDS.LTX[47]: "Bei Brin Vaskendantz im Fuchswinkel kann man eventuell fuer eine Nacht unterkommen. Dragan sagte
		 * uns, wir sollten uns auf jeden Fall auf ihn berufen." */
		diary_add(g_diary, P->scene_txt[47], 0);
		break;
	case 1:
		/* XDS.LTX[48]: "Essen kann man bei Raisha Rotenegger, im Sueden der Stadt bekommen. Der Tip kommt natuerlich von
		 * Dragan, dessen Namen wir auf jeden Fall nennen sollten." */
		diary_add(g_diary, P->scene_txt[48], 0);
		break;
	case 2:
		/* XDS.LTX[49]: "Dragan hat uns an die schwarze Jandora verwiesen. Mal sehen, ob es tatsaechlich einen geheimen
		 * Ausgang aus der Stadt gibt." */
		diary_add(g_diary, P->scene_txt[49], 0);
		break;
	case 3:
		/* XDS.LTX[50]: "Guenstige Waffen bekommt man bei Falkert Duberwin, weit im Norden, an der Oostermauer. Auch hier
		 * ist Dragans Name die Eintrittskarte." */
		diary_add(g_diary, P->scene_txt[50], 0);
		break;
	default:
		break;
	}
	P->scene_buf = mem_free(P->scene_buf);
	P->scene_txt = (char far * far *)mem_free(P->scene_txt);
	font_set_box(g_font, x0, y0, x1, y1);
	font_set_y(g_font, old);
}

void conv_scene_no_idea(void far *conv)
{
	signed short r;

	r = random_schweif(5) - 1;
	txt_load_list(g_font, FILE_XDS, 0x3c, &P->scene_buf, &P->scene_txt);
	/* XDS.LTX[51]: "<Da kann ich euch nichts sagen.>" */
	/* XDS.LTX[52]: "<Da kann ich Euch nicht weiterhelfen.>" */
	/* XDS.LTX[53]: "<Ich habe nicht die leiseste Ahnung.>" */
	/* XDS.LTX[54]: "<Tut mir leid, davon habe ich keine Ahnung!>" */
	/* XDS.LTX[55]: "<Lasst uns lieber ein anderes Thema waehlen.>" */
	conv_print(conv, P->scene_txt[r + 51], 0);
	P->scene_buf = mem_free(P->scene_buf);
	P->scene_txt = (char far * far *)mem_free(P->scene_txt);
}
