/* OVL000 - Overlay O000 (Segment 0x1eec in SCHWEIF.EXE): Ereignisse des Dungeons 10, einer Zwergenbinge mit
 * Ingerimm-Heiligtum (Texte BINGE2.LTX = Datei 0x402; Verteiler in O060, g_area_descs[10]); Ereignisobjekt ev
 * (struct area_events, Texte ev->txt).
 * dng10_altar (Fall 0x6a, Steinaltar mit Muenzen, txt[47..49]): Traeger des Asthenilrings (nicht der NSC,
 * check_hero3) -> dng10_altar_vision, sonst Menue "Muenzen nehmen/opfern" -> dng10_altar_take (g_dng10_altar_coins
 * erbeuten: money_to_string, dng10_altar_rumble = Grollen im Berg: g_god_favor_ingerimm -= 1000, g_sacrilege = 1;
 * group_add_money, Altar leer) bzw. dng10_altar_offer (Silberstuecke opfern: dlg_input bis Gruppengeld / 10,
 * hoechstens 9999; g_god_favor_ingerimm += n, Altar += n * 10 Heller, group_sub_money, Klang 0x47).
 * dng10_altar_vision(ev, h): Held h faellt in Trance (Vision eines riesigen Schmieds, txt[50..63]) und wird von der
 * Gruppe getrennt (make_group(1, h), neue Gruppe in g_groups suchen), Bildfolgen 0x4d2..0x4d5 (new Anim / delete),
 * bis zu 3 Runden dng10_vision_choice der anderen Gruppe (Held ansprechen/beruehren, Muenzen nehmen/opfern; zwei
 * 40-Byte-Puffer new char[40]), Ergebnis 1..3 als switch, einmalig g_dng10_vision_ap -> add_hero_ap(h, 10).
 * dng10_anvil (Fall 0x6b): Hammer bewegen / Amboss untersuchen. dng10_relief (Fall 0x69 an 0xd13): Ingerimm-Relief -
 * Relief bzw. Runen untersuchen (Runendokument, sonst bester in Alte Sprachen), Fackeln entzuenden (Licht
 * ITEM_TORCH_ON/ITEM_LANTERN_ON/ITEM_TINDERBOX -> einmalig g_binge_torches_lit, g_god_favor_ingerimm += 100).
 * dng10_sharp_object (Felder 0x30e..0x70f): Held mit der schlechtesten GE-Probe tritt bei misslungener GE-Probe -3
 * auf etwas Spitzes (random_schweif(1) + 1 LE). dng10_tools/dng10_junk (Felder 0x30c/0x70c): verrostete Geraete
 * bzw. Krempel untersuchen (g_dng10_tools_found/g_dng10_junk_found), dann Ablage new Stash(0x24/0x25, 1).
 * dng10_countdown (Fall 0x6e, ohne Text): Zufallszaehler g_dng10_countdown (random_schweif(4) + 6), bei 0
 * group_add_ap, event_set_cell, automap_cell_write. dng10_rotten_chest (Fall 0x6c): verrottete Truhe, einmalig
 * g_dng10_chest_opened: Held mit Dietrichen (gehen ggf. verloren), Tellereisen: Schaden, Wundfieber-Probe,
 * add_effect(.., 32400L, 3, -4, 0); danach Ablage new Stash(0x26, 0) (zeigt sofort an).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL000.CPP   -> 5689/5689 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - C++ wegen new/delete Anim (Anim_ctor(0, ..)/Anim_dtor(a, 3)), Stash (Stash_ctor(0, ..)/Stash_dtor(s, 3)) und
 *   new char[40]/delete (mem_alloc_w/mem_free_v).
 * - "(signed short)g_group->nr_heroes > 1" -> mov al / cbw / cmp ax,1 (ohne Cast cmp byte).
 * - dng10_altar_vision: [bp-10h] = -Og-Zwischenwert fuer h->name (entsteht von selbst durch "h->name, h->name" in
 *   einem sprintf und wird dann bei jedem h->name gespeichert); pron im else-if-Block hinter dem Zwischenwert.
 * - dng10_altar_vision: Schleife "for (i = 0, g = g_groups; i < 6; i++, g = &g_groups[i])".
 * - dng10_tools/dng10_junk/dng10_rotten_chest: old und das Stash-Objekt in Geschwisterbloecken (teilen [bp-2] bzw.
 *   [bp-6]).
 * Originalfehler: dng10_altar_vision setzt nach "g_group = old" den Gruppenindex g_group_no = get_group_index(g) aus
 *   der FALSCHEN Gruppe (g statt old). dng10_rotten_chest sucht die Position der Dietriche bei h (get_item_pos(h,
 *   0x23)),
 *   entfernt sie aber bei h2 (drop_item(h2, i, 1)) - verschieden, wenn h2 nicht einsatzfaehig war.
 *   dng10_vision_choice: Auswahl 1 und 2 zeigen denselben Text txt[68]; "if (sel == 1)" ohne else vor
 *   "if (sel == 2)".
 */
