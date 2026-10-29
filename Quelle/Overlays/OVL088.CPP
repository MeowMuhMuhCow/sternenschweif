/* OVL088 - Overlay O088 (Segment 0x2103 in SCHWEIF.EXE): Helden der Gruppe suchen und auswaehlen (struct group:
 * sieben Heldenzeiger heroes[], Anzahl nr_heroes). Zufaellig (random_schweif(7) - 1): get_random_alive_hero (nicht
 * tot), get_random_hero (handlungsfaehig, check_hero), get_random_hero2 (irgendeiner), get_random_asleep_hero
 * (alle schlafen: group_all_asleep). get_first_active_hero (sonst g_heroes), get_first_hero,
 * get_second_active_hero, get_most_wounded_hero (groesster LE-Verlust). Auswahllisten get_active_hero
 * (handlungsfaehig), get_active_mage (+ zauberkundig), get_any_hero (alle), get_active_hero_forced (Vorauswahl
 * merken, wiederholen bis gewaehlt), select_hero_ae (mit AE, g_fmt_name_ae), select_hero_le (mit LE,
 * g_fmt_name_le) - Namen nach g_menu_texts[], Helden nach g_radio_heroes[], ohne Held g_hero_excluded;
 * select_hero_from_list(g, titel, n, alt, flag) Auswahl dlg_choose mit den sieben Namen. group_get_npc (npc_id),
 * get_dead_hero, get_renegade_hero, get_diseased_hero (hero_is_diseased), get_mage_hero,
 * get_hero_brewing_in_inn (hero_brewing_in_inn(h, inn)) erster passender Held. get_next_mage/get_prev_mage/
 * get_next_hero/get_next_hero_buggy/get_prev_hero naechster/voriger Held (mit/ohne Zauberkunde).
 * get_random_hero_weapon_type(g, v)/get_random_hero_with_item(g, id, v): zufaelliger Held mit
 * hero_weapon_type2-Ergebnis bzw. Gegenstand id (get_item_pos, check_hero3). get_best_hero_skill/
 * get_worst_hero_skill bester/schlechtester Held fuer Talent (drei Eigenschaften aus g_skill_attribs +
 * get_skill_value), get_hero_attrib_max/get_hero_attrib_min Eigenschaft max/min, get_hero_rs_bonus_min,
 * get_hero_rs_be_max, get_hero_max_le (ohne NSC), get_hero_of_type, get_hero_staffspell (Stabzauberstufe >= v),
 * get_hero_with_item (group_find_item, sonst get_random_hero), get_hero_by_file_name (strcmp).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL088.CPP   -> 6237/6237 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (_g_radio_heroes -> _g_c8f2+2). Ohne -Og 226, ohne -Oi 948 Abweichungen (strcmp in get_hero_by_file_name).
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * - Schleifen "for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])"; Funktionen ohne Aufrufe haben i in CX.
 * - select_hero_from_list: register-Parameter flag (SI) + Local v OHNE register (DI); Dialogfeld +0x10 ueber
 *   die Teilsicht struct dlg_view und inline set_sel() (Kopie [bp-6]); Rueckgabe "r == -1 ? 0 : g_radio_heroes[r]".
 * - Max/Min-Suchen: "v = h->..; if (v > best)" im inneren if (mov di,ax / cmp di,[bp-2]); eingebettet in der
 *   &&-Kette ergibt cmp ax,[bp-2]. In get_most_wounded_hero dagegen eingebettet "(d = h->le_max - h->le) > best"
 *   (cmp ax,si).
 * - get_random_hero_weapon_type/get_random_hero_with_item: zwei if mit return ohne else (mit else zusaetzlicher
 *   jmp short).
 * - get_best_hero_skill/get_worst_hero_skill: i ohne register landet auf dem Stack ([bp-2]), SI dient als
 *   Rechenregister.
 * - Bitfelder unsigned short (frueher unsigned char): Wahrheitstests mit (signed char)-Cast.
 * Originalfehler/Auffaelligkeiten: get_next_hero_buggy setzt bei i == Anzahl "h = cur" und ueberschreibt es
 *   sofort mit g->heroes[i] (gemeint war wohl i = 0) - die Suche laeuft ueber die Gruppengroesse hinaus bis
 *   g->heroes[7] (hinter dem Feld). get_random_hero_weapon_type ruft hero_weapon_type2(h) vor dem Nulltest von h
 *   auf. get_random_hero_with_item liest h->inv[v] auch bei v == -1.
 */
