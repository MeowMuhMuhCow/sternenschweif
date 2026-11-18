/* OVL265 - Overlay O265 (Segment 0x24ca in SCHWEIF.EXE): Ereignisse des Dungeons 7, der zweiten Ebene der Orkfestung
 * (Orkgefaengnis; Texte BLUT2.LTX = Datei 0x374 ueber ev->txt); Unterfunktionen von dng07_handler (O264).
 * dng07_door(ev, pos, x, y, mev): Schloesser (Lock 0x28..0x49 je Position, Wandbild 0x70..0x73); bei g_lock_cell_bits
 *   != -1 Feldbits, Automap, Klang 0x61; hinter Lock 0x2c bzw. 0x2e einmal eine Meute Kampfhunde (BLUT2[9], Kampf
 *   0x6c mit 4 Hunden bei Tag, 8 bei Nacht).
 * dng07_shelves(ev, pos): Regale und Truhen (Ablagen 0x14..0x1d, Frage [1] bzw. [113]/[114]); an 0x1015 das
 *   Ruestungsregal: Sinnesschaerfe +6 ([13]), verschieben ([14]/[15], 5 Minuten), Sinnesschaerfe +4 -> Loch ueber der
 *   Tuer ([16], sonst [17]); mit Speer, Pike oder Kampfstab stochert der Fingerfertigste (FF-Probe -4) - die Wand
 *   (0x15/0x16, 0x10) oeffnet sich ([18]), sonst [19]; ohne Stange [20].
 * dng07_gold_statue: vergoldete Brazoragh-Statue ([3]); der Goldgierigste schabt einmal das Gold ab ([4], 10 Minuten,
 *   3 Goldschmuck).
 * dng07_straw(ev, h, pos): Strohlager ([11], 5 Minuten): an 0x130a/0x130f eingesaut ([12], Wirkung, Sperre
 *   INGAME_TIMER_DNG07_DIRT), an 0x150a Schnapsflasche ([112]), an 0x111 Silberamulett ([59]).
 * dng07_graffiti: zufaellige Kritzelei [60..80] in [81].
 * dng07_web(ev, pos): Spinnennetz - zwei Helden mit geeigneter Waffe (hero_weapon_type2 4/3/2/6) zerteilen es ([84],
 *   Wartezeit 50 - (KK beider + 1) Minuten), Feld (x, y) frei, Wandbild nach automap_cell_read 0x74/0x75, Merker
 *   g_dng07_web_<pos>; an einigen Netzen Hoehlenspinnen (0x26/0x27, Merker g_dng07_spiders_<pos>); ungeeignete Waffen
 *   [86]; ein Held allein verheddert sich ([85]) und verliert alle LE.
 * dng07_web_corpse: Skelett hinter dem Netz ([87]): misslingt mindestens der halben Gruppe die Totenangst-Probe [88],
 *   sonst wie dng07_web - Schwert ([89]), Feld (0x1b, 1), Hoehlenspinnen 3 + 4.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y -E..\bc\BIN\TASM.EXE OVL265.CPP -> 4610/4610 Bytes GLEICH (18.10.2026), cobjfix
 *   laeuft. Ohne -Og 6 Bytes (Epilogkopien), ohne -Oi 34 Bytes zu kurz (strcat-Intrinsic in dng07_straw).
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - C++ wegen Lock (lokal, "{ Lock lk(idx); ret = 1; }" -> Konstruktor/Destruktor direkt hintereinander) und Stash
 *   (new/delete). Zwei Sprungtabellen in dng07_door (0x207..0x210, 0x40b..0x415), Rest Binaersuche.
 * - automap_cell_write ueber AUTOMAP_CELL_WRITE_W; Tageszeit ueber DATE->ticks (Fixup auf g_date + 1 wie im Original
 *   - g_daytime ergaebe ein anderes Fixup-Ziel); Sperre als g_ingame_timers[INGAME_TIMER_DNG07_DIRT].
 * - dng07_door: d/idx als register (DI/SI), ret im Speicher; dng07_shelves: msg (txt[1], mov ax/mov dx) auf
 *   Funktionsebene, a/b/c/nm im Fall 0x1015 und die Liste im Block "if (idx != -1)" teilen Stackplaetze.
 * - dng07_shelves: "a = b = c = -1" (c zuerst), "(a = group_find_item(..)) != -1 || .." (char, cmp al,0ffh).
 * - dng07_web: "v1 = v2 = 0" bzw. "= 2" (v2 im Speicher zuerst, dann DI); KK-Summen (signed char)(attrib[KK].cur +
 *   attrib[KK].mod) (add al / cbw); timewarp_minutes mit (signed long)d -> movsx eax.
 * - dng07_graffiti: "ev->txt[r + 60]" -> mov dx,es:[bx+6] / add ax,es:[bx+4] / [bx+0f0h] (Index in die Verschiebung
 *   gefaltet).
 * Originalfehler: dng07_web schreibt "*f1 = 1" auch, wenn pos in keinem case steht (f1 = 0 -> Schreiben nach
 *   0000:0000). dng07_web: Wartezeit 50 - (..) kann negativ werden.
 */
