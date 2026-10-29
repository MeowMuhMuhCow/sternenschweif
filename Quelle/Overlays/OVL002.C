/* OVL002 - Overlay O002 (Segment 0x1ef9 in SCHWEIF.EXE): Ereignisse des Dungeons 16 (Fortsetzung von OVL001) mit den
 * Golem-Zwischensequenzen (Ereignisobjekt ev; Texte BINGE7.LTX, bei Bedarf per txt_load_list(g_font, 0x45a, 0x5a)).
 * dng16_seq_golem_cb/dng16_seq_forge_cb(bild, ende): Bild-Rueckrufe fuer fli_play (Sequenzen 0x0f/0x10) - Strg-Q
 *   (bioskey 0x1011) bricht ab, an Bild 0x0f/0x48/0x60 bzw. 0x29/0xfe/0x1ab je eine Sprachausgabe snd_speech in
 *   fester
 *   Reihenfolge (Zustand g_dng16_seq_golem_state bzw. g_dng16_seq_forge_state), Rueckgabe 1 = Ende (Bild >= ende).
 * dng16_water_basin: Wasserbecken mit Gitter - Wasser probieren (Durst weg, Schlaeuche fuellen, verrusste Helden
 *   waschen sich: CH + 2, hero.sooty = 0) bzw. Gitter untersuchen (GE-Probe +2, sonst nass: Dumpfschaedel-Probe;
 *   Merker g_dng16_grid_moved/g_dng16_hero_wet), Gitter zurueckschieben (zweiter Held) oder Ansehen -10.
 * dng16_lava_wall: Wandstueck mit Mustern (Hinweis bei g_parchments_read), brennende Fackel benutzen -> Feld (10, 11) offen,
 *   zwei Automap-Eintraege. dng16_stone_door: geschlossene Steintuer oeffnen bzw. Foramen (Magier), bis dreimal.
 * dng16_golem: Palette sichern (mem_alloc_w(0x300)), mit Sprachdatei Zwischensequenz 0x0f und eigene Texte, Frage
 *   angreifen/fliehen/abwarten: 1 dng16_golem_fight, 2 zurueck (group_set_screen_pos/cam_set_from_group), sonst
 *   Dialog GOLEM2.TLK (talk(&g_talk, 0x13c, ..)); mit Steinmedaillon (g_dng16_medallion_given) Sequenz 0x10 bzw.
 *   Texte, dng16_forge_heat, g_dng16_forging = 1, g_dng16_forge_timer = 5; Musik zurueck.
 * dng16_golem_forge: Einstieg (swap_out_screen, dng16_golem) bzw. Folgebesuche (Golem schmiedet noch: Gespraech
 *   0xa5; fertig: Asthenilschwert, sofern nicht gegen den Salamanderstein getauscht, g_dng16_wall_timer = 3).
 * dng16_golem_fight: event_fight1(0x62) == 0 -> g_dng16_sword_done = g_dng16_golem_fought = g_dng16_sacrilege = 1.
 * dng16_forge_heat: alle Helden ohne Lederhose/-stiefel/-wams an Platz 0..14 sammeln, Namensliste mit ", "/"und"
 *   (g_ttx[420]/[422]), Meldung txt[65]/[66] (grammar_pronoun-Formen), diese Helden sterben (sub_hero_le mit LE),
 *   Rueckgabe 0, wenn alle betroffen sind.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL002.C   -> 5054/5054 Bytes, ccmp "1 anders" (0xc3d, siehe unten), sonst GLEICH
 *   (18.10.2026), cobjfix laeuft (g_dng16_heat_marks_init -> _g_00a6+2). Ohne -Og 4 Bytes zu kurz. Kein -Oi:
 *   memset/strcat sind
 *   im Original echte Aufrufe (wie O032).
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH; ccmp jetzt auch ohne den Fehlalarm 0xc3d)
 * - Erster Entwurf sofort GLEICH (reines C, kein register noetig).
 * - Rueckrufe dng16_seq_golem_cb/dng16_seq_forge_cb als far-Funktionszeiger an fli_play: im Original "push seg
 *   OVL002_TEXT / push 43h"
 *   bzw. "push 48h" = Stub-Thunks 0x20 + 5n der Publics Nr. 7/8 (cobjfix setzt die Publics in Originalreihenfolge,
 *   wie bei O048 f_087a). ccmp (selfoff.py) meldet 0xc3d, weil es fuer alle Verweise ins eigene Segment denselben
 *   Versatz erwartet (0x43 - 0x0000 gegenueber 0x48 - 0x00b5) - bei zwei Thunks ein Fehlalarm des Werkzeugs; im
 *   Komplettlink pruefen.
 * - Lokale Kopie "struct score7 sel = g_dng16_heat_marks_init;" (7 Worte, rep movsw) als initialisiertes Aggregat -
 *   ein Initialisierer
 *   {0, ..} wuerde eigene Daten erzeugen; liegt als grosses Local am Ende ([bp-2ah]).
 * - Geschwister-Bloecke in dng16_water_basin teilen [bp-0ah]: "char far *s" (zweimal an sprintf) bzw. "struct hero
 *   far *h2"
 *   (h2->name als -Og-Zwischenwert in DI).
 * - "g_dng16_grid_moved = g_dng16_hero_wet = 0", "g_dng16_sacrilege = g_dng16_sword_done = g_dng16_golem_fought = 1"
 *   (rechts zuerst gespeichert); "g_god_favor_ingerimm -= 10" (long).
 * Originalfehler: dng16_stone_door meldet txt[57] nur bei cnt > 3, die Schleife endet aber bei cnt == 3
 *   (unerreichbar).
 *   dng16_forge_heat zaehlt k fuer jeden Helden herunter statt nur fuer die aufgelisteten - "und" vor dem letzten
 *   Namen steht
 *   daher nur zufaellig richtig; "flag == 0" im Zweig cnt > 1 ist immer wahr. dng16_golem_fight: "r = 0" wird sofort
 *   ueberschrieben.
 */
