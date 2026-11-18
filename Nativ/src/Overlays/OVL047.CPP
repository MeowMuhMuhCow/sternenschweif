/* OVL047 - Overlay O047 (Segment 0x2012 in SCHWEIF.EXE): Klasse ItemUse "Gegenstand benutzen" (0x22 Bytes, s_class.h;
 * Texte txt_buf/txt per txt_load_list(g_font, 0x21, 0x82), Meldungsflag verbose, Methodenzeiger func (8 Bytes), user,
 * target, slot, id, zweiter Platz slot2, zweiter Gegenstand id2). Konstruktoren (user, target, slot, id, verbose;
 * slot2/id2 = -1) und (user, slot, id, slot2, id2, verbose; target 0), Destruktor. use(): Platz leer -> txt[93];
 * handlungsfaehig (check_hero): Gegenstandsdefinition g_items Bits useable/food/herb_potion -> hero_consume_item,
 * "nicht benutzbar"-Texte (txt[0]/txt[1], Name ueber g_items_plural), can_use_item-Pruefung (g_ttx[243], Typtext),
 * Listen g_items_attack/g_items_ask_use mit Rueckfrage dlg_choose, sonst Methodenzeiger aus g_item_use_funcs[g_usable_table[..].func]
 * nach func und Aufruf; sonst Texte 95/66 (schlaeft bzw. bewusstlos / kann nicht) bzw. hero_consume_item fuer
 * Nahrung und Kraeuter. combine(): zwei Gegenstaende, combo_index (O048) liefert Index in g_item_use_funcs (direkter
 * Aufruf (this->*g_item_use_funcs[k])()), sonst txt[60] mit toupper. use_arcano/use_armatrutz/use_flim_flam/
 * use_ignifaxius: Zauber 0x3d/0x3e/0x52/0x35 (Namen nach der DSA1-Liste) ueber ein lokales Cast-Objekt (O233),
 * Ladung des Platzes -1 (inline dec_inv_quantity), g_spell_by_item/g_spell_cost_mod. read_text: Rezept-/Schriftentext je
 * Gegenstand (txt[17] mit txt[4..16]/txt[119]). read_special: Lesen/Benutzen von Sondergegenstaenden (switch mit
 * Sprungtabelle 0x14b..0x15a, show_fullscreen_pic/show_fullscreen_pic2, g_heather_recipe_read/g_ingerimm_verse_read/g_parchments_read, Probe test_skill +
 * timewarp_minutes(g_date, 30L)), Ausgabe in Farbe 7. use_with_item: Gegenstand auf einen zweiten anwenden
 * (select_item, 0xef heilt W(10..25)). poison_weapon: Waffe (rechte Hand) mit Gift/Oel bestreichen (Liste
 * g_weapons_no_poison, Bits weapon/stackable, poison_type/poison_charges, drop_item/give_item_at).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL047.CPP   -> 6191/6191 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (_g_3223 -> _g_3222+1, _g_5024 -> _g_4ff0+0x34). Ohne -Og 4473 Abweichungen.
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH; g_usable_table[..].func trifft _g_3222+1)
 * - Methodenzeiger wie OVL194: "class ItemUse;" vor "typedef void (ItemUse::*ITEMUSE_PMF)();" (s_class.h).
 *   "func = g_item_use_funcs[..]; (this->*func)();" -> SCOPY@ cx=8 + call es:[bx+0ah]; "(this->*g_item_use_funcs[k])();"
 *   direkt -> call dword ptr [bx+4d38h] ohne Kopie.
 * - use(): Bitbyte +2 der Definition als -Og-Zwischenwert [bp-1]; r (dlg_choose) im inneren Block -> [bp-4],
 *   eingebettet "(r = ..) == 1". flag nur im else-Block als register deklariert -> SI (Funktionsebene ergab DI).
 *   Held +bc: Original mit unsigned-char-Bits -> "(unsigned char)user->asleep || (unsigned char)user->unconscious"
 *   (and ax / or al,al; ohne Cast or ax,ax), "!(signed char)user->dead" usw.
 * - read_special: "r = dlg_yes_no(..); if (r == 1)" getrennt -> mov di,ax / cmp di,1.
 * - poison_weapon: poison_type direkt gesetzt, poison_charges ueber inline set_weapon_poison_charges(n)
 *   (Parameterkopie je case [bp-7..-0eh]); bonus/poison ohne register in CX/DX (mit register vertauscht).
 * - Klasse Cast (O233) aus s_class.h.
 * Originalfehler/Auffaelligkeiten: read_text hat kein default - bei anderer Gegenstandsnummer bekommt sprintf einen
 *   uninitialisierten Zeiger s. poison_weapon: poison bleibt uninitialisiert, wenn der benutzte Gegenstand keiner
 *   der acht Gift-/Oelgegenstaende ist.
 */
