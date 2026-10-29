/* OVL144 - Overlay O144 (Segment 0x223a in SCHWEIF.EXE): Bildaufbau des Kampfbildschirms mit Neuzeichnen nur der
 * geaenderten Rechtecke. Rechteckliste ds:c99f (max. 20 x {x0, y0, x1, y1, f}, Zaehler ds:ca67, Ende x0 == -1; bei
 * 19 Eintraegen bzw. ds:ca6a = 1 "alles neu" = Vollbild), Objektliste der Kampfstruktur f->tab (+0x3e0, 255 x 11
 * Bytes {id, x0, y0, x1, y1, v}, Ende 0xff).
 * fight_draw(f): ein Bild - fight_restore_bg, fight_draw_figures, fight_printer(ds:c53a) -> Rueckgabe, fight_draw_flush (bei ds:c952 nur R01_036B).
 * fight_restore_bg(f): Rechteckliste leeren (bei ds:ca68 Vollbild), XMS-Kopie ds:c4db (0xfa00 Bytes, Handle ds:394f -> Puffer
 * *ds:b668 bzw. f->buf je nach ds:c6c6 > 50) per R02_0EDA (ds:0f34 == 1) bzw. R38_0004.
 * fight_draw_hero_box/fight_draw_enemy_box(f, seite, h): Portraet/Name/LE-AE-Balken (O057_0069) eines Helden bzw. Gegners (O192) oben links
 * (seite 0) oder unten (seite 1), R20-Textzustand sichern/zurueck. fight_rect_overlap: Ueberlappung zweier Rechtecke.
 * fight_draw_flush(f): Rechtecke gegen die drei Anzeigebereiche (ds:ca70/72/74) testen, Anzeigen f->f203/f207 bzw. f20b/f20f
 * und Textzeile *ds:ca6c neu zeichnen, dann alle Rechtecke aus f->buf auf den Bildschirm (Bmp R42_054C mit Modus 6
 * bei ds:b5ba / R42_0517, Maus R18_047B/001A/0009). fight_add_rect: Rechteck (geklemmt auf 320x200) anhaengen.
 * fight_add_obj_rect(f, alt, id, rect, flag): Rechteck eines Objekts mit seiner alten Lage (Liste alt) vereinigen.
 * fight_draw_figures(f): alle Figuren der Kampfliste f->f35 zeichnen (Bilder +11 per R42_04DF, animierte Objekte +0d per
 * O112_0267 mit Blickrichtung aus O192_04AC +2f; Sonderkaempfe 0x493 (O141_11A4, ds:6882) und 0x495 (ds:ca7a/7c)),
 * f->tab neu fuellen und mit der alten Liste abgleichen. fight_anim_tick(f): bei laufender Animation (O112_015B) fight_draw
 * (bzw. O148_026A), gegen Wiedereintritt ds:6be6. fight_recolor(p, n, c): Pixel != 0 einfaerben. fight_printer(o): Laufschrift-
 * /Meldungsleiste des Kampfes (Eintraege 14 Bytes, Bild 0x7e/0x7f, Text o->fe6[id] mit Namen per sprintf,
 * einmalige Klaenge R04_06FA je Ereignis 6/7/8/11/13..16). all_heroes_down(tab): 0, wenn ein Held (tab[0..6]) noch
 * handlungsfaehig ist (R02_0342), sonst 1. fight_check_keys(f): Taste ds:cb22 (R32_000F gegen ds:6be8) -> f->f3d9[k] = 1.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL144.CPP   -> 6863/6863 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h, Welle N7: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * - -Oi fuer memset/memcpy (rep stosw/movsw), -Og fuer Epilogkopien und die vielen Zwischenwerte (n*11 und
 *   Zeiger-Offset von f->tab[n] in fight_draw_figures, &rect[i] in fight_draw_flush, &o->e[o->cur] in DI in fight_printer) - ohne -Og oder
 *   -Oi jeweils tausende Abweichungen.
 * - C++ wegen Bmp (lokal in fight_draw_hero_box/fight_draw_enemy_box/fight_draw_flush/fight_printer, Methoden ueber Objektzeiger cur->f11 in fight_draw_figures).
 * - fight_restore_bg: Zielfeld der XMS-Struktur als unsigned long und "c ? (unsigned long)a : (unsigned long)b" (mov eax);
 *   memset(g_c99f, -1, 200) -> mov ax,0ffffh / mov ah,al.
 * - fight_add_obj_rect: "if (x0 != .. || x1 != .. || ..) fight_add_rect(..)"; die Form "if (A && B ..) ; else fight_add_rect(..)" ergibt
 *   eine Epilogkopie und andere Spruenge.
 * - Bitfelder (unsigned short b:1): "(signed char)h->bbc ? 0 : h->f76" -> and ax,1 / or al,al;
 *   "!(signed char)p2->b38" -> and ax,1 / cbw / or ax,ax.
 * - fight_printer: "if (--o->cnt <= 0)" -> dec word es:[..] / jg (Flags des dec). Blocklocals der Faelle 7 bzw. 8/13/14
 *   teilen [bp-20h]; im zweiten Block "register" fuer s (SI).
 * Originalfehler/Auffaelliges: fight_printer prueft im Zweig fuer die Ereignisse 8/13/14 noch "id == 11" (dort nie
 *   wahr, die Klaenge 0x51/0x50/0xa9/0xa6 sind unerreichbar). fight_draw_hero_box/fight_draw_enemy_box lassen R20_03CE drei Ausgaben in
 *   dieselbe Variable schreiben. fight_rect_overlap/fight_add_obj_rect: Parameter f unbenutzt. fight_draw_figures: "n++" hinter dem Endmarker ohne
 *   Wirkung, R07_0002-Ergebnis ungeprueft.
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <mem.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:6be6..6c16, dsana) - initialisierte Daten in Adressreihenfolge */
signed short g_fig_anim_busy = 0;
struct fig_keys g_fig_keys = { { 561, 818, 1075, 1332, 1589, 1846, -1 } };
struct dirs4 g_fig_dirs_back = { { { -1, 0 }, { 0, 1 }, { 1, 0 }, { 0, -1 } } };	/* Gegenrichtung zu Richtung 0..3 */
struct dirs4 g_fig_dirs_search = { { { 1, 0 }, { 0, -1 }, { -1, 0 }, { 0, 1 } } };	/* Kampffeld-Richtung 0: x+1, 1: y-1, 2: x-1, 3: y+1 */

