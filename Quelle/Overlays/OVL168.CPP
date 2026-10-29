/* OVL168 - Overlay O168 (Segment 0x22aa in SCHWEIF.EXE): Heldenerschaffung, Anzeige des Heldenbogens (class CharGen:
 * Held g->hero, Texte g->txt, Seite g->page, Verschiebemerker g->attr_moved[14], g->show_moved).
 * chargen_draw_attribs(g): 14 Eigenschaften an den Positionen g_chargen_attrib_pos (Stackkopie) - bei g->show_moved mit
 * Farbcode code_to_text(0xfd/0xfc/0xff) je nach attr_moved = +1/-1/sonst per sprintf, sonst nur itoa.
 * chargen_draw_page(g): Seite 0..10 (switch, Sprungtabelle): 0 Grundseite (chargen_restore_bg(g, g_font_dst), chargen_draw_name,
 * chargen_draw_attribs, Groesse/Gewicht txt[20]/txt[21], Sternzeichen g_ttx[god + 4], Geld money_to_string, LE/AE,
 * LE + KK, MR + mr_mod), 1..3 Talente get_skill_value zweispaltig (x = 0xde/0x135 - Breite, y = j/2*12 + Basis),
 * Talentsteigerungen; 4 AT/PA (Basis, AT/PA je Waffentalent, Talent), Fernkampf-Basis (KL + GE + KK)/4 plus
 * Talent 7/8; 5..10 Zauberwerte (x = 0x9d/0x12e - Breite), Zaubersteigerungen.
 * chargen_change_atpa(g): AT/PA-Verteilung der Waffentalente (Auswahl txt[28] aus g_ttx[75..81], Wert <= 0 ->
 * txt[54], Richtung txt[48] mit txt[25]/[26]; AT+1/PA-1 nur wenn PA > Basis, sonst txt[49]; umgekehrt txt[50]).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL168.CPP   -> 5571/5571 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (g_chargen_attrib_pos -> _g_6c8a+2). Ohne -Og 3915 Abweichungen.
 * - chargen_draw_attribs: Strukturkopie der Positionstabelle auf Funktionsebene -> keine register-Variablen (si/di gesichert,
 *   aber unbenutzt); -Og-Zwischenwert [bp-0eh] = Offset g + i (Segment per mov es,[bp+8] nachgeladen).
 *   Schleifenbedingung "i <= 13" (jg), "i < 14" ergibt jnl.
 * - chargen_draw_page: Talentseiten mit Locals xe/xo (0xde/0x135, Initialisierung in Deklarationsreihenfolge [bp-8] vor
 *   [bp-0ah]), Zauberseiten mit literalen Konstanten 0x9d/0x12e; Epilogkopien je case kommen mit -Og von selbst.
 * - chargen_change_atpa: inline hero::inc_at/dec_pa/dec_at/inc_pa(i) -> Parameterkopien [bp-2]/[bp-4]/[bp-6]/[bp-8].
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 */
#include <stdio.h>
#include <stdlib.h>
#include "schweif.h"

void chargen_draw_attribs(CharGen far *g)
{
	struct xy14 pos = g_chargen_attrib_pos;
	char buf[10];
	signed short i;

	for (i = 0; i <= 13; i++) {
		if (g->hero->attrib[i].normal) {
			if (g->show_moved) {
				if (g->attr_moved[i] == 1) {
					sprintf(g_text_output_buf, code_to_text(0xfd), itoa(g->hero->attrib[i].normal, buf, 10));
				} else if (g->attr_moved[i] == -1) {
					sprintf(g_text_output_buf, code_to_text(0xfc), itoa(g->hero->attrib[i].normal, buf, 10));
				} else {
					sprintf(g_text_output_buf, code_to_text(0xff), itoa(g->hero->attrib[i].normal, buf, 10));
				}
				font_print(g_font, g_text_output_buf, pos.p[i].x, pos.p[i].y - 1);
			} else {
				font_print(g_font, itoa(g->hero->attrib[i].normal, buf, 10), pos.p[i].x, pos.p[i].y - 1);
			}
		}
	}
}

