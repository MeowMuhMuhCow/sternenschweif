/* OVL252 - Overlay O252 (Segment 0x247e in SCHWEIF.EXE): weitere Methoden der Klasse ItemUse (Gegenstand benutzen,
 * O047; Texte txt, Meldungsflag verbose, Benutzer user, Platz slot, Gegenstand id, zweiter Gegenstand slot2/id2).
 * use_attack_item(): nur ausserhalb des Kampfes (g_in_fight) Ziel-Held per get_any_hero(txt[75]); Goldleim (Gift
 *   POISON_GOLDLEIM, LE - W20, txt[76], danach Glasflasche), Lotusgift (POISON_LOTUSGIFT, 10..40, txt[77]), Bannstaub
 *   (Zauberkundige mit AE: sub_ae_splash 10..20, txt[78], sonst txt[79]), Hylailer Feuer (10..25, txt[65]),
 *   Miasthmaticum (5..10, txt[80]); Gegenstand verbrauchen.
 * play_instrument(): laeuft Musik 5/6 (g_music_id_req), Begleitung per Sound_start(g_snd_music, 3/5/1) fuer Harfe/
 *   Laute/Floete.
 * read_heptagon_book(): txt[22], Zauber Heptagon + 2, verbrauchen.
 * use_light(): Licht - Laterne mit Oel auffuellen (Brenndauer 100, Bronzeflasche bleibt), Zunderkaestchen (Ladungen)
 *   noetig fuer Fackel (-> brennend, Brenndauer 10) bzw. Laterne (Auswahl dlg_choose bei beiden).
 * show_hidden_enemies(): im Kampf fig_hidden_enemies_text(g_fight_obj), Ladung--, sonst txt[21].
 * write_recipe(): Rezeptbuch allein -> read_recipe_book; sonst mit Schreibzeug (Ladungen) ein Rezept ins Rezeptbuch
 *   0xbf bzw. eine Schriftrolle 0x1d (wird zum Rezeptbuch) abschreiben: Rezept in der Hand (item_desc.is_recipe,
 *   g_recipe_items) direkt, sonst Auswahl unter den Rezepten des Helden (g_recipes, Namen g_recipe_outcomes), die
 *   noch nicht im Buch stehen (box_has); box_set, Frage dlg_yes_no -> Rezept wegwerfen, Schreibzeug-Ladung--.
 * read_recipe_book(): Auswahl der Rezepte im Buch und read_text je Rezept.
 * show_note(): txt[71] in Farbe 7.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL252.CPP   -> 4505/4505 Bytes GLEICH (18.10.2026), cobjfix laeuft.
 *   (Namensfassung mit schweif.h, Welle N15: 19.10.2026 GLEICH, cobjfix LINKGLEICH; die freien Funktionen mit
 *   Objektzeiger sind jetzt Methoden von ItemUse - gleicher Code)
 *   Ohne -Og 18 Bytes zu lang; -Oi fuer strcpy (Namensliste).
 * - inline-Elementfunktionen: hero::is_mage() { return type >= 7; } (mov ax,1 / xor ax,ax / or ax,ax),
 *   set_inv_item_id/set_inv_light_timer/dec_inv_quantity (Parameterkopien [bp-8]..[bp-14h] je Aufruf),
 *   item_desc::is_recipe() { return useable && table_index == 2; } mit Offset-Zwischenwert [bp-4]/[bp-6] (Segment
 *   aus ds:b84c).
 * - write_recipe: alle Locals im else-Block -> die -Og-Zwischenwerte stehen oben ([bp-2..6]), Block-Skalare dahinter,
 *   n/found im inneren Block vor den Feldern idx[14]/names[14][20] des aeusseren Blocks.
 * - read_heptagon_book: "spells[SP_HEPTAGON] += (signed char)2" -> add byte es:[..],2 ("+= 2" ergibt mov al / add al /
 *   mov).
 * - read_recipe_book: switch mit einem case (mov ax / cmp ax,0bfh / jz / jmp near); "if (n) {...; return;} else {...}"
 *   und direkt danach "default: break;" (kein break vor default) -> toter jmp short und toter Schlussepilog.
 * - use_light: "torch = lantern = -1" (lantern zuerst); lantern als erste register-Variable deklariert -> SI, torch -> DI.
 * Originalfehler: use_light/write_recipe - leeres Zunderkaestchen/Schreibzeug (quantity <= 0): mit Meldungsflag
 *   erscheint die Meldung in use_light zweimal (am Ende nochmals dlg_message), ohne Flag zeigt use_light den alten
 *   Inhalt von g_text_output_buf. use_light setzt set_inv_item_id(lantern, ITEM_LANTERN_ON) auf einem Platz, der
 *   schon die brennende Laterne enthaelt (wirkungslos).
 */
