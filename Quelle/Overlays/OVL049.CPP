/* OVL049 - Overlay O049 (Segment 0x201e in SCHWEIF.EXE): Gespraechsfuehrung (Dialog-Objekt, meist ds:b86e; weitere
 * Methoden in O257). f_0000(o, id): Satz id (4 Bytes: Skript +36b, Rueckruf +36d) aus Datei ds:35fe (0x203),
 * Stufe +36f = O257_0C8B, Script(+36b) aus O260 (+51e), Texte 0x24/0xf3; schon fertig (+36f >= Script->fa2)
 * -> O257_03A2/110C, sonst Gespraech: Puffer +33e (1000) / +342 (64000) / Bmp +522, Texte 0x409/0x0a, Palette
 * sichern (new 0x300), Schleife bis +8 (f_124c/f_0b8e, Tasten/Maus ds:8bed: Hoch/Runter ds:c696, Enter/Maus ->
 * f_0cc3, Menue txt1a[0..3]), Abschlussmenue bei +8 == 2, Aufraeumen; immer O257_059E/0CC5, Ergebnis 1 bei
 * Gespraech. f_0ae7(o, s, mode): Text ausgeben (mode 0 -> nach +33e), f_15b4. f_0b8e(o, n): Markierung von +4
 * nach n. f_0cc3(o, n): Thema +436[n] waehlen: Antwort (Skript-Abschnitt, Rueckruf ds:778e[+36d](0, t), Sonder-
 * werte -1..-5 -> f_10ef), anzeigen, Zaehler +1 der Antwortliste, Rueckruf (1, m); Ergebnis r == -1. f_0fef(o, f):
 * Themenliste neu (Fenster ds:3303 kurz auf 0xe1), +2 = Anzahl, +4 = -1. f_10ef(o, t, k): Zufallstext aus Tabelle
 * +1ae[t] (t 7..41, 57) bzw. +12 nach +352[t] (9 Bytes). f_124c(o, mode): Bildschirm (1/3: Pic 0x4f, Portraet,
 * Name, Palette; 2: Teilbild; sonst ganz). f_151d: Merker 2 -> 0. f_1546/f_158e: Mausbereiche ds:5048 und
 * Handler R02_025D setzen / zuruecknehmen (+52e). f_15af: leer. f_15b4: Rahmen, wenn +33e nicht leer.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL049.CPP   -> 5623/5623 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (ds:5048 -> _s_5041+7, Handler seg F_R02_0005:025d -> F_R02_025D). Ohne -Og 3085 Abweichungen.
 * - Kein -Oi: memcpy/memset/strlen/strcpy sind echte Aufrufe.
 * - f_0000: -Og-Zwischenwerte [bp-0eh] = o->f2 und [bp-10h] = (int)dir - dafuer "(signed short)g_c4c5->dir == 3"
 *   (ohne Cast cmp byte je Vergleich); "if (--g_menu_line < 1)" -> dec / mov ax / cmp ax,1; sel als register-Variable
 *   in den inneren Bloecken (ausserhalb benutzt BCC SI als Hilfsregister: les si); Fensterwerte x1..y2/c als
 *   Block-Locals hinter den Zwischenwerten.
 * - f_0cc3: "k = e->w[j = e->cur]" eingebettet; Sprungtabelle -5..-1: -1 | -4,-2 | -3 | -5 (Rumpfreihenfolge
 *   -1, -4/-2, -3, -5); "return r == -1 ? 1 : 0;" mit totem Epilog; "++e->cur >= e->n" -> inc al / Store / cmp.
 * - f_0fef: "o->f436[k + 1]" -> [bx+438h]; Fenster ds:3303 als 8-Byte-Struktur (wortweise Kopie hin und zurueck).
 * - f_10ef: register-Parameter t in SI, r ohne register -> DI; Fallreihenfolge 1 | 0,2 | 3.
 * - f_124c: Pic (O269) am Blockanfang, Bmp b(0x20, 0x20, ds:c4fb) mitten im Block und davor deklariert als
 *   x1..y2 (b liegt bei [bp-0ah], Pic als grosses Objekt am Ende [bp-0b8h]).
 * - f_1546: "g_8bf5 = (void far *)R02_025d" -> Segment zuerst, dann Offset (wie im Original).
 * Auffaelliges: f_10ef laesst das Ergebnis fuer k ausserhalb 0..3 uninitialisiert (kommt nicht vor); f_15af leer.
 * (Namensfassung mit schweif.h, class Conv/Script in s_class.h: 19.10.2026 GLEICH, LINKGLEICH) Die Funktionen sind
 *   Methoden von Conv (gleicher Code wie mit dem Objekt als erstem Argument): f_0000 run, f_0ae7 print, f_0b8e
 *   mark_topic, f_0cc3 ask, f_0fef draw_topics, f_10ef random_text, f_124c draw, f_151d clear_new, f_1546 mouse_on,
 *   f_158e mouse_off, f_15af dummy, f_15b4 answer_frame. +0x436[n] = list[n - 1] (Menuezeilen ab 1).
 */
