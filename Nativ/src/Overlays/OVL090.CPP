/* OVL090 - Overlay O090 (Segment 0x2115 in SCHWEIF.EXE): Talentanwendungen mit Texten des Helden (h->txt, +0x5eb),
 * Entsprechungen in DSA 1 (Bright Eyes seg104/seg103): skill_cure_disease, has_herb_for_disease, plan_alchemy.
 * skill_cure_poison(h, mod): Heilkunde Gift - Patient per get_any_hero, Gift hero_is_poisoned, Sperre heal_timer
 *   (6 h), Proben test_skill(TA_HEILEN_GIFT) mit Zuschlag g_poison_prices[gift] (Bit diagnosed spart die erste),
 *   hero_cure_poison, dann LE-Heilung mit Mengenabfrage dlg_input oder Schaden.
 * has_herb_for_disease(h, ill): Heilkraut fuer Krankheit ill suchen (get_item_pos; 999 = zwei Kraeuter 0x82 + 0x3f).
 * skill_cure_disease(h, patient, mod, flag): Heilkunde Krankheiten, flag: Texte selbst laden (txt_load_list nach
 *   h->f5e7/h->txt) und am Ende freigeben; Krankheit hero_is_diseased, Zuschlag g_disease_prices[ill],
 *   hero_cure_disease, Kraeuter ablegen drop_item.
 * plan_alchemy(h): Alchimie - Rezepte g_recipes (14 x 0x1c Bytes) per Gegenstand oder Rezeptbuch (0xbf,
 *   box_has), Auswahlliste in g_text_output_buf (je 50 Bytes) / g_menu_texts[14], Pruefungen
 *   AE/Herberge (Ortsart 3, g_inn_room/g_inn_days)/Dauer, Zeit timewarp, do_alchemy brauen.
 * money_to_rough_string(buf, f1, f2, f3, m): Geldbetrag in Heller grob runden und als D/S/H ausgeben.
 * skill_appraise(h, mod, id): Schaetzen - Preis (Wert * price_unit / 2) mit Fehler je nach Probe, Text ueber
 *   money_to_rough_string.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL090.CPP   -> 5524/5524 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * - erster Wurf GLEICH; Vorlage DSA 1 aus Bright Eyes (seg104) passte fast eins zu eins.
 * - Patient-Pruefung: "!(signed char)patient->petrified" (cbw / or ax,ax) und inline hero::is_healable()
 *   { return dead || petrified ? 0 : 1; } (-Og cacht Bitbyte [bp-0bh] und Bit [bp-0eh]); in skill_cure_disease
 *   "patient->is_healable() == 1" -> cmp ax,1.
 * - "or byte es:[..],2" mit Parameterkopie [bp-0ah]/[bp-6] = inline set_poi_diagnosed(i)/set_ill_diagnosed(i)
 *   (Bitfeld unsigned short, frueher hier unsigned char; Wahrheitstest mit (signed char)-Cast).
 * - Preistabellen g_poison_prices/g_disease_prices sind short-Felder, das Original liest nur das untere Byte:
 *   "*(signed char *)&g_poison_prices[poi]".
 * - plan_alchemy: Ortsart g_group +0x1e ueber inline get_loctype() (mov al / cbw / mov [lt],al); g_recipes[ri]
 *   als -Og-Offset [bp-18h]; strcpy-Intrinsic auf g_text_output_buf + cnt * 50; 14 far-Zeiger als variadische
 *   Argumente.
 * - money_to_rough_string: Geldwerte unsigned long (div), letzter sprintf mit (unsigned short)m (nur ein Wort
 *   gepusht).
 * - skill_appraise: "(signed long)price * price_unit" -> movsx/push eax/cbw/movsx/pop edx/imul; "delta = price /
 *   100 * (n * 2L)" (movsx edx,si / shl edx,1); "price += random_schweif(100) <= 50 ? delta : -delta" (neg eax).
 * Originalfehler: plan_alchemy - sel[13] (wie DSA 1), aber 14 Rezepte (i <= 13): bei 14 verfuegbaren Rezepten wird
 *   ueber das Feld hinaus geschrieben ([bp-19h], zufaellig ein unbenutztes Fuellbyte vor dem -Og-Wert [bp-18h] -
 *   ohne Folgen); skill_appraise - im Fehlschlag-Zweig fehlt "if (price == 0) price = 1" (Preis 0 moeglich),
 *   "price *= 100 ... / 100" ist wirkungslos, Rueckgabe immer 1; skill_cure_poison - Sperre heal_timer wird
 *   schon vor der Probe gesetzt (wie DSA 1).
 */
