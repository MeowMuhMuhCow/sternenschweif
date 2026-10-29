/* OVL180 - Overlay O180 (Segment 0x22df in SCHWEIF.EXE): Klasse Texteingabefeld (0x28 Bytes, Bildobjekt aus
 * R42 als Member +0x1e). Konstruktor (n Zeichen: Zeichenbreite von '_' ueber R20_035E(ds:37e4), Hintergrund-
 * puffer R07_0002(w*8*n), bmp.set(w, 8, 0)), Destruktor (Puffer frei, bmp.reset), Getter/Setter x/y, f_0151
 * Hintergrund je Zeichen sichern, f_01d2 Vorgabetext bzw. '_' zeichnen, f_028c neu zeichnen, f_0348 Rest
 * wiederherstellen, f_03e0 Rueckschritt, f_04d9 Zeichen ausgeben, run(s, x, y, mode): Eingabeschleife ueber
 * O205_0A4D/tolower (Enter, Esc bzw. ds:8bb5 -> -1, Backspace, erlaubte Zeichen inkl. Umlaute -> R30_004A).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL180.CPP   -> 2104/2104 Bytes GLEICH (18.10.2026), cobjfix laeuft
 * - -Oi: strlen(s) inline (repne scasb, daher di gesichert); -Og: Zwischenwerte [bp-4] (w in f_0151),
 *   [bp-6] (len in f_028c); else-Zweige landen hinter dem Epilog mit eigener Epilogkopie.
 * - Registerwahl f_0151: nur i ist "register" (SI), off ohne register-Schluesselwort -> BCC legt es selbst
 *   nach DI; beide "register" ergibt off in SI (Reihenfolge der Deklaration egal). Kette "off = i = 0".
 * - Member-Objekt Bmp: Konstruktor R42_0008 / Destruktor R42_03B3(.., 2) und delete erzeugt BCC selbst;
 *   Konstruktorparameter n als register (SI), wird bei max == 0 auf 1 gesetzt.
 * - Ketten rechts nach links: "str = pos = s", "*str = g_8bb5 = g_8bed = 0" (al fuer *str wiederverwendet).
 * - "if ((c = tolower(...)) == 0)" eingebettet (mov [bp-4],ax / or ax,ax); Enter und " " bei leerer
 *   Eingabe per continue; Esc-Zweig mit break (toter jmp zum Schleifenkopf dahinter).
 * (Namensfassung mit schweif.h, Klasse Input in s_class.h: 19.10.2026 GLEICH, LINKGLEICH) f_0151 save_bg,
 *   f_01d2 draw_init, f_028c redraw, f_0348 restore_rest, f_03e0 backspace, f_04d9 put_char.
 */
#include <string.h>
#include <ctype.h>
#include "schweif.h"

#define DLG		((Dlg far *)g_dialog)

#define CH_ENTER	0x0d
#define CH_ESC		0x1b
#define CH_BACKSPACE	8
#define INPUT_FIELD	0		/* mode: Feld mit '_' je Zeichen (sonst nur Cursor '_' hinter dem Text) */
#define INPUT_CURSOR	1

Input::Input(register signed short n)
{
	max = n;
	len = 0;
	char_w = font_char_width(g_font, '_');
	if (max == 0)
		n = 1;
	save = (char far *)mem_alloc(char_w * 8 * n);
	bmp.init(char_w, 8, 0);
}

Input::~Input()
{
	mem_free(save);
	bmp.free_lines();
}

signed short Input::get_x()
{
	return x;
}

signed short Input::get_y()
{
	return y;
}

void Input::set_x(signed short v)
{
	x = v;
}

void Input::set_y(signed short v)
{
	y = v;
}

void Input::save_bg()
{
	signed short off;
	register signed short i;
	signed short px;

	off = i = 0;
	px = x;
	while (max > i) {
		bmp.set_base(save + off);
		bmp.get(g_vga_buf, px, y);
		i++;
		px += char_w;
		off += char_w * 8;
	}
}

