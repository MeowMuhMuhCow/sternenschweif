/* OVL061 - Overlay O061 (Segment 0x2067 in SCHWEIF.EXE): Kartenereignis-Verteiler und Ereignisse des Dungeons 11, der
 * Binge mit Bolzenfallen, Fallblock und Koboldwohnung (g_area_descs[11]; Texte BINGE3.LTX = Datei 0x403 ueber
 *   ev->txt;
 * Fortsetzung O062). Ereignisobjekt ev (struct area_events), Kartenereignis mev (struct map_event).
 * dng11_handler(ev, mev): pos = ((mev->y - g_map_x0) << 8) | (mev->x - g_map_y0); MAP_EVENT_ENTER: 0x50a
 *   dng11_pressure_plate, 0x905 Grubenrampe scharf (g_dng11_ramp_armed) -> 0x906 dng11_ramp_climb, 0x1003
 *   dng11_plate, Bolzenfallen 0x312/0x910/0xe12 (dng11_bolt_trap: entdecken, ausloesen), 0x120b dng11_block_falls,
 *   Feldgruppe 0x1009..0x120a: abgeschlossener Raum - Zaehler g_dng11_air_counter (5/12/18: txt[59..61], je
 *   group_sub_le_range(g, 10, 20)); 1 Tueren dng11_door; 0x64/0x65 Treppen nach 10 bzw. 12; 0x66 Hebel
 *   dng11_lever_switch (0x60d) bzw. dng11_lever_floor (O062); 0x67 Truhen 0x24..0x27 (new/delete Chest) bzw.
 *   dng11_kobold_chest (0xe01); 0x68 binge_wall_hole(ev, 1/2); 0x69 binge_shaft; 0x6a Bolzenfallen untersuchen;
 *   0x6b dng11_winch; 0x6c dng11_wardrobe; 0x6e dng11_block_falls; 0x6f dng11_lever_switch.
 * dng11_door(ev, pos, x, y): Schloss Lock 0x77..0x7b/0x88..0x8f je Feld, Kartenbild 0x2d bzw. 0x2e.
 * dng11_lever_switch: Hebel an 0x60d schaltet Feld (0xd, 6) um (g_dng11_lever_on; sperrt die Bolzenfalle 2).
 * dng11_pressure_plate: Bodenplatte gibt nach, Klicken (txt[1]); g_dng11_plate_down umschalten (schaerft Falle 3).
 * dng11_bolt_trap(ev, trap, mode): mode 0 = Sinnesschaerfe (+8, je entdeckter Falle leichter) entdeckt die Falle
 *   (txt[2], dng11_bolt_trap_found, AP); mode 1 = Falle loest aus (entdeckt: txt[4] knapp verfehlt; sonst GE+4:
 *   txt[5] ausgewichen bzw. txt[6] W6+6 Schaden), danach entschaerft (g_dng11_trap2_fired/g_dng11_trap3_fired);
 *   mode 2 = untersuchen (txt[3] nicht geladen).
 * dng11_bolt_trap_found(ev, trap): Falle 1..3 als entdeckt merken (g_dng11_trap1_found ..), Feld und Automap 0x1f.
 * dng11_winch: Kurbellager hinter dem Fallblock (txt[9..24]): Kurbel (ITEM_CRANK) einsetzen (g_dng11_winch_state
 *   1), Mechanik ansehen, Kettenstueck (ITEM_CHAIN) einsetzen -> zweiter Block hebt sich (Zustand 2), sonst mit Seil
 *   (ITEM_ROPE) behelfsmaessig, Gruppe unter dem Block durch nach (0x10, 7); ohne Kurbel bleibt der Raum zu (Luft-
 *   zaehler startet).
 * dng11_block_falls: Fallblock senkt sich hinter der Gruppe (txt[7], einmalig g_dng11_block_down), alle Gruppen
 *   auf x 0x12, y 11..13 werden nach (0x12, 10) versetzt; danach txt[8].
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL061.CPP   -> 5166/5166 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Erster Entwurf sofort GLEICH; -Og noetig fuer die Epilogkopien in dng11_lever_switch/dng11_bolt_trap_found (ohne
 *   -Og 1 Byte zu lang).
 * - C++: new/delete fuer Chest (O078; Ktor mit this = 0, Dtor(p, 3)), lokales Lock-Objekt (O108) im if-Block (Dtor am
 *   Blockende), inline-Setter group::set_x_screen/set_y_screen/set_x_v/set_y_v in dng11_block_falls.
 * - dng11_handler: "pos = ((y = mev->y) - g_map_x0) << 8; pos |= (x = mev->x) - g_map_y0;" -> Stores [bp-8]/[bp-0ah]
 *   eingebettet; type in DI, pos in SI. case 0x6d leer mit break (eigener jmp short), letzter case ohne break.
 * - dng11_block_falls: "g->set_x_v(g->set_x_screen(0x12))" (Bildschirmposition konstant gefaltet), Ergebnis wird
 *   Parameterkopie des aeusseren set_x_v (DI bzw. [bp-8] fuer y).
 * - dng11_winch: Heldenzeiger aus g_group->heroes[idx] -> mov ax/mov dx-Paar; "h == 0" -> cmp dword.
 * Originalfehler/Auffaelligkeiten: dng11_block_falls setzt die Position doppelt (set_x_screen und set_x_v je mit
 *   "old_x = x"), damit steht in old_x/old_y danach die NEUE statt der vorherigen Position. dng11_winch sucht den
 *   letzten handlungsfaehigen Helden per "heroes[nr_heroes - i]" ohne Grenze (kein Treffer -> Lesen vor dem Feld).
 */
