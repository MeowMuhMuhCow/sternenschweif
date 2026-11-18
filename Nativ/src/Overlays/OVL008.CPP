/* OVL008 - Overlay O008 (Segment 0x1f24 in SCHWEIF.EXE): Heldenbogen-/Inventarobjekt Inv (0x144 Bytes, Zeiger
 * g_inv ds:8b90; Datensicht struct inv_sheet, Methoden class Inv in s_class.h). Member: +0 txt_buf/+4 txt (txt_load_list(g_font,
 * 0x1c, 0x5f)), +8 page, +0f f0f fuer w/h, +11 cursor (Bild-Nr. des Mauszeigers), +13 hero/+17 owner (Helden), +25 swap/
 * +27 Merker, +28 slot/+2a slot2/+2c/+2e Auswahl, +30 marks je Platz (0/0xfc), +4f Nvf-Objekt (O098, 0x19 Bytes), +68
 * Pic-Objekt (O269, 0xa6), +10e..+13a zwoelf Bmp-Zeiger, +13e Puffer 64000 Bytes. Inv(): Member-Ktoren Nvf/Pic,
 * snd_obj_stop, Bilderliste laden (load_pic_list(g_cd_cache, g_cl_inventory, g_cl_inventory_st, 1, 0)), Puffer, Texte, Bmp-Objekte,
 * 7 Bilder aus Datei 0x10. ~Inv(): Bmp loeschen, Speicher frei, g_pp20_index = -1, g_refresh = 1, Member-Dtoren
 * rueckwaerts. set_cursor(n): Bild n (Datei 0x0b) 16x16 als Mauszeiger (mouse_set_cursor_bmp). reset_cursor:
 * Zeiger zurueck. update_marks: Markierungen marks[0..14] fuer den Gegenstand der zweiten Auswahl (can_hero_use_item/
 * can_item_at_pos/two_hand_collision). draw_slots: alle 15 Koerperplaetze zeichnen. draw_slot(dst, n, color): Platz n
 * zeichnen (Rahmen 0/0xfd/0xf9, Plaetze 31/32 per do_border, Statuszeile mit Gegenstandsname grammar_name, Zusatz
 * g_ttx[75 + Untertyp]). reselect: Auswahl neu (Maus ueber get_mouse_action). sel_next/sel_prev: Auswahl weiter/
 * zurueck. Die Klickfelder g_inv_areas_knapsack (Plaetze 15..30) und g_inv_areas_all (33 Plaetze) haben nicht
 * konstante Initialisierer {x, y, x+17, y+17} aus g_inv_slot_xy (Plaetze 31/32 +20/+16); BCC erzeugt dafuer den
 * statischen Initialisierer O008_0DFC (lokal @_STCON_) und den _INIT_-Eintrag (Prioritaet 32).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL008.CPP   -> 5058/5058 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Erstentwurf sofort GLEICH). Ohne -Og 2150 Byte-Abweichungen.
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - C++: Konstruktor/Destruktor mit Member-Objekten (Ktor-Aufrufe vor dem Rumpf, toter Epilog nach dem Ktor,
 *   Dtoren Pic_dtor(p,2)/Nvf_dtor(p,2) nach dem Rumpf), new Bmp -> R42_00D8/R42_004B(0, ..), delete ->
 *   R42_03B3(p, 3) ohne Nulltest. Bmp aus s_class.h (put(dst, x, y, 4) = frueher "fade").
 * - -Og: &f0f einmal berechnet (SI bzw. [bp-2], wenn SI = n), Zwischenwert slot * 27 [bp-6] in draw_slot, (int)
 *   g_inv_show_body [bp-2] in sel_next/sel_prev; Ketten "done = ill = swap = f27 = 0" und
 *   "old_slot = slot2 = old_slot2 = cursor = -1".
 * - Phase B (19.10.2026): die Felder mit ihren echten (nicht konstanten) Initialisierern; -Og legt im erzeugten
 *   Initialisierer die 32 Ausdruecke "g_inv_slot_xy[k].x + 17" des ersten Feldes als Zwischenwerte [bp-2]..[bp-40h] ab
 *   und benutzt sie im zweiten Feld wieder (sub sp,40h) - Code unveraendert GLEICH. Vorher: normale Funktion
 *   inv_init_areas, _INIT_-Eintrag im ASM-Reststueck. Beide Felder enden mit Restfeld {0, 0, 319, 199, 0x7e} und -1.
 * - Prototyp-Abweichung (Aufruf ueber gecasteten Funktionszeiger, gleicher direkter far call): get_mouse_action
 *   mit char far * (mit dem void-far-Prototyp pusht BCC beim ?: zweier Felder zuerst ds statt dx:ax zu bilden).
 * - Nvf/Pic aus s_class.h. Die Klasse steht als Sicht "InvObj" im Modul: inv_sheet fuehrt nvf/pic als Bytefelder
 *   (O094 castet sie), der Konstruktor braucht echte Elementobjekte. Wird inv_sheet in C++ auf Nvf/Pic umgestellt
 *   (O094: nvf.load statt ((Nvf far *)nvf)->load), kann InvObj in class Inv aufgehen.
 * Auffaelliges: draw_slot "if (g_inv_show_body || !g_inv_show_body && n >= 15 && n < 31)" (redundanter Test);
 *   update_marks prueft die zweite Auswahl slot2, nimmt den Gegenstand aber aus owner->inv[slot].
 */
