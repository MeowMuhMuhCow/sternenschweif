/* OVL077 - Overlay O077 (Segment 0x20c7 in SCHWEIF.EXE): Heldenfunktionen. calc_bp(h): Bewegungspunkte (8, je nach
 * Last gegen wirksame KK * 50/75/100/110 weniger, Axxeleratus +4, Gegenstand 0x154 -1). change_attrib_normal(h, i, v):
 * Grundwert attrib[i].normal um v erhoehen (max. 20), dann change_attrib. change_attrib(h, i, v): Meldung
 * "steigt/sinkt" (Texte g_ttx[0x1a9/0x1aa/0x134+i/0x155/0x1a7], ausser bei g_quiet oder Bit dead), attrib[i].cur
 * += v, abgeleitete Werte neu (calc_atpa_base bei GE/IN/KK, add_mr_mod(h, 0) bei MU/KL/AG), g_refresh = 1 bei
 * Bild 14. set_attrib(h, i, v): attrib[i].cur = v, Rest wie change_attrib. hero_at/hero_pa(h): AT/PA der
 * aktuellen Waffenart weapon_type (at[]/pa[]) + at_mod/pa_mod +- attack_type - rs_be/2 (PA bei ungerader BE -1),
 * min. 0; hero_at_weapon/hero_pa_weapon(h, w) dasselbe fuer Waffenart w ohne Modifikator. Ziel enemy_id
 * (TARGET_ID_*: 0 selbst, 1..9 Held, 10..29 Gegner, 30.. Sondergegner, 0xff keins): hero_mr_target (Held),
 * hero_set_target (setzen ueber get_hero_index), enemy_mr_target (Gegner g_enemies), hero_set_enemy_target
 * (setzen ueber fight_obj_index), target_type (TARGET_*). hero_save/hero_load(h): Held mit Kennung "CHAR" in die
 * Spielstanddatei g_save_fd schreiben/lesen. hero_take_weapons(h): Waffen abgeben (Wert Preis * price_unit *
 * Anzahl zusammenzaehlen, drop_item; Munition 10 bzw. 13 bleibt bei der Fernwaffe 9/0x13/0x13d/0x100 bzw.
 * 12/0x101). hero_find_item_box/hero_find_key(h, id): Gegenstand id am Koerper oder im Behaelter 0xbf
 * bzw. 0x145 (box_has, Liste g_keyring_keys) suchen. can_use_item(h, id): darf der Heldentyp den Gegenstand benutzen
 * (Liste g_forbidden_items[typ], Sonderfall npc_id == 1). hero_mr_bonus(h): MR-Bonus aus Bits dunkelheit/flim_flam bzw.
 * Gegenstaenden (0x85 mit Stabzauberstufe >= 2 oder 0xf9 -> 3, 0x16 -> 2), max. 5.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL077.C   -> 2719/2719 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * - -Og: Epilogkopien/tote Epiloge (hero_mr_target/enemy_mr_target if/else-if mit returns, hero_set_target/
 *   hero_set_enemy_target if/else), Zwischenwerte in hero_take_weapons ([bp-8] Offset von h->inv[i], [bp-0ah]/
 *   [bp-0bh] Offset und Flagbyte von g_items[id]) und hero_mr_bonus (SI = Offset von h->inv[i], [bp-1] Byte +be).
 *   -Oi fuer strcat in change_attrib.
 * - hero_take_weapons: take/n im inneren Block deklarieren - sie liegen hinter den -Og-Zwischenwerten
 *   ([bp-0eh]/[bp-10h]).
 * - hero_at/hero_pa: Ergebnis per ?: in einer Zuweisung (Rechnung in ax, "mov dx,ax / pop ax / add ax,dx", ein
 *   gemeinsames mov cx,ax); mit if/else rechnet BCC in dx und speichert je Zweig.
 * - target_type: ein return mit verschachteltem ?: (mov al,5 / jmp / mov al,2 / jmp zum gemeinsamen Epilog).
 * - hero.enemy_id ist im Header signed char, dieses Modul liest es unsigned -> (unsigned char)-Casts.
 * - change_attrib_normal -> change_attrib mit char-Parameter (push ax ohne cbw); change_attrib/target_type
 *   stehen weiter unten -> far call.
 * - "(signed long)price * (signed long)(signed short)price_unit * (signed long)n" -> movsx/cbw/movsx eax,ax/
 *   imul edx.
 * - kk = (signed char)(a + b) -> add al / cbw; Variablen ohne Aufrufe (hero_at..hero_pa_weapon, hero_mr_bonus)
 *   landen in CX.
 * Eigenheit: hero_save/hero_load schreiben/lesen 0x5ef Bytes ab h + 0x0c (also 12 Bytes des naechsten Helden).
 */
