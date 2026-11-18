/* OVL006 - Overlay O006 (Segment 0x1f18 in SCHWEIF.EXE): Talente. get_skill_value(h, t): Talentwert skills[t],
 * Schleichen mit Gegenstand 0x154 um 1 niedriger. set_skill_value/add_skill_value(h, t, v): Talent setzen/erhoehen,
 * bei Waffentalenten (t <= TA_ZWEIHAENDER) Haelften auf at[]/pa[] verteilen (ungerade: +-1 auf at), g_refresh bei
 * Bild 14. use_skill(h, mod, arg): Talent anwenden (BE use_skill) - Auswahl select_skill, Texte h->f5e7/h->txt
 * (txt_load_list(g_font, 0x1f, 0x12c)), je Talent: Heilen Gift skill_cure_poison, Heilen Krankheiten
 * skill_heal_disease, Heilen Wunden skill_heal_wounds, Akrobatik skill_acrobatics, Musizieren skill_music, Tanzen
 * skill_dance, Falschspiel skill_cheat, Taschendiebstahl skill_pickpocket (erwischt in Schenke/Laden ->
 * g_tavern_banned/g_shop_banned[g_loc_id], tavern_leave/shop_leave), Alchimie plan_alchemy, Schaetzen
 * skill_appraise. Geldtalente: Probe test_skill, Geld random_interval ueber money_to_string/group_add_money,
 * Sperrzeit 8 h in g_music_timer/g_acrobatics_timer/g_dance_timer. skill_music: Instrument (0x25/0x5f/0x2e beim
 * Helden, sonst Gruppe), an Ort 5/6 (g_music_id_req & 0x3fff) Klang 0xaf..0xb4 abspielen. lvl_select_skill(h, show):
 * Talentgruppe (g_skills_index) und Talent waehlen, mit show Werte als "%s %d" (g_select_skill_lvlup) in
 * g_text_output_buf + i*50, Liste g_menu_texts[16]. select_skill: Talentauswahl je Ort (Kopie von
 * g_select_skill_defaults, 9 Bytes, erste 3 fest). skill_heal_wounds/skill_heal_disease: Patient per get_any_hero.
 * Ortsart = g_group->loctype (+0x1e; im Header "Blickrichtung", hier 2 Schenke, 4 Laden, 3/8/9/10 Alchimie).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL006.CPP   -> 5060/5060 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (ds:0117/0118 = zwei benachbarte Leerstrings). Ohne -Og 917, mit -Oi 452 Abweichungen (strcpy).
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * - C++ wegen inline hero::is_healable() { return dead || petrified ? 0 : 1; } (skill_heal_wounds: xor ax,ax /
 *   mov ax,1 / or ax,ax); -Og cacht davor das Bitbyte [bp-5] und den Bitwert petrified [bp-8] aus dem vorigen
 *   if; le/missing/dmg im inneren Block.
 * - set_skill_value: -Og haelt &h->skills[t] in DI und laedt das Segment per mov es,[bp+8] nach; "adj = half =
 *   d / 2" (Kette) -> mov [bp-3],al / mov cl,al; (int)diff als Zwischenwert [bp-6].
 * - select_skill: "signed short n = 3" VOR der 9-Byte-Strukturkopie deklarieren (mov si,3 zuerst); Rueckgabe
 *   "sel != -1 ? l.skill[sel - 1] : sel" (lea ax,[bp-0dh]).
 * - Aufrufe von skill_cure_poison/skill_appraise/skill_cure_disease ueber Funktionszeiger-Casts (Zuschlag als
 *   short, Prototyp signed char).
 * - lvl_select_skill: 16 far-Zeiger g_menu_texts[0..15] einzeln an dlg_choose (Wortpushes von b6ff abwaerts).
 * Originalfehler/Auffaelliges: skill_music - case -1 (Klang 7) ist unerreichbar, "snd != 0 && snd >= 1" doppelt.
 *   skill_dance prueft Musizieren statt Tanzen. skill_heal_wounds wuerfelt die Heilprobe test_skill(h,
 *   TA_HEILEN_WUNDEN, mod - 5) zweimal (erst ob, dann ob Heilung oder Schaden), is_healable testet petrified
 *   erneut; beim Misslingen kann dmg = LE - 1 = 0 werden (sub_hero_le(patient, 0)); dann wird +0xa1
 *   (im Header staffspell_timer - Bedeutung pruefen) auf 36 h gesetzt.
 */