#include <stdio.h>
#include <string.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:5c9c..5ca0, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */

signed short skill_cure_poison(struct hero far *h, signed char mod)
{
	signed short res = 1;
	signed short n;
	struct hero far *patient;
	register signed short dmg;
	register signed short poi;

	g_hero_excluded = group_hero_pos(g_group, h);
	if ((patient = get_any_hero(g_group, h->txt[0x7e])) != 0) {
		if (!(signed char)patient->petrified) {
			if (patient->is_healable()) {
				poi = hero_is_poisoned(patient);
				if (!poi) {
					sprintf(g_text_output_buf, h->txt[0x7f], patient->name);
					dlg_message(g_dialog, g_text_output_buf);
				} else if (patient->heal_timer > 0) {
					sprintf(g_text_output_buf, h->txt[0x81], patient->name);
					dlg_message(g_dialog, g_text_output_buf);
				} else {
					patient->heal_timer = 0x5460L;
					if ((signed char)patient->poison[poi].diagnosed || test_skill(h, TA_HEILEN_GIFT, mod) > 0) {
						timewarp(g_date, 0x708L);
						patient->set_poi_diagnosed(poi);
						if (test_skill(h, TA_HEILEN_GIFT, *(signed char *)&g_poison_prices[poi] + mod - 5) > 0) {
							sprintf(g_text_output_buf, h->txt[0x86], h->name, hero_name_gen(patient, 1));
							dlg_message(g_dialog, g_text_output_buf);
							hero_cure_poison(patient);
							if (patient->le < patient->le_max) {
								sprintf(g_text_output_buf, h->txt[0x87], h->name, patient->name);
								if (dlg_yes_no(g_dialog, g_text_output_buf) == 1) {
									do {
										n = dlg_input(g_dialog, 2, h->txt[0x84], 1, patient->le_max - patient->le, 1);
									} while (n < 0);
									if ((res = test_skill(h, TA_HEILEN_GIFT, n + mod - 5)) > 0) {
										if (patient->le_max - patient->le < n)
											n = patient->le_max - patient->le;
										if (n == 0)
											n = 1;
										sprintf(g_text_output_buf, h->txt[0x83], h->name, patient->name, n,
											n > 1 ? g_ttx[0xf2] : (char far *)"");
										add_hero_le(patient, n);
										snd_play(g_snd_fx, 0x99);
									} else {
										dmg = 3;
										if (patient->le <= dmg)
											dmg = patient->le - 1;
										sub_hero_le(patient, dmg);
										sprintf(g_text_output_buf, h->txt[0x82], patient->name, dmg,
											dmg > 1 ? g_ttx[0x155] : (char far *)"");
										res = 0;
									}
									dlg_message(g_dialog, g_text_output_buf);
								}
							}
						} else {
							sprintf(g_text_output_buf, h->txt[0x85], h->name, hero_name_gen(patient, 1));
							dlg_message(g_dialog, g_text_output_buf);
						}
					} else {
						sprintf(g_text_output_buf, h->txt[0x80], h->name, hero_name_gen(patient, 1));
						dlg_message(g_dialog, g_text_output_buf);
					}
				}
			} else {
				sprintf(g_text_output_buf, h->txt[0x2a], patient->name);
				dlg_message(g_dialog, g_text_output_buf);
			}
		} else {
			sprintf(g_text_output_buf, h->txt[0x102], patient->name);
			dlg_message(g_dialog, g_text_output_buf);
		}
	}
	return res;
}

