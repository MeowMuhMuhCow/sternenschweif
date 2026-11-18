/* OVL075 - Overlay O075 (Segment 0x20b9 in SCHWEIF.EXE): Gruppenverwaltung (entspricht DSA1 seg049
 * GRP_split/GRP_merge/GRP_switch_to_next/GRP_swap_heroes/GRP_move_hero). Gruppen g_groups[6] (struct group),
 * aktive Gruppe g_group, Zahl der Gruppen g_nr_groups.
 * make_group(n, ...): neue Gruppe aus n Helden der aktiven Gruppe (variadisch, far-Zeiger) bilden. grp_split:
 * Gruppe teilen (Helden einzeln per get_any_hero waehlen, mindestens 1 bzw. mit NSC 2 bleiben zurueck).
 * groups_merge: Gruppen vereinen (group_find_mergeable liefert die Gruppen am selben Ort). grp_choose(titel, n,
 * alt, unbenutzt): Auswahlmenue dlg_choose mit den 7 Texten g_menu_texts, liefert g_menu_groups[sel] oder 0.
 * grp_menu_entry(g, k): Menuezeile k ("Gruppe k: Name, Name ...") in den 600-Byte-Puffer g_menu_buf schreiben
 * (sprintf/strcat). grp_select(titel): Gruppenmenue aufbauen (aktive Gruppe zuletzt). group_switch_next(mode): zur
 * naechsten Gruppe wechseln (Pruefungen group_count_check2b/group_has_hero/check_hero/group_any_in_jail, Ortswechsel
 * ueber snd_obj_stop/swap_out_screen). swap_heroes_dialog: zwei Helden tauschen (Strukturkopien 0x5ef ueber Puffer,
 * Bytes g_hero_slot_aux[4][7] mittauschen, group_pos = Platz + 1). drag_hero_portrait(src): Held mit der Maus auf einen anderen
 * Platz ziehen (zwei Bmp-Objekte 32x32, Portraetleiste g_portrait_x[6], Mausbereich g_mouse_min/max_x/y).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL075.CPP   -> 3769/3769 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH; g_hero_slot_aux -> _g_c501+2, g_menu_groups -> _g_c824+2)
 * - C++ wegen new/delete Bmp (R42_00D8(0,..)/R42_03B3(p,3)) und der inline-Setter group::clr_hero (Kopie [bp-14h]/
 *   [bp-16h]) und hero::set_group_pos (int-Parameter, Byte gespeichert).
 * - Strukturzuweisung far->far *g = *g_group (0x49 Bytes) wird INLINE (mov ax,seg / mov si / les di / push ds /
 *   mov ds,ax / mov cx,24h / rep movsw / movsb), nicht SCOPY@ und nicht memcpy (memcpy laedt les di zuerst);
 *   die 0x5ef-Bytes-Heldenkopie geht dagegen ueber SCOPY@. Der Konstruktor von struct group aendert daran nichts.
 * - Wahrheitstest von signed char in while/&& : "!!g->nr_heroes" (mov al/cbw/or); "g->nr_heroes" allein -> cmp byte.
 * - swap_heroes_dialog: "(signed short)g_group->nr_heroes > 1" und "== 2" -> -Og-Zwischenwert [bp-16h] HINTER den
 *   inline-Kopien; eine eigene Variable n (auch im Block oder als dritte register-Variable) landet vor ihnen.
 * - va_start(ap, n) vor dem ersten if; va_arg ergibt add [ap],4 / les / mov eax,es:[bx-4].
 * - grp_split: "if (!h) break; else if (A && B) ; else {...}" erzeugt den toten jmp near hinter dem break.
 * - grp_choose: "return sel == -1 ? 0 : g_menu_groups[sel];" + toter Epilog; group_switch_next: done (DI) vor npc
 *   initialisiert. npc = (char far *)g_heroes + 0x239a (0x20 Bytes vor g_heroes[6], wie im Original).
 * - drag_hero_portrait: "for (dst = 6; g_portrait_x[--dst] > g_mouse_posx + 25; ) ;" ergibt den jmp short auf den direkt
 *   folgenden Test.
 * Originalfehler: grp_split sucht (anders als make_group, das g_nr_groups == 6 prueft) ohne Grenze nach einer freien
 *   Gruppe - sind alle 6 belegt, laeuft die Suche hinter g_groups[5] weiter. drag_hero_portrait liest bei Mausposition
 *   links der ersten Portraetposition g_portrait_x[-1] und weiter davor (Schleife ohne Untergrenze, erst danach
 *   dst < 0 -> 0). grp_choose hat einen unbenutzten 4. Parameter (grp_select uebergibt 1). group_switch_next: im
 *   else-Zweig ist state immer 1, die Bedingung "state || mode != 2" also immer wahr; der NSC-Zeiger 0x239a liegt
 *   0x20 Bytes vor g_heroes[6] (vermutlich mit einer 0x5eb-Bytes-Heldenstruktur gerechnet).
 */
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "schweif.h"

