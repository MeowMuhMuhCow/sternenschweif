/* OVL131 - Overlay O131 (Segment 0x21ff in SCHWEIF.EXE): Ereignisse des Dungeons 3, des Gewoelbes des
 * Namenlosen-Kults unter dem Keller (Texte FINAL.LTX = Datei 0x2ad, gemeinsam mit Dungeon 1/2; g_area_descs[3]);
 * Ereignisobjekt ev (struct area_events, Texte ev->txt), Kartenereignis mev.
 * dng03_handler(ev, mev): pos = (Zeile - g_map_x0) << 8 | (Spalte - g_map_y0). MAP_EVENT_ENTER: 0x131d einmal
 *   Sprachausgabe 15 (g_dng03_voice_heard); 0x1e19/0x0b13/0x1216 Versetzung event_teleport; 0x151d Zahlenschloss
 *   (FINAL[55], dlg_input 0..9999: 1945 oeffnet die Wand (0x1d, 0x16), FINAL[56]; sonst Hohngelaechter FINAL[57], ab
 *   dem fuenften Fehlversuch Sprachausgabe 16 und FINAL[58] "Macht Euch die Tafel zunutze"); sonst dng03_fights.
 *   Danach auf einem neuen Feld ohne Ereignis mit 2 % die Vermutung einer Geheimtuer (FINAL[22]: weitersuchen, 30
 *   Minuten, FINAL[23] Enttaeuschung; g_dng03_last_pos). Typ 1: Schloss (Lock 1..11 je Position) mit Wandbild 9/10.
 *   Typ 2: Treppe 0x131b nach Dungeon 2 auf (0x13, 0x1c), sonst Spielende (swap_out_screen, g_game_state = 0x63). Typ
 *   0x68 Schreibtische: 0x105 Geheimfach (Sinnesschaerfe +5, FINAL[60]; Giftnadel bei misslungener Schloesser-Probe
 *   +15: W6 LE, FINAL[61]; Heiltrank FINAL[62]), danach FINAL[59]; 0x10b Abhandlung des Novizen (FINAL[28]), sonst
 *   FINAL[29]. Typ 0x6a Geheimtueren 0x110e/0xc0b (FINAL[22..24], Schloesser +12). Typ 0x6b Statuen: 0x617 fragt nach
 *   dem Namen des Herrn (FINAL[32], 8 Zeichen): ARKANDOR oeffnet die Wand (FINAL[34]), sonst Fluch und Kampf
 *   (FINAL[35], Gegner 0x52); vier Statuen mit eingemeisselten Buchstaben AR/OR/KA/ND (event_notice, FINAL[33]);
 *   0x11e Saeule mit der Drachenklaue (event_dragon_claw); sonst FINAL[25]/[27]/[30]; g_hero_excluded = -1. Typ 0x6c:
 *   Loch ohne Boden (FINAL[31]). Typ 0x6d: zwei Felder (3/4, 0x11) oeffnen. Typ 0x78: Wandbild nach der Drehung der
 *   Kartenzelle (rot 1/3 -> 0x36, sonst 0x37).
 * dng03_fights(ev, pos): einmalige Kaempfe (event_fight1/2, Merker g_dng03_<gegner>_<pos>; an 0xe1d mit Beute 0x149 in g_fight_defs_ab[0]);
 *   an 0x1608/0x1808/0x1707/0x1709 der Sonderkampf FIGHT_ID_SPECIAL (Gruppe beginnt): gewonnen -> der Salamanderstein
 *   kommt zurueck (wenn g_salamander_stolen), NSC 2 schliesst sich an (add_npc), Waende auf.
 * dng03_stone_thief(ev) (R12 bei g_dng03_thief_pending, nach der Treppe aus Dungeon 2): traegt ein Held den
 *   Salamanderstein, erscheint ein Mann in lila Aura und nimmt ihn (FINAL[126]/[127], Bild 0x45 in zwei Fenstern),
 *   g_salamander_stolen = 1.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y -E..\bc\BIN\TASM.EXE OVL131.CPP -> 4598/4598 Bytes GLEICH (18.10.2026), cobjfix
 *   laeuft. Ohne -Og 15 Bytes, ohne -Oi 50 Bytes zu kurz. -Og-Zwischenwerte [bp-0eh] = (int)mev->y, [bp-10h] =
 *   (int)mev->x (auch dort gespeichert, wo sie nicht wiederverwendet werden; Typ 0x78 benutzt sie ohne Neuladen), -Oi
 *   fuer strncmp. (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - C++ wegen Lock (O108, 0x29 Bytes, Block-Local im Fall Typ 1 bei [bp-3ah], Destruktor am Blockende).
 * - automap_cell_write ueber AUTOMAP_CELL_WRITE_W (letzter Parameter short, wie OVL063).
 * - type/pos/ret/x liegen im Speicher (strncmp-Intrinsic belegt SI/DI auf Funktionsebene), idx (Typ 1) als register
 *   im Block -> SI. dng03_fights: register-Parameter pos (SI), ret OHNE register -> DI.
 * - "(x = dlg_input(..)) == 1945" eingebettet (cmp ax); "(g_dng03_combination_tries += 1) >= 5" -> mov al / inc al /
 *   mov / cmp al.
 * - Kartenzelle als g_cells[x][y].rot (struct map_cell (far *)[64]) -> x << 8, y << 2, +3.
 * Originalfehler: dng03_fights gibt ret (DI) uninitialisiert zurueck, wenn keine Position passt oder im letzten Zweig
 *   (Sonderkampf) - dng03_handler nimmt den Muellwert als "Ereignis behandelt" und fuer die Zufallsvermutung.
 *   dng03_handler uebergibt event_teleport "mev->y - g_map_y0" statt "mev->y - g_map_x0" (wirkt nur richtig, solange
 *   beide Versaetze gleich sind).
 */