/* Phase B: BCC (C-Modus) legt das BSS in umgekehrter Reihenfolge der ersten Deklaration an - Reihenfolge hier vor schweif.h */
extern void far *g_dng16_pal_save;
#include <stdio.h>
#include <string.h>
#include <bios.h>

#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:0094..00bc, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
signed short g_dng16_seq_pics[7] = { -15331, -15330, -15329, -15328, -15326, -15325, -1 };
unsigned short g_dng16_seq_pics_state[1] = { 0x0000 };
signed short g_dng16_seq_golem_state = 0;
signed short g_dng16_seq_forge_state = 0;
struct score7 g_dng16_heat_marks_init = { { 0, 0, 0, 0, 0, 0, 0 } };
/* Phase B: eigenes BSS (ds:8b4e..8b52) in Adressreihenfolge (Reihenfolge festgelegt durch die extern-Zeilen vor den #include) */
void far *g_dng16_pal_save;

void dng16_golem_fight(struct area_events far *ev);
signed short dng16_forge_heat(struct area_events far *ev);

signed short dng16_seq_golem_cb(unsigned short v, unsigned short lim)
{
	signed short ret = 0;

	if (bioskey(1) && bioskey(0) == 0x1011) {
		ret = 1;
		g_dng16_seq_golem_state = 0;
	}
	switch (v) {
	case 0x0f:
		if (g_dng16_seq_golem_state == 0) {
			g_dng16_seq_golem_state = 1;
			snd_speech(g_snd_speech, 0x41f);
		}
		break;
	case 0x48:
		if (g_dng16_seq_golem_state == 1) {
			g_dng16_seq_golem_state = 2;
			snd_speech(g_snd_speech, 0x420);
		}
		break;
	case 0x60:
		if (g_dng16_seq_golem_state == 2) {
			g_dng16_seq_golem_state = 3;
			snd_speech(g_snd_speech, 0x423);
		}
		break;
	}
	if (v >= lim) {
		ret = 1;
		g_dng16_seq_golem_state = 0;
	}
	return ret;
}

