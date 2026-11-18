/* OVL153 - Overlay O153 (Segment 0x2262 in SCHWEIF.EXE): Kampf-Hilfsroutinen. Klasse FigList (s_class.h; frueher "List", 0x0c Bytes, im Kampf-
 * objekt +0x35, vgl. OVL018): doppelt verkettete, nach Feld (x, y, prio) sortierte Liste von Knoten (0x1f Bytes:
 * x/y, id +7, prio +8, sichtbar +9, Partner-id +0b (0xff = keiner, +-20), Objekt +0d, next +15, prev +19,
 * Loeschmarken +1d/+1e) mit Kopfknoten (x = y = -99) und Belegungstabelle used[0xff]. Konstruktor FigList_ctor /
 * Destruktor FigList_dtor, clear fig_list_clear, mark_del/mark_del2 fig_list_mark_remove/fig_list_mark_resort, last fig_list_last, hide/show fig_list_hide/fig_list_show (auch
 * Partner), resort fig_list_resort, purge fig_list_purge (markierte Knoten entfernen bzw. neu einsortieren), exists fig_list_exists,
 * find/find_obj/find_link fig_list_find/fig_list_find_anim/fig_list_find_twofield, getpos fig_list_getpos, alloc_id fig_list_alloc_id, remove fig_list_remove (Knoten nach
 * *ds:c965 sichern), add fig_list_add (Vorlage *ds:c965). Ausserdem Kampffeld *ds:c99b (24 x 24, Zeilenlaenge 25):
 * manhattan_dist Manhattan-Abstand, fight_set_cell Feld setzen, fight_find_cell Wert suchen (x, y), fight_cell_blocked Feld frei/gesperrt (-2);
 * Gegner (0x49 Bytes, Tabelle *ds:c4f7[20]): fig_enemy_pic Bild, enemy_is_active/enemy_is_active_or_asleep/fight_check_state/enemy_is_renegade_on_field Zustandspruefungen
 * (Bits +38/+39, +3c), fight_obj_index Index, enemy_is_twofielded Typ in Liste ds:65b4; fig_turn_steps/dir_opposite Drehrichtung, fight_dir_hero_straight/fight_dir_enemy_straight/
 * fight_dir_hero/fight_dir_enemy Blickrichtung Held/Gegner zum Ziel, fight_hero_pic Heldenbild, fig_get_hero Held mit Nummer n in der Gruppe.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL153.CPP   -> 4293/4293 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h, Welle N7: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * - C++: Konstruktor/Destruktor (this == 0 -> new 0x0c, flags & 1 -> delete), unsigned-char-Bitfelder
 *   (shr ax / and ax,1 / or al,al), inline abs() in manhattan_dist (Parameterkopien [bp-2]/[bp-4]).
 * - -Oi: memset(used, 0, 0xff) (rep stosw / stosb, daher push di); -Og: Zwischenwerte (Bitbyte +38 in DL, +39
 *   in [bp-1] bzw. [bp-5]/[bp-6], p->x in [bp-0eh] in add, p->id in [bp-0bh] in purge).
 * - alloc_id: Index (unsigned)i als "volatile signed short k" (used[k = i]), sonst legt BCC ihn in CX statt
 *   [bp-2].
 * - add: "*n = *g_c965;" ergibt mit -Oi push ds / mov ds,ax / rep movsw / movsb (Quelle zuerst geladen);
 *   memcpy(n, g_c965, 0x1f) laedt das Ziel zuerst. In remove ergibt "*g_c965 = *p;" dagegen SCOPY@.
 * - purge: "id = p->id; remove(p->id, 1);" (Zwischenwert + Kopie), Abfrage "nid != 0xff" (cmp byte).
 * - Rueckgaben: "if (..) return 0; return 1;" (ein toter Epilog) in enemy_is_active/enemy_is_active_or_asleep/fight_check_state/enemy_is_twofielded; mit else
 *   zwei (fight_cell_blocked, Ende von enemy_is_renegade_on_field). fig_turn_steps: "from++; if (from == 4)" (cmp dx,4; mit ++from mov ax,dx).
 * - fight_set_cell/fight_cell_blocked: register-Parameter x, y -> CX, SI; fig_turn_steps: from/to/r in DX/CX/BX.
 * Originalfehler: clear() loescht used[] komplett, auch die Sperre used[0] = 0xff aus dem Konstruktor (danach
 *   kann alloc_id die 0 vergeben); die id des Kopfknotens wird nie gesetzt (find/remove pruefen sie mit).
 *   add() mit Vorlage prio == -1 haengt vor dem Kopfknoten ein: p->prev ist 0 -> Schreiben nach 0000:0015.
 *   hide/show pruefen das Ergebnis von find_link nicht (Partner fehlt -> Schreiben ueber Nullzeiger); getpos
 *   liefert bei unbekannter id die Position des letzten Knotens. fight_find_cell sucht nur x < 24, fight_set_cell schreibt
 *   auch x = 24.
 */
