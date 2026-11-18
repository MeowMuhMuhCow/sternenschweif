/* SEG040 - Lesen/Schreiben grosser Bloecke (ueber 64 KB, huge-Zeiger) in Stuecken zu 64000 Bytes
 * (seg040 in SCHWEIF.EXE)
 * BCC -mlarge -3 -Oe -Ot -Og -c -Y SEG040.C   -> 295/295 Bytes GLEICH
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * - Rueckgabe: Summe der uebertragenen Bytes. read_big endet bei Dateiende (0) oder Fehler (0xffff = -1),
 *   write_big zusaetzlich, wenn alles geschrieben ist (read_big liest dann noch einmal 0 Bytes).
 */
#include <io.h>
#include "schweif.h"

#define CHUNK_SIZE	0xfa00		/* 64000 Bytes je _read/_write */

signed long read_big(signed short fd, void far *buf, signed long len)
{
	register unsigned short n;
	signed long total = 0;
	unsigned char huge *p = buf;

	do {
		n = (len > (signed long)CHUNK_SIZE) ? CHUNK_SIZE : (unsigned short)len;
		n = _read(fd, p, n);
		p += n;
		total += n;
		len -= n;
	} while (n != 0 && n != 0xffff);

	return total;
}

signed long write_big(signed short fd, void far *buf, signed long len)
{
	register unsigned short n;
	signed long total = 0;
	unsigned char huge *p = buf;

	do {
		n = (len > (signed long)CHUNK_SIZE) ? CHUNK_SIZE : (unsigned short)len;
		n = _write(fd, p, n);
		p += n;
		total += n;
		len -= n;
	} while (len > 0 && n != 0 && n != 0xffff);

	return total;
}
