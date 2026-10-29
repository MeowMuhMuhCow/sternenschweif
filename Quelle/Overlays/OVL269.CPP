/* OVL269 - Overlay O269 (Segment 0x24d9 in SCHWEIF.EXE): Bildklasse Pic (0xa6 Bytes, s_class.h), u.a. lokal in
 * O023/O035/O188, per new in O035/O039. Der 30-Byte-Kopf liegt im Objekt selbst (magic "AIF", Packart pack 0..3,
 * no_pal, Breite/Hoehe w/h, Farbzahl ncolors, erste Farbe first_color), Palette pal, Quelle from_arc: 0 = Datei-
 * objekt file (O037, Vorgabe g_file_main), sonst Archivobjekt arc (R06, Vorgabe g_file_3). Konstruktoren
 * Pic(file) / Pic(), Destruktor (eigene Palette per delete, eigene Bilddaten per farfree). Ladefunktionen load/
 * loadf/sload/sloadf (12 Ueberladungen: Datei/Archiv, mit/ohne Ziel und Palette, die "s"-Varianten setzen vorher
 * stream) -> do_load: Kopf (read_header), Palette anlegen (new char[n * 3]) und am Dateiende lesen (read_pal,
 * Werte & 0x3f), Daten (read_data: Groesse = Laenge - 30 - n * 3) entpacken (decode, Ziel ggf. farmalloc(w * h)):
 * Art 0 roh, 1 RLE 0x7f n v (rle ueber den Bytepuffer buf/get_byte, im Strom-Modus Assemblerroutine rle_fast),
 * 2 PackBits (packbits), 3 PP20 (unpack_pp20). set_palette (R02), get_palette/get_data, get_size (ggf. von
 * Bild id), check(id): Kennung pruefen (strncmp mit g_str_aif bzw. g_str_aif2).
 *
 * BCC -mlarge -3 -Ot -Og -Oe -c -Y -E..\bc\BIN\TASM.EXE OVL269.CPP -> 3950/3950 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h, Klasse Pic in s_class.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Schluessel -Oe (globale Registervergabe): ohne -Oe 468 Bytes Abweichung (check haelt id in DX, rle/packbits
 *   Zeichen in DL/CL, Zaehler in DX); mit -Oe sind register-Schluesselwoerter wirkungslos (Quelle ohne register).
 * - rle_fast ist Originalassembler (TASM-Kodierung 31 db / 88 d9 / 29 da, push ds/es/si/di im Rumpf) -> asm mit
 *   -E TASM und C-Marken; BCC ergaenzt push si/di im Prolog selbst.
 * - Ueberladene Elementfunktionen; Reihenfolge in der Datei = Originalreihenfolge (push cs/call near nach oben,
 *   far call nach unten). "return f(..);" -> toter Epilog je Funktion.
 * - "own_pal = own_data = 0" -> mov al,0 / Store +26 / Store +25; "tmp = rp = farmalloc(..)" -> +36 zuerst.
 * - Seek -((long)n * 3) -> movsx/imul/neg; Laenge (long)(n * 3) -> shl/add/movsx; "pal[i].r = pal[i].r & 0x3f"
 *   (Element 3 Bytes) statt &=; "buf_pos++ == 99"; "*(dst + x + k) = v"; "*dst++ = c" -> inc word.
 * - arc_open wird unsigned verglichen ("(unsigned short)arc_open(..) > 0"), file_open signed.
 * Originalfehler/Auffaelligkeiten: Pic(file) setzt das Archivobjekt arc nicht (die f-Ladefunktionen benutzen dann
 *   einen uninitialisierten Zeiger); read_data meldet Erfolg auch, wenn decode kein Ziel anlegen konnte; check
 *   liest den Pruefkopf ins Objekt selbst (ueberschreibt Kopf/Masse).
 */
#include <string.h>
#include <alloc.h>
#include <io.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:85e4..8664, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
unsigned short g_shade_segs[32] = {
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};
unsigned char g_fog_table[32] = "";
char g_unused_8644[24] = { 0 };   /* ds:8644, unbenutzt */

Pic::Pic(void far *f)
{
#if FIX_0074
	/* Fix #74: Archivobjekt wie im Standardkonstruktor setzen */
	file = f;
	arc = g_file_3;
#else
	file = f;
#endif
	own_pal = own_data = 0;
	stream = 0;
}

Pic::Pic()
{
	file = g_file_main;
	arc = g_file_3;
	own_pal = own_data = 0;
	stream = 0;
}

Pic::~Pic()
{
	if (own_pal)
		delete pal;
	if (own_data)
		farfree(data);
}

signed short Pic::load(signed short n)
{
	return load(n, 0, 0);
}

signed short Pic::load(signed short n, void far *dst)
{
	return load(n, dst, 0);
}

