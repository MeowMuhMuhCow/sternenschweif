/* OVL199 - Overlay O199 (Segment 0x2352 in SCHWEIF.EXE): Optionen-Menue, Klasse Opt (s_class.h, 0xed Bytes; Pic als
 * erstes Mitglied, Teilbilder OPTIONS.NVF in buf, Bildpuffer buf_item/buf_popup_bg/buf_popup, gesicherte Maus-/Bereichs-
 * zeiger und Palette). Aufruf aus SEG002 (new, run, delete). Tabelle g_opt_entries (17 Eintraege zu 14 Bytes: x, y,
 * Breite, Zeiger auf den Wert, 4 Bildnummern), Vorgaben g_opt_defaults, Mausbereiche g_opt_areas. Konstruktor/
 * Destruktor (Puffer, Palette g_palette, Klangobjekte g_snd_music/g_snd_fx und g_snd_speech anhalten). run():
 * Hauptschleife (Maus/Tasten, 0x81.. = Klick auf Eintrag, 9..11 Regler slider_mouse, 0x11 Vorgaben defaults, sonst
 * Auswahlliste popup + apply). draw (Bild OPTIONS.AIF), draw_all alle Eintraege, draw_ok OK-Knopf, draw_entry(n),
 * popup(n) Auswahlliste mit popup_select (Listenauswahl mit Maus/Tasten, Markierung vga_save_row/do_border, Blinken
 * bei g_anims_on (Animationen)), draw_slider (Regler zeichnen und Lautstaerke setzen: Sound_volume2/Speech_volume),
 * slider_mouse (Mausbereich g_mouse_range eingeschraenkt), slider_keys, keyboard (Tastennavigation), apply (Musik/
 * Klang ein/aus ueber sound::set_enabled, Sprachausgabe mit Untertitel-Eintrag g_opt_sub_val/g_opt_sub_img0/1,
 * Fall 5 Blickrichtung g_cam_angle einrasten), defaults (Vorgaben).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL199.CPP   -> 6221/6221 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (menu_hover_select: F_R18_0009 -> F_R18_0AC2, g_options_open/g_opt_entries -> _g_6e78+..). Ohne -Og 2804
 *   Abweichungen. (Namensfassung mit schweif.h, Welle N20: 19.10.2026 GLEICH, LINKGLEICH)
 * - Pic als erstes Mitglied (nicht Basisklasse): Destruktoren nvf_delete(+be, 2) und Pic_dtor(this, 2) nach dem Rumpf.
 *   Das NvfMem-Mitglied hat hier nur 0x19 Bytes (Klasse NvfMem0 ohne Fuellbyte).
 * - memcpy(pal, g_palette, 0x300) als -Oi-Intrinsic (push ds / mov ds,ax / rep movsw) -> push si/di im Konstruktor.
 * - "g_mouse_handler = menu_hover_select;" (Funktionszeiger in den residenten Teil): ASM hat nur den Segment-Fixup
 *   und den Offset 0x0ac2 als Zahl, cobjfix biegt den Namen um. "g_mouse_areas = g_opt_areas" -> mov [8bdd],ds /
 *   mov [8bdb],6f7ch.
 * - keyboard: "if (g_mouse_lclick) break;" am Ende des inneren do-while und nach der Schleife -> cmp / jz short /
 *   jmp short (zwei geschachtelte do-while). popup_select: "if (g_mouse_lclick) {..; return sel;} else {..; return
 *   -1;}" (toter Epilog).
 * - popup: Bmp b1 vor der Zuweisung von e deklariert (Konstruktor zuerst), "i = cnt = 0", r als Dummy fuer w/h
 *   und Rueckgabe von popup_select (ein Slot); y0 = e->y + -42 (add ax,0ffd6h, e->y als -Og-Zwischenwert [bp-24h]).
 */