/* Talent i bzw. Zauber i in Spalte n & 1 (gerade: xe/0x9d, ungerade: xo/0x12e) rechtsbuendig, Zeile n / 2 */
#define TAL(n, b) \
	itoa(get_skill_value(g->hero, i), buf, 10); \
	w = font_text_width0(g_font, buf); \
	font_print(g_font, buf, n & 1 ? xo - w : xe - w, n / 2 * 12 + b)

#define SPELL(b) \
	itoa(g->hero->spells[i], buf, 10); \
	w = font_text_width0(g_font, buf); \
	font_print(g_font, buf, j & 1 ? 0x12e - w : 0x9d - w, j / 2 * 12 + b)

void chargen_draw_page(CharGen far *g)
{
	register signed short i;
	register signed short w;
	char buf[4];
	signed short j;
	signed short xe = 0xde;
	signed short xo = 0x135;

	font_set_colors(g_font, 0xff, -1);
	switch (g->page) {
	case CHARGEN_PAGE_MAIN:
		chargen_restore_bg(g, g_font_dst);
		chargen_draw_name(g);
		chargen_draw_attribs(g);
		if (g->hero->type) {
			sprintf(g_text_output_buf, g->txt[20], g->hero->height);
			font_print(g_font, g_text_output_buf, 0xcd, 0x18);
			sprintf(g_text_output_buf, g->txt[21], g->hero->weight);
			font_print(g_font, g_text_output_buf, 0xcd, 0x24);
			font_print(g_font, g_ttx[g->hero->god + 4], 0xcd, 0x30);
			money_to_string(g_text_output_buf, g->hero->money);
			font_print(g_font, g_text_output_buf, 0xcd, 0x3c);
			font_print(g_font, itoa(g->hero->le, buf, 10), 0xac, 0xa3);
			font_print(g_font, itoa(g->hero->ae, buf, 10), 0xdd, 0xa3);
			font_print(g_font, itoa(g->hero->le + g->hero->attrib[ATTRIB_KK].normal, buf, 10), 0x128, 0xa3);
			font_print(g_font, itoa((signed char)(g->hero->mr + g->hero->mr_mod), buf, 10), 0xff, 0xb7);
		}
		break;
	case 1:
		chargen_restore_bg(g, g_font_dst);
		for (i = 0; i < 9; i++) {
			TAL(i, 0x29);
		}
		for (i = 9; i < 19; i++) {
			j = i - 9;
			TAL(j, 0x76);
		}
		font_print(g_font, itoa(g->hero->skill_incs, buf, 10), 0x10f, 0xb7);
		break;
	case 2:
		chargen_restore_bg(g, g_font_dst);
		for (i = 19; i < 26; i++) {
			j = i - 19;
			TAL(j, 0x29);
		}
		for (i = 32; i < 41; i++) {
			j = i - 32;
			TAL(j, 0x70);
		}
		font_print(g_font, itoa(g->hero->skill_incs, buf, 10), 0x10f, 0xb7);
		break;
	case 3:
		chargen_restore_bg(g, g_font_dst);
		for (i = 41; i < 50; i++) {
			j = i - 41;
			TAL(j, 0x29);
		}
		for (i = 26; i < 32; i++) {
			j = i - 26;
			TAL(j, 0x76);
		}
		for (i = 50; i < 52; i++) {
			j = i - 50;
			TAL(j, 0xa9);
		}
		font_print(g_font, itoa(g->hero->skill_incs, buf, 10), 0x10f, 0xb7);
		break;
	case CHARGEN_PAGE_ATPA:
		chargen_restore_bg(g, g_font_dst);
		font_print(g_font, itoa(g->hero->atpa_base, buf, 10), 0xe7, 0x1d);
		font_print(g_font, itoa(g->hero->atpa_base, buf, 10), 0x10c, 0x1d);
		for (i = 0; i < 7; i++) {
			itoa(g->hero->at[i], buf, 10);
			font_print(g_font, buf, 0xed - font_text_width0(g_font, buf), i * 12 + 0x2f);
			itoa(g->hero->pa[i], buf, 10);
			font_print(g_font, buf, 0x112 - font_text_width0(g_font, buf), i * 12 + 0x2f);
			itoa(get_skill_value(g->hero, i), buf, 10);
			font_print(g_font, buf, 0x13b - font_text_width0(g_font, buf), i * 12 + 0x2f);
		}
		j = (g->hero->attrib[ATTRIB_KL].normal + g->hero->attrib[ATTRIB_GE].normal + g->hero->attrib[ATTRIB_KK].normal) / 4;
		font_print(g_font, itoa(j + get_skill_value(g->hero, 7), buf, 10), 0xe7, 0x8f);
		font_print(g_font, itoa(j + get_skill_value(g->hero, 8), buf, 10), 0xe7, 0x9b);
		break;
	case 5:
		chargen_restore_bg(g, g_font_dst);
		for (i = 1; i < 6; i++) {
			j = i - 1;
			SPELL(0x29);
		}
		for (i = 33; i < 38; i++) {
			j = i - 33;
			SPELL(0x5e);
		}
		for (i = 6; i <= 11; i++) {
			j = i - 6;
			SPELL(0x91);
		}
		font_print(g_font, itoa(g->hero->spell_incs, buf, 10), 0xd9, 0xb7);
		break;
	case 6:
		chargen_restore_bg(g, g_font_dst);
		for (i = 12; i <= 17; i++) {
			j = i - 12;
			SPELL(0x29);
		}
		for (i = 18; i < 24; i++) {
			j = i - 18;
			SPELL(0x5e);
		}
		for (i = 24; i < 27; i++) {
			j = i - 24;
			SPELL(0x93);
		}
		font_print(g_font, itoa(g->hero->spell_incs, buf, 10), 0xd9, 0xb7);
		break;
	case 7:
		chargen_restore_bg(g, g_font_dst);
		for (i = 27; i < 33; i++) {
			j = i - 27;
			SPELL(0x29);
		}
		for (i = 38; i < 45; i++) {
			j = i - 38;
			SPELL(0x5e);
		}
		for (i = 45; i <= 46; i++) {
			j = i - 45;
			SPELL(0x9f);
		}
		font_print(g_font, itoa(g->hero->spell_incs, buf, 10), 0xd9, 0xb7);
		break;
	case 8:
		chargen_restore_bg(g, g_font_dst);
		for (i = 47; i <= 48; i++) {
			j = i - 47;
			SPELL(0x29);
		}
		for (i = 49; i < 58; i++) {
			j = i - 49;
			SPELL(0x46);
		}
		for (i = 58; i < 60; i++) {
			j = i - 58;
			SPELL(0x93);
		}
		font_print(g_font, itoa(g->hero->spell_incs, buf, 10), 0xd9, 0xb7);
		break;
	case 9:
		chargen_restore_bg(g, g_font_dst);
		for (i = 60; i < 76; i++) {
			j = i - 60;
			SPELL(0x29);
		}
		font_print(g_font, itoa(g->hero->spell_incs, buf, 10), 0xd9, 0xb7);
		break;
	case 10:
		chargen_restore_bg(g, g_font_dst);
		for (i = 76; i < 86; i++) {
			j = i - 76;
			SPELL(0x29);
		}
		font_print(g_font, itoa(g->hero->spell_incs, buf, 10), 0xd9, 0xb7);
		break;
	}
}

