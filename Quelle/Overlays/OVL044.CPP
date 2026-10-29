/* OVL044 - Overlay O044 (Segment 0x2000 in SCHWEIF.EXE): Tempel (entspricht DSA1 seg061 do_temple/char_add/
 * char_letgo/char_erase). class Temple (s_class.h, 0x11 Bytes, Zeiger g_temple): god +0, Bild pic +2 (Bmp 0x2b x
 * 0x1d), Texte txt_buf/txt +6/+0a (txt_load_list(g_font, 0x16, 0x22)), Ende done +0e, Art kind +10 aus Datei 0x205
 * (g_file_3, Index g_loc_id). Temple(): Ktor (this == 0 -> new 0x11), g_refresh = 1, g_temple_visited = g_temple_visited2 = done = 0.
 * ~Temple(): Bild/Texte frei, g_temple = 0. temple_is_open(t): Tempel entweiht (g_temple_desecrated[g_loc_id]) ->
 * Meldung txt[23], Ergebnis 0. temple_run(t): Hauptschleife - Menue dlg_choose (9 Punkte), 0x81 Gespraech
 * temple_talk, 0x82 Helden aufnehmen temple_add_heroes, 0x83 entlassen temple_dismiss_heroes, 0x84 Spielstand (new
 * SaveMenu / savegame_menu / delete), 0x85 Held erschaffen (new CharGen, chargen_run, delete), 0x86 Held loeschen
 * temple_erase_hero, 0x87 Spende (max. Geld/10, hoechstens 9999; Ansehen g_god_favor[god] += n, Geld -= 10n), 0x88
 * Wunder temple_miracle (O033), 0x89/Esc Ende. temple_add_heroes/temple_erase_hero: Heldenliste (struct chr_entry,
 * chr_list_collect/select_from_list/chr_list_remove), temple_load_pic: Tempelbild (Ifirn 14 -> Firun 7),
 * temple_redraw: Neuzeichnen bei unveraendertem Gott, temple_talk: Dialogmerker (conv_topic_on/conv_set_topic),
 * conv_run je Tempel, temple_desecrate (aus O258): Tempel entweihen, temple_load_kind: Byte kind lesen,
 * temple_find_god: Gott ueber g_temple_god_locs[i] (is_in_byte_array), temple_diary_party: Gruppenliste
 * "A, B und C" einmalig (g_diary_new_items) per diary_add, temple_str_join/temple_str_leave: Tagebuchtexte.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL044.CPP   -> 5316/5316 Bytes GLEICH (18.10.2026), cobjfix laeuft.
 *   Ohne -Og 1 Byte zu lang (ab temple_add_heroes verschoben).
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Erster Entwurf sofort GLEICH. if/else am Funktionsende (temple_add_heroes, temple_redraw) -> Epilogkopie am Ende
 *   des then-Zweigs, else-Teil hinter dem Hauptepilog.
 * - Spende: Grenze "(signed short)don < 100 ? (signed short)don : 100" -> cmp word [bp-8] auf dem long-Local.
 * - "g_temple_visited = g_temple_visited2 = done = 0" (Kette rechts zuerst gespeichert); Spielstand wie OVL046 (new SaveMenu mit
 *   Zwischenwert [bp-8], delete -> if (p) { Pic_dtor(p, 2); mem_free_v(p); }).
 * - buf[sel].arc_id/.slot als zwei int-Argumente (Segment je Wort nachgeladen, Offset als -Og-Zwischenwert
 *   [bp-18h]).
 */
#include <stdio.h>
#include <string.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:49c0..4a3c, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool.
 * Tempel je Gott 1..14 (Namen g_ttx[4 + Gott]): Liste der Tempel-Orte (g_loc_id), Ende 0xff; Efferd und Swafnir ohne Tempel. */
