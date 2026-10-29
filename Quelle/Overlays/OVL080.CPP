/* OVL080 - Overlay O080 (Segment 0x20dc in SCHWEIF.EXE): Heldenzustaende rund um den Kampf, Schleifen ueber die
 * 7 Heldenzeiger einer Gruppe g. count_heroes_except(g, ex): Anzahl handlungsfaehiger Helden (check_hero) ausser ex.
 * count_heroes_in_fight(g): Anzahl Helden, die nicht tot/versteinert/abtruennig/bewusstlos sind und deren Figur
 * (get_hero_index + 1) fight_find_cell auf dem Kampffeld findet. scared_hero_fled(g): 1, wenn ein veraengstigter
 * Held (scared) nicht mehr auf dem Feld steht. fight_round_heroes(g): Rundenbeginn - actions = 1, bp_left =
 * calc_bp (hoechstens 12), escape_position = 0 (nicht veraengstigt), Axxeleratus -> 2 Aktionen, parried = 0; liefert
 * die Zahl der Aktionen. fight_end_heroes(g): Kampfende - Zustaende zuruecksetzen (tied, set_asleep(h, 0),
 * chamaelioni, duplicatus, scared, blind, ecliptifactus, action_id = 12, fb5, fb6_b0, gardianum, renegade); der am schwersten
 * verwundete Held (kleinste LE, LE != LE-Grundwert) bekommt zu 4 % Schlachtfeldfieber (hero_get_ill); danach
 * dieselben Zuruecksetzungen fuer alle Helden aus g_heroes, die nicht in der Gruppe sind (group_has_hero).
 * group_set_fb5/group_set_fb6_b0(g, v): fb5 bzw. Bit fb6_b0 aller Helden ohne NSC-Nummer setzen (inline-Setter),
 * group_set_fb5 zusaetzlich g_key_last = 0. hand_weapon_class_melee/hand_weapon_class(h): Kategorie
 * g_weapons_class[g_items[id].table_index] der Waffe in der rechten Hand (inv[INV_SLOT_RIGHT_HAND], Bit weapon,
 * nicht broken), < 4 (Nahkampf) bzw. >= 4 (Fernkampf, nicht Tabellenindex 0x35), sonst -1.
 * set_asleep/set_unconscious(h, v): Bit asleep bzw. unconscious setzen, vorher ggf. Klang g_snd_fx (snd_busy/
 * snd_play, nach Geschlecht) bzw. im Kampf (g_fight_obj) Lst_add2(g_fight_list, 7, 0, 2, h); g_hero_died = 1.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL080.CPP   -> 1841/1841 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * - Gruppenschleife "for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])" (Startwert mov eax, ohne
 *   Parameterkopie).
 * - Helden-Statusbits "unsigned short b:1": Lesen direkt -> or ax,ax; "(signed char)h->b" -> and/cbw/or ax,ax;
 *   "(unsigned char)h->b" -> or al,al; Schreiben aus signed char -> cbw/and ax,1.
 * - inline-Setter hero::set_fb5/set_fb6_b0(signed char) -> Parameterkopie [bp-5]; "!h->npc_id" -> mov al/cbw/
 *   or ax,ax.
 * - fight_end_heroes: KEINE Variable fuer h->le - der Wert steht dreimal im Ausdruck/Rumpf, -Og legt ihn nach DI
 *   ("mov di,ax / cmp di,es:[bx+70h]"); mit "register v = h->le" kommt cmp es:[bx+70h],di heraus.
 * - "if (h->bp_left > 12) h->bp_left = 12; else ;" erzeugt den jmp short auf den naechsten Befehl.
 * - -Og noetig (Zwischenwert [bp-9] fuer Byte +bc in count_heroes_in_fight, SI = Offset von &g_items[id] in
 *   hand_weapon_class*).
 * - set_asleep steht unten -> far call aus fight_end_heroes (TLINK macht nop/push cs/call near daraus).
 */
#include "schweif.h"

signed short count_heroes_except(struct group far *g, struct hero far *ex)
{
	struct hero far *h;
	register signed short i;
	register signed short n;

	n = 0;
	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h && h != ex && check_hero(h))
			n++;
	return n;
}

signed short count_heroes_in_fight(struct group far *g)
{
	signed short x;				/* Position auf dem Kampffeld (unbenutzt) */
	signed short y;
	struct hero far *h;
	register signed short i;
	register signed short n;

	n = 0;
	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h && !(signed char)h->dead && !(signed char)h->petrified && !(signed char)h->renegade &&
		    !(signed char)h->unconscious && fight_find_cell(get_hero_index(h) + 1, &x, &y))
			n++;
	return n;
}

