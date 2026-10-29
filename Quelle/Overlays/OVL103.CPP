/* OVL103 - Overlay O103 (Segment 0x214e in SCHWEIF.EXE): Tagebuch (Objekt ds:c0c3, 0x2b Bytes, gleiche Struktur
 * wie in O104/O105: +0 Seitenzahl, +2 aktuelle Seite, +4 Text, +10 Texttabelle, +1a/+1b Suchbytes, +1c Merker,
 * +1d[7] Kapitel, +27 Ende, +29 Sperrzaehler). Konstruktor f_0000 (new 0x2b). f_0087(o): Tagebuch-Schleife
 * (Mauszustand ds:8bf1/8bf5/8bef sichern, Seite O104_0D3D, Menue O190_01C6 mit 7 Punkten bzw. Tasten: Schreiben
 * f_06c3, Suchen f_0887, Drucken f_0f1e, Blaettern, Seite waehlen O190_0142, Pos1/Ende, ESC). f_04e1(o, s, v):
 * Eintrag s anhaengen, wenn +29 > 0 (O104_0000 setzt die Seite, Datei ds:b635 Kopf (+0, +1c) lesen, neu schreiben,
 * vorhandene Seiten ueberspringen bzw. ans Ende, Laenge + Text schreiben), liefert 1 bei Erfolg. f_06ab/f_06b7:
 * +29 hoch/runter (Tagebuch sperren). f_06c3(o): eigener Eintrag (Edit-Feld O111 18 x 0xcc in Bmp 0xed x Hoehe,
 * O110_0017, Nullbytes -> '@', Rueckfrage txt[48], f_04e1). f_0887(o): Suche (Kapitelmenue mit Namen aus txt[16..22,
 * 44]; Steuerzeichen ds:6060..6064 = Absatzarten, sonst Text per O225 (new/delete) -> zwei Suchbytes), Trefferliste
 * (new short[o->f0]), Blaettern mit Menue txt[25..28]. f_0f1e(o): Drucken (Datei ds:60f0 bei ds:253c, sonst
 * biosprint-Status 0x90/0x20), Menue Seite/Bereich/alles -> O105_059B.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y -E..\bc\BIN\TASM.EXE OVL103.CPP   -> 4561/4561 Bytes GLEICH (18.10.2026),
 *   cobjfix laeuft. Ohne -Og 10 Bytes, ohne -Oi 18 Bytes zu kurz; -2 statt -3 falsch.
 * - C++: Konstruktor (this == 0 -> new 0x2b, toter Epilog), lokales Bmp (R42) und Edit (O111, 0x38 Bytes) mit
 *   Destruktoren am Funktionsende (Edit zuerst), Suchobjekt O225 per new/delete (ctor(0), dtor(p, 3)).
 * - Konstruktor: "f2 = f0 = f1a = f1b = 0" (mov al,0 / zwei Bytes / cbw / zwei Worte), "f4 = f8 = fc = 0" (eax).
 * - ds:60c0 ist die vom Compiler angelegte statische Kopie eines in C++ mit NICHT-konstanten Werten initialisierten
 *   lokalen Feldes ("char far *names[7] = { txt[16], ... }" -> Elemente nach DS, dann rep movsw ins Local);
 *   seit Phase B (19.10.2026) genau so im Quelltext (Elemente ueber o->f10 als mov ax/mov dx-Paar, nicht eax).
 * - ds:6066 Mausbereiche g_diary_areas[9] (frueher &g_diary_tag5 + 1), Steuerzeichen/Marken als Felder
 *   g_diary_ctrl[4]/g_diary_tag[6] (die Einzelnamen g_diary_ctrl_entry usw. bleiben Publics fuer O104).
 * - Registervariablen: mit memset-/strlen-Intrinsic bzw. Strukturkopie auf Funktionsebene bekommt dort KEINE
 *   register-Variable ein Register (weder SI noch DI). f_06c3: i/j im Block "if (*buf && ..)" (SI/DI); f_0887: j
 *   im Block mit Aufrufen (if (!c1) bzw. if (r != -1)) -> SI; im aufruffreien Schleifenblock landet j in DX.
 * - f_0087: buf[10]/def/n im Block des Falls "Seite waehlen" (sonst verschieben sich die -Og-Zwischenwerte
 *   [bp-6] = o->f0 und [bp-8] = o->f0 - 1); "o->f2 = g_8baf = 0" speichert ds:8baf zuerst.
 * - f_0887: "if (c1 != -1) { .. }" um den ganzen Rest (mit "if (c1 == -1) return;" entsteht eine Epilogkopie);
 *   "g_c4d0 = sel = 0" (sel zuerst); switch auf o->f1d[sel - 1] (-1 in die Verschiebung +1c gefaltet).
 * - f_06c3: "for (j = 999, c = '@'; !buf[j]; j--) ;" - j vor c, c ist ein Block-Local (char [bp-11h]).
 * Originalfehler: f_0887 prueft die Menueauswahl nicht auf -1 (Abbruch): o->f1d[-2] = +1b (ausserhalb der
 * Suche 0) -> Fall 0, d. h. ESC im Suchmenue startet die Suche nach der ersten Absatzart. f_0887 Fall 0:
 * "j += 2" hinter einem Steuerzeichen kann ueber das Stringende hinaus lesen (j > len).
 * (Namensfassung mit schweif.h, class Diary/struct diary: 19.10.2026 GLEICH, LINKGLEICH) Die Funktionen sind Methoden
 *   von Diary: f_0000 Diary(), f_0087 run, f_04e1 add, f_06ab lock_inc, f_06b7 lock_dec, f_06c3 write, f_0887
 *   search, f_0f1e print_menu. Suchobjekt Srch = class Sel (O225), Edit-Feld = class Edit (O111/O110).
 */