#include <string.h>
#include <stdio.h>
#include <io.h>

#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:5988..5990, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */

signed short calc_bp(struct hero far *h)
{
	register signed short bp;
	register signed short kk;		/* wirksame KK */

	bp = 8;
	kk = (signed char)(h->attrib[ATTRIB_KK].cur + h->attrib[ATTRIB_KK].mod);
	if (h->load >= kk * 50)
		bp--;
	if (h->load >= kk * 75)
		bp -= 2;
	if (h->load >= kk * 100)
		bp -= 2;
	if (h->axxeleratus)
		bp += 4;
	if (h->load >= kk * 110)
		bp = 1;
	if (bp > 1 && get_item_pos(h, 0x154) != -1)
		bp--;
	return bp;
}

void change_attrib_normal(struct hero far *h, register signed short i, signed char v)
{
	if (h->attrib[i].normal + v > 20)
		v = 20 - h->attrib[i].normal;
	h->attrib[i].normal += v;
	change_attrib(h, i, v);
}

void change_attrib(struct hero far *h, signed short i, signed char v)
{
	char far *buf;

	if (g_quiet == 0 && !(signed char)h->dead) {
		buf = (char far *)mem_alloc(100L);
		if (v > 0) {
			sprintf(buf, g_ttx[0x1a9], hero_name_gen(h, 1), g_ttx[0x134 + i], v,
				v > 1 ? g_ttx[0x155] : "");
		} else {
			sprintf(buf, g_ttx[0x1aa], hero_name_gen(h, 1), g_ttx[0x134 + i], -v,
				v < -1 ? g_ttx[0x155] : "");
		}
		strcat(buf, g_ttx[0x1a7]);
		dlg_message(g_dialog, buf);
		mem_free(buf);
	}
	h->attrib[i].cur += v;
	if (i == ATTRIB_KK || i == ATTRIB_IN || i == ATTRIB_GE)
		calc_atpa_base(h);
	if (i == ATTRIB_KL || i == ATTRIB_MU || i == ATTRIB_AG)
		add_mr_mod(h, 0);
	if (g_pp20_index == 14)
		g_refresh = 1;
}

void set_attrib(struct hero far *h, register signed short i, signed char v)
{
	h->attrib[i].cur = v;
	if (i == ATTRIB_KK || i == ATTRIB_IN || i == ATTRIB_GE)
		calc_atpa_base(h);
	if (i == ATTRIB_KL || i == ATTRIB_MU || i == ATTRIB_AG)
		add_mr_mod(h, 0);
	if (g_pp20_index == 14)
		g_refresh = 1;
}

signed short hero_at(struct hero far *h)
{
	signed short val;

	val = h->weapon_type < 7 ? h->at[h->weapon_type] + h->at_mod + h->attack_type - h->rs_be / 2 :
		h->at[0] + h->at_mod + h->attack_type - h->rs_be / 2;
	if (val < 0)
		val = 0;
	return val;
}

