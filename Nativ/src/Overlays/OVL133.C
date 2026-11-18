/* OVL133 - Overlay O133 (Segment 0x220a in SCHWEIF.EXE): eine Funktion - Aktion einer Kampffigur ausfuehren
 * (vermutlich Kampfanimation): Figur obj samt Partnerhaelfte (2-Feld-Figuren, +0x0b = Partnernummer, <=20 -> +20)
 * per O153_05CF holen, dann switch (action): 1/3/5/7 = Schritt (Koordinaten +-1), 32 = Figur entfernen (Eintrag
 * aus Tabelle ds:c961 loeschen, O016_009E), 41 = Feld aus letztem Pfadpunkt (Bildschirm -> Feld, abgerundete
 * Division durch 20), 34..37 = Blickrichtung setzen (+ Partner versetzen, ds:650c), 33/38/39/40 = globale Werte
 * bzw. O153_0303, 11 = Sonderfaelle fuer Szene 0x493/0x495; danach bei Schritt Schrittgeraeusch (R04_06FA 54h..56h)
 * und Kollisionspruefung O153_09A7 (blockiert -> +0x1d markieren, sonst neue Position uebernehmen).
 *
 * BCC -mlarge -3 -Ot -c -Y OVL133.C   -> 1808/1808 Bytes GLEICH (18.10.2026, erster Versuch), cobjfix laeuft
 *   (Namensfassung mit schweif.h, Welle N7: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * - switch mit Sprungtabelle (Faelle 1..41, Fall-Reihenfolge im Quelltext = Code-Reihenfolge, default: break;).
 * - Schleifenzaehler i und Rechenwert t als register (SI/DI); x2 = y2 = 0 als Kette (y2 zuerst geschrieben).
 * - o2->x = o1->x + g_650c[n].dx; ergibt reine Byte-Addition (add al,[mem]).
 * - return; im Kollisionszweig erzeugt die Epilogkopie mitten in der Funktion (auch ohne -Og).
 */
#include "schweif.h"