/* font_set_colors mit Wort-Parametern aufrufen (Original: push word; Prototyp hat unsigned char) */
#define font_set_colors_w ((void (*)(void far *, signed short, signed short))font_set_colors)
#define MSG	o->e[o->cur]		/* angezeigte Meldung */
#define HERO2	((struct hero far *)MSG.obj2)
#define ENEMY2	((struct enemy far *)MSG.obj2)

signed short fight_draw(Fight far *f)
{
	register signed short busy = 0;

	if (!g_quick_fight) {
		fight_restore_bg(f);
		fight_draw_figures(f);
		busy = fight_printer((struct fight_msgs far *)g_fight_list);
		fight_draw_flush(f);
	} else
		game_idle();
	return busy;
}

void fight_restore_bg(Fight far *f)
{
	if (!g_quick_fight) {
		g_fig_nr_dirty = 0;
		memset(g_fig_dirty, -1, 200);
		g_fig_dirty_full = 0;
		if (g_fig_redraw_all != 0) {
			fight_add_rect(0, 0, 0x13f, 0xc7);
			g_fig_dirty_full = 1;
		}
		/* Hintergrund (Kampffeld ohne Figuren) aus dem XMS/EMS zurueck */
		g_xms_move.len = 0xfa00L;
		g_xms_move.dst_handle = 0;
		g_xms_move.dst = g_c6c6 > 50 ? (unsigned long)g_vga_buf : (unsigned long)f->buf;
		g_xms_move.src_handle = g_fight_xms;
		g_xms_move.src = 0;
		if (g_fight_use_ems == 1) {
			EMS_copy(&g_xms_move);
			return;
		}
		xms_move_any(&g_xms_move);
	}
}