#include <stdio.h>
#include <string.h>
#include "schweif.h"

/* Wurf-/Giftgegenstand auf einen Helden (nur ausserhalb des Kampfes): Goldleim, Lotusgift, Bannstaub, Hylailer Feuer,
 * Miasthmaticum - Gift setzen, LE bzw. AE abziehen, Gegenstand verbrauchen (Goldleim hinterlaesst die Glasflasche) */
void ItemUse::use_attack_item()
{
	struct hero far *h;
	register signed short rest;

	if (g_in_fight == 0) {
		rest = 0;
		h = get_any_hero(g_group, txt[75]);
		if (h != 0) {
			switch (id) {
			case ITEM_GOLDLEIM:
				rest = ITEM_FLASK_GLASS;
				hero_get_poison(h, POISON_GOLDLEIM);
				sub_hero_le(h, random_interval(1, 20));
				sprintf(g_text_output_buf, txt[76], h->name, grammar_pronoun(h->sex, 0, 0));
				break;
			case ITEM_LOTUS_POISON:
				hero_get_poison(h, POISON_LOTUSGIFT);
				sub_hero_le(h, random_interval(10, 40));
				sprintf(g_text_output_buf, txt[77], h->name);
				break;
			case ITEM_BANNSTAUB:
				if (h->is_mage() && h->ae > 0) {
					sub_ae_splash(h, random_interval(10, 20));
					sprintf(g_text_output_buf, txt[78], h->name, grammar_pronoun(h->sex, 1, 1));
				} else {
					sprintf(g_text_output_buf, txt[79], h->name);
				}
				break;
			case ITEM_HYLAILIC_FIRE:
				sub_hero_le(h, random_interval(10, 25));
				sprintf(g_text_output_buf, txt[65], h->name);
				break;
			case ITEM_MIASTHMATICUM:
				sub_hero_le(h, random_interval(5, 10));
				sprintf(g_text_output_buf, txt[80], h->name);
				break;
			}
			drop_item(user, slot, 1);
			if (rest != 0)
				give_item_at(user, rest, 1, slot);
			dlg_message(g_dialog, g_text_output_buf);
		}
	}
}

/* Musikinstrument: laeuft Musik 5/6, wechselt die Begleitung (Harfe 3, Laute 5, Floete 1) */
void ItemUse::play_instrument()
{
	register signed short music;

	music = g_music_id_req & 0x3fff;
	if (music == 5 || music == 6) {
		switch (id) {
		case ITEM_HARP:
			Sound_start(g_snd_music, 3);
			break;
		case ITEM_LUTE:
			Sound_start(g_snd_music, 5);
			break;
		case ITEM_FLUTE:
			Sound_start(g_snd_music, 1);
			break;
		}
	}
}

/* Buch lesen: Zauber Heptagon + 2, Buch verbrauchen (vermutet) */
void ItemUse::read_heptagon_book()
{
	dlg_message(g_dialog, txt[22]);
	user->spells[SP_HEPTAGON] += (signed char)2;
	drop_item(user, slot, 1);
}

