/* OVL020 - Overlay O020 (Segment 0x1f6a in SCHWEIF.EXE): Sonderkampf 0x493 (Zwischensequenzen mit Untertiteln)
 * und Gegner-Zug mit Zielwahl. Objekt p = Kampf-Objekt (+4 Rohtext, +8 Texte, +35 Liste, +41 Animation 0xa2).
 * sfight_event_text/sfight_event_text2(p, n, &snd): Ereignis n der Sequenz -> Klangnummer, Text p->txt[0..2] bzw. [3..6] (Merker
 *   ds:8b76 = Text zeigen, ds:8b78 = Text wegnehmen). sfight_seq_main(p): 7 Animationsphasen (Tabelle ds:0fa8), Ereignisse
 *   aus ds:cb0c, Untertitel per Textobjekt O248 (bei ds:253b eigene Texte 0x23 nachladen) und Bmp-Rettung
 *   (R42_0414/05F4/04A9/0480), Klaenge R04_05ED. sfight_seq_intro(p): Einleitung mit Bildfolge 0x6e/0x6f/0x70, Ereignisse
 *   nach Bildnummer (Tabelle ds:0fb6, Element 7 = 0x15). sfight_intro(p): Ablauf (sfight_seq_intro, sfight_seq_main nur ohne ds:278f),
 *   Palette 0x372 laden, Figur O136_0268. sfight_seq_end(p): Schluss (Tabelle ds:0fc6, Element 5 = 5), Bit +bc.2 des
 *   aktiven Helden loeschen. sfight_hit_chance(h, v): Trefferwert v nach Lage/Zustand des Helden (O203_0784 == -5, +b4,
 *   +bc.4, +4f7 = 1/2 -> halbieren/dritteln). sfight_enemy_turn(p): Gegner-Zug - Ziel aus fuenf Feldlisten (ds:0ff8/0fd4/
 *   0fe0/0fec/1050) suchen (Held 1..10 auf dem Feld, R02_03AB, W100 < sfight_hit_chance), Ergebnis nach e->action_id/cur_spell/enemy_id.
 *
 * BCC -mlarge -3 -Ot -c -Y OVL020.CPP   -> 5892/5892 Bytes GLEICH (18.10.2026), cobjfix laeuft (mit -Og ebenso).
 *   (Namensfassung mit schweif.h, Welle N7: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * - C++ wegen lokaler Bmp-Objekte (R42_0008/R42_03B3) und inline-Settern enemy::set32/set34 (Kopien [bp-11..-16]).
 * - Initialisierung in Deklarationsreihenfolge (snd, s, mem, Bmp, txo, flag, Tabelle).
 * - Lokale Tabelle mit einem nicht-konstanten Element (BCC schreibt den Wert in die DS-Vorlage und kopiert):
 *   ohne eigene Daten "g_0fb6.e[7].t = v; tab = g_0fb6;" (mov al,[bp-32h] / mov [0fc4],al / rep movsw).
 * - Die fuenf Suchschleifen in sfight_enemy_turn sind identisch gebaut -> Makro SUCHE (eigene Setter-Kopie je Expansion).
 * - sfight_seq_end: der Block "if (flag)" steht INNERHALB von "if (txo)" (in sfight_seq_main/sfight_seq_intro dahinter) - nur am
 *   Sprungziel des jmp near erkennbar.
 * Originalfehler/Auffaelliges: sfight_event_text liefert s uninitialisiert fuer alle Ereignisse ohne Text (Aufrufer nutzt s
 *   nur bei ds:8b76); "flag" wird in sfight_seq_main/sfight_seq_intro/sfight_seq_end nie zurueckgesetzt - wechselt ds:253b waehrend der
 *   Sequenz, wird das alte Textobjekt erneut geloescht und p->raw/txt nochmals freigegeben.
 */