#include <string.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:6e78..704c, dsana) - initialisierte Daten in Adressreihenfolge */
signed short g_npc_file_ids[9] = { 0, 473, 474, 475, 476, 477, 478, 479, 480 };
signed short g_sky_shade = 255;
signed short g_options_open = 0;
struct opt_entry g_opt_entries[17] = {
	{ 117, 46, 28, (signed char far *)&g_opt_music, { 1, 0, -1, -1 } },
	{ 117, 59, 28, (signed char far *)&g_opt_sound, { 1, 0, -1, -1 } },
	{ 117, 72, 28, (signed char far *)&g_cd_speech, { 1, 0, -1, -1 } },
	{ 117, 85, 28, (signed char far *)&g_subtitles, { 1, 0, -1, -1 } },
	{ 117, 98, 28, (signed char far *)&g_anims_on, { 1, 0, -1, -1 } },
	{ 117, 111, 28, (signed char far *)&g_free_move, { 0, 1, -1, -1 } },
	{ 117, 124, 28, (signed char far *)&g_win_centered, { 2, 3, -1, -1 } },
	{ 117, 137, 28, (signed char far *)&g_opt_fight_keep_view, { 1, 0, -1, -1 } },
	{ 117, 180, 85, (signed char far *)&g_game_mode, { 4, 5, -1, -1 } },
	{ 259, 46, 50, (signed char far *)&g_vol_music, { 12, 13, -1, -1 } },
	{ 259, 59, 50, (signed char far *)&g_vol_fx, { 12, 13, -1, -1 } },
	{ 259, 72, 50, (signed char far *)&g_vol_speech, { 12, 13, -1, -1 } },
	{ 259, 85, 50, (signed char far *)&g_print_to_file, { 6, 7, -1, -1 } },
	{ 259, 98, 50, (signed char far *)&g_text_speed, { 8, 9, 10, 11 } },
	{ 259, 111, 50, (signed char far *)&g_anim_speed, { 8, 9, 10, 11 } },
	{ 259, 124, 50, (signed char far *)&g_fight_speed, { 8, 9, 10, 11 } },
	{ 259, 137, 50, (signed char far *)&g_saver_level, { 17, 16, 15, 14 } }
};
struct mouse_area g_opt_areas[19] = {
	{ 117, 46, 144, 58, 129 },
	{ 117, 59, 144, 71, 130 },
	{ 117, 72, 144, 84, 131 },
	{ 117, 85, 144, 97, 132 },
	{ 117, 98, 144, 110, 133 },
	{ 117, 111, 144, 123, 134 },
	{ 117, 124, 144, 136, 135 },
	{ 117, 137, 201, 149, 136 },
	{ 117, 180, 201, 192, 137 },
	{ 259, 46, 308, 58, 138 },
	{ 259, 59, 308, 71, 139 },
	{ 259, 72, 308, 84, 140 },
	{ 259, 85, 308, 97, 141 },
	{ 259, 98, 308, 110, 142 },
	{ 259, 111, 308, 123, 143 },
	{ 259, 124, 308, 136, 144 },
	{ 259, 137, 308, 149, 145 },
	{ 259, 180, 308, 192, 146 },
	{ -1, 0, 0, 0, 0 }
};
signed char g_opt_defaults[18] = { 1, 1, 1, 1, 1, 1, 0, 0, -1, 100, 100, 100, 0, 2, 2, 2, 3, 0 };

#define OPT_PIC		0x2e9		/* OPTIONS.AIF */
#define OPT_NVF		0x2ea		/* OPTIONS.NVF */
#define SOUND(o)	((struct sound far *)(o))	/* Klangobjekt (O042), wie OVL106 */

Opt::Opt()
{
	g_options_open = 1;
	old_pp20 = g_pp20_index;
	buf = mem_alloc(file_length(g_file_main, OPT_NVF));
	buf_item = mem_alloc(0x4b0L);
	buf_popup_bg = mem_alloc(0x10feL);
	buf_popup = mem_alloc(0xff0L);
	pal = mem_alloc(0x300L);
	file_load(g_file_main, OPT_NVF, buf);
	memcpy(pal, g_palette, 0x300);
	old_handler_on = g_mouse_handler_on;
	old_handler = g_mouse_handler;
	old_handler_areas = g_mouse_handler_areas;
	g_mouse_handler_on = 0;
}

Opt::~Opt()
{
	mem_free(buf);
	mem_free(buf_item);
	mem_free(buf_popup_bg);
	mem_free(buf_popup);
	mem_free(pal);
	g_options_open = 0;
	g_pp20_index = old_pp20;
	g_mouse_handler_on = old_handler_on;
	g_mouse_handler = old_handler;
	g_mouse_handler_areas = old_handler_areas;
	snd_obj_stop(g_snd_music);
	snd_obj_stop(g_snd_fx);
	Speech_stop(g_snd_speech);
	if (g_opt_entries[3].img[1] == -1)
		g_subtitles = 1;
}