#include <stdio.h>
#include <string.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:0108..011a, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
struct str6 g_select_skill_lvlup = { "%s~%d" };
struct skill_list9 g_select_skill_defaults = { { 44, 45, 46, -1, -1, -1, -1, -1, -1 } };

/* Aufrufersicht von O006: der Zuschlag geht als short (cbw / push word) an die O090-Funktionen, deren
 * Definitionen (und Prototypen) signed char nehmen -> Funktionszeiger-Cast im Aufruf (gleiche Bytes) */
typedef signed short (far *SKILL_FN_S)(struct hero far *, signed short);
typedef signed short (far *SKILL_FN_SS)(struct hero far *, signed short, signed short);
typedef signed short (far *CURE_FN_SS)(struct hero far *, struct hero far *, signed short, signed short);

signed short skill_pickpocket(struct hero far *h, signed short mod);
signed short skill_cheat(struct hero far *h, signed short mod);
signed short skill_music(struct hero far *h, signed short mod);
signed short skill_acrobatics(struct hero far *h, signed short mod);
signed short skill_dance(struct hero far *h, signed short mod);
signed short select_skill(struct hero far *h);
signed short skill_heal_wounds(struct hero far *h, signed short mod);
signed short skill_heal_disease(struct hero far *h, signed short mod);

signed char get_skill_value(struct hero far *h, signed short skill)
{
	if (skill == TA_SCHLEICHEN && get_item_pos(h, 0x154) != -1)
		return h->skills[skill] - 1;
	else
		return h->skills[skill];
}

/* Talent setzen; Waffentalente: Differenz halb auf AT, halb auf PA (ungerade: Rest auf AT) */
void set_skill_value(struct hero far *h, signed short skill, signed char val)
{
	signed char old, diff, half, adj;

	old = h->skills[skill];
	h->skills[skill] = val;
	diff = val - old;
	if (skill <= TA_ZWEIHAENDER && diff != 0) {
		adj = half = diff / 2;
		if (diff % 2) {
			if (diff < 0)
				adj--;
			else
				adj++;
		}
		h->at[skill] += adj;
		h->pa[skill] += half;
		if (g_pp20_index == 14)
			g_refresh = 1;
	}
}

void add_skill_value(struct hero far *h, signed short skill, signed char val)
{
	signed char half, adj;

	h->skills[skill] += val;
	if (skill <= TA_ZWEIHAENDER) {
		adj = half = val / 2;
		if (val % 2) {
			if (val < 0)
				adj--;
			else
				adj++;
		}
		h->at[skill] += adj;
		h->pa[skill] += half;
		if (g_pp20_index == 14)
			g_refresh = 1;
	}
}

/* Talent anwenden: Auswahl je Ort, Texte 0x1f/0x12c nach h->f5e7/h->txt; Taschendiebstahl/Falschspiel
 * erwischt -> in Schenke bzw. Laden Hausverbot und hinaus */
signed short use_skill(struct hero far *h, signed char mod, signed short arg)
{
	signed short res = 1;
	signed short skill;
	signed short loctype = g_group->loctype;	/* Ortsart LOC_* */

	if (check_hero(h)) {
		skill = select_skill(h);
		if (skill != -1) {
			txt_load_list(g_font, 0x1f, 0x12c, &h->f5e7, &h->txt);
			switch (skill) {
			case TA_HEILEN_GIFT:
				res = ((SKILL_FN_S)skill_cure_poison)(h, mod);
				break;
			case TA_HEILEN_KRANKHEITEN:
				res = skill_heal_disease(h, mod);
				break;
			case TA_HEILEN_WUNDEN:
				res = skill_heal_wounds(h, mod);
				break;
			case TA_AKROBATIK:
				res = skill_acrobatics(h, mod);
				break;
			case TA_MUSIZIEREN:
				res = skill_music(h, mod);
				break;
			case TA_TANZEN:
				res = skill_dance(h, mod);
				break;
			case TA_FALSCHSPIEL:
				res = skill_cheat(h, mod);
				if (res < 0 && loctype == LOC_TAVERN) {
					g_tavern_banned[g_loc_id] = 1;
					tavern_leave(g_tavern);
				}
				break;
			case TA_TASCHENDIEBSTAHL:
				res = skill_pickpocket(h, mod);
				if (res < 0) {
					if (loctype == LOC_TAVERN) {
						g_tavern_banned[g_loc_id] = 1;
						tavern_leave(g_tavern);
					} else if (loctype == LOC_MERCHANT) {
						g_shop_banned[g_loc_id] = 1;
						shop_leave(g_shop);
					}
				}
				break;
			case TA_ALCHIMIE:
				res = plan_alchemy(h);
				break;
			case TA_SCHAETZEN:
				res = ((SKILL_FN_SS)skill_appraise)(h, mod, arg);
				break;
			}
			mem_free(h->f5e7);
			mem_free(h->txt);
		}
	} else
		res = -1;
	return res;
}

