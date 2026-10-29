/* OVL107 - Overlay O107 (Segment 0x2165 in SCHWEIF.EXE): Spielstand-Menue (class SaveMenu, 0x6f7 Bytes wie O106:
 * STUFE D1 (wovd1, 19.10.2026): asm-strcat durch echtes strcat(.., "..") ersetzt - Code GLEICH, das Literal
 *   im eigenen _DATA bildet wovd1/cobjfix_d.py auf ds:6202/6216/6220/6225/622a ab (Bytes geprueft). Hinweise unten zum asm sind alt.
 * Bild-Mitglied pic, Eingabefeld edit (Edit, O110/O111), Texte txt, Zeilenhoehe line_h, markierte Zeile sel,
 * zuletzt bearbeitete Zeile last, erste Zeile first, Handle fd, Beschreibung desc (256 Bytes), 100 Namen names[]
 * (Name 9, Zeit, Datum), zuletzt gespeicherter Name cur, Anzahl count).
 * savegame_menu(b, mode): Menue - Hintergrund sichern (Bmp, Pic-Bild 0x89, fade_in_pic), Texte 0x9b, Edit(6, 0xa0,
 *   0x79, 0x3b), Liste einlesen, Schleife: Tasten/Maus (g_action) per switch: KEY_DEL loeschen (savegame_delete),
 *   hoch/runter (g_key_shift = seitenweise), Mausklick ACTION_ICON_3, Ende ACTION_ICON_5 (sys_exit, exit),
 *   Beschreibung ACTION_ICON_6 (Edit::run, savemenu_write_desc), Speichern KEY_INS/ACTION_ICON_7 (nur mode != 1,
 *   savemenu_new, savegame_op), Laden KEY_RETURN/ACTION_ICON_8 und Doppelklick (savegame_load_cur,
 *   savemenu_touch, g_cam_angle, g_exit_dir), ACTION_ICON_9 savegame_op, Esc (nicht bei g_game_state == 2); Menue
 *   dlg_choose bei g_mouse_rclick/KEY_PGUP. Am Ende alles zuruecksetzen, Rueckgabe Ergebnis von savegame_load_cur.
 * savemenu_input: Eingabe (poll_input, handle_options_key). savemenu_cmp: qsort-Vergleich (neueste zuerst: Datum,
 *   dann Zeit). savemenu_scan: *.GAM suchen (findfirst/findnext, max. 100, Namen bis '.', auf 8 Zeichen mit ' '
 *   gefuellt), sortieren. savemenu_draw_list: Liste zeichnen (Bmp 0x37 x 0x52), savemenu_select_cur: Auswahl auf
 *   den aktuellen Namen, savemenu_scroll_down/_up blaettern, savegame_write_desc/savegame_read_desc: Beschreibung
 *   der offenen Datei schreiben/lesen (ab 0x10), savemenu_write_desc(i)/savemenu_read_desc(i): Beschreibung von
 *   Spielstand i schreiben/lesen (Kopf pruefen savegame_read_header, g_dsa_version_string/g_version_c, sonst
 *   savemenu_clear_desc), savemenu_exists(s): Name vorhanden, savemenu_new: neuer Spielstand (disk_free >= 1 MB
 *   frei, Name dlg_message_flag -> savegame_make_name, Rueckfrage beim Ueberschreiben, _creat,
 *   savegame_write_header, savegame_write_desc), savegame_make_name(d, s): Dateiname (nur Buchstaben/Ziffern/
 *   Umlaute, sonst '_', 8 Zeichen), savemenu_update: Anzeige aktualisieren, savemenu_clear_desc:
 *   Beschreibungsfeld leeren, savemenu_touch: Datei "anfassen" (1 Byte lesen und zurueckschreiben -> neues Datum).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL107.CPP   -> 6832/6832 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (ohne -Og 73 Bytes laenger, ohne -Oi 563 Bytes kuerzer - Intrinsics strcpy/strchr/strcmp/memset).
 * - Inline-Assembler (5 Stellen): strcat(name, ".GAM") mit 5-Byte-DS-Literal (ds:6202/6216/6220/6225/622a) wie
 *   OVL106. NEU: Ein asm-Befehl mit SI/DI im Funktionsrumpf schaltet die register-Variablen der Funktion ab - in
 *   savegame_menu landete "register old" (Laden, SI) dann auf dem Stack (Rahmen +2). Loesung: die strcat-Sequenz in savegame_menu
 *   ganz als "asm db"/"asm dw offset g_6202" schreiben (BCC sieht dann keine Registerbenutzung).
 * - Lokales Bmp-Objekt mitten in savegame_menu (nach Pic::get_size deklariert), Rueckgabe ueber Zwischenwert vor dem
 *   Destruktor; new Edit -> O111_0000(0, ..), delete -> O111_0205(p, 3). "g_clock_update = g_refresh = 1".
 * - switch (g_action): Rumpfreihenfolge 0x5300, 0x4800/0x81, 0x5000/0x82, 0x83, 0x85, 0x86, 0x5200/0x87,
 *   0x1c0d/0x88, 0x89, 0x11b (Sprungtabelle 0x81..0x88, 0x84 = Ende).
 * - "b->count - 10" ohne Variable (-Og-Zwischenwert [bp-10h]/[bp-4] bzw. DX); "++b->sel >= 10" eingebettet;
 *   strcmp(s, names[i]) (s in DS:SI); savegame_make_name "c = d[i]; if (c != 0 && !isalnum(c) && c != '\x99' ..)" (c in DL).
 * - findfirst(.., &ff, n = 0) (Zuweisung im Argument); erster Durchlauf und Schleife stehen doppelt im Quelltext
 *   (eigene strtok-Literale ds:620d/620f).
 * Originalfehler/Auffaelliges: Laden - "if (!g_group->loctype && old) g_exit_dir = g_group->loctype" schreibt
 *   immer 0 (gemeint war wohl old). savegame_make_name: isalnum() mit signed char liest bei Zeichen >= 0x80 vor der Tabelle _ctype. savemenu_select_cur sucht ohne
 *   Grenze nach dem aktuellen Namen. savemenu_touch prueft das Ergebnis von _open nicht.
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <io.h>
#include <dir.h>
#include <fcntl.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:618a..6230, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
struct mouse_area g_savemenu_areas[12] = {
	{ 30, 46, 59, 55, 129 },
	{ 30, 146, 59, 155, 130 },
	{ 61, 46, 89, 55, 139 },
	{ 61, 146, 89, 155, 140 },
	{ 32, 59, 88, 142, 131 },
	{ 263, 115, 301, 154, 133 },
	{ 119, 59, 288, 109, 134 },
	{ 99, 115, 128, 154, 135 },
	{ 133, 115, 192, 154, 136 },
	{ 199, 115, 257, 154, 137 },
	{ 10, 40, 310, 160, 132 },
	{ -1, -1, -1, -1, -1 }
};

/* font_set_colors mit Wort-Parametern (gesicherte Farben als short: push word) */
#define font_set_colors_w ((void (*)(void far *, signed short, signed short))font_set_colors)