#include <stdio.h>
#include "schweif.h"

void dng10_altar_vision(struct area_events far *ev, struct hero far *h);
signed short dng10_vision_choice(struct area_events far *ev, struct hero far *h, signed short round);
void dng10_altar_take(struct area_events far *ev);
void dng10_altar_offer(struct area_events far *ev);
void dng10_altar_rumble(struct area_events far *ev);

void dng10_altar(struct area_events far *ev)
{
	signed short old;
	signed short i;
	struct hero far *h;
	signed short r;

	old = dlg_set16(g_dialog, 5);
	/* BINGE2.LTX[47]: "Der Altar ist direkt aus dem Fels gehauen, und daher fest mit der Umgebung verbunden. Links
	 * und rechts sind flache Mulden in die Oberflaeche gemeisselt; in der rechten liegen ein paar Silber- und
	 * Goldmuenzen, die linke ist leer." */
	dlg_message(g_dialog, ev->txt[47]);
	i = group_find_item(g_group, ITEM_ASTHENIL_RING);
	if (i != -1) {
		h = g_group->heroes[i];
		if (group_get_npc(g_group) != h && check_hero3(h)) {
			dlg_set16(g_dialog, 7);
			dng10_altar_vision(ev, h);
			dlg_set16(g_dialog, 5);
		}
	} else {
		/* BINGE2.LTX[0]: "Wollt Ihr" */
		/* BINGE2.LTX[48]: "Muenzen nehmen" */
		/* BINGE2.LTX[49]: "Muenzen opfern" */
		r = dlg_choose(g_dialog, ev->txt[0], 2, ev->txt[48], ev->txt[49]);
		if (r == 1)
			dng10_altar_take(ev);
		else if (r == 2)
			dng10_altar_offer(ev);
	}
	dlg_set16(g_dialog, old);
}

