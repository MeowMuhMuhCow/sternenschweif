/* OVL033 - Overlay O033 (Segment 0x1fb2 in SCHWEIF.EXE): Tempelwunder, entspricht DSA1 seg062 ask_miracle.
 * temple_miracle(t): Texte 0x17/0x27 per txt_load_list laden, Bonus = g_temple_miracle_bonus[gott] * (Ansehen
 * g_god_favor[gott] / 100) - l3 (l3 immer 0, wie DSA1), Ansehen -= 10 (wenn >= 10), W100 <= Wuerfel + Bonus ->
 * W(Wuerfel): Maximum = Wiederbelebung (miracle_resurrect), sonst switch ueber 14 Goetter (Fall 13 fehlt, 14 = 7 wie
 * DSA1 Firun/Ifirn) mit Zeitzaehlern g_ingame_timers[INGAME_TIMER_*] (long), Gruppenwundern miracle_group_effect,
 * Heilung miracle_heal, Heldenfunktionen (get_..._hero, hero_..., add_effect, group_...). Bei geaendertem Text Animation
 * miracle_show_anim und Meldung (g_ttx[gott + 4], Text 37/38 je nach t->kind) per diary_add.
 * miracle_group_effect(t, a, zeit, b, c): group_add_effect mit g_quiet = 1/0. miracle_heal(t, n, s): Held
 * get_most_wounded_hero um n (hoechstens le_max - le) heilen, Singular/Plural g_ttx[28/29]. miracle_resurrect(t,
 * s): Held get_dead_hero wiederbeleben (dead loeschen, add_hero_le(h, 7)), g_refresh = 1. miracle_show_anim(t):
 * Animation des Gottes (class Anim O051: rate, pal_set, start, wait_frame, wait_cue; Bmp) zeigen. Wunder nur fuer Helden ohne gods_pissed (BE).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL033.CPP   -> 3854/3854 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Erste Fassung sofort gleich. -Og noetig (t->god in DI zwischengespeichert, nach Aufrufen neu geladen;
 *   god * 4 als Zwischenwert [bp-16h]); ohne -Og 3858 Bytes, mit -Oi werden strcpy/strcmp inline.
 * - C++ nur fuer miracle_show_anim: lokale Objekte Anim (O051, 0xaf Bytes) und Bmp (R42) mitten im Block ->
 *   Konstruktoren an der Deklarationsstelle, Destruktoren gb_delete(&b, 2) / Anim_dtor(&o, 2) rueckwaerts am
 *   Blockende.
 * - "h = get_random_alive_hero(g); if (h != 0 && ..)" -> cmp dword [bp-4],0; eingebettet "(h = ..) != 0" -> or ax,dx.
 * - "!(signed char)h->gods_pissed" (unsigned-short-Bitfeld) -> shr ax,5 / and ax,1 / cbw / or ax,ax.
 * - Bonus: "(signed short)(g_god_favor[g] / 100)" -> idiv ebx, dann imul dx (int-Multiplikation).
 * - Boron (Fall 5): "r <= 3 && timer == 0" als eine Bedingung (cmp si / jg / cmp dword / jnz zum naechsten else).
 * - miracle_group_effect: Parameter signed char (mov al,[bp+x] / push ax), Aufrufer "push -1"; long -> push dword.
 * - Lokale Zeiger mit Initialisierer "= 0" -> mov dword ptr [bp-x],0.
 * Originalfehler/Auffaelligkeit: Boron r <= 3 bei laufendem Zeitzaehler INGAME_TIMER_BORON_UNDEAD faellt in den
 *   naechsten else-Zweig (r <= 4: Wiederbelebung) statt nichts zu tun (DSA1: Timer wird ohne Pruefung neu gesetzt).
 */
#include <stdio.h>
#include <string.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:3c5e..3c8c, dsana) - initialisierte Daten in Adressreihenfolge */
signed char g_temple_miracle_bonus[15] = { 0, 2, 15, 10, 20, 5, 10, 1, 15, 3, 15, 5, 10, 0, 1 };
signed char g_temple_miracle_dice[15] = { 0, 9, 9, 10, 17, 6, 10, 10, 18, 10, 19, 8, 15, 0, 10 };
signed char g_temple_god_anim[16] = { 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0 };