#include <stdio.h>
#include <string.h>

#include "schweif.h"

/* automap_cell_write mit short als letztem Parameter (wie im Original deklariert) */
#define AUTOMAP_CELL_WRITE_W \
	((void (*)(void far *, signed short, signed short, signed short, signed short))automap_cell_write)
/* Tageszeit ueber g_date + 1 wie im Original (Fixup auf g_date, nicht g_daytime) */
#define DATE ((struct game_date far *)g_date)

signed short dng07_door(struct area_events far *ev, signed short pos, signed short x, signed short y,
	struct map_event far *mev)
{
	register signed short d = 0;
	signed short ret = 0;
	register signed short idx = -1;

	switch (pos) {
	case 0x1a10: idx = 0x28; d = 0x70; break;
	case 0x1805: idx = 0x29; d = 0x70; break;
	case 0x1706: idx = 0x2a; d = 0x71; break;
	case 0x1303: idx = 0x2b; d = 0x71; break;
	case 0x0a06: idx = 0x2c; d = 0x70; break;
	case 0x0d06: idx = 0x2d; d = 0x70; break;
	case 0x1006: idx = 0x2e; d = 0x70; break;
	case 0x100c: idx = 0x2f; d = 0x70; break;
	case 0x140c: idx = 0x30; d = 0x70; break;
	case 0x120e: idx = 0x31; d = 0x70; break;
	case 0x0e0d: idx = 0x32; d = 0x71; break;
	case 0x0c0a: idx = 0x33; d = 0x71; break;
	case 0x0a0e: idx = 0x34; d = 0x70; break;
	case 0x0c0f: idx = 0x35; d = 0x71; break;
	case 0x0c13: idx = 0x36; d = 0x71; break;
	case 0x0e14: idx = 0x37; d = 0x71; break;
	case 0x1718: idx = 0x38; d = 0x70; break;
	case 0x0c19: idx = 0x39; d = 0x71; break;
	case 0x0d1c: idx = 0x3a; d = 0x70; break;
	case 0x071a: idx = 0x3b; d = 0x70; break;
	case 0x061b: idx = 0x3c; d = 0x71; break;
	case 0x0318: idx = 0x3d; d = 0x72; break;
	case 0x0216: idx = 0x3e; d = 0x73; break;
	case 0x0213: idx = 0x3f; d = 0x73; break;
	case 0x0210: idx = 0x40; d = 0x73; break;
	case 0x020d: idx = 0x41; d = 0x73; break;
	case 0x020a: idx = 0x42; d = 0x73; break;
	case 0x0207: idx = 0x43; d = 0x73; break;
	case 0x0306: idx = 0x44; d = 0x72; break;
	case 0x0407: idx = 0x45; d = 0x73; break;
	case 0x040b: idx = 0x46; d = 0x73; break;
	case 0x040f: idx = 0x47; d = 0x71; break;
	case 0x0411: idx = 0x48; d = 0x73; break;
	case 0x0415: idx = 0x49; d = 0x73; break;
	}
	if (idx != -1) {
		{
			Lock lk(idx);
			ret = 1;
		}
		if (g_lock_cell_bits != -1) {
			event_set_cell_bits(ev, x, y, g_lock_cell_bits);
			AUTOMAP_CELL_WRITE_W(g_map, 0x37b, mev->y - g_map_x0, mev->x - g_map_y0, d);
			snd_play(g_snd_fx, 0x61);
			if (!g_dng07_dogs_door_2c && idx == 0x2c) {
				/* BLUT2.LTX[9]: "Eben als sich die Tuer hebt, hoert Ihr ein lautes Knurren und schon kommt eine Meute
				 * geifernder Kampfhunde unter der Tuer auf Euch zugehetzt!" */
				dlg_message(g_dialog, ev->txt[9]);
				if (event_fight1(ev, 0x6c, DATE->ticks > 0x93a8L && DATE->ticks <= 0x190c8L ? 4 : 8,
					0) == 0)
					g_dng07_dogs_door_2c = 1;
			} else if (!g_dng07_dogs_door_2e && idx == 0x2e) {
				dlg_message(g_dialog, ev->txt[9]);
				if (event_fight1(ev, 0x6c, DATE->ticks > 0x93a8L && DATE->ticks <= 0x190c8L ? 4 : 8,
					0) == 0)
					g_dng07_dogs_door_2e = 1;
			}
		}
	}
	return ret;
}

