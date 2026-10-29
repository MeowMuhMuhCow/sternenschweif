/* OVL091 - Overlay O091 (Segment 0x2119 in SCHWEIF.EXE): Magie (entspricht DSA1 seg098, Bright Eyes).
 * use_magic(h): Menue Meditation / Zaubern / Stabzauber (Stabzauber nur Magier): Meditation mit Thonnys
 * (get_item_pos 0x83, ausser Magier), AE-Eingabe dlg_input (hoechstens AE-Grundwert - AE), Probe
 * test_attrib3(h, MU, CH, KK, 0), LE-Kosten n + 3; Stabzauber aus g_staffspell_descriptions (6 Bytes),
 * Zeitsprung timewarp(g_date, ..); Zaubern -> use_spell. can_use_spellclass(h, cls): Zauberklasse benutzbar
 * (g_spells_index, Zauberbeschreibungen g_spell_descriptions zu 12 Bytes, Kampf g_in_fight). select_spell(h, show):
 * Klassen- und Zaubermenue (Formate aus code_to_text und g_select_spell_tmpl, Liste g_menu_texts).
 * test_spell(h, id, mod): Zauberprobe (Ziel-MR von Gegner enemy_mr_target bzw. Held hero_mr_target).
 * use_spell(h, menu, mod): Zaubertexte laden (txt_load_list Datei 0x1f nach h->f5e7/h->txt), Bild g_render_bmp/
 * g_renderbuf_ptr voruebergehend freigeben, Auswahl, Ziel (hero_set_target/get_any_hero), Spruch ueber lokales
 * Cast-Objekt (O233: Konstruktor, cast, Destruktor).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y -E..\bc\BIN\TASM.EXE OVL091.CPP -> 4753/4753 Bytes GLEICH (18.10.2026),
 *   cobjfix laeuft. Massentest: nur -Og -Oi (+ wirkungsloses -Oc) ergibt den Bestwert.
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * - Zauberwerte "SPELL_RAW(h, k)" = ((signed char far *)h)[k + 0x14f]: der Index wird zuerst gerechnet (-Og-
 *   Zwischenwert first + i, in can_use_spellclass SI), dann "mov es,[h+2] / add bx,[h]". h->spells[first + i]
 *   rechnet les bx,h zuerst (und in can_use_spellclass wandert first nach SI statt des Zwischenwerts);
 *   (unsigned)-Index nimmt ax.
 * - C++ wegen inline-Elementfunktionen: h->set_staffspell_lvl(h->staffspell_lvl + 1) (Parameterkopie [bp-7]),
 *   enemy_mr_target(h)->mr_eff()/is_mushroom(), hero_mr_target(h)->mr_eff()/is_dead() -> je eine this-Kopie als
 *   Local; Bitfeld-Getter mit Rueckgabe unsigned char -> and ax,1 / or al,al. "!(signed char)h->renegade" ->
 *   and ax,1 / cbw / or ax,ax.
 * - use_spell: Schleifenzaehler im inneren Block (sonst Stackplatz wegen memset-Intrinsic/DI), mod als
 *   signed short (push word [bp+0ch]); "delete g_render_bmp" -> gb_delete(p, 3); "new Bmp(320, 135, p)" ->
 *   gb_new(0, ..); lokales Cast-Objekt im Block -> Destruktor (&c, 2) am Blockende.
 * - select_spell: "g_text_output_buf + 50 * (i + 1)" (imul, add off, add 32h); Formatpuffer fmt_on/fmt_off
 *   (10 Bytes) bleiben in Deklarationsreihenfolge, usable (12 Bytes) ans Ende; done im else-Block ([bp-28h] hinter
 *   den Zwischenwerten).
 * Originalfehler/Auffaelligkeiten:
 * - Stabzauber: Probe mit Eigenschaft 2 doppelt (a2 statt a3), wie DSA1 (Bright Eyes Bug 17); die DSA1-Abfrage
 *   "== 7" ist hier zu "== 4" korrigiert.
 * - test_spell: "if (g_spell_by_item != 0) retval = 99;" steht im else-Zweig von "if (g_spell_by_item != 0)" -> tot.
 * - use_spell: h->f5e7/h->txt werden am Ende freigegeben, aber nur im Erfolgszweig vorher genullt; bei Abbruch/
 *   Fehlschlag bleiben im Helden haengende Zeiger (sofern nicht anderswo neu gesetzt).
 * - use_magic Meditation: bei LE < 8 wird n negativ (LE-Kosten n + 3, AE-Gewinn negativ), wie DSA1.
 */
