/* OVL214 - Overlay O214 (Segment 0x23ad in SCHWEIF.EXE): Eingabe der Reisekarte (Reiseobjekt, C-Sicht struct travel):
 * Richtungswahl mit der Kompassrose (O220) und Menue. Taste in g_key (BIOS-Code), Richtung in dir_sel (1..8).
 * travelmap_select_dir(p): Reiseanimation an (travel_anim_on), Karte bzw. Animation neu, Mausbereiche der Rose
 * (g_rose_areas: ganze Rose = Aktion 9, g_rose_dir_areas: Richtungen) per rose_mouse_area, Richtungswechsel nur wenn
 * rose_dir_allowed zustimmt (sonst alte Auswahl zurueck); Strg-Q beendet (sys_exit, exit), 'c' verschiebt die Rose
 * mit den Tasten (rose_move), Esc fragt (g_ttxt[81]) und setzt route[1..39] = -1, Bild auf baut die 8 Richtungen
 * g_ttxt[52..59] in g_text_output_buf + 0x14 * n (nicht freie Richtungen per Bit place.dirs.dn mit Formatstring
 * code_to_text(0xe7)) und waehlt per dlg_choose; Pfeile laufen zyklisch durch die erlaubten Richtungen 1..8.
 * Rueckgabe: neue Richtung g_action (1..8), sonst uninitialisiert. travel_redraw(p): Bildschirmaufbau/Musik
 * (Sound_mtype, snd_play), travelmap_draw bzw. travel_anim/travel_status_update (Tag/Nacht shown[3] gegen g_is_day).
 * travel_menu(p): Funktionstasten -> Aktionscode g_action (0xe1..0xfb), Untermenues per dlg_choose (Informationen
 * travel_text_speed/weather/terrain/road O216, Schleife bis Esc/5), dann Aktion ausfuehren; 1 bei travel_rest_menu/
 * select_hero_bar (O217).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL214.C   -> 3091/3091 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h, C-Sicht struct travel: 19.10.2026 GLEICH, LINKGLEICH)
 * - Reines C. -Og noetig: Zwischenwert [bp-9] = p->dir_sel (signed char, dreimal gelesen), tote Epiloge nach return
 *   in travelmap_select_dir und travel_menu. -Oi noetig: strcpy(g_text_output_buf, g_ttxt[52]) als Intrinsic (Ziel
 *   far-Zeiger-Variable: Ziel ueber push es/push di/mov ax,di.. sichern) bzw. strcpy(g_text_output_buf + 0x14, ..).
 * - travelmap_select_dir liefert signed short (cbw am Ende), obwohl r ein signed char ist.
 * - Bitfeld-Tests "!p->place.dirs.dN" (struct dir_bits, unsigned short b:1, 1 Byte) -> mov al / shr ax,N / and ax,1 /
 *   or ax,ax / jnz.
 * - switch auf unsigned short g_key (ja/jb in der Binaersuche); in travel_menu bleiben Strg-E/F1 und 'h' (gleicher
 *   Rumpf 0xf6) getrennte Faelle. Innerer switch mit 4 Faellen (1..4) -> Sprungtabelle, mit 3 Faellen Vergleichskette.
 * - dlg_choose variadisch; zweiter Parameter (Titel) als (char far *)0 -> push 0 / push 0.
 * Originalfehler: travelmap_select_dir gibt r uninitialisiert zurueck, wenn g_action nicht in 1..8 liegt und kein
 *   Richtungswechsel stattfand (r wird nur im Wechsel-Zweig bzw. im Block 1..8 gesetzt).
 */