char g_temple_locs_praios[4] = { 2, 24, 25, 0xff };	/* Praios */
char g_temple_locs_rondra[2] = { 17, 0xff };	/* Rondra */
char g_temple_locs_efferd[1] = { 0xff };	/* Efferd */
char g_temple_locs_travia[3] = { 6, 18, 0xff };	/* Travia */
char g_temple_locs_boron[4] = { 3, 10, 16, 0xff };	/* Boron */
char g_temple_locs_hesinde[3] = { 7, 14, 0xff };	/* Hesinde */
char g_temple_locs_firun[3] = { 15, 20, 0xff };	/* Firun */
char g_temple_locs_tsa[4] = { 11, 26, 27, 0xff };	/* Tsa */
char g_temple_locs_phex[3] = { 4, 9, 0xff };	/* Phex */
char g_temple_locs_peraine[3] = { 1, 8, 0xff };	/* Peraine */
char g_temple_locs_ingerimm[5] = { 5, 13, 21, 28, 0xff };	/* Ingerimm */
char g_temple_locs_rahja[4] = { 12, 19, 23, 0xff };	/* Rahja */
char g_temple_locs_swafnir[1] = { 0xff };	/* Swafnir */
char g_temple_locs_ifirn[2] = { 22, 0xff };	/* Ifirn */
signed char far *g_temple_god_locs[15] = {
	0, (signed char far *)&g_temple_locs_praios, (signed char far *)&g_temple_locs_rondra, (signed char far *)&g_temple_locs_efferd, (signed char far *)&g_temple_locs_travia, (signed char far *)&g_temple_locs_boron, (signed char far *)&g_temple_locs_hesinde, (signed char far *)&g_temple_locs_firun, (signed char far *)&g_temple_locs_tsa, (signed char far *)&g_temple_locs_phex, (signed char far *)&g_temple_locs_peraine, (signed char far *)&g_temple_locs_ingerimm, (signed char far *)&g_temple_locs_rahja, (signed char far *)&g_temple_locs_swafnir, (signed char far *)&g_temple_locs_ifirn
};
TEMPLE_PTR g_temple = 0;

/* Name 0 der Arbeitsdatei g_ofile (erster Eintrag der Namensliste g_names_star_idx, vgl. O012) */
#define TEMP_NAME0 (*(char far * far *)g_names_star_idx)

Temple::Temple()
{
	g_temple = this;
	pic = new Bmp(0x2b, 0x1d);
	txt_load_list(g_font, 0x16, 0x22, &txt_buf, &txt);
	g_refresh = 1;
	g_temple_visited = g_temple_visited2 = done = 0;
}

Temple::~Temple()
{
	delete pic;
	mem_free(txt_buf);
	mem_free(txt);
	g_temple = 0;
}

signed short temple_is_open(Temple far *t)
{
	signed short ok = 1;

	temple_find_god(t);
	if (g_temple_desecrated[g_loc_id] != 0) {
		if (g_pp20_index == 4 || g_pp20_index == 0x2ae) {
			sprintf(g_text_output_buf, t->txt[0], g_ttx[t->god + 4], g_ttx[g_group->town + 0x163]);
			screen_textbar(g_screen, g_text_output_buf, 0xff);
		}
		dlg_message(g_dialog, t->txt[23]);
		ok = 0;
	}
	return ok;
}

