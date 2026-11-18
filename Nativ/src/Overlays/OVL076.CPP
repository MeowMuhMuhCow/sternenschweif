/* OVL076 - Overlay O076 (Segment 0x20be in SCHWEIF.EXE): Heldenwerte (entspricht DSA1 seg002/seg047-Teilen).
 * hero_name_gen(h, gen): Heldenname in einen von 4 Ringpuffern g_name_bufs[4][40] (Index g_name_buf_no),
 * gen == 1 haengt das Genitiv-"s" an (g_str_gen_s bzw. g_str_gen_s2 nach Gross-/Kleinbuchstabe, nicht bei
 * Endung s/S). is_hero_available_in_group: check_hero und in der aktiven Gruppe (group_no == g_group_no).
 * sub_ae_splash/add_hero_ae: AE abziehen/hinzufuegen (nur Zauberkundige; Magier mit Stabzauberstufe >= 4: 2 AE
 * weniger). sub_hero_le(h, le): LE abziehen (im Schnellkampf doppelt; Schlaf beenden, Tod: Text 0x81/0x2c,
 * Krankheiten/Gifte mit -1 auf 1/0, Bewusstlosigkeit unter 5 LE, Klangeffekte snd_play). hunger_thirst_damage:
 * 1 LE, mindestens 1, dann LE-Grundwert -1 / le_mod +1. add_hero_le (Aufwachen ab 5). test_attrib/test_attrib3:
 * Eigenschafts-/Dreifachprobe (W20, -99 bei Patzer). add_mr_mod/calc_mr: MR neu berechnen ((KL + MU + Stufe) / 3
 * - 2 * AG + g_mr_modificators[Typ], Meldung g_ttx[0x193]). calc_atpa_base: atpa_base aus (IN + KK + GE) / 5
 * (div, Rest >= 3 aufrunden), Differenz auf at[]/pa[] verteilen, Meldung g_ttx[0x15]. test_skill(h, talent,
 * mod): Talentprobe (Schuss-/Wurfwaffen: eigene Rechnung mit W20, 1 -> 99, 20 -> -1; sonst test_attrib3 mit
 * g_skill_attribs[talent]). portrait_file: Kopfbilddatei (0x3a4 krank/schwach, sonst 5). load_portrait/
 * load_portrait_bmp: Heldenportraet nach g_portrait_buf bzw. ins Bmp laden (nvfile_count/nvfile_load, tot/Bits
 * harmless/adler_wolf). hero_inc_skill: add_skill_value(h, a, 1). sub_hunger/sub_thirst: hunger/thirst senken (mindestens 0).
 * fill_saved_values: die 8 Bytes der gemerkten LE/AE-Werte (+0x553) mit v fuellen.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL076.CPP   -> 3843/3843 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * - -Oi fuer strcpy/strlen/strcat/memset als Intrinsics; C++ wegen inline-Praedikat hero::is_mage() ("cmp 7 /
 *   mov ax,1 / xor ax,ax / or ax,ax") und der inline-Setter set_ill_state/set_ill_days/set_poi_state/set_poi_days
 *   (je eigene Kopie des Index [bp-4]..[bp-0ah]) und set_hunger/set_thirst (Kopie [bp-1]).
 * - Bitfelder +0xbc..+0xbe als "unsigned short b:1" (24 Bit belegen nur 3 Bytes -> Fuellbyte +0xbf noetig):
 *   "!(signed char)h->dead" -> and/cbw/or ax; "(unsigned char)h->dead" -> and ax,1 / or al,al; "h->asleep" -> or ax.
 * - hero_name_gen: KEINE Puffervariable - "g_name_bufs[g_name_buf_no]" mehrfach geschrieben ergibt den
 *   -Og-Zwischenwert [bp-8] (im strcpy-Ziel berechnet, strcat-Ziel erst nach der Quelle geladen: mov dx,ds /
 *   mov si,[bp-8]); mit eigener Variable buf wird das strcat-Ziel vorab auf den Stack gelegt (4 Bytes laenger).
 * - calc_mr: ebenso keine Variable fuer "h->mr + h->mr_mod" - der -Og-Zwischenwert [bp-2] liegt VOR der lokalen
 *   13-Byte-Tabelle und ergibt "mov [bp-2],dx / mov al,old / cbw / cmp dx,ax" (eigene Variable: push/pop bzw.
 *   cmp ax,[bp-2]). Tabelle als lokale Strukturkopie "struct mr_table tab = g_mr_modificators;" (rep movsw/movsb).
 * - Registerwahl: register-Parameter SI, Variable OHNE register -> DI (test_attrib r, test_skill v, calc_atpa_base
 *   i); mit register vertauscht sich SI/DI. sub_hunger/sub_thirst: "if ((t = h->hunger - v) < 0) t = 0;" mit t im
 *   Block -> DX.
 * - calc_atpa_base: div() liefert div_t ueber shl eax,10h / shrd eax,edx,10h; "h->at[i] = h->at[i] + diff" (mit SI
 *   als Adresskopie). portrait_file: "if (..) return 0x3a4; else return 5;" (zwei tote Epiloge).
 * - fill_saved_values: memset(&h->saved_le_max, v, 8) (vorher h->f553[8]) - gleiche Adressrechnung.
 * Originalfehler: hero_name_gen behandelt alle Zeichen >= 0x80 ausser ae/ue/oe (0x84/0x81/0x94) wegen signed char
 *   als Grossbuchstaben (z.B. sz 0xe1 -> Endung von g_str_gen_s2).
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "schweif.h"
#include "s_plog.h"	/* Probenlog (FIX_PROBENLOG) */

