/* OVL212 - Overlay O212 (Segment 0x23a3 in SCHWEIF.EXE): Kaempfe unterwegs (Aufrufer travel_route O215).
 * fight_result(p, v): Kampfergebnis v auswerten (g_game_state 6 -> 1, 1/2/5 -> 2): 1: Reiseaktion umkehren
 * (action = 1, Wegereignisse ev_half/ev_start/ev_end zuruecksetzen), 2: g_travel_state = 1 und Bildschirm loeschen,
 * 0 -> Rueckgabe 0, sonst 1. encounter_level(p, dir): Richtung 0..7 eines Weges (route.dir_from/dir_to) -> Stufe
 * 1,1,3,3,2,2,4,4 fuer die Begegnungen. ambush_fight(p, t): Ueberfall der Verfolger - hoechstens 5 Mal
 * (g_ambush_count), zu 50 % t + 4, Kampf start_fight(0x61/0x60/0x5c/0x5a/0x5b/0x62/0x5d/0x5f) zwischen travel_leave/
 * travel_enter (reenter = 1, g_fight_flag = 1). travel_encounter(p, terrain, lvl): W60 waehlt je Gelaende (0/1/2/5,
 * sonst -(terrain + 1) mit 20) einen Eintrag e = lvl + k (lvl teils halbiert) der Gegnertabelle g_encounter_enemies
 * und die Gegnerzahl g_fight_max_enemies (Wuerfel, abs-Differenzen, max(W(Gruppe - 1), 1)); Kampf wie ambush_fight.
 *
 * BCC -mlarge -3 -Ot -c -Y OVL212.CPP   -> 2438/2438 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h, Reiseobjekt Travel: 19.10.2026 GLEICH, LINKGLEICH)
 * - C++ wegen abs(): die inline-Funktion aus STDLIB.H legt je Aufruf eine eigene Parameterkopie an
 *   ([bp-6]/-8/-0a/-0c/-0e, keine Wiederverwendung, da die if-Bloecke keine Deklarationen haben).
 * - max als Makro ((a) > (b) ? (a) : (b)) -> random_schweif(count_check2() - 1) wird zweimal aufgerufen.
 * - lvl = (lvl + 1) / 2 -> cbw/inc/cwd/sub/sar, Ergebnis zurueck in den Byte-Parameter; e = lvl + k -> add al.
 * - Original-Bug in Gelaende 0: zweiter Bereich "r < 5 && r <= 15" (statt r > 5) bewusst nachgebaut.
 * - e = e2 = 99 als Kette ([bp-3] zuerst); switch-Rumpfe in Quellreihenfolge 2, 1, 0, 5 bzw. 1, 2, 0.
 * - g_vga_buf ist ein far-Zeiger (zwei push word; als long kaeme push dword).
 * - encounter_level: Blattfunktion, char-Local r landet in DL; gleiche Rumpfe getrennt.
 */
#include <stdlib.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:70b6..7140, dsana) - initialisierte Daten in Adressreihenfolge */
unsigned char g_scene_places[52] = {
	0x07, 0x09, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x27, 0x28,
	0x29, 0x2c, 0x2d, 0x61, 0x41, 0x42, 0x53, 0x57, 0x67, 0x77, 0x84, 0x85, 0x86, 0x30, 0x68, 0x6a,
	0x72, 0x75, 0x83, 0x80, 0x8e, 0xaa, 0xab, 0xb7, 0xb9, 0xbb, 0xbd, 0xc4, 0xc5, 0xc6, 0x11, 0x12,
	0x2a, 0x64, 0xff, 0x00
};
signed short g_encounter_enemies[43] = {
	126, 127, 101, 100, 147, 148, 102, 103, 104, 105, 121, 120, 123, 122, 108, 109,
	110, 111, 107, 106, 113, 112, 119, 118, 114, 115, 116, 117, 124, 125, 143, 144,
	139, 140, 145, 146, 136, 137, 134, 135, 141, 142, 138
};

#define max(a, b) (((a) > (b)) ? (a) : (b))

signed short fight_result(Travel far *p, signed char v)
{
	register signed short ret;

	ret = 1;
	if (g_game_state == 6)
		v = 1;
	if (g_game_state == 1 || g_game_state == 2 || g_game_state == 5)
		v = 2;
	switch (v) {
	case 1:
		p->action = 1;
		p->ev_half = 0;
		p->ev_start = 0;
		p->ev_end = 0;
		break;
	case 2:
		g_travel_state = 1;
		clear_screen_buf(g_vga_buf);
		break;
	case 0:
		ret = 0;
		break;
	}
	return ret;
}

