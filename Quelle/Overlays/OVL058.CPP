/* OVL058 - Overlay O058 (Segment 0x2059 in SCHWEIF.EXE): Helden fuer Aufgaben vorwaehlen; Ergebnis ist die Position
 * in der Auswahlliste (Helden h != 0, check_hero(h), Index != g_hero_excluded), gespeichert in g_dialog_sel (= Dlg.sel)
 * per inline-Setter dlg_preselect().
 * preselect_best_skill/preselect_best_attrib(g, v): Held get_best_hero_skill(g, v) bzw. get_hero_attrib_max(g, v);
 * preselect_hero(g, h): Held h.
 * preselect_agile_hero(g): Koerperbeherrschung (drei Probeneigenschaften g_skill_attribs[t] + get_skill_value) >= 0 ->
 *   Wertung MU (+ GE), jeweils nur wenn > 4, in eine lokale Tabelle (Vorlage g_score7_zero); Bester vorwaehlen.
 * preselect_hunter(g): Waffen (Armbrust 0x0c/0x101 mit Bolzen, Bogen 0x09/0x13/0x100 mit Pfeilen, Speer 0x05/0xd8)
 *   -> Art 1..3, Wert Schleichen + Schusswaffen bzw. Wurfwaffen, Bester vorwaehlen.
 * preselect_for_task(g, mode): Wertung je mode (0: KK * 4 + LE / 3, 1: Faehrtensuchen + Wildnisleben, 2: Pflanzenkunde)
 *   mit Abzuegen fuer rest_food/rest_herbs und watches, Helden mit rest_magic nicht; Bester vorwaehlen.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL058.CPP   -> 2546/2546 Bytes GLEICH (18.10.2026, erster Versuch), cobjfix
 * laeuft (ds:56fc ohne eigenes Public -> cobjfix setzt _s_56f2+0xa)
 *   (Namensfassung mit schweif.h, Welle N15: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * - C++ nur wegen des inline-Setters dlg_preselect(v) { g_dialog_sel = v; } -> Parameterkopie (mov [bp-x],di /
 *   mov ax,[bp-x]). g_dialog_sel ist ein eigenes Public (nicht ((struct dlg_view far *)g_dialog)->sel).
 * - Eigenschaftswert als Makro ATTR(h, i) = (signed char)(attrib[i].cur + attrib[i].mod) (add al / cbw), KEINE
 *   inline-Funktion: bei konstantem Talent legt -Og (int)g_skill_attribs[t].a1..a3 als Zwischenwert ab
 *   ([bp-12h]...), bei variablem Talent (DI) wird ohne Zwischenwert neu gerechnet; dritter Summand mit les si (DX
 *   haelt die Summe).
 * - "h->attrib[ATTRIB_MU].cur - 4 > 0" + "score.v[k] = h->attrib[ATTRIB_MU].cur" -> -Og-Zwischenwert, add ax,0fffch / jng.
 * - Lokale Tabelle als Struktur "struct score7 score = g_score7_zero;" -> mov si,56fch / lea di / push ss / pop es /
 *   rep movsw; liegt als Aggregat hinter allen Skalaren, der Parameterkopie und den -Og-Zwischenwerten.
 * - Statusbyte +ba: -Og-Zwischenwert [bp-11h]; "!(signed char)h->rest_magic" -> and/cbw/or ax,ax, die anderen Bits
 *   als (unsigned char) -> shr/and/or al,al.
 * - Originalfehler: best in preselect_hunter/preselect_for_task nicht initialisiert (nur gesetzt, wenn ein Held punktet).
 */
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:56fc..570a, dsana) - initialisierte Daten in Adressreihenfolge */
struct score7 g_score7_zero = { { 0, 0, 0, 0, 0, 0, 0 } };

#define ITEM_SPEAR	0x05	/* Speer (BE-Nummer) */
#define ITEM_SHORTBOW	0x09	/* Kurzbogen */
#define ITEM_ARROWS	0x0a	/* Pfeile */
#define ITEM_CROSSBOW	0x0c	/* Armbrust */
#define ITEM_BOLTS	0x0d	/* Bolzen */
#define ITEM_LONGBOW	0x13	/* Langbogen */

inline void dlg_preselect(signed short v) { g_dialog_sel = v; }

#define ATTR(h, i) ((signed char)((h)->attrib[i].cur + (h)->attrib[i].mod))
#define TAL(h, t) (ATTR(h, g_skill_attribs[t].a1) + ATTR(h, g_skill_attribs[t].a2) + \
	ATTR(h, g_skill_attribs[t].a3) + get_skill_value(h, t))

/* den besten Helden fuer Talent skill vorwaehlen */
void preselect_best_skill(struct group far *g, signed short skill)
{
	struct hero far *t;
	struct hero far *h;
	register signed short i;
	register signed short n;

	n = 0;
	t = get_best_hero_skill(g, skill);
	for (i = 0, h = g->heroes[0]; i <= 6; i++, h = g->heroes[i]) {
		if (h && check_hero(h) && g_hero_excluded != i) {
			if (h == t)
				break;
			n++;
		}
	}
	dlg_preselect(n);
}

/* den Helden mit der groessten Eigenschaft attrib vorwaehlen */
void preselect_best_attrib(struct group far *g, signed short attrib)
{
	struct hero far *t;
	struct hero far *h;
	register signed short i;
	register signed short n;

	n = 0;
	t = get_hero_attrib_max(g, attrib);
	for (i = 0, h = g->heroes[0]; i <= 6; i++, h = g->heroes[i]) {
		if (h && check_hero(h) && g_hero_excluded != i) {
			if (h == t)
				break;
			n++;
		}
	}
	dlg_preselect(n);
}