#include <stdio.h>
#include <string.h>
#include <mem.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:0172..0500, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
signed short g_inv_status_slots[5] = { 14, 6, 30, 29, -1 };
struct point g_inv_slot_xy[33] = {
	{ 98, 56 },
	{ 86, 75 },
	{ 89, 105 },
	{ 96, 124 },
	{ 96, 143 },
	{ 91, 162 },
	{ 91, 181 },
	{ 121, 103 },
	{ 142, 54 },
	{ 161, 70 },
	{ 161, 89 },
	{ 154, 108 },
	{ 148, 127 },
	{ 148, 146 },
	{ 150, 178 },
	{ 10, 50 },
	{ 29, 50 },
	{ 10, 68 },
	{ 29, 68 },
	{ 10, 86 },
	{ 29, 86 },
	{ 10, 104 },
	{ 29, 104 },
	{ 10, 122 },
	{ 29, 122 },
	{ 10, 140 },
	{ 29, 140 },
	{ 10, 158 },
	{ 29, 158 },
	{ 10, 176 },
	{ 29, 176 },
	{ 108, 39 },
	{ 129, 39 }
};
char g_inv_mouse_knapsack[80] = {
	0x59, 0x00, 0x06, 0x00, 0xb4, 0x00, 0x0d, 0x00, 0xa1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x19, 0x00,
	0x30, 0x00, 0x00, 0x4b, 0x1e, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x30, 0x00, 0x00, 0x4d, 0x52, 0x00,
	0x1d, 0x00, 0xb8, 0x00, 0x25, 0x00, 0x7a, 0x00, 0x78, 0x00, 0x32, 0x00, 0x8c, 0x00, 0x3c, 0x00,
	0x7b, 0x00, 0x59, 0x00, 0x56, 0x00, 0xb8, 0x00, 0x5c, 0x00, 0xa2, 0x00, 0x59, 0x00, 0xb8, 0x00,
	0xb8, 0x00, 0xbe, 0x00, 0x7c, 0x00, 0x59, 0x00, 0xbf, 0x00, 0xb8, 0x00, 0xc7, 0x00, 0x7d, 0x00
};
struct mouse_area g_inv_areas_knapsack[18] = {	/* Rucksackplaetze 15..30 (Lage aus g_inv_slot_xy, zur Laufzeit), Rest, Ende */
	{ g_inv_slot_xy[15].x, g_inv_slot_xy[15].y, g_inv_slot_xy[15].x + 17, g_inv_slot_xy[15].y + 17, 143 },
	{ g_inv_slot_xy[16].x, g_inv_slot_xy[16].y, g_inv_slot_xy[16].x + 17, g_inv_slot_xy[16].y + 17, 144 },
	{ g_inv_slot_xy[17].x, g_inv_slot_xy[17].y, g_inv_slot_xy[17].x + 17, g_inv_slot_xy[17].y + 17, 145 },
	{ g_inv_slot_xy[18].x, g_inv_slot_xy[18].y, g_inv_slot_xy[18].x + 17, g_inv_slot_xy[18].y + 17, 146 },
	{ g_inv_slot_xy[19].x, g_inv_slot_xy[19].y, g_inv_slot_xy[19].x + 17, g_inv_slot_xy[19].y + 17, 147 },
	{ g_inv_slot_xy[20].x, g_inv_slot_xy[20].y, g_inv_slot_xy[20].x + 17, g_inv_slot_xy[20].y + 17, 148 },
	{ g_inv_slot_xy[21].x, g_inv_slot_xy[21].y, g_inv_slot_xy[21].x + 17, g_inv_slot_xy[21].y + 17, 149 },
	{ g_inv_slot_xy[22].x, g_inv_slot_xy[22].y, g_inv_slot_xy[22].x + 17, g_inv_slot_xy[22].y + 17, 150 },
	{ g_inv_slot_xy[23].x, g_inv_slot_xy[23].y, g_inv_slot_xy[23].x + 17, g_inv_slot_xy[23].y + 17, 151 },
	{ g_inv_slot_xy[24].x, g_inv_slot_xy[24].y, g_inv_slot_xy[24].x + 17, g_inv_slot_xy[24].y + 17, 152 },
	{ g_inv_slot_xy[25].x, g_inv_slot_xy[25].y, g_inv_slot_xy[25].x + 17, g_inv_slot_xy[25].y + 17, 153 },
	{ g_inv_slot_xy[26].x, g_inv_slot_xy[26].y, g_inv_slot_xy[26].x + 17, g_inv_slot_xy[26].y + 17, 154 },
	{ g_inv_slot_xy[27].x, g_inv_slot_xy[27].y, g_inv_slot_xy[27].x + 17, g_inv_slot_xy[27].y + 17, 155 },
	{ g_inv_slot_xy[28].x, g_inv_slot_xy[28].y, g_inv_slot_xy[28].x + 17, g_inv_slot_xy[28].y + 17, 156 },
	{ g_inv_slot_xy[29].x, g_inv_slot_xy[29].y, g_inv_slot_xy[29].x + 17, g_inv_slot_xy[29].y + 17, 157 },
	{ g_inv_slot_xy[30].x, g_inv_slot_xy[30].y, g_inv_slot_xy[30].x + 17, g_inv_slot_xy[30].y + 17, 158 },
	{ 0, 0, 319, 199, 0x7e },
	{ -1, -1, -1, -1, -1 }
};
char g_inv_mouse_all[30] = {
	0x00, 0x00, 0x00, 0x00, 0x19, 0x00, 0x30, 0x00, 0x00, 0x4b, 0x1e, 0x00, 0x00, 0x00, 0x3c, 0x00,
	0x30, 0x00, 0x00, 0x4d, 0x52, 0x00, 0x1d, 0x00, 0xb8, 0x00, 0x25, 0x00, 0x7a, 0x00
};
struct mouse_area g_inv_areas_all[35] = {	/* alle 33 Plaetze (Lage aus g_inv_slot_xy, zur Laufzeit), Rest, Ende */
	{ g_inv_slot_xy[0].x, g_inv_slot_xy[0].y, g_inv_slot_xy[0].x + 17, g_inv_slot_xy[0].y + 17, 128 },
	{ g_inv_slot_xy[1].x, g_inv_slot_xy[1].y, g_inv_slot_xy[1].x + 17, g_inv_slot_xy[1].y + 17, 129 },
	{ g_inv_slot_xy[2].x, g_inv_slot_xy[2].y, g_inv_slot_xy[2].x + 17, g_inv_slot_xy[2].y + 17, 130 },
	{ g_inv_slot_xy[3].x, g_inv_slot_xy[3].y, g_inv_slot_xy[3].x + 17, g_inv_slot_xy[3].y + 17, 131 },
	{ g_inv_slot_xy[4].x, g_inv_slot_xy[4].y, g_inv_slot_xy[4].x + 17, g_inv_slot_xy[4].y + 17, 132 },
	{ g_inv_slot_xy[5].x, g_inv_slot_xy[5].y, g_inv_slot_xy[5].x + 17, g_inv_slot_xy[5].y + 17, 133 },
	{ g_inv_slot_xy[6].x, g_inv_slot_xy[6].y, g_inv_slot_xy[6].x + 17, g_inv_slot_xy[6].y + 17, 134 },
	{ g_inv_slot_xy[7].x, g_inv_slot_xy[7].y, g_inv_slot_xy[7].x + 17, g_inv_slot_xy[7].y + 17, 135 },
	{ g_inv_slot_xy[8].x, g_inv_slot_xy[8].y, g_inv_slot_xy[8].x + 17, g_inv_slot_xy[8].y + 17, 136 },
	{ g_inv_slot_xy[9].x, g_inv_slot_xy[9].y, g_inv_slot_xy[9].x + 17, g_inv_slot_xy[9].y + 17, 137 },
	{ g_inv_slot_xy[10].x, g_inv_slot_xy[10].y, g_inv_slot_xy[10].x + 17, g_inv_slot_xy[10].y + 17, 138 },
	{ g_inv_slot_xy[11].x, g_inv_slot_xy[11].y, g_inv_slot_xy[11].x + 17, g_inv_slot_xy[11].y + 17, 139 },
	{ g_inv_slot_xy[12].x, g_inv_slot_xy[12].y, g_inv_slot_xy[12].x + 17, g_inv_slot_xy[12].y + 17, 140 },
	{ g_inv_slot_xy[13].x, g_inv_slot_xy[13].y, g_inv_slot_xy[13].x + 17, g_inv_slot_xy[13].y + 17, 141 },
	{ g_inv_slot_xy[14].x, g_inv_slot_xy[14].y, g_inv_slot_xy[14].x + 17, g_inv_slot_xy[14].y + 17, 142 },
	{ g_inv_slot_xy[15].x, g_inv_slot_xy[15].y, g_inv_slot_xy[15].x + 17, g_inv_slot_xy[15].y + 17, 143 },
	{ g_inv_slot_xy[16].x, g_inv_slot_xy[16].y, g_inv_slot_xy[16].x + 17, g_inv_slot_xy[16].y + 17, 144 },
	{ g_inv_slot_xy[17].x, g_inv_slot_xy[17].y, g_inv_slot_xy[17].x + 17, g_inv_slot_xy[17].y + 17, 145 },
	{ g_inv_slot_xy[18].x, g_inv_slot_xy[18].y, g_inv_slot_xy[18].x + 17, g_inv_slot_xy[18].y + 17, 146 },
	{ g_inv_slot_xy[19].x, g_inv_slot_xy[19].y, g_inv_slot_xy[19].x + 17, g_inv_slot_xy[19].y + 17, 147 },
	{ g_inv_slot_xy[20].x, g_inv_slot_xy[20].y, g_inv_slot_xy[20].x + 17, g_inv_slot_xy[20].y + 17, 148 },
	{ g_inv_slot_xy[21].x, g_inv_slot_xy[21].y, g_inv_slot_xy[21].x + 17, g_inv_slot_xy[21].y + 17, 149 },
	{ g_inv_slot_xy[22].x, g_inv_slot_xy[22].y, g_inv_slot_xy[22].x + 17, g_inv_slot_xy[22].y + 17, 150 },
	{ g_inv_slot_xy[23].x, g_inv_slot_xy[23].y, g_inv_slot_xy[23].x + 17, g_inv_slot_xy[23].y + 17, 151 },
	{ g_inv_slot_xy[24].x, g_inv_slot_xy[24].y, g_inv_slot_xy[24].x + 17, g_inv_slot_xy[24].y + 17, 152 },
	{ g_inv_slot_xy[25].x, g_inv_slot_xy[25].y, g_inv_slot_xy[25].x + 17, g_inv_slot_xy[25].y + 17, 153 },
	{ g_inv_slot_xy[26].x, g_inv_slot_xy[26].y, g_inv_slot_xy[26].x + 17, g_inv_slot_xy[26].y + 17, 154 },
	{ g_inv_slot_xy[27].x, g_inv_slot_xy[27].y, g_inv_slot_xy[27].x + 17, g_inv_slot_xy[27].y + 17, 155 },
	{ g_inv_slot_xy[28].x, g_inv_slot_xy[28].y, g_inv_slot_xy[28].x + 17, g_inv_slot_xy[28].y + 17, 156 },
	{ g_inv_slot_xy[29].x, g_inv_slot_xy[29].y, g_inv_slot_xy[29].x + 17, g_inv_slot_xy[29].y + 17, 157 },
	{ g_inv_slot_xy[30].x, g_inv_slot_xy[30].y, g_inv_slot_xy[30].x + 17, g_inv_slot_xy[30].y + 17, 158 },
	{ g_inv_slot_xy[31].x, g_inv_slot_xy[31].y, g_inv_slot_xy[31].x + 20, g_inv_slot_xy[31].y + 16, 159 },
	{ g_inv_slot_xy[32].x, g_inv_slot_xy[32].y, g_inv_slot_xy[32].x + 20, g_inv_slot_xy[32].y + 16, 160 },
	{ 0, 0, 319, 199, 0x7e },
	{ -1, -1, -1, -1, -1 }
};
char g_areas_sheet[40] = {
	0x00, 0x00, 0x00, 0x00, 0x19, 0x00, 0x30, 0x00, 0x00, 0x4b, 0x1e, 0x00, 0x00, 0x00, 0x3c, 0x00,
	0x30, 0x00, 0x00, 0x4d, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x01, 0xc7, 0x00, 0x7e, 0x00, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};
