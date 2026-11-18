/* OVL268 - Overlay O268 (Segment 0x24d5 in SCHWEIF.EXE): Feldereignisse des Dungeons 8 (dritte Ebene der
 * Orkfestung, Texte BLUT3.LTX ueber ev->txt); Unterfunktionen von dng08_handler (O267).
 * dng08_door(ev, pos, x, y, mev): Schloesser (Lock 0x4a..0x6b je Position, Wandbild 0x70/0x71), bei g_lock_cell_bits
 *   != -1 Feldbits, Automap, Klang 0x61.
 * dng08_fights(ev, pos): Kaempfe an festen Feldern (Merker g_dng08_<gegner>_<pos>); Gegnerzahlen je nach g_dng08_alarm,
 *   sonst Tag/Nacht (DAY: g_daytime in (0x93a8, 0x190c8]), teils per random_schweif(100) gewuerfelt; event_fight
 *   (drei Gruppen), event_fight2, event_fight1; Rueckgabe 1 = Kampf. In der grossen Halle (0x70d..0x915) der
 *   Hallenkampf (7 Orks, 7 Orkveteranen, 5 x 0x5b; setzt g_dng08_alarm = 1); vor dem geoeffneten Sarkophag die
 *   Zwergenmumie (0x5f).
 * dng08_shelves(ev, pos): Regale und Vorraete (Ablagen mit Fragen [17..20], [67..74]); Fackelregal [49]; Kornsaecke
 *   an 0x1e0d..0x1e10 anstechen ([30]/[31], danach [32]); Plunder [45] + 10 Minuten. Rueckgabe 1, wenn etwas genommen
 *   wurde (Stash::menu).
 * dng08_pit(ev, st): Klappgrube - 0 bemerken (Sinnesschaerfe +4, [54]); 1 warnen ([55]) bzw. hineinfallen (*st = 2,
 *   [56], 3..12 LE fuer alle, 10 Minuten); sonst behutsam umgehen ([57], 5 Minuten).
 *
 * BCC -mlarge -3 -Ot -c -Y OVL268.CPP -> 5631/5631 Bytes GLEICH (18.10.2026), cobjfix laeuft (mit -Og ebenso)
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - C++ wegen lokalem Lock-Objekt (O108) und new/delete der Stash-Ablage (Stash_ctor(0, id, 1) / Dtor(o, 3) ohne
 *   Nulltest).
 * - automap_cell_write ueber AUTOMAP_CELL_WRITE_W (letzter Parameter short, wie OVL063).
 * - dng08_door: Initialisierung in Deklarationsreihenfolge v (DI), r ([bp-2]), id (SI) - ohne register.
 * - dng08_fights: a/b/c/ret am Anfang einmal initialisiert (SI, DI, [bp-2], [bp-4]); Feldgruppe 0x070d..0x0915: der
 *   Merkertest !g_dng08_hall_fight steht IM then-Block (jnz auf das jmp near ans Blockende), sonst && in der
 *   Bedingung.
 * - dng08_pit: "if (!*st) { ..; return; }" -> Epilogkopien (auch ohne -Og).
 * Originalfehler/Auffaelligkeiten: dng08_fights - a/b/c werden nur einmal am Anfang genullt (unkritisch, da pos nur
 *   einen Block trifft); Feld 0x1516/0x1615 prueft c, uebergibt c aber nicht (dritte Gruppe fest -1/0); die Felder
 *   0x70d.. (g_dng08_hall_fight), 0x1907/0x190a (g_dng08_orc_veteran_1907) und die Gruppe 0x1c0b.. (Mumie) melden 0 trotz
 *   Kampf.
 */
#include <stdio.h>

#include "schweif.h"

/* automap_cell_write mit short als letztem Parameter (wie im Original deklariert) */
#define AUTOMAP_CELL_WRITE_W \
	((void (*)(void far *, signed short, signed short, signed short, signed short))automap_cell_write)

#define DAY (g_daytime > 0x93a8 && g_daytime <= 0x190c8)

signed short dng08_door(struct area_events far *ev, signed short pos, signed short x, signed short y,
	struct map_event far *mev)
{
	signed short v = 0;
	signed short r = 0;
	signed short id = -1;

