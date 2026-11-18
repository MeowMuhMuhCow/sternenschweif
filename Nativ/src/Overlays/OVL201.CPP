/* OVL201 - Overlay O201 (Segment 0x2365 in SCHWEIF.EXE): weitere Methoden der Gruppenklasse struct group (siehe O200;
 * Heldenzeiger heroes[7], Schleifen "for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i])").
 * has_hero_named(name): Held mit diesem Namen (strcmp, -Oi) in der Gruppe? has_hero(x): Held x in der Gruppe?
 * count_check2b: Anzahl mit check_hero2. count_alive: Anzahl lebender Helden (!dead). hero_pos(x): Platz von x (-1).
 * hero_with_item(id, pos): Gegenstand id bei einem Helden - pos -1: group_find_item(this, id), pos >= 0: Inventar-
 * platz pos, -2/-3: Plaetze 0..14 bzw. 15..30 -> Platz des Helden. all_have_item(id, pos): haben ALLE Helden mit
 * check_hero3 den Gegenstand (get_item_pos bzw. Platz pos)? 0 = ja, -1 = nein. find_food/find_drink(first, &hero_no,
 * &slot, &id): hero_find_food bzw. hero_find_drink zuerst fuer first, dann fuer die anderen Helden (nicht den
 * Helden auf Platz g_hero_excluded, per get_hero_index), Treffer -> *hero_no = get_hero_index(Held), danach
 * g_hero_excluded = -1. add_effect(time, type, index, value): add_effect(h, get_free_effect(h), ...) fuer alle
 * Lebenden. same_place(o): steht Gruppe o am selben Ort - unterwegs (on_route) ueber Strecke route_from/route_to und
 * route_pos (+-300, Gegenrichtung route_len - route_pos), sonst x/y/dir/f20/f22/f24 gleich. swap_slots(a, b):
 * Plaetze tauschen (action_id = 100, fill_saved_values). portrait_dim(pix): 1024 Bytes Bild umfaerben (Farbtabelle
 * per if-Kette; Portraets inaktiver Gruppen). draw_hero_bar(slot, active): Heldenleiste zeichnen - Namensfeld (Bmp
 * 0x2a x 8, Schrift g_font, Farbe 0xcd bzw. 0xff), Portraet load_portrait (active 0: portrait_dim), Platz
 * update_hero_bars/slot_pos (NSC Platz 6), Rueckgabe naechster Platz. compact: Luecken in heroes[] schliessen
 * (memcpy, heroes[6] = 0). mark_can_use(id): can_use_item fuer alle, Treffer in g_portrait_marks[Platz] bzw. [6] fuer
 * den NSC, screen_draw_names. draw_portraits: Portraetleiste (Breite nr_heroes * 33 + 3, ab x = 240 - Breite).
 * get_on_route/set_on_route: on_route lesen/setzen.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL201.CPP   -> 3591/3591 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h, Methoden an struct group: 19.10.2026 GLEICH, LINKGLEICH)
 * - -Og noetig (Epilogkopien, Zwischenwert this->route_from [bp-1] in same_place, h->inv[j].item_id [bp-8] in
 *   all_have_item), -Oi fuer strcmp/memset/memcpy.
 * - same_place: o->route_pos ueber den inline-Getter get_route_pos() (unsigned long) -> mov eax,[..] / mov si,ax bzw.
 *   "route_len - o->get_route_pos()" -> mov eax / mov dx,[3b] / sub dx,ax; direkt o->route_pos ergibt mov si,[..]
 *   bzw. mov ax / sub ax. Vergleich mit "(unsigned long)route_pos" (das Original rechnet unsigned).
 * - find_food/find_drink: "n = (unsigned char)get_hero_index(p);" als eigene Anweisung (Original deklariert die
 *   Rueckgabe unsigned char -> mov ah,0), dann "if (g_hero_excluded != n && ..)"; eingebettet
 *   "g_hero_excluded != (n = ..)" laedt g_hero_excluded vor dem Aufruf (push ax / pop dx). Die Zeiger &slot/&id
 *   werden wie im Original durchgereicht (frueher als vier short-Parameter geschrieben - gleiche Bytes).
 * - draw_hero_bar: "font_print(g_font, s, font_center_x(g_font, s, x, 0x2b), 0xbe)" - die zuerst gepushte 0xbe
 *   gehoert zum aeusseren Aufruf. Die Farbe uebergibt das Original als short (mov ax,0cdh), s_proto.h hat fuer
 *   font_set_colors unsigned char (mov al) -> Aufruf ueber einen gecasteten Funktionszeiger
 *   "((void (*)(void far *, signed short, signed short))font_set_colors)(..)" (ergibt denselben direkten far call).
 *   fill_saved_values(h, -1) mit char-Prototyp pusht die Konstante weiter als Wort.
 * - count_check2b: "for (i = 0, n = 0, ..)" (xor si vor xor di; O200 hat n = 0 vor der Schleife). compact:
 *   "*(signed long far *)&heroes[6] = 0" -> mov dword (Zeigerzuweisung ergaebe zwei mov word), "!heroes[i]" ->
 *   mov ax / or ax,[+2]. portrait_dim: c als register unsigned char im Block (DX), i in CX.
 * Originalfehler: hero_with_item prueft pos -2/-3 erst NACH der Heldenschleife mit h = heroes[7] (hinter dem Feld)
 *   und liefert Platz 7; all_have_item bricht bei pos -2/-3 schon ab, wenn Platz 0 bzw. 15 nicht der Gegenstand
 *   ist. draw_portraits nimmt die Heldenzahl von g_group statt von this.
 */
