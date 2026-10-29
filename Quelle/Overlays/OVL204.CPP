/* OVL204 - Overlay O204 (Segment 0x237f in SCHWEIF.EXE): Ereignisse des Dungeons 5, des Phex-Tempels, in den Hensger
 * die Gruppe gelockt hat (Texte PHEXTEMP.LTX = Datei 0x34d; g_area_descs[5]); Ereignisobjekt ev (struct area_events,
 * Texte ev->txt), Kartenereignis mev. Zeitfunktion dng05_poltergeist aus R12 bei g_dng05_poltergeist == 1/3.
 * dng05_handler(ev, mev): pos = (Zeile - g_map_x0) << 8 | (Spalte - g_map_y0), h = erster handlungsfaehiger Held.
 *   MAP_EVENT_ENTER: 0xb12 alte Schriftrolle (PHEXTEMP[2]; Rezept fuer Geldscheisser) -> Gitter faellt (Zellobjekt 11
 *   FACE_SOLID aus, Automap, PHEXTEMP[42]), vier Skelette ([43]), Kampfschleife event_fight1(0x5d, 4) bis Sieg (Wand
 *   (0x16, 0x0b) auf) oder Flucht; 0x214 Fallgrube (Sinnesschaerfe +7: entdeckt [3], sonst W6 LE [4]); 0xc18
 *   vergilbte Schriftrolle ([47]..[50], Rezept fuer Zaubertrank) -> Gitter wieder offen (FACE_SOLID, CELL_CHANGED der
 *   Zelle geloescht); 0x114 Schalter g_dng05_switch_0114; 0x160b Bolzenfalle (scharf ueber 0x170b/0x150b, nicht nach
 *   einer Opfergabe; [29], ab 19 LE je Held Zusatz [30]); 0x1a03 Schlussszene phex_finale; 0x250c/0x250d drei Felder
 *   (0x0b, 0x1d..0x1f) frei; 0x1f1d/0x211d/0x231a Drehfalle (Richtung + 2); 0x151c Versetzung event_teleport(0x15)
 *   bei Richtung != 0; sonst dng05_fights. Typ 1: Schloesser (Lock 12..17 je Position) mit Wandbild 0x2c..0x2f. Typ
 *   2: 0x819 Kampf bzw. danach Truhe 8; 0x1a13 Schatzkammer (Truhe 0x30): wird etwas genommen, erscheint der
 *   Poltergeist (PHEXTEMP[61], g_dng05_poltergeist = 1, group.poltergeist_bound = 1, Feld (0x11, 0x1a) zu). Typ 0x64
 *   Vorraete (10 Proviantpakete, [0]/[1]). 0x65 Geheimtuer ([28]). 0x66 Skelette ([31]; zu 30 % bricht der
 *   Totenaengstlichste zusammen, [32], W6 LE). 0x67 Skelettkrieger ([33], Kampf 0x5d x 4 -> dng05_skeleton_key). 0x68
 *   Geheimtueren je Position (Sinnesschaerfe +10..+20, [40]/[51]/[52]/[56]; manche erst nach Raetsel bzw. Legespiel
 *   offen; 1..2 AP). 0x69 Kohlebecken ([44]). 0x6a Opferstock ([34..39], [45], [46]): Dukaten spenden (ab 51 oeffnet
 *   sich die Wand (6, 0x11)) oder pluendern (Sturmwind: 2W6+2 LE fuer alle, Versetzung 0x1d). 0x6b an 0x100e
 *   Steinplatte "Wann ist das Licht des Phex alleine?" ([53], 9 Zeichen: NACHTS / BEI NACHT / NACHT -> [54], sonst
 *   [55] und Kampf 0x62 x 4); an 0x30b Legespiel (new Puzzle, run, delete; dng05_puzzle_result). 0x6c Poltergeist im
 *   Weg ([67]/[68]). 0x78 an 0x1e03 Ausgang: Dungeon 0, Stadt 5 auf (0x14, 0x1c), 170 AP fuer alle; sonst Luke
 *   verriegelt ([59]). 0x79 Tafel ([60]).
 * dng05_fights(ev, pos): einmalige Kaempfe (Merker g_dng05_<gegner>_<pos>); nach den vier Kaempfen um 0x1b1d oeffnet
 *   sich die Wand (0x1a, 0x1b) (g_dng05_wall_1a1b_open).
 * dng05_puzzle_result: gewonnen -> leises Klicken in der Wand ([57]), sonst Frust ([58]). dng05_skeleton_key: blauer
 *   Schluessel zwischen den Gebeinen ([41]).
 * dng05_poltergeist(ev): in Ebene 5 mit poltergeist_bound bei jedem Klangdienst-Takt (g_snd_service_due): nach
 *   Geister austreiben (Zustand 3) loest er sich auf ([69], Zustand 2); nach 2000 Takten nimmt er Phexenhelm und
 *   -schild ([62..66]) und legt sie in die Truhe 0x30 zurueck (Zustand 0).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y -E..\bc\BIN\TASM.EXE OVL204.CPP -> 8077/8077 Bytes GLEICH (18.10.2026), cobjfix
 *   laeuft. -Oi fuer strcat/strncmp/strcpy; -Og nur fuer die Epilogkopie in dng05_puzzle_result (ohne: 2 Bytes).
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Locals: o ([bp-12h], g_cell_objs[11]) und c ([bp-16h], Kartenzelle) sind Variablen der Funktionsebene, obwohl nur
 *   in zwei Bloecken benutzt; [bp-1ah]/[bp-1eh]/[bp-22h] = this-Kopien von get_random_hero(..)->get_name() (inline)
 *   in Typ 0x65/0x6a; in Typ 2 liegt die this-Kopie im Block mit Chest-Objekt bei [bp-26h] und teilt den Platz mit pz
 *   (Typ 0x6b, Block-Local). Lock [bp-50h] und Chest [bp-7ch] enden beide bei [bp-27h].
 * - automap_cell_write ueber AUTOMAP_CELL_WRITE_W (letzter Parameter short, wie OVL063).
 * - "if (!!g_group->dir)" fuer mov al / cbw / or ax,ax (ohne !! cmp byte).
 * - Bitfeld dead +bc.0: "!(signed char)h->dead" im if (and/cbw/or), "(signed char)h->dead" im while (and/or al,al).
 * - "sub_hero_le(h, x += random_schweif(6) + 6)" (Zuweisung im Argument, mov ax,[x] nachgeladen).
 * - dng05_fights: register-Parameter pos (SI), ret OHNE register (DI); mit register tauschen SI/DI.
 * - dng05_poltergeist: inline group::get_hero(i) -> Parameterkopien [bp-12h]/[bp-14h]; delete ch -> dtor(ch, 3) ohne
 *   Nulltest; Parameter void far * wie im Prototyp (R12 uebergibt g_area_events).
 * Originalfehler/Auffaelliges: Typ 1 benutzt x (Wandbild) uninitialisiert, wenn keine Schlossposition passt und
 *   g_lock_cell_bits != -1. Typ 0x6c: im else-Zweig wird h->name auch bei h == 0 benutzt (Name ab 0000:0022); Typ
 *   0x6c setzt ret nicht. Falle 0x160b: x waechst ueber alle Helden weiter (laufende Summe statt Einzelwurf), jeder
 *   Held erhaelt die bisherige Summe als Schaden - spaetere Helden bekommen immer mehr ab.
 */
