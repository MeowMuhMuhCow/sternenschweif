/* OVL093 - Overlay O093 (Segment 0x2121 in SCHWEIF.EXE): Heldenbogen - Seitenmenues (struct inv_sheet s wie
 * OVL095: Texte txt, Seite page, Ende done, Menuewahl sel, Held hero/owner, Bildschirm-Bmp screen, Puffer buf).
 * inv_switch_hero(s): voriger Held (get_prev_mage ab Seite 4, sonst get_prev_hero). inv_draw_spell(s, i, base, x,
 * xr, y): Zaubername g_ttx[0x85 + i] und Wert hero->spells[i] rechtsbuendig. select_portrait(h, msg): Portraet neu
 * waehlen (Bereich je Typ/Geschlecht aus g_portrait_index_m/_f, wie O026_0936) -> hero.portrait.
 * inv_menu_page1(s): Menue Seite 1-3 (bis 16 Punkte, dynamisch g_menu_texts: Gift/Krankheit/Abenteuerpunkte;
 * Krankheit/Gift-Text mit Zusatz bei Bit diagnosed), Aktionen 0..10 (Umbenennen rename_hero, Portraet,
 * Bogen drucken hero_print_sheet, AP bis zur naechsten Stufe ap_for_level, Gegenstand benutzen/ablegen use_item/
 * drop_item/use_skill, Zauber use_spell mit Neuladen der Texte, Seitenwechsel). inv_menu_page2/inv_menu_page3/
 * inv_menu_page45: Menues der weiteren Seiten.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL093.CPP   -> 5368/5368 Bytes GLEICH (18.10.2026), cobjfix laeuft.
 *   -Oi fuer strcat/memcpy (Intrinsics); ohne -Og 51, ohne -Oi 77 Bytes zu kurz.
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * - inv_switch_hero: s->hero ist -Og-Zwischenwert [bp-8] (mov eax vor dem les), keine eigene Variable (sonst les
 *   zuerst).
 * - Gleiche Bloecke (Gegenstand benutzen, Zauber) als Makros USE_ITEM/MAGIC; r im Block als register (DI, da
 *   SI fuer den -Og-Index "s->hero->inv[s->slot]" ueber les si gebraucht wird).
 * - select_portrait: memcpy-Intrinsic ins Local -> push ss / pop es / lea di / mov si (Strukturzuweisung waere
 *   mov si / lea di / push ss / pop es, vgl. OVL026 f_06d6).
 * - Mehrfache Ausdruecke "s->sel == s->nr_entries - 1" laden s zweimal (les bx / push es / les si / pop es).
 * - g_portrait_index_m_next (ds:16b2) hat ein eigenes Public - nicht als g_portrait_index_m[type + 1] schreiben
 *   (gleiche Bytes, aber anderes Fixup-Ziel).
 * Auffaelliges: select_portrait kopiert die Tabelle g_portrait_rects in ein unbenutztes Local (wie O026_0936);
 *   inv_menu_page45 (Seite 4/5) prueft vor dem Zauber nicht type >= HERO_TYPE_WITCH (die anderen Seiten schon),
 *   inv_menu_page3 wechselt auf Seite 4 ohne inv_reselect.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "schweif.h"

void inv_switch_hero(struct inv_sheet far *s)
{
	struct hero far *h;

	if (s->page > 3)
		h = get_prev_mage(g_group, s->hero);
	else
		h = get_prev_hero(g_group, s->hero);
	if (s->hero != h) {
		if (s->slot2 != -1) {
			if (s->swap == 0)
				s->owner = s->hero;
			s->hero = g_sheet_hero = h;
			s->swap = 1;
		} else {
			s->swap = 0;
			s->owner = s->hero = g_sheet_hero = h;
			inv_reselect(s);
		}
		g_refresh = 1;
	}
}

void inv_draw_spell(struct inv_sheet far *s, register signed short i, signed short base, signed short x, signed short xr,
	register signed short y)
{
	signed short k;
	char buf[10];

	k = i - base;
	font_print(g_font, g_ttx[i + 0x85], x, y + k * 7);
	itoa(s->hero->spells[i], buf, 10);
	font_print(g_font, buf, xr - font_text_width0(g_font, buf), y + k * 7);
}

/* Portraet neu waehlen: Bereich je Heldentyp und Geschlecht aus g_portrait_index_m/_f, Auswahl dlg_select_pic */
signed short select_portrait(struct hero far *h, char far *msg)
{
	signed short n;
	signed short sel;
	signed short first;
	signed short last;
	signed short type;
	signed short ret = 0;
	char tab[0x48];

	if (h->npc_id != 0) {
		dlg_message(g_dialog, msg);
	} else {
		memcpy(tab, g_portrait_rects, 0x48);
		type = h->type;
		if (type > 10)
			type = 10;
		if (h->sex != 0) {
			first = g_portrait_index_f[type];
			last = g_portrait_index_m[type + 1] - 1;
		} else {
			first = g_portrait_index_m[type];
			last = g_portrait_index_f[type] - 1;
		}
		n = last - first + 1;
		sel = dlg_select_pic(g_dialog, n, first);
		if (sel != -1) {
			h->set_portrait(sel);
			ret = 1;
		} else
			ret = 0;
	}
	return ret;
}