#include <string.h>
#include <mem.h>
#include "schweif.h"

#define DLG		((Dlg far *)g_dialog)
/* Phase B: eigene Daten des Moduls (ds:5048..514e, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
char g_conv_areas[260] = {
	0xab, 0x00, 0x07, 0x00, 0xd1, 0x00, 0x20, 0x00, 0x81, 0x00, 0xe6, 0x00, 0x03, 0x00, 0x40, 0x01,
	0x0a, 0x00, 0x82, 0x00, 0xe6, 0x00, 0x0b, 0x00, 0x40, 0x01, 0x12, 0x00, 0x83, 0x00, 0xe6, 0x00,
	0x13, 0x00, 0x40, 0x01, 0x1a, 0x00, 0x84, 0x00, 0xe6, 0x00, 0x1b, 0x00, 0x40, 0x01, 0x22, 0x00,
	0x85, 0x00, 0xe6, 0x00, 0x23, 0x00, 0x40, 0x01, 0x2a, 0x00, 0x86, 0x00, 0xe6, 0x00, 0x2b, 0x00,
	0x40, 0x01, 0x32, 0x00, 0x87, 0x00, 0xe6, 0x00, 0x33, 0x00, 0x40, 0x01, 0x3a, 0x00, 0x88, 0x00,
	0xe6, 0x00, 0x3b, 0x00, 0x40, 0x01, 0x42, 0x00, 0x89, 0x00, 0xe6, 0x00, 0x43, 0x00, 0x40, 0x01,
	0x4a, 0x00, 0x8a, 0x00, 0xe6, 0x00, 0x4b, 0x00, 0x40, 0x01, 0x52, 0x00, 0x8b, 0x00, 0xe6, 0x00,
	0x53, 0x00, 0x40, 0x01, 0x5a, 0x00, 0x8c, 0x00, 0xe6, 0x00, 0x5b, 0x00, 0x40, 0x01, 0x62, 0x00,
	0x8d, 0x00, 0xe6, 0x00, 0x63, 0x00, 0x40, 0x01, 0x6a, 0x00, 0x8e, 0x00, 0xe6, 0x00, 0x6b, 0x00,
	0x40, 0x01, 0x72, 0x00, 0x8f, 0x00, 0xe6, 0x00, 0x73, 0x00, 0x40, 0x01, 0x7a, 0x00, 0x90, 0x00,
	0xe6, 0x00, 0x7b, 0x00, 0x40, 0x01, 0x82, 0x00, 0x91, 0x00, 0xe6, 0x00, 0x83, 0x00, 0x40, 0x01,
	0x8a, 0x00, 0x92, 0x00, 0xe6, 0x00, 0x8b, 0x00, 0x40, 0x01, 0x92, 0x00, 0x93, 0x00, 0xe6, 0x00,
	0x93, 0x00, 0x40, 0x01, 0x9a, 0x00, 0x94, 0x00, 0xe6, 0x00, 0x9b, 0x00, 0x40, 0x01, 0xa2, 0x00,
	0x95, 0x00, 0xe6, 0x00, 0xa3, 0x00, 0x40, 0x01, 0xaa, 0x00, 0x96, 0x00, 0xe6, 0x00, 0xab, 0x00,
	0x40, 0x01, 0xb2, 0x00, 0x97, 0x00, 0xe6, 0x00, 0xb3, 0x00, 0x40, 0x01, 0xba, 0x00, 0x98, 0x00,
	0xe6, 0x00, 0xbb, 0x00, 0x40, 0x01, 0xc2, 0x00, 0x99, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff
};
#define CONV_AREAS	((void far *)g_conv_areas)	/* ds:5048 Mausbereiche des Gespraechs (Phase B: eigene Daten von O049) */
#define FILE_CONV	0x203		/* je Gespraech 4 Bytes: Skript, Rueckruf */
#define TXT_CONV	0x24		/* Texte des Gespraechs (0xf3 Eintraege) */
#define TXT_CONV_N	0xf3
#define TXT_MENU	0x409		/* Menuetexte (10 Eintraege) */
#define PIC_CONV	0x4f		/* Vollbild des Gespraechs */
#define TOPIC_BYE	0x61
#define TOPIC_NO_ANSWER	0x63
#define TOPIC_NOTMORE	100
#define MENU_X		0xe1		/* Themenspalte */
#define FONT_SET_COLORS	((void (*)(void far *, signed short, signed short))font_set_colors)	/* Farben als short (alte Sicht) */

