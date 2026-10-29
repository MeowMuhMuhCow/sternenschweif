/* OVL129 - Overlay O129 (SCHWEIF.EXE): Ereignisobjekt eines Dungeons, struct area_events (0x12 Bytes: Texte
 * txt_buf/txt aus txt_load_list(g_font, a, b), Dungeon type +8, Vorgabe desc +0a = g_area_descs[type] mit dem
 * Rueckruf fuer Kartenereignisse). O014 legt das Objekt je Dungeon an (g_area_events), die Ereignismodule rufen die
 * Methoden ueber die freien Namen (event_set_cell ...). Konstruktor (type, a, b): im Dungeon 4 bzw. 3 verlaesst der
 * NSC (group_get_npc, ausser npc_id 5 bzw. 2) die Gruppe - Meldung txt[0x13]/[0x16] bzw. txt[0x3f]/[0x80] mit
 * Pronomen grammar_pronoun(sex, ..), group_remove_npc, Portraets neu (screen_portraits(g_screen)). Destruktor:
 * Texte freigeben. set_cell(y, x, type, flags, rot): Feld g_cells[y + g_map_y0][x + g_map_x0] setzen, rot + 1
 * (ausser rot == -1), cell_forget. fight1/fight2: Kurzformen von fight mit 1 bzw. 2 Gegnergruppen (Rest 0).
 * fight(12 Werte): drei Gegnergruppen nach g_encounters[0..2], Kampf start_fight(0xab, 0). set_cell_bits(y, x, v):
 * Feldbits 12/13 = v und CELL_CHANGED, wenn CELL_CHANGED noch frei. teleport(from): Versetzung nach Ereignis
 * (Dungeon 3: per W100 eines von drei Zielen je Herkunft 0x0b/0x12/0x1e; Dungeon 5: Herkunft 0x1d/0x15 mit
 * zufaelliger Richtung W4 - 1), Klang 0x95 (Schleife bis g_teleport_spin >= 0x80), Blickrichtung group.dir und
 * g_cam_angle, group_set_screen_pos(x, y), g_skip_cell_event = 1. notice(what): Meldung txt[0x21] mit dem ersten
 * Helden, wenn test_skill(.., TA_SINNESSCHAERFE, 10) gelingt, sonst txt[0x1b]. dragon_claw(h) (frueher creature):
 * Dungeon 3, Geheimfach mit Helm und Drachenklaue (FINAL.LTX[36..54]), der Held mit dem kleinsten MU weigert sich
 * weiterzugehen - Tierkunde-Bester (get_best_hero_skill(.., TA_TIERKUNDE)), Menue dlg_choose (txt[0x27..0x2a]) bis
 * g_coward_stuck == 0: 1: sub_hero_le(h2, W6); 2: Held waehlen (get_active_hero), test_attrib(.., ATTRIB_CH, k)
 * mit k += 2, ab k >= 8 Gegenprobe test_attrib(.., ATTRIB_JZ, 2 - k); 3: Zauberkundiger (get_active_mage), bei
 * h->ae >= 5 und test_spell(h, SP_SANFTMUT, 0) > 0 -> sub_ae_splash(.., 6); 4: make_group(1, h2).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL129.CPP   -> 3304/3304 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - C++: Konstruktor mit Rumpf (if (this == 0 && (this = new(0x12)) == 0) -> jmp near ans Ende), Destruktor
 *   mit Rumpf (Test this, flags & 1 -> delete), Strukturkopie desc = g_area_descs[type] per SCOPY@ (8 Bytes).
 * - notice: "get_first_active_hero(g_group)->get_name()" mit inline-Getter hero::get_name() - die this-Kopie ist
 *   [bp-4] (ein Zwischenzeiger im Argument wuerde direkt dx pushen). Argument what wird vor dem inneren Aufruf
 *   gepusht.
 * - dragon_claw: h2 = get_best_hero_skill(..) als eigene Anweisung (neu laden); h3 per "if ((h3 = ..) != 0)" (or ax,dx).
 * - teleport: inline-Setter group::set_dir(signed short) -> Parameterkopie [bp-4] (Wort), Byte gespeichert;
 *   case-Rumpfe in Quellreihenfolge 0x1e/0x0b/0x12 bzw. 0x1d/0x15 (Vergleiche sortiert), case 3 faellt in case 5.
 * - set_cell: "p->rot = ++rot" (inc word [bp+12h]); Kartenzeiger g_cells als struct map_cell (far *)[64]
 *   (shl 8 / shl 2), erster Index = Zeile y.
 * - set_cell_bits: flags als register unsigned short (CX, keine Aufrufe).
 * - fight: start_fight liefert hier short (Aufruf ueber gecasteten Funktionszeiger; s_proto.h: signed char -> cbw).
 * - Ohne Public: g_area_descs (ds:5d9c, in _g_5d52), g_encounters (ds:666b = _g_664f + 0x1c) - cobjfix ordnet zu.
 * Originalfehler: Konstruktor Dungeon 3 mit check_hero(h) == 0 formatiert txt[0x80], zeigt ihn aber nie an
 * (dlg_message fehlt). teleport: x/y/dir bleiben uninitialisiert, wenn Dungeon oder Herkunft zu keinem case passt,
 * und gehen trotzdem an group_set_screen_pos / g_group->dir; case 3 ohne break (harmlos, from passt nie zu
 * case 5). notice ruft get_first_active_hero zweimal.
 */