signed short Pic::load(signed short n, void far *dst, struct rgb far *p)
{
	return do_load(n, dst, p, 0);
}

signed short Pic::loadf(signed short n)
{
	return loadf(n, 0, 0);
}

signed short Pic::loadf(signed short n, void far *dst)
{
	return loadf(n, dst, 0);
}

signed short Pic::loadf(signed short n, void far *dst, struct rgb far *p)
{
	return do_load(n, dst, p, 1);
}

signed short Pic::sload(signed short n)
{
	stream = 1;
	return load(n, 0, 0);
}

signed short Pic::sload(signed short n, void far *dst)
{
	stream = 1;
	return load(n, dst, 0);
}

signed short Pic::sload(signed short n, void far *dst, struct rgb far *p)
{
	stream = 1;
	return do_load(n, dst, p, 0);
}

signed short Pic::sloadf(signed short n)
{
	stream = 1;
	return loadf(n, 0, 0);
}

signed short Pic::sloadf(signed short n, void far *dst)
{
	stream = 1;
	return loadf(n, dst, 0);
}

signed short Pic::sloadf(signed short n, void far *dst, struct rgb far *p)
{
	stream = 1;
	return do_load(n, dst, p, 1);
}

signed short Pic::do_load(signed short n, void far *dst, struct rgb far *p, signed char a)
{
	id = n;
	if (own_pal == 0) {
		pal = p;
	} else if (p != 0) {
		delete pal;
		own_pal = 0;
		pal = p;
	}
	from_arc = a;
	own_data = 0;
	if (read_header()) {
		if (pal == 0) {
			pal = (struct rgb far *)new char[ncolors * 3];
			own_pal = 1;
		}
		if (read_pal()) {
			if (read_data(dst)) {
				stream = 0;
				return 1;
			}
		}
	}
	stream = 0;
	return 0;
}

void Pic::set_palette()
{
	if (no_pal == 0)
		::set_palette(pal, first_color, ncolors);
}

struct rgb far *Pic::get_palette()
{
	return pal;
}

void Pic::get_size(signed short far *pw, signed short far *ph)
{
	*pw = w;
	*ph = h;
}

unsigned char far *Pic::get_data()
{
	return data;
}

void Pic::get_size(signed short n, signed short far *pw, signed short far *ph)
{
	signed short old;

	old = id;
	id = n;
	read_header();
	*pw = w;
	*ph = h;
	id = old;
}

signed short Pic::check(signed short n)
{
	if (!from_arc) {
		if (file_open(file, n) > 0) {
			file_read(file, this, (signed long)PIC_HDR_SIZE);
			file_close(file);
			if (strncmp("AIF", (char far *)this, 3) == 0)
				return 1;
		}
		return 0;
	} else {
		if ((unsigned short)arc_open(arc, n) > 0) {
			arc_read(arc, this, (signed long)PIC_HDR_SIZE);
			arc_close(arc);
			if (strncmp("AIF", (char far *)this, 3) == 0)
				return 1;
		}
		return 0;
	}
}

signed short Pic::read_header()
{
	if (!from_arc) {
		if (file_open(file, id) > 0) {
			file_read(file, this, (signed long)PIC_HDR_SIZE);
			file_close(file);
			return 1;
		}
		return 0;
	} else {
		if ((unsigned short)arc_open(arc, id) > 0) {
			arc_read(arc, this, (signed long)PIC_HDR_SIZE);
			arc_close(arc);
			return 1;
		}
		return 0;
	}
}

signed short Pic::read_pal()
{
	signed short ok = 0;

	if (no_pal == 0) {
		if (!from_arc) {
			if (file_open(file, id) > 0) {
				file_seek(file, -((signed long)ncolors * 3), SEEK_END);
				file_read(file, pal, (signed long)(ncolors * 3));
				file_close(file);
				ok = 1;
			}
		} else {
			if ((unsigned short)arc_open(arc, id) > 0) {
				arc_seek(arc, -((signed long)ncolors * 3), SEEK_END);
				arc_read(arc, pal, (signed long)(ncolors * 3));
				arc_close(arc);
				ok = 1;
			}
		}
		if (ok == 1) {
			signed short i;

			for (i = 0; i < ncolors; i++) {
				pal[i].r = pal[i].r & 0x3f;
				pal[i].g = pal[i].g & 0x3f;
				pal[i].b = pal[i].b & 0x3f;
			}
		}
	}
	return ok;
}

signed short Pic::read_data(void far *dst)
{
	if (!from_arc) {
		if (file_open(file, id) > 0) {
			size = file_length2(file) - PIC_HDR_SIZE - (signed long)ncolors * 3;
			file_seek(file, (signed long)PIC_HDR_SIZE, SEEK_SET);
			decode((unsigned char far *)dst);
			file_close(file);
			return 1;
		}
		return 0;
	} else {
		if ((unsigned short)arc_open(arc, id) > 0) {
			size = arc_size(arc) - PIC_HDR_SIZE - (signed long)ncolors * 3;
			arc_seek(arc, (signed long)PIC_HDR_SIZE, SEEK_SET);
			decode((unsigned char far *)dst);
			arc_close(arc);
			return 1;
		}
		return 0;
	}
}