#include <stdlib.h>
#include <mem.h>
#include <dos.h>
#include "schweif.h"

FigList::FigList()
{
	used = (unsigned char far *)mem_alloc(0xffL);
	head = (struct fig_node far *)mem_alloc(sizeof(struct fig_node));
	head->prev = head->next = 0;
	head->twofielded = FIG_TWOFIELD_NONE;
	head->gfx = 0;
	head->anim = 0;
	head->cbx = -99;
	head->cby = -99;
	memset(used, 0, 0xff);
	used[0] = 0xff;
}

FigList::~FigList()
{
	clear();
	mem_free(head);
	mem_free(used);
}

void FigList::clear()
{
	struct fig_node far *p;

	p = last();
	while (p->prev != 0) {
		p = p->prev;
		mem_free(p->next);
	}
#if FIX_0090
	/* Fix #90: Sperre used[0] (id 0) wie im Konstruktor erhalten */
	head->prev = head->next = 0;
	memset(used, 0, 0xff);
	used[0] = 0xff;
#else
	head->prev = head->next = 0;
	memset(used, 0, 0xff);
#endif
}

void FigList::mark_remove(unsigned char id)
{
	struct fig_node far *p;

	p = head;
	while (p->id != id) {
		if (p->next == 0)
			return;
		p = p->next;
	}
	p->to_remove = 1;
}

void FigList::mark_resort(unsigned char id)
{
	struct fig_node far *p;

	p = head;
	while (p->id != id) {
		if (p->next == 0)
			return;
		p = p->next;
	}
	p->to_resort = 1;
}

struct fig_node far *FigList::last()
{
	struct fig_node far *p;

	p = head;
	while (p->next != 0)
		p = p->next;
	return p;
}

void FigList::hide(unsigned char id)
{
	struct fig_node far *p;
	struct fig_node far *partner;

	p = head;
	while (p->id != id) {
		if (p->next == 0)
			return;
		p = p->next;
	}
	p->visible = 0;
	if (p->twofielded != FIG_TWOFIELD_NONE) {
		if (p->twofielded > 20)
			partner = find_twofield(p->twofielded - 20);
		else
			partner = find_twofield(p->twofielded + 20);
#if FIX_0090
		/* Fix #90: Partner nur setzen, wenn er gefunden wurde */
		if (partner != 0)
			partner->visible = 0;
#else
		partner->visible = 0;
#endif
	}
}

void FigList::show(unsigned char id)
{
	struct fig_node far *p;
	struct fig_node far *partner;

	p = head;
	while (p->id != id) {
		if (p->next == 0)
			return;
		p = p->next;
	}
	p->visible = 1;
	if (p->twofielded != FIG_TWOFIELD_NONE) {
		if (p->twofielded > 20)
			partner = find_twofield(p->twofielded - 20);
		else
			partner = find_twofield(p->twofielded + 20);
#if FIX_0090
		/* Fix #90: Partner nur setzen, wenn er gefunden wurde */
		if (partner != 0)
			partner->visible = 1;
#else
		partner->visible = 1;
#endif
	}
}

void FigList::resort()
{
	unsigned char id;

	for (id = 1; id < 0xff; id++)
		if (used[id] != 0) {
			remove(id, 1);
			add(id);
		}
}

void FigList::purge()
{
	struct fig_node far *p;
	struct fig_node far *next;
	unsigned char id;
	unsigned char next_id;
	register signed short hit;

	p = head;
	do {
		hit = 0;
		if (p->to_remove) {
			hit = 1;
			next_id = 0xff;
			next = p->next;
			if (next != 0)
				next_id = next->id;
			remove(p->id, 0);
		} else if (p->to_resort) {
			hit = 1;
			next_id = 0xff;
			next = p->next;
			if (next != 0)
				next_id = next->id;
			id = p->id;
			remove(p->id, 1);
			g_fig_tmpl->to_resort = 0;
			add(id);
		}
		if (hit) {
			if (next_id != 0xff)
				p = find(next_id);
			else
				p = 0;
		} else
			p = p->next;
	} while (p != 0);
}

signed short FigList::exists(unsigned char id)
{
	struct fig_node far *p;

	p = head;
	while (p->id != id) {
		if (p->next == 0)
			return 0;
		p = p->next;
	}
	return 1;
}

struct fig_node far *FigList::find(unsigned char id)
{
	struct fig_node far *p;

	p = head;
	while (p->id != id) {
		if (p->next == 0)
			return 0;
		p = p->next;
	}
	return p;
}

struct fig_node far *FigList::find_anim(void far *anim)
{
	struct fig_node far *p;

