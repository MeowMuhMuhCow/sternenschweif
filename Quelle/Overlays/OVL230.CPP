/* OVL230 - Overlay O230 (Segment 0x23fe in SCHWEIF.EXE): Schmiede, Klasse Smith (0x56 Bytes, s_class.h; Zeiger
 * g_smith; BE seg058 do_smith) - Konstruktor: Texte 0x1d (0x32 Stueck), Auswahlliste 0xd9 Bytes (struct repair_item)
 * und Symbolpuffer 0x240, 4-Byte-Satz g_loc_id aus der Datei 0x202 (g_file_3) nach price_mod/f1c/female, repair =
 * &g_smith_repairs[g_loc_id] (9-Byte-Satz: Gegenstand, Abgabezeit), Raster cell_x/cell_y. Destruktor gibt alles frei,
 * g_smith = 0. check_open (O230_01C7): Zugang nur zwischen 6 und 20 Uhr und ohne Hausverbot g_smith_banned[g_loc_id];
 * in Schmiede 1 zusaetzlich Verweigerung, wenn ein Elf (Typ 10..12) ohne Verwandlung (harmless/adler_wolf) in der
 * Gruppe ist (Portraet 0x97 in g_portrait_buf, Text 0x23). run(): Menueschleife (ACTION_ICON_1 talk, _2 Gegenstand abholen
 * pickup bzw. abgeben smith_select_hero/smith_repair (O231) + 30 Min., _3/ESC Ende), check_closing(Zeit) schliesst
 * ausserhalb der Oeffnungszeit. ban: Hausverbot setzen (ueber smith_talk_event, Gespraechsereignis 1/1 mit
 * conv_finish). talk: Gespraech conv_run(g_dialog2, n) je Schmiede (if-Kette), + 30 Min. pickup: Gegenstand nach
 * Ablauf (date_since > 6 Stunden) per group_give_item an die Gruppe, sonst Wartetext. grid_input: Auswahlraster
 * 3 x 5 mit Maus/Cursor und Seitenwechsel (gleiche Logik wie O229 f_0206, als Methode).
 *
 * BCC -mlarge -3 -Ot -c -Y OVL230.CPP   -> 2798/2798 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - C++-Klasse wie O070 (Konstruktor mit new bei this==0, Destruktor mit delete bei flags&1, je toter Epilog).
 * - ban: Kette "g_smith_banned[g_loc_id] = done = 1" (mov ax,1 / Wort / Byte aus al).
 * - run(): "female == 0 ? 0x2b : 0x18a" (cmp byte / jnz); "female ? 0x18a : 0x2b" dreht die Sprungbedingung.
 * - grid_input (keine Aufrufe): ret/i beide register -> ret DI, i CX. Nur "register ret" -> ret CX, i DI (!).
 * - pickup: "if (..) { ..; return; }" vor dem Wartetext -> Epilogkopie mitten in der Funktion (ohne -Og).
 * - Texte nach female: txt[0x26]/[0x27] bzw. [0x28]/[0x29].
 */
#include <stdio.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:72de..731e, dsana) - initialisierte Daten in Adressreihenfolge */
signed short g_merchant_items_posx[15] = { 30, 30, 30, 30, 30, 95, 95, 95, 95, 95, 160, 160, 160, 160, 160 };
signed short g_merchant_items_posy[15] = { 35, 55, 75, 95, 115, 35, 55, 75, 95, 115, 35, 55, 75, 95, 115 };
void far *g_smith = 0;

#define SMITH_TXT	0x1d		/* Texte der Schmiede (0x32 Stueck) */
#define SMITH_TXT_N	0x32
#define SMITH_DATA	0x202		/* Datei mit 4-Byte-Saetzen je Schmiede */
#define SMITH_OPEN	(6 * TICKS_PER_HOUR)	/* Oeffnungszeit 6..20 Uhr */
#define SMITH_CLOSE	(20 * TICKS_PER_HOUR)
#define SMITH_REPAIR_TIME (6 * TICKS_PER_HOUR)	/* Reparaturdauer */
#define SMITH		((Smith far *)g_smith)
#define DATE		((struct game_date far *)g_date)