/* Licht: Laterne mit Oel fuellen (Bronzeflasche bleibt), mit Zunderkaestchen Fackel bzw. Laterne anzuenden */
void ItemUse::use_light()
{
	signed short ans;
	signed short tinder;
	signed short oil;
	register signed short lantern;
	register signed short torch;

	torch = get_item_pos(user, ITEM_TORCH_OFF);
	lantern = get_item_pos(user, ITEM_LANTERN_ON);
	tinder = get_item_pos(user, ITEM_TINDERBOX);
	oil = get_item_pos(user, ITEM_OIL);

	if (id == ITEM_LANTERN_ON) {
		if (oil != -1) {
			user->set_inv_light_timer(lantern, 100);
			drop_item(user, oil, 1);
			give_item_at(user, ITEM_FLASK_BRONZE, 1, oil);
			sprintf(g_text_output_buf, txt[23], user->name);
		} else {
			sprintf(g_text_output_buf, txt[24], user->name);
		}
	} else if (id == ITEM_OIL) {
		if (tinder == -1) {
			sprintf(g_text_output_buf, txt[25], user->name);
		} else if (user->inv[tinder].quantity <= 0) {
#if FIX_0095
			/* Fix #95: Meldung nur einmal (am Ende), ohne Merker keinen alten Text zeigen */
			if (verbose != 0)
				sprintf(g_text_output_buf, txt[2], grammar_article(0x8000, user->inv[tinder].item_id, 0));
			else
				*g_text_output_buf = 0;
#else
			if (verbose != 0) {
				sprintf(g_text_output_buf, txt[2], grammar_article(0x8000, user->inv[tinder].item_id, 0));
				dlg_message(g_dialog, g_text_output_buf);
			}
#endif
		} else if (lantern != -1) {
			user->set_inv_item_id(lantern, ITEM_LANTERN_ON);
			user->set_inv_light_timer(lantern, 100);
			drop_item(user, slot, 1);
			give_item_at(user, ITEM_FLASK_BRONZE, 1, slot);
			sprintf(g_text_output_buf, txt[23], user->name);
		} else {
			sprintf(g_text_output_buf, txt[29], user->name);
		}
	} else {
		if (tinder == -1) {
			sprintf(g_text_output_buf, txt[25], user->name);
		} else if (user->inv[tinder].quantity <= 0) {
#if FIX_0095
			/* Fix #95: Meldung nur einmal (am Ende), ohne Merker keinen alten Text zeigen */
			if (verbose != 0)
				sprintf(g_text_output_buf, txt[2], grammar_article(0x8000, user->inv[tinder].item_id, 0));
			else
				*g_text_output_buf = 0;
#else
			if (verbose != 0) {
				sprintf(g_text_output_buf, txt[2], grammar_article(0x8000, user->inv[tinder].item_id, 0));
				dlg_message(g_dialog, g_text_output_buf);
			}
#endif
		} else {
			if (id == ITEM_TORCH_OFF) {
				torch = slot;
			} else if (id == ITEM_LANTERN_OFF) {
				lantern = slot;
			} else if (id2 == ITEM_TORCH_OFF) {
				torch = slot2;
				lantern = -1;
			} else if (id2 == ITEM_LANTERN_OFF) {
				torch = -1;
				lantern = slot2;
			}
			if (torch != -1 && lantern != -1) {
				sprintf(g_text_output_buf, txt[26], user->name);
				ans = dlg_choose(g_dialog, g_text_output_buf, 2, grammar_article(0x4000, ITEM_TORCH_OFF, 0), grammar_article(0x4000, ITEM_LANTERN_OFF, 0));
				if (ans == 1)
					lantern = -1;
				else if (ans == 2)
					torch = -1;
				else
					torch = lantern = -1;
			}
			if (torch != -1) {
				user->set_inv_item_id(torch, ITEM_TORCH_ON);
				user->set_inv_light_timer(torch, 10);
				sprintf(g_text_output_buf, txt[27], user->name);
			} else if (lantern != -1) {
				if (oil != -1) {
					user->set_inv_item_id(lantern, ITEM_LANTERN_ON);
					user->set_inv_light_timer(lantern, 100);
					drop_item(user, oil, 1);
					give_item_at(user, ITEM_FLASK_BRONZE, 1, oil);
					sprintf(g_text_output_buf, txt[23], user->name);
				} else {
					sprintf(g_text_output_buf, txt[24], user->name);
				}
			} else {
				sprintf(g_text_output_buf, txt[29], user->name);
			}
		}
	}
	dlg_message(g_dialog, g_text_output_buf);
}

/* im Kampf: noch nicht erschienene Gegner nennen, eine Ladung verbrauchen; sonst txt[21] */
void ItemUse::show_hidden_enemies()
{
	if (g_in_fight != 0) {
		fig_hidden_enemies_text((Fight far *)g_fight_obj);
		user->dec_inv_quantity(slot);
		dlg_message(g_dialog, g_text_output_buf);
	} else {
		dlg_message(g_dialog, txt[21]);
	}
}

/* Rezeptbuch 0xbf allein: lesen (read_recipe_book); sonst mit Schreibzeug 0x24 ein Rezept ins Rezeptbuch (bzw. eine
 * Schriftrolle 0x1d, die dabei zum Rezeptbuch wird) abschreiben, das Rezept auf Wunsch wegwerfen */