#include <stdio.h>
#include <ctype.h>
#include "schweif.h"

ItemUse::ItemUse(struct hero far *u, struct hero far *t, signed short s, signed short i, signed short f)
{
	txt_load_list(g_font, 0x21, 0x82, &txt_buf, &txt);
	user = u;
	target = t;
	slot = s;
	id = i;
	slot2 = -1;
	id2 = -1;
	verbose = f;
}

ItemUse::ItemUse(struct hero far *u, signed short s, signed short i, signed short s2, signed short i2, signed short f)
{
	txt_load_list(g_font, 0x21, 0x82, &txt_buf, &txt);
	user = u;
	target = 0;
	slot = s;
	id = i;
	slot2 = s2;
	id2 = i2;
	verbose = f;
}

ItemUse::~ItemUse()
{
	mem_free(txt_buf);
	mem_free(txt);
}

void ItemUse::use()
{
	if (id == 0) {
		if (verbose)
			dlg_message(g_dialog, txt[93]);
		return;
	}
	if (check_hero(user)) {
		if (!g_items[id].useable) {
			if (g_items[id].food) {
				hero_consume_item(user, user, slot);
			} else {
				if (is_in_word_array(id, g_items_plural) != -1)
					sprintf(g_text_output_buf, txt[0], grammar_name(g_item_names[id]));
				else
					sprintf(g_text_output_buf, txt[1], grammar_article(0, id, 0));
				if (verbose)
					dlg_message(g_dialog, g_text_output_buf);
			}
		} else if (!can_use_item(user, id)) {
			if (verbose) {
				sprintf(g_text_output_buf, g_ttx[243], user->name,
					g_ttx[(user->sex ? 0x3e : 0x32) + user->type], grammar_article(2, id, 0));
				dlg_message(g_dialog, g_text_output_buf);
			}
		} else if (g_items[id].herb_potion && is_in_word_array(id, g_items_attack) == -1) {
			if (is_in_word_array(id, g_items_ask_use) != -1) {
				signed short r;
				if ((r = dlg_choose(g_dialog, txt[85], 2, txt[82], txt[84])) == 1) {
					hero_consume_item(user, user, slot);
				} else if (r != -1) {
					func = g_item_use_funcs[g_usable_table[g_items[id].table_index].func];
					(this->*func)();
				}
			} else {
				hero_consume_item(user, user, slot);
			}
		} else if (user->inv[slot].quantity <= 0) {
			if (verbose) {
				sprintf(g_text_output_buf, txt[2], grammar_article(0x8000, id, 0));
				dlg_message(g_dialog, g_text_output_buf);
			}
		} else {
			func = g_item_use_funcs[g_usable_table[g_items[id].table_index].func];
			(this->*func)();
		}
	} else if ((unsigned char)user->asleep || (unsigned char)user->unconscious) {
		register signed short flag = 0;

		if (user->type != 0 && !(signed char)user->dead && !(signed char)user->petrified &&
		    !(signed char)user->renegade) {
			if (!g_items[id].useable) {
				if (g_items[id].food)
					flag = 1;
			} else if (g_items[id].herb_potion && is_in_word_array(id, g_items_attack) == -1) {
				flag = 1;
			}
		}
		if (flag) {
			hero_consume_item(user, user, slot);
		} else {
			sprintf(g_text_output_buf, txt[95], user->name);
			if (verbose)
				dlg_message(g_dialog, g_text_output_buf);
		}
	} else {
		sprintf(g_text_output_buf, txt[66], user->name);
		if (verbose)
			dlg_message(g_dialog, g_text_output_buf);
	}
}

void ItemUse::combine()
{
	signed short combo;

	if (slot == slot2) {
		use();
		return;
	}
	if (check_hero(user)) {
		if (!g_items[id].useable) {
			if (is_in_word_array(id, g_items_plural) != -1)
				sprintf(g_text_output_buf, txt[0], grammar_name(g_item_names[id]));
			else
				sprintf(g_text_output_buf, txt[1], grammar_article(0, id, 0));
			if (verbose)
				dlg_message(g_dialog, g_text_output_buf);
		} else if (!g_items[id2].useable) {
			if (is_in_word_array(id2, g_items_plural) != -1)
				sprintf(g_text_output_buf, txt[0], grammar_name(g_item_names[id2]));
			else
				sprintf(g_text_output_buf, txt[1], grammar_article(0, id2, 0));
			if (verbose)
				dlg_message(g_dialog, g_text_output_buf);
		} else if ((combo = combo_index(id, id2)) != -1) {
			(this->*g_item_use_funcs[combo])();
		} else {
			sprintf(g_text_output_buf, txt[60], grammar_article(0, id, 0), grammar_article(3, id2, 0));
			*g_text_output_buf = toupper(*g_text_output_buf);
			if (verbose)
				dlg_message(g_dialog, g_text_output_buf);
		}
	} else if ((unsigned char)user->asleep || (unsigned char)user->unconscious) {
		sprintf(g_text_output_buf, txt[95], user->name);
		if (verbose)
			dlg_message(g_dialog, g_text_output_buf);
	} else {
		sprintf(g_text_output_buf, txt[66], user->name);
		if (verbose)
			dlg_message(g_dialog, g_text_output_buf);
	}
}

