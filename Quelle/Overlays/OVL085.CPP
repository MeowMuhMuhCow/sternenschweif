/* OVL085 - Overlay O085 (Segment 0x20f6 in SCHWEIF.EXE): Inventarfunktionen (vgl. DSA 1 seg105/seg106).
 * pass_item(dst, src, spos, dpos): Gegenstand von src/spos nach dst/dpos bewegen (BE pass_item): leerer Platz ->
 *   Text 0x17 aus 0x2e3; nicht ablegbar (undropable, ausser 0xfc/0x117) -> g_ttx[0x24]; am Koerper (Platz < 15)
 *   Pruefungen can_hero_use_item/can_item_at_pos/two_hand_collision mit Meldungen g_ttx[0xf3..0xf7]; gleicher
 *   stapelbarer Gegenstand -> Anzahl erfragen (dlg_input), bis 99 stapeln, Gift (poison_type/poison_charges)
 *   uebernehmen, Rest per drop_item; sonst Tauschen mit Lastpruefung (KK * 100, g_ttx[0x101]), unequip/
 *   add_equip_boni, Sonderwirkungen 0xa1/0xa3/0x46 (add_skill_value/add_mr_mod).
 * drop_item(h, pos, n): Gegenstand ablegen (BE drop_item), n = -1 fragen; g_pp20_index == 14 -> g_refresh = 1.
 * give_hero_new_item(h, id, mode, n): neuen Gegenstand geben (Rucksack 15..30; Ladungen g_usable_table, Bruch-
 *   faktor g_weapons_bf); erster Fund eines Gegenstands ohne item_desc.f0b wird ueber die Datei 0x36b (g_ofile,
 *   Merker je id) einmal ins Tagebuch (diary_add, g_diary) geschrieben, wenn g_diary_new_items gesetzt ist.
 * give_item_at(h, id, n, pos): Gegenstand in einen bestimmten freien Platz legen, add_equip_boni(h, h, id, -1, pos).
 * hero_take_key(h, slot, id): Platz slot enthaelt den Schluesselbund 0x145 -> Fach mit Schluessel id (Tabelle
 *   g_keyring_keys, 27 Eintraege gesucht) per box_set(.., 0) leeren -> 1, sonst 0.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL085.CPP   -> 7046/7046 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (ohne -Og 5532 Abweichungen, mit -rd bzw. -Oe falsch). (Namensfassung mit schweif.h: 19.10.2026 GLEICH,
 *   LINKGLEICH)
 * - -Og: Zwischenwerte &src->inv[spos]/&dst->inv[dpos]/&g_items[src_id]/&g_items[dst_id] ([bp-38h..-3eh]), Flagbyte
 *   [bp-7]/[bp-9]/[bp-3]; -Oi: memset (rep stosw/stosb) und die inline-Strukturzuweisung inv[k] = s.
 * - hero-inline-Elementfunktionen erklaeren die vielen Kopien in pass_item/drop_item: add_load, add_inv_quantity/
 *   set_inv_quantity (char), set_inv_poison_type (int), set_inv_poison_charges (char), get_inv(k) (SCOPY@ in
 *   Temporaere), set_inv(k, struct inv_slot s) (rep movsw; mit benannter Variable als Argument zusaetzliche
 *   Wertkopie [bp-0a0h]).
 * - Die Tausch-Variable tmp und die Tauschfolge stehen auf Funktionsebene - in einem eigenen Block landen
 *   die inline-Kopien hinter den -Og-Zwischenwerten.
 * - Giftblock 1: src_type/src_charges auf dem Stack, dst_charges/dst_type/moved register (CX/SI/DI), moved per
 *   if/else (nicht ?:), "dst_type = src_type". Giftblock 2: zusaetzliche Locals ("dst_charges = moved;
 *   dst_type = src_type;") - ein aeusseres inline setpoison() erzeugt zwar die Kopien, aber -Og fasst dann dpos*27
 *   zusammen (Original kopiert dpos je Setter neu).
 * - Item-Feld magic (+0c): "!!g_items[id].magic" (mov al/cbw/or ax,ax), sonst cmp byte. inv_slot.magic als
 *   unsigned-short-Bit (frueher unsigned char) ergibt beim Setzen dieselben Bytes.
 * Originalfehler/Auffaelliges: pass_item prueft den Zielgegenstand am Quellplatz mit can_item_at_pos/
 *   two_hand_collision fuer dst statt src (can_hero_use_item und die Meldungen nehmen src); pass_item meldet bei
 *   leerem Quellplatz und bei nicht ablegbarem Quellgegenstand Erfolg (ret = 1), beim Zielgegenstand 0;
 *   Lastpruefung in pass_item nur mit KK cur, in give_hero_new_item mit KK cur + mod; give_hero_new_item liest in der
 *   Platzsuche inv[31] (= load), bevor slot < 31 geprueft wird (wie DSA 1); "if (src_id != 0)" nach dem Tausch ist
 *   immer wahr.
 */
