/* OVL115 - Overlay O115 (Segment 0x2195 in SCHWEIF.EXE): Reiseereignisse der Wege 37..48 (tevent_run O211;
 *   Reiseobjekt
 * o, Ort o->from; Texte g_tev_loc_txt = SECTION0.LTX, g_tev_txt = ALL_SECT.LTX; Merker route.event in g_routes).
 * tevent_037: bei route.event[37] == 2 (sonst Zaehler 1..6) Begegnung mit dem Baron von Meresfeld und seinen Reitern:
 *   Musik 0x19 (Lautstaerke ueber Sound_volume), Bilder 0x197/5 und /6 in zwei new Bmp, Held h0 =
 *   get_first_active_hero, Sprecher h (ein Krieger bzw. get_hero_* nach Typ bzw. h0), Dialogbaum mit dlg_box_bmp-
 *   Menues (txt[16..37]), Ergebnis res/fight/move: Zweikampf (hero_swap tauscht h/h0 vorher und nachher, Kampf 0x20,
 *   fight_result, Tagebuch ALL_SECT[119]) bzw. Gespraech 0x85 (30 Minuten, Tagebuch ALL_SECT[120]); Musik zurueck
 *   (0x23/0x16 je Musikart).
 * tevent_038/tevent_046: guide_offer(o, 4/3) je nach Ort 0x37/0x3d, bei g_guide_won route.event[38]/[46] = 0.
 * tevent_039: Frau am Wegesrand (Bild 0x1f, txt[62]), [39]--, Gespraech 0x86. tevent_041: Pflanzenkunde-Probe mit
 *   [41] * 3 + 5 -> 1 Wirselkraut (ALL_SECT[3]), [41]++. tevent_042: random_encounter(o, W6 + 4 bzw. W6 + 2 bei W10
 *   <= 5, 1/2 je nach Ort 0x3a). tevent_043: Kampf 3, bei Sieg [43] = 0. tevent_044: "Hoi Hoi" aus dem Gebuesch
 *   (txt[38]), Gespraech 0x87, [44]--. tevent_045: Bild 0x196/10 (nvfile_info + lokales Bmp), Held mit Stiefeln
 *   am Platz Fuesse: Stiefel zerfaellt (txt[39], drop_item), [45]--. tevent_048: Glockengeklingel - Orktrupp, zwei
 *   Fragen (dlg_choose), ggf. Kampf 0x32/0x31 (+ 0x9f, wenn verloren), Bild 0x2b mit txt[41]/[43], [48]--.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y -E..\bc\BIN\TASM.EXE OVL115.CPP   -> 4586/4586 Bytes GLEICH (18.10.2026),
 *   cobjfix laeuft. Ohne -Og 2 Bytes, ohne -Oi 40 Bytes zu kurz (strcpy-Intrinsic in tevent_048).
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - C++ wegen R42-Bildklasse (new Bmp / delete; lokales Bmp in tevent_045) und Nvf.
 * - Sprecherwahl als verschachtelte ifs mit getrennter Zuweisung ("h = f(); if (h == 0) {..}" -> cmp dword
 *   [bp-10h],0); eingebettet "(h = f()) == 0" ergaebe "or ax,dx".
 * - Menueergebnis "r = dlg_box_bmp(..); if (r == 1)" (char-Rueckgabe, cmp byte [bp-3]); Wiederholung als
 *   "r = -1; while (r == -1) r = ..". switch-Rumpfreihenfolge 1, 3, 2 (Sprungziele aufsteigend verglichen).
 * - "fight = move = 0" speichert move ([bp-6]) zuerst; tevent_048 "fr = res = 0" ebenso.
 * - tevent_042: random_schweif als signed char -> "add al,4" beim Speichern in signed char v.
 * Originalfehler: tevent_041 "if (g_routes[41].event <= 40) ; g_routes[41].event++;" (cmp / jg auf den naechsten
 *   Befehl) - der
 * Zaehler steigt ohne Grenze (O120 tevent_127 hat dieselbe Stelle richtig), nach 127 Aufrufen wird er negativ und
 * der Probenzuschlag [41] * 3 + 5 kippt.
 */