#include <string.h>
#include <stdlib.h>
#include <bios.h>
#include <io.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:605c..60fe, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
signed char g_diary_ctrl[4] = {		/* Steuerzeichen im Tagebuchtext */
	(signed char)0xa0,			/* Seitenkopf */
	(signed char)0xa1,			/* Eintrag */
	(signed char)0xa2,			/* eigener Eintrag */
	(signed char)0xa3			/* Ende eines Eintrags */
};
signed char g_diary_tag[6] = {		/* Namensmarken (Kennung des Begriffs folgt), je Suchkapitel */
	(signed char)0xa4, (signed char)0xa5, (signed char)0xa6, (signed char)0xa7, (signed char)0xa8, (signed char)0xa9
};
struct mouse_area g_diary_areas[9] = {	/* Mausbereiche des Tagebuchs (x0, y0, x1, y1, Aktion) */
	{ 80, 16, 180, 23, 1 },
	{ 288, 13, 313, 38, 2 },
	{ 288, 44, 313, 69, 3 },
	{ 288, 75, 313, 100, 4 },
	{ 288, 106, 313, 131, 5 },
	{ 288, 137, 313, 162, 6 },
	{ 288, 168, 313, 193, 7 },
	{ 0, 0, 319, 199, 255 },
	{ -1, -1, -1, -1, -1 }
};
/* Phase B: eigenes BSS (ds:c918..c948) in Adressreihenfolge */
struct talk_desc g_talk;
void far *g_diary_saved_areas;
void (far *g_diary_saved_handler)(short);
signed short g_diary_saved_handler_on;

#define DLG		((Dlg far *)g_dialog)
#define STAR_NAME	(*(char far * far *)g_names_star_idx)	/* Dateiname der Arbeitsdatei 0 */
#define DIARY_TEXT_LEN	1000
#define FILE_DIARY	0		/* Arbeitsdatei DIARY.DAT (Name ueber g_names_star_idx[0]) */
/* Menuepunkte (g_action aus den Mausbereichen bzw. dlg_choose) */
#define DM_PAGE		1
#define DM_WRITE	2
#define DM_PRINT	3
#define DM_SEARCH	4
#define DM_BACK		5
#define DM_NEXT		6
#define DM_END		7
#define KEY_S		0x1f73		/* s: Suchen */
#define KEY_D		0x2064		/* d: Drucken */
#define KEY_G		0x2267		/* g: Seite waehlen */

Diary::Diary()
{
	page = n_pages = find_tag = find_id = 0;
	text = buf = txt_buf = 0;
	txt = 0;
	refs = 0;
	lock = 1;
	chapter = -1;
}

