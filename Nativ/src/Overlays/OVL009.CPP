/* OVL009 - Overlay O009 (Segment 0x1f2a in SCHWEIF.EXE): Heldenbogen-/Inventarobjekt Inv (Klasse in s_class.h,
 * Daten struct inv_sheet; Teil 1 in O008), Teil 2.
 * confirm(): Auswahl bestaetigen - zweiten Platz slot2 setzen bzw. Gegenstand verschieben/tauschen (pass_item zwischen
 *   Helden, move_item im eigenen Gepaeck, kombinierbare Gegenstaende (ItemUse::combo_index) nach Rueckfrage txt[90]
 *   -> combine_items), Platz 31 print_item_description, Platz 32 hero_consume_item; Doppelklick g_mouse_ldclick ->
 *   use_item/combine_items; g_sheet_dirty/g_refresh.
 * next_hero(): naechster (Seiten > 3: naechster zauberkundiger) Held (get_next_hero/get_next_mage, g_sheet_hero).
 * warn_signal(): Warnsignal bei Tod/Bewusstlosigkeit/Rausch/Schlaf/Versteinerung, Krankheit, Gift, Hunger/Durst
 *   (statische Zeitmarken g_warn_blink_time/g_warn_snd_time mit Waechtern, Klang 0x82/0x83 bei Dumpfschaedel, bei
 *   Blauer Keuche Sprachausgabe 0x72/0x73, Blinken bei 0x104/4 ueber g_warn_blink_on).
 * toggle_body(): Ansicht g_inv_show_body umschalten.
 * open_hero_sheet(): Heldenbogen oeffnen (new Inv -> hero_sheet_loop -> delete).
 * draw(): Bogen zeichnen (Name, Typ g_ttx[type + 50/62], Werte txt[27..40], Zustaende, Gepaeck 15..30 mit Anzahl,
 *   switch Seite -> draw_page1..5, Rucksack auf/zu).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL009.CPP   -> 5115/5115 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (ebenso mit -Oi). Ohne -Og 2701 Abweichungen.
 *   (Namensfassung mit schweif.h, Welle N15: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * - Statische Locals mit Waechter (C++ "static long t = g_random_seed2;") ohne eigene Daten als Externs:
 *   "if (g_warn_blink_init == 0) { g_warn_blink_init++; g_warn_blink_time = g_random_seed2; }" -> inc byte / movsx
 *   eax / mov dword.
 * - abs(g_random_seed2 - g_warn_blink_time) (long) inline -> Kopie [bp-2]/[bp-4], 16-Bit-Differenz.
 * - confirm: ok zuerst deklarieren (DI), ItemUse-Zeiger/r im inneren Block (hinter dem -Og-Wert slot * 27 [bp-8]);
 *   new/delete Inv bzw. ItemUse rufen die Konstruktoren/Destruktoren (Inv() und ~Inv() in class Inv deklariert).
 * - next_hero: "if (hero != t) .. owner = hero" -> -Og legt hero als Zwischenwert [bp-8] ab (eigene Variable old
 *   laedt les vor mov eax). Ketten "hero = g_sheet_hero = t", "owner = hero = g_sheet_hero = t".
 * - draw: "g_ttx[(sex ? 0x3e : 0x32) + type]" (type zuerst gepusht, add ax,dx; andersherum add dx,ax);
 *   "pic->loadf(g_pp20_index = page < 3 ? 14 : 15, buf)" (buf vor dem ?: gepusht); -Og: i * 27 in DI, &f0f [bp-0eh].
 *   Palette ab Farbe 0xa0: get_palette() + 0xa0 (= alte Rechnung "+ 0x1e0" Bytes). Tabulatoren ueber (&g_font_tab0)[i].
 * - Bitfelder des Helden im Original "unsigned char b:1": bloße Tests mit (unsigned char) (and ax,1 / or al,al), die
 *   der Gegenstaende unsigned short.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:0500..050e, dsana) - initialisierte Daten in Adressreihenfolge */
signed short g_inv_warn_enabled = 1;
signed long g_warn_blink_time = 0;
signed char g_warn_blink_init = 0;
signed long g_warn_snd_time = 0;
signed char g_warn_snd_init = 0;
signed char g_warn_blink_on = 0;

#define FONT_TABS (&g_font_tab0)	/* Tabulatoren g_font.tabs[0..3] (eigenes Public _g_381f) */
#define font_set_colors_w ((void (*)(void far *, signed short, signed short))font_set_colors)