void fight_draw_hero_box(Fight far *f, signed short bottom, struct hero far *h)
{
	signed short col1, col2, ind1, unused, ind_line, ind_x;

	if (h != 0) {
		font_get_box(g_font, &unused, &unused, &unused, &ind1);
		font_get_indent(g_font, &ind_line, &ind_x);
		font_set_box(g_font, -1, -1, -1, 0);
		font_set_indent(g_font, 0, 0);
		font_get_colors(g_font, &col1, &col2);
		font_set_colors_w(g_font, 0xff, 0);
		font_set_screen(g_font, f->buf);
		Bmp b(0x20, 0x20, 0);
		b.set_base(load_portrait(h));
		if (bottom == 0) {
			do_border(f->buf, 1, 9, 0x22, 0x22, 0xfd);
			font_print(g_font, h->name, 1, 1);
			b.put(f->buf, 2, 10);
			Bars_draw(g_bars, 0, 0, h->le, h->le_max, 1, f->buf);
			Bars_draw(g_bars, 1, 0, (signed char)h->dead ? 0 : h->ae, h->ae_max, 1, f->buf);
			fight_add_rect(0, 0, 100, 0x37);
		} else {
			do_border(f->buf, 1, 0x9d, 0x22, 0x22, 0xfd);
			font_print(g_font, h->name, 1, 0xc0);
			b.put(f->buf, 2, 0x9e);
			Bars_draw(g_bars, 0, 0, h->le, h->le_max, 3, f->buf);
			Bars_draw(g_bars, 1, 0, (signed char)h->dead ? 0 : h->ae, h->ae_max, 3, f->buf);
			fight_add_rect(0, 0x91, 100, 0xc7);
		}
		font_set_colors_w(g_font, col1, col2);
		font_set_screen(g_font, g_vga_buf);
		font_set_box(g_font, -1, -1, -1, ind1);
		font_set_indent(g_font, ind_line, ind_x);
	}
}

void fight_draw_enemy_box(Fight far *f, signed short bottom, struct enemy far *e)
{
	signed short col1, col2, ind1, unused, ind_line, ind_x;

	if (e != 0) {
		font_get_box(g_font, &unused, &unused, &unused, &ind1);
		font_get_indent(g_font, &ind_line, &ind_x);
		font_set_box(g_font, -1, -1, -1, 0);
		font_set_indent(g_font, 0, 0);
		font_get_colors(g_font, &col1, &col2);
		font_set_colors_w(g_font, 0xff, 0);
		font_set_screen(g_font, f->buf);
		Bmp b(0x20, 0x28);
		enemy_load_pic(e, b.data());
		if (bottom == 0) {
			do_border(f->buf, 1, 9, 0x22, 0x2a, 0xfd);
			font_print(g_font, enemy_name(e), 1, 1);
			b.put(f->buf, 2, 10);
			fight_add_rect(0, 0, 100, 0x37);
		} else {
			do_border(f->buf, 1, 0x95, 0x22, 0x2a, 0xfd);
			font_print(g_font, enemy_name(e), 1, 0xc0);
			b.put(f->buf, 2, 0x96);
			fight_add_rect(0, 0x91, 100, 0xc7);
		}
		font_set_colors_w(g_font, col1, col2);
		font_set_screen(g_font, g_vga_buf);
		font_set_box(g_font, -1, -1, -1, ind1);
		font_set_indent(g_font, ind_line, ind_x);
	}
}

signed short fight_rect_overlap(Fight far *f, register signed short x0, register signed short y0, signed short x1,
	signed short y1, signed short rx0, signed short ry0, signed short rx1, signed short ry1)
{
	signed short r = 0;

	if (g_quick_fight != 0)
		game_idle();
	if (x0 == rx0 && y0 == ry0 && x1 == rx1 && y1 == ry1)
		r = 0;
	else if (((x0 >= rx0 && x0 <= rx1) || (x1 >= rx0 && x1 <= rx1)) &&
		((y0 >= ry0 && y0 <= ry1) || (y1 >= ry0 && y1 <= ry1)))
		r = 1;
	return r;
}

