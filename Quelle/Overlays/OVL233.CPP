/* OVL233 - Overlay O233 (Segment 0x240c in SCHWEIF.EXE): Zaubern der Helden, Klasse Cast (0x22 Bytes, s_class.h;
 * Gegenstueck zu O194 Spell; lokal in use_spell O091, ItemUse O047, O013). spell_class_first(id): erster Zauber der
 * Zauberklasse (g_spells_index). hero_spell_cost(id, half) (BE get_spell_cost): ae_cost + g_spell_cost_mod (half:
 * -1 -> W(5..10), ab 2 halbiert). select_magic_user: Held waehlen (g_ttx[0x146]) -> use_spell(h, 1, 0).
 * Konstruktor: Zaubertexte (Datei 0x1f bzw. Platzhalter g_b690 im Schnellkampf), result = SPELL_RES_DEFAULT.
 * Destruktor: Texte freigeben (nur wenn txt_buf != 0). heal_ani: 5 Bilder g_heal_ani_frames am Portraet des Ziels
 * (Bmp 32x32, Hintergrund retten/zurueck). damage (BE FIG_do_spell_damage), target_pa (BE get_attackee_parade -
 * nur Kraehenruf), target_rs. cast(id, handicap): Pruefungen (Heldentyp >= Hexe mit check_hero oder renegade,
 * Zauberwert >= -5, AE; entfallen bei g_spell_flags.by_item), Ziel ueber target_type, im Sonderkampf
 * FIGHT_ID_SPECIAL Liste g_spells_no_sfight, sonst g_spells_own_test (ohne Probe) oder test_spell, dann
 * Zauberroutine g_spell_handlers[id] (Methodenzeiger, 8 Bytes); Sprachausgabe/Klang, Kostenabrechnung
 * (sub_ae_splash, Stabzauber ab Stufe 4 -2), Meldung und heal_ani. Zauberroutinen 1..8 (Antimagie/Beherrschung):
 * spell_beherrschung_brechen .. spell_boeser_blick (9..85 in O234..O237).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL233.CPP   -> 5827/5827 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (g_spell_handlers -> _g_1bc3+0xd, g_heal_ani_frames -> _s_7469+0x11)
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Methodenzeiger: "class Cast;" vor "typedef void (Cast::*CAST_PMF)();", func = g_spell_handlers[id] (SCOPY@ cx=8),
 *   (this->*func)() -> call dword ptr es:[bx+0eh].
 * - Der Block ds:8c11..8c17 wird hier ueber die Struktursicht g_spell_flags angesprochen (das Original greift ueber
 *   _g_8c11 + n zu; mit den Einzelnamen g_spell_no_sound/g_spell_event waeren die Fixup-Ziele andere).
 * - -Oi: strcpy(g_text_output_buf, txt[n]) in beiden Formen; memset(g_cast_spell_id, 0, 16) und
 *   memset(&g_spell_flags, 0, 7) (rep stosw / stosb).
 * - Lst_add/change_attrib mit Wort-Parametern ueber gecastete Funktionszeiger (lst_add_w, change_attrib_w).
 * - spell_destructibo: "(slot = select_item(..)) != -1" eingebettet (cmp ax,0ffffh); id*27 als -Og-Zwischenwert.
 * - heal_ani: Feld ani (10 Bytes, aus g_heal_ani_frames) und beide Bmp in Deklarationsreihenfolge hinter den
 *   Skalaren, Destruktoren rueckwaerts (&b, 2).
 * - Alle Bitfelder unsigned short b:1: "(unsigned char)x.b" -> and ax,1 / or al,al, "!(signed char)x.b" -> cbw.
 * Originalfehler/Auffaelligkeiten: spell_class_first laeuft mit "while (i++ < 12)" bis Index 12 (13 Eintraege von
 *   g_spells_index) und liefert 1, wenn id groesser als die Summe ist. heal_ani uebergibt &w als Breite und Hoehe.
 */
