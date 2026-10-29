/* OVL083 - Overlay O083 (Segment 0x20e8 in SCHWEIF.EXE): Inventar der Helden (hero.inv[31] zu 0x1b Bytes, 0..14 am
 * Koerper, 15..30 Rucksack; Gegenstaende g_items zu 14 Bytes). Entspricht Bright Eyes seg105/seg106 (move_item,
 * can_item_at_pos, two_hand_collision, add_equip_boni, unequip, has_hero_equipped, has_hero_stacked, loose_random_item,
 * get_full_waterskin_pos).
 * hero_find_food(h, &slot, &id): ersten Gegenstand mit Bit food und Untertyp 1 suchen. hero_find_drink(h, &slot, &id):
 *   Getraenk suchen (Ids 0x17, Wasserschlauch ueber get_full_waterskin_pos, 0x5c, 0x5b, 0x142 per get_item_pos, sonst
 *   food mit Untertyp 0 ausser dem Wasserschlauch). get_full_waterskin_pos(h): Platz mit 0x1e ohne Bit empty.
 *   has_hero_equipped(h, id): id am Koerper (0..14). make_item_list(h, mode, list): Liste (Platz, Name grammar_name)
 *   aller belegten Plaetze (mode 0: nur Rucksack). select_item(h, titel, mode): Auswahl eines Gegenstands (Liste mit
 *   31 Namenspuffern a 39 Bytes, Blaettern in 12er-Seiten, Eintrag g_ttx[241] "weiter"), Ergebnis Platz oder -1
 *   (leer: Meldung g_ttx[239]).
 * move_item(h, pos1, pos2): Gegenstand verschieben/tauschen inkl. Stapeln (bis 99, Giftladungen poison_type/
 *   poison_charges) und Meldungen (g_ttx[243..247], Textliste aus g_font); Ergebnis 1 = erledigt.
 * can_hero_use_item(h, id) = can_use_item. can_item_at_pos(h, id, pos): passt an pos (Waffen nur in die Haende 3/12,
 *   sonst Untertyp bzw. Bits f03_b0..b6). two_hand_collision(h, id, pos): Zweihaender-Konflikt. drop_item_at(h, pos):
 *   Gegenstand ablegen (drop_item), ausser undropable oder Liste g_items_keep. has_hero_stacked(h, id): Platz mit id
 *   und Anzahl < 99. unequip(h, id, pos): Boni beim Ablegen zuruecknehmen (RS/BE g_armors_table, weapon_type/at_mod/
 *   pa_mod, Sondergegenstaende ueber change_attrib/add_mr_mod, g_sheet_dirty = 1). add_equip_boni(h, h2, id, src, dst):
 *   Boni beim Anlegen (Waffenwerte g_weapons_atpa, Heldenbogen sheet_slot_changed bei g_pp20_index == 14 und
 *   h == g_sheet_hero). set_item_broken(h, pos, broken): Bit broken setzen, wenn zerbrechlich (bf != -99).
 *   loose_random_item(h, chance, fmt, fmt_plural): zufaelligen Gegenstand verlieren (random_schweif) mit Meldung.
 *   hero_dec_weapons_bf(h): bei allen Waffen den Bruchfaktor zweimal um 1 senken (> 0).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL083.CPP   -> 6305/6305 Bytes GLEICH (18.10.2026), cobjfix laeuft.
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Bitfelder in Gegenstand und Platz als "unsigned short b:1" (Test and ax,1 / or ax,ax); mit unsigned char
 *   kommen or al,al und mov ah,0. Held: "!(signed char)h->dead" -> and ax,1 / cbw / or ax,ax (frueher eigene
 *   unsigned-char-Bits +bc, gleiche Bytes).
 * - -Oi: strcpy(list[n].name, grammar_name(..)) inline, memset(&h->inv[pos2], 0, 27) -> rep stosw / stosb;
 *   Strukturtausch: tmp = inv[a] und inv[a] = inv[b] ueber SCOPY@, inv[b] = tmp inline (rep movsw / movsb).
 * - Inline-Elemente des Helden: set_inv_poison_type(int, int) (Wortkopie des Werts), set_inv_poison_charges(int,
 *   char) (Bytekopie), dec_inv_bf(i) "if ((bf == -99 ? 0 : 1) && bf > 0) bf--; else ;" (mov al,0 / mov al,1 /
 *   or al,al, toter jmp short); is_inv_breakable(i) "return bf == -99 ? 0 : 1;" (int: xor ax,ax / mov ax,1 /
 *   or ax,ax) in set_item_broken; get_inv_rs_lost(i) liefert int: "h->rs_bonus -= h2->get_inv_rs_lost(src)" ->
 *   mov al / cbw / mov dl,[42] / sub dl,al (keine Parameterkopie, da src Parameter); direkt geschrieben laedt BCC
 *   rs_bonus zuerst oder nimmt sub mem,al.
 * - move_item Stapelbloecke: Registervergabe nur mit Deklarationsreihenfolge (Block 1 ohne register: charges2,
 *   type1 in SI/DI, moved in CX; Block 2 "a; register b; c; register d;"), "if (a < room) x = a; else x = room;"
 *   statt ?:.
 * - two_hand_collision/loose_random_item: Zuweisung getrennt vor dem Test (mov di,[..] / or di,di); eingebettet ->
 *   mov ax / mov di,ax.
 * - select_item: n als register (DI), SI nimmt BCC als Hilfsregister (les si,ds:[g_ttx]); "page = r = 0".
 * - g_weapons_atpa (ds:2f80) ist ein eigenes Public mitten in g_weapons_table - nicht ueber g_weapons_table[i].at_mod
 *   schreiben (anderes Fixup-Ziel).
 * Auffaelliges im Original: select_item setzt auf Zwischenseiten more = 24 als Konstante (stimmt nur, weil bei
 *   hoechstens 31 Plaetzen nur Seite 12 eine Zwischenseite sein kann); can_item_at_pos benutzt h nicht (Warnung).
 */
