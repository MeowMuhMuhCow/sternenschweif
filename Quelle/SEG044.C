/* SEG044 - Palette schrittweise angleichen: jede Farbkomponente von cur um 1 in Richtung target (ein Schritt
 * beim Ein-/Ausblenden, O034/O039) (seg044 in SCHWEIF.EXE)
 * BCC -mlarge -3 -Ot -Ol -Ov -Z -c -Y SEG044.C   -> 302/302 Bytes GLEICH
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * - Paletten wie in s_proto.h als void far *, Zugriff als signed char ueber die Cast-Makros CUR/TARGET
 *   (3 Bytes je Farbe, Komponenten zuerst auf 6 Bit maskiert).
 */
#include "schweif.h"

#define CUR	((signed char far *)cur)
#define TARGET	((signed char far *)target)

void pal_fade(void far *cur, void far *target, signed short nr_colors)
{
	signed short i;

	for (i = 0; i < nr_colors; i++) {

		TARGET[3 * i] &= DAC_MAX;
		TARGET[3 * i + 1] &= DAC_MAX;
		TARGET[3 * i + 2] &= DAC_MAX;
		CUR[3 * i] &= DAC_MAX;
		CUR[3 * i + 1] &= DAC_MAX;
		CUR[3 * i + 2] &= DAC_MAX;

		if ((TARGET[3 * i] < CUR[3 * i]) && (CUR[3 * i] > 0)) {
			CUR[3 * i]--;
		} else if ((TARGET[3 * i] > CUR[3 * i]) && (CUR[3 * i] < DAC_MAX)) {
			CUR[3 * i]++;
		}

		if ((TARGET[3 * i + 1] < CUR[3 * i + 1]) && (CUR[3 * i + 1] > 0)) {
			CUR[3 * i + 1]--;
		} else if ((TARGET[3 * i + 1] > CUR[3 * i + 1]) && (CUR[3 * i + 1] < DAC_MAX)) {
			CUR[3 * i + 1]++;
		}

		if ((TARGET[3 * i + 2] < CUR[3 * i + 2]) && (CUR[3 * i + 2] > 0)) {
			CUR[3 * i + 2]--;
		} else if ((TARGET[3 * i + 2] > CUR[3 * i + 2]) && (CUR[3 * i + 2] < DAC_MAX)) {
			CUR[3 * i + 2]++;
		}
	}
}