/* Auswahl bestaetigen: zweiten Platz waehlen bzw. den Gegenstand verschieben/tauschen (mit anderem Helden pass_item,
 * im eigenen Gepaeck move_item bzw. Kombination nach Rueckfrage), Platz 31 Beschreibung, Platz 32 verzehren */
void Inv::confirm()
{
	signed short ok = 0;
	signed short save;
	signed short a, b;

	if (slot2 != -1) {
		if (swap != 0 && owner != hero) {
			if (slot2 < 31) {
				g_sheet_dirty = 0;
				if (page == 1) {
					a = hero->load;
					b = owner->load;
				}
				save = cursor;
				reset_cursor();
				if (pass_item(hero, owner, slot, slot2)) {
					if (g_sheet_dirty == 0 && page == 1 && (hero->load != a || owner->load != b))
						g_sheet_dirty = 1;
					if (g_sheet_dirty == 0) {
						draw_inventory();
						reselect();
						swap = 0;
						owner = hero;
						ok = 1;
					} else
						g_refresh = 1;
				} else
					set_cursor(save);
			} else if (slot2 == 31) {
				save = cursor;
				reset_cursor();
				print_item_description(owner, slot);
				set_cursor(save);
				ok = 1;
			} else if (slot2 == 32) {
				hero_consume_item(hero, owner, slot);
				g_refresh = 1;
			}
			if (g_refresh) {
				swap = 0;
				owner = hero;
				reselect();
			} else if (!ok)
				g_refresh = 1;
		} else {
			if (slot2 < 31) {
				if (hero->inv[slot].item_id == 0 && hero->inv[slot2].item_id == 0)
					ok = 1;
				else if (g_mouse_ldclick) {
					g_mouse_ldclick = 0;
					reset_cursor();
					combine_items(hero, slot, slot2);
					g_refresh = 1;
				} else {
					ItemUse far *o;
					signed short r;

					o = new ItemUse(hero, 0, 0, 0, 0);
					r = o->combo_index(hero->inv[slot].item_id, hero->inv[slot2].item_id);
					delete o;
					if (r != -1 && dlg_yes_no(g_dialog, txt[90]) == 1) {
						reset_cursor();
						combine_items(hero, slot, slot2);
						g_refresh = 1;
					} else {
						g_sheet_dirty = 0;
						save = cursor;
						reset_cursor();
						if (move_item(hero, slot, slot2)) {
							if (g_sheet_dirty == 0) {
								draw_inventory();
								reselect();
								swap = 0;
								ok = 1;
							} else
								g_refresh = 1;
						} else
							set_cursor(save);
					}
				}
			} else if (slot2 == 31) {
				save = cursor;
				reset_cursor();
				print_item_description(hero, slot);
				set_cursor(save);
				ok = 1;
			} else if (slot2 == 32) {
				hero_consume_item(hero, hero, slot);
				g_refresh = 1;
			}
			if (g_refresh) {
				reselect();
				redraw = 1;
				swap = 0;
			} else if (!ok)
				g_refresh = 1;
		}
	} else {
		if (g_mouse_ldclick) {
			g_mouse_ldclick = 0;
			use_item(hero, slot);
			g_refresh = 1;
			return;
		}
		slot2 = slot;
		draw_slot(g_vga_buf, slot2, 0xf9);
		update_marks();
		if (hero->inv[slot].item_id)
			set_cursor(g_items[hero->inv[slot].item_id].gfx);
		draw_slots(g_vga_buf);
	}
}

/* naechster (Zauberseiten: naechster zauberkundiger) Held; bei gewaehltem Gegenstand Tausch mit ihm vorbereiten */
void Inv::next_hero()
{
	struct hero far *t;

	if (page > 3)
		t = get_next_mage(g_group, hero);
	else
		t = get_next_hero(g_group, hero);
	if (hero != t) {
		if (slot2 != -1) {
			if (swap == 0)
				owner = hero;
			hero = g_sheet_hero = t;
			swap = 1;
			old_slot2 = -1;
		} else {
			swap = 0;
			owner = hero = g_sheet_hero = t;
			reselect();
		}
		g_refresh = 1;
	}
}

/* Warnsignal: Held tot/bewusstlos/betrunken/schlafend/versteinert, krank, vergiftet, hungrig oder durstig ->
 * Huesteln bzw. Sprachausgabe (hoechstens alle 1000 Takte) und blinkendes Zeichen oben rechts (alle 18 Takte) */
