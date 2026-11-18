/* OVL095 - Overlay O095 (Segment 0x212b in SCHWEIF.EXE): Heldenbogen/Inventaransicht (struct inv_sheet s: Texte
 * txt, Seite page (1..5, Menues in O093), Ende done, Neuzeichnen redraw, Mauszeiger cursor, Held hero, Besitzer
 * owner, Auswahl slot/slot2, alte Auswahl old_slot/old_slot2, Hintergrund status_bg). hero_sheet_loop(s, h)
 * Hauptschleife (Aufruf aus SEG002): Tasten links/rechts (Seite inv_next_hero bzw. inv_switch_hero, mit g_mouse_ldclick
 * select_portrait), hoch/runter (inv_sel_prev/inv_sel_next), Maus 0x80..0xa0 (Inventarplatz), Entf (drop_item),
 * Esc, 0x7c/0x7d Texte ueber hero_is_diseased/hero_is_poisoned, 0xa1 umbenennen, 0xa2 fehlende AP bis zur naechsten
 * Stufe (ap_for_level(Stufe + 1) - AP). inv_poll_input: Eingabe pollen (poll_input/handle_options_key), Seitenwechsel
 * bei geaenderter g_game_mode. rename_hero: Umbenennen (dlg_message_flag, 15 Zeichen, Wortanfaenge gross, Leerzeichen am
 * Ende weg, strcpy nach hero.name). inv_draw_skill: Talentname und -wert (get_skill_value) rechtsbuendig zeichnen.
 * inv_draw_skills: 7 Ueberschriften g_ttx[127..133] und die Gruppen aus g_skills_index mit den Spalten aus
 * g_skill_cols. inv_redraw: Neuzeichnen, Markierungen (inv_draw_slot, 0xfd/0xf9), Name des gewaehlten Gegenstands
 * (grammar_name, Zusatz g_ttx[75 + subtype]).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL095.C   -> 4362/4362 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (ebenso mit zusaetzlich -Oc); ohne -Og 75 Bytes zu lang.
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * - Erster Entwurf sofort GLEICH. -Oi: strcpy/strcat als Intrinsic (daher SI/DI gesichert); -Og: Zwischenwert
 *   [bp-4]/[bp-8] fuer s->slot * 27 (nach Aufrufen neu gerechnet), Epilogkopie beim fruehen return in
 *   hero_sheet_loop.
 * - Ketten: "g_sheet_hunger_shown = g_sheet_thirst_shown = 0xff", "g_refresh = s->redraw = 0", "g_key = g_action = 0" (rechts zuerst
 *   gespeichert), "s->owner = s->hero = g_sheet_hero = h" (mov eax). get_mouse_action(g_inv_show_body ? g_inv_mouse_all :
 *   g_inv_mouse_knapsack) = far-Zeiger auf DS-Tabellen - als char far * uebergeben (Funktionszeiger-Cast
 *   MOUSE_ACTION_FN, Zuweisung an g_mouse_areas ueber char far * far *): mit void far * entsteht "push ds" vor
 *   dem Test.
 * - "font_print(t, s, w - font_text_width0(t, s), y)": das zuerst gepushte y gehoert zum aeusseren Aufruf
 *   (add sp,8).
 * - switch (s->page) mit "case 4: case 5:" -> Sprungtabelle 1..5 hinter der Funktion.
 * Auffaelliges: in hero_sheet_loop (Bereich "old_slot != slot && owner == hero") stammen Gegenstands-Id und
 *   can_use_item-Test von s->hero, die Bitpruefung von s->owner (in inv_redraw durchgehend s->owner) - wegen
 *   owner == hero folgenlos.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:5d0c..5d46, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
struct col_pos7 g_skill_cols = {
	{ { 5, 104, 50 },
	{ 5, 104, 125 },
	{ 112, 209, 125 },
	{ 217, 315, 125 },
	{ 217, 315, 50 },
	{ 112, 209, 50 },
	{ 217, 315, 180 } }
};

/* Dieses Modul uebergibt/speichert die Mausbereichstabellen als char far * (ein ?: aus char-Feldern, das in einen
 * void far * umgewandelt wird, ergibt anderen Code: push ds vor dem Test) */
