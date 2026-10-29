/* OVL030 - Overlay O030 (Segment 0x1f9c in SCHWEIF.EXE): Ortsverteiler (Gruppe *ds:c4c5, Ortsart +1e "dir",
 * vorige +1f). f_0000: Ort betreten - ggf. Ortsdaten nachladen (f_0d47 bei Bit 7), dann switch auf die Ortsart:
 * 1 O044, 2 O240, 3 O178 (Zusatzbild bei +20 == 2), 4 O226 (Haendler), 5 O230, 6 O174 (Tempel/Heiler),
 * 7 O012 (Lager), 12 O070 (Herberge) - jeweils "p = new X; if ((r = p->check()) == 1) { delete p; Karte weg
 * (O042_0C9A/O010_12EE); X o; o.run(); } else delete p;" mit Bildern O003_0BCA(ds:37ce, ..); 14 O173 (Klang
 * ds:3892 & 0x3fff), 15 Rueckfrage ds:b67c[448] -> Gruppe verlaesst den Ort (O201_0DF8, +38/+37 aus f_1135,
 * +22 -> +23, +20 -> +21, +33 = 0), 17 f_0df9(ds:283f), 8/9/10 O071 run(0/1/2); 0, 11, 13, 16 nichts. Danach
 * Sonderfall +22 == 9, bei Erfolg (r) Rueckkehr (14: f_0b88, 9/10: f_0bda mit Blickrichtung +1c erhalten,
 * 8: dir 0 und zurueck, sonst f_0bda) und Bild 0xd0 x 0x87 (R42_078A/R42_04DF nach ds:b668), sonst Umdrehen
 * ((ds:285f + 2) & 3) und Gruppe zuruecksetzen; Aufraeumen. f_0b88/f_0bda: Ortsart zuruecksetzen (+26/+2a aus
 * +28/+2c, +1c = ds:285f), f_0bda zusaetzlich Karte/Bildschirm O055_032F/O188_0CBF. f_0c60: Palette 32-mal
 * ueberblenden (R44_0008/R45_0006/R02_00F9). f_0cbe: bei Palette ds:b799 Ein-/Ausblenden (0xc0 Bytes per new
 * und memcpy aus ds:c71e). f_0d47(n): 3 Bytes Ortsdaten aus Datei 0x204 an (ds:187a[+20] + n) * 3 -> Ortsart,
 * ds:283f, ds:2841. f_0df9(n): Ereignisorte - 1..4 O166 (vier Methoden), sonst O031 (24 Methoden fuer 5..28,
 * 25/26 mit Rueckkehr f_0cbe/f_0bda und Bild). f_1135(n): Ortsart beim Verlassen (switch +20, verschachtelt
 * nach n; 4: O031-Objekt mit O184_0479, Ergebnis 0x67), 0 -> O201_0DF8.
 *
 * GELOEST (19.10.2026): BCC -mlarge -3 -Ot -Og -c -Y OVL030.CPP -> 4695/4695 GLEICH, cobjfix laeuft - aber nur,
 *   wenn im selben DOSBox-Lauf direkt davor OVL198.C uebersetzt wird (tools/frisch.txt: "OVL030 OVL198"). Die
 *   switch-Registerwahl haengt von nicht initialisiertem BCC-Heap ab (XMS-KB 277, Bytes +0x2e8/+0x2f8: Bit 0 und
 *   ein Code 0x17/0x8f gesetzt -> "mov ax,si / dec ax / mov bx,ax", frisch genullt -> "mov bx,si / dec bx").
 *   Von 297 Modulen hinterlaesst nur OVL198 diesen Rest. Der Text unten ist der fruehere Stand.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL030.CPP   -> 4687/4695 Bytes, NICHT GLEICH (18.10.2026): f_0000..f_0d47
 *   bytegleich, in f_0df9 und f_1135 fehlen je Sprungtabellen-switch auf den Register-Parameter n 2 Bytes:
 *   das Original rechnet "mov ax,si / dec ax (bzw. sub ax,5/7) / mov bx,ax", BCC macht daraus immer "mov bx,si /
 *   dec bx" (4 Stellen, zusammen 8 Bytes). Getestet ohne Erfolg: alle Optionen (-O/-Oe/-Ol/-Oc/-Z/-Ov/-r-/-rd/
 *   -1/-2), Casts ((int)/(unsigned)/+n/n|0/(n,n)), switch (n - k) mit Faellen ab 0, enum-, unsigned-, volatile-
 *   Parameter, inline-Identitaet, C statt C++. Daher cobjfix noch nicht anwendbar (Verschiebung).
 *   Welle AJ (18.10.2026), zusaetzlich erfolglos: Parameter ohne register / lokale register-Kopie (vor oder nach
 *   den Locals), switch (k = n), switch (n = n), n += 0, n |= 0, n & -1, n ^ 0, n >> 0, n * 1, ~~n, -(-n), *&n,
 *   (short)(long)n und weitere long-/float-/Zeiger-Umwege, enum mit -b-, register volatile/const, inline-
 *   Funktionen (Identitaet mit Kopie, mit lokalem r, mit zwei returns, mit Referenz, Objekt Q(n).get()),
 *   _SI/_AX = n, flag ? 0 : n, n + flag, Optionen -Op/-Om/-Oa/-Ob/-Os/-O1/-O2/-Ox/-O-/-Z-/-k/-N (nicht im
 *   Massentest enthalten). switch in einer inline-Funktion wird nicht expandiert (Warnung).
 *   Mechanismus (Minitests): BCC rechnet "dec ax / mov bx,ax" nur, wenn AX einen frisch berechneten Wert haelt
 *   (cbw, mov ah,0, Zusammenfuehrung eines ?:-Ausdrucks); ist AX nur Kopie einer Variablen (k = n, _AX = n)
 *   kommt "mov bx,ax / dec bx", eine Variable (Register, Stack, *p) direkt "mov bx,var / dec bx". Genau
 *   "mov ax,si / dec ax / mov bx,ax" entsteht z. B. aus "n ? n : 0" - dann aber mit Test/Spruengen davor.
 *   Im ganzen Spiel kommt das Muster nur in O030 vor (4x), 13 andere Stellen haben "mov bx,si / dec bx" -
 *   Verdacht: O030 wurde anders uebersetzt (andere Compilerversion/IDE?).
 * - Kein -Oi (memcpy ist ein Bibliotheksaufruf).
 * - Schluessel: inline party::getdir() { return dir; } (int!) - jeder Lesezugriff auf +1e wird mit -Og zu
 *   mov al / mov [bp-0fh],al / cbw / mov di,ax (Zwischenwerte als Byte und int) und Vergleiche int (cmp ax,1);
 *   direkt "g_c4c5->dir" vergleicht als Byte. In f_0cbe ebenso (cbw / cmp ax,0eh).
 * - inline-Setter: setdir (wie O183), set1c(int), set37/set38(char), set20(char) mit Rueckgabe -
 *   "g_c4c5->set20(g_c4c5->set20(0))" erzeugt die Parameterkopie [bp-8] aus dem inneren Ergebnis.
 * - Grosse lokale Objekte (>= 12 Bytes) in den case-Bloecken liegen alle hinter dem tiefsten kleinen Local
 *   (Bmp/O173 bis [bp-1ah]), jeweils je Block ueberlappend; "X far *p" und "int snd" bei [bp-14h]/[bp-12h].
 * - f_0df9: "case 1..4: C166 o; switch (n) {..} break;" OHNE eigenen Block (Objekt im switch-Gueltigkeitsbereich,
 *   BCC erlaubt das vor default) -> eigener Stackplatz [bp-0ah], das O031-Objekt im default-Block dahinter
 *   ([bp-12h]); mit Block teilen sich beide [bp-0ah]. flag ohne register (DI), n register (SI).
 * - "if ((loc = getdir()) < 0)" eingebettet (or al,al), "loc = getdir(); if (loc == 14)" getrennt (cmp byte
 *   [bp-5]); "g_8bbb = g_8baf = 0", "g_c4d0 = g_c520 = 1".
 * Originalfehler/Auffaelliges: Ortsart 11 fehlt im switch (default), 0/13/16 sind leer.
 */