void Diary::run()
{
	signed short first = 1;
	signed short save = lock;

	g_diary_saved_areas = g_mouse_handler_areas;
	g_diary_saved_handler = g_mouse_handler;
	g_diary_saved_handler_on = g_mouse_handler_on;
	g_mouse_handler_on = 0;
	mouse_set_cursor(0, 1);
	lock = 1;
	pal_fade_out();
	ani_halt(g_screen_pic);
	anim_free_all(g_screen_pic);
	snd_play(g_snd_music, 0x2f);
	diary_open(this);
	g_refresh = 1;
	done = 0;
	while (done == 0) {
		if (g_refresh) {
			draw(first, 0);
			if (first)
				first = 0;
			g_refresh = 0;
		}
		g_mouse_areas2 = g_diary_areas;
		poll_input();
		g_mouse_areas2 = 0;
		if (g_mouse_rclick || g_key == KEY_PGUP) {
			signed short r = dlg_choose(g_dialog, txt[0], 7, txt[5], txt[1], txt[3],
				txt[2], txt[6], txt[4], txt[7]);
			if (r != -1)
				g_action = r;
		}
		if (g_action == DM_WRITE || g_key == KEY_INS || g_key == KEY_RETURN)
			write();
		else if (g_action == DM_SEARCH || g_key == KEY_S)
			search();
		else if (g_action == DM_PRINT || g_key == KEY_D)
			print_menu();
		else if (g_action == DM_BACK || g_key == KEY_DOWN || g_key == KEY_LEFT) {
			if (page > 0) {
				if (g_mouse_ldclick)
					page = g_mouse_ldclick = 0;
				else
					page--;
				g_refresh = 1;
			}
		} else if (g_action == DM_PAGE || g_key == KEY_G) {
			char buf[10];
			signed short def, n;
			def = page >= n_pages / 2 ? 1 : n_pages;
			n = dlg_input(g_dialog, strlen(itoa(n_pages, buf, 10)), txt[12], 0, n_pages, def);
			if (n != -1 && n - 1 != page) {
				if (n == 0)
					n = 1;
				page = n - 1;
				g_refresh = 1;
			}
		} else if (g_action == DM_NEXT || g_key == KEY_UP || g_key == KEY_RIGHT) {
			if (g_mouse_ldclick) {
				if (page < n_pages - 1) {
					page = n_pages - 1;
					g_refresh = 1;
					g_mouse_ldclick = 0;
				}
			} else if (page < n_pages - 1) {
				page++;
				g_refresh = 1;
			} else if (dlg_yes_no(g_dialog, txt[14]) == 1) {
				write();
				g_refresh = 1;
			}
		} else if (g_key == KEY_HOME) {
			if (page != 0) {
				page = 0;
				g_refresh = 1;
			}
		} else if (g_key == KEY_END) {
			if (page != n_pages - 1) {
				page = n_pages - 1;
				g_refresh = 1;
			}
		} else if (g_action == DM_END || g_key == KEY_ESC)
			done = 1;
	}
	diary_close(this);
	snd_obj_stop(g_snd_music);
	Sound_stop_music(g_snd_music);
	lock = save;
	g_action = g_key = 0;
	g_refresh = 1;
	g_mouse_handler_areas = g_diary_saved_areas;
	g_mouse_handler = g_diary_saved_handler;
	g_mouse_handler_on = g_diary_saved_handler_on;
}

signed short Diary::add(char far *s, signed short v)
{
	signed short ok = 0;
	signed long pos;
	signed long len;
	unsigned short n;
	unsigned short i;

	if (lock > 0) {
		n = 0;
		diary_open(this);
		diary_hero_names(this);
		STAR_NAME = "DIARY.DAT";
		if (ofile_create(g_ofile, 0, 0x8001) != -1) {
			ofile_read(g_ofile, this, 2L);
			ofile_read(g_ofile, &chapter, 1L);
			ofile_close(g_ofile);
			n = n_pages;
		}
		pos = append(s, v);
		if (pos != 0) {
			STAR_NAME = "DIARY.DAT";
			if (ofile_create(g_ofile, 0, 0x8104) != -1) {
				ofile_write(g_ofile, this, 2L);
				ofile_write(g_ofile, &chapter, 1L);
				if (n_pages > n)
					ofile_seek(g_ofile, 0L, 2);
				else
					for (i = 0; i < n - 1; i++) {
						ofile_read(g_ofile, &len, 4L);
						ofile_seek(g_ofile, len, 1);
					}
				ofile_write(g_ofile, &pos, 4L);
				ofile_write(g_ofile, text, pos);
				ofile_close(g_ofile);
				ok = 1;
			}
		}
		STAR_NAME = 0;
		diary_close(this);
	}
	return ok;
}

