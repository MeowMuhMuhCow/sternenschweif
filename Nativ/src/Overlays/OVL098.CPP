/* OVL098 - Overlay O098 (Segment 0x213a in SCHWEIF.EXE): NVF-Bildklasse Nvf (0x19 Bytes, s_class.h: Zeilenpuffer
 * buf, Ziel dst, Quellpuffer src, Bildnummer nr, Zeilenabstand stride, Ausgabeart type, Zeiger auf Breite/Hoehe
 * width/height = struct nvf_desc wie R41), liest ueber das Archivobjekt g_file_3 (arc_open, arc_read, arc_seek,
 * arc_size, arc_close). Konstruktor (new(0x19), Puffer mem_alloc(NVF_ROW_BUF)), Destruktor, set() (dst..height), load(id, ...) = set + process(id). process(id):
 * entspricht DSA1 process_nvf, aber direkt aus der Datei: Typbyte (Bit 7 = va), Bildzahl, Kopf (4 / n*4 / n*4+4 /
 * n*8 Bytes), Nummer begrenzen, Offset/Groesse je Typ 0..5 (switch mit Sprungtabelle), dann Ausgabeart 0: PP20
 * (unpack_pp20, Groesse ggf. aus den letzten 4 Bytes per swap_u32 >> 8), Ausgabeart 2..5: RLE (0x7f n v)
 * zeilenweise, 4/5 gespiegelt ueber den Zeilenpuffer, sonst roh lesen. info(id, n, &w, &h): nur Breite/Hoehe von
 * Bild n; count(id): Bildzahl; pal(id, p, n): n*3 Bytes Palette ab Dateiende - 0x300; dims(id, &a, &b): Bildzahl
 * und zwei Worte des Kopfes.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL098.CPP   -> 2532/2532 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h, Klasse Nvf in s_class.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - C++ wegen Konstruktor/Destruktor (this == 0 -> new, flags & 1 -> delete); set() steht oben (push cs /
 *   call near aus load), process() unten (far call, TLINK nop/push cs/call near).
 * - Member src ist "unsigned char huge *": Nulltest dann "mov ax,[8] / or ax,[0a]" statt -3-"cmp dword"
 *   (der Zeilenpuffer buf ist far -> cmp dword im Destruktor). Lokale Laufzeiger bleiben far (inc word).
 * - Breite/Hoehe unsigned short: w * h -> imul word + movzx eax (unsigned -> long); Schleifen gegen
 *   Breite/Hoehe mit jc/jnc, gegen nr (signed) mit jg.
 * - RLE-Schreiben "*(out + x + j) = val" (les/add bx,di/add bx,si); out[x + j] rechnet erst ax = x + j.
 * - "if ((c = *s++) == 0x7f)" -> cmp al,7fh direkt nach dem Speichern; "b = s = huge ? :" -> s zuerst.
 * - -Og-Zwischenwerte: pics - 1 [bp-2e], Offset hdr + nr * 4 [bp-30] (auch in case 2/4 gespeichert, obwohl
 *   nur einmal benutzt), hdr + nr * 8 [bp-32], (int)cnt [bp-34]; Block-Local b (Puffer zum Freigeben) [bp-38].
 * - Wie DSA1: Typ 3/5 nimmt die Groesse von Eintrag i (nach der Schleife == nr); Originalfehler: im
 *   RLE-Zweig bleibt retval uninitialisiert.
 */
#include <io.h>
#include "schweif.h"

#define NVF_HDR_SIZE(type, pics) \
	((type) == NVF_FILE_RAW ? 4 : (type) == NVF_FILE_RAW_SIZES ? (pics) * 4 : \
	 (type) == NVF_FILE_PACKED || (type) == NVF_FILE_PACKED2 ? (pics) * 4 + 4 : (pics) * 8)

Nvf::Nvf()
{
	buf = (unsigned char far *)mem_alloc(NVF_ROW_BUF);
}

Nvf::~Nvf()
{
	if (buf)
		mem_free(buf);
}

void Nvf::set(void far *d, void far *s, signed short n, signed char m, signed short far *w, signed short far *h,
	signed short p)
{
	dst = (unsigned char far *)d;
	src = (unsigned char huge *)s;
	nr = n;
	type = m;
	width = (unsigned short far *)w;
	height = (unsigned short far *)h;
	stride = p;
}