#include <mem.h>

class Bmp {				/* 10 Bytes (R42) */
	char raw[10];
public:
	Bmp(signed short w, signed short h);
	~Bmp();
	void clear();
};

class C044 { char raw[0x11]; public: C044(); ~C044(); signed short check(); void run(); };
class C240 { char raw[0x1f]; public: C240(); ~C240(); signed short check(); void run(); };
class C178 { char raw[0x16]; public: C178(); ~C178(); signed short check(); void run(); };
class C226 { char raw[0x47]; public: C226(); ~C226(); signed short check(); void run(); };
class C230 { char raw[0x56]; public: C230(); ~C230(); signed short check(); void run(); };
class C174 { char raw[0x0e]; public: C174(); ~C174(); signed short check(); void run(); };
class C012 { char raw[0xb2]; public: C012(); ~C012(); signed short check(); void run(); };
class C070 { char raw[0x14]; public: C070(); ~C070(); signed short check(); void run(); };
class C173 { char raw[8]; public: C173(); ~C173(); void run(); };
class C071 { char raw[0x19]; public: C071(); ~C071(); void run(signed char n); };

class C166 {				/* O166, 8 Bytes */
	char raw[8];
public:
	C166();
	~C166();
	void f1a37();
	void f1687();
	void f0d3b();
	void f1313();
};

