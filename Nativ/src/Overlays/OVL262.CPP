/* OVL262 - Overlay O262 (Segment 0x24bc in SCHWEIF.EXE): Feldereignisse des Dungeons 6 (erste Ebene der Orkfestung,
 * Texte BLUT1.LTX ueber ev->txt) und gemeinsame Routinen der Orkfestung (blut_*, auch aus O264/O267).
 * blut_nop: leer (30 Aufrufer nach jeder Laermerhoehung). blut_noise_check(ev, noise): bei W100 < *noise wird *noise
 *   = 100 (Alarm).
 * dng06_brazoragh: Statue des Kriegsgottes Brazoragh (BLUT1[18..27]): pluendern (Orkschmuck, Laerm + 1, geopferte
 *   Dukaten g_dng06_statue_ducats zurueck [123]/[124]), zerstoeren (Laerm + 3, Ansehen aller Goetter +20, [26]),
 *   opfern (Dukaten bis zum Gruppengeld, hoechstens 999; Ansehen -50), ignorieren (einmal Praios -10); 5 Minuten.
 * dng06_pit(ev, st): Klappgrube - 0 bemerken (Sinnesschaerfe +4, [33]); 1 warnen ([35]) bzw. hineinfallen ([34], 2..6
 *   LE fuer alle, *st = 2); 2 behutsam umgehen ([36]).
 * blut_rockfall(ev, st, a, b, c1, c2, d1, d2, s): hat die Gruppe mehr als b Helden und misslingt eine
 *   Schleichen-Probe (Zuschlag a): Text s, Schaden c1..c2 bzw. d1..d2, wenn GE-Proben misslingen
 *   (group_test_attrib_dmg); *st = 1.
 * dng06_beer_barrel(ev, h, pos): Bierfass an 0xf15 ([94..99]: leertrinken, Ratte, betrunken, Laerm + 15, Wirkung)
 *   bzw. Ogerbier ([112]); danach [95]/[111].
 * dng06_straw(ev, h, pos): Strohlager ([1]/[2], 5 Minuten): Silbertaler [3], eingesaut [4]/[70] (Wirkung, Sperre
 *   INGAME_TIMER_DNG06_DIRT), Saebel der Oger [71]/[72], Dukaten [44], Arbach [45], Orkschmuck [46], Heiltrank [49].
 * dng06_stores(ev, pos): Ablagen 5..19 mit Fragen [11..13]/[92]/[122]. dng06_weapon_rack(ev, pos): an 0x1d02
 *   Waffenstaender der Ork-Wachen ([6..9]: nehmen = Ablage 4, zerstoeren = Laerm + 2), sonst Ablagen 16..19.
 * dng06_door(ev, pos, x, y, mev): Schloesser (Lock 0x12..0x27) mit Wandbild 0x70/0x71.
 * dng06_beetle_pit(ev, h): Grube an 0x814 (scharf ueber g_dng06_beetle_pit_armed, ohne Knochen-/Kupferschluessel):
 *   Sinnesschaerfe +6 ([116]) oder Sturz (8..15 LE, [117], Kampf 2 x 0x0b Schroeter); danach [118].
 * dng06_hornets: Hornissen ([28..32]) - zu schwer bzw. unbrauchbar machen (Laerm + 1). dng06_dwarf_skeleton:
 *   Zwergenskelett ([38..40]: Geld und Kriegsbeil, 25 Minuten); der Totenaengstlichste bekommt einen Anfall ([41]),
 *   der Charismatischste beruhigt ihn ([43]) oder schlaegt ihn nieder ([42], W6 LE).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL262.CPP -> 6102/6102 Bytes GLEICH (18.10.2026, erster Entwurf), cobjfix laeuft
 *   (g_ingame_timers[INGAME_TIMER_DNG06_DIRT] -> _g_246e+0x74). Ohne -Og 3859 Abweichungen; -Oi fuer strcat.
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Muster aus OVL268: Initialisierung in Deklarationsreihenfolge ohne register, Klassen Stash (O238)/Lock (O108),
 *   "if (!*st) {..; return;}".
 * - automap_cell_write ueber AUTOMAP_CELL_WRITE_W; count_failed_skill in blut_rockfall ueber COUNT_FAILED_SKILL_W
 *   (Zuschlag als short, s_proto.h signed char ergaebe mov al / push ax).
 * - switch auf pos mit gestreuten Faellen -> Vergleichsbaum, Rumpfreihenfolge = Quellreihenfolge; dng06_weapon_rack:
 *   letzter case (0x1d02) ohne break (mit break jmp short).
 * - group_sub_le_range bekommt zwei Wortparameter (in blut_rockfall als c1/c2 bzw. d1/d2 durchgereicht).
 * Auffaellig (Original): dng06_dwarf_skeleton - im Misserfolgszweig steht fuer den zweiten Helden h2 das Pronomen von
 *   h (grammar_pronoun(h->sex, 3, 0)), im Erfolgszweig richtig h2->sex; dng06_brazoragh - der innere Test
 *   !g_dng06_statue_destroyed im Zweig 2 ist immer wahr; dng06_weapon_rack - ret = 1 doppelt (auch ohne Nehmen).
 */
