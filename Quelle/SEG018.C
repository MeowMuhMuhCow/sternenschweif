/*
 *	Rekonstruktion DSA 2 Sternenschweif C1.02 de, Segment seg018 (R18): Maus, Tastatur, Eingabe
 *	Mauszeiger (Bild g_cursor_bmp mit Hotspot, Hintergrund g_cursor_bg_bmp/g_mouse_bg, Sichtbarkeitszaehler
 *	g_mouse_show_cnt), Mausbereichslisten (struct mouse_area), poll_input (Tastatur, Pause mit F10/Strg-P,
 *	Strg-Q beendet, beide Umschalttasten -> Infobildschirm O011, Mausbereiche -> g_action, Klangdienste),
 *	Tastatur-ISR fuer die Richtungsbits g_key_flag.
 *	Compiler: Borland C++ 3.1
 *	  BCC -mlarge -3 -Ot -Oi -c -Y -E..\bc\BIN\TASM.EXE SEG018.C
 *	  (OHNE -O und OHNE -Oe, MIT -Oi; Inline-Assembler in keyboard_isr braucht TASM)
 *	  keyboard_isr steht in #pragma option -k- / -k (kein mov bp,sp, wie im Original)
 *
 *	Stand 17.10.2026: KOMPLETT BYTEGLEICH, 2.872/2.872 Bytes ab R18+0x09,
 *	alle 28 Funktionen, 13 Stellen far->near durch TLINK.
 *	(Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 *	- Objektparameter (Mausbereichsliste, Hintergrundpuffer) wie in s_proto.h als void far *, Zugriff ueber
 *	  Cast-Makros. mouse_show/mouse_hide haben das (unbenutzte) Wort, das die Aufrufer uebergeben.
 *	- Die Felder cw/ch/w/h von g_mouse_bg haben eigene Publics (g_mouse_bg_cw ... g_cursor_h) und werden
 *	  direkt darueber angesprochen, wo das Original es tut.
 */
#include <dos.h>
#include <stdlib.h>
#include <mem.h>
#include <bios.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:6288..62e2, dsana) - initialisierte Daten in Adressreihenfolge */
char g_edit_chars[90] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ\x8e\x99\x9a""1234567890abcdefghijklmnopqrstuvwxyz\x84\x94\x81\xe1()!?:<>/_,.'-[];&\xff";

#define AREA	((struct mouse_area far *)areas)
#define BG	((struct mouse_bg far *)bg)

void mouse_show(signed short unused)
{
	if (g_mouse_locked == 0) {
		mouse_draw();
	}
}

void mouse_hide(signed short unused)
{
	if (g_mouse_locked == 0) {
		mouse_erase();
	}
}

/* int 33h: Funktion *fn, Register bx/cx/dx (bzw. es/si/di) aus den Wortzeigern, Ergebnis zurueck */
void mouse_action(void far *fn, void far *bx, void far *cx, void far *dx, void far *es)
{
	struct SREGS sregs;
	union REGS regs;
	unsigned short bx_in;
	unsigned short cx_in;

	if (*(signed short far *)fn >= 0) {

		regs.x.ax = *(unsigned short far *)fn;
		regs.x.bx = bx_in = *(unsigned short far *)bx;
		regs.x.cx = cx_in = *(unsigned short far *)cx;

		switch (*(unsigned short far *)fn) {
		case 0x9:	/* Zeigerform (Grafik) */
		case 0xc:	/* Ereignisroutine */
		case 0x14:	/* Ereignisroutine tauschen */
		case 0x16:	/* Zustand sichern */
		case 0x17:	/* Zustand laden */
		{
			regs.x.dx = *(unsigned short far *)dx;
			sregs.es = *(unsigned short far *)es;
			break;
		}
		case 0x10: {	/* Ausschlussbereich */
			regs.x.cx = bx_in;
			regs.x.dx = cx_in;
			regs.x.si = *(unsigned short far *)dx;
			regs.x.di = *(unsigned short far *)es;
			break;
		}
		default : {
			regs.x.dx = *(unsigned short far *)dx;
		}
		}

		int86x(0x33, &regs, &regs, &sregs);

		*(unsigned short far *)bx = (*(unsigned short far *)fn == 0x14) ? sregs.es : regs.x.bx;

		*(unsigned short far *)fn = regs.x.ax;
		*(unsigned short far *)cx = regs.x.cx;
		*(unsigned short far *)dx = regs.x.dx;
	}
}

