/* OVL079 - Overlay O079 (Segment 0x20d8 in SCHWEIF.EXE): Kampf - Helden waehlen und Waffenschaden (entspricht
 * DSA1 seg041 FIG_choose_next_hero/FIG_get_first_active_hero/FIG_get_hero_weapon_attack_damage).
 * fig_choose_next_hero(g): bei g_fight_end == 5 der erste aktive Held (get_first_active_hero), falls er noch
 *   Aktionen hat, sonst so lange get_random_hero2(g), bis einer Aktionen hat.
 * fig_get_first_active_hero(g): erster der 7 Helden, der nicht tot/versteinert/abtruennig/bewusstlos ist und auf
 *   dem Kampffeld steht (fight_find_cell(Index + 1, &x, &y)), sonst 0.
 * hero_attack_damage(hero, target, kind) (kind 0/2 = Gegner, 1 = Held): Schaden aus g_weapons_table (W6 + Zuschlag),
 *   Fernwaffen mit Entfernungsstufe (manhattan_dist) und g_ranged_weapons_table (Probe test_skill TA_WURFWAFFEN bzw.
 *   TA_SCHUSSWAFFEN, misslungen -> 0), Nahkampf KK-Bonus; Sonderwaffen (magischer Saebel/Silberstreitkolben gegen
 *   Untote, Kukrisdolch/-mengbilar werden zu Dolch/Mengbilar, 0x15c gegen Gegnerart 0x33), Saftkraft, Gift im
 *   Waffenplatz (rechte Hand, bei Schusswaffen die Munition links) mit zwei switch-Tabellen, Ecliptifactus
 *   verdoppelt; dann RS (kind 0) bzw. +W10 (kind 2), Versteinerung, magische Gegner nur mit magischer Waffe,
 *   hoechstens LE + 1.
 * hero_is_inactive(h): 1 = ohne Typ, tot, schlafend oder bewusstlos.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL079.CPP   -> 2095/2095 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h, Welle N15: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * - -Og noetig: Zwischenwerte [bp-9] (Byte +bc in fig_get_first_active_hero), DL in hero_is_inactive,
 *   [bp-36h..3ah] (slot, slot*27, Adresse), [bp-3ch]/[bp-3eh] (LE vor "le + 1").
 * - Inline-Setter set_inv_poison_charges(i, v) / clr_inv_poison_type(i) (Held) und set_bp_max(v) (Gegner) ->
 *   Parameterkopien [bp-2eh]/[bp-2fh], [bp-32h], [bp-33h].
 * - Bitfelder im Original "unsigned char b:1": bloße Tests mit (unsigned char)-Cast, Negationen mit (signed char).
 * - set_asleep (Prototyp signed char) wird mit einem Wort aufgerufen -> gecasteter Funktionszeiger.
 * - Grenzen "if (x > 10) x = 10; else ;" - das leere else erzeugt den toten "jmp short" auf die Folgezeile.
 * - Summen zweier Bytes (KK + Mod, RS + RS-Bonus) als (signed char)(a + b) -> add al / cbw.
 * - Fernkampf-Schaden als if/else (kein ?:), Talent-Argument dagegen als ?: (8 : 7).
 * - switch-Faelle in Quellreihenfolge (10, 1/6/11, 8 bzw. 10, 11, 1, 2, 4, 6, 8, 3), je mit "default: break;".
 * - hero_is_inactive: "if (...) return 1; else return 0;" -> zwei tote Epiloge.
 * Auffaelligkeit: Arax senkt beim Gegner bp_max um 2 und begrenzt dann nach oben (> 10 -> 10) statt nach unten.
 */
#include "schweif.h"

#define DMG_VS_ENEMY	0		/* hero_attack_damage: Ziel Gegner (RS zieht ab) */
#define DMG_VS_HERO	1		/* Ziel Held */
#define DMG_VS_ENEMY2	2		/* Ziel Gegner, statt RS +W10 (vermutet: Sonderkampf) */

#define ITEM_15C		0x15c	/* Waffe mit +10 gegen Gegnerart 0x33 (unbekannt) */

/* naechster Held mit Aktionen (BE FIG_choose_next_hero) */
struct hero far *fig_choose_next_hero(struct group far *g)
{
	struct hero far *h;

	if (g_fight_end == 5) {
		h = get_first_active_hero(g_group);
		if (h->actions != 0)
			return h;
	}
	do {
		h = get_random_hero2(g);
		if (g_quick_fight)
			game_idle();
	} while (!h->actions);
	return h;
}

