/* OVL167 - Overlay O167 (Segment 0x22a6 in SCHWEIF.EXE): Heldenerschaffung, Teil 2 (class CharGen g, Held g->hero).
 * chargen_roll_attribs(g): Eigenschaften neu wuerfeln - Name/Geschlecht retten, Held loeschen, chargen_reset_incs,
 * positive (MU..KK, W 8..13) und negative (AG..JZ, W 2..7) Eigenschaften einzeln einer noch freien zuordnen
 * (Auswahl g_menu_texts mit Namen g_ttx[308 + j], set_attrib, chargen_redraw); Abbruch -> Held wieder leeren.
 * g_quiet als Merker. chargen_calc_atpa(g): AT/PA-Basis = (IN + KK + GE) / 5 gerundet, AT/PA je Waffentalent plus/
 * minus halber Talentwert. chargen_auto_inc(g): automatische Steigerung - Talente nach g->auto_skills[typ]
 * (chargen_auto_inc_skill), Magier: Zauberliste g->auto_spells[2] neu (Akademie g->schools[magic_school],
 * g_spells_index, feste Zauber Flim Flam/Blitz/Ignifaxius/Balsam/Claudibus, Rest W85), dann Zauber nach
 * g->auto_spells[typ - 7] (chargen_auto_inc_spell). chargen_set_type_values(g): Typwerte setzen (Talente g->type_skills,
 * Zauber g->type_spells, Akademiewahl txt[18] fuer Magier, Steigerungen g_chargen_skill_incs/_spell_incs/
 * _incs_swap, Umverteilung txt[63..65], LE/AE g_type_le/_ae, Magier-Option txt[62], Groesse/Gewicht
 * g_type_height/_weight, Geld g_type_money_ranges[typ - 1], Sternzeichen-switch 1..12). chargen_auto_inc_skill/_spell(g, i): ein
 * Steigerungsversuch (max. 18, 3 Fehlversuche in g->skill_incs/spell_incs, Punkte skill_incs/spell_incs).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL167.CPP   -> 5115/5115 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (g_chargen_skill_incs -> _s_6c72+2). Ohne -Og 1371, ohne -Oi 0/523/960 Abweichungen.
 * - C++ wegen vieler inline-Setter (je Expansion eigene Parameterkopie, Index in DI wenn frei), is_mage(), abs().
 * - -Oi: strcpy/memset(0x5ef) inline; -Og: (int)cnt [bp-20h], -half [bp-23h], &tab[k] [bp-3eh].
 * - chargen_set_type_values: Initialisierung ski (12 Bytes, rep movsw, ans Ende [bp-4ah]) vor spi/swp (6 Bytes,
 *   Wortpaare, hinter den Skalaren [bp-12h]/[bp-18h]); "set_magic_school(dlg_choose(..) - 1)" -> dec al;
 *   "spell_incs += (signed char)-10".
 * - chargen_auto_inc: Schleifenbedingung "g->hero->skill_incs > 0" (cmp byte,0 / ja) - der Zaehler kommt darin
 *   nicht vor.
 * Originalfehler: chargen_auto_inc - die Schleifen ueber g->auto_skills[typ][i] (25 Eintraege) bzw.
 *   g->auto_spells[typ - 7][i] (45) enden nur, wenn die Steigerungen verbraucht sind; chargen_auto_inc_skill/_spell
 *   verbrauchen keine bei Wert 18 oder 3 Fehlversuchen -> i laeuft ueber die Zeile hinaus (liest Nachbarzeilen/
 *   fremden Speicher). Die Akademiezauber werden zweimal hintereinander in g->auto_spells[2] eingetragen.
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:6c74..6c8c, dsana) - initialisierte Daten in Adressreihenfolge */
struct incs12 g_chargen_skill_incs = { { 20, 20, 20, 20, 20, 20, 20, 20, 15, 20, 20, 20 } };
struct incs6 g_chargen_spell_incs = { { 25, 25, 40, 20, 20, 20 } };
struct incs6 g_chargen_incs_swap = { { 5, 5, 5, 0, 0, 0 } };