#define USE_ITEM(s) \
	{ \
		register signed short r; \
		r = use_skill((s)->hero, 0, (s)->slot2 != -1 ? (s)->hero->inv[(s)->slot].item_id : -1); \
		if (r == -2) { \
			(s)->done = 1; \
			inv_reselect(s); \
		} else if (r == -1) { \
			sprintf(g_text_output_buf, (s)->txt[80], (s)->hero->name); \
			dlg_message(g_dialog, g_text_output_buf); \
		} \
	}

#define MAGIC(s) \
	mem_free((s)->txt_buf); \
	mem_free((s)->txt); \
	mem_free((s)->buf); \
	if (use_spell((s)->hero, 1, 0) != -2) \
		inv_reselect(s); \
	(s)->buf = mem_alloc(0xfa00L); \
	txt_load_list(g_font, 0x1c, 0x5f, &(s)->txt_buf, &(s)->txt); \
	(s)->screen->set_base((s)->buf); \
	g_refresh = 1;

void inv_menu_page1(struct inv_sheet far *s)
{
	signed short n = 0;
	signed short m4 = 99;
	signed short m6 = 99;
	signed short m8 = 99;

	if (g_game_mode == GAME_MODE_ADVANCED) {
		s->nr_entries = 13;
		g_menu_texts[n] = s->txt[1];
		n++;
		g_menu_texts[n] = s->txt[2];
		n++;
		g_menu_texts[n] = s->txt[3];
		n++;
	} else
		s->nr_entries = 10;
	s->ill = hero_is_diseased(s->hero);
	s->poi = hero_is_poisoned(s->hero);
	if (s->ill != 0) {
		m8 = n + 8;
		g_menu_texts[n] = s->txt[4];
		n++;
		s->nr_entries++;
	}
	if (s->poi != 0) {
		m6 = n + 8;
		g_menu_texts[n] = s->txt[57];
		n++;
		s->nr_entries++;
	}
	m4 = n + 8;
	g_menu_texts[n] = s->txt[82];
	n++;
	g_menu_texts[n] = s->txt[5];
	n++;
	s->sel = dlg_choose(g_dialog, (char far *)0, s->nr_entries, g_inv_show_body ? s->txt[47] : s->txt[46], s->txt[6],
		s->txt[88], s->txt[91], s->txt[7], s->txt[8], s->txt[9], s->txt[10], g_menu_texts[0], g_menu_texts[1],
		g_menu_texts[2], g_menu_texts[3], g_menu_texts[4], g_menu_texts[5], g_menu_texts[6], g_menu_texts[7]) - 1;
	if (s->sel != -2) {
		if (s->sel == s->nr_entries - 1) {
			s->done = 1;
			inv_reselect(s);
			return;
		}
		if (s->sel == m4) {
			if (dlg_yes_no(g_dialog, s->txt[83]) == 1)
				hero_print_sheet(s->hero);
			return;
		}
		if (s->sel == m6) {
			sprintf(g_text_output_buf, s->txt[s->poi + 47], s->hero->name);
			if ((signed char)s->hero->poison[s->poi].diagnosed) {
				sprintf(g_text_buf2, s->txt[84], grammar_pronoun(s->hero->sex, 0, 0), g_ttx[s->poi + 391]);
				strcat(g_text_output_buf, g_text_buf2);
			}
			dlg_message(g_dialog, g_text_output_buf);
			return;
		}
		if (s->sel == m8) {
			sprintf(g_text_output_buf, s->txt[s->ill + 10], s->hero->name);
			if ((signed char)s->hero->illness[s->ill].diagnosed) {
				sprintf(g_text_buf2, s->txt[58], grammar_pronoun(s->hero->sex, 0, 0), g_ttx[s->ill + 43]);
				strcat(g_text_output_buf, g_text_buf2);
			}
			dlg_message(g_dialog, g_text_output_buf);
			return;
		}
		switch (s->sel) {
		case 0:
			inv_toggle_body(s);
			g_refresh = 1;
			break;
		case 1:
			g_refresh = rename_hero(s);
			break;
		case 2:
			g_refresh = select_portrait(s->hero, s->txt[89]);
			break;
		case 3:
			sprintf(g_text_output_buf, s->txt[92], s->hero->name, ap_for_level(s->hero->level + 1) - s->hero->ap);
			dlg_message(g_dialog, g_text_output_buf);
			break;
		case 4:
			if (s->owner != s->hero) {
				dlg_message(g_dialog, s->txt[43]);
			} else {
				use_item(s->hero, s->slot);
				inv_reselect(s);
				g_refresh = 1;
			}
			break;
		case 5:
			if (s->owner != s->hero) {
				dlg_message(g_dialog, s->txt[43]);
			} else {
				drop_item(s->hero, s->slot, -1);
				inv_reselect(s);
				g_refresh = 1;
			}
			break;
		case 6:
			USE_ITEM(s)
			break;
		case 7:
			if (s->hero->type < HERO_TYPE_WITCH) {
				dlg_message(g_dialog, s->txt[44]);
			} else {
				MAGIC(s)
			}
			break;
		case 8:
			s->page = 2;
			g_refresh = 1;
			break;
		case 9:
			inv_reselect(s);
			s->page = 3;
			g_refresh = 1;
			break;
		case 10:
			if (s->hero->type < HERO_TYPE_WITCH) {
				dlg_message(g_dialog, s->txt[44]);
			} else {
				inv_reselect(s);
				s->page = 4;
				g_refresh = 1;
			}
			break;
		}
	}
}