void Opt::run()
{
	signed short key;
	signed short hid = 0;

	g_dialog_depth++;
	mouse_clear_left();
	old_areas2 = g_mouse_areas2;
	g_mouse_areas2 = 0;
	old_areas = g_mouse_areas;
	g_mouse_areas = g_opt_areas;
	mouse_hide(0);
	save_screen();
	mouse_set_cursor(0, 1);
	mouse_show(0);
	if (g_in_fight != 0 && g_mouse_show_cnt < 0) {
		hid = 1;
		mouse_show(0);
	}
	g_refresh = 1;
	first = 1;
	do {
		if (g_refresh) {
			snd_play(g_snd_music, 0x17);
			draw();
			first = 0;
			key = 0;
			if (g_mouse_locked != 0)
				key = 0x4900;
			g_refresh = 0;
		}
		poll_input();
		if (g_action == 0x4900 || g_action == 0x1c0d || key == 0x4900) {
			key = 0;
			keyboard();
		}
		if ((g_action & 0xff00) == 0x4700)
			defaults();
		if (g_action >= 0x81 && g_action <= 0x100) {
			do {
				g_action -= 0x81;
				if (g_action >= 9 && g_action <= 11) {
					slider_mouse(g_action);
				} else if (g_action == 0x11) {
					defaults();
				} else {
					key = g_action;
					popup(key);
					apply(key);
				}
			} while (g_action != 0);
		}
	} while (g_action != 0x11b && g_mouse_rclick == 0);
	g_mouse_areas = old_areas;
	g_mouse_areas2 = old_areas2;
	g_step_active = 0;
	g_step_turn = 0;
	g_cam_speed = 0;
	g_step_pending = 0;
	pal_fade_out();
	mouse_hide(0);
	restore_screen();
	mouse_reset();
	mouse_show(0);
	pal_fade_in(pal);
	if (hid)
		mouse_hide(0);
	g_dialog_depth--;
	poll_input();
	clear_input();
	mouse_clear_left();
	mouse_clear_right();
}

void Opt::draw()
{
	void far *p = mem_alloc(0x300L);

	pal_fade_out();
	mouse_hide(0);
	pic.load(g_pp20_index = OPT_PIC, g_vga_buf, (struct rgb far *)p);
	draw_all();
	mouse_show(0);
	pal_fade_in(p);
	mem_free(p);
}

void Opt::draw_all()
{
	draw_entry(0);
	draw_entry(1);
	draw_entry(2);
	draw_entry(3);
	draw_entry(4);
	draw_entry(5);
	draw_entry(6);
	draw_entry(7);
	draw_entry(8);
	draw_entry(12);
	draw_entry(13);
	draw_entry(14);
	draw_entry(15);
	draw_entry(16);
	draw_slider(9);
	draw_slider(10);
	draw_slider(11);
	draw_ok();
}

void Opt::draw_ok()
{
	signed short w;
	Bmp b(0x32, 0x0c, buf_item);

	nvf.unpack(buf_item, buf, 0x12, 0, &w, &w, 0x140);
	mouse_hide(0);
	b.put(g_vga_buf, 0x103, 0xb4);
	mouse_show(0);
}

void Opt::draw_entry(signed short n)
{
	signed short w;
	signed short v;
	struct opt_entry far *e = &g_opt_entries[n];
	Bmp b(e->w, 0x0c, buf_item);

	v = *e->val;
	nvf.unpack(buf_item, buf, e->img[v], 0, &w, &w, 0x140);
	mouse_hide(0);
	b.put(g_vga_buf, e->x, e->y);
	mouse_show(0);
}

void Opt::popup(signed short n)
{
	signed short cnt;
	signed short w;
	signed short r;
	signed short y0;
	signed short y1;
	struct opt_entry far *e;
	register signed short i;
	register signed short x;
	Bmp b1(0x57, 0x32, buf_popup_bg);

	e = &g_opt_entries[n];
	w = e->w;
	for (i = cnt = 0; i < 4; i++)
		if (e->img[i] != -1)
			cnt++;
	Bmp b2(w, cnt * 12, buf_popup);
	b2.clear();
	x = e->x + 8;
	y0 = e->y + -42;
	y1 = e->y + 6 - cnt * 12;
	mouse_hide(0);
	b1.get(g_vga_buf, x, y0);
	b2.put(g_vga_buf, x + 2, y1 + 2);
	for (i = 0; i < cnt; i++)
		nvf.unpack((char far *)buf_popup + w * i * 12, buf, e->img[i], 0, &r, &r, 0x140);
	b2.put(g_vga_buf, x, y1);
	mouse_show(0);
	if ((r = popup_select(cnt, x, y1, w, *e->val)) == -1) {
		mouse_hide(0);
		b1.put(g_vga_buf, x, y0);
		mouse_show(0);
	} else {
		mouse_hide(0);
		b2.get(g_vga_buf, x, y1);
		mouse_show(0);
		if (g_anims_on == 1) {
			for (i = 0; i < 8; i++) {
				mouse_hide(0);
				b1.put(g_vga_buf, x, y0);
				mouse_show(0);
				wait_ticks(4);
				mouse_hide(0);
				b2.put(g_vga_buf, x, y1);
				mouse_show(0);
				wait_ticks(4);
			}
		}
		*e->val = r;
		screensaver_reset(&g_saver);
		mouse_hide(0);
		b1.put(g_vga_buf, x, y0);
		draw_entry(n);
		mouse_show(0);
	}
	clear_input();
	mouse_clear_left();
}