signed char encounter_level(Travel far *p, signed char t)
{
	signed char r;

	r = 0;
	switch (t) {
	case 0:
		r = 1;
		break;
	case 1:
		r = 1;
		break;
	case 2:
		r = 3;
		break;
	case 3:
		r = 3;
		break;
	case 4:
		r = 2;
		break;
	case 5:
		r = 2;
		break;
	case 6:
		r = 4;
		break;
	case 7:
		r = 4;
		break;
	}
	return r;
}

void ambush_fight(Travel far *p, signed char t)
{
	signed char e;

	if (g_ambush_count < 5) {
		g_ambush_count++;
		if (random_schweif(10) > 5)
			t += 4;
		p->reenter = 1;
		travel_leave(p);
		g_fight_flag = 1;
		switch (t) {
		case 1:
			e = start_fight(0x61, 1);
			break;
		case 2:
			e = start_fight(0x60, 1);
			break;
		case 3:
			e = start_fight(0x5c, 1);
			break;
		case 4:
			e = start_fight(0x5a, 1);
			break;
		case 5:
			e = start_fight(0x5b, 1);
			break;
		case 6:
			e = start_fight(0x62, 1);
			break;
		case 7:
			e = start_fight(0x5d, 1);
			break;
		case 8:
			e = start_fight(0x5f, 1);
			break;
		}
		e = fight_result(p, e);
		travel_enter(p);
		p->reenter = 0;
	}
}

