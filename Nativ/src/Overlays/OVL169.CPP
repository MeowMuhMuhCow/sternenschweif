/* OVL169 - Overlay O169 (Segment 0x22ad in SCHWEIF.EXE): Heldenerschaffung (class CharGen: Held g->hero, Texte g->txt,
 * Typnamen g_ttx). chargen_new_hero(g): Typ waehlen (Rueckfrage txt[58], Auswahl txt[15] aus 12 Namen g_ttx[t + 1..12],
 * t = 0x3e/0x32 je nach Geschlecht), Held loeschen (Name und Geschlecht bleiben, chargen_reset_incs), Eigenschaften
 * auswuerfeln (MU..KK: W 8..13, > 8 -> -1; AG..JZ: W 2..7, < 7 -> +1; je set_attrib), Typvoraussetzungen
 * g->type_reqs[typ][0..5] {Eigenschaft, Wert; TYPE_REQ_MAX = Hoechstwert, sonst Mindestwert} erzwingen, Typbild
 * chargen_draw_type_pic, g->pic_type = min(Typ, 10), Portraetbereich aus g_portrait_index_m/_f (nach Geschlecht),
 * hero.portrait, chargen_set_type_values. chargen_move_attribs(g): Eigenschaften verschieben - ggf. Typ verwerfen
 * (txt[23], Sternzeichen-Boni MU/CH per change_attrib_normal zuruecknehmen), dann Schleife: Eigenschaft waehlen
 * (g_ttx[308..314]), Richtung (g->attr_moved[a] bzw. Auswahl txt[25]/[26]); heben (max. 13) verlangt zwei Punkte
 * bei den negativen Eigenschaften (bis 8), senken (min. 8) zwei Punkte dort (ab 2), Merker attr_moved = +-1.
 * chargen_save_bg(g): drei Hintergrundausschnitte je nach Seite g->page (switch 0..10) aus g->bg sichern (new Bmp,
 * gb_save; alte vorher loeschen), chargen_restore_bg(g, dst) zurueckschreiben (gb_blit). chargen_inc_skill(g, i, max,
 * msg): Steigerungsversuch fuer Talent i (g->skill_incs[i] {Fehlversuche, Steigerungen}): Grenze max -> msg,
 * 3 Fehlversuche -> txt[42], sonst skill_incs - 1, W 2..12 > Talentwert -> add_skill_value (txt[43]), sonst
 * Fehlversuch (txt[44]). chargen_toggle_sex(g): Geschlecht umschalten (Portraetbereich neu, chargen_draw_portrait,
 * Symbol bmp_male/bmp_female nach g_vga_buf (305, 7)).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL169.CPP   -> 4675/4675 Bytes GLEICH (18.10.2026), cobjfix laeuft
 * - -Oi fuer strcpy/memset (rep stosw + stosb fuer 0x5ef); -Og fuer Zwischenwerte (typ * 12 [bp-24h],
 *   v & 0x7f [bp-25h], &g->attr_moved[j] [bp-10h]).
 * - C++ wegen inline-Setter hero::set_portrait/set_type/set_sex/set_attrib_normal(i, v): jede Expansion legt eigene Parameterkopien an
 *   ([bp-0ch], [bp-0dh], [bp-10h]/[bp-11h], ... [bp-22h]), das Namensfeld nm[20] liegt dahinter ([bp-3ah]).
 * - "hero->skill_incs += (signed char)-1" ergibt add byte 0ffh nur mit unsigned char skill_incs; als signed char macht BCC
 *   (mit jeder Schreibweise) dec byte daraus.
 * - "if ((t = g->attr_moved[a]) == 0)" eingebettet (mov [bp-0eh],ax / or ax,ax); Auswahl wiederholen bei -2 als
 *   innere do-while-Schleife (Ruecksprung direkt auf den Aufruf, nicht auf die Zaehlerpruefung).
 * - switch mit gemeinsamen Rumpfen (case 1-3, 5-10) -> Sprungtabelle mit gleichen Eintraegen.
 * Originalfehler: chargen_move_attribs - hat der Held noch keinen Typ (type == 0), wird "again" nie gesetzt und
 *   die Schleife haengt vom Stapelinhalt ab. chargen_save_bg/chargen_restore_bg: fuer g->page ausserhalb 0..10 bleiben die Koordinaten/
 *   Groessen uninitialisiert (Fall 4 setzt nur die beiden letzten Ausschnitte, x1 = 0 schuetzt den ersten).
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 */
#include <string.h>
#include "schweif.h"