class C031 {				/* O031, 8 Bytes */
	char raw[8];
public:
	C031();
	~C031();
	void f0093(); void f00e0(); void f0149(); void f0212(); void f0230(); void f024e();
	void f0266(); void f06a6(); void f0870(); void f08a7(); void f08de(); void f0a2e();
	void f0a73(); void f0ab1(); void f0c56(); void f0d1e(); void f0d53(); void f0f62();
	void f0fd7(); void f1039(); void f10ae(); void f1130(); void f11b9(); void f1216();
	void f184_0479();
};

struct party {
	char pad0[0x1c];
	signed char f1c;		/* +0x1c */
	char pad1d;
	signed char dir;		/* +0x1e */
	signed char olddir;		/* +0x1f */
	signed char f20;		/* +0x20 */
	signed char f21;		/* +0x21 */
	signed char f22;		/* +0x22 */
	signed char f23;		/* +0x23 */
	char pad24[2];
	signed short f26;		/* +0x26 */
	signed short f28;		/* +0x28 */
	signed short f2a;		/* +0x2a */
	signed short f2c;		/* +0x2c */
	char pad2e[0x33 - 0x2e];
	signed long f33;		/* +0x33 */
	signed char f37;		/* +0x37 */
	signed char f38;		/* +0x38 */
	signed short setdir(signed short v)
	{
		if (!(dir & 0x80))
			olddir = dir;
		else
			;
		return dir = v;
	}
	void set1c(signed short v) { f1c = v; }
	signed short getdir() { return dir; }
	signed char set20(signed char v) { f21 = f20; return f20 = v; }
	void set37(signed char v) { f37 = v; }
	void set38(signed char v) { f38 = v; }
};

struct rec3 {
	signed char a, b, c;
};

extern struct party far *g_c4c5;
extern unsigned char g_b59a[], g_37ce[], g_382d[], g_b77f[], g_b77e[], g_35fe[];
extern char g_18ca[], g_18de[], g_18e2[], g_18f8[], g_18fc[], g_1904[], g_195e[], g_1964[], g_1906[], g_1918[],
	g_191c[], g_1926[], g_1928[], g_1932[], g_1934[], g_193e[], g_1940[], g_1948[], g_194a[], g_195c[];
extern char far * far *g_b67c;
extern void far *g_b668;
extern void far *g_b684;
extern void far *g_b799;
extern void far *g_c71e;
extern unsigned char far *g_c6bc;
extern signed short g_b4a0, g_8baf, g_8bbb, g_949e, g_c6c2, g_3892, g_399a, g_283f, g_2841, g_8c30, g_285f,
	g_3996, g_94a4, g_94a2, g_8bd5, g_c4d0;
extern signed char g_2808, g_c715, g_395b, g_256a, g_c520, g_8b94, g_395c, g_b11e, g_b11f;
extern signed long g_187a[];

void O003_0bca(void far *a, char far *b, char far *c, signed short d, signed short e);
void O010_12ee(void);
void O034_26a6(void);
void O042_086f(void far *p);
void O042_0c9a(void far *p);
void O042_0e53(void far *p, signed short a, signed short b, signed short c);
void O046_1a8e(void);
void O054_0ed5(void far *p);
void O055_032f(void far *p);
void O188_0cbf(void far *p);
signed short O190_0082(void far *p, char far *s);
signed short O190_0a4f(void far *p, signed short v);
void O201_0df8(struct party far *g, signed short n);
void R02_00f9(void far *p, signed short a, signed short n);
void R04_0760(void far *p, signed short v);
void R06_0398(void far *f, signed short id);
void R06_080f(void far *f, void far *buf, signed long n);
void R06_0c16(void far *f);
void R06_0d24(void far *f, signed long off, signed short whence);
void R18_0009(signed short a);
void R18_001a(signed short a);
void R18_0175(signed short a, signed short b);
void R18_0ae4(void);
void R18_0afb(void);
void R18_0b12(void);
void R31_0005(void);
void R42_04df(Bmp far *b, void far *p, signed short x, signed short y, signed short m);
void R44_0008(void far *d, void far *s, signed short n);
void R45_0006(void);

