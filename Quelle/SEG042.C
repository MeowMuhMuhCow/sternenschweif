/* SEG042 - Bildpuffer-Klasse Bmp (10 Bytes: Breite, Hoehe, eigener Puffer, Zeilenzeiger) als freie Funktionen
 * gb_*: Anlegen (fremder/eigener Puffer, aus Datei, aus dem Archiv), Freigeben, Zeichnen in einen Puffer mit
 * SCREEN_W Bytes je Zeile (R46 blit_lines, geclippt an g_clip_rect) und Holen daraus (R46 grab_lines),
 * Skalieren (R46 scale_lines), Fuellen, Linien, Rahmen, Kopieren zwischen Bmps (seg042 in SCHWEIF.EXE)
 * BCC -mlarge -3 -O -Oe -Og -Oi -Ol -Om -Op -Ov -Ot -Z  (= -O2 ohne -Ob)   -> 2659/2659 Bytes GLEICH
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * - Objekte wie in s_proto.h als void far *, Zugriff ueber die Cast-Makros BMP/SRC/DST auf struct Bmp (s_class.h,
 *   C-Sicht der C++-Klasse Bmp). gb_new* sind Konstruktoren (Objekt 0 -> mem_alloc_w(10)), gb_delete der
 *   Destruktor (flags & 1 -> Objekt freigeben).
 * - Zeichenmodus (BLIT_*): short in den Prototypen wie in der C++-Klasse; blit_lines nimmt das Low-Byte
 *   (gleicher Code wie die fruehere signed-char-Fassung).
 * - Bei gb_blit_a/gb_blit_ab geht ein Wort an den von blit_lines nicht benutzten Parameter; gb_blit_ab gibt
 *   zusaetzlich die Kopierrichtung vor (backwards != 0: rueckwaerts, fuer ueberlappende Bereiche).
 * - gb_vline ist K&R-definiert (Farbe als signed char, Aufrufer uebergeben short).
 * - Originalfehler: gb_new_file legt fuer das Bild nur 1 Byte an ("new unsigned char(w * h)" statt
 *   "new unsigned char[w * h]") und liest dann w * h Bytes hinein. Kopie ueber ax/dx statt eax durch den
 *   Komma-Ausdruck.
 */
#include <mem.h>
#include <io.h>
#include <fcntl.h>
#include "schweif.h"

#define BMP	((struct Bmp far *)bmp)
#define SRC	((struct Bmp far *)src)
#define DST	((struct Bmp far *)dst)

void far *gb_new0(void far *bmp)
{
	if (bmp || (bmp = mem_alloc_w(sizeof(struct Bmp)))) {
		BMP->lines = 0;
		BMP->owns = BMP->f5 = BMP->w = BMP->h = 0;
	}
	return bmp;
}

/* Bmp w x h ueber einen fremden Puffer */
void far *gb_new(void far *bmp, signed short w, signed short h, void far *buf)
{
	signed short i;

	if (bmp || (bmp = mem_alloc_w(sizeof(struct Bmp)))) {
		BMP->w = w;
		BMP->h = h;
		BMP->owns = BMP->f5 = 0;
		BMP->lines = mem_alloc_w(h * 4);
		for (i = 0; i < h; i++)
			BMP->lines[i] = (unsigned char far *)buf + w * i;
	}
	return bmp;
}

/* Bmp w x h mit eigenem Puffer */
void far *gb_new_alloc(void far *bmp, unsigned short w, signed short h)
{
	signed short i;

	if (bmp || (bmp = mem_alloc_w(sizeof(struct Bmp)))) {
		BMP->w = w;
		BMP->h = h;
		BMP->owns = 1;
		BMP->f5 = 0;
		BMP->lines = mem_alloc_w(h * 4);
		BMP->lines[0] = mem_alloc_w(w * h);
		for (i = 1; i < h; i++)
			BMP->lines[i] = BMP->lines[0] + w * i;
	}
	return bmp;
}

