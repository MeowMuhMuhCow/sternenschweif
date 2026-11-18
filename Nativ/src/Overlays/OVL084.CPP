/* OVL084 - Overlay O084 (Segment 0x20f0 in SCHWEIF.EXE): Gegenstaende eines Helden (vgl. DSA1 seg105/seg106).
 * print_item_description(h, pos): Gegenstandsbeschreibung (Inventarplatz 31 im Heldenbogen, O009): Texte 0x2e3 aus
 *   g_font (txt_load_list, 32 Eintraege), Name per grammar_article (Mehrzahl 0x4004 bei Stapeln/Liste
 *   g_items_plural), Lichtstufe auf Platz 14 (switch inv.f10: 0 / 1..3 / 4..6 / 7..9), Waffe (Typname
 *   g_ttx[0x4b + Untertyp], Schaden calc_damage_range aus g_weapons_table, AT/PA-Mod) bzw. Ruestung (RS/BE aus
 *   g_armors_table), Faecher des Schluesselbunds 0x145 (26) und des Behaelters 0xbf (14) per box_has, Zustaende
 *   (broken/half_empty/empty/magic, rs_lost, Gift, Bruchzustand hero_weapon_state), can_use_item (nicht benutzbar),
 *   Sonderfaelle 0x85/0x102/0x155 (count_failed_skill), Gewicht (bei Stapeln mal Anzahl); leerer Platz txt[23];
 *   Ausgabe dlg_message(g_dialog), Puffer freigeben. hero_add_rs_lost(h, pos, v): rs_lost um v erhoehen (Platz 10:
 *   aus rs_bonus entnehmen, begrenzt), nicht unter 0. hero_add_inv_f10(h, pos, v): inv.f10 += v.
 *   hero_weapon_state(h, pos): Bruchzustand 1/2/3 aus bf gegen weapon_desc.bf. hero_mark_weapon(h, fmt, mode): erste
 *   passende Waffe markieren (magic + magic_revealed) bzw. broken loeschen, Meldung nach g_text_output_buf.
 *   startup_equipment(h, _): Startausruestung (einmalig, start_gear): fuenf Grundgegenstaende g_start_gear,
 *   Typtabelle g_start_gear_types (4 je Typ, -1 Ende), Sonderfaelle Krieger/Magier/Jaeger/Auelf/Waldelf.
 * hero_weapon_type2(h) / hero_weapon_type(h): Waffentyp in der rechten Hand (-1 ohne Waffe oder zerbrochen;
 *   hero_weapon_type auch fuer Schuss-/Wurfwaffen 7/8).
 * hero_remove_item(h, pos): Gegenstand entfernen (undropable: Platz loeschen, nr_items/load anpassen, sonst drop_item).
 * hero_count_item(h, id): Anzahl eines Gegenstands (Stapel zaehlen mit quantity).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL084.CPP   -> 6570/6570 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (_g_5a16 -> _g_5a10+0x6). (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - print_item_description: Waffenzweig als "if (weapon && (!stackable || subtype == 8)) {..} else if (armor)" -
 *   verschachtelt springt der Fehlfall hinter das else statt auf den armor-Test (nur 1 Byte Unterschied im
 *   Sprungziel). Signed-char-Tests rs_lost/poison_type nur mit "!!" (mov al / cbw / or ax,ax). i/cnt in den beiden
 *   Zaehlbloecken (liegen hinter den -Og-Zwischenwerten [bp-20h..-27h]). "at_mod = pa_mod = be / 2" (rechts
 *   zuerst). strcat/strcpy als -Oi-Intrinsics. Die Bitfelder (frueher Unterstruktur "ibits") direkt in item_desc/
 *   inv_slot ergeben dieselben Bytes.
 * - hero_mark_weapon: i ohne register (DI), -Og-Offset &h->inv[i] in SI; "for (i = ok = 0; ..)"; "(id = ..) != 0".
 * - startup_equipment: "if (h->start_gear == 0) { Rumpf }" (cmp byte statt mov al / cbw); gear[5] und by_type[12][4]
 *   sind lokale Felder mit Initialisierer (BCC: Vorlagen ds:5a16/5a20 im eigenen _DATA, rep movsw auf den Stack;
 *   Phase B 19.10.2026, vorher Strukturkopien von g_start_gear/g_start_gear_types). by_type endet bei ds:5a80 -
 *   die 14 Bytes ab ds:5a72 gehoeren noch zu O084 (dsana/korrekturen.tsv). Der zweite Parameter ist unbenutzt
 *   (O029 uebergibt den Heldenplatz; Warnung).
 * Auffaellig (Original): startup_equipment prueft "item[i] != -1" vor "i < 4" (liest bei i == 4 den ersten Eintrag
 *   des naechsten Typs bzw. hinter der Tabelle).
 */
