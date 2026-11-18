/* OVL255 - Overlay O255 (Segment 0x2489 in SCHWEIF.EXE): 3D-Ansicht - Verdeckung. Eine einzige Funktion
 * view_cull_hidden (von view_draw, O198, zwischen den beiden Durchlaeufen gerufen): prueft die Verdeckungsflags der 9x9
 * Nachbarzellen g_near_occl[r*9+c] (Gruppe steht bei 4,4) und loescht in g_near_objs[r*9+c] (Objekt-Nr. + 1) die
 * Eintraege der Zellen, die dahinter liegen und daher nicht gezeichnet werden. Zuerst die vier Diagonalen
 * (3,3/2,2/1,1 usw.) und Achsen in je drei Stufen, dann die vier Quadranten, zuletzt die Randzellen;
 * Merker l01..l21 und rd/rb/rc halten fest, welche Bereiche schon ganz verdeckt sind.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL255.C   -> 4224/4224 Bytes GLEICH (18.10.2026), cobjfix laeuft.
 *   -2 statt -3 und zusaetzlich -Oi ebenfalls GLEICH; ohne -Og 20 Bytes zu kurz, mit -O 48 Bytes zu lang.
 * - Funktion ohne Aufrufe: 36 char-Merker in EINER Zuweisungskette "l01 = ... = 0" (rechts->links gespeichert,
 *   daher [bp-21h] zuerst). Die drei Merker in CL/BL/DL sind "register signed char rd, rb, rc;" - die
 *   Deklarationsreihenfolge bestimmt die Register: erste -> DL, zweite -> BL, dritte -> CL. Ohne register
 *   sucht BCC sich selbst drei (andere) Variablen fuer DL/BL/CL aus und der Stack verschiebt sich.
 * - "!merker" -> mov al / cbw / or ax,ax; mehrfach im selben Block -> -Og-Zwischenwert (int)merker in SI
 *   (l11), DI (rd), danach auf dem Stack [bp-24h] (rb) und [bp-26h] (rc): "or si,si" bzw. "cmp word [bp-24h],0".
 *   "if (a && b)" mit char-Merkern bzw. Tabellenfeldern -> cmp byte ..,0.
 * - Loeschen mehrerer Zellen = Zuweisungskette (mov al,0 / mov [..],al), einzelne Zelle mov byte [..],0.
 * - else-if-Kette am Funktionsende -> mit -Og Epilogkopien (pop di / pop si / leave / retf).
 * - Urspruenglich erzeugt mit tools/gen/gen255.py (Namen g_b4a6/g_b4f7); diese C-Datei ist jetzt massgeblich.
 *   (Namensfassung mit schweif.h, Welle N19: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * Auffaellig (Original): die vier Quadranten sind sonst spiegelbildlich gleich (per Skript geprueft), nur im
 *   Abschnitt "!rc" (Quadrant 2,6/3,5) steht ein zusaetzlicher else-Zweig, der g_near_objs[0*9+7] und [1*9+6]
 *   loescht (Kopie aus dem Abschnitt "!l10", dort zusaetzlich mit Zelle 2,5 in der Bedingung) - in den
 *   anderen drei Quadranten ("!l11", "!rd", "!rb") fehlt das Gegenstueck.
 */
#include "schweif.h"