	switch (pos) {
	case 0x1e16: id = 0x4a; v = 0x70; break;
	case 0x1e12: id = 0x4b; v = 0x71; break;
	case 0x1c0c: id = 0x4c; v = 0x70; break;
	case 0x1b0f: id = 0x4d; v = 0x71; break;
	case 0x1b11: id = 0x4e; v = 0x71; break;
	case 0x1614: id = 0x4f; v = 0x70; break;
	case 0x1517: id = 0x50; v = 0x70; break;
	case 0x181b: id = 0x51; v = 0x70; break;
	case 0x1116: id = 0x52; v = 0x71; break;
	case 0x111b: id = 0x53; v = 0x71; break;
	case 0x0f19: id = 0x54; v = 0x71; break;
	case 0x1013: id = 0x55; v = 0x71; break;
	case 0x1010: id = 0x56; v = 0x71; break;
	case 0x0f0d: id = 0x57; v = 0x70; break;
	case 0x0e0f: id = 0x58; v = 0x71; break;
	case 0x0c14: id = 0x59; v = 0x70; break;
	case 0x0b15: id = 0x5a; v = 0x71; break;
	case 0x0717: id = 0x5b; v = 0x70; break;
	case 0x0518: id = 0x5c; v = 0x71; break;
	case 0x0818: id = 0x5d; v = 0x71; break;
	case 0x071a: id = 0x5e; v = 0x70; break;
	case 0x0515: id = 0x5f; v = 0x71; break;
	case 0x050d: id = 0x60; v = 0x71; break;
	case 0x030b: id = 0x61; v = 0x70; break;
	case 0x080b: id = 0x62; v = 0x70; break;
	case 0x0a05: id = 0x63; v = 0x70; break;
	case 0x0d05: id = 0x64; v = 0x70; break;
	case 0x0b02: id = 0x65; v = 0x71; break;
	case 0x1309: id = 0x66; v = 0x70; break;
	case 0x1906: id = 0x67; v = 0x70; break;
	case 0x1403: id = 0x68; v = 0x71; break;
	case 0x1703: id = 0x69; v = 0x71; break;
	case 0x1a05: id = 0x6a; v = 0x71; break;
	case 0x1107: id = 0x6b; v = 0x71; break;
	}
	if (id != -1) {
		Lock lk(id);
		r = 1;
		if (g_lock_cell_bits != -1) {
			event_set_cell_bits(ev, x, y, g_lock_cell_bits);
			AUTOMAP_CELL_WRITE_W(g_map, 0x37f, mev->y - g_map_x0, mev->x - g_map_y0, v);
			snd_play(g_snd_fx, 0x61);
		}
	}
	return r;
}