signed short Conv::run(signed short conv_id)
{
	signed short col_fg;
	signed short col_bg;
	signed short redraw;
	char far *pal;
	signed short ret = 0;
	register signed short i;

	id = conv_id;
	if (g_cd_music) {
		snd_obj_stop(g_snd_music);
	}
	arc_open(g_file_3, FILE_CONV);
	arc_seek(g_file_3, (signed long)id << 2, 0);
	arc_read(g_file_3, &script_id, 4L);
	arc_close(g_file_3);
	level = get_counter(id);
	script = new Script(script_id);
	txt_load_list(g_font, TXT_CONV, TXT_CONV_N, &txt_buf, &txt);
	clear_kbd();
	clear_input();
	mouse_clear_left();
	if (level >= script->tolerance) {
		script_topics();
		refuse();
	} else {
		g_dialog_depth++;
		mem_free(txt_buf);
		mem_free(txt);
		delete script;
		ani_stop(g_screen_pic);
		script = new Script(script_id);
		txt_load_list(g_font, TXT_CONV, TXT_CONV_N, &txt_buf, &txt);
		script_topics();
		ret = 1;
		snd_play(g_snd_music, 9);
		g_refresh = done = redraw = 0;
		refresh = 0;
		topic = -1;
		answer = (char far *)mem_alloc(1000L);
		bg = mem_alloc(64000L);
		bmp.init(0x140, 0xc8, bg);
		old_line_h = font_set_line_h(g_font, 8);
		font_get_colors(g_font, &col_fg, &col_bg);
		txt_load_list(g_font, TXT_MENU, 0x0a, &menu_buf, &menu_txt);
		place_topics();
		build_list();
		pal = new char[0x300];
		memcpy(pal, g_pal_buf, 0x300);
		get_palette(g_pal_buf, 0, 0x100);
		pal_fade_out();
		mouse_on();
		draw(3);
		greet();
		do {
			if (g_refresh) {
				draw(redraw);
				redraw = 0;
				g_refresh = 0;
			} else {
				mark_topic(g_menu_line);
			}
			poll_input();
			if ((g_mouse_lclick && g_action == 0x81) || g_action == KEY_INS || g_action == KEY_F5 || g_action == KEY_CTRL_J) {
				note();
				clear_input();
				mouse_clear_left();
			} else if (level >= script->tolerance) {
				bye();
			} else if (g_mouse_rclick || g_action == KEY_PGUP) {
				signed short x1, y1, x2, y2, c;
				register signed short sel;

				font_get_box(g_font, &x1, &y1, &x2, &y2);
				c = font_get_y(g_font);
				if (topic != -1 && *answer) {
					sel = dlg_choose(g_dialog, menu_txt[0], 3, menu_txt[1], menu_txt[2], menu_txt[3]);
					if (sel == 1) {
						note();
					} else if (sel == 2) {
						done = 1;
					}
				} else if (dlg_choose(g_dialog, menu_txt[0], 2, menu_txt[2], menu_txt[3]) == 1) {
					done = 1;
				}
				font_set_box(g_font, x1, y1, x2, y2);
				font_set_y(g_font, c);
				mouse_on();
			} else if (g_action == KEY_ESC) {
				done = 1;
			} else if (g_action == KEY_UP) {
				if (--g_menu_line < 1) {
					g_menu_line = n_shown;
				}
				if (n_shown < g_menu_line) {
					g_menu_line = n_shown;
				}
			} else if (g_action == KEY_DOWN) {
				if (n_shown < ++g_menu_line) {
					g_menu_line = 1;
				}
			} else if ((g_mouse_lclick && g_action != 0) || g_action == KEY_RETURN) {
				register signed short sel;

				clear_new();
				if (g_action == KEY_RETURN) {
					g_action = g_menu_line;
				} else if (g_action >= 0x81 && g_action <= 0x99) {
					g_action -= 0x81;
				} else {
					g_action = -1;
				}
				if (g_action != -1 && n_shown >= g_action) {
					sel = list[g_action - 1];
					if (sel != TOPIC_BYE) {
						level++;
					}
					if (((signed short)g_group->loctype == 3 && sel == 4) || ((signed short)g_group->loctype == 3 && sel == 5) ||
						((signed short)g_group->loctype == 2 && sel == 5)) {
						level--;
					}
					if (ask(g_action) == 1) {
						redraw = g_refresh = 1;
					}
				}
				clear_input();
				mouse_clear_left();
				if (refresh && done == 0) {
					if (refresh == 1) {
						remove_temp();
					}
					if (redraw == 0) {
						draw_topics(1);
					}
					refresh--;
				}
			}
		} while (done == 0);
		if (done == 2) {
			set_menu(1, g_ttx[377]);
			draw_menu();
			while (done == 2) {
				poll_input();
				if (g_mouse_rclick || g_action == KEY_PGUP) {
					signed short x1, y1, x2, y2, c;
					register signed short sel;

					font_get_box(g_font, &x1, &y1, &x2, &y2);
					c = font_get_y(g_font);
					if (topic != -1 && *answer) {
						sel = dlg_choose(g_dialog, menu_txt[0], 2, menu_txt[1], menu_txt[2]);
						if (sel == 1) {
							note();
						} else if (sel == 2) {
							done = 1;
						}
					} else {
						sel = dlg_choose(g_dialog, menu_txt[0], 1, menu_txt[2]);
						if (sel == 1) {
							done = 1;
						}
					}
					font_set_box(g_font, x1, y1, x2, y2);
					font_set_y(g_font, c);
					mouse_on();
				} else if (g_action == 0x81 || g_action == KEY_INS || g_action == KEY_F5 || g_action == KEY_CTRL_J) {
					note();
				} else if (g_mouse_lclick || g_action == KEY_RETURN || g_action == KEY_ESC) {
					done = 1;
				}
				g_action = g_mouse_lclick = g_mouse_rclick = 0;
			}
		}
		Sound_volume3(g_snd_music, 0, 1000, 0);
		g_status_on = 1;
		mouse_off();
		pal_fade_out();
		memcpy(g_pal_buf, pal, 0x300);
		delete pal;
		mouse_hide(0);
		memset(g_vga_buf, 0, 64000U);
		mouse_show(0);
		for (i = 0; i < 100; i++) {
			if (topic_buf[i]) {
				topic_buf[i] = mem_free(topic_buf[i]);
			}
			if (topic_txts[i]) {
				topic_txts[i] = (char far * far *)mem_free(topic_txts[i]);
			}
		}
		mem_free(menu_buf);
		mem_free(menu_txt);
		font_set_line_h(g_font, old_line_h);
		FONT_SET_COLORS(g_font, col_fg, col_bg);
		bmp.free_lines();
		mem_free(bg);
		mem_free(answer);
		g_refresh = 1;
		snd_obj_stop(g_snd_music);
		g_dialog_depth--;
		set_palette(g_pal_buf, 0, 0x100);
	}
	relock();
	mem_free(txt_buf);
	mem_free(txt);
	delete script;
	set_counter(id);
	clear_input();
	mouse_clear_left();
	mouse_clear_right();
	g_mouse_event = 0;
	return ret;
}