signed short dng07_shelves(struct area_events far *ev, signed short pos)
{
	/* BLUT2.LTX[1]: "In diesem Regal wird offensichtlich alles moegliche und unmoegliche gelagert.@Wollt Ihr etwas
	 * naeher darin herumstoebern?" */
	char far *msg = ev->txt[1];
	struct hero far *h;
	struct hero far *h2;
	register signed short idx = -1;
	register signed short ret = 0;

	switch (pos) {
	case 0x1602: idx = 0x14; break;
	case 0x1801: idx = 0x15; break;
	case 0x1a03: idx = 0x16; break;
	case 0x0f15: idx = 0x18; break;
	case 0x1015: {
		signed char a, b, c;
		char far *nm;
		ret = 1;
		h = get_first_active_hero(g_group);
		if (!g_dng07_shelf_searched && test_skill(h, TA_SINNESSCHAERFE, 6) <= 0)
			/* BLUT2.LTX[13]: "In diesem Regal liegen abgetragene Ruestungen orkischer Machart. Einmal abgesehen davon, dass
			 * sie Euch nicht passen wuerden, sind sie fast alle so zerschlissen, dass sie auch kaum noch Schutz bieten
			 * wuerden." */
			dlg_message(g_dialog, ev->txt[13]);
		else {
			g_dng07_shelf_searched = 1;
			/* BLUT2.LTX[14]: "In diesem Regal liegen abgetragene Ruestungen orkischer Machart. Einmal abgesehen davon, dass
			 * sie Euch nicht passen wuerden, sind sie fast alle so zerschlissen, dass sie auch kaum noch Schutz bieten
			 * wuerden.@Viel interessanter ist allerdings, dass hinter dem Regal eine schmale Ritze in der Wand erkennbar
			 * ist...@Wollt Ihr versuchen, das Regal zur Seite zu schieben?" */
			if (dlg_yes_no(g_dialog, ev->txt[14]) == 1) {
				timewarp_minutes(g_date, 5L);
				/* BLUT2.LTX[15]: "Ohne allzu grosse Muehe schiebt Ihr das Regal zur Seite und widmet der Wand dahinter eine
				 * genauere Untersuchung." */
				dlg_message(g_dialog, ev->txt[15]);
				if (!g_dng07_shelf_hole_found && test_skill(h, TA_SINNESSCHAERFE, 4) <= 0)
					/* BLUT2.LTX[17]: "Leider koennt Ihr nichts Besonderes entdecken und schiebt das Regal zurueck an seinen
					 * Platz." */
					dlg_message(g_dialog, ev->txt[17]);
				else {
					g_dng07_shelf_hole_found = 1;
					a = b = c = -1;
					/* BLUT2.LTX[16]: "Ihr entdeckt in der Decke ueber der Tuer in etwa drei Schritt Hoehe eine kleine, etwa
					 * silbertalergrosse, runde Oeffnung." */
					dlg_message(g_dialog, ev->txt[16]);
					h2 = get_hero_attrib_max(g_group, ATTRIB_FF);
					if ((a = group_find_item(g_group,
						ITEM_SPEAR)) != -1 || (b = group_find_item(g_group,
						ITEM_PIKE)) != -1 ||
						(c = group_find_item(g_group, ITEM_FIGHTING_STAFF)) != -1) {
						if (a != -1)
							nm = grammar_article(0x8002, ITEM_SPEAR, 0);
						else if (b != -1)
							nm = grammar_article(0x8002, ITEM_PIKE, 0);
						else if (c != -1)
							nm = grammar_article(0x8002, ITEM_FIGHTING_STAFF, 0);
						if (test_attrib(h2, ATTRIB_FF, -4) > 0) {
							/* BLUT2.LTX[18]: "<Lasst uns doch mal das Ding hier versuchen> meint %s, nimmt sich %s und beginnt damit in
							 * der Oeffnung herumzustochern, bis Ihr ein scharfes Klicken hoert und ploetzlich, unter lautem Getoese,
							 * vor Euch die Wand im Boden versinkt." */
							sprintf(g_text_output_buf, ev->txt[18], h2->name, nm);
							dlg_message(g_dialog, g_text_output_buf);
							snd_play(g_snd_fx, 0x61);
							event_set_cell(ev, 0x15, 0x10, 1, 0, -1);
							event_set_cell(ev, 0x16, 0x10, 1, 0, -1);
							AUTOMAP_CELL_WRITE_W(g_map, 0x37b, 0x10, 0x15, 7);
							AUTOMAP_CELL_WRITE_W(g_map, 0x37b, 0x10, 0x16, 3);
						} else {
							/* BLUT2.LTX[19]: "<Lasst uns doch mal das Ding hier versuchen> meint %s, nimmt sich %s und beginnt damit in
							 * der Oeffnung herumzustochern, doch es geschieht nichts.@Leicht frustriert schiebt Ihr das Regal wieder an
							 * seinen Platz." */
							sprintf(g_text_output_buf, ev->txt[19], h2->name, nm);
							dlg_message(g_dialog, g_text_output_buf);
						}
					} else
						/* BLUT2.LTX[20]: "Leider ist das Loch fuer Euch unerreichbar und so schiebt Ihr das Regal wieder zurueck an
						 * seinen Platz, um kein Aufsehen zu erregen." */
						dlg_message(g_dialog, ev->txt[20]);
				}
			}
		}
		break;
	}
	case 0x0a11: idx = 0x19; break;
	case 0x0a13: idx = 0x1a; break;
	case 0x0917:
		idx = 0x1b;
		/* BLUT2.LTX[113]: "Hier lagern Nahrungsmittel verschiedener Art - um genau zu sein, der Frass, den Ihr die
		 * letzte Woche kosten durftet. Auf den gruenlich schimmernden Schinken und dem weiss bepelzten Kaese haben sich
		 * bereits mehrere Hundert Kleintiere niedergelassen.@Wollt Ihr Euch dennoch bedienen?" */
		msg = ev->txt[113];
		break;
	case 0x0918:
		idx = 0x1c;
		/* BLUT2.LTX[114]: "Hier finden sich Nahrungsmittel, die noch gut erhalten scheinen - offensichtlich das Essen
		 * des Wachpersonals." */
		msg = ev->txt[114];
		break;
	case 0x0919: idx = 0x1d; break;
	}
	if (idx != -1) {
		Stash far *l = new Stash(idx, 1);
		if (l->has_items() == 1 && dlg_yes_no(g_dialog, msg) == 1) {
			l->show();
			ret = 1;
		}
		delete l;
	}
	return ret;
}