void ItemUse::write_recipe()
{
	if (id == ITEM_RECIPE_BOOK && id2 == -1) {
		read_recipe_book();
	} else {
		signed short book;
		signed short pen;
		signed short i;
		signed short sel;
		signed char idx[14];
		char names[14][20];

		book = get_item_pos(user, ITEM_RECIPE_BOOK);
		pen = get_item_pos(user, ITEM_WRITING_SET);
		if (pen == -1) {
			sprintf(g_text_output_buf, txt[61], user->name);
			dlg_message(g_dialog, g_text_output_buf);
		} else if (user->inv[pen].quantity <= 0) {
			if (verbose != 0) {
				sprintf(g_text_output_buf, txt[2], grammar_article(0x8000, user->inv[pen].item_id, 0));
				dlg_message(g_dialog, g_text_output_buf);
			}
		} else {
			if (book == -1)
				book = get_item_pos(user, ITEM_SCROLL);
			if (book == -1) {
				sprintf(g_text_output_buf, txt[55], user->name);
				dlg_message(g_dialog, g_text_output_buf);
			} else {
				sel = -2;
				if (g_items[id].is_recipe() && !in_recipe_book(id)) {
					for (i = 0; i <= 13; i++) {
						if (g_recipe_items[i] == id) {
							sel = 0;
							idx[0] = i;
						}
					}
				} else if (g_items[id2].is_recipe() && !in_recipe_book(id2)) {
					for (i = 0; i <= 13; i++) {
						if (g_recipe_items[i] == id2) {
							sel = 0;
							idx[0] = i;
						}
					}
				} else {
					signed short n;
					signed short found;

					n = 0;
					found = 0;
					for (i = 0; i <= 13; i++) {
						if (get_item_pos(user, g_recipes[i].item_id) != -1) {
							found++;
							if (!box_has(user, book, i)) {
								strcpy(names[n], grammar_name(g_item_names[g_recipe_outcomes[i]]));
								idx[n] = i;
								n++;
							}
						}
					}
					if (n != 0) {
						if (n > 1)
							sel = dlg_choose(g_dialog, txt[56], n, names[0], names[1], names[2],
								names[3], names[4], names[5], names[6], names[7], names[8],
								names[9], names[10], names[11], names[12]) - 1;
						else
							sel = 0;
					} else if (found != 0) {
						sprintf(g_text_output_buf, txt[105], user->name);
						dlg_message(g_dialog, g_text_output_buf);
						sel = -2;
					} else {
						sprintf(g_text_output_buf, txt[59], user->name);
						dlg_message(g_dialog, g_text_output_buf);
						sel = -2;
					}
				}
				if (sel != -2) {
					if (book != -1) {
						user->set_inv_item_id(book, ITEM_RECIPE_BOOK);
						box_set(user, book, idx[sel], 1);
					}
					sprintf(g_text_output_buf, txt[58], grammar_name(g_item_names[g_recipe_outcomes[idx[sel]]]));
					if (dlg_yes_no(g_dialog, g_text_output_buf) == 1)
						drop_item(user, get_item_pos(user, g_recipes[idx[sel]].item_id), 1);
					user->dec_inv_quantity(get_item_pos(user, ITEM_WRITING_SET));
				}
			}
		}
	}
}

/* Rezeptbuch lesen: Rezept auswaehlen und seinen Text zeigen (read_text), bis Abbruch */
void ItemUse::read_recipe_book()
{
	signed short i;
	signed short sel = 0;
	signed short n = 0;
	char names[14][20];
	signed char idx[14];

	switch (id) {
	case ITEM_RECIPE_BOOK:
		for (i = 0; i <= 13; i++) {
			if (box_has(user, slot, i)) {
				strcpy(names[n], grammar_name(g_item_names[g_recipe_outcomes[i]]));
				idx[n] = i;
				n++;
			}
		}
		if (n != 0) {
			while (sel != -2) {
				if (n > 1)
					sel = dlg_choose(g_dialog, txt[57], n, names[0], names[1], names[2],
						names[3], names[4], names[5], names[6], names[7], names[8],
						names[9], names[10], names[11], names[12]) - 1;
				else
					sel = 0;
				if (sel != -2) {
					id = g_recipes[idx[sel]].item_id;
					read_text();
					if (n == 1)
						sel = -2;
				}
			}
			return;
		} else {
			dlg_message(g_dialog, txt[73]);
		}
	default:
		break;
	}
}

/* Meldung txt[71] in Farbe 7 */
void ItemUse::show_note()
{
	signed short old;

	old = dlg_set16(g_dialog, 7);
	dlg_message(g_dialog, txt[71]);
	dlg_set16(g_dialog, old);
}
