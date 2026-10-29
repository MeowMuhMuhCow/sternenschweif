/* OVL102 - Overlay O102 (Segment 0x214b in SCHWEIF.EXE): Sonderfunktionen der Dialoge - O101 ruft f_0000(d)
 * fuer Zustaende mit Bit 15. Eine einzige Funktion (5145 Bytes): if/else-Kette ueber den Dialog d->id (0x2b0/
 * 0x4d8, 0x90, 0x2b1..0x2c4, 0x12a, 0x2e4, 0x3d5, 0x13c), darin ueber Partner d->partner und Zustand d->state:
 * Folgezustand d->next / Titel d->title / Ende d->done setzen, Merker ds:25c6..ds:262a, Aktionen in O013
 * (Dialogaktionen), O034, O200 (Geld), O238 (Gegenstandsliste: new/delete bzw. lokales Objekt), O018_0C4D
 * (Texte nachladen), Proben O076_06D5. Dreimal "Ort verlassen" (Makro LEAVE: O042_0C9A, O010_12EE, +33 = 0,
 * set20/setdir/set22 je doppelt geschachtelt, O201_0DF8, +37/+38 = 0x67). 0x13c: Held waehlen (O088_04A5) und
 * Gegenstand 0x13c suchen (O083_02EA).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL102.CPP   -> 5145/5145 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (-Oi ebenso GLEICH; ohne -Og bzw. mit -Ov 12 Bytes zu lang - Epilogkopien am Ende jedes Dialogzweigs).
 * - Erstentwurf sofort GLEICH. struct dlg aus OVL101.CPP, struct party mit den inline-Settern aus OVL030.CPP:
 *   "g_c4c5->set20(g_c4c5->set20(0))" usw. -> je Expansion eine Parameterkopie ([bp-1]/[bp-4]/[bp-5] usw., je
 *   LEAVE-Block eigene Plaetze), gemeinsamer -Og-Byte-Zwischenwert [bp-0eh] fuer dir in setdir.
 * - Block-Locals: "List far *l" (0x2bb) und "struct hero far *h" (0x13c) teilen [bp-12h]; lokales List-Objekt
 *   (0x2bc) bei [bp-28h]. "r = 0; r = O018_0C4D(..)" (xor si,si vor dem Aufruf).
 * - "g_25ce = g_topic_siebenquell = -1" (rechts zuerst), "(signed long)(signed short)g_260c * 10000" (cbw/movsx/imul).
 * Originalfehler/Auffaelliges: 0x13c - "do {..} while (r == -1)": bricht der Spieler die Heldenwahl ab (h == 0),
 *   laeuft die Schleife endlos weiter; danach ist r nie -1, "d->next = 7" ist toter Code.
 * (Namensfassung mit schweif.h, struct talk_desc/struct group: 19.10.2026 GLEICH, LINKGLEICH) f_0000 talk_special;
 *   LEAVE mit den Settern set_town/set_loctype/set_dungeon von struct group; List = class Stash (O238).
 */
#include "schweif.h"

/* Ort verlassen: Musik aus, Bildschirm auslagern, Strecke/Stadt/Ortsart/Dungeon loeschen (Setter je doppelt) */
#define LEAVE() \
	snd_obj_stop(g_snd_music); \
	swap_out_screen(); \
	g_group->route_pos = 0; \
	g_group->set_town(g_group->set_town(0)); \
	g_group->set_loctype(g_group->set_loctype(0)); \
	g_group->set_dungeon(g_group->set_dungeon(0)); \
	group_set_on_route(g_group, 1); \
	g_group->route_from = 0x67; \
	g_group->route_to = 0x67;