#include <stdio.h>
#include <string.h>

#include "schweif.h"

/* automap_cell_write mit short als letztem Parameter (wie im Original deklariert) */
#define AUTOMAP_CELL_WRITE_W \
	((void (*)(void far *, signed short, signed short, signed short, signed short))automap_cell_write)
/* count_failed_skill mit short als Zuschlag (wie im Original deklariert) */
#define COUNT_FAILED_SKILL_W ((signed short (*)(struct group far *, signed short, signed short))count_failed_skill)

void blut_nop(void)
{
}

void blut_noise_check(struct area_events far *ev, signed char far *v)
{
	if (random_schweif(100) < *v)
		*v = 100;
}

void dng06_brazoragh(struct area_events far *ev)
{
	signed short n;
	signed short r;
	signed short i;

	if (!g_dng06_statue_destroyed) {
		/* BLUT1.LTX[18]: "Eine fast zweieinhalb Schritte hohe, steinerne Statue des orkischen Kriegsgottes Brazoragh
		 * verziert mit allerlei Schmuck aus Kupfer und Messing. In der rechten Hand traegt sie einen anscheinend echten
		 * Arbach.@Wollt Ihr" */
		/* BLUT1.LTX[19]: "die Statue pluendern" */
		/* BLUT1.LTX[20]: "die Statue zerstoeren" */
		/* BLUT1.LTX[21]: "Opfern" */
		/* BLUT1.LTX[22]: "die Statue ignorieren" */
		r = dlg_choose(g_dialog, ev->txt[18], 4, ev->txt[19], ev->txt[20], ev->txt[21], ev->txt[22]);
		if (r == 1) {
			if (!g_dng06_statue_looted) {
				g_dng06_noise++;
				blut_noise_check(ev, &g_dng06_noise);
				group_give_item(g_group, ITEM_ORC_JEWELRY, 1);
				g_dng06_statue_looted = 1;
				/* BLUT1.LTX[24]: "Ihr nehmt den orkischen Schmuck an Euch, aber das war's auch schon." */
				dlg_message(g_dialog, ev->txt[24]);
				if (g_dng06_statue_ducats != 0) {
					group_add_money(g_group, (signed long)g_dng06_statue_ducats * 100);
					/* BLUT1.LTX[123]: "Doch Moment mal, Ihr findet noch %d^Dukaten." */
					sprintf(g_text_output_buf, ev->txt[123], g_dng06_statue_ducats);
					dlg_message(g_dialog, g_text_output_buf);
					g_dng06_statue_ducats = 0;
				}
			} else if (g_dng06_statue_ducats != 0) {
				group_add_money(g_group, (signed long)g_dng06_statue_ducats * 100);
				/* BLUT1.LTX[124]: "Ihr nehmt die von Euch geopferten %d^Dukaten wieder an Euch." */
				sprintf(g_text_output_buf, ev->txt[124], g_dng06_statue_ducats);
				dlg_message(g_dialog, g_text_output_buf);
				g_dng06_statue_ducats = 0;
			} else {
				/* BLUT1.LTX[23]: "Das gibt's nichts mehr zu pluendern." */
				dlg_message(g_dialog, ev->txt[23]);
			}
		} else if (r == 2) {
			if (!g_dng06_statue_destroyed) {
				g_dng06_statue_destroyed = 1;
				g_dng06_noise += 3;
				blut_noise_check(ev, &g_dng06_noise);
				for (i = 1; i <= 14; i++)
					g_god_favor[i] += 20;
				/* BLUT1.LTX[26]: "Mit wenigen wuchtigen Hieben zerschlagt Ihr das Goetzenbild." */
				dlg_message(g_dialog, ev->txt[26]);
			}
		} else if (r == 3) {
			n = g_group->money / 100;
			/* BLUT1.LTX[27]: "Wieviele Dukaten wollt Ihr darbringen?" */
			r = dlg_input(g_dialog, 3, ev->txt[27], 0, n > 999 ? 999 : n, 0);
			if (r != -1) {
				g_dng06_statue_ducats += r;
				group_sub_money(g_group, (signed long)r * 100);
				for (i = 1; i <= 14; i++)
					g_god_favor[i] -= 50;
			}
		} else if (!g_dng06_statue_ignored) {
			g_god_favor[1] -= 10;
			g_dng06_statue_ignored = 1;
		}
		timewarp_minutes(g_date, 5L);
	} else {
		/* BLUT1.LTX[25]: "Nun, das ist das, was Ihr von der Statue des Brazorag uebrig gelassen habt." */
		dlg_message(g_dialog, ev->txt[25]);
	}
}