/* Phase B: eigenes BSS (ds:c824..c842) in Adressreihenfolge */
signed char g_camp_fight_pending;
char g_unused_c825[1];   /* ds:c825, unbenutzt */
struct group far *g_menu_groups[6];
char far *g_menu_buf;

void make_group(signed short n, ...)
{
	va_list ap;
	struct hero far *h;
	struct group far *g;
	signed short i;
	signed short j;
	signed short slot;

	va_start(ap, n);
	if (g_nr_groups != NR_GROUPS) {
		i = 0;
		do
			g = &g_groups[i++];
		while (!!g->nr_heroes);
		*g = *g_group;
		for (j = 0; j <= 6; j++)
			g->clr_hero(j);
		g->money = 0;
		g->nr_heroes = 0;
		for (j = 0; j < n; j++) {
			h = va_arg(ap, struct hero far *);
			slot = group_hero_pos(g_group, h);
			group_remove_hero(g_group, slot);
			slot = group_free_slot(g);
			group_add_hero(g, h, slot);
		}
		g_nr_groups++;
		g_hero_died = 1;
		if (g_pp20_index == 4)
			screen_portraits(g_screen);
	}
}

void grp_split(void)
{
	struct hero far *h;
	struct group far *g;
	signed short i;
	signed short done;
	signed short min;
	signed short j;
	signed short slot;
	signed short base;

	min = group_get_npc(g_group) ? 2 : 1;
	if (g_group->nr_heroes <= min) {
		dlg_message(g_dialog, g_ttx[0x19]);
		return;
	}
	done = i = 0;
#if FIX_0061
	/* Fix #61: Suche auf die 6 Gruppen begrenzen, ohne freie Gruppe abbrechen */
	do
		g = &g_groups[i++];
	while (i < 6 && !!g->nr_heroes);
	if (!!g->nr_heroes) {
		dlg_message(g_dialog, g_ttx[0x19]);
		return;
	}
#else
	do
		g = &g_groups[i++];	/* Originalfehler: ohne Grenze (bei 6 Gruppen hinter g_groups[5]) */
	while (!!g->nr_heroes);
#endif
	*g = *g_group;
	for (j = 0; j <= 6; j++)
		g->clr_hero(j);
	g->money = 0;
	g->nr_heroes = 0;
	h = group_get_npc(g_group);
	if (h)
		base = check_hero2(h);
	else
		base = 0;
	while (group_count_check2b(g_group) >= 1 && g_group->nr_heroes > min) {
		h = group_get_npc(g_group);
		g_hero_excluded = group_hero_pos(g_group, h);
		h = get_any_hero(g_group, g_ttx[0x1a]);
		if (!h)
			break;
		else if (check_hero2(h) && group_count_check2(g_group) - base == 1)
			;	/* der letzte handlungsfaehige Held bleibt */
		else {
			done = 1;
			slot = group_hero_pos(g_group, h);
			group_remove_hero(g_group, slot);
			slot = group_free_slot(g);
			group_add_hero(g, h, slot);
		}
	}
	if (done) {
		g_nr_groups++;
		if (g_pp20_index == 4)
			screen_portraits(g_screen);
	}
}

void groups_merge(void)
{
	struct group far *g;
	struct hero far *h;
	signed short slot;
	signed short idx;

	idx = group_find_mergeable();
	if (idx == -1) {
		dlg_message(g_dialog, g_ttx[0x1b]);
		return;
	}
	do {
		g = &g_groups[idx];
		while (!!g->nr_heroes) {
			h = get_first_hero(g);
			slot = group_hero_pos(g, h);
			group_remove_hero(g, slot);
			slot = group_free_slot(g_group);
			group_add_hero(g_group, h, slot);
		}
		g_nr_groups--;
		idx = group_find_mergeable();
	} while (idx != -1);
	if (g_pp20_index == 4)
		screen_portraits(g_screen);
}

struct group far *grp_choose(char far *title, register signed short n, signed short old, signed short unused)
{
	signed short r;
	register signed short sel = -1;

	g_hero_excluded = -1;
	if (n) {
		r = dlg_choose(g_dialog, title, n, g_menu_texts[0], g_menu_texts[1], g_menu_texts[2], g_menu_texts[3],
			g_menu_texts[4], g_menu_texts[5], g_menu_texts[6]) - 1;
		if (r != -2)
			sel = r;
		else
			sel = n - 1;
	}
	dlg_set16(g_dialog, old);
	mem_free(g_menu_buf);
	return sel == -1 ? 0 : g_menu_groups[sel];
}