char g_cl_inventory[70] = {
	0x30, 0x20, 0x31, 0x20, 0x32, 0x20, 0x33, 0x20, 0x34, 0x20, 0x35, 0x20, 0x36, 0x20, 0xa1, 0x21,
	0x37, 0x20, 0x38, 0x20, 0x39, 0x20, 0x3a, 0x20, 0x3b, 0x20, 0x3c, 0x20, 0x3d, 0x20, 0xa2, 0x21,
	0xa0, 0x21, 0x67, 0x23, 0xe2, 0x22, 0xd9, 0x24, 0x9c, 0x20, 0xf8, 0x21, 0x73, 0x40, 0x72, 0x40,
	0x83, 0x40, 0x82, 0x40, 0x74, 0x40, 0xaf, 0x40, 0xb0, 0x40, 0xb1, 0x40, 0xb2, 0x40, 0xb3, 0x40,
	0xb4, 0x40, 0x21, 0xc4, 0xff, 0xff
};
char g_cl_inventory_st[10] = "";

/* Heldenbogen-/Inventarobjekt mit echten Elementobjekten Nvf/Pic: struct inv_sheet (s_class.h) fuehrt nvf/pic als
 * Bytefelder (O094 castet), der Konstruktor braucht aber die Ktor-/Dtor-Aufrufe der Elemente - daher diese Sicht
 * gleichen Aufbaus mit den Feldnamen von inv_sheet (Methodennamen wie class Inv in s_class.h). */
