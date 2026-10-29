/* OVL206 - Overlay O206 (Segment 0x238a in SCHWEIF.EXE): Charakterbogen drucken (Teil 1; Teil 2 und Hilfen in O207).
 * f_0000(h): Texte 0x20d nach h+5e7/5eb (O248_02E3); Ausgabe in Datei (ds:253c: Name O207_13AD -> *ds:b556,
 *   O037_01CB(ds:35c6, 0, 0x8302), Fehler txt[110]) oder auf den Drucker (biosprint-Status 0/0x90, sonst txt[2]
 *   bei 0x20 bzw. txt[1]; Hinweis txt[0]/txt[109]). Magier (+33 >= 7, inline mage()) waehlen Teil 1/2/beide
 *   (txt[112..115]). Teil 1: Kopf (txt[3..6], Typname *ds:b67c, Geburtstag *ds:b67c[+38 + 4]), Attribute
 *   (+46..+6f als {Grund, aktuell, Mod}, O077_0471/04AB), Kampfwerte (avg = Summe dreier Attribute / 4,
 *   O006_0000, O077_0000), Talente/Zauber O006_0000(h, k), 15 Inventarzeilen O207_10A9, Zeile fuer Platz 30
 *   (f_1649, memset/strncpy 14 Zeichen), Schluss ds:707a/txt[111]/ds:707c bzw. Seitenvorschub; Teil 2: O207_0000.
 * f_1580(h, s): Zeile ausgeben (Datei: s und ds:707e per O037_04D6, Rueckgabe 1 wenn geschrieben; Drucker: Zeichen
 *   per biosprint + R01_036B, CR/LF, Status). f_1649(h, n, id, cnt, gew, a, b, c): Inventarplatz n (Tabelle
 *   *ds:b84a 14 Bytes: +2 Bits 0/1/4, +5 Index, +6 Gewicht; ds:31eb/ds:2f7b), Name O056_0000(0x4000/0x4004, id, 0).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL206.CPP   -> 6108/6108 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (ds:707e -> _s_707c+0x2). Ohne -Og 367 Abweichungen (28 Bytes zu lang).
 * - Zusammengesetzt aus OVL206.TPL + Rumpf per gen/mk206.py: gen/g206.py arbeitet die ASM als Stapelmaschine ab
 *   (Pushes -> Ausdruecke, Aufruf + add sp -> Argumentliste, sprintf/O206_1580-Folgen), sechs Sonderzeilen von
 *   Hand; die Heldenstruktur wird aus den benutzten Feldern erzeugt.
 * - Auswahlergebnis r im inneren Block (memset/strncpy-Intrinsics auf Funktionsebene sperren SI).
 * - Locals: 13 Skalare in Stackreihenfolge deklariert, Puffer 10/6 Bytes in Deklarationsreihenfolge, 20 ans Ende.
 * - f_1649: "*pb = *pc = -1" (pc zuerst gespeichert); "if (..) {..; return ..;} else return 0;" (zwei tote
 *   Epiloge); Flagbyte +2 als -Og-Byte [bp-3].
 * Auffaellig: f_1649 prueft "*id != 0" ein zweites Mal, obwohl es schon feststeht (harmlos).
 */
#include <stdio.h>
#include <string.h>
#include <bios.h>

struct slot {				/* 0x1b Bytes */
	signed short id;		/* +0 */
	signed short cnt;		/* +2 */
	char pad4[0x1b - 4];
};

@HERO@
struct itemflags {
	unsigned short b0:1, b1:1, b2:1, b3:1, b4:1, b5:1, b6:1, b7:1;
};

struct item {				/* 14 Bytes, *ds:b84a */
	char pad0[2];
	struct itemflags b;		/* +2 */
	char pad3[2];
	signed char f5;			/* +5 */
	signed short weight;		/* +6 */
	char pad8[6];
};

struct c2 {
	signed char a;
	signed char b;
};

struct c8 {
	signed char a;
	signed char b;
	char pad[6];
};

extern char far * far *g_b67c;
extern char far *g_b674;
extern char far * far *g_b556;
extern struct item far *g_b84a;
extern struct c2 g_31eb[];
extern struct c8 g_2f7b[];
extern char g_b59a[];
extern char g_35c6[];
extern char g_37e4[];
extern signed char g_253c;
extern char g_706a[], g_706c[], g_706e[], g_7070[], g_7072[], g_7074[], g_7076[], g_7078[];
extern char g_707a[], g_707c[], g_707e[];

signed char O006_0000(struct hero far *h, signed short k);
signed short O037_01cb(void far *f, signed short a, signed short mode);
signed long O037_04d6(void far *f, void far *buf, unsigned long n);
void O037_050d(void far *f);
char far *O056_0000(signed short a, signed short id, signed short b);
signed short O077_0000(struct hero far *h);
signed short O077_0471(struct hero far *h, signed short k);
signed short O077_04ab(struct hero far *h, signed short k);
void O190_00e8(void far *p, char far *s);
signed short O190_01c6(void far *p, char far *t, signed short n, ...);
void O207_0000(struct hero far *h);
void O207_10a9(struct hero far *h, signed short a, signed short b);
void O207_13ad(struct hero far *h, char far *dst);
void O248_02e3(void far *a, signed short b, signed short c, void far *d, void far *e);
void R01_036b(void);
void far *R07_0140(void far *p);