signed short dng08_fights(struct area_events far *ev, signed short pos)
{
	signed short a = 0;
	signed short b = 0;
	signed short c = 0;
	signed short ret = 0;

	if (pos == 0x1e19 && !g_dng08_orcs_1e19) {
		if (g_dng08_alarm) {
			if (random_schweif(100) <= 50) {
				a = 2;
				b = 1;
			}
			if (random_schweif(100) <= 10) c = 2;
		} else if (DAY) {
			if (random_schweif(100) <= 20) {
				a = 2;
				b = 1;
			}
			if (random_schweif(100) <= 5) c = 1;
		} else {
			if (random_schweif(100) <= 10) a = 1;
			if (random_schweif(100) <= 10) b = 1;
			if (random_schweif(100) <= 5) c = 1;
		}
		if (a || b || c) {
			if (!event_fight(ev, 2, a, 0, 0, 0x1b, b, 0, 0, 0x5c, c, 0, 0)) g_dng08_orcs_1e19 = 1;
			ret = 1;
		}
	} else if (pos == 0x1e15 && !g_dng08_orcs_1e15) {
		if (g_dng08_alarm) {
			if (random_schweif(100) <= 50) {
				a = 2;
				b = 1;
			}
			if (random_schweif(100) <= 10) c = 2;
		} else if (DAY) {
			if (random_schweif(100) <= 20) {
				a = 2;
				b = 1;
			}
			if (random_schweif(100) <= 5) c = 1;
		} else {
			if (random_schweif(100) <= 10) a = 1;
			if (random_schweif(100) <= 10) b = 1;
			if (random_schweif(100) <= 5) c = 1;
		}
		if (a || b || c) {
			if (!event_fight(ev, 2, a, 0, 0, 0x1b, b, 0, 0, 0x5c, c, 0, 0)) g_dng08_orcs_1e15 = 1;
			ret = 1;
		}
	}
	if (pos == 0x1a11 && !g_dng08_orcs_1a11) {
		if (g_dng08_alarm) {
			a = 3;
			b = 5;
			c = 1;
		} else if (DAY) {
			a = 2;
			b = 2;
			c = 1;
		} else {
			a = 1;
			b = 1;
			c = 1;
		}
		if (a || b || c) {
			if (!event_fight(ev, 2, a, 0, 0, 0x1b, b, 0, 0, 0x5c, c, 0, 0)) g_dng08_orcs_1a11 = 1;
			ret = 1;
		}
	}
	if ((pos == 0x1516 || pos == 0x1615) && !g_dng08_orcs_1516) {
		if (g_dng08_alarm) {
			a = 4;
		} else if (DAY) {
			a = 2;
			b = 1;
		} else {
			a = 2;
			b = 2;
		}
		if (a || b || c) {
			if (!event_fight(ev, 0x48, a, 0, 0, 0x48, b, 0, 5, -1, 0, 0, 0)) g_dng08_orcs_1516 = 1;
			ret = 1;
		}
	}
	if (pos == 0x121b && !g_dng08_orcs_121b) {
		if (g_dng08_alarm) {
			a = 2;
			b = 4;
			c = 2;
		} else if (DAY) {
			a = 2;
			b = 2;
			c = 4;
		} else {
			a = 1;
			b = 1;
			c = 3;
		}
		if (a || b || c) {
			if (!event_fight(ev, 2, a, 0, 0, 0x1b, b, 0, 0, 0x5c, c, 0, 4)) g_dng08_orcs_121b = 1;
			ret = 1;
		}
	}
	if (pos == 0x0e19 && !g_dng08_ogres_0e19) {
		if (g_dng08_alarm) {
			a = 4;
			b = 3;
		} else if (DAY) {
			a = 3;
			b = 0;
			c = 1;
		} else {
			a = 2;
			b = 1;
			c = 4;
		}
		if (a || b || c) {
			if (!event_fight(ev, 1, a, 0, 0, 2, b, 0, 0, 0x5c, c, 0, 5)) g_dng08_ogres_0e19 = 1;
			ret = 1;
		}
	}
	if (pos == 0x1113 && !g_dng08_orcs_1113) {
		if (g_dng08_alarm) {
			a = 3;
			b = 1;
		} else if (DAY) {
			a = 2;
			b = 0;
			c = 1;
		} else {
			a = 1;
			b = 1;
			c = 2;
		}
		if (a || b || c) {
			if (!event_fight(ev, 0x1b, a, 0, 0, 2, b, 0, 0, 0x1b, c, 0, 5)) g_dng08_orcs_1113 = 1;
			ret = 1;
		}
	}
	if (pos == 0x1110 && !g_dng08_orcs_1110) {
		if (g_dng08_alarm) {
			a = 3;
			b = 1;
		} else if (DAY) {
			a = 2;
			b = 0;
			c = 1;
		} else {
			a = 1;
			b = 1;
			c = 2;
		}
		if (a || b || c) {
			if (!event_fight(ev, 0x1b, a, 0, 0, 2, b, 0, 0, 0x1b, c, 0, 5)) g_dng08_orcs_1110 = 1;
			ret = 1;
		}
	}
	if (pos == 0x0d0f && !g_dng08_orcs_0d0f) {
		if (g_dng08_alarm) {
			a = 3;
			b = 1;
		} else if (DAY) {
			a = 2;
			b = 0;
			c = 1;
		} else {
			a = 1;
			b = 1;
			c = 2;
		}
		if (a || b || c) {
			if (!event_fight(ev, 0x1b, a, 0, 0, 2, b, 0, 0, 0x1b, c, 0, 5)) g_dng08_orcs_0d0f = 1;
			ret = 1;
		}
	}
	if (pos == 0x0f0c && !g_dng08_orcs_0f0c) {
		if (g_dng08_alarm) {
			a = 2;
			b = 2;
		} else if (DAY) {
			a = 2;
			b = 1;
			c = 1;
		} else {
			a = 1;
			b = 2;
			c = 2;
		}
		if (a || b || c) {
			if (!event_fight(ev, 0x1b, a, 0, 0, 2, b, 0, 0, 0x1b, c, 0, 5)) g_dng08_orcs_0f0c = 1;
			ret = 1;
		}
	} else if ((pos == 0x070d || pos == 0x090d || pos == 0x060e || pos == 0x070e || pos == 0x090e ||
		pos == 0x0a0e || pos == 0x0611 || pos == 0x0614 || pos == 0x0714 || pos == 0x0715 ||
		pos == 0x0914 || pos == 0x0a14 || pos == 0x0915)) {
		if (!g_dng08_hall_fight) {
			g_dng08_alarm = 1;
#if FIX_0157
			/* Fix #157: Kampf melden (ret = 1) */
			if (!event_fight(ev, 0x1b, 7, 0, 0, 0x48, 7, 0, 0, 0x5b, 5, 0, 0)) g_dng08_hall_fight = 1;
			ret = 1;
#else
			if (!event_fight(ev, 0x1b, 7, 0, 0, 0x48, 7, 0, 0, 0x5b, 5, 0, 0)) g_dng08_hall_fight = 1;
#endif
		}
	} else if ((pos == 0x0511 || pos == 0x0515) && !g_dng08_orcs_0511) {
		if (g_dng08_alarm) {
			if (random_schweif(100) <= 50) {
				a = 2;
				b = 2;
			}
			if (random_schweif(100) <= 10) c = 2;
		} else if (DAY) {
			if (random_schweif(100) <= 20) {
				a = 2;
				b = 1;
			}
			if (random_schweif(100) <= 5) c = 1;
		} else {
			if (random_schweif(100) <= 10) a = 1;
			if (random_schweif(100) <= 10) b = 1;
			if (random_schweif(100) <= 5) c = 1;
		}
		if (a || b || c) {
			if (!event_fight(ev, 2, a, 0, 0, 0x1b, b, 0, 0, 0x5c, c, 0, 0)) g_dng08_orcs_0511 = 1;
			ret = 1;
		}
	}
	if ((pos == 0x020d || pos == 0x040d || pos == 0x020c) && !g_dng08_orcs_020d) {
		if (!event_fight(ev, 2, 2, 0, 0, 0x1b, 2, 0, 0, 0x1b, 5, 0, 10)) g_dng08_orcs_020d = 1;
		ret = 1;
	}
	if ((pos == 0x0309 || pos == 0x0409 || pos == 0x0804 || pos == 0x0805) && !g_dng08_orcs_0309) {
		if (g_dng08_alarm) {
			a = 2;
			b = 1;
			c = 1;
		} else if (DAY) {
			a = 2;
			b = 2;
			c = 1;
		} else {
			a = 1;
			b = 1;
		}
		if (a || b || c) {
			if (!event_fight(ev, 2, a, 0, 0, 0x1b, b, 0, 0, 0x5c, c, 0, 0)) g_dng08_orcs_0309 = 1;
			ret = 1;
		}
	} else if (pos == 0x0e06 && !g_dng08_orcs_0e06) {
		if (g_dng08_alarm) {
			if (random_schweif(100) <= 50) {
				a = 2;
				b = 1;
			}
			if (random_schweif(100) <= 10) c = 1;
		} else if (DAY) {
			if (random_schweif(100) <= 20) {
				a = 1;
				b = 1;
			}
			if (random_schweif(100) <= 5) c = 1;
		} else {
			if (random_schweif(100) <= 10) a = 1;
			if (random_schweif(100) <= 10) b = 2;
			if (random_schweif(100) <= 5) c = 2;
		}
		if (a || b || c) {
			if (!event_fight(ev, 2, a, 0, 0, 0x1b, b, 0, 0, 0x48, c, 0, 0)) g_dng08_orcs_0e06 = 1;
			ret = 1;
		}
	} else if ((pos == 0x0a06 || pos == 0x0a0a) && !g_dng08_orcs_0a06) {
		if (g_dng08_alarm) {
			if (random_schweif(100) <= 50) {
				a = 2;
				b = 1;
			}
			if (random_schweif(100) <= 10) c = 1;
		} else if (DAY) {
			if (random_schweif(100) <= 20) {
				a = 1;
				b = 1;
			}
			if (random_schweif(100) <= 5) c = 1;
		} else {
			if (random_schweif(100) <= 10) a = 1;
			if (random_schweif(100) <= 10) b = 2;
			if (random_schweif(100) <= 5) c = 2;
		}
		if (a || b || c) {
			if (!event_fight(ev, 2, a, 0, 0, 0x1b, b, 0, 0, 0x48, c, 0, 0)) g_dng08_orcs_0a06 = 1;
			ret = 1;
		}
	} else if (pos == 0x0d04 && !g_dng08_orcs_0d04) {
		if (g_dng08_alarm || DAY) {
			if (!event_fight2(ev, 0x1b, 2, 0, 0x48, 1, 0)) g_dng08_orcs_0d04 = 1;
			ret = 1;
		}
	} else if (pos == 0x1603 && !g_dng08_ogres_1603) {
		if (g_dng08_alarm) {
			if (!event_fight2(ev, 1, 1, 0, 0x5c, 2, 0)) g_dng08_ogres_1603 = 1;
		} else if (DAY) {
			if (!event_fight2(ev, 1, 1, 0, 0x5c, 1, 0)) g_dng08_ogres_1603 = 1;
		} else {
			if (!event_fight1(ev, 1, 2, 0)) g_dng08_ogres_1603 = 1;
		}
		ret = 1;
	} else if ((pos == 0x1907 || pos == 0x190a) && !g_dng08_orc_veteran_1907) {
#if FIX_0157
		/* Fix #157: Kampf melden (ret = 1) */
		if (!event_fight2(ev, 0x5b, 1, 0, 1, 2, 0)) g_dng08_orc_veteran_1907 = 1;
		ret = 1;
#else
		if (!event_fight2(ev, 0x5b, 1, 0, 1, 2, 0)) g_dng08_orc_veteran_1907 = 1;
#endif
	} else if (!g_dng08_mummy_fought && g_dng08_sarcophagus_opened &&
		(pos == 0x1f0a || pos == 0x1f09 || pos == 0x1e0a || pos == 0x1e09 ||
		pos == 0x1d0a || pos == 0x1d0b || pos == 0x1c0b || pos == 0x1c0c || pos == 0x1c0d ||
		pos == 0x1c0e || pos == 0x1c0f)) {
#if FIX_0157
		/* Fix #157: Kampf melden (ret = 1) */
		if (!event_fight1(ev, 0x5f, 1, 0)) g_dng08_mummy_fought = 1;
		ret = 1;
#else
		if (!event_fight1(ev, 0x5f, 1, 0)) g_dng08_mummy_fought = 1;
#endif
	}
	return ret;
}