#include <stdio.h>

#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:5758..575e, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */

/* automap_cell_write mit short als letztem Parameter (wie im Original deklariert) */
#define AUTOMAP_CELL_WRITE_W ((void (*)(void far *, signed short, signed short, signed short, signed short))automap_cell_write)

signed short dng11_handler(struct area_events far *ev, struct map_event far *mev)
{
	signed short ret = 0;
	Chest far *o;
	signed short y;
	signed short x;
	register signed short pos;
	register signed short type;

	type = mev->type;
	pos = ((y = mev->y) - g_map_x0) << 8;
	pos |= (x = mev->x) - g_map_y0;

	switch (type) {
	case MAP_EVENT_ENTER:
		switch (pos) {
		case 0x50a:
			dng11_pressure_plate(ev);
			break;
		case 0x905:
			g_dng11_ramp_armed = 1;
			break;
		case 0x906:
			if (g_dng11_ramp_armed != 0) {
				dng11_ramp_climb(ev);
				g_dng11_ramp_armed = 0;
			}
			break;
		case 0x1003:
			dng11_plate(ev);
			break;
		case 0x312:
			dng11_bolt_trap(ev, 1, 0);
			break;
		case 0x910:
			dng11_bolt_trap(ev, 2, 0);
			dng11_bolt_trap(ev, 2, 1);
			break;
		case 0xe12:
			dng11_bolt_trap(ev, 3, 0);
			dng11_bolt_trap(ev, 3, 1);
			break;
		case 0x120b:
			if (!g_dng11_block_down)
				dng11_block_falls(ev);
			break;
		case 0x1009:
		case 0x100a:
		case 0x100b:
		case 0x1109:
		case 0x110a:
		case 0x110b:
		case 0x1209:
		case 0x120a:
			if (g_dng11_air_counter != 0) {
				g_dng11_air_counter++;
				if (g_dng11_air_counter == 5) {
					/* BINGE3.LTX[59]: "Der Raum scheint hermetisch abgeschlossen zu sein. Die Luft ist schon ziemlich stickig und
					 * es wird unangenehm warm." */
					dlg_message(g_dialog, ev->txt[59]);
					group_sub_le_range(g_group, 10, 0x14);
				} else if (g_dng11_air_counter == 12) {
					/* BINGE3.LTX[60]: "Ihr koennt kaum noch atmen, die Luft ist zum schneiden dick. Schon die einfachsten
					 * Bewegungen fallen Euch schwer." */
					dlg_message(g_dialog, ev->txt[60]);
					group_sub_le_range(g_group, 10, 0x14);
				} else if (g_dng11_air_counter >= 18) {
					/* BINGE3.LTX[61]: "Die heisse Luft schmerzt in Euren Lungen, Ihr seid foermlich in Schweiss gebadet. Lange
					 * wird es nicht mehr dauern..." */
					dlg_message(g_dialog, ev->txt[61]);
					group_sub_le_range(g_group, 10, 0x14);
				}
			}
			break;
		}
		break;
	case 1:
		ret = dng11_door(ev, pos, x, y);
		break;
	case 0x64:
		group_set_screen_pos(1, 0xe);
		g_group->old_dungeon = g_group->dungeon;
		g_group->dungeon = 10;
		g_group->dir = 3;
		break;
	case 0x65:
		group_set_screen_pos(0xc, 0xb);
		g_group->old_dungeon = g_group->dungeon;
		g_group->dungeon = 12;
		g_group->dir = 2;
		g_cam_angle = 0x100;
		break;
	case 0x66:
		if (pos == 0x60d)
			dng11_lever_switch(ev);
		else
			dng11_lever_floor(ev);
		break;
	case 0x67:
		switch (pos) {
		case 0x30a:
			o = new Chest(0x24, 1);
			delete o;
			break;
		case 0x105:
			o = new Chest(0x25, 1);
			delete o;
			break;
		case 0x305:
			o = new Chest(0x26, 1);
			delete o;
			break;
		case 0xe06:
			o = new Chest(0x27, 1);
			delete o;
			break;
		case 0xe01:
			dng11_kobold_chest(ev);
			break;
		}
		break;
	case 0x68:
		if (pos == 3)
			binge_wall_hole(ev, 1);
		else if (pos == 0x90a)
			binge_wall_hole(ev, 2);
		break;
	case 0x69:
		binge_shaft(ev);
		break;
	case 0x6a:
		switch (pos) {
		case 0x312:
			dng11_bolt_trap(ev, 1, 2);
			break;
		case 0x910:
			dng11_bolt_trap(ev, 2, 2);
			break;
		case 0xf12:
			dng11_bolt_trap(ev, 3, 2);
			break;
		}
		break;
	case 0x6b:
		dng11_winch(ev);
		break;
	case 0x6c:
		dng11_wardrobe(ev);
		break;
	case 0x6d:
		break;
	case 0x6e:
		dng11_block_falls(ev);
		break;
	case 0x6f:
		if (pos == 0x60d)
			dng11_lever_switch(ev);
	}
	return ret;
}