#include <string.h>
#include <stdio.h>
#include "schweif.h"

signed short hero_find_food(struct hero far *h, signed short far *pslot, signed short far *pid)
{
	signed short found = 0;
	register signed short i;
	register signed short id;

	for (i = 0; i <= 30; i++) {
		id = h->inv[i].item_id;
		if (g_items[id].food && g_items[id].subtype == 1) {	/* BE NUTRITION_TYPE_FOOD */
			*pslot = i;
			*pid = id;
			found = 1;
			break;
		}
	}
	return found;
}

signed short hero_find_drink(struct hero far *h, signed short far *pslot, signed short far *pid)
{
	signed short ok = 1;
	register signed short i;
	register signed short id;

	*pid = 0x17;
	*pslot = get_item_pos(h, 0x17);
	if (*pslot == -1) {
		*pid = ITEM_WATERSKIN;
		*pslot = get_full_waterskin_pos(h);
		if (*pslot == -1) {
			*pid = 0x5c;
			*pslot = get_item_pos(h, 0x5c);
			if (*pslot == -1) {
				*pid = 0x5b;
				*pslot = get_item_pos(h, 0x5b);
				if (*pslot == -1) {
					*pid = 0x142;
					*pslot = get_item_pos(h, 0x142);
					if (*pslot == -1)
						ok = 0;
				}
			}
		}
	}
	if (ok == 0) {
		/* sonst ein Getraenk (Untertyp 0, BE NUTRITION_TYPE_DRINK) ausser dem leeren Wasserschlauch */
		for (i = 0; i <= 30; i++) {
			id = h->inv[i].item_id;
			if (g_items[id].food && g_items[id].subtype == 0 && id != ITEM_WATERSKIN) {
				*pslot = i;
				*pid = id;
				ok = 1;
				break;
			}
		}
	}
	return ok;
}

signed short get_full_waterskin_pos(struct hero far *h)
{
	register signed short i;

	for (i = 0; i < NR_INV_SLOTS; i++)
		if (h->inv[i].item_id == ITEM_WATERSKIN && !h->inv[i].empty)
			return i;
	return -1;
}

signed short has_hero_equipped(struct hero far *h, register signed short id)
{
	register signed short i;

	for (i = 0; i < INV_SLOT_KNAPSACK; i++)
		if (h->inv[i].item_id == id)
			return i;
	return -1;
}

signed short make_item_list(struct hero far *h, signed short mode, struct item_entry far *list)
{
	signed short i;
	signed short id;
	signed short n = 0;

	for (i = mode == 0 ? INV_SLOT_KNAPSACK : 0; i < NR_INV_SLOTS; i++) {
		if ((id = h->inv[i].item_id) != 0) {
			list[n].slot = i;
			strcpy(list[n].name, grammar_name(g_item_names[id]));
			n++;
		}
	}
	return n;
}