#include <stdio.h>
#include <string.h>

#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:631c..6600, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
/* BSS ds:cab8..caba: nur O131 benutzt es (die Kampf-Globalen davor ds:c948..cab8 liefert O128, vorher Reststueck DRO131) */
signed short g_dng03_last_pos;		/* ds:cab8 Feld des letzten Zufallsereignisses */
struct dxy g_fig_obj_dxy[124] = {
	{ -1, 6 },
	{ 4, 7 },
	{ 3, 7 },
	{ 4, 7 },
	{ -4, 7 },
	{ -4, 7 },
	{ -4, 7 },
	{ 2, 7 },
	{ 0, 6 },
	{ 1, 12 },
	{ -1, 17 },
	{ 4, 22 },
	{ 4, 27 },
	{ 3, 32 },
	{ -4, 28 },
	{ -4, 22 },
	{ -4, 17 },
	{ 0, 13 },
	{ 1, 14 },
	{ 1, 23 },
	{ 0, 6 },
	{ 0, 6 },
	{ 0, 5 },
	{ -1, 6 },
	{ 0, 5 },
	{ -1, 5 },
	{ 0, 5 },
	{ 0, 5 },
	{ 0, 6 },
	{ 0, 6 },
	{ -1, 5 },
	{ 0, 5 },
	{ -1, 4 },
	{ 0, 4 },
	{ -1, 4 },
	{ -1, 3 },
	{ 0, 4 },
	{ 0, 3 },
	{ 0, 6 },
	{ 0, 6 },
	{ -1, 5 },
	{ 1, 5 },
	{ 0, 5 },
	{ -2, 5 },
	{ -2, 5 },
	{ -1, 6 },
	{ 4, 7 },
	{ 4, 7 },
	{ 4, 7 },
	{ -4, 6 },
	{ -4, 6 },
	{ -4, 6 },
	{ 0, 6 },
	{ 0, 6 },
	{ 1, 12 },
	{ -1, 7 },
	{ 4, 7 },
	{ 4, 7 },
	{ 4, 7 },
	{ -4, 7 },
	{ -4, 7 },
	{ -4, 7 },
	{ 0, 7 },
	{ 0, 7 },
	{ 0, 13 },
	{ 3, 8 },
	{ -3, 6 },
	{ 1, 2 },
	{ -3, 6 },
	{ 1, 3 },
	{ -1, 5 },
	{ 1, 4 },
	{ -4, 6 },
	{ 3, 5 },
	{ 0, 5 },
	{ -2, 5 },
	{ 0, 4 },
	{ -8, 5 },
	{ 1, 4 },
	{ 7, 5 },
	{ -4, 7 },
	{ 3, 4 },
	{ -7, 8 },
	{ 3, 4 },
	{ -7, 8 },
	{ 4, 4 },
	{ 0, 3 },
	{ -1, 13 },
	{ 0, 5 },
	{ 34, 5 },
	{ 35, 42 },
	{ 0, 2 },
	{ 0, 2 },
	{ 0, 0 },
	{ -5, 16 },
	{ -5, 11 },
	{ 0, 6 },
	{ 14, 6 },
	{ -4, 6 },
	{ -5, 11 },
	{ 0, 6 },
	{ 0, 6 },
	{ 0, 6 },
	{ 0, 6 },
	{ 0, 6 },
	{ 0, 6 },
	{ 0, 6 },
	{ 0, 6 },
	{ -1, 4 },
	{ 0, 4 },
	{ 0, 4 },
	{ 1, 4 },
	{ -2, 4 },
	{ 0, 4 },
	{ 0, 4 },
	{ 1, 4 },
	{ -1, 4 },
	{ 0, 4 },
	{ 0, 4 },
	{ 1, 4 },
	{ -1, 4 },
	{ 0, 4 },
	{ 0, 4 },
	{ 1, 4 }
};
struct dxy g_fig_tail_dxy[4] = { { -1, 0 }, { 0, 1 }, { 1, 0 }, { 0, -1 } };	/* Schwanzfeld hinter einem Zweifeldgegner mit Blickrichtung 0..3 */
signed short g_fig_transparent_objs[20] = { 40, 41, 42, 43, 44, 74, 75, 76, 77, 78, 79, 93, 101, 98, 99, 100, 106, 107, 155, -1 };
signed short g_fig_origin_x = 10;
signed short g_fig_origin_y = 116;
signed char g_fig_stand_frames[1][4] = { { 1, 3, 5, 7 } };
signed char g_fig_hero_stand_wc4[4] = { -77, -75, -73, -71 };
signed char g_fig_hero_stand_wc6[4] = { -69, -67, -65, -63 };
signed char g_fig_hero_sleep_frames[4] = { 10, 11, 12, 13 };
signed char g_fig_stone_frames[4] = { -61, -60, -59, -58 };
signed char g_fig_step_frames[4] = { 14, 16, 18, 20 };
signed char g_fig_step2_frames[4] = { 22, 24, 26, 28 };
signed char g_fig_attack_frames[4] = { 71, 73, 75, 77 };
signed char g_fig_parry_frames[4] = { 111, 113, 115, 117 };
signed char g_fig_hero_attack_frames[4][4] = { { 55, 57, 59, 61 }, { 63, 65, 67, 69 }, { 39, 41, 43, 45 }, { 39, 41, 43, 45 } };
signed char g_fig_action_frames[7][4] = {
	{ 95, 97, 99, 101 },
	{ 103, 105, 107, 109 },
	{ 79, 81, 83, 85 },
	{ 79, 81, 83, 85 },
	{ -113, -111, -109, -107 },
	{ -121, -119, -117, -115 },
	{ -105, -103, -101, -99 }
};
signed char g_fig_spell_frames[2][4] = { { 119, 121, 123, 125 }, { 127, -127, -125, -123 } };
char g_unused_65a0[4] = { 0 };   /* ds:65a0, unbenutzt */
signed char g_fig_spell_hit_frames[4] = { 30, 32, 34, 36 };
signed char g_fig_hero_use_frames[4] = { -97, -95, -93, -91 };
signed char g_fig_hero_throw_frames[4] = { -89, -87, -85, -83 };
signed char g_fig_hero_pose_frames[4] = { -81, -80, -79, -78 };
unsigned char g_twofield_mons[13] = { 0x05, 0x23, 0x24, 0x47, 0x4b, 0x5e, 0x69, 0x6c, 0x6e, 0x6f, 0x51, 0x72, 0xff };
unsigned char g_mons_corpse_blocks[4] = { 0x72, 0x60, 0x61, 0xff };
unsigned char g_gfx_humanoid[24] = {
	0x19, 0x1b, 0x1f, 0x20, 0x21, 0x22, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x33, 0x34, 0x37, 0x38, 0x39,
	0x3a, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0xff
};
unsigned char g_gfx_undead[8] = { 0x19, 0x1b, 0x1f, 0x3f, 0x40, 0x26, 0x28, 0xff };
signed short g_fight_flee_all = 0;
char g_unused_65e7[1] = { 0 };   /* ds:65e7, unbenutzt */