void dng07_gold_statue(struct area_events far *ev)
{
	struct hero far *h;

	/* BLUT2.LTX[3]: "Die hoelzerne Brazoragh-Statue ist mit einer duennen Goldschicht ueberzogen." */
	dlg_message(g_dialog, ev->txt[3]);
	if (!g_dng07_statue_scraped) {
		h = get_hero_attrib_max(g_group, ATTRIB_GG);
		/* BLUT2.LTX[4]: "%s beginnt, das Gold von der Statue abzuschaben. Auf Euer Zurufen reagiert %s nicht und als Ihr
		 * %s beruehrt, faehrt %s Hand sofort zur Waffe...@ <Moment noch!> presst %s zwischen zusammengebissenen Zaehnen
		 * hervor." */
		sprintf(g_text_output_buf, ev->txt[4], h->name, grammar_pronoun(h->sex, 0, 0),
			grammar_pronoun(h->sex, 2, 0),
			grammar_pronoun(h->sex, 1, 1), grammar_pronoun(h->sex, 0, 0));
		dlg_message(g_dialog, g_text_output_buf);
		timewarp_minutes(g_date, 10L);
		group_give_item(g_group, ITEM_GOLD_JEWELRY, 3);
		g_dng07_statue_scraped = 1;
	}
}