#include <stdio.h>
#include <string.h>

#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:5c7e..5c9c, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
struct range8 g_school_spell_ranges[9] = { { 1, 5 }, { 6, 17 }, { 18, 26 }, { 27, 32 }, { 33, 37 }, { 38, 44 }, { 49, 57 }, { 60, 75 }, { 76, 85 } };
/* Phase B: eigenes BSS (ds:c8f4..c910) in Adressreihenfolge */
struct hero far *g_radio_heroes[7];

/* wirksamer Eigenschaftswert (aktuell + Modifikator) */
#define ATTRIB_EFF(h, k) ((signed char)((h)->attrib[k].cur + (h)->attrib[k].mod))

struct hero far *select_hero_from_list(struct group far *g, char far *title, signed short n, signed short old, signed short flag);


struct hero far *get_random_alive_hero(struct group far *g)
{
	struct hero far *h;
	register signed short i;

	if (g->nr_heroes) {
		do {
			i = random_schweif(7) - 1;
			h = g->heroes[i];
		} while (h == 0 || (signed char)h->dead);
	} else {
		h = 0;
	}
	return h;
}

struct hero far *get_random_hero(struct group far *g)
{
	struct hero far *h;
	register signed short i;

	h = 0;
	for (i = 0; g->nr_heroes > i; i++)
		if (check_hero(g->heroes[i]))
			h = g->heroes[i];
	if (g->nr_heroes && h) {
		do {
			i = random_schweif(7) - 1;
			h = g->heroes[i];
		} while (h == 0 || !check_hero(h));
	} else {
		h = 0;
	}
	return h;
}

struct hero far *get_random_hero2(struct group far *g)
{
	struct hero far *h;
	register signed short i;

	if (g->nr_heroes) {
		do {
			i = random_schweif(7) - 1;
			h = g->heroes[i];
		} while (h == 0);
	} else {
		h = 0;
	}
	return h;
}

struct hero far *get_first_active_hero(struct group far *g)
{
	struct hero far *h;
	register signed short i;

	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h && !(signed char)h->dead && check_hero(h))
			return h;
	return g_heroes;
}

struct hero far *get_first_hero(struct group far *g)
{
	struct hero far *h;
	register signed short i;

	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h)
			return h;
	return 0;
}

struct hero far *get_second_active_hero(struct group far *g)
{
	struct hero far *h;
	register signed short i;
	register signed short n;

	for (h = g->heroes[0], i = n = 0; i <= 6; h = g->heroes[++i])
		if (h && !(signed char)h->dead && check_hero(h) && n++)
			return h;
	return 0;
}

struct hero far *get_most_wounded_hero(struct group far *g)
{
	struct hero far *h;
	struct hero far *r;
	register signed short best = -1;
	register signed short i;
	register signed short d;

	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h && !(signed char)h->dead && (d = h->le_max - h->le) > best) {
			best = d;
			r = h;
		}
	return best == 0 ? 0 : r;
}

struct hero far *get_active_hero(struct group far *g, char far *title)
{
	signed short old;
	struct hero far *h;
	register signed short n;
	register signed short i;

	old = dlg_set16(g_dialog, 3);
	n = 0;
	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h && check_hero(h) && g_hero_excluded != i) {
			g_menu_texts[n] = h->name;
			g_radio_heroes[n] = h;
			n++;
		}
	return select_hero_from_list(g, title, n, old, 0);
}

struct hero far *get_active_mage(struct group far *g, char far *title)
{
	signed short old;
	struct hero far *h;
	register signed short n;
	register signed short i;

	old = dlg_set16(g_dialog, 3);
	n = 0;
	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h && check_hero(h) && g_hero_excluded != i && h->is_mage()) {
			g_menu_texts[n] = h->name;
			g_radio_heroes[n] = h;
			n++;
		}
	return select_hero_from_list(g, title, n, old, 0);
}

struct hero far *get_any_hero(struct group far *g, char far *title)
{
	signed short old;
	struct hero far *h;
	register signed short n;
	register signed short i;

	old = dlg_set16(g_dialog, 3);
	n = 0;
	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h && g_hero_excluded != i) {
			g_menu_texts[n] = h->name;
			g_radio_heroes[n] = h;
			n++;
		}
	return select_hero_from_list(g, title, n, old, 0);
}