/* Phase B: eigene Daten des Moduls (ds:5974..5988, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
signed short g_name_buf_no = 0;
struct mr_table g_mr_modificators = { { 0, 2, 0, 0, 2, -2, 2, 2, 2, 2, 3, 4, 3 } };

char far *hero_name_gen(struct hero far *h, signed short gen)
{
	signed char c;
	char far *ret;

	strcpy(g_name_bufs[g_name_buf_no], h->name);
	if (gen == 1) {
		c = g_name_bufs[g_name_buf_no][strlen(g_name_bufs[g_name_buf_no]) - 1];
		if (c != 'S' && c != 's')
#if FIX_0065
			/* Fix #65: Zeichen ab 0x80 ausser den grossen Umlauten gelten als Kleinbuchstaben */
			strcat(g_name_bufs[g_name_buf_no], c >= 'a' || ((unsigned char)c >= 0x80 && c != '\x8e' && c != '\x99' && c != '\x9a') ? "s" : "S");
#else
			strcat(g_name_bufs[g_name_buf_no], c >= 'a' || c == '\x84' || c == '\x81' || c == '\x94' ? "s" : "S");
#endif
	}
	ret = g_name_bufs[g_name_buf_no];
	if (++g_name_buf_no == 4)
		g_name_buf_no = 0;
	return ret;
}

signed short is_hero_available_in_group(struct hero far *h)
{
	if (check_hero(h) && h->group_no == g_group_no)
		return 1;
	return 0;
}

void sub_ae_splash(struct hero far *h, register signed short ae)
{
	if (!(signed char)h->dead && ae > 0 && h->is_mage()) {
		if (h->type == 9 && h->staffspell_lvl >= 4)
			if ((ae -= 2) < 0)
				ae = 0;
		screen_mark_portrait(g_screen, h->slot_pos, 1);
		if ((h->ae -= ae) < 0)
			h->ae = 0;
	}
}

void add_hero_ae(struct hero far *h, register signed short ae)
{
	if (!(signed char)h->dead && ae > 0 && h->is_mage()) {
		if ((h->ae += ae) > h->ae_max)
			h->ae = h->ae_max;
	}
}