signed short has_herb_for_disease(struct hero far *h, signed short ill)
{
	register signed short herb = 0;

	switch (ill) {
	case ILLNESS_WUNDFIEBER:
	case ILLNESS_BLAUE_KEUCHE:
		herb = 0x63;
		break;
	case ILLNESS_DUMPFSCHAEDEL:
		if (get_item_pos(h, 0x7b) != -1)
			herb = 0x7b;
		else if (get_item_pos(h, 0x3d) != -1)
			herb = 0x3d;
		else if (get_item_pos(h, 0x10b) != -1)
			herb = 0x10b;
		else if (get_item_pos(h, 0x3c) != -1)
			herb = 0x3c;
		else if (get_item_pos(h, 0x135) != -1)
			herb = 0x135;
		else if (get_item_pos(h, 0x40) != -1)
			herb = 0x40;
		else if (get_item_pos(h, 0x7c) != -1)
			herb = 0x7c;
		else if (get_item_pos(h, 0x81) != -1)
			herb = 0x81;
		else if (get_item_pos(h, 0x7d) != -1)
			herb = 0x7d;
		else if (get_item_pos(h, 0x9c) != -1)
			herb = 0x9c;
		else if (get_item_pos(h, 0x82) != -1)
			herb = 0x82;
		break;
	case ILLNESS_PARALYSE:
		if (get_item_pos(h, 0x7c) != -1)
			herb = 0x7c;
		break;
	case ILLNESS_SCHLACHTFELDFIEBER:
		if (get_item_pos(h, 0x82) != -1 && get_item_pos(h, 0x3f) != -1)
			herb = 999;
		break;
	case ILLNESS_FROSTSCHAEDEN:
		if (get_item_pos(h, 0x3d) != -1)
			herb = 0x3d;
		else if (get_item_pos(h, 0x10b) != -1)
			herb = 0x10b;
		break;
	case ILLNESS_TOLLWUT:
		if (get_item_pos(h, 0x82) != -1)
			herb = 0x82;
		break;
	}
	return herb;
}

signed short skill_cure_disease(struct hero far *h, struct hero far *patient, signed char mod, signed short flag)
{
	signed short dmg;
	signed short herb;
	register signed short res = 0;
	register signed short ill;

	if (flag)
		txt_load_list(g_font, 0x1f, 0x12c, &h->f5e7, &h->txt);
	if (!(signed char)patient->petrified) {
		if (patient->is_healable() == 1) {
			if ((ill = hero_is_diseased(patient)) == 0) {
				sprintf(g_text_output_buf, h->txt[0x8a], patient->name, h->name);
			} else if (patient->heal_timer > 0) {
				sprintf(g_text_output_buf, h->txt[0x81], patient->name);
			} else {
				timewarp(g_date, 0x1c2L);
				if (flag != 0 || (signed char)patient->illness[ill].diagnosed || test_skill(h, TA_HEILEN_KRANKHEITEN, mod - 5) > 0) {
					patient->set_ill_diagnosed(ill);
					sprintf(g_text_output_buf, h->txt[0xa0], g_ttx[ill + 0x2b]);
					dlg_message(g_dialog, g_text_output_buf);
					if (ill == ILLNESS_WUNDFIEBER || ill == ILLNESS_BLAUE_KEUCHE) {
						sprintf(g_text_output_buf, h->txt[0xff], patient->name);
					} else if ((herb = has_herb_for_disease(h, ill)) == 0) {
						sprintf(g_text_output_buf, h->txt[0x76], h->name);
					} else {
						patient->heal_timer = 0x5460L;
						if ((res = test_skill(h, TA_HEILEN_KRANKHEITEN, *(signed char *)&g_disease_prices[ill] + mod - 5)) > 0) {
							if (patient->le_max - patient->le < res)
								res = patient->le_max - patient->le;
							add_hero_le(patient, res);
							if (res == 0)
								res = 1;
							sprintf(g_text_output_buf, h->txt[0x88], h->name, hero_name_gen(patient, 1), grammar_pronoun(patient->sex, 3, 0),
								res, res > 1 ? g_ttx[0xf2] : (char far *)"");
							hero_cure_disease(patient);
							snd_play(g_snd_fx, 0x99);
							if (herb == 999) {
								drop_item(h, get_item_pos(h, 0x82), 1);
								drop_item(h, get_item_pos(h, 0x3f), 1);
							} else {
								drop_item(h, get_item_pos(h, herb), 1);
							}
							res = 1;
						} else {
							dmg = 3;
							if (patient->le <= dmg)
								dmg = patient->le - 1;
							sub_hero_le(patient, dmg);
							sprintf(g_text_output_buf, h->txt[0x82], patient->name, dmg,
								dmg > 1 ? g_ttx[0x155] : (char far *)"");
						}
					}
				} else {
					sprintf(g_text_output_buf, h->txt[0x89], h->name, patient->name);
				}
			}
			dlg_message(g_dialog, g_text_output_buf);
		} else {
			sprintf(g_text_output_buf, h->txt[0x2a], patient->name);
			dlg_message(g_dialog, g_text_output_buf);
		}
	} else {
		sprintf(g_text_output_buf, h->txt[0x102], patient->name);
		dlg_message(g_dialog, g_text_output_buf);
	}
	if (flag) {
		mem_free(h->f5e7);
		mem_free(h->txt);
	}
	return res;
}