signed short dng16_seq_forge_cb(unsigned short v, unsigned short lim)
{
	signed short ret = 0;

	if (bioskey(1) && bioskey(0) == 0x1011) {
		ret = 1;
		g_dng16_seq_forge_state = 0;
	}
	switch (v) {
	case 0x29:
		if (g_dng16_seq_forge_state == 0) {
			g_dng16_seq_forge_state = 1;
			snd_speech(g_snd_speech, 0x41d);
		}
		break;
	case 0xfe:
		if (g_dng16_seq_forge_state == 1) {
			g_dng16_seq_forge_state = 2;
			snd_speech(g_snd_speech, 0x422);
		}
		break;
	case 0x1ab:
		if (g_dng16_seq_forge_state == 2) {
			g_dng16_seq_forge_state = 3;
			snd_speech(g_snd_speech, 0x41e);
		}
		break;
	}
	if (v >= lim) {
		ret = 1;
		g_dng16_seq_forge_state = 0;
	}
	return ret;
}

void dng16_water_basin(struct area_events far *ev)
{
	struct hero far *h;
	signed short old;
	signed short ans;

	old = dlg_set16(g_dialog, 5);
	if (!g_dng16_grid_moved) {
		/* BINGE7.LTX[0]: "Wollt Ihr" */
		/* BINGE7.LTX[31]: "das Wasser probieren?" */
		/* BINGE7.LTX[32]: "das Gitter untersuchen?" */
		ans = dlg_choose(g_dialog, ev->txt[0], 2, ev->txt[31], ev->txt[32]);
		if (ans == 1) {
			timewarp_minutes(g_date, 5L);
			/* BINGE7.LTX[33]: "Das Wasser ist erfrischend, kuehl und durststillend. Eure Stimmung bessert sich und nachdem
			 * Ihr Eure Wasserschlaeuche gefuellt habt, seid Ihr wieder zu weiteren Taten bereit." */
			dlg_message(g_dialog, ev->txt[33]);
			group_clr_thirst(g_group);
			group_refill_water(g_group);
			h = get_first_hero(g_group);
			do {
				if (check_hero3(h) && h->sooty) {
					/* BINGE7.LTX[30]: "Ausserdem nutzt %s die Gelegenheit um sich den Russ aus den Kleidern zu entfernen und sich
					 * das Gesicht gruendlich zu waschen." */
					sprintf(g_text_output_buf, ev->txt[30], h->name);
					dlg_message(g_dialog, g_text_output_buf);
					change_attrib(h, ATTRIB_CH, 2);
					h->sooty = 0;
				}
				h = get_next_hero(g_group, h);
			} while (get_first_hero(g_group) != h);
		} else if (ans == 2) {
			timewarp_minutes(g_date, 5L);
			h = get_first_active_hero(g_group);
			if (test_attrib(h, ATTRIB_GE, 2) <= 0) {
				/* BINGE7.LTX[34]: "Das Gitter laesst sich ohne grosse Muehe von der Oeffnung schieben, %s ist dabei allerdings
				 * platschnass geworden." */
				sprintf(g_text_output_buf, ev->txt[34], h->name);
				hero_disease_test(h, ILLNESS_DUMPFSCHAEDEL, 10);
				g_dng16_hero_wet = 1;
			} else
				/* BINGE7.LTX[35]: "Das Gitter laesst sich ohne grosse Muehe von der Oeffnung schieben, %s hat mehr Probleme
				 * damit, bei der Aktion nicht nass zu werden." */
				sprintf(g_text_output_buf, ev->txt[35], h->name);
			dlg_message(g_dialog, g_text_output_buf);
			g_dng16_grid_moved = 1;
			/* BINGE7.LTX[36]: "Wollt Ihr die Oeffnung untersuchen?" */
			if (dlg_yes_no(g_dialog, ev->txt[36]) == 1) {
				if (!g_dng16_hero_wet) {
					/* BINGE7.LTX[37]: "%s bemueht sich zwar sehr, aber ohne nass zu werden, laesst sich die Oeffnung nicht
					 * genauer untersuchen." */
					sprintf(g_text_output_buf, ev->txt[37], h->name);
					dlg_message(g_dialog, g_text_output_buf);
					g_dng16_hero_wet = 1;
				}
				hero_disease_test(h, ILLNESS_DUMPFSCHAEDEL, 20);
				/* BINGE7.LTX[38]: "Es handelt sich bei der Oeffnung um ein gewoehnliches Loch im Boden, durch das das Wasser
				 * abfliesst." */
				dlg_message(g_dialog, ev->txt[38]);
			}
		}
	}
	if (g_dng16_grid_moved != 0) {
		h = get_first_active_hero(g_group);
		/* BINGE7.LTX[39]: "Wollt Ihr das Gitter wieder vor die Oeffnung schieben?" */
		if (dlg_yes_no(g_dialog, ev->txt[39]) == 1) {
			if (get_second_active_hero(g_group) == 0) {
				if (g_dng16_hero_wet != 0) {
					/* BINGE7.LTX[40]: "Da %s nun schon mal nass ist, schiebt %s auch gleich das Gitter wieder zurueck vor die
					 * Oeffnung." */
					sprintf(g_text_output_buf, ev->txt[40], h->name, grammar_pronoun(h->sex, 0, 0));
					dlg_message(g_dialog, g_text_output_buf);
				} else {
					if (test_attrib(h, ATTRIB_GE, 2) <= 0) {
						char far *s = grammar_pronoun(h->sex, 0, 0);
						/* BINGE7.LTX[41]: "%s strauchelt ein wenig und muss dann feststellen, dass %s neben der Oeffnung im
						 * Wasserbecken sitzt! Mit einer mueden Bewegung schiebt %s das Gitter wieder vor die Oeffnung." */
						sprintf(g_text_output_buf, ev->txt[41], h->name, s, s);
						hero_disease_test(h, ILLNESS_DUMPFSCHAEDEL, 10);
					} else
						/* BINGE7.LTX[42]: "Ohne grosse Anstrengung schiebt %s das Gitter zurueck auf die Oeffnung." */
						sprintf(g_text_output_buf, ev->txt[42], h->name);
					dlg_message(g_dialog, g_text_output_buf);
				}
			} else {
				struct hero far *h2 = get_second_active_hero(g_group);
				if (test_attrib(h2, ATTRIB_GE, 2) <= 0) {
					/* BINGE7.LTX[43]: "%s ueberlaesst diese Aufgabe laechelnd %s. Schnell ist das Gitter wieder ueber die
					 * Oeffnung geschoben, aber %s wird platschnass dabei." */
					sprintf(g_text_output_buf, ev->txt[43], h->name, h2->name, h2->name);
					hero_disease_test(h2, ILLNESS_DUMPFSCHAEDEL, 10);
				} else
					/* BINGE7.LTX[44]: "%s nimmt %s die Arbeit ab. Ohne grosse Anstrengung schiebt %s das Gitter zurueck auf die
					 * Oeffnung." */
					sprintf(g_text_output_buf, ev->txt[44], h2->name, h->name, grammar_pronoun(h2->sex, 0, 0));
				dlg_message(g_dialog, g_text_output_buf);
			}
			g_dng16_grid_moved = g_dng16_hero_wet = 0;
			timewarp_minutes(g_date, 5L);
		} else
			g_god_favor_ingerimm -= 10;
	}
	dlg_set16(g_dialog, old);
}