void far view_cull_hidden(void)
{
	signed char l01, l02, l03, l04, l05, l06, l07, l08, l09, l0a, l0b, l0c, l0d, l0e, l0f, l10;
	signed char l11, l12, l13, l14, l15, l16, l17, l18, l19, l1a, l1b, l1c, l1d, l1e, l1f, l20, l21;
	register signed char rd, rb, rc;

	l01 = l02 = l03 = l04 = l05 = l06 = l07 = l08 = l09 = l0a = l0b = l0c = l0d = l0e = rd = rb = l0f = l10 = rc =
		l11 = l12 = l13 = l14 = l15 = l16 = l17 = l18 = l19 = l1a = l1b = l1c = l1d = l1e = l1f = l20 = l21 = 0;

	if (g_near_occl[3*9+3]) { g_near_objs[2*9+2] = g_near_objs[1*9+1] = g_near_objs[0*9+0] = 0; l01 = 1; }
	else if (g_near_occl[2*9+2]) { g_near_objs[1*9+1] = g_near_objs[0*9+0] = 0; }
	else if (g_near_occl[1*9+1]) g_near_objs[0*9+0] = 0;

	if (g_near_occl[4*9+3]) { g_near_objs[4*9+2] = g_near_objs[4*9+1] = g_near_objs[4*9+0] = 0; l09 = l02 = 1; }
	else if (g_near_occl[4*9+2]) { g_near_objs[4*9+1] = g_near_objs[4*9+0] = 0; l02 = 1; }
	else if (g_near_occl[4*9+1]) g_near_objs[4*9+0] = 0;

	if (g_near_occl[5*9+3]) { g_near_objs[6*9+2] = g_near_objs[7*9+1] = g_near_objs[8*9+0] = 0; l03 = 1; }
	else if (g_near_occl[6*9+2]) { g_near_objs[7*9+1] = g_near_objs[8*9+0] = 0; }
	else if (g_near_occl[7*9+1]) g_near_objs[8*9+0] = 0;

	if (g_near_occl[5*9+4]) { g_near_objs[6*9+4] = g_near_objs[7*9+4] = g_near_objs[8*9+4] = 0; l0a = l04 = 1; }
	else if (g_near_occl[6*9+4]) { g_near_objs[7*9+4] = g_near_objs[8*9+4] = 0; l04 = 1; }
	else if (g_near_occl[7*9+4]) g_near_objs[8*9+4] = 0;

	if (g_near_occl[5*9+5]) { g_near_objs[6*9+6] = g_near_objs[7*9+7] = g_near_objs[8*9+8] = 0; l05 = 1; }
	else if (g_near_occl[6*9+6]) { g_near_objs[7*9+7] = g_near_objs[8*9+8] = 0; }
	else if (g_near_occl[7*9+7]) g_near_objs[8*9+8] = 0;

	if (g_near_occl[4*9+5]) { g_near_objs[4*9+6] = g_near_objs[4*9+7] = g_near_objs[4*9+8] = 0; l0b = l06 = 1; }
	else if (g_near_occl[4*9+6]) { g_near_objs[4*9+7] = g_near_objs[4*9+8] = 0; l06 = 1; }
	else if (g_near_occl[4*9+7]) g_near_objs[4*9+8] = 0;

	if (g_near_occl[3*9+5]) { g_near_objs[2*9+6] = g_near_objs[1*9+7] = g_near_objs[0*9+8] = 0; l07 = 1; }
	else if (g_near_occl[2*9+6]) { g_near_objs[1*9+7] = g_near_objs[0*9+8] = 0; }
	else if (g_near_occl[1*9+7]) g_near_objs[0*9+8] = 0;

	if (g_near_occl[3*9+4]) { g_near_objs[2*9+4] = g_near_objs[1*9+4] = g_near_objs[0*9+4] = 0; l0c = l08 = 1; }
	else if (g_near_occl[2*9+4]) { g_near_objs[1*9+4] = g_near_objs[0*9+4] = 0; l08 = 1; }
	else if (g_near_occl[1*9+4]) g_near_objs[0*9+4] = 0;

	if (l09 && l0a) {
		g_near_objs[5*9+2] = g_near_objs[5*9+1] = g_near_objs[6*9+1] = g_near_objs[5*9+0] = g_near_objs[6*9+0] = g_near_objs[7*9+0] = 0; l0e = 1;
		g_near_objs[6*9+3] = g_near_objs[7*9+3] = g_near_objs[7*9+2] = g_near_objs[8*9+3] = g_near_objs[8*9+2] = g_near_objs[8*9+1] = 0; rd = 1;
		g_near_objs[5*9+3] = g_near_objs[6*9+2] = g_near_objs[7*9+1] = g_near_objs[8*9+0] = 0; l03 = 1;
	} else {
		if (l02 && l03) { g_near_objs[5*9+2] = g_near_objs[5*9+1] = g_near_objs[6*9+1] = g_near_objs[5*9+0] = g_near_objs[6*9+0] = g_near_objs[7*9+0] = 0; l0e = 1; }
		if (l03 && l04) { g_near_objs[6*9+3] = g_near_objs[7*9+3] = g_near_objs[7*9+2] = g_near_objs[8*9+3] = g_near_objs[8*9+2] = g_near_objs[8*9+1] = 0; rd = 1; }
	}
	if (l0a && l0b) {
		g_near_objs[6*9+5] = g_near_objs[7*9+5] = g_near_objs[7*9+6] = g_near_objs[8*9+5] = g_near_objs[8*9+6] = g_near_objs[8*9+7] = 0; rb = 1;
		g_near_objs[5*9+6] = g_near_objs[5*9+7] = g_near_objs[6*9+7] = g_near_objs[5*9+8] = g_near_objs[6*9+8] = g_near_objs[7*9+8] = 0; l0f = 1;
		g_near_objs[5*9+5] = g_near_objs[6*9+6] = g_near_objs[7*9+7] = g_near_objs[8*9+8] = 0; l05 = 1;
	} else {
		if (l04 && l05) { g_near_objs[6*9+5] = g_near_objs[7*9+5] = g_near_objs[7*9+6] = g_near_objs[8*9+5] = g_near_objs[8*9+6] = g_near_objs[8*9+7] = 0; rb = 1; }
		if (l05 && l06) { g_near_objs[5*9+6] = g_near_objs[5*9+7] = g_near_objs[6*9+7] = g_near_objs[5*9+8] = g_near_objs[6*9+8] = g_near_objs[7*9+8] = 0; l0f = 1; }
	}
	if (l0b && l0c) {
		g_near_objs[3*9+6] = g_near_objs[3*9+7] = g_near_objs[2*9+7] = g_near_objs[3*9+8] = g_near_objs[2*9+8] = g_near_objs[1*9+8] = 0; l10 = 1;
		g_near_objs[2*9+5] = g_near_objs[1*9+5] = g_near_objs[1*9+6] = g_near_objs[0*9+5] = g_near_objs[0*9+6] = g_near_objs[0*9+7] = 0; rc = 1;
		g_near_objs[3*9+5] = g_near_objs[2*9+6] = g_near_objs[1*9+7] = g_near_objs[0*9+8] = 0; l07 = 1;
	} else {
		if (l06 && l07) { g_near_objs[3*9+6] = g_near_objs[3*9+7] = g_near_objs[2*9+7] = g_near_objs[3*9+8] = g_near_objs[2*9+8] = g_near_objs[1*9+8] = 0; l10 = 1; }
		if (l07 && l08) { g_near_objs[2*9+5] = g_near_objs[1*9+5] = g_near_objs[1*9+6] = g_near_objs[0*9+5] = g_near_objs[0*9+6] = g_near_objs[0*9+7] = 0; rc = 1; }
	}
	if (l0c && l09) {
		g_near_objs[2*9+3] = g_near_objs[1*9+3] = g_near_objs[1*9+2] = g_near_objs[0*9+3] = g_near_objs[0*9+2] = g_near_objs[0*9+1] = 0; l11 = 1;
		g_near_objs[3*9+2] = g_near_objs[3*9+1] = g_near_objs[2*9+1] = g_near_objs[3*9+0] = g_near_objs[2*9+0] = g_near_objs[1*9+0] = 0; l0d = 1;
		g_near_objs[3*9+3] = g_near_objs[2*9+2] = g_near_objs[1*9+1] = g_near_objs[0*9+0] = 0; l01 = 1;
	} else {
		if (l01 && l02) { g_near_objs[3*9+2] = g_near_objs[3*9+1] = g_near_objs[2*9+1] = g_near_objs[3*9+0] = g_near_objs[2*9+0] = g_near_objs[1*9+0] = 0; l0d = 1; }
		if (l08 && l01) { g_near_objs[2*9+3] = g_near_objs[1*9+3] = g_near_objs[1*9+2] = g_near_objs[0*9+3] = g_near_objs[0*9+2] = g_near_objs[0*9+1] = 0; l11 = 1; }
	}

	if (!l0d) {
		if ((g_near_occl[2*9+2] || g_near_occl[3*9+3]) && (g_near_occl[3*9+1] || g_near_occl[3*9+2])) { g_near_objs[2*9+0] = g_near_objs[2*9+1] = g_near_objs[1*9+0] = 0; l12 = 1; }
		if (g_near_occl[3*9+2] && g_near_occl[2*9+3]) {
			g_near_objs[0*9+0] = g_near_objs[1*9+1] = g_near_objs[2*9+2] = g_near_objs[2*9+0] = g_near_objs[2*9+1] = g_near_objs[1*9+0] = 0; l12 = 1;
			if (!l11) { g_near_objs[0*9+2] = g_near_objs[1*9+2] = g_near_objs[0*9+1] = 0; l21 = 1; }
		} else {
			if ((g_near_occl[3*9+2] || g_near_occl[3*9+1]) && g_near_occl[3*9+3]) g_near_objs[2*9+1] = 0;
			if (!l11 && (g_near_occl[1*9+3] || g_near_occl[2*9+3]) && g_near_occl[3*9+3]) g_near_objs[1*9+2] = 0;
			if (g_near_occl[2*9+1] && g_near_occl[1*9+2]) {
				g_near_objs[0*9+0] = g_near_objs[1*9+1] = g_near_objs[1*9+0] = 0;
				if (!l11) g_near_objs[0*9+1] = 0;
			} else {
				if (g_near_occl[1*9+0] && g_near_occl[0*9+1]) g_near_objs[0*9+0] = 0;
				if ((g_near_occl[1*9+1] || g_near_occl[2*9+2] || g_near_occl[3*9+3]) && (g_near_occl[2*9+0] || g_near_occl[2*9+1] || g_near_occl[3*9+2]) || g_near_occl[2*9+2] && g_near_occl[4*9+3]) g_near_objs[1*9+0] = 0;
				if (!l11 && ((g_near_occl[1*9+1] || g_near_occl[2*9+2] || g_near_occl[3*9+3]) && (g_near_occl[0*9+2] || g_near_occl[1*9+2] || g_near_occl[2*9+3]) || g_near_occl[2*9+2] && g_near_occl[3*9+4])) g_near_objs[0*9+1] = 0;
			}
		}
		if (g_near_occl[3*9+2] && (g_near_occl[4*9+3] || g_near_occl[4*9+2] || g_near_occl[4*9+1])) { g_near_objs[3*9+1] = g_near_objs[3*9+0] = g_near_objs[2*9+0] = 0; l13 = 1; }
		else if (g_near_occl[3*9+1] && (g_near_occl[4*9+3] || g_near_occl[4*9+2] || g_near_occl[4*9+1] || g_near_occl[4*9+0])) g_near_objs[3*9+0] = 0;
	}

	if (!l0e) {
		if ((g_near_occl[5*9+3] || g_near_occl[6*9+2]) && (g_near_occl[5*9+1] || g_near_occl[5*9+2])) { g_near_objs[6*9+0] = g_near_objs[6*9+1] = g_near_objs[7*9+0] = 0; l15 = 1; }
		if (g_near_occl[5*9+2] && g_near_occl[6*9+3]) {
			g_near_objs[8*9+0] = g_near_objs[7*9+1] = g_near_objs[6*9+2] = g_near_objs[6*9+0] = g_near_objs[6*9+1] = g_near_objs[7*9+0] = 0; l15 = 1;
			if (!rd) { g_near_objs[8*9+2] = g_near_objs[7*9+2] = g_near_objs[8*9+1] = 0; l16 = 1; }
		} else {
			if ((g_near_occl[5*9+2] || g_near_occl[5*9+1]) && g_near_occl[5*9+3]) g_near_objs[6*9+1] = 0;
			if (!rd && (g_near_occl[7*9+3] || g_near_occl[6*9+3]) && g_near_occl[5*9+3]) g_near_objs[7*9+2] = 0;
			if (g_near_occl[6*9+1] && g_near_occl[7*9+2]) {
				g_near_objs[8*9+0] = g_near_objs[7*9+1] = g_near_objs[7*9+0] = 0;
				if (!rd) g_near_objs[8*9+1] = 0;
			} else {
				if (g_near_occl[7*9+0] && g_near_occl[8*9+1]) g_near_objs[8*9+0] = 0;
				if ((g_near_occl[7*9+1] || g_near_occl[6*9+2] || g_near_occl[5*9+3]) && (g_near_occl[6*9+0] || g_near_occl[6*9+1] || g_near_occl[5*9+2]) || g_near_occl[6*9+2] && g_near_occl[4*9+3]) g_near_objs[7*9+0] = 0;
				if (!rd && ((g_near_occl[7*9+1] || g_near_occl[6*9+2] || g_near_occl[5*9+3]) && (g_near_occl[8*9+2] || g_near_occl[7*9+2] || g_near_occl[6*9+3]) || g_near_occl[6*9+2] && g_near_occl[5*9+4])) g_near_objs[8*9+1] = 0;
			}
		}
		if (g_near_occl[5*9+2] && (g_near_occl[4*9+3] || g_near_occl[4*9+2] || g_near_occl[4*9+1])) { g_near_objs[5*9+0] = g_near_objs[5*9+1] = g_near_objs[6*9+0] = 0; l14 = 1; }
		else if (g_near_occl[5*9+1] && (g_near_occl[4*9+3] || g_near_occl[4*9+2] || g_near_occl[4*9+1] || g_near_occl[4*9+0])) g_near_objs[5*9+0] = 0;
	}

	if (!l12 && !l13) {
		if (g_near_occl[3*9+2]) g_near_objs[2*9+0] = 0;
		else if (g_near_occl[3*9+3] && g_near_occl[3*9+1]) g_near_objs[2*9+0] = 0;
		else if (g_near_occl[2*9+1] && (g_near_occl[3*9+1] || g_near_occl[3*9+0])) g_near_objs[2*9+0] = 0;
	}
	if (!l14 && !l15) {
		if (g_near_occl[5*9+2]) g_near_objs[6*9+0] = 0;
		else if (g_near_occl[5*9+3] && g_near_occl[5*9+1]) g_near_objs[6*9+0] = 0;
		else if (g_near_occl[6*9+1] && (g_near_occl[5*9+1] || g_near_occl[5*9+0])) g_near_objs[6*9+0] = 0;
	}

	if (!l0f) {
		if ((g_near_occl[6*9+6] || g_near_occl[5*9+5]) && (g_near_occl[5*9+7] || g_near_occl[5*9+6])) { g_near_objs[6*9+8] = g_near_objs[6*9+7] = g_near_objs[7*9+8] = 0; l1a = 1; }
		if (g_near_occl[5*9+6] && g_near_occl[6*9+5]) {
			g_near_objs[8*9+8] = g_near_objs[7*9+7] = g_near_objs[6*9+6] = g_near_objs[6*9+8] = g_near_objs[6*9+7] = g_near_objs[7*9+8] = 0; l1a = 1;
			if (!rb) { g_near_objs[8*9+6] = g_near_objs[7*9+6] = g_near_objs[8*9+7] = 0; l19 = 1; }
		} else {
			if ((g_near_occl[5*9+6] || g_near_occl[5*9+7]) && g_near_occl[5*9+5]) g_near_objs[6*9+7] = 0;
			if (!rb && (g_near_occl[7*9+5] || g_near_occl[6*9+5]) && g_near_occl[5*9+5]) g_near_objs[7*9+6] = 0;
			if (g_near_occl[6*9+7] && g_near_occl[7*9+6]) {
				g_near_objs[8*9+8] = g_near_objs[7*9+7] = g_near_objs[7*9+8] = 0;
				if (!rb) g_near_objs[8*9+7] = 0;
			} else {
				if (g_near_occl[7*9+8] && g_near_occl[8*9+7]) g_near_objs[8*9+8] = 0;
				if ((g_near_occl[7*9+7] || g_near_occl[6*9+6] || g_near_occl[5*9+5]) && (g_near_occl[6*9+8] || g_near_occl[6*9+7] || g_near_occl[5*9+6]) || g_near_occl[6*9+6] && g_near_occl[4*9+5]) g_near_objs[7*9+8] = 0;
				if (!rb && ((g_near_occl[7*9+7] || g_near_occl[6*9+6] || g_near_occl[5*9+5]) && (g_near_occl[8*9+6] || g_near_occl[7*9+6] || g_near_occl[6*9+5]) || g_near_occl[6*9+6] && g_near_occl[5*9+4])) g_near_objs[8*9+7] = 0;
			}
		}
		if (g_near_occl[5*9+6] && (g_near_occl[4*9+5] || g_near_occl[4*9+6] || g_near_occl[4*9+7])) { g_near_objs[5*9+7] = g_near_objs[5*9+8] = g_near_objs[6*9+8] = 0; l1b = 1; }
		else if (g_near_occl[5*9+7] && (g_near_occl[4*9+5] || g_near_occl[4*9+6] || g_near_occl[4*9+7] || g_near_occl[4*9+8])) g_near_objs[5*9+8] = 0;
	}

	if (!l10) {
		if ((g_near_occl[2*9+6] || g_near_occl[3*9+5]) && (g_near_occl[3*9+7] || g_near_occl[3*9+6])) { g_near_objs[2*9+8] = g_near_objs[2*9+7] = g_near_objs[1*9+8] = 0; l1d = 1; }
		if (g_near_occl[3*9+6] && g_near_occl[2*9+5]) {
			g_near_objs[0*9+8] = g_near_objs[1*9+7] = g_near_objs[2*9+6] = g_near_objs[2*9+8] = g_near_objs[2*9+7] = g_near_objs[1*9+8] = 0; l1d = 1;
			if (!rc) { g_near_objs[0*9+6] = g_near_objs[1*9+6] = g_near_objs[0*9+7] = 0; l1e = 1; }
		} else {
			if ((g_near_occl[3*9+6] || g_near_occl[3*9+7]) && g_near_occl[3*9+5]) g_near_objs[2*9+7] = 0;
			if (!rc && (g_near_occl[1*9+5] || g_near_occl[2*9+5]) && g_near_occl[3*9+5]) g_near_objs[1*9+6] = 0;
			if (g_near_occl[2*9+7] && g_near_occl[1*9+6]) {
				g_near_objs[0*9+8] = g_near_objs[1*9+7] = g_near_objs[1*9+8] = 0;
				if (!rc) g_near_objs[0*9+7] = 0;
			} else {
				if (g_near_occl[1*9+8] && g_near_occl[0*9+7]) g_near_objs[0*9+8] = 0;
				if ((g_near_occl[1*9+7] || g_near_occl[2*9+6] || g_near_occl[3*9+5]) && (g_near_occl[2*9+8] || g_near_occl[2*9+7] || g_near_occl[3*9+6]) || g_near_occl[2*9+6] && g_near_occl[4*9+5]) g_near_objs[1*9+8] = 0;
				if (!rc && ((g_near_occl[1*9+7] || g_near_occl[2*9+6] || g_near_occl[3*9+5]) && (g_near_occl[0*9+6] || g_near_occl[1*9+6] || g_near_occl[2*9+5]) || g_near_occl[2*9+6] && g_near_occl[3*9+4])) g_near_objs[0*9+7] = 0;
			}
		}
		if (g_near_occl[3*9+6] && (g_near_occl[4*9+5] || g_near_occl[4*9+6] || g_near_occl[4*9+7])) { g_near_objs[3*9+7] = g_near_objs[3*9+8] = g_near_objs[2*9+8] = 0; l1c = 1; }
		else if (g_near_occl[3*9+7] && (g_near_occl[4*9+5] || g_near_occl[4*9+6] || g_near_occl[4*9+7] || g_near_occl[4*9+8])) g_near_objs[3*9+8] = 0;
	}

	if (!l1a && !l1b) {
		if (g_near_occl[5*9+6]) g_near_objs[6*9+8] = 0;
		else if (g_near_occl[5*9+5] && g_near_occl[5*9+7]) g_near_objs[6*9+8] = 0;
		else if (g_near_occl[6*9+7] && (g_near_occl[5*9+7] || g_near_occl[5*9+8])) g_near_objs[6*9+8] = 0;
	}
	if (!l1c && !l1d) {
		if (g_near_occl[3*9+6]) g_near_objs[2*9+8] = 0;
		else if (g_near_occl[3*9+5] && g_near_occl[3*9+7]) g_near_objs[2*9+8] = 0;
		else if (g_near_occl[2*9+7] && (g_near_occl[3*9+7] || g_near_occl[3*9+8])) g_near_objs[2*9+8] = 0;
	}

	if (!l11) {
#if FIX_0087
		/* Fix #87: else-Zweig des Quadranten "!rc" gespiegelt ergaenzt (Quadrant "!l11") */
		if ((g_near_occl[3*9+3] || g_near_occl[2*9+2]) && (g_near_occl[1*9+3] || g_near_occl[2*9+3])) { g_near_objs[0*9+2] = g_near_objs[1*9+2] = g_near_objs[0*9+1] = 0; l21 = 1; }
		else {
			if ((g_near_occl[1*9+1] || g_near_occl[2*9+2] || g_near_occl[3*9+3]) && (g_near_occl[0*9+2] || g_near_occl[1*9+2])) g_near_objs[0*9+1] = 0;
			if ((g_near_occl[1*9+3] || g_near_occl[2*9+3]) && g_near_occl[3*9+3]) g_near_objs[1*9+2] = 0;
		}
#else
		if ((g_near_occl[3*9+3] || g_near_occl[2*9+2]) && (g_near_occl[1*9+3] || g_near_occl[2*9+3])) { g_near_objs[0*9+2] = g_near_objs[1*9+2] = g_near_objs[0*9+1] = 0; l21 = 1; }
#endif
		if (g_near_occl[2*9+3] && (g_near_occl[3*9+4] || g_near_occl[2*9+4] || g_near_occl[1*9+4])) { g_near_objs[1*9+3] = g_near_objs[0*9+3] = g_near_objs[0*9+2] = 0; l20 = 1; }
		else if (g_near_occl[1*9+3] && (g_near_occl[3*9+4] || g_near_occl[2*9+4] || g_near_occl[1*9+4] || g_near_occl[0*9+4])) g_near_objs[0*9+3] = 0;
	}
	if (!rc) {
		if (g_near_occl[2*9+5] && (g_near_occl[3*9+4] || g_near_occl[2*9+4] || g_near_occl[1*9+4])) { g_near_objs[0*9+5] = g_near_objs[1*9+5] = g_near_objs[0*9+6] = 0; l1f = 1; }
		else if (g_near_occl[1*9+5] && (g_near_occl[3*9+4] || g_near_occl[2*9+4] || g_near_occl[1*9+4] || g_near_occl[0*9+4])) g_near_objs[0*9+5] = 0;
		if ((g_near_occl[3*9+5] || g_near_occl[2*9+6]) && (g_near_occl[1*9+5] || g_near_occl[2*9+5])) { g_near_objs[0*9+6] = g_near_objs[1*9+6] = g_near_objs[0*9+7] = 0; l1e = 1; }
		else {
			if ((g_near_occl[1*9+7] || g_near_occl[2*9+6] || g_near_occl[3*9+5]) && (g_near_occl[0*9+6] || g_near_occl[1*9+6])) g_near_objs[0*9+7] = 0;
			if ((g_near_occl[1*9+5] || g_near_occl[2*9+5]) && g_near_occl[3*9+5]) g_near_objs[1*9+6] = 0;
		}
	}

	if (!l20 && !l21) {
		if (g_near_occl[2*9+3]) g_near_objs[0*9+2] = 0;
		else if (g_near_occl[3*9+3] && g_near_occl[1*9+3]) g_near_objs[0*9+2] = 0;
		else if (g_near_occl[1*9+2] && (g_near_occl[1*9+3] || g_near_occl[0*9+3])) g_near_objs[0*9+2] = 0;
	}
	if (!l1e && !l1f) {
		if (g_near_occl[2*9+5]) g_near_objs[0*9+6] = 0;
		else if (g_near_occl[3*9+5] && g_near_occl[1*9+5]) g_near_objs[0*9+6] = 0;
		else if (g_near_occl[1*9+6] && (g_near_occl[1*9+5] || g_near_occl[0*9+5])) g_near_objs[0*9+6] = 0;
	}

	if (!rd) {
#if FIX_0087
		/* Fix #87: else-Zweig des Quadranten "!rc" gespiegelt ergaenzt (Quadrant "!rd") */
		if ((g_near_occl[5*9+3] || g_near_occl[6*9+2]) && (g_near_occl[7*9+3] || g_near_occl[6*9+3])) { g_near_objs[8*9+2] = g_near_objs[8*9+1] = g_near_objs[7*9+2] = 0; l16 = 1; }
		else {
			if ((g_near_occl[7*9+1] || g_near_occl[6*9+2] || g_near_occl[5*9+3]) && (g_near_occl[8*9+2] || g_near_occl[7*9+2])) g_near_objs[8*9+1] = 0;
			if ((g_near_occl[7*9+3] || g_near_occl[6*9+3]) && g_near_occl[5*9+3]) g_near_objs[7*9+2] = 0;
		}
#else
		if ((g_near_occl[5*9+3] || g_near_occl[6*9+2]) && (g_near_occl[7*9+3] || g_near_occl[6*9+3])) { g_near_objs[8*9+2] = g_near_objs[8*9+1] = g_near_objs[7*9+2] = 0; l16 = 1; }
#endif
		if (g_near_occl[6*9+3] && (g_near_occl[5*9+4] || g_near_occl[6*9+4] || g_near_occl[7*9+4])) { g_near_objs[7*9+3] = g_near_objs[8*9+3] = g_near_objs[8*9+2] = 0; l17 = 1; }
		else if (g_near_occl[7*9+3] && (g_near_occl[5*9+4] || g_near_occl[6*9+4] || g_near_occl[7*9+4] || g_near_occl[8*9+4])) g_near_objs[8*9+3] = 0;
	}
	if (!rb) {
		if (g_near_occl[6*9+5] && (g_near_occl[5*9+4] || g_near_occl[6*9+4] || g_near_occl[7*9+4])) { g_near_objs[8*9+5] = g_near_objs[7*9+5] = g_near_objs[8*9+6] = 0; l18 = 1; }
		else if (g_near_occl[7*9+5] && (g_near_occl[5*9+4] || g_near_occl[6*9+4] || g_near_occl[7*9+4] || g_near_occl[8*9+4])) g_near_objs[8*9+5] = 0;
#if FIX_0087
		/* Fix #87: else-Zweig des Quadranten "!rc" gespiegelt ergaenzt (Quadrant "!rb") */
		if ((g_near_occl[5*9+5] || g_near_occl[6*9+6]) && (g_near_occl[7*9+5] || g_near_occl[6*9+5])) { g_near_objs[8*9+6] = g_near_objs[7*9+6] = g_near_objs[8*9+7] = 0; l19 = 1; }
		else {
			if ((g_near_occl[7*9+7] || g_near_occl[6*9+6] || g_near_occl[5*9+5]) && (g_near_occl[8*9+6] || g_near_occl[7*9+6])) g_near_objs[8*9+7] = 0;
			if ((g_near_occl[7*9+5] || g_near_occl[6*9+5]) && g_near_occl[5*9+5]) g_near_objs[7*9+6] = 0;
		}
#else
		if ((g_near_occl[5*9+5] || g_near_occl[6*9+6]) && (g_near_occl[7*9+5] || g_near_occl[6*9+5])) { g_near_objs[8*9+6] = g_near_objs[7*9+6] = g_near_objs[8*9+7] = 0; l19 = 1; }
#endif
	}

	if (!l16 && !l17) {
		if (g_near_occl[6*9+3]) g_near_objs[8*9+2] = 0;
		else if (g_near_occl[5*9+3] && g_near_occl[7*9+3]) g_near_objs[8*9+2] = 0;
		else if (g_near_occl[7*9+2] && (g_near_occl[7*9+3] || g_near_occl[8*9+3])) g_near_objs[8*9+2] = 0;
	}
	if (!l18 && !l19) {
		if (g_near_occl[6*9+5]) g_near_objs[8*9+6] = 0;
		else if (g_near_occl[5*9+5] && g_near_occl[7*9+5]) g_near_objs[8*9+6] = 0;
		else if (g_near_occl[7*9+6] && (g_near_occl[7*9+5] || g_near_occl[8*9+5])) g_near_objs[8*9+6] = 0;
	}
}