class InvObj {
public:
	void far *txt_buf;		/* +0x000 Textpuffer (txt_load_list Datei 0x1c) */
	char far * far *txt;		/* +0x004 Texte */
	signed char page;		/* +0x008 Seite */
	signed short done;		/* +0x009 */
	signed short redraw;		/* +0x00b */
	signed short sel;		/* +0x00d */
	signed short f0f;		/* +0x00f Ziel fuer Breite/Hoehe von nvfile_load */
	signed short cursor;		/* +0x011 Mauszeigerbild, -1 = normal */
	struct hero far *hero;		/* +0x013 angezeigter Held */
	struct hero far *owner;		/* +0x017 Held, dessen Gegenstand gewaehlt ist */
	signed char _r1b[4];
	signed short ill;		/* +0x01f */
	signed char _r21[4];
	signed short swap;		/* +0x025 */
	signed char f27;		/* +0x027 */
	signed short slot;		/* +0x028 gewaehlter Platz */
	signed short slot2;		/* +0x02a Zielplatz */
	signed short old_slot;		/* +0x02c */
	signed short old_slot2;		/* +0x02e */
	unsigned char marks[0x1f];	/* +0x030 Markierung je Platz (0 / 0xfc = passt) */
	Nvf nvf;			/* +0x04f */
	Pic pic;			/* +0x068 */
	Bmp far *screen;		/* +0x10e ganzer Bildschirm (320 x 200, Puffer buf) */
	Bmp far *b112;			/* +0x112 32 x 32 */
	Bmp far *item_bmp;		/* +0x116 16 x 16 (Mauszeiger mit Gegenstandsbild) */
	Bmp far *body;			/* +0x11a 0x67 x 0xa7, Bild 0 */
	Bmp far *b11e;			/* +0x11e Rahmen 18 x 18, Bild 1 */
	Bmp far *b122;			/* +0x122 Rahmen Farbe 0xfd (Auswahl), Bild 3 */
	Bmp far *b126;			/* +0x126 Rahmen markiert, Bild 4 */
	Bmp far *b12a;			/* +0x12a Rahmen andere Farbe, Bild 2 */
	Bmp far *knapsack;		/* +0x12e 6 x 6, Bild 5 */
	Bmp far *status_bg;		/* +0x132 Statuszeile 320 x 8 */
	Bmp far *b136;			/* +0x136 21 x 20, Bild 6 */
	Bmp far *b13a;			/* +0x13a 21 x 20 */
	void far *buf;			/* +0x13e 64000 Bytes */
	signed char _r142[2];

