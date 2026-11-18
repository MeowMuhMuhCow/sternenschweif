/* SEG010 - Zeitgeber: Spielzeit-Zaehler verteilen, stuendliche/taegliche Ereignisse, Helden-Zeitgeber
 * (seg010 in SCHWEIF.EXE). Die Zaehler g_timer_* zaehlt time_tick (R21) hoch (Ticks, 5 Minuten, 15 Minuten,
 * Stunden, 4 Stunden, Tage), die Merker g_at_0800/g_at_1000/g_at_1400 setzt er zur jeweiligen Uhrzeit.
 * do_timers(lock): nur ausserhalb von Dialogen und nicht verschachtelt (*lock); sub_timers (Zeitzaehler
 * g_ingame_timers[50] um die Ticks senken), bei vergangenen 5 Minuten timers_events, timers_hourly/
 * timers_4hourly/timers_daily (O005) und heroes_tick, danach alle Zaehler 0. timers_events: um 8 Uhr
 * O005-Tagesereignisse und Zaehler ds:25f9/25fa, Ereignisse je Gruppenstandort. heroes_tick: je Held
 * (Typ != 0) Rausch um 10 Uhr (hero_get_sober), effects_tick + hero_timers je 5 Minuten, sub_light_timers je
 * 15 Minuten, Hunger/Durst je Stunde, Krankheiten je Tag. hero_timers: Gift je 5 Minuten (poison_effect),
 * Schaden bei g_c6c6 > 50 ausserhalb des Kampfs, heal_timer/staffspell_timer (450 Ticks je Einheit), Sucht.
 * sub_light_timers (BE): brennende Fackel (Ablage bei Ende) bzw. Laterne (-> ITEM_LANTERN_OFF) je 15 Minuten.
 *
 * C++! BCC -mlarge -3 -Ot -Og -c -Y SEG010.CPP   -> 1341/1341 Bytes GLEICH (18.10.2026)
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * Schluessel war sub_light_timers: die Originalquelle setzt Felder ueber inline-Memberfunktionen von
 * struct hero (set_inv_light_timer/set_inv_item_id). Ihre Parameterkopien sind die Locals [bp-2]/[bp-3],
 * [bp-6]/[bp-7], [bp-0Ah], [bp-0Ch]; [bp-0Eh] ist der -Og-Zwischenwert i*27. Attic hat also (zumindest hier)
 * C++ benutzt.
 */
#include "schweif.h"

void sub_timers(signed short far *lock)
{
	signed short i;

	for (i = 0; i < 50; i++) {
		g_ingame_timers[i] -= g_timer_ticks;
		if (g_ingame_timers[i] < 0)
			g_ingame_timers[i] = 0;
	}
}

void timers_events(signed short far *lock)
{
	if (g_at_0800) {
		inn_days_tick();
		npc_pay_tick();
		jail_tick();
		if (g_peddler_away) g_peddler_away--;
		if (g_peddler3_away) g_peddler3_away--;
		if (g_low_stone_days == 8) show_daily_hint();
		g_at_0800 = 0;
	}
	if (g_group->town != 4 && g_at_1400)
		g_at_1400 = 0;
	if (g_dng13_smoke_timer)
		dng13_smoke_damage();
	if (g_group->town == 3 && g_finster_anger < 10 && g_finster_anger != 0)
		g_finster_anger++;
	if (g_dng01_ice_state != 0 && g_dng01_ice_state < 3)
		g_dng01_ice_state++;
	if (g_dng16_forge_timer)
		g_dng16_forge_timer--;
	if (g_dng16_wall_timer) {
		if (g_dng16_wall_timer == 1) {
			if (g_group->dungeon == 0x10) {
				event_set_cell(g_area_events, 10, 11, 14, 0, -1);
				automap_cell_write(g_map, 0x312, 11, 10, 0x23);
				automap_cell_write(g_map, 0x312, 12, 10, 2);
				g_dng16_wall_timer = 0;
			}
			return;
		}
		g_dng16_wall_timer--;
	}
}

void hero_timers(signed short far *lock, struct hero far *hero)
{
	signed long i;
	signed long t1;
	signed long t2;

	for (i = 0; i < g_timer_5min; i++) {
		if (!(signed char)hero->dead) {
			if (hero_is_poisoned(hero) || hero_is_poisoned1(hero)) {
				poison_effect(hero);
				if (g_pp20_index == 0xe || g_pp20_index == 0xf)
					g_refresh = 1;
			}
			if (g_c6c6 > 50 && g_in_fight == 0) {
				sub_hero_le(hero, 5);
				sub_le_max(hero, 5);
			}
		}
	}
	if (hero->heal_timer) {
		t1 = g_timer_5min * 450;
		if (hero->heal_timer > t1)
			hero->heal_timer -= t1;
		else
			hero->heal_timer = 0;
	}
	if (hero->staffspell_timer) {
		t2 = g_timer_5min * 450;
		if (hero->staffspell_timer > t2)
			hero->staffspell_timer -= t2;
		else
			hero->staffspell_timer = 0;
	}
	if (!!hero->fur_hands)
		hero_fur_tick(hero);
}

void sub_light_timers(signed short far *lock, struct hero far *hero)
{
	signed short i;

	for (i = 0; i < NR_INV_SLOTS; i++) {
		if (hero->inv[i].item_id == ITEM_TORCH_ON) {
			hero->set_inv_light_timer(i, hero->inv[i].lighting_timer - g_timer_15min);
			if (hero->inv[i].lighting_timer <= 0) {
				drop_item(hero, i, 1);
				if (g_pp20_index == 0xe || g_pp20_index == 0xf)
					g_refresh = 1;
			}
		} else if (hero->inv[i].item_id == ITEM_LANTERN_ON) {
			hero->set_inv_light_timer(i, hero->inv[i].lighting_timer - g_timer_15min);
			if (hero->inv[i].lighting_timer <= 0) {
				hero->set_inv_light_timer(i, 0);
				hero->set_inv_item_id(i, ITEM_LANTERN_OFF);
				if (g_pp20_index == 0xe || g_pp20_index == 0xf)
					g_refresh = 1;
			}
		}
	}
}

void heroes_tick(signed short far *lock)
{
	struct hero far *hero;
	signed short i;

	for (i = 0, hero = g_heroes; i <= 6; i++, hero++) {
		if (hero->type == HERO_TYPE_NONE)
			continue;
		if (g_at_1000 && hero->drunk)
			hero_get_sober(hero);
		if (g_timer_5min) {
			effects_tick(hero, g_timer_ticks);
			hero_timers(lock, hero);
		}
		if (g_timer_15min)
			sub_light_timers(lock, hero);
		if (g_timer_hours)
			hero_hunger_thirst(lock, hero);
		if (g_timer_days)
			hero_disease_tick(lock, hero);
	}
}

void do_timers(signed short far *lock)
{
	if (g_dialog_depth == 0 && *lock == 0) {
		*lock = 1;
		sub_timers(lock);
		if (g_timer_5min) {
			timers_events(lock);
			if (g_timer_hours) timers_hourly(lock);
			if (g_timer_4hours) timers_4hourly(lock);
			if (g_timer_days) timers_daily(lock);
			heroes_tick(lock);
			g_timer_5min = g_timer_15min = g_timer_hours = g_timer_4hours = g_timer_days = 0;
			g_at_1000 = 0;
		}
		g_timer_ticks = 0;
		*lock = 0;
	}
}
