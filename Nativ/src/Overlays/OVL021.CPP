/* OVL021 - Overlay O021 (Segment 0x1f6f in SCHWEIF.EXE): Sonderkampf 2 (Kampfnummer 0x495, Figur 0x42) - der Gegner
 * stellt ein Raetsel, drei Freitext-Eingaben werden gegen Wortlisten geprueft.
 * sfight2_setup(f): Kampfsatz einrichten (field->type = 6, fight_load_grid, 7 Startplaetze aus g_sfight2_starts (je
 *   3 von 4 Bytes), asleep = 1 fuer schlafende Gruppenmitglieder (NSC auf Platz 6), ein Gegner (Art 1), Beute
 *   0x151/0x150, kein Geld).
 * sfight2_place_enemy(f): Animationsobjekt FigAnim_ctor nach f->anims[f->nr_anims++] und f->tmpl.anim, Vorlage
 *   (Figur 0x42, z 0x63), fig_list_add -> g_enemies[0].fighter_id.
 * sfight2_say(f, s, bottom, snd): Text s (font_print_wrapped) oben/unten in den Textkasten, Sprachausgabe snd + 0x3f2
 *   (g_snd_speech), Wartezeit aus Textlaenge und g_text_speed (mindestens 0x8c), Tastaturpuffer leeren, fight_draw.
 * sfight2_check_word(f, s): s gegen die Texte 20..41 (richtig) bzw. 42..63 (falsch) vergleichen (strcmp), neue
 *   Treffer in g_sfight2_right[]/g_sfight2_wrong[] (Zaehler g_sfight2_nr_right/_nr_wrong), 1 bei richtigem Wort.
 * sfight2_intro(f): Ablauf (Texte 0x43 nach g_sfight2_txt_buf/g_sfight2_txt, Datei 0x42 ggf. ins XMS vorladen
 *   (file_length/xms_free_kb), Textfolge, drei Eingaben dlg_message_flag nach g_input_buf), bei >= 2 richtigen und keinem
 *   falschen Wort Folge sfight2_seq_vanish, Figur entfernen, Hintergrund (Pic 0x41) und 64000-Byte-Kopie ueber
 *   g_xms_move (EMS_copy bzw. xms_move_any je nach g_fight_use_ems), g_in_fight = 0, sonst Folge sfight2_seq_attack
 *   und Kampf (g_game_state = 1).
 * sfight2_seq_stand/_appear/_attack/_vanish(f, seq): Ablaufbeschreibung seq (0xa2 Bytes) mit Folge 1/0x0e/0x47/0x6f
 *   fuellen und per seq_set auf der Gegnerfigur starten (Lage g_sfight2_x/y).
 *
 * BCC -mlarge -3 -Ot -c -Y OVL021.CPP   -> 3650/3650 Bytes GLEICH (18.10.2026), cobjfix laeuft (auch mit -Og gleich)
 *   (Namensfassung mit schweif.h, Welle N15: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * - C++ wegen lokaler Objekte: Bmp (gb_new0/gb_delete) in sfight2_say, Pic (Pic_ctor, pic_load, Pic_dtor) im
 *   if-Block von sfight2_intro -> Destruktor am Ende dieses Blocks, nicht am Funktionsende.
 * - unsigned-char-Bitfeld h->asleep mit (unsigned char)-Cast (shr ax,1 / and ax,1 / or al,al).
 * - Lokales Feld "struct sfight_starts tab = g_sfight2_starts;" -> rep movsw aus DS (keine eigenen Daten; kein
 *   eigenes Public, cobjfix: _g_118d+1).
 * - Kette "f->tmpl.anim = f->anims[f->nr_anims++] = FigAnim_ctor(...)" (push ax / pop dx), "ducats = silver = heller = 0".
 * - font_set_colors mit Wort-Argumenten (gecasteter Funktionszeiger font_set_colors_w).
 * - In sfight2_say liegen 10-Byte-Bmp und vier int-Locals in Deklarationsreihenfolge (x, y, b, box_x..box_align).
 * Originalfehler: sfight2_setup liest h->asleep/h->npc_id vor der Nullpruefung (die Schleife prueft get_first_hero()
 *   != 0 erst nach dem ersten Durchlauf). sfight2_say setzt x = 5 in beiden Zweigen (wirkungslos, nur auffaellig).
 */