#include <stdio.h>
#include <string.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:5a0e..5a80, dsana) - initialisierte Daten in Adressreihenfolge */
signed short g_quiet = 0;
signed short g_items_keep[3] = { 255, 256, -1 };
/* ds:5a16..5a80: BCC-Vorlagen der lokalen Tabellen gear und by_type in startup_equipment */

#define NEWLINE() strcat(g_text_output_buf, g_ttx[0x160])

void print_item_description(struct hero far *h, signed short pos)
{
	signed short id;
	signed short state;
	signed short dmg_min;
	signed short dmg_max;
	struct weapon_desc far *wp;
	struct armor_desc far *ap;
	signed short nl;
	signed short at_mod;
	signed short pa_mod;
	void far *txt_buf;
	char far * far *txt;

	nl = 1;
	txt_load_list(g_font, 0x2e3, 0x20, &txt_buf, &txt);
	id = h->inv[pos].item_id;
	if (id != 0) {
		/* Name (Mehrzahl bei Stapeln) */
		if ((h->inv[pos].quantity > 1 && g_items[id].stackable) || is_in_word_array(id, g_items_plural) != -1)
			sprintf(g_text_output_buf, txt[2], grammar_article(0x4004, id, 0));
		else if (id == 0x117 || id == 0xfc)
			sprintf(g_text_output_buf, txt[3], grammar_article(0x8000, id, 0));
		else
			sprintf(g_text_output_buf, txt[3], grammar_article(0, id, 0));
		if (pos == 14) {
			/* Lichtstufe */
			switch (h->inv[pos].f10) {
			case 0:
				NEWLINE();
				strcat(g_text_output_buf, txt[31]);
				break;
			case 1:
			case 2:
			case 3:
				NEWLINE();
				strcat(g_text_output_buf, txt[4]);
				break;
			case 4:
			case 5:
			case 6:
				NEWLINE();
				strcat(g_text_output_buf, txt[5]);
				break;
			case 7:
			case 8:
			case 9:
				NEWLINE();
				strcat(g_text_output_buf, txt[6]);
				break;
			}
		}
		if (g_items[id].weapon && (!g_items[id].stackable || g_items[id].subtype == TA_WURFWAFFEN)) {
			/* Waffe: Art, Schaden, AT/PA-Modifikator */
			NEWLINE();
			NEWLINE();
			sprintf(g_text_buf2, txt[9], g_ttx[g_items[id].subtype + 0x4b]);
			strcat(g_text_output_buf, g_text_buf2);
			NEWLINE();
			wp = &g_weapons_table[g_items[id].table_index];
			calc_damage_range(wp->dmg_d6, 6, wp->dmg_add, &dmg_min, &dmg_max);
			sprintf(g_text_buf2, txt[10], dmg_min, dmg_max);
			strcat(g_text_output_buf, g_text_buf2);
			NEWLINE();
			sprintf(g_text_buf2, txt[11], wp->at_mod, wp->pa_mod);
			strcat(g_text_output_buf, g_text_buf2);
			NEWLINE();
			nl = 0;
		} else if (g_items[id].armor) {
			/* Ruestung: RS/BE, BE halbiert auf AT/PA */
			NEWLINE();
			NEWLINE();
			ap = &g_armors_table[g_items[id].table_index];
			at_mod = pa_mod = ap->be / 2;
			if (ap->be & 1)
				pa_mod++;
			at_mod = -at_mod;
			pa_mod = -pa_mod;
			sprintf(g_text_buf2, txt[12], ap->rs, ap->be);
			strcat(g_text_output_buf, g_text_buf2);
			NEWLINE();
			sprintf(g_text_buf2, txt[11], at_mod, pa_mod);
			strcat(g_text_output_buf, g_text_buf2);
			NEWLINE();
			nl = 0;
		}
		if (id == ITEM_KEYRING) {
			signed short i;
			signed short cnt;

			cnt = 0;
			for (i = 0; i < 26; i++)
				if (box_has(h, pos, i))
					cnt++;
			if (cnt) {
				sprintf(g_text_buf2, txt[0], cnt);
				strcat(g_text_output_buf, g_text_buf2);
			}
		}
		if (id == ITEM_RECIPE_BOOK) {
			signed short i;
			signed short cnt;

			cnt = 0;
			for (i = 0; i <= 13; i++)
				if (box_has(h, pos, i))
					cnt++;
			if (cnt) {
				sprintf(g_text_buf2, txt[1], cnt);
				strcat(g_text_output_buf, g_text_buf2);
			}
		}
		/* Zustand */
		if (h->inv[pos].broken) {
			NEWLINE();
			strcat(g_text_output_buf, txt[13]);
			nl = 1;
		} else if (!!h->inv[pos].rs_lost) {
			if (g_items[id].armor && id != 0x32 && id != 0x33 && id != 0x138) {
				NEWLINE();
				strcat(g_text_output_buf, txt[15]);
				nl = 1;
			}
		} else if (g_items[id].weapon) {
			if ((state = hero_weapon_state(h, pos)) > 1) {
				NEWLINE();
				if (state == 2)
					strcat(g_text_output_buf, txt[19]);
				else
					strcat(g_text_output_buf, txt[20]);
				nl = 1;
			}
		}
		if (!can_use_item(h, id)) {
			NEWLINE();
			strcat(g_text_output_buf, txt[7]);
			nl = 1;
		}
		if (h->inv[pos].magic && h->inv[pos].magic_revealed) {
			NEWLINE();
			strcat(g_text_output_buf, txt[14]);
			nl = 1;
		}
		if (id == 0xc1 || id == 0xc8 || !!h->inv[pos].poison_type) {
			if (h->inv[pos].quantity > 1 && g_items[id].stackable) {
				NEWLINE();
				sprintf(g_text_buf2, txt[27], h->inv[pos].poison_charges);
				strcat(g_text_output_buf, g_text_buf2);
			} else {
				NEWLINE();
				strcat(g_text_output_buf, txt[16]);
			}
			nl = 1;
		}
		if (id == 0x85) {
			NEWLINE();
			sprintf(g_text_buf2, txt[17], h->staffspell_lvl);
			strcat(g_text_output_buf, g_text_buf2);
			nl = 1;
		}
		if (id == 0x102) {
			NEWLINE();
			strcat(g_text_output_buf, txt[25]);
			nl = 1;
		} else if (id == 0x155) {
			NEWLINE();
			strcat(g_text_output_buf, txt[28]);
			if (count_failed_skill(g_group, TA_SPRACHEN, 4) > 0)
				strcat(g_text_output_buf, txt[29]);
			else
				strcat(g_text_output_buf, txt[30]);
			nl = 1;
		}
		if (h->inv[pos].empty) {
			NEWLINE();
			strcat(g_text_output_buf, txt[21]);
			nl = 1;
		} else if (h->inv[pos].half_empty) {
			NEWLINE();
			strcat(g_text_output_buf, txt[22]);
			nl = 1;
		} else if (id == ITEM_WATERSKIN || id == 0xb9) {
			NEWLINE();
			strcat(g_text_output_buf, txt[26]);
			nl = 1;
		}
		/* Gewicht */
		NEWLINE();
		if (nl)
			NEWLINE();
		if (h->inv[pos].quantity > 1 && g_items[id].stackable)
			sprintf(g_text_buf2, txt[8], g_items[id].weight * h->inv[pos].quantity);
		else
			sprintf(g_text_buf2, txt[8], g_items[id].weight);
		strcat(g_text_output_buf, g_text_buf2);
	} else
		strcpy(g_text_output_buf, txt[23]);
	dlg_message(g_dialog, g_text_output_buf);
	mem_free(txt_buf);
	mem_free(txt);
}