/* Bmp w x h aus einer Datei: Bild, danach die Palette nach g_pal_buf (Originalfehler siehe oben) */
void far *gb_new_file(void far *bmp, char far *name, unsigned short w, signed short h)
{
	unsigned char far *pixels;
	signed short i;
	signed short fd;

	if (bmp || (bmp = mem_alloc_w(sizeof(struct Bmp)))) {
		BMP->w = w;
		BMP->h = h;
		BMP->owns = 1;
		BMP->f5 = 0;
		BMP->lines = mem_alloc_w(h * 4);
#if FIX_0211
		/* Fix #211: Puffer fuer w * h Bytes anlegen (statt 1 Byte) */
		pixels = mem_alloc_w(w * h);
		BMP->lines[0] = pixels;
#else
		pixels = mem_alloc_w(1);
		BMP->lines[0] = (*pixels = w * h, pixels);	/* Komma-Ausdruck: Kopie ueber ax/dx statt eax */
#endif
		for (i = 1; i < h; i++)
			BMP->lines[i] = BMP->lines[0] + w * i;
		fd = open(name, O_RDONLY | O_BINARY);
		read(fd, BMP->lines[0], (signed long)BMP->w * BMP->h);
		read(fd, g_pal_buf, NR_COLORS * 3);
		close(fd);
	}
	return bmp;
}

/* Bmp w x h aus Datei id des Archivs (Dateiobjekt g_file_3): Bild, danach die Palette nach g_pal_buf */
void far *gb_new_arch(void far *bmp, unsigned short w, signed short h, signed short id)
{
	signed short i;

	if (bmp || (bmp = mem_alloc_w(sizeof(struct Bmp)))) {
		BMP->w = w;
		BMP->h = h;
		BMP->owns = 1;
		BMP->f5 = 0;
		BMP->lines = mem_alloc_w(h * 4);
		BMP->lines[0] = mem_alloc_w(w * h);
		for (i = 1; i < h; i++)
			BMP->lines[i] = BMP->lines[0] + w * i;
		arc_open(g_file_3, id);
		arc_read(g_file_3, BMP->lines[0], (signed long)BMP->w * BMP->h);
		arc_read(g_file_3, g_pal_buf, NR_COLORS * 3);
		arc_close(g_file_3);
	}
	return bmp;
}

void gb_delete(void far *bmp, signed short flags_arg)
{
	register signed short flags = flags_arg;

	if (bmp) {
		if (BMP->owns)
			mem_free_v(BMP->lines[0]);
		if (BMP->lines) {
			mem_free_v(BMP->lines);
			BMP->lines = 0;
		}
		if (flags & 1)
			mem_free_v(bmp);
	}
}

/* neu einrichten ueber einen fremden Puffer (alte Zeilenzeiger werden nicht freigegeben) */
void gb_init(void far *bmp, signed short w, signed short h, unsigned char far *buf)
{
	signed short i;

	BMP->w = w;
	BMP->h = h;
	BMP->owns = BMP->f5 = 0;
	BMP->lines = mem_alloc_w(h * 4);
	for (i = 0; i < h; i++)
		BMP->lines[i] = buf + w * i;
}

void gb_free_lines(void far *bmp)
{
	if (BMP->lines) {
		mem_free_v(BMP->lines);
		BMP->lines = 0;
	}
}

/* ganzes Bild nach dst (x, y) */
void gb_blit(void far *bmp, void far *dst, signed short x, signed short y)
{
	blit_lines(x, y, BMP->w, BMP->h, 0, BMP->lines, BLIT_COPY, EMS_norm_ptr(dst), 0);
}

void gb_blit_m(void far *bmp, void far *dst, signed short x, signed short y, signed short mode)
{
	blit_lines(x, y, BMP->w, BMP->h, 0, BMP->lines, mode, EMS_norm_ptr(dst), 0);
}

/* linke obere Ecke w x h des Bildes nach dst (x, y) */
void gb_blit_wh(void far *bmp, void far *dst, signed short x, signed short y, signed short w, signed short h)
{
	blit_lines(x, y, w, h, 0, BMP->lines, BLIT_COPY, EMS_norm_ptr(dst), 0);
}

void gb_blit_whm(void far *bmp, void far *dst, signed short x, signed short y, signed short w, signed short h,
		signed short mode)
{
	blit_lines(x, y, w, h, 0, BMP->lines, mode, EMS_norm_ptr(dst), 0);
}

void gb_blit_a(void far *bmp, void far *dst, signed short x, signed short y, signed short w, signed short h,
		signed short unused, signed short mode)
{
	blit_lines(x, y, w, h, unused, BMP->lines, mode, EMS_norm_ptr(dst), 0);
}

void gb_blit_ab(void far *bmp, void far *dst, signed short x, signed short y, signed short w, signed short h,
		signed short unused, signed short mode, signed short backwards)
{
	blit_lines(x, y, w, h, unused, BMP->lines, mode, EMS_norm_ptr(dst), backwards);
}

