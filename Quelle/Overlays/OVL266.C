/* OVL266 - Overlay O266 (Segment 0x24cf in SCHWEIF.EXE): Kaempfe an festen Feldern des Dungeons 7 (zweite Ebene der
 * Orkfestung, Texte BLUT2.LTX ueber ev->txt); dng07_handler (O264) ruft dng07_fights nach jedem Feldereignis.
 * dng07_fights(ev, pos): je Feld bzw. Feldgruppe ein Erledigt-Merker (g_dng07_<gegner>_<pos>, g_dng07_beetles_<pos>,
 *   g_dng07_guards_beaten; die Patrouille an 0x30e/0x310/0x317 ueber g_ingame_timers[INGAME_TIMER_DNG07_PATROL] =
 *   0x2a30, kommt also wieder). Je nach g_dng07_alarm, sonst Tageszeit (DAY: g_daytime in (0x93a8, 0x190c8]) werden
 *   Gegnergruppen per Zufall (random_schweif(100) <= n) gewuerfelt bzw. fest vorgegeben (Orks 2/0x1b, Orkveteranen
 *   0x1a/0x48, Kriegshunde 0x5e, Schroeter 0x0b, Hoehlenspinnen 0x26/0x27) und ueber event_fight (drei Gruppen),
 *   event_fight2 oder event_fight1 gestartet; Rueckgabe 0 heisst gewonnen -> Merker setzen bzw. dng07_pincer_key
 *   (BLUT2[48], Zangenschluessel) / dng07_glasses_key (BLUT2[46], goldener Brillenschluessel). Rueckgabe 1 = Kampf
 *   hat stattgefunden.
 *
 * BCC -mlarge -3 -Ot -c -Y OVL266.C -> 2575/2575 Bytes GLEICH (18.10.2026), cobjfix laeuft (Namensfassung mit
 *   schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Reines C, eine lange if/else-if-Kette; toter Epilog am Ende kommt von selbst ("return ret").
 * - Register: pos als "register"-Parameter (SI), Gegnerzahl a OHNE register (bekommt automatisch DI); mit "register
 *   a" bekaeme a SI und pos DI, ganz ohne register landet b in DI.
 * - "a = b = c = 0" -> xor ax / mov [bp-4] (c) / mov [bp-2] (b) / mov di (a).
 * - g_ingame_timers[INGAME_TIMER_DNG07_PATROL] hat kein eigenes Public (cobjfix: g_246e+0x84).
 * Originalfehler: Feld 0x120f - nach dem g_dng07_alarm-Zweig fehlt das "else", der Tag/Nacht-Kampf folgt immer
 *   (zweiter Kampf, Rueckgabe des ersten wird ueberschrieben). Feld 0x124 setzt seinen Merker g_dng07_nest_destroyed
 *   nie und meldet 0 (Kampf wiederholbar); die Schroeter-Felder 0x1b14.., 0x1d17.., 0x171c.. und 0xa0d melden
 *   ebenfalls 0 trotz Kampf.
 */

#include "schweif.h"

#define DAY (g_daytime > 0x93a8 && g_daytime <= 0x190c8)