void dng10_altar_vision(struct area_events far *ev, struct hero far *h)
{
	signed short res = 0;
	signed short tries = 0;
	struct group far *old = g_group;
	struct group far *g = g_group;
	signed short i;
	Anim far *a;

	if ((signed short)g_group->nr_heroes > 1) {
		make_group(1, h);
		for (i = 0, g = g_groups; i < 6; i++, g = &g_groups[i]) {
			if (group_has_hero(g, h)) {
				g_group = g;
				g_group_no = i;
				break;
			}
		}
		screen_redraw(g_screen);
	}
	/* BINGE2.LTX[50]: "<F1>%s wird es schwarz vor den Augen, dann scheint %s sich in einem voellig anderen Raum zu
	 * befinden, einer endlosen Halle.<F0>" */
	sprintf(g_text_output_buf, ev->txt[50], h->name, grammar_pronoun(h->sex, 0, 0));
	dlg_message(g_dialog, g_text_output_buf);
	a = new Anim(0x4d2, 0, 0);
	/* BINGE2.LTX[51]: "<F2>Eine riesige Gestalt mit einem Schmiedehammer steht vor einem Amboss und winkt %s
	 * heran.<F0>" */
	sprintf(g_text_output_buf, ev->txt[51], grammar_pronoun(h->sex, 2, 0));
	dlg_box_anim(g_dialog, a, (char far *)0, g_text_output_buf, 0);
	delete a;
	a = new Anim(0x4d3, 0, 0);
	/* BINGE2.LTX[52]: "<F2>Als %s naeher tritt, erhaelt %s einen Schmiedehammer und gemeinsam mit dem riesenhaften
	 * Schmied bearbeitet %s ein gluehendes Stueck Eisen.<F0>" */
	sprintf(g_text_output_buf, ev->txt[52], h->name, grammar_pronoun(h->sex, 0, 0), grammar_pronoun(h->sex, 0, 0));
	dlg_box_anim(g_dialog, a, (char far *)0, g_text_output_buf, 0);
	delete a;
	/* BINGE2.LTX[53]: "<F2>Zunaechst faellt %s die Arbeit schwer, doch nach und nach geht es gut von der Hand.<F0>" */
	sprintf(g_text_output_buf, ev->txt[53], grammar_pronoun(h->sex, 3, 0));
	dlg_message(g_dialog, g_text_output_buf);
	do {
		if (g_group != g) {
			g_group = g;
			g_group_no = get_group_index(g);
			screen_redraw(g_screen);
		}
		a = new Anim(0x4d4, 0, 0);
		/* BINGE2.LTX[54]: "<F2>Der Schmied begutachtet die Arbeit, ein Schwert aus dunkelrot schimmerndem Metall.<F0>" */
		dlg_box_anim(g_dialog, a, (char far *)0, ev->txt[54], 0);
		delete a;
		a = new Anim(0x4d5, 0, 0);
		/* BINGE2.LTX[55]: "<F2>Schliesslich wirft er das Schwert in die Esse zurueck bis es geschmolzen ist, dann holt
		 * er das gluehende Eisen aus dem Feuer und beginnt erneut, es zu bearbeiten.<F0>" */
		dlg_box_anim(g_dialog, a, (char far *)0, ev->txt[55], 0);
		delete a;
		a = new Anim(0x4d3, 0, 0);
		/* BINGE2.LTX[56]: "<F2>Der Schmied bedeutet %s, mitzuhelfen. Gemeinsam bearbeiten sie das Eisenstueck.<F0>" */
		sprintf(g_text_output_buf, ev->txt[56], h->name);
		dlg_box_anim(g_dialog, a, (char far *)0, g_text_output_buf, 0);
		delete a;
		if (old == g) {
			tries++;
		} else {
#if FIX_0160
			/* Fix #160: Gruppenindex der wieder aktiven Gruppe old */
			g_group = old;
			g_group_no = get_group_index(old);
#else
			g_group = old;
			g_group_no = get_group_index(g);
#endif
			screen_redraw(g_screen);
			res = dng10_vision_choice(ev, h, tries);
			if (tries == 0)
				tries++;
		}
	} while (res == 0 && tries < 3);
	if (old == g) {
		/* BINGE2.LTX[57]: "<F2>%s scheint in einem Alptraum gefangen zu sein. Ewig wiederholt sich das seltsame
		 * Geschehen, immer wieder winkt die riesige Gestalt mit dem Schmiedehammer, wohl tausende von malen sieht %s das
		 * Schwert entstehen und wieder vergehen, bis es %s irgenwann schwarz vor den Augen wird.<F0>" */
		sprintf(g_text_output_buf, ev->txt[57], h->name, h->name, grammar_pronoun(h->sex, 3, 0));
		dlg_message(g_dialog, g_text_output_buf);
		g_resting = 1;
		timewarp_hours(g_date, 30L);
		g_resting = 0;
		/* BINGE2.LTX[58]: "<F1>Als %s wieder zu sich kommt, liegt %s vor dem Steinaltar.<F0>" */
		sprintf(g_text_output_buf, ev->txt[58], h->name, grammar_pronoun(h->sex, 0, 0));
		dlg_message(g_dialog, g_text_output_buf);
	} else if (res != 0) {
		char far *pron;

		if (old != g) {
			g_group = g;
			g_group_no = get_group_index(g);
			screen_redraw(g_screen);
		}
		pron = grammar_pronoun(h->sex, 0, 0);
		switch (res) {
		case 1:
			/* BINGE2.LTX[59]: "<F2><Du wirst gerufen>, sagt der Schmied ploetzlich.<F0>" */
			dlg_message(g_dialog, ev->txt[59]);
			/* BINGE2.LTX[60]: "<F1>%s wird es schwarz vor den Augen, und als %s die Umwelt wieder wahrnehmen kann, bemerkt
			 * %s, dass %s vor einem Steinaltar steht.<F0>" */
			sprintf(g_text_output_buf, ev->txt[60], h->name, pron, pron, pron);
			dlg_message(g_dialog, g_text_output_buf);
			break;
		case 2:
			/* BINGE2.LTX[61]: "<F2>Irgend jemand greift ploetzlich von hinten nach %s.<F0>" */
			sprintf(g_text_output_buf, ev->txt[61], h->name);
			dlg_message(g_dialog, g_text_output_buf);
			/* BINGE2.LTX[62]: "<F1>Rasch dreht %s sich um, und starrt fassungslos einem der Freunde ins Gesicht. Als %s
			 * sich wieder dem Amboss zuwenden will, steht an dessen Stelle ein Steinaltar.<F0>" */
			sprintf(g_text_output_buf, ev->txt[62], pron, pron);
			dlg_message(g_dialog, g_text_output_buf);
			break;
		case 3:
			/* BINGE2.LTX[63]: "<F1>%s wird ploetzlich jaeh aus der Trance gerissen. Die Erde bebt und es droehnt in %s
			 * Ohren.<F0>" */
			sprintf(g_text_output_buf, ev->txt[63], h->name, grammar_pronoun(h->sex, 3, 1));
			dlg_message(g_dialog, g_text_output_buf);
			break;
		}
	}
	if (!g_dng10_vision_ap) {
		g_dng10_vision_ap = 1;
		add_hero_ap(h, 10);
	}
	if (old != g)
		groups_merge();
}

