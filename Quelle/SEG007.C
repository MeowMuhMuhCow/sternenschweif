/* SEG007 - Speicherverwaltung mit Waechterwoertern "GUYS"/"GUYE" und Belegungstabelle (seg007 in SCHWEIF.EXE)
 * mem_alloc(size): farcalloc(size + 8), Waechter "GUYS" vor und "GUYE" hinter dem Block, Eintrag in
 * g_mem_blocks[1000] (struct mem_block: Start, Ende; Platz ueber mem_free_slot), g_mem_min_free = kleinster
 * gesehener farcoreleft(); Fehlschlag -> guru_meditation (nur bei g_mem_debug). mem_free(p): Waechter pruefen,
 * Block mit 0xcc fuellen, freigeben, liefert NULL; ohne g_mem_debug nur Eintrag loeschen und freigeben.
 * mem_check: alle Waechter pruefen (g_mem_checking verhindert Rekursion). mem_alloc_w/mem_alloc_l/mem_free_v:
 * operator new/delete (Groesse 0 -> 1).
 *
 * BCC -mlarge -3 -Ot -c -Y SEG007.C   -> 1148/1148 Bytes GLEICH
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * - mem_free hat wie der Prototyp in s_proto.h einen void-far-Parameter; "p -= 4" als Offsetrechnung
 *   ((unsigned short *)&p)[0] -= 4 (sub word [bp+6],4 wie mit unsigned-char-Parameter; die Zuweisung
 *   p = (unsigned char far *)p - 4 rechnet ueber Register).
 */
/* Phase B: BCC (C-Modus) legt das BSS in umgekehrter Reihenfolge der ersten Deklaration an - Reihenfolge hier vor schweif.h */
extern signed short g_mem_checking;
#include <alloc.h>
#include <stdio.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:5158..5284, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
/* Phase B: eigenes BSS (ds:c7fe..c800) in Adressreihenfolge (Reihenfolge festgelegt durch die extern-Zeilen vor den #include) */
signed short g_mem_checking;

#define MEM_MAGIC_START	0x53595547UL	/* "GUYS" */
#define MEM_MAGIC_END	0x45595547UL	/* "GUYE" */

void far *mem_alloc(unsigned long size)
{
	unsigned char far *block;
	unsigned char far *end;
	signed long core_left;
	register signed short slot;

	if (!g_mem_blocks)
		g_mem_blocks = farcalloc(NR_MEM_BLOCKS, sizeof(struct mem_block));
	mem_check();
	block = farcalloc(size + 8, 1);
	core_left = farcoreleft();
	if (g_mem_min_free > core_left)
		g_mem_min_free = core_left;

	if (block) {
		end = block;
		*(unsigned long far*)end = MEM_MAGIC_START;
		end = (unsigned char huge*)block + size + 4;
		*(unsigned long far*)end = MEM_MAGIC_END;
		slot = mem_free_slot();
		g_mem_blocks[slot].start = block;
		g_mem_blocks[slot].end = end;
		return block + 4;
	} else {
		if (g_mem_debug) {
			sprintf(g_text_output_buf, "Memory Allocation Failure (%ld Bytes / %ld left)!\r\n", size, farcoreleft());
			guru_meditation(g_text_output_buf, 0);
		}
	}
	return 0;
}

void far *mem_free(void far *p)
{
	unsigned char far *start;
	unsigned char far *end;
	struct mem_block far *mb;
	signed long core_left;
	register signed short i;
	register signed short found;

	mem_check();
	if (g_mem_debug) {
		if (p) {
			unsigned long k;
			unsigned long len;
			unsigned char huge *q;

			((unsigned short *)&p)[0] -= 4;	/* p -= 4 (nur Offset) */
			i = found = 0;
			for (mb = g_mem_blocks; i < NR_MEM_BLOCKS; i++, mb++) {
				if (mb->start == p) {
					found = 1;
					break;
				}
			}
			if (found) {
				start = mb->start;
				end = mb->end;
				if (*(unsigned long far*)start != MEM_MAGIC_START || *(unsigned long far*)end != MEM_MAGIC_END) {
					guru_meditation("Someone has destroyed the magic values for this block!\r\nSystem's Integrity endangered!", p);
				} else {
					mb->start = 0;
					len = (unsigned char huge*)end - (unsigned char huge*)start;
					q = start;
					len += 4;
					if (len & 1) {
						*q = 0xcc;
						((unsigned short*)&q)[0]++;
					}
					if (len & 2) {
						*(unsigned short huge*)q = 0xcccc;
						((unsigned short*)&q)[0] += 2;
					}
					len /= 4;
					if (len) {
						for (k = 0; k < len; k++) {
							*(unsigned long huge*)q = 0xccccccccUL;
							q += 4;
						}
					}
					farfree(p);
					p = 0;
				}
			} else {
				guru_meditation("Error freeing up allocated Memory!\r\n", p);
			}
		}
	} else {
		if (p) {
			unsigned char far *block;

			block = (unsigned char far *)p - 4;
			i = found = 0;
			for (mb = g_mem_blocks; i < NR_MEM_BLOCKS; i++, mb++) {
				if (mb->start == block) {
					found = 1;
					break;
				}
			}
			if (found) {
				mb->start = 0;
				p = block;
			}
		}
		farfree(p);
		p = 0;
	}
	core_left = farcoreleft();
	if (g_mem_min_free > core_left)
		g_mem_min_free = core_left;
	return p;
}

signed short mem_free_slot(void)
{
	struct mem_block far *mb;
	register signed short i;

	for (i = 0, mb = g_mem_blocks; i < NR_MEM_BLOCKS; i++, mb++) {
		if (!mb->start)
			return i;
	}
	guru_meditation("Running out of Allocation Entries!\r\n", 0);
	return -1;
}

void mem_check(void)
{
	struct mem_block far *mb;
	unsigned char far *start;
	unsigned char far *end;
	register signed short i;

	if (!g_mem_checking) {
		g_mem_checking = 1;
		if (g_mem_debug) {
			for (i = 0, mb = g_mem_blocks; i < NR_MEM_BLOCKS; i++, mb++) {
				if (mb->start) {
					start = mb->start;
					end = mb->end;
					if (*(unsigned long far*)start != MEM_MAGIC_START || *(unsigned long far*)end != MEM_MAGIC_END)
						guru_meditation("Someone has destroyed the magic values for this block!\r\nSystem's Integrity endangered!", start);
				}
			}
		}
		g_mem_checking = 0;
	}
}

void far *mem_alloc_w(unsigned short n_arg)
{
	register unsigned short n = n_arg;

	n = n ? n : 1;
	return mem_alloc(n);
}

void far *mem_alloc_l(unsigned long n)
{
	n = n ? n : 1;
	return mem_alloc(n);
}

void mem_free_v(void far *p)
{
	mem_free(p);
}
