/* OVL086 - Overlay O086 (Segment 0x20fa in SCHWEIF.EXE): Stufenanstieg (vgl. DSA1 seg050) - Methoden der Klasse Lvl
 * (s_class.h; Daten struct inv_sheet: Texte txt, Modus page, Auswahl sel, Held hero, lvl_auto). Held: Typ (>= 7
 * zauberkundig), Stufe, AP, Eigenschaften attrib[] (MU..KK positiv, AG..JZ negativ), Talentsteigerungen
 * skill_incs (unsigned char), Zauberwerte spells[], Zaubersteigerungen spell_incs, Akademie magic_school.
 * redraw: Heldenbogen neu (Inv_g_09bb(this)), g_sheet_hunger_shown/g_sheet_thirst_shown = -1, bars_refresh. inc_attrib/dec_attrib: positive
 * Eigenschaft +1 (alle < 20?) bzw. negative -1 (mit 3 Wuerfen W20 <= Wert), automatisch nach Reihenfolge
 * g_lvl_attrib_order[Typ - 1] mit 75 %, sonst Menue mit Namen g_ttx[0x134..0x141]. inc_le: W6+1 LE (Zauberkundige
 * +2, Aufteilung LE/AE per dlg_input; Magier: 10 Zaubersteigerungen gegen W(3..8) AE). inc_mr: add_mr_mod.
 * auto_skills: Talente nach Vorlage g_auto_skills[Typ] (25 x {Talent, max}) bis zu 6 Runden, Rest zufaellig
 * (inc_skill_auto). auto_spells: Zauber je Typ 7..12 (Herkunft g_spell_descriptions[i].herotype, Listen
 * g_lvl_spells_*, Magier Akademieliste g_school_spells[magic_school] und g_lvl_spells_school), Rest zufaellig
 * (inc_spell_auto). inc_points: Steigerungen aus g_lvl_skill_incs/_spell_incs/_incs_swap (lokale Kopien),
 * Umverteilung Talent <-> Zauber, Schleifen mit lvl_select_skill -> inc_skill bzw. select_spell -> inc_spell und
 * Menue (Ende/automatisch/abbrechen). run: Puffer anlegen (g_inc_skills/g_inc_spells, Dateien 0x9c und 0x1f8),
 * Zaehler loeschen, Stufe +1, Meldungen, Automatik-Frage, alle Schritte, Puffer freigeben.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL086.CPP   -> 5759/5759 Bytes GLEICH (18.10.2026), cobjfix laeuft
 * - C++: Methoden mit this als erstem Parameter (Aufrufe der O087-Methoden inc_spell/inc_skill/..._auto),
 *   inline hero::is_mage() { return type >= 7; } (mov ax,1 / xor ax,ax / or ax,ax), add_skill_incs/
 *   add_spell_incs(signed char) mit Byte-Parameterkopien ([bp-13h]..[bp-18h]; "add_skill_incs(-n)" -> neg al).
 * - dec_attrib, manueller Zweig: negative Eigenschaft als Bytezugriff "((signed char far *)hero)[sel * 3 +
 *   0x5b]" (Index zuerst, Zeiger als mov dx,[+15h] / add ax,[+13h]); hero->attrib[sel + 7].normal und alle
 *   anderen Struktur-/Cast-Varianten laden zuerst les bx,hero. Im automatischen Zweig dagegen attrib[sel].
 * - inc_points: Scalare done/n/v zuerst deklarieren, dann die drei initialisierten Kopien (ta 12 Bytes per rep
 *   movsw ans Ende [bp-24h], sa/xa 6 Bytes wortweise nach [bp-0ch]/[bp-12h]) von g_lvl_skill_incs/g_lvl_spell_incs/
 *   g_lvl_incs_swap (globale Tabellen: lokale Felder mit Initialisierer laegen hinter den Zauberlisten).
 * - Phase B (19.10.2026): eigene Daten ab ds:5a80. g_lvl_attrib_order[12] wird mit Typ - 1 indiziert; der MAP-Name
 *   g_lvl_attrib_prio (ds:5a72) ist g_lvl_attrib_order - 1, die 14 Bytes davor gehoeren zu O084 (Ende der Vorlage
 *   by_type, dsana/korrekturen.tsv). dec_attrib: "sel = (signed short)g_lvl_attrib_order[..].neg[i]" - ohne den Cast
 *   faltet BCC -14 + 7 nicht in die Adresse (mov dx,ofs / add dx,7 / add ax,dx). Die Zauberlisten der Akademien
 *   g_lvl_spells_school1..9 hatten keine Namen.
 * - "g_sheet_hunger_shown = g_sheet_thirst_shown = -1", "hero = g_sheet_hero = h", "slot = slot2 = -1", "tries = incs = 0": Ketten
 *   rechts -> links.
 * - "for (i = k = 0; ...)" -> xor ax,ax / mov di,ax / mov si,ax.
 * Originalfehler: auto_spells - die erste Schleife jedes Falls (Typus 7, 8, 10, 11, 12) hat keine Initialisierung,
 *   i ist nur in der ersten Runde 1, danach der Endstand der vorigen Listenschleife; Typus 9 liest list[0]
 *   doppelt ("t = list[i], i++" nach dem Rumpf) und geht die Schulliste g_school_spells zweimal identisch durch;
 *   Typus 7/8 fehlt die dritte Schleife der anderen Faelle. inc_points: "v -= n" wird nicht mehr benutzt.
 *   (Hinweis: der Kopfkommentar von OVL087 vertauschte Talente und Zauber - spells/spell_incs sind die Zauber,
 *   get_skill_value/skill_incs die 52 Talente.)
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 */
#include <stdio.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:5a80..5c7e, dsana) in Adressreihenfolge */
struct attrib_prio g_lvl_attrib_order[12] = {	/* je Typ 1..12 (Index type - 1): Eigenschaften in Versuchsreihenfolge */
	{ { ATTRIB_GE, ATTRIB_FF, ATTRIB_MU, ATTRIB_KK, ATTRIB_KL, ATTRIB_CH, ATTRIB_IN },
	  { ATTRIB_AG, ATTRIB_JZ, ATTRIB_HA, ATTRIB_RA, ATTRIB_TA, ATTRIB_GG, ATTRIB_NG } },	/* HERO_TYPE_JUGGLER */
	{ { ATTRIB_IN, ATTRIB_GE, ATTRIB_KK, ATTRIB_FF, ATTRIB_MU, ATTRIB_KL, ATTRIB_CH },
	  { ATTRIB_RA, ATTRIB_HA, ATTRIB_JZ, ATTRIB_TA, ATTRIB_GG, ATTRIB_AG, ATTRIB_NG } },	/* HERO_TYPE_HUNTER */
	{ { ATTRIB_KK, ATTRIB_MU, ATTRIB_GE, ATTRIB_FF, ATTRIB_IN, ATTRIB_CH, ATTRIB_KL },
	  { ATTRIB_JZ, ATTRIB_TA, ATTRIB_GG, ATTRIB_HA, ATTRIB_RA, ATTRIB_AG, ATTRIB_NG } },	/* HERO_TYPE_WARRIOR */
	{ { ATTRIB_GE, ATTRIB_FF, ATTRIB_KK, ATTRIB_MU, ATTRIB_IN, ATTRIB_KL, ATTRIB_CH },
	  { ATTRIB_GG, ATTRIB_AG, ATTRIB_RA, ATTRIB_HA, ATTRIB_TA, ATTRIB_JZ, ATTRIB_NG } },	/* HERO_TYPE_ESTRAY */
	{ { ATTRIB_KK, ATTRIB_MU, ATTRIB_GE, ATTRIB_FF, ATTRIB_CH, ATTRIB_KL, ATTRIB_IN },
	  { ATTRIB_AG, ATTRIB_RA, ATTRIB_NG, ATTRIB_JZ, ATTRIB_TA, ATTRIB_GG, ATTRIB_HA } },	/* HERO_TYPE_THORWALIAN */
	{ { ATTRIB_KK, ATTRIB_FF, ATTRIB_MU, ATTRIB_KL, ATTRIB_IN, ATTRIB_GE, ATTRIB_CH },
	  { ATTRIB_RA, ATTRIB_AG, ATTRIB_NG, ATTRIB_HA, ATTRIB_TA, ATTRIB_JZ, ATTRIB_GG } },	/* HERO_TYPE_DWARF */
	{ { ATTRIB_IN, ATTRIB_CH, ATTRIB_KL, ATTRIB_FF, ATTRIB_GE, ATTRIB_MU, ATTRIB_KK },
	  { ATTRIB_HA, ATTRIB_RA, ATTRIB_AG, ATTRIB_TA, ATTRIB_JZ, ATTRIB_GG, ATTRIB_NG } },	/* HERO_TYPE_WITCH */
	{ { ATTRIB_KL, ATTRIB_MU, ATTRIB_IN, ATTRIB_FF, ATTRIB_GE, ATTRIB_KK, ATTRIB_CH },
	  { ATTRIB_TA, ATTRIB_AG, ATTRIB_JZ, ATTRIB_GG, ATTRIB_NG, ATTRIB_HA, ATTRIB_RA } },	/* HERO_TYPE_DRUID */
	{ { ATTRIB_KL, ATTRIB_CH, ATTRIB_IN, ATTRIB_KK, ATTRIB_FF, ATTRIB_GE, ATTRIB_MU },
	  { ATTRIB_NG, ATTRIB_JZ, ATTRIB_TA, ATTRIB_AG, ATTRIB_HA, ATTRIB_RA, ATTRIB_GG } },	/* HERO_TYPE_MAGE */
	{ { ATTRIB_CH, ATTRIB_KK, ATTRIB_GE, ATTRIB_IN, ATTRIB_KL, ATTRIB_MU, ATTRIB_FF },
	  { ATTRIB_GG, ATTRIB_JZ, ATTRIB_NG, ATTRIB_AG, ATTRIB_RA, ATTRIB_HA, ATTRIB_TA } },	/* HERO_TYPE_GREEN_ELF */
	{ { ATTRIB_CH, ATTRIB_KK, ATTRIB_GE, ATTRIB_IN, ATTRIB_KL, ATTRIB_MU, ATTRIB_FF },
	  { ATTRIB_GG, ATTRIB_JZ, ATTRIB_NG, ATTRIB_AG, ATTRIB_RA, ATTRIB_HA, ATTRIB_TA } },	/* HERO_TYPE_ICE_ELF */
	{ { ATTRIB_CH, ATTRIB_KK, ATTRIB_GE, ATTRIB_IN, ATTRIB_KL, ATTRIB_MU, ATTRIB_FF },
	  { ATTRIB_GG, ATTRIB_JZ, ATTRIB_NG, ATTRIB_AG, ATTRIB_RA, ATTRIB_HA, ATTRIB_TA } }	/* HERO_TYPE_SYLVAN_ELF */
};
struct incs12 g_lvl_skill_incs = { { 20, 20, 20, 20, 20, 20, 20, 20, 15, 20, 20, 20 } };	/* Talentsteigerungen je Stufe, Typ 1..12 */
struct incs6 g_lvl_spell_incs = { { 25, 25, 40, 20, 20, 20 } };	/* Zaubersteigerungen je Stufe, Typ 7..12 */
struct incs6 g_lvl_incs_swap = { { 5, 5, 5, 0, 0, 0 } };	/* davon umverteilbar (Talente <-> Zauber), Typ 7..12 */
signed short g_lvl_spells_witch[] = { SP_ODEM_ARCANUM, SP_BLITZ, -1 };	/* Zusatzliste Hexe (Ende -1) */
signed short g_lvl_spells_druid[] = {	/* Zusatzliste Druide */
	SP_ODEM_ARCANUM, SP_VERWANDLUNG_BEENDEN, SP_EXPOSAMI, -1
};
signed short g_lvl_spells_school1[] = {	/* Magierakademie 0 (BE MAGIC_SCHOOLS_1) */
	SP_GEISTER_AUSTREIBEN, SP_BALSAM, SP_ARCANO_PSYCHOSTABILIS, SP_IGNIFAXIUS, SP_KLARUM_PURUM, SP_ANALUES,
	SP_FLIM_FLAM, SP_ARMATRUTZ, SP_HEPTAGON, SP_ABVENENUM, -1
};
signed short g_lvl_spells_school2[] = {	/* Magierakademie 1 */
	SP_ODEM_ARCANUM, SP_SENSIBAR, SP_IGNIFAXIUS, SP_BAND_UND_FESSEL, SP_DUPLICATUS, SP_GROSSE_VERWIRRUNG,
	SP_BALSAM, SP_HARMLOSE_GESTALT, SP_FLIM_FLAM, SP_HERR_UEBER_DAS_TIERREICH, SP_EIGENSCHAFTEN, -1
};
signed short g_lvl_spells_school3[] = {	/* Magierakademie 2 */
	SP_NIHILATIO_GRAVITAS, SP_ODEM_ARCANUM, SP_SOLIDIRID, SP_FLIM_FLAM, SP_BALSAM, SP_IGNIFAXIUS, SP_DUNKELHEIT,
	SP_GARDIANUM, SP_ECLIPTIFACTUS, SP_GEISTER_BESCHWOEREN, SP_ANALUES, -1
};
signed short g_lvl_spells_school4[] = {	/* Magierakademie 3 */
	SP_BALSAM, SP_IGNIFAXIUS, SP_FLIM_FLAM, SP_PENETRIZZEL, SP_CLAUDIBUS, SP_SOLIDIRID, SP_ODEM_ARCANUM,
	SP_PLUMBUMBARUM, SP_IN_SEE_UND_FLUSS, SP_VISIBILI, SP_UEBER_EIS, -1
};
signed short g_lvl_spells_school5[] = {	/* Magierakademie 4 */
	SP_ABVENENUM, SP_ODEM_ARCANUM, SP_EIGENSCHAFTEN, SP_ARMATRUTZ, SP_GARDIANUM, SP_BLITZ, SP_TIERE_BESPRECHEN,
	SP_KK_STEIGERN, SP_FLIM_FLAM, SP_EISENROST, -1
};
signed short g_lvl_spells_school6[] = {	/* Magierakademie 5 */
	SP_BALSAM, SP_FLIM_FLAM, SP_RESPONDAMI, SP_IGNIFAXIUS, SP_BANNBALADIN, SP_ARMATRUTZ, SP_ARCANO_PSYCHOSTABILIS,
	SP_GARDIANUM, SP_HORRIPHOBUS, -1
};
signed short g_lvl_spells_school7[] = {	/* Magierakademie 6 */
	SP_GARDIANUM, SP_BALSAM, SP_PARALUE, SP_FLIM_FLAM, SP_ECLIPTIFACTUS, SP_HORRIPHOBUS, SP_DUNKELHEIT,
	SP_SAFT_KRAFT, SP_ODEM_ARCANUM, SP_FUROR_BLUT, SP_ARCANO_PSYCHOSTABILIS, SP_KK_STEIGERN, SP_GE_STEIGERN,
	SP_MU_STEIGERN, -1
};
signed short g_lvl_spells_school8[] = {	/* Magierakademie 7 */
	SP_BALSAM, SP_KLARUM_PURUM, SP_IGNIFAXIUS, SP_FLIM_FLAM, SP_KK_STEIGERN, SP_ARCANO_PSYCHOSTABILIS,
	SP_GE_STEIGERN, SP_ECLIPTIFACTUS, SP_IN_SEE_UND_FLUSS, SP_ODEM_ARCANUM, SP_EIGENSCHAFTEN, -1
};
signed short g_lvl_spells_school9[] = {	/* Magierakademie 8 */
	SP_PARALUE, SP_ARMATRUTZ, SP_GARDIANUM, SP_BALSAM, SP_IGNIFAXIUS, SP_PENETRIZZEL, SP_FORAMEN,
	SP_WEICHES_ERSTARRE, SP_AEOLITUS, SP_STURMGEBRUELL, SP_PLUMBUMBARUM, SP_DESTRUCTIBO, -1
};
signed short far *g_lvl_spells_school[9] = {	/* je hero.magic_school (BE MAGIC_SCHOOLS_INDEX) */
	g_lvl_spells_school1, g_lvl_spells_school2, g_lvl_spells_school3, g_lvl_spells_school4, g_lvl_spells_school5,
	g_lvl_spells_school6, g_lvl_spells_school7, g_lvl_spells_school8, g_lvl_spells_school9
};
signed short g_lvl_spells_green_elf[] = {	/* Zusatzliste Auelf */
	SP_FLIM_FLAM, SP_BLITZ, SP_AXXELERATUS, SP_KLARUM_PURUM, SP_TIERE_BESPRECHEN, SP_DUNKELHEIT, -1
};
signed short g_lvl_spells_sylvan_elf[] = {	/* Zusatzliste Waldelf */
	SP_SILENTIUM, SP_ARMATRUTZ, SP_FULMINICTUS, SP_ADLERAUG, SP_ODEM_ARCANUM, SP_FLIM_FLAM, -1
};
signed short g_lvl_spells_ice_elf[] = {	/* Zusatzliste Firnelf */
	SP_BALSAM, SP_CHAMAELIONI, SP_BLITZ, SP_ADLERAUG, SP_FULMINICTUS, SP_ARMATRUTZ, SP_SILENTIUM, SP_ODEM_ARCANUM,
	SP_BAND_UND_FESSEL, SP_AXXELERATUS, SP_EXPOSAMI, -1
};
/* Phase B: eigenes BSS (ds:c842..c8f4) in Adressreihenfolge */
char g_name_bufs[4][40];
signed short g_sheet_dirty;
struct skill_tpl (far *g_auto_skills)[25];
struct inc_counter far *g_inc_skills;
struct inc_counter far *g_inc_spells;
struct school_spells far *g_school_spells;

