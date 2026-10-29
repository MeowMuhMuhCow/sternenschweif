/* OVL192 - Overlay O192 (SCHWEIF.EXE): Gegnerklasse Enemy (struct enemy, 0x49 Bytes, Tabelle g_enemies, 20 Eintraege;
 * Klasse in s_class.h, freie Namen enemy_* in s_proto.h).
 * Konstruktor/Destruktor leer (Enemy_ctor/Enemy_dtor). name: Gegnername g_mon_names[mon_id] (grammar_name).
 * load_pic(d): Bild NVF 0x4a Nr. gfx_id - 20 nach d laden (g_nvf_heads). is_gfx_*: Pruefungen der Kampfgrafik
 * (Bedeutung meist nur ueber die Klaenge belegt), is_fire (gfx 0x2b, Feuerwesen), type_check (Gegnerart 0x33),
 * is_humanoid/is_undead: Grafik in g_gfx_humanoid bzw. g_gfx_undead (is_in_byte_array). is_dead. damage_range(w,
 * &lo, &hi): Schadensspanne aus dem Wuerfelcode (dam1/dam2/shot_dam/throw_dam: Anzahl Bits 12..15, Wuerfel 1 = W6,
 * 2 = W20, 3 = W3, sonst W4, Zuschlag Low-Byte). target_hero/set_target_hero: Ziel-Held enemy_id 1..9 (0xff =
 * keins), target_enemy/set_target_enemy: Ziel-Gegner (enemy_id >= 10). sub_le(n, keep_renegade): Schaden (doppelt
 * im Schnellkampf), tot -> dead und Kampfende-Merker g_fight_end/g_in_fight/g_fight_flee_all; ohne keep_renegade
 * renegade loeschen. sub_ae(n). fig_get_enemy(n) (frei): Gegner zur Figur n (fig_list_find/fig_list_find_twofield,
 * Figur-ID gegen fighter_id). attack_damage(p, vs_enemy) (BE FIG_get_enemy_attack_damage): Trefferpunkte gegen einen
 * Helden (RS, Feuerwesen: Gegenstand 0x11e/Feuerbann halbieren, Waffe 0xc5 in Platz 10, Amulett 0xb6 gegen
 * Untote 0x19/0x3f/0x1f mit 5 % Bruch + Text g_ttx[0x15f], Boron-Schutz) bzw. gegen einen Gegner (vs_enemy).
 * range_damage(h): Fernangriff (Schuesse shots, sonst Wuerfe throws herunterzaehlen; Wurf W20 gegen at_value und
 * gegen das Mittel MU/GE/IN des Helden, Schaden aus shot_dam bzw. throw_dam minus RS). test_attrib3/test_spell:
 * Zauberprobe (3W20 per dice_roll, Eigenschaften attrib[].cur, Gegnerzauber g_mon_spell_descriptions, Zuschlag MR
 * des Ziels bei vs_mr). cast_spell(m): Zauber cur_spell (Probe, AE-Kosten cost, Fehlschlag: Text 0x109 aus
 * txt_load_list(g_font, 0x1f, 0x12c) nach g_text_output_buf, sonst lokales Spell-Objekt). range_kind: 4 Schuss
 * (shot_dam > 0) bzw. 5 Wurf. at_value/pa_value: AT/PA mit Abzuegen (group_mr_bonus == -5, blind, tied/dancing,
 * null bei petrified/busy/mushroom bzw. zusaetzlich asleep, PA nicht negativ). blocks: lebt bzw. Leiche bleibt
 * liegen (Gegnerart in g_mons_corpse_blocks, is_in_byte_array2).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL192.CPP   -> 3245/3245 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h, Welle N11: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * - C++: Klasse mit leerem Konstruktor (if (this == 0) new(0x49) + toter Test) und Destruktor (flags & 1 -> delete),
 *   Bitfelder im Original "unsigned char b:1" (hier (unsigned char)-Casts), inline-Setter set_shots/set_throws
 *   (Parameterkopien [bp-4]/[bp-6] in range_damage), lokales Spell-Objekt im inneren Block von cast_spell
 *   ([bp-2eh], Destruktor Spell_dtor(&s, 2) am Blockende).
 * - -Oi: memset(g_cast_spell_id, 0, 16), strcpy und _rotl(d & 0xf000, 4) / _rotl(d & 0x0f00, 8) (rol ax statt shr).
 * - damage_range: w ohne register -> CX (keine Aufrufe), n/s ohne register -> SI/DI, t/i im inneren Block -> DX;
 *   [bp-6] = -Og-Zwischenwert (signed char)m.
 * - target_hero/target_enemy: "&g_heroes[enemy_id - 1]" (imul, Basis addieren, dann -0x5ef); enemy_id unsigned.
 * - test_spell: n register (SI), r ohne register -> DI; "m += (signed char)(a + b)" -> add al / add [bp+0c],al;
 *   Gegnertyp-Ausdruck "(signed char)(enemy_id < 10 ? (enemy_id == 0 ? 5 : 2) : 1) == 1" wie in O128.
 * - Praedikate "return a == x || a == y;" -> mov ax,1 / jmp / xor ax,ax + toter Epilog.
 * - ds:2482 hat kein Public: g_ingame_timers[INGAME_TIMER_BORON_UNDEAD].
 */
