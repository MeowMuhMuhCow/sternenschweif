/* OVL202 - Overlay O202 (Segment 0x236d in SCHWEIF.EXE): dritter Teil der Gruppenklasse struct group (siehe O200/O201;
 * Heldenzeiger heroes[7], Schleifen "for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i])").
 * refill_water: bei allen Helden thirst = 0 und an Wasserschlaeuchen (0x1e) die Bits empty/half_empty loeschen,
 * Rueckgabe gefunden. add_ap(n)/add_ap_each(n)/sub_ap_each(n): fuer alle add_hero_ap (n / count_alive bzw. n) bzw.
 * sub_hero_ap, nur n >= 0. free_slot: erster freier Platz (-1). add_hero(x, slot): Held x in die Gruppe (nr_heroes++,
 * g_group_sizes[Gruppe]++); ist schon ein NSC (group_get_npc) da, tauscht er den Platz (group_pos Platz + 1,
 * group_no Gruppe, fill_saved_values), danach add_money(x->money) und hero_save_file, wenn npc_id == 0.
 * dismiss_hero(slot, v)/remove_hero(slot): Held entfernen (Anteil money_share -> money, group_no/group_pos = 0,
 * dismiss_hero zusaetzlich hero_dismiss(v)), sub_money, compact. inc_skill(skill)/dec_weapons_bf: hero_inc_skill
 * bzw. hero_dec_weapons_bf fuer alle. mark_weapon(fmt, mode): hero_mark_weapon fuer die Lebenden bis zum ersten
 * Treffer (kein Ergebnis). kk_at_least(n)/kk2_at_least(n): KK (cur + mod, signed char) von get_first_active_hero
 * (plus get_second_active_hero) >= n. reset_rest: watches = 0, Bits rest_magic..rest_herbs loeschen. sleep(mod): hero_sleep
 * bei watches < 2 und !rest_magic. give_item(id, n): n Stueck des Gegenstands an die Gruppe verteilen (0x161/0x162/0x163 ->
 * 10/13/40 mit 200/50/20 Stueck; 0x2d bei mehreren Helden zuerst je 1 Stueck an get_random_hero2, hoechstens 3n
 * Versuche), dann give_hero_new_item der Reihe nach; passt der Rest nicht -> Frage g_ttx[0xe8] (dlg_yes_no), Held
 * get_active_hero(g_ttx[0xe9]) legt per select_item(g_ttx[0xf0]) etwas ab (drop_item) und es geht von vorn los;
 * Rueckgabe verteilte Anzahl. give_item_each(id, n): JEDER Held bekommt n Stueck (Frage g_ttx[0x195] mit
 * Heldenname). mage_test_spell(spell, mod): Zauberkundiger (is_mage), check_hero und test_spell(spell, mod) > 0?
 * drop_item(id)/remove_item(id): Gegenstand beim Helden group_find_item per get_item_pos suchen, 1 Stueck ablegen
 * drop_item(.., 1) bzw. hero_remove_item (Rueckgabe 1/0). drop_most_items: Gegenstaende ohne g_items[id].magic
 * ablegen (drop_item_at), ausser Plaetze 5/14 (sex != 0: auch 10). lose_random_items(n): n zufaellige Gegenstaende
 * random_schweif(g_nr_items - 1) ablegen. count_free_knapsack: freie Rucksackplaetze 15..30 aller Helden.
 * save/load: Gruppe im Spielstand g_save_fd schreiben/lesen ("PART" g_str_part, Laenge 0x49, Struktur;
 * Heldenzeiger als Index (h - g_heroes) + 4). any_in_jail: ein Held mit jail? has_dwarf: Zwerg mit check_hero3?
 * has_elf: Typ 10..12?
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL202.CPP   -> 4234/4234 Bytes GLEICH (18.10.2026), cobjfix laeuft.
 *   Ohne -Og ebenfalls GLEICH; ohne -Oi 3 Bytes zu kurz (memcpy in save), mit -2 viel zu lang (mov eax).
 *   (Namensfassung mit schweif.h, Methoden an struct group: 19.10.2026 GLEICH, LINKGLEICH)
 * - C++: inline-Elementfunktionen der Heldenstruktur erklaeren die Register bzw. Locals: clr_inv_empty(j)/
 *   clr_inv_half_empty(j) in refill_water (Parameterkopien von j in SI und DI, j selbst im inneren Block -> CX),
 *   set_group_pos/set_group_no(signed short) in add_hero ([bp-0ch]..[bp-14h], mov ax / mov [bp-x],ax / mov al,[bp-x]),
 *   set_money(long) in dismiss_hero/remove_hero ([bp-10h]), is_mage() "return type >= 7;" in mage_test_spell
 *   (mov ax,1 / xor ax,ax / or ax,ax).
 * - Registerverteilung: ein register-Local bekommt SI VOR dem register-Parameter (give_item, drop_item/remove_item) -
 *   Local ohne register lassen, dann DI. Ohne register vergibt BCC Register in Deklarationsreihenfolge: in
 *   drop_item/remove_item muss slot VOR pos stehen (sonst bekommt pos DI). In aufruffreien Funktionen
 *   (count_free_knapsack, any_in_jail) ganz ohne register: erste Variable SI, zweite DI bzw. CX, Block-Local CX.
 * - give_item: "max_tries = n * 3" -> mov ax,si / shl ax,1 / add ax,si. "(n > 1 ? 4 : 0) + 2" als erstes Argument.
 * - save: Zeiger -> Index "h - g_heroes + 4" (far-Zeigerdifferenz als long: xor dx / sub / sbb / idiv ebx,
 *   Ergebnis long -> far-Zeiger per shld); load: "g_heroes + (signed short)h - 4" (add bx,0e844h). Zaehler in load
 *   unsigned (jc), dieselbe Variable wie die gelesene Laenge. memcpy(saved, heroes) mit -Oi -> rep movsw.
 * - hero_dismiss: Original uebergibt v als short (push word), s_proto.h hat signed char (mov al / push ax) ->
 *   Aufruf ueber gecasteten Funktionszeiger. Gleichnamige freie Funktion drop_item mit "::" aufrufen.
 * Originalfehler: lose_random_items setzt slot nur vor der Schleife auf -1 - nur der erste Gegenstand wird
 *   ausgelost, alle weiteren Durchlaeufe nehmen denselben Helden und dieselbe Nummer (fehlt sie, drop_item_at(h, -1)).
 *   give_item_each gibt nach dem Platzschaffen wieder n Stueck (nicht den Rest n - got) - ein Held kann so mehr als
 *   n Stueck bekommen.
 *   add_hero/dismiss_hero/remove_hero: ohne Treffer in g_groups wird g_group_sizes[6] veraendert (wie O200
 *   remove_npc).
 */