/* Bild aus src (x, y) holen */
void gb_save(void far *bmp, void far *src, signed short x, signed short y)
{
	grab_lines(x, y, BMP->w, BMP->h, BMP->lines, EMS_norm_ptr(src));
}

void gb_save_wh(void far *bmp, void far *src, signed short x, signed short y, signed short w, signed short h)
{
	grab_lines(x, y, w, h, BMP->lines, EMS_norm_ptr(src));
}

/* Zeilenzeiger auf einen anderen Puffer setzen */
void gb_set_base(void far *bmp, void far *buf)
{
	signed short i;

	for (i = 0; i < BMP->h; i++)
		BMP->lines[i] = (unsigned char far *)buf + i * BMP->w;
}

void gb_load_mem(void far *bmp, void far *src)
{
	memmove(BMP->lines[0], src, BMP->w * BMP->h);
}

/* Bild aus einer Datei laden, danach die Palette nach g_pal_buf */
void gb_load_file(void far *bmp, char far *name)
{
	signed short fd;

	if (BMP->lines[0]) {
		fd = open(name, O_RDONLY | O_BINARY);
		read(fd, BMP->lines[0], BMP->w * BMP->h);
		read(fd, g_pal_buf, NR_COLORS * 3);
		close(fd);
	}
}

void far *gb_data(void far *bmp)
{
	return BMP->lines[0];
}

/* bmp auf die Groesse von dst skalieren (scale_lines erzeugt dafuer Code in g_scale_code) */
void gb_scale(void far *bmp, void far *dst)
{
	g_scale_code = mem_alloc_w(0x1000);
	scale_lines(BMP->lines, BMP->w, BMP->h, DST->lines, DST->w, DST->h);
	mem_free_v(g_scale_code);
}

void gb_clear(void far *bmp)
{
	gb_fill(bmp, 0);
}

void gb_fill(void far *bmp, signed short color)
{
	signed short i;

	for (i = 0; i < BMP->h; i++)
		memset(BMP->lines[i], color, BMP->w);
}

void gb_get_size(void far *bmp, signed short far *w, signed short far *h)
{
	*w = BMP->w;
	*h = BMP->h;
}

/* Rahmen w x h mit der linken oberen Ecke (x, y), ohne Clipping */
void gb_rect(void far *bmp, signed short x, signed short y, signed short w, signed short h, signed short color)
{
	gb_hline(bmp, x, y, w, color);
	gb_hline(bmp, x, y + h - 1, w, color);
	gb_vline(bmp, x, y, h, color);
	gb_vline(bmp, x + w - 1, y, h, color);
}

void gb_hline(void far *bmp, signed short x, signed short y, signed short len, signed short color)
{
	memset(BMP->lines[y] + x, color, len);
}

void gb_vline(bmp, x, y, len, color)
void far *bmp;
signed short x;
signed short y;
signed short len;
signed char color;
{
	signed short i;

	for (i = y; i < y + len; i++)
		BMP->lines[i][x] = color;
}

/* src nach bmp (x, y) kopieren: BLIT_COPY ganz, BLIT_MASKED ohne Farbe 0 (andere Modi tun nichts) */
void gb_put(void far *bmp, void far *src, signed short x, signed short y, signed short mode)
{
	signed short i;

	if (mode == BLIT_COPY) {
		for (i = 0; i < SRC->h; i++)
			memcpy(BMP->lines[y + i] + x, SRC->lines[i], SRC->w);
	} else if (mode == BLIT_MASKED) {
		for (i = 0; i < SRC->h; i++) {
			signed short j;
			unsigned char c;

			for (j = 0; j < SRC->w; j++) {
				if ((c = SRC->lines[i][j]) != 0)
					(BMP->lines[y + i] + j)[x] = c;
			}
		}
	}
}

/* bmp aus src (x, y) fuellen */
void gb_get(void far *bmp, void far *src, signed short x, signed short y)
{
	signed short i;

	for (i = 0; i < BMP->h; i++)
		memcpy(BMP->lines[i], SRC->lines[y + i] + x, BMP->w);
}

void gb_get_wh(void far *bmp, void far *src, signed short x, signed short y, signed short w, signed short h)
{
	signed short i;

	for (i = 0; i < h; i++)
		memcpy(BMP->lines[i], SRC->lines[y + i] + x, w);
}