#include <stdio.h>
#include <string.h>

#include "schweif.h"

void tevent_037(Travel far *o)
{
	signed short w;
	signed char r;
	signed char res;
	signed char fight;
	signed char move;
	signed char x;
	struct hero far *h0;
	struct hero far *h;
	Bmp far *b1;
	Bmp far *b2;
	register signed short sw = 0;

	if (g_routes[37].event == 2) {
		snd_obj_stop(g_snd_music);
		snd_play(g_snd_music, 0x19);
		Sound_volume3(g_snd_music, g_vol_music, 500, 0);
		res = 2;
		fight = move = 0;
		h0 = get_first_active_hero(g_group);
		h = get_hero_of_type(g_group, HERO_TYPE_WARRIOR);
		if (h == 0) {
			h = get_random_hero_weapon_type(g_group, 3);
			if (h == 0) {
				h = get_random_hero_weapon_type(g_group, 6);
				if (h == 0) {
					h = get_random_hero_weapon_type(g_group, 4);
					if (h == 0) {
						h = get_random_hero_weapon_type(g_group, 1);
						if (h == 0)
							h = get_first_active_hero(g_group);
					}
				}
			}
		}
		b1 = new Bmp(0x40, 0x40);
		b2 = new Bmp(0x40, 0x40);
		nvfile_load(g_nvf_heads, 0x197, b1->data(), 0, 5, 0, &w, &w, 0x140);
		nvfile_load(g_nvf_heads, 0x197, b2->data(), 0, 6, 0, &w, &w, 0x140);
		/* SECTION0.LTX[16]: "Bereits von weitem seht Ihr eine grosse Staubfahne, die sich sehr schnell auf Euch zu
		 * bewegt. Wollt Ihr Euch verstecken?" */
		/* ALL_SECT.LTX[1]: "Ja" */
		/* ALL_SECT.LTX[2]: "Nein" */
		r = dlg_box_bmp(g_dialog, b1, (char far *)0, g_tev_loc_txt[16], 2, g_tev_txt[1], g_tev_txt[2]);
		if (r == 1) {
			/* SECTION0.LTX[17]: "Ohne die Umgebung mehr als eines fluechtigen Blickes zu wuerdigen, ziehen fuenf schwer
			 * gepanzerte Reiter an Euch vorbei. Die Gesichter koennt Ihr unter den Helmen nicht erkennen, aber ihre ganze
			 * Haltung drueckt eine schlaefrige, nur gewohnheitsmaessige Alarmbereitschaft aus. Diese Reiter sind sich Ihrer
			 * Staerke sehr sicher...Mit lautem Klirren ziehen sie an Euch vorueber und sind kurze Zeit spaeter in Ihrer
			 * Staubwolke nicht mehr zu sehen." */
			dlg_box_bmp(g_dialog, b1, (char far *)0, g_tev_loc_txt[17], 0);
			res = 4;
		} else {
			/* SECTION0.LTX[18]: "Unbeirrt und ohne von Euch Notiz zu nehmen, reiten die Krieger weiter. Sie sind schon auf
			 * wenige Schritt heran, und sie  machen keinerlei Anstalten, anzuhalten. Wollt Ihr beiseite springen?" */
			r = dlg_box_bmp(g_dialog, b1, (char far *)0, g_tev_loc_txt[18], 2, g_tev_txt[1], g_tev_txt[2]);
			if (r == 1) {
				/* SECTION0.LTX[19]: "Ihr werdet keines Blickes gewuerdigt, als die Gruppe Euch passiert. Wenige Minuten
				 * spaeter ist nur noch eine Staubwolke zu sehen." */
				dlg_box_bmp(g_dialog, b1, (char far *)0, g_tev_loc_txt[19], 0);
				res = 4;
			} else {
				/* SECTION0.LTX[20]: "Unmittelbar vor %s hebt der vorderste der Reiter eine Hand, und wie ein Mann bleiben die
				 * Reiter stehen. <Heda, hat %s Schwarzpelze gesehen hier in der Region?> kommt es etwas dumpf unter dem Helm
				 * hervor." */
				sprintf(g_text_output_buf, g_tev_loc_txt[20], h0->name, grammar_pronoun(h0->sex, 0, 0));
				r = -1;
				while (r == -1)
					/* SECTION0.LTX[21]: "<Etwas freundlicher, wenn ich bitten darf, Ja?>" */
					/* SECTION0.LTX[22]: "<Vielleicht, vielleicht nicht; kommt darauf an, wer fragt.>" */
					/* SECTION0.LTX[23]: "<Oh ja, hoher Herr, fuer Euch nur etwa 6 Stunden noerdlich, gleich hinter Gashok.>" */
					r = dlg_box_bmp(g_dialog, b2, (char far *)0, g_text_output_buf, 3, g_tev_loc_txt[21], g_tev_loc_txt[22], g_tev_loc_txt[23]);
				switch (r) {
				case 1:
					/* SECTION0.LTX[24]: "<%s darf nicht bitten; %s hat %s Zunge zu hueten. Also hat %s Schwarzpelze gesehen?>" */
					sprintf(g_text_output_buf, g_tev_loc_txt[24], grammar_pronoun(h0->sex, 0, 0), grammar_pronoun(h0->sex, 0, 0),
						grammar_pronoun(h0->sex, 1, 1), grammar_pronoun(h0->sex, 0, 0));
					r = -1;
					while (r == -1)
						/* SECTION0.LTX[27]: "<Vorsichtig, Maenneken!>" */
						/* SECTION0.LTX[28]: "<Nein.>" */
						r = dlg_box_bmp(g_dialog, b2, (char far *)0, g_text_output_buf, 3, g_tev_loc_txt[23], g_tev_loc_txt[27], g_tev_loc_txt[28]);
					switch (r) {
					case 1:
						/* SECTION0.LTX[26]: "<Danke. Antraben!>" */
						dlg_box_bmp(g_dialog, b1, (char far *)0, g_tev_loc_txt[26], 0);
						res = 3;
						break;
					case 2:
						fight = 1;
						break;
					case 3:
						dlg_box_bmp(g_dialog, b1, (char far *)0, g_tev_loc_txt[26], 0);
						res = 3;
						break;
					}
					break;
				case 3:
					dlg_box_bmp(g_dialog, b1, (char far *)0, g_tev_loc_txt[26], 0);
					res = 3;
					break;
				case 2:
					/* SECTION0.LTX[30]: "<Das ist natuerlich etwas anderes; verzeiht meine ungehobelte Zunge. Darf ich mich
					 * vorstellen: %s.>" */
					sprintf(g_text_buf2, g_tev_loc_txt[30], h->name);
					/* SECTION0.LTX[25]: "<Sei %s nicht so impertinent; %s redet mit dem Baron von Meresfeld und nicht mit einem
					 * %s Standes.>" */
					sprintf(g_text_output_buf, g_tev_loc_txt[25], grammar_pronoun(h->sex, 0, 0), grammar_pronoun(h->sex, 0, 0),
						grammar_pronoun(h->sex, 1, 1));
					r = -1;
					while (r == -1)
						/* SECTION0.LTX[29]: "<Oh. Aber wer soll das auch ahnen bei Eurer Aufmachung.>" */
						r = dlg_box_bmp(g_dialog, b2, (char far *)0, g_text_output_buf, 3, g_tev_loc_txt[27], g_tev_loc_txt[29], g_text_buf2);
					switch (r) {
					case 1:
					case 2:
						fight = 1;
						break;
					case 3:
						/* SECTION0.LTX[31]: "<Schon besser. Sagt %s mir nun, ob %s Orks gesehen hat?>" */
						sprintf(g_text_output_buf, g_tev_loc_txt[31], grammar_pronoun(h->sex, 0, 0), grammar_pronoun(h->sex, 0, 0));
						r = -1;
						while (r == -1)
							/* SECTION0.LTX[32]: "<Nein, bedaure; das haben wir nicht.>" */
							/* SECTION0.LTX[33]: "<Haben wir; aber die zu jagen lohnt es sich nicht mehr.>" */
							r = dlg_box_bmp(g_dialog, b2, (char far *)0, g_text_output_buf, 3, g_tev_loc_txt[32], g_tev_loc_txt[33],
								/* SECTION0.LTX[34]: "<Reitet nur nach Norden; dort sind sie zuhauf.>" */
								g_tev_loc_txt[34]);
						switch (r) {
						case 1:
							dlg_box_bmp(g_dialog, b1, (char far *)0, g_tev_loc_txt[26], 0);
							res = 3;
							break;
						case 3:
							dlg_box_bmp(g_dialog, b1, (char far *)0, g_tev_loc_txt[26], 0);
							res = 3;
							break;
						case 2:
							/* SECTION0.LTX[35]: "<Recht so; %s scheint mir tapferer zu sein, als %s wirkt.>" */
							sprintf(g_text_output_buf, g_tev_loc_txt[35], grammar_pronoun(h->sex, 0, 0),
								grammar_pronoun(h->sex, 0, 0));
							dlg_box_bmp(g_dialog, b2, (char far *)0, g_text_output_buf, 0);
							move = 1;
							res = 1;
							break;
						}
						break;
					}
					break;
				}
			}
		}
		g_routes[37].event = 0;
		if (fight == 1) {
			/* SECTION0.LTX[37]: "<%s fuehrt ein loses Maul; jedoch nicht mehr lange. Nehmt diesen Handschuh, Lump! Und der
			 * Rest halte sich heraus!>" */
			sprintf(g_text_output_buf, g_tev_loc_txt[37], grammar_pronoun(h->sex, 0, 0));
			dlg_box_bmp(g_dialog, b2, (char far *)0, g_text_output_buf, 0);
		}
		delete b1;
		delete b2;
		if (fight == 1) {
			if (h != h0) {
				sw = 1;
				hero_swap(g_group, h, h0);
			}
			travel_leave(o);
			g_fight_flag = 1;
			g_fight_end = 5;
			x = start_fight(0x20, 0);
			fight_result(o, x);
			travel_enter(o);
			if (sw)
				hero_swap(g_group, h, h0);
			/* ALL_SECT.LTX[119]: "Wir sind unterwegs dem Baron von Meresfeld begegnet. Offensichtlich hat %s seinen Zorn
			 * erregt, denn der Baron forderte %s zum Zweikampf." */
			sprintf(g_text_output_buf, g_tev_txt[119], h->name, grammar_pronoun(h->sex, 2, 0));
			diary_add(g_diary, g_text_output_buf, 0);
		}
		if (move == 1) {
			travel_leave(o);
			if (conv_run(g_dialog2, 0x85))
				timewarp_minutes(g_date, 30L);
			mouse_hide(0);
			travel_enter(o);
			mouse_show(0);
			travelmap_draw(o);
			travel_redraw(o);
			/* ALL_SECT.LTX[120]: "Wir sind unterwegs einem Baron von Meresfeld begegnet." */
			diary_add(g_diary, g_tev_txt[120], 0);
		}
		Sound_volume3(g_snd_music, 0, 500, 0);
		if (Sound_mtype(g_snd_music) == 1 || Sound_mtype(g_snd_music) == 2)
			snd_play(g_snd_music, 0x23);
		else
			snd_play(g_snd_music, 0x16);
		Sound_volume3(g_snd_music, g_vol_music, 500, 0);
	} else if (g_routes[37].event >= 6)
		g_routes[37].event = 1;
	else
		g_routes[37].event += 1;
}

