/* OVL012 - Overlay O012 (Segment 0x1f3b in SCHWEIF.EXE): Lagerhaus - class Store (s_class.h, 0xb2 Bytes; lokales
 * Objekt in O030 bei LOC_WAREHOUSE, Einlagern O099 store_deposit, Abholen O100 store_fetch). Drei Lagerhaeuser
 * (g_loc_id 1..3) mit je einem Lagerplatz struct depot (depots[3] ab +0x70: Zustand state, Wert value, Zahl
 * nr_items, Zeitstempel stored/checked) und 0x1b00 Bytes Gegenstaenden (items[3] ab +0x60, inv_slot je 0x1b),
 * aktueller Platz depot = &depots[g_loc_id - 1]. Store(): Dateiname g_str_depot_dat ("DEPOT.DAT") als Name 0 der
 * Arbeitsdatei (TEMP_NAME0), Texte 0x22/0x26, Puffer, Raster 3 x 5 (grid_x/grid_y). ~Store(): Puffer frei.
 * is_open/can_enter: Oeffnungszeit (g_daytime 8..18 Uhr) und Lagergebuehr (check_fee: Tage = date_since(stored) /
 * TICKS_PER_DAY, Gebuehr Tage * (value / 300 + 1), Gruppengeld) pruefen; can_enter nach 30 Tagen seit checked mit
 * W50 <= 2 Diebstahl/Verlust (Platz leeren, Sperre lock fuer 6/3 Monate), gesperrter Platz bis checked geschlossen.
 * run: Hauptschleife (Menue dlg_choose mit 4 Punkten -> g_action 0x81 reden talk, 0x82 store_deposit, 0x83
 * store_fetch, 0x84 Ende; Ende auch bei g_leave_location), zum Schluss speichern. check_time(zeit): Ladenschluss -> done = 1.
 * load: Datei lesen (open/read, ohne Datei create_file), save(art) ueber die Arbeitsdatei g_ofile (O243), danach
 * neu laden. grid_mark: Auswahlrahmen (do_border) und Gegenstandsname (g_item_names). talk: Gespraech 0x9d/0x9f/
 * 0x9e je Lagerhaus 1/2/3 (conv_run). grid_input(&neu): Maus (g_merchant_items_posx/posy, 15 Felder) und
 * Pfeiltasten 0x4800/0x5000/0x4d00/0x4b00, Blaettern um 15.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL012.CPP   -> 4021/4021 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Erste Fassung sofort gleich. -Og noetig (load: Zwischenwert this + i * 0x16 in [bp-4], Segment
 *   nachgeladen); -Oe zerstoert run/load/grid_mark (Register statt Stack).
 * - register-Schluesselwoerter wirkungslos (automatische SI/DI-Vergabe, Quelle ohne register). Destruktor:
 *   flags in DI, Schleifenzaehler auf [bp-2] (SI dient als Adressregister fuer "items[i] = mem_free(items[i])"
 *   -> les si,this / add si,bx).
 * - "TEMP_NAME0 = 0" (far-Zeiger ueber Zeiger) -> mov dword ptr es:[bx],0; "= g_str_depot_dat" -> mov [bx+2],ds.
 * - Long-Test "date_since(..) >= 0" ohne Zwischenspeicher -> or ax,dx / jl (keine shl/shrd-Rueckgabe).
 * - "fee *= value / 300 + 1" -> idiv / inc eax / imul edx,eax; Groessen "(long)(n - nr_sel) * 0x1b" bzw. n * 0x1bL.
 * - "(old = old_sel) != sel" eingebettet -> mov si,ax / cmp ax,[..]; "if (g_loc_id - 1 != i)" -> dec ax / cmp ax,si.
 * - grid_input: Maustest "g_mouse_locked == 0 && (mx != .. || my != ..)" als ein if; wares[base + sel + 5] ->
 *   +23h-Versatz.
 * Originalfehler: create_file (Lagerdatei anlegen) prueft "fd == -1" statt "!= -1" - schreibt die drei Saetze nur,
 *   wenn open fehlschlug (auf Handle -1), bei Erfolg wird nichts geschrieben (und das Handle nicht geschlossen);
 *   open mit O_CREAT (0x8102) ohne Zugriffsmodus.
 */