signed short O206_1580(struct hero far *h, char far *s);
char far *f_1649(struct hero far *h, signed short n, signed short far *a, signed short far *b,
	signed short far *c, signed short far *d, signed short far *e, signed short far *f);

void f_0000(struct hero far *h)
{
	signed short st;
	signed short avg;
	signed short v6;
	signed short v8;
	signed short va;
	signed short vc;
	signed short ve;
	signed short v10;
	char far *name;
	char far *fn;
	signed short part2 = 0;
	signed short part1 = 0;
	signed short abort = 0;
	char b10[10];
	char b6[6];
	char b20[20];

	O248_02e3(g_37e4, 0x20d, 0x82, &h->f5e7, &h->txt);
	if (g_253c != 0) {
		fn = new char[0x14];
		O207_13ad(h, fn);
		*g_b556 = fn;
		if (O037_01cb(g_35c6, 0, 0x8302) == -1) {
			abort = 1;
			O190_00e8(g_b59a, h->txt[110]);
			delete fn;
		}
	} else {
		st = biosprint(2, 0, 0);
		if (st != 0 && st != 0x90) {
			O190_00e8(g_b59a, st == 0x20 ? h->txt[2] : h->txt[1]);
			abort = 1;
		} else {
			O190_01c6(g_b59a, h->txt[0], 1, h->txt[109]);
		}
	}
	if (h->mage()) {
		signed short r;

		r = O190_01c6(g_b59a, h->txt[112], 3, h->txt[113], h->txt[114], h->txt[115]);
		if (r == -1)
			abort = 1;
		else if (r == 1)
			part1 = 1;
		else if (r == 2)
			part2 = 1;
		else if (r == 3)
			part2 = part1 = 1;
	} else {
		part1 = 1;
	}
	if (abort == 0) {
		if (part1 != 0) {
@BODY@
			O207_10a9(h, 0, 1);
			O207_10a9(h, 2, 3);
			O207_10a9(h, 4, 5);
			O207_10a9(h, 6, 7);
			O207_10a9(h, 8, 9);
			O207_10a9(h, 10, 11);
			O207_10a9(h, 12, 13);
			O207_10a9(h, 14, 15);
			O207_10a9(h, 16, 17);
			O207_10a9(h, 18, 19);
			O207_10a9(h, 20, 21);
			O207_10a9(h, 22, 23);
			O207_10a9(h, 24, 25);
			O207_10a9(h, 26, 27);
			O207_10a9(h, 28, 29);
			name = f_1649(h, 0x1e, &v8, &ve, &v10, &v6, &va, &vc);
			sprintf(b10, va != -1 ? h->txt[50] : h->txt[51], va, vc);
			sprintf(b6, v6 != -1 ? h->txt[52] : h->txt[53], v6);
			memset(b20, 0, 20);
			strncpy(b20, name, 14);
			sprintf(g_b674, v8 == 0 ? h->txt[49] : h->txt[45], ve, b20, v10, b6, b10);
			O206_1580(h, g_b674);
			if (g_253c != 0) {
				O206_1580(h, g_707a);
				O206_1580(h, h->txt[111]);
				O206_1580(h, g_707c);
			} else {
				biosprint(0, 0x0c, 0);
			}
		}
		if (part2 != 0)
			O207_0000(h);
		if (g_253c != 0) {
			O037_050d(g_35c6);
			delete fn;
		}
	}
	R07_0140(h->f5e7);
	R07_0140(h->txt);
}

signed short O206_1580(struct hero far *h, char far *s)
{
	signed short r;
	char far *nl;

	r = 0;
	if (g_253c != 0) {
		nl = g_707e;
		if (O037_04d6(g_35c6, s, strlen(s))) {
			r = 1;
			O037_04d6(g_35c6, nl, strlen(nl));
		}
	} else {
		while (*s != 0) {
			biosprint(0, *s++, 0);
			R01_036b();
		}
		biosprint(0, 0x0d, 0);
		biosprint(0, 0x0a, 0);
		r = biosprint(2, 0, 0);
	}
	return r;
}

char far *f_1649(struct hero far *h, signed short n, signed short far *id, signed short far *cnt,
	signed short far *w, signed short far *pa, signed short far *pb, signed short far *pc)
{
	signed short t;

	if ((*id = h->inv[n].id) != 0) {
		if (((*cnt = h->inv[n].cnt) == 0 && *id != 0) || !g_b84a[*id].b.b4)
			*cnt = 1;
		*w = g_b84a[*id].weight * *cnt;
		if (g_b84a[*id].b.b0)
			*pa = g_31eb[g_b84a[*id].f5].a;
		else
			*pa = -1;
		if (g_b84a[*id].b.b1 && !g_b84a[*id].b.b4) {
			*pb = g_2f7b[g_b84a[*id].f5].a;
			*pc = g_2f7b[g_b84a[*id].f5].b;
		} else {
			*pb = *pc = -1;
		}
		t = *cnt == 1 ? 0x4000 : 0x4004;
		return O056_0000(t, *id, 0);
	} else
		return 0;
}