void dng16_lava_wall(struct area_events far *ev)
{
	signed short old;
	struct hero far *h;
	signed short slot;

	old = dlg_set16(g_dialog, 5);
	if (g_parchments_read != 0)
		/* BINGE7.LTX[45]: "Die Muster und Bilder erinnern Euch an die Zeichnungen auf den Pergamentstuecken, die Ihr in
		 * einer Truhe gefunden habt." */
		dlg_message(g_dialog, ev->txt[45]);
	/* BINGE7.LTX[46]: "Wollt Ihr einen Gegenstand benutzen?" */
	if (dlg_yes_no(g_dialog, ev->txt[46]) == 1) {
		/* BINGE7.LTX[47]: "Wer soll einen Gegenstand benutzen?" */
		h = get_active_hero(g_group, ev->txt[47]);
		if (h != 0) {
			/* BINGE7.LTX[48]: "Welchen Gegenstand soll %s benutzen?" */
			sprintf(g_text_output_buf, ev->txt[48], h->name);
			slot = select_item(h, g_text_output_buf, 1);
			if (slot != -1) {
				if (h->inv[slot].item_id == ITEM_TORCH_ON) {
					/* BINGE7.LTX[49]: "%s haelt die Fackel an die glatte, runde Flaeche. Nach einer Weile beginnt sie zu glimmen,
					 * schliesslich leuchtet es wie gluehende Lava. Ploetzlich ist das Wandstueck verschwunden." */
					sprintf(g_text_output_buf, ev->txt[49], h->name);
					dlg_message(g_dialog, g_text_output_buf);
					event_set_cell(ev, 10, 11, 0, 0, -1);
					automap_cell_write(g_map, 0x312, 11, 10, 3);
					automap_cell_write(g_map, 0x312, 12, 10, 5);
				} else
					/* BINGE7.LTX[50]: "Nichts passiert." */
					dlg_message(g_dialog, ev->txt[50]);
			}
		}
	}
	dlg_set16(g_dialog, old);
}