void fig_anim_event(struct fight far *f, struct fig_node far *fig, unsigned char event)
{
	signed short x;
	signed short y;
	signed short x2;
	signed short y2;
	signed short a;
	signed short b;
	signed short moved;
	signed short twofield;
	struct enemy far *e;
	struct fig_node far *tail;
	struct fig_node far *head;
	struct fig_node far *q;
	struct hero far *h;
	signed short found;
	register signed short i;
	register signed short t;

	twofield = 0;
	if (fig->twofielded != FIG_TWOFIELD_NONE) {
		twofield = 1;
		if (fig->twofielded <= 20) {
			tail = fig_list_find_twofield(&f->list, fig->twofielded + 20);
			head = fig;
		} else {
			head = fig_list_find_twofield(&f->list, fig->twofielded - 20);
			tail = fig;
		}
		x2 = tail->cbx;
		y2 = tail->cby;
	} else {
		head = fig;
		tail = 0;
		x2 = y2 = 0;
	}
	x = head->cbx;
	y = head->cby;
	moved = 0;

	switch (event) {
	case 1:
		x++;
		x2++;
		moved = 1;
		break;
	case 3:
		y--;
		y2--;
		moved = 1;
		break;
	case 5:
		x--;
		x2--;
		moved = 1;
		break;
	case 7:
		y++;
		y2++;
		moved = 1;
		break;
	case 32:
		/* Figur entfernen */
		if (fig == head) {
			for (i = 0; i < 10; i++) {
				if (g_fig_extra_anims[i] == head->anim) {
					g_fig_extra_anims[i] = 0;
					break;
				}
			}
			FigAnim_dtor(head->anim, 3);
			f->anim_removed = 1;
			g_fig_anim_removed = 1;
		}
		if (fig_list_exists(&f->list, fig->id))
			fig->to_remove = 1;
		break;
	case 41:
		/* Feld aus dem letzten Pfadpunkt (Bildschirm -> Feld) */
		a = fig->anim->seq.path[fig->anim->path_pos - 1].x;
		b = fig->anim->seq.path[fig->anim->path_pos - 1].y;
		t = a + b * 2 - 213;
		if (t < 0)
			t -= 19;
		fig->cbx = t / 20 - 1;
		t = a - b * 2 + 251;
		if (t < 0)
			t -= 19;
		fig->cby = t / 20 - 1;
		fig->to_resort = 1;
		break;
	case 34:
		e = fig_get_enemy(head->id);
		e->viewdir = 0;
		if (twofield) {
			tail->cbx = head->cbx + g_fig_tail_dxy[0].dx;
			tail->cby = head->cby + g_fig_tail_dxy[0].dy;
			tail->to_resort = 1;
		}
		break;
	case 35:
		e = fig_get_enemy(fig->id);
		e->viewdir = 1;
		if (twofield) {
			tail->cbx = head->cbx + g_fig_tail_dxy[1].dx;
			tail->cby = head->cby + g_fig_tail_dxy[1].dy;
			tail->to_resort = 1;
		}
		break;
	case 36:
		e = fig_get_enemy(fig->id);
		e->viewdir = 2;
		if (twofield) {
			tail->cbx = head->cbx + g_fig_tail_dxy[2].dx;
			tail->cby = head->cby + g_fig_tail_dxy[2].dy;
			tail->to_resort = 1;
		}
		break;
	case 37:
		e = fig_get_enemy(fig->id);
		e->viewdir = 3;
		if (twofield) {
			tail->cbx = head->cbx + g_fig_tail_dxy[3].dx;
			tail->cby = head->cby + g_fig_tail_dxy[3].dy;
			tail->to_resort = 1;
		}
		break;
	case 33:
		g_sfight_pos = fig->anim->seq_id;
		break;
	case 38:
		g_sfight_ofs_x = g_sfight_ofs2_x;
		g_sfight_ofs_y = g_sfight_ofs2_y;
		fig_list_show(&f->list, g_sfight_figs[0]);
		break;
	case 39:
		g_sfight_ofs_x = g_sfight_ofs3_x;
		g_sfight_ofs_y = g_sfight_ofs3_y;
		break;
	case 40:
		g_sfight_ofs_x = g_sfight_ofs2_x;
		g_sfight_ofs_y = g_sfight_ofs2_y;
		fig_list_show(&f->list, g_sfight_figs[0]);
		fig_list_show(&f->list, g_sfight_fig2);
		break;
	case 11:
		/* Sonderkaempfe: Treffer */
		if (f->id == FIGHT_ID_SPECIAL) {
			e = g_enemies;
			if (fig->id == g_sfight_fig3) {
				h = enemy_target_hero(e);
				if (h) {
					q = fig_list_find(&f->list, h->fighter_id);
					fight_set_cell(f, q->cbx, q->cby, q->obj_id);
					q->to_remove = 1;
					sub_hero_le(h, h->le);
				}
			}
		} else if (f->id == FIGHT_ID_SPECIAL2) {
			if (fig->kind == FIG_KIND_ENEMY) {
				found = 1;
				h = get_first_hero(g_group);
				while (!fight_find_cell(get_hero_index(h) + FIG_ID_HERO, &a, &b)) {
					h = get_next_hero(g_group, h);
					if (get_first_hero(g_group) == h) {
						found = 0;
						break;
					}
				}
				if (found) {
					q = fig_list_find(&f->list, h->fighter_id);
					fight_set_cell(f, q->cbx, q->cby, q->obj_id);
					q->to_remove = 1;
					sub_hero_le(h, h->le);
				}
			}
		}
		break;
	default:
		break;
	}

	if (moved) {
		/* Schrittgeraeusch */
		if (head->kind == FIG_KIND_HERO)
			snd_play2(g_snd_fx, 0x54);
		else if (head->kind == FIG_KIND_ENEMY) {
			e = fig_get_enemy(head->id);
			snd_play2(g_snd_fx, e->is_animal ? 0x56 : (enemy_is_humanoid(e) ? 0x54 : 0x55));
		}
		if (fight_cell_blocked(x, y)) {
			head->to_remove = 1;
			if (twofield)
				tail->to_remove = 1;
			return;
		}
		head->cbx = x;
		head->cby = y;
		head->to_resort = 1;
		if (twofield) {
			tail->cbx = x2;
			tail->cby = y2;
			tail->to_resort = 1;
		}
	}
}