	InvObj();					/* O008_0000 Inv_ctor */
	~InvObj();					/* O008_048B Inv_dtor */
	void set_cursor(signed short n);		/* O008_0635 */
	void reset_cursor();				/* O008_06C1 */
	void update_marks();				/* O008_06DB */
	void draw_slots(void far *dst);			/* O008_07AE */
	void draw_slot(void far *dst, signed short n, signed short color); /* O008_07EE */
	void reselect();				/* O008_0B9F */
	void sel_next();				/* O008_0D14 */
	void sel_prev();				/* O008_0D88 */
};

InvObj::InvObj()
{
	snd_obj_stop(g_snd_music);
	load_pic_list(g_cd_cache, g_cl_inventory, g_cl_inventory_st, 1, 0);
	buf = mem_alloc(0xfa00UL);
	txt_load_list(g_font, 0x1c, 0x5f, &txt_buf, &txt);
	screen = new Bmp(320, 200, buf);
	status_bg = new Bmp(320, 8);
	b112 = new Bmp(32, 32, 0);
	item_bmp = new Bmp(16, 16);
	knapsack = new Bmp(6, 6);
	body = new Bmp(0x67, 0xa7);
	b11e = new Bmp(18, 18);
	b12a = new Bmp(18, 18);
	b122 = new Bmp(18, 18);
	b126 = new Bmp(18, 18);
	b136 = new Bmp(21, 20);
	b13a = new Bmp(21, 20);
	memset(marks, 0, 0x1f);
	nvf.load(0x10, body->data(), 0, 0, 0, 0, &f0f, &f0f, 0x140);
	nvf.load(0x10, b11e->data(), 0, 0, 1, 0, &f0f, &f0f, 0x140);
	nvf.load(0x10, b12a->data(), 0, 0, 2, 0, &f0f, &f0f, 0x140);
	nvf.load(0x10, b122->data(), 0, 0, 3, 0, &f0f, &f0f, 0x140);
	nvf.load(0x10, b126->data(), 0, 0, 4, 0, &f0f, &f0f, 0x140);
	nvf.load(0x10, knapsack->data(), 0, 0, 5, 0, &f0f, &f0f, 0x140);
	nvf.load(0x10, b136->data(), 0, 0, 6, 0, &f0f, &f0f, 0x140);
	done = ill = swap = f27 = 0;
	redraw = 1;
	slot = INV_SLOT_KNAPSACK;
	if (g_mouse_locked == 0)
		slot = -1;
	old_slot = slot2 = old_slot2 = cursor = -1;
	g_inv = this;
}