#include <mem.h>
#include <string.h>
#include <stdlib.h>
#include "schweif.h"

#define ITEM_FIRE_PROTECT	0x11e	/* halbiert den Schaden von Feuerwesen */
#define ITEM_UNDEAD_AMULET	0xb6	/* schuetzt vor Untoten 0x19/0x3f/0x1f */
#define ITEM_WEAPON_C5		0xc5	/* in Platz 10: Schaden der Untoten -3 */
#define INV_SLOT_10		10
#define TTX_AMULET_BROKEN	0x15f	/* g_ttx: Amulett zerbrochen */
#define TXT_SPELL_FAILED	0x109	/* Texte 0x1f: Zauber misslungen */

Enemy::Enemy()
{
}

Enemy::~Enemy()
{
}

char far *Enemy::name()
{
	return grammar_name(g_mon_names[mon_id]);
}

void Enemy::load_pic(void far *dst)
{
	signed short tmp;

	((Nvf far *)g_nvf_heads)->load(0x4a, dst, 0, gfx_id - 20, 0, &tmp, &tmp, 0x140);
}

signed short Enemy::is_gfx_19_3f()
{
	return gfx_id == 0x19 || gfx_id == 0x3f;
}

signed short Enemy::is_gfx_1b()
{
	return gfx_id == 0x1b;
}

signed short Enemy::is_gfx_1f()
{
	return gfx_id == 0x1f;
}

signed short Enemy::is_gfx_15()
{
	return gfx_id == 0x15;
}

signed short Enemy::is_gfx_17_29()
{
	return gfx_id == 0x17 || gfx_id == 0x29;
}

signed short Enemy::is_gfx_24_18()
{
	return gfx_id == 0x24 || gfx_id == 0x18;
}

signed short Enemy::is_gfx_25_35_36_23()
{
	return gfx_id == 0x25 || gfx_id == 0x35 || gfx_id == 0x36 || gfx_id == 0x23;
}

signed short Enemy::is_gfx_2a()
{
	return gfx_id == 0x2a;
}

signed short Enemy::is_gfx_1c()
{
	return gfx_id == 0x1c;
}

signed short Enemy::is_gfx_42()
{
	return gfx_id == 0x42;
}

signed short Enemy::is_gfx_33_mon_7a()
{
	return gfx_id == 0x33 && mon_id == 0x7a;
}

signed short Enemy::is_fire()
{
	return gfx_id == 0x2b;
}

signed short Enemy::type_check()
{
	return mon_id == 0x33;
}

signed short Enemy::is_humanoid()
{
	if (is_in_byte_array(gfx_id, g_gfx_humanoid) == -1)
		return 0;
	else
		return 1;
}

signed short Enemy::is_undead()
{
	if (is_in_byte_array(gfx_id, g_gfx_undead) == -1)
		return 0;
	else
		return 1;
}

signed short Enemy::is_dead()
{
	if ((unsigned char)dead)
		return 1;
	else
		return 0;
}

/* Spanne des Wuerfelcodes w (0 dam1, 1 dam2, 2 shot_dam, sonst throw_dam) */
void Enemy::damage_range(signed short w, signed short far *lo, signed short far *hi)
{
	signed short code;
	signed char add;
	signed short n;
	signed short sides;

	code = w == 0 ? dam1 : (w == 1 ? dam2 : (w == 2 ? shot_dam : throw_dam));
	n = _rotl(code & 0xf000, 4);
	{
		register signed short t = _rotl(code & 0x0f00, 8);
		sides = t == 1 ? 6 : (t == 2 ? 20 : (t == 3 ? 3 : 4));
	}
	add = code;
	*lo = *hi = 0;
	{
		register signed short i;
		for (i = 0; i < n; i++) {
			(*lo)++;
			*hi += sides;
		}
	}
	*lo += add;
	*hi += add;
}

struct hero far *Enemy::target_hero()
{
	if ((unsigned char)enemy_id == TARGET_ID_NONE)
		return 0;
	else
		return &g_heroes[(unsigned char)enemy_id - TARGET_ID_HERO];
}

