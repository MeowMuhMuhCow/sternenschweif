/* SEG041 - NVF-Bilddateien: Bild Nummer nr aus einer NVF-Datei im Speicher auspacken (seg041 in SCHWEIF.EXE)
 * Vorlage fuer den Aufbau: Bright Eyes c102de_seg041.cpp (process_nvf, decomp_rle); gleiches Verfahren direkt
 * aus der Datei: Klasse Nvf (O098).
 * BCC -mlarge -3 -Oe -Ot -Og -c -Y SEG041.C   -> 1870/1870 Bytes GLEICH
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * - Objekt struct nvf_desc (0x19 Bytes), im Prototyp void far *, Zugriff ueber den Cast-Makro NVF.
 *   nvf_new/nvf_delete sind Konstruktor/Destruktor (Objekt 0 -> anlegen; flags & 1 -> Objekt freigeben).
 * - NVF-Datei: Typbyte (NVF_FILE_*, Bit 7 NVF_FLAG_LEN), Bildzahl (Wort), dann je Typ:
 *     0    Breite, Hoehe, Bilder ungepackt hintereinander
 *     1    je Bild Breite/Hoehe (4 Bytes), Bilder ungepackt
 *     2/4  Breite, Hoehe, je Bild Laenge (long), Bilder gepackt
 *     3/5  je Bild Breite/Hoehe/Laenge (8 Bytes), Bilder gepackt
 * - Ausgabe nach nvf_desc.type: NVF_OUT_RAW (Typ 0/1 kopieren, sonst PP20 entpacken; Rueckgabe Bildgroesse,
 *   bei NVF_FLAG_LEN aus dem Laengenfeld am Ende der PP20-Daten), NVF_OUT_RLE* (RLE mit Marke 7Fh zeilenweise,
 *   gespiegelt ueber den Zeilenpuffer), sonst unveraendert kopieren (Rueckgabe gepackte Laenge).
 * - Wie DSA1: Typ 3/5 nimmt die Laenge von Eintrag n nach der Schleife (== nr). Originalfehler: in den
 *   RLE-Zweigen bleibt die Rueckgabe size uninitialisiert (wie ORIGINALFEHLER Nr. 9 bei O098).
 */
#include <mem.h>
#include "schweif.h"

#define NVF	((struct nvf_desc far *)nvf)

void far *nvf_new(void far *nvf)
{
	if (nvf || (nvf = mem_alloc_w(sizeof(struct nvf_desc)))) {
		NVF->buf = mem_alloc_w(NVF_ROW_BUF);
	}
	return nvf;
}

void nvf_delete(void far *nvf, signed short flags_arg)
{
	register signed short flags = flags_arg;

	if (nvf) {
		mem_free_v(NVF->buf);
		if (flags & 1)
			mem_free_v(nvf);
	}
}

void nvf_set(void far *nvf, unsigned char far *dst, unsigned char huge *src, signed short nr,
		signed char type, unsigned short far *width, unsigned short far *height, signed short stride)
{
	NVF->dst = dst;
	NVF->src = src;
	NVF->nr = nr;
	NVF->type = type;
	NVF->width = width;
	NVF->height = height;
	NVF->stride = stride;
}