void dng07_straw(struct area_events far *ev, struct hero far *h, signed short pos)
{
	signed short v;

	timewarp_minutes(g_date, 5L);
	/* BLUT2.LTX[11]: "Ein Haufen ziemlich zerwuehltes Stroh." */
	sprintf(g_text_output_buf, ev->txt[11]);
	if (pos == 0x130a || pos == 0x130f) {
		/* BLUT2.LTX[12]: "@Mit der orkischen Sauberkeit scheint es nicht zum Besten zu stehen. %s hat sich ziemlich
		 * eingesaut." */
		sprintf(g_text_buf2, ev->txt[12], h->name);
		strcat(g_text_output_buf, g_text_buf2);
		dlg_message(g_dialog, g_text_output_buf);
		if (g_ingame_timers[INGAME_TIMER_DNG07_DIRT] == 0) {
			v = get_free_effect(h);
			add_effect(h, v, 0x1fa40L, 2, -1, 0);
			g_ingame_timers[INGAME_TIMER_DNG07_DIRT] = 0x1fa40L;
		}
	} else if (pos == 0x150a) {
		if (!g_dng07_brandy_taken) {
			/* BLUT2.LTX[112]: "Ein Haufen ziemlich zerwuehltes Stroh.@Nach naeherem Untersuchen entdeckt Ihr jedoch eine
			 * angesoffene Flasche Schnaps.@Wollt Ihr sie mitnehmen?" */
			if (dlg_yes_no(g_dialog, ev->txt[112]) == 1) {
				group_give_item(g_group, ITEM_BRANDY, 1);
				g_dng07_brandy_taken = 1;
			}
		} else
			dlg_message(g_dialog, g_text_output_buf);
	} else if (pos == 0x0111 && !g_dng07_straw_amulet) {
		/* BLUT2.LTX[59]: "Nach einigem Wuehlen im fauligen Stroh entdeckt Ihr ein kleines, silbernes Amulett." */
		dlg_message(g_dialog, ev->txt[59]);
		group_give_item(g_group, ITEM_MIRROR_AMULET, 1);
		g_dng07_straw_amulet = 1;
	} else
		dlg_message(g_dialog, g_text_output_buf);
}

void dng07_graffiti(struct area_events far *ev)
{
	signed short r;

	/* BLUT2.LTX[60..80] (Kritzeleien, eine davon per Zufall):
	 *   [60] "Tod allen Orken!"
	 *   [61] "ToT aLLem OrGs!!!"
	 *   [62] "Tod dem Hurensohn Answin!"
	 *   [63] "Hoch lebe das III.^Regiment"
	 *   [64] "Hoch lebe die 19.^Kompanie"
	 *   [65] "Hoch lebe das 11.^Banner"
	 *   [66] "Hoch lebe Kaiser Hal!"
	 *   [67] "Huetet Euch vor Belgor!"
	 *   [68] "IIII"
	 *   [69] "IIIII IIIII II"
	 *   [70] "IIIII II"
	 *   [71] "Alrik war hier"
	 *   [72] "B.J. Blaskowitz war hier"
	 *   [73] "Keiner kommt hier lebend raus!"
	 *   [74] "Rondra hilf!"
	 *   [75] "Praios hilf!"
	 *   [76] "Phex hilf!"
	 *   [77] "Der Polardiamant liegt im Tempel von aaarrrghhhh..."
	 *   [78] "Sadrak Whassoi ist ein XXXXXX"
	 *   [79] "ISCh lEbe NocH!"
	 *   [80] "Scheisse!" */
	r = random_schweif(21) - 1;
	/* BLUT2.LTX[81]: "Ihr nehmt die Kritzeleien auf der Wand naeher in Augenschein und entdeckt
	 * Folgendes.@@<F1>%s<F0>" */
	sprintf(g_text_output_buf, ev->txt[81], ev->txt[r + 60]);
	dlg_message(g_dialog, g_text_output_buf);
}