void Inv::warn_signal()
{
	if (g_warn_blink_init == 0) {
		g_warn_blink_init++;
		g_warn_blink_time = g_random_seed2;
	}
	if (g_warn_snd_init == 0) {
		g_warn_snd_init++;
		g_warn_snd_time = g_random_seed2 + 1000L;
	}
	if ((unsigned char)hero->dead || (unsigned char)hero->unconscious || hero->drunk != 0 || (unsigned char)hero->asleep ||
	    (unsigned char)hero->petrified || hero_is_diseased(hero) ||
		hero_is_poisoned(hero) || hero->hunger >= 100 || hero->thirst >= 100) {
		if (abs(g_random_seed2 - g_warn_blink_time) > 18) {
			if (abs(g_random_seed2 - g_warn_snd_time) > 1000) {
				g_warn_snd_time = g_random_seed2;
				if (hero->illness[ILLNESS_DUMPFSCHAEDEL].state == ILL_STATE_SICK)
					snd_play(g_snd_fx, hero->sex == 0 ? 0x82 : 0x83);
				else if (hero->illness[ILLNESS_BLAUE_KEUCHE].state == ILL_STATE_SICK) {
					if (g_speech_ext_avail)
						snd_play(g_snd_fx, hero->sex == 0 ? 0x72 : 0x73);
					else
						snd_play(g_snd_fx, hero->sex == 0 ? 0x82 : 0x83);
				}
			}
			g_warn_blink_time = g_random_seed2;
			if (g_blink_off == 0 && !dlg_overlaps(g_dialog, 0x104, 4, 0x118, 0x19)) {
				mouse_hide(0);
				if (g_warn_blink_on == 0) {
					b136->put(g_vga_buf, 0x104, 4, BLIT_MASKED);
					g_warn_blink_on = 1;
				} else {
					b13a->put(g_vga_buf, 0x104, 4);
					g_warn_blink_on = 0;
				}
				mouse_show(0);
			}
		}
	}
}

/* Ansicht umschalten: Koerperplaetze <-> nur Rucksack */
void Inv::toggle_body()
{
	g_inv_show_body ^= 1;
	f27 = 1;
}

/* Heldenbogen oeffnen (F1/Strg-E): Held waehlen, new Inv -> hero_sheet_loop -> delete */
void open_hero_sheet()
{
	struct hero far *h;
	Inv far *s;

	if (g_sheet_enabled) {
		if ((signed short)g_group->nr_heroes > 1)
			h = get_any_hero(g_group, g_ttx[231]);
		else
			h = get_first_hero(g_group);
		if (h != 0) {
			snd_obj_stop(g_snd_music);
			ani_stop(g_screen_pic);
			swap_out_screen();
			s = new Inv;
			hero_sheet_loop(s, h);
			delete s;
		}
	}
}