#include <stdio.h>
#include "schweif.h"

area_events::area_events(register signed short t, signed short txt_file, signed short txt_n)
{
	struct hero far *npc;

	type = t;
	txt_load_list(g_font, txt_file, txt_n, &txt_buf, &txt);
	desc = g_area_descs[t];
	if (t == 4) {
		if ((npc = group_get_npc(g_group)) != 0 && npc->npc_id != 5) {
			if (check_hero(npc))
				sprintf(g_text_output_buf, txt[0x13], npc->name, grammar_pronoun(npc->sex, 0, 0),
					grammar_pronoun(npc->sex, 0, 0));
			else
				sprintf(g_text_output_buf, txt[0x16], npc->name, grammar_pronoun(npc->sex, 0, 0));
			dlg_message(g_dialog, g_text_output_buf);
			group_remove_npc(g_group, npc);
			screen_portraits(g_screen);
		}
	} else if (t == 3) {
		if ((npc = group_get_npc(g_group)) != 0 && npc->npc_id != 2) {
			if (check_hero(npc)) {
				sprintf(g_text_output_buf, txt[0x3f], npc->name, grammar_pronoun(npc->sex, 0, 0),
					grammar_pronoun(npc->sex, 1, 1));
				dlg_message(g_dialog, g_text_output_buf);
			} else {
#if FIX_0059
				/* Fix #59: Abschiedstext auch anzeigen */
				sprintf(g_text_output_buf, txt[0x80], npc->name, grammar_pronoun(npc->sex, 2, 0));
				dlg_message(g_dialog, g_text_output_buf);
#else
				sprintf(g_text_output_buf, txt[0x80], npc->name, grammar_pronoun(npc->sex, 2, 0));
#endif
			}
			group_remove_npc(g_group, npc);
			screen_portraits(g_screen);
		}
	}
}

area_events::~area_events()
{
	mem_free(txt_buf);
	mem_free(txt);
}

void area_events::set_cell(register signed short y, register signed short x, signed char cell_type, signed short flags,
	signed short rot)
{
	struct map_cell far *p;

	y += g_map_y0;
	x += g_map_x0;
	p = &g_cells[y][x];
	p->type = cell_type;
	p->flags = flags;
	if (rot != -1)
		p->rot = ++rot;
	cell_forget(y, x);
}