signed short dng11_door(struct area_events far *ev, signed short pos, signed short x, signed short y)
{
	signed short ret = 0;
	register signed short nr = -1;
	register signed short pic = 0x2d;

	switch (pos) {
	case 0x401:
		nr = 0x77;
		break;
	case 0x403:
		nr = 0x78;
		break;
	case 0x207:
		nr = 0x79;
		pic = 0x2e;
		break;
	case 0x608:
		nr = 0x7a;
		break;
	case 0x20e:
		nr = 0x7b;
		break;
	case 0x10f:
		nr = 0x88;
		pic = 0x2e;
		break;
	case 0x808:
		nr = 0x89;
		break;
	case 0x907:
		nr = 0x8a;
		pic = 0x2e;
		break;
	case 0xc09:
		nr = 0x8b;
		pic = 0x2e;
		break;
	case 0x80c:
		nr = 0x8c;
		break;
	case 0x120f:
		nr = 0x8d;
		pic = 0x2e;
		break;
	case 0x1005:
		nr = 0x8e;
		pic = 0x2e;
		break;
	case 0xb01:
		nr = 0x8f;
		break;
	default:
		break;
	}
	if (nr != -1) {
		Lock lk(nr);
		ret = 1;
		if (g_lock_cell_bits != -1) {
			event_set_cell_bits(ev, x, y, g_lock_cell_bits);
			snd_play(g_snd_fx, 0x61);
			AUTOMAP_CELL_WRITE_W(g_map, 0x302, y - g_map_x0, x - g_map_y0, pic);
		}
	}
	return ret;
}