#include <stdio.h>
#include <string.h>
#include <mem.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:747a..7486, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
struct heal_ani_frames g_heal_ani_frames = { { 0, 1, 2, 1, 0 } };

/* change_attrib mit Wort-Parameter (das Original uebergibt den Betrag als short: push 1) */
#define change_attrib_w ((void (*)(struct hero far *, signed short, signed short))change_attrib)
/* Lst_add mit Wort-Parametern (das Original uebergibt Seite und Art als short: push 2) */
#define lst_add_w ((void (*)(void far *, signed short, signed short, signed short, void far *, signed short, void far *))Lst_add)

signed short spell_class_first(signed short id)
{
	signed short i = 0;
	signed short sum = 0;
	signed short first = 1;

	do {
		sum += g_spells_index[i].count;
		if (id <= sum) {
			first = g_spells_index[i].first;
			break;
		}
#if FIX_0163
	/* Fix #163: nur die 12 Eintraege der Tabelle lesen */
	} while (++i < 12);
#else
	} while (i++ < 12);
#endif
	return first;
}

signed short hero_spell_cost(signed short id, signed short half)
{
	signed char cost;

	cost = g_spell_descriptions[id].ae_cost;
	cost += g_spell_flags.cost_mod;
	if (half) {
		if (cost == -1)
			cost = random_interval(5, 10);
		else if (cost >= 2)
			cost = cost / 2;
	}
	return cost;
}

signed short select_magic_user(void)
{
	struct hero far *h;

	h = get_active_hero(g_group, g_ttx[0x146]);
	if (h != 0)
		return use_spell(h, 1, 0);
	return 0;
}

Cast::Cast(struct hero far *h)
{
	signed short i;

	if (!g_quick_fight) {
		txt_load_list(g_font, 0x1f, 300, &txt_buf, &txt);
	} else {
		txt = (char far * far *)mem_alloc(0x4b4L);
		txt_buf = mem_alloc(10L);
		for (i = 0; i <= 300; i++)
			txt[i] = g_b690;
	}
	caster = h;
	result = SPELL_RES_DEFAULT;
}

Cast::~Cast()
{
	if (txt_buf) {
		mem_free(txt_buf);
		mem_free(txt);
	}
}

void Cast::heal_ani()
{
	signed short pos;
	signed short i;
	signed short w;
	signed short x;
	struct heal_ani_frames ani = g_heal_ani_frames;
	Bmp star(32, 32);
	Bmp bg(32, 32);

	if (target->npc_id != 0)
		pos = 6;
	else
		pos = group_hero_pos(g_group, target);
	x = g_portrait_x[pos];
	bg.get(g_vga_buf, x, 0x9d);
	for (i = 0; i < 5; i++) {
		bg.put(g_vga_buf, x, 0x9d, 0);
		nvfile_load(g_nvf_heads, 0x49, star.data(), 0, ani.frame[i], 0, &w, &w, 0x140);
		star.put(g_vga_buf, x, 0x9d, 4);
		wait_ticks_key(10);
	}
	screen_portraits(g_screen);
}

signed short Cast::damage(signed short le)
{
	if (le <= 0)
		return 0;
	if (target_type(caster) == TARGET_HERO) {
		target = hero_mr_target(caster);
		if (target->le < le)
			le = target->le;
		sub_hero_le(target, le);
		lst_add_w(g_fight_list, 8, le, 2, caster, 2, target);
		if ((unsigned char)target->dead)
			g_fig_target_killed = 1;
	} else {
		enemy = enemy_mr_target(caster);
		if (enemy->le < le)
			le = enemy->le;
		enemy_sub_le(enemy, le, 0);
		lst_add_w(g_fight_list, 0x0b, le, 2, caster, 1, enemy);
		if ((unsigned char)enemy->dead)
			g_fig_target_killed = 1;
	}
	return le;
}