void dng06_pit(struct area_events far *ev, signed char far *st)
{
	struct hero far *h;

	h = get_first_active_hero(g_group);
	if (!*st) {
		if (test_skill(h, TA_SINNESSCHAERFE, 4) >= 0)
			/* BLUT1.LTX[33]: "Hier scheint der Boden so konstruiert zu sein, dass er in einem Stueck wegklappen und -
			 * wahrscheinlich - eine haessliche Grube freilegen kann." */
			dlg_message(g_dialog, ev->txt[33]);
		return;
	}
	if (*st == 1) {
		if (test_skill(h, TA_SINNESSCHAERFE, 4) >= 0) {
			/* BLUT1.LTX[35]: "<Vorsicht!> warnt Euch %s.<Da ist eine Fallgrube vor uns. Seht Ihr die Fugen?>@Mit dem Finger
			 * zeigt %s Euch den genauen Verlauf der Grube, so dass Ihr sie alle unbeschadet passieren koennt." */
			sprintf(g_text_output_buf, ev->txt[35], h->name, grammar_pronoun(h->sex, 0, 0));
			dlg_message(g_dialog, g_text_output_buf);
		} else {
			*st = 2;
			/* BLUT1.LTX[34]: "Ploetzlich bewegt sich der Boden unter Euren Fuessen und gibt eine Grube frei, deren Boden
			 * mit scharfkantigen Tonscherben bedeckt ist.Da es sich in der Luft so schlecht steht, fallt Ihr und zieht Euch
			 * einige haessliche Schnitte und Wunden zu. Nach einiger Zeit gelingt es Euch allerdings, wieder aus der Grube
			 * herauszuklettern." */
			dlg_message(g_dialog, ev->txt[34]);
			group_sub_le_range(g_group, 2, 6);
			timewarp_minutes(g_date, 10L);
		}
		return;
	}
	/* BLUT1.LTX[36]: "Vorsichtig geht Ihr der Fallgrube aus dem Weg. Da sie allerdings sehr gross ist und Eure Fuesse
	 * fast keinen Halt finden koennen, muesst Ihr sehr behutsam und gemaechlich vorangehen." */
	dlg_message(g_dialog, ev->txt[36]);
	timewarp_minutes(g_date, 5L);
}

signed short blut_rockfall(struct area_events far *ev, signed char far *st, signed short a, signed short b,
	signed short c1,
	signed short c2, signed short d1, signed short d2, char far *s)
{
	signed short r = 0;

	if (g_group->nr_heroes > b) {
		if (COUNT_FAILED_SKILL_W(g_group, TA_SCHLEICHEN, a) > 0) {
			if (s != 0)
				dlg_message(g_dialog, s);
			if (group_test_attrib_dmg(g_group, 4, 0, 0) > 0)
				group_sub_le_range(g_group, d1, d2);
			else
				group_sub_le_range(g_group, c1, c2);
			*st = 1;
			r = 1;
		}
	}
	return r;
}

