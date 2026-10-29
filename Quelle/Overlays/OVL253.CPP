/* OVL253 - Overlay O253 (Segment 0x2483 in SCHWEIF.EXE): Methode ItemUse::consume (item_consume) - Nahrung, Getraenk,
 * Kraut oder Trank verzehren (Aufruf aus hero_consume_item O092 und O005 mit einem lokalen ItemUse-Objekt; Texte txt,
 * Meldungsflag verbose, Benutzer user, Ziel target (Besitzer des Platzes), Platz slot, Gegenstand id).
 * Ist der Benutzer bewusstlos (nicht tot/versteinert/abtruennig), meldet Text 113 das Fuettern durch das Ziel bzw.
 * get_first_active_hero und es geht trotzdem weiter. Dann je nach Gegenstandsdefinition g_items[id]: food -> Art 1
 *   isst (sub_hunger um table_index; magischer Brotbeutel bleibt), sonst trinkt (sub_thirst; Wasserschlauch: halb
 *   leer -> leer, Schnaps/Wein/Bier: betrunken, 0x142: Durst 0 und betrunken, Glasflasche zurueck; magischer
 *   Wasserschlauch bleibt); herb_potion -> Kraut (Art 0: nicht essbar g_herbs_uneatable, giftig g_herbs_toxic, sonst
 *   herb_effect) bzw. Trank (Elixier g_elixir_potions: Eigenschaft + 5, schlechtes Elixier g_bad_elixirs: - 7, je
 *   0x1518 Takte per add_effect; Gifttrank g_poison_potions nicht fuer NSC; sonst potion_effect).
 *   Andernfalls Texte 95 bzw. 66 (Held schlaeft/bewusstlos bzw. kann nicht).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL253.CPP   -> 2092/2092 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h, Welle N15: 19.10.2026 GLEICH, cobjfix LINKGLEICH; als Methode von ItemUse)
 * - -Og noetig: g_items[id] steht als CSE in DI (Segment jeweils neu aus ds:b84c), Byte +2 als
 *   Zwischenwert [bp-0fh]; eine Zeigervariable "it" ergibt dagegen einen echten far-Local.
 * - Bitfelder +bc im Original unsigned char: bloße Tests mit (unsigned char) (or al,al), Negationen mit (signed char)
 *   -> cbw.
 * - Inventarbits per inline set_inv_empty/clr_inv_half_empty/set_inv_half_empty -> Parameterkopien [bp-0ah..0eh].
 * - add_effect mit Wort-Argumenten fuer Betrag/Art (gecasteter Funktionszeiger add_effect_cw).
 * - Zielzeiger t im inneren Block deklariert -> [bp-14h] hinter Kopien/Zwischenwert; "t == user".
 * - Der else-Zweig (Texte 95/66) landet hinter dem Hauptepilog mit eigenen Epilogkopien (BCC-Sprungopt.).
 */
#include <stdio.h>
#include "schweif.h"

#define add_effect_cw ((void (*)(struct hero far *, signed short, signed long, signed char, signed short, signed short))add_effect)

/* Nahrung, Getraenk, Kraut oder Trank verzehren (Aufrufer hero_consume_item, O005) */
void ItemUse::consume()
{
	signed short eff;
	signed short up;
	signed short down;
	signed short fed = 0;

	if (user->type != 0 && (unsigned char)user->unconscious && !(signed char)user->dead &&
	    !(signed char)user->petrified && !(signed char)user->renegade) {
		fed = 1;
		if (verbose) {
			struct hero far *t = target;	/* wer ihn fuettert */
			if (t == 0 || t == user)
				t = get_first_active_hero(g_group);
			sprintf(g_text_output_buf, txt[113], user->name, t->name, grammar_pronoun(user->sex, 2, 0));
			dlg_message(g_dialog, g_text_output_buf);
		}
	}

	if (check_hero(user) || fed) {
		if (g_items[id].food) {
			if (g_items[id].subtype == 1) {
				sub_hunger(user, g_items[id].table_index);
				if (verbose)
					dlg_message(g_dialog, txt[30]);
				if (id != ITEM_MAGIC_BREADBAG)
					drop_item(target, slot, 1);
			} else if (!target->inv[slot].empty) {
				sub_thirst(user, g_items[id].table_index);
				if (verbose)
					dlg_message(g_dialog, txt[31]);
				if (id == ITEM_WATERSKIN) {
					if (target->inv[slot].half_empty) {
						target->set_inv_empty(slot);
						target->clr_inv_half_empty(slot);
					} else {
						target->set_inv_half_empty(slot);
					}
				} else if (id == ITEM_BRANDY || id == ITEM_WINE || id == ITEM_BEER) {
					hero_get_drunken(user);
					drop_item(target, slot, 1);
					give_item_at(target, ITEM_FLASK_GLASS, 1, slot);
				} else if (id == 0x142) {
					user->thirst = 0;
					hero_get_drunken(user);
				} else if (id != ITEM_MAGIC_WATERSKIN) {
					drop_item(target, slot, 1);
					if (id != ITEM_BEER)
						give_item_at(target, ITEM_FLASK_GLASS, 1, slot);
				}
			} else {
				if (verbose)
					dlg_message(g_dialog, txt[32]);
			}
			if (g_pp20_index == 0x0e)
				g_refresh = 1;
		} else if (g_items[id].herb_potion) {
			if (g_items[id].subtype == 0) {
				if (is_in_word_array(id, g_herbs_uneatable) != -1) {
					if (verbose)
						dlg_message(g_dialog, txt[33]);
				} else if (is_in_word_array(id, g_herbs_toxic) != -1) {
					if (verbose)
						dlg_message(g_dialog, txt[34]);
				} else {
					drop_item(target, slot, 1);
					*g_text_output_buf = 0;
					herb_effect();
				}
			} else {
				up = is_in_word_array(id, g_elixir_potions);
				down = is_in_word_array(id, g_bad_elixirs);
				if (up != -1) {
					if (user->attrib[up].cur > user->attrib[up].normal) {
						sprintf(g_text_output_buf, txt[126], user->name, g_ttx[up + 0x134]);
						if (verbose)
							dlg_message(g_dialog, g_text_output_buf);
					} else {
						drop_item(target, slot, 1);
						give_item_at(target, ITEM_FLASK_GLASS, 1, slot);
						eff = get_free_effect(user);
						add_effect_cw(user, eff, 0x1518L, up, 5, EFFECT_ATTRIB);
					}
				} else if (down != -1) {
					drop_item(target, slot, 1);
					give_item_at(target, ITEM_FLASK_GLASS, 1, slot);
					eff = get_free_effect(user);
					add_effect_cw(user, eff, 0x1518L, down, -7, EFFECT_ATTRIB);
				} else if (is_in_word_array(id, g_poison_potions) != -1 && user->npc_id != 0) {
					if (verbose)
						dlg_message(g_dialog, txt[81]);
				} else {
					drop_item(target, slot, 1);
					potion_effect();
				}
			}
			if (g_pp20_index == 0x0e)
				g_refresh = 1;
		} else {
			if (verbose)
				dlg_message(g_dialog, txt[28]);
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