signed short dng08_shelves(struct area_events far *ev, signed short pos)
{
	char far *t;
	Stash far *o;
	signed short id;
	signed short ret;

	/* BLUT3.LTX[17]: "In diesem Regal wird offensichtlich alles moegliche und unmoegliche gelagert.@Wollt Ihr etwas
	 * naeher darin herumstoebern?" */
	t = ev->txt[17];
	id = -1;
	ret = 0;
	switch (pos) {
	case 0x0218:
		id = 0x2a;
		/* BLUT3.LTX[67]: "Hier tuermen sich Schinken, Wuerste, Schweinehaelften, Ziegenkoepfe, saures Gemuese,
		 * Schwarzbrot und dergleichen Leckerein mehr.@Wollt Ihr Euch mit Proviant eindecken?" */
		t = ev->txt[67];
		break;
	case 0x0317:
		id = 0x2b;
		/* BLUT3.LTX[68]: "In diesem Regal lagern mehrere Faesschen und Flaschen.@Wollt Ihr Euch hier bedienen?" */
		t = ev->txt[68];
		break;
	case 0x0319:
		id = 0x2c;
		/* BLUT3.LTX[69]: "Zwischen den Regalboeden sind mehrere grosse Faesser festgeduebelt.@Wollt Ihr mal nachsehen,
		 * was da drin ist?" */
		t = ev->txt[69];
		break;
	case 0x041e:
		id = 0x2d;
		/* BLUT3.LTX[70]: "Hier lagern orkische Lederruestungen und eine Art wattierter Waffenroecke - bedauerlicherweise
		 * alle in Groesse <O> (Ork).@Wollt Ihr welche mitnehmen?" */
		t = ev->txt[70];
		break;
	case 0x051e:
		id = 0x2e;
		/* BLUT3.LTX[71]: "In diesem Regal findet sich eine mehr oder weniger erlesen Auswahl orkischer Hieb- und
		 * Stichwaffen.@Wollt Ihr Euch daran bedienen?" */
		t = ev->txt[71];
		break;
	case 0x071e:
		id = 0x2f;
		/* BLUT3.LTX[72]: "Auf den Regalboeden sind Unmengen Schilde gestapelt.@Wollt Ihr Euch daran bedienen?" */
		t = ev->txt[72];
		break;
	case 0x031b:
		id = 0x30;
		/* BLUT3.LTX[73]: "Hier findet Ihr einige Schusswaffen.@Wollt Ihr welche mitnehmen?" */
		t = ev->txt[73];
		break;
	case 0x041b:
		id = 0x31;
		/* BLUT3.LTX[74]: "Auf den oberen Regalboeden sind mehrere Helme gestapelt, auf den unteren liegen ein paar
		 * schwere Hiebwaffen.@Wollt Ihr Euch daran bedienen?" */
		t = ev->txt[74];
		break;
	case 0x1c01:
		id = 0x1e;
		/* BLUT3.LTX[18]: "In diesem Regal werden offensichtlich Ausruestungsgegenstaende allgemeiner Art gelagert. Vor
		 * allem Seile und Decken finden sich hier.@Wollt Ihr etwas davon mitnehmen?" */
		t = ev->txt[18];
		break;
	case 0x1f01:
		id = 0x1f;
		t = ev->txt[18];
		break;
	case 0x1f04:
		id = 0x20;
		/* BLUT3.LTX[19]: "Hier findet sich in erster Linie Werkzeug verschieder Art.@Wollt Ihr etwas davon mitnehmen?" */
		t = ev->txt[19];
		break;
	case 0x1f05:
		id = 0x21;
		t = ev->txt[19];
		break;
	case 0x1d07:
		id = 0x22;
		/* BLUT3.LTX[20]: "In diesem Regal haben die Orks ihre Winterausruestung gestapelt.@Wollt Ihr etwas von dem
		 * muffigen Zeug an Euch nehmen?" */
		t = ev->txt[20];
		break;
	case 0x1d18:
		/* BLUT3.LTX[49]: "In diesem Regal hat es ausser ein paar abgebrannten Fackeln nichts von Interesse." */
		dlg_message(g_dialog, ev->txt[49]);
		break;
	case 0x1e0e:
		if (!g_dng08_grain_1e0e) {
			/* BLUT3.LTX[30]: "In diesem Regal hat es einige Saecke Korn. Wollt Ihr sie anstechen, um zu sehen, was darin
			 * ist?" */
			if (dlg_yes_no(g_dialog, ev->txt[30]) == 1) {
				g_dng08_grain_1e0e = 1;
				/* BLUT3.LTX[31]: "Ihr stecht die Saecke an und seht zu, wie das wertvolle Korn langsam auf den Boden rieselt,
				 * doch es kommt nichts Besonderes zum Vorschein." */
				dlg_message(g_dialog, ev->txt[31]);
			}
		} else
			/* BLUT3.LTX[32]: "In dem Regal liegen einige leere Kornsaecke und der Boden ist ueber und ueber mit ihrem
			 * ehemaligen Inhalt uebersaet." */
			dlg_message(g_dialog, ev->txt[32]);
		break;
	case 0x1e0f:
		if (!g_dng08_grain_1e0f) {
			if (dlg_yes_no(g_dialog, ev->txt[30]) == 1) {
				g_dng08_grain_1e0f = 1;
				dlg_message(g_dialog, ev->txt[31]);
			}
		} else
			dlg_message(g_dialog, ev->txt[32]);
		break;
	case 0x1e10:
		if (!g_dng08_grain_1e10) {
			if (dlg_yes_no(g_dialog, ev->txt[30]) == 1) {
				g_dng08_grain_1e10 = 1;
				dlg_message(g_dialog, ev->txt[31]);
			}
		} else
			dlg_message(g_dialog, ev->txt[32]);
		break;
	case 0x1e0d:
		if (!g_dng08_grain_1e0d) {
			if (dlg_yes_no(g_dialog, ev->txt[30]) == 1) {
				g_dng08_grain_1e0d = 1;
				dlg_message(g_dialog, ev->txt[31]);
			}
		} else
			dlg_message(g_dialog, ev->txt[32]);
		break;
	case 0x1a0e:
		/* BLUT3.LTX[45]: "In diesem Regal liegt allerlei Plunder. Leere Saecke - gestapelt - aber auch so wertvolle
		 * Sachen wie rostige und verbogene Naegel. So Zeug eben." */
		dlg_message(g_dialog, ev->txt[45]);
		timewarp_minutes(g_date, 10L);
		break;
	}
	if (id != -1) {
		o = new Stash(id, 1);
		if (o->has_items() == 1 && dlg_yes_no(g_dialog, t) == 1) {
			o->menu();
			ret = 1;
		}
		delete o;
	}
	return ret;
}