void chargen_new_hero(CharGen far *g)
{
	signed short r;
	signed short i;
	signed short v;
	signed short t;
	signed short k;
	signed char c;
	char nm[20];

	g_quiet = 1;
	if (dlg_yes_no(g_dialog, g->txt[58]) == 1) {
		t = g->hero->sex ? 0x3e : 0x32;
		r = dlg_choose(g_dialog, g->txt[15], 12, g_ttx[t + 1], g_ttx[t + 2], g_ttx[t + 3],
			g_ttx[t + 4], g_ttx[t + 5], g_ttx[t + 6], g_ttx[t + 7], g_ttx[t + 8],
			g_ttx[t + 9], g_ttx[t + 10], g_ttx[t + 11], g_ttx[t + 12]);
		if (r != -1) {
			strcpy(nm, g->hero->name);
			c = g->hero->sex;
			memset(g->hero, 0, sizeof(struct hero));
			chargen_reset_incs(g);
			g->hero->set_sex(c);
			strcpy(g->hero->name, nm);
			strcpy(g->hero->file_name, nm);
			g->hero->set_type(r);
			for (i = ATTRIB_MU; i <= ATTRIB_KK; i++) {
				v = random_interval(8, 13);
				if (v > 8)
					v--;
				g->hero->set_attrib_normal(i, v);
				set_attrib(g->hero, i, v);
			}
			for (i = ATTRIB_AG; i <= ATTRIB_JZ; i++) {
				v = random_interval(2, 7);
				if (v < 7)
					v++;
				g->hero->set_attrib_normal(i, v);
				set_attrib(g->hero, i, v);
			}
			for (i = 0; i < 6; i++) {
				k = g->type_reqs[r][i].attrib;
				v = g->type_reqs[r][i].value;
				if (v) {
					if (v & TYPE_REQ_MAX) {
						if (g->hero->attrib[k].normal > (v & 0x7f)) {
							g->hero->set_attrib_normal(k, v & 0x7f);
							set_attrib(g->hero, k, v & 0x7f);
						}
					} else if (g->hero->attrib[k].normal < v) {
						g->hero->set_attrib_normal(k, v);
						set_attrib(g->hero, k, v);
					}
				}
			}
			chargen_draw_type_pic(g, g_vga_buf, g->hero->type);
			g->pic_type = g->hero->type > HERO_TYPE_GREEN_ELF ? HERO_TYPE_GREEN_ELF : g->hero->type;
			if (g->hero->sex) {
				g->portrait_first = g->portrait = g_portrait_index_f[g->pic_type];
				g->portrait_last = g_portrait_index_m_next[g->pic_type] - 1;
				g->hero->set_portrait(g->portrait);
			} else {
				g->portrait_first = g->portrait = g_portrait_index_m[g->pic_type];
				g->portrait_last = g_portrait_index_f[g->pic_type] - 1;
				g->hero->set_portrait(g->portrait);
			}
			chargen_set_type_values(g);
			g_refresh = 1;
		}
	}
	g_quiet = 0;
}

