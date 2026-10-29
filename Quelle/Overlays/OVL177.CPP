/* OVL177 - Overlay O177 (Segment 0x22d1 in SCHWEIF.EXE): Umsetzung von DSA-1-Helden (Schicksalsklinge) in das
 * Sternenschweif-Heldenformat (Aufrufer OVL176, class Save; DSA-1-Saetze struct dsa1_hero / dsa1_hero_v1).
 * swap16/swap32(p): 2 bzw. 4 Bytes tauschen (Amiga-Byteordnung), bitrev16/bitrev32(p): 16 bzw. 32 Bits spiegeln.
 * dsa1_swap_hero_v1/dsa1_swap_hero(h, len): Amiga-Satz des fruehen bzw. BE-Formats umbauen (Bytes entfernen per
 *   memmove, je Gegenstand 2 Bytes, Woerter/Langwoerter aus den Listen g_dsa1_words(_v1)/g_dsa1_longs(_v1) drehen;
 *   Zaehler in g_swap_i(_v1)/g_swap_p(_v1)). Save::import_heroes/import_heroes_v1(fd, amiga): Helden aus der Datei
 *   lesen (BE-Format 0x2da bzw. frueh 0x2ca Bytes + Bild 0x300/0x400 + 0x30), bis zu 6 umsetzen: Versionsbytes,
 *   Portraet (find_portrait), Name (str_tolower, toupper), Werte, 23 Gegenstaende ueber die Platztabelle
 *   g_dsa1_slot_map(_v1), entfallende Gegenstaende (is_in_word_array mit g_dsa1_drop_items(_v1)) entfernen, Magier/
 *   Hexe bekommen Magierstab/Hexenbesen neu, Last neu, Geld auf 0..1000, Held nach g_heroes[group_free_slot],
 *   group_add_hero, check_hero.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL177.CPP   -> 5980/5980 Bytes GLEICH (18.10.2026), cobjfix laeuft.
 * - -Oi: memset/strcpy/memcpy als Intrinsics; "memcpy(dst, h, sizeof(struct hero))" laedt die QUELLE zuerst
 *   (mov ax,seg / mov si,off / les di,dst / rep movsw / movsb) - "*dst = *h" ergibt SCOPY@.
 * - inline-Elementfunktionen von hero erklaeren alle Kopien: set_ver0/set_ver1/set_portrait, set_name/
 *   set_file_name (strcpy, Kopie des far-Zeigers [bp-2ah]/[bp-2eh]), set_from_nr_items (memcpy, Kopien
 *   [bp-32h]/[bp-34h]), set_inv_item_id/_quantity/_bf.._poison_charges (Index-Kopie der register-Variablen s: erste
 *   Expansion DI, weitere auf dem Stack), set_inv_half_empty.._magic_known, add_load, set_money.
 *   Die erste Gegenstandsschleife hat eine eigene Deklaration (register s) -> ihre Kopien hinter den -Og-Werten.
 * - "h + (unsigned)(g_swap_i_v1 * 14 + 0x186)" -> Index in ax, dann mov dx,[h] / add dx,ax (ohne unsigned add ax,[bp+6]).
 * - memmove-Laenge aus long-Parameter: "len - 0x7c" rechnet nur das untere Wort (add ax,0ff84h).
 * - "size - pic - 0x20 - 23 * sizeof(struct dsa1_item)" -> add ax,0ffe0h / sub ax,142h.
 * - bitrev16 ohne register (v SI, r DX, i CX); hero_clear_slot (O176_0EDD) ist eine freie Funktion (h, slot).
 * - bitrev16/bitrev32/dsa1_swap_hero(_v1) haben im Original keine Publics (nur im Modul gerufen); ihre Namen in
 *   dieser C-Fassung sind frei.
 * Originalfehler/Auffaelliges: Magier - die Schleife zaehlt j nicht hoch (entfernt immer Platz 15, Hexe mit j++);
 *   negatives Geld wird auf 1000 statt auf 0 gesetzt.
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 */
#include <io.h>
#include <mem.h>
#include <string.h>
#include <ctype.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:6d18..6e44, dsana) - initialisierte Daten in Adressreihenfolge */
signed char g_fig_msg_colors[24] = { 0, -5, -5, -8, -8, -12, -21, -6, -3, -7, -42, -4, -7, -3, -3, -4, -4, -3, -3, -4, -4, -7, -7, 0 };
struct portrait_idx12 g_dsa1_pics_m = { { 0, 0, 6, 12, 18, 24, 30, 36, 42, 48, 54, 62 } };
struct portrait_idx11 g_dsa1_pics_f = { { 0, 3, 9, 15, 21, 27, 34, 37, 46, 51, 58 } };
char g_str_roa_game[9] = "ROA GAME";
signed short g_dsa1_words_v1[8] = { 20, 78, 80, 82, 84, 141, 712, -1 };
signed short g_dsa1_longs_v1[5] = { 24, 28, 123, 127, -1 };
signed short g_dsa1_words[8] = { 36, 94, 96, 98, 100, 157, 728, -1 };
signed short g_dsa1_longs[5] = { 40, 44, 139, 143, -1 };
struct item_list23 g_dsa1_slot_map = { { 0, 1, 10, 3, 12, 5, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30 } };
struct item_list25 g_dsa1_drop_items = {
	{ 29, 120, 164, 170, 176, 179, 181, 187, 192, 194, 195, 201, 208, 209, 210, 211, 212, 219, 221, 222, 224, 225, 235, 247, -1 }
};
struct item_list23 g_dsa1_slot_map_v1 = { { 0, 1, 10, 3, 12, 5, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30 } };
struct item_list25 g_dsa1_drop_items_v1 = {
	{ 29, 120, 164, 170, 176, 179, 181, 187, 192, 194, 195, 201, 208, 209, 210, 211, 212, 219, 221, 222, 224, 225, 235, 247, -1 }
};
/* Phase B: eigenes BSS (ds:cb00..cb1a) in Adressreihenfolge */
signed short g_sfight_ofs_x;
signed short g_sfight_ofs_y;
signed short g_sfight_ofs2_x;
signed short g_sfight_ofs2_y;
signed short g_sfight_ofs3_x;
signed short g_sfight_ofs3_y;
signed short g_sfight_event;
signed short g_swap_i_v1;
signed short far *g_swap_p_v1;
signed short g_swap_i;
signed short far *g_swap_p;

