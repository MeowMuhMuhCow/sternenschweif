/* OVL222 - Overlay O222 (Segment 0x23d8 in SCHWEIF.EXE): zwei Testmenues ueber dlg_choose (ohne Aufrufer im Spiel) -
 * debug_ending_menu: Schlussszenen mit zwei lokalen Objekten aus O039/O040 (Botschafter, Sudran Alatzer, Adeptus,
 * "der Mob poebelt", zurueck; "gutes Ende" -> g_trial_lost), debug_travel_events: 11 Reiseereignisse, je Eintrag
 * place_event (Ort p->from) bzw. tevent_run; dazu vier leere Funktionen debug_nop1..4.
 *
 * BCC -mlarge -3 -Ot -c -Y OVL222.C   -> 760/760 Bytes GLEICH (18.10.2026; -Og ebenso)
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Beide Sprungtabellen = switch mit allen Faellen 1..n (auch leere case-Zweige mit break).
 * - Lokale Objekte (Konstruktor/Destruktor(this, 2) aus O039/O040, je 0x24 Bytes) wie in O045 als C ausgeschrieben;
 *   Stack: c [bp-1], anim [bp-26h], intro [bp-4Ah] in Deklarationsreihenfolge.
 * - g_ttxt3 = Texttabelle (char far * far *), jeder Zugriff laedt den Zeiger neu (les bx).
 */
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:7254..729a, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
signed char g_rose_placed = 0;
char g_unused_7255[1] = { 0 };   /* ds:7255, unbenutzt */

void debug_nop1(void)
{
}

void debug_nop2(void)
{
}

void debug_ending_menu(void)
{
	signed char c;
	char anim[0x24];			/* Objekt Anim3 (O040) */
	char intro[0x24];			/* Objekt Intro (O039) */

	Anim3_ctor(anim);
	Intro_ctor_o039(intro);
	c = 0;
	while (c != 5) {
		c = dlg_choose(g_dialog, (char far *)0, 5, "Botschafter", "Sudran Alatzer", "Adeptus",
			"der Mob p\x94""belt", "zur\x81""ck");
		switch (c) {
		case 1:
			break;
		case 2:
			break;
		case 3:
			Anim3_run(anim);
			break;
		case 4:
			c = dlg_yes_no(g_dialog, "gutes Ende");
			if (c == 1) {
				g_trial_lost = 0;
			} else {
				g_trial_lost = 1;
			}
			Intro_run_o039(intro);
			break;
		case 5:
			break;
		}
	}
	Intro_dtor_o039(intro, 2);
	Anim3_dtor(anim, 2);
}

void debug_nop3(void)
{
}

void debug_nop4(void)
{
}

void debug_travel_events(struct travel far *p)
{
	signed char c;

	c = 0;
	while (c != 11) {
		c = dlg_choose(g_dialog, g_ttxt3[0x3d], 11,
			g_ttxt3[0x54], g_ttxt3[0x55], g_ttxt3[0x56], g_ttxt3[0x57], g_ttxt3[0x58],
			g_ttxt3[0x59], g_ttxt3[0x5a], g_ttxt3[0x5b], g_ttxt3[0x5c], g_ttxt3[0x5d], g_ttxt3[0x47]);
		switch (c) {
		case 1:
			p->from = 7;
			place_event(p);
			break;
		case 2:
			group_give_item(g_group, 0x117, 1);
			tevent_run(p, 0x34);
			break;
		case 3:
			tevent_run(p, 0x5e);
			break;
		case 4:
			tevent_run(p, 0x4e);
			break;
		case 5:
			p->from = 0xbd;
			place_event(p);
			break;
		case 6:
			tevent_run(p, 0xb2);
			break;
		case 7:
			tevent_run(p, 0x43);
			break;
		case 8:
			group_give_item(g_group, 0x5a, 0x1e);
			tevent_run(p, 0x72);
			break;
		case 9:
			p->from = 0x1c;
			place_event(p);
			break;
		case 10:
			g_slow4_on = 1;
			tevent_run(p, 0x101);
			break;
		case 11:
			break;
		}
	}
	p->from = 1;
}