InvObj::~InvObj()
{
	delete b13a;
	delete b136;
	delete knapsack;
	delete b126;
	delete b122;
	delete b12a;
	delete b11e;
	delete body;
	delete item_bmp;
	delete b112;
	delete status_bg;
	delete screen;
	mem_free(txt_buf);
	mem_free(txt);
	mem_free(buf);
	g_pp20_index = -1;
	g_refresh = 1;
}

void InvObj::set_cursor(signed short n)
{
	if (g_mouse_locked == 0) {
		cursor = n;
		nvf.load(0x0b, item_bmp->data(), 0, 0, n, 0, &f0f, &f0f, 0x140);
		mouse_set_cursor_bmp(item_bmp->data(), 16, 16, 8, 8, 1);
	}
}

void InvObj::reset_cursor()
{
	cursor = -1;
	mouse_set_cursor(0, 1);
}

void InvObj::update_marks()
{
	signed short id = 0;
	signed short i;

	/* Auffaellig: prueft slot2, nimmt den Gegenstand aber von owner->inv[slot] */
	if (slot2 != -1)
		id = owner->inv[slot].item_id;
	if (id != 0 && !can_hero_use_item(hero, id))
		id = 0;
	for (i = 0; i < INV_SLOT_KNAPSACK; i++) {
		if (id == 0)
			marks[i] = 0;
		else if (!can_item_at_pos(hero, id, i))
			marks[i] = 0;
		else if (two_hand_collision(hero, id, i))
			marks[i] = 0;
		else
			marks[i] = 0xfc;
	}
}

