/* OVL147 - Overlay O147 (Segment 0x224a in SCHWEIF.EXE): Kampf-Hilfsroutinen fuer die Gegner (g_enemies, 20 x 0x49
 * Bytes) und das Kampffeld g_fight_walkmap (25 Bytes je Zeile), entspricht teils DSA1 seg032/seg034/seg039.
 * fig_is_cell_89_8c(v): 1 fuer Feldwert 0x89..0x8c. fig_all_heroes_escaped/fig_fight_continues: Kampf fortsetzen?
 * (fig_get_first_active_hero/scared_hero_fled, g_game_state = 6 bzw. 1, sonst fig_count_active_enemies > 0).
 * fig_tidy_monsters(f): tote/Pilz-/versteinerte Gegner (bzw. f->monsters[i].round_appear gesetzt und kein aktiver
 *   Gegner) aus der Liste nehmen - folgende Gegner und Aufstellung f->monsters per Strukturkopie nachruecken, frei
 *   gewordenen Eintrag nullen und als tot markieren. fig_find_free_cell(f, x, y, px, py, dir, two): freies Feld in
 *   wachsendem Abstand suchen (g_fig_tail_dxy; two = Zweifeldgegner, Schwanzfeld mit pruefen).
 * fight_latecomers(f): Gegner mit Erscheinungsrunde round_appear erscheinen lassen (Figuren fig_list_find/
 *   fig_list_find_twofield, fig_list_remove/fig_list_add/fig_list_show, place_obj_on_cb, Summen
 *   f->enemy_le_sum/enemy_ae_sum += le/ae), fliehende (scared) dagegen tot.
 * fig_round_enemies(f, n): Rundenbeginn (attacks_left = attacks, bp = bp_max, parried = 0, Summe der Aktionen,
 *   Sonderfaelle g_fight_end == 5/3). fig_count_active_enemies: Anzahl fight_check_state.
 * fight_status_text(s): Text in die Textzeile g_fig_text (erster Buchstabe gross, toupper), Wartezeit
 *   wait_ticks_key (g_fight_speed * 20 + 40 bzw. g_text_speed * 20 + 20). screen_flash: Bildschirm weiss aufblitzen
 *   (Palette 0x300 sichern/set_palette, fuenfmal wait_for_vsync). place_obj_on_cb(f, x, y, obj, type, dir): Objekt
 *   aufs Kampffeld setzen (= DSA1 place_obj_on_cb; Dekoration >= FIG_CELL_OBJ mit Mehrfeld-Typen, Zweifeldgegner
 *   ab 10 per Liste g_twofield_mons, is_in_byte_array2). fig_clear_hits: g_fig_hit_heroes/g_fig_hit_anims loeschen.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL147.CPP   -> 3188/3188 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH; fig_list_add mit char-Argument ueber FIG_LIST_ADD_C)
 * - C++ nur wegen inline-Settern der Gegnerstruktur (set_round_appear/set_attacks_left/set_bp): die Parameterkopie ergibt
 *   "mov al,.. / mov [bp-x],al / les bx,e / mov al,[bp-x] / mov es:[..],al" ([bp-14h] in fight_latecomers, [bp-5]/[bp-6]
 *   in fig_round_enemies); "e->round_appear -= 1", "--", "= e->round_appear - 1" usw. erzeugen das nicht.
 * - -Og noetig (Zwischenwerte: Bitbyte [bp-9], Adressen [bp-0ch]/[bp-0eh] in fig_tidy_monsters, Tabellenoffset in SI mit
 *   nachgeladenem Segment in fight_latecomers, dir * 4 [bp-4] und Feldoffset [bp-2] mit "mov es,[c99d]" in place_obj_on_cb);
 *   -Oi fuer memset/strcpy, Strukturkopien ueber SCOPY@.
 * - "if (..) return 1; return 0;" (ein toter Epilog) vs "if .. return a; else return 0;" (zwei, fig_all_heroes_escaped).
 * - Kampffeld als "*(g_fight_walkmap + y * 25 + x)" (unsigned char): "!= 0" -> cmp byte, "!*(..)" -> mov al/mov ah,0/or;
 *   "> 0" -> ja. "y + g_fig_tail_dxy[dir].dy < 0" -> add ax,.. / jl (Flags der Addition).
 * - fig_tidy_monsters als while-Schleife: nach dem Nachruecken wird derselbe Index erneut geprueft (Sprung hinter das i++).
 *   g_enemies[j + 1].dead = 1 wird zu "or byte [bx+81h],1" gefaltet.
 * - fig_clear_hits: register i landet ohne Aufrufe in AX.
 * - Lokale Reihenfolge fight_latecomers: id [bp-13h] (echter Local) vor der Setter-Kopie [bp-14h].
 * Originalfehler: place_obj_on_cb prueft wie DSA1 "Feld belegt oder ungueltiges Objekt" (Feld > 0 || obj == 0xfe), aber
 *   der Rumpf ist leer (DSA1: return 0) - die Pruefung bleibt wirkungslos. fig_find_free_cell sucht ohne Abbruch (Endlos-
 *   schleife, wenn kein freies Feld existiert), wie DSA1.
 */