#include <string.h>
#include "schweif.h"

signed short group::has_hero_named(char far *name)
{
	signed short i;
	struct hero far *h;

	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0 && !strcmp(h->name, name))
			return 1;
	}
	return 0;
}

signed short group::has_hero(struct hero far *x)
{
	struct hero far *h;
	signed short i;

	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0 && h == x)
			return 1;
	}
	return 0;
}

signed short group::count_check2b()
{
	struct hero far *h;
	signed short n;
	signed short i;

	for (i = 0, n = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0 && check_hero2(h))
			n++;
	}
	return n;
}

signed short group::count_alive()
{
	struct hero far *h;
	signed short n;
	signed short i;

	n = 0;
	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0 && !(signed char)h->dead)
			n++;
	}
	return n;
}

signed short group::hero_pos(struct hero far *x)
{
	struct hero far *h;
	signed short i;

	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h == x)
			return i;
	}
	return -1;
}

signed short group::hero_with_item(signed short id, signed short pos)
{
	struct hero far *h;
	signed short j;
	signed short i;

	if (pos == -1) {
		return group_find_item(this, id);
	} else {
#if FIX_0051
		/* Fix #51: Suche nach -2 (Hand/Koerper 0..15) bzw. -3 (Rucksack 16..30) fuer jeden Helden in der Schleife */
		for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
			if (h == 0)
				continue;
			if (pos >= 0 && h->inv[pos].item_id == id)
				return i;
			if (pos == -2) {
				for (j = 0; j < INV_SLOT_KNAPSACK; j++) {
					if (h->inv[j].item_id == id)
						return i;
				}
			}
			if (pos == -3) {
				for (j = INV_SLOT_KNAPSACK; j <= 30; j++) {
					if (h->inv[j].item_id == id)
						return i;
				}
			}
		}
		return -1;
#else
		for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
			if (h != 0 && pos >= 0 && h->inv[pos].item_id == id)
				return i;
		}
		/* Originalfehler: -2/-3 erst nach der Schleife, h = heroes[7] (hinter dem Feld) */
		if (pos == -2) {
			for (j = 0; j < INV_SLOT_KNAPSACK; j++) {
				if (h->inv[j].item_id == id)
					return i;
			}
		}
		if (pos == -3) {
			for (j = INV_SLOT_KNAPSACK; j <= 30; j++) {
				if (h->inv[j].item_id == id)
					return i;
			}
		}
		return -1;
#endif
	}
}

signed short group::all_have_item(register signed short id, signed short pos)
{
	signed short i;
	struct hero far *h;
	signed short j;

	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0 && check_hero3(h)) {
			if (pos == -1 && get_item_pos(h, id) < 0)
				return -1;
			if (pos >= 0 && h->inv[pos].item_id != id)
				return -1;
			/* Originalfehler: -2/-3 brechen schon am ersten Platz ohne den Gegenstand ab */
#if FIX_0052
			/* Fix #52: echte Suche ueber alle Plaetze statt Abbruch am ersten Platz */
			if (pos == -2) {
				for (j = 0; j < INV_SLOT_KNAPSACK && h->inv[j].item_id != id; j++)
					;
				if (j == INV_SLOT_KNAPSACK)
					return -1;
			}
			if (pos == -3) {
				for (j = INV_SLOT_KNAPSACK; j <= 30 && h->inv[j].item_id != id; j++)
					;
				if (j > 30)
					return -1;
			}
#else
			if (pos == -2) {
				for (j = 0; h->inv[j].item_id != id; j++) {
					if (h->inv[j].item_id != id)
						return -1;
				}
			}
			if (pos == -3) {
				for (j = INV_SLOT_KNAPSACK; h->inv[j].item_id != id; j++) {
					if (h->inv[j].item_id != id)
						return -1;
				}
			}
#endif
		}
	}
	return 0;
}

