/* OVL074 - Overlay O074 (Segment 0x20ad in SCHWEIF.EXE): Heldenfunktionen. money_to_string(buf, v): Geldbetrag
 * (Heller) als Dukaten/Silber/Heller (unsigned long / 100, % 10, Format g_ttx[20]). select_from_list(list, n, mode):
 * Auswahl aus n Eintraegen zu 0x24 Bytes, je 10 pro Seite (g_menu_texts[12], "weiter" g_ttx[22]).
 * get_free_hero_slot: erster freier Platz in g_heroes (type == 0). count_heroes_check2/count_heroes: Helden
 * zaehlen (check_hero2 bzw. type != 0). check_heroes_down: alle kampfunfaehig -> g_game_state = 1, aktive Gruppe
 * kampfunfaehig -> group_switch_next(0). find_hero_with_item(id, grp): Held der Gruppe mit Gegenstand.
 * sub_ae_max/add_hero_ae2: AE-Grundwert und AE senken/erhoehen (nur Zauberkundige), add_hero_ap/sub_hero_ap: AP
 * +/- (long), sub_le_max/add_le_max: LE-Grundwert, g_refresh bei Bild 14/15. hero_get_poison/hero_get_ill: Gift
 * bzw. Krankheit setzen (poison[] +0xe8 / illness[] +0xc0, 5-Byte-Eintraege), hero_disease_test: Krankheit mit
 * Wahrscheinlichkeit. hero_get_drunken/hero_get_sober: Zustand drunk setzen/loeschen (14 Eigenschaften +-1 ueber
 * change_attrib). hero_sleep(h, mod): Regeneration LE/AE mit Meldungen (g_ttx[388/389/31/32/33]).
 * bargain: Feilsch-Probe (TA_FEILSCHEN). hero_has_ingredients/hero_use_ingredients/do_alchemy:
 * Alchimie - Zutaten pruefen/verbrauchen (g_recipes, 0x1c Bytes), Brauen (Probe TA_ALCHIMIE, Ergebnis outcome,
 * Sonderfall 0x153). hero_clear_renegade (Bit renegade, Gruppe g_group_no), hero_cure_disease/hero_cure_poison:
 * erste Krankheit bzw. erstes Gift heilen. hero_restore_le_mod: LE-Bonus le_mod. hero_brewing_in_inn: Bit
 * brewing und alchemy_inn_id == v. hero_dismiss: Held entlassen (money_share = Tempel, hero_save_file, memset
 * 0x5ef).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL074.CPP -> 5157/5157 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (-Oi fuer memset; ohne -Og 1923, ohne -Oi 92 Abweichungen; -Ov/-Oe/-rd falsch).
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * - C++: inline hero::is_mage() { return type >= 7; } (mov ax,1 / xor ax,ax / or ax,ax), Setter set_ill_.., 
 *   clr_ill_.., set_poi_.., clr_poi_.. mit je eigener Parameterkopie [bp-2]..[bp-0ch] (i kommt aus CX; konstantes v ohne
 *   Kopie), raise_ap(signed long) -> movsx in die Kopie [bp-4]. abs() inline mit Kopie [bp-6]/[bp-8].
 *   Bitfelder unsigned short (frueher hier unsigned char b:1): "!(signed char)h->dead" -> and ax,1 / cbw /
 *   or ax,ax; "(signed char)h->unconscious" -> and ax,1 / or al,al.
 * - "ap = ap + v" im inline (mov eax,mem / add eax,[bp-4] / mov mem,eax); "ap += v" ergibt add mem,eax.
 * - "if ((h->ae += v) > h->ae_max)" eingebettet (kein zweites les); "if ((h->ae_max -= v) < 0)" (Flags der sub).
 * - hero_sleep: "ae = ae / 3" (mov ax,si / mov bx,3); "ae /= 3" laedt bx zuerst. "ae += ae" (add si,si).
 *   (int)h->rest_bonus als -Og-Zwischenwert [bp-0ah]. if (hero_is_diseased || hero_is_poisoned) als
 *   "if (!a && !b) {..} else {..}", damit der Meldungszweig hinter der Epilogkopie liegt.
 * - bargain: "signed char mod = mod_init" (Kopie [bp-1]), verschachtelte ?: in al, "mod = a / 5 + mod + 1".
 * - do_alchemy steht unten -> far call aus hero_sleep; hero_use_ingredients oben -> near.
 * Auffaelliges: hero_sleep setzt bei negativer Regeneration und Ruhe Koerper die LE-Regeneration auf +2
 *   bzw. AE auf +1; do_alchemy gibt beim Misslingen die erste Zutat ingredients[0] zurueck.
 */