signed short dng10_vision_choice(struct area_events far *ev, struct hero far *h, signed short round)
{
	char far *opt1 = new char[40];
	char far *opt2 = new char[40];
	signed short ret = 0;
	signed short sel;

	if (round == 0)
		/* BINGE2.LTX[64]: "%s ist vor den Altar getreten und verharrt dort in einer eigenartigen Starre. Die eingelegten
		 * Metallstuecke und die Sockelleiste des Altars gluehen schwach. Wollt Ihr" */
		sprintf(g_text_output_buf, ev->txt[64], h->name);
	else
		/* BINGE2.LTX[65]: "Ihr steht vor dem Altar und beobachtet %s. Wollt Ihr" */
		sprintf(g_text_output_buf, ev->txt[65], h->name);
	/* BINGE2.LTX[66]: "%s ansprechen." */
	sprintf(opt1, ev->txt[66], h->name);
	/* BINGE2.LTX[67]: "%s beruehren." */
	sprintf(opt2, ev->txt[67], h->name);
	/* BINGE2.LTX[48]: "Muenzen nehmen" */
	/* BINGE2.LTX[49]: "Muenzen opfern" */
	sel = dlg_choose(g_dialog, g_text_output_buf, 4, opt1, opt2, ev->txt[48], ev->txt[49]);
	if (sel == 1) {
		/* BINGE2.LTX[68]: "%s reagiert nicht." */
		sprintf(g_text_output_buf, ev->txt[68], h->name);
		ret = 1;
		dlg_message(g_dialog, g_text_output_buf);
	}
	if (sel == 2) {
		sprintf(g_text_output_buf, ev->txt[68], h->name);
		ret = 2;
		dlg_message(g_dialog, g_text_output_buf);
	} else if (sel == 3) {
		dng10_altar_take(ev);
		ret = 3;
	} else if (sel == 4) {
		dng10_altar_offer(ev);
	}
	delete opt2;
	delete opt1;
	return ret;
}

