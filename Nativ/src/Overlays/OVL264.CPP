/* OVL264 - Overlay O264 (Segment 0x24c7 in SCHWEIF.EXE): Dungeon 7 - das Orkgefaengnis in den Blutzinnen (Texte
 * BLUT2.LTX = Datei 0x374 im Ereignisobjekt c = struct area_events, g_area_descs[7].handler; weitere Teile O262,
 * O265, O266 und O263 group_confiscate). Feld pos = (y - g_map_x0) << 8 | (x - g_map_y0).
 * dng07_handler(c, ev): Art -2 (Feld betreten): Lauschfelder - misslungene Schleichen-Proben (count_failed_skill)
 *   erhoehen den Laerm g_dng07_noise (je Feld x1/x2/x3/x10, danach die leere blut_nop); Versteck [0], Truhe mit der
 *   Ausruestung der Gruppe (Ablage 0x17, [5]/[111]), Blutfleck [21], Hundezwinger [6..8] (Gestank: Wirkung fuer einen
 *   Tag, INGAME_TIMER_DNG07_STENCH; im zweiten Zwinger der rote Ring 0xa5), Stimmen hinter der Tuer [10] (bis
 *   g_dng07_guards_beaten), Fallgruben [22..24] (ohne Alarm g_dng07_alarm bemerkt Sinnesschaerfe +4 sie nur [22],
 *   bei Alarm W6-Schaden und Laerm), Lagerraum [25], Steinschlag
 *   (blut_rockfall, [26]), Madenhaufen [27..29] (Kampf), Hoehle [30], Schacht in der Decke [31..36] (Klettern; Ausbruch
 *   in Dungeon 8), Unrat mit Funden [37..43] (Dolch, Orkschmuck, Goldschluessel, Bronzeflasche), Futterkammer [44]/
 *   [45], Latrine [47], Zellen [49..51]/[58], Gespraeche BLUT.TLK an den Zellen, Spiegelamulett im Stroh [59], die
 *   Leiche mit dem Salamanderstein [105..107] (g_blackmage_done 1 -> 2), Spinnengeruch [82]/[83], Spinnennest
 *   0x225 [91]/[92] (Kampf, Bild SPIDER.BOB, dng07_spider_queen), Nische der Koenigin [98]; danach immer dng07_fights.
 *   Art 1 dng07_door (Schloesser), 100 dng07_straw, 0x65 dng07_shelves (Truhen), 0x66 Treppen [108]/[109] in
 *   Dungeon 6/8, 0x67 Truhen (Chest 0x0f/0x10/0x12) bzw. Ablagen 0x34/0x35, 0x6c dng07_gold_statue, 0x6f Ziehbrunnen
 *   [52]/[53] (LE + 1 einmal am Tag, INGAME_TIMER_WELL_DNG07), 0x76 Eintopf [54..56], 0x77 Kochgeraete [57],
 *   0x78 dng07_web, 0x79 dng07_graffiti, 0x7a dng07_web_corpse.
 * dng07_prison_day(c) (R12 bei g_dng07_prison_event == 1): Tag in der Gefaengniszelle g_dng07_prison_days 1..6 -
 *   [99..104] (am ersten Tag group_confiscate, am sechsten vier Gegenstaende: Skraja, schwerer Dolch, roter
 *   Bogenschluessel, Fackel), alle Gruppen Hunger/Durst 0, ein Tag vergeht; nach dem sechsten Tag zurueckgesetzt.
 * dng07_spider_queen(ev): die Hoehlenspinnenkoenigin [93] - ohne Helden mit Bit hero.bd2 (BE: Duplicatus) sterben
 *   alle [94]; haben es alle Lebenden, wird sie besiegt [96]/[97], sonst sterben die anderen [95]; LE - 50 (nicht
 *   unter 10) und je 50 AP. Danach das 3D-Fenster neu (Bmp 0xd0 x 0x87, BLIT_BEHIND).
 *
 * BCC -mlarge -3 -Ot -Oi -c -Y OVL264.CPP   -> 6640/6640 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (g_ingame_timers + 0x80/+0x8c); mit -Og ebenfalls gleich. (Namensfassung mit schweif.h: 19.10.2026 GLEICH,
 *   LINKGLEICH)
 * - register-Variable v (Gegenstand / Truhennummer) nur in den inneren Bloecken in SI: das strcat-Intrinsic
 *   (pos 0x1118) sperrt SI/DI fuer register-Variablen der Funktionsebene (Leitfaden Welle Q/AM).
 * - dng07_prison_day: "g_dng07_prison_days++; switch (g_dng07_prison_days)" (inc byte / mov al / cbw); "switch
 *   (++g_dng07_prison_days)" ergibt mov/inc al/mov.
 * - Listenzeiger/Textzeiger p teilen sich [bp-10h] -> Block-Locals; das 0x56-Byte-Truhenobjekt liegt am Ende.
 * Auffaellig (Original): Position 0x090f steht in zwei Bedingungen (die zweite, Laerm * 10, ist dafuer
 *   unerreichbar); die Latrine 0xd1d setzt ihren Merker wieder auf 0 statt 1 (Text kommt jedes Mal); die Funde im
 *   Unrat zeigen immer BLUT2.LTX[40] "einen Dolch" (statt [40..43]); bei g_dng07_junk_finds >= 4 laeuft der switch
 *   ohne passenden Fall -> p und v uninitialisiert (sprintf/group_give_item mit Zufallswerten), der Zaehler laeuft
 *   weiter; der Brunnen setzt die Zeitmarke auf 0x1fa40, uebergibt aber 0x546 (aus dem Zwinger kopiert?); das
 *   abschliessende "ret = dng07_fights(c, pos)" im Zweig -2 ueberschreibt jedes ret = 1 davor.
 */