#include <string.h>
#include <bios.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:118c..11aa, dsana) - initialisierte Daten in Adressreihenfolge */
signed char g_sfight2_nr_right = 0;
signed char g_sfight2_nr_wrong = 0;
struct sfight_starts g_sfight2_starts = {
	{ { 12, 0, 3, 0 },
	{ 15, 0, 3, 0 },
	{ 20, 5, 2, 0 },
	{ 21, 7, 2, 0 },
	{ 20, 9, 2, 0 },
	{ 18, 12, 1, 0 },
	{ 15, 13, 1, 0 } }
};
/* Phase B: eigenes BSS (ds:8b7a..8b88) in Adressreihenfolge */
void far *g_sfight2_txt_buf;
char far * far *g_sfight2_txt;
signed char g_sfight2_right[3];
signed char g_sfight2_wrong[3];

#define font_set_colors_w ((void (*)(void far *, signed short, signed short))font_set_colors)

/* Kampfsatz fuer den Sonderkampf 0x495 einrichten: Feldart 6, Grundgitter, Startplaetze aus g_sfight2_starts
 * (schlafende Helden schlafen weiter, NSC auf Platz 6), keine Gegner der Aufstellung, Beute 0x151/0x150, kein Geld */
void sfight2_setup(Fight far *f)
{
	signed short n = 0;
	signed short i;
	struct hero far *h;
	struct sfight_starts tab = g_sfight2_starts;

	f->field->type = 6;
	fight_load_grid(f);
	memset(f->heroes, 0, sizeof(f->heroes));
	for (i = 0; i < 7; i++) {
		f->heroes[i].x = tab.p[i].x;
		f->heroes[i].y = tab.p[i].y;
		f->heroes[i].viewdir = tab.p[i].viewdir;
	}
	h = get_first_hero(g_group);
	n = 0;
#if FIX_0049
	/* Fix #49: Nullpruefung vor dem ersten Zugriff */
	if (h != 0) {
		do {
			if ((unsigned char)h->asleep)
				f->heroes[h->npc_id ? 6 : n].asleep = 1;
			h = get_next_hero(g_group, h);
			n++;
		} while (get_first_hero(g_group) != h && get_first_hero(g_group) != 0);
	}
#else
	do {
		if ((unsigned char)h->asleep)
			f->heroes[h->npc_id ? 6 : n].asleep = 1;
		h = get_next_hero(g_group, h);
		n++;
	} while (get_first_hero(g_group) != h && get_first_hero(g_group) != 0);
#endif
	memset(f->monsters, 0, sizeof(f->monsters));
	f->monsters[0].mon_id = 1;
	f->monsters[0].x = 0;
	f->monsters[0].y = 0;
	memset(f->loot, 0, 0x1e);
	f->loot[0] = 0x151;
	f->loot[1] = 0x150;
	f->ducats = f->silver = f->heller = 0;
}

/* Figur des Gegners anlegen (Animationsobjekt, Vorlage tmpl: Figur 0x42) und einsortieren */
void sfight2_place_enemy(Fight far *f)
{
	struct enemy far *e = g_enemies;
	signed char id;

	f->tmpl.anim = f->anims[f->nr_anims++] = (FIGANIM_PTR)FigAnim_ctor(0, &f->seq, 1);
	f->tmpl.cbx = f->tmpl.offsetx = 0;
	f->tmpl.cby = f->tmpl.offsety = 0;
	f->tmpl.obj_id = 0;
	f->tmpl.twofielded = FIG_TWOFIELD_NONE;
	f->tmpl.kind = FIG_KIND_ENEMY;
	f->tmpl.figure = FIG_FIGURE_SFIGHT2;
	f->tmpl.gfx = 0;
	f->tmpl.z = 0x63;
	f->tmpl.visible = 0;
	id = fig_list_add(&f->list, -1);
	e->fighter_id = id;
}