#include <stdio.h>
#include <string.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:5ca0..5cb2, dsana) - initialisierte Daten in Adressreihenfolge */
struct select_spell_tmpl g_select_spell_tmpl = { { "%s~%d" }, { { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 } } };

/* Zauberwert per Byte-Index ab Heldenanfang (Index wird zuerst gerechnet, dann h addiert; h->spells[k] rechnet
 * les bx,h zuerst) */
#define SPELL_RAW(h, k) (((signed char far *)(h))[(k) + 0x14f])

signed short use_magic(struct hero far *h)
{
	signed short answer;
	signed short retval;
	signed short pos;
	signed short n;

	retval = 0;
	n = 2;
	if (h->type == HERO_TYPE_MAGE)
		n = 3;
	answer = dlg_choose(g_dialog, g_ttx[0x147], n, g_ttx[0x148], g_ttx[0x14a], g_ttx[0x149]);
	if (answer != -1) {
		switch (answer) {
		case 1:
			if (h->ae_max == h->ae) {
				sprintf(g_text_output_buf, g_ttx[0x1b3], h->name);
				dlg_message(g_dialog, g_text_output_buf);
				return 0;
			}
			if (h->type != HERO_TYPE_MAGE) {
				if ((pos = get_item_pos(h, 0x83)) == -1) {
					dlg_message(g_dialog, g_ttx[0x14b]);
					return 0;
				}
			} else
				pos = -1;
			n = dlg_input(g_dialog, 2, g_ttx[0x14c], 0, h->ae_max - h->ae, 0);
			if (n != -1) {
				retval = 2;
				if (pos != -1)
					drop_item(h, pos, 1);
				sub_ae_splash(h, 1);
				if (test_attrib3(h, ATTRIB_MU, ATTRIB_CH, ATTRIB_KK, 0) > 0) {
#if FIX_0220
					/* Fix #220: bei zu wenig LE keine Umwandlung (kein negativer AE-Gewinn) */
					if (h->le <= n + 8)
						n = h->le - 8;
					if (n > 0) {
						sub_hero_le(h, n + 3);
						add_hero_ae(h, n);
					}
#else
					if (h->le <= n + 8)
						n = h->le - 8;
					sub_hero_le(h, n + 3);
					add_hero_ae(h, n);
#endif
				} else {
					sprintf(g_text_output_buf, g_ttx[0x14d], h->name);
					dlg_message(g_dialog, g_text_output_buf);
				}
			}
			break;
		case 2:
			use_spell(h, 1, 0);
			break;
		case 3:
			if (h->staffspell_lvl == 4) {
				dlg_message(g_dialog, g_ttx[0x14f]);
			} else if (g_staffspell_descriptions[h->staffspell_lvl].ae_cost <= h->ae) {
				retval = 1;
#if FIX_0110
				/* Fix #110: Stabzauber-Probe mit der dritten Eigenschaft a3 */
				if (test_attrib3(h, g_staffspell_descriptions[h->staffspell_lvl].a1, g_staffspell_descriptions[h->staffspell_lvl].a2, g_staffspell_descriptions[h->staffspell_lvl].a3,
#else
				if (test_attrib3(h, g_staffspell_descriptions[h->staffspell_lvl].a1, g_staffspell_descriptions[h->staffspell_lvl].a2, g_staffspell_descriptions[h->staffspell_lvl].a2,
#endif
						g_staffspell_descriptions[h->staffspell_lvl].mod) > 0) {
					sprintf(g_text_output_buf, g_ttx[0x150], h->staffspell_lvl + 1);
					dlg_message(g_dialog, g_text_output_buf);
					sub_ae_splash(h, g_staffspell_descriptions[h->staffspell_lvl].ae_cost);
					add_hero_ae2(h, -g_staffspell_descriptions[h->staffspell_lvl].ae_mod);
					h->set_staffspell_lvl(h->staffspell_lvl + 1);
					h->staffspell_timer = 0xfd20L;
					timewarp(g_date, 0x6978L);
				} else {
					dlg_message(g_dialog, g_ttx[0x151]);
					sub_ae_splash(h, g_staffspell_descriptions[h->staffspell_lvl].ae_cost / 2);
					timewarp(g_date, 0x2a30L);
				}
			} else {
				sprintf(g_text_output_buf, g_ttx[0x154], h->name);
				dlg_message(g_dialog, g_text_output_buf);
			}
			break;
		}
	}
	return retval;
}

signed short can_use_spellclass(struct hero far *h, signed short cls)
{
	signed short first;
	signed short i;

	first = g_spells_index[cls].first;
	for (i = 0; g_spells_index[cls].count > i; i++) {
		if (SPELL_RAW(h, first + i) >= -5 &&
			((g_in_fight != 0 && g_spell_descriptions[first + i].where != -1) ||
			(g_in_fight == 0 && g_spell_descriptions[first + i].where != 1)))
			return 1;
	}
	return 0;
}

signed short select_spell(struct hero far *h, signed short show)
{
	signed short answer;
	signed short first;
	signed short retval = -2;
	signed short i;
	struct str6 fmt = g_select_spell_tmpl.fmt;
	char fmt_on[10];			/* Name und Wert, Klasse benutzbar */
	char fmt_off[10];			/* Name und Wert, gesperrt (Steuercode 0xe7) */
	struct bytes12 usable = g_select_spell_tmpl.usable;

	sprintf(fmt_on, code_to_text(0xff), fmt.s);
	sprintf(fmt_off, code_to_text(0xe7), fmt.s);
	if (show == 0 && g_game_mode == GAME_MODE_ADVANCED)
		show = 2;
	if (h->type < HERO_TYPE_WITCH) {
		dlg_message(g_dialog, g_ttx[0x152]);
	} else {
		signed short done;

		do {
			done = 1;
			retval = -1;
			if (show == 1) {
				strcpy(g_text_buf2, g_ttx[0x153]);
				if (h->spell_incs > 1)
					strcat(g_text_buf2, g_ttx[0x155]);
				sprintf(g_text_output_buf, g_ttx[0x156], h->spell_incs > 1 ? g_ttx[0x106] : g_ttx[0x105],
					h->spell_incs, g_text_buf2);
				answer = dlg_choose(g_dialog, g_text_output_buf, 12,
					g_ttx[0xdb], g_ttx[0xdc], g_ttx[0xdd], g_ttx[0xde],
					g_ttx[0xdf], g_ttx[0xe0], g_ttx[0xe1], g_ttx[0xe2],
					g_ttx[0xe3], g_ttx[0xe4], g_ttx[0xe5], g_ttx[0xe6]) - 1;
			} else {
				strcpy(g_text_output_buf, g_ttx[0x107]);
				for (i = 0; i < 12; i++) {
					g_menu_texts[i] = g_text_output_buf + 50 * (i + 1);
					usable.b[i] = can_use_spellclass(h, i);
					sprintf(g_menu_texts[i], usable.b[i] ? code_to_text(0xff) : code_to_text(0xe7), g_ttx[i + 0xdb]);
				}
				answer = dlg_choose(g_dialog, g_text_output_buf, 12,
					g_menu_texts[0], g_menu_texts[1], g_menu_texts[2], g_menu_texts[3],
					g_menu_texts[4], g_menu_texts[5], g_menu_texts[6], g_menu_texts[7],
					g_menu_texts[8], g_menu_texts[9], g_menu_texts[10], g_menu_texts[11]) - 1;
			}
			if (answer != -2) {
				if (!usable.b[answer]) {
					sprintf(g_text_output_buf, g_ttx[0x157], h->name);
					dlg_message(g_dialog, g_text_output_buf);
					retval = -2;
				} else {
					first = g_spells_index[answer].first;
					for (i = 0; g_spells_index[answer].count > i; i++) {
						g_menu_texts[i] = g_text_output_buf + 50 * i;
						if (show == 1) {
							sprintf(g_menu_texts[i], fmt_on, g_ttx[first + i + 0x85], SPELL_RAW(h, first + i));
						} else if (((g_in_fight != 0 && g_spell_descriptions[first + i].where != -1) ||
							(g_in_fight == 0 && g_spell_descriptions[first + i].where != 1)) &&
							SPELL_RAW(h, first + i) >= -5) {
							if (show == 2)
								sprintf(g_menu_texts[i], fmt_on, g_ttx[first + i + 0x85], SPELL_RAW(h, first + i));
							else
								sprintf(g_menu_texts[i], code_to_text(0xff), g_ttx[first + i + 0x85]);
						} else if (show == 2) {
							sprintf(g_menu_texts[i], fmt_off, g_ttx[first + i + 0x85], SPELL_RAW(h, first + i));
						} else {
							sprintf(g_menu_texts[i], code_to_text(0xe7), g_ttx[first + i + 0x85]);
						}
					}
					retval = dlg_choose(g_dialog, g_ttx[0x15a], g_spells_index[answer].count,
						g_menu_texts[0], g_menu_texts[1], g_menu_texts[2], g_menu_texts[3],
						g_menu_texts[4], g_menu_texts[5], g_menu_texts[6], g_menu_texts[7],
						g_menu_texts[8], g_menu_texts[9], g_menu_texts[10], g_menu_texts[11],
						g_menu_texts[12], g_menu_texts[13], g_menu_texts[14], g_menu_texts[15]);
					if (retval != -1) {
						if (SPELL_RAW(h, retval + first - 1) < -5 && show == 0) {
							sprintf(g_text_output_buf, g_ttx[0x15b], h->name);
							dlg_message(g_dialog, g_text_output_buf);
							retval = -1;
						} else
							retval += first - 1;
					} else
						done = 0;
				}
				if (retval > 0) {
					if (g_in_fight == 0 && g_spell_descriptions[retval].where == 1 && show == 0) {
						dlg_message(g_dialog, g_ttx[0x158]);
						retval = -2;
					} else if (g_in_fight != 0 && g_spell_descriptions[retval].where == -1) {
						dlg_message(g_dialog, g_ttx[0x159]);
						retval = -2;
					}
				}
			} else
				retval = -2;
		} while (!done);
	}
	return retval;
}

signed short test_spell(struct hero far *h, signed short spell, signed char mod)
{
	struct spell_desc far *p;
	signed short retval;

	retval = 1;
	if (g_spell_by_item != 0)
		retval = 1;
	else if (g_ingame_timers[0] != 0)
		retval = -1;
	else {
		p = &g_spell_descriptions[spell];
		if (p->fight != 0) {
			if (target_type(h) == 1) {
				mod += enemy_mr_target(h)->mr_eff();
				if (enemy_mr_target(h)->is_mushroom())
					retval = 0;
			} else
				mod += hero_mr_target(h)->mr_eff();
		}
		if (retval != 0 && spell >= 1 && spell <= 0x55) {
			if (g_spell_by_item != 0)
				retval = 99;
			else {
				mod -= h->spells[spell];
				retval = test_attrib3(h, p->a1, p->a2, p->a3, mod);
			}
		}
	}
	return retval;
}

signed short use_spell(struct hero far *h, signed short menu, signed short mod)
{
	signed char freed;
	signed short spell;
	signed short retval;
	signed short old;
	struct spell_desc far *p;
	signed short show;

	freed = 0;
	retval = 1;
	show = 1;
	if (g_renderbuf_ptr != 0) {
		delete g_render_bmp;
		g_render_bmp = 0;
		g_renderbuf_ptr = mem_free(g_renderbuf_ptr);
		freed = 1;
	}
	if (!g_quick_fight) {
		txt_load_list(g_font, 0x1f, 300, &h->f5e7, &h->txt);
	} else {
		signed short i;

		h->txt = (char far * far *)mem_alloc(0x4b4L);
		h->f5e7 = mem_alloc(10L);
		for (i = 0; i <= 300; i++)
			h->txt[i] = g_input_buf;
	}
	memset(g_cast_spell_id, 0, 16);
	old = dlg_set16(g_dialog, 3);
	*g_text_output_buf = 0;
	if ((h->type < HERO_TYPE_WITCH || (!check_hero(h) && !(signed char)h->renegade)) && g_spell_by_item == 0) {
		g_spell_cancelled = 1;
		memset(&g_spell_by_item, 0, 7);
		sprintf(g_text_output_buf, h->txt[0xb6], h->name);
		retval = -1;
	} else {
		if (menu == 1) {
			spell = select_spell(h, 0);
			if (spell > 0) {
				p = &g_spell_descriptions[spell];
				hero_set_target(h, h);
				if (p->target_type != 0 && p->target_type != 4) {
					if (p->target_type == 1) {
						dlg_message(g_dialog, g_ttx[0x15c]);
						spell = -1;
					} else {
						hero_set_target(h, get_any_hero(g_group, g_ttx[0x15d]));
						if (h->enemy_id == -1)
							spell = -1;
						else if (hero_mr_target(h)->is_dead() && spell != 0x3b && spell != 0x21) {
							dlg_message(g_dialog, h->txt[0x50]);
							spell = -1;
						}
					}
				}
			} else
				*g_text_output_buf = 0;
		} else
			spell = h->spell_id;
		if (spell > 0) {
			p = &g_spell_descriptions[spell];
			if (g_in_fight == 0 && p->where == 1) {
				strcpy(g_text_output_buf, g_ttx[0x158]);
				g_spell_cancelled = 1;
				retval = -1;
			} else if (g_in_fight != 0 && p->where == -1) {
				strcpy(g_text_output_buf, g_ttx[0x159]);
				g_spell_cancelled = 1;
				retval = -1;
			}
			if (retval != -1) {
				h->f5e7 = mem_free(h->f5e7);
				h->txt = (char far * far *)mem_free(h->txt);
				Cast c(h);
				retval = c.cast(spell, mod);
				show = 0;
			}
		} else {
			g_spell_cancelled = 1;
			retval = -1;
			*g_text_output_buf = 0;
		}
	}
	if (g_in_fight == 0 && show != 0 && *g_text_output_buf != 0)
		dlg_message(g_dialog, g_text_output_buf);
	dlg_set16(g_dialog, old);
	memset(&g_spell_by_item, 0, 7);
#if FIX_0110
	/* Fix #110: Textzeiger auch nach Abbruch/Fehlschlag nullen */
	if (h->f5e7 != 0) {
		h->f5e7 = mem_free(h->f5e7);
		h->txt = (char far * far *)mem_free(h->txt);
	}
#else
	if (h->f5e7 != 0) {
		mem_free(h->f5e7);
		mem_free(h->txt);
	}
#endif
	if (freed) {
		g_renderbuf_ptr = mem_alloc(0xaa00L);
		g_render_bmp = new Bmp(320, 135, g_renderbuf_ptr);
	}
	return retval;
}