#include <stdio.h>
#include <string.h>
#include <io.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:704c..7052, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */

signed short group::refill_water()
{
	signed short i;
	struct hero far *h;
	signed short found;

	found = 0;
	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0) {
			signed short j;

			h->thirst = 0;
			for (j = 0; j < NR_INV_SLOTS; j++) {
				if (h->inv[j].item_id == ITEM_WATERSKIN) {
					h->clr_inv_empty(j);
					h->clr_inv_half_empty(j);
					found = 1;
				}
			}
		}
	}
	return found;
}

void group::add_ap(register signed short ap)
{
	struct hero far *h;
	register signed short i;

	if (ap >= 0) {
		ap /= count_alive();
		for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
			if (h != 0)
				add_hero_ap(h, ap);
		}
	}
}

void group::add_ap_each(register signed short ap)
{
	struct hero far *h;
	register signed short i;

	if (ap >= 0) {
		for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
			if (h != 0)
				add_hero_ap(h, ap);
		}
	}
}

void group::sub_ap_each(register signed short ap)
{
	struct hero far *h;
	register signed short i;

	if (ap >= 0) {
		for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
			if (h != 0)
				sub_hero_ap(h, ap);
		}
	}
}

signed short group::free_slot()
{
	struct hero far *h;
	signed short i;

	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h == 0)
			return i;
	}
	return -1;
}

