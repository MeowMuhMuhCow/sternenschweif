/* OVL229 - Overlay O229 (Segment 0x23fb in SCHWEIF.EXE): Warenraster der Laeden mit 3x5 Feldern (15 Waren je Seite,
 * Warenliste shop.wares zu 7 Bytes) - shop_compar: qsort-Vergleich zweier Waren nach Gesamtpreis price * unit (long);
 * shop_grid_mark: Markierungsrahmen alt/neu zeichnen (do_border) und Namen der gewaehlten Ware in die Textleiste
 * (Farbe nach group_mark_can_use bei check_use); shop_grid_input: Maus (Trefferpruefung gegen g_merchant_items_posx/
 * _posy) oder Cursortasten mit Seitenwechsel (*changed = 1), Rueckgabe 1 = links aus der ersten Spalte.
 *
 * BCC -mlarge -3 -Ot -c -Y OVL229.C   -> 1088/1088 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Vergleich als verschachtelter ?:-Ausdruck (mov ax,-1 / jmp short statt Epilogkopien) + toter Epilog.
 * - shop_grid_mark: col/old als register (SI/DI), sonst nimmt BCC r nach DI; if ((old = s->old_sel) != s->sel)
 *   eingebettet.
 * - shop_grid_input: Schleifenzaehler i landet ohne -Og in CX (Schleife ohne Aufrufe).
 * - screen_textbar (Prototyp: Farbe unsigned char) hier mit int-Farbe (push word) ueber gecasteten Funktionszeiger.
 * - wares[base + sel + 5] ergibt die Form mov dx,seg / add ax,off / [bx+23h] (Konstante in die Verschiebung gefaltet).
 */
#include "schweif.h"

int shop_compar(const void far *a, const void far *b)
{
	signed long la;
	signed long lb;
	struct ware far *wa;
	struct ware far *wb;

	wa = (struct ware far *)a;
	wb = (struct ware far *)b;
	la = wa->price * wa->unit;
	lb = wb->price * wb->unit;
	return la < lb ? -1 : la == lb ? 0 : 1;
}

void shop_grid_mark(struct shop far *s, signed short check_use)
{
	void far *p;
	signed short r;
	signed short x1, y1, x0, y0;
	register signed short col;
	register signed short old;

	if ((old = s->old_sel) != s->sel) {
		col = 0xff;
		r = 0;
		x1 = s->grid_x[s->sel / 5];
		y1 = s->grid_y[s->sel % 5];
		x0 = s->grid_x[old / 5];
		y0 = s->grid_y[old % 5];
		mouse_hide(0);
		if (s->old_sel != -1)
			do_border(g_vga_buf, x0 - 1, y0 - 1, 0x12, 0x12, 0);
		do_border(g_vga_buf, x1 - 1, y1 - 1, 0x12, 0x12, 0xff);
		screen_clear_textbar(g_screen);
		if (check_use) {
			r = group_mark_can_use(g_group, s->wares[s->sel + s->base].id);
			if (r)
				col = 0xff;
			else
				col = 0xcd;
		}
		if ((p = g_item_names[s->wares[s->sel + s->base].id]) != 0)
			((void (*)(void far *, void far *, signed short))screen_textbar)(g_screen, grammar_name(p), col);
		s->old_sel = s->sel;
		mouse_show(0);
	}
}

signed short shop_grid_input(struct shop far *s, signed short far *changed)
{
	signed short i;
	signed short ret;

	ret = 0;
	if (g_mouse_locked == 0 && (s->mx != g_mouse_posx || s->my != g_mouse_posy)) {
		s->mx = g_mouse_posx;
		s->my = g_mouse_posy;
		for (i = 0; i < 15; i++) {
			if (g_merchant_items_posx[i] <= g_mouse_posx && g_merchant_items_posx[i] + 50 >= g_mouse_posx &&
			    g_merchant_items_posy[i] <= g_mouse_posy && g_merchant_items_posy[i] + 17 >= g_mouse_posy &&
			    s->wares[s->base + i].id != 0) {
				s->sel = i;
				break;
			}
		}
	} else if (g_action == 0x4800) {
		if (s->sel > 0)
			s->sel--;
		else {
			s->sel = 14;
			while (s->wares[s->base + s->sel].id == 0 && s->sel != 0)
				s->sel--;
		}
	} else if (g_action == 0x5000) {
		if (s->sel < 14) {
			s->sel++;
			if (s->wares[s->base + s->sel].id == 0)
				s->sel = 0;
		} else
			s->sel = 0;
	} else if (g_action == 0x4d00) {
		if (s->sel < 10) {
			if (s->wares[s->base + s->sel + 5].id != 0)
				s->sel += 5;
		} else {
			if (s->wares[s->base + 15].id != 0) {
				*changed = 1;
				s->base += 15;
			}
		}
	} else if (g_action == 0x4b00) {
		if (s->sel > 4)
			s->sel -= 5;
		else {
			ret = 1;
			if (s->base != 0) {
				s->base -= 15;
				*changed = 1;
#if FIX_0024
			/* Fix #24: erste Spalte der ersten Seite: Cursor bleibt stehen (sel -= 10 ergab immer 0) */
			}
#else
			} else {
				s->sel -= 10;
				if (s->sel < 0)
					s->sel = 0;
			}
#endif
		}
	}
	return ret;
}