signed short savegame_menu(SaveMenu far *b, signed short mode)
{
	signed short w;
	signed short h;
	signed short x;
	signed short y;
	signed short k;
	signed short i;
	signed short n;
	signed short ret = 0;
	signed short again = 0;
	signed short fg;
	signed short bg;
	void far *pic;
	void far *areas;
	void far *areas2;
	signed short res = 0;
	char far *p;

	if (g_c6c6 > 50)
		return res;
	g_clock_update = 0;
	g_status_on = 0;
	g_dialog_depth++;
	snd_obj_stop(g_snd_music);
	ani_stop(g_screen_pic);
	b->pic.get_size(0x89, &w, &h);
	Bmp save((w + 8) & 0xfff8, h);
	pic = mem_alloc((unsigned short)(w * h));
	b->pic.load(0x89, pic);
	x = (0x140 - w) >> 1;
	y = (0xc8 - h) >> 1;
	mouse_hide(0);
	save.get(g_vga_buf, x, y);
	mouse_show(0);
	fade_in_pic(b->fade, pic, save.data(), w, h);
	mem_free(pic);
	snd_play(g_snd_music, 10);
	txt_load_list(g_font, 0x9b, 0x1e, &b->txt_buf, &b->txt);
	font_get_colors(g_font, &fg, &bg);
	font_set_colors(g_font, 0xda, 0xce);
	b->edit = new Edit(6, 0xa0, 0x79, 0x3b, 0, 0, 0);
	b->line_h = font_set_line_h(g_font, -1);
	memset(b->names, 0, sizeof(b->names));
	areas = g_mouse_areas;
	areas2 = g_mouse_areas2;
	g_mouse_areas = g_savemenu_areas;
	g_mouse_areas2 = 0;
	b->count = savemenu_scan(b);
	b->last = -1;
	b->sel = 0;
	b->first = 0;
	savemenu_draw_list(b);
	savemenu_update(b);
	do {
		if (again == 0)
			savemenu_input(b);
		if (g_mouse_rclick != 0 || g_key == KEY_PGUP) {
			k = dlg_choose(g_dialog, b->txt[4], 7, b->txt[5], b->txt[6], b->txt[7], b->txt[8], b->txt[11],
				b->txt[12], b->txt[10]) - 1;
			if (k != -2)
				g_action = k + ACTION_ICON_6;
			if (k == 4)
				g_action = KEY_DEL;
			else if (k == 5)
				g_action = ACTION_ICON_5;
			else if (k == 6)
				g_action = KEY_ESC;
		}
		again = 0;
		if (g_action == 0x8b || g_action == 0x8c) {
			g_action -= 10;
			g_key_shift = 1;
		}
		switch (g_action) {
		case KEY_DEL:
			if (b->count > 0) {
				char a[14];
				char c[20];

				strcpy(c, b->names[b->sel + b->first].name);
				strcpy(a, c);
				p = strchr(a, ' ');
				if (p != 0)
					*p = 0;
				else {
					p = strchr(a, '.');
					if (p != 0)
						*p = 0;
				}
				sprintf(g_text_output_buf, b->txt[3], a);
				str_cut_at(c, ' ');
				str_cut_at(c, '.');
				strcat(c, ".GAM");
				if (dlg_yes_no(g_dialog, g_text_output_buf) == 1) {
					savegame_delete(b, b->sel + b->first);
					b->last = -1;
					if (b->sel + b->first == b->count - 1 && b->first == 0)
						b->sel--;
					else if (b->first != 0)
						b->first--;
					b->count = savemenu_scan(b);
					savemenu_draw_list(b);
					savemenu_update(b);
				}
			} else
				dlg_message(g_dialog, b->txt[19]);
			break;
		case KEY_UP:
		case ACTION_ICON_1:
			n = g_key_shift ? 9 : 1;
			if (b->sel + b->first < n)
				n = b->sel + b->first;
			for (i = 0; i < n; i++) {
				if (--b->sel < 0) {
					b->sel = 0;
					savemenu_scroll_up(b);
				}
			}
			savemenu_draw_list(b);
			savemenu_update(b);
			break;
		case KEY_DOWN:
		case ACTION_ICON_2:
			n = g_key_shift ? 9 : 1;
			if (b->count - (b->sel + b->first) < n)
				n = b->count - (b->sel + b->first);
			for (i = 0; i < n; i++) {
				if (++b->sel >= 10 && b->sel < b->count) {
					b->sel = 9;
					savemenu_scroll_down(b);
				} else if (b->sel >= b->count)
					b->sel--;
			}
			savemenu_draw_list(b);
			savemenu_update(b);
			break;
		case ACTION_ICON_3:
			b->sel = (g_mouse_clicky - 0x3c) / b->line_h;
			if (b->sel >= b->count)
				b->sel = b->count - 1;
			if (b->sel >= 10)
				b->sel = 9;
			if (b->sel != b->last)
				savemenu_draw_list(b);
			savemenu_update(b);
			break;
		case ACTION_ICON_5:
			if (dlg_yes_no(g_dialog, b->txt[1]) == 1) {
				sys_exit(0);
				exit(0);
			}
			break;
		case ACTION_ICON_6:
			if (b->count > 0) {
				b->edit->run(b->desc);
				savemenu_write_desc(b, b->sel + b->first);
				b->last = b->sel;
				if (g_action == KEY_PGUP || g_mouse_rclick != 0) {
					g_mouse_rclick = 0;
					again = 1;
					g_key = KEY_PGUP;
				}
				g_action = 0;
			} else
				dlg_message(g_dialog, b->txt[19]);
			break;
		case KEY_INS:
		case ACTION_ICON_7:
			if (mode == 1)
				dlg_message(g_dialog, b->txt[22]);
			else if (dlg_yes_no(g_dialog, b->txt[2]) == 1 && savemenu_new(b) == 1) {
				b->sel = b->first = 0;
				b->count = savemenu_scan(b);
				savemenu_select_cur(b);
				savemenu_draw_list(b);
				savemenu_update(b);
				savegame_op(b, mode, 0);
			}
			break;
		case KEY_RETURN:
		case ACTION_ICON_8:
			if (b->count > 0) {
				char far *p2;
				char buf[30];

				strcpy(buf, b->names[b->sel + b->first].name);
				p2 = strchr(buf, ' ');
				if (p2 != 0)
					*p2 = 0;
				else {
					p2 = strchr(buf, '.');
					if (p2 != 0)
						*p2 = 0;
				}
				sprintf(g_text_output_buf, b->txt[23], buf);
				if (dlg_yes_no(g_dialog, g_text_output_buf) == 1) {
					register signed short old = g_group->loctype;

					res = savegame_load_cur(b);
					savemenu_touch(b);
					g_cam_angle += 0x80;
					g_cam_angle &= 0x700;
					ret = 1;
					if (!g_group->loctype && old)
#if FIX_0197
						/* Fix #197: alten Ortstyp uebernehmen (der neue ist hier immer 0) */
						g_exit_dir = old;
#else
						g_exit_dir = g_group->loctype;
#endif
					g_pp20_index = -1;
				}
			} else
				dlg_message(g_dialog, b->txt[19]);
			break;
		case ACTION_ICON_9:
			ret = savegame_op(b, mode, 1);
			break;
		case KEY_ESC:
			if (g_game_state == 2)
				dlg_message(g_dialog, b->txt[25]);
			else
				ret = 1;
			break;
		}
		if (g_mouse_ldclick != 0) {
			g_mouse_ldclick = 0;
			if (is_click_in_rect(0x20, 0x3b, 0x58, 0x8e) != 0) {
				if (b->sel >= 0 && b->count > 0) {
					char far *p2;
					char buf[30];

					strcpy(buf, b->names[b->sel + b->first].name);
					p2 = strchr(buf, ' ');
					if (p2 != 0)
						*p2 = 0;
					else {
						p2 = strchr(buf, '.');
						if (p2 != 0)
							*p2 = 0;
					}
					sprintf(g_text_output_buf, b->txt[23], buf);
					if (dlg_yes_no(g_dialog, g_text_output_buf) == 1) {
						register signed short old = g_group->loctype;

						res = savegame_load_cur(b);
						savemenu_touch(b);
						g_cam_angle += 0x80;
						g_cam_angle &= 0x700;
						ret = 1;
						if (!g_group->loctype && old)
#if FIX_0197
							/* Fix #197: alten Ortstyp uebernehmen (der neue ist hier immer 0) */
							g_exit_dir = old;
#else
							g_exit_dir = g_group->loctype;
#endif
						g_pp20_index = -1;
					}
				} else
					dlg_message(g_dialog, b->txt[19]);
			}
		}
	} while (ret == 0);
	delete b->edit;
	g_mouse_areas = areas;
	g_mouse_areas2 = areas2;
	font_set_colors_w(g_font, fg, bg);
	mem_free(b->txt_buf);
	mem_free(b->txt);
	mouse_hide(0);
	save.put(g_vga_buf, x, y);
	mouse_show(0);
	snd_obj_stop(g_snd_music);
	g_clock_update = g_refresh = 1;
	g_status_on = 1;
	g_dialog_depth--;
	return res;
}