void dng16_stone_door(struct area_events far *ev)
{
	signed short old;
	struct hero far *h;
	signed short cnt;
	signed short ans;

	old = dlg_set16(g_dialog, 5);
	cnt = 0;
	do {
		/* BINGE7.LTX[51]: "Die Steintuer ist wieder geschlossen, auf dieser Seite steht Ihr einfach vor einer glatten
		 * Wand.@Wollt Ihr" */
		/* BINGE7.LTX[52]: "versuchen, die Tuer zu oeffnen?" */
		/* BINGE7.LTX[53]: "oder ein Foramen sprechen?" */
		ans = dlg_choose(g_dialog, ev->txt[51], 2, ev->txt[52], ev->txt[53]);
		if (ans == 1) {
			/* BINGE7.LTX[54]: "Ihr sucht nach einem Oeffnungsmechanismus, aber Ihr koennt nichts finden." */
			dlg_message(g_dialog, ev->txt[54]);
			cnt++;
		} else if (ans == 2) {
			/* BINGE7.LTX[55]: "Wer soll ein Foramen sprechen?" */
			h = get_active_mage(g_group, ev->txt[55]);
			if (h != 0) {
				/* BINGE7.LTX[56]: "Es passiert nichts." */
				dlg_message(g_dialog, ev->txt[56]);
				cnt++;
			}
		}
	} while (ans != -1 && cnt < 3);
#if FIX_0122
	/* Fix #122: Meldung nach drei vergeblichen Versuchen (Schleife endet bei 3) */
	if (cnt >= 3)
#else
	if (cnt > 3)
#endif
		/* BINGE7.LTX[57]: "Langsam wird Euch klar, dass Ihr hier unten eingeschlossen seid - Ihr muesst wohl den Rest
		 * Eures Lebens bei dem Golem verbringen." */
		dlg_message(g_dialog, ev->txt[57]);
	dlg_set16(g_dialog, old);
}