/* Text s im Kampfbild ausgeben (bottom 0: oben, sonst unten), Sprachausgabe snd + 0x3f2, warten */
void sfight2_say(Fight far *f, char far *s, signed short bottom, signed short snd)
{
	signed short x;
	signed short y;
	Bmp b;
	signed short box_x, box_w, box_y, box_align;	/* alter Textkasten */
	register signed short lines;
	register signed short wait;

	if (bottom == 0) {
		font_set_colors_w(g_font, 0xfc, -1);
		x = 5;
		y = 10;
	} else {
		font_set_colors_w(g_font, 0xfd, -1);
		x = 5;
		y = 0x8c;
	}
	font_get_box(g_font, &box_x, &box_w, &box_y, &box_align);
	font_set_box(g_font, x, g_fight_screen_on ? 0x8c : 0x136, y, 0);
	if (g_2546 && g_speech_ext_avail) {
		while (snd_speech_busy(g_snd_speech))
			game_idle();
		snd_speech(g_snd_speech, snd + 0x3f2);
	}
	lines = font_print_wrapped(g_font, s) + 1;
	if (!g_fight_screen_on)
		lines *= 3;
	font_restore_colors(g_font);
	font_set_box(g_font, box_x, box_w, box_y, box_align);
	while (bioskey(1))
		bioskey(0);
	clear_input();
	mouse_clear_left();
	mouse_clear_right();
	wait = lines * (g_text_speed + 1) * 20;
	if (wait < 0x8c)
		wait = 0x8c;
	if (g_2546 && g_speech_ext_avail) {
		while (snd_speech_busy(g_snd_speech))
			game_idle();
		wait_ticks_key(40);
	} else
		wait_ticks_key(wait);
	g_fig_redraw_all = 1;
	fight_draw(f);
}

/* eingegebenes Wort pruefen: neu genanntes richtiges Wort (Texte 20..41) -> 1, sonst falsches (42..63) merken */
signed short sfight2_check_word(Fight far *f, char far *word)
{
	signed short i;
	register signed short found = 0;

	for (i = 20; i <= 41; i++) {
		if (!strcmp(word, g_sfight2_txt[i]) && g_sfight2_right[0] != i && g_sfight2_right[1] != i && g_sfight2_right[2] != i) {
			found = 1;
			g_sfight2_right[g_sfight2_nr_right] = i;
			g_sfight2_nr_right++;
			break;
		}
	}
	if (!found) {
		for (i = 42; i <= 63; i++) {
			if (!strcmp(word, g_sfight2_txt[i]) && g_sfight2_wrong[0] != i && g_sfight2_wrong[1] != i && g_sfight2_wrong[2] != i) {
				g_sfight2_wrong[g_sfight2_nr_wrong] = i;
				g_sfight2_nr_wrong++;
				break;
			}
		}
	}
	return found;
}

/* Ablauf vor dem Kampf: Gespraech mit drei Eingaben; mindestens zwei richtige und kein falsches Wort -> der Gegner
 * verschwindet und es gibt keinen Kampf, sonst greift er an */