signed short skill_pickpocket(struct hero far *h, signed short mod)
{
	signed short res = 1;
	signed long money;

	if (test_skill(h, TA_TASCHENDIEBSTAHL, mod) > 0) {
		money = random_interval(500, 1000);
		money_to_string(g_text_buf2, money);
		sprintf(g_text_output_buf, h->txt[40], g_text_buf2, h->name);
		group_add_money(g_group, money);
		g_refresh = 1;
	} else {
		sprintf(g_text_output_buf, h->txt[41], h->name);
		g_group->money = 0;
		res = -2;
	}
	dlg_message(g_dialog, g_text_output_buf);
	return res;
}

signed short skill_cheat(struct hero far *h, signed short mod)
{
	signed short res = 1;
	signed long money;

	if (test_skill(h, TA_FALSCHSPIEL, mod) > 0) {
		money = random_interval(500, 1000);
		money_to_string(g_text_buf2, money);
		sprintf(g_text_output_buf, h->txt[38], g_text_buf2, hero_name_gen(h, 1));
		dlg_message(g_dialog, g_text_output_buf);
		group_add_money(g_group, money);
		g_refresh = 1;
	} else {
		dlg_message(g_dialog, h->txt[39]);
		res = -2;
	}
	return res;
}

/* Musizieren mit Instrument 0x25/0x5f/0x2e (Held, sonst Gruppe); an Ort 5/6 (g_music_id_req) Klang 0xaf..0xb4 */
signed short skill_music(struct hero far *h, signed short mod)
{
	signed short res = 1;
	signed short item;
	signed short loc;
	signed long money;
	signed short pos;
	signed short snd;
	signed short old_cursor;

	if (g_music_timer > 0) {
		dlg_message(g_dialog, h->txt[37]);
	} else {
		item = 0x25;
		pos = get_item_pos(h, 0x25);
		if (pos == -1) {
			item = 0x5f;
			pos = get_item_pos(h, 0x5f);
			if (pos == -1) {
				item = 0x2e;
				pos = get_item_pos(h, 0x2e);
			}
		}
		if (pos == -1) {
			item = 0x25;
			pos = group_find_item(g_group, 0x25);
			if (pos == -1) {
				item = 0x5f;
				pos = group_find_item(g_group, 0x5f);
				if (pos == -1) {
					item = 0x2e;
					pos = group_find_item(g_group, 0x2e);
				}
			}
		}
		if (pos == -1) {
			dlg_message(g_dialog, h->txt[262]);
			res = -1;
		} else {
			if (test_skill(h, TA_MUSIZIEREN, mod) > 0) {
				money = random_interval(100, 300);
				money_to_string(g_text_buf2, money);
				sprintf(g_text_output_buf, h->txt[35], h->name, g_text_buf2);
				dlg_message(g_dialog, g_text_output_buf);
				group_add_money(g_group, money);
				g_refresh = 1;
			} else {
				dlg_message(g_dialog, h->txt[36]);
				res = -1;
			}
			g_music_timer = 8 * TICKS_PER_HOUR;
			loc = g_music_id_req & 0x3fff;
			if (loc == 5 || loc == 6) {
				snd = 0;
				switch (item) {
				case -1:
					snd = 7;
					break;
				case 0x25:
					snd = res == -1 ? 4 : 3;
					break;
				case 0x5f:
					snd = res == -1 ? 6 : 5;
					break;
				case 0x2e:
					snd = res == -1 ? 2 : 1;
					break;
				}
				if (snd != 0 && snd >= 1 && snd <= 6) {
					old_cursor = g_cursor_nr_prev;
					mouse_set_cursor(5, 1);
					switch (snd) {
					case 1:
						snd_play(g_snd_fx, 0xb1);
						break;
					case 2:
						snd_play(g_snd_fx, 0xb2);
						break;
					case 3:
						snd_play(g_snd_fx, 0xaf);
						break;
					case 4:
						snd_play(g_snd_fx, 0xb0);
						break;
					case 5:
						snd_play(g_snd_fx, 0xb3);
						break;
					case 6:
						snd_play(g_snd_fx, 0xb4);
						break;
					}
					while (snd_busy(g_snd_fx, 0) == 1) {
						snd_service(g_snd_music);
						snd_service(g_snd_fx);
						snd_speech_service(g_snd_speech);
						wait_for_vsync();
					}
					mouse_reset();
					g_cursor_nr_prev = old_cursor;
				}
			}
		}
	}
	return res;
}

