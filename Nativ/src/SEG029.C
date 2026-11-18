/* SEG029 - PowerPacker-Aufruf: PP20-Daten an den Entpacker R33 ppDecrunch uebergeben (seg029 in SCHWEIF.EXE)
 * BCC -mlarge -1 -O -Ot -Z -c -Y SEG029.C   -> 41/41 Bytes GLEICH
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * - Die gepackten Daten beginnen nach dem 4-Byte-Kopf (Kennung "PP20" bzw. Laengenwort; Offset + 4, Segment
 *   unveraendert). size ist die Laenge der gepackten Daten samt Kopf (BE DSA2 decomp_pp20: plen).
 */
#include <dos.h>
#include "schweif.h"

void unpack_pp20(void far *dst, void far *src, unsigned long size)
{
	ppDecrunch(src, dst, FP_OFF(src) + 4, FP_SEG(src), size);
}