void dng06_beer_barrel(struct area_events far *ev, struct hero far *h, signed short pos)
{
	signed short v;

	if (pos == 0xf15) {
		if (!g_dng06_beer_drained) {
			/* BLUT1.LTX[94]: "Hier steht ein ziemlich volles Bierfass.@Wollt Ihr davon trinken?" */
			if (dlg_yes_no(g_dialog, ev->txt[94]) == 1) {
				timewarp_minutes(g_date, 5L);
				snd_play(g_snd_fx, 0x5d);
				/* BLUT1.LTX[96]: "<Wow! Das ist ein Zeug!> meint %s, als %s sich den Mund abwischt.@<Echt lecker!>" */
				sprintf(g_text_output_buf, ev->txt[96], h->name, grammar_pronoun(h->sex, 0, 0));
				dlg_message(g_dialog, g_text_output_buf);
				g_dng06_beer_drunk = 1;
				g_dng06_beer_drained = 1;
				h = get_random_hero(g_group);
				/* BLUT1.LTX[97]: "Als sich %s ueber das Fass beugt, um noch einen Schluck zu nehmen wird %s stutzig.@<Da liegt
				 * was am Boden...> meint %s und versucht, mit den Haenden den Boden des Fasses zu erreichen, doch... zu kurz!" */
				sprintf(g_text_output_buf, ev->txt[97], h->name, grammar_pronoun(h->sex, 0, 0),
					grammar_pronoun(h->sex, 0, 0));
				dlg_message(g_dialog, g_text_output_buf);
				/* BLUT1.LTX[98]: "<Ich muss wissen, was das ist...>@%s versucht nun, kopfueber in das riesige Bierfass zu
				 * steigen und ploetzlich beginnt %s, wie wild mit den Beinen zu zappeln. Das Fass geraet ins Schwanken, kippt
				 * um und zerbricht unter dem Gewicht. In einem breiten Strom verteilt sich das Bier auf dem Boden und bildet
				 * eine riesige Lache." */
				sprintf(g_text_output_buf, ev->txt[98], h->name, grammar_pronoun(h->sex, 0, 0));
				dlg_message(g_dialog, g_text_output_buf);
				/* BLUT1.LTX[99]: "Nun koennt Ihr alle erkennen, was da in den Tiefen des Fasses verborgen lag.@Eine Ratte." */
				dlg_message(g_dialog, ev->txt[99]);
				g_dng06_noise += 15;
				hero_get_drunken(h);
				v = get_free_effect(h);
				add_effect(h, v, 0x1fa40L, 2, -1, 0);
			}
		} else {
			/* BLUT1.LTX[95]: "Ihr habt's doch tatsaechlich geschafft, das ganze Fass leerzusaufen! Ihr seid ja die reinsten
			 * Kampftrinker!" */
			dlg_message(g_dialog, ev->txt[95]);
		}
	} else if (!g_dng06_ogre_beer_tasted) {
		if (dlg_yes_no(g_dialog, ev->txt[94]) == 1) {
			timewarp_minutes(g_date, 5L);
			snd_play(g_snd_fx, 0x5d);
			g_dng06_ogre_beer_tasted = 1;
			/* BLUT1.LTX[112]: "<Bei Phex!> meint %s und verzieht das Gesicht zu einer Grimasse. <Das Zeug schmeckt wie
			 * Ogerpisse! Sieht zwar aus wie Bier, aber ich denke nicht, dass es irgendwas dieser Art ist!>" */
			sprintf(g_text_output_buf, ev->txt[112], h->name);
			dlg_message(g_dialog, g_text_output_buf);
		}
	} else {
		/* BLUT1.LTX[111]: "Nein, nein. Von dem Zeug laesst man besser die Finger!" */
		dlg_message(g_dialog, ev->txt[111]);
	}
}