void savemenu_input(SaveMenu far *b)
{
	poll_input();
	handle_options_key();
}

signed short savemenu_cmp(const void far *pa, const void far *pb)
{
	struct savegame_entry far *a = (struct savegame_entry far *)pa;
	struct savegame_entry far *c = (struct savegame_entry far *)pb;

	if (a->date > c->date)
		return -1;
	if (a->date < c->date)
		return 1;
	if (a->date == c->date) {
		if (a->time > c->time)
			return -1;
		if (a->time < c->time)
			return 1;
	}
	return 0;
}

signed short savemenu_scan(SaveMenu far *b)
{
	signed short n;
	struct ffblk ff;

	sprintf(g_text_output_buf, g_path_fmts[1], "*.GAM");
	if (findfirst(g_text_output_buf, &ff, n = 0) == 0) {
		strtok(ff.ff_name, ".");
		str_pad(ff.ff_name, 8, 0x20);
		strcpy(b->names[n].name, ff.ff_name);
		b->names[n].time = ff.ff_ftime;
		b->names[n].date = ff.ff_fdate;
		n++;
		while (findnext(&ff) == 0 && n < NR_SAVEGAMES) {
			strtok(ff.ff_name, ".");
			str_pad(ff.ff_name, 8, 0x20);
			strcpy(b->names[n].name, ff.ff_name);
			b->names[n].time = ff.ff_ftime;
			b->names[n].date = ff.ff_fdate;
			n++;
		}
	}
	qsort(b->names, n, sizeof(struct savegame_entry), (int (*)(const void *, const void *))savemenu_cmp);
	return n;
}