void tevent_038(Travel far *o)
{
	if (o->from == 0x37)
		guide_offer(o, 4);
	else
		guide_offer(o, 3);
	if (g_guide_won != 0)
		g_routes[38].event = 0;
}

void tevent_039(Travel far *o)
{
	signed short w;
	Bmp far *b;

	b = new Bmp(0x40, 0x40);
	nvfile_load(g_nvf_heads, 0x197, b->data(), 0, 0x1f, 0, &w, &w, 0x140);
	/* SECTION0.LTX[62]: "Am Wegesrand sitzt eine Frau, die Ihr dabei ueberrascht, wie sie ausgiebig ihre Socken
	 * examiniert. Als sie Euch bemerkt, gruesst sie etwas aengstlich, aber keineswegs unhoeflich." */
	dlg_box_bmp(g_dialog, b, (char far *)0, g_tev_loc_txt[62], 0);
	delete b;
	g_routes[39].event -= 1;
	travel_leave(o);
	if (conv_run(g_dialog2, 0x86))
		timewarp_minutes(g_date, 30L);
	mouse_hide(0);
	travel_enter(o);
	mouse_show(0);
	travelmap_draw(o);
	travel_redraw(o);
}

void tevent_041(Travel far *o)
{
	struct hero far *h;
	signed short r;

	h = get_best_hero_skill(g_group, TA_PFLANZENKUNDE);
	r = test_skill(h, TA_PFLANZENKUNDE, g_routes[41].event * 3 + 5);
	if (r > 0) {
		/* ALL_SECT.LTX[3]: "%s entdeckt am Wegesrand %d Portion%s Wirselkraut!" */
		/* ALL_SECT.LTX[4]: "" */
		sprintf(g_text_output_buf, g_tev_txt[3], h->name, 1, g_tev_txt[4]);
		dlg_message(g_dialog, g_text_output_buf);
		group_give_item(g_group, ITEM_WHIRLWEED, 1);
	}
#if FIX_0101
	/* Fix #101: verirrtes Semikolon entfernt, Zaehler begrenzt */
	if (g_routes[41].event < 40)
		g_routes[41].event++;
#else
	if (g_routes[41].event <= 40)
		;
	g_routes[41].event++;
#endif
}