void grp_menu_entry(struct group far *g, signed short k)
{
	signed short i;
	struct hero far *h;

	g_menu_texts[k] = g_menu_buf + k * 100;
	sprintf(g_menu_texts[k], g_ttx[0x1a3], k + 1);
	h = get_first_hero(g);
	i = 0;
	do {
		if (i)
			strcat(g_menu_texts[k], g_ttx[0x1a4]);
		strcat(g_menu_texts[k], h->name);
		h = get_next_hero(g, h);
		i++;
	} while (get_first_hero(g) != h);
	strcat(g_menu_texts[k], g_ttx[0x1a7]);
}

struct group far *grp_select(char far *title)
{
	signed short old;
	struct group far *g;
	signed short n;
	signed short i;

	g_menu_buf = (char far *)mem_alloc(600);
	old = dlg_set16(g_dialog, 6);
	for (n = 0, i = 0, g = g_groups; i < NR_GROUPS; i++, g = &g_groups[i]) {
		if (g_hero_excluded != i && !!g->nr_heroes && g != g_group) {
			grp_menu_entry(g, n);
			g_menu_groups[n] = g;
			n++;
		}
	}
	grp_menu_entry(g_group, n);
	g_menu_groups[n] = g_group;
	n++;
	return grp_choose(title, n, old, 1);
}

void group_switch_next(signed short mode)
{
	register signed short done = 0;
	struct group far *g;
	/* NSC-Platz: 0x20 Bytes vor g_heroes[6] (Original rechnet wohl mit 0x5eb Bytes je Held) */
#if FIX_0219
	/* Fix #219: NSC-Zeiger auf g_heroes[6] (0x239a lag 0x20 Bytes davor) */
	struct hero far *npc = &g_heroes[6];
#else
	struct hero far *npc = (struct hero far *)((char far *)g_heroes + 0x239a);
#endif
	signed short dungeon;
	signed short town;
	register signed short state;

	if (g_nr_groups == 1)
		return;
	dungeon = g_group->dungeon;
	town = g_group->town;
	do {
		g = grp_select(g_ttx[0x1a8]);
		state = 1;
		if (!group_count_check2b(g) || (group_count_check2b(g) == 1 && group_has_hero(g, npc) && check_hero(npc)))
			state = 0;
		else if (group_any_in_jail(g))
			state = 2;
		if (state == 0 || state == 2) {
			if (mode == 0) {
				if (state == 0)
					dlg_message(g_dialog, g_ttx[0x29]);
				else if (state == 2) {
					if (dlg_yes_no(g_dialog, g_ttx[0x28]) == 1) {
						while (group_any_in_jail(g))
							timewarp_until(g_date, 0xa8c0L);
						done = 1;
					}
				}
			}
		} else if (state || mode != 2)	/* Originalfehler: state ist hier immer 1 */
			done = 1;
	} while (!done);
	if (g != g_group) {
		if (g->dungeon != dungeon || g->town != town) {
			snd_obj_stop(g_snd_music);
			swap_out_screen();
			g_group = g;
			g_group_no = get_group_index(g);
			g_loaded_town = g_loaded_dungeon = -1;
			g_pp20_index = -1;
			g_refresh = 1;
			return;
		}
		g_group_no = get_group_index(g);
		g_group = g;
		if (g_pp20_index == 4)
			screen_portraits(g_screen);
	}
}

void swap_heroes_dialog(void)
{
	struct hero far *tmp;
	struct hero far *h1;
	struct hero far *h2;
	signed char m0;
	signed char m1;
	signed char m2;
	signed char m3;
	register signed short i1;
	register signed short i2;

	if (g_portrait_drag != 0) {
		if ((signed short)g_group->nr_heroes > 1) {
			h1 = h2 = 0;
			if ((signed short)g_group->nr_heroes == 2) {
				h1 = get_first_hero(g_group);
				h2 = get_second_active_hero(g_group);
			}
			if (!h1)
				h1 = get_any_hero(g_group, g_ttx[0x2a]);
			if (h1 && !h1->npc_id) {
				if (!h2)
					h2 = get_any_hero(g_group, g_ttx[0x2b]);
				if (h2 && h1 != h2 && !h2->npc_id) {
					tmp = (struct hero far *)mem_alloc(sizeof(struct hero));
					i1 = group_hero_pos(g_group, h1);
					i2 = group_hero_pos(g_group, h2);
					m0 = g_hero_slot_aux[0][i1];
					m1 = g_hero_slot_aux[1][i1];
					m2 = g_hero_slot_aux[2][i1];
					m3 = g_hero_slot_aux[3][i1];
					*tmp = *h1;
					*h1 = *h2;
					*h2 = *tmp;
					g_hero_slot_aux[0][i1] = g_hero_slot_aux[0][i2];
					g_hero_slot_aux[1][i1] = g_hero_slot_aux[1][i2];
					g_hero_slot_aux[2][i1] = g_hero_slot_aux[2][i2];
					g_hero_slot_aux[3][i1] = g_hero_slot_aux[3][i2];
					g_hero_slot_aux[0][i2] = m0;
					g_hero_slot_aux[1][i2] = m1;
					g_hero_slot_aux[2][i2] = m2;
					g_hero_slot_aux[3][i2] = m3;
					if (h1->type != HERO_TYPE_NONE)
						h1->action_id = 100;
					if (h2->type != HERO_TYPE_NONE)
						h2->action_id = 100;
					h1->set_group_pos(i1 + 1);
					h2->set_group_pos(i2 + 1);
					mem_free(tmp);
				}
			}
			screen_portraits(g_screen);
		}
	}
}