void group::add_hero(struct hero far *x, register signed short slot)
{
	signed short pos;
	struct group far *g;
	struct hero far *npc;
	register signed short i;

	for (i = 0, g = g_groups; i < NR_GROUPS; i++, g++) {
		if (g == this)
			break;
	}
	nr_heroes++;
#if FIX_0083
	/* Fix #83: nur eine gefundene Gruppe vergroessern */
	if (i < NR_GROUPS)
		g_group_sizes[i]++;
#else
	g_group_sizes[i]++;
#endif
	npc = group_get_npc(this);
	if (npc != 0) {
		/* der NSC gibt seinen Platz an den neuen Helden ab */
		pos = hero_pos(npc);
		heroes[slot] = npc;
		npc->set_group_pos(slot + 1);
		x->set_group_pos(pos + 1);
		x->set_group_no(i);
		heroes[pos] = x;
		fill_saved_values(npc, -1);
	} else {
		x->set_group_pos(slot + 1);
		x->set_group_no(i);
		heroes[slot] = x;
	}
	fill_saved_values(x, -1);
	if (!x->npc_id) {
		add_money(x->money);
		hero_save_file(x);
	}
}

void group::dismiss_hero(register signed short slot, signed short v)
{
	signed long share;
	struct hero far *h;
	struct group far *g;
	register signed short i;

	for (i = 0, g = g_groups; i < NR_GROUPS; i++, g++) {
		if (g == this)
			break;
	}
	share = money_share();
	h = heroes[slot];
	nr_heroes--;
#if FIX_0083
	/* Fix #83: nur eine gefundene Gruppe verkleinern */
	if (i < NR_GROUPS)
		g_group_sizes[i]--;
#else
	g_group_sizes[i]--;
#endif
	h->group_no = 0;
	h->group_pos = 0;
#if FIX_0356
	/* Fix #356: der NSC bekommt keinen Anteil am Gruppengeld und behaelt sein eigenes Geld */
	if (h->npc_id)
		share = 0;
	else
		h->set_money(share);
#else
	h->set_money(share);
#endif
	/* Original uebergibt v als short (s_proto.h: signed char wie die Definition in O074) */
	((void (*)(struct hero far *, signed short))hero_dismiss)(h, v);
	heroes[slot] = 0;
	sub_money(share);
	compact();
}

void group::remove_hero(register signed short slot)
{
	signed long share;
	struct hero far *h;
	struct group far *g;
	register signed short i;

	for (i = 0, g = g_groups; i < NR_GROUPS; i++, g++) {
		if (g == this)
			break;
	}
	share = money_share();
	h = heroes[slot];
	nr_heroes--;
#if FIX_0083
	/* Fix #83: nur eine gefundene Gruppe verkleinern */
	if (i < NR_GROUPS)
		g_group_sizes[i]--;
#else
	g_group_sizes[i]--;
#endif
	h->group_no = 0;
	h->group_pos = 0;
#if FIX_0356
	/* Fix #356: der NSC bekommt keinen Anteil am Gruppengeld und behaelt sein eigenes Geld */
	if (h->npc_id)
		share = 0;
	else
		h->set_money(share);
#else
	h->set_money(share);
#endif
	heroes[slot] = 0;
	sub_money(share);
	compact();
}

void group::inc_skill(register signed short skill)
{
	struct hero far *h;
	register signed short i;

	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0)
			hero_inc_skill(h, skill);
	}
}

void group::dec_weapons_bf()
{
	struct hero far *h;
	register signed short i;

	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0)
			hero_dec_weapons_bf(h);
	}
}

void group::mark_weapon(char far *fmt, signed short mode)
{
	struct hero far *h;
	register signed short i;
	register signed short done;

	for (i = done = 0, h = heroes[0]; i <= 6 && !done; h = heroes[++i]) {
		if (h != 0 && !(signed char)h->dead)
			done = hero_mark_weapon(h, fmt, mode);
	}
}

signed short group::kk_at_least(signed short n)
{
	struct hero far *h;

	h = get_first_active_hero(this);
	return (signed char)(h->attrib[ATTRIB_KK].cur + h->attrib[ATTRIB_KK].mod) >= n ? 1 : 0;
}