void dng06_straw(struct area_events far *ev, struct hero far *h, signed short pos)
{
	signed short v;

	timewarp_minutes(g_date, 5L);
	/* BLUT1.LTX[1]: "Ein Haufen ziemlich zerwuehltes Stroh." */
	sprintf(g_text_output_buf, ev->txt[1]);
	if (pos == 0x1502) {
		if (!g_dng06_straw_1502) {
			/* BLUT1.LTX[3]: "@Unter dem Stroh findet Ihr 11^Silbertaler." */
			strcat(g_text_output_buf, ev->txt[3]);
			group_add_money(g_group, 0x6eL);
			g_dng06_straw_1502 = 1;
		}
		dlg_message(g_dialog, g_text_output_buf);
	} else if (pos == 0x1702 || pos == 0x1007 || pos == 0xe1a) {
		/* BLUT1.LTX[4]: "@Mit der orkischen Sauberkeit scheint es nicht zum Besten zu stehen. %s hat sich ziemlich
		 * eingesaut." */
		sprintf(g_text_buf2, ev->txt[4], h->name);
		strcat(g_text_output_buf, g_text_buf2);
		dlg_message(g_dialog, g_text_output_buf);
		if (g_ingame_timers[INGAME_TIMER_DNG06_DIRT] == 0) {
			v = get_free_effect(h);
			add_effect(h, v, 0x1fa40L, 2, -1, 0);
			g_ingame_timers[INGAME_TIMER_DNG06_DIRT] = 0x1fa40L;
		}
	} else if (pos == 0xf07) {
		h = get_random_hero(g_group);
		/* BLUT1.LTX[70]: "@%s schreit ploetzlich erstickt auf, als %s Haende in etwas Weiches fassen, das sich beim
		 * zweiten Hinsehen als die Reste einer ogrischen Mahlzeit erweist..." */
		sprintf(g_text_buf2, ev->txt[70], h->name, grammar_pronoun(h->sex, 1, 1));
		strcat(g_text_output_buf, g_text_buf2);
		dlg_message(g_dialog, g_text_output_buf);
		if (g_ingame_timers[INGAME_TIMER_DNG06_DIRT] == 0) {
			v = get_free_effect(h);
			add_effect(h, v, 0x1fa40L, 2, -1, 0);
			g_ingame_timers[INGAME_TIMER_DNG06_DIRT] = 0x1fa40L;
		}
	} else if (pos == 0x100b && !g_dng06_straw_100b) {
		/* BLUT1.LTX[71]: "@Ihr findet einen einigermassen brauchbaren Saebel, der von den Ogern offensichtlich als
		 * Essmesser benutzt wurde.@Wollt Ihr ihn mitnehmen?" */
		strcat(g_text_output_buf, ev->txt[71]);
		if (dlg_yes_no(g_dialog, g_text_output_buf) == 1 && !g_dng06_straw_100b) {
			/* BLUT1.LTX[72]: "<Ziemlich abstossend das Teil, aber was soll's> mault %s, als Ihr den Saebel einpackt." */
			sprintf(g_text_output_buf, ev->txt[72], h->name);
			dlg_message(g_dialog, g_text_output_buf);
			group_give_item(g_group, ITEM_SABRE, 1);
			g_dng06_straw_100b = 1;
		}
	} else if (pos == 0xd02) {
		if (!g_dng06_straw_0d02) {
			/* BLUT1.LTX[44]: "@Ihr findet 1^Dukaten und 8^Silbertaler." */
			strcat(g_text_output_buf, ev->txt[44]);
			group_add_money(g_group, 0xb4L);
			g_dng06_straw_0d02 = 1;
		}
		dlg_message(g_dialog, g_text_output_buf);
	} else if (pos == 0xa08) {
		if (!g_dng06_straw_0a08) {
			/* BLUT1.LTX[45]: "@Unter dem Strohlager entdeckt Ihr einen gut erhaltenen Arbach." */
			strcat(g_text_output_buf, ev->txt[45]);
			group_give_item(g_group, ITEM_ORC_SABRE, 1);
			g_dng06_straw_0a08 = 1;
		}
		dlg_message(g_dialog, g_text_output_buf);
	} else if (pos == 0xb08) {
		if (!g_dng06_straw_0b08) {
			/* BLUT1.LTX[46]: "@Ihr findet etwas orkischen Schmuck." */
			strcat(g_text_output_buf, ev->txt[46]);
			group_give_item(g_group, ITEM_ORC_JEWELRY, 1);
			g_dng06_straw_0b08 = 1;
		}
		dlg_message(g_dialog, g_text_output_buf);
	} else if (pos == 0xf0a) {
		if (!g_dng06_straw_0f0a) {
			/* BLUT1.LTX[49]: "@Den Goettern sei Dank! Ihr findet eine Flasche Heiltrank." */
			strcat(g_text_output_buf, ev->txt[49]);
			group_give_item(g_group, ITEM_LE_POTION, 1);
			g_dng06_straw_0f0a = 1;
		}
		dlg_message(g_dialog, g_text_output_buf);
	} else {
		/* BLUT1.LTX[2]: "@Beim Durchsuchen koennt Ihr nichts Besonderes entdecken." */
		strcat(g_text_output_buf, ev->txt[2]);
		dlg_message(g_dialog, g_text_output_buf);
	}
}