void swap16(void far *p)
{
	unsigned char far *q = (unsigned char far *)p;
	unsigned char t;

	t = q[0];
	q[0] = q[1];
	q[1] = t;
}

void swap32(void far *p)
{
	unsigned char far *q = (unsigned char far *)p;
	register unsigned char t;

	t = q[0];
	q[0] = q[3];
	q[3] = t;
	t = q[1];
	q[1] = q[2];
	q[2] = t;
}

void bitrev16(unsigned short far *p)
{
	unsigned short v = *p;
	unsigned short r;
	signed short i;

	for (r = 0, i = 0; i < 16; i++) {
		r += r;
		if (v & 1)
			r++;
		v >>= 1;
	}
	*p = r;
}

void bitrev32(unsigned long far *p)
{
	unsigned long v = *p;
	unsigned long r = 0;
	signed short i;

	for (i = 0; i < 32; i++) {
		r += r;
		if (v & 1)
			r++;
		v >>= 1;
	}
	*p = r;
}

void dsa1_swap_hero_v1(char far *h, signed long len)
{
	memmove(h + 0x7b, h + 0x7c, len - 0x7c);
	memmove(h + 0x9a, h + 0x9b, len - 0x9b);
	for (g_swap_i_v1 = 0; g_swap_i_v1 < 23; g_swap_i_v1++) {
		memmove(h + g_swap_i_v1 * 14 + 0x18c, h + g_swap_i_v1 * 14 + 0x18d, len - (g_swap_i_v1 * 14 + 0x18c) - 1);
		memmove(h + g_swap_i_v1 * 14 + 0x18c, h + g_swap_i_v1 * 14 + 0x18d, len - (g_swap_i_v1 * 14 + 0x18c) - 1);
	}
	for (g_swap_p_v1 = g_dsa1_words_v1; *g_swap_p_v1 != -1; g_swap_p_v1++)
		swap16(h + *g_swap_p_v1);
	for (g_swap_p_v1 = g_dsa1_longs_v1; *g_swap_p_v1 != -1; g_swap_p_v1++)
		swap32(h + *g_swap_p_v1);
	bitrev32((unsigned long far *)(h + 0x9a));
	for (g_swap_i_v1 = 0; g_swap_i_v1 < 23; g_swap_i_v1++) {
		swap16(h + (unsigned)(g_swap_i_v1 * 14 + 0x186));
		swap16(h + (unsigned)(g_swap_i_v1 * 14 + 0x186) + 2);
		bitrev16((unsigned short far *)(h + g_swap_i_v1 * 14 + 0x18a));
	}
}