signed short group::find_food(struct hero far *first, signed short far *hero_no, signed short far *pslot,
	signed short far *pid)
{
	signed short found;
	struct hero far *h;
	register signed short i;
	register signed short n;

	found = 0;
	n = 99;
	if (g_hero_excluded != -1)
		n = (unsigned char)get_hero_index(first);
	if (g_hero_excluded != n && hero_find_food(first, pslot, pid)) {
		found = 1;
		*hero_no = (unsigned char)get_hero_index(first);
	} else {
		for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
			if (h != 0) {
				n = (unsigned char)get_hero_index(h);
				if (g_hero_excluded != n && h != first && hero_find_food(h, pslot, pid)) {
					found = 1;
					*hero_no = (unsigned char)get_hero_index(h);
					break;
				}
			}
		}
	}
	g_hero_excluded = -1;
	return found;
}

signed short group::find_drink(struct hero far *first, signed short far *hero_no, signed short far *pslot,
	signed short far *pid)
{
	signed short found;
	struct hero far *h;
	register signed short i;
	register signed short n;

	found = 0;
	n = 99;
	if (g_hero_excluded != -1)
		n = (unsigned char)get_hero_index(first);
	if (g_hero_excluded != n && hero_find_drink(first, pslot, pid)) {
		found = 1;
		*hero_no = (unsigned char)get_hero_index(first);
	} else {
		for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
			if (h != 0) {
				n = (unsigned char)get_hero_index(h);
				if (g_hero_excluded != n && h != first && hero_find_drink(h, pslot, pid)) {
					found = 1;
					*hero_no = (unsigned char)get_hero_index(h);
					break;
				}
			}
		}
	}
	g_hero_excluded = -1;
	return found;
}

void group::add_effect(signed long time, signed char type, signed char index, signed char value)
{
	struct hero far *h;
	signed short i;

	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0 && !(signed char)h->dead)
			::add_effect(h, get_free_effect(h), time, type, index, value);
	}
}

signed short group::same_place(struct group far *o)
{
	register unsigned short dist = -36;

	if (on_route != 0 && o->get_on_route()) {
		if (route_from == o->route_from && route_to == o->route_to)
			dist = o->get_route_pos();
		if (route_to == o->route_from && route_from == o->route_to)
			dist = route_len - o->get_route_pos();
		return dist - 300 < (unsigned long)route_pos && dist + 300 > (unsigned long)route_pos ? 1 : 0;
	} else {
		return x == o->x && y == o->y && (signed short)loctype == (signed short)o->loctype && town == o->town &&
			dungeon == o->dungeon && (signed short)f24 == (signed short)o->f24 ? 1 : 0;
	}
}

void group::swap_slots(register signed short a, signed short b)
{
	struct hero far *h;

	h = heroes[a];
	heroes[a] = heroes[b];
	heroes[b] = h;
	h = heroes[a];
	h->action_id = 100;
	fill_saved_values(h, -1);
	h = heroes[b];
	h->action_id = 100;
	fill_saved_values(h, -1);
}