Smith::Smith()
{
	g_smith = this;
	txt_load_list(g_font, SMITH_TXT, SMITH_TXT_N, &txt_buf, &txt);
	items = (struct repair_item far *)mem_alloc(0xd9);
	icon_buf = mem_alloc(0x240);
	arc_open(g_file_3, SMITH_DATA);
	arc_seek(g_file_3, (signed long)g_loc_id * 4, 0);
	arc_read(g_file_3, &price_mod, 4);
	arc_close(g_file_3);
	repair = &g_smith_repairs[g_loc_id];
	cell_x[0] = 0x1e;
	cell_x[1] = 0x5f;
	cell_x[2] = 0xa0;
	cell_y[0] = 0x23;
	cell_y[1] = 0x37;
	cell_y[2] = 0x4b;
	cell_y[3] = 0x5f;
	cell_y[4] = 0x73;
	count = 0;
	g_refresh = 1;
	done = 0;
}

Smith::~Smith()
{
	mem_free(icon_buf);
	mem_free(items);
	mem_free(txt_buf);
	mem_free(txt);
	g_smith = 0;
}

signed short Smith::check_open()
{
	struct hero far *h;
	register signed short found;
	register signed short ret;

	ret = 1;
	if (DATE->ticks < SMITH_OPEN || DATE->ticks > SMITH_CLOSE) {
		if (g_pp20_index == 4 || g_pp20_index == 0x2ae)
			screen_textbar(g_screen, txt[g_loc_name], 0xff);
		dlg_message(g_dialog, txt[0]);
		ret = 0;
	} else if (g_smith_banned[g_loc_id]) {
		if (g_pp20_index == 4 || g_pp20_index == 0x2ae)
			screen_textbar(g_screen, txt[g_loc_name], 0xff);
		dlg_message(g_dialog, txt[0x24]);
		ret = 0;
	} else if (g_loc_id == 1) {
		found = 0;
		h = get_first_hero(g_group);
		do {
			if (check_hero3(h) && h->type >= HERO_TYPE_GREEN_ELF && h->type <= HERO_TYPE_SYLVAN_ELF &&
				!h->harmless && !h->adler_wolf)
				found = 1;
			h = get_next_hero(g_group, h);
		} while (get_first_hero(g_group) != h && !found);
		if (found) {
			screen_textbar(g_screen, txt[g_loc_name], 0xff);
			load_portrait_buf(0x97);
			dlg_box(g_dialog, g_portrait_buf, txt[g_loc_name], txt[0x23], 0);
			ret = 0;
		}
	}
	return ret;
}

void Smith::run()
{
	struct hero far *h;
	register signed short ans;

	while (done == 0) {
		if (g_refresh) {
			draw_loc_icons(g_screen, 3, 0x15, 0x12, 8);
			screen_redraw(g_screen);
			anim_load(g_screen_pic, female == 0 ? 0x2b : 0x18a, 0);
			screen_textbar(g_screen, txt[g_loc_name], 0xff);
			snd_play(g_snd_music, 4);
			g_refresh = 0;
		}
		check_closing(DATE->ticks);
		handle_input(g_screen);
		if (g_mouse_rclick != 0 || g_key == KEY_PGUP) {
			ans = dlg_choose(g_dialog, txt[1], 3, txt[2], txt[3], txt[4]) - 1;
			if (ans != -2)
				g_action = ans + ACTION_ICON_1;
		}
		if (g_action == ACTION_ICON_1)
			talk();
		else if (g_action == ACTION_ICON_2) {
			if (repair->item_id)
				pickup();
			else {
				h = smith_select_hero(this);
				if (h) {
					ani_halt(g_screen_pic);
					anim_free_all(g_screen_pic);
					smith_repair(this, h);
					timewarp_minutes(g_date, 30);
				}
			}
		} else if (g_action == ACTION_ICON_3 || g_key == KEY_ESC)
			done = 1;
		if (g_leave_location)
			done = 1;
	}
}

void Smith::ban()
{
	g_smith_banned[g_loc_id] = done = 1;
}