signed short Opt::popup_select(signed short n, register signed short x, signed short y, signed short w, signed short cur)
{
	signed short i;
	signed short sel;
	signed short old;
	signed short yy;
	unsigned char far *s1;
	unsigned char far *s2;
	unsigned char far *s3;
	unsigned char far *s4;
	void far *saved;
	struct mouse_area a[5];

	s1 = new unsigned char[0x140];
	s2 = new unsigned char[0x140];
	s3 = new unsigned char[0xc8];
	s4 = new unsigned char[0xc8];
	for (i = 0; i < n; i++) {
		a[i].x0 = x;
		a[i].y0 = y + i * 12;
		a[i].x1 = x + w;
		a[i].y1 = i * 12 + y + 12;
		a[i].action = i + 0xf0;
	}
	a[i].x0 = -1;
	old = sel = -1;
	g_menu_sel = cur;
	saved = g_mouse_areas;
	g_mouse_handler_areas = a;
	g_mouse_handler = menu_hover_select;
	g_mouse_handler_on = 1;
	mouse_clear_left();
	do {
		g_mouse_areas2 = a;
		poll_input();
		g_mouse_areas2 = 0;
		if (g_action == 0x5000) {
			if (++g_menu_sel >= n)
				g_menu_sel = 0;
		} else if (g_action == 0x4800) {
			if (--g_menu_sel < 0)
				g_menu_sel = n - 1;
		} else if (g_action == 0x1c0d) {
			g_mouse_lclick = 1;
			break;
		}
		if (g_action >= 0x80 && g_action < 0xf0) {
			g_mouse_lclick = 0;
			break;
		}
		if (sel != g_menu_sel) {
			sel = g_menu_sel;
			yy = y + sel * 12;
			mouse_hide(0);
			if (old != -1) {
				vga_restore_row(old * 0x140, s1);
				vga_restore_row(old * 0x140 + 0xdc0, s2);
				vga_restore_col(x, s3);
				vga_restore_col(x + w - 1, s4);
				old = -1;
			}
			vga_save_row(yy * 0x140, s1);
			vga_save_row(yy * 0x140 + 0xdc0, s2);
			vga_save_col(x, s3);
			vga_save_col(x + w - 1, s4);
			do_border(g_vga_buf, x, yy, w, 0x0c, 0xfd);
			mouse_show(0);
			old = yy;
		}
	} while (g_mouse_lclick == 0 && g_mouse_rclick == 0 && g_action != 0x11b);
	g_mouse_handler_on = 0;
	g_mouse_areas = saved;
	delete s1;
	delete s2;
	delete s3;
	delete s4;
	if (g_mouse_lclick) {
		clear_input();
		mouse_clear_left();
		return sel;
	} else {
		if (g_action < 0x80 || g_action >= 0xf0)
			g_action = 0;
		return -1;
	}
}