void Conv::print(char far *s, signed short mode)
{
	signed short old;

	if (strlen(s)) {
		if (mode == 0) {
			strcpy(answer, s);
		}
		font_print_wrapped(g_font, s);
		old = font_set_line_h(g_font, 2);
		font_print_wrapped(g_font, "@");
		font_set_line_h(g_font, old);
	} else {
		memset(answer, 0, 1000);
	}
	answer_frame();
}

void Conv::mark_topic(signed short n)
{
	signed short t;
	register signed short i;
	register signed short k;

	if (mark != n && n > 0) {
		if (mark != -1) {
			for (i = k = 0; i < 100; i++) {
				if ((t = topic_state[i]) != 0) {
					if (mark == ++k) {
						FONT_SET_COLORS(g_font, t == 1 ? 0xa4 : 0xaa, -1);
						font_print(g_font, script->titles[i], MENU_X, --k * 8 + 3);
						break;
					}
				}
			}
		}
		for (i = k = 0; i < 100; i++) {
			if ((t = topic_state[i]) != 0) {
				if (++k == n) {
					FONT_SET_COLORS(g_font, t == 1 ? 0xff : 0xf9, -1);
					font_print(g_font, script->titles[i], MENU_X, --k * 8 + 3);
					break;
				}
			}
		}
		mark = n;
	}
}