signed short skill_acrobatics(struct hero far *h, signed short mod)
{
	signed short res = 1;
	signed long money;

	if (g_acrobatics_timer > 0) {
		dlg_message(g_dialog, h->txt[34]);
	} else {
		if (test_skill(h, TA_AKROBATIK, mod) > 0) {
			money = random_interval(10, 200);
			money_to_string(g_text_buf2, money);
			sprintf(g_text_output_buf, h->txt[35], h->name, g_text_buf2);
			dlg_message(g_dialog, g_text_output_buf);
			group_add_money(g_group, money);
			g_refresh = 1;
		} else {
			dlg_message(g_dialog, h->txt[36]);
			res = -1;
		}
		g_acrobatics_timer = 8 * TICKS_PER_HOUR;
	}
	return res;
}

signed short skill_dance(struct hero far *h, signed short mod)
{
	signed short res = 1;
	signed long money;

	if (g_dance_timer > 0) {
		dlg_message(g_dialog, h->txt[34]);
	} else {
#if FIX_0214
		/* Fix #214: Probe auf Tanzen */
		if (test_skill(h, TA_TANZEN, mod) > 0) {
#else
		if (test_skill(h, TA_MUSIZIEREN, mod) > 0) {	/* Probe auf Musizieren, nicht Tanzen */
#endif
			money = random_interval(100, 300);
			money_to_string(g_text_buf2, money);
			sprintf(g_text_output_buf, h->txt[35], h->name, g_text_buf2);
			dlg_message(g_dialog, g_text_output_buf);
			group_add_money(g_group, money);
			g_refresh = 1;
		} else {
			dlg_message(g_dialog, h->txt[36]);
			res = -1;
		}
		g_dance_timer = 8 * TICKS_PER_HOUR;
	}
	return res;
}

/* Talentgruppe und Talent waehlen (BE LVL_select_skill); show_values: Werte "%s %d" anzeigen
 * (Steigerungsmenue); -1 Abbruch, -2 keine Gruppe */
signed short lvl_select_skill(struct hero far *h, signed short show_values)
{
	signed short group;
	signed short first;
	signed short res = -1;
	struct str6 fmt = g_select_skill_lvlup;
	signed short i;

	if (show_values)
		sprintf(g_text_output_buf, g_ttx[258], h->skill_incs > 1 ? g_ttx[262] : g_ttx[261], h->skill_incs,
			h->skill_incs > 1 ? g_ttx[260] : g_ttx[259]);
	else
		strcpy(g_text_output_buf, g_ttx[263]);
	group = dlg_choose(g_dialog, g_text_output_buf, 7, g_ttx[127], g_ttx[128], g_ttx[129], g_ttx[132],
		g_ttx[130], g_ttx[131], g_ttx[133]) - 1;
	if (group != -2) {
		first = g_skills_index[group].first;
		for (i = 0; g_skills_index[group].count > i; i++) {
			if (show_values) {
				sprintf(g_text_output_buf + i * 50, fmt.s, g_ttx[first + i + 75], get_skill_value(h, first + i));
				g_menu_texts[i] = g_text_output_buf + i * 50;
			} else
				g_menu_texts[i] = g_ttx[first + i + 75];
		}
		res = dlg_choose(g_dialog, g_ttx[264], g_skills_index[group].count,
			g_menu_texts[0], g_menu_texts[1], g_menu_texts[2], g_menu_texts[3],
			g_menu_texts[4], g_menu_texts[5], g_menu_texts[6], g_menu_texts[7],
			g_menu_texts[8], g_menu_texts[9], g_menu_texts[10], g_menu_texts[11],
			g_menu_texts[12], g_menu_texts[13], g_menu_texts[14], g_menu_texts[15]);
		if (res != -1)
			res += first - 1;
	} else
		res = -2;
	return res;
}

/* Talentauswahl je Ort: drei Heiltalente fest, dazu je nach Bild/Ortsart weitere */
signed short select_skill(struct hero far *h)
{
	signed short n = 3;
	signed short sel;
	struct skill_list9 l = g_select_skill_defaults;
	signed short loctype = g_group->loctype;

	if (g_pp20_index == 14 || g_pp20_index == 15)
		l.skill[n++] = TA_SCHAETZEN;
	if (loctype == LOC_TAVERN && g_tavern_crowd > 1) {
		l.skill[n++] = TA_AKROBATIK;
		l.skill[n++] = TA_FALSCHSPIEL;
		l.skill[n++] = TA_TASCHENDIEBSTAHL;
		l.skill[n++] = TA_MUSIZIEREN;
		l.skill[n++] = TA_TANZEN;
	}
	if (loctype == LOC_CAMP_TOWN || loctype == LOC_CAMP_DUNGEON || loctype == LOC_CAMP_WILD || loctype == LOC_INN)
		l.skill[n++] = TA_ALCHIMIE;
	if (loctype == LOC_MERCHANT) {
		if (g_loc_id == 0x18)
			l.skill[n++] = TA_AKROBATIK;
		l.skill[n++] = TA_TASCHENDIEBSTAHL;
	}
	sel = dlg_choose(g_dialog, g_ttx[264], n, g_ttx[l.skill[0] + 75], g_ttx[l.skill[1] + 75],
		g_ttx[l.skill[2] + 75], g_ttx[l.skill[3] + 75], g_ttx[l.skill[4] + 75], g_ttx[l.skill[5] + 75],
		g_ttx[l.skill[6] + 75], g_ttx[l.skill[7] + 75], g_ttx[l.skill[8] + 75]);
	return sel != -1 ? l.skill[sel - 1] : sel;
}

/* Heilkunde Wunden an einem gewaehlten Helden (Sperre heal_timer 36 h) */
signed short skill_heal_wounds(struct hero far *h, signed short mod)
{
	signed short res = 1;
	struct hero far *patient;

	g_hero_excluded = group_hero_pos(g_group, h);
	if ((patient = get_any_hero(g_group, h->txt[126])) != 0) {
		if (!(signed char)patient->petrified) {
			if (patient->is_healable()) {
				signed short le;
				signed short missing;
				signed short dmg;

				if ((le = patient->le) >= patient->le_max) {
					sprintf(g_text_output_buf, h->txt[123], patient->name);
					dlg_message(g_dialog, g_text_output_buf);
				} else if (patient->heal_timer > 0) {
					sprintf(g_text_output_buf, h->txt[129], patient->name);
					dlg_message(g_dialog, g_text_output_buf);
				} else {
					patient->heal_timer = 0x1fa40L;
					if (test_skill(h, TA_HEILEN_WUNDEN, mod - 5) > 0) {
						if (test_skill(h, TA_HEILEN_WUNDEN, mod - 5) > 0) {
							res = get_skill_value(h, TA_HEILEN_WUNDEN) > 1 ? get_skill_value(h, TA_HEILEN_WUNDEN) : 1;
							if ((missing = patient->le_max - patient->le) < res)
								res = missing;
							if (!res)
								res = 1;
							add_hero_le(patient, res);
							sprintf(g_text_output_buf, h->txt[131], h->name, patient->name, res,
								res > 1 ? g_ttx[242] : (char far *)"");
							snd_play(g_snd_fx, 0x99);
						} else {
							dmg = 3;
							if ((le = patient->le) <= dmg)
								dmg = le - 1;
							sub_hero_le(patient, dmg);
							sprintf(g_text_output_buf, h->txt[130], patient->name, dmg,
								dmg > 1 ? g_ttx[341] : (char far *)"");
							res = 0;
							patient->staffspell_timer = 0x1fa40L;
						}
						dlg_message(g_dialog, g_text_output_buf);
					} else {
						if (random_schweif(20) <= 7) {
							sprintf(g_text_output_buf, h->txt[125], h->name, patient->name);
							hero_get_ill(patient, ILLNESS_WUNDFIEBER);
						} else
							sprintf(g_text_output_buf, h->txt[124], h->name, patient->name);
						dlg_message(g_dialog, g_text_output_buf);
					}
				}
			} else {
				sprintf(g_text_output_buf, h->txt[42], patient->name);
				dlg_message(g_dialog, g_text_output_buf);
			}
		} else {
			sprintf(g_text_output_buf, h->txt[258], patient->name);
			dlg_message(g_dialog, g_text_output_buf);
		}
	}
	return res;
}

signed short skill_heal_disease(struct hero far *h, signed short mod)
{
	struct hero far *patient;

	g_hero_excluded = group_hero_pos(g_group, h);
	if ((patient = get_any_hero(g_group, h->txt[126])) != 0)
		((CURE_FN_SS)skill_cure_disease)(h, patient, mod, 0);
	return 1;
}
