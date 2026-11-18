/* OVL186 - Overlay O186 (Segment 0x22fc in SCHWEIF.EXE): Kartenereignis-Rueckruf der Stadt Lowangen (Stadt 4,
 * g_town_descs[4].handler; Ereignisobjekt o = Town aus O183 mit Texten TJOLMAR.LTX = Datei 0x21e, die O046 auch fuer
 * Stadt 4 laedt). town_lowangen_event(o, e): e = struct map_event (Art type, Feld +2/+3), Feld relativ zum
 * Kartenursprung als pos = (y << 8) | x.
 * Art -2 (Feld betreten): um 14 Uhr (g_at_1400) auf der Strasse die Strassenbande (lowangen_food_thugs) bzw. die
 *   Stadtgarde (lowangen_weapon_check); sonst an den Stadttoren (0x505..0x707 bzw. 0x92a..0xc2a: lowangen_arrival(0),
 *   0x1c04..0x1e04: lowangen_arrival(1)) die Ankunftsszene, wenn g_low_arrival_due; Opferschale 0x1309
 *   (ev_offering_bowl), Brueckenzoll 0xc1d (ev_bridge_toll; Zeile vorher in g_low_bridge_y), Gebaeude LOC_DATA | 0x36..0x38 beim
 *   Betreten (0x604/0xc2b/0x406, letzteres mit g_exit_dir = 1), Wasserfeld (Feldtyp 3) -> town_swim_check.
 * Art 1 (Tuer angeklickt): Feld -> Ort LOC_DATA | 1 .. 0x46 der Stadtdaten (town_enter; 0x1d03 merkt das Ereignis in
 *   g_pick_event), sonst ein Wohnhaus LOC_HOUSE (O173).
 * Art 0x6f (Brunnen): TJOLMAR.LTX[28] trinken? -> 5 Minuten, Klang 0x5d, [29], Wasserschlaeuche fuellen
 *   (group_refill_water) und einmal am Tag (Zeitzaehler INGAME_TIMER_WELL_LOWANGEN) LE + 1 und eine Wirkung
 *   group_add_effect(0x546 Ticks, 0, 1, 0).
 *
 * BCC -mlarge -3 -Ot -c -Y OVL186.CPP   -> 2127/2127 Bytes GLEICH (18.10.2026), cobjfix laeuft (auch mit -Og gleich)
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - C++ wegen "g_low_bridge_y = g_group->get_y()": inline-Getter mit int-Ergebnis laedt das Wort (mov ax) und
 *   speichert al; direkter Zugriff/Casts ergeben immer mov al.
 * - pos in zwei Anweisungen ("pos = ... << 8; pos |= ...;"), sonst rechnet BCC den x-Teil in DX.
 * - switch-Faelle in Quellreihenfolge = Reihenfolge der Ortsnummern (LOC_DATA | 0x46 steht zwischen 0x2d und 0x2e);
 *   default am Ende; Mehrfachmarken 0xd0b/0xd0c, 0x70a/0x80a, 0x1801/0x1802; zwei Sprungtabellen
 *   (0xd0b..0xd13, 0x1525..0x1530) erzeugt BCC selbst.
 * - Ev-Objekte als C-Aufrufe Ev_ctor((void far *)0) / Ev_dtor(p, 3).
 */
#include "schweif.h"