void mouse_move(signed short x, signed short y)
{
	signed short fn;
	signed short bx;
	signed short cx;
	signed short dx;
	signed short es;

	fn = 4;		/* int 33h: Zeiger setzen */
	cx = x;
	dx = y;
	mouse_action(&fn, &bx, &cx, &dx, &es);
	g_mouse_posx = x;
	g_mouse_posy = y;
	g_mouse_moved = 1;
	mouse_update();
}

void call_mouse_isr(void)
{
	mouse_isr();
}

/* Zeigerbild nr aus g_cursor_nvf */
void mouse_set_cursor(signed short nr, signed short redraw)
{
	signed short w;
	signed short h;
	unsigned char nvf[26];		/* NVF-Objekt (R41) */

	nvf_new(nvf);

	if (g_mouse_locked == 0) {
		g_cursor_nr_prev = g_cursor_nr;
		g_cursor_nr = nr;
		nvf_unpack(nvf, gb_data(g_cursor_bmp), g_cursor_nvf, nr, 0, &w, &h, SCREEN_W);
		g_cursor_w = g_mouse_bg_cw = w;
		g_cursor_h = g_mouse_bg_ch = h;
		g_cursor_hot_x = g_cursor_hot_y = 1;
		g_mouse_moved = 1;
		if (redraw == 1) {
			mouse_update();
		}
	}

	nvf_delete(nvf, 2);
}

/* Zeigerbild aus dem Speicher (w x h, Hotspot hot_x/hot_y) */
void mouse_set_cursor_bmp(void far *src, signed short w, signed short h, signed short hot_x, signed short hot_y,
		signed short redraw)
{
	if (g_mouse_locked == 0) {
		g_cursor_nr_prev = g_cursor_nr;
		g_cursor_nr = -1;
		memcpy(gb_data(g_cursor_bmp), src, w * h);
		g_cursor_w = g_mouse_bg_cw = w;
		g_cursor_h = g_mouse_bg_ch = h;
		g_cursor_hot_x = hot_x;
		g_cursor_hot_y = hot_y;
		g_mouse_moved = 1;
		if (redraw == 1) {
			mouse_update();
		}
	}
}

void mouse_erase(void)
{
	struct rect clip_bak;

	if (!g_mouse_busy) {
		if (g_mouse_show_cnt == 0) {
			g_mouse_busy = 1;
			clip_bak = g_clip_rect;
			g_clip_rect.y0 = 0;
			g_clip_rect.y1 = SCREEN_H - 1;
			g_clip_rect.x0 = 0;
			g_clip_rect.x1 = SCREEN_W - 1;
			mouse_restore_bg((signed short far *)&g_mouse_bg);
			g_clip_rect = clip_bak;
			g_mouse_busy = 0;
		}
		g_mouse_show_cnt--;
	}
}

void mouse_draw(void)
{
	struct rect clip_bak;
	register signed short x_max;
	register signed short y_max;

	if (!g_mouse_busy) {
		g_mouse_show_cnt++;
		if (g_mouse_show_cnt == 0) {
			g_mouse_busy = 1;
			x_max = g_mouse_max_x - 1;
			y_max = g_mouse_max_y - 1;

			if (g_mouse_posx < g_cursor_hot_x) g_mouse_posx = g_cursor_hot_x;
			if (g_mouse_posx > x_max) g_mouse_posx = x_max;
			if (g_mouse_posy < g_cursor_hot_y) g_mouse_posy = g_cursor_hot_y;
			if (g_mouse_posy > y_max) g_mouse_posy = y_max;

			clip_bak = g_clip_rect;
			g_clip_rect.y0 = 0;
			g_clip_rect.y1 = SCREEN_H - 1;
			g_clip_rect.x0 = 0;
			g_clip_rect.x1 = SCREEN_W - 1;
			mouse_save_bg(g_mouse_posx - g_cursor_hot_x, g_mouse_posy - g_cursor_hot_y, &g_mouse_bg);
			g_mouse_posx_bak = g_mouse_posx;
			g_mouse_posy_bak = g_mouse_posy;
			mouse_blit(g_mouse_posx - g_cursor_hot_x, g_mouse_posy - g_cursor_hot_y, &g_mouse_bg);
			g_clip_rect = clip_bak;
			g_mouse_busy = 0;
		}
	}
}