unsigned char Pic::get_byte()
{
	if (!stream) {
		if (buf_pos++ == 99) {
			if (!from_arc)
				file_read(file, buf, 100L);
			else
				arc_read(arc, buf, 100L);
			buf_pos = 0;
		}
		return buf[buf_pos];
	} else {
		return *rp++;
	}
}

signed short Pic::decode(unsigned char far *dst)
{
	data = 0;
	if (dst == 0) {
		if ((dst = (unsigned char far *)farmalloc((signed long)w * h)) == 0)
			return 0;
		own_data = 1;
	}
	data = dst;
	buf_pos = 99;
	if (pack == PIC_PACK_RAW)
		unpack_raw(dst);
	else if (pack == PIC_PACK_RLE)
		unpack_rle(dst);
	else if (pack == PIC_PACK_PACKBITS)
		unpack_packbits(dst);
	else if (pack == PIC_PACK_PP20)
		unpack_pp20(dst);
	return 1;
}

void Pic::unpack_raw(unsigned char far *dst)
{
	if (!from_arc)
		file_read(file, dst, size);
	else
		arc_read(arc, dst, size);
}

void Pic::unpack_rle(unsigned char far *dst)
{
	if (!stream) {
		rle(dst);
		return;
	}
	tmp = rp = (unsigned char far *)farmalloc(size);
	if (!from_arc)
		file_read(file, tmp, size);
	else
		arc_read(arc, tmp, size);
	rle_fast(dst, tmp, w, h);
	farfree(tmp);
}

void Pic::rle(unsigned char far *dst)
{
	signed short y;
	signed short x;
	unsigned char n;

	for (y = 0; y < h; y++, dst += w) {
		for (x = 0; x < w; ) {
			unsigned char c;

			c = get_byte();
			if (c == PIC_RLE_MARK) {
				unsigned char v;
				signed short k;

				n = get_byte();
				v = get_byte();
				for (k = 0; k < n; k++)
					*(dst + x + k) = v;
				x += n;
			} else {
				*(dst + x) = c;
				x++;
			}
		}
	}
}

void Pic::rle_fast(unsigned char far *dst, unsigned char far *src, signed short ww, signed short hh)
{
	asm push ds
	asm push es
	asm push si
	asm push di
	asm mov dx, hh
l1:
	asm les di, src
	asm lds si, dst
	asm push dx
	asm mov dx, ww
l2:
	asm mov al, es:[di]
	asm inc di
	asm cmp al, 7fh
	asm jnz l4
	asm xor bx, bx
	asm xor cx, cx
	asm mov bl, es:[di]
	asm inc di
	asm mov cl, bl
	asm mov al, es:[di]
	asm inc di
l3:
	asm mov [si], al
	asm inc si
	asm loop l3
	asm sub dx, bx
	asm jmp short l5
l4:
	asm mov [si], al
	asm inc si
	asm dec dx
l5:
	asm cmp dx, 0
	asm jg l2
	asm pop dx
	asm mov word ptr src, di
	asm mov word ptr src + 2, es
	asm les di, dst
	asm add di, ww
	asm mov word ptr dst, di
	asm mov word ptr dst + 2, es
	asm dec dx
	asm jnz l1
	asm pop di
	asm pop si
	asm pop es
	asm pop ds
}

void Pic::unpack_packbits(unsigned char far *dst)
{
	if (!stream) {
		packbits(dst);
		return;
	}
	tmp = rp = (unsigned char far *)farmalloc(size);
	if (!from_arc)
		file_read(file, tmp, size);
	else
		arc_read(arc, tmp, size);
	packbits(dst);
	farfree(tmp);
}

void Pic::packbits(unsigned char far *dst)
{
	signed long i;
	unsigned char n;

	for (i = 0; i < size; i++) {
		unsigned char c;

		c = get_byte();
		if (c & 0x80) {			/* 1 - c Wiederholungen des naechsten Bytes */
			n = 1 - c;
			c = get_byte();
			i++;
			do {
				*dst++ = c;
			} while ((n += -1) > 0);
		} else {			/* c + 1 Bytes kopieren */
			n = c + 1;
			do {
				*dst++ = get_byte();
				i++;
			} while ((n += -1) > 0);
		}
	}
}

void Pic::unpack_pp20(unsigned char far *dst)
{
	tmp = (unsigned char far *)farmalloc(size);
	if (!from_arc)
		file_read(file, tmp, size);
	else
		arc_read(arc, tmp, size);
	::unpack_pp20(dst, tmp, size);
	farfree(tmp);
}