void chargen_roll_attribs(CharGen far *g)
{
	signed char roll;
	signed char cnt;
	signed char sex;
	char nm[10];
	signed short i;
	signed short j;
	signed short sel;
	signed char list[14];

	g_quiet = 1;
	if (g->hero->type != 0)
		g_refresh = 1;
	strcpy(nm, g->hero->name);
	sex = g->hero->sex;
	memset(g->hero, 0, sizeof(struct hero));
	chargen_reset_incs(g);
	g->hero->set_sex(sex);
	strcpy(g->hero->file_name, nm);
	strcpy(g->hero->name, nm);
	chargen_redraw(g);
	for (i = ATTRIB_MU; i <= ATTRIB_KK; i++) {
		roll = random_interval(8, 13);
		cnt = 0;
		for (j = ATTRIB_MU; j <= ATTRIB_KK; j++) {
			if (!g->hero->attrib[j].normal) {
				list[cnt] = j;
				g_menu_texts[cnt] = g_ttx[j + 308];
				cnt++;
			}
		}
		sprintf(g_text_output_buf, g->txt[17], roll);
		sel = dlg_choose(g_dialog, g_text_output_buf, cnt, g_menu_texts[0], g_menu_texts[1], g_menu_texts[2],
			g_menu_texts[3], g_menu_texts[4], g_menu_texts[5], g_menu_texts[6]);
		if (sel == -1)
			break;
		sel = list[sel - 1];
		g->hero->set_attrib_normal(sel, roll);
		set_attrib(g->hero, sel, roll);
		chargen_redraw(g);
	}
	if (sel != -1) {
		for (i = ATTRIB_AG; i <= ATTRIB_JZ; i++) {
			roll = random_interval(2, 7);
			cnt = 0;
			for (j = ATTRIB_AG; j <= ATTRIB_JZ; j++) {
				if (!g->hero->attrib[j].normal) {
					list[cnt] = j;
					g_menu_texts[cnt] = g_ttx[j + 308];
					cnt++;
				}
			}
			sprintf(g_text_output_buf, g->txt[17], roll);
			sel = dlg_choose(g_dialog, g_text_output_buf, cnt, g_menu_texts[0], g_menu_texts[1],
				g_menu_texts[2], g_menu_texts[3], g_menu_texts[4], g_menu_texts[5], g_menu_texts[6]);
			if (sel == -1)
				break;
			sel = list[sel - 1];
			g->hero->set_attrib_normal(sel, roll);
			set_attrib(g->hero, sel, roll);
			chargen_redraw(g);
		}
	}
	if (sel == -1) {
		memset(g->hero, 0, sizeof(struct hero));
		chargen_reset_incs(g);
		g->hero->set_sex(sex);
		strcpy(g->hero->file_name, nm);
		strcpy(g->hero->name, nm);
		chargen_redraw(g);
	}
	g_quiet = 0;
}

void chargen_calc_atpa(CharGen far *g)
{
	div_t r;
	signed short half;
	signed short i;

	r = div(g->hero->attrib[ATTRIB_IN].normal + g->hero->attrib[ATTRIB_KK].normal +
		g->hero->attrib[ATTRIB_GE].normal, 5);
	if (r.rem >= 3)
		r.quot++;
	g->hero->set_atpa_base(r.quot);
	for (i = TA_WAFFENLOS; i <= TA_ZWEIHAENDER; i++) {
		g->hero->set_at(i, r.quot);
		g->hero->set_pa(i, r.quot);
		if (get_skill_value(g->hero, i) < 0) {
			half = abs(get_skill_value(g->hero, i)) / 2;
			g->hero->add_at(i, -half);
			g->hero->add_pa(i, -half);
			if (get_skill_value(g->hero, i) % 2)
				g->hero->dec_pa(i);
		} else {
			half = get_skill_value(g->hero, i) / 2;
			g->hero->add_at(i, half);
			g->hero->add_pa(i, half);
			if (get_skill_value(g->hero, i) % 2)
				g->hero->inc_at(i);
		}
	}
}