void Smith::check_closing(signed long t)
{
	if (t > SMITH_CLOSE || t < SMITH_OPEN) {
		done = 1;
		sprintf(g_text_output_buf, txt[0x2a], female ? txt[0x27] : txt[0x26], grammar_pronoun(female, 0, 0));
		dlg_message(g_dialog, g_text_output_buf);
		g_mouse_rclick = g_action = 0;
	}
}

void Smith::talk()
{
	register signed short script = -1;

	conv_topic_on(g_dialog2, 0x17);
	conv_topic_on(g_dialog2, 0x1a);
	if (g_loc_id == 1)
		script = 0xf;
	else if (g_loc_id >= 2 && g_loc_id <= 4)
		script = g_loc_id + 0x16;
	else if (g_loc_id >= 5 && g_loc_id <= 7)
		script = g_loc_id + 0x33;
	else if (g_loc_id == 8)
		script = 0x58;
	else if (g_loc_id >= 9 && g_loc_id <= 10)
		script = g_loc_id + 0x5e;
	else if (g_loc_id == 11)
		script = 0x7d;
	if (script != -1 && conv_run(g_dialog2, script))
		timewarp_minutes(g_date, 30);
	conv_set_topic(g_dialog2, TOPIC_WAFFEN, 0);
	conv_set_topic(g_dialog2, TOPIC_SCHMIEDEKUNST, 0);
}

void Smith::pickup()
{
	if (date_since((struct timestamp far *)repair->since) > SMITH_REPAIR_TIME) {
		if (group_give_item(g_group, repair->item_id, 1)) {
			sprintf(g_text_output_buf, txt[6], female ? txt[0x29] : txt[0x28],
				grammar_article(0x8002, repair->item_id, 0));
			dlg_message(g_dialog, g_text_output_buf);
			repair->item_id = 0;
		}
		return;
	}
	sprintf(g_text_output_buf, txt[5], female ? txt[0x27] : txt[0x26]);
	dlg_message(g_dialog, g_text_output_buf);
}

signed short Smith::grid_input(signed short far *redraw)
{
	register signed short ret;
	register signed short i;

	ret = 0;
	if (g_mouse_locked == 0 && (mouse_x != g_mouse_posx || mouse_y != g_mouse_posy)) {
		mouse_x = g_mouse_posx;
		mouse_y = g_mouse_posy;
		for (i = 0; i < 15; i++) {
			if (g_merchant_items_posx[i] <= g_mouse_posx && g_merchant_items_posx[i] + 50 >= g_mouse_posx &&
			    g_merchant_items_posy[i] <= g_mouse_posy && g_merchant_items_posy[i] + 17 >= g_mouse_posy &&
			    items[page + i].item_id != 0) {
				sel = i;
				break;
			}
		}
	} else if (g_action == KEY_UP) {
		if (sel > 0)
			sel--;
		else {
			sel = 14;
			while (items[page + sel].item_id == 0 && sel != 0)
				sel--;
		}
	} else if (g_action == KEY_DOWN) {
		if (sel < 14) {
			sel++;
			if (items[page + sel].item_id == 0)
				sel = 0;
		} else
			sel = 0;
	} else if (g_action == KEY_RIGHT) {
		if (sel < 10) {
			if (items[page + sel + 5].item_id != 0)
				sel += 5;
		} else {
			if (items[page + 15].item_id != 0) {
				*redraw = 1;
				page += 15;
			}
		}
	} else if (g_action == KEY_LEFT) {
		if (sel > 4)
			sel -= 5;
		else {
			ret = 1;
			if (page != 0) {
				page -= 15;
				*redraw = 1;
#if FIX_0024
			/* Fix #24: erste Spalte der ersten Seite: Cursor bleibt stehen (sel -= 10 ergab immer 0) */
			}
#else
			} else {
				sel -= 10;
				if (sel < 0)
					sel = 0;
			}
#endif
		}
	}
	return ret;
}

signed short smith_talk_event(signed short state, signed short answer)
{
	if (state == 1 && answer == 1) {
		conv_finish(g_dialog2);
		if (g_smith)
			SMITH->ban();
	}
	return 0;
}