signed short dng06_stores(struct area_events far *ev, signed short pos)
{
	/* BLUT1.LTX[11]: "Hier lagern Nahrungsmittel fuer mehrere Dutzend Orks: Fleisch, Wuerste, Kaese, Brot und
	 * vielerlei Dinge mehr...@Ihr koenntet Euch hier billig bedienen. Wollt Ihr?" */
	char far *t = ev->txt[11];
	Stash far *o;
	signed short id = -1;
	signed short ret = 0;

	switch (pos) {
	case 0x061a:
		id = 5;
		break;
	case 0x061b:
		id = 6;
		break;
	case 0x061c:
		id = 7;
		break;
	case 0x021a:
		/* BLUT1.LTX[12]: "Zwischen den Regalboeden und der Wand sind mehrere grosse Faesser befestigt, die
		 * offensichtlich Bier enthalten.@Wollt Ihr etwas mitnehmen?" */
		t = ev->txt[12];
		id = 8;
		break;
	case 0x021b:
		t = ev->txt[12];
		id = 9;
		break;
	case 0x021c:
		t = ev->txt[12];
		id = 10;
		break;
	case 0x031d:
		/* BLUT1.LTX[13]: "In mehreren kleineren Faessern und Faesschen, aber auch in Flaschen aus aller Herren Laender
		 * lagern hier Weine und hochprotzentige Getraenke.@Wollt Ihr Euch bedienen?" */
		t = ev->txt[13];
		id = 11;
		break;
	case 0x041d:
		t = ev->txt[13];
		id = 12;
		break;
	case 0x051d:
		t = ev->txt[13];
		id = 13;
		break;
	case 0x0c0d:
		id = 16;
		/* BLUT1.LTX[122]: "In den Regalen liegen jede Menge Kisten und Ballen. Es scheinen Ruestungen und dergleichen zu
		 * sein.@Wollt Ihr nachsehen?" */
		t = ev->txt[122];
		break;
	case 0x0d0d:
		id = 17;
		/* BLUT1.LTX[92]: "Hier lagern eine Menge Waffen.@Wollt Ihr Euch bedienen?" */
		t = ev->txt[92];
		break;
	case 0x0e0d:
		id = 18;
		t = ev->txt[92];
		break;
	case 0x0f0d:
		id = 19;
		t = ev->txt[92];
		break;
	}
	if (id != -1) {
		o = new Stash(id, 1);
		if (o->has_items() == 1) {
			if (dlg_yes_no(g_dialog, t) == 1) {
				o->show();
				ret = 1;
			}
		} else {
			o->show();
			ret = 1;
		}
		delete o;
	}
	return ret;
}

signed short dng06_weapon_rack(struct area_events far *ev, signed short pos)
{
	signed short a;
	signed short ret = 0;
	signed short id = -1;
	/* BLUT1.LTX[122]: "In den Regalen liegen jede Menge Kisten und Ballen. Es scheinen Ruestungen und dergleichen zu
	 * sein.@Wollt Ihr nachsehen?" */
	char far *t = ev->txt[122];
	Stash far *o;

	switch (pos) {
	case 0x0c0d:
		id = 16;
		break;
	case 0x0d0d:
		id = 17;
		break;
	case 0x0e0d:
		id = 18;
		break;
	case 0x0f0d:
		id = 19;
		break;
	case 0x1d02:
		if (!g_dng06_rack_done) {
			/* BLUT1.LTX[6]: "In diesem Staender stehen und liegen diverse Waffen der Ork-Wachen.@Wollt Ihr" */
			/* BLUT1.LTX[7]: "Waffen nehmen" */
			/* BLUT1.LTX[8]: "Waffen zerstoeren" */
			a = dlg_choose(g_dialog, ev->txt[6], 2, ev->txt[7], ev->txt[8]);
			if (a == 1) {
				id = 4;
			} else if (a == 2) {
				/* BLUT1.LTX[9]: "Ihr zerschlagt die Waffen der Orks. Leider geht das nicht ganz lautlos vor sich!" */
				dlg_message(g_dialog, ev->txt[9]);
				g_dng06_noise += 2;
				blut_noise_check(ev, &g_dng06_noise);
				g_dng06_rack_done = 1;
				ret = 1;
			}
			timewarp_minutes(g_date, 5L);
			ret = 1;
		}
	}
	if (id != -1) {
		o = new Stash(id, 1);
		if (id != 4 && o->has_items() == 1) {
			if (dlg_yes_no(g_dialog, t) == 1) {
				o->show();
				ret = 1;
			}
		} else {
			o->show();
			ret = 1;
			if (id == 4 && o->has_items() == 0)
				g_dng06_rack_done = 1;
		}
		delete o;
	}
	return ret;
}

signed short dng06_door(struct area_events far *ev, signed short pos, signed short x, signed short y,
	struct map_event far *mev)
{
	signed short v = 0;
	signed short r = 0;
	signed short id = -1;