void dng10_altar_take(struct area_events far *ev)
{
	timewarp_minutes(g_date, 5L);
	if (g_dng10_altar_coins != 0) {
		money_to_string(g_text_buf2, g_dng10_altar_coins);
		/* BINGE2.LTX[2]: "Ihr erbeutet %s." */
		sprintf(g_text_output_buf, ev->txt[2], g_text_buf2);
		dlg_message(g_dialog, g_text_output_buf);
		dng10_altar_rumble(ev);
		group_add_money(g_group, g_dng10_altar_coins);
		g_dng10_altar_coins = 0;
		return;
	}
	/* BINGE2.LTX[3]: "Hier gibt es nichts mehr zu holen!" */
	dlg_message(g_dialog, ev->txt[3]);
}

void dng10_altar_offer(struct area_events far *ev)
{
	signed long m = g_group->money;
	signed short n;

	timewarp_minutes(g_date, 5L);
	if (g_group->money == 0) {
		/* BINGE2.LTX[5]: "Ihr habt doch gar kein Geld mehr!" */
		dlg_message(g_dialog, ev->txt[5]);
		return;
	}
	n = m / 10;
	if (n > 9999)
		n = 9999;
	/* BINGE2.LTX[4]: "Wieviele Silberstuecke wollt Ihr opfern?" */
	m = dlg_input(g_dialog, 4, ev->txt[4], 0, n, n < 100 ? n : 100);
	if (m > 0) {
		g_god_favor_ingerimm += m;
		m *= 10;
		g_dng10_altar_coins += m;
		group_sub_money(g_group, m);
		snd_play(g_snd_fx, 0x47);
	}
}

void dng10_altar_rumble(struct area_events far *ev)
{
	/* BINGE2.LTX[1]: "Ein dumpfes Grollen geht durch den Berg. Einen Moment lang bebt die Erde, dann ist es wieder
	 * still." */
	dlg_message(g_dialog, ev->txt[1]);
	g_god_favor_ingerimm -= 1000;
	g_sacrilege = 1;
}

void dng10_anvil(struct area_events far *ev)
{
	signed short old;
	signed short r;

	old = dlg_set16(g_dialog, 5);
	/* BINGE2.LTX[0]: "Wollt Ihr" */
	/* BINGE2.LTX[69]: "den Hammer bewegen?" */
	/* BINGE2.LTX[70]: "oder den Amboss untersuchen?" */
	r = dlg_choose(g_dialog, ev->txt[0], 2, ev->txt[69], ev->txt[70]);
	if (r == 1) {
		timewarp_minutes(g_date, 5L);
		/* BINGE2.LTX[71]: "Der Hammer laesst sich selbst mit Euren vereinten Kraeften nicht in Bewegung setzen." */
		dlg_message(g_dialog, ev->txt[71]);
	} else if (r == 2) {
		timewarp_minutes(g_date, 5L);
		/* BINGE2.LTX[72]: "Abgesehen von seiner enormen Groesse, koennt Ihr an dem Amboss nichts besonderes erkennen." */
		dlg_message(g_dialog, ev->txt[72]);
	}
	dlg_set16(g_dialog, old);
}