void Diary::lock_inc()
{
	lock++;
}

void Diary::lock_dec()
{
	lock--;
}

void Diary::write()
{
	char far *buf;

	buf = (char far *)mem_alloc(1000L);
	page = n_pages;
	n_pages++;
	draw(2, 0);
	n_pages--;
	memset(buf, 0, 1000);
	font_set_colors(g_font, 0xbb, 0xfe);
	Bmp bmp(0xed, font_set_line_h(g_font, -1));
	mouse_hide(0);
	gb_save(&bmp, g_vga_buf, 0, 0x64);
	mouse_show(0);
	Edit w(0x12, 0xcc, 0x19, 0x2e, &bmp, g_vga_buf, 0);
	w.run(buf);
	if (*buf != 0 && dlg_yes_no(g_dialog, txt[48]) == 1) {
		register unsigned short i, j;
		signed char c;
		for (j = 999, c = '@'; !buf[j]; j--)
			;
		for (i = 0; i <= j; i++)
			if (!buf[i])
				buf[i] = c;
		add(buf, 1);
	}
	page = n_pages - 1;
	g_refresh = 1;
	mem_free(buf);
	clear_input();
	mouse_clear_left();
	mouse_clear_right();
	g_mouse_event = 0;
}

void Diary::search()
{
	signed short sel;
	signed char c1;
	signed char c2 = -1;
	signed short n;
	signed short i;
	signed short len;
	signed short done;
	signed short cnt;
	signed short far *list;
	Sel far *sr;
	signed short r;

	n = diary_chapters(this);
	char far *names[7] = { txt[16], txt[17], txt[18], txt[20], txt[21], txt[22], txt[44] };	/* Kapitelnamen der Suche */
	sel = dlg_choose(g_dialog, txt[15], n, names[chapters[0]], names[chapters[1]],
		names[chapters[2]], names[chapters[3]], names[chapters[4]], names[chapters[5]],
		names[chapters[6]]);
#if FIX_0103
	/* Fix #103: ESC (-1) bricht ab (default: c1 = -1) */
	switch (sel == -1 ? -1 : chapters[sel - 1]) {
#else
	switch (chapters[sel - 1]) {
#endif
	case 0: c1 = g_diary_tag[0]; break;
	case 1: c1 = g_diary_tag[1]; break;
	case 2: c1 = g_diary_tag[2]; break;
	case 3: c1 = g_diary_tag[3]; break;
	case 4: c1 = g_diary_tag[4]; break;
	case 5: c1 = g_diary_tag[0]; c2 = g_diary_tag[4]; break;
	case 6: c1 = 0; break;
	default: c1 = -1; break;
	}
	if (c1 != -1) {
	done = 0;
	cnt = 0;
	list = (signed short far *)mem_alloc_w(n_pages * 2);
	memset(list, 0, n_pages * 2);
	if (!c1) {
		register signed short j;
		mouse_set_cursor(5, 1);
		find_tag = -1;
		for (i = 0; i < n_pages; i++) {
			if (load_page(i)) {
				for (j = 0, len = strlen(text); j < len; ) {
#if FIX_0103
					/* Fix #103: nicht ueber das Stringende lesen */
					if (text[j] >= g_diary_tag[0] && text[j] <= g_diary_tag[5])
						j += 2;
					if (j >= len)
						break;
					if (text[j++] == g_diary_ctrl[2]) {
#else
					if (text[j] >= g_diary_tag[0] && text[j] <= g_diary_tag[5])
						j += 2;
					if (text[j++] == g_diary_ctrl[2]) {
#endif
						list[cnt] = i;
						cnt++;
						break;
					}
				}
			}
		}
		mouse_reset();
		if (cnt == 0) {
			dlg_message(g_dialog, txt[43]);
			done = 1;
		}
	} else {
		sr = new Sel;
		r = sr->run(txt[2], c1, c2);
		delete sr;
		if (r != -1) {
			register signed short j;
			mouse_set_cursor(5, 1);
			find_tag = r >> 8;
			find_id = r;
			for (i = 0; i < n_pages; i++) {
				if (load_page(i)) {
					for (j = 0, len = strlen(text); j < len; ) {
						if (text[j++] == find_tag && text[j++] == find_id) {
							list[cnt] = i;
							cnt++;
							break;
						}
					}
				}
			}
			mouse_reset();
		} else
			done = 1;
	}
	i = 0;
	g_refresh = 1;
	while (!done) {
		if (g_refresh) {
			page = list[i];
			draw(0, 0);
			mouse_clear_left();
			mouse_clear_right();
			g_refresh = sel = 0;
		}
		poll_input();
		if (cnt == 1) {
			if (g_key || g_mouse_rclick || g_mouse_lclick) {
				if (g_key != KEY_ESC)
					dlg_message(g_dialog, txt[47]);
				done = 1;
			}
		} else {
			if ((g_key == KEY_UP || g_key == KEY_RIGHT) && cnt - 1 > i)
				sel = 1;
			else if ((g_key == KEY_DOWN || g_key == KEY_LEFT) && i > 0)
				sel = 2;
			else if (g_key == KEY_ESC)
				sel = 3;
			else if (g_key == 0 && g_mouse_rclick == 0 && g_mouse_lclick == 0)
				;
			else if (i == 0) {
				sel = dlg_choose(g_dialog, txt[25], 2, txt[26], txt[28]);
				if (sel == 2)
					sel++;
			} else if (cnt - 1 == i) {
				sel = dlg_choose(g_dialog, txt[25], 2, txt[27], txt[28]);
				sel++;
			} else
				sel = dlg_choose(g_dialog, txt[25], 3, txt[26], txt[27], txt[28]);
			if (sel == 1) {
				if (cnt - 1 > i) {
					g_refresh = 1;
					i++;
				}
			} else if (sel == 2) {
				if (i > 0) {
					g_refresh = 1;
					i--;
				}
			} else if (sel == 3)
				done = 1;
		}
	}
	mem_free_v(list);
	find_tag = find_id = 0;
	g_refresh = 1;
	}
}

void Diary::print_menu()
{
	signed short sel;
	signed short err = 0;
	char far *name = "..\\DIARY.TXT";
	char buf[10];
	register signed short st;

	if (g_print_to_file) {
		unlink(name);
		STAR_NAME = name;
		if (file_open_mode(g_file_main, 0, 0x8102) == -1) {
			err = 1;
			dlg_message(g_dialog, txt[32]);
		}
	} else {
		st = biosprint(2, 0, 0);
		while (st != 0 && st != 0x90) {
			if (dlg_choose(g_dialog, st == 0x20 ? txt[30] : txt[29], 2, txt[35], txt[36]) != 1) {
				err = 1;
				break;
			}
			st = biosprint(2, 0, 0);
		}
	}
	if (!err) {
		sel = dlg_choose(g_dialog, txt[37], 3, txt[38], txt[39], txt[40]);
		if (sel != -1) {
			switch (sel) {
			case 1:
				diary_print_pages(this, page, page);
				break;
			case 2: {
				signed short to;
				st = dlg_input(g_dialog, strlen(itoa(n_pages, buf, 10)), txt[41], 1, n_pages, 1);
				if (st != -1) {
					to = dlg_input(g_dialog, strlen(itoa(n_pages, buf, 10)), txt[42], 1, n_pages, n_pages);
					if (to != -1) {
						if (st > to)
							diary_print_pages(this, to - 1, st - 1);
						else
							diary_print_pages(this, st - 1, to - 1);
					}
				}
				break;
			}
			case 3:
				diary_print_pages(this, 0, n_pages - 1);
				break;
			}
		}
		if (g_print_to_file)
			file_close(g_file_main);
	}
}