void chargen_move_attribs(CharGen far *g)
{
	signed short cnt;
	signed short a;
	signed short mode;
	signed char v;
	signed char w;
	signed short sum;
	signed short again;
	signed short t;
	register signed short j;
	register signed short i;

	g_quiet = 1;
	g->show_moved = 1;
	if (!g->hero->attrib[ATTRIB_MU].normal)
		dlg_message(g_dialog, g->txt[13]);
	else if (!chargen_can_move_attribs(g))
		dlg_message(g_dialog, g->txt[60]);
#if FIX_0107
	/* Fix #107: ohne Typ direkt verschieben (again war uninitialisiert) */
	else {
		again = 1;
		if (g->hero->type) {
			again = 0;
#else
	else {
		if (g->hero->type) {
			again = 0;
#endif
			if (dlg_yes_no(g_dialog, g->txt[23]) == 1) {
				g->hero->type = 0;
				if (g->mu_bonus) {
					change_attrib_normal(g->hero, ATTRIB_MU, -1);
					g->mu_bonus = 0;
				}
				if (g->ch_bonus) {
					change_attrib_normal(g->hero, ATTRIB_CH, -1);
					g->ch_bonus = 0;
				}
				g_refresh = 1;
				chargen_redraw(g);
				again = 1;
			}
		}
		while (again) {
			if (!chargen_can_move_attribs(g)) {
				dlg_message(g_dialog, g->txt[60]);
				break;
			}
			a = dlg_choose(g_dialog, g->txt[28], 7, g_ttx[308 + ATTRIB_MU], g_ttx[309], g_ttx[310], g_ttx[311],
				g_ttx[312], g_ttx[313], g_ttx[314]) - 1;
			if (a == -2)
				break;
			if ((t = g->attr_moved[a]) == 0) {
				mode = dlg_choose(g_dialog, (char far *)0, 2, g->txt[25], g->txt[26]);
				if (mode == -1)
					break;
			} else
				mode = t;
			v = g->hero->attrib[a].normal;
			if (mode == 1) {
				if (v == 13)
					dlg_message(g_dialog, g->txt[27]);
				else {
					sum = 0;
					for (i = ATTRIB_AG; i <= ATTRIB_JZ; i++) {
						if (g->attr_moved[i] != -1 && g->hero->attrib[i].normal < 8)
							sum += 8 - g->hero->attrib[i].normal;
					}
					if (sum < 2)
						dlg_message(g_dialog, g->txt[35]);
					else {
						change_attrib_normal(g->hero, a, 1);
						g->attr_moved[a] = 1;
						chargen_redraw(g);
						cnt = 0;
						while (cnt != 2) {
							do
								j = dlg_choose(g_dialog, g->txt[30], 7, g_ttx[315], g_ttx[316],
									g_ttx[317], g_ttx[318], g_ttx[319], g_ttx[320],
									g_ttx[321]) - 1;
							while (j == -2);
							j += ATTRIB_AG;
							if (g->attr_moved[j] == -1)
								dlg_message(g_dialog, g->txt[33]);
							else {
								w = g->hero->attrib[j].normal;
								if (w == 8)
									dlg_message(g_dialog, g->txt[27]);
								else {
									cnt++;
									g->attr_moved[j] = 1;
									change_attrib_normal(g->hero, j, 1);
									chargen_redraw(g);
								}
							}
						}
					}
				}
			} else {
				if (v == 8)
					dlg_message(g_dialog, g->txt[31]);
				else {
					sum = 0;
					for (i = ATTRIB_AG; i <= ATTRIB_JZ; i++) {
						if (g->attr_moved[i] != 1 && g->hero->attrib[i].normal > 2)
							sum += g->hero->attrib[i].normal - 2;
					}
					if (sum < 2)
						dlg_message(g_dialog, g->txt[35]);
					else {
						change_attrib_normal(g->hero, a, -1);
						g->attr_moved[a] = -1;
						chargen_redraw(g);
						cnt = 0;
						while (cnt != 2) {
							do
								j = dlg_choose(g_dialog, g->txt[29], 7, g_ttx[315], g_ttx[316],
									g_ttx[317], g_ttx[318], g_ttx[319], g_ttx[320],
									g_ttx[321]) - 1;
							while (j == -2);
							j += ATTRIB_AG;
							if (g->attr_moved[j] == 1)
								dlg_message(g_dialog, g->txt[32]);
							else {
								w = g->hero->attrib[j].normal;
								if (w == 2)
									dlg_message(g_dialog, g->txt[31]);
								else {
									cnt++;
									change_attrib_normal(g->hero, j, -1);
									g->attr_moved[j] = -1;
									chargen_redraw(g);
								}
							}
						}
					}
				}
			}
		}
	}
	g_quiet = 0;
	g->show_moved = 0;
}

void chargen_save_bg(CharGen far *g)
{
	signed short x3;
	signed short y1;
	signed short y2;
	signed short y3;
	signed short w1;
	signed short w2;
	signed short w3;
	signed short h1;
	signed short h2;
	signed short h3;
	register signed short x1;
	register signed short x2;

	if (g->bmp_save1) {
		delete g->bmp_save1;
		g->bmp_save1 = 0;
	}
	if (g->bmp_save2) {
		delete g->bmp_save2;
		g->bmp_save2 = 0;
	}
	if (g->bmp_save3) {
		delete g->bmp_save3;
		g->bmp_save3 = 0;
	}
	x1 = 0;
	switch (g->page) {
	case CHARGEN_PAGE_MAIN:
		x1 = 0xb0;
		y1 = 0x0c;
		w1 = 0x5e;
		h1 = 8;
		x2 = 0xcd;
		y2 = 0x49;
		w2 = 0x14;
		h2 = 0x5a;
		x3 = 0x111;
		y3 = 0x49;
		w3 = 0x14;
		h3 = 0x5a;
		break;
	case 1:
	case 2:
	case 3:
		x1 = 0x10e;
		y1 = 0xb8;
		w1 = 0x0f;
		h1 = 8;
		x2 = 0xce;
		y2 = 0x28;
		w2 = 0x18;
		h2 = 0x8a;
		x3 = 0x128;
		y3 = 0x28;
		w3 = 0x18;
		h3 = 0x8a;
		break;
	case CHARGEN_PAGE_ATPA:
		x2 = 0xe1;
		y2 = 0x30;
		w2 = 0x14;
		h2 = 0x82;
		x3 = 0x104;
		y3 = 0x30;
		w3 = 0x14;
		h3 = 0x82;
		break;
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
		x1 = 0xd7;
		y1 = 0xb8;
		w1 = 0x0f;
		h1 = 8;
		x2 = 0x8d;
		y2 = 0x2a;
		w2 = 0x14;
		h2 = 0x8c;
		x3 = 0x11e;
		y3 = 0x2a;
		w3 = 0x14;
		h3 = 0x8c;
		break;
	}
	if (x1) {
		g->bmp_save1 = new Bmp(w1, h1);
		gb_save(g->bmp_save1, g->bg, x1, y1);
	}
	g->bmp_save2 = new Bmp(w2, h2);
	gb_save(g->bmp_save2, g->bg, x2, y2);
	g->bmp_save3 = new Bmp(w3, h3);
	gb_save(g->bmp_save3, g->bg, x3, y3);
}

void chargen_restore_bg(CharGen far *g, void far *dst)
{
	signed short x3;
	signed short y1;
	signed short y2;
	signed short y3;
	register signed short x1;
	register signed short x2;

	x1 = 0;
	switch (g->page) {
	case CHARGEN_PAGE_MAIN:
		x1 = 0xb0;
		y1 = 0x0c;
		x2 = 0xcd;
		y2 = 0x49;
		x3 = 0x111;
		y3 = 0x49;
		break;
	case 1:
	case 2:
	case 3:
		x1 = 0x10e;
		y1 = 0xb8;
		x2 = 0xce;
		y2 = 0x28;
		x3 = 0x128;
		y3 = 0x28;
		break;
	case CHARGEN_PAGE_ATPA:
		x2 = 0xe1;
		y2 = 0x30;
		x3 = 0x104;
		y3 = 0x30;
		break;
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
		x1 = 0xd7;
		y1 = 0xb8;
		x2 = 0x8d;
		y2 = 0x2a;
		x3 = 0x11e;
		y3 = 0x2a;
		break;
	}
	if (x1)
		gb_blit(g->bmp_save1, dst, x1, y1);
	gb_blit(g->bmp_save2, dst, x2, y2);
	gb_blit(g->bmp_save3, dst, x3, y3);
}

void chargen_inc_skill(CharGen far *g, register signed short i, signed short lim, char far *msg)
{
	g_quiet = 1;
	if (g->skill_incs[i].incs >= lim)
		dlg_message(g_dialog, msg);
	else if (g->skill_incs[i].tries == 3)
		dlg_message(g_dialog, g->txt[42]);
	else {
		g->hero->skill_incs += (signed char)-1;
		if (random_interval(2, 12) > get_skill_value(g->hero, i)) {
			dlg_message(g_dialog, g->txt[43]);
			add_skill_value(g->hero, i, 1);
			g->skill_incs[i].tries = 0;
			g->skill_incs[i].incs++;
			chargen_redraw(g);
		} else {
			dlg_message(g_dialog, g->txt[44]);
			g->skill_incs[i].tries++;
			chargen_redraw(g);
		}
	}
	g_quiet = 0;
}

void chargen_toggle_sex(CharGen far *g)
{
	g->hero->set_sex(g->hero->sex ^ 1);
	mouse_hide(0);
	if (g->hero->type) {
		if (g->hero->sex) {
			g->portrait_first = g->portrait = g_portrait_index_f[g->pic_type];
			g->portrait_last = g_portrait_index_m_next[g->pic_type] - 1;
			g->hero->set_portrait(g->portrait);
		} else {
			g->portrait_first = g->portrait = g_portrait_index_m[g->pic_type];
			g->portrait_last = g_portrait_index_f[g->pic_type] - 1;
			g->hero->set_portrait(g->portrait);
		}
		chargen_draw_portrait(g);
	}
	if (g->hero->sex)
		gb_blit(g->bmp_female, g_vga_buf, 0x131, 7);
	else
		gb_blit(g->bmp_male, g_vga_buf, 0x131, 7);
	mouse_show(0);
}