signed short select_item(struct hero far *h, char far *title, signed short mode)
{
	signed short page;
	signed short r;
	signed short cnt;
	signed short more;
	signed short old;
	signed short ret = -1;
	char far *saved;
	register signed short n;
	struct item_entry list[31];

	for (n = 0; n < 31; n++)
		list[n].name = (char far *)mem_alloc(39);
	cnt = make_item_list(h, mode, list);
	if (cnt == 0) {
		sprintf(g_text_output_buf, g_ttx[239], h->name);
		dlg_message(g_dialog, g_text_output_buf);
		for (n = 0; n < 31; n++)
			mem_free(list[n].name);
		return -1;
	}
	page = r = 0;
	while (r != -1) {
		/* mehr als 12 Eintraege: Seiten zu 12, letzter Eintrag "weiter" */
		more = -1;
		if (cnt > 12) {
			if (page == 0) {
				n = 13;
				more = n - 1;
			} else if (page + 12 < cnt) {
				n = 13;
				more = 24;
			} else {
				n = cnt + 1;
				more = n - 1;
				n -= page;
			}
			saved = list[more].name;
			list[more].name = g_ttx[241];
		} else
			n = cnt;
		old = dlg_set16(g_dialog, 6);
		r = dlg_choose(g_dialog, title, n, list[page].name, list[page + 1].name, list[page + 2].name,
			list[page + 3].name, list[page + 4].name, list[page + 5].name, list[page + 6].name,
			list[page + 7].name, list[page + 8].name, list[page + 9].name, list[page + 10].name,
			list[page + 11].name, list[page + 12].name);
		dlg_set16(g_dialog, old);
		if (more != -1)
			list[more].name = saved;
		if (cnt > 12 && r == n) {
			page += 12;
			if (page >= cnt)
				page = 0;
		} else if (r != -1) {
			ret = list[page + r - 1].slot;
			r = -1;
		}
	}
	for (n = 0; n < 31; n++)
		mem_free(list[n].name);
	return ret;
}

