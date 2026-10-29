/* OVL087 - Overlay O087 (Segment 0x20ff in SCHWEIF.EXE): Steigern - Methoden der Klasse Lvl (Stufenanstieg O086;
 * Texte +4, Held +0x13). lvl_inc_spell(id) (BE inc_spell_advanced): Steigerungsgrenze 1, 2 fuer Zauber der eigenen
 * Herkunft (spell_desc.herotype nach Heldentyp, Magier auch Akademiebereich g_school_spell_ranges), 3 fuer die
 * Akademieliste g_school_spells[magic_school]; Zaehler g_inc_spells[id] (tries/incs) pruefen (txt[59]/txt[60]),
 * spell_incs vermindern, Wurf random_interval(3,18) bzw. (2,12) gegen den Wert -> txt[61] und +1 bzw. txt[62].
 * lvl_inc_skill(id) (BE inc_skill_advanced): dasselbe fuer Talente (Grenze g_skill_max_incs, Zaehler g_inc_skills,
 * Werte per get_skill_value/add_skill_value, Versuche skill_incs). can_inc_skills/can_inc_spells(h): 1, wenn noch
 * ein Talent (0..51) bzw. Zauber (1..85) unter 18 ist. lvl_inc_skill_auto/lvl_inc_spell_auto(id) (BE inc_*_novice):
 * automatisch in einer Schleife bis Erfolg, Wert 18, Grenze erreicht oder keine Versuche mehr.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL087.CPP   -> 1942/1942 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Parameter "register signed short id" (SI); die lokalen Variablen OHNE register - BCC legt sie selbst nach
 *   DI. Mit "register" an der lokalen Variable bekommt diese SI und id DI (register-Locals zuerst).
 * - "hero->spell_incs += (signed char)-1" (unsigned char) -> add byte es:[..],0ffh; "-= 1" ergibt
 *   mov al / add al,0ffh / mov, "--" ergibt dec. "!hero->spell_incs" -> mov al / mov ah,0 / or ax,ax.
 * - inline hero::inc_spell(i) { spells[i]++; } -> Parameterkopie [bp-4]/[bp-6] vor inc byte.
 * - -Og noetig: Zwischenwert [bp-6]/[bp-8] = id*12 fuer g_spell_descriptions[id] (dreimal im Ausdruck).
 * - add_skill_value mit Wort-Betrag (push 1) ueber gecasteten Funktionszeiger add_skill_value_w.
 * - can_inc_spells ohne Aufrufe -> Schleifenzaehler in DX.
 * Originalfehler: lvl_inc_skill_auto setzt im Erfolgszweig tries = 0 und dann tries++ (statt incs++), die Grenze
 *   g_skill_max_incs greift dort also nie.
 */
#include "schweif.h"

/* add_skill_value mit Wort-Parameter (das Original uebergibt den Betrag hier als short: push 1) */
#define add_skill_value_w ((void (*)(struct hero far *, signed short, signed short))add_skill_value)

void Lvl::inc_spell(register signed short id)
{
	signed short r;
	signed short max_incs;

	max_incs = 1;
	if (hero->type == HERO_TYPE_WITCH && g_spell_descriptions[id].herotype == SPELL_DESC_HEROTYPE_WITCH)
		max_incs = 2;
	else if (hero->type >= HERO_TYPE_GREEN_ELF && (g_spell_descriptions[id].herotype == SPELL_DESC_HEROTYPE_GELF ||
			g_spell_descriptions[id].herotype == SPELL_DESC_HEROTYPE_SELF ||
			g_spell_descriptions[id].herotype == SPELL_DESC_HEROTYPE_IELF))
		max_incs = 2;
	else if (hero->type == HERO_TYPE_DRUID && g_spell_descriptions[id].herotype == SPELL_DESC_HEROTYPE_DRUID)
		max_incs = 2;
	else if (hero->type == HERO_TYPE_MAGE) {
		if (g_spell_descriptions[id].herotype == SPELL_DESC_HEROTYPE_MAGE)
			max_incs = 2;
		if (g_school_spell_ranges[hero->magic_school].lo <= id && g_school_spell_ranges[hero->magic_school].hi >= id)
			max_incs = 2;
		if (is_in_word_array(id, g_school_spells[hero->magic_school].spells) != -1)
			max_incs = 3;
	}
	if (g_inc_spells[id].incs >= max_incs) {
		dlg_message(g_dialog, txt[59]);
		return;
	}
	if (g_inc_spells[id].tries == 3) {
		dlg_message(g_dialog, txt[60]);
		return;
	}
	hero->spell_incs += (signed char)-1;
	if (hero->spells[id] >= 11)
		r = random_interval(3, 18);
	else
		r = random_interval(2, 12);
	if (hero->spells[id] < r) {
		dlg_message(g_dialog, txt[61]);
		hero->inc_spell(id);
		g_inc_spells[id].tries = 0;
		g_inc_spells[id].incs++;
		return;
	}
	dlg_message(g_dialog, txt[62]);
	g_inc_spells[id].tries++;
}