signed short group::kk2_at_least(signed short n)
{
	struct hero far *h;
	struct hero far *h2;
	register signed short kk;

	h = get_first_active_hero(this);
	h2 = get_second_active_hero(this);
	kk = (signed char)(h->attrib[ATTRIB_KK].cur + h->attrib[ATTRIB_KK].mod);
	if (h2 != 0)
		kk += (signed char)(h2->attrib[ATTRIB_KK].cur + h2->attrib[ATTRIB_KK].mod);
	return kk >= n ? 1 : 0;
}

void group::reset_rest()
{
	struct hero far *h;
	signed short i;

	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0) {
			h->watches = 0;
			h->rest_magic = 0;
			h->rest_food = 0;
			h->rest_herbs = 0;
		}
	}
}

void group::sleep(register signed short mod)
{
	struct hero far *h;
	register signed short i;

	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0 && h->watches < 2 && !(signed char)h->rest_magic)
			hero_sleep(h, mod);
	}
}

signed short group::give_item(signed short id, register signed short n)
{
	signed short i;
	signed short total;
	signed short done;
	struct hero far *h;
	signed short got;

	total = 0;
	done = 0;
	/* Sammelposten: Gegenstand und Stueckzahl */
	if (id == 0x161) {
		id = 10;
		n = 200;
	} else if (id == 0x162) {
		id = 13;
		n = 50;
	} else if (id == 0x163) {
		id = 40;
		n = 20;
	}
	do {
		if (id == 0x2d && n > 1 && nr_heroes > 1) {
			/* zuerst einzeln an zufaellige Helden */
			signed short tries;
			signed short max_tries;
			signed short stop;

			tries = 0;
			max_tries = n * 3;
			stop = 0;
			do {
				if (++tries < max_tries && n > 0) {
					h = get_random_hero2(this);
					if ((got = give_hero_new_item(h, id, 0, 1)) > 0) {
						n -= got;
						total += got;
					}
				} else
					stop = 1;
			} while (!stop);
		}
		for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
			if (h != 0) {
				while (n > 0 && (got = give_hero_new_item(h, id, 0, n)) > 0) {
					n -= got;
					total += got;
				}
			}
		}
		if (n > 0) {
			/* kein Platz mehr: fragen, ob ein Held etwas ablegt */
			sprintf(g_text_output_buf, g_ttx[0xe8], grammar_article((n > 1 ? 4 : 0) + 2, id, 0));
			if (dlg_yes_no(g_dialog, g_text_output_buf) == 1) {
				h = get_active_hero(this, g_ttx[0xe9]);
				if (h != 0) {
					signed short pos;

					pos = select_item(h, g_ttx[0xf0], 0);
					if (pos != -1)
						::drop_item(h, pos, -1);
				}
			} else
				done = 1;
		} else
			done = 1;
	} while (!done);
	return total;
}

signed short group::give_item_each(signed short id, register signed short n)
{
	signed short total;
	signed short i;
	struct hero far *h;
	register signed short got;

	total = 0;
	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0) {
			signed short stop;

			stop = 0;
			got = 0;
			while (!stop) {
#if FIX_0082
				/* Fix #82: nur den Rest verteilen */
				got += give_hero_new_item(h, id, 0, n - got);
#else
				got += give_hero_new_item(h, id, 0, n);	/* Originalfehler: wieder n statt n - got */
#endif
				if (got < n) {
					sprintf(g_text_output_buf, g_ttx[0x195], h->name,
						grammar_article((n > 1 ? 4 : 0) + 2, id, 0), h->name);
					if (dlg_yes_no(g_dialog, g_text_output_buf) == 1) {
						signed short pos;

						pos = select_item(h, g_ttx[0xf0], 0);
						if (pos != -1)
							::drop_item(h, pos, -1);
					} else
						stop = 1;
				} else
					stop = 1;
			}
			total += got;
		}
	}
	return total;
}

signed short group::mage_test_spell(register signed short spell, signed char mod)
{
	struct hero far *h;
	register signed short i;

	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0 && h->is_mage() && check_hero(h) && test_spell(h, spell, mod) > 0)
			return 1;
	}
	return 0;
}