struct hero far *get_active_hero_forced(struct group far *g, char far *title)
{
	signed short old;
	struct hero far *h;
	register signed short n;
	register signed short i;

	old = dlg_set16(g_dialog, 3);
	n = 0;
	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h && check_hero(h) && g_hero_excluded != i) {
			g_menu_texts[n] = h->name;
			g_radio_heroes[n] = h;
			n++;
		}
	return select_hero_from_list(g, title, n, old, 1);
}

struct hero far *select_hero_from_list(struct group far *g, char far *title, signed short n, signed short old, register signed short flag)
{
	signed short r = -1;
	signed short save;
	signed short v;

	g_hero_excluded = -1;
	if (n) {
		if (flag == 1)
			save = ((struct dlg_view far *)g_dialog)->sel;
		do {
			if (flag)
				((struct dlg_view far *)g_dialog)->set_sel(save);
			v = dlg_choose(g_dialog, title, n, g_menu_texts[0], g_menu_texts[1], g_menu_texts[2], g_menu_texts[3], g_menu_texts[4],
				g_menu_texts[5], g_menu_texts[6]) - 1;
		} while (v == -2 && flag == 1);
		if (v != -2)
			r = v;
	}
	dlg_set16(g_dialog, old);
	return r == -1 ? 0 : g_radio_heroes[r];
}

struct hero far *group_get_npc(struct group far *g)
{
	struct hero far *h;
	register signed short i;

	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h && h->npc_id != 0)
			return h;
	return 0;
}

struct hero far *get_dead_hero(struct group far *g)
{
	struct hero far *h;
	register signed short i;

	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h && (signed char)h->dead)
			return h;
	return 0;
}

struct hero far *get_renegade_hero(struct group far *g)
{
	struct hero far *h;
	register signed short i;

	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h && (signed char)h->renegade)
			return h;
	return 0;
}

struct hero far *get_diseased_hero(struct group far *g)
{
	struct hero far *h;
	register signed short i;

	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h && hero_is_diseased(h))
			return h;
	return 0;
}

struct hero far *get_mage_hero(struct group far *g)
{
	struct hero far *h;
	register signed short i;

	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h && h->is_mage())
			return h;
	return 0;
}

struct hero far *get_hero_brewing_in_inn(struct group far *g, register signed short inn)
{
	struct hero far *h;
	register signed short i;

	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h && hero_brewing_in_inn(h, inn))
			return h;
	return 0;
}

struct hero far *get_next_mage(struct group far *g, struct hero far *cur)
{
	struct hero far *h;
	register signed short i;

	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h == cur)
			break;
	h = 0;
	do {
		i++;
		if (g->nr_heroes == i)
			i = 0;
		h = g->heroes[i];
	} while (h == 0 || h->type < HERO_TYPE_WITCH);
	return h;
}

struct hero far *get_prev_mage(struct group far *g, struct hero far *cur)
{
	struct hero far *h;
	register signed short i;

	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h == cur)
			break;
	h = 0;
	do {
		i--;
		if (i == -1)
			i = g->nr_heroes - 1;
		h = g->heroes[i];
	} while (h == 0 || h->type < HERO_TYPE_WITCH);
	return h;
}

struct hero far *get_next_hero(struct group far *g, struct hero far *cur)
{
	struct hero far *h;
	register signed short i;

	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h == cur)
			break;
	h = 0;
	do {
		i++;
		if (g->nr_heroes == i)
			i = 0;
		h = g->heroes[i];
	} while (h == 0);
	return h;
}

struct hero far *get_next_hero_buggy(struct group far *g, struct hero far *cur)
{
	struct hero far *h;
	register signed short i;

	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h == cur)
			break;
	h = 0;
	do {
		i++;
#if FIX_0171
		/* Fix #171: am Gruppenende vorn weitersuchen (i = 0 statt h = cur) */
		if (g->nr_heroes == i)
			i = 0;
#else
		if (g->nr_heroes == i)
			h = cur;
#endif
		h = g->heroes[i];
	} while (h == 0);
	return h;
}