void dng16_golem(struct area_events far *ev)
{
	signed short old;
	signed short music;
	void far *raw;
	char far * far *txt;
	signed short ans;

	old = dlg_set16(g_dialog, 5);
	music = g_music_id_req & 0x3fff;
	g_dng16_pal_save = mem_alloc_w(0x300);
	if (!g_dng16_sword_done && !g_dng16_forging && !g_dng16_golem_fought) {
		if (g_speech_ext_avail != 0) {
			mouse_hide(0);
			g_pp20_index = -1;
			clear_screen_buf(g_vga_buf);
			get_palette(g_dng16_pal_save, 0, 0x100);
			snd_obj_stop(g_snd_music);
			load_pic_list(g_cd_cache, g_dng16_seq_pics, g_dng16_seq_pics_state, 1, 0);
			if (!fli_play(0x0f, -1, dng16_seq_golem_cb, 1))
				wait_ticks_key(0x96);
			pal_blend(g_fli_pal, 0x19);
			clear_screen_buf(g_vga_buf);
			set_palette(g_dng16_pal_save, 0, 0x100);
			mouse_show(0);
			txt_load_list(g_font, 0x45a, 0x5a, &raw, &txt);
			ev->txt_buf = raw;
			ev->txt = txt;
			/* BINGE7.LTX[0]: "Wollt Ihr" */
			/* BINGE7.LTX[59]: "angreifen?" */
			/* BINGE7.LTX[60]: "fliehen?" */
			/* BINGE7.LTX[61]: "oder abwarten?" */
			ans = dlg_choose(g_dialog, ev->txt[0], 3, ev->txt[59], ev->txt[60], ev->txt[61]);
			mem_free(raw);
			mem_free(txt);
			ev->txt_buf = 0;
			ev->txt = 0;
		} else
			/* BINGE7.LTX[58]: "Ploetzlich beginnt der Boden unter Euch zu vibrieren. Ihr hoert ein lautes, aber behaebiges
			 * Stapfen, und aus dem Durchgang im Osten der Halle tritt ein Wesen aus steinhartem Lehm, dessen plumpe
			 * Koerpergestalt entfernt an einen Menschen erinnert. Der dreieinhalb Schritt grosse Golem kommt auf Euch zu.
			 * In seiner rechten haelt er einen gewaltigen Schmiedehammer mit langem Stiel. Wollt Ihr" */
			ans = dlg_choose(g_dialog, ev->txt[58], 3, ev->txt[59], ev->txt[60], ev->txt[61]);
		if (ans == 1) {
			dng16_golem_fight(ev);
		} else if (ans == 2) {
			group_set_screen_pos(12, 10);
			cam_set_from_group();
		} else {
			talk(&g_talk, 0x13c, 0, 0);
			if (g_dng16_medallion_given != 0) {
				group_drop_item(g_group, ITEM_STONE_MEDALLION);
				if (g_speech_ext_avail != 0) {
					mouse_hide(0);
					fli_play(0x10, -1, dng16_seq_forge_cb, 1);
					mouse_show(0);
				} else {
					/* BINGE7.LTX[62]: "Ihr reicht dem Golem das Steinmedaillon. Eine Zeitlang betrachtet er die Muenze und wendet
					 * sie in der Hand von einer auf die andere Seite. Dann verschwindet er kurz im Nebenraum und kommt mit einem
					 * Pulverbeutelchen wieder." */
					dlg_message(g_dialog, ev->txt[62]);
					/* BINGE7.LTX[63]: "Der Golem schuettet das Pulver aus dem Beutelchen in den Schlund. In der Tiefe rumort es.
					 * Langsam steigt heisse Glut im Schlund auf und verwandelt den Krater zwischen den Pfeilern in einen
					 * brodelnden Lavasee." */
					dlg_message(g_dialog, ev->txt[63]);
					/* BINGE7.LTX[64]: "Es wird immer waermer in der Halle.@Der Golem stapft stumm zu einem Erzhaufen an der
					 * Nordwestwand und schaufelt grosse Erzbrocken in die Lava, anschliessend schaufelt er von einem zweiten
					 * Haufen Holzkohle dazu. Er tritt neben den Schlund, beinahe achtlos wirft er die Muenze in die brodelnde
					 * Glut. Es zischt und brodelt auf dem Lavasee, ueber dem eine intensive, orangefarbene Flamme entsteht, deren
					 * Licht so gleissend ist, dass Ihr wegsehen muesst. Gleichzeitig wird es unertraeglich heiss im Raum." */
					dlg_message(g_dialog, ev->txt[64]);
				}
				if (dng16_forge_heat(ev)) {
					if (g_speech_ext_avail != 0) {
						mouse_hide(0);
						fli_play_file(0x11, 0);
						pal_blend(g_fli_pal, 0x19);
						clear_screen_buf(g_vga_buf);
						set_palette(g_dng16_pal_save, 0, 0x100);
						mouse_show(0);
						g_refresh = 1;
					} else {
						/* BINGE7.LTX[67]: "Schweissschauer laufen Euch ueber die Stirn. Der Golem holt eine grosse Eisenzange und
						 * fischt einen hell gluehenden Klumpen aus der Glut, um ihn auf dem Amboss neben dem Schlund zu bearbeiten." */
						dlg_message(g_dialog, ev->txt[67]);
						/* BINGE7.LTX[68]: "<Schaut Euch ruhig um hier, es wird eine Weile dauern>, raunt Euch der Golem herueber.
						 * <In der Kammer im Osten, koennt Ihr dem Ingerimm ein Geschenk fuer das Werk lassen, dort liegen auch ein
						 * paar excellente Schmiedearbeiten, des Ingerimm eigene Werke.>" */
						dlg_message(g_dialog, ev->txt[68]);
					}
				}
				g_dng16_forging = 1;
				g_dng16_forge_timer = 5;
			} else
				dng16_golem_fight(ev);
		}
	}
	if (g_speech_ext_avail != 0)
		g_refresh = 1;
	mem_free_v(g_dng16_pal_save);
	snd_play(g_snd_music, music);
	dlg_set16(g_dialog, old);
}