void dsa1_swap_hero(char far *h, signed long len)
{
	memmove(h + 0x8b, h + 0x8c, len - 0x8c);
	memmove(h + 0xaa, h + 0xab, len - 0xab);
	for (g_swap_i = 0; g_swap_i < 23; g_swap_i++) {
		memmove(h + g_swap_i * 14 + 0x19c, h + g_swap_i * 14 + 0x19d, len - (g_swap_i * 14 + 0x19c) - 1);
		memmove(h + g_swap_i * 14 + 0x19c, h + g_swap_i * 14 + 0x19d, len - (g_swap_i * 14 + 0x19c) - 1);
	}
	for (g_swap_p = g_dsa1_words; *g_swap_p != -1; g_swap_p++)
		swap16(h + *g_swap_p);
	for (g_swap_p = g_dsa1_longs; *g_swap_p != -1; g_swap_p++)
		swap32(h + *g_swap_p);
	bitrev32((unsigned long far *)(h + 0xaa));
	for (g_swap_i = 0; g_swap_i < 23; g_swap_i++) {
		swap16(h + (unsigned)(g_swap_i * 14 + 0x196));
		swap16(h + (unsigned)(g_swap_i * 14 + 0x196) + 2);
		bitrev16((unsigned short far *)(h + g_swap_i * 14 + 0x19a));
	}
}

signed short Save::import_heroes(signed short fd, signed short amiga)
{
	signed short ret = 1;
	signed long money;
	signed short k1;
	signed short k2;
	signed short n;
	signed short size;
	signed short i;
	signed short j;
	signed short count = 0;
	signed short id;
	signed short pic = amiga ? 0x300 : 0x400;
	signed short extra = amiga ? 0x30 : 0;
	struct dsa1_hero far *buf;
	struct hero far *dst;
	struct item_list23 slots = g_dsa1_slot_map;
	struct item_list25 drop = g_dsa1_drop_items;

	diary_unlock(g_diary);
	size = pic + extra + 0x2da;
	buf = (struct dsa1_hero far *)mem_alloc(size);
	do {
		n = _read(fd, buf, size);
		if (n == size) {
			if (amiga)
				dsa1_swap_hero((char far *)buf, size);
			if (buf->group_pos != 0 && !buf->npc_id && count < 6) {
				k1 = k2 = 0;
				memset(hero, 0, sizeof(struct hero));
				undo_timers(buf, amiga);
				hero->set_ver0(g_version_a);
				hero->set_ver1(g_version_c);
				hero->set_portrait(find_portrait(buf->pic, buf->sex, buf->type, amiga));
				strcpy(g_text_buf2, buf->name2);
				str_tolower(g_text_buf2);
				*g_text_buf2 = toupper(*g_text_buf2);
				hero->set_name(g_text_buf2);
				hero->set_file_name(buf->name);
				hero->set_from_nr_items(&buf->nr_items, size - pic - 0x20 - 23 * sizeof(struct dsa1_item));
				for (i = 0; i < 23; i++) {
					register signed short s = slots.v[i];

					hero->set_inv_item_id(s, buf->inv[i].item_id);
					hero->set_inv_quantity(s, buf->inv[i].quantity);
					hero->set_inv_bf(s, buf->inv[i].bf);
					hero->set_inv_rs_lost(s, buf->inv[i].rs_lost);
					hero->set_inv_light_timer(s, buf->inv[i].lighting_timer);
					hero->set_inv_poison_type(s, buf->inv[i].poison_type);
					hero->set_inv_poison_charges(s, buf->inv[i].poison_charges);
					if (buf->inv[i].broken)
						set_item_broken(hero, s, 1);
					if (buf->inv[i].half_empty)
						hero->set_inv_half_empty(s);
					if (buf->inv[i].empty)
						hero->set_inv_empty(s);
					if (buf->inv[i].magic)
						hero->set_inv_magic(s);
					if (buf->inv[i].bit4)
						hero->set_inv_b4(s);
					if (buf->inv[i].magic_revealed)
						hero->set_inv_magic_known(s);
				}
				for (i = 0; i < NR_INV_SLOTS; i++) {
					if (is_in_word_array(hero->inv[i].item_id, &drop) != -1 || hero->inv[i].item_id > 0xf9 || hero->inv[i].item_id < 0)
						hero_clear_slot(hero, i);
					else if (hero->inv[i].item_id == ITEM_MAGE_STAFF || hero->inv[i].item_id == ITEM_WITCHES_BROOM)
						hero_clear_slot(hero, i);
					else if (g_items[hero->inv[i].item_id].stackable && hero->inv[i].quantity <= 0)
						hero->set_inv_quantity(i, 1);
					else if (hero->inv[i].item_id == ITEM_WRITING_SET)
						hero->set_inv_quantity(i, 10);
					else if ((hero->inv[i].item_id == ITEM_HARP || hero->inv[i].item_id == ITEM_LUTE || hero->inv[i].item_id == ITEM_FLUTE ||
						  hero->inv[i].item_id == ITEM_OIL) && hero->inv[i].quantity <= 0)
						hero->set_inv_quantity(i, 1);
				}
				if (hero->type == HERO_TYPE_MAGE) {
					give_hero_new_item(hero, ITEM_MAGE_STAFF, 2, 1);
					k1 = get_item_pos(hero, ITEM_MAGE_STAFF);
					j = INV_SLOT_KNAPSACK;
					while (k1 == -1) {
#if FIX_0165
						/* Fix #165: naechsten Platz frei machen (j++ wie beim Hexenbesen) */
						hero_clear_slot(hero, j);
						give_hero_new_item(hero, ITEM_MAGE_STAFF, 2, 1);
						k1 = get_item_pos(hero, ITEM_MAGE_STAFF);
						j++;
#else
						hero_clear_slot(hero, j);
						give_hero_new_item(hero, ITEM_MAGE_STAFF, 2, 1);
						k1 = get_item_pos(hero, ITEM_MAGE_STAFF);
#endif
					}
					move_item(hero, INV_SLOT_RIGHT_HAND, k1);
				}
				if (hero->type == HERO_TYPE_WITCH) {
					give_hero_new_item(hero, ITEM_WITCHES_BROOM, 2, 1);
					k2 = get_item_pos(hero, ITEM_WITCHES_BROOM);
					j = INV_SLOT_KNAPSACK;
					while (k2 == -1) {
						hero_clear_slot(hero, j);
						give_hero_new_item(hero, ITEM_WITCHES_BROOM, 2, 1);
						k2 = get_item_pos(hero, ITEM_WITCHES_BROOM);
						j++;
					}
					move_item(hero, INV_SLOT_RIGHT_HAND, k2);
				}
				hero->load = 0;
				for (i = 0; i < NR_INV_SLOTS; i++) {
					id = hero->inv[i].item_id;
					if (id != 0) {
						if (g_items[id].stackable)
							hero->add_load(g_items[id].weight * hero->inv[i].quantity);
						else
							hero->add_load(g_items[id].weight);
					}
				}
				money = hero->money;
#if FIX_0165
				/* Fix #165: negatives Geld auf 0 statt 1000 */
				if (money < 0)
					money = 0;
#else
				if (money < 0)
					money = 1000;
#endif
				else if (money > 1000)
					money = 1000;
				hero->set_money(money);
				i = group_free_slot(g_group);
				dst = &g_heroes[i];
				memcpy(dst, hero, sizeof(struct hero));
				group_add_hero(g_group, dst, i);
				check_hero(dst);
				count++;
			}
		}
	} while (n);
	diary_lock(g_diary);
	if (count == 0)
		ret = -1;
	mem_free(buf);
	return ret;
}