/* Phase B: BCC (C-Modus) legt das BSS in umgekehrter Reihenfolge der ersten Deklaration an - Reihenfolge hier vor schweif.h */
extern signed short g_rose_last_my;
extern signed short g_rose_last_mx;
extern signed short g_menu_sel;
extern unsigned short g_view_frames;
extern char g_unused_cb25[1];
extern signed char g_key_pad5;
extern signed short g_key_last;
extern signed short g_key_alt;
extern signed short g_key_ctrl;
extern char g_unused_cb1d[1];
extern signed char g_tmap_loaded;
extern signed short g_nr_items;
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:7140..7202, dsana) - initialisierte Daten in Adressreihenfolge */
struct mouse_area g_rose_dir_areas[9] = {
	{ 4, 4, 24, 24, 8 },
	{ 26, 4, 46, 24, 2 },
	{ 4, 26, 24, 46, 6 },
	{ 26, 26, 46, 46, 4 },
	{ 20, 0, 30, 20, 1 },
	{ 20, 30, 30, 50, 5 },
	{ 0, 20, 20, 30, 7 },
	{ 30, 20, 50, 30, 3 },
	{ -1, -1, -1, -1, -1 }
};
struct mouse_area g_rose_areas[2] = { { 0, 0, 50, 50, 9 }, { -1, -1, -1, -1, -1 } };
struct route_list42 g_water_routes = {
	{ 14, 15, 16, 17, 19, 20, 21, 22, 24, 56, 57, 58, 59, 60, 61, 62, 63, 65, 66, 94, 95, 96, 153, 154, 174, 175, 176, 177, 178, 179, 186, 218, 219, 220, 221, 222, 243, 244, 245, 246, 248, 254 }
};
/* Phase B: eigenes BSS (ds:cb1a..cb2e) in Adressreihenfolge (Reihenfolge festgelegt durch die extern-Zeilen vor den #include) */
signed short g_nr_items;
signed char g_tmap_loaded;
char g_unused_cb1d[1];   /* ds:cb1d, unbenutzt */
signed short g_key_ctrl;
signed short g_key_alt;
signed short g_key_last;
signed char g_key_pad5;
char g_unused_cb25[1];   /* ds:cb25, unbenutzt */
unsigned short g_view_frames;
signed short g_menu_sel;
signed short g_rose_last_mx;
signed short g_rose_last_my;

#define KEY_C		0x2e63		/* 'c': Kompassrose verschieben */
#define KEY_H		0x2368		/* Buchstabentasten (BIOS-Code) */
#define KEY_S		0x1f73
#define KEY_W		0x1177
#define KEY_L		0x266c
#define KEY_P		0x1970
#define ACT_ROSE	9		/* Mausbereich ganze Rose */
#define ACT_REST	0xf5		/* Aktionen der Reisekarte: Rastmenue (travel_rest_menu) */
#define ACT_HEROES	0xf6		/* Heldenauswahl (select_hero_bar) */
#define ACT_INFO_ROAD	0xf8		/* Informationen: Wegzustand */
#define ACT_INFO_TERRAIN 0xf9		/* Gelaende */
#define ACT_INFO_WEATHER 0xfa		/* Wetter */
#define ACT_INFO_SPEED	0xfb		/* Tempo */