void Input::draw_init()
{
	register signed short px;
	register signed short i;
	char far *p;

	px = x;
	p = str;
	if (mode == INPUT_FIELD) {
		for (i = 0; max > i; i++) {
			if (g_input_preset && preset_len >= i) {
				font_put_char(g_font, *p, px, y);
				len++;
				p++;
				px += char_w;
				x = px;
			} else {
				font_put_char(g_font, '_', px, y);
				px += char_w;
			}
		}
		px = x;
	} else {
		font_put_char(g_font, '_', px, y);
	}
}

void Input::redraw()
{
	register signed short i;
	register signed short px;
	char far *p;

	px = x - len * char_w;
	for (i = 0, p = str; len > i; i++, px += char_w)
		font_put_char(g_font, *p++, px, y);
	if (mode == INPUT_FIELD) {
		for (i = len; max > i; i++) {
			font_put_char(g_font, '_', px, y);
			px += char_w;
		}
	} else {
		font_put_char(g_font, '_', px, y);
	}
}

void Input::restore_rest()
{
	register signed short i;

	mouse_hide(0);
	while ((i = len) < max) {
		bmp.set_base(save + i * 8 * char_w);
		bmp.put(g_vga_buf, x, y);
		x += char_w;
		len++;
	}
	mouse_show(0);
}

void Input::backspace()
{
	mouse_hide(0);
	if (len > 0) {
		if (mode == INPUT_CURSOR && len != max)
			bmp.put(g_vga_buf, x, y);
		x -= char_w;
		len--;
		pos--;
		bmp.set_base(save + len * 8 * char_w);
		bmp.put(g_vga_buf, x, y);
		font_put_char(g_font, '_', x, y);
	}
	mouse_show(0);
}

void Input::put_char(signed char c)
{
	mouse_hide(0);
	bmp.put(g_vga_buf, x, y);
	font_put_char(g_font, c, x, y);
	x += char_w;
	len++;
	if (mode == INPUT_CURSOR && len != max)
		font_put_char(g_font, '_', x, y);
	mouse_show(0);
}

signed short Input::run(char far *s, signed short px, signed short py, signed short pmode)
{
	signed short r;
	signed short c;

	active = 1;
	x = px;
	y = py;
	mode = pmode;
	str = pos = s;
	r = 0;
	preset_len = strlen(s);
	mouse_hide(0);
	save_bg();
	draw_init();
	mouse_show(0);
	if (g_in_fight == 0)
		wait_ticks(7);
	clear_input();
	mouse_clear_left();
	mouse_clear_right();
	g_mouse_event = 0;
	clear_kbd();
	c = 0;
	while (c != CH_ENTER || len == 0) {
		if ((c = tolower(DLG->wait(this))) == 0)
			c = CH_ENTER;
		g_action = g_key_scan;
		bmp.set_base(save + len * 8 * char_w);
		if (c == CH_ENTER)
			continue;
		if (c == CH_ESC || g_mouse_rclick) {
			*str = g_mouse_rclick = g_action = 0;
			r = -1;
			break;
		} else if (c == CH_BACKSPACE) {
			backspace();
		} else if (c == ' ' || c == '(' || c == ')' || (c >= '-' && c != '=' && c <= 'z') ||
			   c == 0x81 || c == 0x84 || c == 0x94 || c == 0xe1 || c == 0x99 || c == 0x8e ||
			   c == 0x9a || c == '\\') {		/* Umlaute ue ae oe ss Oe Ae Ue (CP437) */
			if (c == ' ' && len == 0)
				continue;
			c = char_toupper(c);
			if (len == max) {
				pos--;
				x -= char_w;
				len--;
				bmp.set_base(save + len * 8 * char_w);
			}
			*pos++ = c;
			put_char(c);
		}
	}
	if (mode == INPUT_FIELD)
		restore_rest();
	*pos = 0;
	active = 0;
	return r;
}