signed short Save::import_heroes_v1(signed short fd, signed short amiga)
{
	signed long money;
	signed short ret = 1;
	signed short k1;
	signed short k2;
	signed short n;
	signed short size;
	signed short i;
	signed short j;
	signed short count = 0;
	signed short id;
	signed short pic = amiga ? 0x300 : 0x400;
	signed short extra = amiga ? 0x30 : 0;
	struct dsa1_hero_v1 far *buf;
	struct hero far *dst;
	struct item_list23 slots = g_dsa1_slot_map_v1;
	struct item_list25 drop = g_dsa1_drop_items_v1;

	size = pic + extra + 0x2ca;
	buf = (struct dsa1_hero_v1 far *)mem_alloc(size);
	diary_unlock(g_diary);
	do {
		n = _read(fd, buf, size);
		if (n == size) {
			if (amiga)
				dsa1_swap_hero_v1((char far *)buf, size);
			if (buf->group_pos != 0 && !buf->npc_id && count < 6) {
				k1 = k2 = 0;
				memset(hero, 0, sizeof(struct hero));
				undo_timers_v1(buf, amiga);
				hero->set_ver0(g_version_a);
				hero->set_ver1(g_version_c);
				hero->set_portrait(find_portrait(buf->pic, buf->sex, buf->type, amiga));
				strcpy(g_text_buf2, buf->name);
				str_tolower(g_text_buf2);
				*g_text_buf2 = toupper(*g_text_buf2);
				hero->set_name(g_text_buf2);
				hero->set_file_name(buf->name);
				hero->set_from_nr_items(&buf->nr_items, size - pic - 0x10 - 23 * sizeof(struct dsa1_item));
				for (i = 0; i < 23; i++) {
					register signed short s = slots.v[i];

					hero->set_inv_item_id(s, buf->inv[i].item_id);
					hero->set_inv_quantity(s, buf->inv[i].quantity);
					hero->set_inv_bf(s, buf->inv[i].bf);
					hero->set_inv_rs_lost(s, buf->inv[i].rs_lost);
					hero->set_inv_light_timer(s, buf->inv[i].lighting_timer);
					hero->set_inv_poison_type(s, buf->inv[i].poison_type);
					hero->set_inv_poison_charges(s, buf->inv[i].poison_charges);
					if (buf->inv[i].broken)
						set_item_broken(hero, s, 1);
					if (buf->inv[i].half_empty)
						hero->set_inv_half_empty(s);
					if (buf->inv[i].empty)
						hero->set_inv_empty(s);
					if (buf->inv[i].magic)
						hero->set_inv_magic(s);
					if (buf->inv[i].bit4)
						hero->set_inv_b4(s);
					if (buf->inv[i].magic_revealed)
						hero->set_inv_magic_known(s);
				}
				for (i = 0; i < NR_INV_SLOTS; i++) {
					if (is_in_word_array(hero->inv[i].item_id, &drop) != -1 || hero->inv[i].item_id > 0xf9 || hero->inv[i].item_id < 0)
						hero_clear_slot(hero, i);
					else if (hero->inv[i].item_id == ITEM_MAGE_STAFF || hero->inv[i].item_id == ITEM_WITCHES_BROOM)
						hero_clear_slot(hero, i);
					else if (g_items[hero->inv[i].item_id].stackable && hero->inv[i].quantity <= 0)
						hero->set_inv_quantity(i, 1);
					else if (hero->inv[i].item_id == ITEM_WRITING_SET)
						hero->set_inv_quantity(i, 10);
					else if ((hero->inv[i].item_id == ITEM_HARP || hero->inv[i].item_id == ITEM_LUTE || hero->inv[i].item_id == ITEM_FLUTE ||
						  hero->inv[i].item_id == ITEM_OIL) && hero->inv[i].quantity <= 0)
						hero->set_inv_quantity(i, 1);
				}
				if (hero->type == HERO_TYPE_MAGE) {
					give_hero_new_item(hero, ITEM_MAGE_STAFF, 2, 1);
					k1 = get_item_pos(hero, ITEM_MAGE_STAFF);
					j = INV_SLOT_KNAPSACK;
					while (k1 == -1) {
#if FIX_0165
						/* Fix #165: naechsten Platz frei machen (j++ wie beim Hexenbesen) */
						hero_clear_slot(hero, j);
						give_hero_new_item(hero, ITEM_MAGE_STAFF, 2, 1);
						k1 = get_item_pos(hero, ITEM_MAGE_STAFF);
						j++;
#else
						hero_clear_slot(hero, j);
						give_hero_new_item(hero, ITEM_MAGE_STAFF, 2, 1);
						k1 = get_item_pos(hero, ITEM_MAGE_STAFF);
#endif
					}
					move_item(hero, INV_SLOT_RIGHT_HAND, k1);
				}
				if (hero->type == HERO_TYPE_WITCH) {
					give_hero_new_item(hero, ITEM_WITCHES_BROOM, 2, 1);
					k2 = get_item_pos(hero, ITEM_WITCHES_BROOM);
					j = INV_SLOT_KNAPSACK;
					while (k2 == -1) {
						hero_clear_slot(hero, j);
						give_hero_new_item(hero, ITEM_WITCHES_BROOM, 2, 1);
						k2 = get_item_pos(hero, ITEM_WITCHES_BROOM);
						j++;
					}
					move_item(hero, INV_SLOT_RIGHT_HAND, k2);
				}
				hero->load = 0;
				for (i = 0; i < NR_INV_SLOTS; i++) {
					id = hero->inv[i].item_id;
					if (id != 0) {
						if (g_items[id].stackable)
							hero->add_load(g_items[id].weight * hero->inv[i].quantity);
						else
							hero->add_load(g_items[id].weight);
					}
				}
				money = hero->money;
#if FIX_0165
				/* Fix #165: negatives Geld auf 0 statt 1000 */
				if (money < 0)
					money = 0;
#else
				if (money < 0)
					money = 1000;
#endif
				else if (money > 1000)
					money = 1000;
				hero->set_money(money);
				i = group_free_slot(g_group);
				dst = &g_heroes[i];
				memcpy(dst, hero, sizeof(struct hero));
				group_add_hero(g_group, dst, i);
				check_hero(dst);
				count++;
			}
		}
	} while (n);
	diary_lock(g_diary);
	if (count == 0)
		ret = -1;
	mem_free(buf);
	return ret;
}