void sub_hero_le(struct hero far *h, register signed short le)
{
	signed short old;
	register signed short i;

	if (!(signed char)h->dead && le > 0) {
		if (g_quick_fight)
			le *= 2;
		old = h->le;
		h->le -= le;
		if (h->asleep)
			h->asleep = 0;
		screen_mark_portrait(g_screen, h->slot_pos, 0);
		if (h->le <= 0) {
			if (!g_fight_obj || g_fight_id == FIGHT_ID_SPECIAL2)
				snd_play(g_snd_fx, h->sex == 0 ? 0x4c : 0x4b);
			if (g_silent_mode == 0 && g_in_fight == 0) {
				void far *raw;
				char far *s;
				char far * far *txt;

				txt_load_list(g_font, 0x81, 0x32, &raw, &txt);
				s = (char far *)mem_alloc(100);
				sprintf(s, txt[0x2c], h->name);
				diary_add(g_diary, s, 0);
				dlg_message(g_dialog, s);
				mem_free(s);
				mem_free(raw);
				mem_free(txt);
			}
			h->le = 0;
			h->dead = 1;
			set_unconscious(h, 0);
			h->duplicatus = 0;
			if (h->npc_id == 2)
				g_game_state = 8;
			h->action_id = 100;
			g_refresh_portraits = 1;
			if (g_pp20_index != 0x94)
				g_refresh = 1;
			for (i = 1; i <= 7; i++)
				if (h->illness[i].state == -1) {
					h->set_ill_state(i, 1);
					h->set_ill_days(i, 0);
				}
			for (i = 1; i <= 9; i++)
				if (h->poison[i].state == -1) {
					h->set_poi_state(i, 1);
					h->set_poi_days(i, 0);
				}
		} else {
			if (!g_fight_obj && !snd_busy(g_snd_fx, 0)) {
				if (le > 20)
					snd_play(g_snd_fx, h->sex == 0 ? 0x51 : 0x50);
				else
					snd_play(g_snd_fx, h->sex == 0 ? 0xa9 : 0xa6);
			}
		}
		if (!(signed char)h->dead && old >= 5 && h->le < 5) {
			set_unconscious(h, 1);
			h->action_id = 12;
		}
		if (g_fight_end == 5 && ((unsigned char)h->dead || (unsigned char)h->unconscious)) {
			g_in_fight = 0;
			g_game_state = 7;
			g_fight_end = 0;
		}
		if ((unsigned char)h->dead && g_in_fight == 0)
			g_hero_died = 1;
	}
}

void hunger_thirst_damage(struct hero far *h)
{
	signed short old;

	if (!(signed char)h->dead) {
		old = portrait_file(h);
		h->le--;
		if (portrait_file(h) != old)
			g_refresh_portraits = 1;
		if (h->le < 5 && !(signed char)h->unconscious)
			set_unconscious(h, 1);
		else if ((!g_fight_obj || g_fight_screen_on == 1) && group_has_hero(g_group, h))
			snd_play(g_snd_fx, h->sex == 0 ? 0xa9 : 0xa6);
		if (h->le <= 0) {
			h->le = 1;
			if (h->le_max >= 2) {
				h->le_max--;
				h->le_mod++;
			}
		}
	}
	if (g_pp20_index == 14)
		g_refresh = 1;
}

void add_hero_le(struct hero far *h, register signed short le)
{
	if (!(signed char)h->dead && le > 0) {
		if ((h->le += le) > h->le_max)
			h->le = h->le_max;
		if (h->le >= 5 && (unsigned char)h->unconscious)
			set_unconscious(h, 0);
	}
	if (g_pp20_index == 14 || g_pp20_index == 15) {
		g_refresh = 1;
		return;
	}
	if (g_pp20_index == 4)
		g_refresh_portraits = 1;
}

signed short test_attrib(struct hero far *h, register signed short at, signed short hc)
{
#if FIX_PROBENLOG
	/* Probenlog: dieselbe Probe (gleich viele Wuerfe, gleiches Ergebnis), dazu eine Zeile Art 1:
	 * Eigenschaft, Eigenschaftswert, Zuschlag, W20, Ergebnis */
	signed short w[5];

	w[3] = random_schweif(20);
	w[0] = at;
	w[1] = (signed char)(h->attrib[at].cur + h->attrib[at].mod);
	w[2] = hc;
	w[4] = (w[3] == 20) ? -99 : (w[1] - (w[3] + hc) + 1);
	probenlog(1, h->name, 5, w);
	return w[4];
#else
	signed short tmp;
	signed short r;

	r = random_schweif(20);
	if (r == 20)
		return -99;
	else
		r += hc;
	tmp = (signed char)(h->attrib[at].cur + h->attrib[at].mod);
	return tmp - r + 1;
#endif
}