void chargen_change_atpa(CharGen far *g)
{
	signed short sel;
	signed short r;

	do {
		sel = dlg_choose(g_dialog, g->txt[28], 7, g_ttx[75], g_ttx[76], g_ttx[77], g_ttx[78],
			g_ttx[79], g_ttx[80], g_ttx[81]) - 1;
		if (sel != -2) {
			if (get_skill_value(g->hero, sel) <= 0) {
				dlg_message(g_dialog, g->txt[54]);
			} else {
				r = dlg_choose(g_dialog, g->txt[48], 2, g->txt[25], g->txt[26]);
				if (r != -1) {
					if (r == 1) {
						if (get_skill_value(g->hero, sel) >= 0 && g->hero->pa[sel] > g->hero->atpa_base) {
							g->hero->inc_at(sel);
							g->hero->dec_pa(sel);
							chargen_redraw(g);
						} else {
							dlg_message(g_dialog, g->txt[49]);
						}
					} else {
						if (get_skill_value(g->hero, sel) >= 0 && g->hero->at[sel] > g->hero->atpa_base) {
							g->hero->dec_at(sel);
							g->hero->inc_pa(sel);
							chargen_redraw(g);
						} else {
							dlg_message(g_dialog, g->txt[50]);
						}
					}
				}
			}
		}
	} while (sel != -2);
}
