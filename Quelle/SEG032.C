/* SEG032 - Suche in Listen mit Endmarke -1 (seg032 in SCHWEIF.EXE)
 * BCC -mlarge -Oe -c -Y SEG032.C   -> 127/127 Bytes GLEICH
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * - Rueckgabe: Index des ersten Eintrags == val, sonst -1. Die Liste ist im Prototyp ein void far * (Aufrufer
 *   uebergeben Tabellenstrukturen, z. B. struct item_list7); der Zugriff geht ueber die Cast-Makros WORDS bzw.
 *   BYTES, deren "++" die Borland-Erweiterung "Cast als lvalue" nutzt (gleicher Code wie ein typisierter
 *   Zeiger).
 * - is_in_byte_array2 ist eine wortgleiche Kopie von is_in_byte_array.
 */
#include "schweif.h"

#define WORDS	((signed short far *)list)
#define BYTES	((signed char far *)list)

signed short is_in_word_array(signed short val, void far *list)
{
	signed short i = 0;

	while (*WORDS != -1) {
		if (*WORDS++ == val)
			return i;
		i++;
	}
	return -1;
}

signed short is_in_byte_array(signed char val, void far *list)
{
	signed short i = 0;

	while (*BYTES != -1) {
		if (*BYTES++ == val)
			return i;
		i++;
	}
	return -1;
}

signed short is_in_byte_array2(signed char val, void far *list)
{
	signed short i = 0;

	while (*BYTES != -1) {
		if (*BYTES++ == val)
			return i;
		i++;
	}
	return -1;
}