#include <stdio.h>
#include <string.h>

#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:7052..706a, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */

/* automap_cell_write mit short als letztem Parameter (wie im Original deklariert) */
#define AUTOMAP_CELL_WRITE_W \
	((void (*)(void far *, signed short, signed short, signed short, signed short))automap_cell_write)

signed short dng05_handler(struct area_events far *ev, struct map_event far *mev)
{
	signed short type;
	signed short pos;
	struct hero far *h;
	signed short x;
	signed short i;
	signed short ret;
	struct obj3d far *o;
	struct map_cell far *c;

	type = mev->type;
	pos = (mev->y - g_map_x0) << 8;
	pos |= mev->x - g_map_y0;
	h = get_first_active_hero(g_group);
	ret = 0;

	if (type == MAP_EVENT_ENTER) {
		if (pos == 0x0b12 && !g_dng05_recipe_taken) {
			/* PHEXTEMP.LTX[2]: "Am Boden vor Euch liegt eine alte Schriftrolle.@Wollt Ihr sie mitnehmen?" */
			if (dlg_yes_no(g_dialog, ev->txt[2]) == 1 && group_give_item(g_group,
				ITEM_RECIPE_GELDSCHEISSER, 1)) {
				g_dng05_recipe_taken = 1;
				o = g_cell_objs[11];
				o->faces[0]->flags &= ~FACE_SOLID;
				AUTOMAP_CELL_WRITE_W(g_map, 0x34e, 8, 0x16, 0x21);
				event_set_cell_bits(ev, g_map_y0 + 0x16, g_map_x0 + 8, 0);
				snd_play(g_snd_fx, 0xa5);
				/* PHEXTEMP.LTX[42]: "Als Ihr das Rezept aufhebt, geht ein leises Raunen durch den Raum und Ihr hoert, wie sich
				 * ein schweres Metallgitter in Bewegung setzt und kurz darauf zu Boden knallt.@<Die Tuer...> ruft %s." */
				sprintf(g_text_output_buf, ev->txt[42], h->name);
				dlg_message(g_dialog, g_text_output_buf);
				snd_play(g_snd_fx, 0x60);
				/* PHEXTEMP.LTX[43]: "Dann hoert Ihr das schwere Schaben von Stein auf Stein. Irgendwo in Eurer unmittelbaren
				 * Naehe oeffnet sich eben eine schwere Steintuere und schon koennt Ihr das Klirren von Waffengehaengen hoeren,
				 * als ploetzlich 4^Skelette im Raum auftauchen und Euch angreifen." */
				dlg_message(g_dialog, ev->txt[43]);
				do {
					x = event_fight1(ev, 0x5d, 4, 0);
					if (x == 0) {
						event_set_cell(ev, 0x16, 0x0b, 7, 0, -1);
						AUTOMAP_CELL_WRITE_W(g_map, 0x34e, 0x0b, 0x16, 0x33);
					}
				} while (x != 0 && x != 2);
			}
			ret = 1;
		} else if (pos == 0x0214) {
			if (!g_dng05_pit_seen) {
				g_dng05_pit_seen = 1;
				if (test_skill(h, TA_SINNESSCHAERFE, 7) >= 0) {
					/* PHEXTEMP.LTX[3]: "%s erkennt kurz vor Euch ploetzlich eine Fallgrube.@<Vorsicht!> warnt %s den Rest der
					 * Gruppe." */
					sprintf(g_text_output_buf, ev->txt[3], h->name, grammar_pronoun(h->sex, 0,
						0));
					dlg_message(g_dialog, g_text_output_buf);
				} else {
					/* PHEXTEMP.LTX[4]: "Ploetzlich beginnt %s heftig mit den Armen zu rudern und stuerzt Sekundenbruchteile
					 * spaeter in eine Fallgrube.@<Au...> hoert Ihr es aus der Grube, doch bereits kurz darauf taucht %s Kopf
					 * wieder aus dem Boden auf. Ihr reicht %s die Hand und helft %s aus der Grube." */
					sprintf(g_text_output_buf, ev->txt[4], h->name, grammar_pronoun(h->sex, 1,
						0),
						grammar_pronoun(h->sex, 3, 0), grammar_pronoun(h->sex, 3, 0));
					dlg_message(g_dialog, g_text_output_buf);
					sub_hero_le(h, random_schweif(6));
				}
				ret = 1;
			}
		} else if (pos == 0x0c18) {
			if (!g_dng05_scroll_taken) {
				/* PHEXTEMP.LTX[47]: "Vor Euch am Boden liegt eine alte, vergilbte und schwer verstaubte Schriftrolle.@Wollt
				 * Ihr sie mitnehmen?" */
				if (dlg_yes_no(g_dialog, ev->txt[47]) == 1) {
					h = get_random_hero(g_group);
					/* PHEXTEMP.LTX[48]: "<Vorsicht!> meint %s etwas besorgt. <Ihr wisst doch noch, was voriges Mal passiert ist,
					 * als wir eine Schriftrolle... Ich meine... okay, sollen wir ewig leben?>@Dann hebt %s vorsichtig das Stueck
					 * Pergament auf und identifiziert es als ein uraltes Rezept." */
					sprintf(g_text_output_buf, ev->txt[48], h->name, grammar_pronoun(h->sex, 0,
						0));
					dlg_message(g_dialog, g_text_output_buf);
					group_give_item(g_group, ITEM_RECIPE_MAGIC_POTION, 1);
					o = g_cell_objs[11];
					o->faces[0]->flags |= FACE_SOLID;
					AUTOMAP_CELL_WRITE_W(g_map, 0x34e, 8, 0x16, 0x30);
					c = &g_cells[g_map_y0 + 0x16][g_map_x0 + 8];
					c->flags &= ~CELL_CHANGED;
					event_set_cell_bits(ev, g_map_y0 + 0x16, g_map_x0 + 8, 0x2000);
					snd_play(g_snd_fx, 0x61);
					h = get_random_hero(g_group);
					/* PHEXTEMP.LTX[49]: "Dann, ploetzlich, hoert Ihr ein Geraeusch.@<Nein, nicht schon wieder...> Ihr greift
					 * bereits zu den Waffen, als %s meint, <Ich glaube, das war die Tuere.>" */
					sprintf(g_text_output_buf, ev->txt[49], h->name);
					dlg_message(g_dialog, g_text_output_buf);
					g_dng05_scroll_taken = 1;
				} else {
					h = get_random_hero(g_group);
					/* PHEXTEMP.LTX[50]: "<Ich denke echt, wir sollten hier etwas vorsichtiger sein.> meint %s." */
					sprintf(g_text_output_buf, ev->txt[50], h->name);
					dlg_message(g_dialog, g_text_output_buf);
				}
				ret = 1;
			}
		} else if (pos == 0x0114) {
			g_dng05_switch_0114 = g_dng05_switch_0114 == 1 ? 0 : 1;
		} else if (pos == 0x160b) {
			if (g_dng05_bolt_trap_armed == 1 && g_dng05_offering_made == 0) {
				/* PHEXTEMP.LTX[29]: "Ihr seid heftig von Bolzen aus der Wand getroffen worden!" */
				sprintf(g_text_output_buf, ev->txt[29]);
				i = 0;
				x = 0;
				h = get_first_hero(g_group);
				for (; i < group_count_alive(g_group); i++) {
					if (!(signed char)h->dead) {
#if FIX_0192
						/* Fix #192: je Held 2 x (W6 + 6) LE; x summiert nur fuer die Ohnmachtsgrenze */
						{
							signed short d;
							d = random_schweif(6) + 6;
							x += d;
							sub_hero_le(h, d);
							d = random_schweif(6) + 6;
							x += d;
							sub_hero_le(h, d);
						}
#else
						sub_hero_le(h, x += random_schweif(6) + 6);
						sub_hero_le(h, x += random_schweif(6) + 6);
#endif
					}
					do {
						h = get_next_hero(g_group, h);
					} while (h && (signed char)h->dead);
				}
				if (g_group->nr_heroes * 19 < x)
					/* PHEXTEMP.LTX[30]: "^Euch wird Schwarz vor Augen und es dauert eine Weile, bis Ihr wieder zu Euch kommt." */
					strcat(g_text_output_buf, ev->txt[30]);
				dlg_message(g_dialog, g_text_output_buf);
				ret = 1;
			}
		} else if (pos == 0x170b) {
			g_dng05_bolt_trap_armed = 1;
		} else if (pos == 0x150b) {
			g_dng05_bolt_trap_armed = 0;
		} else if (pos == 0x1a03) {
			if (!g_dng05_finale_seen) {
				swap_out_screen();
				phex_finale();
				g_dng05_finale_seen = 1;
				swap_in_screen();
				ret = 1;
			}
		} else if (pos == 0x250c || pos == 0x250d) {
			event_set_cell(ev, 0x0b, 0x1d, 0, 0, -1);
			event_set_cell(ev, 0x0b, 0x1e, 0, 0, -1);
			event_set_cell(ev, 0x0b, 0x1f, 0, 0, -1);
			AUTOMAP_CELL_WRITE_W(g_map, 0x34e, 0x1d, 0x0b, 0x33);
			AUTOMAP_CELL_WRITE_W(g_map, 0x34e, 0x1e, 0x0b, 0x33);
			AUTOMAP_CELL_WRITE_W(g_map, 0x34e, 0x1f, 0x0b, 0x33);
		} else if (pos == 0x1f1d || pos == 0x211d || pos == 0x231a) {
			g_group->dir = (g_group->dir + 2) & 3;
		} else if (pos == 0x151c) {
			if (!!g_group->dir)
				event_teleport(ev, 0x15);
		} else
			ret = dng05_fights(ev, pos);
	} else if (type == 1) {
		if (pos == 0x031e) {
			Lock lk(13);
			x = 0x2d;
			ret = 1;
		} else if (pos == 0x0e1c) {
			Lock lk(12);
			x = 0x2c;
			ret = 1;
		} else if (pos == 0x0703) {
			Lock lk(17);
			x = 0x2f;
			ret = 1;
		} else if (pos == 0x120c) {
			Lock lk(16);
			x = 0x2f;
			ret = 1;
		} else if (pos == 0x140d) {
			Lock lk(15);
			x = 0x2e;
			ret = 1;
		}
		if (g_lock_cell_bits != -1) {
			event_set_cell_bits(ev, mev->x, mev->y, g_lock_cell_bits);
			AUTOMAP_CELL_WRITE_W(g_map, 0x34e, mev->y - g_map_x0, mev->x - g_map_y0, x);
		}
	} else if (type == 2) {
		if (pos == 0x0819) {
			if (!g_dng05_priests_0819) {
				if (event_fight(ev, 0x3a, 4, 0, 0, 0x3a, 2, 1, 0, 0x3b, 1, 1, 5) == 0)
					g_dng05_priests_0819 = 1;
			} else {
				Chest ch(8, 1);
			}
			ret = 1;
		} else if (pos == 0x1a13 && !g_dng05_poltergeist) {
			Chest ch(0x30, 1);
			if (ch.count_items() != 2) {
				g_dng05_poltergeist = 1;
				g_group->poltergeist_bound = 1;
				g_dng05_poltergeist_ticks = 0;
				event_set_cell(ev, 0x11, 0x1a, 0x14, 0, 2);
				/* PHEXTEMP.LTX[61]: "<Irgendetwas stimmt hier nicht!> meint %s indigniert.@<Das war zu einfach!>@<Bei Rondra!>
				 * entfaehrt es da %s und als Ihr Euch umdreht, erkennt Ihr, was los ist. In der Tuere steht ein riesiger
				 * Poltergeist und versperrt Euch den Ausgang." */
				sprintf(g_text_output_buf, ev->txt[61], h->name,
					get_random_hero(g_group)->get_name());
				snd_play(g_snd_fx, 0xa0);
				dlg_message(g_dialog, g_text_output_buf);
			}
			ret = 1;
		}
	} else if (type == 0x64) {
		if (!g_dng05_supplies_taken) {
			/* PHEXTEMP.LTX[0]: "Hier liegen eine Menge Vorraete. Wollt Ihr etwas davon mitnehmen?" */
			if (dlg_yes_no(g_dialog, ev->txt[0]) == 1 && group_give_item(g_group, ITEM_FOOD_PACKAGE, 10))
				g_dng05_supplies_taken = 1;
		} else
			/* PHEXTEMP.LTX[1]: "Ihr koennt nicht die gesamten Vorraete hier pluendern. Sich einmal an wildfremder Leute
			 * Eigentum zu bedienen ist dreist, dasselbe allerdings mehrmals zu tun ist einfach nicht drin." */
			dlg_message(g_dialog, ev->txt[1]);
		ret = 1;
	} else if (type == 0x65) {
		/* PHEXTEMP.LTX[28]: "<Sieh mal einer an!> entfaehrt es %s. <Eine Geheimtuere. Mal sehen, was uns diese
		 * Dumpfbacke von Hensger hier eingebrockt hat!>" */
		sprintf(g_text_output_buf, ev->txt[28], get_random_hero(g_group)->get_name());
		dlg_message(g_dialog, g_text_output_buf);
		event_set_cell_bits(ev, mev->x, mev->y, 0x2000);
		AUTOMAP_CELL_WRITE_W(g_map, 0x34e, mev->y - g_map_x0, mev->x - g_map_y0, 0x31);
		snd_play(g_snd_fx, 0x60);
		ret = 1;
	} else if (type == 0x66) {
		/* PHEXTEMP.LTX[31]: "Die Skelette sind offensichtlich aus Knochen zusammengesetzt, die nicht von derselben
		 * Person stammen!" */
		dlg_message(g_dialog, ev->txt[31]);
		if (random_schweif(100) <= 30) {
			h = get_hero_attrib_max(g_group, ATTRIB_TA);
			/* PHEXTEMP.LTX[32]: "Urploetzlich bricht %s zusammen, kreidebleich im Gesicht mit kaltem Schweiss auf der
			 * Stirn. Die Augen blicken starr ins Nichts.@<Bald werden wir auch so aussehen...> stoesst %s hervor und
			 * uebergibt sich. Kurz danach scheint %s sich jedoch wieder im Griff zu haben." */
			sprintf(g_text_output_buf, ev->txt[32], h->name, grammar_pronoun(h->sex, 0, 0),
				grammar_pronoun(h->sex, 0, 0));
			dlg_message(g_dialog, g_text_output_buf);
			sub_hero_le(h, random_schweif(6));
		}
		ret = 1;
	} else if (type == 0x67) {
		if (!g_dng05_skeletons_beaten) {
			snd_play(g_snd_fx, 0xa0);
			/* PHEXTEMP.LTX[33]: "Ein markerschuetternder Schrei hallt durch die Gaenge und ploetzlich hoert Ihr Schritte
			 * und das Gerassel von Waffengehaengen hinter Euch. Als Ihr Euch umwendet, steht Ihr 4^Skeletten
			 * gegenueber!@<Ohoh...> stoehnt %s. <Das bedeutet Aerger!>" */
			sprintf(g_text_output_buf, ev->txt[33], h->name);
			dlg_message(g_dialog, g_text_output_buf);
			if (event_fight1(ev, 0x5d, 4, 0) == 0)
				dng05_skeleton_key((struct area_events far *)g_area_events);
			ret = 1;
		}
	} else if (type == 0x68) {
		register signed short k;

		x = 0;
		switch (pos) {
		case 0x061a:
			if (test_skill(h, TA_SINNESSCHAERFE, 0x0f) >= 0) {
				sprintf(g_text_output_buf, ev->txt[28], h->name);
				x = 2;
				k = 0x33;
			}
			break;
		case 0x0402:
			if (test_skill(h, TA_SINNESSCHAERFE, 0x14) >= 0) {
				if (g_dng05_puzzle_won == 1) {
					/* PHEXTEMP.LTX[56]: "<Hab' ich's nicht gesagt? Eine Geheimtuere. Und jetzt geht sie auch auf!>" */
					/* PHEXTEMP.LTX[52]: "<Schon wieder eine Geheimtuere.> mault %s los. <Wo sind wir hier bloss hineingeraten?>" */
					sprintf(g_text_output_buf,
						g_dng05_door_0402_found == 1 ? ev->txt[56] : ev->txt[52], h->name);
					x = 3;
					k = 0x32;
				} else {
					g_dng05_door_0402_found = 1;
					/* PHEXTEMP.LTX[40]: "<Ich weiss genau, dass da eine Tuere ist, aber ich bekomme sie beim besten Willen nicht
					 * auf...> meint %s etwas ratlos. <Vielleicht gibt es da einen Trick.>" */
					sprintf(g_text_output_buf, ev->txt[40], h->name);
					x = 1;
				}
			}
			break;
		case 0x0418:
			if (test_skill(h, TA_SINNESSCHAERFE, 0x0a) >= 0) {
				sprintf(g_text_output_buf, ev->txt[52], h->name);
				x = 2;
				k = 0x33;
			}
			break;
		case 0x070d:
			if (test_skill(h, TA_SINNESSCHAERFE, 0x0f) >= 0) {
				if (g_dng05_riddle_solved == 1) {
					sprintf(g_text_output_buf,
						g_dng05_door_070d_found == 1 ? ev->txt[56] : ev->txt[52], h->name);
					x = 2;
					k = 0x32;
				} else {
					g_dng05_door_070d_found = 1;
					sprintf(g_text_output_buf, ev->txt[40], h->name);
					x = 1;
				}
			}
			break;
		case 0x1502:
			if (test_skill(h, TA_SINNESSCHAERFE, 0x0f) >= 0) {
				sprintf(g_text_output_buf, ev->txt[52], h->name);
				x = 2;
				k = 0x32;
			}
			break;
		case 0x1a08:
			if (test_skill(h, TA_SINNESSCHAERFE, 0x14) >= 0) {
				sprintf(g_text_output_buf, ev->txt[52], h->name);
				x = 3;
				k = 0x32;
			}
			break;
		case 0x1106:
			if (test_skill(h, TA_SINNESSCHAERFE, 0x0a) >= 0) {
				sprintf(g_text_output_buf, ev->txt[40], h->name);
				x = 1;
				g_dng05_door_1106_found = 1;
			}
			break;
		case 0x0c17:
			if (test_skill(h, TA_SINNESSCHAERFE, 0x14) >= 0) {
				/* PHEXTEMP.LTX[51]: "<Oh Mann>, stoehnt %s. <Da ist eine Geheimtuere. So gut getarnt, dass man sie fast nicht
				 * entdecken kann. So macht das Spass!> Mit diesen Worten betaetigt %s einen geheimen Mechanismus und die Tuer
				 * gleitet unter lautem Getoese nach unten." */
				sprintf(g_text_output_buf, ev->txt[51], h->name, grammar_pronoun(h->sex, 0, 0));
				x = 3;
				k = 0x32;
			}
			break;
		}
		if (x != 0) {
			dlg_message(g_dialog, g_text_output_buf);
			if (x > 1) {
				event_set_cell_bits(ev, mev->x, mev->y, 0x2000);
				AUTOMAP_CELL_WRITE_W(g_map, 0x34e, mev->y - g_map_x0, mev->x - g_map_y0, k);
				snd_play(g_snd_fx, 0x60);
				add_hero_ap(h, x - 1);
			}
			ret = 1;
		}
	} else if (type == 0x69) {
		/* PHEXTEMP.LTX[44]: "Das ist ein Kohlebecken, das den Raum in ein warmes Licht versetzt und zudem noch ein klein
		 * wenig waermt. Nichts Besonderes an sich." */
		dlg_message(g_dialog, ev->txt[44]);
		ret = 1;
	} else if (type == 0x6a) {
		/* PHEXTEMP.LTX[34]: "Ihr steht vor einem Opferstock des Phex. In der Schale befinden sich einige Dukaten und
		 * Einiges an Schmuck.@Wollt Ihr" */
		/* PHEXTEMP.LTX[35]: "selbst etwas darbringen" */
		/* PHEXTEMP.LTX[36]: "den Opferstock pluendern?" */
		if ((x = dlg_choose(g_dialog, ev->txt[34], 2, ev->txt[35], ev->txt[36])) != -1) {
			if (x == 1) {
				/* PHEXTEMP.LTX[45]: "Wieviele Dukaten wollt Ihr spenden?" */
				x = dlg_input(g_dialog, 3, ev->txt[45], 0, (signed short)(g_group->money / 100), 0);
				if (x != -1) {
					if ((signed long)x * 100 > g_group->money)
						/* GLOBLTXT.LTX[4]: "Soviel Geld habt Ihr nicht bei Euch!" */
						dlg_message(g_dialog, g_ttx[4]);
					else if (x > 0) {
						g_dng05_offered_ducats += x;
						snd_play(g_snd_fx, 0x47);
						group_sub_money(g_group, (signed long)x * 100);
						if (g_dng05_offered_ducats > 50) {
							/* PHEXTEMP.LTX[39]: "Als Ihr Eure Gabe in die Schale legt, kann man aus der Ferne ein seltsames Geraeusch
							 * hoeren.@<Was war das?> fragt %s und greift langsam zur Waffe." */
							sprintf(g_text_output_buf, ev->txt[39],
								get_random_hero(g_group)->get_name());
							dlg_message(g_dialog, g_text_output_buf);
							snd_play(g_snd_fx, 0x9f);
							event_set_cell(ev, 6, 0x11, 0x0e, 0x2000, -1);
							AUTOMAP_CELL_WRITE_W(g_map, 0x34e, 0x11, 6, 0x13);
						} else
							/* PHEXTEMP.LTX[46]: "Vorsichtig legt Ihr Euer Geld in die Opferschale, doch nichts weiter geschieht." */
							dlg_message(g_dialog, ev->txt[46]);
						g_dng05_offering_made = 1;
					}
				}
			} else {
				snd_play(g_snd_fx, 0x47);
				/* PHEXTEMP.LTX[37]: "Eben, als %s in die Schale greift, verschwindet ihr Inhalt, wie von Geisterhand. Eine
				 * ohrenbetaeubende, koerperlose Stimme ruft erzuernt." */
				sprintf(g_text_output_buf, ev->txt[37], get_random_hero(g_group)->get_name());
				dlg_message(g_dialog, g_text_output_buf);
				/* PHEXTEMP.LTX[38]: "<Macht das nie wieder! Nie wieder! Niemand bestiehlt ungestraft den Gott der Diebe!>@Dann
				 * erfasst Euch ein heftiger Wind, hebt Euch in die Luefte und wirbelt Euch umher. Der Raum um Euch herum
				 * verschwimmt vor Euren Augen und erst Sekunden spaeter, als der Sog erstirbt und Ihr heftig auf den Boden
				 * prallt, wird Euch klar, was geschehen ist..." */
				dlg_message(g_dialog, ev->txt[38]);
				group_sub_le(g_group, dice_roll(2, 6, 2));
				event_teleport(ev, 0x1d);
			}
			ret = 1;
		}
	} else if (type == 0x6b) {
		if (!g_dng05_riddle_solved && pos == 0x100e) {
			/* PHEXTEMP.LTX[53]: "Ploetzlich geraet Bewegung in die Platte und schemenhaft erkennt Ihr ein undeutliches
			 * Gesicht.@<Wann ist das Licht des Phex alleine?>@fragt Euch eine Stimme, die aus dem Nichts zu kommen
			 * scheint." */
			dlg_message_flag(g_dialog, 9, ev->txt[53]);
			if (!strncmp(g_input_buf, "NACHTS", 9) || !strncmp(g_input_buf, "BEI NACHT",
				9) || !strncmp(g_input_buf, "NACHT", 9)) {
				/* PHEXTEMP.LTX[54]: "<Jawohl.> raunt die Stimme und das Gesicht verblasst. Irgendwo in der Ferne hoert Ihr ein
				 * Geraeusch, als ob eine Tuer entriegelt wird." */
				dlg_message(g_dialog, ev->txt[54]);
				g_dng05_riddle_solved = 1;
			} else {
				/* PHEXTEMP.LTX[55]: "<Es scheint, als kennet Ihr die Antwort nicht, Unglaeubige!> mault die Stimme.@<Ihr habt
				 * in diesem, meinem Tempel nichts zu suchen!>" */
				dlg_message(g_dialog, ev->txt[55]);
				event_fight1(ev, 0x62, 4, 0);
			}
			ret = 1;
		} else if (pos == 0x030b && !g_dng05_puzzle_won) {
			Puzzle far *pz;

			swap_out_screen();
			mouse_hide(0);
			clear_screen_buf(g_vga_buf);
			mouse_show(0);
			pz = new Puzzle;
			g_dng05_puzzle_won = pz->run();
			delete pz;
			mouse_hide(0);
			clear_screen_buf(g_vga_buf);
			mouse_show(0);
			screen_redraw(g_screen);
			draw_loc_icons(g_screen, 7, 0x0c, g_icon_group == 1 ? 0x0f : 0x2d,
				g_icon_groups == 1 ? 0x1d : 0x44, 0x27, 0x0b, 0x41, 0x28);
			swap_in_screen();
			dng05_puzzle_result((struct area_events far *)g_area_events);
			ret = 1;
		}
	} else if (type == 0x6c) {
		h = get_mage_hero(g_group);
		if (h && check_hero3(h))
			/* PHEXTEMP.LTX[67]: "Ihr versucht, Euch an dem Geist vorbeizuschlaengeln, doch es gelingt Euch nicht. Als Ihr
			 * versucht, Ihn mit Waffen anzugreifen, schlagt Ihr nur ins Leere.@<So geht das nicht!> meint %s. <Wir koennen
			 * einen Poltergeist nicht mit Waffen zerstoeren!>" */
			sprintf(g_text_output_buf, ev->txt[67], h->name);
		else
			/* PHEXTEMP.LTX[68]: "Ihr versucht, Euch an dem Geist vorbeizuschlaengeln, doch es gelingt Euch nicht. Als Ihr
			 * versucht, Ihn mit Waffen anzugreifen, schlagt Ihr nur ins Leere." */
			sprintf(g_text_output_buf, ev->txt[68], h->name);
		dlg_message(g_dialog, g_text_output_buf);
	} else if (type == 0x78) {
		if (pos == 0x1e03) {
			view_leave();
			g_group->old_dungeon = g_group->dungeon;
			g_group->dungeon = 0;
			g_group->old_town = g_group->town;
			g_group->town = 5;
			g_group->dir = 2;
			group_set_screen_pos(0x14, 0x1c);
			group_add_ap_each(g_group, 0xaa);
			g_pp20_index = -1;
			g_refresh = 1;
		} else
			/* PHEXTEMP.LTX[59]: "Am Ende der Treppe ist eine Luke, die allerdings von aussen verriegelt ist. Hensger hat
			 * Euch hier eingeschlossen!" */
			dlg_message(g_dialog, ev->txt[59]);
		ret = 1;
	} else if (type == 0x79) {
		/* PHEXTEMP.LTX[60]: "<Kennen Sie schon die Schicksalsklinge?> steht da auf der Tafel.@<Was soll das denn?> fragt
		 * %s indigniert. <Schicksalsklinge... so was!>@<Ist doch klar!> meint da jemand aus der Gruppe. <Das war doch
		 * der erste Teil der Nordland-Trilogie von Attic und FanPro! Hast Du den etwa  noch nicht gespielt?>@@<Das
		 * Schwarze Auge - Die Schicksalsklinge> gibt's nach wie vor im gut sortierten Softwarehandel!" */
		sprintf(g_text_output_buf, ev->txt[60], h->name);
		dlg_message(g_dialog, g_text_output_buf);
		ret = 1;
	}
	return ret;
}

