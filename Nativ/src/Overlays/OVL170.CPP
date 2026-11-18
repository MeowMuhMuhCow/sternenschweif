/* OVL170 - Overlay O170 (Segment 0x22b1 in SCHWEIF.EXE): Heldenerschaffung - Zauber/Talente steigern und Typ waehlen
 * (class CharGen: txt, Portraetbereich pic_type/portrait/portrait_first/portrait_last, Seite page, Held hero).
 * chargen_reset_incs(g): Steigerungsstand zuruecksetzen (mu_bonus/ch_bonus, Portraetfelder, hero.portrait per inline
 *   set_portrait, attr_moved[14], spell_incs[86] und skill_incs[52] je zwei Bytes auf 0), Stufe 1.
 * chargen_inc_spell(g, n): Zauber n einmal steigern - Grenze 1/2/3 nach Typ (7..10) und Herkunft des Zaubers
 *   g_spell_descriptions[n].herotype, Hauszauber (Bereich g_chargen_school_ranges[magic_school], Akademieliste
 *   g->schools[magic_school].spells via is_in_word_array); zu oft -> txt[51], dreimal misslungen -> txt[42],
 *   sonst Zaubersteigerung - 1, Wurf random_interval gegen den Zauberwert: Erfolg txt[43] (inline inc_spell,
 *   Fehlversuche 0, Zaehler +1), sonst txt[44] (Fehlversuch +1); chargen_redraw.
 * chargen_menu_spells(g): Zaubermenue (Seite 5..10, Sprungtabelle) mit Untermenues aus Texten g_ttx, Wahl -> chargen_inc_spell.
 * chargen_menu_skills(g): Talentmenue (Seite 1..3, Vergleichskette), Wahl -> chargen_inc_skill(g, n, Grenze, txt).
 * chargen_select_type(g): Typ waehlen - erlaubte Typen 1..12 nach den Typvoraussetzungen g->type_reqs[i][6]
 *   (TYPE_REQ_MAX = Hoechstwert) gegen die Eigenschaften, Namen nach g_menu_texts[12], Auswahl dlg_choose, neuer
 *   Typ -> set_type, Typbild chargen_draw_type_pic, pic_type = min(Typ, 10), Portraetbereich aus g_portrait_index_m/_f
 *   (je nach Geschlecht), chargen_set_type_values, g_refresh = 1, chargen_redraw; bei Abbruch Sternzeichen-Boni per
 *   change_attrib_normal zurueck. Ohne Eigenschaften: Meldung txt[59].
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL170.CPP   -> 6293/6293 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (g_chargen_school_ranges -> _g_6c8a+0x3a, g_chargen_type_list_init -> _g_6c8a+0x4c). Ohne -Og 30 Bytes zu lang.
 * - Untermenues per Generator aus der ASM erzeugt (wovav/gen170.py + calls.py): "sel = dlg_choose(..) - 1;
 *   if (sel != -2) { sel += k; chargen_inc_spell(g, sel); }" -> dec ax / mov si,ax / cmp si,-2 / add si,k.
 * - Rahmen: "if (!g->hero->spell_incs) { Meldung; return; } else switch (..)" -> toter jmp near nach der Epilogkopie;
 *   do { } while (r != -1) (Abbruch der Einzelmenues setzt r = -1).
 * - Initialisierte lokale Tabellen als Strukturkopie aus DS (struct range8x9 t = g_chargen_school_ranges; struct type_list13
 *   list = g_chargen_type_list_init - 13 Bytes -> rep movsw + movsb), Reihenfolge der Initialisierung = Deklaration (max_incs = 1 zuerst).
 * - Inline-Elemente des Helden: set_portrait/set_type (Parameterkopien), add_spell_incs(-1) -> add byte 0ffh,
 *   inc_spell(n) mit
 *   Kopie [bp-6]. Ketten "g->pic_type = g->portrait_first = g->portrait_last = g->portrait = 0" speichern rechts nach links.
 * - chargen_select_type: "g_menu_texts[k] = g_ttx[(sex ? 0x3e : 0x32) + i]" (Zieladresse zuerst, push/pop), "if (!k)" mit
 *   -Og-Zwischenwert (int)k, "&list.type[r - 1]" als Zwischenwert [bp-18h].
 * Auffaelliges: chargen_select_type vergleicht den Hoechstwert mit Vorzeichen, den Mindestwert ohne Vorzeichen
 *   ("b > (unsigned char)v").
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 */
#include "schweif.h"