void travel_encounter(Travel far *p, signed char type, signed char lvl)
{
	signed char e;
	signed char r;
	signed char e2;

	r = random_schweif(60);
	e = e2 = 99;
	switch (type) {
	case 2:
		if (r <= 3) {
			g_fight_max_enemies = random_schweif(3) + 2;
			lvl = (lvl + 1) / 2;
			e = lvl + 0x23;
		}
		if (r > 3 && r <= 6) {
			g_fight_max_enemies = random_schweif(3) + 3;
			lvl = (lvl + 1) / 2;
			e = lvl - 1;
		}
		if (r > 6 && r <= 10) {
			g_fight_max_enemies = random_schweif(4);
			lvl = (lvl + 1) / 2;
			e = lvl + 1;
		}
		if (r > 10 && r <= 15) {
			g_fight_max_enemies = random_schweif(6) + 2;
			lvl = (lvl + 1) / 2;
			e = lvl + 0x1d;
		}
		if (r > 15 && r <= 20) {
			g_fight_max_enemies = abs(random_schweif(8) - random_schweif(7)) + 4;
			e = lvl + 5;
		}
		if (r > 20 && r <= 25) {
			g_fight_max_enemies = random_schweif(6) + 2;
			lvl = (lvl + 1) / 2;
			e = lvl + 3;
		}
		if (r > 25 && r <= 35) {
			g_fight_max_enemies = random_schweif(4) + 1;
			e = lvl + 9;
		}
		if (r > 35 && r <= 50) {
			g_fight_max_enemies = random_schweif(5) + 2;
			e = lvl + 0x0d;
		}
		if (r > 50 && r <= 60) {
			g_fight_max_enemies = random_schweif(6) + 3;
			lvl = (lvl + 1) / 2;
			e = lvl + 0x1f;
		}
		break;
	case 1:
		if (r <= 10) {
			g_fight_max_enemies = random_schweif(5) + 2;
			lvl = (lvl + 1) / 2;
			e = lvl + 0x21;
		}
		if (r > 10 && r <= 15) {
			g_fight_max_enemies = random_schweif(4) + 1;
			lvl = (lvl + 1) / 2;
			e = lvl + 0x23;
		}
		if (r > 15 && r <= 20) {
			g_fight_max_enemies = random_schweif(7) + 1;
			lvl = (lvl + 1) / 2;
			e = lvl + 0x27;
		}
		if (r > 20 && r <= 25) {
			g_fight_max_enemies = abs(random_schweif(8) - random_schweif(7)) + 3;
			e = lvl + 5;
		}
		if (r > 25 && r <= 30) {
			g_fight_max_enemies = random_schweif(4) + 2;
			lvl = (lvl + 1) / 2;
			e = lvl + 3;
		}
		if (r > 30 && r <= 35) {
			g_fight_max_enemies = random_schweif(3) + 1;
			lvl = (lvl + 1) / 2;
			e = lvl + 0x11;
		}
		if (r > 35 && r <= 45) {
			g_fight_max_enemies = random_schweif(3) + 2;
			lvl = (lvl + 1) / 2;
			e = lvl + 0x13;
		}
		if (r > 45 && r <= 50) {
			g_fight_max_enemies = random_schweif(6);
			lvl = (lvl + 1) / 2;
			e = lvl + 0x15;
		}
		if (r > 50 && r <= 60) {
			g_fight_max_enemies = abs(random_schweif(7) - random_schweif(6));
			e = lvl + 0x0d;
		}
		break;
	case 0:
		if (r <= 5) {
			g_fight_max_enemies = random_schweif(4) + 1;
			lvl = (lvl + 1) / 2;
			e = lvl + 0x21;
		}
#if FIX_0008
		/* Fix #8: Bereich 6..15 (wie die Nachbarbereiche) */
		if (r > 5 && r <= 15) {
#else
		if (r < 5 && r <= 15) {
#endif
			g_fight_max_enemies = random_schweif(4) + 2;
			lvl = (lvl + 1) / 2;
			e = lvl + 0x1d;
		}
		if (r > 15 && r <= 20) {
			g_fight_max_enemies = abs(random_schweif(10) - random_schweif(9)) + 2;
			e = lvl + 5;
		}
		if (r > 20 && r <= 25) {
			g_fight_max_enemies = random_schweif(4) + 3;
			lvl = (lvl + 1) / 2;
			e = lvl + 3;
		}
		if (r > 25 && r <= 30) {
			g_fight_max_enemies = random_schweif(3);
			lvl = (lvl + 1) / 2;
			e = lvl + 0x25;
		}
		if (r > 30 && r <= 35) {
			g_fight_max_enemies = random_schweif(3) + 1;
			lvl = (lvl + 1) / 2;
			e = lvl + 9;
		}
		if (r > 35 && r <= 40) {
			g_fight_max_enemies = abs(random_schweif(5) - random_schweif(4));
			lvl = (lvl + 1) / 2;
			e = lvl + 0x17;
		}
		if (r > 40 && r <= 45) {
			g_fight_max_enemies = max(random_schweif(g_group->count_check2() - 1), 1);
			e = 0x2a;
		}
		if (r > 45 && r <= 60) {
			g_fight_max_enemies = random_schweif(9) + 1;
			lvl = (lvl + 1) / 2;
			e = lvl + 0x1f;
		}
		break;
	case 5:
		if (r <= 15) {
			g_fight_max_enemies = random_schweif(4) + 2;
			lvl = (lvl + 1) / 2;
			e = lvl + 0x21;
		}
		if (r > 15 && r <= 20) {
			g_fight_max_enemies = random_schweif(6) + 1;
			lvl = (lvl + 1) / 2;
			e = lvl + 3;
		}
		if (r > 20 && r <= 25) {
			g_fight_max_enemies = random_schweif(3) + 1;
			e = lvl + 1;
		}
		if (r > 25 && r <= 30) {
			g_fight_max_enemies = random_schweif(4);
			e = lvl + 0x17;
		}
		if (r > 30 && r <= 35) {
			g_fight_max_enemies = random_schweif(4);
			lvl = (lvl + 1) / 2;
			e = lvl + 9;
		}
		if (r > 35 && r <= 40) {
			g_fight_max_enemies = random_schweif(2) + 2;
			lvl = (lvl + 1) / 2;
			e = lvl + 0x1b;
		}
		if (r > 40 && r <= 55) {
			g_fight_max_enemies = max(random_schweif(g_group->count_check2() - 1), 1);
			e = 0x2a;
		}
		if (r > 55 && r <= 60) {
			g_fight_max_enemies = random_schweif(4) + 1;
			lvl = (lvl + 1) / 2;
			e = lvl + 0x25;
		}
		break;
	default:
		e = -(type + 1);
		g_fight_max_enemies = 20;
		break;
	}
	if (e != 99 && g_fight_max_enemies > 0) {
		p->reenter = 1;
		travel_leave(p);
		g_fight_flag = 1;
		e2 = start_fight(g_encounter_enemies[e], 1);
		e2 = fight_result(p, e2);
		travel_enter(p);
		p->reenter = 0;
	}
}