void ItemUse::use_arcano()
{
	user->fb5 = 99;
	hero_set_target(user, user);
	g_spell_by_item = 1;
	g_spell_cost_mod = -1;
	Cast cast(user);
	cast.cast(0x3d, 0);
	user->dec_inv_quantity(slot);
	user->fb5 = 0;
}

void ItemUse::read_text()
{
	char far *s;

	switch (id) {
	case 0xa7: s = txt[4]; break;
	case 0xa9: s = txt[5]; break;
	case 0xca: s = txt[6]; break;
	case 0xcb: s = txt[7]; break;
	case 0xcc: s = txt[8]; break;
	case 0xcd: s = txt[9]; break;
	case 0xce: s = txt[10]; break;
	case 0xba: s = txt[11]; break;
	case 0xf0: s = txt[12]; break;
	case 0xf2: s = txt[13]; break;
	case 0xf3: s = txt[14]; break;
	case 0xf4: s = txt[15]; break;
	case 0xf1: s = txt[16]; break;
#if FIX_0169
	/* Fix #169: unbekannter Gegenstand: abbrechen statt Muellzeiger an sprintf */
	case 0x152: s = txt[119]; break;
	default: return;
	}
	sprintf(g_text_output_buf, txt[17], s);
#else
	case 0x152: s = txt[119]; break;
	}
	sprintf(g_text_output_buf, txt[17], s);
#endif
	dlg_message(g_dialog, g_text_output_buf);
}

void ItemUse::read_special()
{
	char far *s;
	signed short old;
	signed short r;

	if (user->npc_id == 8) {
		dlg_message(g_dialog, txt[125]);
		return;
	}
	s = 0;
	switch (id) {
	case 0x129: s = txt[69]; break;
	case 0x13f: show_fullscreen_pic(); break;
	case 0x13e: s = txt[72]; break;
	case 0x149: s = txt[94]; break;
	case 0x14b: s = txt[96]; break;
	case 0x14c: s = txt[97]; break;
	case 0x14d: show_fullscreen_pic2(); break;
	case 0x150: s = txt[98]; g_heather_recipe_read = 1; break;
	case 0x12b:
		if (!user->inv[slot].inv_b4) {
			s = txt[106];
		} else {
			r = dlg_yes_no(g_dialog, txt[107]);
			if (r == 1) {
				r = test_skill(user, TA_SPRACHEN, 2);
				timewarp_minutes(g_date, 30L);
				if (r > 0) {
					dlg_message(g_dialog, txt[108]);
					s = txt[106];
					user->clr_inv_b4(slot);
					g_ingerimm_verse_read = 1;
				} else {
					s = txt[109];
				}
			}
		}
		break;
	case 0x13a:
		dlg_message(g_dialog, txt[115]);
		dlg_message(g_dialog, txt[116]);
		dlg_message(g_dialog, txt[117]);
		dlg_message(g_dialog, txt[118]);
		g_parchments_read = 1;
		break;
	case 0x15a: s = txt[121]; break;
	default: s = txt[18]; break;
	}
	if (s) {
		old = dlg_set16(g_dialog, 7);
		dlg_message(g_dialog, s);
		dlg_set16(g_dialog, old);
	}
}

void ItemUse::use_armatrutz()
{
	if (g_in_fight) {
		user->fb5 = 99;
		hero_set_target(user, user);
		Cast cast(user);
		g_spell_by_item = 1;
		g_spell_cost_mod = -1;
		cast.cast(0x3e, 0);
		user->dec_inv_quantity(slot);
		user->fb5 = 0;
		dlg_message(g_dialog, g_text_output_buf);
	} else {
		dlg_message(g_dialog, txt[21]);
	}
}

void ItemUse::use_flim_flam()
{
	user->fb5 = 99;
	g_spell_by_item = 1;
	g_spell_cost_mod = -1;
	Cast cast(user);
	cast.cast(0x52, 0);
	user->fb5 = 0;
	user->dec_inv_quantity(slot);
}