void f_0b88(void);
void f_0bda(void);
void f_0cbe(void);
void f_0d47(signed char n);
void f_0df9(signed short n);
signed char f_1135(signed short n);

void f_0000(void)
{
	signed short old;
	signed short s2808;
	signed char loc;
	register signed short r;

	r = 0;
	g_b4a0 = 1;
	R31_0005();
	R18_0b12();
	R18_0ae4();
	R18_0afb();
	g_8bbb = g_8baf = 0;
	s2808 = g_2808;
	g_2808 = 0;
	old = O190_0a4f(g_b59a, 3);
	g_949e = 0;
	g_c6c2 = 1;
	R18_0175(0, 1);
	if ((loc = g_c4c5->getdir()) < 0) {
		loc = (loc & 0x7f) - 1;
		f_0d47(loc);
	}
	switch (g_c4c5->getdir()) {
	case 0:
		break;
	case 1:
		O034_26a6();
		O003_0bca(g_37ce, g_18ca, g_18de, 1, 0);
		{
			C044 far *p = new C044;

			if ((r = p->check()) == 1) {
				delete p;
				O042_0c9a(g_382d);
				O010_12ee();
				C044 o;
				o.run();
			} else
				delete p;
		}
		break;
	case 2:
		O003_0bca(g_37ce, g_18e2, g_18f8, 1, 0);
		{
			C240 far *p = new C240;

			if ((r = p->check()) == 1) {
				delete p;
				O042_0c9a(g_382d);
				O010_12ee();
				C240 o;
				o.run();
			} else
				delete p;
		}
		break;
	case 3:
		O003_0bca(g_37ce, g_18fc, g_1904, 1, 0);
		if (g_c4c5->f20 == 2)
			O003_0bca(g_37ce, g_195e, g_1964, 1, 0);
		{
			C178 far *p = new C178;

			if ((r = p->check()) == 1) {
				delete p;
				O042_0c9a(g_382d);
				O010_12ee();
				C178 o;
				o.run();
			} else
				delete p;
		}
		break;
	case 4:
		O003_0bca(g_37ce, g_1906, g_1918, 1, 0);
		{
			C226 far *p = new C226;

			if ((r = p->check()) == 1) {
				delete p;
				O042_0c9a(g_382d);
				O010_12ee();
				C226 o;
				o.run();
			} else
				delete p;
		}
		break;
	case 5:
		O003_0bca(g_37ce, g_191c, g_1926, 1, 0);
		{
			C230 far *p = new C230;

			if ((r = p->check()) == 1) {
				delete p;
				O042_0c9a(g_382d);
				O010_12ee();
				C230 o;
				o.run();
			} else
				delete p;
		}
		break;
	case 6:
		O003_0bca(g_37ce, g_1928, g_1932, 1, 0);
		{
			C174 far *p = new C174;

			if ((r = p->check()) == 1) {
				delete p;
				O042_0c9a(g_382d);
				O010_12ee();
				C174 o;
				o.run();
			} else
				delete p;
		}
		break;
	case 7:
		O003_0bca(g_37ce, g_1934, g_193e, 1, 0);
		{
			C012 far *p = new C012;

			if ((r = p->check()) == 1) {
				delete p;
				O042_0c9a(g_382d);
				O010_12ee();
				C012 o;
				o.run();
			} else
				delete p;
		}
		break;
	case 12:
		O003_0bca(g_37ce, g_1940, g_1948, 1, 0);
		{
			C070 far *p = new C070;

			if ((r = p->check()) == 1) {
				delete p;
				O042_0c9a(g_382d);
				O010_12ee();
				C070 o;
				o.run();
			} else
				delete p;
		}
		break;
	case 13:
		break;
	case 14:
		{
			signed short snd = g_3892 & 0x3fff;

			if (!g_399a)
				O042_0c9a(g_382d);
			O010_12ee();
			R04_0760(g_382d, snd);
			C173 o;
			o.run();
			r = 1;
		}
		break;
	case 15:
		if (O190_0082(g_b59a, g_b67c[448]) == 1) {
			if (g_c715 == 1)
				g_395b = -1;
			g_256a = 0;
			O042_0c9a(g_382d);
			O010_12ee();
			O201_0df8(g_c4c5, 1);
			g_c4c5->set38(f_1135(g_283f));
			g_c4c5->set37(f_1135(g_283f));
			g_c4c5->f23 = g_c4c5->f22;
			g_c4c5->f22 = 0;
			g_c4c5->f21 = g_c4c5->f20;
			g_c4c5->f20 = 0;
			g_c4c5->f33 = 0;
		}
		break;
	case 16:
		break;
	case 17:
		f_0df9(g_283f);
		break;
	case 8:
		O034_26a6();
		O003_0bca(g_37ce, g_194a, g_195c, 1, 0);
		O042_0c9a(g_382d);
		O010_12ee();
		{
			C071 o;

			r = 1;
			o.run(0);
		}
		break;
	case 9:
		O034_26a6();
		O003_0bca(g_37ce, g_194a, g_195c, 1, 0);
		O042_0c9a(g_382d);
		O010_12ee();
		{
			C071 o;

			r = 1;
			o.run(1);
		}
		break;
	case 10:
		O034_26a6();
		O003_0bca(g_37ce, g_194a, g_195c, 1, 0);
		O042_0c9a(g_382d);
		O010_12ee();
		{
			C071 o;

			r = 1;
			o.run(2);
		}
		break;
	default:
		break;
	}
	if (g_c4c5->f22 == 9) {
		g_c4c5->dir = g_c4c5->olddir;
		g_c4c5->olddir = 0;
		if (g_c4c5->getdir() != 1)
			g_c4c5->set20(g_c4c5->set20(0));
		g_c4c5->setdir(g_c4c5->getdir());
	}
	if (r) {
		f_0cbe();
		loc = g_c4c5->getdir();
		if (loc == 14) {
			O055_032f(g_b77f);
			f_0b88();
			g_c520 = 1;
			g_8c30 = (g_c4c5->f1c << 8) + 0x100;
		} else if (loc == 9 || loc == 10) {
			signed short t = g_c4c5->f1c;

			f_0bda();
			O042_086f(g_382d);
			g_c4c5->set1c(t);
		} else if (loc == 8) {
			g_c4c5->dir = g_c4c5->olddir;
			g_c4c5->olddir = 0;
			loc = g_c4c5->getdir();
			g_c4c5->setdir(0);
			g_c4c5->setdir(loc);
			loc = 8;
			O042_086f(g_382d);
			g_8c30 = (g_c4c5->f1c << 8) + 0x100;
		} else {
			f_0bda();
			O042_086f(g_382d);
			g_8c30 = (g_c4c5->f1c << 8) + 0x100;
		}
		O054_0ed5(g_b77f);
		{
			Bmp b(0xd0, 0x87);

			b.clear();
			R18_001a(0);
			R42_04df(&b, g_b668, 0x10, 2, 5);
			R18_0009(0);
		}
		g_8b94 = 1;
	} else {
		if (g_c4c5->getdir() != 15) {
			g_8b94 = 1;
			g_c4c5->set1c((g_285f + 2) & 3);
			g_8c30 = (g_c4c5->f1c << 8) + 0x100;
		}
		g_c4c5->dir = g_c4c5->olddir;
		g_c4c5->olddir = 0;
		g_c4c5->f26 = g_c4c5->f28;
		g_c4c5->f2a = g_c4c5->f2c;
		R31_0005();
	}
	O046_1a8e();
	if (g_3996)
		g_3996 = 0;
	O190_0a4f(g_b59a, old);
	if (!g_2808)
		g_2808 = s2808;
	g_395c = -1;
	g_94a4 = 0;
	g_94a2 = 0;
	g_8bd5 = 0;
	g_b11e = 0;
	g_b11f = 0;
}