/* erster handlungsfaehiger Held auf dem Kampffeld (BE FIG_get_first_active_hero) */
struct hero far *fig_get_first_active_hero(struct group far *g)
{
	signed short x;
	signed short y;
	struct hero far *h;
	register signed short i;

	if (g_quick_fight)
		game_idle();
	for (i = 0, h = g->heroes[0]; i <= 6; h = g->heroes[++i]) {
		if (h && !(signed char)h->dead && !(signed char)h->petrified && !(signed char)h->renegade &&
		    !(signed char)h->unconscious && fight_find_cell(get_hero_index(h) + 1, &x, &y))
			return h;
	}
	return 0;
}

/* Trefferpunkte des Helden hero mit seiner Waffe gegen target (BE FIG_get_hero_weapon_attack_damage) */
signed short hero_attack_damage(struct hero far *hero, void far *target, signed short kind)
{
	struct weapon_desc far *p_wpn;
	struct ranged_weapon far *p_rng;
	register signed short damage;
	signed short damage_mod = 0;
	signed short target_size;
	signed short right_hand;
	signed short beeline;
	signed short distance;
	signed short x_hero;
	signed short y_hero;
	signed short x_target;
	signed short y_target;
	signed short hero_idx;
	struct enemy far *enemy_p;
	struct hero far *hero_t;
	signed short weapon_type;
	signed char slot = INV_SLOT_RIGHT_HAND;
	signed short flag;
	signed short poison = 0;

	if (g_quick_fight)
		game_idle();
	if (kind == DMG_VS_ENEMY || kind == DMG_VS_ENEMY2)
		enemy_p = (struct enemy far *)target;
	else
		hero_t = (struct hero far *)target;

	right_hand = hero->inv[INV_SLOT_RIGHT_HAND].item_id;
	weapon_type = hero_weapon_type2(hero);
	if (weapon_type != -1) {
		p_wpn = &g_weapons_table[g_items[right_hand].table_index];
		damage = dice_roll(p_wpn->dmg_d6, 6, p_wpn->dmg_add);
		if (p_wpn->ranged != -1) {
			hero_idx = get_hero_index(hero);
			fight_find_cell(hero_idx + 1, &x_hero, &y_hero);
			fight_find_cell(hero->enemy_id, &x_target, &y_target);
			beeline = manhattan_dist(x_hero, y_hero, x_target, y_target);
			if (beeline <= 2)
				distance = 0;
			else if (beeline <= 4)
				distance = 1;
			else if (beeline <= 6)
				distance = 2;
			else if (beeline <= 9)
				distance = 3;
			else if (beeline <= 15)
				distance = 4;
			else if (beeline <= 20)
				distance = 5;
			else
				distance = 6;
			p_rng = &g_ranged_weapons_table[p_wpn->ranged];
			if (kind == DMG_VS_HERO) {
				if (hero_t->type == HERO_TYPE_DWARF)
					target_size = 2;
				else
					target_size = 3;
			} else {
				target_size = enemy_p->size;
			}
			if (test_skill(hero, g_items[right_hand].subtype == TA_WURFWAFFEN ? TA_WURFWAFFEN : TA_SCHUSSWAFFEN,
					p_rng->handicap + 2 * distance - 2 * target_size) > 0)
				damage_mod = g_ranged_weapons_table[p_wpn->ranged].dmg_mod[distance];
			else
				damage_mod = -damage;
			damage += damage_mod;
			if (g_items[right_hand].subtype == TA_SCHUSSWAFFEN)
				slot = INV_SLOT_LEFT_HAND;	/* Munition */
		} else {
			damage_mod = (signed char)(hero->attrib[ATTRIB_KK].cur + hero->attrib[ATTRIB_KK].mod) - p_wpn->kk_bonus;
			if (damage_mod > 0)
				damage += damage_mod;
		}
	} else {
		damage = random_schweif(6);
	}

	if (g_quick_fight)
		game_idle();
	if (kind == DMG_VS_ENEMY || kind == DMG_VS_ENEMY2) {
		if (right_hand == ITEM_SABER_MAGIC && (enemy_is_gfx_19_3f(enemy_p) || enemy_is_gfx_1f(enemy_p))) {
			damage++;
		} else if (right_hand == ITEM_KUKRIS_DAGGER) {
			if (enemy_type_check(enemy_p))
				damage = random_interval(4, 0x19);
			else
				damage = random_interval(0x14, 0x63);
			hero->inv[INV_SLOT_RIGHT_HAND].item_id = ITEM_DAGGER;
		} else if (right_hand == ITEM_KUKRIS_MENGBILAR) {
			if (enemy_type_check(enemy_p))
				damage = random_interval(4, 0x19);
			else
				damage = random_interval(0x14, 0x63);
			hero->inv[INV_SLOT_RIGHT_HAND].item_id = ITEM_MENGBILAR;
		} else if (right_hand == ITEM_SILVER_MACE && enemy_is_gfx_19_3f(enemy_p)) {
			damage += 4;
		} else if (right_hand == ITEM_15C && enemy_type_check(enemy_p)) {
			damage += 10;
		}
	}
	damage += hero->saftkraft;

	if (g_quick_fight)
		game_idle();
	flag = 0;
	if (hero->inv[slot].poison_charges > 0) {
		poison = hero->inv[slot].poison_type;
		if (((kind == DMG_VS_ENEMY || kind == DMG_VS_ENEMY2) && enemy_p->rs < damage) ||
		    (kind == DMG_VS_HERO && (signed char)(hero_t->rs_bonus + hero_t->rs_bonus2) < damage))
			flag = 1;
		hero->set_inv_poison_charges(slot, hero->inv[slot].poison_charges - 1);
		if (hero->inv[slot].poison_charges <= 0)
			hero->clr_inv_poison_type(slot);
	}

	if (flag && poison) {
		if ((kind == DMG_VS_ENEMY || kind == DMG_VS_ENEMY2) && enemy_type_check(enemy_p)) {
			switch (poison) {
			case 10:	/* (Gift 10/11: Art unbekannt) */
				damage += random_interval(1, 5);
				break;
			case POISON_SHURINKNOLLE:
			case POISON_KROETENSCHEMEL:
			case 11:
				damage++;
				break;
			case POISON_KUKRIS:
				damage += random_interval(4, 0x19);
				break;
			default:
				break;
			}
		} else {
			switch (poison) {
			case 10:
				damage += random_interval(5, 0x19);
				break;
			case 11:
				damage += random_interval(3, 8);
				break;
			case POISON_SHURINKNOLLE:
				damage += random_interval(1, 6);
				break;
			case POISON_ARAX:
				if (kind == DMG_VS_ENEMY || kind == DMG_VS_ENEMY2) {
					enemy_p->set_bp_max(enemy_p->bp_max - 2);
#if FIX_0213
					/* Fix #213: nach unten begrenzen (mindestens 1 BP) */
					if (enemy_p->bp_max < 1)
						enemy_p->bp_max = 1;
#else
					if (enemy_p->bp_max > 10)
						enemy_p->bp_max = 10;
#endif
					else
						;
					enemy_p->at--;
					if (enemy_p->at < 0)
						enemy_p->at = 0;
					else
						;
					enemy_p->pa--;
					if (enemy_p->pa < 0)
						enemy_p->pa = 0;
					else
						;
				} else {
					damage += random_interval(1, 6);
				}
				break;
			case POISON_SCHLAFGIFT:
				if (kind == DMG_VS_ENEMY || kind == DMG_VS_ENEMY2)
					enemy_p->asleep = 1;
				else
					((void (*)(struct hero far *, signed short))set_asleep)(hero_t, 1);
				break;
			case POISON_KROETENSCHEMEL:
				damage += random_interval(1, 3);
				break;
			case POISON_KUKRIS:
				damage += random_interval(0x14, 0x63);
				break;
			case POISON_ANGSTGIFT:
				if (kind == DMG_VS_ENEMY || kind == DMG_VS_ENEMY2) {
					enemy_p->scared = 1;
					enemy_p->renegade = 0;
				} else {
					hero_t->scared = 1;
				}
				break;
			default:
				break;
			}
		}
	}

	if (hero->ecliptifactus)
		damage *= 2;

	if (kind == DMG_VS_ENEMY || kind == DMG_VS_ENEMY2) {
		if (kind == DMG_VS_ENEMY)
			damage -= enemy_p->rs;
		else
			damage += random_schweif(10);
		if ((unsigned char)enemy_p->petrified)
			damage = 0;
		if (enemy_p->magic != 0 && !hero->inv[INV_SLOT_RIGHT_HAND].magic)
			damage = 0;
		if (enemy_p->le < damage)
			damage = enemy_p->le + 1;
	} else {
		damage -= (signed char)(hero_t->rs_bonus + hero_t->rs_bonus2);
		if ((unsigned char)hero_t->petrified)
			damage = 0;
		if (hero_t->le < damage)
			damage = hero_t->le + 1;
	}

	if (g_quick_fight)
		game_idle();
	return damage;
}

/* Held kann nicht handeln */
signed short hero_is_inactive(struct hero far *h)
{
	if (!h->type || (unsigned char)h->dead || (unsigned char)h->asleep || (unsigned char)h->unconscious)
		return 1;
	else
		return 0;
}