#include <stdio.h>
#include <stdlib.h>
#include <mem.h>
#include "schweif.h"

void money_to_string(char far *buf, unsigned long money)
{
	unsigned long ducats;
	signed short silver;

	ducats = money / 100;
	money %= 100;
	silver = money / 10;
	money %= 10;
	sprintf(buf, g_ttx[20], ducats, silver, (signed short)money);
}

/* Auswahl aus n Eintraegen zu 0x24 Bytes ab base, je 10 pro Seite ("weiter" g_ttx[22]); -1 = Abbruch */
signed short select_from_list(char far *base, signed short n, signed short mode)
{
	signed short top;			/* erster Eintrag der Seite */
	signed short cnt;
	signed short answer;
	register signed short i;

	answer = 0;
	top = 0;
	while (answer != -1) {
		cnt = top + 10 < n ? 10 : n - top;
		for (i = 0; i < cnt; i++)
			g_menu_texts[i] = base + (i + top) * 0x24;
		i = cnt;
		if (n > 10)
			g_menu_texts[i++] = g_ttx[22];
		answer = dlg_choose(g_dialog, mode == -1 ? g_ttx[23] : g_ttx[24], i,
			g_menu_texts[0], g_menu_texts[1], g_menu_texts[2], g_menu_texts[3],
			g_menu_texts[4], g_menu_texts[5], g_menu_texts[6], g_menu_texts[7],
			g_menu_texts[8], g_menu_texts[9], g_menu_texts[10], g_menu_texts[11]);
		if (n > 10 && answer == i) {
			top += 10;
			if (top >= n)
				top = 0;
		} else if (answer != -1)
			return top + answer - 1;
	}
	return -1;
}

signed short get_free_hero_slot(void)
{
	struct hero far *h;
	register signed short i;

	for (i = 0, h = g_heroes; i <= 6; i++, h++) {
		if (!h->type)
			return i;
	}
	return -1;
}

signed short count_heroes_check2(void)
{
	struct hero far *h;
	register signed short i;
	register signed short n;

	for (i = 0, n = 0, h = g_heroes; i <= 6; i++, h++) {
		if (check_hero2(h))
			n++;
	}
	return n;
}

/* alle Helden kampfunfaehig -> g_game_state = 1; aktive Gruppe kampfunfaehig -> zur naechsten Gruppe */
void check_heroes_down(void)
{
	struct hero far *npc;

	npc = g_heroes + 6;
	g_hero_died = 0;
	if (!count_heroes_check2() || count_heroes_check2() == 1 && check_hero(npc)) {
		g_game_state = 1;
		return;
	}
	if (!group_count_check2b(g_group) || group_count_check2b(g_group) == 1 && group_has_hero(g_group, npc) &&
	    check_hero(npc))
		group_switch_next(0);
}

signed short find_hero_with_item(register signed short item_id, signed char group_no)
{
	struct hero far *h;
	register signed short i;

	for (i = 0, h = g_heroes; i <= 6; i++, h++) {
		if (h->type && h->group_no == group_no && get_item_pos(h, item_id) >= 0)
			return i;
	}
	return -1;
}