#include <stdio.h>
#include <mem.h>
#include "schweif.h"

/* Gegenstand von src/spos nach dst/dpos bewegen bzw. tauschen */
signed short pass_item(struct hero far *dst, struct hero far *src, signed short spos, signed short dpos)
{
	signed short src_id;
	signed short dst_id;
	signed short n;
	signed short swap;
	signed short src_weight;
	signed short dst_weight;
	signed short ret = 1;
	struct inv_slot tmp;

	if (src == dst && spos == dpos) return ret;

	src_id = src->inv[spos].item_id;
	if (src_id == 0) {
		void far *txt_buf;
		char far * far *txt;
		txt_load_list(g_font, 0x2e3, 0x20, &txt_buf, &txt);
		dlg_message(g_dialog, txt[0x17]);
		mem_free(txt_buf);
		mem_free(txt);
		return ret;
	}
	dst_id = dst->inv[dpos].item_id;

	if (g_items[src_id].undropable && src_id != 0xfc && src_id != 0x117) {
		sprintf(g_text_output_buf, g_ttx[0x24], grammar_article(0x8002, src_id, 0), src->name);
		dlg_message(g_dialog, g_text_output_buf);
		return ret;
	}
	if (g_items[dst_id].undropable && dst_id != 0xfc && dst_id != 0x117) {
		sprintf(g_text_output_buf, g_ttx[0x24], grammar_article(0x8002, dst_id, 0), dst->name);
		dlg_message(g_dialog, g_text_output_buf);
		ret = 0;
		return ret;
	}

	if (dpos < 15) {
		if (!can_hero_use_item(dst, src_id)) {
			sprintf(g_text_output_buf, g_ttx[0xf3], dst->name,
				g_ttx[(dst->sex ? 0x3e : 0x32) + dst->type], grammar_article(2, src_id, 0));
			dlg_message(g_dialog, g_text_output_buf);
			ret = 0;
			return ret;
		} else if (!can_item_at_pos(dst, src_id, dpos)) {
			if (is_in_word_array(src_id, g_items_plural) != -1) {
				sprintf(g_text_output_buf, g_ttx[0xf4], grammar_article(0x4000, src_id, 0), g_ttx[0xf5]);
			} else {
				sprintf(g_text_output_buf, g_ttx[0xf4], grammar_article(0, src_id, 0), g_ttx[0xf6]);
			}
			dlg_message(g_dialog, g_text_output_buf);
			ret = 0;
			return ret;
		} else if (two_hand_collision(dst, src_id, dpos)) {
			sprintf(g_text_output_buf, g_ttx[0xf7], dst->name, grammar_pronoun(dst->sex, 0, 0));
			dlg_message(g_dialog, g_text_output_buf);
			ret = 0;
			return ret;
		}
	}

	if (dst_id != 0 && spos < 15) {
		if (!can_hero_use_item(src, dst_id)) {
			sprintf(g_text_output_buf, g_ttx[0xf3], src->name,
				g_ttx[(src->sex ? 0x3e : 0x32) + src->type], grammar_article(2, dst_id, 0));
			dlg_message(g_dialog, g_text_output_buf);
			ret = 0;
			return ret;
#if FIX_0185
		/* Fix #185: Zielgegenstand am Quellplatz beim Quellhelden src pruefen */
		} else if (!can_item_at_pos(src, dst_id, spos)) {
#else
		} else if (!can_item_at_pos(dst, dst_id, spos)) {
#endif
			if (is_in_word_array(dst_id, g_items_plural) != -1) {
				sprintf(g_text_output_buf, g_ttx[0xf4], grammar_article(0x4000, dst_id, 0), g_ttx[0xf5]);
			} else {
				sprintf(g_text_output_buf, g_ttx[0xf4], grammar_article(0, dst_id, 0), g_ttx[0xf6]);
			}
			dlg_message(g_dialog, g_text_output_buf);
			ret = 0;
			return ret;
#if FIX_0185
		/* Fix #185: Zweihandpruefung am Quellplatz beim Quellhelden src */
		} else if (two_hand_collision(src, dst_id, spos)) {
#else
		} else if (two_hand_collision(dst, dst_id, spos)) {
#endif
			sprintf(g_text_output_buf, g_ttx[0xf7], src->name, grammar_pronoun(src->sex, 0, 0));
			dlg_message(g_dialog, g_text_output_buf);
			ret = 0;
			return ret;
		}
	}

	if (dst_id != 0) {
		swap = 1;
		if (dst_id == src_id && g_items[dst_id].stackable) {
			swap = 0;
			n = 1;
			if (src->inv[spos].quantity > 1) {
				sprintf(g_text_output_buf, g_ttx[0x100], src->inv[spos].quantity, grammar_article(6, src_id, 0), src->name);
				n = dlg_input(g_dialog, 2, g_text_output_buf, 0, src->inv[spos].quantity, src->inv[spos].quantity);
			}
			if (src->inv[spos].quantity < n) n = src->inv[spos].quantity;
			if (n > 0) {
				if (dst->inv[dpos].quantity < 99) {
					if (dst->inv[dpos].quantity + n > 99) n = 99 - dst->inv[dpos].quantity;
#if FIX_0185
					/* Fix #185: Last einheitlich mit KK + Mod (wie give_hero_new_item) */
					while ((dst->attrib[ATTRIB_KK].cur + dst->attrib[ATTRIB_KK].mod) * 100 <= dst->load + g_items[src_id].weight * n && n > 0) n--;
#else
					while (dst->attrib[ATTRIB_KK].cur * 100 <= dst->load + g_items[src_id].weight * n && n > 0) n--;
#endif
					if (n > 0) {
						if (src->inv[spos].poison_charges > 0) {
							signed short src_type;
							signed short src_charges;
							register signed short dst_charges;
							register signed short dst_type;
							register signed short moved;
							src_charges = src->inv[spos].poison_charges;
							src_type = src->inv[spos].poison_type;
							dst_charges = dst->inv[dpos].poison_charges;
							dst_type = dst->inv[dpos].poison_type;
							if (src_charges < n) moved = src_charges; else moved = n;
							if (src_type == dst_type) dst_charges += moved; else dst_charges = moved;
							dst_type = src_type;
							dst->set_inv_poison_type(dpos, dst_type);
							dst->set_inv_poison_charges(dpos, dst_charges);
						}
						dst->add_load(g_items[src_id].weight * n);
						dst->add_inv_quantity(dpos, n);
						drop_item(src, spos, n);
					} else {
						sprintf(g_text_output_buf, g_ttx[0x101], dst->name);
						dlg_message(g_dialog, g_text_output_buf);
					}
				}
			}
			if (src->inv[spos].quantity != 0) g_refresh = ret = 0;
		}
		if (swap == 1) {
			src_weight = g_items[src_id].stackable ? src->inv[spos].quantity * g_items[src_id].weight : g_items[src_id].weight;
			dst_weight = g_items[dst_id].stackable ? dst->inv[dpos].quantity * g_items[dst_id].weight : g_items[dst_id].weight;
#if FIX_0185
			/* Fix #185: Last einheitlich mit KK + Mod */
			if ((dst->attrib[ATTRIB_KK].cur + dst->attrib[ATTRIB_KK].mod) * 100 <= dst->load + src_weight - dst_weight) {
#else
			if (dst->attrib[ATTRIB_KK].cur * 100 <= dst->load + src_weight - dst_weight) {
#endif
				sprintf(g_text_output_buf, g_ttx[0x101], dst->name);
				dlg_message(g_dialog, g_text_output_buf);
			} else {
				if (spos < 15) {
					unequip(src, src_id, spos);
					add_equip_boni(src, dst, dst_id, dpos, spos);
				}
				if (dpos < 15) {
					unequip(dst, dst_id, dpos);
					add_equip_boni(dst, src, src_id, spos, dpos);
				}
				tmp = dst->get_inv(dpos);
				dst->add_load(-dst_weight);
				dst->set_inv(dpos, src->get_inv(spos));
				dst->add_load(src_weight);
				src->add_load(-src_weight);
				src->set_inv(spos, tmp);
				src->add_load(dst_weight);
				if (src_id != 0) {
					src->nr_items -= 1;
					dst->nr_items += 1;
				}
				if (dst_id != 0) {
					src->nr_items += 1;
					dst->nr_items -= 1;
				}
				if (dst_id == 0xa1) {
					add_skill_value(src, TA_PFLANZENKUNDE, 3);
					add_skill_value(dst, TA_PFLANZENKUNDE, -3);
				}
				if (dst_id == 0xa3) {
					add_mr_mod(src, 5);
					add_mr_mod(dst, -5);
				}
				if (dst_id == 0x46) {
					add_skill_value(src, TA_GEFAHRENSINN, 2);
					add_skill_value(dst, TA_GEFAHRENSINN, -2);
				}
				if (src_id == 0xa1) {
					add_skill_value(src, TA_PFLANZENKUNDE, -3);
					add_skill_value(dst, TA_PFLANZENKUNDE, 3);
				}
				if (src_id == 0xa3) {
					add_mr_mod(src, -5);
					add_mr_mod(dst, 5);
				}
				if (src_id == 0x46) {
					add_skill_value(src, TA_GEFAHRENSINN, -2);
					add_skill_value(dst, TA_GEFAHRENSINN, 2);
				}
			}
		}
	} else {
		if (g_items[src_id].stackable) {
			n = 1;
			if (src->inv[spos].quantity > 1) {
				sprintf(g_text_output_buf, g_ttx[0x100], src->inv[spos].quantity, grammar_article(6, src_id, 0), src->name);
				n = dlg_input(g_dialog, 2, g_text_output_buf, 0, src->inv[spos].quantity, src->inv[spos].quantity);
			}
			if (n > 0) {
				if (src->inv[spos].quantity < n) n = src->inv[spos].quantity;
#if FIX_0185
				/* Fix #185: Last einheitlich mit KK + Mod (wie give_hero_new_item) */
				while ((dst->attrib[ATTRIB_KK].cur + dst->attrib[ATTRIB_KK].mod) * 100 <= dst->load + g_items[src_id].weight * n && n > 0) n--;
#else
				while (dst->attrib[ATTRIB_KK].cur * 100 <= dst->load + g_items[src_id].weight * n && n > 0) n--;
#endif
				if (n > 0) {
					dst->set_inv(dpos, src->get_inv(spos));
					if (src->inv[spos].poison_charges > 0) {
						signed short src_type;
						signed short dst_type;
						signed short dst_charges;
						register signed short src_charges;
						register signed short moved;
						src_charges = src->inv[spos].poison_charges;
						src_type = src->inv[spos].poison_type;
						if (src_charges < n) moved = src_charges; else moved = n;
						dst_charges = moved;
						dst_type = src_type;
						dst->set_inv_poison_type(dpos, dst_type);
						dst->set_inv_poison_charges(dpos, dst_charges);
					}
					dst->add_load(g_items[src_id].weight * n);
					dst->set_inv_quantity(dpos, n);
					drop_item(src, spos, n);
					dst->nr_items += 1;
				} else {
					sprintf(g_text_output_buf, g_ttx[0x101], dst->name);
					dlg_message(g_dialog, g_text_output_buf);
				}
			}
		} else {
#if FIX_0185
			/* Fix #185: Last einheitlich mit KK + Mod */
			if ((dst->attrib[ATTRIB_KK].cur + dst->attrib[ATTRIB_KK].mod) * 100 <= dst->load + g_items[src_id].weight) {
#else
			if (dst->attrib[ATTRIB_KK].cur * 100 <= dst->load + g_items[src_id].weight) {
#endif
				sprintf(g_text_output_buf, g_ttx[0x101], dst->name);
				dlg_message(g_dialog, g_text_output_buf);
			} else {
				if (spos < 15) unequip(src, src_id, spos);
				if (dpos < 15) add_equip_boni(dst, src, src_id, spos, dpos);
				dst->set_inv(dpos, src->get_inv(spos));
				dst->add_load(g_items[src_id].weight);
				src->add_load(-g_items[src_id].weight);
				src->nr_items -= 1;
				dst->nr_items += 1;
				memset(&src->inv[spos], 0, sizeof(struct inv_slot));
				if (src_id == 0xa1) {
					add_skill_value(src, TA_PFLANZENKUNDE, -3);
					add_skill_value(dst, TA_PFLANZENKUNDE, 3);
				}
				if (src_id == 0xa3) {
					add_mr_mod(src, -5);
					add_mr_mod(dst, 5);
				}
				if (src_id == 0x46) {
					add_skill_value(src, TA_GEFAHRENSINN, -2);
					add_skill_value(dst, TA_GEFAHRENSINN, 2);
				}
			}
		}
		if (src->inv[spos].quantity != 0) g_refresh = ret = 0;
	}
	return ret;
}

/* Gegenstand ablegen (DSA 1 drop_item): n Stueck, -1 = fragen */
signed short drop_item(struct hero far *h, signed short pos, signed short n)
{
	signed short id;
	signed short ret = 0;

	id = h->inv[pos].item_id;
	if (id != 0) {
		if (g_items[id].undropable) {
			sprintf(g_text_buf2, g_ttx[0x24], grammar_article(0x8002, id, 0), h->name);
			dlg_message(g_dialog, g_text_buf2);
		} else if (g_items[id].stackable) {
			if (h->inv[pos].quantity != 1) {
				if (n == -1) {
					sprintf(g_text_buf2, g_ttx[0x25], grammar_article(6, id, 0));
					n = dlg_input(g_dialog, 2, g_text_buf2, 0, h->inv[pos].quantity, h->inv[pos].quantity);
				}
			} else {
				sprintf(g_text_buf2, g_ttx[0x26], grammar_article(n == 1 ? 0x8002 : 0x8006, id, 0));
				if (!(n != -1 || dlg_yes_no(g_dialog, g_text_buf2) == 1)) {
				} else {
					n = 1;
				}
			}
			if (n != -1) {
				if (h->inv[pos].quantity > n) {
					h->inv[pos].quantity -= n;
					h->load -= g_items[id].weight * n;
					ret = 1;
					if (h->inv[pos].poison_charges != 0) {
						signed short c;
						c = h->inv[pos].poison_charges - n;
						if (c <= 0) {
							c = 0;
							h->set_inv_poison_type(pos, 0);
						}
						h->set_inv_poison_charges(pos, c);
					}
				} else {
					h->load -= g_items[id].weight * h->inv[pos].quantity;
					memset(&h->inv[pos], 0, sizeof(struct inv_slot));
					h->nr_items -= 1;
					ret = 1;
				}
			}
		} else {
			sprintf(g_text_buf2, g_ttx[0x26], grammar_article(0x8002, id, 0));
			if (!(n != -1 || dlg_yes_no(g_dialog, g_text_buf2) == 1)) {
			} else {
				if (pos < 15) unequip(h, id, pos);
				h->load -= g_items[id].weight;
				if (id == 0xa1) add_skill_value(h, TA_PFLANZENKUNDE, -3);
				if (id == 0xa3) add_mr_mod(h, -5);
				if (id == 0x46) add_skill_value(h, TA_GEFAHRENSINN, -2);
				memset(&h->inv[pos], 0, sizeof(struct inv_slot));
				h->nr_items -= 1;
				ret = 1;
			}
		}
	}
	if (g_pp20_index == 14) g_refresh = 1;
	return ret;
}

/* neuen Gegenstand geben (DSA 1 give_hero_new_item): mode 0 still, 1 Meldung, 2 Last egal */
signed short give_hero_new_item(struct hero far *h, register signed short id, signed short mode, signed short n)
{
	signed short slot;
	signed short stack_pos;
	signed short ret = 0;
	signed short done;

	if (mode != 2 && (signed char)(h->attrib[ATTRIB_KK].cur + h->attrib[ATTRIB_KK].mod) * 100 <= h->load) {
		if (mode != 0) {
			sprintf(g_text_output_buf, g_ttx[0x27], h->name, grammar_pronoun(h->sex, 3, 0));
			dlg_message(g_dialog, g_text_output_buf);
		}
	} else if (g_items[id].stackable && (stack_pos = has_hero_stacked(h, id)) != -1) {
		if (h->inv[stack_pos].quantity + n > 99) n = 99 - h->inv[stack_pos].quantity;
		h->inv[stack_pos].quantity += n;
		h->load += g_items[id].weight * n;
		ret = n;
	} else if (h->nr_items < 0x1f) {
		done = 0;
		do {
			if (h->nr_items < 0x1f) {
				slot = 14;
#if FIX_0185
				/* Fix #185: Grenze vor dem Zugriff pruefen (nicht inv[31] lesen) */
				while (++slot < 31 && h->inv[slot].item_id != 0) ;
#else
				while (h->inv[++slot].item_id != 0 && slot < 31) ;
#endif
				if (slot < 31) {
					if (n > 99) n = 99;
					h->nr_items++;
					h->inv[slot].item_id = id;
					h->inv[slot].quantity = g_items[id].stackable ? n :
						g_items[id].useable ? g_usable_table[g_items[id].table_index].charges : 0;
					if (!!g_items[id].magic) h->inv[slot].magic = 1;
					if (g_items[id].weapon) h->inv[slot].bf = g_weapons_bf[g_items[id].table_index * 8];
					if (g_items[id].stackable) {
						h->load += g_items[id].weight * n;
						ret = n;
						n = 0;
					} else {
						h->load += g_items[id].weight;
						n--;
						ret++;
					}
					if (n == 0) done = 1;
					if (id == 0xa1) add_skill_value(h, TA_PFLANZENKUNDE, 3);
					if (id == 0xa3) add_mr_mod(h, 5);
					if (id == 0x46) add_skill_value(h, TA_GEFAHRENSINN, 2);
					if (!g_items[id].f0b) {
						char far *buf;
						buf = new char[g_nr_items];
						ofile_open(g_ofile, 0x36b);
						ofile_read(g_ofile, buf, (signed long)g_nr_items);
						ofile_close(g_ofile);
						if (g_diary_new_items != 0 && buf[id] == 1) {
							void far *txt_buf;
							char far * far *txt;
							char far *msg;
							msg = new char[0x80];
							buf[id] = 0;
							ofile_create(g_ofile, 0x36b, 0x8002);
							ofile_write(g_ofile, buf, (signed long)g_nr_items);
							ofile_close(g_ofile);
							txt_load_list(g_font, 0x2e3, 0x20, &txt_buf, &txt);
							if (ret > 1 || is_in_word_array(id, g_items_plural) != -1) {
								sprintf(msg, txt[0x18], h->name, grammar_article(0x4006, id, 0));
							} else {
								sprintf(msg, txt[0x18], h->name, grammar_article(2, id, 0));
							}
							diary_add(g_diary, msg, 0);
							mem_free(txt_buf);
							mem_free(txt);
							delete msg;
						}
						delete buf;
					}
				} else {
					done = 1;
				}
			} else {
				done = 1;
			}
		} while (done == 0);
	}
	if (g_pp20_index == 14 || g_pp20_index == 15) g_refresh = 1;
	return ret;
}

/* Gegenstand id (n Stueck) in einen bestimmten freien Platz legen */
void give_item_at(struct hero far *h, register signed short id, signed short n, signed short pos)
{
	if (h->inv[pos].item_id == 0) {
		if (n > 99) n = 99;
		h->nr_items++;
		h->inv[pos].item_id = id;
		h->inv[pos].quantity = g_items[id].stackable ? n :
			g_items[id].useable ? g_usable_table[g_items[id].table_index].charges : 0;
		if (!!g_items[id].magic) h->inv[pos].magic = 1;
		if (g_items[id].weapon) h->inv[pos].bf = g_weapons_bf[g_items[id].table_index * 8];
		h->load += g_items[id].weight * n;
		if (id == 0xa1) add_skill_value(h, TA_PFLANZENKUNDE, 3);
		if (id == 0xa3) add_mr_mod(h, 5);
		if (id == 0x46) add_skill_value(h, TA_GEFAHRENSINN, 2);
		add_equip_boni(h, h, id, -1, pos);
		if (g_pp20_index == 14 || g_pp20_index == 15) g_refresh = 1;
	}
}

/* Schluessel id aus dem Schluesselbund (ITEM_KEYRING) im Platz slot nehmen (box_set(.., 0)) */
signed short hero_take_key(struct hero far *h, register signed short slot, signed short id)
{
	register signed short i;

	if (h->inv[slot].item_id == ITEM_KEYRING) {
		for (i = 0; i <= 0x1a; i++) {
			if (g_keyring_keys[i] == id) {
				box_set(h, slot, i, 0);
				return 1;
			}
		}
	}
	return 0;
}