signed short hero_pa(struct hero far *h)
{
	signed short val;

	val = h->weapon_type < 7 ? h->pa[h->weapon_type] + h->pa_mod - h->attack_type - h->rs_be / 2 :
		h->pa[0] + h->pa_mod - h->attack_type - h->rs_be / 2;
	if (h->rs_be & 1)
		val--;
	if (val < 0)
		val = 0;
	return val;
}

signed short hero_at_weapon(struct hero far *h, signed short wtype)
{
	signed short val;

	val = h->at[wtype] + h->attack_type - h->rs_be / 2;
	if (val < 0)
		val = 0;
	return val;
}

signed short hero_pa_weapon(struct hero far *h, signed short wtype)
{
	signed short val;

	val = h->pa[wtype] - h->attack_type - h->rs_be / 2;
	if (h->rs_be & 1)
		val--;
	if (val < 0)
		val = 0;
	return val;
}

/* Ziel hero.enemy_id (unsigned gelesen): 0 selbst, 1..9 Held (Index + 1), 10..29 Gegner, 30.. Sondergegner,
 * 0xff keins */
struct hero far *hero_mr_target(struct hero far *h)
{
	signed short id;

	if ((id = (unsigned char)h->enemy_id) == TARGET_ID_SELF)
		return h;
	else if ((unsigned char)h->enemy_id == TARGET_ID_NONE)
		return 0;
	else
		return &g_heroes[id - TARGET_ID_HERO];
}

void hero_set_target(struct hero far *h, struct hero far *target)
{
	if (target) {
		if (target == h)
			h->enemy_id = TARGET_ID_SELF;
		else
			h->enemy_id = get_hero_index(target) + TARGET_ID_HERO;
	} else {
		h->enemy_id = TARGET_ID_NONE;
	}
}

struct enemy far *enemy_mr_target(struct hero far *h)
{
	if ((unsigned char)h->enemy_id == TARGET_ID_NONE)
		return 0;
	else if (target_type(h) == TARGET_SPECIAL)
		return &g_enemies[(unsigned char)h->enemy_id - TARGET_ID_SPECIAL];
	else
		return &g_enemies[(unsigned char)h->enemy_id - TARGET_ID_ENEMY];
}

void hero_set_enemy_target(struct hero far *h, struct enemy far *e)
{
	if (e)
		h->enemy_id = fight_obj_index(e) + TARGET_ID_ENEMY;
	else
		h->enemy_id = TARGET_ID_NONE;
}

signed char target_type(struct hero far *h)
{
	return (unsigned char)h->enemy_id > 50 ? TARGET_NONE :
		(unsigned char)h->enemy_id == TARGET_ID_SELF ? TARGET_SELF :
		(unsigned char)h->enemy_id < TARGET_ID_ENEMY ?
			(&g_heroes[(unsigned char)h->enemy_id - TARGET_ID_HERO] == h ? TARGET_SELF : TARGET_HERO) :
		(unsigned char)h->enemy_id < TARGET_ID_SPECIAL ? TARGET_ENEMY : TARGET_SPECIAL;
}

/* Spielstand: Kennung "CHAR", Laenge, Daten ab +0x0c (Eigenheit: 0x5ef Bytes ab h + 0x0c, also bis in den
 * naechsten Helden) */
signed short hero_save(struct hero far *h)
{
	signed short len;

	len = 0x5ef;
	_write(g_save_fd, "CHAR", 4);
	_write(g_save_fd, &len, 2);
	_write(g_save_fd, h->ver, len);
	return 1;
}

signed short hero_load(struct hero far *h)
{
	signed long magic;
	signed short len;

	_read(g_save_fd, &magic, 4);
	if (magic == 0x52414843L) {		/* "CHAR" */
		_read(g_save_fd, &len, 2);
		_read(g_save_fd, h->ver, len);
		return 1;
	}
	return 0;
}

/* Waffen abgeben: Wert (Preis * price_unit * Anzahl) aller Waffen im Inventar, Munition bleibt bei der
 * passenden Fernwaffe (10 bei 9/0x13/0x13d/0x100, 13 bei 12/0x101) */