void sub_ae_max(struct hero far *h, signed short v)
{
	if (!(signed char)h->dead && v > 0 && h->is_mage()) {
		if ((h->ae_max -= v) < 0)
			h->ae_max = 0;
		if ((h->ae -= v) < 0)
			h->ae = 0;
	}
}

void add_hero_ae2(struct hero far *h, signed short v)
{
	if (!(signed char)h->dead && v > 0 && h->is_mage()) {
		h->ae_max += v;
		if ((h->ae += v) > h->ae_max)
			h->ae = h->ae_max;
	}
}

signed long add_hero_ap(struct hero far *h, signed short v)
{
	if (!(signed char)h->dead)
		h->raise_ap(v);
	return h->ap;
}

signed long sub_hero_ap(struct hero far *h, signed short v)
{
	if (!(signed char)h->dead) {
		if ((h->ap -= v) < 0)
			h->ap = 0;
	}
	return h->ap;
}

void sub_le_max(struct hero far *h, signed short v)
{
	if (!(signed char)h->dead && v > 0) {
		if ((h->le_max -= v) <= 0)
			h->le_max = 1;
		if (h->le > h->le_max)
			h->le = h->le_max;
	}
	if (g_pp20_index == 14 || g_pp20_index == 15)
		g_refresh = 1;
}

void add_le_max(struct hero far *h, signed short v)
{
	if (!(signed char)h->dead && v > 0) {
		h->le_max += v;
		if ((h->le += v) > h->le_max)
			h->le = h->le_max;
		if (h->le >= 5 && (signed char)h->unconscious)
			set_unconscious(h, 0);
	}
	if (g_pp20_index == 14 || g_pp20_index == 15)
		g_refresh = 1;
}

/* Gift poi setzen (Bannstaub nur bei Zauberkundigen) */
void hero_get_poison(struct hero far *h, signed short poi)
{
	if (!(signed char)h->dead && !h->poison[poi].state && (poi != POISON_BANNSTAUB || h->is_mage())) {
		h->set_poi_state(poi, ILL_STATE_SICK);
		h->set_poi_days(poi, 0);
		h->set_poi_kk(poi, 0);
		h->set_poi_ge(poi, 0);
		h->clr_poi_effect(poi);
		h->clr_poi_diagnosed(poi);
	}
}

/* Krankheit ill setzen (Dumpfschaedel nicht bei Blauer Keuche) */
void hero_get_ill(struct hero far *h, signed short ill)
{
	if (!(signed char)h->dead && !h->illness[ill].state &&
	    (ill != ILLNESS_DUMPFSCHAEDEL || !h->illness[ILLNESS_BLAUE_KEUCHE].state)) {
		h->set_ill_state(ill, ILL_STATE_SICK);
		h->set_ill_days(ill, 0);
		h->set_ill_kk(ill, 0);
		h->set_ill_ge(ill, 0);
		h->clr_ill_effect(ill);
		h->clr_ill_diagnosed(ill);
	}
}

void hero_disease_test(struct hero far *h, signed short ill, signed short chance)
{
	if (random_schweif(100) <= chance && h->illness[ill].state != ILL_STATE_SICK)
		hero_get_ill(h, ill);
}

/* betrunken: MU/GE.. +-1 je Eigenschaft (g_quiet), Klang 0x5e */
void hero_get_drunken(struct hero far *h)
{
	if (!h->drunk) {
		h->drunk = 1;
		g_quiet = 1;
		change_attrib(h, ATTRIB_MU, 1);
		change_attrib(h, ATTRIB_KL, -1);
		change_attrib(h, ATTRIB_CH, -1);
		change_attrib(h, ATTRIB_FF, -1);
		change_attrib(h, ATTRIB_GE, -1);
		change_attrib(h, ATTRIB_IN, 1);
		change_attrib(h, ATTRIB_KK, 1);
		change_attrib(h, ATTRIB_AG, 1);
		change_attrib(h, ATTRIB_HA, -1);
		change_attrib(h, ATTRIB_RA, -1);
		change_attrib(h, ATTRIB_GG, 1);
		change_attrib(h, ATTRIB_TA, -1);
		change_attrib(h, ATTRIB_NG, 1);
		change_attrib(h, ATTRIB_JZ, 1);
		snd_play(g_snd_fx, 0x5e);
		g_quiet = 0;
	}
}