/* automap_cell_write mit short als letztem Parameter (wie im Original deklariert) */
#define AUTOMAP_CELL_WRITE_W \
	((void (*)(void far *, signed short, signed short, signed short, signed short))automap_cell_write)

signed short dng03_handler(struct area_events far *ev, struct map_event far *mev)
{
	signed short type;
	signed short pos;
	signed short ret;
	signed short x;
	struct hero far *h;

	type = mev->type;
	pos = (mev->y - g_map_x0) << 8;
	pos |= mev->x - g_map_y0;
	h = get_first_active_hero(g_group);
	ret = 0;

	if (type == MAP_EVENT_ENTER) {
		if (!g_dng03_voice_heard && pos == 0x131d) {
			snd_speech(g_snd_speech, 0x0f);
			g_dng03_voice_heard = 1;
		} else if (pos == 0x1e19 || pos == 0x0b13 || pos == 0x1216)
#if FIX_0102
			/* Fix #102: Zeile mit g_map_x0 wie bei pos */
			event_teleport(ev, mev->y - g_map_x0);
#else
			event_teleport(ev, mev->y - g_map_y0);
#endif
		else if (pos == 0x151d && !g_dng03_combination_open) {
			h = get_best_hero_skill(g_group, TA_SINNESSCHAERFE);
			/* FINAL.LTX[55]: "<Holla,> ruft %s entzueckt aus und wendet sich der Wand vor Euch zu, <ein
			 * Zahlenschloss.>@Tatsaechlich entdeckt %s, gut unter einem schmalen Sims versteckt ein Schloss.@Welche Ziffer
			 * wollt Ihr einstellen?" */
			sprintf(g_text_output_buf, ev->txt[55], h->name, grammar_pronoun(h->sex, 0, 0));
			if ((x = dlg_input(g_dialog, 4, g_text_output_buf, 0, 9999, 0)) == 1945) {
				event_set_cell(ev, 0x1d, 0x16, 0x13, 0, 1);
				AUTOMAP_CELL_WRITE_W(g_map, 0x26a, 0x16, 0x1d, 0x1a);
				snd_play(g_snd_fx, 0x55);
				/* FINAL.LTX[56]: "Nachdem Ihr die Nummer eingestellt habt, hoert Ihr neben Euch ein schweres, schabendes
				 * Geraeusch, als ob Stein auf Stein reibt." */
				dlg_message(g_dialog, ev->txt[56]);
				g_dng03_combination_open = 1;
			} else if (x != -1) {
				if ((g_dng03_combination_tries += 1) >= 5) {
					snd_speech(g_snd_speech, 0x10);
					if (g_subtitles != 0)
						/* FINAL.LTX[58]: "Ihr vernehmt ein hoehnische koerperloses Lachen, das sich durch das Gewoelbe zieht. Nach
						 * kurzer Zeit spricht eine Stimme aus dem Nichts zu Euch.@>Macht Euch die Tafel zunutze, Ihr
						 * Prembuttnasen!>" */
						dlg_message(g_dialog, ev->txt[58]);
				} else {
					snd_play(g_snd_fx, 0xaa);
					if (g_subtitles != 0)
						/* FINAL.LTX[57]: "Ihr vernehmt ein hoehnisches, koerperloses Lachen, das sich durch das Gewoelbe zieht." */
						dlg_message(g_dialog, ev->txt[57]);
				}
			}
			ret = 1;
		} else
			ret = dng03_fights(ev, pos);
		if (g_dng03_last_pos != pos && ret == 0) {
			if (random_schweif(100) <= 2) {
				/* FINAL.LTX[22]: "%s hat eine Geheimtuer entdeckt, aber Ihr koennt den Oeffnungsmechanismus nicht
				 * finden!@Wollt Ihr weiter danach suchen?" */
				sprintf(g_text_output_buf, ev->txt[22], h->name);
				if (dlg_yes_no(g_dialog, g_text_output_buf) == 1) {
					timewarp_minutes(g_date, 30L);
					/* FINAL.LTX[23]: "Je laenger Ihr sucht, desto sicherer wird sich %s, dass %s sich getaeuscht hat. Enttaeuscht
					 * gebt Ihr irgendwann auf." */
					sprintf(g_text_output_buf, ev->txt[23], h->name, grammar_pronoun(h->sex, 0,
						0));
					dlg_message(g_dialog, g_text_output_buf);
				}
				ret = 1;
			}
			g_dng03_last_pos = pos;
		}
	} else if (type == 1) {
		register signed short idx = -1;
		switch (pos) {
		case 0x0f1d: idx = 1; x = 10; break;
		case 0x1318: idx = 2; x = 9; break;
		case 0x1717: idx = 3; x = 9; break;
		case 0x0a17: idx = 4; x = 10; break;
		case 0x090d: idx = 5; x = 9; break;
		case 0x0d14: idx = 6; x = 10; break;
		case 0x0f13: idx = 7; x = 9; break;
		case 0x1613: idx = 8; x = 10; break;
		case 0x1512: idx = 9; x = 9; break;
		case 0x1911: idx = 10; x = 10; break;
		case 0x0c01: idx = 11; x = 10; break;
		}
		if (idx != -1) {
			Lock lk(idx);
			ret = 1;
			if (g_lock_cell_bits != -1) {
				event_set_cell_bits(ev, mev->x, mev->y, g_lock_cell_bits);
				AUTOMAP_CELL_WRITE_W(g_map, 0x26a, mev->y - g_map_x0, mev->x - g_map_y0, x);
				snd_play(g_snd_fx, 0x61);
			}
		}
	} else if (type == 2) {
		if (pos == 0x131b) {
			group_set_screen_pos(0x13, 0x1c);
			g_group->old_dungeon = g_group->dungeon;
			g_group->dungeon = 2;
			g_group->dir = 2;
		} else {
			swap_out_screen();
			g_game_state = 0x63;
		}
	} else if (type == 0x68) {
		if (pos == 0x010b)
			/* FINAL.LTX[28]: "Ihr findet eine Abhandlung, geschrieben von einem Novizen, die sich detailiert mit der
			 * Schwaechung des Zwoelfgoettlichen Glaubens befasst, die durch den Orkkrieg erfolgen wird. Das Erschreckendste
			 * daran ist, dass sich der Autor dieses Machwerks ueber diese Tatsache ausserordentlich zu freuen scheint..." */
			dlg_message(g_dialog, ev->txt[28]);
		else if (pos == 0x0105) {
			if (!g_dng03_desk_opened) {
				if (test_skill(h, TA_SINNESSCHAERFE, 5) >= 0) {
					/* FINAL.LTX[60]: "<Dachte ich's doch!> ruft %s entzueckt aus als %s den Schreibtisch abtastet.@<Ein
					 * Geheimfach... und es laesst sich oeffnen!>" */
					sprintf(g_text_output_buf, ev->txt[60], h->name, grammar_pronoun(h->sex, 0,
						0));
					dlg_message(g_dialog, g_text_output_buf);
					g_dng03_desk_opened = 1;
					if (test_skill(h, TA_SCHLOESSER, 0x0f) < 0) {
						/* FINAL.LTX[61]: "<Autsch!> entfaehrt es %s da. Eine versteckte Giftnadel sticht %s ins Handgelenk. Zum
						 * Glueck scheint das Gift schon so alt zu sein, dass es keinen ernsten Schaden mehr anrichtet." */
						sprintf(g_text_output_buf, ev->txt[61], h->name,
							grammar_pronoun(h->sex, 2, 0));
						dlg_message(g_dialog, g_text_output_buf);
						sub_hero_le(h, random_schweif(6));
					}
					/* FINAL.LTX[62]: "In dem Geheimfach steht eine verstaubte Phiole mit Heiltrank.@Wollt Ihr sie mitnehmen?" */
					if (dlg_yes_no(g_dialog, ev->txt[62]) == 1)
						group_give_item(g_group, ITEM_STRONG_LE_POTION, 1);
				}
			} else {
				h = get_worst_hero_skill(g_group, TA_SCHLEICHEN);
				/* FINAL.LTX[59]: "<Na prima> mault %s ploetzlich los. <Da haben wir vorhin schon mal nachgesehen. Habt Ihr
				 * vielleicht maraskanische Tomaten auf den Augen?>" */
				sprintf(g_text_output_buf, ev->txt[59], h->name);
				dlg_message(g_dialog, g_text_output_buf);
			}
		} else
			/* FINAL.LTX[29]: "Das ist ein gewoehnlicher Schreibtisch." */
			dlg_message(g_dialog, ev->txt[29]);
		ret = 1;
	} else if (type == 0x6a) {
		if (pos == 0x110e) {
			if (!g_dng03_secret_door1) {
				sprintf(g_text_output_buf, ev->txt[22], h->name);
				if (dlg_yes_no(g_dialog, g_text_output_buf) == 1) {
					if (test_skill(h, TA_SCHLOESSER, 0x0c) >= 0) {
						/* FINAL.LTX[24]: "Nach weiterem Suchen entdeckt %s endlich den Oeffnungsmechanismus der Tuer und mit einem
						 * lauten Grollen oeffnet sich vor Euch ein Durchgang in der Wand." */
						sprintf(g_text_output_buf, ev->txt[24], h->name,
							grammar_pronoun(h->sex, 3, 0));
						event_set_cell(ev, 0x0e, 0x11, 0x0e, 0, -1);
						AUTOMAP_CELL_WRITE_W(g_map, 0x26a, 0x11, 0x0e, 0x38);
						snd_play(g_snd_fx, 0xac);
						g_dng03_secret_door1 = 1;
					} else
						sprintf(g_text_output_buf, ev->txt[23], h->name,
							grammar_pronoun(h->sex, 0, 0));
					dlg_message(g_dialog, g_text_output_buf);
				}
				ret = 1;
			}
		} else if (pos == 0x0c0b && !g_dng03_secret_door2) {
			sprintf(g_text_output_buf, ev->txt[22], h->name);
			if (dlg_yes_no(g_dialog, g_text_output_buf) == 1) {
				if (test_skill(h, TA_SCHLOESSER, 0x0c) >= 0) {
					sprintf(g_text_output_buf, ev->txt[24], h->name, grammar_pronoun(h->sex, 3,
						0));
					event_set_cell(ev, 0x0b, 0x0c, 0x0e, 0, -1);
					AUTOMAP_CELL_WRITE_W(g_map, 0x26a, 0x0c, 0x0b, 0x37);
					snd_play(g_snd_fx, 0xac);
					g_dng03_secret_door2 = 1;
				} else
					sprintf(g_text_output_buf, ev->txt[23], h->name, grammar_pronoun(h->sex, 0,
						0));
				dlg_message(g_dialog, g_text_output_buf);
			}
			ret = 1;
		}
	} else if (type == 0x6b) {
		if (!g_dng03_password_solved && pos == 0x0617) {
			/* FINAL.LTX[32]: "Als Ihr Euch an der Statue zu schaffen macht, ertoent eine sonore Stimme.@<Nennt mir den
			 * Namen des Herrn dieser Gemaeuer und ich werde mein Geheimnis preisgeben!>" */
			dlg_message_flag(g_dialog, 8, ev->txt[32]);
			if (strncmp(g_input_buf, "ARKANDOR", 8) == 0) {
				event_set_cell(ev, 0x1c, 4, 0, 0, -1);
				AUTOMAP_CELL_WRITE_W(g_map, 0x26a, 4, 0x1c, 2);
				snd_play(g_snd_fx, 0x9f);
				/* FINAL.LTX[34]: "<Jawohl. Dies ist des Herrn Name... und er erwartet Euch bereits!> meint die Stimme
				 * hoehnisch. Aus nicht allzu weiter Ferne hoert Ihr ein schweres, scharrendes Geraeusch!" */
				dlg_message(g_dialog, ev->txt[34]);
				g_dng03_password_solved = 1;
			} else {
				snd_speech(g_snd_speech, 0x11);
				/* FINAL.LTX[35]: "<Verflucht seiet Ihr, Unwuerdige!> bruellt die Stimme laut, so dass sie in der Halle nahezu
				 * endlos widerhallt und ploetzlich bemerkt Ihr, dass sich hinter Euch etwas bewegt!" */
				dlg_message(g_dialog, ev->txt[35]);
				event_fight1(ev, 0x52, 2, 0);
			}
		} else if (pos == 0x0615)
			event_notice(ev, "AR");
		else if (pos == 0x0619)
			event_notice(ev, "OR");
		else if (pos == 0x0314)
			event_notice(ev, "KA");
		else if (pos == 0x0316)
			event_notice(ev, "ND");
		else if (pos == 0x0901 || pos == 0x0806)
			/* FINAL.LTX[30]: "Dieses <Kunstwerk> stellt das dar, was sich ein krankes Hirn unter der Vermehrung von
			 * Daemonen vorstellen koennte." */
			dlg_message(g_dialog, ev->txt[30]);
		else if (pos == 0x0e06)
			/* FINAL.LTX[25]: "Diese Statue ist eine recht einfache Arbeit, aber es ist dennoch eindeutig zu erkennen, dass
			 * sie den Gott ohne Namen darstellt!" */
			dlg_message(g_dialog, ev->txt[25]);
		else if (pos == 0x011e && !g_dragon_claw_found)
			event_dragon_claw(ev, h);
		else
			/* FINAL.LTX[27]: "Diese Statue stellt offensichtlich eine Art Daemon dar." */
			dlg_message(g_dialog, ev->txt[27]);
		ret = 1;
		g_hero_excluded = -1;
	} else if (type == 0x6c) {
		/* FINAL.LTX[31]: "Das Loch scheint endlos in die Tiefe zu fuehren. Einen hineingeworfenen Stein hoert man nicht
		 * aufprallen aber noch schlimmer sind die Reste von Blut an der Kante." */
		dlg_message(g_dialog, ev->txt[31]);
		ret = 1;
	} else if (type == 0x6d) {
		event_set_cell_bits(ev, g_map_y0 + 3, g_map_x0 + 0x11, 0x2000);
		event_set_cell_bits(ev, g_map_y0 + 4, g_map_x0 + 0x11, 0x2000);
		AUTOMAP_CELL_WRITE_W(g_map, 0x26a, 0x11, 3, 0x18);
		AUTOMAP_CELL_WRITE_W(g_map, 0x26a, 0x11, 4, 0x19);
		snd_play(g_snd_fx, 0x61);
	} else if (type == 0x78) {
		x = g_cells[mev->x][mev->y].rot;
		x = (x == 1 || x == 3) ? 0x36 : 0x37;
		AUTOMAP_CELL_WRITE_W(g_map, 0x26a, mev->y - g_map_x0, mev->x - g_map_y0, x);
	}
	return ret;
}