typedef signed short (far *MOUSE_ACTION_FN)(char far *);

void inv_poll_input(struct inv_sheet far *s)
{
	signed char old = g_game_mode;

	poll_input();
	handle_options_key();

	if (g_options_shown == 1) {
		if (old != g_game_mode) {
			s->page = 1;
			g_refresh = 1;
		}
		if (s->cursor != -1) {
			inv_set_cursor(s, s->cursor);
		}
	}
}

signed short rename_hero(struct inv_sheet far *s)
{
	signed short ret = 0;
	signed short i;

	if (s->hero->npc_id != 0) {
		dlg_message(g_dialog, s->txt[41]);
	} else if (dlg_message_flag(g_dialog, 15, s->txt[42]) != -1) {
		str_tolower(g_input_buf);
		g_input_buf[0] = char_toupper(g_input_buf[0]);
		for (i = 1; g_input_buf[i] != 0; i++) {
			if (g_input_buf[i] == ' ' && g_input_buf[i + 1] != 0) {
				g_input_buf[i + 1] = toupper(g_input_buf[i + 1]);
			}
		}
		i--;
		while (g_input_buf[i] == ' ') {
			g_input_buf[i] = 0;
			i--;
		}
		strcpy(s->hero->name, g_input_buf);
		ret = 1;
	}
	return ret;
}

void inv_draw_skill(struct inv_sheet far *s, register signed short j, signed short base, signed short x, signed short x_val, register signed short y)
{
	signed short row;
	char buf[10];

	row = j - base;
	font_print(g_font, g_ttx[j + 75], x, y + row * 7);
	itoa(get_skill_value(s->hero, j), buf, 10);
	font_print(g_font, buf, x_val - font_text_width0(g_font, buf), y + row * 7);
}

void inv_draw_skills(struct inv_sheet far *s)
{
	signed short i;
	signed short j;
	struct col_pos7 cols = g_skill_cols;

	font_set_colors(g_font, 0xf9, -1);
	font_print(g_font, g_ttx[127], font_center_x(g_font, g_ttx[127], 5, 100), 40);
	font_print(g_font, g_ttx[131], font_center_x(g_font, g_ttx[131], 110, 100), 40);
	font_print(g_font, g_ttx[128], font_center_x(g_font, g_ttx[128], 5, 100), 115);
	font_print(g_font, g_ttx[132], font_center_x(g_font, g_ttx[132], 215, 100), 115);
	font_print(g_font, g_ttx[130], font_center_x(g_font, g_ttx[130], 215, 100), 40);
	font_print(g_font, g_ttx[129], font_center_x(g_font, g_ttx[129], 110, 100), 115);
	font_print(g_font, g_ttx[133], font_center_x(g_font, g_ttx[133], 215, 100), 170);
	font_set_colors(g_font, 0xff, -1);

	for (i = 0; i < 7; i++) {
		for (j = g_skills_index[i].first; g_skills_index[i].first + g_skills_index[i].count > j; j++) {
			inv_draw_skill(s, j, g_skills_index[i].first, cols.c[i].x, cols.c[i].x_val, cols.c[i].y);
		}
	}
}