void InvObj::draw_slots(void far *dst)
{
	signed short i;

	if (g_inv_show_body == 1 && f27 == 0 && g_pp20_index == 14)
		for (i = 0; i < INV_SLOT_KNAPSACK; i++)
			draw_slot(dst, i, 0);
}

void InvObj::draw_slot(void far *dst, signed short n, signed short color)
{
	signed short x, y;

	if (n == -1)
		return;
	if (g_inv_show_body != 0 || !g_inv_show_body && n >= INV_SLOT_KNAPSACK && n < NR_INV_SLOTS) {
		mouse_hide(0);
		if (dst == g_vga_buf && is_in_word_array(n, g_inv_status_slots) != -1)
			status_bg->put(dst, 0, 0xc0);
		x = g_inv_slot_xy[n].x;
		y = g_inv_slot_xy[n].y;
		if (n >= 0 && n < NR_INV_SLOTS) {
			if (color == 0) {
				if (marks[n] == 0)
					b11e->put(dst, x, y, 4);
				else
					b126->put(dst, x, y, 4);
			} else if (color == 0xfd)
				b122->put(dst, x, y, 4);
			else
				b12a->put(dst, x, y, 4);
		} else if (n >= NR_INV_SLOTS) {
			/* Plaetze 31/32 (Bildschirmfelder) mit Rahmen */
			if (color == 0)
				color = 0xe8;
			do_border(dst, x, y, 0x13, 0x10, color);
			if (n == 32 && color == 0xe8) {
				if (marks[8] == 0xfc)
					b126->put(dst, g_inv_slot_xy[8].x, g_inv_slot_xy[8].y, 4);
				else
					b11e->put(dst, g_inv_slot_xy[8].x, g_inv_slot_xy[8].y, 4);
			}
		}
		if (dst == g_vga_buf && is_in_word_array(n, g_inv_status_slots) != -1) {
			/* Statuszeile: Name des gewaehlten Gegenstands */
			status_bg->get(dst, 0, 0xc0);
			if (owner->inv[slot].item_id != 0) {
				sprintf(g_text_output_buf, "%s %s ", grammar_name(g_item_names[owner->inv[slot].item_id]),
					can_use_item(hero, owner->inv[slot].item_id) ? (char far *)"" : txt[0]);
				if (g_items[owner->inv[slot].item_id].weapon && !g_items[owner->inv[slot].item_id].stackable)
					strcat(g_text_output_buf, g_ttx[g_items[owner->inv[slot].item_id].subtype + 75]);
				font_set_colors(g_font, 0xff, -1);
				font_print(g_font, g_text_output_buf, 0, 0xc0);
			}
		}
		mouse_show(0);
	}
}