void dng08_pit(struct area_events far *ev, signed char far *st)
{
	struct hero far *h;

	h = get_first_active_hero(g_group);
	if (!*st) {
		if (test_skill(h, TA_SINNESSCHAERFE, 4) >= 0)
			/* BLUT3.LTX[54]: "Hier scheint der Boden so konstruiert zu sein, dass er in einem Stueck wegklappen kann und -
			 * wahrscheinlich - eine haessliche Fallgrube freilegen kann! Mit etwas Muehe gelingt es Euch, die Falle zu
			 * umgehen." */
			dlg_message(g_dialog, ev->txt[54]);
		return;
	}
	if (*st == 1) {
		if (test_skill(h, TA_SINNESSCHAERFE, 4) >= 0) {
			/* BLUT3.LTX[55]: "<Vorsicht!> warnt Euch %s.<Da ist eine Fallgrube vor uns. Seht Ihr die Fugen?>@Mit dem Finger
			 * zeigt %s Euch den genauen Verlauf der Grube, so dass Ihr sie alle unbeschadet passieren koennt." */
			sprintf(g_text_output_buf, ev->txt[55], h->name, grammar_pronoun(h->sex, 0, 0));
			dlg_message(g_dialog, g_text_output_buf);
		} else {
			*st = 2;
			/* BLUT3.LTX[56]: "Ploetzlich bewegt sich der Boden unter Euren Fuessen und gibt eine Grube frei, deren Boden
			 * mit scharfkantigen Tonscherben bedeckt ist.Da es sich in der Luft so schlecht steht, fallt Ihr und zieht Euch
			 * einige haessliche Schnitte und Wunden zu. Nach einiger Zeit gelingt es Euch allerdings, wieder aus der Grube
			 * herauszuklettern." */
			dlg_message(g_dialog, ev->txt[56]);
			group_sub_le_range(g_group, 3, 12);
			timewarp_minutes(g_date, 10L);
		}
		return;
	}
	/* BLUT3.LTX[57]: "Vorsichtig geht Ihr der Fallgrube aus dem Weg. Da sie allerdings sehr gross ist und Eure Fuesse
	 * fast keinen Halt finden koennen, muesst Ihr sehr behutsam und gemaechlich vorangehen." */
	dlg_message(g_dialog, ev->txt[57]);
	timewarp_minutes(g_date, 5L);
}