void mouse_update(void)
{
	if (g_mouse_moved) {
		g_mouse_moved = 0;
		mouse_erase();
		mouse_draw();
		if (g_mouse_handler_on == 1) {
			mouse_call_handler();
		}
	}
}

signed short is_mouse_in_rect(signed short x0, signed short y0, signed short x1, signed short y1)
{
	signed short m_x;
	signed short m_y;

	m_x = g_mouse_posx;
	m_y = g_mouse_posy;

	return ((m_x >= x0) && (m_x <= x1) && (m_y >= y0) && (m_y <= y1)) ? 1 : 0;
}

/* r = {x0, y0, x1, y1} */
signed short is_mouse_in_rect_p(signed short far *r)
{
	signed short m_x;
	signed short m_y;

	m_x = g_mouse_posx;
	m_y = g_mouse_posy;

	return ((r[0] <= m_x) && (r[2] >= m_x) && (r[1] <= m_y) && (r[3] >= m_y)) ? 1 : 0;
}

signed short mouse_overlaps_rect(signed short x0, signed short y0, signed short x1, signed short y1)
{
	signed short m_x;
	signed short m_y;

	m_x = g_mouse_posx;
	m_y = g_mouse_posy;

	return ((m_x + g_cursor_w - g_cursor_hot_x >= x0) && (m_y + g_cursor_h - g_cursor_hot_y >= y0) &&
		(m_x - g_cursor_hot_x <= x1) && (m_y - g_cursor_hot_y <= y1)) ? 1 : 0;
}

signed short is_click_in_rect(signed short x0, signed short y0, signed short x1, signed short y1)
{
	signed short m_x;
	signed short m_y;

	m_x = g_mouse_clickx;
	m_y = g_mouse_clicky;

	return ((m_x >= x0) && (m_x <= x1) && (m_y >= y0) && (m_y <= y1)) ? 1 : 0;
}

signed short get_click_action(void far *areas)
{
	while (AREA->x0 != -1) {
		if (is_click_in_rect(AREA->x0, AREA->y0, AREA->x1, AREA->y1)) {
			return AREA->action;
		}
		((struct mouse_area far *)areas)++;
	}
	return 0;
}

signed short get_mouse_action(void far *areas)
{
	while (AREA->x0 != -1) {
		if (is_mouse_in_rect(AREA->x0, AREA->y0, AREA->x1, AREA->y1)) {
			return AREA->action;
		}
		((struct mouse_area far *)areas)++;
	}
	return 0;
}