void fight_draw_flush(Fight far *f)
{
	signed short i = 0, x0, x1, y0, y1, hidden = 0;
	Bmp b(320, 200, 0);
	signed short masked = 0, empty_text = 0, col1, col2;

	if (g_silent_mode != 0) {
		dlg_fill_area(g_dialog, f->buf);
		masked = 1;
	}
	/* welche Anzeigen liegen unter den geaenderten Rechtecken? */
	for (; g_fig_dirty[i].x0 != -1; i++) {
		x0 = g_fig_dirty[i].x0;
		x1 = g_fig_dirty[i].x1;
		y0 = g_fig_dirty[i].y0;
		y1 = g_fig_dirty[i].y1;
		if (fight_rect_overlap(f, x0, y0, x1, y1, 0, 0, 100, 0x37))
			g_fig_redraw_top = 1;
		if (fight_rect_overlap(f, x0, y0, x1, y1, 0, 0x91, 100, 0xc7))
			g_fig_redraw_bottom = 1;
		if (fight_rect_overlap(f, x0, y0, x1, y1, 0, 0xc0, 300, 0xc7))
			g_fig_redraw_text = 1;
	}
	if (g_fig_redraw_top != 0) {
		if (f->top_hero != 0)
			fight_draw_hero_box(f, 0, f->top_hero);
		else if (f->top_enemy != 0)
			fight_draw_enemy_box(f, 0, f->top_enemy);
		g_fig_redraw_top = 0;
	}
	if (g_fig_redraw_bottom != 0) {
		if (f->bottom_hero != 0)
			fight_draw_hero_box(f, 1, f->bottom_hero);
		else if (f->bottom_enemy != 0)
			fight_draw_enemy_box(f, 1, f->bottom_enemy);
		g_fig_redraw_bottom = 0;
	}
	if (g_fig_redraw_text != 0) {
		if (!*g_fig_text) {
			memset(g_fig_text, ' ', 50);
			g_fig_text[50] = 0;
			empty_text = 1;
		}
		if (*g_fig_text != 1) {
			font_get_colors(g_font, &col1, &col2);
			font_set_colors_w(g_font, 0xff, -1);
			font_set_screen(g_font, f->buf);
			font_print(g_font, g_fig_text, 1, 0xc0);
			font_set_colors_w(g_font, col1, col2);
			font_set_screen(g_font, g_vga_buf);
		} else
			*g_fig_text = 0;
		if (empty_text != 0)
			*g_fig_text = 0;
		fight_add_rect(0, 0xc0, 300, 0xc7);
		g_fig_redraw_text = 0;
	}
	/* Rechtecke aus dem Puffer auf den Bildschirm */
	for (i = 0; g_fig_dirty[i].x0 != -1; i++) {
		b.set_base((char far *)f->buf + g_fig_dirty[i].y0 * 320 + g_fig_dirty[i].x0);
		x0 = g_fig_dirty[i].x0;
		x1 = g_fig_dirty[i].x1;
		y0 = g_fig_dirty[i].y0;
		y1 = g_fig_dirty[i].y1;
		if (mouse_overlaps_rect(x0, y0, x1, y1)) {
			hidden = 1;
			mouse_hide(0);
		}
		if (masked != 0)
			b.put_part(g_vga_buf, x0, y0, x1 - x0 + 1, y1 - y0 + 1, BLIT_MASKED_FF);
		else
			b.put_part(g_vga_buf, x0, y0, x1 - x0 + 1, y1 - y0 + 1);
		if (hidden != 0) {
			mouse_show(0);
			hidden = 0;
		}
	}
	g_fig_redraw_all = 0;
}

void fight_add_rect(signed short x0, signed short y0, signed short x1, signed short y1)
{
	if (g_fig_dirty_full == 0) {
		if (g_fig_nr_dirty == 19) {
			/* Liste voll: Vollbild */
			g_fig_dirty[0].x0 = 0;
			g_fig_dirty[0].y0 = 0;
			g_fig_dirty[0].x1 = 0x13f;
			g_fig_dirty[0].y1 = 0xc7;
			g_fig_dirty[0].used = 1;
			g_fig_dirty_full = 1;
			return;
		}
		g_fig_dirty[g_fig_nr_dirty].x0 = x0 < 0 ? 0 : x0 > 0x13f ? 0x13f : x0;
		g_fig_dirty[g_fig_nr_dirty].y0 = y0 < 0 ? 0 : y0 > 0xc7 ? 0xc7 : y0;
		g_fig_dirty[g_fig_nr_dirty].x1 = x1 < 0 ? 0 : x1 > 0x13f ? 0x13f : x1;
		g_fig_dirty[g_fig_nr_dirty].y1 = y1 < 0 ? 0 : y1 > 0xc7 ? 0xc7 : y1;
		g_fig_dirty[g_fig_nr_dirty].used = 1;
		g_fig_nr_dirty++;
	}
}