#include <mem.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:0fa6..118c, dsana) - initialisierte Daten in Adressreihenfolge */
signed short g_finster_anger_stage = 0;
struct sfight_frames g_sfight_frames = { { 78, 102, 104, 106, 107, 103, 105 } };
struct frame_events8 g_sfight_intro_events = { { { 0x00, 1 }, { 0x07, 2 }, { 0x09, 3 }, { 0x0b, 4 }, { 0x0c, 5 }, { 0x10, 6 }, { 0x11, 7 }, { 0x00, 8 } } };
struct frame_events6 g_sfight_end_events = { { { 0x00, 1 }, { 0x01, 2 }, { 0x02, 3 }, { 0x03, 4 }, { 0x04, 5 }, { 0x00, 6 } } };
char g_unused_0fd2[2] = { 0xff, 0xff };   /* ds:0fd2, unbenutzt */
struct point g_sfight_targets_b[3] = { { 2, 10 }, { 2, 9 }, { -1, -1 } };
struct point g_sfight_targets_c[3] = { { 6, 17 }, { 7, 17 }, { -1, -1 } };
struct point g_sfight_targets_d[3] = { { 6, 17 }, { 7, 17 }, { -1, -1 } };
struct point g_sfight_targets_a[22] = {
	{ 1, 9 },
	{ 2, 9 },
	{ 3, 9 },
	{ 3, 8 },
	{ 4, 8 },
	{ 5, 8 },
	{ 6, 8 },
	{ 7, 8 },
	{ 7, 9 },
	{ 8, 8 },
	{ 9, 8 },
	{ 9, 9 },
	{ 9, 10 },
	{ 9, 11 },
	{ 9, 12 },
	{ 8, 12 },
	{ 9, 13 },
	{ 9, 14 },
	{ 9, 15 },
	{ 8, 16 },
	{ 9, 16 },
	{ -1, -1 }
};
struct point g_sfight_targets_far[79] = {
	{ 7, 0 },
	{ 7, 1 },
	{ 7, 2 },
	{ 7, 3 },
	{ 7, 4 },
	{ 7, 5 },
	{ 7, 6 },
	{ 7, 7 },
	{ 8, 0 },
	{ 8, 1 },
	{ 8, 2 },
	{ 8, 3 },
	{ 8, 4 },
	{ 8, 5 },
	{ 8, 6 },
	{ 8, 7 },
	{ 9, 0 },
	{ 9, 1 },
	{ 9, 2 },
	{ 9, 3 },
	{ 9, 4 },
	{ 9, 5 },
	{ 9, 6 },
	{ 9, 7 },
	{ 11, 11 },
	{ 10, 12 },
	{ 10, 5 },
	{ 11, 4 },
	{ 12, 3 },
	{ 13, 2 },
	{ 14, 1 },
	{ 15, 0 },
	{ 10, 6 },
	{ 11, 5 },
	{ 12, 4 },
	{ 13, 3 },
	{ 14, 2 },
	{ 15, 1 },
	{ 16, 0 },
	{ 10, 7 },
	{ 11, 6 },
	{ 12, 5 },
	{ 13, 4 },
	{ 14, 3 },
	{ 15, 2 },
	{ 16, 1 },
	{ 10, 8 },
	{ 11, 7 },
	{ 12, 6 },
	{ 13, 5 },
	{ 14, 4 },
	{ 15, 3 },
	{ 16, 2 },
	{ 10, 9 },
	{ 11, 8 },
	{ 12, 7 },
	{ 13, 6 },
	{ 14, 5 },
	{ 15, 4 },
	{ 16, 3 },
	{ 11, 9 },
	{ 12, 8 },
	{ 13, 7 },
	{ 14, 6 },
	{ 15, 5 },
	{ 16, 4 },
	{ 17, 3 },
	{ 10, 10 },
	{ 11, 10 },
	{ 12, 10 },
	{ 13, 10 },
	{ 14, 10 },
	{ 15, 10 },
	{ 16, 10 },
	{ 17, 10 },
	{ 18, 10 },
	{ 19, 10 },
	{ 20, 10 },
	{ -1, -1 }
};
/* Phase B: eigenes BSS (ds:8b76..8b7a) in Adressreihenfolge */
signed short g_sfight_show_text;
signed short g_sfight_hide_text;