signed long Nvf::load(signed short id, void far *d, void far *s, signed short n, signed char m,
	signed short far *w, signed short far *h, signed short p)
{
	set(d, s, n, m, w, h, p);
	return process(id);
}

signed long Nvf::process(signed short id)
{
	signed long offs;
	signed long p_size;
	signed long retval;
	signed short pics;
	signed short size;
	signed short i;
	unsigned short pic_w;
	unsigned short pic_h;
	signed char nvf_type;
	signed char va;
	unsigned char far *hdr;
	unsigned char far *s;
	unsigned char far *d;
	unsigned char far *out;
	signed char cnt;
	signed char val;
	signed char c;
	register signed short j;
	register unsigned short x;

	arc_open(g_file_3, id);
	arc_read(g_file_3, &nvf_type, 1);
	va = nvf_type & NVF_FLAG_LEN;
	nvf_type &= NVF_TYPE_MASK;
	arc_read(g_file_3, &pics, 2);
	size = NVF_HDR_SIZE(nvf_type, pics);
	hdr = (unsigned char far *)mem_alloc(size);
	arc_read(g_file_3, hdr, size);
	if (nr < 0)
		nr = 0;
	if (nr > pics - 1)
		nr = pics - 1;
	offs = size + 3L;
	switch (nvf_type) {
	case NVF_FILE_RAW:
		pic_w = ((struct nvf_dim far *)hdr)->w;
		pic_h = ((struct nvf_dim far *)hdr)->h;
		p_size = pic_w * pic_h;
		offs += nr * p_size;
		break;
	case NVF_FILE_RAW_SIZES:
		for (i = 0; i < nr; i++) {
			pic_w = ((struct nvf_dim far *)hdr)[i].w;
			pic_h = ((struct nvf_dim far *)hdr)[i].h;
			offs += pic_w * pic_h;
		}
		pic_w = ((struct nvf_dim far *)hdr)[nr].w;
		pic_h = ((struct nvf_dim far *)hdr)[nr].h;
		p_size = pic_w * pic_h;
		break;
	case NVF_FILE_PACKED:
	case NVF_FILE_PACKED2:
		pic_w = ((struct nvf_dims_sizes far *)hdr)->w;
		pic_h = ((struct nvf_dims_sizes far *)hdr)->h;
		for (i = 0; i < nr; i++)
			offs += ((struct nvf_dims_sizes far *)hdr)->size[i];
		p_size = ((struct nvf_dims_sizes far *)hdr)->size[nr];
		break;
	case NVF_FILE_PACKED_SIZES:
	case NVF_FILE_PACKED_SIZES2:
		for (i = 0; i < nr; i++)
			offs += ((struct nvf_dim_size far *)hdr)[i].size;
		pic_w = ((struct nvf_dim_size far *)hdr)[nr].w;
		pic_h = ((struct nvf_dim_size far *)hdr)[nr].h;
		p_size = ((struct nvf_dim_size far *)hdr)[i].size;
		break;
	}
	arc_seek(g_file_3, offs, SEEK_SET);
	mem_free(hdr);
	if (!type) {
		if (va != 0) {			/* ungepackte Groesse aus den letzten 4 Bytes */
			arc_read(g_file_3, &retval, 4);
			arc_seek(g_file_3, retval + (-8L), SEEK_CUR);
			arc_read(g_file_3, &retval, 4);
			retval = swap_u32(retval) >> 8;
		} else
			retval = pic_w * pic_h;
		if (nvf_type != NVF_FILE_RAW && nvf_type != NVF_FILE_RAW_SIZES) {
			s = src == 0 ? (unsigned char huge *)mem_alloc(p_size) : src;
			arc_read(g_file_3, s, p_size);
			unpack_pp20(dst, s, p_size);
			if (src == 0)
				mem_free(s);
		} else
			arc_read(g_file_3, dst, p_size);
	} else if (type >= NVF_OUT_RLE && type <= NVF_OUT_RLE_MIRROR_DENSE) {
		unsigned char far *b;

#if FIX_0009
		/* Fix #9: Rueckgabe im RLE-Zweig setzen */
		retval = (signed long)pic_w * pic_h;
		b = s = src == 0 ? (unsigned char huge *)mem_alloc(p_size) : src;
#else
		b = s = src == 0 ? (unsigned char huge *)mem_alloc(p_size) : src;
#endif
		arc_read(g_file_3, s, p_size);
		d = dst;
		for (i = 0; i < pic_h; i++) {
			if (type == NVF_OUT_RLE_MIRROR || type == NVF_OUT_RLE_MIRROR_DENSE)
				out = buf;
			else
				out = d;
			for (x = 0; x < pic_w; ) {
				if ((c = *s++) == NVF_RLE_MARK) {
					cnt = *s;
					s++;
					val = *s;
					s++;
					for (j = 0; j < cnt; j++)
						*(out + x + j) = val;
					x += cnt;
				} else {
					out[x] = c;
					x++;
				}
			}
			if (type == NVF_OUT_RLE_MIRROR || type == NVF_OUT_RLE_MIRROR_DENSE) {
				out += pic_w - 1;
				for (j = 0; j < pic_w; j++)
					*d++ = *out--;
			}
			if (type == NVF_OUT_RLE)
				d += stride;
			else if (type == NVF_OUT_RLE_MIRROR)
				d += stride - pic_w;
			else if (type == NVF_OUT_RLE_DENSE)
				d += pic_w;
		}
		if (src == 0)
			mem_free(b);
	} else {
		arc_read(g_file_3, dst, p_size);
		retval = p_size;
	}
	arc_close(g_file_3);
	*width = pic_w;
	*height = pic_h;
	return retval;
}