signed short Conv::ask(signed short n)
{
	signed short k = 0;
	signed short m = 0;
	signed short r = 0;
	char far *ans;
	struct script_topic far *s;
	struct script_ent far *e;
	signed short j;
	CONV_CB fp;
	register signed short t;

	t = list[n - 1];
	ans = 0;
	s = script->topics + t;
	e = s->list[k];
	topic = t;
	if (t == TOPIC_BYE) {
		level--;
		bye();
	} else if (s->list != 0) {
		if (callback) {
			fp = g_conv_callbacks[callback];
			k = fp(0, t);
		}
		e = s->list[k];
		k = e->answers[j = e->cur];
		m = e->vals[j];
		switch (k) {
		case SCRIPT_ANS_NULL:
			if (no_answer != 0) {
				ans = no_answer;
			} else {
				ans = random_text(TOPIC_NO_ANSWER, 0);
			}
			break;
		case SCRIPT_ANS_LIST:
		case SCRIPT_ANS_LIST_TRUE:
			ans = random_text(t, 2);
			break;
		case SCRIPT_ANS_LIST_FALSE:
			ans = random_text(t, 3);
			break;
		case SCRIPT_ANS_NOTMORE:
			ans = random_text(TOPIC_NOTMORE, 0);
			break;
		default:
			ans = script->texts[k];
			break;
		}
	} else {
		if (no_answer != 0) {
			ans = no_answer;
		} else {
			ans = random_text(TOPIC_NO_ANSWER, 0);
		}
	}
	if (t != 0x61) {
		draw(2);
		font_set_colors(g_font, 0xf9, -1);
		print(random_text(t, 1), 1);
		font_set_colors(g_font, 0xff, -1);
		print(ans, 0);
	}
	if (s->list != 0) {
		if (m < 0) {
			m = -m;
		} else if (++e->cur >= e->n) {
			e->cur = e->n - 1;
		}
		if (m && callback) {
			mouse_off();
			fp = g_conv_callbacks[callback];
			r = fp(1, m);
			mouse_on();
		}
	}
	return r == -1 ? 1 : 0;
}