void sfight2_intro(Fight far *f)
{
	struct enemy far *e;
	struct fig_node far *sl;
	register signed short i;
	register signed short k;

	txt_load_list(g_font, 0x43, 0x40, &g_sfight2_txt_buf, &g_sfight2_txt);
	g_sfight2_nr_right = g_sfight2_nr_wrong = 0;
	g_sfight2_right[0] = g_sfight2_right[1] = g_sfight2_right[2] = g_sfight2_wrong[0] = g_sfight2_wrong[1] = g_sfight2_wrong[2] = 0;
	g_fig_redraw_all = 1;
	fight_draw(f);
	g_fight_screen_on = 0;
	if (file_length(g_file_main, FIG_FIGURE_SFIGHT2) / 1024 < xms_free_kb()) {
		arc_open(g_file_3, FIG_FIGURE_SFIGHT2);
		arc_close(g_file_3);
	}
	sfight2_say(f, g_sfight2_txt[0], 0, 1);
	sfight2_say(f, g_sfight2_txt[1], 0, 2);
	sfight2_say(f, g_sfight2_txt[2], 0, 3);
	sfight2_say(f, g_sfight2_txt[3], 0, 4);
	g_fight_screen_on = 1;
	f->action_anim = sfight2_seq_appear(f, &f->seq);
	do
		game_idle();
	while (!seq_done(f->action_anim));
	sfight2_seq_stand(f, &f->seq);
	fight_draw(f);
	sfight2_say(f, g_sfight2_txt[8], 1, 9);
	sfight2_say(f, g_sfight2_txt[9], 1, 10);
	sfight2_say(f, g_sfight2_txt[4], 0, 5);
	sfight2_say(f, g_sfight2_txt[10], 1, 11);
	sfight2_say(f, g_sfight2_txt[5], 0, 6);
	do
		k = dlg_message_flag(g_dialog, 15, g_sfight2_txt[17]);
	while (k == -1);
	if (sfight2_check_word(f, g_input_buf))
		sfight2_say(f, g_sfight2_txt[11], 1, 12);
	else
		sfight2_say(f, g_sfight2_txt[12], 1, 13);
	sfight2_say(f, g_sfight2_txt[6], 0, 7);
	do
		k = dlg_message_flag(g_dialog, 15, g_sfight2_txt[18]);
	while (k == -1);
	if (sfight2_check_word(f, g_input_buf))
		sfight2_say(f, g_sfight2_txt[13], 1, 14);
	else
		sfight2_say(f, g_sfight2_txt[14], 1, 15);
	sfight2_say(f, g_sfight2_txt[7], 0, 8);
	do
		k = dlg_message_flag(g_dialog, 15, g_sfight2_txt[19]);
	while (k == -1);
	sfight2_check_word(f, g_input_buf);
	if (g_sfight2_nr_right >= 2 && !g_sfight2_nr_wrong) {
		sfight2_say(f, g_sfight2_txt[15], 1, 16);
		if (g_speech_ext_avail)
			Speech_wait(g_snd_speech);
		f->action_anim = sfight2_seq_vanish(f, &f->seq);
		do
			game_idle();
		while (!seq_done(f->action_anim));
		g_in_fight = 0;
		g_game_state = 0;
		e = g_enemies;
		sl = fig_list_find(&f->list, e->fighter_id);
		for (i = 0; i < 30; i++) {
			if (f->anims[i] == sl->anim) {
				FigAnim_dtor(f->anims[i], 3);
				f->anims[i] = 0;
			}
		}
		fig_list_remove(&f->list, e->fighter_id, 0);
		Pic pic;
		pic.load(0x41, f->buf);
		g_xms_move.len = 64000L;
		g_xms_move.src_handle = 0;
		g_xms_move.src = (signed long)f->buf;
		g_xms_move.dst_handle = (signed short)g_fight_xms;
		g_xms_move.dst = 0;
		if (g_fight_use_ems == 1)
			EMS_copy(&g_xms_move);
		else
			xms_move_any(&g_xms_move);
	} else {
		sfight2_say(f, g_sfight2_txt[16], 1, 17);
		if (g_speech_ext_avail)
			Speech_wait(g_snd_speech);
		f->action_anim = sfight2_seq_attack(f, &f->seq);
		snd_obj_stop(g_snd_fx);
		do
			game_idle();
		while (!seq_done(f->action_anim));
		sfight2_seq_stand(f, &f->seq);
		g_in_fight = 0;
		g_game_state = 1;
	}
	g_fig_redraw_all = 1;
	fight_draw(f);
	wait_ticks(50);
	e = g_enemies;
	fig_list_hide(&f->list, e->fighter_id);
	mem_free(g_sfight2_txt_buf);
	mem_free(g_sfight2_txt);
}