void Nvf::info(signed short id, signed short n, signed short far *w, signed short far *h)
{
	signed short pics;
	signed short size;
	signed short pic_h;
	signed char nvf_type;
	unsigned char far *hdr;
	register signed short i;
	register signed short pic_w;

	arc_open(g_file_3, id);
	arc_read(g_file_3, &nvf_type, 1);
	nvf_type &= NVF_TYPE_MASK;
	arc_read(g_file_3, &pics, 2);
	size = NVF_HDR_SIZE(nvf_type, pics);
	hdr = (unsigned char far *)mem_alloc(size);
	arc_read(g_file_3, hdr, size);
	if (n < 0)
		nr = 0;
	else
		nr = n;
	if (nr > pics - 1)
		nr = pics - 1;
	switch (nvf_type) {
	case NVF_FILE_RAW:
		pic_w = ((struct nvf_dim far *)hdr)->w;
		pic_h = ((struct nvf_dim far *)hdr)->h;
		break;
	case NVF_FILE_RAW_SIZES:
		for (i = 0; i < nr; i++) {
			pic_w = ((struct nvf_dim far *)hdr)[i].w;
			pic_h = ((struct nvf_dim far *)hdr)[i].h;
		}
		pic_w = ((struct nvf_dim far *)hdr)[nr].w;
		pic_h = ((struct nvf_dim far *)hdr)[nr].h;
		break;
	case NVF_FILE_PACKED:
	case NVF_FILE_PACKED2:
		pic_w = ((struct nvf_dims_sizes far *)hdr)->w;
		pic_h = ((struct nvf_dims_sizes far *)hdr)->h;
		break;
	case NVF_FILE_PACKED_SIZES:
	case NVF_FILE_PACKED_SIZES2:
		pic_w = ((struct nvf_dim_size far *)hdr)[nr].w;
		pic_h = ((struct nvf_dim_size far *)hdr)[nr].h;
		break;
	}
	mem_free(hdr);
	arc_close(g_file_3);
	*w = pic_w;
	*h = pic_h;
}

signed short Nvf::count(signed short id)
{
	signed short pics;
	signed char nvf_type;

	arc_open(g_file_3, id);
	arc_read(g_file_3, &nvf_type, 1);
	arc_read(g_file_3, &pics, 2);
	arc_close(g_file_3);
	return pics;
}

void Nvf::pal(signed short id, void far *p, register unsigned short n)
{
	signed long off;

	arc_open(g_file_3, id);
	off = arc_size(g_file_3);
	off -= 0x300;
	arc_seek(g_file_3, off, SEEK_SET);
	arc_read(g_file_3, p, n * 3);
	arc_close(g_file_3);
}

signed short Nvf::dims(signed short id, signed short far *a, signed short far *b)
{
	signed short v;

	arc_open(g_file_3, id);
	arc_read(g_file_3, &v, 1);
	arc_read(g_file_3, &v, 2);
	arc_read(g_file_3, a, 2);
	arc_read(g_file_3, b, 2);
	arc_close(g_file_3);
	return v;
}