signed short Cast::target_pa()
{
	if (target_kind == TARGET_HERO || target_kind == TARGET_SELF)
		return hero_pa(target);
	else
		return enemy_parade_value(enemy);
}

signed short Cast::target_rs()
{
	if (target_kind == TARGET_HERO || target_kind == TARGET_SELF)
		return (signed char)(target->rs_bonus + target->rs_bonus2);
	else
		return enemy->rs;
}

signed short Cast::cast(signed short id, signed short handicap)
{
	signed short cost;
	signed short retval;
	signed short in_fight;

	in_fight = g_in_fight;
	memset(g_cast_spell_id, 0, 16);
	retval = 1;
	*g_text_output_buf = 0;
	if ((caster->type < HERO_TYPE_WITCH || !check_hero(caster) && !(signed char)caster->renegade) &&
		g_spell_flags.by_item == 0) {
		g_spell_cancelled = 1;
		sprintf(g_text_output_buf, txt[0xb6], caster->name);
		retval = -1;
	} else if (caster->spells[id] < -5 && g_spell_flags.by_item == 0) {
		g_spell_cancelled = 1;
		sprintf(g_text_output_buf, txt[0xb7], caster->name);
		retval = -1;
	} else {
		cost = hero_spell_cost(id, 0);
		if (g_spell_flags.cost_mod != -1 && (caster->ae == 0 || cost != -1 && caster->ae < cost)) {
			g_spell_cancelled = 1;
			sprintf(g_text_output_buf, txt[0x8c], caster->name);
			retval = -1;
		} else {
			target_kind = target_type(caster);
			mod = handicap;
			fail_cost = 0;
			if (target_kind == TARGET_HERO || target_kind == TARGET_SELF) {
				target = hero_mr_target(caster);
				enemy = 0;
			} else {
				enemy = enemy_mr_target(caster);
				target = 0;
				if (enemy_type_check(enemy))
					mod += 5;
			}
			if (g_fight_id == FIGHT_ID_SPECIAL && (enemy != 0 || target == caster) &&
				is_in_word_array(id, g_spells_no_sfight) != -1) {
				result = SPELL_RES_FAIL;
				strcpy(g_text_output_buf, txt[0x107]);
			} else if (is_in_word_array(id, g_spells_own_test) != -1) {
				func = g_spell_handlers[id];
				(this->*func)();
				g_cast_spell_id[0] = id;
			} else if (test_spell(caster, id, mod) > 0) {
				func = g_spell_handlers[id];
				(this->*func)();
				g_cast_spell_id[0] = id;
			} else
				result = SPELL_RES_FAIL;
			if (result != SPELL_RES_ABORT && g_spell_flags.no_sound == 0) {
				if (g_speech_on == 1 && speech_active(g_snd_speech)) {
					snd_speech(g_snd_speech, caster->sex == 0 ? id + 0x1e : id + 0x73);
					Speech_wait(g_snd_speech);
				}
				if (g_spell_descriptions[id].snd != 0 && (g_fight_obj == 0 || g_fight_screen_on == 1)) {
					if (arc_exists(g_arc_fx, g_spell_descriptions[id].snd))
						snd_play2(g_snd_fx, g_spell_descriptions[id].snd);
					else
						snd_play2(g_snd_fx, 0x97);
				}
			}
			if (result == SPELL_RES_NONE) {
				retval = -1;
				if (!*g_text_output_buf)
					strcpy(g_text_output_buf, txt[0x8d]);
			} else if (result == SPELL_RES_FAIL) {
				retval = -1;
				if (!*g_text_output_buf)
					strcpy(g_text_output_buf, txt[0x8d]);
				if (fail_cost != 0)
					sub_ae_splash(caster, fail_cost);
				else
					sub_ae_splash(caster, hero_spell_cost(id, 1));
			} else if (result == SPELL_RES_ABORT) {
				g_spell_cancelled = 1;
				retval = 0;
			} else if (result == SPELL_RES_DEFAULT) {
				/* Stabzauber ab Stufe 4: 2 AE weniger */
				if (caster->staffspell_lvl >= 4 && cost > 2)
					cost -= 2;
				if (g_spell_flags.cost_mod != -1)
					sub_ae_splash(caster, cost);
				g_spell_success = 1;
				g_spell_cost_paid = cost;
			} else {
				if (caster->staffspell_lvl >= 4 && result > 2)
					result -= 2;
				if (g_spell_flags.cost_mod != -1)
					sub_ae_splash(caster, result);
				g_spell_success = 1;
				g_spell_cost_paid = result;
			}
		}
	}
	if (in_fight == 0) {
		dlg_message(g_dialog, g_text_output_buf);
		if (retval > 0 && g_spell_cancelled == 0 && target_kind == TARGET_HERO && g_pp20_index == 4)
			heal_ani();
	}
	memset(&g_spell_flags, 0, 7);
	return retval;
}