signed short dng05_fights(struct area_events far *ev, register signed short pos)
{
	signed short ret = 0;

	if (pos == 0x0e1d && !g_dng05_mummies_0e1d) {
		if (event_fight(ev, 8, 2, 0, 0, 0x2d, 3, 1, 5, 0, 0, 0, 0) == 0)
			g_dng05_mummies_0e1d = 1;
		ret = 1;
	} else if (pos == 0x091f && !g_dng05_mummies_091f) {
		if (event_fight1(ev, 0x2d, 2, 0) == 0)
			g_dng05_mummies_091f = 1;
		ret = 1;
	} else if (pos == 0x021e && !g_dng05_mummies_021e) {
		if (event_fight(ev, 8, 2, 0, 0, 0x2d, 1, 0, 0, 0x2d, 3, 0, 5) == 0)
			g_dng05_mummies_021e = 1;
		ret = 1;
	} else if ((pos == 0x0819 || pos == 0x0719 || pos == 0x0919) && !g_dng05_priests_0819) {
		if (event_fight(ev, 0x3a, 4, 0, 0, 0x3a, 2, 1, 0, 0x3b, 1, 2, 5) == 0)
			g_dng05_priests_0819 = 1;
		ret = 1;
	} else if (pos == 0x0a0e && !g_dng05_priests_0a0e) {
		if (event_fight1(ev, 0x39, 2, 0) == 0)
			g_dng05_priests_0a0e = 1;
		ret = 1;
	} else if (pos == 0x0a0a && !g_dng05_priests_0a0a) {
		if (event_fight1(ev, 0x39, 2, 0) == 0)
			g_dng05_priests_0a0a = 1;
		ret = 1;
	} else if (pos == 0x020e && !g_dng05_skeletons_020e) {
		if (event_fight2(ev, 0x21, 2, 0, 0x2d, 2, 0) == 0)
			g_dng05_skeletons_020e = 1;
		ret = 1;
	} else if (pos == 0x1c0d && !g_dng05_priests_1c0d) {
		if (event_fight2(ev, 0x39, 3, 0, 0x3a, 1, 0) == 0)
			g_dng05_priests_1c0d = 1;
		ret = 1;
	} else if (pos == 0x1501 && !g_dng05_priests_1501) {
		if (event_fight(ev, 0x39, 3, 0, 0, 0x3a, 3, 0, 0, 0x3b, 1, 1, 0) == 0)
			g_dng05_priests_1501 = 1;
		ret = 1;
	} else if (pos == 0x1214 && !g_dng05_skeletons_1214) {
		if (event_fight2(ev, 0x74, 2, 0, 0x2d, 2, 0) == 0)
			g_dng05_skeletons_1214 = 1;
		ret = 1;
	} else if (pos == 0x1a1d && !g_dng05_skeletons_1a1d) {
		if (event_fight2(ev, 0x74, 2, 0, 0x5d, 1, 0) == 0)
			g_dng05_skeletons_1a1d = 1;
		ret = 1;
	} else if (pos == 0x1b1e && !g_dng05_skeletons_1b1e) {
		if (event_fight2(ev, 0x74, 2, 0, 0x5d, 1, 0) == 0)
			g_dng05_skeletons_1b1e = 1;
		ret = 1;
	} else if (pos == 0x1c1d && !g_dng05_skeletons_1c1d) {
		if (event_fight2(ev, 0x74, 2, 0, 0x5d, 1, 0) == 0)
			g_dng05_skeletons_1c1d = 1;
		ret = 1;
	} else if (pos == 0x1b1c && !g_dng05_skeletons_1b1c) {
		if (event_fight2(ev, 0x74, 2, 0, 0x5d, 1, 0) == 0)
			g_dng05_skeletons_1b1c = 1;
		ret = 1;
	}
	if (!g_dng05_wall_1a1b_open && g_dng05_skeletons_1a1d != 0 && g_dng05_skeletons_1b1e != 0 && g_dng05_skeletons_1c1d != 0 &&
		g_dng05_skeletons_1b1c != 0) {
		g_dng05_wall_1a1b_open = 1;
		event_set_cell(ev, 0x1a, 0x1b, 0, 0, -1);
		AUTOMAP_CELL_WRITE_W(g_map, 0x34e, 0x1b, 0x1a, 0x12);
		snd_play(g_snd_fx, 0x9f);
	}
	return ret;
}