#include <stdio.h>
#include <string.h>
#include "schweif.h"

#define FILE_BLUT2	0x374		/* BLUT2.LTX */

void dng07_spider_queen(void far *ev);

signed short dng07_handler(struct area_events far *c, struct map_event far *ev)
{
	signed short type;
	signed short pos;
	struct hero far *h;
	signed short n;
	signed short ret;
	char chest[0x56];		/* Truhenobjekt (O078 Chest_ctor/Chest_dtor) */

	type = ev->type;
	pos = (ev->y - g_map_x0) << 8;
	pos |= ev->x - g_map_y0;
	h = get_first_active_hero(g_group);
	ret = 0;
	if (type == -2) {
		if (pos == 0x100d || pos == 0x120d || pos == 0x140d || pos == 0x307 || pos == 0x317) {
			n = count_failed_skill(g_group, TA_SCHLEICHEN, 0);
			g_dng07_noise += n;
			blut_nop();
		} else if (pos == 0x71b) {
			n = count_failed_skill(g_group, TA_SCHLEICHEN, -2);
			g_dng07_noise += n * 2;
			blut_nop();
		} else if (pos == 0xb0f || pos == 0x90f) {
			n = count_failed_skill(g_group, TA_SCHLEICHEN, 2);
			g_dng07_noise += n * 3;
			blut_nop();
		} else if (pos == 0xa0f || pos == 0x90f) {
			n = count_failed_skill(g_group, TA_SCHLEICHEN, 4);
			g_dng07_noise += n * 10;
			blut_nop();
		} else if (pos == 0x170f) {
			/* BLUT2.LTX[0]: "In dieser finsteren Ecke - offensichtlich wurde hier nicht weitergegraben - koenntet Ihr Euch
			 * im aeussersten Notfall verstecken." */
			dlg_message(g_dialog, c->txt[0]);
			ret = 1;
		} else if (pos == 0xd01) {
			void far *lst;

			lst = Stash_ctor((void far *)0, 0x17, 1);
			if (stash_has_items(lst) == 1) {
				/* BLUT2.LTX[5]: "Hier liegen einige Waffen, die wohl fuer die Arenakaempfer gedacht sind.@Wollt Ihr sie naeher
				 * untersuchen?" */
				if (dlg_yes_no(g_dialog, c->txt[5]) == 1)
					stash_show(lst);
			} else
				/* BLUT2.LTX[111]: "Hier liegen noch immer ein paar alte, rostige und zum Teil zerbrochene Waffen herum, die
				 * aber allesamt unbrauchbar sind." */
				dlg_message(g_dialog, c->txt[111]);
			Stash_dtor(lst, 3);
			ret = 1;
		} else if (pos == 0xf02) {
			/* BLUT2.LTX[21]: "Hier hat es am Boden einen riesigen getrockneten Blutfleck." */
			dlg_message(g_dialog, c->txt[21]);
			ret = 1;
		} else if (pos == 0xa07 && !g_dng07_kennel1_searched) {
			/* BLUT2.LTX[6]: "Ihr steht in einem kleinen Raum, der goettererbaermlich nach Hunden, fauligem Fleisch und
			 * Schlimmerem stinkt.@Wollt Ihr den Raum wirklich durchsuchen?" */
			if (dlg_yes_no(g_dialog, c->txt[6]) == 1) {
				timewarp_minutes(g_date, 10L);
				g_dng07_kennel1_searched = 1;
				/* BLUT2.LTX[7]: "Ihr wuehlt Euch durch Hundekacke, Knochenreste - nicht nur von Tieren - und halb angefressene
				 * anatomische Details, koennt jedoch nichts Besonderes entdecken, habt Euch dabei jedoch von oben bis unten
				 * mit Fluessigkeiten und Schleim, dessen genaue Konsistenz Euch lieber unbekannt bleibt, bekleckert." */
				dlg_message(g_dialog, c->txt[7]);
				if (g_ingame_timers[INGAME_TIMER_DNG07_STENCH] == 0) {
					group_add_effect(g_group, 0x1fa40L, 2, -2, 0);
					g_ingame_timers[INGAME_TIMER_DNG07_STENCH] = 0x1fa40L;
				}
			}
			ret = 1;
		} else if (pos == 0x1007 && !g_dng07_kennel2_searched) {
			if (dlg_yes_no(g_dialog, c->txt[6]) == 1) {
				timewarp_minutes(g_date, 10L);
				/* BLUT2.LTX[8]: "Ihr wuehlt Euch durch Dinge, die hier besser ungenannt bleiben, versaut Euch dabei von oben
				 * bis unten und macht dabei eine makabre Entdeckung. Die Ueberreste einer angefressenen - offensichtlich
				 * elfischen - Hand, an der noch ein Ring steckt.@Wollt Ihr den Ring an Euch nehmen?" */
				if (dlg_yes_no(g_dialog, c->txt[8]) == 1) {
					g_dng07_kennel2_searched = 1;
					group_give_item(g_group, 0xa5, 1);
				}
				if (g_ingame_timers[INGAME_TIMER_DNG07_STENCH] == 0) {
					group_add_effect(g_group, 0x1fa40L, 2, -2, 0);
					g_ingame_timers[INGAME_TIMER_DNG07_STENCH] = 0x1fa40L;
				}
			}
			ret = 1;
		} else if (pos == 0xd07 && !g_dng07_guards_beaten) {
			/* BLUT2.LTX[10]: "Hinter der grossen Tuer im Norden koennt Ihr deutlich Stimmen vernehmen." */
			dlg_message(g_dialog, c->txt[10]);
			ret = 1;
		} else if (pos == 0xd18) {
			if (!g_dng07_alarm) {
				if (test_skill(h, TA_SINNESSCHAERFE, 4) > 0) {
					/* BLUT2.LTX[22]: "Hier scheint sich - von den Ritzen im Boden zu schliessen - eine Grubenfalle zu befinden." */
					dlg_message(g_dialog, c->txt[22]);
					ret = 1;
				}
			} else {
				if (g_dng07_pit1_fallen == 1) {
					/* BLUT2.LTX[23]: "Vor Euch tut sich eine etwa vier Schritt tiefe Grube auf, die gluecklicherweise mit
					 * fauligme Stroh gepolstert ist. Dennoch kostet es Euch einige Zeit, die Grube sicher zu ueberqueren." */
					dlg_message(g_dialog, c->txt[23]);
					timewarp_minutes(g_date, 5L);
				} else {
					g_dng07_noise += g_group->nr_heroes * 3;
					/* BLUT2.LTX[24]: "Ploetzlich gibt der Boden unter Euch nach und Ihr stuerzt in eine etwa vier Schritt tiefe
					 * Fallgrube. Zum Glueck ist sie mit Stroh ausgepolstert, aber dennoch habt Ihr Euch bei dem Sturz leicht
					 * verletzt." */
					dlg_message(g_dialog, c->txt[24]);
					group_sub_le_range(g_group, 1, 6);
					timewarp_minutes(g_date, 15L);
					event_set_cell(c, 0x18, 0xd, 0xd, 0, -1);
					g_dng07_pit1_fallen = 1;
				}
				ret = 1;
			}
		} else if (pos == 0xd14) {
			g_dng07_store_seen = 0;
		} else if (pos == 0xf14 && !g_dng07_store_seen) {
			/* BLUT2.LTX[25]: "Ihr steht in einem modrigen, wohl seit laengerem nicht mehr benutzten Lagerraum." */
			dlg_message(g_dialog, c->txt[25]);
			g_dng07_store_seen = 1;
			ret = 1;
		} else if (pos == 0x1616) {
			/* BLUT2.LTX[26]: "Von der Decke prasseln Steinbrocken auf Euch herab!" */
			ret = blut_rockfall(c, &g_dng07_rockfall1, -4, 3, 1, 3, 5, 10, c->txt[26]);
		} else if (pos == 0x1817 && !g_dng07_maggots_done) {
			/* BLUT2.LTX[27]: "Auf dem Boden koennt Ihr einen Haufen Unrat erkennen, in dem sich offensichtlich etwas
			 * bewegt.@Wollt Ihr ihn naeher untersuchen?" */
			if (dlg_yes_no(g_dialog, c->txt[27]) == 1) {
				/* BLUT2.LTX[28]: "Ihr schiebt den Muell ein wenig zur Seite und erkennt, dass es sich um fette, fingerlange,
				 * graue Maden handelt..." */
				dlg_message(g_dialog, c->txt[28]);
				/* BLUT2.LTX[29]: "In diesem Moment hoert Ihr ein Schaben hinter Euch!" */
				dlg_message(g_dialog, c->txt[29]);
				if (!event_fight1(c, 0xb, 2, 0))
					g_dng07_maggots_done = 1;
			}
			ret = 1;
		} else if (pos == 0x171a) {
			g_dng07_cave_seen = 0;
		} else if (pos == 0x181a && !g_dng07_cave_seen) {
			/* BLUT2.LTX[30]: "Vor Euch oeffnet sich eine grosse, natuerliche Hoehle. Ein beissender Gestank dringt in Eure
			 * Nasen." */
			dlg_message(g_dialog, c->txt[30]);
			g_dng07_cave_seen = 1;
			ret = 1;
			if (random_schweif(100) <= 20)
				event_fight1(c, 0xb, 2, 0);
		} else if (pos == 0x1e18) {
			/* BLUT2.LTX[31]: "In etwa dreieinhalb Schritt Hoehe entdeckt Ihr eine Oeffnung in der Decke.@Wollt Ihr" */
			/* BLUT2.LTX[32]: "hinaufklettern" */
			/* BLUT2.LTX[33]: "genauer nachsehen" */
			n = dlg_choose(g_dialog, c->txt[31], 2, c->txt[32], c->txt[33]);
			if (n == 1) {
				if (count_failed_skill(g_group, TA_KLETTERN, 8) <= 0) {
					/* BLUT2.LTX[110]: "Mit Muehe zwaengt Ihr Euch durch den engen Schacht und nach einiger Zeit kommt ihr -
					 * hoellisch verschmutzt und stinkend zwar - in einem anderen Raum heraus. Irgendwie werdet Ihr das Gefuehl
					 * nicht los, dass ihr eben durch eine Kloake geklettert seid!" */
					dlg_message(g_dialog, c->txt[110]);
					g_group->old_dungeon = g_group->dungeon;
					g_group->dungeon = 8;
					group_set_screen_pos(0x16, 0x1d);
					group_add_effect(g_group, 0x3f480L, 2, -4, 0);
				} else if (count_failed_skill(g_group, TA_KLETTERN, 2) <= 0) {
					/* BLUT2.LTX[35]: "Ihr schafft es nicht, da hinaufzukommen und bei dem Versuch, eine Pyramide zu bilden,
					 * stuerzt Ihr alle zu Boden." */
					dlg_message(g_dialog, c->txt[35]);
					group_sub_le_range(g_group, 1, 3);
				} else {
					/* BLUT2.LTX[36]: "Mit Muehe zwaengt Ihr Euch durch den engen Schacht, doch %s rutscht ab, schuerft sich
					 * heftig die Haut und reisst Euch alle wieder in die Tiefe..." */
					sprintf(g_text_output_buf, c->txt[36], h->name);
					dlg_message(g_dialog, g_text_output_buf);
					sub_hero_le(h, 2);
					group_sub_le_range(g_group, 1, 3);
				}
			} else if (n == 2) {
				timewarp_minutes(g_date, 5L);
				/* BLUT2.LTX[34]: "Jenseits der Oeffnung scheint sich ein schmaler Schacht nach oben zu winden." */
				dlg_message(g_dialog, c->txt[34]);
			}
			ret = 1;
		} else if (pos == 0x1a15 || pos == 0x171d) {
			/* BLUT2.LTX[37]: "In eine grossen Haufen Unrat, wimmeln etwa ein Dutzend fingerlange, graue Maden herum.@Wollt
			 * Ihr den Haufen dennoch durchsuchen?" */
			if (dlg_yes_no(g_dialog, c->txt[37]) == 1) {
#if FIX_0199
				/* Fix #199: nach vier Funden nichts mehr finden (kein switch ohne Fall mit Muellwerten) */
				if (g_dng07_junk_finds >= 4 || random_schweif(100) <= 95)
#else
				if (g_dng07_junk_finds < 4 && random_schweif(100) <= 95)
#endif
					/* BLUT2.LTX[38]: "Ihr stochert ein wenig in dem Haufen herum, koennt aber nichts von Interesse entdecken." */
					dlg_message(g_dialog, c->txt[38]);
				else {
					char far *p;
					register signed short v;

					switch (g_dng07_junk_finds) {
					case 0:
						/* BLUT2.LTX[40]: "einen Dolch" */
						p = c->txt[40];
						v = 0x0e;
						break;
#if FIX_0218
					/* Fix #218: Fundtext BLUT2.LTX[41] */
					case 1:
						p = c->txt[41];
#else
					case 1:
						p = c->txt[40];
#endif
						v = 0x113;
						break;
#if FIX_0218
					/* Fix #218: Fundtext BLUT2.LTX[42] */
					case 2:
						p = c->txt[42];
#else
					case 2:
						p = c->txt[40];
#endif
						v = 0xd3;
						break;
#if FIX_0218
					/* Fix #218: Fundtext BLUT2.LTX[43] */
					case 3:
						p = c->txt[43];
#else
					case 3:
						p = c->txt[40];
#endif
						v = 0x2a;
						break;
					}
					/* BLUT2.LTX[39]: "Zu Eurer grossen Ueberraschung findet Ihr beim Herumstochern %s." */
					sprintf(g_text_output_buf, c->txt[39], p);
					dlg_message(g_dialog, g_text_output_buf);
					group_give_item(g_group, v, 1);
					g_dng07_junk_finds++;
				}
			}
			ret = 1;
		} else if (pos == 0x1a1a || pos == 0x1c1d) {
			ret = blut_rockfall(c, &g_dng07_rockfall2, -2, 3, 2, 4, 6, 12, c->txt[26]);
		} else if (pos == 0xc0a) {
			g_dng07_feed_room_seen = 0;
		} else if (pos == 0xb0a && !g_dng07_feed_room_seen) {
			/* BLUT2.LTX[44]: "In diesem kleinen Raum wird offensichtlich Tierfutter gelagert. In Troegen entlang der Wand
			 * liegen angefaultes Fleisch, Gemuesereste, schimmeliges Korn und erschlagene Ratten. Die Tiere, an die so
			 * etwas verfuettert wird, duerfen nicht sehr waehlerisch bei der Wahl Ihrer Nahrung sein...@Wollt Ihr das Zeug
			 * naeher untersuchen?" */
			if (dlg_yes_no(g_dialog, c->txt[44]) == 1)
				/* BLUT2.LTX[45]: "Obwohl Ihr selbst die fauligen Essensreste mit geruempfter Nase beiseiteschiebt, findet Ihr
				 * nichts von Interesse. Andererseits ist dieser Raum so uninteressant und so leicht zu ueberblicken, dass er
				 * sich fast schon wieder als ideales Versteck eignet..." */
				dlg_message(g_dialog, c->txt[45]);
			g_dng07_feed_room_seen = 1;
			ret = 1;
		} else if (pos == 0xd1c) {
			g_dng07_latrine_seen = 0;
		} else if (pos == 0xd1d && !g_dng07_latrine_seen) {
			/* BLUT2.LTX[47]: "Dieser Raum dient offensichtlich als Latrine, was sowohl die Loecher im Boden, als auch der
			 * Gestank bezeugen." */
			dlg_message(g_dialog, c->txt[47]);
			g_dng07_latrine_seen = 0;
			ret = 1;
		} else if (pos == 0x312) {
			if (!g_dng07_alarm) {
				if (test_skill(h, TA_SINNESSCHAERFE, 4) > 0) {
					dlg_message(g_dialog, c->txt[22]);
					ret = 1;
				}
			} else {
				if (g_dng07_pit2_fallen == 1) {
					dlg_message(g_dialog, c->txt[23]);
					timewarp_minutes(g_date, 5L);
				} else {
					g_dng07_noise += g_group->nr_heroes * 3;
					dlg_message(g_dialog, c->txt[24]);
					group_sub_le_range(g_group, 1, 6);
					timewarp_minutes(g_date, 15L);
					event_set_cell(c, 0x12, 3, 0xd, 0, -1);
					g_dng07_pit2_fallen = 1;
				}
				ret = 1;
			}
		} else if (pos == 0x30c) {
			if (!g_dng07_alarm) {
				if (test_skill(h, TA_SINNESSCHAERFE, 4) > 0) {
					dlg_message(g_dialog, c->txt[22]);
					ret = 1;
				}
			} else {
				if (g_dng07_pit3_fallen == 1) {
					dlg_message(g_dialog, c->txt[23]);
					timewarp_minutes(g_date, 5L);
				} else {
					g_dng07_noise += g_group->nr_heroes * 3;
					dlg_message(g_dialog, c->txt[24]);
					group_sub_le_range(g_group, 1, 6);
					timewarp_minutes(g_date, 15L);
					event_set_cell(c, 0xc, 3, 0xd, 0, -1);
					g_dng07_pit3_fallen = 1;
				}
				ret = 1;
			}
		} else if (pos == 0x507 || pos == 0x511) {
			/* BLUT2.LTX[58]: "Ihr steht in einer sehr engen Kammer, die offensichtlich als Gefaengniszelle dient. Auf dem
			 * Boden ist etwas Stroh verteilt, in einer Ecke steht ein Eimer." */
			dlg_message(g_dialog, c->txt[58]);
			ret = 1;
		} else if (pos == 0x50b) {
			/* BLUT2.LTX[50]: "Ihr steht in einem kleinen Raum, der als Gefaengniszelle hergerichtet ist: fauliges Stroh auf
			 * dem Boden, ein uebel riechender Eimer fuer die Notdurft, ein Fackelhalter und ein paar Kritzeleien an den
			 * Waenden. An einer Stelle der Wand hat wohl jemand versucht, einen Stein aus der Wand zu loesen." */
			dlg_message(g_dialog, c->txt[50]);
			ret = 1;
		} else if (pos == 0x515) {
			if (g_dng07_was_prisoner != 0)
				/* BLUT2.LTX[51]: "Ihr steht in einem kleinen Raum, der als Gefaengniszelle hergerichtet ist: fauliges Stroh
				 * auf dem Boden, ein uebel riechender Eimer fuer die Notdurft, ein Fackelhalter und ein paar Kritzeleien an
				 * den Waenden. Dies ist der Raum in dem Ihr gefangen gehalten wurdet. Ihr glaubt, mittlerweile jeden
				 * Rechtspann Eurer ehemaligen Zelle zu kennen." */
				dlg_message(g_dialog, c->txt[51]);
			else
				dlg_message(g_dialog, c->txt[50]);
			ret = 1;
		} else if (pos == 0x10d || pos == 0x10e) {
			if (!g_dng07_talk1_done) {
				talk(&g_talk, 0x3d5, 0, 0);
				g_dng07_talk1_done = 1;
				ret = 1;
			}
		} else if (pos == 0x113 || pos == 0x114) {
			if (!g_dng07_talk2_done) {
				talk(&g_talk, 0x3d5, 1, 0);
				g_dng07_talk2_done = 1;
				ret = 1;
			}
		} else if (pos == 0x111 && !g_dng07_amulet_found) {
			/* BLUT2.LTX[59]: "Nach einigem Wuehlen im fauligen Stroh entdeckt Ihr ein kleines, silbernes Amulett." */
			dlg_message(g_dialog, c->txt[59]);
			group_give_item(g_group, 0x141, 1);
			g_dng07_amulet_found = 1;
		} else if (pos == 0x116 || pos == 0x117) {
			if (g_blackmage_done != 1)
				dlg_message(g_dialog, c->txt[58]);
			/* BLUT2.LTX[105]: "Schon als Ihr den Raum betretet steigt Euch ein suesslicher Geruch in die Nase, der Euch den
			 * Atem raubt. Es ist Verwesungsgeruch und als Ihr um die Ecke seht, entdeckt Ihr am Boden eine Leiche.@Wollt
			 * Ihr den toten Koerper naeher untersuchen?" */
			else if (dlg_yes_no(g_dialog, c->txt[105]) == 1) {
				/* BLUT2.LTX[106]: "Ihr dreht die Leiche um und erkennt das Gesicht eines Mannes, der wohl verdurstet ist.@<Den
				 * Burschen kenn' ich doch irgendwoher!> meint %s. <Klar, das ist einer der Schwarzmagier, die uns vor Lowangen
				 * den Salamanderstein abgenommen haben!>@Wollt Ihr den Toten durchsuchen?" */
				sprintf(g_text_output_buf, c->txt[106], h->name);
				if (dlg_yes_no(g_dialog, g_text_output_buf) == 1) {
					/* BLUT2.LTX[107]: "Nach Kurzem entdeckt Ihr in einem kleinen Taeschchen, das in die Innenseite seines Mantels
					 * genaeht wurde, den Salamanderstein!@<Was fuer ein Glueck!> entfaehrt es jemandem." */
					dlg_message(g_dialog, c->txt[107]);
					group_give_item(g_group, 0x117, 1);
					g_blackmage_done = 2;
				}
			}
			ret = 1;
		} else if (pos == 0x110 || pos == 0x10a || pos == 0x107) {
			/* BLUT2.LTX[49]: "Ihr steht in einem kleinen Raum, der als Gefaengniszelle hergerichtet ist: fauliges Stroh auf
			 * dem Boden, ein uebel riechender Eimer fuer die Notdurft, ein Fackelhalter und ein paar Kritzeleien an den
			 * Waenden." */
			dlg_message(g_dialog, c->txt[49]);
			ret = 1;
		} else if (pos == 0x1118) {
			/* BLUT2.LTX[82]: "Hier ist der Boden mit Geroell uebersaet und ein seltsamer, scharfer Geruch liegt in der
			 * Luft." */
			sprintf(g_text_output_buf, c->txt[82]);
#if FIX_0366
			/* Fix #366: warnen, wenn mindestens ein Held die Probe schafft (nicht, wenn einer scheitert) */
			if (count_failed_skill(g_group, TA_TIERKUNDE, 6) < (signed char)g_group->nr_heroes) {
#else
			if (count_failed_skill(g_group, TA_TIERKUNDE, 6) > 0) {
#endif
				h = get_best_hero_skill(g_group, TA_TIERKUNDE);
				/* BLUT2.LTX[83]: "@<Ich rieche Hoehlenspinnen!> warnt Euch %s. <Das hat uns zu unserem Glueck gerade noch
				 * gefehlt!>" */
				sprintf(g_text_buf2, c->txt[83], h->name);
				strcat(g_text_output_buf, g_text_buf2);
			}
			dlg_message(g_dialog, g_text_output_buf);
			ret = 1;
		} else if (pos == 0xe22) {
			ret = blut_rockfall(c, &g_dng07_rockfall3, -4, 3, 1, 4, 5, 8, c->txt[26]);
		} else if (pos == 0xa27 || pos == 0xb27) {
			ret = blut_rockfall(c, &g_dng07_rockfall4, -4, 3, 8, 8, 8, 15, c->txt[26]);
		} else if (pos == 0x225 && !g_dng07_nest_destroyed) {
			/* BLUT2.LTX[91]: "Ihr steht vor einem Nest aus Abfall, in dem Dutzende von kleinen Spinnen herumkrabbeln.@Wollt
			 * Ihr dieses Nest zerstoeren?" */
			if (dlg_yes_no(g_dialog, c->txt[91]) == 1) {
				/* BLUT2.LTX[92]: "Ihr benoetigt geraume Zeit, aber schliesslich habt Ihr alle Spinnen zertrampelt und das Nest
				 * bis zur Unkenntlichkeit zerwuehlt. Ob dies allerdings die Beste Idee war, dessen seid Ihr Euch nicht mehr so
				 * sicher, denn schon hoert Ihr ein seltsam pfeifendes Geraeusch hinter Euch und fahrt herum...@<Die Koenigin!>
				 * entfaehrt es einem von Euch." */
				dlg_message(g_dialog, c->txt[92]);
				g_dng07_nest_destroyed = 1;
				event_set_cell(c, 0x26, 1, 0, 0, -1);
				automap_cell_write(g_map, 0x37b, 1, 0x26, 3);
				swap_out_screen();
				ani_set_pal_size(g_screen_pic, 0xa0);
				anim_load(g_screen_pic, 0x3d6, 1);
				ani_set_pal_size(g_screen_pic, 0x40);
				dng07_spider_queen(g_area_events);
			}
		} else if (pos == 0x126 && !g_dng07_niche_looted) {
			/* BLUT2.LTX[98]: "Ihr steht hier in einer kleinen Nische, die ueber und ueber mit seidigen Faeden versponnen
			 * ist. Das muss der Aufenthaltsort der Koenigin gewesen sein, denn Ihr koennt Euch nicht daran erinnern, diese
			 * Nische vorher schon gesehen zu haben. Als Ihr Euch naeher umseht, entdeckt Ihr am Boden einige Juwelen und
			 * einen seltsamen Schluessel. Sachen, die Ihr sofort mitnehmt." */
			dlg_message(g_dialog, c->txt[98]);
			group_give_item(g_group, 0x4a, 1);
			group_give_item(g_group, 0xd4, 1);
			g_dng07_niche_looted = 1;
		}
		ret = dng07_fights(c, pos);
	} else if (type == 1) {
		ret = dng07_door(c, pos, ev->x, ev->y, ev);
	} else if (type == 100) {
		dng07_straw(c, h, pos);
		ret = 1;
	} else if (type == 0x65) {
		ret = dng07_shelves(c, pos);
	} else if (type == 0x66) {
		if (pos == 0x1401) {
			/* BLUT2.LTX[109]: "Vorsichtig steigt Ihr die Treppe hinab..." */
			dlg_message(g_dialog, c->txt[109]);
			g_group->old_dungeon = g_group->dungeon;
			g_group->dungeon = 6;
			group_set_screen_pos(1, 2);
			g_group->dir = 0;
		} else if (pos == 0x1b06) {
			/* BLUT2.LTX[108]: "Behutsam schleicht Ihr die Treppe hinauf..." */
			dlg_message(g_dialog, c->txt[108]);
			g_group->old_dungeon = g_group->dungeon;
			g_group->dungeon = 8;
			group_set_screen_pos(0x19, 2);
			g_group->dir = 2;
		} else if (pos == 0x1c10) {
			dlg_message(g_dialog, c->txt[108]);
			g_group->old_dungeon = g_group->dungeon;
			g_group->dungeon = 8;
			group_set_screen_pos(0x16, 0xd);
			g_group->dir = 1;
		} else if (pos == 0x1c12) {
			dlg_message(g_dialog, c->txt[109]);
			g_group->old_dungeon = g_group->dungeon;
			g_group->dungeon = 6;
			group_set_screen_pos(8, 0x12);
			g_group->dir = 1;
		}
	} else if (type == 0x67) {
		void far *lst;
		register signed short v;

		v = -1;
		switch (pos) {
		case 0x110b:
			v = 0x0f;
			break;
		case 0x1310:
			v = 0x10;
			break;
		case 0x1013:
			/* BLUT2.LTX[115]: "Den Inhalt der Truhe koennte man bestenfalls mit <Plunder> umschreiben: mottenzerfressene
			 * Decken, rostige Helme, abgebrochene Hammerkoepfe und dergleichen mehr." */
			dlg_message(g_dialog, c->txt[115]);
			break;
		case 0x503:
			v = 0x12;
			break;
		case 0xa12:
			lst = Stash_ctor((void far *)0, 0x35, 0);
			Stash_dtor(lst, 3);
			ret = 1;
			break;
		case 0x1b02:
			lst = Stash_ctor((void far *)0, 0x34, 0);
			Stash_dtor(lst, 3);
			ret = 1;
			break;
		}
		if (v != -1) {
			Chest_ctor(chest, v, 1);
			ret = 1;
			Chest_dtor(chest, 2);
		}
	} else if (type == 0x6c) {
		dng07_gold_statue(c);
		ret = 1;
	} else if (type == 0x6f) {
		if (pos == 0x11a) {
			/* BLUT2.LTX[52]: "Hier befindet sich ein grosser Ziehbrunnen.@Wollt Ihr etwas trinken und Eure Wasserschlaeuche
			 * auffuellen?" */
			if (dlg_yes_no(g_dialog, c->txt[52]) == 1) {
				timewarp_minutes(g_date, 5L);
				snd_play(g_snd_fx, 0x5d);
				/* BLUT2.LTX[53]: "Echt lecker und erfrischend!" */
				dlg_message(g_dialog, c->txt[53]);
				group_refill_water(g_group);
				if (g_ingame_timers[INGAME_TIMER_WELL_DNG07] == 0) {
					group_add_le(g_group, 1);
					group_add_effect(g_group, 0x546L, 0, 1, 0);
					g_ingame_timers[INGAME_TIMER_WELL_DNG07] = 0x1fa40L;
				}
			}
			ret = 1;
		}
	} else if (type == 0x76) {
		/* BLUT2.LTX[54]: "In diesem Kessel blubbert irgendeine Art von Eintopf vor sich hin.@Wollt Ihr diesen Orkenfrass
		 * etwa probieren?" */
		if (dlg_yes_no(g_dialog, c->txt[54]) == 1) {
			if (!g_dng07_stew_eaten) {
				/* BLUT2.LTX[55]: "Naja, das ist nicht eben eine Schlemmermahl, aber es saettigt wenigstens und schmeckt fuer
				 * orkische Verhaeltnisse ausgezeichnet." */
				dlg_message(g_dialog, c->txt[55]);
				g_dng07_stew_eaten = 1;
				group_clr_hunger(g_group);
			} else
				/* BLUT2.LTX[56]: "Diesen Topf haben wir vorhin bereits ausgefressen." */
				dlg_message(g_dialog, c->txt[56]);
		}
		ret = 1;
	} else if (type == 0x77) {
		if (!g_dng07_kitchen_looted) {
			/* BLUT2.LTX[57]: "Auf dem Tisch liegen einige Kochgeraete, einige Nahrungsmittel und einige grosse
			 * Messer.@Wollt Ihr alles Brauchbare mitnehmen?" */
			if (dlg_yes_no(g_dialog, c->txt[57]) == 1) {
				group_give_item(g_group, 0x2d, 1);
				group_give_item(g_group, 0x40, 1);
				group_give_item(g_group, 4, 2);
				group_give_item(g_group, 0x0e, 1);
				g_dng07_kitchen_looted = 1;
			}
			ret = 1;
		}
	} else if (type == 0x78) {
		dng07_web(c, pos);
		ret = 1;
	} else if (type == 0x79) {
		dng07_graffiti(c);
		ret = 1;
	} else if (type == 0x7a) {
		dng07_web_corpse(c);
		ret = 1;
	}
	return ret;
}

void dng07_prison_day(struct area_events far *c)
{
	signed short old;
	char far *msg;
	struct group far *g;
	register signed short i;

	old = dlg_set16(g_dialog, 5);
	g_dng07_prison_days++;
	switch (g_dng07_prison_days) {
	case 1:
		/* BLUT2.LTX[99]: "Ihr befindet Euch in einer Gefaengniszelle. Gegen Abend bringen zwei schwer bewaffnete Orks
		 * eine nicht sonderlich wohlschmeckende, aber halbwegs nahrhafte Mahlzeit." */
		msg = c->txt[99];
		group_confiscate(c);
		break;
	case 2:
		/* BLUT2.LTX[100]: "Ihr befindet Euch noch immer in einer Gefaengniszelle. Gegen Abend bringen zwei schwer
		 * bewaffnete Orks eine nicht sonderlich wohlschmeckende, aber halbwegs nahrhafte Mahlzeit.@Als Ihr den Raum
		 * untersucht, stellt ihr fest, dass die Tuer massiv ist und von aussen teilweise mit Metall verstaerkt wurde. In
		 * der Nordostecke ist ein ehemaliger Gefangener mit seinem Ausbruchversuch besonders weit gekommen: Ein Stein
		 * laesst sich komplett aus der Wand loesen. Dahinter findet Ihr die rostigen Reste des Dolches, mit dem die
		 * Grabarbeit wohl vonstatten ging." */
		msg = c->txt[100];
		break;
	case 3:
		/* BLUT2.LTX[101]: "Ihr befindet Euch seit zwei Tagen in einer Gefaengniszelle. Gegen Abend bringt Euch ein
		 * schwer bewaffneter Ork eine nicht sonderlich wohlschmeckende, aber halbwegs nahrhafte Mahlzeit. Ein weiterer
		 * Ork wartet auf dem Gang.@Im Verlauf der letzten Tage glaubt Ihr einen bestimmten Rhythmus ausgemacht zu haben.
		 * Ausserdem vermeint Ihr in regelmaessigen Abstaenden Klopfzeichen gehoert zu haben: 2 Lang, 2 Kurz und 3 Kurz,
		 * 2 Lang." */
		msg = c->txt[101];
		break;
	case 4:
		/* BLUT2.LTX[102]: "Ihr befindet Euch in einer Gefaengniszelle und wisst immer noch nicht, was mit Euch geschehen
		 * soll. Gegen Abend bringt Euch ein schwer bewaffneter Ork eine nicht sonderlich wohlschmeckende, aber halbwegs
		 * nahrhafte Mahlzeit. Ein weiterer Ork wartet auf dem Gang.@Ihr habt in der Tat eine bestimmte Regelmaessigkeit
		 * feststellen koennen. Euer Essen wird jeweils kurz vor einem Wachwechsel gebracht. Ausser Euren beiden
		 * Zellenwaechtern scheint sich dann niemand auf dem Gang aufzuhalten. Ausserdem koennt Ihr die Klopfzeichen
		 * bestaetigen: 2 Lang, 2 Kurz und 3 Kurz, 2 Lang." */
		msg = c->txt[102];
		break;
	case 5:
		/* BLUT2.LTX[103]: "Ihr befindet Euch in einer Gefaengniszelle und gruebelt ueber Euer Schicksal nach. Heute
		 * konntet Ihr die Klopfzeichen nur undeutlich vernehmen. Dafuer seid ihr mittlerweile der Meinung, dass Eure
		 * Zelle an einem langen Gang liegt. In einigen Zellen scheinen sich weitere Gefangene zu befinden.@Gegen Abend
		 * bringt Euch ein schwer bewaffneter Ork eine nicht sonderlich wohlschmeckende, aber halbwegs nahrhafte
		 * Mahlzeit. Ein weiterer Ork wartet auf dem Gang." */
		msg = c->txt[103];
		break;
	default:
		/* BLUT2.LTX[104]: "Ihr befindet Euch in einer goetterverdammten, beschissenen, orkischen Gefaengniszelle und
		 * wisst nur eins: Ihr muesst raus hier... irgendwie!@Gegen Abend bringt Euch ein schwer bewaffneter Ork eine
		 * nicht sonderlich wohlschmeckende, aber halbwegs nahrhafte Mahlzeit.@In einem Akt der Verzweiflung
		 * ueberwaeltigt Ihr Euren Waechter. Noch waehrend Ihr ueber seine Leiche steigt, haltet Ihr seinen Arbach und
		 * Dolch in der Hand. Ihr befindet Euch tief im Inneren einer Orkenfestung. Ihr muesst fliehen - oder bei einem
		 * Versuch umkommen..." */
		msg = c->txt[104];
		group_give_item(g_group, 0x86, 1);
		group_give_item(g_group, 0x6e, 1);
		group_give_item(g_group, 0xdb, 1);
		group_give_item(g_group, 0x16, 1);
		break;
	}
	for (i = 0, g = g_groups; i < NR_GROUPS; g = &g_groups[++i])
		group_clr_hunger_thirst(g);
	g_resting = 1;
	timewarp_days(g_date, 1L);
	g_resting = 0;
	dlg_message(g_dialog, msg);
	if (g_dng07_prison_days == 6)
		g_dng07_prison_event = g_dng07_prison_days = 0;
	dlg_set16(g_dialog, old);
}

void dng07_spider_queen(void far *ev)
{
	void far *buf;
	char far * far *txt;
	struct hero far *h;
	register signed short r;

	h = get_first_active_hero(g_group);
	txt_load_list(g_font, FILE_BLUT2, 0x78, &buf, &txt);
	/* BLUT2.LTX[93]: "Mit maechtigen Schritten naehert sich Euch die Hoehlenspinnenkoenigin und starrt Euch eine
	 * Sekunde reglos durch Ihre zahllosen Augen an.@<Bei Rondra!> entfaehrt es %s. <Das Teil ist ja riesig!> und
	 * tatsaechlich ist diese Spinne etwa vier Schritt hoch und fuellt den gesamten Gang aus, so dass an eine Flucht
	 * nicht zu denken ist." */
	sprintf(g_text_output_buf, txt[93], h->name);
	dlg_message(g_dialog, g_text_output_buf);
	r = group_count_bd2(g_group);
	if (r == 0) {
		/* BLUT2.LTX[94]: "Dann, urploetzlich setzen sich Ihre unzaehligen Beine in Bewegung und sie baeumt sich vor Euch
		 * auf, um anzugreifen. Innerhalb weniger Sekunden hat sie jeden von Euch vergiftet und niedergestreckt. Das
		 * letzte, was Ihr seht, ist, wie sich ein grosser, achtbeiniger Schatten ueber Euch niedersenkt!" */
		dlg_message(g_dialog, txt[94]);
		group_kill_all(g_group);
	} else if (group_count_alive(g_group) <= r) {
		/* BLUT2.LTX[96]: "Dann, urploetzlich setzen sich Ihre unzaehligen Beine in Bewegung und sie baeumt sich vor Euch
		 * auf, um anzugreifen. Obwohl die Spinne erfolgreich versucht, Euch zu beissen und Ihr Gift in Eure Leiber zu
		 * pumpen, bleibt es dennoch erfolglos. Irgendetwas in Euch schuetzt Euch vor dem sonst todbringenden Gift der
		 * Spinnenkoenigin." */
		dlg_message(g_dialog, txt[96]);
		group_sub_le_keep(g_group, 0x32, 10);
		/* BLUT2.LTX[97]: "Durch diese Erkenntnis bestaerkt, greift Ihr nun frontal an und es gelingt Euch, nach einem
		 * kurzen, aber sehr erbitterten Kampf, die Spinne zu toeten!" */
		dlg_message(g_dialog, txt[97]);
		group_add_ap_each(g_group, 0x32);
	} else {
		/* BLUT2.LTX[95]: "Dann, urploetzlich setzen sich Ihre unzaehligen Beine in Bewegung und sie baeumt sich vor Euch
		 * auf, um anzugreifen. Innerhalb weniger Sekunden hat sie einen Grossteil von Euch vergiftet und
		 * niedergestreckt. Ein Teil von Euch scheint allerdings immun gegen das sonst todbringende Gift dieser
		 * riesenhaften Spinne zu sein. Tapfer greift Ihr die Spinne an und es gelingt Euch, nach einem kurzen, aber sehr
		 * erbitterten Kampf, die Spinne zu toeten!" */
		dlg_message(g_dialog, txt[95]);
		group_sub_le_keep(g_group, 0x32, 10);
		group_kill_but_bd2(g_group);
		group_add_ap_each(g_group, 0x32);
	}
	anim_free_all(g_screen_pic);
	mem_free(buf);
	mem_free(txt);
	{
		Bmp b(0xd0, 0x87);

		b.clear();
		mouse_hide(0);
		b.put(g_vga_buf, 0x10, 2, BLIT_BEHIND);
		mouse_show(0);
	}
}