void dng07_web(struct area_events far *ev, signed short pos)
{
	signed short d;
	signed short v2;
	signed short y;
	signed short x;
	signed char far *f1;
	signed char far *f2;
	struct hero far *h;
	struct hero far *h2;
	register signed short v1;
	register signed short r;

	f1 = 0;
	f2 = 0;
	x = pos & 0xff;
	y = (pos >> 8) & 0xff;
	v1 = v2 = 0;
	h = get_first_active_hero(g_group);
	if ((h2 = get_second_active_hero(g_group)) != 0) {
		switch (pos) {
		case 0x131a:
			f1 = &g_dng07_web_131a;
			break;
		case 0x1321:
			f1 = &g_dng07_web_1321;
			f2 = &g_dng07_spiders_1321;
			if (!g_dng07_spiders_1321)
				v1 = v2 = 2;
			break;
		case 0x1022:
			f1 = &g_dng07_web_1022;
			f2 = &g_dng07_spiders_1022;
			if (!g_dng07_spiders_1022) {
				v1 = 2;
				v2 = 1;
			}
			break;
		case 0x1024:
			f1 = &g_dng07_web_1024;
			break;
		case 0x0e25:
			f1 = &g_dng07_web_0e25;
			f2 = &g_dng07_spiders_0e25;
			if (!g_dng07_spiders_0e25)
				v2 = 3;
			break;
		case 0x0922:
			f1 = &g_dng07_web_0922;
			f2 = &g_dng07_spiders_0922;
			if (!g_dng07_spiders_0922) {
				v1 = 2;
				v2 = 1;
			}
			break;
		case 0x0720:
			f1 = &g_dng07_web_0720;
			break;
		case 0x0320:
		case 0x0323:
			f1 = &g_dng07_web_0320;
			f2 = &g_dng07_spiders_0320;
			if (!g_dng07_spiders_0320) {
				v1 = 2;
				v2 = 3;
			}
			break;
		case 0x0927:
			f1 = &g_dng07_web_0927;
			f2 = &g_dng07_spiders_0927;
			if (!g_dng07_spiders_0927)
				v1 = 2;
			break;
		case 0x032b:
			f1 = &g_dng07_web_032b;
			f2 = &g_dng07_spiders_032b;
			if (!g_dng07_spiders_032b) {
				v1 = 2;
				v2 = 3;
			}
			break;
		}
		r = hero_weapon_type2(h);
		if (r == 4 || r == 3 || r == 2 || r == 6) {
			r = hero_weapon_type2(h2);
			if (r == 4 || r == 3 || r == 2 || r == 6) {
				signed char c;
				d = 50 - ((signed char)(h->attrib[ATTRIB_KK].cur + h->attrib[ATTRIB_KK].mod) +
					(signed char)(h2->attrib[ATTRIB_KK].cur + h2->attrib[ATTRIB_KK].mod) + 1);
				/* BLUT2.LTX[84]: "Mit maechtigen Hieben zerteilt Ihr die klebrigen, nahezu fingerdicken Faeden des Netzes." */
				dlg_message(g_dialog, ev->txt[84]);
#if FIX_0105
				/* Fix #105: Wartezeit nicht negativ */
				timewarp_minutes(g_date, (signed long)(d < 0 ? 0 : d));
#else
				timewarp_minutes(g_date, (signed long)d);
#endif
				event_set_cell(ev, x, y, 0x0f, 0, -1);
				c = automap_cell_read(g_map, 0x37b, y, x);
				AUTOMAP_CELL_WRITE_W(g_map, 0x37b, y, x, c == 0x55 ? 0x74 : 0x75);
#if FIX_0105
				/* Fix #105: unbekanntes Feld: nicht nach 0000:0000 schreiben */
				if (f1 != 0)
					*f1 = 1;
#else
				*f1 = 1;
#endif
				if (v1 || v2) {
					if (event_fight2(ev, 0x26, v1, 0, 0x27, v2, 0) == 0)
						*f2 = 1;
				}
			} else {
				/* BLUT2.LTX[86]: "Es gelingt %s und %s nicht, das Netz zu zerteilen. Staendig bleiben Eure Waffen an den
				 * Faeden kleben." */
				sprintf(g_text_output_buf, ev->txt[86], h->name, h2->name);
				dlg_message(g_dialog, g_text_output_buf);
			}
		} else {
			sprintf(g_text_output_buf, ev->txt[86], h->name, h2->name);
			dlg_message(g_dialog, g_text_output_buf);
		}
	} else {
		/* BLUT2.LTX[85]: "Es gelingt %s nicht, das Netz zu zerteilen, denn %s verheddert sich komplett in den klebrigen
		 * Faeden und kurz darauf hoert %s bereits das unheimliche, bedrohliche Pfeifen von Hoehlenspinnen. Hilflos
		 * gefangen in dem Netz erlebt %s, wie es ist, bei lebendigem Leibe verdaut zu werden!" */
		sprintf(g_text_output_buf, ev->txt[85], h->name, grammar_pronoun(h->sex, 0, 0),
			grammar_pronoun(h->sex, 0, 0), h->name);
		dlg_message(g_dialog, g_text_output_buf);
		sub_hero_le(h, h->le);
	}
}