void dng05_puzzle_result(struct area_events far *ev)
{
	struct hero far *h;

	h = get_first_active_hero(g_group);
	if (g_dng05_puzzle_won == 1) {
		/* PHEXTEMP.LTX[57]: "Als Ihr die drei Felder aufdeckt, hoert Ihr ein leises Klicken in der Wand, doch sonst
		 * geschieht nichts weiter.@<Was war das denn?> gruebelt %s." */
		sprintf(g_text_output_buf, ev->txt[57], h->name);
		dlg_message(g_dialog, g_text_output_buf);
		snd_play(g_snd_fx, 0x5b);
	} else {
		/* PHEXTEMP.LTX[58]: "Frustriert gebt Ihr das seltsame Spiel auf.@<Wenn man nur wuesste, worum es dabei geht,
		 * verflucht!> %s ist etwas entmutigt." */
		sprintf(g_text_output_buf, ev->txt[58], h->name);
		dlg_message(g_dialog, g_text_output_buf);
	}
}

void dng05_skeleton_key(struct area_events far *ev)
{
	g_dng05_skeletons_beaten = 1;
	/* PHEXTEMP.LTX[41]: "Nachdem die Skelettkrieger besiegt sind und Ihre blanken Knochen quer ueber den Boden
	 * verteilt sind, entdeckt Ihr einen schweren Schluessel zwischen den bleichen Gebeinen, den Ihr schnell an Euch
	 * nehmt." */
	dlg_message(g_dialog, ev->txt[41]);
	group_give_item(g_group, ITEM_BLUE_KEY, 1);
}

