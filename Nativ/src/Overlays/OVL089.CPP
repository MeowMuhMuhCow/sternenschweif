/* OVL089 - Overlay O089 (Segment 0x2112 in SCHWEIF.EXE): Giftwirkung eines Helden (wie poison_effect in DSA 1
 * seg121, hier je Held aus hero_timers R10 aufgerufen). poison_effect(h): nur fuer lebende Helden; Bit
 * unconscious vorher/nachher merken. Gifte h->poison[1..9] (struct hero_ill: Zustand -1 vergiftet / 1 Erholung,
 * Dauer, KK-/AE-Verlust, GE-Verlust, Bit "Wirkung"): 1 Shurinknolle (W6 LE, ab Dauer 6 KK/GE -1 ueber
 * change_attrib), 2 Arax (ab 2: KK/GE -2, Text txt[4]; ab 120 Erholung, txt[5]), 3 Angstgift (MU -2, negative
 * Eigenschaften +2, im Kampf Bit scared, txt[6]/[7]), 4 Schlafgift (set_asleep, txt[8]/[9]), 5 Goldleim,
 * 6 Kroetenschemel, 7 Lotus, 8 Kukris (Schaden dice_roll(n, 6, m) ueber sub_hero_le), 9 Bannstaub (AE-Verlust
 * 1W6+2 ueber sub_ae_splash, txt[10]; Erholung je 12 Ticks +1 AE add_hero_ae, txt[11]). Danach Dauer aller
 * aktiven Gifte +1; KK oder GE <= 0 -> alle Verluste zurueck (g_quiet = 1 unterdrueckt die Meldungen) und Tod
 * (sub_hero_le mit LE). Texte jeweils frisch mit txt_load_list(g_font, 0x81, 0x32) geladen und wieder
 * freigegeben. Hat sich Bit unconscious geaendert: txt[42], g_refresh = 1 bei Bild 14.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL089.CPP   -> 5253/5253 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * - C++ wegen der inline-Funktionen hero::set_poi_state/set_poi_days/set_poi_effect und add_poi_days/
 *   add_poi_kk/add_poi_ge ("+= v; if (< 0) = 0; else ;"): jede Expansion hat eine eigene Kopie des Index
 *   ([bp-0eh]..[bp-60h]), v nur wenn nicht konstant ([bp-53h], [bp-5dh], [bp-61h]); konstante v ergeben
 *   inc/dec/add byte es:[..],imm (+1 inc, -1 dec, +-2 add). add_* hat je Expansion zwei eigene -Og-Zwischenwerte
 *   (poi*5 und Offset), das "else ;" den toten jmp short.
 * - -Og: [bp-62h] Byte +bc, [bp-64h] Bitwert "!(signed char)h->dead" (int), [bp-66h] poi*5 + DI = &h->poison[poi];
 *   [bp-90h]/[bp-92h] in den Schleifen. Kein register noetig (poi SI, k [bp-2], was_unconscious [bp-3]).
 * - Bit "Wirkung" (effect, wie alle Bitfelder unsigned short): "!(signed char)effect" -> and/cbw/or ax,ax;
 *   "if ((signed char)effect)" -> and ax,1/or al,al (frueher "unsigned char e0:1" ohne Cast; ohne Cast jetzt or ax,ax).
 * - Schluss "(unsigned char)h->unconscious != was_unconscious" -> and ax,1 / cmp al,[bp-3]; ohne Cast
 *   int-Vergleich (dl, cmp ax,dx).
 * Originalfehler: Shurinknollen-Erholung prueft "!h->poison[poi].days % 12" statt "!(days % 12)" (wie DSA 1) -
 *   KK/GE kommen nur zurueck, wenn der Dauerzaehler 0 ist, also praktisch nur nach dessen Ueberlauf (add_poi_days
 *   klemmt -128 auf 0, d. h. alle 128 Ticks).
 */
#include <stdio.h>
#include "schweif.h"

