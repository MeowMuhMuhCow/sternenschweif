/*
 *	Rekonstruktion DSA 2 Sternenschweif C1.02 de, Segment seg037 (R37)
 *	Zufall und Wuerfel. Grundlage: Bright Eyes seg007 (DSA 1) und c102de_seg037.
 *	Compiler: Borland C++ 3.1, BCC -mlarge -O -3 -Oe -Ot -Ol -Z -c -Y SEG037.C
 *
 *	Stand 18.10.2026: Segment 191/191 Bytes, KOMPLETT GLEICH.
 *	  (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 *	  random_schweif: copy1 = r als eigener Komma-Ausdruck, dazu -Z
 *	  (Registerinhalte merken) -> mov [bp-2],bx wie im Original.
 *	Zufallsgenerator: g_random_seed wird mit g_random_seed2 (Zaehler der Timer-ISR, BE RANDOM_SCHICK_SEED2)
 *	verknuepft und rotiert; copy1/copy2 sind tote Kopien, die das Original ebenfalls speichert.
 */
#include <stdlib.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:869e..86a0, dsana) - initialisierte Daten in Adressreihenfolge */
unsigned short g_random_seed = 0x327b;

/* Zufallszahl lo..hi */
signed short random_interval(signed short lo, signed short hi)
{
	return lo + random_schweif(hi - lo + 1) - 1;
}

/* Zufallszahl 1..range (ein Wuerfel mit range Seiten), 0 bei range == 0 */
signed short random_schweif(signed short range)
{
	signed short r;
	signed short copy1, copy2;

	if (range) {
		g_random_seed = abs((r = _rotl(r = ((r = _rotl(r = g_random_seed ^ g_random_seed2, 2)) + g_random_seed2) ^ g_random_seed, 3), copy1 = r, r)) + 1;
		copy2 = r;
		r = abs(r) % range + 1;
	} else {
		r = 0;
	}

	return r;
}

/* dice Wuerfel mit sides Seiten + bonus (DSA-Schreibweise dice W sides + bonus), mindestens 0 */
signed short dice_roll(signed short dice, signed short sides, signed short bonus)
{
	signed short sum = 0;
	signed short i;

	i = 0;
	while (i < dice) {
		sum += random_schweif(sides);
		i++;
	}

	sum += bonus;
	if (sum < 0)
		sum = 0;

	return sum;
}

/* kleinster und groesster Wert von dice W sides + bonus */
void calc_damage_range(signed short dice, signed short sides, signed short bonus,
		signed short far *min, signed short far *max)
{
	*min = dice + bonus;
	*max = dice * sides + bonus;
}