void dng07_web_corpse(struct area_events far *ev)
{
	struct hero far *h;
	struct hero far *h2;
	register signed short r;

	/* BLUT2.LTX[87]: "Ihr untersucht das Skelett genauer, doch es scheint nichts von Interesse zu geben, bis Ihr
	 * hinter dem Netz etwas zu blitzen sehen scheint.@Wollt Ihr nachsehen, was da ist?" */
	if (!g_dng07_corpse_sword_taken && dlg_yes_no(g_dialog, ev->txt[87]) == 1) {
		h = get_first_active_hero(g_group);
		if ((h2 = get_second_active_hero(g_group)) != 0) {
			if (group_test_attrib_dmg(g_group, ATTRIB_TA, 4, 0) < g_group->nr_heroes / 2)
				/* BLUT2.LTX[88]: "Ihr schafft es nicht, Eure Blicke von den skelettierten Ueberresten dieses Opfers der
				 * Riesenspinnen zu nehmen und Eure Fuesse fuehlen sich wie Blei an. Leider schafft Ihr es nicht, Eure
				 * Totenangst zu ueberwinden und muesst von diesem Unterfangen ablassen." */
				dlg_message(g_dialog, ev->txt[88]);
			else {
				r = hero_weapon_type2(h);
				if (r == 4 || r == 3 || r == 2 || r == 6) {
					r = hero_weapon_type2(h2);
					if (r == 4 || r == 3 || r == 2 || r == 6) {
						/* BLUT2.LTX[89]: "In einer Nische hinter dem Leichnam koennt Ihr ein Schwert mit reich verziertem Griff
						 * finden, das dem Ungluecklichen aber offensichtlich wenig genutzt hat." */
						dlg_message(g_dialog, ev->txt[89]);
						group_give_item(g_group, ITEM_ORNATE_SWORD, 1);
						event_set_cell(ev, 0x1b, 1, 0x0f, 0, -1);
						event_fight2(ev, 0x26, 3, 0, 0x27, 4, 0);
						g_dng07_corpse_sword_taken = 1;
					} else {
						/* BLUT2.LTX[86]: "Es gelingt %s und %s nicht, das Netz zu zerteilen. Staendig bleiben Eure Waffen an den
						 * Faeden kleben." */
						sprintf(g_text_output_buf, ev->txt[86], h->name, h2->name);
						dlg_message(g_dialog, g_text_output_buf);
					}
				} else {
					sprintf(g_text_output_buf, ev->txt[86], h->name, h2->name);
					dlg_message(g_dialog, g_text_output_buf);
				}
			}
		} else {
			/* BLUT2.LTX[85]: "Es gelingt %s nicht, das Netz zu zerteilen, denn %s verheddert sich komplett in den klebrigen
			 * Faeden und kurz darauf hoert %s bereits das unheimliche, bedrohliche Pfeifen von Hoehlenspinnen. Hilflos
			 * gefangen in dem Netz erlebt %s, wie es ist, bei lebendigem Leibe verdaut zu werden!" */
			sprintf(g_text_output_buf, ev->txt[85], h->name, grammar_pronoun(h->sex, 0, 0),
				grammar_pronoun(h->sex, 0, 0),
				h->name);
			dlg_message(g_dialog, g_text_output_buf);
			sub_hero_le(h, h->le);
		}
	}
}