	switch (pos) {
	case 0x1a05: id = 0x12; v = 0x71; break;
	case 0x1405: id = 0x13; v = 0x70; break;
	case 0x1504: id = 0x14; v = 0x70; break;
	case 0x0f04: id = 0x15; v = 0x70; break;
	case 0x0b06: id = 0x16; v = 0x70; break;
	case 0x0605: id = 0x17; v = 0x70; break;
	case 0x0403: id = 0x18; v = 0x70; break;
	case 0x1109: id = 0x19; v = 0x71; break;
	case 0x160c: id = 0x1a; v = 0x71; break;
	case 0x160f: id = 0x1b; v = 0x71; break;
	case 0x1c15: id = 0x1c; v = 0x70; break;
	case 0x1618: id = 0x1d; v = 0x70; break;
	case 0x1719: id = 0x1e; v = 0x71; break;
	case 0x151d: id = 0x1f; v = 0x71; break;
	case 0x131b: id = 0x20; v = 0x71; break;
	case 0x1217: id = 0x21; v = 0x71; break;
	case 0x1111: id = 0x22; v = 0x70; break;
	case 0x0e12: id = 0x23; v = 0x70; break;
	case 0x0e0f: id = 0x24; v = 0x70; break;
	case 0x080e: id = 0x25; v = 0x71; break;
	case 0x0b18: id = 0x26; v = 0x71; break;
	case 0x0719: id = 0x27; v = 0x71; break;
	}
	if (id != -1) {
		Lock lk(id);
		r = 1;
		if (g_lock_cell_bits != -1) {
			event_set_cell_bits(ev, x, y, g_lock_cell_bits);
			AUTOMAP_CELL_WRITE_W(g_map, 0x377, mev->y - g_map_x0, mev->x - g_map_y0, v);
			snd_play(g_snd_fx, 0x61);
		}
	}
	return r;
}

void dng06_beetle_pit(struct area_events far *ev, struct hero far *h)
{
	if (!g_dng06_beetle_pit_fallen) {
		if (g_dng06_beetle_pit_armed != 0 && (group_find_key(g_group, 0x10a) < 0 || group_find_key(g_group,
			0x119) < 0)) {
			if (test_skill(h, TA_SINNESSCHAERFE, 6) >= 0) {
				/* BLUT1.LTX[116]: "%s erkennt vor Euch eine ziemlich gut getarnte Fallgrube." */
				sprintf(g_text_output_buf, ev->txt[116], h->name);
				dlg_message(g_dialog, g_text_output_buf);
			} else {
				group_sub_le_range(g_group, 8, 15);
				/* BLUT1.LTX[117]: "Eben als Ihr den Gang entlanggeht, oeffnet sich unter Euch der Boden und gibt eine Grube
				 * frei. Ihr stuerzt etwa drei Schritt in die Tiefe und sofort koennt Ihr das Euch wohl bekannte Geraeusch
				 * rasselnder Schroeterzangen hoeren. In dem Moment seht Ihr sie auch schon auf Euch zukommen. Zwei Schroeter,
				 * die nicht eben zu den kleinsten ihrer Gattung zaehlen..." */
				dlg_message(g_dialog, ev->txt[117]);
				g_dng06_beetle_pit_fallen = 1;
				event_fight1(ev, 0x0b, 2, 0);
			}
		}
	} else {
		/* BLUT1.LTX[118]: "Vor Euch tut sich eine etwa drei Schritt grosse Grube auf, die Ihr jedoch problemlos umgehen
		 * koennt." */
		dlg_message(g_dialog, ev->txt[118]);
	}
}

void dng06_hornets(struct area_events far *ev)
{
	signed short r;

	/* BLUT1.LTX[28]: "In schweren hoelzernen Kisten verpackt, scheint sich hier ein mechanisches Geraet zu befinden.
	 * Als Ihr die Kisten vorsichtig oeffnet, erkennt Ihr zwei Hornissen und zwei Kisten mit schweren Bolzen. Wissen
	 * die Goetter, wie die Orks an dieses Kriegsgeraet gekommen sind.@Wollt Ihr" */
	/* BLUT1.LTX[29]: "die Hornissen mitnehmen" */
	/* BLUT1.LTX[30]: "die Hornissen zerstoeren" */
	r = dlg_choose(g_dialog, ev->txt[28], 2, ev->txt[29], ev->txt[30]);
	if (r == 1) {
		/* BLUT1.LTX[31]: "Leider ist diese Waffe zu schwer zu transportieren!" */
		dlg_message(g_dialog, ev->txt[31]);
	} else if (r == 2) {
		timewarp_minutes(g_date, 5L);
		g_dng06_hornets_destroyed = 1;
		g_dng06_noise++;
		blut_noise_check(ev, &g_dng06_noise);
		/* BLUT1.LTX[32]: "Mit wenigen, sicheren Schnitten durchtrennt Ihr die Sehnen der beiden riesigen Armbrueste und
		 * blockiert ihren Lademechanismus. Dann deckt Ihr das Segeltuch wieder ueber die Konstruktion." */
		dlg_message(g_dialog, ev->txt[32]);
	}
}

