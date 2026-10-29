/* SEG011 - Kompass der 3D-Ansicht (seg011 in SCHWEIF.EXE): Objekt struct compass (6 Bytes: Puffer mit der
 * NVF-Datei 0x3b4 aus g_file_main, Bildnummer). Compass_ctor(p, pic_no) legt das Objekt an (p == NULL ->
 * mem_alloc_w(6)) und laedt die Datei, Compass_dtor(p, flags) gibt den Puffer frei (flags & 1: auch das Objekt).
 * compass_draw(p, angle): Kompassrahmen (Bild pic_no, 0x32 x 0x0d) nach g_renderbuf_ptr (0x4e, 0), dann
 * compass_draw_strip mit (angle + 0x100) & 0x3ff. compass_draw_strip: Bild 0 (Richtungsband, 0x4b0 Bytes)
 * entpacken, Ausschnitt 0x18 x 9 ab Spalte angle * 0x3b3 / 10000 nach (0x5b, 2). Aufrufer: SEG012 je Bild
 * (compass_draw(g_compass, g_cam_angle)), O010/O014/O046 legen g_compass an.
 *
 * BCC -mlarge -3 -Ot -c -Y SEG011.C   -> 595/595 Bytes GLEICH
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * - Bildpuffer als C-Struktur struct Bmp (s_class.h, gleicher Aufbau wie die C++-Klasse), Methoden als gb_*.
 */
#include "schweif.h"

#define COMPASS_FILE	0x3b4		/* NVF-Datei mit Kompassrahmen und Richtungsband */

struct compass far *Compass_ctor(struct compass far *p, signed short pic_no)
{
	signed long size;

	if (p || (p = mem_alloc_w(sizeof(struct compass)))) {
		p->pic_no = pic_no;
		size = file_length(g_file_main, COMPASS_FILE);
		p->buf = mem_alloc(size);
		file_load(g_file_main, COMPASS_FILE, p->buf);
	}
	return p;
}

void Compass_dtor(struct compass far *p, signed short flags_arg)
{
	register signed short flags = flags_arg;

	if (p) {
		p->buf = mem_free(p->buf);
		if (flags & 1)
			mem_free_v(p);
	}
}

void compass_draw(struct compass far *p, signed short angle)
{
	signed short w;			/* Breite/Hoehe aus nvf_unpack (unbenutzt) */
	BMP_PTR bmp;
	char nvf[26];			/* NVF-Entpacker (R41) */

	nvf_new(nvf);
	bmp = gb_new_alloc(0, 0x32, 0xd);
	nvf_unpack(nvf, gb_data(bmp), p->buf, p->pic_no, 0, &w, &w, 320);
	gb_blit_m(bmp, g_renderbuf_ptr, 0x4e, 0, 4);
	gb_delete(bmp, 3);
	compass_draw_strip(p, (angle + 0x100) & 0x3ff);
	nvf_delete(nvf, 2);
}

void compass_draw_strip(struct compass far *p, signed short angle)
{
	signed short col;		/* erste sichtbare Spalte des Richtungsbands */
	signed short w;
	unsigned char far *strip;
	struct Bmp bmp;
	char nvf[26];

	nvf_new(nvf);
	strip = mem_alloc(0x4b0);
	nvf_unpack(nvf, strip, p->buf, 0, 0, &w, &w, 320);
	col = (signed short)((signed long)angle * 0x3b3 / 10000);
	gb_new(&bmp, 0x78, 9, strip + col);
	gb_blit_whm(&bmp, g_renderbuf_ptr, 0x5b, 2, 0x18, 9, 4);
	mem_free(strip);
	gb_delete(&bmp, 2);
	nvf_delete(nvf, 2);
}