/* Gegnerfigur: Folge 1, Dauerschleife (stehen) */
FIGANIM_PTR sfight2_seq_stand(Fight far *f, struct ani_seq far *seq)
{
	struct fig_node far *sl;
	struct enemy far *e = g_enemies;

	fig_list_show(&f->list, e->fighter_id);
	sl = fig_list_find(&f->list, e->fighter_id);
	memset(seq, 0, sizeof(struct ani_seq));
	seq->figure = FIG_FIGURE_SFIGHT2;
	seq->step[0].frame = 1;
	seq->step[0].type = 3;
	seq->step[0].count = -1;
	seq->step[1].frame = ANI_FRAME_END;
	g_sfight2_x = 0x84;
	g_sfight2_y = 0;
	seq_set(sl->anim, seq);
	return sl->anim;
}

/* Gegnerfigur: Folge 0x0e einmal (Erscheinen) */
FIGANIM_PTR sfight2_seq_appear(Fight far *f, struct ani_seq far *seq)
{
	struct fig_node far *sl;
	struct enemy far *e = g_enemies;

	fig_list_show(&f->list, e->fighter_id);
	sl = fig_list_find(&f->list, e->fighter_id);
	memset(seq, 0, sizeof(struct ani_seq));
	seq->figure = FIG_FIGURE_SFIGHT2;
	seq->step[0].frame = 0x0e;
	seq->step[0].type = 0;
	seq->step[0].count = 1;
	seq->step[1].frame = ANI_FRAME_END;
	g_sfight2_x = 0x34;
	g_sfight2_y = 0;
	seq_set(sl->anim, seq);
	return sl->anim;
}

/* Gegnerfigur: Folge 0x47 einmal (Angriff - Raetsel nicht geloest) */
FIGANIM_PTR sfight2_seq_attack(Fight far *f, struct ani_seq far *seq)
{
	struct fig_node far *sl;
	struct enemy far *e = g_enemies;

	fig_list_show(&f->list, e->fighter_id);
	sl = fig_list_find(&f->list, e->fighter_id);
	memset(seq, 0, sizeof(struct ani_seq));
	seq->figure = FIG_FIGURE_SFIGHT2;
	seq->step[0].frame = 0x47;
	seq->step[0].type = 0;
	seq->step[0].count = 1;
	seq->step[1].frame = ANI_FRAME_END;
	g_sfight2_x = 0x34;
	g_sfight2_y = 0;
	seq_set(sl->anim, seq);
	return sl->anim;
}

/* Gegnerfigur: Folge 0x6f einmal (Verschwinden - Raetsel geloest) */
FIGANIM_PTR sfight2_seq_vanish(Fight far *f, struct ani_seq far *seq)
{
	struct fig_node far *sl;
	struct enemy far *e = g_enemies;

	fig_list_show(&f->list, e->fighter_id);
	sl = fig_list_find(&f->list, e->fighter_id);
	memset(seq, 0, sizeof(struct ani_seq));
	seq->figure = FIG_FIGURE_SFIGHT2;
	seq->step[0].frame = 0x6f;
	seq->step[0].type = 0;
	seq->step[0].count = 1;
	seq->step[1].frame = ANI_FRAME_END;
	g_sfight2_x = 0;
	g_sfight2_y = 0;
	seq_set(sl->anim, seq);
	return sl->anim;
}