void Lvl::inc_skill(register signed short id)
{
	signed short max_incs;
	signed short r;

	max_incs = g_skill_max_incs[id].max_incs;
	if (g_inc_skills[id].incs >= max_incs) {
		dlg_message(g_dialog, txt[59]);
		return;
	}
	if (g_inc_skills[id].tries == 3) {
		dlg_message(g_dialog, txt[60]);
		return;
	}
	hero->skill_incs += (signed char)-1;
	if (get_skill_value(hero, id) >= 11)
		r = random_interval(3, 18);
	else
		r = random_interval(2, 12);
	if (get_skill_value(hero, id) < r) {
		dlg_message(g_dialog, txt[61]);
		add_skill_value_w(hero, id, 1);
		g_inc_skills[id].tries = 0;
		g_inc_skills[id].incs++;
		return;
	}
	dlg_message(g_dialog, txt[62]);
	g_inc_skills[id].tries++;
}

signed short can_inc_skills(struct hero far *h)
{
	register signed short i;

	for (i = 0; i < NR_SKILLS; i++)
		if (get_skill_value(h, i) < 18)
			return 1;
	return 0;
}

signed short can_inc_spells(struct hero far *h)
{
	signed short i;

	for (i = 1; i < NR_SPELLS; i++)
		if (h->spells[i] < 18)
			return 1;
	return 0;
}

void Lvl::inc_skill_auto(register signed short id)
{
	signed short r;
	signed short max_incs;
	signed short done;

	done = 0;
	max_incs = g_skill_max_incs[id].max_incs;
	if (!can_inc_skills(hero)) {
		hero->skill_incs = 0;
		done = 1;
	}
	while (!done) {
		if (g_inc_skills[id].tries == 3 || get_skill_value(hero, id) == 18 || g_inc_skills[id].incs >= max_incs)
			done = 1;
		else {
			hero->skill_incs += (signed char)-1;
			if (!hero->skill_incs)
				done = 1;
			if (get_skill_value(hero, id) >= 11)
				r = random_interval(3, 18);
			else
				r = random_interval(2, 12);
			if (get_skill_value(hero, id) < r) {
				add_skill_value_w(hero, id, 1);
#if FIX_0002
				/* Fix #2: Steigerungen zaehlen (incs), nicht tries */
				g_inc_skills[id].tries = 0;
				g_inc_skills[id].incs++;
#else
				g_inc_skills[id].tries = 0;
				g_inc_skills[id].tries++;
#endif
				done = 1;
			} else
				g_inc_skills[id].tries++;
		}
	}
}

void Lvl::inc_spell_auto(register signed short id)
{
	signed short r;
	signed short max_incs;
	signed short done;

	done = 0;
	max_incs = 1;
	if (hero->type == HERO_TYPE_WITCH && g_spell_descriptions[id].herotype == SPELL_DESC_HEROTYPE_WITCH)
		max_incs = 2;
	else if (hero->type >= HERO_TYPE_GREEN_ELF && (g_spell_descriptions[id].herotype == SPELL_DESC_HEROTYPE_GELF ||
			g_spell_descriptions[id].herotype == SPELL_DESC_HEROTYPE_SELF ||
			g_spell_descriptions[id].herotype == SPELL_DESC_HEROTYPE_IELF))
		max_incs = 2;
	else if (hero->type == HERO_TYPE_DRUID && g_spell_descriptions[id].herotype == SPELL_DESC_HEROTYPE_DRUID)
		max_incs = 2;
	else if (hero->type == HERO_TYPE_MAGE) {
		if (g_spell_descriptions[id].herotype == SPELL_DESC_HEROTYPE_MAGE)
			max_incs = 2;
		if (g_school_spell_ranges[hero->magic_school].lo <= id && g_school_spell_ranges[hero->magic_school].hi >= id)
			max_incs = 2;
		if (is_in_word_array(id, g_school_spells[hero->magic_school].spells) != -1)
			max_incs = 3;
	}
	if (!can_inc_spells(hero)) {
		hero->spell_incs = 0;
		done = 1;
	}
	while (!done) {
		if (g_inc_spells[id].tries == 3 || hero->spells[id] == 18 || g_inc_spells[id].incs >= max_incs)
			done = 1;
		else {
			hero->spell_incs += (signed char)-1;
			if (!hero->spell_incs)
				done = 1;
			if (hero->spells[id] >= 11)
				r = random_interval(3, 18);
			else
				r = random_interval(2, 12);
			if (hero->spells[id] < r) {
				hero->inc_spell(id);
				g_inc_spells[id].tries = 0;
				g_inc_spells[id].incs++;
				done = 1;
			} else
				g_inc_spells[id].tries++;
		}
	}
}