void Enemy::set_target_hero(struct hero far *h)
{
	if (h)
		enemy_id = get_hero_index(h) + TARGET_ID_HERO;
	else
		enemy_id = TARGET_ID_NONE;
}

Enemy far *Enemy::target_enemy()
{
	if ((unsigned char)enemy_id == TARGET_ID_NONE)
		return 0;
	else
		return (Enemy far *)&g_enemies[(unsigned char)enemy_id - TARGET_ID_ENEMY];
}

void Enemy::set_target_enemy(struct enemy far *e)
{
	if (e)
		enemy_id = fight_obj_index(e) + TARGET_ID_ENEMY;
	else
		enemy_id = TARGET_ID_NONE;
}

void Enemy::sub_le(register signed short n, signed short keep_renegade)
{
	if (g_quick_fight)
		n *= 2;
	le -= n;
	if (le <= 0) {
		dead = 1;
		le = 0;
		if (g_fight_end == 5) {
			g_in_fight = 0;
			g_fight_end = 0;
		}
		if (g_fight_end == 2)
			g_fight_flee_all = 1;
	}
	if (!keep_renegade)
		renegade = 0;
}

void Enemy::sub_ae(signed short n)
{
	if (!(signed char)dead) {
		ae -= n;
		if (ae < 0)
			ae = 0;
	}
}

/* Gegner zur Figur n (Schwanz eines Zweifeldgegners: Kopf) */
struct enemy far *fig_get_enemy(unsigned char n)
{
	signed char id;
	struct enemy far *e;
	struct fig_node far *fig;
	register signed short i;

	fig = fig_list_find(g_fig_list, n);
	if (fig->twofielded > 20 && fig->twofielded != FIG_TWOFIELD_NONE) {
		fig = fig_list_find_twofield(g_fig_list, fig->twofielded - 20);
		id = fig->id;
	} else {
		id = fig->id;
	}
	for (i = 0; i < NR_ENEMIES; i++) {
		e = &g_enemies[i];
		if (e->fighter_id == id)
			return e;
	}
	return 0;
}

/* Trefferpunkte gegen einen Helden (vs_enemy 0) bzw. Gegner (BE FIG_get_enemy_attack_damage) */
signed short Enemy::attack_damage(void far *target, signed short vs_enemy)
{
	signed short pos;
	struct hero far *h;
	register signed short dmg;
	register signed short code;

	code = dam1;
	if (dam2 && random_schweif(100) < 50)
		code = dam2;
	dmg = dice_code_roll(code);
	if (!vs_enemy) {
		h = (struct hero far *)target;
		if (!is_fire()) {
			dmg -= (signed char)(h->rs_bonus + h->rs_bonus2);
		} else {
			/* Feuer: Ruestung hilft nicht, Schutzgegenstand/Feuerbann halbieren */
			if (get_item_pos(h, ITEM_FIRE_PROTECT) != -1)
				dmg /= 2;
			if (h->fireban)
				dmg /= 2;
		}
		if (h->inv[INV_SLOT_10].item_id == ITEM_WEAPON_C5 && (is_gfx_1f() || is_gfx_19_3f()))
			dmg -= 3;
		if ((pos = get_item_pos(h, ITEM_UNDEAD_AMULET)) != -1 && (is_gfx_1f() || is_gfx_19_3f())) {
			dmg = 0;
			if (random_schweif(100) < 5) {
				drop_item(h, pos, 1);
				dlg_message(g_dialog, g_ttx[TTX_AMULET_BROKEN]);
			}
		}
		dmg += saftkraft;
		if ((unsigned char)h->petrified)
			dmg = 0;
		if (g_ingame_timers[INGAME_TIMER_BORON_UNDEAD] && is_undead())
			dmg /= 2;
	} else {
		dmg += saftkraft;
		dmg -= ((Enemy far *)target)->rs;
		if ((unsigned char)((Enemy far *)target)->petrified)
			dmg = 0;
		if (((Enemy far *)target)->magic && !magic)
			dmg = 0;
	}
	if (broken)
		dmg /= 2;
	return dmg;
}