void temple_run(Temple far *t)
{
	signed short redraw = 0;
	signed short sel;

	while (t->done == 0) {
		if (g_refresh != 0) {
			draw_loc_icons(g_screen, 9, 0x15, 0, 1, 0x28, 0x40, 2, 7, 6, 8);
			screen_redraw(g_screen);
			g_refresh = 0;
			redraw = 1;
		}
		if (redraw) {
			temple_load_kind(t);
			temple_find_god(t);
			temple_load_pic(t, t->god);
			anim_load(g_screen_pic, t->kind == 0 ? 0x188 : 0x27, 0);
			mouse_hide(0);
			t->pic->put(g_vga_buf, 0x62, 2, 4);
			mouse_show(0);
			snd_play(g_snd_music, 1);
			sprintf(g_text_output_buf, t->txt[0], g_ttx[t->god + 4], g_ttx[g_group->town + 0x163]);
			screen_textbar(g_screen, g_text_output_buf, 0xff);
			redraw = 0;
		}
		temple_diary_party(t);
		handle_input(g_screen);
		if (g_action == 0x4100 || g_action == 0x2004)
			g_action = 0x84;
		if (g_mouse_rclick != 0 || g_key == 0x4900) {
			sel = dlg_choose(g_dialog, t->txt[1], 9, t->txt[t->kind + 0x15], t->txt[2], t->txt[3],
				t->txt[20], t->txt[26], t->txt[4], t->txt[9], t->txt[8], t->txt[10]) - 1;
			if (sel != -2)
				g_action = sel + 0x81;
		}
		if (g_action == 0x81) {
			temple_talk(t);
			if (t->done == 0)
				timewarp_minutes(g_date, 0x1eL);
			else
				memcpy(g_palette, g_pal_buf, 0x300);
		} else if (g_action == 0x82) {
			temple_add_heroes(t, g_loc_id);
			g_refresh = 0;
		} else if (g_action == 0x83) {
			temple_dismiss_heroes(t, g_loc_id);
			g_refresh = 0;
		} else if (g_action == 0x84) {
			SaveMenu far *p = new SaveMenu;
			savegame_menu(p, 0);
			delete p;
			if ((signed short)g_group->loctype == LOC_TEMPLE) {
				if (g_refresh != 0) {
					screen_portraits(g_screen);
					g_refresh = 0;
					redraw = 1;
				}
			} else {
				g_refresh = 0;
				t->done = 1;
			}
		} else if (g_action == 0x85) {
			CharGen far *cg;
			loc_fade_out();
			cg = new CharGen;
			if (cg->run())
				dlg_message(g_dialog, t->txt[27]);
			delete cg;
		} else if (g_action == 0x86) {
			temple_erase_hero(t);
		} else if (g_action == 0x87) {
			if (!g_group->nr_heroes) {
				dlg_message(g_dialog, t->txt[11]);
			} else {
				signed long money;
				signed long don;
				money = g_group->money;
				if (money == 0) {
					dlg_message(g_dialog, g_ttx[4]);
				} else {
					don = money / 10;
					if (don > 9999)
						don = 9999;
					money_to_string(g_text_buf2, money);
					sprintf(g_text_output_buf, t->txt[13], g_ttx[t->god + 4], g_text_buf2);
					money = dlg_input(g_dialog, 4, g_text_output_buf, 0, (signed short)don,
						(signed short)don < 100 ? (signed short)don : 100);
					if (money > 0) {
						g_god_favor[t->god] += money;
						money *= 10;
						group_sub_money(g_group, money);
						snd_play(g_snd_fx, 0x47);
						sprintf(g_text_output_buf, t->txt[14], t->kind ? t->txt[25] : t->txt[24]);
						dlg_message(g_dialog, g_text_output_buf);
					}
				}
			}
		} else if (g_action == 0x88) {
			if (!g_group->nr_heroes)
				dlg_message(g_dialog, t->txt[11]);
			else
				temple_miracle(t);
		} else if (g_action == 0x89 || g_key == 0x11b) {
			if (!g_group->nr_heroes)
				dlg_message(g_dialog, t->txt[11]);
			else
				t->done = 1;
		}
		if (g_leave_location != 0)
			t->done = 1;
	}
	g_temple_visited = g_temple_visited2 = 1;
}

void temple_add_heroes(Temple far *t, signed short typ)
{
	signed short n;
	signed short sel;
	signed short slot;
	signed short left;
	signed short pos;
	signed short i;
	signed short cnt;
	struct hero far *h;
	struct chr_entry far *list;
	signed long size;

	size = ofile_length_id(g_ofile, 0xfa);
	TEMP_NAME0 = file_getname(g_file_main, 0xfb);
	size += ofile_length_id(g_ofile, 0);
	n = size / (signed long)sizeof(struct hero);
	if (n > 0) {
		list = (struct chr_entry far *)mem_alloc(n * sizeof(struct chr_entry));
		memset(list, 0, n * sizeof(struct chr_entry));
		n = chr_list_collect(list, typ);
		for (cnt = 0, i = 0, h = g_heroes; i < 6; i++, h++) {
			if (h->type != 0)
				cnt++;
		}
		left = 6 - cnt;
		if (left == 0) {
			dlg_message(g_dialog, t->txt[15]);
		} else {
			do {
				if (n == 0) {
					dlg_message(g_dialog, t->txt[16]);
					sel = -1;
				} else if ((sel = select_from_list((char far *)list, n, 1)) != -1) {
					if ((slot = group_free_slot(g_group)) != -1) {
						if ((pos = get_free_hero_slot()) != -1) {
							h = g_heroes + pos;
							mouse_set_cursor(5, 1);
							if (hero_load_chr(list[sel].arc_id, list[sel].slot, pos, g_group_no)) {
								left--;
								group_add_hero(g_group, h, slot);
								screen_portraits(g_screen);
								chr_list_remove(list, sel, n);
								n--;
								if (g_diary_new_items)
									diary_add(g_diary, temple_str_join(t, h), 0);
							}
							mouse_reset();
						}
					}
				}
			} while (sel != -1 && left > 0);
		}
		mem_free(list);
	} else {
		dlg_message(g_dialog, t->txt[16]);
	}
}