void dng06_dwarf_skeleton(struct area_events far *ev)
{
	struct hero far *h;
	struct hero far *h2;

	/* BLUT1.LTX[38]: "Unter einem Haufen von der Decke herabgestuertztem Geroell koennt Ihr die bleichen Reste eines
	 * Skeletts erkennen.@Wollt Ihr es naeher untersuchen?" */
	if (dlg_yes_no(g_dialog, ev->txt[38]) == 1) {
		/* BLUT1.LTX[39]: "Ihr raeumt den Schutt beiseite... Nach geraumer Zeit habt Ihr den traurigen Haufen Knochen
		 * freigelegt. Es handelt sich offensichtlich um das Skelett eines zwergischen Abenteurers, der hier von einem
		 * Einsturz ueberrascht wurde. Ihr koennt noch die Reste eines Kettenhemdes, ein paar verrottete Lederriemen,
		 * eine zerschlagene Laterne, den Kopf einer Axt und einige Muenzen erkennen.@Wollt Ihr alles Brauchbare
		 * mitnehmen?" */
		if (dlg_yes_no(g_dialog, ev->txt[39]) == 1) {
			/* BLUT1.LTX[40]: "Ihr nehmt etwas Geld, sowie eine alte Axt an Euch." */
			dlg_message(g_dialog, ev->txt[40]);
			group_add_money(g_group, 0x208L);
			group_give_item(g_group, ITEM_WAR_AXE_8, 1);
			g_dng06_skeleton_dug = 1;
			timewarp_minutes(g_date, 0x19L);
		}
		if (group_count_check1(g_group) > 1) {
			h = get_hero_attrib_max(g_group, ATTRIB_TA);
			if (test_attrib(h, ATTRIB_TA, 4)) {
				/* BLUT1.LTX[41]: "Ploetzlich beginnt %s am ganzen Leib zu zittern. Mit weitaufgerissenen Augen starrt %s die
				 * Gebeine des Zwerges an und verkrampft die Haende zu einer Faust, bis die Knoechel weiss hervortreten." */
				sprintf(g_text_output_buf, ev->txt[41], h->name, grammar_pronoun(h->sex, 0, 0));
				dlg_message(g_dialog, g_text_output_buf);
				h2 = get_hero_attrib_max(g_group, ATTRIB_CH);
				if (h2 == h)
					h2 = get_first_active_hero(g_group);
				if (h2 == h)
					h2 = get_second_active_hero(g_group);
				if (test_attrib(h2, ATTRIB_CH, 0) >= 0) {
					/* BLUT1.LTX[43]: "%s versucht, %s gut zuzureden und die Totenangst zu nehmen und tatsaechlich gelingt es %s,
					 * %s langsam wieder zu beruhigen und von dem Anblick der blanken Gebeine wegzufuehren." */
					sprintf(g_text_output_buf, ev->txt[43], h2->name, h->name,
						grammar_pronoun(h2->sex, 3, 0),
						grammar_pronoun(h->sex, 2, 0));
					dlg_message(g_dialog, g_text_output_buf);
				} else {
					/* BLUT1.LTX[42]: "%s versucht, %s gut zuzureden, doch %s scheint durch nichts zu beruhigen zu sein, so bleibt
					 * Euch nichts weiter uebrig, als %s durch einen gut gezielten Schlag wieder zur Vernunft zu bringen." */
					sprintf(g_text_output_buf, ev->txt[42], h2->name, grammar_pronoun(h->sex, 3,
						0), h->name,
						grammar_pronoun(h->sex, 2, 0));
					dlg_message(g_dialog, g_text_output_buf);
					snd_play(g_snd_fx, 0x7c);
					sub_hero_le(h, random_schweif(6));
				}
				timewarp_minutes(g_date, 5L);
			}
		}
	}
}