void Lvl::redraw()
{
	Inv_g_09bb(this);
	g_sheet_hunger_shown = g_sheet_thirst_shown = -1;
	bars_refresh(g_bars);
}

void Lvl::inc_attrib()
{
	signed short ok = 0;
	signed short i;

	for (i = ATTRIB_MU; i <= ATTRIB_KK; i++)
		if (hero->attrib[i].normal < 20)
			ok = 1;
	if (ok) {
		if (lvl_auto) {
			sel = -1;
			do {
				for (i = 0; i < 7; i++) {
					if (hero->attrib[g_lvl_attrib_order[hero->type - 1].pos[i]].normal < 20 && random_schweif(100) < 75) {
						sel = g_lvl_attrib_order[hero->type - 1].pos[i];
						break;
					}
				}
			} while (sel == -1);
			change_attrib_normal(hero, sel, 1);
			return;
		}
		do {
			sel = dlg_choose(g_dialog, txt[63], 7, g_ttx[0x134 + ATTRIB_MU], g_ttx[0x135], g_ttx[0x136],
				g_ttx[0x137], g_ttx[0x138], g_ttx[0x139], g_ttx[0x13a]) - 1;
			if (hero->attrib[sel].normal >= 20) {
				sel = -2;
				dlg_message(g_dialog, txt[64]);
			}
		} while (sel == -2);
		change_attrib_normal(hero, sel, 1);
		redraw();
	}
}