#include <string.h>
#include <ctype.h>
#include "schweif.h"

/* fig_list_add mit char-Argument (Original: push ax ohne cbw) */
#define FIG_LIST_ADD_C(l, id) ((signed char (*)(void far *, signed char))fig_list_add)(l, id)

signed short fig_is_cell_89_8c(signed short v)
{
	if (v >= 0x89 && v <= 0x8c)
		return 1;
	return 0;
}

signed short fig_all_heroes_escaped(FIGHT_PTR f)
{
	if (!fig_get_first_active_hero(g_group))
		return scared_hero_fled(g_group);
	else
		return 0;
}

signed short fig_fight_continues(FIGHT_PTR f)
{
	if (fig_all_heroes_escaped(f)) {
		if (g_quick_fight != 0)
			fig_draw_hero_bar(f);
		g_game_state = 6;
		return 0;
	}
	if (!fig_get_first_active_hero(g_group)) {
		if (g_quick_fight != 0)
			fig_draw_hero_bar(f);
		g_game_state = 1;
		return 0;
	}
	if (!fig_count_active_enemies(f))
		return 0;
	return 1;
}

void fig_tidy_monsters(FIGHT_PTR f)
{
	signed short i;
	signed short cnt;
	struct enemy far *e;
	register signed short j;

	if (g_quick_fight != 0)
		game_idle();
	cnt = fig_count_active_enemies(f);
	i = 0;
	while (i < 20) {
		e = &g_enemies[i];
		if (e->mon_id != 0 && ((unsigned char)e->dead || (unsigned char)e->mushroom || (unsigned char)e->petrified ||
		    (f->monsters[i].round_appear != 0 && cnt == 0))) {
			if (i == 19) {
				memset(&f->monsters[i], 0, sizeof(struct fight_monster));
				return;
			} else {
				for (j = i; j < 19; j++) {
					f->monsters[j] = f->monsters[j + 1];
					memset(&f->monsters[j + 1], 0, sizeof(struct fight_monster));
					g_enemies[j] = g_enemies[j + 1];
					memset(&g_enemies[j + 1], 0, sizeof(struct enemy));
					g_enemies[j + 1].dead = 1;
				}
			}
		} else
			i++;
	}
}

