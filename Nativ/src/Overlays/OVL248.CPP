/* OVL248 - Overlay O248 (Segment 0x246b in SCHWEIF.EXE): Schrift-/Textobjekt Font (0x49 Bytes, Daten struct font;
 * Objekt g_font ds:37e4) - Konstruktor laedt die Zeichensatzdatei id ueber das Dateiobjekt g_file_main (O037), legt
 * die 256er-Zeigertabelle glyphs und den Glyphpuffer bg_pic (max_w+1)*(max_h+1) an (font_init liest max_w/max_h),
 * Destruktor gibt alles frei; set_line_h (alter Wert, -1 = nur lesen), Schreibposition ueber font_set_x/font_set_y,
 * Textdatei laden und an Nullbytes in eine Zeigerliste zerlegen (split/load_text/load_list = txt_load_list, auch von
 * OVL182 benutzt), max_width: breiteste Zeile in Pixeln ('@' = Zeilenumbruch, Zeichenbreite font_glyph + 1).
 *
 * BCC -mlarge -3 -Ot -c -Y OVL248.CPP   -> 1005/1005 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h, Klasse Font in s_class.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Gewoehnlicher C++-Konstruktor (if (this==0) this = new(0x49) + toter Epilog) und Destruktor (flags&1 -> delete).
 * - box_x = box_y = colsel = 0 als Kette (mov al,0 / Byte speichern / mov ah,0 / zwei Worte).
 * - split() steht vor load_text()/load_list() -> push cs / call near; !*s ergibt mov al/cbw/or ax,ax.
 * - set_line_h: Parameter h landet ohne -Og in DX (Blattfunktion), Registervariablen sonst automatisch (SI/DI).
 * - bits_start = Byte 1 der Zeichensatzdatei (hoeherwertiges Byte des Zaehlers file->n).
 */
#include "schweif.h"

Font::Font(signed short id)
{
	file_open(g_file_main, id);
	file = (struct font_file far *)mem_alloc(file_length2(g_file_main));
	File_read_all(g_file_main, file);
	file_close(g_file_main);
	bits_start = ((unsigned char far *)file)[1];
	glyphs = (struct glyph far * far *)mem_alloc(0x400L);
	font_init(this);
	bg_pic = mem_alloc((signed long)((max_w + 1) * (max_h + 1)));
	indent = 0;
	align = 0;
	box_w = SCREEN_W;
	box_x = box_y = colsel = 0;
	shadow = 0;
	line_h = max_h;
	screen = (unsigned char far *)g_vga_buf;
}

Font::~Font()
{
	signed short i;

	for (i = 0; i < 0x100; i++)
		if (glyphs[i])
			mem_free(glyphs[i]);
	mem_free(glyphs);
	mem_free(bg_pic);
	mem_free(file);
}

signed short Font::set_line_h(signed short h)
{
	signed short old;

	old = line_h;
	if (h != -1)
		line_h = h;
	return old;
}

signed short Font::goto_x(signed short x)
{
	font_set_x(this, x);
	return x;
}

void Font::goto_xy(signed short x, signed short y)
{
	font_set_x(this, x);
	font_set_y(this, y);
}

void Font::split(char far * far *list, char far *s, signed long len)
{
	signed long i;

	i = 0;
	*list++ = s;
	while (i != len) {
		if (!*s)
			*list++ = s + 1;
		s++;
		i++;
	}
}

void Font::load_text(signed short id, char far *buf, char far * far *list)
{
	signed long len;

	len = file_load(g_file_main, id, buf);
	split(list, buf, len);
}

void Font::load_list(signed short id, signed short n, void far * far *pbuf, char far * far * far *plist)
{
	signed long len;

	if (id != -1) {
		len = file_length(g_file_main, id);
		*pbuf = mem_alloc(len);
		n++;
		*plist = (char far * far *)mem_alloc((signed long)(n * 4));
		len = file_load(g_file_main, id, *pbuf);
		split(*plist, (char far *)*pbuf, len);
	}
}

signed short Font::max_width(char far *s)
{
	unsigned char c;
	signed short cw;
	signed short cur, max;

	max = 0;
	cur = 0;
	while ((c = *s++) != 0) {
		font_glyph(this, c, &cw);
		cur += cw + 1;
		if (c == FONT_CH_NEWLINE) {
			if (cur > max)
				max = cur;
			cur = 0;
		}
	}
	if (cur > max)
		max = cur;
	return max;
}