void savemenu_draw_list(SaveMenu far *b)
{
	signed short y;
	signed short top;
	Bmp t(0x37, 0x52);
	register signed short i;
	register signed short k;

	mouse_hide(0);
	t.fill(0xce);
	top = b->first;
	if (b->count - 10 > 0 && b->count - 10 < top)
		top = b->count - 10;
	t.put(g_vga_buf, 0x1f, 0x3b);
	y = 0x3c;
	for (k = top, i = 0; i < 10 && b->count > i; i++, k++) {
		font_set_colors_w(g_font, b->sel == i ? 0xd8 : 0xda, 0xce);
		font_print(g_font, b->names[k].name, 0x22, y);
		y += b->line_h;
	}
	font_set_colors(g_font, 0xda, 0xce);
	mouse_show(0);
}

void savemenu_select_cur(SaveMenu far *b)
{
	signed short i = 0;

#if FIX_0197
	/* Fix #197: Suche auf die Zahl der Spielstaende begrenzen */
	while (i < b->count && strcmp(b->names[i++].name, b->cur) != 0) {
#else
	while (strcmp(b->names[i++].name, b->cur) != 0) {
#endif
		if (++b->sel >= 10) {
			b->sel = 9;
			b->first++;
			if (b->count - 10 > 0 && b->first > b->count - 10)
				b->first = b->count - 10;
		}
	}
}

void savemenu_scroll_down(SaveMenu far *b)
{
	b->first++;
	if (b->count - 10 > 0 && b->first > b->count - 10)
		b->first = b->count - 10;
}

void savemenu_scroll_up(SaveMenu far *b)
{
	if (--b->first < 0)
		b->first = 0;
}

signed short savegame_write_desc(SaveMenu far *b)
{
	register signed short r;

	lseek(b->fd, 0x10L, SEEK_SET);
	r = write(b->fd, "DESC", 4);
	r += write(b->fd, b->desc, 0x100);
	return r;
}

signed short savegame_read_desc(SaveMenu far *b)
{
	char buf[4];
	register signed short r;

	lseek(b->fd, 0x10L, SEEK_SET);
	r = read(b->fd, buf, 4);
	r += read(b->fd, b->desc, 0x100);
	return r;
}

signed short savemenu_write_desc(SaveMenu far *b, signed short i)
{
	signed short r = 0;
	char name[14];
	char path[40];

	strcpy(name, b->names[i].name);
	strcat(name, ".GAM");
	sprintf(path, g_path_fmts[1], name);
	if ((b->fd = _open(path, O_BINARY | O_WRONLY)) != -1) {
		lseek(b->fd, 0x10L, SEEK_SET);
		r = write(b->fd, "DESC", 4);
		r += write(b->fd, b->desc, 0x100);
		close(b->fd);
		b->last = -1;
	}
	return r;
}

signed short savemenu_read_desc(SaveMenu far *b, signed short i)
{
	signed short r = 0;
	char buf[4];
	char name[14];
	char path[40];

	strcpy(name, b->names[i].name);
	strcat(name, ".GAM");
	sprintf(path, g_path_fmts[1], name);
	if ((b->fd = _open(path, O_BINARY | O_RDONLY)) != -1) {
		savegame_read_header(b);
		lseek(b->fd, 0L, SEEK_SET);
		if (strcmp(b->hdr, g_dsa_version_string) != 0 || b->ver_c != g_version_c) {
			close(b->fd);
			savemenu_clear_desc(b);
		} else {
			lseek(b->fd, 0x10L, SEEK_SET);
			r = read(b->fd, buf, 4);
			r += read(b->fd, b->desc, 0x100);
			close(b->fd);
		}
	}
	return r;
}

signed short savemenu_exists(SaveMenu far *b, char far *s)
{
	signed short i;
	signed short found = 0;

	for (i = 0; b->count > i; i++) {
		if (strcmp(s, b->names[i].name) == 0) {
			found = 1;
			break;
		}
	}
	return found;
}

signed short savemenu_new(SaveMenu far *b)
{
	char far *p;
	signed long free;
	signed short ret = 0;
	signed short ok = 0;
	signed short done = 0;
	char tmp[14];
	char name[14];
	char msg[100];

	free = disk_free();
	if (b->count == NR_SAVEGAMES)
		dlg_message(g_dialog, b->txt[13]);
	else if (free < 0x100000L)
		dlg_message(g_dialog, b->txt[15]);
	else if (!g_group->nr_heroes)
		dlg_message(g_dialog, b->txt[20]);
	else {
		if (b->last != -1)
			savemenu_write_desc(b, b->last + b->first);
		while (done == 0) {
			if (dlg_message_flag(g_dialog, 8, b->txt[0]) != -1) {
				savegame_make_name(name, g_input_buf);
				strcpy(b->cur, name);
				str_pad(b->cur, 8, 0x20);
				strcpy(tmp, name);
				p = strchr(tmp, ' ');
				if (p != 0)
					*p = 0;
				else {
					p = strchr(tmp, '.');
					if (p != 0)
						*p = 0;
				}
				sprintf(msg, b->txt[14], tmp);
				if (!savemenu_exists(b, b->cur) || dlg_yes_no(g_dialog, msg) == 1)
					ok = done = 1;
			} else
				done = 1;
		}
		if (ok != 0) {
			strcat(name, ".GAM");
			sprintf(msg, g_path_fmts[1], name);
			memset(b->desc, 0, sizeof(b->desc));
			if ((b->fd = _creat(msg, 0)) != -1) {
				savegame_write_header(b);
				savegame_write_desc(b);
				close(b->fd);
				ret = 1;
			}
		}
	}
	return ret;
}

void savegame_make_name(char far *d, char far *s)
{
	signed short i;
	register signed char c;

	strcpy(d, s);
	for (i = 0; i < 8; i++) {
		c = d[i];
#if FIX_0197
		/* Fix #197: isalnum mit unsigned char (nicht vor _ctype lesen) */
		if (c != 0 && !isalnum((unsigned char)c) && c != '\x99' && c != '\x8e' && c != '\x9a' && c != '\xe1')
#else
		if (c != 0 && !isalnum(c) && c != '\x99' && c != '\x8e' && c != '\x9a' && c != '\xe1')
#endif
			d[i] = '_';
	}
	d[8] = 0;
}

void savemenu_update(SaveMenu far *b)
{
	if (b->last != -1)
		savemenu_write_desc(b, b->last + b->first);
	savemenu_clear_desc(b);
	savemenu_read_desc(b, b->sel + b->first);
	b->edit->settext(b->desc);
}

void savemenu_clear_desc(SaveMenu far *b)
{
	register signed short i;

	mouse_hide(0);
	for (i = 0; i < 0x31; i++)
		do_h_line(g_vga_buf, 0x77, i + 0x3b, 0xa9, 0xce);
	mouse_show(0);
	memset(b->desc, 0, sizeof(b->desc));
}

void savemenu_touch(SaveMenu far *b)
{
	signed short fd;
	signed short c;

	strcpy(g_text_output_buf, b->names[b->sel + b->first].name);
	strcat(g_text_output_buf, ".GAM");
	sprintf(g_text_buf2, g_path_fmts[1], g_text_output_buf);
#if FIX_0197
	/* Fix #197: Ergebnis von _open pruefen */
	fd = _open(g_text_buf2, O_RDWR);
	if (fd != -1) {
		_read(fd, &c, 1);
		lseek(fd, 0L, SEEK_SET);
		_write(fd, &c, 1);
		close(fd);
	}
#else
	fd = _open(g_text_buf2, O_RDWR);
	_read(fd, &c, 1);
	lseek(fd, 0L, SEEK_SET);
	_write(fd, &c, 1);
	close(fd);
#endif
}