void dng05_poltergeist(void far *ev)
{
	Chest far *ch;
	struct hero far *a;
	struct hero far *b;
	struct hero far *h;

	h = get_first_active_hero(g_group);
	if (g_group->dungeon == 5 && g_group->poltergeist_bound != 0) {
		event_set_cell(ev, 0x11, 0x1a, 0x14, 0, 2);
		if (g_snd_service_due == 1) {
			g_snd_service_due = 0;
			if (g_dng05_poltergeist == 3) {
				snd_play(g_snd_fx, 0x4d);
				/* PHEXTEMP.LTX[69]: "Kaum wird der Poltergeist von dem Zauber erfasst, beginnt er sich wie ein Aal zu winden.
				 * Seine Konturen verzerren sich in eine monstroese, neblige Fratze, aus deren Mund ein markerschuetternder
				 * Aufschrei kommt. Dann loest sich der Geist vor Euren Augen quaelend langsam auf, doch der Todesschrei des
				 * Geistes will nicht aus Euren Ohren gehen.@<So etwas habe ich ja noch nie erlebt!> meint %s kurz darauf und
				 * starrt noch immer den - nun leeren - Durchgang an." */
				sprintf(g_text_output_buf, ((struct area_events far *)ev)->txt[69], h->name);
				dlg_message(g_dialog, g_text_output_buf);
				g_dng05_poltergeist = 2;
				g_group->poltergeist_bound = 0;
				event_set_cell(ev, 0x11, 0x1a, 7, 0, -1);
				return;
			}
			if (g_dng05_poltergeist_ticks++ >= 2000) {
				g_dng05_poltergeist = 0;
				g_group->poltergeist_bound = 0;
				event_set_cell(ev, 0x11, 0x1a, 7, 0, -1);
				a = g_group->get_hero(group_find_item(g_group, ITEM_PHEX_HELMET));
				b = g_group->get_hero(group_find_item(g_group, ITEM_PHEX_SHIELD));
				if (a && b) {
					if (a != b)
						/* PHEXTEMP.LTX[62]: "<Ihr koennt Euch nicht einfach an Phexens Hab und Gut bereichern! Nicht solange ich
						 * existiere, denn ich bin ihr Waechter!> donnert der Poltergeist und nimmt %s den Helm und %s den Schild in
						 * einer rasend schnellen Bewegung ab. Danach loest er sich mitsamt der Artefakte vor Euren Augen in Luft
						 * auf." */
						sprintf(g_text_output_buf, ((struct area_events far *)ev)->txt[62],
							a->name, b->name);
					else
						/* PHEXTEMP.LTX[63]: "<Ihr koennt Euch nicht einfach an Phexens Hab und Gut bereichern! Nicht solange ich
						 * existiere, denn ich bin ihr Waechter!> donnert der Poltergeist und nimmt %s den Helm und den Schild in
						 * einer rasend schnellen Bewegung ab. Danach loest er sich mitsamt der Artefakte vor Euren Augen in Luft
						 * auf." */
						sprintf(g_text_output_buf, ((struct area_events far *)ev)->txt[63],
							a->name);
					group_drop_item(g_group, ITEM_PHEX_HELMET);
					group_drop_item(g_group, ITEM_PHEX_SHIELD);
				} else if (a) {
					/* PHEXTEMP.LTX[64]: "<Ihr koennt Euch nicht einfach an Phexens Hab und Gut bereichern! Nicht solange ich
					 * existiere, denn ich bin ihr Waechter!> donnert der Poltergeist und nimmt %s den Helm in einer rasend
					 * schnellen Bewegung ab. Danach loest er sich mitsamt der Artefakte vor Euren Augen in Luft auf." */
					sprintf(g_text_output_buf, ((struct area_events far *)ev)->txt[64], a->name);
					group_drop_item(g_group, ITEM_PHEX_HELMET);
				} else if (b) {
					/* PHEXTEMP.LTX[65]: "<Ihr koennt Euch nicht einfach an Phexens Hab und Gut bereichern! Nicht solange ich
					 * existiere, denn ich bin ihr Waechter!> donnert der Poltergeist und nimmt %s den Schild in einer rasend
					 * schnellen Bewegung ab. Danach loest er sich mitsamt der Artefakte vor Euren Augen in Luft auf." */
					sprintf(g_text_output_buf, ((struct area_events far *)ev)->txt[65], b->name);
					group_drop_item(g_group, ITEM_PHEX_SHIELD);
				} else
					/* PHEXTEMP.LTX[66]: "<Ihr seid schlauer, als ich dachte... Menschlein... doch gebrochen habt Ihr meine Macht
					 * nicht!> donnert der Poltergeist, nimmt den Helm und den Schild an sich und loest sich vor Euren Augen in
					 * Luft auf!" */
					strcpy(g_text_output_buf, ((struct area_events far *)ev)->txt[66]);
				snd_play(g_snd_fx, 0x87);
				dlg_message(g_dialog, g_text_output_buf);
#if FIX_0354
				/* Fix #354: Helm und Schild bei allen Gruppen suchen; nur zuruecklegen, was abgenommen wurde (keine Vervielfachung) */
				{
					/* fehlende Teile auch bei den anderen Gruppen abnehmen; nur zuruecklegen, was abgenommen wurde */
					signed short gi;
					struct group far *g;
					signed short helm = a != 0;
					signed short shield = b != 0;

					for (gi = 0, g = g_groups; gi < NR_GROUPS; gi++, g++) {
						if (g == g_group)
							continue;
						if (!helm && group_find_item(g, ITEM_PHEX_HELMET) != -1) {
							group_drop_item(g, ITEM_PHEX_HELMET);
							helm = 1;
						}
						if (!shield && group_find_item(g, ITEM_PHEX_SHIELD) != -1) {
							group_drop_item(g, ITEM_PHEX_SHIELD);
							shield = 1;
						}
					}
					ch = new Chest(0x30, 0);
					if (helm)
						ch->set_item(ITEM_PHEX_HELMET, 1, 0);
					if (shield)
						ch->set_item(ITEM_PHEX_SHIELD, 1, 1);
					delete ch;
				}
#else
				ch = new Chest(0x30, 0);
				ch->set_item(ITEM_PHEX_HELMET, 1, 0);
				ch->set_item(ITEM_PHEX_SHIELD, 1, 1);
				delete ch;
#endif
			}
		}
	}
}