signed short dng07_fights(struct area_events far *ev, register signed short pos)
{
	signed short a;
	signed short b;
	signed short c;
	signed short ret;

	ret = 0;
	if (pos == 0x1606 && !g_dng07_orcs_1606) {
		a = b = c = 0;
		if (g_dng07_alarm) {
			if (random_schweif(100) <= 30) {
				a = 2;
				b = 1;
			}
			if (random_schweif(100) <= 5) c = 1;
		} else if (DAY) {
			if (random_schweif(100) <= 5) a = 1;
			if (random_schweif(100) <= 5) b = 2;
		} else {
			if (random_schweif(100) <= 5) a = 1;
			if (random_schweif(100) <= 10) b = 2;
			if (random_schweif(100) <= 5) c = 2;
		}
		if (a || b || c) {
			if (!event_fight(ev, 2, a, 0, 0, 0x1b, b, 0, 0, 0x48, c, 0, 0))
				g_dng07_orcs_1606 = 1;
			ret = 1;
		}
	} else if (pos == 0x0d05 && !g_dng07_guards_beaten) {
		if (g_dng07_alarm) {
			if (!event_fight(ev, 2, 1, 0, 0, 0x1b, 1, 0, 0, 0x5e, 2, 0, 0))
				g_dng07_guards_beaten = 1;
		} else if (DAY) {
			if (!event_fight1(ev, 0x1a, 2, 0))
				g_dng07_guards_beaten = 1;
		} else {
			if (!event_fight(ev, 2, 6, 0, 0, 0x1b, 3, 0, 0, 0x5e, 2, 0, 0))
				g_dng07_guards_beaten = 1;
		}
		ret = 1;
	} else if ((pos == 0x0d08 || pos == 0x0d0d || pos == 0x0d0f || pos == 0x0d19) && !g_dng07_orcs_0d08) {
		a = b = c = 0;
		if (g_dng07_alarm) {
			if (random_schweif(100) <= 50) {
				a = 2;
				b = 1;
			}
			if (random_schweif(100) <= 10) c = 2;
		} else if (DAY) {
			if (random_schweif(100) <= 20) {
				a = 1;
				b = 1;
			}
			if (random_schweif(100) <= 5) c = 2;
		} else {
			if (random_schweif(100) <= 10) a = 1;
			if (random_schweif(100) <= 10) b = 1;
			if (random_schweif(100) <= 5) c = 1;
		}
		if (a || b || c) {
			if (!event_fight(ev, 2, a, 0, 0, 0x1a, b, 0, 0, 1, c, 0, 0))
				g_dng07_orcs_0d08 = 1;
			ret = 1;
		}
	} else if (pos == 0x100b && !g_dng07_orcs_100b) {
		if (g_dng07_alarm) {
			if (!event_fight(ev, 0x48, 2, 0, 0, 0x1a, 2, 0, 0, 0x1b, 2, 1, 3))
				dng07_pincer_key((struct area_events far *)g_area_events);
		} else if (DAY) {
			if (!event_fight(ev, 2, 2, 0, 0, 0x48, 1, 0, 0, 0x1b, 2, 1, 3))
				dng07_pincer_key((struct area_events far *)g_area_events);
		} else {
			if (!event_fight(ev, 0x48, 1, 0, 0, 0x1a, 3, 0, 4, 2, 2, 1, 6))
				dng07_pincer_key((struct area_events far *)g_area_events);
		}
		ret = 1;
	} else if (pos == 0x140b && !g_dng07_orcs_140b) {
		if (g_dng07_alarm) {
			if (!event_fight(ev, 2, 2, 0, 0, 0x1a, 2, 0, 0, 0x1b, 3, 1, 3))
				g_dng07_orcs_140b = 1;
		} else if (DAY) {
			if (!event_fight(ev, 2, 2, 0, 0, 0x1a, 1, 0, 0, 0x1b, 2, 1, 3))
				g_dng07_orcs_140b = 1;
		} else {
			if (!event_fight(ev, 0x1b, 2, 0, 0, 0x1a, 2, 0, 4, 2, 2, 1, 6))
				g_dng07_orcs_140b = 1;
		}
		ret = 1;
	} else if (pos == 0x120f && !g_dng07_orcs_120f) {
#if FIX_0012
		/* Fix #12: else wie bei den Nachbarfeldern: bei Alarm nur ein Kampf */
		if (g_dng07_alarm) {
			if (!event_fight(ev, 2, 2, 0, 0, 0x1a, 2, 0, 0, 0x1b, 3, 1, 3))
				g_dng07_orcs_120f = 1;
		} else if (DAY) {
#else
		/* Originalfehler: hier fehlt das "else" (Nacht-Pruefung folgt immer) */
		if (g_dng07_alarm) {
			if (!event_fight(ev, 2, 2, 0, 0, 0x1a, 2, 0, 0, 0x1b, 3, 1, 3))
				g_dng07_orcs_120f = 1;
		}
		if (DAY) {
#endif
			if (!event_fight(ev, 2, 2, 0, 0, 0x1a, 1, 0, 0, 0x1b, 2, 1, 3))
				g_dng07_orcs_120f = 1;
		} else {
			if (!event_fight(ev, 0x1b, 2, 0, 0, 0x1a, 2, 0, 4, 2, 2, 1, 6))
				g_dng07_orcs_120f = 1;
		}
		ret = 1;
	} else if ((pos == 0x1b16 || pos == 0x1b15 || pos == 0x1b14) && !g_dng07_beetles_1b15) {
#if FIX_0013
		/* Fix #13: Kampf melden (ret = 1) */
		if (!event_fight1(ev, 0x0b, 2, 0))
			g_dng07_beetles_1b15 = 1;
		ret = 1;
#else
		if (!event_fight1(ev, 0x0b, 2, 0))
			g_dng07_beetles_1b15 = 1;
#endif
	} else if ((pos == 0x1d17 || pos == 0x1e17 || pos == 0x1d18 || pos == 0x1d19) && !g_dng07_beetles_1d17) {
#if FIX_0013
		/* Fix #13: Kampf melden (ret = 1) */
		if (!event_fight1(ev, 0x0b, 2, 0))
			g_dng07_beetles_1d17 = 1;
		ret = 1;
#else
		if (!event_fight1(ev, 0x0b, 2, 0))
			g_dng07_beetles_1d17 = 1;
#endif
	} else if ((pos == 0x171c || pos == 0x181c || pos == 0x181d) && !g_dng07_beetles_181c) {
#if FIX_0013
		/* Fix #13: Kampf melden (ret = 1) */
		if (!event_fight1(ev, 0x0b, 2, 0))
			g_dng07_beetles_181c = 1;
		ret = 1;
#else
		if (!event_fight1(ev, 0x0b, 2, 0))
			g_dng07_beetles_181c = 1;
#endif
	} else if (pos == 0x0a0d && !g_dng07_orcs_0a0d) {
		if (g_dng07_alarm) {
			if (!event_fight(ev, 0x1a, 2, 0, 0, 0x1b, 2, 0, 0, 0x48, 1, 0, 0))
				dng07_glasses_key((struct area_events far *)g_area_events);
		} else {
#if FIX_0013
			/* Fix #13: Kampf melden (ret = 1) */
			if (!event_fight2(ev, 0x1a, 2, 0, 0x1b, 2, 0))
				dng07_glasses_key((struct area_events far *)g_area_events);
		}
		ret = 1;
#else
			if (!event_fight2(ev, 0x1a, 2, 0, 0x1b, 2, 0))
				dng07_glasses_key((struct area_events far *)g_area_events);
		}
#endif
	} else if ((pos == 0x050f || pos == 0x0c0f) && !g_dng07_orcs_050f) {
		a = b = c = 0;
		if (g_dng07_alarm) {
			if (random_schweif(100) <= 50) {
				a = 2;
				b = 1;
			}
			if (random_schweif(100) <= 10) c = 2;
		} else if (DAY) {
			if (random_schweif(100) <= 5) a = 1;
			if (random_schweif(100) <= 5) b = 2;
		} else {
			if (random_schweif(100) <= 50) b = 1;
		}
		if (a || b || c) {
			if (!event_fight(ev, 0x1a, a, 0, 0, 0x1b, b, 0, 0, 1, c, 0, 0))
				g_dng07_orcs_050f = 1;
			ret = 1;
		}
	} else if ((pos == 0x030e || pos == 0x0310 || pos == 0x0317) && g_ingame_timers[INGAME_TIMER_DNG07_PATROL] == 0) {
		a = b = c = 0;
		if (g_dng07_alarm) {
			if (random_schweif(100) <= 50) {
				a = 3;
				c = 1;
			}
			if (random_schweif(100) <= 10) c = 2;
		} else if (DAY) {
			if (random_schweif(100) <= 20) {
				a = 1;
				b = 1;
			}
			if (random_schweif(100) <= 5) c = 2;
		} else {
			if (random_schweif(100) <= 10) a = 1;
			if (random_schweif(100) <= 10) b = 1;
			if (random_schweif(100) <= 5) c = 1;
		}
		if (a || b || c) {
			if (!event_fight(ev, 2, a, 0, 0, 0x1a, b, 0, 0, 0x1b, c, 0, 0))
				g_ingame_timers[INGAME_TIMER_DNG07_PATROL] = 0x2a30;
			ret = 1;
		}
	} else if (pos == 0x0305 && !g_dng07_orcs_0305) {
		if (!event_fight2(ev, 0x1a, 2, 0, 0x48, 1, 0))
			g_dng07_orcs_0305 = 1;
		ret = 1;
	} else if ((pos == 0x0319 || pos == 0x051b) && !g_dng07_orcs_0319) {
		if (g_dng07_alarm || DAY) {
			if (!event_fight2(ev, 2, 2, 0, 0x1a, 1, 0))
				g_dng07_orcs_0319 = 1;
			ret = 1;
		}
	} else if (pos == 0x0124 && !g_dng07_nest_destroyed) {
#if FIX_0013
		/* Fix #13: Kampf melden (ret = 1); die Spinnen kommen weiter nach, bis das Nest an 0x225 zerstoert ist */
		event_fight2(ev, 0x26, 2, 0, 0x27, 3, 0);
		ret = 1;
#else
		event_fight2(ev, 0x26, 2, 0, 0x27, 3, 0);
#endif
	}
	return ret;
}

void dng07_pincer_key(struct area_events far *ev)
{
	g_dng07_orcs_100b = 1;
	/* BLUT2.LTX[48]: "Als Ihr den letzten Gegner erledigt habt, entdeckt Ihr, dass einer der Orks einen wirklich
	 * seltsam aussehenden Schluessel verloren hat, den Ihr schnell an Euch nehmt." */
	dlg_message(g_dialog, ev->txt[0x30]);
	group_give_item(g_group, ITEM_PINCER_KEY, 1);
}

void dng07_glasses_key(struct area_events far *ev)
{
	/* BLUT2.LTX[46]: "Als Ihr den letzten Gegner niedergestreckt habt, entdeckt Ihr, dass einer der Orks einen
	 * Schluessel verloren hat, den Ihr schnell an Euch nehmt." */
	dlg_message(g_dialog, ev->txt[0x2e]);
	group_give_item(g_group, ITEM_GLASSES_KEY, 1);
	g_dng07_orcs_0a0d = 1;
}