signed short plan_alchemy(struct hero far *h)
{
	signed short ret = 1;
	signed short w;
	signed short kit;
	signed short cnt = 0;
	signed short ans;
	signed short dec;
	signed short i;
	signed short book;
	signed char ri;
	signed short miss;
	signed char loctype;		/* Ortsart (group +0x1e) */
#if FIX_0152
	/* Fix #152: Platz fuer alle 14 Rezepte */
	signed char sel[14];
#else
	signed char sel[13];
#endif

	kit = get_item_pos(h, 0x2f);
	if (kit == -1) {
		dlg_message(g_dialog, h->txt[0xa4]);
		ret = 0;
	} else {
		loctype = g_group->get_loctype();
		book = get_item_pos(h, 0xbf);
		for (i = 0; i <= 13; i++) {
			if (get_item_pos(h, g_recipes[i].item_id) != -1 || (book != -1 && box_has(h, book, i))) {
				strcpy(g_text_output_buf + cnt * 50, grammar_name(g_item_names[g_recipes[i].outcome]));
				g_menu_texts[cnt] = g_text_output_buf + cnt * 50;
				sel[cnt] = i;
				cnt++;
			}
		}
		if (cnt != 0) {
			w = dlg_set16(g_dialog, 7);
			ans = dlg_choose(g_dialog, h->txt[0xa5], cnt, g_menu_texts[0], g_menu_texts[1], g_menu_texts[2], g_menu_texts[3],
				g_menu_texts[4], g_menu_texts[5], g_menu_texts[6], g_menu_texts[7], g_menu_texts[8], g_menu_texts[9], g_menu_texts[10],
				g_menu_texts[11], g_menu_texts[12], g_menu_texts[13]);
			dlg_set16(g_dialog, w);
			if (ans != -1) {
				ri = sel[ans - 1];
				if ((miss = hero_has_ingredients(h, ri)) == 0) {
					if (g_recipes[ri].ae > h->ae) {
						sprintf(g_text_output_buf, g_ttx[0x154], h->name);
						dlg_message(g_dialog, g_text_output_buf);
						ret = 0;
					} else if (loctype == 3 && g_inn_room == -1) {
						dlg_message(g_dialog, h->txt[0xa6]);
						ret = 0;
					} else if (g_recipes[ri].duration > 8 && (loctype != 3 || g_recipes[ri].duration / 24 > g_inn_days)) {
						sprintf(g_text_output_buf, h->txt[0xa7], g_recipes[ri].duration);
						dlg_message(g_dialog, g_text_output_buf);
						ret = 0;
					} else {
						if (g_recipes[ri].duration > 8) {
							sprintf(g_text_output_buf, h->txt[0xa8], g_recipes[ri].duration);
							w = dlg_set16(g_dialog, 7);
							dec = dlg_choose(g_dialog, g_text_output_buf, 2, h->txt[0xaa], h->txt[0xab]);
							dlg_set16(g_dialog, w);
						} else {
							dec = 1;
						}
						if (dec == 1) {
							timewarp(g_date, (signed long)g_recipes[ri].duration * 5400);
							if (loctype != 8 && loctype != 10 && loctype != 9)
								group_sleep(g_group, g_inn_room);
							else
								h->staffspell_timer = 0x1fa40L;
							ret = do_alchemy(h, ri, 0);
							g_inn_room = 0;
						} else {
							ret = 0;
						}
					}
				} else {
					sprintf(g_text_output_buf, h->txt[0xac], grammar_name(g_item_names[miss]));
					dlg_message(g_dialog, g_text_output_buf);
				}
			}
		} else {
			sprintf(g_text_output_buf, h->txt[0xad], h->name);
			dlg_message(g_dialog, g_text_output_buf);
		}
	}
	return ret;
}