void Lvl::dec_attrib()
{
	signed short ok = 0;
	signed short i;
	signed short hit;

	for (i = ATTRIB_AG; i <= ATTRIB_JZ; i++)
		if (hero->attrib[i].normal > 2)
			ok = 1;
	if (ok) {
		if (lvl_auto) {
			sel = -1;
			do {
				for (i = 0; i < 7; i++) {
					if (hero->attrib[g_lvl_attrib_order[hero->type - 1].neg[i]].normal > 2 && random_schweif(100) < 75) {
						sel = (signed short)g_lvl_attrib_order[hero->type - 1].neg[i];
						break;
					}
				}
			} while (sel == -1);
			hit = 0;
			for (i = 0; i < 3; i++)
				if (random_schweif(20) <= hero->attrib[sel].normal)
					hit = 1;
			if (hit) {
				change_attrib_normal(hero, sel, -1);
				redraw();
			}
			return;
		}
		do {
			sel = dlg_choose(g_dialog, txt[65], 7, g_ttx[0x134 + ATTRIB_AG], g_ttx[0x13c], g_ttx[0x13d],
				g_ttx[0x13e], g_ttx[0x13f], g_ttx[0x140], g_ttx[0x141]) - 1;
			/* attrib[ATTRIB_AG + sel].normal als Bytezugriff (+0x46 + 3 * 7 = +0x5b) */
			if (((signed char far *)hero)[sel * 3 + 0x5b] <= 2) {
				sel = -2;
				dlg_message(g_dialog, txt[66]);
			}
		} while (sel == -2);
		hit = 0;
		for (i = 0; i < 3; i++)
			if (random_schweif(20) <= ((signed char far *)hero)[sel * 3 + 0x5b])
				hit = 1;
		if (hit) {
			dlg_message(g_dialog, txt[61]);
			change_attrib_normal(hero, sel + ATTRIB_AG, -1);
			redraw();
			return;
		}
		dlg_message(g_dialog, txt[62]);
	}
}