void temple_miracle(Temple far *t)
{
	struct hero far *h;
	signed short l3 = 0;
	signed short bonus;
	signed short slot;
	void far *txt_buf = 0;
	char far * far *txt = 0;
	signed short shown = 0;
	signed short r;

	txt_load_list(g_font, 0x17, 0x27, &txt_buf, &txt);
	strcpy(g_text_output_buf, txt[0]);

	bonus = g_temple_miracle_bonus[t->god] * (signed short)(g_god_favor[t->god] / 100) - l3;
	if (g_god_favor[t->god] >= 10)
		g_god_favor[t->god] -= 10;

	if (random_schweif(100) <= g_temple_miracle_dice[t->god] + bonus) {
		r = random_schweif(g_temple_miracle_dice[t->god]);
		if (g_temple_miracle_dice[t->god] == r) {
			miracle_resurrect(t, txt[35]);
		} else {
			switch (t->god) {
			case 1:		/* Praios */
				if (!get_mage_hero(g_group)) {
					if (r <= 5) {
						if (g_ingame_timers[INGAME_TIMER_PRAIOS_MU] == 0) {
							miracle_group_effect(t, ATTRIB_MU, 0x1fa40L, 1, 0);
							strcpy(g_text_output_buf, txt[1]);
							g_ingame_timers[INGAME_TIMER_PRAIOS_MU] = 0x1fa40L;
						}
					} else if (r <= 7) {
						if (g_ingame_timers[INGAME_TIMER_PRAIOS_MR] == 0) {
							h = get_random_alive_hero(g_group);
							if (h != 0 && !(signed char)h->gods_pissed) {
								slot = get_free_effect(h);
								add_effect(h, slot, 0x5eec0L, 0, 1, 5);
								sprintf(g_text_output_buf, txt[2], h->name);
								g_ingame_timers[INGAME_TIMER_PRAIOS_MR] = 0x5eec0L;
							}
						}
					} else if (r <= 8) {
						if ((h = get_renegade_hero(g_group)) != 0 && !(signed char)h->gods_pissed) {
							hero_clear_renegade(h);
							sprintf(g_text_output_buf, txt[3], h->name);
						}
					}
				}
				break;
			case 2:		/* Rondra */
				if (r <= 5) {
					if (g_ingame_timers[INGAME_TIMER_RONDRA_SWORDS] == 0) {
						miracle_group_effect(t, TA_SCHWERTER, 0x5eec0L, 1, 1);
						miracle_group_effect(t, TA_SCHWERTER, 0x5eec0L, 1, 2);
						g_ingame_timers[INGAME_TIMER_RONDRA_SWORDS] = 0x5eec0L;
						strcpy(g_text_output_buf, txt[4]);
					}
				} else if (r <= 7) {
					if (g_ingame_timers[INGAME_TIMER_RONDRA_NO_SPELLS] == 0) {
						g_ingame_timers[INGAME_TIMER_RONDRA_NO_SPELLS] = 0x7e90L;
						strcpy(g_text_output_buf, txt[5]);
					}
				} else if (r <= 8) {
					if (g_ingame_timers[INGAME_TIMER_RONDRA_MAGIC_WEAPON] == 0) {
						group_mark_weapon(g_group, txt[6], 0);
						g_ingame_timers[INGAME_TIMER_RONDRA_MAGIC_WEAPON] = 0x1fa40L;
					}
				}
				break;
			case 3:		/* Efferd */
				if (r <= 5) {
					g_ingame_timers[INGAME_TIMER_EFFERD_FIND_WATER] = 0x5eec0L;
					strcpy(g_text_output_buf, txt[7]);
				} else if (r <= 8) {
					g_ingame_timers[INGAME_TIMER_EFFERD_SAFE_PASSAGE] = 0x5eec0L;
					strcpy(g_text_output_buf, txt[8]);
				} else if (r <= 9) {
					if (g_ingame_timers[INGAME_TIMER_EFFERD_SWIM] == 0) {
						miracle_group_effect(t, TA_SCHWIMMEN, 0x7e900L, 2, 1);
						strcpy(g_text_output_buf, txt[9]);
						g_ingame_timers[INGAME_TIMER_EFFERD_SWIM] = 0x7e900L;
					}
				}
				break;
			case 4:		/* Travia */
				if (r <= 10) {
					group_clr_hunger_thirst(g_group);
					strcpy(g_text_output_buf, txt[10]);
				} else if (r <= 15) {
					miracle_heal(t, dice_roll(1, 6, 2), txt[11]);
				} else if (r <= 16) {
					g_ingame_timers[INGAME_TIMER_TRAVIA_SAFE_REST] = 0xdd7c0L;
					strcpy(g_text_output_buf, txt[12]);
				}
				break;
			case 5:		/* Boron */
#if FIX_0072
				/* Fix #72: r <= 3 bei laufendem Zeitzaehler tut nichts (keine Wiederbelebung) */
				if (r <= 3) {
					if (g_ingame_timers[INGAME_TIMER_BORON_UNDEAD] == 0) {
						g_ingame_timers[INGAME_TIMER_BORON_UNDEAD] = 0x5eec0L;
						strcpy(g_text_output_buf, txt[13]);
					}
				} else if (r <= 4) {
#else
				if (r <= 3 && g_ingame_timers[INGAME_TIMER_BORON_UNDEAD] == 0) {
					g_ingame_timers[INGAME_TIMER_BORON_UNDEAD] = 0x5eec0L;
					strcpy(g_text_output_buf, txt[13]);
				} else if (r <= 4) {
#endif
					miracle_resurrect(t, txt[14]);
				} else if (r <= 5) {
					if (g_ingame_timers[INGAME_TIMER_BORON_TA] == 0) {
						miracle_group_effect(t, ATTRIB_TA, 0x7e900L, -1, 0);
						strcpy(g_text_output_buf, txt[15]);
						g_ingame_timers[INGAME_TIMER_BORON_TA] = 0x7e900L;
					}
				}
				break;
			case 6:		/* Hesinde */
				if (r <= 3) {
					if (g_ingame_timers[INGAME_TIMER_HESINDE_ANALUES] == 0) {
						miracle_group_effect(t, SP_ANALUES, 0x7e900L, 1, 4);
						strcpy(g_text_output_buf, txt[16]);
						g_ingame_timers[INGAME_TIMER_HESINDE_ANALUES] = 0x7e900L;
					}
				} else if (r <= 6) {
					if ((h = get_renegade_hero(g_group)) != 0 && !(signed char)h->gods_pissed) {
						hero_clear_renegade(h);
						sprintf(g_text_output_buf, txt[17], h->name);
					}
				} else if (r <= 7) {
					if (g_ingame_timers[INGAME_TIMER_HESINDE_MR] == 0) {
						miracle_group_effect(t, 0, 0x5eec0L, 5, 5);
						strcpy(g_text_output_buf, txt[18]);
						g_ingame_timers[INGAME_TIMER_HESINDE_MR] = 0x5eec0L;
					}
				}
				break;
			case 7:		/* Firun */
			case 14:	/* Ifirn */
				if (r <= 3) {
					g_ingame_timers[INGAME_TIMER_FIRUN_HUNT] = 0x5eec0L;
					strcpy(g_text_output_buf, txt[19]);
				} else if (r <= 6) {
					g_ingame_timers[INGAME_TIMER_FIRUN_HUNT] = 0xdd7c0L;
					strcpy(g_text_output_buf, txt[19]);
				} else if (r <= 7) {
					if (g_ingame_timers[INGAME_TIMER_FIRUN_SATED] == 0) {
						h = get_random_alive_hero(g_group);
						if (h != 0 && !(signed char)h->gods_pissed) {
							slot = get_free_effect(h);
							add_effect(h, slot, 0xdd7c0L, 0, 1, 6);
							h->hunger = 0;
							h->thirst = 0;
							sprintf(g_text_output_buf, txt[20], h->name);
							g_ingame_timers[INGAME_TIMER_FIRUN_SATED] = 0xdd7c0L;
						}
					}
				}
				break;
			case 8:		/* Tsa */
				if (r <= 10) {
					miracle_heal(t, dice_roll(2, 6, 0), txt[21]);
				} else if (r <= 15) {
					group_add_le(g_group, 1000);
					strcpy(g_text_output_buf, txt[22]);
				} else if (r <= 18) {
					miracle_resurrect(t, txt[23]);
				}
				break;
			case 9:		/* Phex */
				if (g_god_favor[t->god] > 500) {
					if (r <= 5) {
						if (g_ingame_timers[INGAME_TIMER_PHEX_THIEF] == 0) {
							miracle_group_effect(t, TA_TASCHENDIEBSTAHL, 0x5eec0L, 1, 1);
							miracle_group_effect(t, TA_SCHLOESSER, 0x5eec0L, 1, 1);
							strcpy(g_text_output_buf, txt[24]);
							g_ingame_timers[INGAME_TIMER_PHEX_THIEF] = 0x5eec0L;
						}
					} else if (r <= 8) {
						if (g_ingame_timers[INGAME_TIMER_PHEX_FEILSCHEN] == 0) {
							miracle_group_effect(t, TA_FEILSCHEN, 0x5eec0L, 1, 1);
							strcpy(g_text_output_buf, txt[25]);
							g_ingame_timers[INGAME_TIMER_PHEX_FEILSCHEN] = 0x5eec0L;
						}
					} else if (r <= 9) {
						if (g_ingame_timers[INGAME_TIMER_PHEX_FF] == 0) {
							miracle_group_effect(t, ATTRIB_FF, 0x5eec0L, 1, 0);
							strcpy(g_text_output_buf, txt[26]);
							g_ingame_timers[INGAME_TIMER_PHEX_FF] = 0x5eec0L;
						}
					}
				}
				break;
			case 10:	/* Peraine */
				if (r <= 10) {
					miracle_heal(t, dice_roll(1, 6, 0), txt[27]);
				} else if (r <= 16) {
					miracle_heal(t, dice_roll(2, 6, 0), txt[27]);
				} else if (r <= 18) {
					if ((h = get_diseased_hero(g_group)) != 0 && !(signed char)h->gods_pissed) {
						hero_cure_disease(h);
						sprintf(g_text_output_buf, txt[28], h->name);
					}
				}
				break;
			case 11:	/* Ingerimm */
				if (r <= 5) {
					group_dec_weapons_bf(g_group);
					strcpy(g_text_output_buf, txt[29]);
				} else if (r <= 6) {
					if (g_ingame_timers[INGAME_TIMER_INGERIMM_MAGIC_WEAPON] == 0) {
						group_mark_weapon(g_group, txt[30], 0);
						g_ingame_timers[INGAME_TIMER_INGERIMM_MAGIC_WEAPON] = 0x1fa40L;
					}
				} else if (r <= 7) {
					group_mark_weapon(g_group, txt[31], 1);
				}
				break;
			case 12:	/* Rahja */
				if (r <= 8) {
					if (g_ingame_timers[INGAME_TIMER_RAHJA_TALENTS] == 0) {
						miracle_group_effect(t, TA_BETOEREN, 0xdd7c0L, 2, 1);
						miracle_group_effect(t, TA_TANZEN, 0xdd7c0L, 2, 1);
						strcpy(g_text_output_buf, txt[32]);
						g_ingame_timers[INGAME_TIMER_RAHJA_TALENTS] = 0xdd7c0L;
					}
				} else if (r <= 13) {
					if (g_ingame_timers[INGAME_TIMER_RAHJA_CH] == 0) {
						miracle_group_effect(t, ATTRIB_CH, 0x5eec0L, 1, 0);
						strcpy(g_text_output_buf, txt[33]);
						g_ingame_timers[INGAME_TIMER_RAHJA_CH] = 0x5eec0L;
					}
				} else if (r <= 14) {
					if (g_ingame_timers[INGAME_TIMER_RAHJA_TALENTS_PERMANENT] == 0) {
						group_inc_skill(g_group, TA_BETOEREN);
						group_inc_skill(g_group, TA_TANZEN);
						strcpy(g_text_output_buf, txt[34]);
						g_ingame_timers[INGAME_TIMER_RAHJA_TALENTS_PERMANENT] = 0xdd7c0L;
					}
				}
				break;
			}
		}
	}

	if (strcmp(g_text_output_buf, txt[0])) {
		shown = 1;
		miracle_show_anim(t);
	}
	dlg_message(g_dialog, g_text_output_buf);
	if (shown) {
		strcpy(g_text_buf2, g_text_output_buf);
		sprintf(g_text_output_buf, txt[36], g_ttx[t->god + 4], t->kind != 0 ? txt[38] : txt[37], g_text_buf2);
		diary_add(g_diary, g_text_output_buf, 0);
	}
	mem_free(txt_buf);
	mem_free(txt);
	timewarp_minutes(g_date, 15L);
}

void miracle_group_effect(Temple far *t, signed char a, signed long time, signed char b, signed char c)
{
	g_quiet = 1;
	group_add_effect(g_group, time, a, b, c);
	g_quiet = 0;
}

void miracle_heal(Temple far *t, signed short n, char far *fmt)
{
	struct hero far *h;
	signed short max;

	if ((h = get_most_wounded_hero(g_group)) != 0) {
		max = h->le_max - h->le;
		if (n > max)
			n = max;
		add_hero_le(h, n);
		sprintf(g_text_output_buf, fmt, h->name, n, n > 1 ? g_ttx[29] : g_ttx[28]);
	}
}

void miracle_resurrect(Temple far *t, char far *fmt)
{
	struct hero far *h;

	if ((h = get_dead_hero(g_group)) != 0) {
		h->dead = 0;
		add_hero_le(h, 7);
		sprintf(g_text_output_buf, fmt, h->name);
		g_refresh = 1;
	}
}

void miracle_show_anim(Temple far *t)
{
	if (g_anims_on) {
		signed short w;
		signed short hh;

		ani_halt(g_screen_pic);
		mouse_hide(0);
		g_dialog_depth++;
		Anim o(g_temple_god_anim[t->god] == 0 ? 0x7e : 0x7f, 0, 0);
		o.size(&w, &hh);
		Bmp b(w, hh);
		o.setbg(&b);
		b.get(g_vga_buf, g_temple_god_anim[t->god] == 0 ? 0x6c : 0x6a, 0);
		o.rate = 4;
		o.pal_set(0x40, 0x60);
		o.start(g_temple_god_anim[t->god] == 0 ? 0x6c : 0x6a, 0, 0);
		o.wait_frame(0x0f);
		o.wait_cue();
		b.put(g_vga_buf, g_temple_god_anim[t->god] == 0 ? 0x6c : 0x6a, 0);
		g_dialog_depth--;
		mouse_show(0);
		anim_reset(g_screen_pic, 2);
	}
}
