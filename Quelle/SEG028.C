/* SEG028 - Byte-Tausch (Motorola-/Amiga-Reihenfolge, z. B. Laengenangaben in PP20-Daten) (seg028 in
 * SCHWEIF.EXE)
 * BCC -mlarge -1 -O -Ot -Z -c -Y SEG028.C   -> 146/146 Bytes GLEICH
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * - swap_u16 ist reiner Inline-Assembler, Ergebnis in ax (kein return).
 * - swap_u32 tauscht die beiden Woerter und in jedem die Bytes; die Rueckgabe ist signed (R41 nvf_process
 *   schiebt das Ergebnis arithmetisch).
 * - swap_u32_unused tauscht nur die Woerter (nirgends aufgerufen).
 */
#include "schweif.h"

unsigned short swap_u16(unsigned short value)
{
	asm push bx
	asm mov bx, value
	asm mov ax, 0
	asm mov al, bh
	asm mov ah, bl
	asm pop bx
}

signed long swap_u32(unsigned long value)
{
	register signed short low;
	signed short words[2];
	signed long far *both = (signed long far*)&words[0];

	*both = value;
	low = words[0];
	words[0] = swap_u16(words[1]);
	words[1] = swap_u16(low);
	return *both;
}

unsigned long swap_u32_unused(unsigned long value)
{
	signed short words[2];
	signed short low;
	signed long far *both = (signed long far*)&words[0];

	low = (signed short)(*both = value);
	words[0] = words[1];
	words[1] = low;
	return *both;
}