void Cast::spell_beherrschung_brechen()
{
	if (target == caster) {
		strcpy(g_text_output_buf, txt[0]);
		result = SPELL_RES_ABORT;
		return;
	}
	if (!(signed char)target->renegade) {
		sprintf(g_text_output_buf, txt[0x103], target->name);
		result = SPELL_RES_ABORT;
		return;
	}
	result = random_interval(2, 5) * 4;
	result += g_spell_flags.cost_mod;
	if (result > caster->ae) {
		fail_cost = result / 2;
		result = SPELL_RES_FAIL;
		return;
	}
	target->renegade = 0;
	sprintf(g_text_output_buf, txt[1], target->name, grammar_pronoun(target->sex, 3, 1));
}

void Cast::spell_destructibo()
{
	signed short slot;
	signed short id;
	signed short n;

	if (g_spell_flags.event == 0) {
		if ((slot = select_item(caster, txt[0xce], 1)) != -1) {
			id = caster->inv[slot].item_id;
			n = caster->inv[slot].quantity;
			if (!drop_item_at(caster, slot)) {
				if (n > 1 && g_items[id].stackable || is_in_word_array(id, g_items_plural) != -1)
					sprintf(g_text_output_buf, txt[0x105], grammar_article(6, id, 0));
				else
					sprintf(g_text_output_buf, txt[0x105], grammar_article(2, id, 0));
			} else {
				if (n > 1 && g_items[id].stackable || is_in_word_array(id, g_items_plural) != -1)
					sprintf(g_text_output_buf, txt[0x104], grammar_article(0x8004, id, 0), txt[0x1a]);
				else
					sprintf(g_text_output_buf, txt[0x104], grammar_article(0x8000, id, 0), "");
				sheet_cast_anim();
				result = random_interval(5, 50);
				g_refresh = 1;
			}
		} else
			result = SPELL_RES_ABORT;
	}
}

void Cast::spell_gardianum()
{
	signed short ae;
	signed short points;

	sprintf(g_text_output_buf, txt[2], caster->name);
	ae = dlg_input(g_dialog, 2, g_text_output_buf, 0, caster->ae, 0);
	*g_text_output_buf = 0;
	if (ae > 0) {
		if (test_spell(caster, SP_GARDIANUM, mod) > 0) {
			result = ae;
			result += g_spell_flags.cost_mod;
			if (result > caster->ae) {
				fail_cost = result / 2;
				result = SPELL_RES_FAIL;
			} else {
				strcpy(g_text_output_buf, txt[7]);
				points = ae + caster->level;
				caster->gardianum += points;
			}
		} else {
			fail_cost = ae / 2;
			result = SPELL_RES_FAIL;
		}
	} else
		result = SPELL_RES_ABORT;
}

