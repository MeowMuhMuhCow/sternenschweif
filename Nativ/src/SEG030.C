/* SEG030 - Gross-/Kleinschreibung mit deutschen Umlauten (Codepage 437) (seg030 in SCHWEIF.EXE)
 * BCC -mlarge -O -Oe -Ot -Og -c -Y SEG030.C   -> 206/206 Bytes GLEICH
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * - A..Z/a..z ueber ctype (isalpha/isupper/islower), dazu Ae/Oe/Ue; ss bleibt unveraendert.
 */
#include <ctype.h>
#include "schweif.h"

unsigned char char_tolower(unsigned char c)
{
	if (isalpha(c) && isupper(c))
		c += 'a' - 'A';
	else if (c == CH_OE_UPPER)
		c = CH_OE_LOWER;
	else if (c == CH_AE_UPPER)
		c = CH_AE_LOWER;
	else if (c == CH_UE_UPPER)
		c = CH_UE_LOWER;
	return c;
}

unsigned char char_toupper(unsigned char c)
{
	if (isalpha(c) && islower(c))
		c -= 'a' - 'A';
	else if (c == CH_OE_LOWER)
		c = CH_OE_UPPER;
	else if (c == CH_AE_LOWER)
		c = CH_AE_UPPER;
	else if (c == CH_UE_LOWER)
		c = CH_UE_UPPER;
	return c;
}

void str_tolower(char far *str)
{
	register unsigned char c;

	while ((c = *str) != 0) {
		*str = char_tolower(c);
		str++;
	}
}

void str_toupper(char far *str)
{
	register unsigned char c;

	while ((c = *str) != 0) {
		*str = char_toupper(c);
		str++;
	}
}