void Lvl::inc_le()
{
	signed short r;

	r = random_schweif(6) + 1;
	if (lvl_auto) {
		if (hero->is_mage()) {
			r += 2;
			sel = r / 2;
			add_le_max(hero, r / 2);
			add_hero_ae2(hero, r - sel);
		} else {
			add_le_max(hero, r);
		}
		redraw();
		return;
	}
	if (hero->is_mage()) {
		r += 2;
		sprintf(g_text_output_buf, txt[67], r);
		do {
			sel = dlg_input(g_dialog, 1, g_text_output_buf, 0, r, 0);
		} while (sel < 0);
		if (sel > r)
			sel = r;
		add_le_max(hero, sel);
		add_hero_ae2(hero, r - sel);
		if (hero->type == HERO_TYPE_MAGE && g_game_mode == GAME_MODE_ADVANCED && dlg_yes_no(g_dialog, txt[68]) == 1) {
			hero->spell_incs += (signed char)-10;
			r = random_interval(3, 8);
			add_hero_ae2(hero, r);
		}
	} else {
		sprintf(g_text_output_buf, txt[69], r);
		dlg_message(g_dialog, g_text_output_buf);
		add_le_max(hero, r);
	}
	redraw();
}

void Lvl::inc_mr()
{
	add_mr_mod(hero, 0);
	if (g_refresh)
		redraw();
}