void hero_add_rs_lost(struct hero far *h, register signed short pos, register signed char v)
{
	signed char rs;

	if (pos == 10) {
		if ((rs = h->rs_bonus) < v)
			v = rs;
		h->rs_bonus -= v;
	}
	h->inv[pos].rs_lost += v;
	if (h->inv[pos].rs_lost < 0)
		h->inv[pos].rs_lost = 0;
}

void hero_add_inv_f10(struct hero far *h, signed short pos, signed char v)
{
	h->inv[pos].f10 += v;
}

signed short hero_weapon_state(struct hero far *h, signed short pos)
{
	signed short half;
	signed short id;
	register signed short bf;
	register signed short base;

	id = h->inv[pos].item_id;
	bf = h->inv[pos].bf;
	base = g_weapons_table[g_items[id].table_index].bf;
	half = (15 - base) / 2;
	if (bf <= base)
		return 1;
	else if (half + base >= bf)
		return 2;
	else
		return 3;
}

signed short hero_mark_weapon(struct hero far *h, char far *fmt, signed short mode)
{
	signed short ok;
	signed short id;
	signed short i;

	for (i = ok = 0; i < NR_INV_SLOTS; i++) {
		if ((id = h->inv[i].item_id) != 0) {
			if (g_items[id].weapon) {
				if (mode == 0) {
					if (!h->inv[i].broken && !h->inv[i].magic) {
						h->inv[i].magic = 1;
						h->inv[i].magic_revealed = 1;
						sprintf(g_text_output_buf, fmt, grammar_article(0x8000, id, 0), h->name);
						ok = 1;
						break;
					}
				} else if (h->inv[i].broken) {
					h->inv[i].broken = 0;
					sprintf(g_text_output_buf, fmt, grammar_article(0x8000, id, 0), h->name);
					ok = 1;
					break;
				}
			}
		}
	}
	return ok;
}