signed short move_item(struct hero far *h, signed short pos1, signed short pos2)
{
	signed short item1;
	signed short item2;
	signed short temp;
	signed short ret = 0;
	void far *txt_buf;
	char far * far *txt;

	if (check_hero3(h) == 0 || pos1 == pos2) {
		if (pos1 == pos2)
			ret = 1;
		else {
			txt_load_list(g_font, 0x1c, 0x5f, &txt_buf, &txt);
			sprintf(g_text_output_buf, txt[79], h->name);
			dlg_message(g_dialog, g_text_output_buf);
			mem_free(txt_buf);
			mem_free(txt);
		}
		return ret;
	}
	if (pos2 >= INV_SLOT_KNAPSACK && pos1 >= INV_SLOT_KNAPSACK) {
		ret = 1;
		item1 = h->inv[pos1].item_id;
		item2 = h->inv[pos2].item_id;
	} else {
		item1 = h->inv[pos1].item_id;
		item2 = h->inv[pos2].item_id;
		if (pos2 < pos1 || (pos1 < INV_SLOT_KNAPSACK && pos2 < INV_SLOT_KNAPSACK)) {
			if (pos1 < INV_SLOT_KNAPSACK) {
				if (item1 != 0)
					ret = 1;
			} else
				ret = 1;
			if (ret == 1) {
				temp = pos1;
				pos1 = pos2;
				pos2 = temp;
				temp = item1;
				item1 = item2;
				item2 = temp;
			}
		}
		ret = 0;
		if (item1 == 0 && item2 == 0) {
			void far *txt_buf2;
			char far * far *txt2;

			txt_load_list(g_font, 0x2e3, 0x20, &txt_buf2, &txt2);
			dlg_message(g_dialog, txt2[23]);
			mem_free(txt_buf2);
			mem_free(txt2);
		} else if (item2 != 0) {
			if (item2 == item1 && g_items[item1].stackable) {
				ret = 1;
			} else if (!can_hero_use_item(h, item2)) {
				sprintf(g_text_output_buf, g_ttx[243], h->name, g_ttx[(h->sex ? 0x3e : 0x32) + h->type],
					grammar_article(2, item2, 0));
				dlg_message(g_dialog, g_text_output_buf);
			} else if (!can_item_at_pos(h, item2, pos1)) {
				if (is_in_word_array(item2, g_items_plural) != -1)
					sprintf(g_text_output_buf, g_ttx[244], grammar_article(0x4000, item2, 0), g_ttx[245]);
				else
					sprintf(g_text_output_buf, g_ttx[244], grammar_article(0, item2, 0), g_ttx[246]);
				dlg_message(g_dialog, g_text_output_buf);
			} else if (item1 != 0 && pos2 < INV_SLOT_KNAPSACK && !can_item_at_pos(h, item1, pos2)) {
				if (is_in_word_array(item1, g_items_plural) != -1)
					sprintf(g_text_output_buf, g_ttx[244], grammar_article(0x4000, item1, 0), g_ttx[245]);
				else
					sprintf(g_text_output_buf, g_ttx[244], grammar_article(0, item1, 0), g_ttx[246]);
				dlg_message(g_dialog, g_text_output_buf);
			} else if (two_hand_collision(h, item2, pos1)) {
				sprintf(g_text_output_buf, g_ttx[247], h->name, grammar_pronoun(h->sex, 0, 0));
				dlg_message(g_dialog, g_text_output_buf);
			} else {
				if (item1 != 0 && pos1 < INV_SLOT_KNAPSACK)
					unequip(h, item1, pos1);
				if (pos2 < INV_SLOT_KNAPSACK)
					unequip(h, item2, pos2);
				add_equip_boni(h, h, item2, pos2, pos1);
				if (item1 != 0 && pos2 < INV_SLOT_KNAPSACK)
					add_equip_boni(h, h, item1, pos1, pos2);
				ret = 1;
			}
		} else {
			unequip(h, item1, pos1);
			ret = 1;
		}
	}
	if (ret == 1) {
		if (item2 == item1 && g_items[item1].stackable) {
			/* stapeln (hoechstens 99), Giftladungen mitnehmen */
			if (h->inv[pos1].quantity + h->inv[pos2].quantity > 99) {
				if (h->inv[pos2].poison_charges > 0) {
					signed short charges2;
					signed short type1;
					signed short type2;
					signed short charges1;
					signed short room;
					signed short moved;

					room = 99 - h->inv[pos1].quantity;
					charges2 = h->inv[pos2].poison_charges;
					type2 = h->inv[pos2].poison_type;
					charges1 = h->inv[pos1].poison_charges;
					type1 = h->inv[pos1].poison_type;
					if (charges2 < room)
						moved = charges2;
					else
						moved = room;
					if (type2 == type1)
						charges1 += moved;
					else
						charges1 = moved;
					type1 = type2;
					h->set_inv_poison_type(pos1, type1);
					h->set_inv_poison_charges(pos1, charges1);
					moved = charges2 - room;
					if (moved <= 0) {
						h->set_inv_poison_type(pos2, 0);
						moved = 0;
					}
					h->set_inv_poison_charges(pos2, moved);
				}
				h->inv[pos2].quantity = h->inv[pos1].quantity + h->inv[pos2].quantity - 99;
				h->inv[pos1].quantity = 99;
			} else {
				if (h->inv[pos2].poison_charges > 0) {
					signed short charges2;
					register signed short type2;
					signed short charges1;
					register signed short type1;

					charges2 = h->inv[pos2].poison_charges;
					type2 = h->inv[pos2].poison_type;
					charges1 = h->inv[pos1].poison_charges;
					type1 = h->inv[pos1].poison_type;
					if (type2 == type1)
						charges1 += charges2;
					type1 = type2;
					h->set_inv_poison_type(pos1, type1);
					h->set_inv_poison_charges(pos1, charges1);
				}
				h->inv[pos1].quantity += h->inv[pos2].quantity;
				memset(&h->inv[pos2], 0, sizeof(struct inv_slot));
				h->nr_items -= 1;
			}
		} else {
			struct inv_slot tmp;

			tmp = h->inv[pos1];
			h->inv[pos1] = h->inv[pos2];
			h->inv[pos2] = tmp;
		}
	}
	return ret;
}

signed short can_hero_use_item(struct hero far *h, signed short id)
{
	return can_use_item(h, id);
}