void tevent_042(Travel far *o)
{
	signed char v = 0;

	if (g_routes[42].event == 1) {
		g_routes[42].event += 1;
		v = random_schweif(6) + 4;
		if (o->from == 0x3a)
			random_encounter(o, v, 1);
		else
			random_encounter(o, v, 2);
	} else {
		v = random_schweif(10);
		if (v <= 5) {
			v = random_schweif(6) + 2;
			if (o->from == 0x3a)
				random_encounter(o, v, 1);
			else
				random_encounter(o, v, 2);
		}
	}
}

void tevent_043(Travel far *o)
{
	signed char r;

	travel_leave(o);
	g_fight_flag = 1;
	r = start_fight(3, 0);
	r = fight_result(o, r);
	travel_enter(o);
	if (r == 0)
		g_routes[43].event = 0;
}

void tevent_044(Travel far *o)
{
	/* SECTION0.LTX[38]: "<Hoi Hoi> brummt es unvermittelt aus einem Gebuesch am Wegesrand. <Bleibt stehen und zeigt
	 * mir Eure Gesichter!> Es knackt und bricht eine Weile, dann stapft ein staemmiger, stattlicher Zwerg von nahezu
	 * siebeneinhalb Spann hervor. Als er vor Euch steht, laesst er seine Armbrust sinken." */
	dlg_message(g_dialog, g_tev_loc_txt[38]);
	travel_leave(o);
	if (conv_run(g_dialog2, 0x87))
		timewarp_minutes(g_date, 30L);
	mouse_hide(0);
	travel_enter(o);
	mouse_show(0);
	travelmap_draw(o);
	travel_redraw(o);
	g_routes[44].event -= 1;
}