void temple_dismiss_heroes(Temple far *t, register signed short typ)
{
	signed short min;
	struct hero far *h;

	min = group_get_npc(g_group) ? 1 : 0;
	if (!g_group->nr_heroes) {
		dlg_message(g_dialog, t->txt[11]);
	} else {
		do {
			h = get_any_hero(g_group, t->txt[17]);
			mouse_set_cursor(5, 1);
			if (h != 0) {
				if (h->npc_id != 0) {
					g_npc_months = 0x63;
					groups_check_npc();
					screen_portraits(g_screen);
				} else {
					if (g_diary_new_items)
						diary_add(g_diary, temple_str_leave(t, h), 0);
					group_set_slot(g_group, group_hero_pos(g_group, h), typ);
					screen_portraits(g_screen);
				}
			}
			mouse_reset();
		} while (h != 0 && g_group->nr_heroes > min);
	}
	if (!!g_group->nr_heroes && group_get_npc(g_group) == get_first_hero(g_group)) {
		g_npc_months = 0x63;
		groups_check_npc();
		screen_portraits(g_screen);
	}
}

signed short temple_erase_hero(Temple far *t)
{
	signed short ret;
	struct chr_entry far *list;
	signed long size;
	signed short sel;
	signed short n;

	size = ofile_length_id(g_ofile, 0xfa);
	TEMP_NAME0 = file_getname(g_file_main, 0xfb);
	size += ofile_length_id(g_ofile, 0);
	n = size / (signed long)sizeof(struct hero);
	if (n > 0) {
		list = (struct chr_entry far *)mem_alloc(n * sizeof(struct chr_entry));
		mouse_set_cursor(5, 1);
		n = chr_list_collect(list, -1);
		mouse_reset();
		ret = 1;
		do {
			if (n == 0) {
				sel = -1;
				dlg_message(g_dialog, t->txt[28]);
			} else {
				sel = select_from_list((char far *)list, n, -1);
				if (sel != -1) {
					strcpy(g_text_output_buf, list[sel].name);
					sprintf(g_text_buf2, t->txt[19], g_text_output_buf);
					if (dlg_yes_no(g_dialog, g_text_buf2) == 1) {
						mouse_set_cursor(5, 1);
						hero_delete_chr(list[sel].file_name);
						chr_list_remove(list, sel, n);
						n--;
						mouse_reset();
					}
				} else {
					ret = 0;
					break;
				}
			}
		} while (sel != -1);
		mem_free(list);
	} else {
		dlg_message(g_dialog, t->txt[28]);
		ret = 1;
	}
	return ret;
}

void temple_load_pic(Temple far *t, register signed short god)
{
	signed short w, h;

	if (god == 14)
		god = 7;		/* Ifirn -> Firun */
	nvfile_load(g_nvf_heads, 0x11, t->pic->data(), 0, god - 1, 0, &w, &h, 320);
}

void temple_redraw(Temple far *t)
{
	signed short kind = t->kind;
	signed short god = t->god;

	temple_find_god(t);
	temple_load_kind(t);
	if (t->god == god && t->kind == kind) {
		screen_redraw(g_screen);
		anim_reset(g_screen_pic, 2);
		sprintf(g_text_output_buf, t->txt[0], g_ttx[t->god + 4], g_ttx[g_group->town + 0x163]);
		screen_textbar(g_screen, g_text_output_buf, 0xff);
	} else {
		g_refresh = 1;
	}
}