signed short can_item_at_pos(struct hero far *h, signed short id, register signed short pos)
{
	signed short ret = 0;

	if (g_items[id].weapon) {
		if (pos == INV_SLOT_RIGHT_HAND || pos == INV_SLOT_LEFT_HAND)
			ret = 1;
		else
			ret = 0;
	} else if ((pos == 0 && g_items[id].subtype == 0 && g_items[id].armor) ||
		(pos == 0 && g_items[id].f03_b5) ||
		(pos == 10 && g_items[id].subtype == 10) ||
		(pos == 5 && g_items[id].subtype == 5) ||
		(pos == 14 && g_items[id].subtype == 14) ||
		(pos == 7 && g_items[id].f03_b0) ||
		(pos == 8 && g_items[id].f03_b3) ||
		(pos == 9 && g_items[id].f03_b4) ||
		((pos == 2 || pos == 11) && g_items[id].f03_b2) ||
		((pos == 4 || pos == 13) && g_items[id].f03_b1) ||
		((pos == 6 || pos == 1) && g_items[id].f03_b6) ||
		pos == INV_SLOT_RIGHT_HAND || pos == INV_SLOT_LEFT_HAND)
		ret = 1;
	return ret;
}

signed short two_hand_collision(struct hero far *h, signed short id, register signed short pos)
{
	signed short ret = 0;
	signed short in_hand;
	signed short other;

	if (pos == INV_SLOT_RIGHT_HAND || pos == INV_SLOT_LEFT_HAND) {
		other = INV_SLOT_RIGHT_HAND;
		if (pos == INV_SLOT_RIGHT_HAND)
			other = INV_SLOT_LEFT_HAND;
		in_hand = h->inv[other].item_id;
		if (in_hand != 0) {
			/* Waffenart 6 = Zweihaender */
			if ((g_items[id].weapon && g_items[id].subtype == 6) ||
				(g_items[in_hand].weapon && g_items[in_hand].subtype == 6))
				ret = 1;
		}
	}
	return ret;
}

signed short drop_item_at(struct hero far *h, register signed short pos)
{
	if (!g_items[h->inv[pos].item_id].undropable && is_in_word_array(h->inv[pos].item_id, g_items_keep) == -1)
		return drop_item(h, pos, h->inv[pos].quantity);
	else
		return 0;
}

signed short has_hero_stacked(struct hero far *h, register signed short id)
{
	register signed short i;

	for (i = 0; i < NR_INV_SLOTS; i++)
		if (h->inv[i].item_id == id && h->inv[i].quantity < 99)
			return i;
	return -1;
}

void unequip(struct hero far *h, register signed short id, signed short pos)
{
	signed char old_rs;
	signed char old_be;

	if (id != 0) {
		if (g_items[id].armor && ((pos != INV_SLOT_RIGHT_HAND && pos != INV_SLOT_LEFT_HAND) ||
			g_items[id].subtype == 9)) {
			old_rs = h->rs_bonus;
			old_be = h->rs_be;
			h->rs_bonus -= g_armors_table[g_items[id].table_index].rs;
			h->rs_bonus += h->inv[pos].rs_lost;
			h->rs_be -= g_armors_table[g_items[id].table_index].be;
			if (h->rs_bonus != old_rs || h->rs_be != old_be)
				g_sheet_dirty = 1;
		}
		if (g_items[id].weapon && pos == INV_SLOT_RIGHT_HAND) {
			h->weapon_type = 0;
			h->at_mod = h->pa_mod = 0;
			g_sheet_dirty = 1;
		}
		if (pos != INV_SLOT_RIGHT_HAND && pos != INV_SLOT_LEFT_HAND) {
			if (id == 0xb7) {
				change_attrib(h, ATTRIB_KK, -5);
				g_sheet_dirty = 1;
			} else if (id == 0xc4) {
				change_attrib(h, ATTRIB_CH, 1);
				g_sheet_dirty = 1;
			} else if (id == 0xd7) {
				change_attrib(h, ATTRIB_TA, 2);
				g_sheet_dirty = 1;
			} else if (id == 0xd9 || id == 0xa5) {
				add_mr_mod(h, -2);
				g_sheet_dirty = 1;
			} else if (id == 0xb6) {
				change_attrib(h, ATTRIB_TA, 4);
				g_sheet_dirty = 1;
			} else if (id == 0x143) {
				change_attrib(h, ATTRIB_HA, 3);
				g_sheet_dirty = 1;
			} else if (id == 0x144) {
				change_attrib(h, ATTRIB_CH, -5);
				g_sheet_dirty = 1;
			}
		}
	}
}