signed long nvf_process(void far *nvf)
{
	unsigned long data_off;
	signed short has_len;
	unsigned short height;
	unsigned long packed_size;
	signed long size;
	signed char file_type;
	unsigned char huge *tail;
	unsigned char far *src;
	unsigned char far *dst;
	unsigned char far *row;
	signed char run;
	unsigned char run_val;
	register signed short n;	/* erst Zahl der Bilder, dann Laufvariable */
	register unsigned short width;
	unsigned short x;
	signed short j;
	unsigned char c;

	file_type = *NVF->src;
	has_len = file_type & NVF_FLAG_LEN;
	file_type &= NVF_TYPE_MASK;
	n = *(signed short huge*)(NVF->src + 1);

	if (NVF->nr < 0)
		NVF->nr = 0;
	if (NVF->nr > n - 1)
		NVF->nr = n - 1;

	switch (file_type) {
	case NVF_FILE_RAW:
		width = *(unsigned short huge*)(NVF->src + 3);
		height = *(unsigned short huge*)(NVF->src + 5);
		packed_size = (unsigned short)(width * height);
		src = NVF->src + NVF->nr * packed_size + 7;
		break;
	case NVF_FILE_RAW_SIZES:
		data_off = n * 4 + 3L;
		for (n = 0; n < NVF->nr; n++) {
			width = *(unsigned short huge*)(NVF->src + n * 4 + 3);
			height = *(unsigned short huge*)(NVF->src + n * 4 + 5);
			data_off += (unsigned short)(width * height);
		}
		width = *(unsigned short huge*)(NVF->src + NVF->nr * 4 + 3);
		height = *(unsigned short huge*)(NVF->src + NVF->nr * 4 + 5);
		packed_size = (unsigned short)(width * height);
		src = NVF->src + data_off;
		break;
	case NVF_FILE_PACKED:
	case NVF_FILE_PACKED2:
		width = *(unsigned short huge*)(NVF->src + 3);
		height = *(unsigned short huge*)(NVF->src + 5);
		data_off = n * 4 + 7L;
		for (n = 0; n < NVF->nr; n++)
			data_off += *(unsigned long huge*)(NVF->src + n * 4 + 7);
		packed_size = *(unsigned long huge*)(NVF->src + NVF->nr * 4 + 7);
		src = NVF->src + data_off;
		break;
	case NVF_FILE_PACKED_SIZES:
	case NVF_FILE_PACKED_SIZES2:
		data_off = n * 8 + 3L;
		for (n = 0; n < NVF->nr; n++)
			data_off += *(unsigned long huge*)(NVF->src + n * 8 + 7);
		width = *(unsigned short huge*)(NVF->src + NVF->nr * 8 + 3);
		height = *(unsigned short huge*)(NVF->src + NVF->nr * 8 + 5);
		packed_size = *(unsigned long huge*)(NVF->src + n * 8 + 7);
		src = NVF->src + data_off;
		break;
	}

	if (!NVF->type) {
		/* NVF_OUT_RAW */
		if (has_len) {
			/* PP20: erstes Langwort = Laenge der gepackten Daten (BE), in den letzten 4 Bytes steht
			 * (Motorola-Reihenfolge) die entpackte Laenge in den oberen 24 Bit */
			size = *(signed long far*)src;
			tail = src;
			tail += size - 4;
			size = *(signed long huge*)tail;
			size = swap_u32(size) >> 8;
		} else {
			size = (unsigned short)(width * height);
		}
		if (file_type != NVF_FILE_RAW && file_type != NVF_FILE_RAW_SIZES)
			unpack_pp20(NVF->dst, src, packed_size);
		else
			memcpy(NVF->dst, src, (unsigned short)packed_size);
	} else if (NVF->type >= NVF_OUT_RLE && NVF->type <= NVF_OUT_RLE_MIRROR_DENSE) {
		dst = NVF->dst;
		for (n = 0; n < height; n++) {
			/* eine Zeile auspacken (gespiegelt: erst in den Zeilenpuffer) */
			if (NVF->type == NVF_OUT_RLE_MIRROR || NVF->type == NVF_OUT_RLE_MIRROR_DENSE)
				row = NVF->buf;
			else
				row = dst;
			for (x = 0; x < width; ) {
				if ((c = *src++) == NVF_RLE_MARK) {
					run = *src;
					src++;
					run_val = *src;
					src++;
					for (j = 0; j < run; j++)
						(row + x)[j] = run_val;
					x += run;
				} else {
					row[x] = c;
					x++;
				}
			}
			/* gespiegelt: Zeilenpuffer rueckwaerts ins Ziel */
			if (NVF->type == NVF_OUT_RLE_MIRROR || NVF->type == NVF_OUT_RLE_MIRROR_DENSE) {
				row += width - 1;
				for (j = 0; j < width; j++) {
					*dst = *row;
					row--;
					dst++;
				}
			}
			/* naechste Zielzeile */
			if (NVF->type == NVF_OUT_RLE)
				dst += NVF->stride;
			else if (NVF->type == NVF_OUT_RLE_MIRROR)
				dst += NVF->stride - width;
			else if (NVF->type == NVF_OUT_RLE_DENSE)
				dst += width;
		}
	} else {
		memmove(NVF->dst, src, (unsigned short)packed_size);
		size = packed_size;
	}

	*NVF->width = width;
	*NVF->height = height;
	return size;
}

signed long nvf_unpack(void far *nvf, void far *dst, void far *src, signed short nr,
		signed char type, void far *width, void far *height, signed short stride)
{
	nvf_set(nvf, dst, src, nr, type, width, height, stride);
	return nvf_process(nvf);
}