void dng16_golem_forge(struct area_events far *ev)
{
	if (!g_dng16_sword_done && !g_dng16_forging && !g_dng16_golem_fought) {
		swap_out_screen();
		dng16_golem(ev);
		return;
	}
	if (g_dng16_forge_timer != 0) {
		/* BINGE7.LTX[69]: "Der Golem ist noch immer mit seinem Schmiedewerk beschaeftigt. Mit langsamen aber kraeftigen
		 * Schlaegen bearbeitet er das abkuehlende Eisenstueck. Noch laesst sich nicht erkennen, was der Golem schmieden
		 * will." */
		dlg_message(g_dialog, ev->txt[69]);
		swap_out_screen();
		if (conv_run(g_dialog2, 0xa5))
			timewarp_minutes(g_date, 10L);
		return;
	}
	if (!g_dng16_sword_done) {
		if (g_dng16_artifact_traded == 1)
			/* BINGE7.LTX[70]: "Der Golem beendet stumm seine Arbeit, ein Schwert aus dunkelrot schimmerndem Metall, er
			 * raeuspert sich kurz, dann legt er es in die Truhe in der oestlichen Kammer." */
			dlg_message(g_dialog, ev->txt[70]);
		else {
			/* BINGE7.LTX[71]: "<Das Werk ist vollbracht>, sagt der Golem ploetzlich, und haelt Euch mit selbstzufriedener
			 * Miene ein Schwert aus dunkelrot schimmerndem Metall entgegen. <Nehmt dies Schwert von Ingerimm entgegen.>" */
			dlg_message(g_dialog, ev->txt[71]);
			group_give_item(g_group, ITEM_ASTHENIL_SWORD, 1);
		}
		/* BINGE7.LTX[72]: "<...und nun muss wieder Ruhe einkehren in Ingerimms Schmiede.>@Er schaut Euch auffordernd
		 * an." */
		dlg_message(g_dialog, ev->txt[72]);
		g_dng16_wall_timer = 3;
		g_dng16_sword_done = 1;
	}
}

void dng16_golem_fight(struct area_events far *ev)
{
	signed short r = 0;

	r = event_fight1(ev, 0x62, 1, 0);
	if (r == 0)
		g_dng16_sacrilege = g_dng16_sword_done = g_dng16_golem_fought = 1;
}