void Conv::draw_topics(signed short flag)
{
	struct rect save;
	register signed short i;
	register signed short k;

	if (flag) {
		mouse_hide(0);
		save = g_clip_rect;
		g_clip_rect.x0 = MENU_X;
		bmp.put(g_vga_buf, 0, 0);
		g_clip_rect = save;
		mouse_show(0);
	}
	for (i = k = 0; i < 100; i++) {
		if (list[k + 1 - 1] == i) {
			FONT_SET_COLORS(g_font, topic_state[i] == 1 ? 0xa4 : 0xaa, -1);
			font_print(g_font, script->titles[i], MENU_X, k * 8 + 3);
			k++;
		}
	}
	n_shown = k;
	mark = -1;
}

char far *Conv::random_text(register signed short t, signed char kind)
{
	char far *res;
	char far * far *tab;
	signed short r;

	if ((t >= 7 && t < 42) || t == 57) {
		tab = topic_txts[t];
	} else {
		tab = txt;
	}
	switch (kind) {
	case 1:
		r = random_schweif(topic_txt[t].q_n) - 1;
		res = tab[topic_txt[t].q_first + r];
		break;
	case 0:
	case 2:
		r = random_schweif(topic_txt[t].a_n) - 1;
		res = tab[topic_txt[t].a_first + r];
		break;
	case 3:
		r = random_schweif(topic_txt[t].alt_n) - 1;
		res = tab[topic_txt[t].alt_first + r];
		break;
	}
	return res;
}

void Conv::draw(register signed short mode)
{
	font_set_colors(g_font, 0xff, -1);
	font_set_box(g_font, 0x0a, 0xd2, 0x2e, -1);
	font_set_tab(g_font, 0xcd, 0);
	if (mode == 1 || mode == 3) {
		ani_halt(g_screen_pic);
		g_status_on = 0;
		mouse_hide(0);
		Pic big;
		big.loadf(g_pp20_index = PIC_CONV, bg);
		bmp.put(g_vga_buf, 0, 0);
		Bmp b(0x20, 0x20, g_portrait_buf);
		signed short x1, y1, x2, y2;
		load_portrait_buf(script->pic - 1);
		do_border(g_vga_buf, 9, 9, 0x22, 0x22, 0xff);
		b.put(g_vga_buf, 10, 10);
		font_set_colors(g_font, 0xfd, -1);
		font_get_box(g_font, &x1, &y1, &x2, &y2);
		font_set_box(g_font, 0x2e, 0x78, 8, -1);
		font_print_wrapped(g_font, script->name);
		font_set_box(g_font, x1, y1, x2, y2);
		bmp.get(g_vga_buf, 0, 0);
		mouse_show(0);
		g_menu_line = 1;
		memcpy(g_pal_buf, big.get_palette(), 0x30);
		if (mode == 3) {
			pal_fade_in(g_pal_buf);
		} else {
			set_palette(g_pal_buf, 0, 0xa0);
		}
	} else if (mode == 2) {
		mouse_hide(0);
		bmp.put_part(g_vga_buf, 0, 0, 0xe1, 0xc8);
		mouse_show(0);
	} else {
		mouse_hide(0);
		bmp.put(g_vga_buf, 0, 0);
		mouse_show(0);
	}
	if (refresh == 0) {
		draw_topics(0);
	}
	answer_frame();
}

void Conv::clear_new()
{
	register signed short i;

	for (i = 0; i < 100; i++) {
		if (topic_state[i] == 2) {
			topic_state[i] = 0;
		}
	}
}

void Conv::mouse_on()
{
	if (g_mouse_areas != 0) {
		saved_areas = g_mouse_areas;
		g_mouse_areas = 0;
	}
	g_mouse_areas2 = g_mouse_handler_areas = CONV_AREAS;
	g_mouse_handler = set_menu_action;
	g_mouse_handler_on = 1;
}

void Conv::mouse_off()
{
	g_mouse_handler_on = 0;
	g_mouse_areas = saved_areas;
	g_mouse_areas2 = 0;
}

void Conv::dummy()
{
}

void Conv::answer_frame()
{
	if (*answer) {
		mouse_hide(0);
		do_border(g_vga_buf, 0xab, 7, 0x27, 0x1a, 0xd6);
		mouse_show(0);
	}
}