void fig_find_free_cell(FIGHT_PTR f, signed short x, signed short y, signed short far *px, signed short far *py,
	signed short dir, signed short two)
{
	signed short k;
	signed short r;
	signed char done = 0;
	register signed short nx;
	register signed short ny;

	if (g_quick_fight != 0)
		game_idle();
	*px = x;
	*py = y;
	if (*(g_fight_walkmap + y * 25 + x) != 0 ||
	    (two && *(g_fight_walkmap + (y + g_fig_tail_dxy[dir].dy) * 25 + x + g_fig_tail_dxy[dir].dx) != 0)) {
#if FIX_0039
		/* Fix #39: Suche begrenzen, danach das ganze Feld zeilenweise absuchen (kein Haenger) */
		for (r = 1; !done && r < 24; r++)
			for (k = 0; k < 4; k++) {
				nx = *px + g_fig_tail_dxy[k].dx * r;
				ny = *py + g_fig_tail_dxy[k].dy * r;
				if (nx >= 0 && nx < 24 && ny >= 0 && ny < 24 && !*(g_fight_walkmap + ny * 25 + nx) &&
				    (!two || !*(g_fight_walkmap + (ny + g_fig_tail_dxy[dir].dy) * 25 + nx + g_fig_tail_dxy[dir].dx))) {
					done = 1;
					*px = nx;
					*py = ny;
					break;
				}
			}
		for (ny = 0; !done && ny < 24; ny++)
			for (nx = 0; !done && nx < 24; nx++)
				if (!*(g_fight_walkmap + ny * 25 + nx) &&
				    (!two || !*(g_fight_walkmap + (ny + g_fig_tail_dxy[dir].dy) * 25 + nx + g_fig_tail_dxy[dir].dx))) {
					done = 1;
					*px = nx;
					*py = ny;
				}
#else
		for (r = 1; !done; r++)
			for (k = 0; k < 4; k++) {
				nx = *px + g_fig_tail_dxy[k].dx * r;
				ny = *py + g_fig_tail_dxy[k].dy * r;
				if (nx >= 0 && nx < 24 && ny >= 0 && ny < 24 && !*(g_fight_walkmap + ny * 25 + nx) &&
				    (!two || !*(g_fight_walkmap + (ny + g_fig_tail_dxy[dir].dy) * 25 + nx + g_fig_tail_dxy[dir].dx))) {
					done = 1;
					*px = nx;
					*py = ny;
					break;
				}
			}
#endif
	}
}

void fight_latecomers(FIGHT_PTR f)
{
	signed short i;
	signed short x;
	signed short y;
	struct enemy far *e;
	struct fig_node far *head;
	struct fig_node far *tail;
	signed char id;

	if (g_quick_fight != 0)
		game_idle();
	for (i = 0; i < g_nr_enemies; i++) {
		e = &g_enemies[i];
		if (e->round_appear > 0) {
			e->set_round_appear(e->round_appear - 1);
			if (!e->round_appear) {
				if (!(signed char)e->scared) {
					if (enemy_is_twofielded(e)) {
						fig_find_free_cell(f, f->monsters[i].x, f->monsters[i].y, &x, &y, f->monsters[i].viewdir, 1);
						head = fig_list_find(&f->list, e->fighter_id);
						head->cbx = x;
						head->cby = y;
						tail = fig_list_find_twofield(&f->list, head->twofielded + 20);
						tail->cbx = x + g_fig_tail_dxy[f->monsters[i].viewdir].dx;
						tail->cby = y + g_fig_tail_dxy[f->monsters[i].viewdir].dy;
						id = e->fighter_id;
						fig_list_remove(&f->list, id, 1);
						FIG_LIST_ADD_C(&f->list, id);
						id = tail->id;
						fig_list_remove(&f->list, id, 1);
						FIG_LIST_ADD_C(&f->list, id);
					} else {
						fig_find_free_cell(f, f->monsters[i].x, f->monsters[i].y, &x, &y, f->monsters[i].viewdir, 0);
						head = fig_list_find(&f->list, e->fighter_id);
						head->cbx = x;
						head->cby = y;
						fig_list_remove(&f->list, e->fighter_id, 1);
						FIG_LIST_ADD_C(&f->list, e->fighter_id);
					}
					place_obj_on_cb(f, x, y, i + 10, e->gfx_id, f->monsters[i].viewdir);
					fig_list_show(&f->list, e->fighter_id);
					f->enemy_le_sum += e->le;
					f->enemy_ae_sum += e->ae;
				} else
					e->dead = 1;
			}
		}
	}
}

signed short fig_round_enemies(FIGHT_PTR f, signed short n)
{
	struct enemy far *e;
	register signed short sum = 0;
	register signed short i;

	if (g_quick_fight != 0)
		game_idle();
	for (i = 0; i < n; i++) {
		e = &g_enemies[i];
		e->set_attacks_left(e->attacks);
		sum += e->attacks;
		e->set_bp(e->bp_max);
		e->parried = 0;
		if (g_fight_end == 5 && e != g_enemies) {
			e->attacks_left = 0;
			sum -= e->attacks;
		}
		if (g_fight_end == 3 && e == g_enemies) {
			e->attacks_left = 0;
			sum -= e->attacks;
		}
	}
	return sum;
}

signed short fig_count_active_enemies(FIGHT_PTR f)
{
	struct enemy far *e;
	register signed short i;
	register signed short n = 0;

	if (g_quick_fight != 0)
		game_idle();
	for (i = 0; i < 20; i++) {
		e = &g_enemies[i];
		if (fight_check_state(e))
			n++;
	}
	return n;
}