void Opt::draw_slider(register signed short n)
{
	signed short w;
	signed short h;
	signed short xx;
	unsigned char far *knob = new unsigned char[0x38];
	struct opt_entry far *e = &g_opt_entries[n];
	register signed short v;

	v = *e->val;
	if (v < 1)
		v = *e->val = 1;
	else if (v > 100)
		v = *e->val = 100;
	w = e->w;
	Bmp b1(w, h = 12, buf_item);
	Bmp b2(7, 8, knob);
	nvf.unpack(buf_item, buf, e->img[0], 0, &w, &h, 0x140);
	nvf.unpack(knob, buf, e->img[1], 0, &w, &h, 0x140);
	xx = e->x + v / 4 + 9;
	mouse_hide(0);
	b1.put(g_vga_buf, e->x, e->y);
	b2.put(g_vga_buf, xx, e->y + 2);
	mouse_show(0);
	delete knob;
	g_opt_areas[n].x0 = xx;
	g_opt_areas[n].x1 = xx + 7;
	if (!first) {
		if (n == 9) {
			Sound_volume2(g_snd_music, v, 0);
		} else if (n == 10) {
			if (!snd_busy(g_snd_fx, 0)) {
				if (snd_speech_busy(g_snd_speech) == 1)
					Speech_stop(g_snd_speech);
				snd_play(g_snd_fx, 0xaa);
			}
			Sound_volume2(g_snd_fx, v, 0);
		} else if (n == 11) {
			if (!snd_speech_busy(g_snd_speech)) {
				if (snd_busy(g_snd_fx, 0) == 1)
					snd_obj_stop(g_snd_fx);
				snd_speech(g_snd_speech, 5);
			}
			Speech_volume(g_snd_speech, v);
		}
	}
}

void Opt::slider_mouse(signed short n)
{
	signed short d;
	signed short oy;
	signed short x0;
	signed short x1;
	struct opt_entry far *e;
	struct mouse_range save;
	signed short m1;
	signed short m2;
	signed short m3;
	signed short m4;
	signed short m5;
	register signed short last;
	register signed short ox;

	ox = -1;
	oy = -1;
	e = &g_opt_entries[n];
	x0 = e->x + 14;
	x1 = x0 + 32;
	save = g_mouse_range;
	g_mouse_range.min_x = x0;
	g_mouse_range.max_x = x1;
	g_mouse_range.min_y = e->y;
	g_mouse_range.max_y = e->y + 12;
	last = g_mouse_clickx;
	draw_slider(n);
	g_mouse_isr_lock = 1;
	do {
		poll_input();
		m1 = 3;
		m3 = g_mouse_posx;
		m4 = g_mouse_posy;
		mouse_action(&m1, &m2, &m3, &m4, &m5);
		g_mouse_posx = m3;
		g_mouse_posy = m4;
		if (g_mouse_posx > g_mouse_range.max_x)
			g_mouse_posx = g_mouse_range.max_x;
		if (g_mouse_posx < g_mouse_range.min_x)
			g_mouse_posx = g_mouse_range.min_x;
		if (g_mouse_posy < g_mouse_range.min_y)
			g_mouse_posy = g_mouse_range.min_y;
		if (g_mouse_posy > g_mouse_range.max_y)
			g_mouse_posy = g_mouse_range.max_y;
		m1 = 4;
		m3 = g_mouse_posx;
		m4 = g_mouse_posy;
		mouse_action(&m1, &m2, &m3, &m4, &m5);
		if (ox != g_mouse_posx || oy != g_mouse_posy) {
			ox = g_mouse_posx;
			oy = g_mouse_posy;
			g_mouse_moved = 1;
			mouse_hide(0);
			mouse_update();
			mouse_show(0);
		} else {
			mouse_update();
		}
		if (g_mouse_posx != last) {
			d = g_mouse_posx - last;
			*e->val += d << 2;
			last = g_mouse_posx;
			draw_slider(n);
		}
	} while (g_mouse_ldown);
	g_mouse_isr_lock = 0;
	clear_input();
	mouse_clear_left();
	g_mouse_range = save;
}

void Opt::slider_keys(register signed short n)
{
	signed short v;
	struct opt_entry far *e;
	register signed short last;

	e = &g_opt_entries[n];
	last = v = *e->val / 4;
	do {
		poll_input();
		if (g_action == 0x4d00) {
			if (++v >= 26)
				v = 25;
		} else if (g_action == 0x4b00) {
			if (--v < 0)
				v = 0;
		}
		if (g_action != 0x11b && g_action != 0x1c0d && g_action != 0x5000 && g_action != 0x4800)
			g_action = 0;
		if (v != last) {
			last = v;
			*e->val = v << 2;
			draw_slider(n);
		}
	} while (g_action != 0x11b && g_action != 0x1c0d && g_action != 0x5000 && g_action != 0x4800);
	g_action = 0;
}