signed short area_events::fight1(signed short id, signed short n, signed short c)
{
	return fight(id, n, c, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

signed short area_events::fight2(signed short id, signed short n, signed short c, signed short id2, signed short n2,
	signed short c2)
{
	return fight(id, n, c, 0, id2, n2, c2, 0, 0, 0, 0, 0);
}

signed short area_events::fight(signed short id, signed short n, signed short c, signed short d,
	signed short id2, signed short n2, signed short c2, signed short d2,
	signed short id3, signed short n3, signed short c3, signed short d3)
{
	signed short res;
	struct encounter far *e;

	e = &g_encounters[0];
	e->id = id;
	e->n = n;
	e->f2 = c;
	e->f3 = d;
	e = &g_encounters[1];
	e->id = id2;
	e->n = n2;
	e->f2 = c2;
	e->f3 = d2;
	e = &g_encounters[2];
	e->id = id3;
	e->n = n3;
	e->f2 = c3;
	e->f3 = d3;
	res = ((signed short (*)(signed short, signed short))start_fight)(0xab, 0);	/* Rueckgabe als short (s_proto.h: signed char) */
	return res;
}

void area_events::set_cell_bits(signed short y, signed short x, signed short bits)
{
	struct map_cell far *p;
	register unsigned short flags;

	p = &g_cells[y][x];
	flags = p->flags;
	if (!(flags & CELL_CHANGED)) {
		flags &= 0xcfff;
		p->flags = flags | bits | CELL_CHANGED;
	}
}

void area_events::teleport(signed short from)
{
	signed short y;
	register signed short dir;
	register signed short x;

#if FIX_0060
	/* Fix #60: x als Merker vorbelegen */
	if (!g_skip_cell_event) {
		x = -1;
		switch (type) {
#else
	if (!g_skip_cell_event) {
		switch (type) {
#endif
		case 3:
			if (random_schweif(100) <= 50) {
				switch (from) {
				case 0x1e:
					x = 0x0b;
					y = 0x13;
					dir = 2;
					break;
				case 0x0b:
					x = 0x12;
					y = 0x16;
					dir = 0;
					break;
				case 0x12:
					x = 0x1e;
					y = 0x19;
					dir = 2;
					break;
				}
			} else {
				switch (from) {
				case 0x1e:
					x = 0x12;
					y = 0x16;
					dir = 1;
					break;
				case 0x0b:
					x = 0x1e;
					y = 0x19;
					dir = 2;
					break;
				case 0x12:
					x = 0x0b;
					y = 0x13;
					dir = 2;
					break;
				}
			}
		case 5:
			switch (from) {
			case 0x1d:
				x = 0x1b;
				y = 0x1d;
				dir = random_schweif(4) - 1;
				break;
			case 0x15:
				x = 0x17;
				y = 0x1e;
				dir = random_schweif(4) - 1;
				break;
			}
		}
#if FIX_0060
		/* Fix #60: ohne passenden Fall nicht mit uninitialisierten x/y/dir teleportieren */
		if (x == -1)
			return;
		snd_play(g_snd_fx, 0x95);
		g_teleport_spin = 0;
#else
		snd_play(g_snd_fx, 0x95);
		g_teleport_spin = 0;
#endif
		do {
			poll_input();
			view_draw();
		} while (g_teleport_spin < 0x80);
		g_teleport_spin = -1;
		snd_play(g_snd_fx, 0x95);
		g_group->set_dir(dir);
		g_cam_angle = (dir << 8) + 0x100;
		group_set_screen_pos(x, y);
		g_skip_cell_event = 1;
	}
}

void area_events::notice(char far *what)
{
	if (!g_dng03_password_solved && test_skill(get_first_active_hero(g_group), TA_SINNESSCHAERFE, 10) >= 0) {
		sprintf(g_text_output_buf, txt[0x21], get_first_active_hero(g_group)->get_name(), what);
		dlg_message(g_dialog, g_text_output_buf);
	} else {
		dlg_message(g_dialog, txt[0x1b]);
	}
}

void area_events::dragon_claw(struct hero far *h)
{
	signed short old;
	struct hero far *h2;
	struct hero far *h3;
	register signed short sel;
	register signed short mod;

	g_dragon_claw_found = g_coward_stuck = 1;
	group_give_item(g_group, 0x147, 1);
	group_give_item(g_group, 0x148, 1);
	old = dlg_set16(g_dialog, 5);
	mod = 4;
	sprintf(g_text_output_buf, txt[0x24], h->name, grammar_pronoun(h->sex, 0, 0));
	dlg_message(g_dialog, g_text_output_buf);
	h2 = get_best_hero_skill(g_group, TA_TIERKUNDE);
	sprintf(g_text_output_buf, txt[0x25], h2->name);
	dlg_message(g_dialog, g_text_output_buf);
	if (group_count_alive(g_group) > 1) {
		h2 = get_hero_attrib_min(g_group, ATTRIB_MU);
		while (g_coward_stuck == 1) {
			sprintf(g_text_output_buf, txt[0x26], h2->name, grammar_pronoun(h2->sex, 0, 0),
				grammar_pronoun(h2->sex, 3, 0), grammar_pronoun(h2->sex, 0, 0), h2->name);
			do {
				sel = dlg_choose(g_dialog, g_text_output_buf, 4, txt[0x27], txt[0x28], txt[0x29], txt[0x2a]);
			} while (sel == -1);
			g_hero_excluded = group_hero_pos(g_group, h2);
			switch (sel) {
			case 1:
				sprintf(g_text_output_buf, txt[0x2b], h2->name);
				dlg_message(g_dialog, g_text_output_buf);
				sub_hero_le(h2, random_schweif(6));
				g_coward_stuck = 0;
				break;
			case 2:
				sprintf(g_text_output_buf, txt[0x31], h2->name);
				if ((h3 = get_active_hero(g_group, g_text_output_buf)) != 0) {
					timewarp_hours(g_date, 1);
					if (test_attrib(h3, ATTRIB_CH, mod) >= 0) {
						sprintf(g_text_output_buf, txt[0x33], h3->name, h2->name);
						g_coward_stuck = 0;
					} else {
						if (mod < 8) {
							sprintf(g_text_output_buf, txt[0x34], h2->name, grammar_pronoun(h2->sex, 0, 0));
						} else {
							sprintf(g_text_output_buf, txt[0x35], h2->name, grammar_pronoun(h2->sex, 0, 0),
								h3->name);
							sub_hero_le(h3, random_schweif(6));
							if (test_attrib(h3, ATTRIB_JZ, 2 - mod) > 0) {
								dlg_message(g_dialog, g_text_output_buf);
								sprintf(g_text_output_buf, txt[0x36], h3->name, h2->name);
								sub_hero_le(h2, random_schweif(10));
							}
						}
						mod += 2;
					}
					dlg_message(g_dialog, g_text_output_buf);
				} else {
					dlg_message(g_dialog, txt[0x32]);
				}
				break;
			case 3:
				if ((h3 = get_active_mage(g_group, txt[0x2c])) != 0) {
					if (h->ae >= 5 && test_spell(h, SP_SANFTMUT, 0) > 0) {
						sub_ae_splash(h3, 6);
						sprintf(g_text_output_buf, txt[0x2d], h2->name);
						g_coward_stuck = 0;
					} else {
						sprintf(g_text_output_buf, txt[0x2e], h2->name);
					}
				} else {
					sprintf(g_text_output_buf, txt[0x2f], h2->name);
				}
				dlg_message(g_dialog, g_text_output_buf);
				break;
			case 4:
				make_group(1, h2);
				sprintf(g_text_output_buf, txt[0x30], h2->name, grammar_pronoun(h2->sex, 2, 0));
				dlg_message(g_dialog, g_text_output_buf);
				g_coward_stuck = 0;
				break;
			}
		}
	}
	dlg_set16(g_dialog, old);
}
