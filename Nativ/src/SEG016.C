/* SEG016 - Entpacken von Bilddaten (NVF-Dateien R41): Verteiler nach Packart, RLE mit Marke 7Fh, RLE mit
 * Laengenbyte; PP20 geht an R29 unpack_pp20 (seg016 in SCHWEIF.EXE)
 * BCC -mlarge -3 -Oi -Oe -Ot -c -Y -E..\bc\BIN\TASM.EXE SEG016.C   -> 453/453 Bytes GLEICH
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * - Der erste Parameter (NVF-Objekt) wird nirgends benutzt; decomp_rle ist Inline-Assembler (braucht TASM).
 */
#include <mem.h>
#include "schweif.h"

void decomp(void far *nvf, signed short mode, signed long len, signed short width, signed short height,
		void far *dst, void far *src)
{
	switch (mode) {
	case PACK_RLE:
		decomp_rle(nvf, dst, src, width, height);
		return;
	case PACK_RLE2:
		decomp_rle2(nvf, dst, src, len);
		return;
	case PACK_PP20:
		unpack_pp20(dst, src, len);
		return;
	default:
		memcpy(dst, src, (unsigned short)len);
		return;
	}
}

/* RLE: 7Fh, Anzahl, Wert; sonst Wert direkt. Zeile fuer Zeile, Ziel mit Zeilenbreite width */
void decomp_rle(void far *nvf, unsigned char far *dst, unsigned char far *src,
		signed short width, signed short height)
{
    (void)nvf;
    signed short y, x;
    const unsigned char *s = (const unsigned char *)src;
    for (y = 0; y < height; y++) {
        unsigned char *d = (unsigned char *)dst;
        x = width;
        while (x > 0) {
            unsigned char al = *s++;
            if (al == 0x7f) {
                unsigned char cnt = *s++;
                unsigned char val = *s++;
                for (unsigned char k = 0; k < cnt; k++)
                    *d++ = val;
                x -= cnt;
            } else {
                *d++ = al;
                x--;
            }
        }
        dst = (unsigned char *)dst + width;
    }
}

/* RLE mit Laengenbyte: Bit 7 gesetzt = Wiederholung (1-n mal), sonst n+1 Bytes direkt */
void decomp_rle2(void far *nvf, unsigned char huge *dst, unsigned char huge *src, signed long len)
{
	signed long i;
	unsigned char val;
	unsigned char count;

	for (i = 0; i < len; i++) {
		val = *src;
		src++;
		if (val & 0x80) {
			count = 1 - val;
			val = *src;
			src++;
			i++;
			do {
				*dst = val;
				dst++;
			} while (--count > 0);
		} else {
			count = val + 1;
			do {
				*dst = *src;
				src++;
				dst++;
				i++;
			} while (--count > 0);
		}
	}
}