/* Heldenbogen zeichnen: Hintergrund, Name, Typ, Werte, Zustaende, Rucksack mit Anzahl, Seite page, Auswahlrahmen */
void Inv::draw()
{
	signed short c0, c1, c2, c3;
	void far *p;
	signed short i;

	mouse_hide(0);
	g_status_on = 0;
	if (g_pp20_index != 14 && g_pp20_index != 15) {
		screen->clear();
		screen->put(g_vga_buf, 0, 0);
	}
	update_marks();
	font_set_screen(g_font, buf);
	c0 = FONT_TABS[0];
	c1 = FONT_TABS[1];
	c2 = FONT_TABS[2];
	c3 = FONT_TABS[3];
	if (page < 3 && (g_inv_show_body == 1 || f27 == 1))
		draw_body_slots();
	font_set_colors_w(g_font, 0xff, -1);
	((Pic far *)pic)->loadf(g_pp20_index = page < 3 ? 14 : 15, buf);
	set_palette(((Pic far *)pic)->get_palette() + 0xa0, 0xa0, 0x60);
	b13a->get(buf, 0x104, 4);
	font_set_colors_w(g_font, 0xf9, -1);
	font_print(g_font, hero->name, 0x59, 6);
	font_set_colors_w(g_font, 0xff, -1);
	font_print(g_font, g_ttx[(hero->sex ? 0x3e : 0x32) + hero->type], 0x59, 0x0f);
	if (page < 3) {
		b112->set_base(load_portrait(hero));
		b112->put(buf, 0x0c, 0x10);
		if (g_inv_show_body == 1 && f27 == 0) {
			body->put(buf, 0x50, 0x21);
			knapsack->put(buf, 0x50, 0x32, BLIT_MASKED);
		} else {
			font_set_tab(g_font, 0x87, 0);
			font_print(g_font, txt[39], 0x59, 0x41);
			font_print(g_font, txt[40], 0x59, 0x48);
			sprintf(g_text_output_buf, txt[31], hero->ap);
			font_print(g_font, g_text_output_buf, 0x59, 0x56);
			sprintf(g_text_output_buf, txt[29], hero->level);
			font_print(g_font, g_text_output_buf, 0x59, 0x5d);
			money_to_string(g_text_buf2, g_group->money);
			sprintf(g_text_output_buf, txt[30], g_text_buf2);
			font_print(g_font, g_text_output_buf, 0x59, 0x6b);
			sprintf(g_text_output_buf, txt[27], hero->height);
			font_print(g_font, g_text_output_buf, 0x59, 0x80);
			sprintf(g_text_output_buf, txt[28], hero->weight);
			font_print(g_font, g_text_output_buf, 0x59, 0x87);
			if ((unsigned char)hero->dead)
				font_print(g_font, txt[32], 0x59, 0x95);
			else if ((unsigned char)hero->unconscious)
				font_print(g_font, txt[37], 0x59, 0x9c);
			else if (hero->drunk != 0)
				font_print(g_font, txt[38], 0x59, 0xa3);
			if ((unsigned char)hero->asleep)
				font_print(g_font, txt[33], 0x59, 0xaa);
			if ((unsigned char)hero->petrified)
				font_print(g_font, txt[34], 0x59, 0xb1);
			if (hero_is_diseased(hero))
				font_print(g_font, txt[36], 0x59, 0xb8);
			if (hero_is_poisoned(hero))
				font_print(g_font, txt[35], 0x59, 0xbf);
		}
		p = item_bmp->data();
		for (i = 15; i < 31; i++) {
			if (hero->inv[i].item_id) {
				((Nvf far *)nvf)->load(0x0b, p, 0, 0, g_items[hero->inv[i].item_id].gfx, 0, &f0f, &f0f, 0x140);
				item_bmp->put(buf, g_inv_slot_xy[i].x + 1, g_inv_slot_xy[i].y + 1);
				if (g_items[hero->inv[i].item_id].stackable) {
					font_set_colors_w(g_font, 0xff, 0);
					itoa(hero->inv[i].quantity, g_text_output_buf, 10);
					font_print(g_font, g_text_output_buf, g_inv_slot_xy[i].x + 15 - font_text_width0(g_font, g_text_output_buf),
						g_inv_slot_xy[i].y + 8);
					font_set_colors_w(g_font, 0xff, -1);
				}
				draw_slot(buf, i, 0);
			}
		}
	} else {
		b112->set_base(load_portrait(hero));
		b112->put(buf, 10, 3);
	}
	switch (page) {
	case 1:
		draw_page1();
		break;
	case 2:
		draw_page2();
		break;
	case 3:
		draw_page3();
		break;
	case 4:
		draw_page4();
		break;
	case 5:
		draw_page5();
		break;
	}
	if (page >= 3) {
		do_v_line(buf, 0x6b, 0x28, 0xc7, 0xff);
		do_v_line(buf, 0xd4, 0x28, 0xc7, 0xff);
	}
	font_set_screen(g_font, g_vga_buf);
	font_set_tab(g_font, c0, 0);
	font_set_tab(g_font, c1, 1);
	font_set_tab(g_font, c2, 2);
	font_set_tab(g_font, c3, 3);
	if (f27 == 1 && page < 3) {
		status_bg->put(g_vga_buf, 0, 0xc0);
		if (g_inv_show_body == 1)
			knapsack_close();
		else
			knapsack_open();
	} else
		screen->put(g_vga_buf, 0, 0);
	f27 = 0;
	if (page < 3)
		status_bg->get(g_vga_buf, 0, 0xc0);
	if (slot != -1 && owner == hero && page < 3)
		draw_slot(g_vga_buf, slot, 0xfd);
	if (slot2 != -1 && page < 3)
		draw_slot(g_vga_buf, slot2, 0xf9);
	if (!g_inv_show_body && page < 3)
		g_status_on = 1;
	mouse_show(0);
}