void inv_menu_page2(struct inv_sheet far *s)
{
	s->sel = dlg_choose(g_dialog, (char far *)0, 9, g_inv_show_body ? s->txt[47] : s->txt[46], s->txt[7], s->txt[8],
		s->txt[9], s->txt[10], s->txt[25], s->txt[2], s->txt[3], s->txt[5]) - 1;
	if (s->sel != -2) {
		switch (s->sel) {
		case 0:
			inv_toggle_body(s);
			g_refresh = 1;
			break;
		case 1:
			if (s->owner != s->hero) {
				dlg_message(g_dialog, s->txt[43]);
			} else {
				use_item(s->hero, s->slot);
				inv_reselect(s);
				g_refresh = 1;
			}
			break;
		case 2:
			if (s->owner != s->hero) {
				dlg_message(g_dialog, s->txt[43]);
			} else {
				drop_item(s->hero, s->slot, -1);
				inv_reselect(s);
				g_refresh = 1;
			}
			break;
		case 3:
			USE_ITEM(s)
			break;
		case 4:
			if (s->hero->type < HERO_TYPE_WITCH) {
				dlg_message(g_dialog, s->txt[44]);
			} else {
				MAGIC(s)
			}
			break;
		case 5:
			s->page = 1;
			g_refresh = 1;
			break;
		case 6:
			inv_reselect(s);
			s->page = 3;
			g_refresh = 1;
			break;
		case 7:
			if (s->hero->type < HERO_TYPE_WITCH) {
				dlg_message(g_dialog, s->txt[44]);
			} else {
				inv_reselect(s);
				s->page = 4;
				g_refresh = 1;
			}
			break;
		case 8:
			s->done = 1;
			inv_reselect(s);
			break;
		}
	}
}

void inv_menu_page3(struct inv_sheet far *s)
{
	s->sel = dlg_choose(g_dialog, (char far *)0, 6, s->txt[9], s->txt[10], s->txt[25], s->txt[1], s->txt[3],
		s->txt[5]);
	if (s->sel != -1) {
		switch (s->sel) {
		case 1:
			USE_ITEM(s)
			break;
		case 2:
			if (s->hero->type < HERO_TYPE_WITCH) {
				dlg_message(g_dialog, s->txt[44]);
			} else {
				MAGIC(s)
			}
			break;
		case 3:
			s->page = 1;
			g_refresh = 1;
			break;
		case 4:
			s->page = 2;
			g_refresh = 1;
			break;
		case 5:
			if (s->hero->type < HERO_TYPE_WITCH) {
				dlg_message(g_dialog, s->txt[44]);
#if FIX_0148
			/* Fix #148: beim Wechsel auf Seite 4 neu auswaehlen (inv_reselect wie auf Seite 1) */
			} else {
				inv_reselect(s);
				s->page = 4;
				g_refresh = 1;
			}
#else
			} else {
				s->page = 4;
				g_refresh = 1;
			}
#endif
			break;
		case 6:
			s->done = 1;
			break;
		}
	}
}

void inv_menu_page45(struct inv_sheet far *s)
{
	s->sel = dlg_choose(g_dialog, (char far *)0, 7, s->txt[9], s->txt[10], s->txt[25], s->txt[1], s->txt[2],
		s->txt[45], s->txt[5]);
	if (s->sel != -1) {
		switch (s->sel) {
		case 1:
			USE_ITEM(s)
			break;
#if FIX_0148
		/* Fix #148: Zaubern nur fuer Zauberkundige (wie auf den anderen Seiten) */
		case 2:
			if (s->hero->type < HERO_TYPE_WITCH) {
				dlg_message(g_dialog, s->txt[44]);
			} else {
				MAGIC(s)
			}
			break;
		case 3:
			s->page = 1;
			g_refresh = 1;
			break;
		case 4:
			s->page = 2;
			g_refresh = 1;
			break;
		case 5:
			s->page = 3;
#else
		case 2:
			MAGIC(s)
			break;
		case 3:
			s->page = 1;
			g_refresh = 1;
			break;
		case 4:
			s->page = 2;
			g_refresh = 1;
			break;
		case 5:
			s->page = 3;
#endif
			g_refresh = 1;
			break;
		case 6:
			if (s->page == 4)
				s->page = 5;
			else
				s->page = 4;
			g_refresh = 1;
			break;
		case 7:
			s->done = 1;
			break;
		}
	}
}