	p = head;
	while (p->anim != anim) {
		if (p->next == 0)
			return 0;
		p = p->next;
	}
	return p;
}

struct fig_node far *FigList::find_twofield(unsigned char tf)
{
	struct fig_node far *p;

	p = head;
	while (p->twofielded != tf) {
		if (p->next == 0)
			return 0;
		p = p->next;
	}
	return p;
}

void FigList::getpos(unsigned char id, signed short far *x, signed short far *y)
{
	struct fig_node far *p;

	p = head;
	while (p->id != id) {
		if (p->next == 0)
			break;
		p = p->next;
	}
	*x = p->cbx;
	*y = p->cby;
}

unsigned char FigList::alloc_id()
{
	unsigned char i;
	volatile signed short k;

	for (i = 0; used[k = i] != 0; i++)
		;
	used[k] = 1;
	return i;
}

void FigList::remove(unsigned char id, signed char keep)
{
	struct fig_node far *p;

	p = head;
	while (p->id != id) {
		if (p->next == 0)
			return;
		p = p->next;
	}
	if (!keep)
		used[id] = 0;
	else
		*g_fig_tmpl = *p;
	if (p->next == 0)
		p->prev->next = 0;
	else {
		p->prev->next = p->next;
		p->next->prev = p->prev;
	}
	mem_free(p);
}

unsigned char FigList::add(unsigned char id)
{
	struct fig_node far *n;
	struct fig_node far *p;
	signed short x;
	signed short y;

	n = (struct fig_node far *)mem_alloc(sizeof(struct fig_node));
	x = g_fig_tmpl->cbx;
	y = g_fig_tmpl->cby;
	*n = *g_fig_tmpl;
	if (id == 0xff)
		n->id = alloc_id();
	else
		n->id = id;
	p = head;
	if (g_fig_tmpl->z != -1)
		while (!(p->cbx > x || (p->cbx == x && p->cby < y) || (p->cbx == x && p->cby == y && p->z > g_fig_tmpl->z))) {
			if (p->next == 0) {
				p->next = n;
				n->prev = p;
				n->next = 0;
				return n->id;
			}
			p = p->next;
		}
	n->prev = p->prev;
	p->prev->next = n;
	p->prev = n;
	n->next = p;
	return n->id;
}

signed short manhattan_dist(signed short x1, signed short y1, signed short x2, signed short y2)
{
	return abs(x1 - x2) + abs(y1 - y2);
}

void fight_set_cell(void far *f, register signed short x, register signed short y, unsigned char v)
{
	if (y >= 0 && y < FIG_FIELD_W && x >= 0 && x < FIG_MAP_PITCH)
		*(g_fight_walkmap + y * FIG_MAP_PITCH + x) = v;
}

signed short fight_find_cell(signed char id, signed short far *x, signed short far *y)
{
	for (*x = 0; *x < FIG_FIELD_W; (*x)++) {
		if (g_quick_fight)
			game_idle();
		for (*y = 0; *y < FIG_FIELD_W; (*y)++)
			if (*((signed char far *)g_fight_walkmap + *y * FIG_MAP_PITCH + *x) == id)
				return 1;
	}
	return 0;
}

signed short fight_cell_blocked(register signed short x, register signed short y)
{
	if (x > 23 || x < 0 || y > 23 || y < 0 || *((signed char far *)g_fight_walkmap + y * FIG_MAP_PITCH + x) == -2)
		return 1;
	else
		return 0;
}

signed short fig_enemy_pic(struct enemy far *e)
{
	return e->gfx_id + 0xb5;
}

signed short enemy_is_active(struct enemy far *e)
{
	if ((signed char)e->asleep || (signed char)e->dead || (signed char)e->petrified || (signed char)e->dancing ||
		(signed char)e->tied || (signed char)e->mushroom || (signed char)e->busy || (signed char)e->tame ||
		e->round_appear > 0)
		return 0;
	return 1;
}

signed short enemy_is_active_or_asleep(struct enemy far *e)
{
	if ((signed char)e->dead || (signed char)e->petrified || (signed char)e->dancing || (signed char)e->tied ||
		(signed char)e->mushroom || (signed char)e->tame || (signed char)e->busy || e->round_appear > 0)
		return 0;
	return 1;
}

unsigned char fight_obj_index(struct enemy far *e)
{
	unsigned char i;
	struct enemy far *p;

	for (i = 0, p = g_enemies; i < NR_ENEMIES; p = &g_enemies[++i])
		if (e == p)
			return i;
	return 0;
}

signed short fight_check_state(struct enemy far *e)
{
	signed short x;
	signed short y;

	if (!e->mon_id || (signed char)e->dead || (signed char)e->petrified || (signed char)e->mushroom ||
		(signed char)e->tame || (signed char)e->renegade || (signed char)e->tied || (signed char)e->busy ||
		(signed char)e->dancing || e->round_appear > 0 || !fight_find_cell(fight_obj_index(e) + FIG_ID_ENEMY, &x, &y))
		return 0;
	return 1;
}