void startup_equipment(struct hero far *h, signed short unused)
{
	signed char type;
	signed short i;
	signed short gear[5] = { ITEM_WATERSKIN, ITEM_FOOD_PACKAGE, ITEM_FOOD_PACKAGE, ITEM_BLANKET, ITEM_TROUSERS };	/* Grundausstattung jedes Helden */
	signed short pos = 0x12;
	signed short by_type[12][4] = {	/* je Heldentyp 1..12 (Index type - 1), -1 = Ende */
		{ ITEM_DAGGER, ITEM_THROWING_KNIFE, ITEM_THROWING_KNIFE, -1 },	/* HERO_TYPE_JUGGLER */
		{ ITEM_LONGBOW, ITEM_DAGGER, -1, -1 },	/* HERO_TYPE_HUNTER */
		{ ITEM_SWORD, ITEM_DAGGER, ITEM_LEATHER_ARMOR, -1 },	/* HERO_TYPE_WARRIOR */
		{ ITEM_RAPIER, ITEM_DAGGER, ITEM_PICKLOCKS, -1 },	/* HERO_TYPE_ESTRAY */
		{ ITEM_SKRAJA, ITEM_SABRE, ITEM_BRANDY, -1 },	/* HERO_TYPE_THORWALIAN */
		{ ITEM_MACE, ITEM_CROWBAR, ITEM_HAMMER, -1 },	/* HERO_TYPE_DWARF */
		{ ITEM_WITCHES_BROOM, ITEM_EINBEERE, ITEM_EINBEERE, -1 },	/* HERO_TYPE_WITCH */
		{ ITEM_OBSIDIAN_DAGGER, ITEM_WHIRLWEED, ITEM_JORUGA_ROOT, -1 },	/* HERO_TYPE_DRUID */
		{ ITEM_MAGE_STAFF, ITEM_DAGGER, ITEM_WRITING_SET, ITEM_ROBE },	/* HERO_TYPE_MAGE */
		{ ITEM_LONGBOW, ITEM_RAPIER, ITEM_HARP, -1 },	/* HERO_TYPE_GREEN_ELF */
		{ ITEM_SEAL_KILLER, ITEM_SPEAR, ITEM_FLUTE, -1 },	/* HERO_TYPE_ICE_ELF */
		{ ITEM_LONGBOW, ITEM_KNIFE, ITEM_FLUTE, -1 } 	/* HERO_TYPE_SYLVAN_ELF */
	};

	if (h->start_gear == 0) {
		diary_unlock(g_diary);
		type = h->type;
		h->start_gear = 1;
		for (i = 0; i < 5; i++)
			give_hero_new_item(h, gear[i], 1, 1);
		move_item(h, 5, pos);
		give_hero_new_item(h, 0x32, 1, 1);
		move_item(h, 14, pos);
		if (h->sex && type != HERO_TYPE_WARRIOR && type != HERO_TYPE_MAGE) {
			give_hero_new_item(h, 0x30, 1, 1);
			move_item(h, 10, pos);
		}
		/* Originalfehler: != -1 vor i < 4 geprueft */
#if FIX_0201
		/* Fix #201: Grenze vor dem Zugriff pruefen */
		for (i = 0; i < 4 && by_type[type - 1][i] != -1; ) {
#else
		for (i = 0; by_type[type - 1][i] != -1 && i < 4; ) {
#endif
			give_hero_new_item(h, by_type[type - 1][i++], 1, 1);
			if (i == 1)
				move_item(h, INV_SLOT_RIGHT_HAND, pos);
		}
		if (type == HERO_TYPE_WARRIOR)
			move_item(h, 10, get_item_pos(h, 0x35));
		if (type == HERO_TYPE_MAGE)
			move_item(h, 10, get_item_pos(h, 0x4b));
		if (type == HERO_TYPE_HUNTER || type == HERO_TYPE_GREEN_ELF || type == HERO_TYPE_SYLVAN_ELF) {
			give_hero_new_item(h, 10, 1, 0x14);
			move_item(h, INV_SLOT_LEFT_HAND, get_item_pos(h, 10));
		}
		diary_lock(g_diary);
	}
}

signed short hero_weapon_type2(struct hero far *h)
{
	signed short id;

	id = h->inv[INV_SLOT_RIGHT_HAND].item_id;
	if (!g_items[id].weapon || h->inv[INV_SLOT_RIGHT_HAND].broken)
		return -1;
	else
		return g_items[id].subtype;
}

signed short hero_weapon_type(struct hero far *h)
{
	signed short id;
	register signed short type;

	id = h->inv[INV_SLOT_RIGHT_HAND].item_id;
	if (!g_items[id].weapon || h->inv[INV_SLOT_RIGHT_HAND].broken)
		type = -1;
	else {
		type = g_items[id].subtype;
		if (type == TA_SCHUSSWAFFEN || type == TA_WURFWAFFEN)
			type = -1;
	}
	return type;
}

signed short hero_remove_item(struct hero far *h, register signed short pos)
{
	signed short id;
	signed short ret;

	ret = 0;
	id = h->inv[pos].item_id;
	if (id != 0) {
		if (g_items[id].undropable) {
			if (pos < INV_SLOT_KNAPSACK)
				unequip(h, id, pos);
			h->nr_items--;
			h->load -= g_items[id].weight;
			memset(&h->inv[pos], 0, sizeof(struct inv_slot));
			ret = 1;
		} else
			drop_item(h, pos, h->inv[pos].quantity);
	}
	return ret;
}

signed short hero_count_item(struct hero far *h, signed short id)
{
	register signed short n;
	register signed short i;

	n = 0;
	for (i = 0; i < NR_INV_SLOTS; i++)
		if (h->inv[i].item_id == id) {
			if (g_items[id].stackable)
				n += h->inv[i].quantity;
			else
				n++;
		}
	return n;
}