void hero_get_sober(struct hero far *h)
{
	if (h->drunk) {
		h->drunk = 0;
		g_quiet = 1;
		change_attrib(h, ATTRIB_MU, -1);
		change_attrib(h, ATTRIB_KL, 1);
		change_attrib(h, ATTRIB_CH, 1);
		change_attrib(h, ATTRIB_FF, 1);
		change_attrib(h, ATTRIB_GE, 1);
		change_attrib(h, ATTRIB_IN, -1);
		change_attrib(h, ATTRIB_KK, -1);
		change_attrib(h, ATTRIB_AG, -1);
		change_attrib(h, ATTRIB_HA, 1);
		change_attrib(h, ATTRIB_RA, 1);
		change_attrib(h, ATTRIB_GG, -1);
		change_attrib(h, ATTRIB_TA, 1);
		change_attrib(h, ATTRIB_NG, -1);
		change_attrib(h, ATTRIB_JZ, -1);
		g_quiet = 0;
	}
}

/* Regeneration LE/AE (W6 + mod, Ruhe Koerper, Bonus rest_bonus) mit Meldungen; braut der Held, stattdessen
 * do_alchemy; krank/vergiftet -> nur Meldung g_ttx[33] */
void hero_sleep(struct hero far *h, signed short mod)
{
	signed short diff;			/* fehlende LE bzw. AE */
	signed short tmp;
	register signed short le;
	register signed short ae;

	if (!(signed char)h->dead && h->staffspell_timer == 0 && !h->recipe_timer) {
		if (g_on_boat && random_schweif(100) < 10)
			return;
		if (!hero_is_diseased(h) && !hero_is_poisoned(h)) {
			if (!(signed char)h->brewing) {
				le = random_schweif(6) + mod;
				ae = random_schweif(6) + mod;
				if (le < 0)
					le = 0;
				if (ae < 0)
					ae = 0;
				if (h->is_mage() && le < ae) {
					tmp = ae;
					ae = le;
					le = tmp;
				}
				if (h->ruhe_koerper > 0) {
					le += random_schweif(10);
					ae += ae;
				}
				le += h->rest_bonus;
				ae += h->rest_bonus;
				h->rest_bonus = 0;
				if (le < 0) {
					le /= 2;
					if (h->ruhe_koerper)
						le = 2;
				}
				if (ae < 0) {
					ae = ae / 3;
					if (h->ruhe_koerper)
						ae = 1;
				}
				if (h->ruhe_koerper > 0)
					h->ruhe_koerper--;
				diff = h->le_max - h->le;
				*g_text_output_buf = 0;
				if (le < 0 && check_hero3(h)) {
					sub_hero_le(h, -le);
					sprintf(g_text_output_buf, g_ttx[388], h->name, -le, le < -1 ? g_ttx[29] : g_ttx[28]);
				} else if (diff && le) {
					if (diff < le)
						le = diff;
					add_hero_le(h, le);
					sprintf(g_text_output_buf, g_ttx[31], h->name, abs(le), le > 1 ? g_ttx[29] : g_ttx[28]);
				}
				if (*g_text_output_buf) {
					if (g_pp20_index == 4) {
						screen_textbar(g_screen, g_text_output_buf, 0xff);
						wait_ticks_key(200);
					} else if (g_on_boat)
						dlg_message(g_dialog, g_text_output_buf);
				}
				*g_text_output_buf = 0;
				if (h->is_mage()) {
					diff = h->ae_max - h->ae;
					if (ae < 0) {
						sub_ae_splash(h, -ae);
						sprintf(g_text_output_buf, g_ttx[389], hero_name_gen(h, 1), abs(ae),
							ae < -1 ? g_ttx[29] : g_ttx[28]);
					} else if (diff && ae) {
						if (diff < ae)
							ae = diff;
						add_hero_ae(h, ae);
						sprintf(g_text_output_buf, g_ttx[32], hero_name_gen(h, 1), ae, ae > 1 ? g_ttx[29] : g_ttx[28]);
					}
					if (*g_text_output_buf) {
						if (g_pp20_index == 4) {
							screen_textbar(g_screen, g_text_output_buf, 0xff);
							wait_ticks_key(200);
						} else if (g_on_boat)
							dlg_message(g_dialog, g_text_output_buf);
					}
				}
			} else
				do_alchemy(h, h->recipe_id, 0);
		} else {
			sprintf(g_text_output_buf, g_ttx[33], h->name);
			dlg_message(g_dialog, g_text_output_buf);
		}
	}
}