void Cast::spell_illusionen_zerstoeren()
{
	if ((unsigned char)enemy->illusion) {
		result = (enemy->level - caster->level) * 2;
		result += g_spell_flags.cost_mod;
		if (result < 5)
			result = 5;
		if (result > caster->ae) {
			result = SPELL_RES_FAIL;
			fail_cost = caster->ae;
		} else
			enemy->dead = 1;
		return;
	}
	sprintf(g_text_output_buf, txt[3], grammar_article(0x8000, enemy->mon_id, 1));
	result = SPELL_RES_FAIL;
	fail_cost = 2;
}

void Cast::spell_verwandlung_beenden()
{
	signed short i;

	if ((unsigned char)target->petrified) {
		if (g_fight_id == FIGHT_ID_SPECIAL && target->npc_id != 0) {
			result = SPELL_RES_FAIL;
			fail_cost = caster->ae;
			sprintf(g_text_output_buf, txt[0x108], target->name);
		} else {
			result = random_schweif(10) * 5;
			result += g_spell_flags.cost_mod;
			if (result > caster->ae) {
				result = SPELL_RES_FAIL;
				fail_cost = caster->ae;
			} else {
				target->petrified = 0;
				sprintf(g_text_output_buf, txt[4], target->name);
			}
		}
		return;
	}
	if ((unsigned char)target->transformed) {
		target->transformed = 0;
		g_quiet = 1;
		for (i = 0; i <= 6; i++)
			change_attrib_w(target, i, 1);
		g_quiet = 0;
		sprintf(g_text_output_buf, txt[1], target->name, grammar_pronoun(target->sex, 3, 1));
		return;
	}
	sprintf(g_text_output_buf, txt[5], target->name);
	result = SPELL_RES_ABORT;
}

void Cast::spell_band_und_fessel()
{
	if (target_kind == TARGET_ENEMY || target_kind == TARGET_SPECIAL) {
		if (enemy_is_gfx_19_3f(enemy)) {
			result = SPELL_RES_FAIL;
			strcpy(g_text_output_buf, txt[0xb8]);
		} else {
			enemy->tied = 1;
			sprintf(g_text_output_buf, txt[6], grammar_article(0x8000, enemy->mon_id, 1));
		}
		return;
	}
	if (target == caster) {
		result = SPELL_RES_ABORT;
		strcpy(g_text_output_buf, txt[0x70]);
		return;
	}
	target->tied = 1;
	sprintf(g_text_output_buf, txt[6], target->name);
}

void Cast::spell_bannbaladin()
{
	if (enemy->is_animal != 0) {
		result = SPELL_RES_ABORT;
		sprintf(g_text_output_buf, txt[8], grammar_article(0, enemy->mon_id, 1));
		return;
	}
	if (enemy_is_gfx_19_3f(enemy)) {
		if (g_fight_end == 6) {
			g_in_fight = 0;
			g_fight_end = 0;
			enemy->tame = 1;
			sprintf(g_text_output_buf, txt[9], grammar_article(0x8000, enemy->mon_id, 1));
		} else {
			result = SPELL_RES_FAIL;
			strcpy(g_text_output_buf, txt[0xb8]);
		}
		return;
	}
	enemy->tame = 1;
	sprintf(g_text_output_buf, txt[9], grammar_article(0x8000, enemy->mon_id, 1));
}

void Cast::spell_boeser_blick()
{
	if (enemy_is_gfx_19_3f(enemy)) {
		result = SPELL_RES_FAIL;
		strcpy(g_text_output_buf, txt[0xb8]);
		return;
	}
	if (test_spell(caster, SP_BOESER_BLICK, mod + enemy->mr) > 0) {
		enemy->renegade = 1;
		enemy->attacks = 2;
		sprintf(g_text_output_buf, all_heroes_down(g_group) ? txt[0xee] : txt[10],
			grammar_article(0x8000, enemy->mon_id, 1));
		if (g_fight_end == 5)
			g_fight_end = 0;
		return;
	}
	result = SPELL_RES_FAIL;
}