void talk_special(struct talk_desc far *d)
{
	register signed short r;

	if (d->id == 0x2b0 || d->id == 0x4d8) {
		if (d->partner == 0) {
			if (d->state == 3)
				talk_hand_over_weapons(d);
			else if (d->state == 4)
				talk_hand_over_items(d);
			else if (d->state == 5)
				group_imprison(2);
			else if (d->state == 6)
				talk_reward(d, 1);
			else if (d->state == 7)
				talk_reward(d, 2);
			else if (d->state == 8)
				load_portrait_buf(0xaa);
			else if (d->state == 9)
				talk_reward(d, 3);
			else if (d->state == 10)
				group_imprison(2);
		}
	} else if (d->id == 0x90) {
		if (d->partner == 1) {
			if (d->state == 3)
				load_portrait_buf(0xac);
			else if (d->state == 6)
				d->title = 0x0e;
			else if (d->state == 9)
				dlg_message(g_dialog, d->txt[19]);
			else if (d->state == 0x0b || d->state == 0x0e)
				group_add_money(g_group, (signed long)group_count_alive(g_group) * 10);
			else if (d->state == 0x0c)
				talk_check_dwarf(d);
			else if (d->state == 0x0f)
				talk_check_group_size(d);
			else if (d->state == 0x13)
				g_game_state = 1;
			else if (d->state == 0x16)
				talk_take_heroes(d);
			else if (d->state == 0x18)
				talk_give_lowangen_key(d);
		} else if (d->partner == 2) {
			if (d->state == 1 || d->state == 7)
				group_leave_place(0);
			if (d->state == 3)
				talk_check_quest_anim(d);
			if (d->state == 5)
				talk_rejoin_leave(d);
			else if (d->state == 8)
				talk_game_won(d);
			else if (d->state == 10) {
				LEAVE();
			}
		} else if (d->partner == 3) {
			if (d->state == 0)
				talk_npc_leaves(d);
			else if (d->state == 3)
				talk_rejoin_leave(d);
			else if (d->state == 5)
				group_leave_place(0);
			else if (d->state == 6)
				talk_game_won(d);
			else if (d->state == 8) {
				LEAVE();
			}
		} else if (d->partner == 4) {
			if (d->state == 0) {
				talk_npc_leaves(d);
				g_eolan_state = 3;
			}
			if (d->state == 3) {
				LEAVE();
			}
		} else if (d->partner == 5 && d->state == 1) {
			dlg_message(g_dialog, d->txt[64]);
			g_game_state = 1;
		}
	} else if (d->id == 0x2b1) {
		if (d->partner == 0) {
			if (d->state == 0)
				d->title = 0x0e;
			else if (d->state == 4)
				talk_cast_analues(d);
			else if (d->state == 7)
				group_sub_money(g_group, 100);
		}
	} else if (d->id == 0x2b2) {
		if (d->partner == 0) {
			if (d->state == 0)
				talk_check_parcel(d);
			else if (d->state == 1 || d->state == 4)
				d->title = 7;
			else if (d->state == 5)
				g_low_deringorn_state = 1;
		}
	} else if (d->id == 0x2b3) {
		if (d->partner == 0 && d->state == 0)
			d->title = 5;
	} else if (d->id == 0x2b5) {
		if (d->partner == 0) {
			if (d->state == 0)
				d->title = 0x10;
			else if (d->state == 2) {
				talk_lose_items(d, 0x32);
				g_low_guard_talk = 1;
			} else if (d->state == 4) {
				talk_lose_items(d, 0x1e);
				g_low_guard_talk = 1;
			} else if (d->state == 5)
				talk_lose_items(d, 0);
			else if (d->state == 10)
				group_scatter(0, 1);
		}
	} else if (d->id == 0x2b6) {
		if (d->partner == 0) {
			if (d->state == 0) {
				if (g_low_gate1_open) {
					d->title = 0x0a;
					load_portrait_buf(0xb1);
				} else if (g_low_gate2_open) {
					d->title = 0x0b;
					load_portrait_buf(0xb2);
				}
			} else if (d->state == 1)
				d->title = 0x0c;
			else if (d->state == 6) {
				g_fight_flag = 1;
				talk_fight_95(d);
			}
		}
	} else if (d->id == 0x2ba) {
		if (d->partner == 0) {
			if (d->state == 0) {
				if (g_low_dragan_task1)
					d->next = 1;
				else
					d->next = 2;
			} else if (d->state == 3) {
				if (g_low_dragan_task1)
					d->next = 4;
				else
					d->next = 5;
			} else if (d->state == 8) {
				switch (talk_hand_over_document(d)) {
				case 1:
					d->next = 0x0a;
					break;
				case 2:
					d->next = 0x0c;
					break;
				default:
					d->next = 9;
					break;
				}
			} else if (d->state == 0x0c || d->state == 0x0e || d->state == 0x10)
				g_altrogolan_absent = 1;
			else if (d->state == 0x0d) {
				if (test_attrib(get_first_active_hero(g_group), 2, 4) > 0)
					d->next = 0x0f;
				else
					d->next = 0x0e;
			} else if (d->state == 0x11) {
				g_altrogolan_absent = -1;
				g_npc8_days = 1;
				add_npc(8);
			}
		}
	} else if (d->id == 0x2bb) {
		if (d->partner == 1 && d->state == 7) {
			Stash far *l;

			r = 0;
			r = start_fight(0xa3, 0);
			if (!r) {
				g_low_ailian_state = g_topic_siebenquell = -1;
				g_gavron_chase = 0xfc;
				l = new Stash(0x37, 0);
				delete l;
			} else
				g_low_ailian_state = 5;
		} else if (d->partner == 2 && d->state == 4) {
			Stash far *l;

			r = 0;
			r = start_fight(0xa4, 0);
			if (!r) {
				g_low_ailian_state = g_topic_siebenquell = -1;
				g_gavron_chase = 0xfc;
				l = new Stash(0x37, 0);
				delete l;
			} else
				g_low_ailian_state = 5;
		}
	} else if (d->id == 0x2bc) {
		if (d->partner == 0) {
			if (d->state == 0) {
				g_vindaria_state = 2;
				g_vindaria_choice = 0;
			} else if (d->state == 1 || d->state == 2)
				g_vindaria_choice = d->state;
			else if (d->state == 3)
				d->next = g_vindaria_choice + 4;
			else if (d->state == 4) {
				if (talk_fight_97(d, 0))
					d->next = 7;
			} else if (d->state == 5 || d->state == 6) {
				if (talk_fight_97(d, 1))
					d->next = 7;
			} else if (d->state == 8) {
				if (g_vindaria_choice == 2) {
					Stash l(0, 0);

					g_vindaria_state = -1;
				} else {
					Stash l(1, 0);

					g_vindaria_state = -2;
				}
			}
		}
	} else if (d->id == 0x2bd) {
		if (d->partner == 0) {
			if (d->state == 1) {
				load_portrait_buf(0xb6);
				d->title = 0x14;
			} else if (d->state == 2) {
				if (g_dragan_tip_jandora)
					d->next = 7;
				else
					d->next = 3;
			} else if (d->state == 5 || d->state == 10) {
				load_portrait_buf(0xb5);
				d->title = 0x15;
			} else if (d->state == 0x0b) {
				if (talk_price(d, 0))
					d->next = 0x11;
				else
					d->next = 0x0c;
			} else if (d->state == 0x0d) {
				if (talk_price(d, 1))
					d->next = 0x0f;
				else
					d->next = 0x0e;
			} else if (d->state == 0x10) {
				load_portrait_buf(0xb5);
				group_sub_money(g_group, (signed long)(signed short)g_price_level * 10000);
				g_eolan_state = 1;
				diary_add(g_diary, d->txt[23], 0);
			}
		} else if (d->partner == 1) {
			if (d->state == 3) {
				if (talk_price(d, 2))
					d->next = 4;
				else {
					d->next = 7;
					g_price_level++;
					if (g_price_level > 100)
						g_price_level = 100;
				}
			} else if (d->state == 6) {
				group_sub_money(g_group, (signed long)(signed short)g_price_level * 10000);
				g_eolan_state = 1;
				g_dragan_tip_jandora = 0;
				diary_add(g_diary, d->txt[23], 0);
			}
		}
	} else if (d->id == 0x2c0) {
		if (d->partner == 0) {
			if (d->state == 2) {
				if (test_attrib(get_first_active_hero(g_group), 2, 5) > 0)
					d->next = 3;
				else
					d->next = 4;
			} else if (d->state == 3)
				g_low_gavron_house = 0x40;
		}
	} else if (d->id == 0x2c2) {
		if (d->partner == 0) {
			if (d->state == 0) {
				if (g_dragan_tip_raisha)
					d->next = 1;
				else
					d->next = 2;
			} else if (d->state == 5) {
				if (g_dragan_tip_raisha == 1)
					d->next = 6;
				else if (g_dragan_tip_raisha == 3)
					d->next = 7;
				else
					d->next = 8;
			} else if (d->state == 0x0c)
				talk_set_peddler(d, 1);
			else if (d->state == 0x0d)
				talk_set_peddler(d, 2);
		}
	} else if (d->id == 0x2c3) {
		if (d->partner == 0 && d->state == 0) {
			talk_set_peddler(d, 3);
			g_low_falkert_known = 0;
		}
	} else if (d->id == 0x2c4) {
		if (d->partner == 0) {
			if (d->state == 3 || d->state == 6) {
				g_low_deringorn_state = 2;
				group_drop_item(g_group, ITEM_PARCEL);
			}
			if (d->state == 6)
				group_sub_le_range(g_group, 1, 4);
		}
	} else if (d->id == 0x12a) {
		if (d->partner == 0) {
			if (d->state == 0) {
				if (g_swamp_chief_refused || g_swamp_chief_offended)
					d->next = -1;
				else if (g_dng04_monster_killed)
					d->next = 8;
				else
					d->next = 1;
			} else if (d->state == 2) {
				g_swamp_chief_offended = 1;
				talk_nop2(d);
			} else if (d->state == 4) {
				g_swamp_chief_refused = 1;
				talk_nop3(d);
			} else if (d->state == 9) {
				swap_out_screen();
				if (conv_run(g_dialog2, 0xa3))
					timewarp_minutes(g_date, 30);
				swap_in_screen();
			}
		} else if (d->partner == 1) {
			if (d->state == 0) {
				if (g_eolan_state < 2)
					d->next = 7;
				else if (!g_dng04_tower_open)
					d->next = 1;
				else if (g_dng04_tower_open == -1)
					d->done = 1;
				else if (group_find_item(g_group, 0x151) != -1)
					d->next = 8;
				else
					d->next = 7;
			} else if (d->state == 6)
				g_dng04_tower_open = 1;
			else if (d->state == 7)
				dlg_message(g_dialog, d->txt[24]);
			else if (d->state == 9) {
				if (!start_fight(0xa9, 1))
					g_dng04_tower_open = -1;
			}
		}
	} else if (d->id == 0x2e4) {
		if (d->partner == 0) {
			if (d->state == 3) {
				g_fight_flag = 1;
				g_fight_max_enemies = g_street_thugs;
				start_fight(0x9d, 0);
			} else if (d->state == 4)
				lowangen_drop_food();
		} else if (d->partner == 1) {
			if (d->state == 3) {
				r = random_schweif(g_street_guards);
				if (r <= 2)
					d->next = 4;
				else if (r <= 4)
					d->next = 5;
				else
					d->next = 6;
			} else if (d->state == 4)
				group_scatter(1, 0);
			else if (d->state == 5)
				group_scatter(2, 0);
			else if (d->state == 6) {
				talk_lose_items(d, 0);
				talk_lose_items(d, 0x1e);
			} else if (d->state == 7)
				talk_lose_items(d, 0);
			else if (d->state == 8) {
				r = 0;
				g_fight_flag = 1;
				g_fight_max_enemies = g_street_guards;
				r = start_fight(0x9e, 0);
				switch (r) {
				case 0:
					d->next = 9;
					break;
				case 1:
					d->next = 3;
					break;
				case 2:
					break;
				case 3:
					group_scatter(3, 0);
					break;
				}
			} else if (d->state == 9)
				group_scatter(3, 0);
		}
	} else if (d->id == 0x3d5) {
		if (d->partner == 0) {
			if (d->state == 3)
				add_npc(1);
		} else if (d->partner == 1 && d->state == 3)
			add_npc(3);
	} else if (d->id == 0x13c && d->partner == 0 && (d->state == 1 || d->state == 5)) {
		struct hero far *h;

		h = 0;
		r = -1;
		do {
			h = get_any_hero(g_group, d->txt[4]);
			if (h) {
				r = select_item(h, d->txt[5], 1);
				if (r != -1)
					r = h->inv[r].item_id;
			}
#if FIX_0135
		/* Fix #135: Abbruch der Helden-/Gegenstandswahl beendet die Abfrage (d->next = 7) */
		} while (0);
		if (r == ITEM_STONE_MEDAILLON) {
#else
		} while (r == -1);
		if (r == ITEM_STONE_MEDAILLON) {
#endif
			g_dng16_medallion_given = 1;
			return;
		}
		if (r != -1) {
			d->next = 4;
			return;
		}
		d->next = 7;
	}
}