signed short enemy_is_renegade_on_field(struct enemy far *e)
{
	signed short x;
	signed short y;

	if (!e->mon_id || (signed char)e->dead || (signed char)e->petrified || (signed char)e->mushroom ||
		(signed char)e->tame || (signed char)e->dancing || (signed char)e->tied || (signed char)e->busy ||
		e->round_appear > 0 || !fight_find_cell(fight_obj_index(e) + FIG_ID_ENEMY, &x, &y))
		return 0;
	if ((signed char)e->renegade)
		return 1;
	else
		return 0;
}

signed short enemy_is_twofielded(struct enemy far *e)
{
	if (is_in_byte_array2(e->mon_id, g_twofield_mons) == -1)
		return 0;
	return 1;
}

signed short fig_turn_steps(void far *f, register signed short from, register signed short to)
{
	signed short r;

	r = 1;
	from++;
	if (from == 4)
		from = 0;
	if (from != to) {
		r = 2;
		from++;
		if (from == 4)
			from = 0;
		if (from != to)
			r = -1;
	}
	return r;
}

signed short dir_opposite(void far *f, register signed short d)
{
	return d == 0 ? 2 : d == 1 ? 3 : d == 2 ? 0 : 1;
}

signed short fight_dir_hero_straight(void far *f, struct hero far *h)
{
	signed short x;
	signed short y;
	signed short tx;
	signed short ty;
	register signed short dir;

	if (g_quick_fight)
		game_idle();
	fight_find_cell(h->enemy_id, &tx, &ty);
	fight_find_cell(get_hero_index(h) + FIG_ID_HERO, &x, &y);
	if (x == tx) {
		if (ty == y)
			dir = h->viewdir;
		else if (ty < y)
			dir = 3;
		else
			dir = 1;
	} else if (tx < x)
		dir = 0;
	else
		dir = 2;
	return dir;
}

signed short fight_dir_enemy_straight(void far *f, struct enemy far *e)
{
	signed short x;
	signed short y;
	signed short tx;
	signed short ty;
	register signed short dir;

	if (g_quick_fight)
		game_idle();
	fight_find_cell(e->enemy_id, &tx, &ty);
	fight_find_cell(fight_obj_index(e) + FIG_ID_ENEMY, &x, &y);
	if (x == tx) {
		if (ty == y)
			dir = e->viewdir;
		else if (ty < y)
			dir = 3;
		else
			dir = 1;
	} else if (tx < x)
		dir = 0;
	else
		dir = 2;
	return dir;
}

signed short fight_dir_hero(void far *f, struct hero far *h)
{
	signed short x;
	signed short y;
	signed short tx;
	signed short ty;
	register signed short dir;

	if (g_quick_fight)
		game_idle();
	if (h->enemy_id != 0)
		fight_find_cell(h->enemy_id, &tx, &ty);
	else {
		tx = h->target_x;
		ty = h->target_y;
	}
	fight_find_cell(get_hero_index(h) + FIG_ID_HERO, &x, &y);
	if (x == tx && y == ty)
		dir = h->viewdir;
	else if (tx + ty <= x + y) {
		if (ty >= y || x - tx > y - ty)
			dir = 2;
		else
			dir = 1;
	} else {
		if (tx <= x || ty - y > tx - x)
			dir = 3;
		else
			dir = 0;
	}
	return dir;
}

signed short fight_dir_enemy(void far *f, struct enemy far *e)
{
	signed short x;
	signed short y;
	signed short tx;
	signed short ty;
	register signed short dir;

	if (g_quick_fight)
		game_idle();
	fight_find_cell(e->enemy_id, &tx, &ty);
	fight_find_cell(fight_obj_index(e) + FIG_ID_ENEMY, &x, &y);
	if (x == tx && y == ty)
		dir = e->viewdir;
	else if (tx + ty <= x + y) {
		if (ty >= y || x - tx > y - ty)
			dir = 2;
		else
			dir = 1;
	} else {
		if (tx <= x || ty - y > tx - x)
			dir = 3;
		else
			dir = 0;
	}
	return dir;
}

signed short fight_hero_pic(struct hero far *h)
{
	if (h->npc_id == 1)
		return 0xfc;
	else
		return (signed char)((h->type < 10 ? h->type - 1 : 9) * 2 + h->sex) + 0xb5;
}

struct hero far *fig_get_hero(struct group far *g, signed char fighter_id)
{
	register signed short i;
	struct hero far *h;

	if (g_quick_fight)
		game_idle();
	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h != 0 && h->fighter_id == fighter_id)
			return h;
	return 0;
}