void dng11_lever_switch(struct area_events far *ev)
{
	if (!g_dng11_lever_on) {
		g_dng11_lever_on = 1;
		event_set_cell(ev, 0xd, 6, 5, 0, 3);
	} else {
		g_dng11_lever_on = 0;
		event_set_cell(ev, 0xd, 6, 5, 0, 0);
	}
}

void dng11_pressure_plate(struct area_events far *ev)
{
	signed short old;
	struct hero far *h;

	old = dlg_set16(g_dialog, 5);
	h = get_first_active_hero(g_group);
	if (!g_dng11_plate_down)
		g_dng11_plate_down = 1;
	else
		g_dng11_plate_down = 0;
	/* BINGE3.LTX[1]: "%s spuert, wie der Boden unter %s ein wenig nachgibt. Ein leises Klicken ist zu hoeren, sonst
	 * passiert nichts weiter." */
	sprintf(g_text_output_buf, ev->txt[1], h->name, grammar_pronoun(h->sex, 3, 0));
	dlg_message(g_dialog, g_text_output_buf);
	dlg_set16(g_dialog, old);
}

void dng11_bolt_trap(struct area_events far *ev, register signed short trap, signed short mode)
{
	signed short mod;
	signed short old;
	struct hero far *h;

	old = dlg_set16(g_dialog, 5);
	h = get_first_active_hero(g_group);
	switch (mode) {
	case 0:
		if ((trap == 1 && !g_dng11_trap1_found) || (trap == 2 && !g_dng11_trap2_found) ||
		    (trap == 3 && !g_dng11_trap3_found)) {
			mod = 8;
			if (g_dng11_trap1_found != 0)
				mod -= 3;
			if (g_dng11_trap2_found != 0)
				mod -= 2;
			if (g_dng11_trap3_found != 0)
				mod -= 2;
			if (test_skill(h, TA_SINNESSCHAERFE, mod) > 0) {
				switch (trap) {
				case 1:
					dng11_bolt_trap_found(ev, 1);
					break;
				case 2:
					dng11_bolt_trap_found(ev, 2);
					break;
				case 3:
					dng11_bolt_trap_found(ev, 3);
					break;
				}
				add_hero_ap(h, mod);
				/* BINGE3.LTX[2]: "Ploetzlich bleibt %s stehen, %s hat zwei kleine runde Oeffnungen in der Wand entdeckt -
				 * offensichtlich die Abschussoeffnungen einer Bolzenfalle." */
				sprintf(g_text_output_buf, ev->txt[2], h->name, grammar_pronoun(h->sex, 0, 0), grammar_pronoun(h->sex, 0, 0));
				dlg_message(g_dialog, g_text_output_buf);
			}
		}
		break;
	case 1:
		if ((trap == 2 && !g_dng11_lever_on && !g_dng11_trap2_fired) ||
		    (trap == 3 && g_dng11_plate_down != 0 && !g_dng11_trap3_fired)) {
			if ((trap == 2 && g_dng11_trap2_found != 0) || (trap == 3 && g_dng11_trap3_found != 0)) {
				/* BINGE3.LTX[4]: "%s ist auf der Hut, als %s sich den Oeffnungen in der Wand naehert. An den Bolzen, die von
				 * beiden Seiten heranzischen und %s nur knapp verfehlen, koennt Ihr erkennen, dass die Falle noch geladen war.
				 * Jetzt scheint die Gefahr allerdings ueberstanden zu sein, denn %s passiert die Stelle, ohne dass etwas
				 * geschieht." */
				sprintf(g_text_output_buf, ev->txt[4], h->name, grammar_pronoun(h->sex, 0, 0), grammar_pronoun(h->sex, 2, 0),
					grammar_pronoun(h->sex, 0, 0));
				dlg_message(g_dialog, g_text_output_buf);
				if (trap == 2)
					g_dng11_trap2_fired = 1;
				else if (trap == 3)
					g_dng11_trap3_fired = 1;
			} else {
				if (test_attrib(h, ATTRIB_GE, 4) > 0) {
					/* BINGE3.LTX[5]: "%s hat zwei kleine Runde Loecher neben sich in der Wand entdeckt. Mit einem gewaltigen Satz
					 * springt %s zurueck und kann gerade noch den Bolzen ausweichen, die ploetzlich von beiden Seiten
					 * heranjagen!" */
					sprintf(g_text_output_buf, ev->txt[5], h->name, grammar_pronoun(h->sex, 0, 0));
					dlg_message(g_dialog, g_text_output_buf);
				} else {
					/* BINGE3.LTX[6]: "Ploetzlich pfeifen Euch von links und rechts Armbrustbolzen um die Ohren. %s ist getroffen
					 * worden. Erschrocken schaut %s sich um, aber es ist niemand zu sehen. Nach einer kurzen Untersuchung
					 * entdeckt %s zwei Oeffnungen in der Wand." */
					sprintf(g_text_output_buf, ev->txt[6], h->name, grammar_pronoun(h->sex, 0, 0), grammar_pronoun(h->sex, 0, 0));
					dlg_message(g_dialog, g_text_output_buf);
					sub_hero_le(h, random_schweif(6) + 6);
				}
				if (trap == 2) {
					g_dng11_trap2_fired = 1;
					dng11_bolt_trap_found(ev, 2);
				} else if (trap == 3) {
					g_dng11_trap3_fired = 1;
					dng11_bolt_trap_found(ev, 3);
				}
			}
		}
		break;
	case 2:
		/* BINGE3.LTX[3]: "%s untersucht vorsichtig die Bolzenfalle, aber es passiert nichts: die Falle ist nicht
		 * geladen." */
		sprintf(g_text_output_buf, ev->txt[3], h->name);
		dlg_message(g_dialog, g_text_output_buf);
		break;
	}
	dlg_set16(g_dialog, old);
}