#include <stdio.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:0626..0630, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */

/* Name 0 der Arbeitsdatei g_ofile (ofile_create(g_ofile, 0, ..) und Pfad g_path_fmts[0]): erster Eintrag der
 * Namensliste g_names_star_idx */
#define TEMP_NAME0 (*(char far * far *)g_names_star_idx)

Store::Store()
{
	signed short i;

	TEMP_NAME0 = "DEPOT.DAT";
	txt_load_list(g_font, 0x22, 0x26, &txt_buf, &txt);
	for (i = 0; i < 3; i++)
		items[i] = (struct inv_slot far *)mem_alloc(0x1b00L);
	pic_buf = mem_alloc(0x240L);
	wares = (struct ware far *)mem_alloc(0x700L);
	grid_x[0] = 0x1e;
	grid_x[1] = 0x5f;
	grid_x[2] = 0xa0;
	grid_y[0] = 0x23;
	grid_y[1] = 0x37;
	grid_y[2] = 0x4b;
	grid_y[3] = 0x5f;
	grid_y[4] = 0x73;
	g_refresh = 1;
}

Store::~Store()
{
	signed short i;

	mem_free(txt_buf);
	mem_free(txt);
	for (i = 0; i < 3; i++)
		items[i] = (struct inv_slot far *)mem_free(items[i]);
	pic_buf = mem_free(pic_buf);
	wares = (struct ware far *)mem_free(wares);
	TEMP_NAME0 = 0;
}

signed short Store::is_open()
{
	signed short ok = 1;

	if (depot->state == 0) {
		if (g_daytime < 0xa8c0L || g_daytime > 0x17bb0L) {	/* vor 8 bzw. nach 18 Uhr */
			screen_textbar(g_screen, txt[g_loc_name], 0xff);
			dlg_message(g_dialog, txt[0]);
			ok = 0;
		}
		if (ok) {
			if (!check_fee())
				ok = 0;
		}
	}
	return ok;
}

void Store::run()
{
	signed short sel;

	done = 0;
	load();
	if (can_enter()) {
		while (done == 0) {
			if (g_refresh) {
				draw_loc_icons(g_screen, 4, 0x15, 0x30, 0x31, 8);
				screen_redraw(g_screen);
				anim_load(g_screen_pic, 0x2d, 0);
				screen_textbar(g_screen, txt[g_loc_name], 0xff);
				snd_play(g_snd_music, 7);
				g_refresh = 0;
			}
			check_time(g_daytime);
			handle_input(g_screen);
			if (g_mouse_rclick != 0 || g_key == 0x4900) {
				sel = dlg_choose(g_dialog, txt[1], 4, txt[35], txt[2], txt[3], txt[4]) - 1;
				if (sel != -2)
					g_action = sel + 0x81;
			}
			if (g_action == 0x81) {
				talk();
			} else if (g_action == 0x82) {
				ani_stop(g_screen_pic);
				store_deposit(this);
			} else if (g_action == 0x83) {
				ani_stop(g_screen_pic);
				store_fetch(this);
			} else if (g_action == 0x84 || g_key == 0x11b) {
				done = 1;
			}
			if (g_leave_location)
				done = 1;
		}
	}
	save(2);
}