void poison_effect(struct hero far *h)
{
	signed short poi;			/* Gift POISON_* */
	signed short k;				/* Schleifenzaehler bzw. Bannstaub-AE-Verlust */
	unsigned char was_unconscious;
	void far *txt_buf;
	char far * far *txt;

	if (!(signed char)h->dead) {
		was_unconscious = h->unconscious;

		poi = POISON_SHURINKNOLLE;
		if (!(signed char)h->dead && h->poison[poi].state == ILL_STATE_SICK) {
			sub_hero_le(h, random_schweif(6));
			if (h->poison[poi].days >= 6) {
				if (!(signed char)h->dead && h->attrib[ATTRIB_KK].cur != 0) {
					h->add_poi_kk(poi, 1);
					change_attrib(h, ATTRIB_KK, -1);
				}
				if (!(signed char)h->dead && h->attrib[ATTRIB_GE].cur != 0) {
					h->add_poi_ge(poi, 1);
					change_attrib(h, ATTRIB_GE, -1);
				}
			}
		}
		if (!(signed char)h->dead && h->poison[poi].state == ILL_STATE_RECOVER) {
			if (!h->poison[poi].loss_kk && !h->poison[poi].loss_ge) {
				h->set_poi_days(poi, 0);
				h->set_poi_state(poi, 0);
			}
#if FIX_0138
			/* Fix #138: Praezedenz: !(days % 12) */
			if (h->poison[poi].loss_kk != 0 && !(h->poison[poi].days % 12)) {
#else
			if (h->poison[poi].loss_kk != 0 && !h->poison[poi].days % 12) {	/* Originalfehler */
#endif
				h->add_poi_kk(poi, -1);
				change_attrib(h, ATTRIB_KK, 1);
			}
#if FIX_0138
			/* Fix #138: Praezedenz: !(days % 12) */
			if (h->poison[poi].loss_ge != 0 && !(h->poison[poi].days % 12)) {
#else
			if (h->poison[poi].loss_ge != 0 && !h->poison[poi].days % 12) {
#endif
				h->add_poi_ge(poi, -1);
				change_attrib(h, ATTRIB_GE, 1);
			}
		}

		poi = POISON_ARAX;
		if (!(signed char)h->dead && h->poison[poi].state == ILL_STATE_SICK) {
			if (h->poison[poi].days >= 2 && !(signed char)h->poison[poi].effect) {
				h->set_poi_effect(poi, 1);
				if (!(signed char)h->dead && h->attrib[ATTRIB_KK].cur != 0) {
					h->add_poi_kk(poi, 2);
					change_attrib(h, ATTRIB_KK, -2);
				}
				if (!(signed char)h->dead && h->attrib[ATTRIB_GE].cur != 0) {
					h->add_poi_ge(poi, 2);
					change_attrib(h, ATTRIB_GE, -2);
				}
				txt_load_list(g_font, 0x81, 0x32, &txt_buf, &txt);
				sprintf(g_text_output_buf, txt[4], h->name);
				dlg_message(g_dialog, g_text_output_buf);
				mem_free(txt_buf);
				mem_free(txt);
			}
			if (h->poison[poi].days >= 120)
				h->set_poi_state(poi, ILL_STATE_RECOVER);
		}
		if (!(signed char)h->dead && h->poison[poi].state == ILL_STATE_RECOVER) {
			h->set_poi_state(poi, 0);
			h->set_poi_days(poi, 0);
			if ((signed char)h->poison[poi].effect) {
				h->set_poi_effect(poi, 0);
				h->add_poi_ge(poi, -2);
				change_attrib(h, ATTRIB_GE, 2);
				h->add_poi_kk(poi, -2);
				change_attrib(h, ATTRIB_KK, 2);
				txt_load_list(g_font, 0x81, 0x32, &txt_buf, &txt);
				sprintf(g_text_output_buf, txt[5], h->name);
				dlg_message(g_dialog, g_text_output_buf);
				mem_free(txt_buf);
				mem_free(txt);
			}
		}

		poi = POISON_ANGSTGIFT;
		if (!(signed char)h->dead && h->poison[poi].state == ILL_STATE_SICK) {
			if (!(signed char)h->poison[poi].effect) {
				h->set_poi_effect(poi, 1);
				change_attrib(h, ATTRIB_MU, -2);
				change_attrib(h, ATTRIB_AG, 2);
				change_attrib(h, ATTRIB_HA, 2);
				change_attrib(h, ATTRIB_RA, 2);
				change_attrib(h, ATTRIB_GG, 2);
				change_attrib(h, ATTRIB_TA, 2);
				change_attrib(h, ATTRIB_NG, 2);
				change_attrib(h, ATTRIB_JZ, 2);
				if (g_in_fight)
					h->scared = 1;
				txt_load_list(g_font, 0x81, 0x32, &txt_buf, &txt);
				sprintf(g_text_output_buf, txt[6], h->name);
				dlg_message(g_dialog, g_text_output_buf);
				mem_free(txt_buf);
				mem_free(txt);
			}
			if (h->poison[poi].days >= 7)
				h->set_poi_state(poi, ILL_STATE_RECOVER);
		}
		if (!(signed char)h->dead && h->poison[poi].state == ILL_STATE_RECOVER) {
			h->set_poi_state(poi, 0);
			h->set_poi_days(poi, 0);
			if ((signed char)h->poison[poi].effect) {
				h->set_poi_effect(poi, 0);
				change_attrib(h, ATTRIB_MU, 2);
				change_attrib(h, ATTRIB_AG, -2);
				change_attrib(h, ATTRIB_HA, -2);
				change_attrib(h, ATTRIB_RA, -2);
				change_attrib(h, ATTRIB_GG, -2);
				change_attrib(h, ATTRIB_TA, -2);
				change_attrib(h, ATTRIB_NG, -2);
				change_attrib(h, ATTRIB_JZ, -2);
				h->scared = 0;
				txt_load_list(g_font, 0x81, 0x32, &txt_buf, &txt);
				sprintf(g_text_output_buf, txt[7], hero_name_gen(h, 1));
				dlg_message(g_dialog, g_text_output_buf);
				mem_free(txt_buf);
				mem_free(txt);
			}
		}

		poi = POISON_SCHLAFGIFT;
		if (!(signed char)h->dead && h->poison[poi].state == ILL_STATE_SICK) {
			if (!(signed char)h->poison[poi].effect) {
				h->set_poi_effect(poi, 1);
				set_asleep(h, 1);
				txt_load_list(g_font, 0x81, 0x32, &txt_buf, &txt);
				sprintf(g_text_output_buf, txt[8], h->name);
				dlg_message(g_dialog, g_text_output_buf);
				mem_free(txt_buf);
				mem_free(txt);
			}
			if (h->poison[poi].days >= 20)
				h->set_poi_state(poi, ILL_STATE_RECOVER);
		}
		if (!(signed char)h->dead && h->poison[poi].state == ILL_STATE_RECOVER) {
			h->set_poi_state(poi, 0);
			h->set_poi_days(poi, 0);
			if ((signed char)h->poison[poi].effect) {
				h->set_poi_effect(poi, 0);
				set_asleep(h, 0);
				txt_load_list(g_font, 0x81, 0x32, &txt_buf, &txt);
				sprintf(g_text_output_buf, txt[9], h->name);
				dlg_message(g_dialog, g_text_output_buf);
				mem_free(txt_buf);
				mem_free(txt);
			}
		}

		poi = POISON_GOLDLEIM;
		if (!(signed char)h->dead && h->poison[poi].state == ILL_STATE_SICK) {
			if (h->poison[poi].days >= dice_roll(2, 6, 0) * 12)
				h->set_poi_state(poi, 0);
			if (!(h->poison[poi].days % 12))
				sub_hero_le(h, dice_roll(2, 6, -3));
		}
		if (h->poison[poi].state == ILL_STATE_RECOVER)
			h->set_poi_state(poi, 0);

		poi = POISON_KROETENSCHEMEL;
		if (!(signed char)h->dead && h->poison[poi].state == ILL_STATE_SICK) {
			if (h->poison[poi].days >= 48)
				h->set_poi_state(poi, 0);
			if (!(h->poison[poi].days % 12))
				sub_hero_le(h, dice_roll(1, 6, 2));
		}
		if (h->poison[poi].state == ILL_STATE_RECOVER)
			h->set_poi_state(poi, 0);

		poi = POISON_LOTUSGIFT;
		if (!(signed char)h->dead && h->poison[poi].state == ILL_STATE_SICK) {
			if (h->poison[poi].days >= 24)
				h->set_poi_state(poi, 0);
			sub_hero_le(h, dice_roll(2, 6, 0));
		}
		if (h->poison[poi].state == ILL_STATE_RECOVER)
			h->set_poi_state(poi, 0);

		poi = POISON_KUKRIS;
		if (!(signed char)h->dead && h->poison[poi].state == ILL_STATE_SICK) {
			if (h->poison[poi].days >= 3)
				h->set_poi_state(poi, 0);
			sub_hero_le(h, dice_roll(50, 6, 0));
		}
		if (h->poison[poi].state == ILL_STATE_RECOVER)
			h->set_poi_state(poi, 0);

		poi = POISON_BANNSTAUB;
		if (!(signed char)h->dead && h->poison[poi].state == ILL_STATE_SICK) {
			if (h->poison[poi].days >= dice_roll(3, 6, 0))
				h->set_poi_state(poi, ILL_STATE_RECOVER);
			k = dice_roll(1, 6, 2);
			if (h->ae >= k) {
				h->add_poi_kk(poi, k);		/* Bannstaub: loss_kk zaehlt die abgezogene AE */
				sub_ae_splash(h, k);
				txt_load_list(g_font, 0x81, 0x32, &txt_buf, &txt);
				sprintf(g_text_output_buf, txt[10], h->name, k);
				dlg_message(g_dialog, g_text_output_buf);
				mem_free(txt_buf);
				mem_free(txt);
			}
		}
		if (!(signed char)h->dead && h->poison[poi].state == ILL_STATE_RECOVER) {
			if (!h->poison[poi].loss_kk) {
				h->set_poi_state(poi, 0);
			} else if (!(h->poison[poi].days % 12)) {
				h->add_poi_kk(poi, -1);
				add_hero_ae(h, 1);
				txt_load_list(g_font, 0x81, 0x32, &txt_buf, &txt);
				sprintf(g_text_output_buf, txt[11], hero_name_gen(h, 1));
				dlg_message(g_dialog, g_text_output_buf);
				mem_free(txt_buf);
				mem_free(txt);
			}
		}

		/* Dauer aller aktiven Gifte weiterzaehlen */
		for (k = 1; k <= 9; k++) {
			if (h->poison[k].state != 0)
				h->add_poi_days(k, 1);
		}

		/* KK oder GE erschoepft: alle Verluste zuruecknehmen, Held stirbt */
		if (h->attrib[ATTRIB_KK].cur <= 0 || h->attrib[ATTRIB_GE].cur <= 0) {
			g_quiet = 1;
			for (k = 1; k <= 9; k++) {
				if (k == POISON_ANGSTGIFT && (signed char)h->poison[k].effect) {
					change_attrib(h, ATTRIB_MU, 2);
					change_attrib(h, ATTRIB_AG, -2);
					change_attrib(h, ATTRIB_HA, -2);
					change_attrib(h, ATTRIB_RA, -2);
					change_attrib(h, ATTRIB_GG, -2);
					change_attrib(h, ATTRIB_TA, -2);
					change_attrib(h, ATTRIB_NG, -2);
					change_attrib(h, ATTRIB_JZ, -2);
				} else {
					if (h->poison[k].loss_kk != 0 && k != POISON_BANNSTAUB) {
						change_attrib(h, ATTRIB_KK, h->poison[k].loss_kk);
						h->add_poi_kk(k, -h->poison[k].loss_kk);
					}
					if (h->poison[k].loss_ge != 0) {
						change_attrib(h, ATTRIB_GE, h->poison[k].loss_ge);
						h->add_poi_ge(k, -h->poison[k].loss_ge);
					}
				}
			}
			g_quiet = 0;
			sub_hero_le(h, h->le);
		}

		if ((unsigned char)h->unconscious != was_unconscious && g_silent_mode == 0 && !(signed char)h->dead) {
			txt_load_list(g_font, 0x81, 0x32, &txt_buf, &txt);
			sprintf(g_text_output_buf, txt[42], h->name);
			dlg_message(g_dialog, g_text_output_buf);
			mem_free(txt_buf);
			mem_free(txt);
			if (g_pp20_index == 0xe)
				g_refresh = 1;
		}
	}
}
