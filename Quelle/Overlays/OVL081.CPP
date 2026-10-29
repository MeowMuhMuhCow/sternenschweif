/* OVL081 - Overlay O081 (Segment 0x20e2 in SCHWEIF.EXE): Krankheiten eines Helden fortschreiben (DSA2-Gegenstueck
 * zu disease_effect aus DSA 1 seg119, hier je Held aus hero_timers R10). Tabelle h->illness[1..7] (struct
 * hero_ill: Zustand -1 krank / 1 genesend, Tage, KK-Verlust, GE-Verlust, Bit effect = Wirkung angewandt);
 * Texte txt_load_list(g_font, 0x81, 0x32) txt[12..22]/txt[42] (Puffer am Ende frei). 1 Wundfieber (ab Tag 14
 * genesend, sonst Schaden 2W6-(Tage-1), KK -1), 2 Dumpfschaedel (GE -2/KK -5, ab Tag 3 Ansteckung
 * group_infect(g_group, 2, 20 %, h), 5 % Selbstheilung + hero_get_ill(h, 3), sonst ab W3+4 Tagen genesend),
 * 3 Blaue Keuche (halbe GE/KK, ab Tag 4 zu 25 % KK-Grundwert -1 und LE-Max/LE -Tage/3), 4 Paralyse (W6 GE/KK),
 * 5 Schlachtfeldfieber (ab Tag 8 Tod zu 30 % bei Elfen, sonst 20 %; ab Tag 4 KK -5, Ansteckung 5 %),
 * 6 Frostschaeden (Tage*5 % GE-Grundwert -1, KK/GE -1), 7 Tollwut (KK -2, ab Tag 3 rabies_effect); dann Tage
 * aller Krankheiten +1; KK oder GE <= 0 -> alle Verluste zurueck und Tod (sub_hero_le(h, LE)); Bit unconscious
 * geaendert -> txt[42] (ausser g_silent_mode), bei Bild 14 g_refresh = 1.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL081.CPP   -> 5595/5595 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * - C++ wegen inline-Elementfunktionen hero::set_ill_state/set_ill_days/set_ill_kk/set_ill_ge/set_ill_effect/
 *   clr_ill_effect(i[, v]) (jede Expansion eigene Kopie von i und ggf. v, Konstanten nicht) und add_ill_*(i, v)
 *   { x += v; if (x < 0) x = 0; else ; } (toter jmp short, eigene -Og-Plaetze i*5/Offset je Expansion,
 *   v = 1/-1 -> inc/dec byte, v = 2 -> add byte,2).
 * - -Og: Statusbyte +bc [bp-7eh], ill*5 [bp-80h], Offset h + ill*5 in DI (Segment nachgeladen).
 * - Bitfelder unsigned short; "!(signed char)h->dead" -> and/cbw/or, "(signed char)h->illness[i].effect" ->
 *   and/or al,al; Vergleich "(signed char)h->unconscious != was_unconscious" (signed char) -> cmp al,[bp-3];
 *   ohne Cast int-Vergleich (cbw / mov dl / cmp ax,dx).
 * Originalfehler: Blaue Keuche (3) steckt die Gruppe mit Krankheit 2 (Dumpfschaedel) an - group_infect(.., 2, 20, h)
 *   wie beim Dumpfschaedel (DSA 1: hero_gets_diseased(.., 3)).
 */
#include <stdio.h>
#include "schweif.h"