signed short dng16_forge_heat(struct area_events far *ev)
{
	struct hero far *h = get_first_hero(g_group);
	struct hero far *last = 0;
	signed short ret = 1;
	struct score7 sel = g_dng16_heat_marks_init;
	signed short cnt = 0;
	signed short old = dlg_set16(g_dialog, 5);
	signed short k;
	signed short i;
	signed short flag = 0;
	void far *raw;
	char far * far *txt;

	txt_load_list(g_font, 0x45a, 0x5a, &raw, &txt);
	ev->txt_buf = raw;
	ev->txt = txt;
	memset(g_text_buf2, 0, 0x96);
	do {
		if (!(signed char)h->dead) {
			if (get_item_pos(h, ITEM_LEATHER_TROUSERS) < 0 || get_item_pos(h, ITEM_LEATHER_TROUSERS) > 14 ||
				get_item_pos(h, ITEM_LEATHER_BOOTS) < 0 || get_item_pos(h, ITEM_LEATHER_BOOTS) > 14 ||
				get_item_pos(h, ITEM_LEATHER_JERKIN) < 0 || get_item_pos(h, ITEM_LEATHER_JERKIN) > 14)
			{
				cnt++;
				sel.v[group_hero_pos(g_group, h)] = 1;
			}
		}
		h = get_next_hero(g_group, h);
	} while (get_first_hero(g_group) != h);

	h = get_first_hero(g_group);
	k = cnt;
#if FIX_0122
	/* Fix #122: "und" vor dem letzten aufgelisteten Namen: k nur fuer aufgelistete Helden zaehlen */
	do {
		if (sel.v[group_hero_pos(g_group, h)] != 0) {
			k--;
			if (last != 0) {
				if (k == 0) {
					sprintf(g_text_output_buf, " %s ", g_ttx[422]);
					strcat(g_text_buf2, g_text_output_buf);
				} else
					strcat(g_text_buf2, g_ttx[420]);
			}
			strcat(g_text_buf2, h->name);
			last = h;
		}
		h = get_next_hero(g_group, h);
	} while (get_first_hero(g_group) != h);
#else
	do {
		if (sel.v[group_hero_pos(g_group, h)] != 0) {
			if (last != 0) {
				if (k == 0) {
					/* GLOBLTXT.LTX[422]: "und" */
					sprintf(g_text_output_buf, " %s ", g_ttx[422]);
					strcat(g_text_buf2, g_text_output_buf);
				} else
					/* GLOBLTXT.LTX[420]: "," */
					strcat(g_text_buf2, g_ttx[420]);
			}
			strcat(g_text_buf2, h->name);
			last = h;
		}
		h = get_next_hero(g_group, h);
		k--;
	} while (get_first_hero(g_group) != h);
#endif

	if (cnt == 1) {
		/* BINGE7.LTX[65]: "%s kann kaum atmen, %s Kehle wird von der Hitze zugeschnuert. @Verschwommen nimmt %s wahr,
		 * dass der Golem eine grosse Eisenzange holt, damit einen hell gluehenden Klumpen aus der Glut fischt, um ihn
		 * auf dem Amboss neben dem Schlund zu bearbeiten. Dann wird es %s schwarz vor Augen und %s spuert nur noch die
		 * grauenvolle Hitze, die von %s Koerper nicht mehr als ein Haeufchen Asche uebrig laesst." */
		sprintf(g_text_output_buf, ev->txt[65], g_text_buf2, grammar_pronoun(last->sex, 1, 1), grammar_pronoun(last->sex, 0, 0),
			grammar_pronoun(last->sex, 3, 0), grammar_pronoun(last->sex, 0, 0),
			/* BINGE7.LTX[76]: "ihrem" */
			/* BINGE7.LTX[77]: "seinem" */
			last->sex ? ev->txt[76] : ev->txt[77]);
		if (g_speech_ext_avail != 0) {
			pal_blend(g_fli_pal, 0x19);
			clear_screen_buf(g_vga_buf);
			load_palette_arc3();
			flag = 1;
		}
		dlg_message(g_dialog, g_text_output_buf);
	} else if (cnt > 1) {
		if (g_speech_ext_avail != 0 && flag == 0) {
			pal_blend(g_fli_pal, 0x19);
			clear_screen_buf(g_vga_buf);
			load_palette_arc3();
		}
		/* BINGE7.LTX[66]: "%s koennen kaum atmen, ihre Kehlen werden von der Hitze zugeschnuert. @Verschwommen nehmen
		 * sie wahr, dass der Golem eine grosse Eisenzange holt, damit einen hell gluehenden Klumpen aus der Glut fischt,
		 * um ihn auf dem Amboss neben dem Schlund zu bearbeiten. Dann wird es ihnen schwarz vor Augen und sie spueren
		 * nur noch die grauenvolle Hitze, die von ihren Koerpern nicht mehr als ein Haeufchen Asche uebrig laesst." */
		sprintf(g_text_output_buf, ev->txt[66], g_text_buf2);
		dlg_message(g_dialog, g_text_output_buf);
	}
	for (i = 0; i < 7; i++) {
		if (sel.v[i] != 0) {
			h = g_group->heroes[i];
			sub_hero_le(h, h->le);
		}
	}
	if (g_group->nr_heroes == cnt)
		ret = 0;
	dlg_set16(g_dialog, old);
	mem_free(raw);
	mem_free(txt);
	ev->txt_buf = 0;
	ev->txt = 0;
	return ret;
}