/* Portraet (32 x 32 Bildpunkte) auf die gedimmte Farbtabelle umfaerben */
void portrait_dim(unsigned char far *pix)
{
	register signed short i;

	for (i = 0; i < 0x400; i++) {
		register unsigned char c = *pix;

		if (c == 0xbd || (c >= 0xd0 && c < 0xd5) || (c >= 0xe0 && c < 0xee))
			c = c + 2;
		else if (c == 0xa3 || c == 0xa9 || c == 0xc7 || c == 0xcf || c == 0xd7 || c == 0xdf || c == 0xef ||
		    c == 0xf6 || c == 0xfe)
			c = 0xfe;
		else if (c == 0xa6)
			c = 0xcc;
		else if (c == 0xad)
			c = 0xed;
		else if (c == 0xae)
			c = 0xd7;
		else if (c == 0xaf)
			c = 0xd5;
		else if (c == 0xb7 || c == 0xbc)
			c = 0xea;
		else if (c == 0xbf)
			c = 0xb5;
		else if (c == 0xd6)
			c = 0xae;
		else if (c == 0xf7)
			c = 0xa1;
		else if (c == 0xf8)
			c = 0xbe;
		else if (c == 0xf9)
			c = 0xb0;
		else if (c == 0xfa)
			c = 0xf7;
		else if (c == 0xfb)
			c = 0xf4;
		else if (c == 0xfc)
			c = 0xd1;
		else if (c == 0xfd)
			c = 0xb9;
		else if (c == 0xff)
			c = 0xc8;
		else if (c == 0)
			c = 0;
		else
			c = c + 1;
		*pix++ = c;
	}
}

signed short group::draw_hero_bar(signed short slot, signed short active)
{
	signed short i;
	signed short pos;
	struct hero far *h;
	unsigned char far *pix;
	char name[10];
	Bmp name_bmp(0x2a, 8);
	Bmp portrait(0x20, 0x20, 0);
	register signed short x;

	pix = (unsigned char far *)name_bmp.data();
	memset(pix, 0, 0x150);
	wait_for_vsync();
	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0) {
			mouse_hide(0);
			pos = !h->npc_id ? slot++ : 6;
			x = g_portrait_x[pos];
			name_bmp.put(g_vga_buf, x, 0xbe);
			screen_short_name(g_screen, name, h->name);
			/* Original uebergibt die Farbe als short (s_proto.h: unsigned char, siehe konflikte.tsv) */
			((void (*)(void far *, signed short, signed short))font_set_colors)(g_font, active == 0 ? 0xcd : 0xff, 0);
			font_print(g_font, name, font_center_x(g_font, name, x, 0x2b), 0xbe);
			font_restore_colors(g_font);
			portrait.set_base(load_portrait(h));
			if (active == 0)
				portrait_dim((unsigned char far *)portrait.data());
			portrait.put(g_vga_buf, x, 0x9d);
			fill_saved_values(h, -1);
			update_hero_bars(h, pos);
			h->set_slot_pos(pos);
			mouse_show(0);
		}
	}
	return slot;
}

void group::compact()
{
	signed short i;
	signed short j;
	struct hero far *h;

	for (i = 0; i <= 6; i++) {
		if (!heroes[i]) {
			for (h = heroes[j = i + 1]; j <= 6; h = heroes[++j]) {
				if (h != 0)
					fill_saved_values(h, -1);
			}
			memcpy(&heroes[i], &heroes[i + 1], (6 - i) * 4);
			*(signed long far *)&heroes[6] = 0;
		}
	}
}

signed short group::mark_can_use(signed short id)
{
	struct hero far *h;
	register signed short i;
	register signed short ret = 0;

	for (i = 0; i < 7; i++)
		g_portrait_marks[i] = 0;
	for (i = 0, h = heroes[0]; i < 7; h = heroes[++i]) {
		if (h != 0) {
			if (h->npc_id != 0) {
				if (can_use_item(h, id))
					g_portrait_marks[6] = ret = 1;
			} else {
				if (can_use_item(h, id))
					g_portrait_marks[i] = ret = 1;
			}
		}
	}
	screen_draw_names(g_screen);
	return ret;
}

void group::draw_portraits()
{
	struct hero far *h;
	signed short n;
	register signed short i;
	register signed short w;

#if FIX_0053
	/* Fix #53: Heldenzahl der eigenen Gruppe */
	n = nr_heroes;
#else
	n = g_group->nr_heroes;			/* Originalfehler: aktive Gruppe statt this */
#endif
	w = n * 33 + 3;
	Bmp bar(w, 0x26);
	bar.fill(0);
	mouse_hide(0);
	bar.put(g_vga_buf, 240 - w, 0xa2);
	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0) {
			Bmp portrait(0x20, 0x20, load_portrait(h));

			portrait.put(g_vga_buf, 240 - w + i * 33 + 2, 0xa5);
		}
	}
	mouse_show(0);
}

signed short group::get_on_route()
{
	return on_route;
}

void group::set_on_route(signed short v)
{
	on_route = v;
}