signed short town_lowangen_event(Town far *o, struct map_event far *e)
{
	signed short loc;
	signed short r;
	void far *p;
	register signed short pos;
	register signed short act;

	act = e->type;
	pos = (e->y - g_map_x0) << 8;
	pos |= e->x - g_map_y0;
	r = 0;
	if (act == -2) {
		if (g_at_1400) {
			if (!lowangen_food_thugs()) {
				lowangen_weapon_check();
				r = 1;
			}
			g_at_1400 = 0;
		} else {
			if ((pos == 0x605 || pos == 0x506 || pos == 0x505 || pos == 0x606 || pos == 0x706 ||
			     pos == 0x607 || pos == 0xc2a || pos == 0xb2a || pos == 0xa2a || pos == 0x92a) && g_low_arrival_due) {
				lowangen_arrival(0);
				r = 1;
			}
			if ((pos == 0x1c04 || pos == 0x1d05 || pos == 0x1e04) && g_low_arrival_due) {
				lowangen_arrival(1);
				r = 1;
			}
			if (pos == 0x1309) {
				p = Ev_ctor((void far *)0);
				ev_offering_bowl(p);
				Ev_dtor(p, 3);
			}
			if (pos == 0xc1b || pos == 0xc1c || pos == 0xc1e || pos == 0xc1f)
				g_low_bridge_y = g_group->get_y();
			if (pos == 0xc1d) {
				p = Ev_ctor((void far *)0);
				ev_bridge_toll(p);
				Ev_dtor(p, 3);
			}
			if (pos == 0x604) {
				town_enter(o, LOC_DATA | 0x36, e);
				r = 1;
			}
			if (pos == 0xc2b) {
				town_enter(o, LOC_DATA | 0x37, e);
				r = 1;
			}
			if (pos == 0x406) {
				town_enter(o, LOC_DATA | 0x38, e);
				g_exit_dir = 1;
				r = 1;
			}
			if (get_cell_a(e->x, e->y) == 3)
				town_swim_check(o);
		}
	} else if (act == 1) {
		loc = 0;
		switch (pos) {
		case 0x020e:
			loc = LOC_DATA | 0x01;
			break;
		case 0x090a:
			loc = LOC_DATA | 0x02;
			break;
		case 0x1522:
			loc = LOC_DATA | 0x03;
			break;
		case 0x0d0f:
			loc = LOC_DATA | 0x04;
			break;
		case 0x0705:
			loc = LOC_DATA | 0x05;
			break;
		case 0x1808:
			loc = LOC_DATA | 0x06;
			break;
		case 0x1b12:
			loc = LOC_DATA | 0x07;
			break;
		case 0x1325:
			loc = LOC_DATA | 0x08;
			break;
		case 0x0c11:
			loc = LOC_DATA | 0x09;
			break;
		case 0x1606:
			loc = LOC_DATA | 0x0a;
			break;
		case 0x170a:
			loc = LOC_DATA | 0x0b;
			break;
		case 0x0d20:
			loc = LOC_DATA | 0x0c;
			break;
		case 0x040f:
			loc = LOC_DATA | 0x0d;
			break;
		case 0x071b:
			loc = LOC_DATA | 0x0e;
			break;
		case 0x1816:
			loc = LOC_DATA | 0x0f;
			break;
		case 0x1c20:
			loc = LOC_DATA | 0x10;
			break;
		case 0x1d16:
			loc = LOC_DATA | 0x11;
			break;
		case 0x070f:
			loc = LOC_DATA | 0x12;
			break;
		case 0x1526:
			loc = LOC_DATA | 0x13;
			break;
		case 0x030c:
			loc = LOC_DATA | 0x14;
			break;
		case 0x1726:
			loc = LOC_DATA | 0x15;
			break;
		case 0x1810:
			loc = LOC_DATA | 0x16;
			break;
		case 0x0417:
			loc = LOC_DATA | 0x17;
			break;
		case 0x0713:
			loc = LOC_DATA | 0x18;
			break;
		case 0x112c:
			loc = LOC_DATA | 0x19;
			break;
		case 0x1b19:
			loc = LOC_DATA | 0x1a;
			break;
		case 0x0e05:
			loc = LOC_DATA | 0x1b;
			break;
		case 0x1b0f:
			loc = LOC_DATA | 0x1c;
			break;
		case 0x0213:
			loc = LOC_DATA | 0x1d;
			break;
		case 0x1e19:
			loc = LOC_DATA | 0x1e;
			break;
		case 0x161f:
			loc = LOC_DATA | 0x1f;
			break;
		case 0x1b25:
			loc = LOC_DATA | 0x20;
			break;
		case 0x1114:
			loc = LOC_DATA | 0x21;
			break;
		case 0x1a26:
			loc = LOC_DATA | 0x22;
			break;
		case 0x1905:
			loc = LOC_DATA | 0x23;
			break;
		case 0x0411:
			loc = LOC_DATA | 0x24;
			break;
		case 0x1b1d:
			loc = LOC_DATA | 0x25;
			break;
		case 0x0b24:
			loc = LOC_DATA | 0x26;
			break;
		case 0x0b11:
			loc = LOC_DATA | 0x27;
			break;
		case 0x0a07:
			loc = LOC_DATA | 0x28;
			break;
		case 0x1320:
			loc = LOC_DATA | 0x29;
			break;
		case 0x1607:
			loc = LOC_DATA | 0x2a;
			break;
		case 0x0d13:
			loc = LOC_DATA | 0x2b;
			break;
		case 0x0b1b:
			loc = LOC_DATA | 0x2c;
			break;
		case 0x0518:
			loc = LOC_DATA | 0x2d;
			break;
		case 0x090e:
			loc = LOC_DATA | 0x46;
			break;
		case 0x0c0d:
			loc = LOC_DATA | 0x2e;
			break;
		case 0x0d0b:
		case 0x0d0c:
			loc = LOC_DATA | 0x2f;
			break;
		case 0x1d03:
			loc = LOC_DATA | 0x30;
			g_pick_event = (signed long)e;	/* Ereignis merken (Zeiger als long) */
			break;
		case 0x070a:
		case 0x080a:
			loc = LOC_DATA | 0x31;
			break;
		case 0x1801:
		case 0x1802:
			loc = LOC_DATA | 0x32;
			break;
		case 0x1525:
			loc = LOC_DATA | 0x33;
			break;
		case 0x1b08:
			loc = LOC_DATA | 0x34;
			break;
		case 0x0d1c:
			loc = LOC_DATA | 0x35;
			break;
		case 0x0604:
			loc = LOC_DATA | 0x36;
			break;
		case 0x0c2b:
			loc = LOC_DATA | 0x37;
			break;
		case 0x0406:
			loc = LOC_DATA | 0x38;
			break;
		case 0x0b0b:
			loc = LOC_DATA | 0x39;
			break;
		case 0x1613:
			loc = LOC_DATA | 0x3a;
			break;
		case 0x190b:
			loc = LOC_DATA | 0x3b;
			break;
		case 0x1829:
			loc = LOC_DATA | 0x3c;
			break;
		case 0x1e15:
			loc = LOC_DATA | 0x3d;
			break;
		case 0x0117:
			loc = LOC_DATA | 0x3e;
			break;
		case 0x152c:
			loc = LOC_DATA | 0x3f;
			break;
		case 0x161a:
			loc = LOC_DATA | 0x40;
			break;
		case 0x0c17:
			loc = LOC_DATA | 0x41;
			break;
		case 0x1530:
			loc = LOC_DATA | 0x42;
			break;
		case 0x1105:
			loc = LOC_DATA | 0x43;
			break;
		case 0x0c01:
			loc = LOC_DATA | 0x44;
			break;
		case 0x1409:
			loc = LOC_DATA | 0x45;
			break;
		default:
			town_enter(o, LOC_HOUSE, e);
			r = 1;
			break;
		}
		if (loc) {
			town_enter(o, loc, e);
			r = 1;
		}
	} else if (act == 0x6f) {
		/* TJOLMAR.LTX[28]: "Hier befindet sich ein Brunnen.@Wollt Ihr davon trinken und Eure Wasserschlaeuche
		 * auffuellen?" */
		if (dlg_yes_no(g_dialog, o->txt[28]) == 1) {
			timewarp_minutes(g_date, 5L);
			snd_play(g_snd_fx, 0x5d);
			/* TJOLMAR.LTX[29]: "Ihr fuehlt Euch herrlich erfrischt." */
			dlg_message(g_dialog, o->txt[29]);
			group_refill_water(g_group);
			if (g_ingame_timers[INGAME_TIMER_WELL_LOWANGEN] == 0) {
				group_add_le(g_group, 1);
				group_add_effect(g_group, 0x546L, 0, 1, 0);
				g_ingame_timers[INGAME_TIMER_WELL_LOWANGEN] = 0x1fa40L;
			}
		}
	}
	return r;
}