void chargen_reset_incs(CharGen far *g)
{
	register signed short i;

	g->mu_bonus = g->ch_bonus = 0;
	g->pic_type = g->portrait_first = g->portrait_last = g->portrait = 0;
	g->hero->set_portrait(g->portrait);
	for (i = 0; i < 14; i++)
		g->attr_moved[i] = 0;
	for (i = 0; i < NR_SPELLS; i++)
		g->spell_incs[i].tries = g->spell_incs[i].incs = 0;
	for (i = 0; i < NR_SKILLS; i++)
		g->skill_incs[i].tries = g->skill_incs[i].incs = 0;
	g->hero->level = 1;
}

void chargen_inc_spell(CharGen far *g, signed short n)
{
	signed short max_incs = 1;
	signed short r;
	struct range8x9 t = g_chargen_school_ranges;

	g_quiet = 1;
	if (g->hero->type == HERO_TYPE_WITCH && g_spell_descriptions[n].herotype == SPELL_DESC_HEROTYPE_WITCH)
		max_incs = 2;
	else if (g->hero->type >= HERO_TYPE_GREEN_ELF && (g_spell_descriptions[n].herotype == SPELL_DESC_HEROTYPE_GELF ||
			g_spell_descriptions[n].herotype == SPELL_DESC_HEROTYPE_SELF || g_spell_descriptions[n].herotype == SPELL_DESC_HEROTYPE_IELF))
		max_incs = 2;
	else if (g->hero->type == HERO_TYPE_DRUID && g_spell_descriptions[n].herotype == SPELL_DESC_HEROTYPE_DRUID)
		max_incs = 2;
	else if (g->hero->type == HERO_TYPE_MAGE) {
		if (g_spell_descriptions[n].herotype == SPELL_DESC_HEROTYPE_MAGE)
			max_incs = 2;
		if (t.r[g->hero->magic_school].lo <= n && t.r[g->hero->magic_school].hi >= n)
			max_incs = 2;
		if (is_in_word_array(n, g->schools[g->hero->magic_school].spells) != -1)
			max_incs = 3;
	}
	if (g->spell_incs[n].incs >= max_incs) {
		dlg_message(g_dialog, g->txt[51]);
	} else if (g->spell_incs[n].tries == 3) {
		dlg_message(g_dialog, g->txt[42]);
	} else {
		g->hero->add_spell_incs(-1);
		if (g->hero->spells[n] >= 11)
			r = random_interval(3, 0x12);
		else
			r = random_interval(2, 0x0c);
		if (g->hero->spells[n] < r) {
			dlg_message(g_dialog, g->txt[43]);
			g->hero->inc_spell(n);
			g->spell_incs[n].tries = 0;
			g->spell_incs[n].incs++;
			chargen_redraw(g);
		} else {
			dlg_message(g_dialog, g->txt[44]);
			g->spell_incs[n].tries++;
			chargen_redraw(g);
		}
	}
	g_quiet = 0;
}