void Opt::keyboard()
{
	signed short ow;
	signed short ox;
	signed short osel;
	signed short w;
	signed short y;
	signed short x;
	unsigned char far *s1;
	unsigned char far *s2;
	unsigned char far *s3;
	unsigned char far *s4;
	register signed short sel;
	register signed short oy;

	s1 = new unsigned char[0x140];
	s2 = new unsigned char[0x140];
	s3 = new unsigned char[0xc8];
	s4 = new unsigned char[0xc8];
	oy = ox = ow = osel = -1;
	sel = 0;
	do {
		do {
			poll_input();
			if ((g_action & 0xff00) == 0x4700) {
				defaults();
			} else if (g_action == 0x5000) {
				if (++sel >= 17)
					sel = 0;
			} else if (g_action == 0x4800) {
				if (--sel < 0)
					sel = 16;
			}
			if (sel != osel) {
				struct opt_entry far *e;
				osel = sel;
				e = &g_opt_entries[sel];
				y = e->y;
				x = e->x == 0x75 ? 8 : 0x9e;
				w = e->x == 0x75 ? 0x6c : 0x64;
				mouse_hide(0);
				if (oy != -1) {
					vga_restore_row(oy * 0x140, s1);
					vga_restore_row(oy * 0x140 + 0xdc0, s2);
					vga_restore_col(ox, s3);
					vga_restore_col(ox + ow - 1, s4);
					oy = -1;
				}
				vga_save_row(y * 0x140, s1);
				vga_save_row(y * 0x140 + 0xdc0, s2);
				vga_save_col(x, s3);
				vga_save_col(x + w - 1, s4);
				do_border(g_vga_buf, x, y, w, 0x0c, 0xfd);
				mouse_show(0);
				oy = y;
				ow = w;
				ox = x;
			}
			if (g_mouse_lclick)
				break;
		} while (g_action != 0x11b && g_action != 0x1c0d && g_action != 0x5100 && g_action != 0x4d00);
		if (g_mouse_lclick)
			break;
		if (g_action == 0x1c0d || g_action == 0x4d00) {
			if (sel == 9 || sel == 10 || sel == 11) {
				slider_keys(sel);
			} else {
				popup(sel);
				apply(sel);
			}
		}
	} while (g_action != 0x11b && g_action != 0x5100);
	if (oy != -1) {
		mouse_hide(0);
		vga_restore_row(oy * 0x140, s1);
		vga_restore_row(oy * 0x140 + 0xdc0, s2);
		vga_restore_col(ox, s3);
		vga_restore_col(ox + ow - 1, s4);
		mouse_show(0);
	}
	delete s1;
	delete s2;
	delete s3;
	delete s4;
	if (!g_mouse_lclick && g_action != 0x11b)
		clear_input();
	mouse_clear_left();
}

void Opt::apply(register signed short n)
{
	register signed short v;

	v = *g_opt_entries[n].val;
	switch (n) {
	case 0:
		if (v == 1) {
			SOUND(g_snd_music)->set_enabled(v);
			snd_play(g_snd_music, 0x17);
		} else {
			snd_obj_stop(g_snd_music);
			SOUND(g_snd_music)->set_enabled(v);
		}
		break;
	case 1:
		if (v == 1) {
			SOUND(g_snd_fx)->set_enabled(v);
			snd_play(g_snd_fx, 0xaa);
		} else {
			snd_obj_stop(g_snd_fx);
			SOUND(g_snd_fx)->set_enabled(v);
		}
		break;
	case 2:
		if (v == 1) {
			Speech_set(g_snd_speech, v);
			snd_speech(g_snd_speech, 5);
			g_opt_entries[3].img[0] = 1;
			g_opt_entries[3].img[1] = 0;
			if (*g_opt_entries[3].val == 0)
				*g_opt_entries[3].val = 1;
			draw_entry(3);
		} else {
			Speech_stop(g_snd_speech);
			Speech_set(g_snd_speech, v);
			g_opt_entries[3].img[0] = 0;
			g_opt_entries[3].img[1] = -1;
			if (*g_opt_entries[3].val == 1)
				*g_opt_entries[3].val = 0;
			draw_entry(3);
		}
		break;
	case 5:
		g_cam_pitch = 0;
		g_cam_angle += 0x80;
		g_cam_angle &= 0x700;
		cam_set_from_group();
		break;
	}
}

void Opt::defaults()
{
	register signed short i;

	for (i = 0; i < 17; i++) {
		struct opt_entry far *e = &g_opt_entries[i];
		if (g_opt_defaults[i] != -1)
			*e->val = g_opt_defaults[i];
	}
	mouse_hide(0);
	draw_all();
	mouse_show(0);
	g_action = 0;
}
