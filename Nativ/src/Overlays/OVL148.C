/* OVL148 - Overlay O148 (Segment 0x2250 in SCHWEIF.EXE): Kampf-/Anzeigehilfen. fig_obj_transparent(f, v): 1, wenn
 * is_in_word_array(v, g_fig_transparent_objs) != -1. fig_set_mark(f, id): Markierung unter die Figur id
 * (fig_list_getpos -> tmpl.cbx/cby, Vorlage tmpl mit gfx = pic_mark, f->mark_id = fig_list_add(&f->list, -1)).
 * fig_choose_next_enemy: zufaelligen Gegner aus g_enemies (random_schweif(g_nr_enemies) - 1) suchen, bis mon_id
 * und attacks_left gesetzt sind (ggf. game_idle). fig_next_side(f, flag, res, x, y): Verhaeltnis-/Umschaltlogik
 * (flag ^= 1, g_fight_end == 4, res = x / y bzw. y / x). fig_enemies_le_sum/fig_enemies_ae_sum: Summe le bzw. ae
 * der 20 Gegner, fuer die fight_check_state gilt.
 * fig_draw_hero_bar(f): Heldenleiste neu zeichnen (Ring get_first_hero/get_next_hero): Portrait (portrait_file/
 * load_portrait_bmp, gb_scale), Zustandsbild nach check_hero/Zustandsbits/Waffe (pic_dead, pic_unable,
 * pic_fleeing, pic_unarmed), LE/AE-Balken Bars_draw(g_bars, ...), do_border, Ausgabe per gb_save/gb_blit nach
 * g_vga_buf; danach zwei Summenbalken (200 * a / enemy_le_sum, 200 * b / enemy_ae_sum, begrenzt 1..200) mit
 * do_h_line/do_v_line. fight_msg_names(f, h1, h2): Meldung f->txt[74] bzw. [73] mit den Namen der Helden mit
 * dead per sprintf in einen 100-Byte-Puffer, ausgegeben ueber fight_status_text (wenn all_heroes_down) bzw.
 * dlg_message(g_dialog).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL148.C   -> 2530/2530 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - -Og noetig (tote Epilogkopien in fig_obj_transparent/fig_choose_next_enemy/fig_enemies_*_sum,
 *   Epilogkopien in fig_next_side).
 * - Helden-Statusbits als unsigned short b:1: "!(signed char)h->dead" -> and/cbw/or ax,ax,
 *   "(unsigned char)h->dead" -> or al,al, direkt (inv[3].broken, g_items[w].weapon) -> or ax,ax.
 * - fig_draw_hero_bar: "register w = h->inv[3].item_id" im innersten else-Block (ohne Aufrufe) landet in CX (SI/DI belegt).
 * - "l = 200; l *= a; l /= f->enemy_le_sum;" (long l, int a) -> movsx eax / mov edx,l / imul edx,eax bzw.
 *   movsx ebx / mov eax,l / cdq / idiv ebx; long an int-Parameter -> push word [l].
 * - Ketten "if (..) {..} else if" mit innerem if/else -> jmp-auf-jmp (L_0351) entsteht auch mit -Og.
 * - Stolperstein: Tabelle head_no (+16f) hat 7 Eintraege (nicht 8), sonst shown_status (+17d) um 2 verschoben.
 */
#include <stdio.h>
#include "schweif.h"

signed short fig_obj_transparent(FIGHT_PTR f, signed short v)
{
	if (is_in_word_array(v, g_fig_transparent_objs) == -1)
		return 0;
	else
		return 1;
}

void fig_set_mark(FIGHT_PTR f, signed char id)
{
	signed short a;
	signed short b;

	fig_list_getpos(&f->list, id, &a, &b);
	f->tmpl.obj_pic = 0;
	f->tmpl.cbx = a;
	f->tmpl.cby = b;
	f->tmpl.offsetx = 0;
	f->tmpl.offsety = 6;
	f->tmpl.kind = 0;
	f->tmpl.anim = 0;
	f->tmpl.gfx = f->pic_mark;
	f->tmpl.z = 1;
	f->tmpl.visible = 1;
	f->tmpl.twofielded = -1;
	f->mark_id = fig_list_add(&f->list, -1);
}

struct enemy far *fig_choose_next_enemy(FIGHT_PTR f)
{
	struct enemy far *p;
	register signed short i;

	do {
		i = random_schweif(g_nr_enemies) - 1;
		p = &g_enemies[i];
		if (g_quick_fight != 0)
			game_idle();
	} while (p->mon_id == 0 || p->attacks_left == 0);
	return p;
}

void fig_next_side(FIGHT_PTR f, signed char far *flag, signed short far *res, register signed short x, register signed short y)
{
	*flag ^= 1;
	if (g_fight_end == 4)
		*flag = 1;
	if (!*flag) {
		if (x <= y)
			*res = 1;
		else if (x == 0)
			*flag = 1;
		else if (y != 0)
			*res = x / y;
		else
			*res = x;
	}
	if (*flag == 1) {
		if (y == 0) {
			*flag = 0;
			*res = 1;
		} else if (y <= x)
			*res = 1;
		else if (x != 0)
			*res = y / x;
		else
			*res = y;
	}
}