/* Feilschen: Probe TA_FEILSCHEN mit Zuschlag nach Anzahl n und Preis */
signed short bargain(struct hero far *h, signed short n, signed long price, signed short percent,
	signed char mod_init)
{
	signed char mod = mod_init;

	mod += n == 1 ? 2 : (n == 2 ? 1 : (n < 5 ? 0 : (n < 9 ? -1 : -2)));
	mod += price < 100 ? 2 : (price < 500 ? 1 : (price < 1000 ? 0 : (price < 2000 ? -1 : -2)));
	mod = percent / 5 + mod + 1;
	return test_skill(h, TA_FEILSCHEN, mod);
}

/* Zutaten von Rezept nr pruefen (nehmen und zurueckgeben); liefert die fehlende Zutat oder 0 */
signed short hero_has_ingredients(struct hero far *h, signed short nr)
{
	struct recipe far *r;
	signed short ok;
	signed short missing;
	register signed short i;
	register signed short slot;

	i = 0;
	ok = 1;
	missing = 0;
	r = &g_recipes[nr];
	while (r->ingredients[i] != -1 && ok) {
		slot = get_item_pos(h, r->ingredients[i]);
		if (slot == -1) {
			ok = 0;
			missing = r->ingredients[i];
		} else
			drop_item(h, slot, 1);
		i++;
	}
	i--;
	if (!ok)
		i--;
	while (i >= 0)
		give_hero_new_item(h, r->ingredients[i--], 1, 1);
	return missing;
}

/* Zutaten von Rezept nr verbrauchen; Flaschen (0x1f bzw. 0x2a) der Zutaten 0x5b/0x5c bzw. 0x29 zurueck */
void hero_use_ingredients(struct hero far *h, signed short nr)
{
	struct recipe far *r;
	signed short slot;
	register signed short i;
	register signed short flask;

	i = 0;
	flask = 0;
	r = &g_recipes[nr];
	while (r->ingredients[i] != -1) {
		slot = get_item_pos(h, r->ingredients[i]);
		drop_item(h, slot, 1);
		if (r->ingredients[i] == 0x1f || r->ingredients[i] == 0x2a)
			flask = 1;
		if (r->ingredients[i] == 0x5b || r->ingredients[i] == 0x5c) {
			if (flask)
				give_hero_new_item(h, 0x1f, 1, 1);
			else
				flask = 1;
		}
		if (r->ingredients[i] == 0x29) {
			if (flask)
				give_hero_new_item(h, 0x2a, 1, 1);
			else
				flask = 1;
		}
		i++;
	}
}

/* Rezept nr brauen: Zutaten verbrauchen, AE abziehen, Probe TA_ALCHIMIE; Ergebnis 0x153 betrifft den Helden
 * mit dem groessten Wert ATTRIB_GG (group_set_slot) */