void group::drop_item(register signed short id)
{
	signed short slot;
	signed short pos;
	struct hero far *h;

	slot = group_find_item(this, id);
	if (slot != -1) {
		h = heroes[slot];
		if (h != 0) {
			pos = get_item_pos(h, id);
			if (pos != -1)
				::drop_item(h, pos, 1);
		}
	}
}

void group::drop_most_items()
{
	signed short i;
	struct hero far *h;
	register signed short j;
	register signed short id;

	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0) {
			for (j = 0; j <= 30; j++) {
				id = h->inv[j].item_id;
				if (!g_items[id].magic) {
					if (h->sex == 0) {
						if (j != 5 && j != 14)
							drop_item_at(h, j);
					} else {
						if (j != 5 && j != 10 && j != 14)
							drop_item_at(h, j);
					}
				}
			}
		}
	}
}

void group::lose_random_items(signed short n)
{
	signed short pos;
	struct hero far *h;
	signed char slot;
	register signed short id;
	register signed short i;

#if FIX_0081
	/* Fix #81: fuer jeden Gegenstand neu auslosen */
	for (i = 0; i < n; i++) {
		slot = -1;
		while (slot < 0) {
#else
	slot = -1;			/* Originalfehler: nur einmal vor der Schleife */
	for (i = 0; i < n; i++) {
		while (slot < 0) {
#endif
			id = random_schweif(g_nr_items - 1);
			slot = group_find_item(this, id);
		}
		h = heroes[slot];
		pos = get_item_pos(h, id);
		drop_item_at(h, pos);
	}
}

signed short group::remove_item(register signed short id)
{
	signed short slot;
	signed short pos;
	struct hero far *h;

	slot = group_find_item(this, id);
	if (slot != -1) {
		h = heroes[slot];
		if (h != 0) {
			pos = get_item_pos(h, id);
			if (pos != -1) {
				hero_remove_item(h, pos);
				return 1;
			}
		}
	}
	return 0;
}

signed short group::count_free_knapsack()
{
	struct hero far *h;
	signed short n;
	signed short i;

	n = 0;
	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0) {
			signed short j;

			for (j = INV_SLOT_KNAPSACK; j < NR_INV_SLOTS; j++) {
				if (h->inv[j].item_id == 0)
					n++;
			}
		}
	}
	return n;
}

signed short group::save()
{
	signed short i;
	signed short len;
	struct hero far *h;
	struct hero far *saved[7];

	memcpy(saved, heroes, sizeof(heroes));
	for (i = 0, h = heroes[0]; i < 7; h = heroes[++i]) {
		if (h != 0)
			heroes[i] = (struct hero far *)(h - g_heroes + 4);	/* Index + 4 statt Zeiger */
	}
	len = sizeof(struct group);
	_write(g_save_fd, "PART", 4);
	_write(g_save_fd, &len, 2);
	_write(g_save_fd, this, len);
	memcpy(heroes, saved, sizeof(heroes));
	return 1;
}

signed short group::load()
{
	struct hero far *h;
	signed long magic;
	unsigned short n;

	_read(g_save_fd, &magic, 4);
	if (magic == 0x54524150L) {		/* "PART" */
		_read(g_save_fd, &n, 2);
		_read(g_save_fd, this, n);
		for (n = 0, h = heroes[0]; n < 7; h = heroes[++n]) {
			if (h != 0)
				heroes[n] = g_heroes + (signed short)h - 4;
		}
		return 1;
	}
	return 0;
}

signed short group::any_in_jail()
{
	struct hero far *h;
	signed short ret;
	signed short i;

	ret = 0;
	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0 && h->jail) {
			ret = 1;
			break;
		}
	}
	return ret;
}

signed short group::has_dwarf()
{
	struct hero far *h;
	register signed short i;

	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0 && h->type == HERO_TYPE_DWARF && check_hero3(h))
			return 1;
	}
	return 0;
}

signed short group::has_elf()
{
	struct hero far *h;
	signed short i;

	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0 && h->type >= HERO_TYPE_GREEN_ELF && h->type <= HERO_TYPE_SYLVAN_ELF)
			return 1;
	}
	return 0;
}