void InvObj::reselect()
{
	signed short action;

	if (page < 3) {
		if (g_pp20_index == 14 || g_pp20_index == 15) {
			draw_slot(g_vga_buf, slot, 0);
			draw_slot(g_vga_buf, slot2, 0);
			draw_slot(g_vga_buf, old_slot, 0);
			draw_slot(g_vga_buf, old_slot2, 0);
		}
		slot = -1;
		if (g_mouse_locked == 0) {
			/* Original deklariert den Parameter als char far * (mit void far * wird ds vorab gepusht) */
			action = ((signed short (*)(char far *))get_mouse_action)(g_inv_show_body ? g_inv_mouse_all : g_inv_mouse_knapsack);
			if (action >= 0x80 && action < 0x9f)
				slot = action - 0x80;
		}
		if (slot == -1)
			slot = INV_SLOT_KNAPSACK;
		old_slot = -1;
		slot2 = old_slot2 = -1;
		if (g_pp20_index == 14 || g_pp20_index == 15)
			draw_slot(g_vga_buf, slot, 0xfd);
		update_marks();
		draw_slots(g_vga_buf);
	} else
		old_slot2 = -1;
	reset_cursor();
}

void InvObj::sel_next()
{
	signed short lo = !g_inv_show_body ? INV_SLOT_KNAPSACK : 0;
	signed short hi = !g_inv_show_body || slot2 == -1 ? 30 : 32;

	if (slot2 != -1) {
		if (slot2 == hi)
			slot2 = lo;
		else
			slot2++;
	} else if (slot == hi)
		slot = lo;
	else
		slot++;
}

void InvObj::sel_prev()
{
	signed short lo = !g_inv_show_body ? INV_SLOT_KNAPSACK : 0;
	signed short hi = !g_inv_show_body || slot2 == -1 ? 30 : 32;

	if (slot2 != -1) {
		if (slot2 == lo)
			slot2 = hi;
		else
			slot2--;
	} else if (slot == lo)
		slot = hi;
	else
		slot--;
}