void Lvl::auto_skills()
{
	signed short done = 0;
	signed short i;
	signed short k;
	signed short v;

	if (!can_inc_skills(hero))
		hero->skill_incs = 0;
	for (i = k = 0; hero->skill_incs != 0 && done == 0; ) {
		sel = g_auto_skills[hero->type][i].skill;
		v = get_skill_value(hero, sel);
		if (v != 18 && g_auto_skills[hero->type][i].max > v)
			inc_skill_auto(sel);
		i++;
		if (i > 24) {
			i = 0;
			k++;
			if (k > 5)
				done = 1;
		}
	}
	while (hero->skill_incs != 0)
		inc_skill_auto(random_schweif(0x33));
}

void Lvl::auto_spells()
{
	signed short i;
	signed short rounds;
	signed short t;

	if (!can_inc_spells(hero))
		hero->spell_incs = 0;
	if (hero->is_mage()) {
		i = 1;
		rounds = 0;
		while (hero->spell_incs != 0 && rounds < 3) {
			rounds++;
			switch (hero->type) {
#if FIX_0161
			/* Fix #161: erste Schleife ab 1, dritte Schleife wie bei den Elfen */
			case HERO_TYPE_WITCH:
				for (i = 1; hero->spell_incs != 0 && i < NR_SPELLS; i++)
					if (g_spell_descriptions[i].herotype == SPELL_DESC_HEROTYPE_WITCH && hero->spells[i] < 11)
						inc_spell_auto(i);
				for (i = 0; hero->spell_incs != 0 && g_lvl_spells_witch[i] != -1; i++)
					inc_spell_auto(g_lvl_spells_witch[i]);
				for (i = 1; hero->spell_incs != 0 && i < NR_SPELLS; i++)
					if (g_spell_descriptions[i].herotype == SPELL_DESC_HEROTYPE_WITCH && hero->spells[i] < 11)
						inc_spell_auto(i);
				break;
#else
			case HERO_TYPE_WITCH:
				for (; hero->spell_incs != 0 && i < NR_SPELLS; i++)
					if (g_spell_descriptions[i].herotype == SPELL_DESC_HEROTYPE_WITCH && hero->spells[i] < 11)
						inc_spell_auto(i);
				for (i = 0; hero->spell_incs != 0 && g_lvl_spells_witch[i] != -1; i++)
					inc_spell_auto(g_lvl_spells_witch[i]);
				break;
#endif
#if FIX_0161
			/* Fix #161: erste Schleife ab 1, dritte Schleife wie bei den Elfen */
			case HERO_TYPE_DRUID:
				for (i = 1; hero->spell_incs != 0 && i < NR_SPELLS; i++)
					if (g_spell_descriptions[i].herotype == SPELL_DESC_HEROTYPE_DRUID && hero->spells[i] < 11)
						inc_spell_auto(i);
				for (i = 0; hero->spell_incs != 0 && g_lvl_spells_druid[i] != -1; i++)
					inc_spell_auto(g_lvl_spells_druid[i]);
				for (i = 1; hero->spell_incs != 0 && i < NR_SPELLS; i++)
					if (g_spell_descriptions[i].herotype == SPELL_DESC_HEROTYPE_DRUID && hero->spells[i] < 11)
						inc_spell_auto(i);
				break;
#else
			case HERO_TYPE_DRUID:
				for (; hero->spell_incs != 0 && i < NR_SPELLS; i++)
					if (g_spell_descriptions[i].herotype == SPELL_DESC_HEROTYPE_DRUID && hero->spells[i] < 11)
						inc_spell_auto(i);
				for (i = 0; hero->spell_incs != 0 && g_lvl_spells_druid[i] != -1; i++)
					inc_spell_auto(g_lvl_spells_druid[i]);
				break;
#endif
			case HERO_TYPE_MAGE:
#if FIX_0161
				/* Fix #161: Schulliste ab list[0] je einmal (list[0] nicht doppelt) */
				for (i = 0; hero->spell_incs != 0 && (t = g_school_spells[hero->magic_school].spells[i]) > 0; i++)
#else
				for (i = 0, t = g_school_spells[hero->magic_school].spells[i]; hero->spell_incs != 0 && t > 0;
					t = g_school_spells[hero->magic_school].spells[i], i++)
#endif
					if (hero->spells[t] < 11)
						inc_spell_auto(t);
				for (i = 0; hero->spell_incs != 0 && g_lvl_spells_school[hero->magic_school][i] != -1; i++)
					if (hero->spells[g_lvl_spells_school[hero->magic_school][i]] < 11)
						inc_spell_auto(g_lvl_spells_school[hero->magic_school][i]);
#if FIX_0161
				/* Fix #161: Schulliste ab list[0] je einmal (list[0] nicht doppelt) */
				for (i = 0; hero->spell_incs != 0 && (t = g_school_spells[hero->magic_school].spells[i]) > 0; i++)
#else
				for (i = 0, t = g_school_spells[hero->magic_school].spells[i]; hero->spell_incs != 0 && t > 0;
					t = g_school_spells[hero->magic_school].spells[i], i++)
#endif
					if (hero->spells[t] < 11)
						inc_spell_auto(t);
				break;
#if FIX_0161
			/* Fix #161: erste Schleife ab 1 (nicht ab dem Endstand der vorigen Schleife) */
			case HERO_TYPE_GREEN_ELF:
				for (i = 1; hero->spell_incs != 0 && i < NR_SPELLS; i++)
#else
			case HERO_TYPE_GREEN_ELF:
				for (; hero->spell_incs != 0 && i < NR_SPELLS; i++)
#endif
					if (g_spell_descriptions[i].herotype == SPELL_DESC_HEROTYPE_GELF && hero->spells[i] < 11)
						inc_spell_auto(i);
				for (i = 0; hero->spell_incs != 0 && g_lvl_spells_green_elf[i] != -1; i++)
					inc_spell_auto(g_lvl_spells_green_elf[i]);
				for (i = 1; hero->spell_incs != 0 && i < NR_SPELLS; i++)
					if (g_spell_descriptions[i].herotype == SPELL_DESC_HEROTYPE_GELF && hero->spells[i] < 11)
						inc_spell_auto(i);
				break;
#if FIX_0161
			/* Fix #161: erste Schleife ab 1 (nicht ab dem Endstand der vorigen Schleife) */
			case HERO_TYPE_ICE_ELF:
				for (i = 1; hero->spell_incs != 0 && i < NR_SPELLS; i++)
#else
			case HERO_TYPE_ICE_ELF:
				for (; hero->spell_incs != 0 && i < NR_SPELLS; i++)
#endif
					if (g_spell_descriptions[i].herotype == SPELL_DESC_HEROTYPE_IELF && hero->spells[i] < 11)
						inc_spell_auto(i);
				for (i = 0; hero->spell_incs != 0 && g_lvl_spells_ice_elf[i] != -1; i++)
					inc_spell_auto(g_lvl_spells_ice_elf[i]);
				for (i = 1; hero->spell_incs != 0 && i < NR_SPELLS; i++)
					if (g_spell_descriptions[i].herotype == SPELL_DESC_HEROTYPE_IELF && hero->spells[i] < 11)
						inc_spell_auto(i);
				break;
#if FIX_0161
			/* Fix #161: erste Schleife ab 1 (nicht ab dem Endstand der vorigen Schleife) */
			case HERO_TYPE_SYLVAN_ELF:
				for (i = 1; hero->spell_incs != 0 && i < NR_SPELLS; i++)
#else
			case HERO_TYPE_SYLVAN_ELF:
				for (; hero->spell_incs != 0 && i < NR_SPELLS; i++)
#endif
					if (g_spell_descriptions[i].herotype == SPELL_DESC_HEROTYPE_SELF && hero->spells[i] < 11)
						inc_spell_auto(i);
				for (i = 0; hero->spell_incs != 0 && g_lvl_spells_sylvan_elf[i] != -1; i++)
					inc_spell_auto(g_lvl_spells_sylvan_elf[i]);
				for (i = 1; hero->spell_incs != 0 && i < NR_SPELLS; i++)
					if (g_spell_descriptions[i].herotype == SPELL_DESC_HEROTYPE_SELF && hero->spells[i] < 11)
						inc_spell_auto(i);
				break;
			}
		}
		while (hero->spell_incs != 0)
			inc_spell_auto(random_schweif(0x55));
	}
}