void chargen_menu_spells(CharGen far *g)
{
	signed short r;
	signed short sel;

	do {
		if (!g->hero->spell_incs) {
			dlg_message(g_dialog, g->txt[37]);
			return;
		} else
		switch (g->page) {
		case 5:
			r = dlg_choose(g_dialog, g->txt[46], 3, g_ttx[219], g_ttx[224], g_ttx[220]);
			if (r != -1) {
				switch (r) {
				case 1:
					sel = dlg_choose(g_dialog, g->txt[47], 5, g_ttx[134], g_ttx[135], g_ttx[136], g_ttx[137], g_ttx[138]) - 1;
					if (sel != -2) {
						sel += SP_BEHERRSCHUNG_BRECHEN;
						chargen_inc_spell(g, sel);
					}
					break;
				case 2:
					sel = dlg_choose(g_dialog, g->txt[47], 5, g_ttx[166], g_ttx[167], g_ttx[168], g_ttx[169], g_ttx[170]) - 1;
					if (sel != -2) {
						sel += SP_BALSAM;
						chargen_inc_spell(g, sel);
					}
					break;
				case 3:
					sel = dlg_choose(g_dialog, g->txt[47], 6, g_ttx[139], g_ttx[140], g_ttx[141], g_ttx[142], g_ttx[143], g_ttx[144]) - 1;
					if (sel != -2) {
						sel += SP_BAND_UND_FESSEL;
						chargen_inc_spell(g, sel);
					}
					break;
				}
			}
			break;
		case 6:
			r = dlg_choose(g_dialog, g->txt[46], 3, g_ttx[220], g_ttx[221], g_ttx[222]);
			if (r != -1) {
				switch (r) {
				case 1:
					sel = dlg_choose(g_dialog, g->txt[47], 6, g_ttx[145], g_ttx[146], g_ttx[147], g_ttx[148], g_ttx[149], g_ttx[150]) - 1;
					if (sel != -2) {
						sel += SP_HORRIPHOBUS;
						chargen_inc_spell(g, sel);
					}
					break;
				case 2:
					sel = dlg_choose(g_dialog, g->txt[47], 6, g_ttx[151], g_ttx[152], g_ttx[153], g_ttx[154], g_ttx[155], g_ttx[156]) - 1;
					if (sel != -2) {
						sel += SP_FUROR_BLUT;
						chargen_inc_spell(g, sel);
					}
					break;
				case 3:
					sel = dlg_choose(g_dialog, g->txt[47], 3, g_ttx[157], g_ttx[158], g_ttx[159]) - 1;
					if (sel != -2) {
						sel += SP_ELEMENTARE_BESCHWOEREN;
						chargen_inc_spell(g, sel);
					}
					break;
				}
			}
			break;
		case 7:
			r = dlg_choose(g_dialog, g->txt[46], 3, g_ttx[223], g_ttx[225], g_ttx[226]);
			if (r != -1) {
				switch (r) {
				case 1:
					sel = dlg_choose(g_dialog, g->txt[47], 6, g_ttx[160], g_ttx[161], g_ttx[162], g_ttx[163], g_ttx[164], g_ttx[165]) - 1;
					if (sel != -2) {
						sel += SP_AXXELERATUS;
						chargen_inc_spell(g, sel);
					}
					break;
				case 2:
					sel = dlg_choose(g_dialog, g->txt[47], 7, g_ttx[171], g_ttx[172], g_ttx[173], g_ttx[174], g_ttx[175], g_ttx[176], g_ttx[177]) - 1;
					if (sel != -2) {
						sel += SP_ADLERAUG;
						chargen_inc_spell(g, sel);
					}
					break;
				case 3:
					sel = dlg_choose(g_dialog, g->txt[47], 2, g_ttx[178], g_ttx[179]) - 1;
					if (sel != -2) {
						sel += SP_CHAMAELIONI;
						chargen_inc_spell(g, sel);
					}
					break;
				}
			}
			break;
		case 8:
			r = dlg_choose(g_dialog, g->txt[46], 3, g_ttx[226], g_ttx[227], g_ttx[228]);
			if (r != -1) {
				switch (r) {
				case 1:
					sel = dlg_choose(g_dialog, g->txt[47], 2, g_ttx[180], g_ttx[181]) - 1;
					if (sel != -2) {
						sel += SP_HARMLOSE_GESTALT;
						chargen_inc_spell(g, sel);
					}
					break;
				case 2:
					sel = dlg_choose(g_dialog, g->txt[47], 9, g_ttx[182], g_ttx[183], g_ttx[184], g_ttx[185], g_ttx[186], g_ttx[187], g_ttx[188], g_ttx[189], g_ttx[190]) - 1;
					if (sel != -2) {
						sel += SP_BLITZ;
						chargen_inc_spell(g, sel);
					}
					break;
				case 3:
					sel = dlg_choose(g_dialog, g->txt[47], 2, g_ttx[191], g_ttx[192]) - 1;
					if (sel != -2) {
						sel += SP_HEXENBLICK;
						chargen_inc_spell(g, sel);
					}
					break;
				}
			}
			break;
		case 9:
			sel = dlg_choose(g_dialog, g->txt[47], 16, g_ttx[193], g_ttx[194], g_ttx[195], g_ttx[196], g_ttx[197], g_ttx[198], g_ttx[199], g_ttx[200], g_ttx[201], g_ttx[202], g_ttx[203], g_ttx[204], g_ttx[205], g_ttx[206], g_ttx[207], g_ttx[208]) - 1;
			if (sel != -2) {
				sel += SP_ADLER_WOLF;
				chargen_inc_spell(g, sel);
			} else
				r = -1;
			break;
		case 10:
			sel = dlg_choose(g_dialog, g->txt[47], 10, g_ttx[209], g_ttx[210], g_ttx[211], g_ttx[212], g_ttx[213], g_ttx[214], g_ttx[215], g_ttx[216], g_ttx[217], g_ttx[218]) - 1;
			if (sel != -2) {
				sel += SP_ABVENENUM;
				chargen_inc_spell(g, sel);
			} else
				r = -1;
			break;
		}
	} while (r != -1);
}