/* Helden t vorwaehlen */
void preselect_hero(struct group far *g, struct hero far *t)
{
	struct hero far *h;
	register signed short i;
	register signed short n;

	n = 0;
	for (i = 0, h = g->heroes[0]; i <= 6; i++, h = g->heroes[i]) {
		if (h && check_hero(h) && g_hero_excluded != i) {
			if (h == t)
				break;
			n++;
		}
	}
	dlg_preselect(n);
}

/* gewandten Helden vorwaehlen: Koerperbeherrschung gelingt rechnerisch (>= 0), Wertung MU (+ GE), je nur ueber 4 */
void preselect_agile_hero(struct group far *g)
{
	signed short i;
	signed short k = 0;
	signed short val = 0;
	signed short best = 0;
	signed short max = -1;
	struct hero far *h;
	struct score7 score = g_score7_zero;

	h = 0;
	for (i = 0, h = g->heroes[0]; i <= 6; i++, h = g->heroes[i]) {
		if (h && check_hero3(h)) {
			val = TAL(h, TA_KOERPERBEHERRSCHUNG);
			if (val >= 0 && h->attrib[ATTRIB_MU].cur - 4 > 0) {
				score.v[k] = h->attrib[ATTRIB_MU].cur;
				if (h->attrib[ATTRIB_GE].cur - 4 > 0)
					score.v[k] += h->attrib[ATTRIB_GE].cur;
			}
			k++;
		}
	}
	for (i = 0; i < 7; i++) {
		if (score.v[i] > max) {
			max = score.v[i];
			best = i;
		}
	}
	dlg_preselect(best);
}

/* Jaeger vorwaehlen: Armbrust mit Bolzen, Bogen mit Pfeilen bzw. Speer; Wertung Schleichen + Schuss-/Wurfwaffen */
void preselect_hunter(struct group far *g)
{
	signed char kind;
	signed short r;
	signed short i;
	signed short val;
	signed short max;
	signed short n;
	struct hero far *h;
	struct hero far *best;
#if FIX_0010
	/* Fix #10: best vorbelegen (kein Held punktet) */
	register signed short t;

	best = 0;
	max = 0;
#else
	register signed short t;

	max = 0;
#endif
	for (i = 0, h = g->heroes[0]; i <= 6; i++, h = g->heroes[i]) {
		if (h && check_hero3(h)) {
			kind = 0;
			r = get_item_pos(h, ITEM_BOLTS);
			if ((get_item_pos(h, ITEM_CROSSBOW) != -1 || get_item_pos(h, 0x101) != -1) && r != -1)
				kind = 1;
			else {
				r = get_item_pos(h, ITEM_ARROWS);
				if ((get_item_pos(h, ITEM_SHORTBOW) != -1 || get_item_pos(h, ITEM_LONGBOW) != -1 || get_item_pos(h, 0x100) != -1) && r != -1)
					kind = 2;
				else if (get_item_pos(h, ITEM_SPEAR) != -1 || get_item_pos(h, 0xd8) != -1)
					kind = 3;
			}
			if (kind != 0) {
				if (kind == 3)
					t = TA_WURFWAFFEN;
				else
					t = TA_SCHUSSWAFFEN;
				val = TAL(h, TA_SCHLEICHEN) + TAL(h, t);
				if (val > max) {
					max = val;
					best = h;
				}
			}
		}
	}
	n = 0;
	for (i = 0, h = g->heroes[0]; i <= 6; i++, h = g->heroes[i]) {
		if (h && check_hero(h) && g_hero_excluded != i) {
			if (h == best)
				break;
			n++;
		}
	}
	if (best)
		dlg_preselect(n);
}

/* Helden fuer eine Aufgabe vorwaehlen (vermutet Lager): 0 KK * 4 + LE / 3, 1 Faehrtensuchen + Wildnisleben,
 * 2 Pflanzenkunde; Abzuege fuer rest_food/rest_herbs/watches, Helden mit rest_magic nicht */
void preselect_for_task(struct group far *g, signed char mode)
{
	signed short i;
	signed short max;
	signed short n;
	struct hero far *h;
	struct hero far *best;
#if FIX_0010
	/* Fix #10: best vorbelegen (kein Held punktet) */
	register signed short val;

	best = 0;
	max = 0;
#else
	register signed short val;

	max = 0;
#endif
	for (i = 0, h = g->heroes[0]; i <= 6; i++, h = g->heroes[i]) {
		if (h && check_hero3(h) && !(signed char)h->rest_magic) {
			val = 0;
			switch (mode) {
			case 0:
				if ((unsigned char)h->rest_food || h->watches != 0)
					val -= 20;
				if ((unsigned char)h->rest_herbs)
					val -= 30;
				val = val + h->attrib[ATTRIB_KK].cur * 4 + h->le / 3;
				break;
			case 1:
				if ((unsigned char)h->rest_food)
					val = -100;
				else {
					if (h->watches != 0)
						val -= 30;
					if ((unsigned char)h->rest_herbs)
						val -= 50;
					val += TAL(h, TA_FAEHRTENSUCHEN) + TAL(h, TA_WILDNISLEBEN);
				}
				break;
			case 2:
				if ((unsigned char)h->rest_food || h->watches != 0)
					val -= 20;
				else
					val += TAL(h, TA_PFLANZENKUNDE);
				break;
			}
			if (val > max) {
				max = val;
				best = h;
			}
		}
	}
	n = 0;
	for (i = 0, h = g->heroes[0]; i <= 6; i++, h = g->heroes[i]) {
		if (h && check_hero(h) && g_hero_excluded != i) {
			if (h == best)
				break;
			n++;
		}
	}
	if (best)
		dlg_preselect(n);
}