signed short do_alchemy(struct hero far *h, register signed short nr, signed short no_result)
{
	struct recipe far *r;
	struct hero far *h2;

	r = &g_recipes[nr];
	hero_use_ingredients(h, nr);
	sub_ae_splash(h, r->ae);
	h->brewing = 0;
	h->recipe_timer = 0;
	h->recipe_id = 0;
	h->alchemy_inn_id = 0;
	if (test_skill(h, TA_ALCHIMIE, r->difficulty) > 0 && !no_result) {
		if (r->outcome == 0x153) {
			h2 = get_hero_attrib_max(g_group, ATTRIB_GG);
#if FIX_0363
			/* Fix #363: den letzten Helden der Gruppe nicht entfernen (leere Gruppe haengt das Spiel auf) */
			if ((signed char)g_group->nr_heroes - (group_get_npc(g_group) != 0) > 1) {
				sprintf(g_text_output_buf, g_ttx[441], h->name, h2->name, grammar_pronoun(h2->sex, 3, 0));
				group_set_slot(g_group, group_hero_pos(g_group, h2), -1);
			} else
				*g_text_output_buf = 0;
#else
			sprintf(g_text_output_buf, g_ttx[441], h->name, h2->name, grammar_pronoun(h2->sex, 3, 0));
			group_set_slot(g_group, group_hero_pos(g_group, h2), -1);
#endif
		} else {
			give_hero_new_item(h, r->outcome, 1, 1);
			sprintf(g_text_output_buf, g_ttx[34], h->name, grammar_article(2, r->outcome, 0));
		}
		dlg_message(g_dialog, g_text_output_buf);
		return 1;
	} else {
		give_hero_new_item(h, r->ingredients[0], 1, 1);
		sprintf(g_text_output_buf, g_ttx[35], h->name, grammar_article(2, r->outcome, 0));
		dlg_message(g_dialog, g_text_output_buf);
		return 0;
	}
}

signed short hero_clear_renegade(struct hero far *h)
{
	if ((signed char)h->renegade && h->group_no == g_group_no) {
		h->renegade = 0;
		return 1;
	}
	return 0;
}

/* erste Krankheit heilen (Zustand genesend, Tage 0), nur in der aktiven Gruppe */
signed short hero_cure_disease(struct hero far *h)
{
	register signed short ill;

	ill = hero_is_diseased(h);
	if (ill && h->group_no == g_group_no) {
		h->set_ill_state(ill, ILL_STATE_RECOVER);
		h->set_ill_days(ill, 0);
		return 1;
	}
	return 0;
}

/* erstes Gift heilen (Zustand Erholung, Dauer 0) */
signed short hero_cure_poison(struct hero far *h)
{
	register signed short poi;

	poi = hero_is_poisoned(h);
	if (poi) {
		h->set_poi_days(poi, 0);
		h->set_poi_state(poi, ILL_STATE_RECOVER);
		return 1;
	}
	return 0;
}

void hero_restore_le_mod(struct hero far *h)
{
	h->le_max += h->le_mod;
	h->le_mod = 0;
}

signed short hero_brewing_in_inn(struct hero far *h, signed short inn)
{
	return (signed char)h->brewing && h->alchemy_inn_id == inn ? 1 : 0;
}

/* Held entlassen (bleibt im Tempel temple_id), Heldenstruktur loeschen */
void hero_dismiss(struct hero far *h, signed char temple_id)
{
	h->set_money_share(temple_id);
	h->group_pos = 0;
	hero_save_file(h);
	memset(h, 0, sizeof(struct hero));
}

signed short count_heroes(void)
{
	struct hero far *h;
	register signed short i;
	register signed short n;

	for (i = 0, n = 0, h = g_heroes; i <= 6; i++, h++) {
		if (h->type)
			n++;
	}
	return n;
}