void Lvl::inc_points()
{
	signed short done;
	signed short n;
	signed short v;
	struct incs12 ta = g_lvl_skill_incs;
	struct incs6 sa = g_lvl_spell_incs;
	struct incs6 xa = g_lvl_incs_swap;

	hero->add_skill_incs(ta.v[hero->type - 1]);
	if (hero->is_mage())
		hero->add_spell_incs(sa.v[hero->type - 7]);
	if (g_game_mode == GAME_MODE_BEGINNER || lvl_auto != 0) {
		auto_skills();
		auto_spells();
		return;
	}
	page = 3;
	redraw();
	if (hero->is_mage() && (v = xa.v[hero->type - 7]) != 0 && dlg_yes_no(g_dialog, txt[71]) == 1) {
		sprintf(g_text_output_buf, txt[72], v);
		n = dlg_input(g_dialog, 1, g_text_output_buf, 0, v, 0);
		if (n > 0) {
			v -= n;
			hero->add_spell_incs(n);
			hero->add_skill_incs(-n);
		} else {
			sprintf(g_text_output_buf, txt[73], v);
			n = dlg_input(g_dialog, 1, g_text_output_buf, 0, v, 0);
			if (n > 0) {
				hero->add_spell_incs(-n);
				hero->add_skill_incs(n);
			}
		}
	}
	done = 0;
	while (hero->skill_incs != 0 && done == 0) {
		sel = lvl_select_skill(hero, 1);
		if (sel >= 0) {
			inc_skill(sel);
		} else if (sel == -2) {
			do {
				sel = dlg_choose(g_dialog, txt[76], 3, txt[74], txt[77], txt[78]) - 1;
				if (sel == 0) {
					if (hero->skill_incs >= 200)
						dlg_message(g_dialog, txt[70]);
					else {
						done = 1;
						break;
					}
				} else if (sel == 1) {
					auto_skills();
					break;
				} else if (sel == 2)
					break;
			} while (sel == -2);
		}
	}
	if (hero->is_mage()) {
		page = 4;
		redraw();
	}
	done = 0;
	while (hero->spell_incs != 0 && done == 0) {
		sel = select_spell(hero, 1);
		if (sel > 0) {
			inc_spell(sel);
		} else if (sel == -2) {
			do {
				sel = dlg_choose(g_dialog, txt[76], 3, txt[74], txt[77], txt[78]) - 1;
				if (sel == 0) {
					if (hero->spell_incs >= 200)
						dlg_message(g_dialog, txt[70]);
					else {
						done = 1;
						break;
					}
				} else if (sel == 1) {
					auto_spells();
					break;
				} else if (sel == 2)
					break;
			} while (sel == -2);
		}
	}
}