void fight_add_obj_rect(Fight far *f, struct fig_rect far *tab, unsigned char id, signed short x0, signed short y0,
	signed short x1, signed short y1, signed short frame, signed char removed)
{
	signed short found = -1;
	signed short j;

	for (j = 0; tab[j].id != 0xff; j++) {
		if (tab[j].id == id) {
			found = j;
			break;
		}
	}
	if (found == -1) {
		fight_add_rect(x0, y0, x1, y1);
		return;
	}
	if (!removed) {
		if (tab[found].x0 != x0 || tab[found].x1 != x1 || tab[found].y0 != y0 || tab[found].y1 != y1 ||
			tab[found].frame != frame)
			fight_add_rect(tab[found].x0 < x0 ? tab[found].x0 : x0, tab[found].y0 < y0 ? tab[found].y0 : y0,
				tab[found].x1 > x1 ? tab[found].x1 : x1, tab[found].y1 > y1 ? tab[found].y1 : y1);
	}
}

void fight_draw_figures(Fight far *f)
{
	signed short cbx, cby, x, y, k;
	struct fig_node far *cur;
	struct fig_node far *head;
	signed short w, h, event, dir;
	struct enemy far *e;
	signed char viewdir;
	signed short idx;
	struct fig_rect far *old_tab;
	signed short n;

	n = 0;
	old_tab = (struct fig_rect far *)mem_alloc(0xaf5L);
	memcpy(old_tab, f->draw_tab, 0xaf5);
	memset(f->draw_tab, 0, 0xaf5);
	cur = f->list.head;
	do {
		if (cur->visible == 1 && cur->gfx != 0) {
			/* Feldobjekt mit Standbild */
			cur->gfx->size(&w, &h);
			cbx = cur->cbx;
			cby = cur->cby;
			x = g_fig_origin_x - w / 2 + (cbx + cby) * 10;
			y = g_fig_origin_y - h + (cbx - cby) * 5;
			x += cur->offsetx;
			y += cur->offsety;
			cur->gfx->put(f->buf, x, y, BLIT_MASKED);
			f->draw_tab[n].id = cur->id;
			f->draw_tab[n].x0 = x;
			f->draw_tab[n].y0 = y;
			f->draw_tab[n].x1 = x + w;
			f->draw_tab[n].y1 = y + h;
			f->draw_tab[n].frame = (signed char)cur->obj_pic;
			if (g_fig_dirty_full == 0)
				fight_add_obj_rect(f, old_tab, f->draw_tab[n].id, f->draw_tab[n].x0, f->draw_tab[n].y0,
					f->draw_tab[n].x1, f->draw_tab[n].y1, f->draw_tab[n].frame, 0);
			n++;
		} else if (cur->visible == 1 && (cur->anim != 0 || (cur->twofielded > 20 && cur->twofielded != FIG_TWOFIELD_NONE))) {
			/* animierte Figur (beim Schwanz eines Zweifeldgegners die des Kopfes) */
			head = cur;
			if (cur->twofielded != FIG_TWOFIELD_NONE && cur->twofielded > 20)
				head = fig_list_find_twofield(&f->list, cur->twofielded - 20);
			if (head->anim->loaded == 0) {
				head->offsetx = head->cbx;
				head->offsety = head->cby;
			}
			cbx = head->offsetx;
			cby = head->offsety;
			x = g_fig_origin_x + (cbx + cby) * 10;
			y = g_fig_origin_y + (cbx - cby) * 5;
			if (f->id == FIGHT_ID_SPECIAL) {
				if ((idx = is_in_byte_array2(head->id, g_sfight_figs)) != -1)
					sfight_fig_pos(f, idx, &x, &y);
			} else if (f->id == FIGHT_ID_SPECIAL2 && head->kind == FIG_KIND_ENEMY) {
				x = g_sfight2_x;
				y = g_sfight2_y;
			}
			dir = 0;
			if (cur->twofielded != FIG_TWOFIELD_NONE) {
				e = fig_get_enemy(head->id);
				viewdir = e->viewdir;
				if (cur->twofielded > 20)
					dir = viewdir == 0 ? 1 : viewdir == 1 ? 4 : viewdir == 2 ? 2 : 3;
				else
					dir = viewdir == 0 ? 2 : viewdir == 1 ? 3 : viewdir == 2 ? 1 : 4;
			}
			f->draw_tab[n].id = head->id;
			event = fig_anim_draw(head->anim, f->buf, x, y, dir, &f->draw_tab[n].x0, &f->draw_tab[n].y0,
				&f->draw_tab[n].x1, &f->draw_tab[n].y1, &f->draw_tab[n].frame,
				cur->twofielded != FIG_TWOFIELD_NONE && cur->twofielded > 20 ? 1 : 0);
			if (g_fig_dirty_full == 0)
				fight_add_obj_rect(f, old_tab, f->draw_tab[n].id, f->draw_tab[n].x0, f->draw_tab[n].y0,
					f->draw_tab[n].x1, f->draw_tab[n].y1, f->draw_tab[n].frame, 0);
			n++;
			if (f->id == FIGHT_ID_SPECIAL && is_in_byte_array2(head->id, g_sfight_figs) != -1 && g_sfight_seq_on != 0)
				g_sfight_event = event;
			else if (event != 0 && event != 9 && event != 10)
				fig_anim_event(f, cur, event);
		}
	} while ((cur = cur->next) != 0);
	f->draw_tab[n].id = 0xff;
	n++;
	fig_list_purge(&f->list);
	/* verschwundene Figuren */
	for (k = 0; g_fig_dirty_full == 0 && old_tab[k].id != 0xff; k++)
		fight_add_obj_rect(f, f->draw_tab, old_tab[k].id, old_tab[k].x0, old_tab[k].y0, old_tab[k].x1, old_tab[k].y1,
			old_tab[k].frame, 1);
	mem_free(old_tab);
}