signed short Store::can_enter()
{
	signed short ok = 1;
	signed short r;

	if (depot->state == 0) {
		if (date_since(&depot->checked) > 0x3b5380L) {	/* 30 Tage */
			r = random_schweif(50);
			if (r <= 2) {
				ok = 0;
				lock(r);
				depot->value = 0;
				depot->nr_items = 0;
				anim_load(g_screen_pic, r == 2 ? 0x2e : 0x2f, 0);
				screen_textbar(g_screen, txt[g_loc_name], 0xff);
				snd_play(g_snd_music, 7);
				dlg_message(g_dialog, r == 2 ? txt[6] : txt[5]);
			} else {
				lock(0);
			}
		}
	} else {
		if (date_since(&depot->checked) >= 0) {
			dlg_message(g_dialog, txt[7]);
			depot->state = 0;
		} else {
			dlg_message(g_dialog, depot->state == 1 ? txt[25] : txt[26]);
			ok = 0;
		}
	}
	if (ok) {
		if (g_daytime < 0xa8c0L || g_daytime > 0x17bb0L) {
			dlg_message(g_dialog, txt[0]);
			ok = 0;
		}
	}
	if (ok) {
		if (!check_fee())
			ok = 0;
	}
	return ok;
}

signed short Store::check_time(signed long t)
{
	if (t < 0xa8c0L || t > 0x17bb0L) {
		dlg_message(g_dialog, txt[34]);
		done = 1;
	}
	return done;
}

void Store::lock(signed short k)
{
	signed long months;

	months = k == 1 ? 6 : k == 2 ? 3 : 1;
	depot->state = k;
	date_set(&depot->checked, 1, 0L, 0, months, 0);
}

signed short Store::check_fee()
{
	signed short ok = 1;
	signed long fee;

	if (depot->nr_items != 0) {
		fee = date_since(&depot->stored) / TICKS_PER_DAY;
		fee *= depot->value / 300 + 1;
		if (fee != 0) {
			if (g_group->money < fee) {
				screen_textbar(g_screen, txt[g_loc_name], 0xff);
				money_to_string(g_text_buf2, fee);
				sprintf(g_text_output_buf, txt[17], g_text_buf2);
				dlg_message(g_dialog, g_text_output_buf);
				ok = 0;
			} else {
#if FIX_0352
				/* Fix #352: Gebuehr wirklich abbuchen und den Zeitstempel neu setzen (Store::run speichert am Ende) */
				money_to_string(g_text_buf2, fee);
				sprintf(g_text_output_buf, txt[37], g_text_buf2);
				dlg_message(g_dialog, g_text_output_buf);
				group_sub_money(g_group, fee);
				date_now(&depot->stored);
#else
				money_to_string(g_text_buf2, fee);
				sprintf(g_text_output_buf, txt[37], g_text_buf2);
				dlg_message(g_dialog, g_text_output_buf);
#endif
			}
		}
	}
	return ok;
}

void Store::load()
{
	signed short fd;
	char name[40];
	signed short i;

	for (i = 0; i < 3; i++) {
		memset(items[i], 0, 0x1b00);
		memset(&depots[i], 0, sizeof(struct depot));
	}
	sprintf(name, g_path_fmts[0], TEMP_NAME0);
	fd = open(name, O_BINARY | O_RDONLY);
	if (fd == -1) {
		create_file();
	} else {
		for (i = 0; i < 3; i++) {
			read(fd, &depots[i], sizeof(struct depot));
			if (depots[i].nr_items != 0)
				read(fd, items[i], depots[i].nr_items * 0x1b);
		}
		close(fd);
	}
	depot = &depots[g_loc_id - 1];
}

void Store::save(signed short mode)
{
	signed short i;

	if (ofile_create(g_ofile, 0, O_BINARY | O_CREAT | O_WRONLY) != -1) {
		if (mode == 2) {
			for (i = 0; i < 3; i++) {
				ofile_write(g_ofile, &depots[i], 0x16L);
				ofile_write(g_ofile, items[i], depots[i].nr_items * 0x1bL);
			}
		} else {
			for (i = 0; i < 3; i++) {
				if (g_loc_id - 1 != i) {
					ofile_write(g_ofile, &depots[i], 0x16L);
					ofile_write(g_ofile, items[i], depots[i].nr_items * 0x1bL);
				} else {
					ofile_write(g_ofile, depot, 0x16L);
					if (mode == 0) {
						ofile_write(g_ofile, items[i], (signed long)(depots[i].nr_items - nr_sel) * 0x1b);
						ofile_write(g_ofile, new_items, nr_sel * 0x1bL);
					} else {
						ofile_write(g_ofile, rest, depot->nr_items * 0x1bL);
					}
				}
			}
		}
		ofile_close(g_ofile);
	}
	load();
}