signed short test_attrib3(struct hero far *h, signed short a1, signed short a2, signed short a3, signed char hc)
{
#if FIX_PROBENLOG
	/* Probenlog: dieselbe Probe (gleich viele Wuerfe in gleicher Reihenfolge, gleiches Ergebnis), dazu eine Zeile
	 * Art 3: drei Eigenschaften, ihre Werte, Zuschlag, drei W20 (0 = nicht mehr gewuerfelt), Ergebnis */
	signed short w[11];
	signed short i;
	signed short n20 = 0;

	w[0] = a1; w[1] = a2; w[2] = a3;
	w[3] = (signed char)(h->attrib[a1].cur + h->attrib[a1].mod);
	w[4] = (signed char)(h->attrib[a2].cur + h->attrib[a2].mod);
	w[5] = (signed char)(h->attrib[a3].cur + h->attrib[a3].mod);
	w[6] = hc;
	w[7] = w[8] = w[9] = 0;
	for (i = 0; i < 3; i++) {
		if ((w[7 + i] = random_schweif(20)) == 20 && ++n20 == 2)
			break;
	}
	w[10] = (n20 == 2) ? -99 : (w[3] + w[4] + w[5] - (w[7] + w[8] + w[9] + hc) + 1);
	probenlog(3, h->name, 11, w);
	return w[10];
#else
	signed short sum = 0;
	signed short i;
	signed short n20 = 0;
	register signed short tmp;

	for (i = 0; i < 3; i++) {
		tmp = random_schweif(20);
		if (tmp == 20)
			if (++n20 == 2)
				return -99;
		sum += tmp;
	}
	sum += hc;
	tmp = (signed char)(h->attrib[a1].cur + h->attrib[a1].mod) + (signed char)(h->attrib[a2].cur + h->attrib[a2].mod) +
		(signed char)(h->attrib[a3].cur + h->attrib[a3].mod);
	return tmp - sum + 1;
#endif
}

void add_mr_mod(struct hero far *h, signed char v)
{
	signed char old;

	old = h->mr + h->mr_mod;
	h->mr_mod += v;
	calc_mr(h, old);
}

void calc_mr(struct hero far *h, signed char old)
{
	struct mr_table tab = g_mr_modificators;

	h->mr = ((signed char)(h->attrib[ATTRIB_KL].cur + h->attrib[ATTRIB_KL].mod) + (signed char)(h->attrib[ATTRIB_MU].cur + h->attrib[ATTRIB_MU].mod) + h->level) / 3 -
		(h->attrib[ATTRIB_AG].cur + h->attrib[ATTRIB_AG].mod) * 2;
	h->mr += tab.v[h->type];
	if (h->mr + h->mr_mod != old) {
		if (g_quiet == 0 && !(signed char)h->dead) {
			sprintf(g_text_output_buf, g_ttx[0x193], hero_name_gen(h, 1), h->mr + h->mr_mod);
			dlg_message(g_dialog, g_text_output_buf);
		}
		if (g_pp20_index == 14)
			g_refresh = 1;
	}
}

void calc_atpa_base(struct hero far *h)
{
	div_t q;
	signed short diff;
	signed short i;

	remove_effect(h, 2);
	remove_effect(h, 3);
	q = div(h->attrib[ATTRIB_IN].cur + h->attrib[ATTRIB_KK].cur + h->attrib[ATTRIB_GE].cur, 5);
	if (q.rem >= 3)
		q.quot++;
	diff = q.quot - h->atpa_base;
	if (diff != 0) {
		h->atpa_base = q.quot;
		if (g_quiet == 0 && !(signed char)h->dead) {
			sprintf(g_text_output_buf, g_ttx[0x15], hero_name_gen(h, 1), h->atpa_base);
			dlg_message(g_dialog, g_text_output_buf);
		}
		for (i = 0; i < 7; i++) {
			h->at[i] = h->at[i] + diff;
			h->pa[i] = h->pa[i] + diff;
		}
	}
}