void f_0b88(void)
{
	g_c4c5->dir = g_c4c5->olddir;
	g_c4c5->olddir = 0;
	g_c4c5->f26 = g_c4c5->f28;
	g_c4c5->f2a = g_c4c5->f2c;
	g_c4c5->set1c(g_285f);
}

void f_0bda(void)
{
	if (!g_399a)
		O042_0c9a(g_382d);
	O055_032f(g_b77f);
	g_c4c5->dir = g_c4c5->olddir;
	g_c4c5->olddir = 0;
	g_c4c5->f26 = g_c4c5->f28;
	g_c4c5->f2a = g_c4c5->f2c;
	g_c4c5->set1c(g_285f);
	O188_0cbf(g_b77e);
	g_c4d0 = g_c520 = 1;
}

void f_0c60(void)
{
	register signed short i;

	for (i = 0; i < 32; i++) {
		R44_0008(g_c6bc, g_b684, 0x40);
		R44_0008(g_c6bc, g_b684, 0x40);
		R45_0006();
		R02_00f9(g_c6bc, 0, 0x40);
	}
}

void f_0cbe(void)
{
	if (g_b799) {
		if (g_c4c5->getdir() != 14)
			O042_0e53(g_382d, 0, 1000, 0);
		g_c6bc = new unsigned char[0xc0];
		memcpy(g_c6bc, g_c71e, 0xc0);
		f_0c60();
		delete g_c6bc;
		if (g_c4c5->getdir() != 14)
			O042_0c9a(g_382d);
	}
}