signed short dng03_fights(struct area_events far *ev, register signed short pos)
{
#if FIX_0102
	/* Fix #102: Rueckgabe vorbelegen (keine Position / Sonderkampf) */
	signed short ret;

	ret = 0;
	if (pos == 0x171d && !g_dng03_skeleton_warriors_171d) {
#else
	signed short ret;

	if (pos == 0x171d && !g_dng03_skeleton_warriors_171d) {
#endif
		if (event_fight1(ev, 0x5d, 3, 1) == 0)
			g_dng03_skeleton_warriors_171d = 1;
		ret = 1;
	} else if (pos == 0x191c && !g_dng03_skeletons_191c) {
		if (event_fight1(ev, 0x74, 6, 0) == 0)
			g_dng03_skeletons_191c = 1;
		ret = 1;
	} else if (pos == 0x1519 && !g_dng03_zombies_1519) {
		if (event_fight1(ev, 0x73, 2, 1) == 0)
			g_dng03_zombies_1519 = 1;
		ret = 1;
	} else if (pos == 0x041d && !g_dng03_skeleton_warriors_041d) {
		if (event_fight1(ev, 0x5d, 2, 0) == 0)
			g_dng03_skeleton_warriors_041d = 1;
		ret = 1;
	} else if (pos == 0x1b15 && !g_dng03_zombies_1b15) {
		if (event_fight2(ev, 0x73, 3, 2, 0x74, 2, 0) == 0)
			g_dng03_zombies_1b15 = 1;
		ret = 1;
	} else if (pos == 0x0b17 && !g_dng03_skeleton_warriors_0b17) {
		if (event_fight2(ev, 0x5d, 2, 0, 0x74, 4, 0) == 0)
			g_dng03_skeleton_warriors_0b17 = 1;
		ret = 1;
	} else if (pos == 0x1313 && !g_dng03_zombies_1313) {
		if (event_fight2(ev, 0x73, 2, 0, 0x10, 2, 0) == 0)
			g_dng03_zombies_1313 = 1;
		ret = 1;
	} else if (pos == 0x130e && !g_dng03_heshthot_130e) {
		if (event_fight1(ev, 0x11, 1, 0) == 0)
			g_dng03_heshthot_130e = 1;
		ret = 1;
	} else if (pos == 0x080c && !g_dng03_sorcerer_080c) {
		if (event_fight1(ev, 0x31, 1, 0) == 0)
			g_dng03_sorcerer_080c = 1;
		ret = 1;
	} else if (pos == 0x0702 && !g_dng03_mages_0702) {
		if (event_fight2(ev, 0x2e, 1, 2, 0x39, 2, 0) == 0)
			g_dng03_mages_0702 = 1;
		ret = 1;
	} else if (pos == 0x0704 && !g_dng03_mages_0704) {
		if (event_fight2(ev, 0x2e, 1, 2, 0x39, 2, 0) == 0)
			g_dng03_mages_0704 = 1;
		ret = 1;
	} else if (pos == 0x0d01 && !g_dng03_priests_0d01) {
		if (event_fight2(ev, 0x3a, 4, 0, 0x38, 2, 2) == 0)
			g_dng03_priests_0d01 = 1;
		ret = 1;
	} else if (pos == 0x0e1d && !g_dng03_knight_0e1d) {
		g_fight_defs_ab[0].loot[0] = 0x149;	/* Beute des Kampfes 0xab: Kult-Dokument */
		if (event_fight1(ev, 0x67, 1, 0) == 0)
			g_dng03_knight_0e1d = 1;
		ret = 1;
	} else if (pos == 0x0e0c && !g_dng03_sorcerer_0e0c) {
		if (event_fight1(ev, 0x31, 1, 0) == 0)
			g_dng03_sorcerer_0e0c = 1;
		ret = 1;
	} else if ((pos == 0x1608 || pos == 0x1808 || pos == 0x1707 || pos == 0x1709) && !g_final_fight_won &&
		start_fight(FIGHT_ID_SPECIAL, 1) == 0) {
		g_final_fight_won = 1;
		if (g_salamander_stolen == 1)
			group_give_item(g_group, ITEM_SALAMANDER_STONE, 1);
		add_npc(2);
		screen_portraits(g_screen);
		event_set_cell(ev, 4, 0x15, 0, 0, -1);
		AUTOMAP_CELL_WRITE_W(g_map, 0x26a, 0x15, 4, 5);
		AUTOMAP_CELL_WRITE_W(g_map, 0x26a, 0x15, 5, 1);
		event_set_cell(ev, 8, 0x17, 0, 0, -1);
		AUTOMAP_CELL_WRITE_W(g_map, 0x26a, 0x17, 8, 1);
	}
	return ret;
}

void dng03_stone_thief(void far *ev)
{
	signed short old;
	void far *w;
	struct hero far *h;
	struct hero far *h2;

	if (!g_dng03_stone_stolen && (h = get_hero_with_item(g_group, ITEM_SALAMANDER_STONE)) != 0) {
		old = dlg_set16(g_dialog, 5);
		h2 = get_random_hero(g_group);
		w = Anim_ctor(0, 0x45, 0, 0);
		/* FINAL.LTX[126]: "<Was ist denn nun los?> meint %s und ploetzlich macht sich Unruhe in Eurer Gruppe breit. Ihr
		 * werdet heftig beiseite gestossen und dann erscheint wenig vor Euch ein Mann. Komplett in eine schillernde
		 * lilafarbene Aura gehuellt, steht er schallend lachend da und haelt den Salamanderstein in die Hoehe!@<Nein!>
		 * entfaehrt es %s als %s %s Gepaeck vergeblich nach dem Stein durchsucht.@<Der Schweinepriester hat sich
		 * unsichtbar gemacht und uns den Stein gestohlen!>" */
		sprintf(g_text_output_buf, ((struct area_events far *)ev)->txt[126], h2->name, h->name,
			grammar_pronoun(h->sex, 0, 0),
			grammar_pronoun(h->sex, 1, 0));
		dlg_box_anim(g_dialog, w, "", g_text_output_buf, 0);
		Anim_dtor(w, 3);
		w = Anim_ctor(0, 0x45, 0, 0);
		/* FINAL.LTX[127]: "Langsam, als schwebe er, entfernt sich der Mann von Euch und meint haemisch@<Ihr haettet den
		 * Stein besser nach Lowangen gebracht, aber Ihr wolltet ja partout anders. Auch wenn es mir Bewunderung
		 * abnoetigt, dass Ihr es ueberhaupt bis hierher geschafft habt, denke ich, wird Eure Reise hier enden. Seht, es
		 * passt nicht in unsere Plaene, dass Elfen und Zwerge eine Allianz bilden; und mein Meister will den
		 * Stein...>@Mit diesen Worten verschwindet er einfach durch eine Wand!" */
		dlg_box_anim(g_dialog, w, "", ((struct area_events far *)ev)->txt[127], 0);
		Anim_dtor(w, 3);
		group_remove_item(g_group, ITEM_SALAMANDER_STONE);
		g_salamander_stolen = 1;
		dlg_set16(g_dialog, old);
		g_dng03_stone_stolen = 1;
		g_dng03_thief_pending = 0;
	}
}