void temple_talk(Temple far *t)
{
	signed short script;

	if (!g_group->nr_heroes) {
		dlg_message(g_dialog, t->txt[11]);
		return;
	}
	script = -1;
	conv_topic_on(g_dialog2, t->god + 0x1b);
	if (t->god == 11) {
		conv_topic_on(g_dialog2, 0x17);
		conv_topic_on(g_dialog2, 0x1a);
	} else if (t->god == 14) {
		conv_topic_on(g_dialog2, 0x22);
	} else if (t->god == 9) {
		conv_topic_on(g_dialog2, 0x15);
	}
	if (g_loc_id == 1)
		script = 0;
	else if (g_loc_id >= 2 && g_loc_id <= 4)
		script = g_loc_id + 5;
	else if (g_loc_id == 5)
		script = 0x14;
	else if (g_loc_id >= 6 && g_loc_id <= 13)
		script = g_loc_id + 0x17;
	else if (g_loc_id >= 14 && g_loc_id <= 19)
		script = g_loc_id + 0x3d;
	else if (g_loc_id >= 20 && g_loc_id <= 23)
		script = g_loc_id + 0x4a;
	else if (g_loc_id == 24)
		script = 0x71;
	else if (g_loc_id == 25)
		script = 0x73;
	else if (g_loc_id == 26)
		script = 0x77;
	else if (g_loc_id == 27)
		script = 0x7a;
	else if (g_loc_id == 28)
		script = 0x7e;
	if (script != -1)
		conv_run(g_dialog2, script);
	conv_set_topic(g_dialog2, t->god + 0x1b, 0);
	conv_set_topic(g_dialog2, TOPIC_WAFFEN, 0);
	conv_set_topic(g_dialog2, TOPIC_SCHMIEDEKUNST, 0);
	conv_set_topic(g_dialog2, TOPIC_GOD_FIRUN, 0);
	conv_set_topic(g_dialog2, TOPIC_HANDEL, 0);
}

void temple_desecrate(Temple far *t)
{
	g_temple_desecrated[g_loc_id] = 1;
	t->done = 1;
}

void temple_load_kind(Temple far *t)
{
	arc_open(g_file_3, 0x205);
	arc_seek(g_file_3, (signed long)g_loc_id, 0);
	arc_read(g_file_3, &t->kind, 1L);
	arc_close(g_file_3);
}

void temple_find_god(Temple far *t)
{
	signed short i;

	t->god = 1;
	for (i = 1; i < 15; i++) {
		if (is_in_byte_array((signed char)g_loc_id, g_temple_god_locs[i]) != -1) {
			t->god = i;
			return;
		}
	}
}

void temple_diary_party(Temple far *t)
{
	struct hero far *h;
	signed short n;

	if (!g_diary_new_items && !!g_group->nr_heroes) {
		h = get_first_hero(g_group);
		n = g_group->nr_heroes;
		*g_text_buf2 = 0;
		do {
			n--;
			if (get_first_hero(g_group) != h) {
				if (n == 0) {
					sprintf(g_text_output_buf, " %s ", g_ttx[422]);
					strcat(g_text_buf2, g_text_output_buf);
				} else {
					strcat(g_text_buf2, g_ttx[420]);
				}
			}
			strcat(g_text_buf2, h->name);
			h = get_next_hero(g_group, h);
		} while (get_first_hero(g_group) != h);
		sprintf(g_text_output_buf, (signed short)g_group->nr_heroes > 1 ? t->txt[29] : t->txt[33], g_text_buf2);
		diary_add(g_diary, g_text_output_buf, 0);
		g_diary_new_items = 1;
	}
}

char far *temple_str_join(Temple far *t, struct hero far *h)
{
	sprintf(g_text_buf2, "%s %s", h->sex ? g_ttx[254] : g_ttx[253],
		h->sex ? g_ttx[h->type + 62] : g_ttx[h->type + 50]);
	sprintf(g_text_output_buf, t->txt[30], h->name, g_text_buf2);
	return g_text_output_buf;
}

char far *temple_str_leave(Temple far *t, struct hero far *h)
{
	sprintf(g_text_buf2, "%s %s", h->sex ? g_ttx[254] : g_ttx[253],
		h->sex ? g_ttx[h->type + 62] : g_ttx[h->type + 50]);
	sprintf(g_text_output_buf, t->txt[31], h->name, g_text_buf2);
	return g_text_output_buf;
}