signed long hero_take_weapons(struct hero far *h)
{
	signed long sum;
	signed short slot;
	register signed short prev;		/* zuletzt gesehene Waffe */
	register signed short id;

	sum = 0;
	prev = 0;
	for (slot = 0; slot < NR_INV_SLOTS; slot++) {
		id = h->inv[slot].item_id;
		if (id != 0) {
			if (g_items[id].weapon) {
				if (prev == 0) {
					prev = id;
				} else {
					signed short take;
					signed short n;

					take = 1;
					if (!g_items[id].undropable) {
						if (id == 10) {
							if (prev == 9 || prev == 0x13 || prev == 0x13d || prev == 0x100)
								take = 0;
						} else if (id == 13) {
							if (prev == 12 || prev == 0x101)
								take = 0;
						}
					} else if (prev) {
						move_item(h, slot, get_item_pos(h, prev));
						prev = id;
						id = h->inv[slot].item_id;
					}
					if (take) {
						if (!g_items[id].undropable) {
							if (g_items[id].stackable)
								n = h->inv[slot].quantity;
							else
								n = 1;
							sum += (signed long)g_items[id].price * (signed long)(signed short)g_items[id].price_unit * (signed long)n;
							drop_item(h, slot, n);
						}
					}
				}
			}
		}
	}
	return sum;
}

/* Platz des Gegenstands id, sonst Platz des Behaelters 0xbf (Rezeptbuch), wenn er irgendeinen der 14 Eintraege
 * hat, sonst 0 */
signed short hero_find_item_box(struct hero far *h, signed short id)
{
	signed short res;
	register signed short i;
	register signed short pos;

	pos = get_item_pos(h, id);
	res = 0;
	if (pos != -1) {
		res = pos;
	} else {
		pos = get_item_pos(h, 0xbf);
		if (pos != -1) {
			for (i = 0; i <= 13; i++) {
				if (box_has(h, pos, i)) {
					res = pos;
					break;
				}
			}
		}
	}
	return res;
}

/* Platz des Gegenstands id, sonst Platz des Schluesselbunds 0x145, wenn er id enthaelt (Liste g_keyring_keys), sonst -1 */
signed short hero_find_key(struct hero far *h, signed short id)
{
	signed short res;
	register signed short i;
	register signed short pos;

	pos = get_item_pos(h, id);
	res = -1;
	if (pos != -1) {
		res = pos;
	} else {
		pos = get_item_pos(h, 0x145);
		if (pos != -1) {
			for (i = 0; i < 26; i++) {
				if (g_keyring_keys[i] == id && box_has(h, pos, i)) {
					res = pos;
					break;
				}
			}
		}
	}
	return res;
}

signed short can_use_item(struct hero far *h, register signed short id)
{
	if (h->npc_id == 1 && (id == 0x13d || id == 0x11b || id == 0x11d))
		return 1;
	else
		return is_in_word_array(id, g_forbidden_item_lists[h->type - 1]) == -1 ? 1 : 0;
}

/* MR-Bonus: Bit dunkelheit -5, Bit flim_flam +5, sonst Gegenstaende (0x85 bei Stabzauberstufe >= 2 oder 0xf9: +3, 0x16: +2),
 * hoechstens 5 */
signed short hero_mr_bonus(struct hero far *h)
{
	signed short bonus;
	signed short slot;

	bonus = 0;
	if (h->dunkelheit) {
		bonus = -5;
	} else if (h->flim_flam) {
		bonus = 5;
	} else {
		for (slot = 0; slot < NR_INV_SLOTS; slot++) {
			if ((h->inv[slot].item_id == 0x85 && h->staffspell_lvl >= 2) || h->inv[slot].item_id == 0xf9)
				bonus += 3;
			else if (h->inv[slot].item_id == 0x16)
				bonus += 2;
		}
	}
	if (bonus > 5)
		bonus = 5;
	return bonus;
}