void f_0d47(signed char n)
{
	struct rec3 r;
	signed long off;

	off = (g_187a[g_c4c5->f20] + n) * 3;
	R06_0398(g_35fe, 0x204);
	R06_0d24(g_35fe, off, 0);
	R06_080f(g_35fe, &r, 3);
	R06_0c16(g_35fe);
	g_c4c5->setdir(r.a);
	g_283f = r.b;
	g_2841 = r.c;
}

void f_0df9(register signed short n)
{
	signed short snd;
	signed short flag;

	flag = 0;
	snd = g_3892 & 0x3fff;
	if (!g_399a)
		O042_0c9a(g_382d);
	O010_12ee();
	R04_0760(g_382d, snd);
	switch (n) {
	case 1:
	case 2:
	case 3:
	case 4:
		C166 o;
		switch (n) {
		case 1:
			o.f1a37();
			break;
		case 2:
			o.f1687();
			break;
		case 3:
			o.f0d3b();
			break;
		case 4:
			o.f1313();
			break;
		}
		break;
	default:
		{
			C031 o;

			switch (n) {
			case 5: o.f0093(); break;
			case 6: o.f00e0(); break;
			case 7: o.f0149(); break;
			case 8: o.f0212(); break;
			case 9: o.f0230(); break;
			case 10: o.f024e(); break;
			case 11: o.f0266(); break;
			case 12: o.f06a6(); break;
			case 13: o.f0870(); break;
			case 14: o.f08a7(); break;
			case 15: o.f08de(); break;
			case 16: o.f0a2e(); break;
			case 17: o.f0a73(); break;
			case 18: o.f0ab1(); break;
			case 19: o.f0c56(); break;
			case 20: o.f0d1e(); break;
			case 21: o.f0d53(); break;
			case 22: o.f0f62(); break;
			case 23: o.f0fd7(); break;
			case 24: o.f1039(); break;
			case 25:
				o.f10ae();
				f_0cbe();
				f_0bda();
				O054_0ed5(g_b77f);
				flag = 1;
				break;
			case 26:
				o.f1130();
				f_0cbe();
				f_0bda();
				O054_0ed5(g_b77f);
				flag = 1;
				break;
			case 27: o.f11b9(); break;
			case 28: o.f1216(); break;
			}
		}
		break;
	}
	if (flag) {
		Bmp b(0xd0, 0x87);

		b.clear();
		R18_001a(0);
		R42_04df(&b, g_b668, 0x10, 2, 5);
		R18_0009(0);
	}
}

signed char f_1135(register signed short n)
{
	signed short r = 0;

	switch (g_c4c5->f20) {
	case 1:
		r = 1;
		break;
	case 2:
		switch (n) {
		case 2:
			r = 2;
			break;
		case 3:
			r = 3;
			break;
		case 4:
			r = 4;
			break;
		}
		break;
	case 3:
		r = 5;
		break;
	case 4:
		{
			C031 o;

			o.f184_0479();
			r = 0x67;
		}
		break;
	case 5:
		switch (n) {
		case 7:
			r = 12;
			break;
		case 8:
			r = 13;
			break;
		case 9:
			r = 14;
			break;
		case 10:
			r = 15;
			break;
		case 11:
			r = 16;
			break;
		}
		break;
	case 6:
		switch (n) {
		case 12:
			r = 0x13;
			break;
		case 13:
			r = 0x14;
			break;
		}
		break;
	}
	if (r == 0)
		O201_0df8(g_c4c5, 1);
	return r;
}