void chargen_menu_skills(CharGen far *g)
{
	signed short r;
	signed short sel;

	do {
		if (!g->hero->skill_incs) {
			dlg_message(g_dialog, g->txt[37]);
			return;
		} else
		switch (g->page) {
		case 1:
			r = dlg_choose(g_dialog, g->txt[36], 2, g_ttx[127], g_ttx[128]);
			if (r != -1) {
				switch (r) {
				case 1:
					sel = dlg_choose(g_dialog, g->txt[38], 9, g_ttx[75], g_ttx[76], g_ttx[77], g_ttx[78], g_ttx[79], g_ttx[80], g_ttx[81], g_ttx[82], g_ttx[83]) - 1;
					if (sel != -2)
						chargen_inc_skill(g, sel, 1, g->txt[39]);
					break;
				case 2:
					sel = dlg_choose(g_dialog, g->txt[38], 10, g_ttx[84], g_ttx[85], g_ttx[86], g_ttx[87], g_ttx[88], g_ttx[89], g_ttx[90], g_ttx[91], g_ttx[92], g_ttx[93]) - 1;
					if (sel != -2) {
						sel += TA_AKROBATIK;
						chargen_inc_skill(g, sel, 2, g->txt[40]);
					}
					break;
				}
			}
			break;
		case 2:
			r = dlg_choose(g_dialog, g->txt[36], 2, g_ttx[129], g_ttx[130]);
			if (r != -1) {
				switch (r) {
				case 1:
					sel = dlg_choose(g_dialog, g->txt[38], 7, g_ttx[94], g_ttx[95], g_ttx[96], g_ttx[97], g_ttx[98], g_ttx[99], g_ttx[100]) - 1;
					if (sel != -2) {
						sel += TA_BEKEHREN;
						chargen_inc_skill(g, sel, 2, g->txt[40]);
					}
					break;
				case 2:
					sel = dlg_choose(g_dialog, g->txt[38], 9, g_ttx[107], g_ttx[108], g_ttx[109], g_ttx[110], g_ttx[111], g_ttx[112], g_ttx[113], g_ttx[114], g_ttx[115]) - 1;
					if (sel != -2) {
						sel += TA_ALCHIMIE;
						chargen_inc_skill(g, sel, 3, g->txt[41]);
					}
					break;
				}
			}
			break;
		case 3:
			r = dlg_choose(g_dialog, g->txt[36], 3, g_ttx[131], g_ttx[132], g_ttx[133]);
			if (r != -1) {
				switch (r) {
				case 1:
					sel = dlg_choose(g_dialog, g->txt[38], 9, g_ttx[116], g_ttx[117], g_ttx[118], g_ttx[119], g_ttx[120], g_ttx[121], g_ttx[122], g_ttx[123], g_ttx[124]) - 1;
					if (sel != -2) {
						sel += TA_ABRICHTEN;
						chargen_inc_skill(g, sel, 2, g->txt[40]);
					}
					break;
				case 2:
					sel = dlg_choose(g_dialog, g->txt[38], 6, g_ttx[101], g_ttx[102], g_ttx[103], g_ttx[104], g_ttx[105], g_ttx[106]) - 1;
					if (sel != -2) {
						sel += TA_FAEHRTENSUCHEN;
						chargen_inc_skill(g, sel, 2, g->txt[40]);
					}
					break;
				case 3:
					sel = dlg_choose(g_dialog, g->txt[38], 2, g_ttx[125], g_ttx[126]) - 1;
					if (sel != -2) {
						sel += TA_GEFAHRENSINN;
						chargen_inc_skill(g, sel, 1, g->txt[39]);
					}
					break;
				}
			}
			break;
		}
	} while (r != -1);
}