void inv_redraw(struct inv_sheet far *s)
{
	signed short key;

	mouse_hide(0);
	inv_draw(s);
	g_sheet_hunger_shown = g_sheet_thirst_shown = 0xff;
	bars_refresh(g_bars);

	if (s->page < 3) {
		if (g_mouse_locked == 0) {
			if (s->slot != -1 && s->owner == s->hero && s->slot2 == -1) {
				inv_draw_slot(s, g_vga_buf, s->slot, 0);
			}
			key = ((MOUSE_ACTION_FN)get_mouse_action)(g_inv_show_body ? g_inv_mouse_all : g_inv_mouse_knapsack);
			if (key >= 0x80 && key <= 0xa0) {
				if (s->slot2 != -1) {
					s->slot2 = key - 0x80;
				} else if (key < 0x9f) {
					s->slot = key - 0x80;
				}
			}
		}
		if (s->slot != -1 && s->owner == s->hero) {
			inv_draw_slot(s, g_vga_buf, s->slot, 0xfd);
		}
		if (s->slot2 != -1) {
			inv_draw_slot(s, g_vga_buf, s->slot2, 0xf9);
		}
		if (s->slot != -1) {
			if (s->owner->inv[s->slot].item_id != 0) {
				sprintf(g_text_output_buf, "%s %s ", grammar_name(g_item_names[s->owner->inv[s->slot].item_id]),
					can_use_item(s->hero, s->owner->inv[s->slot].item_id) ? "" : s->txt[0]);
				if (g_items[s->owner->inv[s->slot].item_id].weapon && !g_items[s->owner->inv[s->slot].item_id].stackable) {
					strcat(g_text_output_buf, g_ttx[g_items[s->owner->inv[s->slot].item_id].subtype + 75]);
				}
				font_set_colors(g_font, 0xff, -1);
				font_print(g_font, g_text_output_buf, 0, 0xc0);
			}
		}
	}
	mouse_show(0);
	g_refresh = s->redraw = 0;
}