/* Fernangriff gegen einen Helden: Treffer gegen AT, Ausweichen gegen das Mittel MU/GE/IN */
signed short Enemy::range_damage(struct hero far *h)
{
	signed short dodge;
	register signed short dmg;
	register signed short kind;

	if (shots > 0) {
		kind = 4;
		set_shots(shots - 1);
	} else {
		kind = 5;
		set_throws(throws - 1);
	}
	if (random_schweif(20) <= at_value()) {
		dodge = ((signed char)(h->attrib[ATTRIB_MU].cur + h->attrib[ATTRIB_MU].mod) +
			(signed char)(h->attrib[ATTRIB_IN].cur + h->attrib[ATTRIB_IN].mod) +
			(signed char)(h->attrib[ATTRIB_GE].cur + h->attrib[ATTRIB_GE].mod)) / 4;
		if (random_schweif(20) > dodge) {
			dmg = dice_code_roll(kind == 4 ? shot_dam : throw_dam);
			dmg -= (signed char)(h->rs_bonus + h->rs_bonus2);
			if ((unsigned char)h->petrified)
				dmg = 0;
		} else {
			dmg = 0;
		}
	} else {
		dmg = 0;
	}
	return dmg;
}

/* Probe auf drei Eigenschaften (3W20, Zuschlag mod); Ergebnis Punkte uebrig + 1 */
signed short Enemy::test_attrib3(register signed short a1, register signed short a2, signed short a3, signed char mod)
{
	signed short roll;
	signed short sum;

	roll = dice_roll(3, 20, mod);
	sum = attrib[a1].cur + attrib[a2].cur + attrib[a3].cur;
	return sum - roll + 1;
}

/* Zauberprobe fuer den Gegnerzauber n (gegen MR des Ziels bei vs_mr); -1 bei Rondra-Wunder */
signed short Enemy::test_spell(register signed short n, signed char mod)
{
	struct mon_spell_desc far *sp;
	struct enemy far *te;
	struct hero far *th;
	signed short r;

	r = 0;
	if (g_ingame_timers[INGAME_TIMER_RONDRA_NO_SPELLS]) {
		r = -1;
	} else {
		sp = &g_mon_spell_descriptions[n];
		if (sp->vs_mr) {
			if ((signed char)((unsigned char)enemy_id < TARGET_ID_ENEMY ?
			    ((unsigned char)enemy_id == 0 ? TARGET_SELF : TARGET_HERO) : TARGET_ENEMY) == TARGET_ENEMY) {
				te = target_enemy();
				mod += te->mr;
			} else {
				th = target_hero();
				mod += (signed char)(th->mr + th->mr_mod);
			}
		}
		if (n >= 1 && n <= 14)
			r = test_attrib3(sp->attrib1, sp->attrib2, sp->attrib3, mod);
	}
	return r;
}

/* Zauber cur_spell wirken; -1 = AE reicht nicht bzw. gesperrt */
signed short Enemy::cast_spell(signed char mod)
{
	signed short n;
	signed short ae_cost;
	signed short r;
	signed short res;
	void far *raw;
	char far * far *txt;

	r = 0;
	memset(g_cast_spell_id, 0, 16);
	n = cur_spell;
	if (n > 0) {
		ae_cost = mon_spell_cost(n, 0);
		if (ae < ae_cost) {
			r = -1;
			g_spell_cancelled = 1;
		}
		if (r != -1) {
			res = test_spell(n, mod);
			if (res <= 0) {
				/* misslungen: halbe Kosten, Meldung */
				sub_ae(mon_spell_cost(n, 1));
				txt_load_list(g_font, 0x1f, 0x12c, &raw, &txt);
				strcpy(g_text_output_buf, txt[TXT_SPELL_FAILED]);
				mem_free(raw);
				mem_free(txt);
			} else {
				Spell s(this);
				r = s.cast(n);
			}
		}
	}
	return r;
}

signed short Enemy::range_kind()
{
	register signed short r = -1;

	if (shot_dam > 0)
		r = 4;
	else
		r = 5;
	return r;
}

signed char Enemy::at_value()
{
	signed char v;

	v = at;
	if (group_mr_bonus(g_group) == -5)
		v -= 4;
	if (blind)
		v -= 6;
	if ((unsigned char)tied)
		v -= 2;
	else if ((unsigned char)dancing)
		v -= 5;
	if ((unsigned char)petrified || (unsigned char)busy || (unsigned char)mushroom)
		v = 0;
	return v;
}

signed char Enemy::pa_value()
{
	signed char v;

	v = pa;
	if (group_mr_bonus(g_group) == -5)
		v -= 4;
	if (blind)
		v -= 6;
	if ((unsigned char)tied)
		v -= 2;
	else if ((unsigned char)dancing)
		v -= 5;
	if ((unsigned char)petrified || (unsigned char)busy || (unsigned char)mushroom || (unsigned char)asleep)
		v = 0;
	if (v < 0)
		v = 0;
	return v;
}

signed short Enemy::blocks()
{
	register signed short r;

	if ((unsigned char)dead) {
		if (is_in_byte_array2(mon_id, g_mons_corpse_blocks) == -1)
			r = 0;
		else
			r = 1;
	} else {
		r = 1;
	}
	return r;
}
