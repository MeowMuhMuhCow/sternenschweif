/* OVL203 - Overlay O203 (Segment 0x2378 in SCHWEIF.EXE): Gruppenschleifen ueber die 7 Heldenzeiger einer Gruppe g
 * (freie Funktionen group_*, reines C). group_free_capacity: Summe der freien Traglast (signed char)(KK cur + mod) * 100
 * - load. group_find_item_box/group_find_key(g, id): Held ueber get_first_hero/get_next_hero (Ringdurchlauf)
 * suchen, fuer den hero_find_item_box bzw. hero_find_key != -1 liefert -> group_hero_pos(g, h), sonst -1.
 * group_all_asleep: 1, solange alle Helden schlafen (Bit asleep). count_failed_skill(g, skill, mod): Anzahl
 * test_skill < 0. group_count_bd2: Anzahl mit Bit bd2. group_kill_but_bd2/group_kill_all: sub_hero_le(h, h->le) fuer
 * alle (ohne bzw. mit Bit bd2). hero_swap(_, h1, h2): zwei Helden per Strukturkopie tauschen (nur wenn
 * g_group->nr_heroes > 1, beide npc_id == 0). group_test_attrib_disease/_msg/_dmg: Probe test_attrib(attrib, mod) fuer
 * alle Helden mit check_hero3, Meldung sprintf(g_text_output_buf, fmt, Name) + dlg_message, bei Misserfolg Schaden
 * bzw. Abzug (sub_hero_le, get_item_pos/drop_item_at, hero_disease_test(ill, chance)) und Zaehler (signed char).
 * group_count_item(g, id): Summe hero_count_item (+1 bei id == 0x79 und staffspell_lvl >= 3). group_mr_bonus: Summe
 * hero_mr_bonus (Abbruch bei -5), hoechstens 5. group_take_item(g, id): Gegenstand id (sonst den Schluesselbund
 * ITEM_KEYRING) per group_find_item/get_item_pos finden und drop_item bzw. hero_take_key anwenden.
 *
 * BCC -mlarge -3 -Ot -c -Y OVL203.C   -> 2263/2263 Bytes GLEICH (18.10.2026), cobjfix laeuft (mit und ohne -Og gleich)
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Reines C. Gruppenschleife "for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])"; Zaehler i ohne Aufrufe in CX.
 * - group_count_bd2: "for (i = 0, n = 0, h = ...)" (xor cx,cx vor xor si,si); sonst "n = 0;" vor der Schleife.
 * - Zuweisungen getrennt vor dem Vergleich ("v = f(); if (v != -1)") -> cmp si,-1; eingebettet -> cmp ax,-1 bzw.
 *   bei Zeigern "or dx,ax" statt "cmp dword [bp-4],0" (deshalb verschachtelte ifs).
 * - "(signed short)g_group->nr_heroes > 1" -> mov al/cbw/cmp ax,1 (ohne Cast cmp byte).
 * - signed-char-Zaehler "failed = failed + 1" (mov al/inc al/mov), "dmg = 0 - dmg" (xor ax,ax/sub ax,si statt neg).
 * - hero_swap: Parameter 1 unbenutzt (Warnung); get_item_pos liefert short (in group_test_attrib_disease auf signed
 *   char gekuerzt). test_attrib liefert short, das Original deklarierte signed char - gleiche Bytes.
 * - group_test_attrib_disease: die letzten beiden Parameter (ill, chance) gehen unveraendert an hero_disease_test;
 *   frueher als ein far-Zeiger geschrieben (gleiche push-Folge).
 */
#include <stdio.h>
#include "schweif.h"

signed short group_free_capacity(struct group far *g)
{
	struct hero far *h;
	signed short sum;
	register signed short load;
	register signed short max_load;
	register signed short i;

	sum = 0;
	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h) {
			load = h->load;
			max_load = (signed char)(h->attrib[ATTRIB_KK].cur + h->attrib[ATTRIB_KK].mod) * 100;
			if (load < max_load)
				sum += max_load - load;
		}
	return sum;
}

signed short group_find_item_box(struct group far *g, signed short id)
{
	struct hero far *h;
	register signed short pos = -1;
	register signed short ret = -1;

	h = get_first_hero(g);
	do {
		pos = hero_find_item_box(h, id);
		if (pos != -1) {
			ret = group_hero_pos(g, h);
			break;
		}
		h = get_next_hero(g, h);
	} while (get_first_hero(g) != h);
	return ret;
}

signed short group_find_key(struct group far *g, signed short id)
{
	struct hero far *h;
	register signed short pos = -1;
	register signed short ret = -1;

	h = get_first_hero(g);
	do {
		pos = hero_find_key(h, id);
		if (pos != -1) {
			ret = group_hero_pos(g, h);
			break;
		}
		h = get_next_hero(g, h);
	} while (get_first_hero(g) != h);
	return ret;
}

signed short group_all_asleep(struct group far *g)
{
	struct hero far *h;
	register signed short ret = 1;

	h = get_first_hero(g);
	do {
		ret = (signed char)h->asleep;
		h = get_next_hero(g, h);
	} while (get_first_hero(g) != h && ret);
	return ret;
}