signed short test_skill(struct hero far *h, register signed short sk, signed char hc)
{
	signed short v;
	signed short r;

	if (sk >= TA_SCHUSSWAFFEN && sk <= TA_SINNESSCHAERFE) {
		if (sk == TA_SCHUSSWAFFEN || sk == TA_WURFWAFFEN) {
			v = ((signed char)(h->attrib[ATTRIB_KL].cur + h->attrib[ATTRIB_KL].mod) + (signed char)(h->attrib[ATTRIB_GE].cur + h->attrib[ATTRIB_GE].mod) +
				(signed char)(h->attrib[ATTRIB_KK].cur + h->attrib[ATTRIB_KK].mod)) / 4;
			v += get_skill_value(h, sk);
			v -= hc;
#if FIX_PROBENLOG
			/* Probenlog: dieselbe Fernkampfprobe, dazu eine Zeile Art 4: Talent, Zielwert, Zuschlag, 1, W20, Ergebnis */
			{
				signed short w[6];

				w[0] = sk; w[1] = v; w[2] = hc; w[3] = 1;
				w[4] = r = random_schweif(20);
				w[5] = (r == 20) ? -1 : ((r == 1) ? 99 : ((r <= v) ? (v - r + 1) : 0));
				probenlog(4, h->name, 6, w);
				return w[5];
			}
#else
			r = random_schweif(20);
			if (r == 20)
				return -1;
			if (r == 1)
				return 99;
			if (r <= v)
				return v - r + 1;
#endif
		} else {
#if FIX_PROBENLOG
			/* Probenlog: Zeile Art 4 vor der Dreifachprobe (Wuerfe und Ergebnis in der folgenden Zeile Art 3):
			 * Talent, Talentwert, Zuschlag, 0, 0, 0 */
			{
				signed short w[6];

				w[0] = sk; w[1] = get_skill_value(h, sk); w[2] = hc; w[3] = w[4] = w[5] = 0;
				probenlog(4, h->name, 6, w);
			}
#endif
			hc -= get_skill_value(h, sk);
			return test_attrib3(h, g_skill_attribs[sk].a1, g_skill_attribs[sk].a2, g_skill_attribs[sk].a3, hc);
		}
	}
	return 0;
}

signed short portrait_file(struct hero far *h)
{
	if (hero_is_poisoned(h) || hero_is_diseased(h) || (h->le < h->le_max / 3 && h->le < 25) || h->le < 10)
		return 0x3a4;
	else
		return 5;
}

void far *load_portrait(struct hero far *h)
{
	signed short dummy;
	register signed short n;
	register signed short id;

	if ((unsigned char)h->dead) {
		n = nvfile_count(g_nvf_heads, 5);
		nvfile_load(g_nvf_heads, 5, g_portrait_buf, 0, n - 12, 0, &dummy, &dummy, 0x140);
	} else if (h->harmless) {
		n = nvfile_count(g_nvf_heads, 0x3a4);
		nvfile_load(g_nvf_heads, 0x3a4, g_portrait_buf, 0, n - (h->sex ? 2 : 3), 0, &dummy, &dummy, 0x140);
	} else if (h->adler_wolf) {
		n = nvfile_count(g_nvf_heads, 0x3a4);
		nvfile_load(g_nvf_heads, 0x3a4, g_portrait_buf, 0, n - 1, 0, &dummy, &dummy, 0x140);
	} else {
		id = portrait_file(h);
		if (h->npc_id != 0) {
			n = nvfile_count(g_nvf_heads, id);
			nvfile_load(g_nvf_heads, id, g_portrait_buf, 0, n + h->npc_id - 12, 0, &dummy, &dummy, 0x140);
		} else
			nvfile_load(g_nvf_heads, id, g_portrait_buf, 0, h->portrait, 0, &dummy, &dummy, 0x140);
	}
	return g_portrait_buf;
}

void load_portrait_bmp(struct hero far *h, Bmp far *b)
{
	signed short dummy;
	signed short n;
	register signed short id;

	id = portrait_file(h);
	if (h->npc_id != 0) {
		n = nvfile_count(g_nvf_heads, id);
		nvfile_load(g_nvf_heads, id, b->data(), 0, n + h->npc_id - 12, 0, &dummy, &dummy, 0x140);
		return;
	}
	nvfile_load(g_nvf_heads, id, b->data(), 0, h->portrait, 0, &dummy, &dummy, 0x140);
}

void hero_inc_skill(struct hero far *h, signed short a)
{
	if (!(signed char)h->dead)
		add_skill_value(h, a, 1);
}

void sub_hunger(struct hero far *h, signed short v)
{
	if (!(signed char)h->dead) {
		register signed short t;
		if ((t = h->hunger - v) < 0)
			t = 0;
		h->set_hunger(t);
		h->f4f0 = 0;
	}
}

void sub_thirst(struct hero far *h, signed short v)
{
	if (!(signed char)h->dead) {
		register signed short t;
		if ((t = h->thirst - v) < 0)
			t = 0;
		h->set_thirst(t);
		h->f4f0 = 0;
	}
}

void fill_saved_values(struct hero far *h, signed char v)
{
	memset(&h->saved_le_max, v, 8);
}