void fight_status_text(char far *s)
{
	strcpy(g_fig_text, s);
	*g_fig_text = toupper(*g_fig_text);
	g_fig_redraw_text = 1;
	fight_draw((FIGHT_PTR)g_fight_obj);
	if (all_heroes_down(g_group))
		wait_ticks_key(g_fight_speed * 20 + 40);
	else
		wait_ticks_key(g_text_speed * 20 + 20);
	*g_fig_text = 0;
	g_fig_redraw_text = 1;
	fight_draw((FIGHT_PTR)g_fight_obj);
}

void screen_flash(void)
{
	void far *pal;
	register signed short i;

	pal = mem_alloc(0x300);
	get_palette(pal, 0, 0x100);
	wait_for_vsync();
	memset(g_text_output_buf, 0x3f, 0x300);
	set_palette(g_text_output_buf, 0, 0x100);
	memset(g_text_output_buf, 0, 0x300);
	for (i = 0; i < 5; i++)
		wait_for_vsync();
	set_palette(pal, 0, 0x100);
	mem_free(pal);
}

signed short place_obj_on_cb(FIGHT_PTR f, register signed short x, signed short y, unsigned char obj,
	unsigned char type, signed char dir)
{
	register signed short i;

#if FIX_0038
	/* Fix #38: belegtes Feld bzw. ungueltiges Objekt: nicht setzen (wie DSA 1) */
	if (*(g_fight_walkmap + y * 25 + x) > 0 || obj == 0xfe)
		return 0;
#else
	if (*(g_fight_walkmap + y * 25 + x) > 0 || obj == 0xfe)
		;
#endif
	if (obj >= 50) {
		if (type == 0x56 || type == 0x55 || type == 0x5d) {
			fight_set_cell(f, x, y + 1, obj);
			fight_set_cell(f, x - 1, y + 1, obj);
			fight_set_cell(f, x - 1, y, obj);
		} else if (type == 9 || type == 0x13 || type == 0x36 || type == 0x40) {
			fight_set_cell(f, x + 1, y, obj);
			fight_set_cell(f, x, y - 1, obj);
		} else if (type == 0x48 || type == 0x49 || type == 0x4d || type == 0x4e || type == 0x4f ||
			   type == 0x50 || type == 0x51 || type == 0x52 || type == 0x53 || type == 0x54) {
			fight_set_cell(f, x, y + 1, obj);
		} else if (type == 0x59) {
			for (i = 0; i < 7; i++)
				fight_set_cell(f, x, y + i, obj);
		} else if (type == 0x5a) {
			for (i = 0; i < 7; i++)
				fight_set_cell(f, x + i, y, obj);
		} else if (type == 0x62) {
			fight_set_cell(f, x - 1, y + 1, obj);
			fight_set_cell(f, x - 1, y + 2, obj);
			fight_set_cell(f, x, y + 2, obj);
		}
	} else if (obj >= 10 && is_in_byte_array2(type, g_twofield_mons) != -1) {
		if ((*(g_fight_walkmap + (y + g_fig_tail_dxy[dir].dy) * 25 + x + g_fig_tail_dxy[dir].dx) != 0 &&
		     *(g_fight_walkmap + (y + g_fig_tail_dxy[dir].dy) * 25 + x + g_fig_tail_dxy[dir].dx) != 0xfe) ||
		    y + g_fig_tail_dxy[dir].dy < 0 || y + g_fig_tail_dxy[dir].dy > 23 ||
		    x + g_fig_tail_dxy[dir].dx < 0 || x + g_fig_tail_dxy[dir].dx > 23)
			return 0;
		fight_set_cell(f, x + g_fig_tail_dxy[dir].dx, y + g_fig_tail_dxy[dir].dy, obj + 20);
	}
	if (*(g_fight_walkmap + y * 25 + x) == 0 || *(g_fight_walkmap + y * 25 + x) == 0xfe)
		fight_set_cell(f, x, y, obj);
	return 1;
}

void fig_clear_hits(void)
{
	register signed short i;

	g_fig_nr_hits = 0;
	for (i = 0; i < 7; i++) {
		g_fig_hit_heroes[i] = 0;
		g_fig_hit_anims[i] = 0;
	}
}