void dng10_relief(struct area_events far *ev)
{
	struct hero far *h;
	signed short old;
	signed short done;
	signed short r;
	signed short i;

	old = dlg_set16(g_dialog, 5);
	done = 0;
	while (done == 0) {
		/* BINGE2.LTX[73]: "Das Relief ist ein riesiges, in Stein gehauenes Bildnis des Schmiedegottes Ingerimm. Die
		 * Konturen sind grob strukturiert aber praezise gearbeitet. In der Mitte des Reliefs ist eine Runeninschrift in
		 * das Bildnis eingearbeitet. Zu beiden Seiten des Reliefs sind in verschiedenen Hoehen Fackelhalter in den Fels
		 * gemeisselt, in denen Pechfackeln stecken.@Wollt Ihr" */
		/* BINGE2.LTX[74]: "das Relief untersuchen?" */
		/* BINGE2.LTX[75]: "die Runen untersuchen?" */
		/* BINGE2.LTX[76]: "oder die Fackeln entzuenden?" */
		r = dlg_choose(g_dialog, ev->txt[73], 3, ev->txt[74], ev->txt[75], ev->txt[76]);
		if (r == 1) {
			timewarp_minutes(g_date, 5L);
			/* BINGE2.LTX[77]: "Bei genauerer Betrachtung zeigt sich, dass selbst die glatt erscheinenden Flaechen aeusserst
			 * fein strukturiert sind, als seien sie aus dem Stein gewoben und nicht gemeisselt worden." */
			dlg_message(g_dialog, ev->txt[77]);
		} else if (r == 2) {
			timewarp_minutes(g_date, 5L);
			i = group_find_item(g_group, ITEM_RUNE_DOCUMENT);
			if (i == -1) {
				h = get_best_hero_skill(g_group, TA_ALTE_SPRACHEN);
				/* BINGE2.LTX[78]: "Selbst %s, %s sich von Euch am besten mit Schriftzeichen auskennt, kann die Runeninschrift
				 * nicht entziffern." */
				sprintf(g_text_output_buf, ev->txt[78], h->name, grammar_def_article(h->sex, 0));
			} else {
				h = g_group->heroes[i];
				if (!check_hero3(h))
					h = get_first_active_hero(g_group);
				/* BINGE2.LTX[79]: "Die Runen erinnern Euch an die Schriftzeichen auf dem Pergament, das Ihr zuvor bei dem
				 * Skelett gefunden habt.@%s kramt umstaendlich das Pergament hervor - tatsaechlich, die Runen auf dem
				 * Pergament und an der Wand sind identisch." */
				sprintf(g_text_output_buf, ev->txt[79], h->name);
			}
			dlg_message(g_dialog, g_text_output_buf);
		} else if (r == 3) {
			if (!g_binge_torches_lit) {
				if (group_find_item(g_group, ITEM_TORCH_ON) < 0 && group_find_item(g_group, ITEM_LANTERN_ON) < 0 &&
				    group_find_item(g_group, ITEM_TINDERBOX) < 0) {
					/* BINGE2.LTX[80]: "Ihr habt nichts dabei, womit Ihr die Fackeln anzuenden koenntet." */
					dlg_message(g_dialog, ev->txt[80]);
				} else {
					/* BINGE2.LTX[81]: "Als Ihr eine der Fackeln ansteckt, lodern ploetzlich die uebrigen Fackeln auch auf. Ihr
					 * bemerkt, dass ein mattorangefarbener Schimmer auf den Augen des Ingerimmbildnisses entsteht, dessen
					 * Intensitaet beinahe unmerklich zunimmt, bis Euch der Schmiedegott schliesslich aus lavafunkelnden Augen
					 * anblickt. Irgendwo aus der Tiefe ist leise ein von rasselnden Geraeuschen begleitetes Rumpeln zu
					 * vernehmen." */
					dlg_message(g_dialog, ev->txt[81]);
					g_binge_torches_lit = 1;
					g_god_favor_ingerimm += 100;
				}
			} else {
				/* BINGE2.LTX[82]: "Die Fackeln brennen bereits." */
				dlg_message(g_dialog, ev->txt[82]);
			}
			timewarp_minutes(g_date, 5L);
		} else
			done = 1;
	}
	dlg_set16(g_dialog, old);
}