void poll_input(void)
{
	signed short action;
	signed short shift;
	signed short unused;
	struct Bmp bmp;

	unused = 0;
	clear_input();
	action = 0;
	screensaver_run(&g_saver);

	shift = bioskey(2);
	g_key_shift = shift & 3;
	g_key_ctrl = shift & 4;
	g_key_alt = shift & 8;

	g_key = 0;

	if (g_key_buf != 0) {
		screensaver_reset(&g_saver);
		g_key = g_key_buf;
		g_key_buf = 0;
	} else if (bioskey(1)) {
		screensaver_reset(&g_saver);
		g_key = bioskey(0);
	}

	if (g_key != 0) {

		g_key_ascii = g_key & 0xff;
		g_key_scan = (g_key >> 8) & 0xff;
		action = g_key;
		g_key_last = g_key;

		/* mit Umschalttaste: Cursortasten ohne ASCII-Teil */
		if (g_key_shift && g_key_scan == SCAN_RIGHT) action = KEY_RIGHT;
		else if (g_key_shift && g_key_scan == SCAN_LEFT) action = KEY_LEFT;
		else if (g_key_shift && g_key_scan == SCAN_DOWN) action = KEY_DOWN;
		else if (g_key_shift && g_key_scan == SCAN_UP) action = KEY_UP;

		if (g_paused == 0) {

			if (g_key == KEY_F10 || g_key == KEY_CTRL_P) {
				/* Pause: "P" oben links, bis zur naechsten Eingabe */
				gb_new_alloc(&bmp, 8, 8);
				g_paused = 1;
				g_dialog_depth++;
				gb_save(&bmp, g_vga_buf, 0, 0);
				font_set_colors(g_font, 0xff, -1);
				font_put_char(g_font, 'P', 0, 0);

				do {
					poll_input();
					saver_tick(&g_saver);
				} while (g_action == 0);

				gb_blit(&bmp, g_vga_buf, 0, 0);
				font_restore_colors(g_font);
				g_paused = 0;
				g_dialog_depth--;
				action = 0;
				g_mouse_event = 0;
				clear_input();
				mouse_clear_left();
				mouse_clear_right();
				gb_delete(&bmp, 2);
			}

			if (g_key == KEY_CTRL_Q) {
				sys_exit(0);
				exit(0);
			}
		}
	}

	if (g_key_shift == 3 && g_dev_screen_on == 0) {
		dev_info_screen();
	}

	if (g_mouse_event != 0) {
		screensaver_reset(&g_saver);
		action = 0;
		g_mouse_event = 0;

		if (g_mouse_locked == 0) {
			if (g_mouse_areas2 != 0) {
				action = get_click_action(g_mouse_areas2);
			}
			if (action == 0 && g_mouse_areas != 0) {
				action = get_click_action(g_mouse_areas);
			}
		}
	}

	mouse_update();

	if (g_options_open == 0 && g_paused == 0) {
		game_idle();
	} else if (g_snd_service_due == 1) {
		snd_service(g_snd_music);
		snd_speech_service(g_snd_speech);
		snd_service(g_snd_fx);
		g_snd_service_due = 0;
	}

	g_action = action;
}

/* r = {x, y, cw, ch} (Anfang von g_mouse_bg) */
void mouse_restore_bg(signed short far *r)
{
	gb_blit_wh(g_cursor_bg_bmp, g_vga_buf, r[0], r[1], r[2], r[3]);
}

void mouse_save_bg(signed short x, signed short y, void far *bg)
{
	signed short w;
	signed short h;
	signed short x_max;
	signed short y_max;

	w = BG->w;
	h = BG->h;
	x_max = SCREEN_W - 1;
	y_max = SCREEN_H - 1;

	if (x_max - BG->w < x) {
		w = x_max - x + 1;
	}
	if (y_max - BG->h < y) {
		h = y_max - y + 1;
	}

	BG->x = x;
	BG->y = y;
	BG->cw = w;
	BG->ch = h;

	gb_save_wh(g_cursor_bg_bmp, g_vga_buf, x, y, w, h);
}

void mouse_blit(signed short x, signed short y, void far *bg)
{
	signed short w;
	signed short h;
	signed short x_max;
	signed short y_max;

	w = BG->w;
	h = BG->h;
	x_max = SCREEN_W - 1;
	y_max = SCREEN_H - 1;

	if (x_max - BG->w < x) {
		w = x_max - x + 1;
	}
	if (y_max - BG->h < y) {
		h = y_max - y + 1;
	}

	gb_blit_whm(g_cursor_bmp, g_vga_buf, x, y, w, h, 4);
}

/* Tastatur-ISR: alter Handler, dann Richtungsbits in g_key_flag (1 hoch, 2 runter, 4 links, 8 rechts,
 * 0x10 Pos1, 0x20 Ende; gesetzt beim Druecken, geloescht beim Loslassen) und g_key_pad5 */