struct hero far *get_prev_hero(struct group far *g, struct hero far *cur)
{
	struct hero far *h;
	register signed short i;

	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h == cur)
			break;
	h = 0;
	do {
		i--;
		if (i == -1)
			i = g->nr_heroes - 1;
		h = g->heroes[i];
	} while (h == 0);
	return h;
}

struct hero far *get_random_hero_weapon_type(struct group far *g, signed short v)
{
	struct hero far *h;
	signed short match;
	signed short any;
	register signed short i;
	register signed short t;

	match = any = 0;
	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (check_hero3(h)) {
#if FIX_0171
			/* Fix #171: Nulltest vor hero_weapon_type2 */
			t = h != 0 ? hero_weapon_type2(h) : -1;
#else
			t = hero_weapon_type2(h);
#endif
			if (t >= 0)
				any = 1;
			if (t == v)
				match = 1;
		}
	if (g->nr_heroes >= 1) {
		if (v == -1 && any) {
			do {
				i = random_schweif(7) - 1;
				h = g->heroes[i];
#if FIX_0171
				/* Fix #171: Nulltest vor hero_weapon_type2 */
				t = h != 0 ? hero_weapon_type2(h) : -1;
#else
				t = hero_weapon_type2(h);
#endif
			} while (h == 0 || !check_hero3(h) || t == -1);
			return h;
		}
		if (v != 0 && match) {
			do {
				i = random_schweif(7) - 1;
				h = g->heroes[i];
				t = hero_weapon_type2(h);
			} while (h == 0 || !check_hero3(h) || t != v);
			return h;
		}
	}
	h = 0;
	return h;
}

struct hero far *get_random_hero_with_item(struct group far *g, register signed short id, signed short v)
{
	signed short r;
	struct hero far *h;
	signed char found;
	signed char found2;
	register signed short i;

	found = 0;
	found2 = 0;
	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h) {
			if (get_item_pos(h, id) >= 0 && check_hero3(h))
				found = 1;
#if FIX_0171
			/* Fix #171: inv[v] nur bei v >= 0 lesen */
			if (v >= 0 && h->inv[v].item_id == id && check_hero3(h))
#else
			if (h->inv[v].item_id == id && check_hero3(h))
#endif
				found2 = 1;
		}
	if (g->nr_heroes >= 1) {
		if (v == -1 && found == 1) {
			do {
				i = random_schweif(7) - 1;
				h = g->heroes[i];
			} while (h == 0 || !check_hero3(h) || get_item_pos(h, id) < 0);
			return h;
		}
		if (v >= 0 && found2 == 1) {
			do {
				i = random_schweif(7) - 1;
				h = g->heroes[i];
#if FIX_0171
				/* Fix #171: Nulltest vor get_item_pos */
				r = h != 0 ? get_item_pos(h, id) : -1;
#else
				r = get_item_pos(h, id);
#endif
			} while (h == 0 || !check_hero3(h) || r != v);
			return h;
		}
	}
	h = 0;
	return h;
}

struct hero far *get_best_hero_skill(struct group far *g, register signed short skill)
{
	signed short i;
	signed short sum;
	signed short best = -1;
	struct hero far *h;
	struct hero far *r;

	r = get_first_active_hero(g);
	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h && check_hero3(h)) {
			sum = ATTRIB_EFF(h, g_skill_attribs[skill].a1) + ATTRIB_EFF(h, g_skill_attribs[skill].a2) + ATTRIB_EFF(h, g_skill_attribs[skill].a3) + get_skill_value(h, skill);
			if (sum > best) {
				best = sum;
				r = h;
			}
		}
	return r;
}

struct hero far *get_worst_hero_skill(struct group far *g, register signed short skill)
{
	signed short i;
	signed short sum;
	signed short best = 999;
	struct hero far *h;
	struct hero far *r;

	r = get_first_active_hero(g);
	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h && check_hero3(h)) {
			sum = ATTRIB_EFF(h, g_skill_attribs[skill].a1) + ATTRIB_EFF(h, g_skill_attribs[skill].a2) + ATTRIB_EFF(h, g_skill_attribs[skill].a3) + get_skill_value(h, skill);
			if (sum < best) {
				best = sum;
				r = h;
			}
		}
	return r;
}

struct hero far *get_hero_attrib_max(struct group far *g, signed short a)
{
	signed short best = -1;
	struct hero far *h;
	struct hero far *r;
	register signed short i;
	register signed short v;