void dng10_sharp_object(struct area_events far *ev)
{
	struct hero far *h;
	struct hero far *worst;
	signed short old;
	signed short v;
	signed short min;

	h = get_first_hero(g_group);
	worst = 0;
	min = 0;
	v = 0;
	do {
		if (check_hero3(h)) {
			v = test_attrib(h, ATTRIB_GE, 0);
			if (v < min) {
				min = v;
				worst = h;
			}
		}
		h = get_next_hero(g_group, h);
	} while (get_first_hero(g_group) != h);
	if (worst != 0 && test_attrib(worst, ATTRIB_GE, -3) <= 0) {
		old = dlg_set16(g_dialog, 5);
		/* BINGE2.LTX[83]: "%s ist auf irgendetwas spitzes getreten. %s Fuss schmerzt!" */
		sprintf(g_text_output_buf, ev->txt[83], worst->name, grammar_pronoun(worst->sex, 1, 0));
		dlg_message(g_dialog, g_text_output_buf);
		sub_hero_le(worst, random_schweif(1) + 1);
		screen_portraits(g_screen);
		h = get_first_active_hero(g_group);
		if (h == worst && get_second_active_hero(g_group))
			h = get_second_active_hero(g_group);
		timewarp_minutes(g_date, 5L);
		/* BINGE2.LTX[84]: "%s untersucht die Wunde. Halb so schlimm, es blutet nicht einmal." */
		sprintf(g_text_output_buf, ev->txt[84], h->name);
		dlg_message(g_dialog, g_text_output_buf);
		dlg_set16(g_dialog, old);
	}
}

void dng10_tools(struct area_events far *ev)
{
	if (!g_dng10_tools_found) {
		signed short old = dlg_set16(g_dialog, 5);

		/* BINGE2.LTX[85]: "In der Ecke liegen verrostete Geraete und Werkzeuge. Wollt Ihr die Geraete untersuchen?" */
		if (dlg_yes_no(g_dialog, ev->txt[85]) == 1) {
			timewarp_minutes(g_date, 5L);
			g_dng10_tools_found = 1;
		}
		dlg_set16(g_dialog, old);
	}
	if (g_dng10_tools_found != 0) {
		Stash far *s = new Stash(0x24, 1);

		if (s->has_items() == 1)
			s->show();
		delete s;
	}
}

void dng10_junk(struct area_events far *ev)
{
	if (!g_dng10_junk_found) {
		signed short old = dlg_set16(g_dialog, 5);

		/* BINGE2.LTX[86]: "An der Wand stehen vergammelte Spitzhacken und Schaufeln, auf dem Boden liegt allerhand
		 * verrosteter Krempel. Einiges von dem Zeug macht den Eindruck, als waere es noch zu etwas zu gebrauchen. Wollt
		 * Ihr Euch den Krempel genauer anschauen?" */
		if (dlg_yes_no(g_dialog, ev->txt[86]) == 1) {
			timewarp_minutes(g_date, 5L);
			g_dng10_junk_found = 1;
		}
		dlg_set16(g_dialog, old);
	}
	if (g_dng10_junk_found != 0) {
		Stash far *s = new Stash(0x25, 1);

		if (s->has_items() == 1)
			s->show();
		delete s;
	}
}

void dng10_countdown(struct area_events far *ev)
{
	if (g_dng10_countdown == 0)
		g_dng10_countdown = random_schweif(4) + 6;
	if (g_dng10_countdown != -1) {
		g_dng10_countdown--;
		if (g_dng10_countdown == 0) {
			g_dng10_countdown = -1;
			group_add_ap(g_group, group_count_alive(g_group) * 3);
			event_set_cell(ev, 5, 9, 0, 0, -1);
			automap_cell_write(g_map, 0x2fe, 9, 5, 2);
			automap_cell_write(g_map, 0x2fe, 9, 6, 5);
		}
	}
}