void add_equip_boni(struct hero far *h, struct hero far *h2, register signed short id, signed short src,
	signed short dst)
{
	signed char old1;
	signed char old2;
	signed char old3;

	if (id != 0) {
		if (g_pp20_index == 14 && h == g_sheet_hero)
			sheet_slot_changed(h->sex, dst, id);
		if (g_items[id].armor && ((dst != INV_SLOT_RIGHT_HAND && dst != INV_SLOT_LEFT_HAND) ||
			g_items[id].subtype == 9)) {
			old1 = h->rs_bonus;
			old2 = h->rs_be;
			h->rs_bonus += g_armors_table[g_items[id].table_index].rs;
			if (src != -1)
				h->rs_bonus -= h2->get_inv_rs_lost(src);
			h->rs_be += g_armors_table[g_items[id].table_index].be;
			if (h->rs_bonus != old1 || h->rs_be != old2)
				g_sheet_dirty = 1;
		}
		if (g_items[id].weapon && dst == INV_SLOT_RIGHT_HAND) {
			old1 = h->weapon_type;
			old2 = h->at_mod;
			old3 = h->pa_mod;
			h->weapon_type = g_items[id].subtype;
			h->at_mod = g_weapons_atpa[g_items[id].table_index].at_mod;
			h->pa_mod = g_weapons_atpa[g_items[id].table_index].pa_mod;
			if (h->weapon_type != old1 || h->at_mod != old2 || h->pa_mod != old3)
				g_sheet_dirty = 1;
		}
		if (dst != INV_SLOT_RIGHT_HAND && dst != INV_SLOT_LEFT_HAND) {
			if (id == 0xb7) {
				change_attrib(h, ATTRIB_KK, 5);
				g_sheet_dirty = 1;
			} else if (id == 0xc4) {
				change_attrib(h, ATTRIB_CH, -1);
				g_sheet_dirty = 1;
			} else if (id == 0xd7) {
				change_attrib(h, ATTRIB_TA, -2);
				g_sheet_dirty = 1;
			} else if (id == 0xd9 || id == 0xa5) {
				add_mr_mod(h, 2);
				g_sheet_dirty = 1;
			} else if (id == 0xb6) {
				change_attrib(h, ATTRIB_TA, -4);
				g_sheet_dirty = 1;
			} else if (id == 0x143) {
				change_attrib(h, ATTRIB_HA, -3);
				g_sheet_dirty = 1;
			} else if (id == 0x144) {
				change_attrib(h, ATTRIB_CH, 5);
				g_sheet_dirty = 1;
			}
		}
	}
}

signed short set_item_broken(struct hero far *h, signed short pos, register signed char broken)
{
	if (h->is_inv_breakable(pos)) {
		if (broken && h->inv[pos].broken) {
			return 0;
		} else {
			h->inv[pos].broken = broken;
			return 1;
		}
	} else
		return 0;
}

void loose_random_item(struct hero far *h, signed short chance, char far *fmt, char far *fmt_plural)
{
	register signed short id;
	register signed short pos;

	if (random_schweif(100) <= chance) {
		pos = random_schweif(31) - 1;
		id = h->inv[pos].item_id;
		if (id != 0 && !g_items[id].undropable) {
			drop_item(h, pos, 1);
			if (fmt != 0) {
				if (is_in_word_array(id, g_items_plural) != -1) {
					if (fmt_plural != 0)
						sprintf(g_text_output_buf, fmt_plural, h->name, grammar_article(0x4004, id, 0));
					else
						sprintf(g_text_output_buf, fmt, h->name, grammar_article(0x4004, id, 0));
				} else if (is_in_word_array(id, g_items_plural2) != -1)
					sprintf(g_text_output_buf, fmt, h->name, grammar_article(0x4004, id, 0));
				else
					sprintf(g_text_output_buf, fmt, h->name, grammar_article(0, id, 0));
				dlg_message(g_dialog, g_text_output_buf);
			}
		}
	}
}

void hero_dec_weapons_bf(struct hero far *h)
{
	register signed short i;
	signed short id;

	if (!(signed char)h->dead) {
		for (i = 0; i < NR_INV_SLOTS; i++) {
			if ((id = h->inv[i].item_id) != 0 && g_items[id].weapon) {
				h->dec_inv_bf(i);
				h->dec_inv_bf(i);
			}
		}
	}
}