void tevent_045(Travel far *o)
{
	signed short w;
	struct hero far *h;
	signed short bw;
	signed short bh;

	nvfile_info(g_nvf_heads, 0x196, 10, &bw, &bh);
	Bmp b(bw, bh);
	nvfile_load(g_nvf_heads, 0x196, b.data(), 0, 10, 0, &w, &w, 0x140);
	if (group_count_check3(g_group) > 0)
		h = get_random_hero_with_item(g_group, ITEM_BOOTS, INV_SLOT_FEET);
	else
		h = 0;
	if (h != 0) {
		g_dlg_box_flag = 0;
		/* SECTION0.LTX[39]: "Mit einem leisen Fluch bleibt %s ploetzlich stehen und bueckt sich. Nach kurzem Hantieren
		 * wirft %s den linken Stiefel im hohen Bogen ins Gebuesch; noch waehrend des Fluges loest er sich in seine
		 * Bestandteile auf." */
		sprintf(g_text_output_buf, g_tev_loc_txt[39], h->name, grammar_pronoun(h->sex, 0, 0));
		dlg_box_bmp(g_dialog, &b, (char far *)0, g_text_output_buf, 0);
		drop_item(h, INV_SLOT_FEET, 1);
		g_routes[45].event -= 1;
	}
}