signed short travelmap_select_dir(struct travel far *p)
{
	signed short w;
	signed short sel;
	signed char r;
	signed char i;
	signed short k = 0;

#if FIX_0036
	/* Fix #36: r vorbelegen */
	r = 0;
	travel_anim_on(p, 0);
	if (g_refresh == 1) {
		g_refresh = 0;
		travelmap_draw(p);
		rose_draw(p);
#else
	travel_anim_on(p, 0);
	if (g_refresh == 1) {
		g_refresh = 0;
		travelmap_draw(p);
		rose_draw(p);
#endif
	} else if (p->anim_on != 0)
		travel_anim(p);
	w = g_mouse_event;
	poll_input();
	if (g_mouse_rclick != 0) {
		if (rose_mouse_area(p, g_rose_areas) == ACT_ROSE)
			g_action = ACT_ROSE;
		else
			g_key = KEY_PGUP;
		g_mouse_rclick = 0;
	}
	if (w != 0 && g_action == 0)
		g_action = p->dir_sel;
	if (g_rose_last_mx != g_mouse_posx || g_rose_last_my != g_mouse_posy) {
		k = rose_mouse_area(p, g_rose_dir_areas);
		g_rose_last_mx = g_mouse_posx;
		g_rose_last_my = g_mouse_posy;
	}
	if (k != 0 && p->dir_sel != k && k < 9 && g_action != ACT_ROSE) {
		r = p->dir_sel;
		p->dir_sel = k;
		if (rose_dir_allowed(p))
			rose_draw(p);
		else
			p->dir_sel = r;
	}

	switch (g_key) {
	case KEY_CTRL_Q:
		g_key = KEY_ESC;
		sys_exit(0);
		exit(0);
		break;
	case KEY_C:
		rose_move(p, 1);
		break;
	case KEY_ESC:
		if (p->route[1] == 0)
			sel = 1;
		else
			sel = dlg_yes_no(g_dialog, g_ttxt[81]);
		if (sel == 1) {
			for (i = 1; i < 40; i++)
				p->route[i] = -1;
			g_travel_state = 9;
		} else
			g_travel_state = 10;
		g_key = KEY_ESC;
		break;
	case KEY_PGUP:
		strcpy(g_text_output_buf, g_ttxt[52]);
		if (!p->place.dirs.d0)
			sprintf(g_text_output_buf, code_to_text(0xe7), g_ttxt[52]);
		strcpy(g_text_output_buf + 0x14, g_ttxt[56]);
		if (!p->place.dirs.d1)
			sprintf(g_text_output_buf + 0x14, code_to_text(0xe7), g_ttxt[56]);
		strcpy(g_text_output_buf + 0x28, g_ttxt[53]);
		if (!p->place.dirs.d2)
			sprintf(g_text_output_buf + 0x28, code_to_text(0xe7), g_ttxt[53]);
		strcpy(g_text_output_buf + 0x3c, g_ttxt[58]);
		if (!p->place.dirs.d3)
			sprintf(g_text_output_buf + 0x3c, code_to_text(0xe7), g_ttxt[58]);
		strcpy(g_text_output_buf + 0x50, g_ttxt[55]);
		if (!p->place.dirs.d4)
			sprintf(g_text_output_buf + 0x50, code_to_text(0xe7), g_ttxt[55]);
		strcpy(g_text_output_buf + 0x64, g_ttxt[59]);
		if (!p->place.dirs.d5)
			sprintf(g_text_output_buf + 0x64, code_to_text(0xe7), g_ttxt[59]);
		strcpy(g_text_output_buf + 0x78, g_ttxt[54]);
		if (!p->place.dirs.d6)
			sprintf(g_text_output_buf + 0x78, code_to_text(0xe7), g_ttxt[54]);
		strcpy(g_text_output_buf + 0x8c, g_ttxt[57]);
		if (!p->place.dirs.d7)
			sprintf(g_text_output_buf + 0x8c, code_to_text(0xe7), g_ttxt[57]);
		sel = dlg_choose(g_dialog, g_ttxt[82], 8, g_text_output_buf, g_text_output_buf + 0x14, g_text_output_buf + 0x28, g_text_output_buf + 0x3c,
			g_text_output_buf + 0x50, g_text_output_buf + 0x64, g_text_output_buf + 0x78, g_text_output_buf + 0x8c);
		if (sel != -1) {
			p->dir_sel = sel;
			g_key = KEY_RETURN;
		} else
			rose_draw(p);
		break;
	default:
		if (g_key == KEY_RIGHT || g_key == KEY_DOWN) {
			do {
				p->dir_sel++;
				if (p->dir_sel > 8)
					p->dir_sel = 1;
			} while (!rose_dir_allowed(p));
			rose_draw(p);
		} else if (g_key == KEY_LEFT || g_key == KEY_UP) {
			do {
				p->dir_sel--;
				if (p->dir_sel < 1)
					p->dir_sel = 8;
			} while (!rose_dir_allowed(p));
			rose_draw(p);
		}
		break;
	}

	if (g_action == ACT_ROSE) {
		rose_move(p, 0);
		g_action = 0;
	}
	if (g_action >= 1 && g_action <= 8) {
		if (p->f10a != 0) {
			if (rose_dir_allowed(p)) {
				p->dir_sel = g_action;
				rose_draw(p);
				wait_ticks(15);
				g_action = p->dir_sel;
			} else
				p->f10a = 0;
		}
		r = g_action;
	}
	return r;
}

void travel_redraw(struct travel far *p)
{
	if (g_travel_state != 1) {
		if (g_options_shown != 0) {
			if (Sound_mtype(g_snd_music) == 1 || Sound_mtype(g_snd_music) == 2)
				snd_play(g_snd_music, 0x23);
			else
				snd_play(g_snd_music, 0x16);
		}
		if (g_refresh == 1) {
			g_refresh = 0;
			travelmap_draw(p);
		} else {
			if (p->anim_on != 0 && p->mode != 2)
				travel_anim(p);
			if (p->shown[3] != g_is_day)
				travel_status_update(p);
		}
	} else
		g_refresh = 0;
	poll_input();
	handle_options_key();
}

signed short travel_menu(struct travel far *p)
{
	signed char sel;
	register signed short r = 0;

	if (g_mouse_rclick != 0)
		g_key = KEY_PGUP;
	switch (g_key) {
	case KEY_CTRL_Q:
		sys_exit(0);
		exit(0);
		break;
	case KEY_CTRL_J:
	case KEY_F5:
		g_action = ACTION_DIARY;
		break;
	case KEY_F8:
		g_action = ACTION_MENU_SCREEN;
		break;
	case KEY_CTRL_E:
	case KEY_F1:
		g_action = ACT_HEROES;
		break;
	case KEY_H:
		g_action = ACT_HEROES;
		break;
	case KEY_F4:
		g_action = ACTION_DATE;
		break;
	case KEY_S:
		g_action = ACT_INFO_SPEED;
		break;
	case KEY_W:
		g_action = ACT_INFO_WEATHER;
		break;
	case KEY_L:
		g_action = ACT_INFO_TERRAIN;
		break;
	case KEY_P:
		g_action = ACT_INFO_ROAD;
		break;
	case KEY_ESC:
	case KEY_PGUP:
		sel = dlg_choose(g_dialog, (char far *)0, 3, g_ttxt[49], g_ttxt[48], g_ttxt[60]);
		switch (sel) {
		case 1:
			g_action = ACT_REST;
			break;
		case 2:
			g_action = ACT_HEROES;
			break;
		case 3:
			do {
				sel = dlg_choose(g_dialog, g_ttxt[74], 5, g_ttxt[61], g_ttxt[62], g_ttxt[63],
					g_ttxt[64], g_ttxt[65]);
				switch (sel) {
				case 1:
					travel_text_weather(p, g_text_output_buf);
					dlg_message(g_dialog, g_text_output_buf);
					break;
				case 2:
					travel_text_terrain(p, g_text_output_buf);
					dlg_message(g_dialog, g_text_output_buf);
					break;
				case 3:
					travel_text_road(p, g_text_output_buf);
					dlg_message(g_dialog, g_text_output_buf);
					break;
				case 4:
					travel_text_speed(p, g_text_output_buf);
					dlg_message(g_dialog, g_text_output_buf);
					break;
				}
			} while (sel != -1 && sel != 5);
			break;
		}
		break;
	}
	game_menu();
	if (g_action == ACTION_DATE) {
		Date_print(g_date);
		dlg_message(g_dialog, g_text_output_buf);
	}
	if (g_action == ACT_INFO_SPEED) {
		travel_text_speed(p, g_text_output_buf);
		dlg_message(g_dialog, g_text_output_buf);
	}
	if (g_action == ACT_INFO_WEATHER) {
		travel_text_weather(p, g_text_output_buf);
		dlg_message(g_dialog, g_text_output_buf);
	}
	if (g_action == ACT_INFO_TERRAIN) {
		travel_text_terrain(p, g_text_output_buf);
		dlg_message(g_dialog, g_text_output_buf);
	}
	if (g_action == ACT_INFO_ROAD) {
		travel_text_road(p, g_text_output_buf);
		dlg_message(g_dialog, g_text_output_buf);
	}
	if (g_action == ACT_REST) {
		travel_rest_menu(p);
		r = 1;
	}
	if (g_action == ACT_HEROES) {
		select_hero_bar(p);
		r = 1;
	}
	return r;
}