signed short fig_enemies_le_sum(FIGHT_PTR f)
{
	struct enemy far *p;
	register signed short i;
	register signed short n;

	n = 0;
	for (i = 0; i < 20; i++) {
		p = &g_enemies[i];
		if (fight_check_state(p))
			n += p->le;
	}
	return n;
}

signed short fig_enemies_ae_sum(FIGHT_PTR f)
{
	struct enemy far *p;
	register signed short i;
	register signed short n;

	n = 0;
	for (i = 0; i < 20; i++) {
		p = &g_enemies[i];
		if (fight_check_state(p))
			n += p->ae;
	}
	return n;
}

void fig_draw_hero_bar(FIGHT_PTR f)
{
	struct hero far *h;
	signed short x;
	signed short y;
	signed short a;
	signed short b;
	signed long l;
	void far *img;
	register signed short i;
	register signed short v;

	h = get_first_hero(g_group);
	i = 0;
	do {
		v = portrait_file(h);
		if (f->head_no[i] != v) {
			f->head_no[i] = v;
			load_portrait_bmp(h, f->tmp32);
			gb_scale(f->tmp32, f->heads[i]);
		}
		img = 0;
		if (!check_hero(h) && !(signed char)h->dead) {
			if ((unsigned char)h->scared)
				img = f->pic_fleeing;
			else
				img = f->pic_unable;
		} else if ((unsigned char)h->dead)
			img = f->pic_dead;
		else if (h->blind != 0)
			img = f->pic_unable;
		else {
			register signed short w;

			w = h->inv[INV_SLOT_RIGHT_HAND].item_id;
			if (!w || !g_items[w].weapon || h->inv[INV_SLOT_RIGHT_HAND].broken)
				img = f->pic_unarmed;
		}
		x = i * 0x1c + 0x3f;
		y = 6;
		if (f->shown_status[i] != img || h->le != f->shown_hero_le[i] || h->ae != f->shown_hero_ae[i]) {
			gb_blit(f->bar_bg[i], f->buf, 0, 0);
			gb_blit(f->heads[i], f->buf, 1, 0x14);
			Bars_draw(g_bars, 0, i, h->le, h->le_max, 2, f->buf);
			f->shown_hero_le[i] = h->le;
			Bars_draw(g_bars, 1, i, (unsigned char)h->dead ? 0 : h->ae, h->ae_max, 2, f->buf);
			f->shown_hero_ae[i] = h->ae;
			if (img)
				gb_blit(img, f->buf, 1, 1);
			do_border(f->buf, 0, 0x13, 0x12, 0x12, 0xcd);
			if (f->shown_status[i] != img && img)
				do_border(f->buf, 0, 0, 0x12, 0x12, 0xcd);
			gb_save(f->bar_tmp, f->buf, 0, 0);
			gb_blit(f->bar_tmp, g_vga_buf, x, y);
			f->shown_status[i] = img;
		}
		h = get_next_hero(g_group, h);
		i++;
	} while (get_first_hero(g_group) != h);

	a = fig_enemies_le_sum(f);
	b = fig_enemies_ae_sum(f);
	if (f->shown_le_max != f->enemy_le_sum || f->shown_ae_max != f->enemy_ae_sum || f->shown_le != a || f->shown_ae != b) {
		f->shown_le = a;
		f->shown_ae = b;
		f->shown_le_max = f->enemy_le_sum;
		f->shown_ae_max = f->enemy_ae_sum;
		gb_blit(f->status_bg, f->buf, 0, 0);
		if (a != 0) {
			l = 200;
			l *= a;
			l /= f->enemy_le_sum;
			if (l < 1)
				l = 1;
			else if (l > 200)
				l = 200;
			for (i = 0; i < 6; i++)
				do_h_line(f->buf, 0, i, l, 0xfc);
			do_v_line(f->buf, l, 0, 6, 0xd6);
		}
		if (b != 0) {
			l = 200;
			l *= b;
			l /= f->enemy_ae_sum;
			if (l < 1)
				l = 1;
			else if (l > 200)
				l = 200;
			for (i = 0; i < 6; i++)
				do_h_line(f->buf, 0, i + 9, l, 0xfd);
			do_v_line(f->buf, l, 9, 6, 0xbb);
		}
		gb_save(f->sums_tmp, f->buf, 0, 0);
		gb_blit(f->sums_tmp, g_vga_buf, 0x40, 0xa0);
	}
}

void fight_msg_names(FIGHT_PTR f, struct hero far *h1, struct hero far *h2)
{
	char far *buf;

	buf = mem_alloc(100);
	if (all_heroes_down(g_group)) {
		if ((unsigned char)h1->dead) {
			sprintf(buf, f->txt[74], h1->name);
			fight_status_text(buf);
		}
		if (h2 && (unsigned char)h2->dead) {
			sprintf(buf, f->txt[74], h2->name);
			fight_status_text(buf);
		}
	} else {
		if ((unsigned char)h1->dead) {
			sprintf(buf, f->txt[73], h1->name);
			dlg_message(g_dialog, buf);
		}
		if (h2 && (unsigned char)h2->dead) {
			sprintf(buf, f->txt[73], h2->name);
			dlg_message(g_dialog, buf);
		}
	}
	mem_free(buf);
}