signed short count_failed_skill(struct group far *g, signed short skill, signed char mod)
{
	struct hero far *h;
	register signed short i;
	register signed short n;

	n = 0;
	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h && test_skill(h, skill, mod) < 0)
			n++;
	return n;
}

signed short group_count_bd2(struct group far *g)
{
	struct hero far *h;
	register signed short n;
	register signed short i;

	for (i = 0, n = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h && h->bd2 == 1)
			n++;
	return n;
}

void group_kill_but_bd2(struct group far *g)
{
	struct hero far *h;
	register signed short i;

	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h && h->bd2 != 1)
			sub_hero_le(h, h->le);
}

void group_kill_all(struct group far *g)
{
	struct hero far *h;
	register signed short i;

	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h)
			sub_hero_le(h, h->le);
}

void hero_swap(struct group far *unused, struct hero far *h1, struct hero far *h2)
{
	struct hero far *tmp;

	if ((signed short)g_group->nr_heroes > 1 && !h1->npc_id && !h2->npc_id && h1 && h2) {
		tmp = (struct hero far *)mem_alloc((signed long)sizeof(struct hero));
		*tmp = *h2;
		*h2 = *h1;
		*h1 = *tmp;
		mem_free(tmp);
	}
}

signed char group_test_attrib_disease(struct group far *g, register signed short attrib, signed short mod,
	char far *fmt_ok, char far *fmt_fail, register signed short dmg, signed short ill, signed short chance)
{
	signed char i;
	signed char res;
	signed char pos;
	struct hero far *h;
	signed char failed;

	failed = 0;
	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i]) {
		if (h && check_hero3(h)) {
			res = test_attrib(h, attrib, mod);
			if (res <= 0) {
				sprintf(g_text_output_buf, fmt_fail, h->name);
				dlg_message(g_dialog, g_text_output_buf);
				if (dmg <= 0) {
					/* dmg <= 0: LE-Verlust -dmg, sonst Gegenstand dmg ablegen */
					dmg = 0 - dmg;
					sub_hero_le(h, dmg);
				} else {
					pos = get_item_pos(h, dmg);
					if (pos >= 0)
						drop_item_at(h, pos);
				}
				hero_disease_test(h, ill, chance);
				failed = failed + 1;
			} else {
				sprintf(g_text_output_buf, fmt_ok, h->name);
				dlg_message(g_dialog, g_text_output_buf);
			}
		}
	}
	return failed;
}

signed char group_test_attrib_msg(struct group far *g, register signed short attrib, register signed short mod,
	char far *fmt_ok, char far *fmt_fail, signed short dmg)
{
	signed char i;
	signed char res;
	struct hero far *h;
	signed char failed;

	failed = 0;
	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i]) {
		if (h && check_hero3(h)) {
			res = test_attrib(h, attrib, mod);
			if (res <= 0) {
				sprintf(g_text_output_buf, fmt_fail, h->name);
				dlg_message(g_dialog, g_text_output_buf);
				sub_hero_le(h, dmg);
				failed = failed + 1;
			} else {
				sprintf(g_text_output_buf, fmt_ok, h->name);
				dlg_message(g_dialog, g_text_output_buf);
			}
		}
	}
	return failed;
}

signed char group_test_attrib_dmg(struct group far *g, register signed short attrib, register signed short mod,
	signed short dmg)
{
	signed char i;
	signed char res;
	struct hero far *h;
	signed char failed;

	failed = 0;
	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i]) {
		if (h && check_hero3(h)) {
			res = test_attrib(h, attrib, mod);
			if (res <= 0) {
				sub_hero_le(h, dmg);
				failed = failed + 1;
			}
		}
	}
	return failed;
}

signed short group_count_item(struct group far *g, signed short id)
{
	struct hero far *h;
	register signed short sum;
	register signed short i;

	sum = 0;
	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h) {
			if (id == 0x79 && h->staffspell_lvl >= 3)
				sum++;
			sum += hero_count_item(h, id);
		}
	return sum;
}

signed short group_mr_bonus(struct group far *g)
{
	struct hero far *h;
	signed short bonus;
	register signed short sum;
	register signed short i;

	sum = 0;
	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h) {
			bonus = hero_mr_bonus(h);
			if (bonus == -5) {
				sum = -5;
				break;
			} else
				sum += bonus;
		}
	if (sum > 5)
		sum = 5;
	return sum;
}

signed short group_take_item(struct group far *g, signed short id)
{
	struct hero far *h;
	signed short ret;
	register signed short slot;
	register signed short pos;

	ret = 0;
	slot = group_find_item(g, id);
	if (slot != -1) {
		h = g->heroes[slot];
		if (h) {
			pos = get_item_pos(h, id);
			if (pos != -1)
				ret = drop_item(h, pos, 1);
		}
	}
	if (!ret) {
		slot = group_find_item(g, ITEM_KEYRING);
		if (slot != -1) {
			h = g->heroes[slot];
			if (h) {
				pos = get_item_pos(h, ITEM_KEYRING);
				if (pos != -1)
					ret = hero_take_key(h, pos, id);
			}
		}
	}
	return ret;
}