void disease_effect(struct hero far *h)
{
	signed short k;				/* Zwischenwert bzw. Schleifenzaehler */
	signed char was_unconscious;	/* Bit unconscious vor der Bearbeitung */
	void far *txt_buf;
	char far * far *txt;
	register signed short ill;		/* Krankheit ILLNESS_* */

	if (!(signed char)h->dead) {
		txt_load_list(g_font, 0x81, 0x32, &txt_buf, &txt);
		was_unconscious = h->unconscious;
		ill = ILLNESS_WUNDFIEBER;
		/* 1: Wundfieber */
		if (!(signed char)h->dead && h->illness[ill].state == ILL_STATE_SICK) {
			if (h->illness[ill].days > 13) {
				h->set_ill_days(ill, 0);
				h->set_ill_state(ill, ILL_STATE_RECOVER);
				sprintf(g_text_output_buf, txt[12], h->name);
				dlg_message(g_dialog, g_text_output_buf);
			} else {
				sub_hero_le(h, dice_roll(2, 6, -(h->illness[ill].days - 1)));
				if (!(signed char)h->dead && h->attrib[ATTRIB_KK].cur > 1) {
					h->add_ill_kk(ill, 1);
					change_attrib(h, ATTRIB_KK, -1);
				}
			}
		}
		if (!(signed char)h->dead && h->illness[ill].state == ILL_STATE_RECOVER) {
			if (!h->illness[ill].loss_kk) {
				h->set_ill_days(ill, 0);
				h->set_ill_state(ill, 0);
			} else {
				h->add_ill_kk(ill, -1);
				change_attrib(h, ATTRIB_KK, 1);
			}
		}
		ill = ILLNESS_DUMPFSCHAEDEL;
		/* 2: Dumpfschaedel */
		if (!(signed char)h->dead && h->illness[ill].state == ILL_STATE_SICK) {
			if (!(signed char)h->illness[ill].effect) {
				sprintf(g_text_output_buf, txt[13], hero_name_gen(h, 1));
				dlg_message(g_dialog, g_text_output_buf);
				h->set_ill_effect(ill);
				change_attrib(h, ATTRIB_GE, -2);
				change_attrib(h, ATTRIB_KK, -5);
			}
			if (h->illness[ill].days > 2) {
				group_infect(g_group, 2, 20, h);
			}
			if (random_schweif(100) < 5) {
				h->set_ill_days(ill, 0);
				h->set_ill_state(ill, 0);
				if ((signed char)h->illness[ill].effect) {
					h->clr_ill_effect(ill);
					g_quiet = 1;
					change_attrib(h, ATTRIB_GE, 2);
					change_attrib(h, ATTRIB_KK, 5);
					g_quiet = 0;
				}
				hero_get_ill(h, 3);
			} else if (h->illness[ill].days > dice_roll(1, 3, 4)) {
				h->set_ill_state(ill, ILL_STATE_RECOVER);
			} else {
				sub_hero_le(h, dice_roll(1, 6, -1));
			}
		}
		if (!(signed char)h->dead && h->illness[ill].state == ILL_STATE_RECOVER) {
			h->set_ill_days(ill, 0);
			h->set_ill_state(ill, 0);
			if ((signed char)h->illness[ill].effect) {
				h->clr_ill_effect(ill);
				change_attrib(h, ATTRIB_GE, 2);
				change_attrib(h, ATTRIB_KK, 5);
			}
			sprintf(g_text_output_buf, txt[14], hero_name_gen(h, 1));
			dlg_message(g_dialog, g_text_output_buf);
		}
		ill = ILLNESS_BLAUE_KEUCHE;
		/* 3: Blaue Keuche */
		if (!(signed char)h->dead && h->illness[ill].state == ILL_STATE_SICK) {
			if (!(signed char)h->illness[ill].effect) {
				h->set_ill_effect(ill);
				sprintf(g_text_output_buf, txt[15], hero_name_gen(h, 1));
				dlg_message(g_dialog, g_text_output_buf);
				k = h->attrib[ATTRIB_GE].cur;
				change_attrib(h, ATTRIB_GE, -k / 2);
				h->set_ill_ge(ill, k / 2);
				k = h->attrib[ATTRIB_KK].cur;
				change_attrib(h, ATTRIB_KK, -k / 2);
				h->set_ill_kk(ill, k / 2);
			}
			if (h->illness[ill].days > 3 && random_schweif(100) <= 25) {
				change_attrib_normal(h, ATTRIB_KK, -1);
				sub_le_max(h, h->illness[ill].days / 3);
				sub_hero_le(h, h->illness[ill].days / 3);
				h->set_ill_state(ill, ILL_STATE_RECOVER);
			} else {
#if FIX_0154
				/* Fix #154: Blaue Keuche steckt mit Blauer Keuche (3) an */
				group_infect(g_group, 3, 20, h);
#else
				group_infect(g_group, 2, 20, h);	/* Originalfehler: 2 statt 3 */
#endif
				sub_hero_le(h, dice_roll(1, 6, 2));
			}
		}
		if (!(signed char)h->dead && h->illness[ill].state == ILL_STATE_RECOVER) {
			sprintf(g_text_output_buf, txt[16], h->name);
			dlg_message(g_dialog, g_text_output_buf);
			h->set_ill_days(ill, 0);
			h->set_ill_state(ill, 0);
			if ((signed char)h->illness[ill].effect) {
				h->clr_ill_effect(ill);
				change_attrib(h, ATTRIB_GE, h->illness[ill].loss_ge);
				change_attrib(h, ATTRIB_KK, h->illness[ill].loss_kk);
				h->set_ill_ge(ill, 0);
				h->set_ill_kk(ill, 0);
			}
		}
		ill = ILLNESS_PARALYSE;
		/* 4: Paralyse */
		if (!(signed char)h->dead && h->illness[ill].state == ILL_STATE_SICK) {
			if (h->illness[ill].days > dice_roll(1, 3, 4)) {
				h->set_ill_days(ill, 0);
				h->set_ill_state(ill, ILL_STATE_RECOVER);
			} else {
				sprintf(g_text_output_buf, txt[17], h->name);
				dlg_message(g_dialog, g_text_output_buf);
				k = random_schweif(6);
				h->add_ill_ge(ill, k);
				change_attrib(h, ATTRIB_GE, -k);
				k = random_schweif(6);
				h->add_ill_kk(ill, k);
				change_attrib(h, ATTRIB_KK, -k);
			}
		}
		if (!(signed char)h->dead && h->illness[ill].state == ILL_STATE_RECOVER) {
			if (!h->illness[ill].loss_kk && !h->illness[ill].loss_ge) {
				h->set_ill_days(ill, 0);
				h->set_ill_state(ill, 0);
			} else {
				if (h->illness[ill].loss_kk) {
					h->add_ill_kk(ill, -1);
					change_attrib(h, ATTRIB_KK, 1);
				}
				if (h->illness[ill].loss_ge) {
					h->add_ill_ge(ill, -1);
					change_attrib(h, ATTRIB_GE, 1);
				}
			}
		}
		ill = ILLNESS_SCHLACHTFELDFIEBER;
		/* 5: Schlachtfeldfieber */
		if (!(signed char)h->dead && h->illness[ill].state == ILL_STATE_SICK) {
			if (h->illness[ill].days > 7) {
				if (random_schweif(100) <= (h->type >= HERO_TYPE_GREEN_ELF ? 30 : 20)) {
					sprintf(g_text_output_buf, txt[18], h->name);
					dlg_message(g_dialog, g_text_output_buf);
					sub_hero_le(h, h->le);
				} else if ((signed char)h->illness[ill].effect) {
					h->set_ill_state(ill, ILL_STATE_RECOVER);
				}
			} else if (h->illness[ill].days > 3) {
				if (!(signed char)h->illness[ill].effect) {
					sprintf(g_text_output_buf, txt[19], hero_name_gen(h, 1));
					dlg_message(g_dialog, g_text_output_buf);
					h->set_ill_effect(ill);
					change_attrib(h, ATTRIB_KK, -5);
				}
				group_infect(g_group, 5, 5, h);
				sub_hero_le(h, dice_roll(h->type >= HERO_TYPE_GREEN_ELF ? 2 : 1, 6, h->illness[ill].days - 1));
			}
		}
		if (!(signed char)h->dead && h->illness[ill].state == ILL_STATE_RECOVER) {
			h->set_ill_days(ill, 0);
			h->set_ill_state(ill, 0);
			if ((signed char)h->illness[ill].effect) {
				h->clr_ill_effect(ill);
				change_attrib(h, ATTRIB_KK, 5);
				sprintf(g_text_output_buf, txt[20], hero_name_gen(h, 1));
				dlg_message(g_dialog, g_text_output_buf);
			}
		}
		ill = ILLNESS_FROSTSCHAEDEN;
		/* 6: Frostschaeden */
		if (!(signed char)h->dead && h->illness[ill].state == ILL_STATE_SICK) {
			k = 0;
			if (random_schweif(100) <= h->illness[ill].days * 5) {
				sprintf(g_text_output_buf, txt[21], h->name);
				dlg_message(g_dialog, g_text_output_buf);
				change_attrib_normal(h, ATTRIB_GE, -1);
				k = 1;
			}
			if (k == 0) {
				sprintf(g_text_output_buf, txt[22], h->name);
				dlg_message(g_dialog, g_text_output_buf);
			}
			sub_hero_le(h, dice_roll(1, 6, 0));
			if (!(signed char)h->dead) {
				if (h->attrib[ATTRIB_KK].cur) {
					h->add_ill_kk(ill, 1);
					change_attrib(h, ATTRIB_KK, -1);
				}
				if (h->attrib[ATTRIB_GE].cur) {
					h->add_ill_ge(ill, 1);
					change_attrib(h, ATTRIB_GE, -1);
				}
			}
		}
		if (!(signed char)h->dead && h->illness[ill].state == ILL_STATE_RECOVER) {
			if (!h->illness[ill].loss_kk && !h->illness[ill].loss_ge) {
				h->set_ill_days(ill, 0);
				h->set_ill_state(ill, 0);
			} else {
				if (h->illness[ill].loss_kk) {
					h->add_ill_kk(ill, -1);
					change_attrib(h, ATTRIB_KK, 1);
				}
				if (h->illness[ill].loss_ge) {
					h->add_ill_ge(ill, -1);
					change_attrib(h, ATTRIB_GE, 1);
				}
			}
		}
		ill = ILLNESS_TOLLWUT;
		/* 7: Tollwut */
		if (!(signed char)h->dead && h->illness[ill].state == ILL_STATE_SICK) {
			if (h->illness[ill].days > dice_roll(1, 6, 6)) {
				h->set_ill_days(ill, 0);
				h->set_ill_state(ill, ILL_STATE_RECOVER);
			} else {
				group_infect(g_group, 7, 10, h);
				sub_hero_le(h, dice_roll(h->illness[ill].days > 3 ? 3 : h->illness[ill].days, 6, 0));
				if (!(signed char)h->dead) {
					h->add_ill_kk(ill, 2);
					change_attrib(h, ATTRIB_KK, -2);
					if (h->illness[ill].days > 2) {
						rabies_effect(h, txt);
					}
				}
			}
		}
		if (!(signed char)h->dead && h->illness[ill].state == ILL_STATE_RECOVER) {
			if (!h->illness[ill].loss_kk) {
				h->set_ill_days(ill, 0);
				h->set_ill_state(ill, 0);
			} else {
				h->add_ill_kk(ill, -1);
				change_attrib(h, ATTRIB_KK, 1);
			}
		}
		for (k = 1; k <= 7; k++) {
			if (h->illness[k].state) {
				h->add_ill_days(k, 1);
			}
		}
		if (h->attrib[ATTRIB_KK].cur <= 0 || h->attrib[ATTRIB_GE].cur <= 0) {
			g_quiet = 1;
			for (k = 1; k <= 7; k++) {
				if (h->illness[k].loss_kk) {
					change_attrib(h, ATTRIB_KK, h->illness[k].loss_kk);
					h->add_ill_kk(k, -h->illness[k].loss_kk);
				}
				if (h->illness[k].loss_ge) {
					change_attrib(h, ATTRIB_GE, h->illness[k].loss_ge);
					h->add_ill_ge(k, -h->illness[k].loss_ge);
				}
			}
			g_quiet = 0;
			sub_hero_le(h, h->le);
		}
		if ((signed char)h->unconscious != was_unconscious && !(signed char)h->dead && g_silent_mode == 0) {
			sprintf(g_text_output_buf, txt[42], h->name);
			dlg_message(g_dialog, g_text_output_buf);
			if (g_pp20_index == 14) {
				g_refresh = 1;
			}
		}
		mem_free(txt_buf);
		mem_free(txt);
	}
}