void ItemUse::use_with_item()
{
	if (id2 == -1) {
		slot2 = select_item(user, txt[62], 1);
		id2 = slot2 != -1 ? user->inv[slot2].item_id : 0;
	} else if (id2 == 0x16) {
		id2 = id;
		id = 0x16;
		slot2 = slot;
	}
	if (slot2 != -1 && id2 != 0) {
		if (id2 == 0xef) {
			sub_hero_le(user, random_interval(10, 25));
			drop_item(user, slot2, 1);
			sprintf(g_text_output_buf, txt[65], user->name);
		} else {
			sprintf(g_text_output_buf, txt[63], grammar_article(0x8002, id2, 0));
		}
		dlg_message(g_dialog, g_text_output_buf);
	}
}

void ItemUse::use_ignifaxius()
{
	if (g_in_fight) {
		user->fb5 = 99;
		g_spell_by_item = 1;
		g_spell_cost_mod = -1;
		Cast cast(user);
		cast.cast(0x35, 0);
		user->fb5 = 0;
		user->dec_inv_quantity(slot);
		dlg_message(g_dialog, g_text_output_buf);
	} else {
		dlg_message(g_dialog, txt[21]);
	}
}

void ItemUse::poison_weapon()
{
	signed short weapon;
	signed short charges = 5;
	signed short plural = 0;
	struct item_list7 no_poison = g_weapons_no_poison;

	weapon = user->inv[INV_SLOT_RIGHT_HAND].item_id;
	if (weapon != 0 && is_in_word_array(weapon, no_poison.item) == -1 && g_items[weapon].weapon) {
		if (g_items[weapon].stackable) {
#if FIX_0169
			/* Fix #169: poison vorbelegen (kein Gift-/Oelgegenstand) */
			signed short bonus = 0;
			signed short poison = -1;
#else
			signed short bonus = 0;
			signed short poison;
#endif
			switch (id) {
			case 0xa8: poison = 11; break;
			case 0xa6: poison = 10; break;
			case 0x37: poison = 1; break;
			case 0x38: poison = 2; break;
			case 0x39: poison = 3; break;
			case 0x3a: poison = 4; break;
			case 0x8e: poison = 8; break;
			case 0x90: poison = 6; break;
			}
			if (user->inv[INV_SLOT_RIGHT_HAND].poison_type == poison)
				bonus = user->inv[INV_SLOT_RIGHT_HAND].poison_charges;
			charges = bonus + 10;
			if (user->inv[INV_SLOT_RIGHT_HAND].quantity < charges)
				charges = user->inv[INV_SLOT_RIGHT_HAND].quantity;
			if (charges > 1)
				plural = 1;
		}
		switch (id) {
		case 0xa8: user->inv[INV_SLOT_RIGHT_HAND].poison_type = 11; user->set_weapon_poison_charges(charges); weapon = 0x2a; break;
		case 0xa6: user->inv[INV_SLOT_RIGHT_HAND].poison_type = 10; user->set_weapon_poison_charges(charges); weapon = 0x2a; break;
		case 0x37: user->inv[INV_SLOT_RIGHT_HAND].poison_type = 1; user->set_weapon_poison_charges(charges); weapon = 0x1f; break;
		case 0x38: user->inv[INV_SLOT_RIGHT_HAND].poison_type = 2; user->set_weapon_poison_charges(charges); weapon = 0x1f; break;
		case 0x39: user->inv[INV_SLOT_RIGHT_HAND].poison_type = 3; user->set_weapon_poison_charges(charges); weapon = 0x1f; break;
		case 0x3a: user->inv[INV_SLOT_RIGHT_HAND].poison_type = 4; user->set_weapon_poison_charges(charges); weapon = 0x1f; break;
		case 0x8e: user->inv[INV_SLOT_RIGHT_HAND].poison_type = 8; user->set_weapon_poison_charges(charges); weapon = 0x1f; break;
		case 0x90: user->inv[INV_SLOT_RIGHT_HAND].poison_type = 6; user->set_weapon_poison_charges(charges); weapon = 0x1f; break;
		}
		drop_item(user, slot, 1);
		give_item_at(user, weapon, 1, slot);
		if (plural)
			sprintf(g_text_output_buf, txt[90], grammar_article(0x8004, user->inv[INV_SLOT_RIGHT_HAND].item_id, 0));
		else
			sprintf(g_text_output_buf, txt[19], grammar_article(0x8000, user->inv[INV_SLOT_RIGHT_HAND].item_id, 0));
		dlg_message(g_dialog, g_text_output_buf);
	} else {
		sprintf(g_text_output_buf, txt[20], user->name);
		dlg_message(g_dialog, g_text_output_buf);
	}
}
