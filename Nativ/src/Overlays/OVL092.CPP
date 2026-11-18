/* OVL092 - Overlay O092: Huellen um ein lokales ItemUse (O047, 0x22 Bytes; Konstruktoren ItemUse(user, target,
 * slot, id, verbose) bzw. ItemUse(user, slot, id, slot2, id2, verbose), Methoden use/combine, Destruktor) bzw.
 * item_consume (O253) fuer den Inventarplatz slot (struct inv_slot, hero +0x1a8): use_item, combine_items,
 * hero_consume_item; dazu Fach i des Schluesselbunds lesen/setzen im long-Bitfeld inv[pos].key_bits (+0x0b)
 * per _lrotl(1, i) (box_has/box_set; O048 use_keyring, O077).
 *
 * BCC -mlarge -3 -Ot -c -Y OVL092.CPP   -> 479/479 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h, Welle N20: 19.10.2026 GLEICH, LINKGLEICH)
 * Schluessel: "if (mask & bits)" (nicht bits & mask) - BCC laedt den rechten Operanden nach eax und testet gegen
 *   den linken. Zwei Konstruktoren derselben Klasse (Ueberladung) -> cobjfix ordnet ueber Fixups zu.
 */
#include <stdlib.h>
#include "schweif.h"

void use_item(struct hero far *hero, signed short slot)
{
	ItemUse iu(hero, hero, slot, hero->inv[slot].item_id, 1);

	iu.use();
}

void combine_items(struct hero far *hero, signed short slot, signed short slot2)
{
	ItemUse iu(hero, slot, hero->inv[slot].item_id, slot2, hero->inv[slot2].item_id, 1);

	iu.combine();
}

void hero_consume_item(struct hero far *user, struct hero far *owner, signed short slot)
{
	ItemUse iu(user, owner, slot, owner->inv[slot].item_id, 1);

	item_consume(&iu);
}

signed short box_has(struct hero far *hero, signed short pos, signed short i)
{
	signed short ret = 0;
	unsigned long far *p;
	unsigned long bits;
	unsigned long mask;

	p = &hero->inv[pos].key_bits;
	bits = *p;
	mask = _lrotl(1, i);
	if (mask & bits)
		ret = 1;
	return ret;
}

void box_set(struct hero far *hero, signed short pos, signed short i, signed short on)
{
	unsigned long far *p;
	unsigned long bits;
	unsigned long mask;

	p = &hero->inv[pos].key_bits;
	bits = *p;
	mask = _lrotl(1, i);
	if (on == 0)
		bits &= ~mask;
	else
		bits |= mask;
	*p = bits;
}