char far *sfight_event_text(Fight far *p, signed short n, signed short far *snd)
{
#if FIX_0167
	/* Fix #167: s vorbelegen (Ereignisse ohne Text) */
	char far *s = 0;

	switch (n) {
#else
	char far *s;

	switch (n) {
#endif
	case 1:
	case 2:
		*snd = 10;
		break;
	case 3:
	case 7:
		*snd = 6;
		break;
	case 4:
	case 5:
	case 6:
		*snd = 7;
		break;
	case 12:
		*snd = 8;
		break;
	case 15:
		*snd = 9;
		break;
	case 8:
		g_sfight_show_text = 1;
		s = p->txt[0];
		*snd = 2;
		break;
	case 10:
		g_sfight_show_text = 1;
		s = p->txt[1];
		*snd = 3;
		break;
	case 13:
		g_sfight_show_text = 1;
		s = p->txt[2];
		*snd = 4;
		break;
	case 9:
	case 11:
	case 14:
		g_sfight_hide_text = 1;
	default:
		break;
	}
	return s;
}

char far *sfight_event_text2(Fight far *p, signed short n, signed short far *snd)
{
	char far *s = 0;

	switch (n) {
	case 1:
		g_sfight_show_text = 1;
		s = p->txt[3];
		*snd = 0x19;
		break;
	case 3:
		g_sfight_show_text = 1;
		s = p->txt[4];
		*snd = 0x1a;
		break;
	case 5:
		g_sfight_show_text = 1;
		s = p->txt[5];
		*snd = 0x1b;
		break;
	case 7:
		g_sfight_show_text = 1;
		s = p->txt[6];
		*snd = 0x1c;
		break;
	case 2:
	case 4:
	case 6:
	case 8:
		g_sfight_hide_text = 1;
	default:
		break;
	}
	return s;
}

void sfight_seq_main(Fight far *p)
{
	signed short i;
	signed short w;
	signed short h;
	signed short last;
	signed short snd = 0;
	char far *s = 0;
	void far *mem = 0;
	struct fig_node far *sl;
	FigAnim far *an;
	Bmp b;
	char far *txo = 0;
	void far *sr;
	char far * far *st;
	signed short flag = 0;
	struct sfight_frames tab = g_sfight_frames;

	g_sfight_show_text = g_sfight_hide_text = 0;
	if (g_speech_on == 1 && speech_active(g_snd_speech)) {
		snd_speech(g_snd_speech, 5);
		wait_ticks(250);
		snd_speech(g_snd_speech, 11);
		wait_ticks(50);
		snd_speech(g_snd_speech, 11);
		wait_ticks(30);
	}
	fig_list_show(&p->list, g_sfight_fig3);
	sl = fig_list_find(&p->list, g_sfight_fig3);
	g_sfight_seq_on = 1;
	for (i = 0; i < 7; i++) {
		if (i == 0) {
			g_sfight_ofs_x = 0x74;
			g_sfight_ofs_y = 0;
		} else {
			g_sfight_ofs_x = 0xaf;
			g_sfight_ofs_y = 5;
		}
		if (i == 1) {
			g_sfight_pos = 0x3b;
			sfight_enemy_reset(p, &p->seq);
			fig_list_hide(&p->list, g_sfight_fig2);
			fig_list_show(&p->list, g_sfight_fig3);
		}
		memset(&p->seq, 0, 0xa2);
		p->seq.figure = 0x9f;
		p->seq.step[0].frame = tab.frame[i];
		p->seq.step[0].type = 0;
		p->seq.step[0].count = 1;
		p->seq.step[1].frame = -1;
		seq_set(sl->anim, &p->seq);
		sl->anim->delay_fix = 10;
		last = 0;
		do {
			game_idle();
			if (g_sfight_event && last != g_sfight_event) {
				last = g_sfight_event;
				if (g_subtitles == 1) {
					sr = p->txt_buf;
					st = p->txt;
					txo = (char far *)Font_ctor(0, 9);
					txt_load_list(txo, 0x23, 10, &p->txt_buf, &p->txt);
					flag = 1;
				}
				s = sfight_event_text(p, g_sfight_event, &snd);
				if (txo) {
					if (g_sfight_show_text == 1) {
						g_sfight_show_text = 0;
						w = txt_max_width(txo, s);
						h = font_wrap(txo, s) * font_height(txo) + 1;
						mem = mem_alloc(w * h);
						b.init(w, h, mem);
						b.get(g_vga_buf, 10, 10);
						font_set_colors(txo, 0xff, -1);
						font_push_colset(txo, 0xe2, 0xba, 0xff, 0xff, 0xff);
						font_print(txo, s, 10, 10);
						font_pop_colset(txo);
						font_restore_colors(txo);
					}
					if (g_sfight_hide_text == 1) {
						g_sfight_hide_text = 0;
						b.put(g_vga_buf, 10, 10);
						b.free_lines();
						mem = mem_free(mem);
					}
				}
				if (flag) {
					Font_dtor(txo, 3);
					mem_free(p->txt_buf);
					mem_free(p->txt);
					p->txt_buf = sr;
#if FIX_0167
					/* Fix #167: Merker und Zeiger nach dem Freigeben zuruecksetzen (kein zweites Freigeben) */
					p->txt = st;
					flag = 0;
					txo = 0;
#else
					p->txt = st;
#endif
				}
				if (g_speech_on == 1 && g_cd_speech == 1 && snd) {
					snd_speech(g_snd_speech, snd);
					snd = 0;
				}
			}
		} while (!seq_done(sl->anim));
	}
	g_sfight_seq_on = 0;
	sfight_enemy_reset(p, &p->seq);
	an = sfight_enemy_move(p, &p->seq, 4);
	do
		game_idle();
	while (!seq_done(an));
	fig_list_hide(&p->list, g_sfight_fig3);
}

void sfight_seq_intro(Fight far *p)
{
	signed short i;
	unsigned char cur;
	struct fig_node far *sl;
	signed short code;
	signed short snd = 0;
	char far *s = 0;
	void far *mem = 0;
	Bmp b;
	signed short w;
	signed short h;
	char far *txo = 0;
	void far *sr;
	char far * far *st;
	signed short flag = 0;
	signed short v = 0x15;
	struct frame_events8 tab;

	g_sfight_intro_events.e[7].frame = v;
	tab = g_sfight_intro_events;
	g_sfight_show_text = g_sfight_hide_text = 0;
	g_sfight_seq_on = 1;
	fig_list_show(&p->list, g_sfight_fig3);
	sl = fig_list_find(&p->list, g_sfight_fig3);
	memset(&p->seq, 0, 0xa2);
	p->seq.figure = 0x9f;
	p->seq.step[0].frame = 0x6e;
	p->seq.step[0].type = 0;
	p->seq.step[0].count = 1;
	p->seq.step[1].frame = -1;
	seq_set(sl->anim, &p->seq);
	sl->anim->delay_fix = 10;
	g_sfight_ofs_x = 0xa5;
	g_sfight_ofs_y = 0x5f;
	do
		game_idle();
	while (!seq_done(sl->anim));
	memset(&p->seq, 0, 0xa2);
	p->seq.figure = 0x9f;
	p->seq.step[0].frame = 0x6f;
	p->seq.step[0].type = 3;
	p->seq.step[0].count = v;
	p->seq.step[1].frame = -1;
	seq_set(sl->anim, &p->seq);
	sl->anim->delay_fix = 10;
	g_sfight_ofs_x = 0xa5;
	g_sfight_ofs_y = 0x5f;
	i = 0;
	do {
		game_idle();
		code = 0;
		cur = sl->anim->loops;
		if (tab.e[i].frame <= cur) {
			code = tab.e[i].code;
			i++;
		}
		if (code) {
			if (g_subtitles == 1) {
				sr = p->txt_buf;
				st = p->txt;
				txo = (char far *)Font_ctor(0, 9);
				txt_load_list(txo, 0x23, 10, &p->txt_buf, &p->txt);
				flag = 1;
			}
			s = sfight_event_text2(p, code, &snd);
			if (txo) {
				if (g_sfight_show_text == 1) {
					g_sfight_show_text = 0;
					w = txt_max_width(txo, s);
					h = font_wrap(txo, s) * font_height(txo) + 1;
					mem = mem_alloc(w * h);
					b.init(w, h, mem);
					b.get(g_vga_buf, 10, 10);
					font_set_colors(txo, 0xff, -1);
					font_push_colset(txo, 0xe2, 0xba, 0xff, 0xff, 0xff);
					font_print(txo, s, 10, 10);
					font_pop_colset(txo);
					font_restore_colors(txo);
				}
				if (g_sfight_hide_text == 1) {
					g_sfight_hide_text = 0;
					b.put(g_vga_buf, 10, 10);
					b.free_lines();
					mem = mem_free(mem);
				}
			}
			if (flag) {
				Font_dtor(txo, 3);
				mem_free(p->txt_buf);
				mem_free(p->txt);
				p->txt_buf = sr;
#if FIX_0167
				/* Fix #167: Merker und Zeiger nach dem Freigeben zuruecksetzen (kein zweites Freigeben) */
				p->txt = st;
				flag = 0;
				txo = 0;
#else
				p->txt = st;
#endif
			}
			if (g_speech_on == 1 && g_cd_speech == 1 && snd) {
				snd_speech(g_snd_speech, snd);
				snd = 0;
			}
		}
	} while (!seq_done(sl->anim));
	memset(&p->seq, 0, 0xa2);
	p->seq.figure = 0x9f;
	p->seq.step[0].frame = 0x70;
	p->seq.step[0].type = 0;
	p->seq.step[0].count = 1;
	p->seq.step[1].frame = -1;
	seq_set(sl->anim, &p->seq);
	sl->anim->delay_fix = 10;
	g_sfight_ofs_x = 0xa5;
	g_sfight_ofs_y = 0x25;
	while (snd_speech_busy(g_snd_speech) == 1) {
		snd_service(g_snd_music);
		snd_service(g_snd_fx);
		snd_speech_service(g_snd_speech);
		wait_for_vsync();
	}
	snd_play(g_snd_fx, 0x87);
	do
		game_idle();
	while (!seq_done(sl->anim));
	fig_list_hide(&p->list, g_sfight_fig3);
	g_sfight_seq_on = 0;
}

void sfight_intro(Fight far *p)
{
	void far *buf;
	register signed short s = 1;

	if (g_sfight_seen)
		s = 0;
	if (s) {
		sfight_seq_intro(p);
		g_sfight_phase = 1;
		sfight_seq_main(p);
		Sound_volume2(g_snd_music, g_vol_music, 0);
	} else
		g_sfight_phase = 1;
	buf = mem_alloc(0x300L);
	nvfile_pal(g_nvf_heads, 0x372, buf, 0x100);
	set_palette(buf, 0, 0x10);
	mem_free(buf);
	fig_stand_seq(p, g_enemies, &p->seq);
	g_fig_redraw_all = 1;
}

void sfight_seq_end(Fight far *p)
{
	struct fig_node far *sl;
	FigAnim far *an;
	signed short i;
	unsigned char cur;
	void far *sr;
	char far * far *st;
	signed short code;
	signed short snd = 0;
	char far *txo = 0;
	signed short flag = 0;
	char far *s = 0;
	void far *mem = 0;
	Bmp b;
	signed short w;
	signed short h;
	signed short v = 5;
	struct hero far *hp;
	struct frame_events6 tab;

	g_sfight_end_events.e[5].frame = v;
	tab = g_sfight_end_events;
	g_sfight_show_text = g_sfight_hide_text = 0;
	if (g_sfight_pos != 0x2d) {
		an = sfight_enemy_move(p, &p->seq, 0x2d);
		do
			game_idle();
		while (!seq_done(an));
	}
	fig_list_hide(&p->list, g_sfight_fig2);
	fig_list_show(&p->list, g_sfight_fig3);
	sl = fig_list_find(&p->list, g_sfight_fig3);
	memset(&p->seq, 0, 0xa2);
	p->seq.figure = 0x9f;
	p->seq.step[0].frame = 0x71;
	p->seq.step[0].type = 3;
	p->seq.step[0].count = v;
	p->seq.step[1].frame = -1;
	seq_set(sl->anim, &p->seq);
	sl->anim->delay_fix = 10;
	g_sfight_ofs_x = 0xa0;
	g_sfight_ofs_y = -5;
	i = 0;
	do {
		game_idle();
		code = 0;
		cur = sl->anim->loops;
		if (tab.e[i].frame <= cur) {
			code = tab.e[i].code;
			i++;
		}
		if (code) {
			s = 0;
			if (g_subtitles == 1) {
				sr = p->txt_buf;
				st = p->txt;
				txo = (char far *)Font_ctor(0, 9);
				txt_load_list(txo, 0x23, 10, &p->txt_buf, &p->txt);
				flag = 1;
			}
			switch (code) {
			case 1:
				g_sfight_show_text = 1;
				s = p->txt[7];
				snd = 0x1d;
				break;
			case 3:
				g_sfight_show_text = 1;
				s = p->txt[8];
				snd = 0x1e;
				break;
			case 5:
				g_sfight_show_text = 1;
				s = p->txt[9];
				snd = 0xf0;
				break;
			case 2:
			case 4:
			case 6:
				g_sfight_hide_text = 1;
			default:
				break;
			}
			if (txo) {
				if (g_sfight_show_text == 1) {
					g_sfight_show_text = 0;
					w = txt_max_width(txo, s);
					h = font_wrap(txo, s) * font_height(txo) + 1;
					mem = mem_alloc(w * h);
					b.init(w, h, mem);
					b.get(g_vga_buf, 10, 100);
					font_set_colors(txo, 0xff, -1);
					font_push_colset(txo, 0xe2, 0xba, 0xff, 0xff, 0xff);
					font_print(txo, s, 10, 100);
					font_pop_colset(txo);
					font_restore_colors(txo);
				}
				if (g_sfight_hide_text == 1) {
					g_sfight_hide_text = 0;
					b.put(g_vga_buf, 10, 100);
					b.free_lines();
					mem = mem_free(mem);
				}
				if (flag) {
					Font_dtor(txo, 3);
					mem_free(p->txt_buf);
					mem_free(p->txt);
					p->txt_buf = sr;
#if FIX_0167
					/* Fix #167: Merker und Zeiger nach dem Freigeben zuruecksetzen (kein zweites Freigeben) */
					p->txt = st;
					flag = 0;
					txo = 0;
#else
					p->txt = st;
#endif
				}
			}
			if (g_speech_on == 1 && g_cd_speech == 1 && snd) {
				snd_speech(g_snd_speech, snd);
				snd = 0;
			}
		}
	} while (!seq_done(sl->anim));
	an = sfight_enemy_end_seq(p, &p->seq);
	do
		game_idle();
	while (!seq_done(an));
	fig_list_hide(&p->list, g_sfight_fig3);
	hp = group_get_npc(g_group);
	hp->petrified = 0;
}

signed short sfight_hit_chance(struct hero far *h, register signed short v)
{
	if (group_mr_bonus(g_group) == -5)
		v -= 2;
	if (h->parried == 1)
		v += 2;
	if ((unsigned char)h->chamaelioni == 1)
		v -= 3;
	if (h->duplicatus == 1)
		v /= 2;
	if (h->duplicatus == 2)
		v /= 3;
	return v;
}

#define SUCHE(tb, ch, cp, extra) \
	if (!found) { \
		tab = tb; \
		for (i = 0; !found && tab[i].x != -1; i++) { \
			c = *(g_fight_walkmap + tab[i].y * FIG_MAP_PITCH + tab[i].x); \
			if (c > 0 && c <= 10) { \
				h = &g_heroes[c - 1]; \
				if (check_hero2(h) && random_schweif(100) < sfight_hit_chance(h, ch)) { \
					found = 1; \
					e->set_enemy_id(c); \
					r = cp; \
					extra \
				} \
			} \
		} \
	}

signed short sfight_enemy_turn(Fight far *p)
{
	struct enemy far *e;
	signed short r = 12;
	register signed short found = 0;
	unsigned char c;
	struct hero far *h;
	struct point far *tab;
	register signed short i;

	e = g_enemies;
	if (p->mark_id != -1) {
		fig_list_remove(&p->list, p->mark_id, 0);
		p->mark_id = -1;
	}
	if (e->le_flee >= e->le) {
		g_sfight_won = 1;
		sfight_seq_end(p);
		e->dead = 1;
		found = 1;
	} else {
		e->enemy_id = -1;
		e->action_id = FIG_ACTION_WAIT;
		SUCHE(g_sfight_targets_a, 0x1e, 2, e->cur_spell = 3;)
		SUCHE(g_sfight_targets_b, 0x0a, 2, e->cur_spell = 1;)
		SUCHE(g_sfight_targets_c, 0x0a, 2, e->cur_spell = 2;)
		SUCHE(g_sfight_targets_d, 0x1e, 2, e->cur_spell = 0;)
		SUCHE(g_sfight_targets_far, 0x28, 15, ;)
		if (!found && random_schweif(100) < 20)
			r = FIG_ACTION_SPELL;
	}
	e->set_action_id(r);
	return r;
}