void dng11_bolt_trap_found(struct area_events far *ev, signed short trap)
{
	switch (trap) {
	case 1:
		g_dng11_trap1_found = 1;
		event_set_cell(ev, 0x12, 3, 9, 0, 1);
		automap_cell_write(g_map, 0x302, 3, 0x12, 0x1f);
		break;
	case 2:
		g_dng11_trap2_found = 1;
		event_set_cell(ev, 0x10, 9, 9, 0, 1);
		automap_cell_write(g_map, 0x302, 9, 0x10, 0x1f);
		break;
	case 3:
		g_dng11_trap3_found = 1;
		event_set_cell(ev, 0x12, 0xf, 9, 0, 1);
		automap_cell_write(g_map, 0x302, 0xf, 0x12, 0x1f);
		break;
	}
}

void dng11_winch(struct area_events far *ev)
{
	signed short idx;
	signed short old;
	struct hero far *h;
	struct hero far *h2;
	register signed short i;
	register signed short choice;

	old = dlg_set16(g_dialog, 5);
	h = get_first_active_hero(g_group);
	if (!g_dng11_winch_state) {
		/* BINGE3.LTX[0]: "Wollt Ihr" */
		/* BINGE3.LTX[9]: "den Hebel bewegen?" */
		/* BINGE3.LTX[10]: "oder das Kurbellager untersuchen?" */
		choice = dlg_choose(g_dialog, ev->txt[0], 2, ev->txt[9], ev->txt[10]);
		if (choice == 1)
			/* BINGE3.LTX[11]: "Ein leises Klicken ertoent. Sonst passiert nichts erkennbares." */
			dlg_message(g_dialog, ev->txt[11]);
		else if (choice == 2) {
			idx = group_find_item(g_group, ITEM_CRANK);
			if (idx != -1) {
				h = g_group->heroes[idx];
				if (!check_hero3(h))
					h = get_first_active_hero(g_group);
				/* BINGE3.LTX[12]: "Die Kurbel, die Ihr gefunden habt, passt genau in das Lager. %s beginnt heftig, zuerst in
				 * die eine, dann in die andere Richtung zu drehen. Obwohl %s spueren kann, dass sich hinter der Wand etwas
				 * mitbewegt, geht es eigentlich viel zu leicht und, - abgesehen von einem metallischen Klingeln ist auch keine
				 * Wirkung festzustellen." */
				sprintf(g_text_output_buf, ev->txt[12], h->name, grammar_pronoun(h->sex, 0, 0));
				dlg_message(g_dialog, g_text_output_buf);
				event_set_cell(ev, 10, 0x13, 10, 0, 1);
				group_drop_item(g_group, ITEM_CRANK);
				g_dng11_winch_state = 1;
			} else {
				/* BINGE3.LTX[24]: "Da fehlt eigentlich nur noch eine Kurbel!" */
				dlg_message(g_dialog, ev->txt[24]);
				g_dng11_air_counter = 1;
			}
		}
	} else if (g_dng11_winch_state == 1) {
		/* BINGE3.LTX[13]: "weiterkurbeln?" */
		/* BINGE3.LTX[14]: "oder die Sache nochmal genauer anschauen?" */
		choice = dlg_choose(g_dialog, ev->txt[0], 2, ev->txt[13], ev->txt[14]);
		while (choice == 1) {
			/* BINGE3.LTX[15]: "%s kurbelt fleissig weiter, aber es scheint nichts zu passieren." */
			sprintf(g_text_output_buf, ev->txt[15], h->name);
			dlg_message(g_dialog, g_text_output_buf);
			choice = dlg_choose(g_dialog, ev->txt[0], 2, ev->txt[13], ev->txt[14]);
		}
		if (choice == 2) {
			h = get_best_hero_skill(g_group, TA_SCHLOESSER);
			/* BINGE3.LTX[16]: "Nach eingehender Betrachtung zieht %s die Kurbel aus dem Lager und entfernt mit einem
			 * Handgriff eine Abdeckung von der Wand, so dass die gesamte Mechanik vor Euch liegt." */
			sprintf(g_text_output_buf, ev->txt[16], h->name);
			dlg_message(g_dialog, g_text_output_buf);
			/* BINGE3.LTX[17]: "Gezahnte Raeder werden ueber die Kurbel betrieben. Oberhalb der Mechanik haengt das
			 * abgerissene Ende einer Kette. In der Mechanik steckt ebenfalls ein Stueck der Kette. Der Hebel ist ein
			 * Feststellhebel und sorgt offenbar dafuer, dass das Gewicht des Blockes die Winde nicht zurueckdreht. Es ist
			 * klar, dass das ganze mit zerrissener Kette nicht funktionieren kann." */
			dlg_message(g_dialog, ev->txt[17]);
			idx = group_find_item(g_group, ITEM_CHAIN);
			if (idx != -1) {
				h = g_group->heroes[idx];
				if (!check_hero3(h))
					h = get_second_active_hero(g_group);
				if (h == 0)
					h = get_first_active_hero(g_group);
				/* BINGE3.LTX[18]: "Unter deftigen Fluechen fummelt %s das gefundene Kettenstueck in die Mechanik." */
				sprintf(g_text_output_buf, ev->txt[18], h->name);
				dlg_message(g_dialog, g_text_output_buf);
				timewarp_hours(g_date, 2);
				/* BINGE3.LTX[19]: "Mit neuem Mut faengt %s an zu kurbeln. Es quietscht gewaltig, dann hoert Ihr Kettenrasseln
				 * und ein mahlendes Geraeusch. Dann hebt sich nicht etwa der Block hinter Euch, sondern ein zweiter, der
				 * bisher das andere Ende des Ganges blockiert hat. Mit einem dumpfen Geraeusch verschwindet er in der Decke." */
				sprintf(g_text_output_buf, ev->txt[19], h->name);
				dlg_message(g_dialog, g_text_output_buf);
				group_drop_item(g_group, ITEM_CHAIN);
				event_set_cell(ev, 8, 0x10, 0, 0, -1);
				automap_cell_write(g_map, 0x302, 0x10, 8, 2);
				automap_cell_write(g_map, 0x302, 0x10, 9, 2);
				g_dng11_winch_state = 2;
				g_dng11_air_counter = 0;
			} else {
				idx = group_find_item(g_group, ITEM_ROPE);
				if (idx != -1) {
#if FIX_0133
					/* Fix #133: Suche nicht vor den Anfang des Heldenfelds laufen lassen */
					for (i = 1, h2 = g_group->heroes[g_group->nr_heroes - i]; i < g_group->nr_heroes && !check_hero3(h2);
					     i++, h2 = g_group->heroes[g_group->nr_heroes - i])
						;
#else
					for (i = 1, h2 = g_group->heroes[g_group->nr_heroes - i]; !check_hero3(h2);
					     i++, h2 = g_group->heroes[g_group->nr_heroes - i])
						;
#endif
					h = g_group->heroes[idx];
					if (!check_hero3(h))
						h = get_second_active_hero(g_group);
					if (h == 0)
						h = get_first_active_hero(g_group);
					/* BINGE3.LTX[20]: "Mit Hilfe eines Seiles versucht %s die Bruchstelle zu ueberbruecken. Es vergehen einige
					 * Stunden, bis %s das Ding so reingefummelt hat, dass es das Gewicht wohl fuer kurze Zeit halten kann." */
					sprintf(g_text_output_buf, ev->txt[20], h->name, grammar_pronoun(h->sex, 0, 0));
					dlg_message(g_dialog, g_text_output_buf);
					timewarp_hours(g_date, 5);
					h = get_first_active_hero(g_group);
					if ((signed short)g_group->nr_heroes > 1)
						/* BINGE3.LTX[22]: ", die anderen hinterher" */
						sprintf(g_text_buf2, "%s", ev->txt[22]);
					else
						/* BINGE3.LTX[23]: "." */
						sprintf(g_text_buf2, "%s", ev->txt[23]);
					/* BINGE3.LTX[21]: "%s sichert die Kurbel mit dem Hebel und sieht zu, dass %s unter dem Block durchkommt%s -
					 * Glueck gehabt! Kaum hat %s die Stelle passiert, als aus der Mechanik ein schnalzendes Geraeusch ertoent,
					 * das aber sofort wieder von dem Block uebertoent wird, der mit lautem Rumpeln herunter kommt und mit einem
					 * noch viel lauterem Knall auf dem Boden landet." */
					sprintf(g_text_output_buf, ev->txt[21], h->name, grammar_pronoun(h->sex, 0, 0), g_text_buf2, h2->name);
					dlg_message(g_dialog, g_text_output_buf);
					group_drop_item(g_group, ITEM_ROPE);
					group_set_screen_pos(0x10, 7);
					g_group->dir = 0;
					g_cam_angle = 0x100;
					cam_set_from_group();
					g_dng11_winch_state = 2;
					g_dng11_air_counter = 0;
				} else
					g_dng11_air_counter = 1;
			}
		}
	}
	dlg_set16(g_dialog, old);
}