void dng10_rotten_chest(struct area_events far *ev)
{
	signed short old;
	signed short i;

	old = dlg_set16(g_dialog, 5);
	/* BINGE2.LTX[87]: "An der stabil gearbeiteten Truhe hat der Zahn der Zeit kraeftig genagt: das Holz ist ziemlich
	 * verrottet, nur das eingelassene Schloss macht noch einen halbwegs stabilen Eindruck. Wollt Ihr die Truhe
	 * oeffnen?" */
	if (!g_dng10_chest_opened && dlg_yes_no(g_dialog, ev->txt[87]) == 1) {
		struct hero far *h;
		struct hero far *h2;

		h = get_first_active_hero(g_group);
		h2 = get_first_active_hero(g_group);
		i = group_find_item(g_group, ITEM_PICKLOCKS);
		if (i != -1)
			h2 = g_group->heroes[i];
		if (check_hero3(h2))
			h = h2;
		else
			h = get_first_active_hero(g_group);
		i = get_item_pos(h, ITEM_PICKLOCKS);
		if (i <= 0) {
			/* BINGE2.LTX[88]: "%s betrachtet sich das Schloss genauer, dann klopft %s probeweise auf den Truhendeckel:
			 * Scheppernd kracht das Schloss auf den Boden und die Truhe ist offen!" */
			sprintf(g_text_output_buf, ev->txt[88], h->name, grammar_pronoun(h->sex, 0, 0));
		} else {
			/* BINGE2.LTX[89]: "Mit einem lauten Knarren verkanntet sich das Werkzeug im Schloss. Als %s fluchend daran
			 * ruettelt, haelt %s ploetzlich das Schloss mitsamt Buegel in den Haenden. Die Dietriche sind wohl verloren,
			 * aber wenigstens ist die Truhe offen!" */
			sprintf(g_text_output_buf, ev->txt[89], h->name, grammar_pronoun(h->sex, 0, 0));
#if FIX_0160
			/* Fix #160: Dietriche dort entfernen, wo sie gefunden wurden (h) */
			drop_item(h, i, 1);
#else
			drop_item(h2, i, 1);
#endif
		}
		dlg_message(g_dialog, g_text_output_buf);
		timewarp_minutes(g_date, 5L);
		/* BINGE2.LTX[90]: "Die Truhe ist ueber und ueber mit verrotteten Lumpen und verrostetem Plunder gefuellt.
		 * Ploetzlich schreit %s laut auf und zieht %s Hand ruckartig aus der Truhe. Ein rostiges Tellereisen umschliesst
		 * das Handgelenk. Nachdem Ihr es vorsichtig entfernt habt, beginnt die Hand anzuschwillen." */
		sprintf(g_text_output_buf, ev->txt[90], h->name, grammar_pronoun(h->sex, 1, 1));
		dlg_message(g_dialog, g_text_output_buf);
		sub_hero_le(h, random_schweif(2) + 2);
		hero_disease_test(h, ILLNESS_WUNDFIEBER, 10);
		add_effect(h, get_free_effect(h), 32400L, 3, -4, 0);
		timewarp_minutes(g_date, 5L);
		screen_portraits(g_screen);
		/* BINGE2.LTX[91]: "Verbissen beginnt %s erneut in der Truhe zu wuehlen." */
		sprintf(g_text_output_buf, ev->txt[91], h->name);
		dlg_message(g_dialog, g_text_output_buf);
		g_dng10_chest_opened = 1;
	}
	if (g_dng10_chest_opened != 0) {
		Stash far *s = new Stash(0x26, 0);

		delete s;
	}
	dlg_set16(g_dialog, old);
}