void Lvl::run(struct hero far *h)
{
	signed long len;
	signed short i;

	g_dialog_depth++;
	g_inv_warn_enabled = 0;
	g_inc_skills = (struct inc_counter far *)mem_alloc(NR_SKILLS * sizeof(struct inc_counter));
	g_inc_spells = (struct inc_counter far *)mem_alloc(NR_SPELLS * sizeof(struct inc_counter));
	file_open(g_file_main, 0x9c);
	len = file_length2(g_file_main);
	g_auto_skills = (struct skill_tpl (far *)[25])mem_alloc(len);
	file_read(g_file_main, g_auto_skills, len);
	file_close(g_file_main);
	g_school_spells = (struct school_spells far *)mem_alloc(0x129L);
	file_load(g_file_main, 0x1f8, g_school_spells);
	for (i = 0; i < NR_SPELLS; i++)
		g_inc_spells[i].tries = g_inc_spells[i].incs = 0;
	for (i = 0; i < NR_SKILLS; i++)
		g_inc_skills[i].tries = g_inc_skills[i].incs = 0;
	g_action = 0;
	page = 1;
	g_inv_show_body = 0;
	hero = g_sheet_hero = h;
	slot = slot2 = -1;
	hero->level++;
	sprintf(g_text_output_buf, txt[75], hero->name);
	dlg_message(g_dialog, g_text_output_buf);
	if (ap_for_level(hero->level + 1) > hero->ap) {
		sprintf(g_text_output_buf, txt[81], hero->name, hero->ap, hero->level);
		diary_add(g_diary, g_text_output_buf, 0);
	}
	snd_play(g_snd_music, 5);
	redraw();
	lvl_auto = dlg_choose(g_dialog, txt[85], 2, txt[86], txt[87]) - 1;
	if (lvl_auto == -2)
		lvl_auto = 0;
	if (lvl_auto == 1)
		g_status_on = 0;
	dlg_set_pos_x(g_dialog, -100);
	inc_attrib();
	dec_attrib();
	inc_mr();
	inc_le();
	dlg_set_pos_x(g_dialog, 0);
	inc_points();
	if (lvl_auto == 1)
		g_status_on = 1;
	g_refresh = 1;
	snd_obj_stop(g_snd_music);
	g_dialog_depth--;
	g_inv_warn_enabled = 1;
	mem_free(g_inc_skills);
	mem_free(g_inc_spells);
	mem_free(g_auto_skills);
	mem_free(g_school_spells);
}