void fight_anim_tick(Fight far *f)
{
	register signed short any = 0;
	struct fig_node far *p;

	if (g_fig_anim_busy == 0) {
		g_fig_anim_busy = 1;
		if (!g_quick_fight) {
			if (g_fight_screen_on == 1) {
				p = f->list.head;
				do {
					if (p->visible == 1 && p->anim != 0 && fig_anim_step(p->anim))
						any = 1;
				} while ((p = p->next) != 0);
				if (any)
					fight_draw(f);
			}
		} else if (g_snd_service_due == 1)
			fig_draw_hero_bar(f);
		g_fig_anim_busy = 0;
	}
}

void fight_recolor(char far *p, signed short n, signed char color)
{
	char far *q = p;

	while (n--) {
		if (*q != 0)
			*q = color;
		q++;
	}
}

signed short fight_printer(struct fight_msgs far *o)
{
	signed short pic, w, h, tx, ret, col1, col2;
	char buf[6];

	ret = 0;
	if (o->active == 1 && !*g_fig_text && g_silent_mode == 0 && MSG.id != 0) {
		ret = 1;
		if (--o->delay <= 0) {
			/* naechste Meldung */
			o->cur++;
			o->snd_done = 0;
			if (MSG.id != 0)
				o->delay = o->delay_init;
			else {
				ret = 0;
				fight_add_rect(0, 0x8c, 0x5a, 0xc7);
				fight_add_rect(0, 0xc0, 300, 0xc7);
				o->cur = 0;
				Lst_init(o);
			}
		}
		if (ret == 1) {
			g_fig_redraw_text = 1;
			*g_fig_text = 1;
			font_get_colors(g_font, &col1, &col2);
			font_set_screen(g_font, o->screen);
			/* Stern 0x7e/0x7f in der Farbe der Meldung, darin der Wert */
			pic = o->cur % 2 ? 0x7e : 0x7f;
			nvfile_info(g_nvf_heads, 0x47, pic, &w, &h);
			Bmp b(w, h);
			nvfile_load(g_nvf_heads, 0x47, b.data(), 0, pic, 0, &w, &h, 0x140);
			fight_recolor((char far *)b.data(), w * h, g_fig_msg_colors[MSG.id]);
			b.put(o->screen, 8, 0xc3 - h, BLIT_MASKED);
			if (MSG.value != 0) {
				font_set_colors_w(g_font, 0xff, g_fig_msg_colors[MSG.id]);
				itoa(MSG.value, buf, 10);
				tx = font_center_x(g_font, buf, 8, w);
				font_print(g_font, buf, tx, 0xbf - h / 2);
			}
			font_set_colors_w(g_font, 0xff, -1);
			sprintf(o->buf, o->txt[MSG.id],
				MSG.type1 == 2 ? ((struct hero far *)MSG.obj1)->name :
					grammar_article(0x8000, ((struct enemy far *)MSG.obj1)->mon_id, 1),
				MSG.type2 == 2 ? HERO2->name :
					grammar_article(0x8002, ENEMY2->mon_id, 1));
			o->buf[0] = toupper(o->buf[0]);
			font_print(g_font, o->buf, 1, 0xc0);
			fight_add_rect(0, 0x8c, 0x5a, 0xc7);
			fight_add_rect(0, 0xc0, 300, 0xc7);
			font_set_colors_w(g_font, col1, col2);
			font_set_screen(g_font, g_vga_buf);
			if (o->snd_done == 0) {
				/* Klang zur Meldung, einmal */
				o->snd_done = 1;
				if (MSG.id == 11 || MSG.id == 15 || MSG.id == 16) {
					if (!(signed char)ENEMY2->dead) {
						if (enemy_is_gfx_25_35_36_23(ENEMY2))
							snd_play2(g_snd_fx, 0x7f);
						else if (enemy_is_gfx_24_18(ENEMY2))
							snd_play2(g_snd_fx, 0x66);
						else if (ENEMY2->is_animal != 0)
							snd_play2(g_snd_fx, 0x53);
						else if (enemy_is_humanoid(ENEMY2)) {
							if (MSG.id == 11)
								snd_play2(g_snd_fx, 0x51);
							else
								snd_play2(g_snd_fx, random_schweif(100) > 50 ? 0x7b : 0x7c);
						} else
							snd_play2(g_snd_fx, 0x75);
					}
				} else if (MSG.id == 7) {
					signed short sex = HERO2->sex;
					snd_play2(g_snd_fx, sex == 0 ? 0x62 : 0x63);
				} else if (MSG.id == 8 || MSG.id == 13 || MSG.id == 14) {
					signed short value = MSG.value;
					register signed short sex = HERO2->sex;
					if ((signed char)HERO2->dead)
						snd_play2(g_snd_fx, sex == 0 ? 0x4c : 0x4b);
#if FIX_0181
					/* Fix #181: Heldenzweig: erstes Ereignis 8 (wie 11 im Gegnerzweig) statt der dort unmoeglichen 11 */
					else if (MSG.id == 8) {
						if (value > 20)
#else
					else if (MSG.id == 11) {
						if (value > 20)
#endif
							snd_play2(g_snd_fx, sex == 0 ? 0x51 : 0x50);
						else
							snd_play2(g_snd_fx, sex == 0 ? 0xa9 : 0xa6);
					} else
						snd_play2(g_snd_fx, random_schweif(100) > 50 ? 0x7b : 0x7c);
				} else if (MSG.id == 6)
					snd_play2(g_snd_fx, 0x6c);
			}
		}
	}
	return ret;
}

signed short all_heroes_down(struct group far *g)
{
	register signed short res = 1;
	register signed short i = 0;
	struct hero far *h = g->heroes[0];

	for (; i <= 6; h = g->heroes[++i]) {
		if (h != 0 && !h->npc_id && !h->fb5 && check_hero(h)) {
			res = 0;
			break;
		}
	}
	return res;
}

void fight_check_keys(Fight far *f)
{
	signed short k;
	struct fig_keys keys = g_fig_keys;

	poll_input();
	handle_options_key();
	if (g_key_last != 0) {
		k = is_in_word_array(g_key_last, &keys);
		if (g_quick_fight != 0 || (all_heroes_down(g_group) && k == -1))
			f->keys[6] = 1;
		else if (k != -1)
			f->keys[k] = 1;
		else if (all_heroes_down(g_group))
			f->keys[6] = 1;
		g_key_last = 0;
	}
}