void Store::create_file()
{
	char name[40];
	signed short fd;
	signed short i;

	sprintf(name, g_path_fmts[0], TEMP_NAME0);
#if FIX_0071
	/* Fix #71: Datei mit Zugriffsmodus anlegen und nur bei Erfolg schreiben */
	fd = open(name, O_BINARY | O_CREAT | O_WRONLY, 0x0100 | 0x0080);	/* S_IREAD | S_IWRITE */
	if (fd != -1) {
#else
	fd = open(name, O_BINARY | O_CREAT | O_WRONLY);
	if (fd == -1) {
#endif
		for (i = 0; i < 3; i++) {
			date_now(&depots[i].checked);
			write(fd, &depots[i], sizeof(struct depot));
		}
		close(fd);
	}
}

void Store::grid_mark()
{
	void far *name;
	signed short x0;
	signed short x1;
	signed short y0;
	signed short y1;
	signed short old;

	if ((old = old_sel) != sel) {
		x0 = grid_x[old / 5];
		x1 = grid_x[sel / 5];
		y0 = grid_y[old % 5];
		y1 = grid_y[sel % 5];
		mouse_hide(0);
		if (old_sel != -1)
			do_border(g_vga_buf, x0 - 1, y0 - 1, 0x12, 0x12, 0);
		do_border(g_vga_buf, x1 - 1, y1 - 1, 0x12, 0x12, 0xff);
		screen_clear_textbar(g_screen);
		if ((name = g_item_names[wares[sel + base].id]) != 0)
			screen_textbar(g_screen, grammar_name((char far *)name), 0xff);
		old_sel = sel;
		mouse_show(0);
	}
}

void Store::talk()
{
	signed short id = -1;

	if (g_loc_id == 1)
		id = 0x9d;
	else if (g_loc_id == 3)
		id = 0x9e;
	else if (g_loc_id == 2)
		id = 0x9f;
	if (id != -1 && conv_run(g_dialog2, id))
		timewarp_minutes(g_date, 30L);
}

signed short Store::grid_input(signed short far *redraw)
{
	signed short ret = 0;

	if (g_mouse_locked == 0 && (mx != g_mouse_posx || my != g_mouse_posy)) {
		signed short i;

		mx = g_mouse_posx;
		my = g_mouse_posy;
		for (i = 0; i < 15; i++) {
			if (g_merchant_items_posx[i] <= g_mouse_posx &&
				g_merchant_items_posx[i] + 50 >= g_mouse_posx &&
				g_merchant_items_posy[i] <= g_mouse_posy &&
				g_merchant_items_posy[i] + 17 >= g_mouse_posy &&
				wares[base + i].id != 0) {
				sel = i;
				break;
			}
		}
	} else if (g_action == 0x4800) {
		if (sel > 0) {
			sel--;
		} else {
			for (sel = 14; wares[base + sel].id == 0 && sel != 0; sel--)
				;
		}
	} else if (g_action == 0x5000) {
		if (sel < 14) {
			sel++;
			if (wares[base + sel].id == 0)
				sel = 0;
		} else {
			sel = 0;
		}
	} else if (g_action == 0x4d00) {
		if (sel < 10) {
			if (wares[base + sel + 5].id != 0)
				sel += 5;
		} else {
			if (wares[base + 15].id != 0) {
				*redraw = 1;
				base += 15;
			}
		}
	} else if (g_action == 0x4b00) {
		if (sel > 4) {
			sel -= 5;
		} else {
			ret = 1;
			if (base != 0) {
				base -= 15;
				*redraw = 1;
			} else {
				sel -= 10;
				if (sel < 0)
					sel = 0;
			}
		}
	}
	return ret;
}