void hero_sheet_loop(struct inv_sheet far *s, struct hero far *h)
{
	signed short key;
	signed short old_x = -1;		/* letzte Mausposition */
	signed short old_y = -1;

	if (h == 0 || !h->type || !group_has_hero(g_group, h) || g_sheet_enabled == 0) {
		return;
	}

	s->owner = s->hero = g_sheet_hero = h;
	g_dialog_depth++;
	g_action = 0;
	s->page = 1;
	inv_reselect(s);

	while (s->done == 0) {
		if (g_refresh != 0 || s->redraw != 0) {
			inv_redraw(s);
			snd_play(g_snd_music, 5);
		}
		g_mouse_areas2 = 0;
		*(char far * far *)&g_mouse_areas = s->page < 3 ? (g_inv_show_body ? g_inv_mouse_all : g_inv_mouse_knapsack) : g_areas_sheet;
		inv_poll_input(s);

		if (g_action == 0x4d00) {
			if (g_mouse_ldclick == 0) {
				inv_next_hero(s);
			} else {
				g_refresh = select_portrait(s->hero, s->txt[89]);
			}
		} else if (g_action == 0x4b00) {
			if (g_mouse_ldclick == 0) {
				inv_switch_hero(s);
			} else {
				g_refresh = select_portrait(s->hero, s->txt[89]);
			}
		}

		if (s->page < 3) {
			if (g_action == 0x4800) {
				if (s->slot2 == -1 && s->old_slot == -1) {
					inv_draw_slot(s, g_vga_buf, s->slot, 0);
				}
				inv_sel_prev(s);
			} else if (g_action == 0x5000) {
				if (s->slot2 == -1 && s->old_slot == -1) {
					inv_draw_slot(s, g_vga_buf, s->slot, 0);
				}
				inv_sel_next(s);
			} else if (g_action == 0x7a || (g_action == 0x7b && !g_inv_show_body)) {
				inv_toggle_body(s);
				g_refresh = 1;
			}

			if (g_action >= 0x80 && g_action <= 0xa0) {
				if (s->slot2 != -1) {
					s->slot2 = g_action - 0x80;
					g_action = 0x1c0d;
				} else if (g_action < 0x9f) {
					s->slot = g_action - 0x80;
					g_action = 0x1c0d;
				}
			}

			if (g_action == 0x1c0d) {
				inv_confirm(s);
				g_mouse_ldclick = 0;
			}

			if (g_action == 0x5300 && s->slot != -1 && s->slot2 == -1) {
				if (s->owner != s->hero) {
					dlg_message(g_dialog, s->txt[43]);
				} else {
					drop_item(s->hero, s->slot, -1);
					inv_reselect(s);
				}
			}

			if ((g_action == 0x11b || g_action == 0x7e) && s->slot2 != -1 && g_mouse_ldclick == 0) {
				s->swap = 0;
				s->owner = s->hero;
				inv_reselect(s);
				g_action = 0;
			}

			if (g_mouse_locked == 0) {
				if (old_x != g_mouse_posx || old_y != g_mouse_posy) {
					old_x = g_mouse_posx;
					old_y = g_mouse_posy;
					key = ((MOUSE_ACTION_FN)get_mouse_action)(s->page < 3 ? (g_inv_show_body ? g_inv_mouse_all : g_inv_mouse_knapsack) : g_areas_sheet);
					if (key >= 0x80 && key <= 0xa0) {
						if (s->slot2 != -1) {
							s->slot2 = key - 0x80;
						} else if (key < 0x9f) {
							s->slot = key - 0x80;
						}
					}
				}
			}

			if (s->old_slot != s->slot && s->owner == s->hero) {
				mouse_hide(0);
				inv_draw_slot(s, g_vga_buf, s->old_slot, 0);
				inv_draw_slot(s, g_vga_buf, s->slot, 0xfd);
				s->old_slot = s->slot;
				gb_blit(s->status_bg, g_vga_buf, 0, 0xc0);
				if (s->hero->inv[s->slot].item_id != 0) {
					sprintf(g_text_output_buf, "%s %s ", grammar_name(g_item_names[s->hero->inv[s->slot].item_id]),
						can_use_item(s->hero, s->hero->inv[s->slot].item_id) ? "" : s->txt[0]);
					if (g_items[s->owner->inv[s->slot].item_id].weapon && !g_items[s->owner->inv[s->slot].item_id].stackable) {
						strcat(g_text_output_buf, g_ttx[g_items[s->owner->inv[s->slot].item_id].subtype + 75]);
					}
					font_set_colors(g_font, 0xff, -1);
					font_print(g_font, g_text_output_buf, 0, 0xc0);
				}
				mouse_show(0);
			}

			if (s->old_slot2 != s->slot2 && s->slot2 != -1) {
				inv_draw_slot(s, g_vga_buf, s->old_slot2, 0);
				if (s->owner == s->hero) {
					inv_draw_slot(s, g_vga_buf, s->slot, 0xfd);
				}
				inv_draw_slot(s, g_vga_buf, s->slot2, 0xf9);
				s->old_slot2 = s->slot2;
			}
		}

		if (g_action == 0x11b) {
			s->done = 1;
			g_key = g_action = 0;
		}

		if (g_action == 0x7c && hero_is_diseased(s->hero)) {
			sprintf(g_text_output_buf, s->txt[hero_is_diseased(s->hero) + 10], s->hero->name);
			dlg_message(g_dialog, g_text_output_buf);
		} else if (g_action == 0x7d && hero_is_poisoned(s->hero)) {
			sprintf(g_text_output_buf, s->txt[hero_is_poisoned(s->hero) + 47], s->hero->name);
			dlg_message(g_dialog, g_text_output_buf);
		} else if (g_action == 0xa1) {
			rename_hero(s);
			g_refresh = 1;
		} else if (g_action == 0xa2) {
			sprintf(g_text_output_buf, s->txt[92], s->hero->name, ap_for_level(s->hero->level + 1) - s->hero->ap);
			dlg_message(g_dialog, g_text_output_buf);
		}

		if (g_mouse_rclick != 0 || g_key == 0x4900) {
			register signed short cursor = s->cursor;
			inv_reset_cursor(s);
			s->cursor = cursor;
			switch (s->page) {
			case 1:
				inv_menu_page1(s);
				break;
			case 2:
				inv_menu_page2(s);
				break;
			case 3:
				inv_menu_page3(s);
				break;
			case 4:
			case 5:
				inv_menu_page45(s);
				break;
			}
			if (s->cursor != -1) {
				inv_set_cursor(s, s->cursor);
			}
		}
	}

	g_refresh = 1;
	snd_obj_stop(g_snd_music);
	g_dialog_depth--;
}