	r = get_first_active_hero(g);
	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h && check_hero3(h)) {
			v = h->attrib[a].cur;
			if (v > best) {
				best = v;
				r = h;
			}
		}
	return r;
}

struct hero far *get_hero_attrib_min(struct group far *g, signed short a)
{
	signed short best = 999;
	struct hero far *h;
	struct hero far *r;
	register signed short i;
	register signed short v;

	r = get_first_active_hero(g);
	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h && check_hero3(h)) {
			v = h->attrib[a].cur;
			if (v < best) {
				best = v;
				r = h;
			}
		}
	return r;
}

struct hero far *get_hero_rs_bonus_min(struct group far *g)
{
	signed short best = 999;
	struct hero far *h;
	struct hero far *r;
	register signed short i;
	register signed short v;

	r = get_first_active_hero(g);
	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h && check_hero3(h)) {
			v = h->rs_bonus;
			if (v < best) {
				best = v;
				r = h;
			}
		}
	return r;
}

struct hero far *get_hero_rs_be_max(struct group far *g)
{
	signed short best = 0;
	struct hero far *h;
	struct hero far *r;
	register signed short i;
	register signed short v;

	r = get_first_active_hero(g);
	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h && check_hero3(h)) {
			v = h->rs_be;
			if (v > best) {
				best = v;
				r = h;
			}
		}
	return r;
}

struct hero far *get_hero_of_type(struct group far *g, signed char type)
{
	struct hero far *h = 0;
	register signed short i;

	for (i = 0; g->nr_heroes > i; i++)
		if (check_hero3(g->heroes[i]) && g->heroes[i]->type == type)
			return g->heroes[i];
	return h;
}

struct hero far *get_random_asleep_hero(struct group far *g)
{
	struct hero far *h;
	register signed short i;

	if (g->nr_heroes && group_all_asleep(g)) {
		do {
			i = random_schweif(7) - 1;
			h = g->heroes[i];
		} while (h == 0 || (signed char)h->dead || !(signed char)h->asleep);
	} else {
		h = 0;
	}
	return h;
}

struct hero far *get_hero_staffspell(struct group far *g, signed char v)
{
	struct hero far *h;
	struct hero far *r = 0;
	register signed short i;

	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h && check_hero(h) && h->staffspell_lvl >= v)
			r = h;
	return r;
}

struct hero far *get_hero_with_item(struct group far *g, signed short id)
{
	signed short k;
	struct hero far *h = 0;

	if ((k = group_find_item(g, id)) != -1) {
		h = g->heroes[k];
		if (!check_hero3(h))
			h = get_random_hero(g);
	}
	return h;
}

struct hero far *select_hero_ae(struct group far *g, char far *title)
{
	signed short n;
	signed short old;
	struct hero far *h;
	register signed short i;

	old = dlg_set16(g_dialog, 3);
	n = 0;
	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h && check_hero(h) && g_hero_excluded != i && h->is_mage()) {
			sprintf(g_text_output_buf + i * 100, "%s~%d", h->name, h->ae);
			g_menu_texts[n] = g_text_output_buf + i * 100;
			g_radio_heroes[n] = h;
			n++;
		}
	return select_hero_from_list(g, title, n, old, 0);
}

struct hero far *select_hero_le(struct group far *g, char far *title)
{
	signed short n;
	signed short old;
	struct hero far *h;
	register signed short i;

	old = dlg_set16(g_dialog, 3);
	n = 0;
	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h && check_hero(h) && g_hero_excluded != i) {
			sprintf(g_text_output_buf + i * 100, "%s~%d", h->name, h->le);
			g_menu_texts[n] = g_text_output_buf + i * 100;
			g_radio_heroes[n] = h;
			n++;
		}
	return select_hero_from_list(g, title, n, old, 0);
}

struct hero far *get_hero_by_file_name(struct group far *g, char far *name)
{
	signed short i;
	struct hero far *h;

	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h && !strcmp(h->file_name, name))
			return h;
	return 0;
}

struct hero far *get_hero_max_le(struct group far *g)
{
	signed short best = 0;
	struct hero far *h;
	struct hero far *r;
	register signed short i;
	register signed short v;

	r = get_first_active_hero(g);
	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h && !h->npc_id) {
			v = h->le;
			if (v > best) {
				best = v;
				r = h;
			}
		}
	return r;
}
