/* OVL151 - Overlay O151: Gegnerzauber - Zielart und Sinnpruefung (zu DSA1 seg037 get_foe_attack_mode).
 * get_foe_attack_mode(f, spell, attack): Zielsuche-Modus 0..3 aus g_mon_spell_descriptions[spell].mode, je nach
 *   attack (Gegner greift an: mode 3 -> 1, 1 -> 0, 2 -> 3, sonst 2; sonst mode 3 -> 0, 0 -> 2, sonst -1).
 * enemy_spell_target_ok(e, spell): Ziel des Gegners (enemy_id: 0 selbst, 1..9 Held, >= 10 Gegner) holen
 *   (enemy_target_enemy bzw. enemy_target_hero) und pruefen, ob der Zauber dort noch wirkt -> 1 = sinnvoll:
 *   Blitz nicht auf Geblendete/Fliehende, Horriphobus nicht auf Fliehende (Held auch nicht versteinert/gefesselt),
 *   Paralue nicht auf Versteinerte/Fliehende, Balsam nur bei LE < LE-Grundwert - 5, Axxeleratus nur bis 3 Aktionen,
 *   Saft Kraft nur ohne +0x39.4, Verwandlung beenden nur bei Versteinerung/Pilz, Boeser Blick nicht auf Abtruennige,
 *   Bannbaladin nicht auf Gezaehmte (hero.tame), beide nicht auf Fliehende.
 * Beide beginnen mit "if (g_quick_fight) game_idle();".
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL151.CPP   -> 782/782 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h, Welle N15: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * Schluessel: Bitfelder im Original "unsigned char b:1" (Header unsigned short): bloße Tests mit (unsigned char)-
 *   Cast -> "or al,al" statt "or ax,ax".
 *   [bp-0ch..0eh] sind -Og-Zwischenwerte fuer die Flag-Bytes +0xbd/+0xbc/+0x38 (nur Bytes, die irgendwo
 *   zweimal in einem Ausdruck vorkommen, bekommen einen; +0x39 nie).
 *   Verwandlung beenden: "!(signed char)e->petrified && !(signed char)e->mushroom" -> cbw / or ax,ax.
 *   if/else-if-Kette (kein switch); toter Epilog = return am Funktionsende.
 * Auffaelligkeit: Axxeleratus, Saft Kraft und Verwandlung beenden lesen das Ziel immer als Gegner (te), Bannbaladin
 *   und Boeser Blick immer als Held (th) - bei der anderen Zielart waere der Zeiger nicht gesetzt (die Zielarten der
 *   Zauber in g_mon_spell_descriptions schliessen das vermutlich aus).
 */
#include "schweif.h"


/* Zielsuche-Modus fuer den Gegnerzauber spell (BE get_foe_attack_mode; f unbenutzt) */
signed short get_foe_attack_mode(void far *f, signed short spell, signed short attack)
{
	struct mon_spell_desc far *desc;
	register signed short mode = -1;

	if (g_quick_fight) game_idle();
	desc = &g_mon_spell_descriptions[spell];
	if (attack) {
		if (desc->mode == 3) mode = 1;
		else if (desc->mode == 1) mode = 0;
		else if (desc->mode == 2) mode = 3;
		else mode = 2;
	} else {
		if (desc->mode == 3) mode = 0;
		else if (desc->mode == 0) mode = 2;
	}
	return mode;
}

/* wirkt der Gegnerzauber spell beim aktuellen Ziel des Gegners e noch? */
signed short enemy_spell_target_ok(struct enemy far *e, signed short spell)
{
	struct enemy far *te;
	struct hero far *th;
	signed short type;
	signed char target;
	register signed short ok = 1;

	if (g_quick_fight) game_idle();
	target = ENEMY_TARGET_TYPE(e);
	if (target == TARGET_ENEMY || target == TARGET_SELF) {
		te = enemy_target_enemy(e);
		type = TARGET_ENEMY;
	} else if (target == TARGET_HERO) {
		th = enemy_target_hero(e);
		type = TARGET_HERO;
	} else {
		return 0;
	}
	if (spell == MSP_BLITZ) {
		if (type == TARGET_ENEMY) {
			if (te->blind != 0 || (unsigned char)te->scared) ok = 0;
		} else {
			if (th->blind != 0 || (unsigned char)th->scared) ok = 0;
		}
	} else if (spell == MSP_HORRIPHOBUS) {
		if (type == TARGET_ENEMY) {
			if ((unsigned char)te->scared) ok = 0;
		} else {
			if ((unsigned char)th->scared || (unsigned char)th->petrified || (unsigned char)th->tied) ok = 0;
		}
	} else if (spell == MSP_PARALUE) {
		if (type == TARGET_ENEMY) {
			if ((unsigned char)te->petrified || (unsigned char)te->scared) ok = 0;
		} else {
			if ((unsigned char)th->petrified || (unsigned char)th->scared) ok = 0;
		}
	} else if (spell == MSP_BALSAM) {
		if (type == TARGET_ENEMY) {
			if (te->le >= te->le_max - 5) ok = 0;
		} else {
			if (th->le >= th->le_max - 5) ok = 0;
		}
	} else if (spell == MSP_AXXELERATUS) {
		if (te->attacks > 3) ok = 0;
	} else if (spell == MSP_SAFT_KRAFT) {
		if ((unsigned char)te->f39_b4) ok = 0;
	} else if (spell == MSP_VERWANDLUNG_BEENDEN) {
		if (!(signed char)te->petrified && !(signed char)te->mushroom) ok = 0;
	} else if (spell == MSP_BOESER_BLICK) {
		if ((unsigned char)th->renegade || (unsigned char)th->scared) ok = 0;
	} else if (spell == MSP_BANNBALADIN) {
		if ((unsigned char)th->tame || (unsigned char)th->scared) ok = 0;
	}
	return ok;
}