void chargen_select_type(CharGen far *g)
{
	signed char old = -1;
	signed char k;
	signed char v;
	signed short i;
	signed short j;
	signed short bad;
	signed short r;
	struct type_list13 list = g_chargen_type_list_init;

	g_quiet = 1;
	if (g->hero->attrib[ATTRIB_MU].normal != 0) {
		old = g->hero->type;
		if (g->mu_bonus)
			change_attrib_normal(g->hero, ATTRIB_MU, -1);
		if (g->ch_bonus)
			change_attrib_normal(g->hero, ATTRIB_CH, -1);
		for (k = 0, i = HERO_TYPE_JUGGLER; i <= HERO_TYPE_SYLVAN_ELF; i++) {
			for (bad = 0, j = 0; j < 6; j++) {
				if (g->type_reqs[i][j].value != 0) {
					v = g->hero->attrib[g->type_reqs[i][j].attrib].normal;
					if (g->type_reqs[i][j].value & TYPE_REQ_MAX) {
						if (v > (g->type_reqs[i][j].value & 0x7f))
							bad = 1;
#if FIX_0178
					/* Fix #178: Mindestwert wie den Hoechstwert mit Vorzeichen vergleichen */
					} else if ((signed short)g->type_reqs[i][j].value > (signed short)v)
#else
					} else if (g->type_reqs[i][j].value > (unsigned char)v)
#endif
						bad = 1;
				}
			}
			if (bad == 0) {
				g_menu_texts[k] = g_ttx[(g->hero->sex ? 0x3e : 0x32) + i];
				list.type[k] = i;
				k++;
			}
		}
		if (!k) {
			if (chargen_can_move_attribs(g) == 0)
				dlg_choose(g_dialog, g->txt[66], 0);
			else
				dlg_choose(g_dialog, g->txt[16], 0);
		} else {
			r = dlg_choose(g_dialog, g->txt[15], k, g_menu_texts[0], g_menu_texts[1], g_menu_texts[2], g_menu_texts[3], g_menu_texts[4],
				g_menu_texts[5], g_menu_texts[6], g_menu_texts[7], g_menu_texts[8], g_menu_texts[9], g_menu_texts[10], g_menu_texts[11]);
			if (r != -1 && list.type[r - 1] != old) {
				g->hero->set_type(list.type[r - 1]);
				chargen_draw_type_pic(g, g_vga_buf, g->hero->type);
				g->pic_type = g->hero->type > HERO_TYPE_GREEN_ELF ? HERO_TYPE_GREEN_ELF : g->hero->type;
				if (g->hero->sex) {
					g->portrait_first = g->portrait = g_portrait_index_f[g->pic_type];
					g->portrait_last = g_portrait_index_m_next[g->pic_type] - 1;
					g->hero->set_portrait(g->portrait);
				} else {
					g->portrait_first = g->portrait = g_portrait_index_m[g->pic_type];
					g->portrait_last = g_portrait_index_f[g->pic_type] - 1;
					g->hero->set_portrait(g->portrait);
				}
				g->mu_bonus = g->ch_bonus = 0;
				chargen_set_type_values(g);
				g_refresh = 1;
				chargen_redraw(g);
			} else {
				if (g->mu_bonus)
					change_attrib_normal(g->hero, ATTRIB_MU, 1);
				if (g->ch_bonus)
					change_attrib_normal(g->hero, ATTRIB_CH, 1);
			}
		}
	} else
		dlg_message(g_dialog, g->txt[59]);
	g_quiet = 0;
}