void chargen_auto_inc(CharGen far *g)
{
	signed short typ;
	signed short skill;
	signed short spell;
	signed short i;
	signed short k;

	typ = g->hero->type;
#if FIX_0116
	/* Fix #116: nicht ueber die Zeile (25 Talente) hinaus lesen */
	for (i = 0; i < 25 && g->hero->skill_incs > 0; i++) {
		skill = g->auto_skills[typ][i].skill;
#else
	for (i = 0; g->hero->skill_incs > 0; i++) {
		skill = g->auto_skills[typ][i].skill;
#endif
		chargen_auto_inc_skill(g, skill);
	}
	k = 0;
	if (typ == HERO_TYPE_MAGE) {
		for (i = 0; g->schools[g->hero->magic_school].count > i; i++) {
			g->auto_spells[HERO_TYPE_MAGE - HERO_TYPE_WITCH][k] = g->schools[g->hero->magic_school].spells[i];
			k++;
		}
		for (i = 0; g_spells_index[g->hero->magic_school].count > i; i++) {
			g->auto_spells[HERO_TYPE_MAGE - HERO_TYPE_WITCH][k] = g_spells_index[g->hero->magic_school].first + i;
			k++;
		}
		g->auto_spells[HERO_TYPE_MAGE - HERO_TYPE_WITCH][k] = SP_FLIM_FLAM;
		k++;
		g->auto_spells[HERO_TYPE_MAGE - HERO_TYPE_WITCH][k] = SP_BLITZ;
		k++;
		g->auto_spells[HERO_TYPE_MAGE - HERO_TYPE_WITCH][k] = SP_IGNIFAXIUS;
		k++;
		g->auto_spells[HERO_TYPE_MAGE - HERO_TYPE_WITCH][k] = SP_BALSAM;
		k++;
		g->auto_spells[HERO_TYPE_MAGE - HERO_TYPE_WITCH][k] = SP_CLAUDIBUS;
		k++;
		for (i = 0; g->schools[g->hero->magic_school].count > i; i++) {
			g->auto_spells[HERO_TYPE_MAGE - HERO_TYPE_WITCH][k] = g->schools[g->hero->magic_school].spells[i];
			k++;
		}
		for (i = 0; g->schools[g->hero->magic_school].count > i; i++) {
			g->auto_spells[HERO_TYPE_MAGE - HERO_TYPE_WITCH][k] = g->schools[g->hero->magic_school].spells[i];
			k++;
		}
		while (k < 45) {
			g->auto_spells[HERO_TYPE_MAGE - HERO_TYPE_WITCH][k] = random_schweif(85);
			k++;
		}
	}
#if FIX_0116
	/* Fix #116: nicht ueber die Zeile (45 Zauber) hinaus lesen */
	for (i = 0; i < 45 && g->hero->spell_incs > 0; i++) {
		spell = g->auto_spells[typ - HERO_TYPE_WITCH][i];
#else
	for (i = 0; g->hero->spell_incs > 0; i++) {
		spell = g->auto_spells[typ - HERO_TYPE_WITCH][i];
#endif
		chargen_auto_inc_spell(g, spell);
	}
}

void chargen_set_type_values(CharGen far *g)
{
	signed short i;
	signed short k;
	signed short n;
	signed short typ;
	struct money_range far *tab;
	struct incs12 ski = g_chargen_skill_incs;
	struct incs6 spi = g_chargen_spell_incs;
	struct incs6 swp = g_chargen_incs_swap;

	g_quiet = 1;
	typ = g->hero->type;
	for (i = 0; i < NR_SKILLS; i++) {
		set_skill_value(g->hero, i, g->type_skills[typ][i]);
		g->skill_incs[i].tries = g->skill_incs[i].incs = 0;
	}
#if FIX_0372
	/* Fix #372: Zaubersteigerungen eines vorher gewaehlten Zaubertyps zuruecksetzen */
	g->hero->set_skill_incs(ski.v[typ - 1]);
	g->hero->set_spell_incs(0);
	if (g->hero->is_mage()) {
#else
	g->hero->set_skill_incs(ski.v[typ - 1]);
	if (g->hero->is_mage()) {
#endif
		for (i = 0; i < NR_SPELLS; i++) {
			g->hero->set_spell(i, g->type_spells[typ - HERO_TYPE_WITCH][i]);
			g->spell_incs[i].tries = g->spell_incs[i].incs = 0;
		}
		if (typ == HERO_TYPE_MAGE) {
			g->hero->staffspell_lvl = 1;
			do {
				g->hero->set_magic_school(dlg_choose(g_dialog, g->txt[18], 9, g_ttx[219], g_ttx[220],
					g_ttx[221], g_ttx[223], g_ttx[224], g_ttx[225], g_ttx[227], g_ttx[229],
					g_ttx[230]) - 1);
			} while (g->hero->magic_school == -2);
			for (i = 0; g->schools[g->hero->magic_school].count > i; i++)
				g->hero->add_spell(g->schools[g->hero->magic_school].spells[i],
					g->schools[g->hero->magic_school].bonus[i]);
		}
		g->hero->set_spell_incs(spi.v[typ - HERO_TYPE_WITCH]);
		n = swp.v[typ - HERO_TYPE_WITCH];
		if (n != 0 && g_game_mode == GAME_MODE_ADVANCED && dlg_yes_no(g_dialog, g->txt[63]) == 1) {
			sprintf(g_text_output_buf, g->txt[64], n);
			i = dlg_input(g_dialog, 1, g_text_output_buf, 0, 5, 0);
			if (i > 0) {
				if (i > n)
					i = n;
				n -= i;
				g->hero->add_spell_incs(-i);
				g->hero->add_skill_incs(i);
			} else {
				sprintf(g_text_output_buf, g->txt[65], n);
				i = dlg_input(g_dialog, 1, g_text_output_buf, 0, 5, 0);
				if (i > 0) {
					if (i > n)
						i = n;
					g->hero->add_spell_incs(i);
					g->hero->add_skill_incs(-i);
				}
			}
		}
	}
	g->hero->set_le_max(g_type_le[typ]);
	g->hero->set_le(g_type_le[typ]);
	g->hero->set_ae_max(g_type_ae[typ]);
	g->hero->set_ae(g_type_ae[typ]);
	/* Magier: 10 Zaubersteigerungen gegen W(3..8) AE */
	if (typ == HERO_TYPE_MAGE && g_game_mode == GAME_MODE_ADVANCED && dlg_yes_no(g_dialog, g->txt[62]) == 1) {
		g->hero->spell_incs += (signed char)-10;
		g->hero->set_ae_max(g->hero->ae_max + random_interval(3, 8));
		g->hero->set_ae(g->hero->ae_max);
	}
	g->hero->set_height(random_interval(g_type_height[typ].lo, g_type_height[typ].hi));
	g->hero->set_weight((g->hero->height - g_type_weight[typ]) * 40);
	i = random_schweif(20);
	tab = g_type_money_ranges[typ - 1];
	for (k = 0; tab[k].lim < i; k++)
		;
	g->hero->set_money(random_interval(tab[k].lo, tab[k].hi) * 10);
	calc_mr(g->hero, 0);
	g->hero->set_god(random_schweif(12));
	switch (g->hero->god) {		/* Sternzeichen */
	case 1:
		change_attrib_normal(g->hero, ATTRIB_MU, 1);
		g->mu_bonus = 1;
		break;
	case 2:
		add_skill_value(g->hero, TA_SCHWERTER, 1);
		break;
	case 3:
		add_skill_value(g->hero, TA_SCHWIMMEN, 1);
		break;
	case 4:
		add_skill_value(g->hero, TA_HEILEN_GIFT, 1);
		break;
	case 5:
		add_skill_value(g->hero, TA_MENSCHENKENNTNIS, 1);
		break;
	case 6:
		add_skill_value(g->hero, TA_ALCHIMIE, 1);
		break;
	case 7:
		add_skill_value(g->hero, TA_FAEHRTENSUCHEN, 1);
		add_skill_value(g->hero, TA_SCHUSSWAFFEN, 1);
		break;
	case 8:
		change_attrib_normal(g->hero, ATTRIB_CH, 1);
		g->ch_bonus = 1;
		break;
	case 9:
		add_skill_value(g->hero, TA_TASCHENDIEBSTAHL, 1);
		add_skill_value(g->hero, TA_SCHLEICHEN, 1);
		break;
	case 10:
		add_skill_value(g->hero, TA_HEILEN_KRANKHEITEN, 1);
		add_skill_value(g->hero, TA_HEILEN_WUNDEN, 1);
		break;
	case 11:
		add_skill_value(g->hero, TA_KRIEGSKUNST, 1);
		break;
	case 12:
		add_skill_value(g->hero, TA_BETOEREN, 1);
		add_skill_value(g->hero, TA_TANZEN, 1);
		add_skill_value(g->hero, TA_MUSIZIEREN, 1);
		break;
	}
	chargen_calc_atpa(g);
	if (g_game_mode == GAME_MODE_BEGINNER)
		chargen_auto_inc(g);
	g_quiet = 0;
}

void chargen_auto_inc_skill(CharGen far *g, signed short i)
{
	signed short done = 0;
	signed short r;

	while (!done) {
		if (g->skill_incs[i].tries == 3 || get_skill_value(g->hero, i) == 18)
			done = 1;
		else {
			g->hero->skill_incs += (signed char)-1;
			if (!g->hero->skill_incs)
				done = 1;
			if (get_skill_value(g->hero, i) >= 11)
				r = random_interval(3, 18);
			else
				r = random_interval(2, 12);
			if (get_skill_value(g->hero, i) < r) {
				add_skill_value(g->hero, i, 1);
				g->skill_incs[i].tries = 0;
				done = 1;
			} else
				g->skill_incs[i].tries++;
		}
	}
}

void chargen_auto_inc_spell(CharGen far *g, signed short i)
{
	signed short done = 0;
	signed short r;

	while (!done) {
		if (g->spell_incs[i].tries == 3 || g->hero->spells[i] == 18)
			done = 1;
		else {
			g->hero->spell_incs += (signed char)-1;
			if (!g->hero->spell_incs)
				done = 1;
			if (g->hero->spells[i] >= 11)
				r = random_interval(3, 18);
			else
				r = random_interval(2, 12);
			if (g->hero->spells[i] < r) {
				g->hero->inc_spell(i);
				g->spell_incs[i].tries = 0;
				done = 1;
			} else
				g->spell_incs[i].tries++;
		}
	}
}