void drag_hero_portrait(signed short src)
{
	struct hero far *h2;
	struct hero far *h1;
	struct hero far *tmp;
	signed short dst;
	signed short cnt;
	signed char m0;
	signed char m1;
	signed char m2;
	signed char m3;
	Bmp far *under = new Bmp(0x20, 0x20);	/* Hintergrund unter dem gezogenen Portraet */
	Bmp far *portrait = new Bmp(0x20, 0x20);
	register signed short i1;
	register signed short i2;

	if (src != 6 && g_mouse_ldown != 0) {
		mouse_hide(0);
		g_mouse_min_y = 0x9d;
		g_mouse_max_y = 0xbd;
		g_mouse_min_x = 1;
		g_mouse_max_x = 0xf4;
		portrait->get(g_vga_buf, g_portrait_x[src], 0x9d);
		under->get(g_vga_buf, g_portrait_x[src], 0x9d);
		mouse_clear_left();
		g_mouse_ldown = 1;
		mouse_move(g_portrait_x[src], g_mouse_posy);
		g_mouse_posx_bak = g_mouse_posx;
		while (g_mouse_ldown) {
			if (g_mouse_posx != g_mouse_posx_bak) {
				wait_for_vsync();
				under->put(g_vga_buf, g_mouse_posx_bak, 0x9d);
				g_mouse_busy = 1;
				under->get(g_vga_buf, g_mouse_posx, 0x9d);
				portrait->put(g_vga_buf, g_mouse_posx, 0x9d);
				g_mouse_busy = 0;
				g_mouse_posx_bak = g_mouse_posx;
			}
		}
		mouse_clear_left();
		g_mouse_event = 0;
		under->put(g_vga_buf, g_mouse_posx_bak, 0x9d);
		/* Originalfehler: ohne Untergrenze (liest g_portrait_x[-1] ...) */
#if FIX_0062
		/* Fix #62: Untergrenze: nicht vor g_portrait_x[0] lesen */
		for (dst = 6; dst > 0 && g_portrait_x[--dst] > g_mouse_posx + 25; )
			;
#else
		for (dst = 6; g_portrait_x[--dst] > g_mouse_posx + 25; )
			;
#endif
		if (dst < 0)
			dst = 0;
		cnt = g_group->nr_heroes;
		if (group_get_npc(g_group))
			cnt--;
		if (src != dst && dst != 6 && dst < cnt) {
			tmp = (struct hero far *)mem_alloc(sizeof(struct hero));
			h2 = g_group->heroes[dst];
			h1 = g_group->heroes[src];
			i1 = src;
			i2 = dst;
			m0 = g_hero_slot_aux[0][i1];
			m1 = g_hero_slot_aux[1][i1];
			m2 = g_hero_slot_aux[2][i1];
			m3 = g_hero_slot_aux[3][i1];
			*tmp = *h1;
			*h1 = *h2;
			*h2 = *tmp;
			g_hero_slot_aux[0][i1] = g_hero_slot_aux[0][i2];
			g_hero_slot_aux[1][i1] = g_hero_slot_aux[1][i2];
			g_hero_slot_aux[2][i1] = g_hero_slot_aux[2][i2];
			g_hero_slot_aux[3][i1] = g_hero_slot_aux[3][i2];
			g_hero_slot_aux[0][i2] = m0;
			g_hero_slot_aux[1][i2] = m1;
			g_hero_slot_aux[2][i2] = m2;
			g_hero_slot_aux[3][i2] = m3;
			if (h1->type != HERO_TYPE_NONE)
				h1->action_id = 100;
			if (h2->type != HERO_TYPE_NONE)
				h2->action_id = 100;
			h1->set_group_pos(i1 + 1);
			h2->set_group_pos(i2 + 1);
			mem_free(tmp);
		}
		g_mouse_min_y = 0;
		g_mouse_max_y = 0xc7;
		g_mouse_min_x = 0;
		g_mouse_max_x = 0x13f;
		mouse_show(0);
		screen_portraits(g_screen);
	}
	delete under;
	delete portrait;
}