void money_to_rough_string(char far *buf, char far *f1, char far *f2, char far *f3, unsigned long m)
{
	unsigned long d = 0;
	register unsigned short s = 0;

	d = m / 100;
	m %= 100;
	s = m / 10;
	m %= 10;
	if (m > 5) {
		s++;
		m = 0;
	}
	if (s > 5) {
		d++;
		s = 0;
	}
	if (d == 1) {
		s = 9;
		d = 0;
	} else if (s == 1) {
		m = 9;
		s = 0;
	} else if (m == 1) {
		m = 2;
	}
	if (d != 0) {
		sprintf(buf, f1, d);
		return;
	}
	if (s != 0) {
		sprintf(buf, f2, s);
		return;
	}
	sprintf(buf, f3, (unsigned short)m);
}

signed short skill_appraise(struct hero far *h, signed char mod, signed short id)
{
	signed short ret = 1;
	signed short res_test;
	signed long price;
	signed long delta;
	signed short pos;
	register signed short n;
	register signed short item;

	if (id == -1) {
		pos = select_item(h, h->txt[0x8e], 1);
		item = pos != -1 ? h->inv[pos].item_id : 0;
	} else {
		pos = 0;
		item = id;
	}
	if (pos != -1 && item != 0) {
		price = (signed long)g_items[item].price * g_items[item].price_unit;
		price /= 2;
		if ((res_test = test_skill(h, TA_SCHAETZEN, mod)) >= 0) {
			n = 10 - res_test;
			if (n < 1)
				n = 1;
			price *= 100;
			delta = price / 100 * (n * 2L);
			price += random_schweif(100) <= 50 ? delta : -delta;
			price /= 100;
			if (price == 0)
				price = 1;
			money_to_rough_string(g_text_buf2, h->txt[0xa1], h->txt[0xa2], h->txt[0xa3], price);
			sprintf(g_text_output_buf, h->txt[0x8f], h->name, grammar_article(2, item, 0), g_text_buf2);
		} else if (res_test == -99) {
			sprintf(g_text_output_buf, h->txt[0x90], h->name);
		} else {
			n = 14 - res_test;
			if (n > 40)
				n = 40;
			price *= 100;
			delta = price / 100 * (n * 2L);
			price += random_schweif(100) <= 50 ? delta : -delta;
#if FIX_0152
			/* Fix #152: Fehlschlag: Mindestpreis 1 wie im Erfolgszweig */
			price /= 100;
				if (price <= 0)
					price = 1;
				money_to_rough_string(g_text_buf2, h->txt[0xa1], h->txt[0xa2], h->txt[0xa3], price);
				sprintf(g_text_output_buf, h->txt[0x8f], h->name, grammar_article(2, item, 0), g_text_buf2);
			}
#else
			price /= 100;
			money_to_rough_string(g_text_buf2, h->txt[0xa1], h->txt[0xa2], h->txt[0xa3], price);
			sprintf(g_text_output_buf, h->txt[0x8f], h->name, grammar_article(2, item, 0), g_text_buf2);
		}
#endif
		dlg_message(g_dialog, g_text_output_buf);
	}
	return ret;
}