signed short scared_hero_fled(struct group far *g)
{
	signed short x;				/* Position auf dem Kampffeld (unbenutzt) */
	signed short y;
	struct hero far *h;
	register signed short i;

	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h && !(signed char)h->dead && (unsigned char)h->scared && !fight_find_cell(get_hero_index(h) + 1, &x, &y))
			return 1;
	return 0;
}

signed short fight_round_heroes(struct group far *g)
{
	signed short x;				/* Position auf dem Kampffeld (unbenutzt) */
	signed short y;
	struct hero far *h;
	signed char bp;
	register signed short n;
	register signed short i;

	n = 0;
	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i]) {
		if (h) {
			if (!(signed char)h->scared || fight_find_cell(get_hero_index(h) + 1, &x, &y)) {
				h->actions = 1;
				bp = calc_bp(h);
				h->bp_left = bp;
				if (h->bp_left > 12)
					h->bp_left = 12;
				else
					;
				if (!(signed char)h->scared)
					h->escape_position = 0;
				n++;
				if (h->axxeleratus != 0) {
					h->actions = 2;
					n++;
				}
			}
			h->parried = 0;
		}
	}
	return n;
}

void fight_end_heroes(struct group far *g)
{
	signed short min_le;			/* kleinste LE eines verwundeten Helden */
	struct hero far *h;
	struct hero far *worst;		/* am schwersten verwundeter Held */
	register signed short i;

	min_le = 10000;
	worst = 0;
	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i]) {
		if (h) {
			h->tied = 0;
			set_asleep(h, 0);
			h->chamaelioni = 0;
			h->duplicatus = 0;
			h->scared = 0;
			h->blind = 0;
			h->ecliptifactus = 0;
			h->action_id = 12;
			h->fb5 = 0;
			h->fb6_b0 = 0;
			h->gardianum = 0;
			h->renegade = 0;
			if (!(signed char)h->dead && h->le < min_le && h->le != h->le_max) {
				min_le = h->le;
				worst = h;
			}
		}
	}
	if (random_schweif(100) < 5 && worst)
		hero_get_ill(worst, ILLNESS_SCHLACHTFELDFIEBER);
	for (i = 0, h = g_heroes; i <= 6; i++, h++) {
		if (h->type != 0 && !group_has_hero(g, h)) {
			h->tied = 0;
			set_asleep(h, 0);
			h->chamaelioni = 0;
			h->duplicatus = 0;
			h->scared = 0;
			h->blind = 0;
			h->ecliptifactus = 0;
			h->action_id = 12;
			h->fb5 = 0;
			h->fb6_b0 = 0;
			h->gardianum = 0;
			h->renegade = 0;
		}
	}
}

void group_set_fb5(struct group far *g, signed char v)
{
	struct hero far *h;
	signed short i;

	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h && !h->npc_id)
			h->set_fb5(v);
	g_key_last = 0;
}

void group_set_fb6_b0(struct group far *g, signed char v)
{
	struct hero far *h;
	signed short i;

	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i])
		if (h && !h->npc_id)
			h->set_fb6_b0(v);
}

signed short hand_weapon_class_melee(struct hero far *h)
{
	signed short id;
	signed short cls;

	id = h->inv[INV_SLOT_RIGHT_HAND].item_id;
	if (!g_items[id].weapon || h->inv[INV_SLOT_RIGHT_HAND].broken)
		cls = -1;
	else {
		cls = g_weapons_class[g_items[id].table_index].category;
		if (cls >= 4)
			cls = -1;
	}
	return cls;
}

signed short hand_weapon_class(struct hero far *h)
{
	signed short id;
	signed short cls;

	id = h->inv[INV_SLOT_RIGHT_HAND].item_id;
	if (!g_items[id].weapon || h->inv[INV_SLOT_RIGHT_HAND].broken)
		cls = -1;
	else {
		cls = g_weapons_class[g_items[id].table_index].category;
		if (cls < 4)
			cls = -1;
		if (g_items[id].table_index == 0x35)
			cls = -1;
	}
	return cls;
}

void set_asleep(struct hero far *h, signed char v)
{
	if (g_fight_obj == 0 && !v && h->asleep && !snd_busy(g_snd_fx, 0))
		snd_play(g_snd_fx, h->sex == 1 ? 0x6e : 0x6d);
	h->asleep = v;
}

void set_unconscious(struct hero far *h, signed char v)
{
	if (g_fight_obj == 0) {
		if (v && !h->unconscious) {
			if (!snd_busy(g_snd_fx, 0))
				snd_play(g_snd_fx, h->sex == 0 ? 0x62 : 0x63);
			h->duplicatus = 0;
			g_hero_died = 1;
		}
	} else if (v && !h->unconscious) {
		Lst_add2(g_fight_list, 7, 0, 2, h);
		h->duplicatus = 0;
	}
	h->unconscious = v;
}