void tevent_046(Travel far *o)
{
	if (o->from == 0x3d)
		guide_offer(o, 4);
	else
		guide_offer(o, 3);
	if (g_guide_won != 0)
		g_routes[46].event = 0;
}

void tevent_048(Travel far *o)
{
	signed short w;
	struct hero far *h;
	signed char r;
	signed char fr;
	signed char res;
	Bmp far *b;

	fr = res = 0;
	h = get_best_hero_skill(g_group, TA_SINNESSCHAERFE);
	/* SECTION0.LTX[40]: "Ruckartig bleibt %s stehen, von vorne ist leises Geklingel hoerbar, das allmaehlich lauter
	 * wird. Je naeher es kommt, desto deutlicher ist es als das Klirren von Waffen und Ruestungen zu erkennen. Wollt
	 * Ihr Euch verstecken?" */
	sprintf(g_text_output_buf, g_tev_loc_txt[40], h->name);
	/* ALL_SECT.LTX[1]: "Ja" */
	/* ALL_SECT.LTX[2]: "Nein" */
	r = dlg_choose(g_dialog, g_text_output_buf, 2, g_tev_txt[1], g_tev_txt[2]);
	switch (r) {
	case 1:
		res = 1;
		break;
	case -1:
	case 2:
		/* SECTION0.LTX[42]: "Etwas spaeter koennt Ihr auch zahlreiche rauhe Stimmen hoeren, die sich in ungenierter
		 * Lautstaerke und orkischer Sprache unterhalten. Wollt Ihr Euch jetzt verstecken?" */
		r = dlg_choose(g_dialog, g_tev_loc_txt[42], 2, g_tev_txt[1], g_tev_txt[2]);
		switch (r) {
		case 1:
			res = 2;
			break;
		case -1:
		case 2:
			/* SECTION0.LTX[68]: "Da stuerzen schon die naechsten zwanzig Orks heran." */
			strcpy(g_text_buf2, g_tev_loc_txt[68]);
			travel_leave(o);
			g_fight_flag = 1;
			if (o->from == 0x3e)
				fr = start_fight(0x32, 1);
			else
				fr = start_fight(0x31, 1);
			fr = fight_result(o, fr);
			if (!fr) {
				dlg_message(g_dialog, g_text_buf2);
				g_fight_flag = 1;
				fr = start_fight(0x9f, 1);
				fr = fight_result(o, fr);
			}
			travel_enter(o);
			if (fr == 2)
				/* SECTION0.LTX[69]: "Was sollte das denn jetzt?" */
				dlg_message(g_dialog, g_tev_loc_txt[69]);
			break;
		}
		break;
	}
	if (res > 0) {
		b = new Bmp(0x40, 0x40);
		nvfile_load(g_nvf_heads, 0x197, b->data(), 0, 0x2b, 0, &w, &w, 0x140);
		if (res == 1)
			/* SECTION0.LTX[41]: "Einige Minuten spaeter seht Ihr einen Trupp Orks vorbeiziehen; die etwa dreissig
			 * Schwarzpelze unterhalten sich lautstark ueber etwas offensichtlich sehr lustiges. Dennoch bewegen sie sich in
			 * disziplinierter Marschordnung vorwaerts... Nach kurzer Zeit sind sie nur noch zu hoeren." */
			dlg_box_bmp(g_dialog, b, (char far *)0, g_tev_loc_txt[41], 0);
		else
			/* SECTION0.LTX[43]: "In der Tat sind es etwa dreissig Orks, die jetzt an Euch vorueberziehen. Nach kurzer Zeit
			 * sind sie ausser Sichtweite." */
			dlg_box_bmp(g_dialog, b, (char far *)0, g_tev_loc_txt[43], 0);
		delete b;
	}
	g_routes[48].event -= 1;
}