#pragma option -k-	/* nur fuer die Interruptroutine: kein mov bp,sp */
void interrupt keyboard_isr(void)
{
	g_old_int9();
	asm cli
	asm in al, 60h
	asm mov bl, al
	asm xor ah, ah
	asm sti
	asm mov cx, g_key_flag
	asm and al, 7fh
	asm cmp bl, 7fh
	asm ja kb_release
	asm cmp bl, 48h		/* SCAN_UP */
	asm jnz kb_p0
	asm or cx, 0001h
	asm jmp kb_done
kb_p0:
	asm cmp bl, 50h		/* SCAN_DOWN */
	asm jnz kb_p1
	asm or cx, 0002h
	asm jmp kb_done
kb_p1:
	asm cmp bl, 4bh		/* SCAN_LEFT */
	asm jnz kb_p2
	asm or cx, 0004h
	asm jmp kb_done
kb_p2:
	asm cmp bl, 4dh		/* SCAN_RIGHT */
	asm jnz kb_p3
	asm or cx, 0008h
	asm jmp kb_done
kb_p3:
	asm cmp bl, 47h		/* SCAN_HOME */
	asm jnz kb_p4
	asm or cx, 0010h
	asm jmp kb_done
kb_p4:
	asm cmp bl, 4fh		/* SCAN_END */
	asm jnz kb_p5
	asm or cx, 0020h
	asm jmp kb_done
kb_p5:
	asm cmp bl, 4ch		/* SCAN_PAD5 */
	asm jnz kb_done
	asm mov byte ptr g_key_pad5, 1
	asm jmp kb_done
kb_release:
	asm cmp al, 48h
	asm jnz kb_r0
	asm and cx, 0fffeh
	asm jmp kb_done
kb_r0:
	asm cmp al, 50h
	asm jnz kb_r1
	asm and cx, 0fffdh
	asm jmp kb_done
kb_r1:
	asm cmp al, 4bh
	asm jnz kb_r2
	asm and cx, 0fffbh
	asm jmp kb_done
kb_r2:
	asm cmp al, 4dh
	asm jnz kb_r3
	asm and cx, 0fff7h
	asm jmp kb_done
kb_r3:
	asm cmp al, 47h
	asm jnz kb_r4
	asm and cx, 0ffefh
	asm jmp kb_done
kb_r4:
	asm cmp al, 4fh
	asm jnz kb_r5
	asm and cx, 0ffdfh
	asm jmp kb_done
kb_r5:
	asm cmp al, 4ch
	asm jnz kb_done
	asm mov byte ptr g_key_pad5, 0
kb_done:
	asm mov g_key_flag, cx
}
#pragma option -k

void set_keyboard_isr(void)
{
	g_old_int9 = getvect(9);
	setvect(9, keyboard_isr);
	g_key_flag = 0;
	atexit(reset_keyboard_isr);
}

void reset_keyboard_isr(void)
{
	setvect(9, g_old_int9);
}

void mouse_call_handler(void)
{
	signed short action;

	action = get_mouse_action(g_mouse_handler_areas);
	g_mouse_handler(action);
}

/* Rueckruf des Optionen-Menues (O199): Bereiche haben die Aktionen 0xf0 + Eintrag */
void menu_hover_select(signed short action)
{
	if (action && !g_mouse_lclick && !g_mouse_lpending) {
		g_menu_sel = action - 0xf0;
	}
}

/* Zustand der linken Maustaste loeschen */
void mouse_clear_left(void)
{
	g_mouse_lclick = 0;
	g_mouse_ldown = 0;
	g_mouse_lpending = 0;
}

/* Zustand der rechten Maustaste loeschen */
void mouse_clear_right(void)
{
	g_mouse_rclick = 0;
	g_mouse_rdown = 0;
	g_mouse_rpending = 0;
}

void clear_input(void)
{
	g_key_ascii = 0;
	g_key_scan = 0;
	g_key_shift = 0;
	g_key_ctrl = 0;
	g_key_alt = 0;
	g_key = 0;
	g_action = 0;
}