void dng11_block_falls(struct area_events far *ev)
{
	signed short old;
	struct group far *g;
	register signed short i;

	old = dlg_set16(g_dialog, 5);
	if (g_dng11_block_down != 0)
		/* BINGE3.LTX[8]: "Zwischen dem Block und der Wand verbleibt nur eine duenne Fuge. Der Block ist im Boden
		 * versenkt, so dass es dort keine Fuge gibt. Hier kommt Ihr bestimmt nicht wieder durch." */
		dlg_message(g_dialog, ev->txt[8]);
	else {
		for (i = 0, g = g_groups; i < 6; g = &g_groups[++i]) {
			if (g->dungeon == g_loaded_dungeon && g->x == 0x12 && g->y >= 0xb && g->y <= 0xd) {
#if FIX_0133
				/* Fix #133: Position nur einmal setzen (old_x/old_y behalten die vorherige) */
				g->set_x_screen(0x12);
				g->set_y_screen(10);
#else
				g->set_x_v(g->set_x_screen(0x12));
				g->set_y_v(g->set_y_screen(10));
#endif
				if (g == g_group)
					cam_set_from_group();
			}
		}
		event_set_cell(ev, 0xc, 0x12, 0xe, 0, -1);
		automap_cell_write(g_map, 0x302, 0x12, 0xc, 6);
		automap_cell_write(g_map, 0x302, 0x12, 0xd, 4);
		view_frame();
		/* BINGE3.LTX[7]: "Ploetzlich hoert Ihr hinter Euch ein lautes Rumpeln. Mit einem dumpfen Knall senkt sich ein
		 * massiver Block vor den Eingang." */
		dlg_message(g_dialog, ev->txt[7]);
		g_dng11_block_down = 1;
	}
	dlg_set16(g_dialog, old);
}
