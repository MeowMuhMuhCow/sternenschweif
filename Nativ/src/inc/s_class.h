/* s_class.h - C++-Klassen, die modulübergreifend benutzt werden (Teil von schweif.h, nur fuer C++)
 *
 * Die Methoden sind normale (nicht-inline) Elementfunktionen; cobjfix ordnet sie ueber die Fixups den
 * Linknamen zu, die Namen der Klassen/Methoden sind also frei. In C-Modulen stattdessen die freien
 * Funktionen aus s_proto.h benutzen (gb_* usw.) - gleiche Bytes, der this-Zeiger ist das erste Argument.
 *
 * Klassen: Bmp (R42), NumIn (O197), Win/Dlg (O190/O205/O189), Cast (O233), ItemUse (O047), Nvf (O098), Pic (O269),
 * Inv (O008/O009/O094), File (O037), TempFile (O243), Font (O248), Stash (O238), Screen (O188), Init (SEG005),
 * Travel (O038), Shop (O226), Sel (O225).
 * Globale Objekte stehen in s_data.h als char-Felder (C-Module und freie Funktionen brauchen void far *); in C++
 * ueber einen Cast-Makro im Modul ansprechen, z. B. #define NVF_HEADS ((Nvf far *)g_nvf_heads).
 * Offene Kandidaten: O153 Listen (Kampf), O200/O201 Gruppe (Methoden fuer struct group stehen in s_struct.h).
 */
#ifndef S_CLASS_H
#define S_CLASS_H

/* Zeigertyp fuer Prototypen mit Bmp-Parameter: in C++ die Klasse, in C ein void-Zeiger (gleicher Code) */
#ifdef __cplusplus
class Bmp;
#define BMP_PTR Bmp far *
class Town;
#define TOWN_PTR Town far *		/* Ereignisobjekt einer Stadt (O183), Prototypen der Stadtereignisse (O249) */
class Travel;
#define TRAVEL_PTR Travel far *		/* Reiseobjekt (O038), Prototypen der Reisefunktionen (O038, O164, O209..O223) */
class Shop;
#define SHOP_PTR Shop far *		/* Ladenobjekt (O226), Prototypen der Ladenfunktionen (O226..O229) */
class Store;
#define STORE_PTR Store far *		/* Lagerhaus (O012), Prototypen der Lagerfunktionen (O012, O099, O100) */
class Peddler;
#define PEDDLER_PTR Peddler far *	/* fahrender Haendler (O069), Prototypen O068/O069 */
class Lodging;
#define LODGING_PTR Lodging far *	/* Rasthaus LOC_LODGING (O070) */
class Temple;
#define TEMPLE_PTR Temple far *		/* Tempel (O044), Prototypen O033/O044 */
class Healer;
#define HEALER_PTR Healer far *		/* Heiler (O174) */
class Inn;
#define INN_PTR Inn far *		/* Herberge (O178; O179) */
class Camp;
#define CAMP_PTR Camp far *		/* Lager/Rast (O071, Rastfunktionen O072; Kontext auch O067/O073) */

class Cast;
#define CAST_PTR Cast far *		/* Zauberobjekt (O233), Prototypen der Zauberroutinen spell_* (O233..O237) */
class Smith;
#define SMITH_PTR Smith far *		/* Schmiede (O230), Prototypen der Reparaturfunktionen (O231) */
class CharGen;
#define CHARGEN_PTR CharGen far *		/* Heldenerschaffung (O026), Prototypen O026/O167..O171 */
class SaveMenu;
#define SAVEMENU_PTR SaveMenu far *	/* Spielstandobjekt (O106/O107) */
class Save;
#define SAVE_PTR Save far *		/* DSA-1-Import (O176/O177) */
#else
#define CAST_PTR void far *
#define BMP_PTR void far *
#define TOWN_PTR void far *
#define TRAVEL_PTR struct travel far *	/* C-Module O211/O214: Sicht struct travel (unten) */
#define SHOP_PTR struct shop far *	/* C-Modul O229: Datenteil von class Shop (struct shop unten) */
#define STORE_PTR struct store far *
#define PEDDLER_PTR struct peddler far *
#define LODGING_PTR struct lodging far *
#define TEMPLE_PTR struct temple far *
#define HEALER_PTR struct healer far *
#define INN_PTR struct inn far *
#define CAMP_PTR struct camp far *

#define SMITH_PTR void far *
#define CHARGEN_PTR void far *
#define SAVEMENU_PTR void far *
#define SAVE_PTR void far *
/* C-Module: Bmp-Objekt als Struktur gleichen Aufbaus (fuer lokale Objekte, z. B. R11 compass_draw_strip) */
struct Bmp {
	signed short w;			/* +0 Breite */
	signed short h;			/* +2 Hoehe */
	signed char owns;		/* +4 Puffer selbst angelegt */
	signed char f5;			/* +5 */
	unsigned char far * far *lines;	/* +6 Zeilenzeiger */
};

/* C-Module (O015): Animationsobjekt als Struktur gleichen Aufbaus wie class Anim (Beschreibung dort), fuer lokale
 * Objekte mit den freien Funktionen Anim_* */
struct Anim {
	unsigned char far *pal;		/* +0x00 */
	signed char from_file;		/* +0x04 */
	signed short x;			/* +0x05 */
	signed short w;			/* +0x07 */
	unsigned char y;		/* +0x09 */
	unsigned char h;		/* +0x0a */
	void far *dst;			/* +0x0b */
	void far *data;			/* +0x0f */
	struct fig_frame far *frame;	/* +0x13 */
	signed short id;		/* +0x17 */
	signed short nr_seqs;		/* +0x19 */
	signed short seq;		/* +0x1b */
	signed short nr_frames;		/* +0x1d */
	signed short cur;		/* +0x1f */
	unsigned char rate;		/* +0x21 */
	unsigned char cue;		/* +0x22 */
	unsigned char loop;		/* +0x23 */
	unsigned char f24;		/* +0x24 */
	signed char _r25;
	unsigned char rounds;		/* +0x26 */
	struct Bmp bmp;			/* +0x27 */
	struct Bmp bg;			/* +0x31 */
	void far *bmp_buf;		/* +0x3b */
	void far *bg_buf;		/* +0x3f */
	signed short bmp_ext;		/* +0x43 */
	signed short bg_ext;		/* +0x45 */
	unsigned char cues[0x64];	/* +0x47 */
	signed long buf_size;		/* +0xab */
};

/* C-Module (O211, O214): Reiseobjekt als Struktur gleichen Aufbaus wie class Travel (Beschreibung dort) */
struct travel {
	signed char _r00[8];
	void far *saved_areas;		/* +0x008 */
	signed char shown[4];		/* +0x00c */
	signed char status[4];		/* +0x010 TST_* */
	signed short bg_x1;		/* +0x014 */
	signed short bg_x2;		/* +0x016 */
	signed short frame;		/* +0x018 */
	signed short anim_tick;		/* +0x01a */
	signed short anim_rate;		/* +0x01c */
	signed short anim_on;		/* +0x01e */
	signed char _r20[0x4c - 0x20];
	signed char dir_sel;		/* +0x04c */
	signed char dir_def;		/* +0x04d */
	signed short rose_x;		/* +0x04e */
	signed short rose_y;		/* +0x050 */
	signed short old_rose_x;	/* +0x052 */
	signed short old_rose_y;	/* +0x054 */
	struct dir_set place;		/* +0x056 */
	signed short evtxt_file;	/* +0x059 */
	signed short evtxt_n;		/* +0x05b */
	void far *rose_bg_buf;		/* +0x05d */
	void far *rose_buf;		/* +0x061 */
	void far *rose_pics[17];	/* +0x065 */
	struct Bmp rose_bg;		/* +0x0a9 */
	struct Bmp rose;		/* +0x0b3 */
	signed short sub_scene;		/* +0x0bd */
	signed char reenter;		/* +0x0bf */
	unsigned char from;		/* +0x0c0 */
	unsigned char to;		/* +0x0c1 */
	signed char prev_place;	/* +0x0c2 */
	signed char road;		/* +0x0c3 */
	signed char weather;		/* +0x0c4 */
	unsigned long speed;		/* +0x0c5 */
	signed char hurry;		/* +0x0c9 */
	unsigned char route_len;	/* +0x0ca */
	unsigned long route_pos;	/* +0x0cb */
	signed char terrain;		/* +0x0cf */
	unsigned char route[40];	/* +0x0d0 */
	signed char action;		/* +0x0f8 */
	signed char danger;		/* +0x0f9 */
	signed short far *points;	/* +0x0fa */
	signed short nr_points;		/* +0x0fe */
	signed char f100;		/* +0x100 */
	signed char resume_10;		/* +0x101 */
	signed char resume_50;		/* +0x102 */
	signed char resume_90;		/* +0x103 */
	signed char mode;		/* +0x104 */
	signed char rest;		/* +0x105 */
	signed short turn_pos;		/* +0x106 */
	signed char turns;		/* +0x108 */
	signed char f109;		/* +0x109 (O125 place_event_170 setzt 1 mit neuer Route) */
	signed short f10a;		/* +0x10a */
	signed char ev_half;		/* +0x10c */
	signed char ev_start;		/* +0x10d */
	signed char ev_end;		/* +0x10e */
	signed char planned;		/* +0x10f */
	signed short map_pts[6];	/* +0x110 */
};
#endif

/* --- Heldenbogen/Inventar (0x144 Bytes): Klasse Inv (Konstruktor/Methoden O008/O009, Zeichnen O094); als
 * Datenstruktur auch in C (O093, O095). Objekt g_8b90; aktueller Held zusaetzlich in g_sheet_hero. ----------- */
struct inv_sheet {
	void far *txt_buf;		/* +0x000 Textpuffer (txt_load_list Datei 0x1c) */
	char far * far *txt;		/* +0x004 Texte */
	signed char page;		/* +0x008 Seite 1..5 (1 Werte, 2 Kampf, 3 Talente, 4/5 Zauber) */
	signed short done;		/* +0x009 Ende */
	signed short redraw;		/* +0x00b neu zeichnen */
	signed short sel;		/* +0x00d Menuewahl (O093) */
	signed short f0f;		/* +0x00f (O094: Breite/Hoehe von nvfile_load, unbenutzt) */
	signed short cursor;		/* +0x011 Mauszeigerbild (inv_set_cursor), -1 = keins */
	struct hero far *hero;		/* +0x013 angezeigter Held */
	struct hero far *owner;		/* +0x017 Held, dessen Gegenstand gewaehlt ist (Tausch) */
	signed char _r1b[4];
	signed short ill;		/* +0x01f O093: erste Krankheit */
	signed short poi;		/* +0x021 O093: erstes Gift */
	signed short nr_entries;	/* +0x023 O093: Zahl der Menuepunkte */
	signed short swap;		/* +0x025 1 = Tausch mit einem anderen Helden laeuft */
	signed char f27;		/* +0x027 */
	signed short slot;		/* +0x028 gewaehlter Inventarplatz, -1 = keiner */
	signed short slot2;		/* +0x02a Zielplatz beim Verschieben, -1 = keiner */
	signed short old_slot;		/* +0x02c zuletzt markierter slot */
	signed short old_slot2;		/* +0x02e zuletzt markierter slot2 */
	unsigned char marks[0x1f];	/* +0x030 Markierungen je Platz (O008) */
	signed char nvf[0x19];		/* +0x04f NVF-Objekt (Klasse Nvf, O098) */
	signed char pic[0xa6];		/* +0x068 Bildobjekt (Klasse Pic, O269) */
	BMP_PTR screen;			/* +0x10e Bildschirm-Bmp */
	BMP_PTR b112;			/* +0x112 */
	BMP_PTR item_bmp;		/* +0x116 Gegenstandsbild 16 x 16 (O094) */
	BMP_PTR body;			/* +0x11a Koerperbild (O094, Bild 0x10) */
	BMP_PTR b11e;			/* +0x11e */
	BMP_PTR b122;			/* +0x122 */
	BMP_PTR b126;			/* +0x126 */
	BMP_PTR b12a;			/* +0x12a */
	BMP_PTR knapsack;		/* +0x12e Rucksackbild (O094) */
	BMP_PTR status_bg;		/* +0x132 Hintergrund der Statuszeile y = 0xc0 */
	BMP_PTR b136;			/* +0x136 */
	BMP_PTR b13a;			/* +0x13a */
	void far *buf;			/* +0x13e Bildpuffer 0xfa00 */
	signed short lvl_auto;		/* +0x142 Stufenanstieg automatisch (O086, class Lvl) */
};

#ifdef __cplusplus

/* --- Bmp: Bildpuffer (10 Bytes), Code in SEG042 (C, Funktionen gb_*) ---------------------------------- */
class Bmp {
public:
	signed short w;			/* +0 Breite */
	signed short h;			/* +2 Hoehe */
	signed char owns;		/* +4 Puffer selbst angelegt */
	signed char f5;			/* +5 */
	unsigned char far * far *lines;	/* +6 Zeilenzeiger */

	Bmp();						/* R42_0008 gb_new0 */
	Bmp(signed short w, signed short h, void far *p); /* R42_004B gb_new (fremder Puffer) */
	Bmp(signed short w, signed short h);		/* R42_00D8 gb_new_alloc (eigener Puffer) */
	~Bmp();						/* R42_03B3 gb_delete */
	void init(signed short w, signed short h, void far *p); /* R42_0414 gb_init */
	void free_lines();				/* R42_0480 gb_free_lines */
	void put(void far *dst, signed short x, signed short y); /* R42_04A9 gb_blit */
	void put(void far *dst, signed short x, signed short y, signed short mode); /* R42_04DF gb_blit_m */
	void put_part(void far *dst, signed short x, signed short y, signed short w, signed short h); /* R42_0517 */
	void put_part(void far *dst, signed short x, signed short y, signed short w, signed short h,
		signed short mode);			/* R42_054C gb_blit_whm */
	void put_part(void far *dst, signed short x, signed short y, signed short w, signed short h,
		signed short mode, signed short unused, signed short back);	/* R42_05BB gb_blit_ab (back: rueckwaerts) */
	void get(void far *src, signed short x, signed short y); /* R42_05F4 gb_save */
	void set_base(void far *p);			/* R42_0653 gb_set_base */
	void load(void far *src);			/* R42_0699 gb_load_mem */
	void far *data();				/* R42_072D gb_data */
	void scale(Bmp far *dst);			/* R42_0740 gb_scale: auf die Groesse von dst skalieren */
	void clear();					/* R42_078A gb_clear */
	void fill(signed short c);			/* R42_079D gb_fill */
	void size(signed short far *w, signed short far *h); /* R42_07DA gb_get_size */
	void over(Bmp far *b, signed short x, signed short y, signed short mode); /* R42_08CC gb_put */
};


/* --- NumIn: Zahleneingabe mit Ziffernfeldern, Pfeilen und OK-Knopf (O197, 0x35 Bytes; lokal in Win::num_box) ------- */
class NumIn {
public:
	signed short active;		/* +0x00 */
	signed short x;			/* +0x02 Lage */
	signed short y;			/* +0x04 */
	signed short step;		/* +0x06 Abstand der Ziffern */
	signed short ndig;		/* +0x08 Stellen (hoechstens 5) */
	signed short f0a;		/* +0x0a */
	signed short cur;		/* +0x0c Cursorstelle */
	signed char dig[5];		/* +0x0e Ziffern */
	signed short hi;		/* +0x13 Grenzen (Konstruktor min/max) */
	signed short lo;		/* +0x15 */
	Bmp far *field;			/* +0x17 Ziffernfeld 13 x 16 */
	Bmp far *ok;			/* +0x1b OK-Knopf 34 x 16 */
	Bmp far *ok_down;		/* +0x1f */
	Bmp far *up;			/* +0x23 Pfeile 13 x 7 */
	Bmp far *down;			/* +0x27 */
	Bmp far *up_down;		/* +0x2b */
	Bmp far *down_down;		/* +0x2f */
	signed short redraw;		/* +0x33 alle Ziffernfelder neu zeichnen (draw_digits setzt 0 zurueck) */

	NumIn(signed short n, void far *gfx, signed short min, signed short max, signed short val);	/* O197_0000 */
	~NumIn();					/* O197_0391 */
	signed long value();				/* O197_044C Wert aus den Ziffern */
	signed short get_x();				/* O197_049E */
	signed short get_y();				/* O197_04AC */
	void set_x(signed short v);			/* O197_04BA */
	void set_y(signed short v);			/* O197_04C9 */
	void draw_frame();				/* O197_04D8 */
	void draw_digits();				/* O197_0601 */
	signed short run(signed short x, signed short y);	/* O197_0712 Tastenschleife (Dlg::num_loop) */
	void set_cur(signed short v) { cur = v; }	/* O190 num_loop */
};

/* --- Input: einzeiliges Texteingabefeld (O180, 0x28 Bytes; lokal in Win::text_box, Win::wait verschiebt es mit) --- */
class Input {
public:
	signed short active;		/* +0x00 Eingabe laeuft */
	signed short x;			/* +0x02 Lage */
	signed short y;			/* +0x04 */
	signed short char_w;		/* +0x06 Zeichenbreite ('_' in g_font) */
	signed short max;		/* +0x08 hoechstens max Zeichen */
	signed short mode;		/* +0x0a Zeichensatz (run) */
	signed short f0c;		/* +0x0c */
	signed short len;		/* +0x0e Laenge */
	signed short preset_len;	/* +0x10 Laenge des Vorgabetexts (run: strlen(s)) */
	char far *save;			/* +0x12 Hintergrund je Zeichen (char_w * 8 * max) */
	char far *str;			/* +0x16 Eingabetext */
	char far *pos;			/* +0x1a Schreibposition */
	Bmp bmp;			/* +0x1e Hintergrundbild eines Zeichens */

	Input(signed short n);				/* O180_0000 Input_ctor */
	~Input();					/* O180_00B9 Input_dtor */
	signed short get_x();				/* O180_0117 */
	signed short get_y();				/* O180_0125 */
	void set_x(signed short v);			/* O180_0133 */
	void set_y(signed short v);			/* O180_0142 */
	void save_bg();					/* O180_0151 input_save_bg: Hintergrund je Zeichen sichern */
	void draw_init();				/* O180_01D2 input_draw_init: Vorgabetext (g_input_preset) bzw. '_' */
	void redraw();					/* O180_028C neu zeichnen (nach dem Verschieben) */
	void restore_rest();				/* O180_0348 input_restore_rest: Rest des Felds wiederherstellen */
	void backspace();				/* O180_03E0 input_backspace: Zeichen links loeschen */
	void put_char(signed char c);			/* O180_04D9 input_put_char: Zeichen ausgeben */
	signed short run(char far *s, signed short x, signed short y, signed short mode);	/* O180_0583 Eingabeschleife */
};

/* --- Win: Fenster (Basis des Dialogobjekts, 0x46 Bytes; Konstruktor/Destruktor O190, Fensterfunktionen O205) --------
 * Breite w = tiles * 32 + 64; Rahmen aus Teilbildern der Datei 3 (frame_gfx); Hintergrund per XMS gesichert. */
class Win {
public:
	void far *frame_gfx;		/* +0x00 Rahmenteile (NVF, Datei 3 aus g_file_main) */
	signed char _r04[4];
	char far *text;			/* +0x08 auszugebender Text */
	char far *text_buf;		/* +0x0c Textpuffer 0x400 Bytes */
	signed short sel;		/* +0x10 vorgewaehlte Zeile der Auswahl, -1 = keine (Farbe 0xf9) */
	signed long xms;		/* +0x12 XMS-Block fuer den Hintergrund (Handle im unteren Wort) */
	signed short tiles;		/* +0x16 Breite in 32-Pixel-Teilen */
	signed short x;			/* +0x18 Lage */
	signed short y;			/* +0x1a */
	signed short new_x;		/* +0x1c Lage nach dem Verschieben (drag) */
	signed short new_y;		/* +0x1e */
	signed short is_open;		/* +0x20 Fenster offen (overlaps prueft es) */
	signed short dragging;		/* +0x22 wird gerade verschoben */
	signed short no_move;		/* +0x24 nicht verschiebbar (lock_move) */
	signed short pos_x;		/* +0x26 Versatz zur Bildmitte (set_pos_x), 0 = Maus */
	signed short pos_y;		/* +0x28 */
	signed short w;			/* +0x2a Breite */
	signed short lines;		/* +0x2c Textzeilen */
	signed short h;			/* +0x2e Hoehe */
	signed short old_fg;		/* +0x30 Schriftfarben vor dem Oeffnen */
	signed short old_bg;		/* +0x32 */
	signed short old_clock;	/* +0x34 g_clock_update vor dem Oeffnen */
	signed short old_b76d;		/* +0x36 g_status_on vor dem Oeffnen */
	signed short no_timeout;	/* +0x38 Meldung ohne Zeitlimit (set_no_timeout) */
	signed short old_box[4];	/* +0x3a Schriftkasten vor dem Oeffnen (font_get_box) */
	signed short f42;		/* +0x42 */
	signed short old_mouse_cb;	/* +0x44 g_mouse_handler_on vor dem Oeffnen */

	Win();						/* O190_08B1 Win_ctor: Rahmenteile laden, Textpuffer, XMS */
	~Win();						/* O190_0986 Win_dtor */
	void save_bg();					/* O205_0000 win_save_bg: Hintergrund ins XMS */
	void restore_bg();				/* O205_0087 win_restore_bg */
	void draw_frame(signed short lines);		/* O205_01C5 win_draw_frame */
	void draw_frame_row(signed short row, signed short type, signed short h);	/* O205_0270 win_draw_frame_row */
	signed short calc_x(signed short w);		/* O205_04F4 win_calc_x */
	signed short calc_y(signed short h);		/* O205_0559 win_calc_y */
	void save_border(signed short x, signed short y, signed short w, signed short h, char far *buf);	/* O205_05C3 */
	void restore_border(signed short x, signed short y, signed short w, signed short h, char far *buf);	/* O205_0634 */
	void drag(signed short x, signed short y, signed short w, signed short h);	/* O205_06DA win_drag */
	void poll_input();				/* O205_0A37 win_poll_input */
	signed short wait(Input far *in);		/* O205_0A4D win_wait: Warteschleife (Zeitlimit, Verschieben) */
	void set_no_timeout();				/* O205_0D27 win_set_no_timeout */
	void close();					/* O205_0D35 win_close */
	void prepare(char far *s, signed short x, signed short y);	/* O205_0DDB win_prepare */
	signed short text_box(signed short n, char far *s);	/* O205_0F3D win_text_box: Meldung bzw. Texteingabe (n Zeichen) */
	signed short num_box(signed short n, char far *s, signed short min, signed short max, signed short val);
							/* O205_11DD win_num_box: Zahleneingabe (NumIn) */
	/* Bildauswahl (O172): n Bilder ab Nummer start (g_nvf_heads Typ 5), Rueckgabe start + Auswahl bzw. -1 */
	void draw_pics(signed short n, signed short start);		/* O172_0000 win_draw_pics */
	signed short select_pic_run(signed short n, signed short start);	/* O172_0123 win_select_pic_run */
	signed short select_pic(signed short n, signed short start);	/* O172_0250 dlg_select_pic */
	signed short select_pic_loop(signed short n, signed short start);	/* O172_026A win_select_pic_loop */
};

/* --- Dlg: Dialogfenster (0x9b Bytes), Objekt g_dialog ds:b59a; Code in O190, Auswahl in O189 -------------------------
 * Freie Funktionen dlg_* in s_proto.h (Objekt als erstes Argument). */
class Dlg : public Win {
public:
	signed short f46;		/* +0x46 (open_dummy setzt 1) */
	signed short yes_no;		/* +0x48 Ja/Nein-Abfrage (O189: Tasten j/n) */
	signed short text_x;		/* +0x4a Textversatz (neben dem Bild) */
	signed short text_y;		/* +0x4c Textversatz in Zeilen */
	signed short cur_sel;		/* +0x4e markierte Zeile (O189) */
	signed short old_sel;		/* +0x50 */
	signed short no_drag;		/* +0x52 Verschieben gesperrt (lock) */
	signed short choice_lines;	/* +0x54 Zeilen der Auswahltexte */
	signed short total_lines;	/* +0x56 */
	signed short mark_y;		/* +0x58 y der markierten Zeile, -1 = keine */
	char far *mark_text;		/* +0x5a Text der markierten Zeile */
	char far *new_text;		/* +0x5e Text der neu markierten Zeile */
	void far *pic;			/* +0x62 Rohbild (32 x 32), 0 = keins */
	char far *title;		/* +0x66 Titel */
	char far *body;			/* +0x6a Text */
	signed short n_choices;		/* +0x6e Zahl der Auswahltexte */
	unsigned char far *line_offs;	/* +0x70 y-Versatz je Auswahlzeile (n + 1, new) */
	Bmp far *bmp;			/* +0x74 Bild als Bmp */
	signed short pic_w;		/* +0x78 Bildgroesse */
	signed short pic_h;		/* +0x7a */
	signed short f7c;		/* +0x7c (O189: 0x11) */
	signed short f7e;		/* +0x7e (O189: 0x0b) */
	signed short old_tiles;	/* +0x80 Breite vor dem Titel */
	signed short old_tab;		/* +0x82 Tabulator (g_381f) */
	unsigned char pic_frame;	/* +0x84 Rahmen um das Bmp-Bild */
	void far *anim;			/* +0x85 Animation (O051) statt Bild */
	void far *anim_buf;		/* +0x89 */
	Bmp anim_bmp;			/* +0x8d Hintergrund der Animation */
	void far *choices;		/* +0x97 va_list der Auswahltexte */

	Dlg();						/* O190_0000 Dlg_ctor */
	signed short yes_no_box(char far *s);		/* O190_0082 dlg_yes_no: 1 ja, -1 nein/Abbruch, 0 */
	signed short message(char far *s);		/* O190_00E8 dlg_message */
	signed short input(signed short ndig, char far *s, signed short min, signed short max, signed short val);
							/* O190_0142 dlg_input: Zahleneingabe */
	signed short choose(char far *s, signed short n, ...);	/* O190_01C6 dlg_choose: Auswahl unter n Texten */
	signed short box(void far *pic, char far *title, char far *s, signed short n, ...);	/* O190_0237 dlg_box */
	signed short box_anim(void far *anim, char far *title, char far *s, signed short n, ...);	/* O190_02C2 dlg_box_anim */
	signed short box_bmp(Bmp far *bmp, char far *title, char far *s, signed short n, ...);	/* O190_037F dlg_box_bmp */
	signed short lock();				/* O190_0435 dlg_lock: nicht verschiebbar, alter Wert */
	void unlock();					/* O190_0463 dlg_unlock */
	void draw_pic();				/* O190_047F dlg_draw_pic */
	void print_title();				/* O190_05FC dlg_print_title */
	void print_body();				/* O190_06FA dlg_print_body */
	void print_choices();				/* O190_074A dlg_print_choices: Zeilen, line_offs */
	signed short input_text(signed short n, char far *s);	/* O190_0852 dlg_message_flag: Meldung/Texteingabe */
	signed short lock_move();			/* O190_09DF dlg_lock_move: alter Wert */
	void unlock_move();				/* O190_09FF dlg_unlock_move */
	signed short set_pos_x(signed short v);		/* O190_0A0D dlg_set_pos_x: alter Wert */
	signed short set_pos_y(signed short v);		/* O190_0A2E dlg_set_pos_y */
	signed short set_width(signed short tiles);	/* O190_0A4F dlg_set16: Breite in 32er-Teilen, alter Wert */
	signed short num_loop(NumIn far *n);		/* O190_0A70 dlg_num_loop: Tasten-/Mausschleife der Zahleneingabe */
	signed short overlaps(signed short x0, signed short y0, signed short x1, signed short y1);	/* O190_0EA5 dlg_overlaps */
	void fill_area(void far *dst);			/* O190_0F28 dlg_fill_area: Inneres und Ecken mit 0xff */
	void dummy();					/* O190_119F dlg_dummy */
	void set_open();				/* O190_11A4 dlg_set_open: is_open = f46 = 1 */
	/* Auswahl/Meldung, Code in O189 */
	signed short count_lines(signed short n, void far *ap);	/* O189_0000 dlg_count_lines */
	void draw_choices(signed short n, void far *ap);	/* O189_00C4 dlg_draw_choices: Rahmen und Knoepfe */
	void draw_button(signed short y, signed short nr);	/* O189_0269 dlg_draw_button */
	void mark_line(signed short y);			/* O189_033A dlg_mark_line */
	signed short choose_loop(signed short n, void far *ap);	/* O189_0449 dlg_choose_loop */
	void click_sound();				/* O189_0B6A dlg_click_sound */
	signed short run(void far *pic, char far *title, char far *s, signed short far *pn, void far *ap);
							/* O189_0B7D dlg_run: Aufbau, Auswahl, Aufraeumen */
	void layout();					/* O189_103B dlg_layout */
	void wait_message();				/* O189_119F dlg_wait_message */
	void calc_text();				/* O189_149A dlg_calc_text */
};

/* --- Cast: Zaubern eines Helden (O233, 0x22 Bytes; lokal in use_spell O091, ItemUse O047, O013) ----------------
 * cast(id, mod) holt das Ziel (target_type), prueft und ruft die Zauberroutine g_spell_handlers[id] (Methoden-
 * zeiger, 8 Bytes) auf: Cast-Methoden spell_* in O233 (Zauber 1..8), freie Funktionen spell_*(CAST_PTR) in O234
 * (9..26), O235 (27..48), O236 (49..71) und O237 (72..85) - gleicher Code, this = erstes Argument.
 * Die Zauberroutine meldet in result: SPELL_RES_DEFAULT (Grundkosten), SPELL_RES_FAIL (misslungen, Kosten
 * fail_cost bzw. halbe Grundkosten), SPELL_RES_ABORT (ohne Kosten), SPELL_RES_NONE (ohne Wirkung, ohne Kosten)
 * oder die AE-Kosten (> 0). Text der Meldung in g_text_output_buf. */
class Cast;
typedef void (Cast::*CAST_PMF)();		/* Methodenzeiger, 8 Bytes */

class Cast {
public:
	void far *txt_buf;		/* +0x00 Zaubertexte (Datei 0x1f, 300 Eintraege; Schnellkampf: g_b690) */
	char far * far *txt;		/* +0x04 */
	signed short result;		/* +0x08 Ergebnis/AE-Kosten (SPELL_RES_*, > 0 Kosten) */
	signed short fail_cost;		/* +0x0a AE-Kosten bei SPELL_RES_FAIL (0: halbe Grundkosten) */
	signed char mod;		/* +0x0c Probenzuschlag (+5 gegen Sondergegner) */
	signed char target_kind;	/* +0x0d Zielart TARGET_* (target_type(caster)) */
	CAST_PMF func;			/* +0x0e Zauberroutine (g_spell_handlers[id]) */
	struct hero far *caster;	/* +0x16 Zaubernder */
	struct hero far *target;	/* +0x1a Ziel-Held (TARGET_HERO/TARGET_SELF) */
	struct enemy far *enemy;	/* +0x1e Ziel-Gegner (TARGET_ENEMY/TARGET_SPECIAL) */

	Cast(struct hero far *h);			/* O233_00DC Cast_ctor */
	~Cast();					/* O233_019A Cast_dtor */
	void heal_ani();				/* O233_01EE cast_heal_ani (BE magic_heal_ani) */
	signed short damage(signed short le);		/* O233_0343 cast_damage (BE FIG_do_spell_damage) */
	signed short target_pa();			/* O233_048F cast_target_pa (BE get_attackee_parade) */
	signed short target_rs();			/* O233_04D5 cast_target_rs (BE get_attackee_rs) */
	signed short cast(signed short id, signed short mod);	/* O233_0517 cast_spell (BE use_spell, Kern) */
	/* Zauberroutinen in O233 (Antimagie, Beherrschung) */
	void spell_beherrschung_brechen();		/* O233_0B4B  1 */
	void spell_destructibo();			/* O233_0CB5  2 */
	void spell_gardianum();				/* O233_0EAD  3 */
	void spell_illusionen_zerstoeren();		/* O233_100C  4 */
	void spell_verwandlung_beenden();		/* O233_10F8  5 */
	void spell_band_und_fessel();			/* O233_12E2  6 */
	void spell_bannbaladin();			/* O233_1439  7 */
	void spell_boeser_blick();			/* O233_1599  8 */
	/* Zauberroutinen in O234 (dort als freie Funktionen mit CAST_PTR definiert; hier fuer g_spell_handlers) */
	void spell_grosse_gier();                      /* O234_0000  9 */
	void spell_grosse_verwirrung();                /* O234_0101 10 */
	void spell_herr_ueber_das_tierreich();         /* O234_01F5 11 */
	void spell_horriphobus();                      /* O234_02E6 12 */
	void spell_magischer_raub();                   /* O234_041A 13 */
	void spell_respondami();                       /* O234_064E 14 */
	void spell_sanftmut();                         /* O234_070F 15 */
	void spell_somnigravis();                      /* O234_0721 16 */
	void spell_zwingtanz();                        /* O234_0880 17 */
	void spell_furor_blut();                       /* O234_092C 18 */
	void spell_geister_austreiben();               /* O234_0C6B 19 */
	void spell_geister_beschwoeren();              /* O234_0CCC 20 */
	void spell_heptagon();                         /* O234_0D1D 21 */
	void spell_kraehenruf();                       /* O234_1146 22 */
	void spell_skelettarius();                     /* O234_123C 23 */
	void spell_elementare_beschwoeren();           /* O234_12ED 24 */
	void spell_nihilatio_gravitas();               /* O234_14BA 25 */
	void spell_solidirid();                        /* O234_162F 26 */
	/* Zauberroutinen in O235 (dort als freie Funktionen mit CAST_PTR definiert; hier fuer g_spell_handlers) */
	void spell_axxeleratus();                      /* O235_0000 27 */
	void spell_foramen();                          /* O235_0192 28 */
	void spell_motoricus();                        /* O235_01DC 29 */
	void spell_spurlos();                          /* O235_023A 30 */
	void spell_transversalis();                    /* O235_0276 31 */
	void spell_ueber_eis();                        /* O235_0438 32 */
	void spell_balsam();                           /* O235_05FF 33 */
	void spell_hexenspeichel();                    /* O235_0902 34 */
	void spell_klarum_purum();                     /* O235_0B1E 35 */
	void spell_ruhe_koerper();                     /* O235_0C33 36 */
	void spell_tiere_besprechen();                 /* O235_0C75 37 */
	void spell_adleraug();                         /* O235_0DB1 38 */
	void spell_analues();                          /* O235_0E1F 39 */
	void spell_eigenschaften();                    /* O235_1139 40 */
	void spell_exposami();                         /* O235_1254 41 */
	void spell_odem_arcanum();                     /* O235_1269 42 */
	void spell_penetrizzel();                      /* O235_1434 43 */
	void spell_sensibar();                         /* O235_1481 44 */
	void spell_chamaelioni();                      /* O235_1542 45 */
	void spell_duplicatus();                       /* O235_159F 46 */
	void spell_harmlose_gestalt();                 /* O235_1694 47 */
	void spell_hexenknoten();                      /* O235_177A 48 */
	/* Zauberroutinen in O236 (dort als freie Funktionen mit CAST_PTR definiert; hier fuer g_spell_handlers) */
	void spell_blitz();                            /* O236_0000 49 */
	void spell_ecliptifactus();                    /* O236_00FF 50 */
	void spell_eisenrost();                        /* O236_02B5 51 */
	void spell_fulminictus();                      /* O236_04F4 52 */
	void spell_ignifaxius();                       /* O236_05B6 53 */
	void spell_plumbumbarum();                     /* O236_09F8 54 */
	void spell_radau();                            /* O236_0BA5 55 */
	void spell_saft_kraft();                       /* O236_0C5E 56 */
	void spell_scharfes_auge();                    /* O236_0E4C 57 */
	void spell_hexenblick();                       /* O236_0F68 58 */
	void spell_nekropathia();                      /* O236_0FB9 59 */
	void spell_adler_wolf();                       /* O236_10B7 60 */
	void spell_arcano_psychostabilis();            /* O236_119D 61 */
	void spell_armatrutz();                        /* O236_11E5 62 */
	void spell_ch_steigern();                      /* O236_1485 63 */
	void spell_feuerbann();                        /* O236_1499 64 */
	void spell_ff_steigern();                      /* O236_1565 65 */
	void spell_ge_steigern();                      /* O236_1579 66 */
	void spell_in_steigern();                      /* O236_158D 67 */
	void spell_kk_steigern();                      /* O236_15A1 68 */
	void spell_kl_steigern();                      /* O236_15B5 69 */
	void spell_mu_steigern();                      /* O236_15C9 70 */
	void spell_mutabili();                         /* O236_15DD 71 */
	/* Zauberroutinen in O237 (dort als freie Funktionen mit CAST_PTR definiert; hier fuer g_spell_handlers) */
	void spell_paralue();                          /* O237_0000 72 */
	void spell_salander();                         /* O237_00FE 73 */
	void spell_in_see_und_fluss();                 /* O237_01D2 74 */
	void spell_visibili();                         /* O237_038E 75 */
	void spell_abvenenum();                        /* O237_0581 76 */
	void spell_aeolitus();                         /* O237_064A 77 */
	void spell_brenne_toter_stoff();               /* O237_0686 78 */
	void spell_claudibus();                        /* O237_09DB 79 */
	void spell_dunkelheit();                       /* O237_0A0E 80 */
	void spell_weiches_erstarre();                 /* O237_0AA0 81 */
	void spell_flim_flam();                        /* O237_0AF1 82 */
	void spell_hartes_schmelze();                  /* O237_0B83 83 */
	void spell_silentium();                        /* O237_0D79 84 */
	void spell_sturmgebruell();                    /* O237_0E40 85 */
};

extern CAST_PMF g_spell_handlers[];	/* ds:1bd0 Zauberroutine je Zauber SP_* (86; [0] = 0) - kein eigenes Public */


/* --- Lvl: Stufenanstieg und Steigern (Code O086 und O087) - Methoden auf dem Heldenbogen-Objekt (struct inv_sheet,
 * 0x144 Bytes, Aufrufer O005 hero_sheet_loop: Lvl_run(sheet, h)). Felder aus Sicht von O086: page = Modus (1 Eigen-
 * schaften, 3 Talente, 4 Zauber), sel = Auswahl, hero = aufsteigender Held, slot/slot2 = -1, lvl_auto = automatisch. */
class Lvl : public inv_sheet {
public:
	void redraw();					/* O086_0000 lvl_redraw: Bogen neu (Inv_g_09bb), Balken */
	void inc_attrib();				/* O086_0027 lvl_inc_attrib: positive Eigenschaft +1 */
	void dec_attrib();				/* O086_0219 lvl_dec_attrib: negative Eigenschaft -1 (3 Wuerfe) */
	void inc_le();					/* O086_04F1 lvl_inc_le: W6 LE (Zauberkundige + 2, Aufteilung LE/AE) */
	void inc_mr();					/* O086_070E lvl_inc_mr */
	void auto_skills();				/* O086_073C lvl_auto_skills: Talente nach Vorlage g_auto_skills */
	void auto_spells();				/* O086_0857 lvl_auto_spells: Zauber nach Typlisten */
	void inc_points();				/* O086_0E24 lvl_inc_points: Steigerungen verteilen */
	void run(struct hero far *h);			/* O086_1307 Lvl_run */
	void inc_spell(signed short id);		/* O087_0000 lvl_inc_spell (BE inc_spell_advanced) */
	void inc_skill(signed short id);		/* O087_0255 lvl_inc_skill (BE inc_skill_advanced) */
	void inc_skill_auto(signed short id);		/* O087_0405 lvl_inc_skill_auto (BE inc_skill_novice) */
	void inc_spell_auto(signed short id);		/* O087_054C lvl_inc_spell_auto (BE inc_spell_novice) */
};

/* --- Lock: Schloss/Tuer (O108, 0x29 Bytes; lokal in O262/O265/O268): +0..+7 = Datensatz nr der Datei 0x2c8 ---- */
class Lock;
typedef signed short (Lock::*LOCK_PMF)();	/* Methodenzeiger, 8 Bytes */

class Lock {
public:
	unsigned char type;		/* +0x00 Schlossart (Index g_lock_types) */
	signed char _r01;
	signed short key;		/* +0x02 Schluessel (Gegenstand), -1 = keiner */
	signed char special;		/* +0x04 Sonderbehandlung statt Auswahl (Index g_lock_special), -1 = keine */
	signed char fail_special;	/* +0x05 Folge eines Fehlschlags (Index g_lock_fail, z. B. Falle), -1 = keine */
	unsigned char state;		/* +0x06 LOCK_* */
	unsigned char tries;		/* +0x07 Versuche (ab 10 Hinweis; Fallenprobe -tries) */
	signed short nr;		/* +0x08 Satznummer */
	signed char force;		/* +0x0a noetige KK zum Aufbrechen */
	signed char pick_mod;		/* +0x0b Zuschlag Schloesserknacken */
	signed char spell_mod;		/* +0x0c Zuschlag Foramen/Claudibus */
	LOCK_PMF special_func;		/* +0x0d Sonderbehandlung (g_lock_special_default bzw. [special]) */
	LOCK_PMF fail_func;		/* +0x15 Fehlschlag (g_lock_fail_default bzw. [fail_special]) */
	void far *txt_buf;		/* +0x1d Texte 0x2c9 */
	char far * far *txt;		/* +0x21 */
	struct hero far *hero;		/* +0x25 handelnder Held */

	Lock(signed short nr);		/* O108_0000 Lock_ctor: Satz lesen, oeffnen versuchen, Falle */
	~Lock();			/* O108_02CB Lock_dtor: Satz zurueckschreiben */
	void init();			/* O108_0373 lock_init */
	signed short try_open();	/* O108_048D lock_try_open: 1 offen, 0 misslungen, -1 abgebrochen */
	signed short pick();		/* O108_05D5 lock_pick: Dietrich, Schloesser-Probe */
	signed short claudibus();	/* O108_06D1 lock_claudibus */
	signed short foramen();		/* O108_0866 lock_foramen */
	signed short force_open();	/* O108_09A8 lock_force_open: Aufbrechen (KK) */
	signed short use_key();		/* O108_0A6B lock_use_key */
	signed short trap();		/* O108_0A9F lock_trap: Falle (g_lock_fail[0]) */
};

extern LOCK_PMF g_lock_special[];	/* ds:6260 Sonderbehandlungen (nur [0] = 0) */
extern LOCK_PMF g_lock_fail[];		/* ds:6268 Fehlschlag-Folgen ([0] = lock_trap) */
extern LOCK_PMF g_lock_special_default;	/* ds:6278 (0) */
extern LOCK_PMF g_lock_fail_default;	/* ds:6280 (0) */

/* --- ItemUse: Gegenstand benutzen (0x22 Bytes), Code in O047 (und O048: Methoden der Tabelle g_item_use_funcs) --- */
class ItemUse;
typedef void (ItemUse::*ITEMUSE_PMF)();		/* Methodenzeiger, 8 Bytes */

class ItemUse {
public:
	void far *txt_buf;		/* +0x00 Texte 0x21 (txt_load_list) */
	char far * far *txt;		/* +0x04 */
	signed short verbose;		/* +0x08 Meldungen ausgeben */
	ITEMUSE_PMF func;		/* +0x0a gewaehlte Methode (use) */
	struct hero far *user;		/* +0x12 */
	struct hero far *target;	/* +0x16 */
	signed short slot;		/* +0x1a Inventarplatz */
	signed short id;		/* +0x1c Gegenstand */
	signed short slot2;		/* +0x1e zweiter Platz (combine), -1 */
	signed short id2;		/* +0x20 zweiter Gegenstand, -1 */

	ItemUse(struct hero far *user, struct hero far *target, signed short slot, signed short id,
		signed short verbose);				/* O047_0000 */
	ItemUse(struct hero far *user, signed short slot, signed short id, signed short slot2, signed short id2,
		signed short verbose);				/* O047_0091 */
	~ItemUse();						/* O047_0124 */
	void use();						/* O047_016E */
	void combine();						/* O047_0744 */
	void use_arcano();					/* O047_0AAD Arcano Psychostabilis (SP 61, bestaetigt N13) */
	void read_text();					/* O047_0B50 */
	void read_special();					/* O047_0D60 */
	void use_armatrutz();					/* O047_10B0 Armatrutz (SP 62) */
	void use_flim_flam();					/* O047_118E Flim Flam (SP 82) */
	void use_with_item();					/* O047_1213 */
	void use_ignifaxius();					/* O047_1391 Ignifaxius (SP 53) */
	void poison_weapon();					/* O047_1451 */
	signed short combo_index(signed short a, signed short b);	/* O048_0704 */
	/* Welle N15: weitere Methoden der Tabelle g_item_use_funcs (O048, O252..O254) */
	void use_keyring();					/* O048_0000 Schluesselbund 0x145 */
	void use_quest_items();					/* O048_0468 0x146 mit 0x14e (vermutet: Questabschluss) */
	void maintain_weapons();				/* O048_0581 Bruchfaktor der Waffen senken (vermutet) */
	signed short in_recipe_book(signed short item);	/* O048_0760 Rezept schon im Rezeptbuch 0xbf? */
	signed short on_keyring(signed short item);		/* O048_07ED schon im Schluesselbund? */
	void quest_anim();					/* O048_08E3 Zwischensequenz 0x0c */
	void use_attack_item();					/* O252_0000 Goldleim/Gifte/Hylailer Feuer auf einen Helden */
	void play_instrument();					/* O252_02D4 Harfe/Laute/Floete */
	void read_heptagon_book();				/* O252_0339 Heptagon + 2 (vermutet: Buch) */
	void use_light();					/* O252_0382 Fackel/Laterne/Oel/Zunder */
	void show_hidden_enemies();				/* O252_0960 im Kampf: kommende Gegner nennen */
	void write_recipe();					/* O252_09D0 Rezept ins Rezeptbuch abschreiben */
	void read_recipe_book();				/* O252_0F97 Rezeptbuch lesen */
	void show_note();					/* O252_1154 Meldung txt[71] */
	void consume();						/* O253_0000 item_consume: Nahrung/Getraenk/Kraut/Trank */
	void herb_effect();					/* O254_004B Wirkung eines Krauts */
	void potion_effect();					/* O254_06CA Wirkung eines Tranks/Gifts */
	void dec_user_inv_bf(signed short i) { if ((user->inv[i].bf == -99 ? 0 : 1) && user->inv[i].bf > 0) user->inv[i].bf--; }
								/* O048 (Parameterkopie, this nicht kopiert) */
};

extern ITEMUSE_PMF g_item_use_funcs[];	/* ds:4d38 Methoden fuer benutzbare Gegenstaende (Index usable_desc.func bzw. combo_index) */

/* --- NvfMem: NVF-Entpacker fuer Dateien im Speicher (R41 nvf_new/nvf_delete/nvf_unpack; als lokales Objekt 0x1a Bytes
 * in O189/O197/O205 - struct nvf_desc plus ein Fuellbyte). Gegenstueck zu Nvf (O098), das aus der Datei liest. ----- */
class NvfMem : public nvf_desc {
	signed char _r19;
public:
	NvfMem();					/* R41_000A nvf_new */
	~NvfMem();					/* R41_0046 nvf_delete */
	signed long unpack(void far *dst, void far *src, signed short nr, signed short type, signed short far *w,
		signed short far *h, signed short stride);	/* R41_0715 nvf_unpack (type als short: push word) */
};

/* --- Nvf: Bild aus einer NVF-Datei auspacken (O098, 0x19 Bytes = struct nvf_desc; liest ueber g_file_3, R06) -------
 * Wie DSA1 process_nvf, aber direkt aus der Datei statt aus dem Speicher (Gegenstueck im Speicher: R41 nvf_*, gleiche
 * Daten struct nvf_desc: buf Zeilenpuffer, dst, src, nr, stride, type NVF_OUT_*, width/height Ergebniszeiger).
 * Freie Funktionen nvfile_* in s_proto.h. Eingebettet in inv_sheet (O008), Objekt g_nvf_heads. */
class Nvf : public nvf_desc {
public:
	Nvf();						/* O098_0000 Nvf_ctor */
	~Nvf();						/* O098_003D Nvf_dtor */
	void set(void far *dst, void far *src, signed short nr, signed char type, signed short far *w,
		signed short far *h, signed short stride);	/* O098_007E nvfile_set */
	signed long load(signed short id, void far *dst, void far *src, signed short nr, signed char type,
		signed short far *w, signed short far *h, signed short stride);	/* O098_00D1 nvfile_load: set + process */
	signed long process(signed short id);		/* O098_0118 nvfile_process: Bild nr der Datei id auspacken */
	void info(signed short id, signed short nr, signed short far *w, signed short far *h);	/* O098_06DB nvfile_info */
	signed short count(signed short id);		/* O098_08A1 nvfile_count: Bildzahl */
	void pal(signed short id, void far *p, unsigned short n);	/* O098_08F1 nvfile_pal: n Farben ab Dateiende - 0x300 */
	signed short dims(signed short id, signed short far *a, signed short far *b);	/* O098_096A nvfile_dims */
	/* alte Sicht der Aufrufer O008/O094 (Quelle als zwei Worte, Auspackart als short) - dieselbe Funktion O098_00D1 */
	void load(signed short id, void far *dst, signed short src_lo, signed short src_hi, signed short nr,
		signed short type, signed short far *w, signed short far *h, signed short stride);
};

/* --- Pic: AIF-Bild mit Palette (O269, 0xa6 Bytes), lokal u. a. in O023/O035/O188, eingebettet in inv_sheet --------
 * Der 30-Byte-Dateikopf wird ins Objekt selbst gelesen (+0x00..+0x1d). Quelle: Archivdatei-Objekt file (O037
 * File, Vorgabe g_file_main) oder Archivobjekt arc (R06, Vorgabe g_file_3; Ladefunktionen mit "f"). Die
 * "s"-Varianten lesen gepackte Daten in einem Stueck (stream). Freie Funktionen pic_* / Pic_* in s_proto.h. */
class Pic {
public:
	char magic[4];			/* +0x00 Kennung "AIF" */
	unsigned char pack;		/* +0x04 Packart PIC_PACK_* */
	signed char no_pal;		/* +0x05 keine Palette */
	signed short w;			/* +0x06 Breite */
	signed short h;			/* +0x08 Hoehe */
	signed short ncolors;		/* +0x0a Zahl der Farben */
	signed short first_color;	/* +0x0c erste Farbe (set_palette) */
	signed char _r0e[0x1e - 0x0e];
	struct rgb far *pal;		/* +0x1e Palette (eigene: own_pal) */
	signed short id;		/* +0x22 Datei */
	signed char from_arc;		/* +0x24 0 = aus file (O037), 1 = aus arc (R06) */
	signed char own_pal;		/* +0x25 Palette selbst angelegt (new) */
	signed char own_data;		/* +0x26 Bilddaten selbst angelegt (farmalloc) */
	signed char stream;		/* +0x27 gepackte Daten in einem Stueck lesen */
	signed short buf_pos;		/* +0x28 Leseposition in buf (99 = leer) */
	signed long size;		/* +0x2a Groesse der Bilddaten in der Datei */
	unsigned char far *data;	/* +0x2e Bilddaten (Ziel) */
	unsigned char far *tmp;		/* +0x32 gepackte Daten (stream, PP20) */
	unsigned char far *rp;		/* +0x36 Lesezeiger in tmp */
	unsigned char buf[100];		/* +0x3a Bytepuffer fuer get_byte */
	void far *file;			/* +0x9e Dateiobjekt (O037 File) */
	void far *arc;			/* +0xa2 Archivobjekt (R06 struct arc) */

	Pic(void far *file);					/* O269_0000 Pic_ctor_file (arc bleibt ungesetzt) */
	Pic();							/* O269_004E Pic_ctor (g_file_main, g_file_3) */
	~Pic();							/* O269_00AD Pic_dtor */
	signed short load(signed short id);			/* O269_010C pic_load0 */
	signed short load(signed short id, void far *dst);	/* O269_012C pic_load */
	signed short load(signed short id, void far *dst, struct rgb far *pal);	/* O269_014E pic_load_pal */
	signed short loadf(signed short id);			/* O269_0174 pic_loadf0 */
	signed short loadf(signed short id, void far *dst);	/* O269_0194 pic_loadf */
	signed short loadf(signed short id, void far *dst, struct rgb far *pal);	/* O269_01B6 pic_loadf_pal */
	signed short sload(signed short id);			/* O269_01DC pic_sload0 */
	signed short sload(signed short id, void far *dst);	/* O269_0203 pic_sload */
	signed short sload(signed short id, void far *dst, struct rgb far *pal);	/* O269_022C pic_sload_pal */
	signed short sloadf(signed short id);			/* O269_025A pic_sloadf0 */
	signed short sloadf(signed short id, void far *dst);	/* O269_0281 pic_sloadf */
	signed short sloadf(signed short id, void far *dst, struct rgb far *pal);	/* O269_02AA pic_sloadf_pal */
	signed short do_load(signed short id, void far *dst, struct rgb far *pal, signed char from_arc);
								/* O269_02D8 pic_do_load: Kopf, Palette, Daten */
	void set_palette();					/* O269_03C8 pic_set_palette */
	struct rgb far *get_palette();				/* O269_03F8 pic_getpal */
	void get_size(signed short far *w, signed short far *h);	/* O269_040A pic_get_size */
	unsigned char far *get_data();				/* O269_0429 pic_get_data */
	void get_size(signed short id, signed short far *w, signed short far *h);	/* O269_043B pic_get_size_id */
	signed short check(signed short id);			/* O269_0489 pic_check: Kennung "AIF"? */
	signed short read_header();				/* O269_057B pic_read_header */
	signed short read_pal();				/* O269_0646 pic_read_pal (vom Dateiende, Werte & 0x3f) */
	signed short read_data(void far *dst);			/* O269_0827 pic_read_data */
	unsigned char get_byte();				/* O269_098F pic_get_byte */
	signed short decode(unsigned char far *dst);		/* O269_0A43 pic_decode (Ziel ggf. farmalloc) */
	void unpack_raw(unsigned char far *dst);		/* O269_0B29 pic_unpack_raw */
	void unpack_rle(unsigned char far *dst);		/* O269_0B82 pic_unpack_rle */
	void rle(unsigned char far *dst);			/* O269_0C6D pic_rle: ueber get_byte */
	void rle_fast(unsigned char far *dst, unsigned char far *src, signed short w, signed short h);
								/* O269_0CF5 pic_rle_fast (Assembler) */
	void unpack_packbits(unsigned char far *dst);		/* O269_0D51 pic_unpack_packbits */
	void packbits(unsigned char far *dst);			/* O269_0E23 pic_packbits */
	void unpack_pp20(unsigned char far *dst);		/* O269_0EBC pic_unpack_pp20 */
};

/* --- Anim: Animationsobjekt (O051 Konstruktor/Methoden, O050 Abspieler; 0xaf Bytes, aktuelles Objekt g_anim_cur) ----
 * Spielt eine ACE-Animationsdatei (Kopf 8 Bytes "ACE", Sequenzen zu 16 Bytes bzw. Kurzkopf 6 Bytes, Bilder mit Kopf
 * 14 Bytes = struct fig_frame, Palette in den letzten 0x300 Bytes) an der Lage (x, y) in den Zielpuffer dst; Takt ueber
 * den Zeitgeber (g_anim_running, g_anim_timer, tick). Jedes Bild kann eine Marke cues[] tragen: beim Bildwechsel steht
 * sie in cue (Bildfolgen O039/O040/O041 schalten daran Texte und Sprache weiter). Freie Namen Anim_* in s_proto.h
 * (this = erstes Argument). C-Module: struct Anim gleichen Aufbaus (oben); Teilsicht struct anim_view (s_struct.h). */
class Anim {
public:
	unsigned char far *pal;		/* +0x00 Palette (0x300 Bytes) */
	signed char from_file;		/* +0x04 aus dem Dateiobjekt g_file_main (O037) statt aus dem Archiv g_file_3 (R06) */
	signed short x;			/* +0x05 Lage */
	signed short w;			/* +0x07 Breite (Sequenzsatz) */
	unsigned char y;		/* +0x09 */
	unsigned char h;		/* +0x0a Hoehe (Sequenzsatz) */
	void far *dst;			/* +0x0b Zielpuffer (Vorgabe g_vga_buf) */
	void far *data;			/* +0x0f gepackte Daten des aktuellen Bildes (buf_size Bytes) */
	struct fig_frame far *frame;	/* +0x13 Kopf des aktuellen Bildes (Versatz x/y, w, h, size, pack, event) */
	signed short id;		/* +0x17 Datei */
	signed short nr_seqs;		/* +0x19 Zahl der Sequenzen */
	signed short seq;		/* +0x1b Sequenz */
	signed short nr_frames;		/* +0x1d Zahl der Bilder */
	signed short cur;		/* +0x1f aktuelles Bild */
	unsigned char rate;		/* +0x21 Takte je Bild (Dateikopf; O134 setzt 3/2) */
	unsigned char cue;		/* +0x22 Marke des aktuellen Bildes (cues[cur]), next() setzt 0 */
	unsigned char loop;		/* +0x23 3 = wiederholen, 0 = am Ende anhalten (Sequenzsatz: 1 -> 3) */
	unsigned char f24;		/* +0x24 (start: 0) */
	signed char _r25;
	unsigned char rounds;		/* +0x26 abgeschlossene Durchlaeufe */
	Bmp bmp;			/* +0x27 Bild (Puffer bmp_buf) */
	Bmp bg;				/* +0x31 Hintergrund (setbg) */
	void far *bmp_buf;		/* +0x3b */
	void far *bg_buf;		/* +0x3f */
	signed short bmp_ext;		/* +0x43 bmp_buf fremd (nicht freigeben) */
	signed short bg_ext;		/* +0x45 bg_buf fremd */
	unsigned char cues[0x64];	/* +0x47 Marke je Bild (fig_frame.event) */
	signed long buf_size;		/* +0xab groesstes gepacktes Bild aller Sequenzen (max_frame_size) */

	Anim(signed short id, signed short seq, signed short from_file);	/* O051_0000 Anim_ctor (O051: from_file als
							   unsigned char definiert, Aufrufer uebergeben ein Wort) */
	~Anim();					/* O051_011B Anim_dtor */
	void size(signed short far *w, signed short far *h);	/* O051_0242 Anim_size: w, h */
	void area(signed short far *x, signed short far *y, signed short far *w, signed short far *h);
							/* O051_0263 Anim_area: Lage und Groesse */
	void frame_size(signed short far *w, signed short far *h);	/* O051_02A0 Anim_frame_size */
	void rect(signed short far *x, signed short far *y, signed short far *w, signed short far *h);
							/* O051_02C7 Anim_rect: Rahmen des aktuellen Bildes */
	void pal_set(signed short first, signed short n);	/* O051_030F Anim_pal: n Farben ab first setzen */
	void setbuf(Bmp far *b);			/* O051_0338 Anim_setbuf_bmp: Bildpuffer aus Bmp (fremd) */
	void setbuf(signed short w, signed short h, void far *p);	/* O051_03AB Anim_setbuf: Bildpuffer fremd */
	void setbg(Bmp far *b);				/* O051_03F6 Anim_setbg: Hintergrund aus Bmp */
	void setbg(signed short w, signed short h, void far *p);	/* O051_0469 Anim_setbg_buf: Hintergrund fremd */
	void clr_buf();					/* O051_04B4 Anim_clr_buf: Bildpuffer loesen */
	void clr_bg();					/* O051_04D4 Anim_clr_bg: Hintergrund loesen */
	void start(signed short x, signed short y, signed short no_skip);	/* O051_04F4 Anim_start (x/y < 0: alte
							   Lage; no_skip -> g_anim_no_skip) */
	void stop();					/* O051_05D0 Anim_stop */
	void wait_cue();				/* O051_05ED Anim_step: warten, bis ein Bild eine Marke hat */
	void wait_frame(signed short n);		/* O051_061A Anim_wait: warten bis Bild n */
	void next();					/* O051_064C Anim_next: Marke abwarten und loeschen */
	void tick();					/* O050_0000 Anim_tick: Takt, Bildwechsel */
	void advance();					/* O050_024D Anim_advance: naechstes Bild (Ende: rounds, loop) */
	void unpack(Bmp far *b);			/* O050_0289 Anim_unpack: aktuelles Bild nach b entpacken (frueher
							   setbg2) */
	unsigned char running();			/* O050_02E5 Anim_running: g_anim_running == 1 */
	void load();					/* O050_02F9 Anim_load: Kopf, Sequenz, erstes Bild, Palette */
	void load_pal(signed short first, signed short n);	/* O050_09C0 Anim_load_pal: Palettenteil nachladen */
	signed long max_frame_size();			/* O050_0AC5 Anim_max_frame_size -> buf_size */
	void load_frame();				/* O050_0CC9 Anim_load_frame: Bild cur laden */
	void select_seq(signed short n);		/* O050_1023 Anim_select_seq: Sequenz n waehlen, neu starten */
	void set_dst(void far *p) { dst = p; }		/* O039/O040 (Parameterkopie) */
	void set_loop(unsigned char v) { loop = v; }	/* O040 (Parameterkopie) */
};

/* --- Inv: Heldenbogen (Daten struct inv_sheet; Methoden in O008/O009/O094, freie Namen inv_* in s_proto.h) -- */
class Inv : public inv_sheet {
public:
	Inv();						/* O008_0000 Inv_ctor (Code als Sicht InvObj in O008) */
	~Inv();						/* O008_048B Inv_dtor */
	void set_cursor(signed short n);		/* O008_0635 inv_set_cursor */
	void reset_cursor();				/* O008_06C1 inv_reset_cursor */
	void draw_slot(void far *dst, signed short slot, signed short color);	/* O008_07EE inv_draw_slot */
	void update_marks();				/* O008_06DB inv_update_marks */
	void draw_slots(void far *dst);			/* O008_07AE inv_draw_slots */
	void reselect();				/* O008_0B9F inv_reselect */
	void confirm();					/* O009_0000 inv_confirm: verschieben/tauschen/benutzen */
	void next_hero();				/* O009_05B7 inv_next_hero */
	void warn_signal();				/* O009_0698 inv_warn_signal */
	void toggle_body();				/* O009_08F3 inv_toggle_body */
	void draw();					/* O009_09BB inv_draw */
	void draw_page1();				/* O094_0000 Eigenschaften */
	void draw_page2();				/* O094_04BA Kampfwerte */
	void draw_page3();				/* O094_078D Talente (inv_draw_skills) */
	void draw_page4();				/* O094_07A0 Zauber 1 */
	void draw_page5();				/* O094_0A3E Zauber 2 */
	void draw_inventory();				/* O094_0BEC */
	void draw_body_slots();				/* O094_0F55 */
	void knapsack_open();				/* O094_11A8 */
	void knapsack_close();				/* O094_1489 */
};


/* --- File: Spieldatei einzeln oder im Archiv (O037, 0x38 Bytes), Objekt g_file_main ds:35c6, Klaenge (R04) ---------
 * Archivformat: Wort n, n Eintraege zu 20 Bytes (Wort "vorhanden", Name[14], long Versatz ab 2 + n * 20). Freie
 * Funktionen file_* / File_* in s_proto.h (gleiche Bytes, Objekt als erstes Argument). */
class File {
public:
	char far * far *names;		/* +0x00 Namenstabelle (id -> Dateiname) */
	signed short fd;		/* +0x04 Handle; Bit FILE_FD_ARC = Eintrag im Archiv (Handle der Archivdatei) */
	char far *arc_name;		/* +0x06 Name der Archivdatei */
	unsigned char far *loose;	/* +0x0a Bitfeld je id: Datei liegt einzeln vor (nur mit g_str_star_dat) */
	char name_buf[14];		/* +0x0e Namenspuffer (getname aus der Namensliste, Archivkopf) */
	signed short have_arc;		/* +0x1c Archivdatei gefunden (Konstruktor 2) */
	signed long size;		/* +0x1e Groesse des Archiveintrags */
	signed long left;		/* +0x22 davon noch zu lesen */
	signed long start;		/* +0x26 Anfang im Archiv */
	signed long end;		/* +0x2a Ende im Archiv */
	signed short count;		/* +0x2e Zahl der Dateien */
	char far *name_list;		/* +0x30 Datei mit der Namensliste (14 Bytes je Name) */
	signed char _r34[4];

	File(char far * far *names, signed short count);	/* O037_0000 File_ctor_names: nur Namenstabelle */
	File(char far *arc_name, unsigned char far *loose, char far * far *names, char far *name_list, signed short count);
								/* O037_0067 File_ctor: mit Archiv */
	~File();						/* O037_014F File_dtor */
	signed short open(signed short id);			/* O037_0174 file_open */
	signed short open(signed short id, signed short mode);	/* O037_01CB file_open_mode */
	signed long load(signed short id, void huge *buf);	/* O037_0225 file_load: oeffnen, ganz lesen, schliessen */
	signed long read_all(void huge *buf);			/* O037_02AB File_read_all */
	signed long read(void huge *buf, signed long len);	/* O037_03C9 file_read */
	signed long write(void huge *buf, signed long len);	/* O037_04D6 File_write (nicht im Archiv) */
	signed short close();					/* O037_050D file_close */
	signed long length();					/* O037_0537 file_length2: der offenen Datei */
	signed long length(signed short id);			/* O037_057F file_length: der Datei id */
	signed long seek(signed long off, signed short whence);	/* O037_05C9 file_seek (im Archiv relativ zum Eintrag) */
	signed short create(signed short id, signed short attr);	/* O037_0706 file_create (_creat) */
	char far *getname(signed short id);			/* O037_0739 file_getname */
	signed short find(char far *name);			/* O037_07D3 File_find: Name -> id, -1 */
	signed short is_loose(signed short id);			/* O037_094C file_is_loose: Bit id im Bitfeld loose */
	signed short open_handle(signed short id);		/* O037_09C6 file_open_handle: einzeln, Pfade, Archiv */
	signed short open_handle(signed short id, signed short mode);	/* O037_0B42 file_open_handle_mode */
	signed short exists(signed short id);			/* O037_0B9E File_exists */
	signed short open_in_arc(signed short id);		/* O037_0D84 file_open_in_arc: Handle | FILE_FD_ARC */
};

/* --- TempFile: Arbeitsdatei in ..\TEMP (O243, 4 Bytes), Objekt g_ofile ds:b635 -----------------------------------
 * Name sprintf(g_path_fmts[0], file_getname(g_file_main, id)); fehlt die Datei, wird sie zuerst aus dem Archiv
 * (g_file_main) dorthin kopiert (extract). Freie Funktionen ofile_* in s_proto.h. */
class TempFile {
public:
	signed short fd;		/* +0 DOS-Handle, 0 = geschlossen */
	signed short access;		/* +2 Zugriffsart beim Oeffnen (O_RDONLY ...); ohne O_RDONLY leert close die DOS-Puffer */

	TempFile();						/* O243_0000 TempFile_ctor */
	signed short extract(signed short id);			/* O243_0028 ofile_extract: aus dem Archiv nach TEMP kopieren */
	signed short open(signed short id);			/* O243_01DD ofile_open (O_RDONLY | O_BINARY) */
	signed short open(signed short id, signed short access);	/* O243_0291 ofile_create (z. B. O_RDWR | O_BINARY) */
	signed short open(signed short id, signed short access, unsigned short mode);	/* O243_0343 ofile_open_mode */
	signed long load(signed short id, void far *buf);	/* O243_03FB ofile_load: oeffnen, ganz lesen, schliessen */
	signed long read_all(void far *buf);			/* O243_0475 ofile_read_all */
	signed long read(void far *buf, signed long len);	/* O243_04A0 ofile_read */
	signed long write(void far *buf, signed long len);	/* O243_04BF ofile_write (Meldung bei Kurzschreiben) */
	signed short close();					/* O243_051B ofile_close */
	signed long length();					/* O243_0554 ofile_length: der offenen Datei */
	signed long length(signed short id);			/* O243_057C ofile_length_id: der Datei id */
	signed long seek(signed long off, signed short whence);	/* O243_05C6 ofile_seek */
};

/* --- Font: Schrift-/Textobjekt (O248, 0x49 Bytes; Daten struct font in s_struct.h), Objekt g_font ds:37e4 -----
 * Laden des Zeichensatzes und der Textlisten in O248; Zeichnen/Messen sind freie Funktionen font_* in R20 (SEG020,
 * erster Parameter das Objekt). */
class Font : public font {
public:
	Font(signed short id);				/* O248_0000 Font_ctor: Zeichensatzdatei id laden (g_file_main) */
	~Font();					/* O248_0138 Font_dtor */
	signed short set_line_h(signed short h);	/* O248_01DA font_set_line_h: Zeilenabstand (-1 = nur lesen), alter Wert */
	signed short goto_x(signed short x);		/* O248_0200 font_goto_x: font_set_x, liefert x */
	void goto_xy(signed short x, signed short y);	/* O248_021E font_goto_xy: font_set_x + font_set_y */
	void split(char far * far *list, char far *s, signed long len);	/* O248_0245 txt_split: an Nullbytes zerlegen */
	void load_text(signed short id, char far *buf, char far * far *list);	/* O248_029C txt_load: in buf laden + zerlegen */
	void load_list(signed short id, signed short n, void far * far *pbuf, char far * far * far *plist);
							/* O248_02E3 txt_load_list: Puffer/Liste (n + 1 Zeiger) anlegen */
	signed short max_width(char far *s);		/* O248_0394 txt_max_width: breiteste Zeile ('@' = Umbruch) in Pixeln */
};

/* --- Stash: Gegenstandsablage eines Ortes (O238, 0x16 Bytes; new Stash(nr, flag) in O000/O062/O065/O262..O264) ---
 * Datensatz nr der Arbeitsdatei 0x2c6 (je Satz Laengenwort + Eintraege struct stash_entry), Texte 0x2c7. Die
 * Gruppe kann Gegenstaende/Geld entnehmen (menu); der Destruktor schreibt den Satz zurueck. Freie Funktionen
 * stash_* / Stash_* in s_proto.h. */
class Stash {
public:
	struct stash_entry far *items;	/* +0x00 Eintraege */
	signed short nr;		/* +0x04 Satznummer in Datei 0x2c6 */
	signed short cap;		/* +0x06 Zahl der Plaetze (Laenge / 4) */
	signed short count;		/* +0x08 belegte Plaetze (update) */
	signed short top;		/* +0x0a erster angezeigter Eintrag (blaettern um 9) */
	signed short more;		/* +0x0c Menuezeile "weiter", 0 = keine */
	void far *txt_buf;		/* +0x0e Texte 0x2c7 (txt_load_list) */
	char far * far *txt;		/* +0x12 [0] leer, [1] Frage, [2] wie viele, [3] weiter */

	Stash(signed short nr, signed short quiet);		/* O238_0000 Stash_ctor: laden, bei quiet == 0 gleich show */
	~Stash();						/* O238_011D Stash_dtor: Satz zurueckschreiben */
	void set(signed short i, signed short id, unsigned char n);	/* O238_0208 stash_set */
	signed short is(signed short i, signed short id, unsigned char n);	/* O238_0237 stash_is */
	void add(signed short i, signed short id, unsigned char n);	/* O238_0275 stash_add: einfuegen bzw. dazuzaehlen */
	signed short has_items();				/* O238_036C stash_has_items */
	signed short count_items();				/* O238_038E stash_count_items */
	signed short show();					/* O238_03AA stash_show: Meldung "leer" bzw. menu */
	signed short update();					/* O238_03F1 stash_update: count, top = 0 */
	signed short fill(char far *lines);			/* O238_0441 stash_fill: bis zu 10 Menuezeilen zu 40 Bytes */
	signed short menu();					/* O238_066F stash_menu: Entnahme, 1 = etwas genommen */
};

/* --- Screen: Spielbildschirm mit Heldenleiste, Textleiste und Symbolfeldern (O188, leer, 1 Byte), Objekt g_screen -----
 * Zustand in globalen Daten: Symbolnummern (9 Felder, -1 = Grundsymbol), geladene Symbole g_loc_icons_loaded,
 * Symbolpuffer g_loc_icon_buf (9 x 24 x 24), Rahmenbilder g_bmp_mark_le/g_bmp_mark_ae. Freie Funktionen screen_* in
 * s_proto.h (Aufrufe aus O014/O069/O070/O076/O082/O251 u. a.). */
class Screen {
public:
	Screen();					/* O188_0000 Screen_ctor: Rahmenbilder laden, Symbole zuruecksetzen */
	void draw_base();				/* O188_009B screen_draw_base: Grundbild, Palette, Schrift, Mausbereiche */
	void short_name(char far *dst, char far *name);	/* O188_01D0 screen_short_name: bis 7 Zeichen/1. Leerzeichen */
	void draw_portraits();				/* O188_020C screen_portraits: Heldenleisten aller Gruppen */
	void clear_portrait(signed short pos);		/* O188_02A7 screen_clear_portrait: leeren Platz schwarz */
	void load_icon(signed short file, signed short nr, signed short field);	/* O188_03D2 screen_load_icon */
	void draw_icons();				/* O188_045A screen_draw_icons: die 9 Symbolfelder */
	void redraw();					/* O188_0541 screen_redraw: Bildschirm komplett neu */
	void clear_textbar();				/* O188_05B2 screen_clear_textbar */
	void clear_view();				/* O188_0625 screen_clear_view: 3D-Fenster 0xd0 x 0x87 */
	void draw_names();				/* O188_068B screen_draw_names: Namensleisten */
	void draw_name(signed short pos, signed short color);	/* O188_092D screen_draw_name */
	void draw_name_normal(signed short pos);	/* O188_0A2D screen_draw_name_normal */
	void textbar(char far *s, signed short color);	/* O188_0B2C screen_textbar: Meldung in der Textleiste */
	void mark_portrait(signed short pos, signed short ae);	/* O188_0BC7 screen_mark_portrait: Rahmen LE/AE */
	void set_icons(signed short n, ...);		/* O188_0C2F draw_loc_icons: n Symbolnummern, bei Aenderung zeichnen */
	void reset_icons();				/* O188_0CBF screen_reset_icons: alle Felder -1 */
};

/* --- Edit: mehrzeiliges Texteingabefeld (0x38 Bytes, Code O110/O111; Automap O052, Tagebuch O103). Zeilen zu buflen
 * Bytes, Zeichenbreite + 1 Pixel je Zeichen, Wortumbruch bei width Pixeln; Hintergrund aus dem Bild bg_pic oder
 * Linien in Farbe col_bg. Zeichnet in scr (Zielpuffer des Zeichensatzes umgestellt), flush kopiert nach g_vga_buf. */
class Edit {
public:
	signed short nlines;		/* +0x00 Zeilen */
	signed short width;		/* +0x02 Breite in Pixeln */
	signed short buflen;		/* +0x04 Zeilenpuffer (2 * width Bytes) */
	signed short x;			/* +0x06 Lage */
	signed short y;			/* +0x08 */
	signed short line_h;		/* +0x0a Zeilenhoehe (font_set_line_h) */
	signed short col_fg;		/* +0x0c Schriftfarben (font_get_colors) */
	signed short col_bg;		/* +0x0e */
	signed short line;		/* +0x10 Cursor: Zeile */
	signed short col;		/* +0x12 Cursor: Spalte */
	signed short need_redraw;	/* +0x14 alles neu zeichnen */
	signed short need_line;		/* +0x16 Zeile neu zeichnen */
	signed short need_char;		/* +0x18 Zeile ab dem Cursor neu zeichnen */
	signed short cursor_cnt;	/* +0x1a Zaehler cursor_on/cursor_off (0 = sichtbar) */
	signed short cx;		/* +0x1c Cursor in Pixeln */
	signed short cy;		/* +0x1e */
	void far *bg_pic;		/* +0x20 Hintergrundbild (Bmp), 0 = Linien */
	void far *scr;			/* +0x24 Zielpuffer (0 im Konstruktor: g_vga_buf) */
	void far *old_dst;		/* +0x28 Zielpuffer des Zeichensatzes vorher */
	char far * far *lines;		/* +0x2c nlines + 1 Zeilen */
	struct rect clip;		/* +0x30 Clip-Rechteck vorher (x0 == -1: nicht umgestellt) */

	Edit(signed short nl, signed short w, signed short x, signed short y, void far *pic, void far *scr,
		signed short clip);			/* O111_0000 Edit_ctor */
	~Edit();					/* O111_0205 Edit_dtor */
	void del_char();				/* O111_02A6 edit_del_char: Zeichen links loeschen (Spalte 0: join) */
	signed short insert_char(char c);		/* O111_0470 edit_insert_char */
	signed short last_space(char far *s);		/* O111_052F edit_last_space */
	signed short word_len(char far *s);		/* O111_055C edit_word_len */
	void next_line();				/* O111_057F edit_next_line */
	void prev_line();				/* O111_05AD edit_prev_line */
	void cursor_left();				/* O111_05D1 edit_cursor_left */
	void home();					/* O111_06DD edit_home */
	void cursor_right();				/* O111_0723 edit_cursor_right */
	void end();					/* O111_07F2 edit_end */
	void line_up();					/* O111_0888 edit_line_up */
	void line_down();				/* O111_09DF edit_line_down */
	void flush();					/* O111_0B43 edit_flush: scr nach g_vga_buf */
	void getkey();					/* O110_0000 edit_getkey: poll_input, '"' -> '>' */
	void run(char far *out);			/* O110_0017 Edit_run: Eingabeschleife */
	void prepend(char far *d, char far *s);		/* O110_05F5 edit_prepend */
	signed short measure(char far *s);		/* O110_066F Edit_measure: Pixelbreite */
	void wrap(signed short l);			/* O110_06A1 Edit_wrap */
	void redraw();					/* O110_09E7 edit_redraw */
	void redraw_line();				/* O110_0AFF edit_redraw_line */
	void cursor_on();				/* O110_0BDF Edit_curon */
	void cursor_off();				/* O110_0D41 Edit_curoff */
	void newline();					/* O110_0E4C edit_newline */
	void join();					/* O110_0F57 Edit_join */
	void settext(char far *s);			/* O110_1140 Edit_settext */
	void clear_tails();				/* O110_11F4 edit_clear_tails */
};

/* --- Script: Gespraechsskript eines Gespraechspartners (O260, 0xb4 Bytes; new Script(id) in O049 Conv::run).
 * Themenliste Datei 0x50 (je Zeile Themenname und "Titel"), Skriptdatei id: Zeilen "TOLERANCE n", "PIC n",
 * "NAME "..."", "TOPIC name" (Themawechsel), "END", sonst "a b "Text"" bzw. "a b SCHLUESSELWORT" (Antwort b
 * zur Bedingung a, Schluesselwoerter g_script_answer_kw -> SCRIPT_ANS_*), Fortsetzungszeilen mit '"'.
 * Freie Namen Script_* in s_proto.h. */
struct script_ent {			/* 10 Bytes: Antworten eines Themas zu einer Bedingung a */
	signed char n;			/* +0 Zahl der Antworten */
	signed char cur;		/* +1 naechste Antwort (Conv::ask zaehlt weiter bis n - 1) */
	signed char far *vals;		/* +2 je Antwort Wert b (Rueckruf; < 0: -b, ohne Weiterzaehlen) */
	signed short far *answers;	/* +6 je Antwort Text in texts[] bzw. SCRIPT_ANS_* (< 0) */
};

struct script_topic {			/* 8 Bytes: Thema */
	unsigned char n;		/* +0 Zahl der Eintraege (Bedingungen) */
	signed char _r1;
	signed short cap;		/* +2 Plaetze in list */
	struct script_ent far * far *list;	/* +4 Eintraege je Bedingung */
};

class Script {
public:
	unsigned short n_texts;		/* +0x00 Zahl der Texte in texts */
	signed char used[0x80];		/* +0x02 Thema kommt im Skript vor (memset nur 100 Bytes) */
	char far *topics_raw;		/* +0x82 Themenliste (Datei 0x50) */
	char far * far *topic_names;	/* +0x86 Themennamen */
	signed short n_topics;		/* +0x8a Zahl der Themen */
	signed short cur;		/* +0x8c aktuelles Thema beim Lesen */
	char far *raw;			/* +0x8e Skriptdatei */
	char far * far *lines;		/* +0x92 Zeilen der Skriptdatei */
	signed long raw_size;		/* +0x96 Laenge der Skriptdatei */
	signed short n_lines;		/* +0x9a Zahl der Zeilen */
	signed short last;		/* +0x9c Bedingung a des letzten Eintrags (TOPIC: -1) */
	signed short f9e;		/* +0x9e (je Schluesselwort 0) */
	signed short pic;		/* +0xa0 Portraet (PIC; load_portrait_buf(pic - 1)) */
	signed short tolerance;		/* +0xa2 Geduld: Fragen bis zum Abbruch (TOLERANCE) */
	char far *name;			/* +0xa4 Name des Gespraechspartners (NAME) */
	struct script_topic far *topics;	/* +0xa8 Themen (n_topics + 1) */
	char far * far *titles;		/* +0xac Titel je Thema */
	char far * far *texts;		/* +0xb0 Antworttexte (1000 Zeiger) */

	Script(signed short id);			/* O260_0000 Script_ctor */
	~Script();					/* O260_011F Script_dtor */
	signed short find(char far *name);		/* O260_02DD script_find: Themenname -> Index, -1 */
	void make_titles();				/* O260_0393 script_make_titles */
	signed short load_topics();			/* O260_0415 script_load_topics: Datei 0x50 */
	signed short count_lines(char far *s);		/* O260_0560 script_count_lines (CR) */
	signed short load(signed short id);		/* O260_05B7 script_load_file */
	void split(char far *s, char far * far *arr, signed short n, signed short strip);	/* O260_0635 script_split */
	char far *cut(char far *p, char c);		/* O260_06E3 script_cut: bis c abschneiden */
	char far *next_line(char far *src, char far *dst);	/* O260_0712 script_next_line: ohne Vorspann nach dst */
	char far *word(char far *dst, char far *src);	/* O260_07A5 script_word */
	char far *quoted(char far *p);			/* O260_0801 script_quoted: Text in "..." */
	signed short keyword(char far *p);		/* O260_0851 script_keyword: -1 - Index in g_script_answer_kw */
	signed short is_quote(char far *p);		/* O260_0905 script_is_quote */
	signed short is_used(signed short i);		/* O260_0944 Script_is_used */
	void add(signed short a, signed short b, signed short c);	/* O260_0961 script_add: Antwort zum Thema cur */
	void parse();					/* O260_0BF5 script_parse */
};

/* --- Conv: Gespraech mit Themenliste (0x53a Bytes, Objekt g_dialog2 ds:b86e; Code O049 und O257, Szenen O259).
 * Themen 0..99 (topic_state: 0 aus, 1 bekannt, 2 neu), Themenliste list[] (sortiert, Ende 0), Antworten aus dem
 * Skript (class Script, O260), Zufallstexte je Thema nach topic_txt (Datei 0x1f2). Freie Namen conv_* / Conv_* in
 * s_proto.h (this = erstes Argument, fuer Aufrufer mit g_dialog2). */
struct conv_topic_txt {			/* 9 Bytes, Datei 0x1f2: Zufallstexte je Thema (Conv::random_text) */
	signed short q_first;		/* +0 erste Frage (Art 1) */
	signed short a_first;		/* +2 erste Antwort (Art 0/2) */
	signed short alt_first;		/* +4 erste Alternativantwort (Art 3) */
	signed char q_n;		/* +6 Zahl der Fragen */
	signed char a_n;		/* +7 Zahl der Antworten */
	signed char alt_n;		/* +8 Zahl der Alternativantworten */
};

struct conv_counter {			/* 3 Bytes, Stufe je Gespraechspartner (Spielstand) */
	signed short id;		/* +0 Gespraech (Satz in Datei 0x203) */
	signed char level;		/* +2 erreichte Stufe (O005 zaehlt alle 4 Stunden herunter) */
};

class Conv {
public:
	signed short old_line_h;	/* +0x000 Zeilenabstand vor dem Gespraech */
	signed short n_shown;		/* +0x002 Zahl der angezeigten Themen */
	signed short mark;		/* +0x004 markiertes Thema (1..n_shown, -1 = keins) */
	signed short f6;		/* +0x006 */
	signed short done;		/* +0x008 0 laeuft, 1 Ende, 2 Abschlussmenue */
	signed short id;		/* +0x00a Gespraech (Satz in Datei 0x203) */
	signed short topic;		/* +0x00c zuletzt gewaehltes Thema, -1 = keins */
	void far *txt_buf;		/* +0x00e Texte 0x24 (0xf3 Eintraege) */
	char far * far *txt;		/* +0x012 */
	void far *menu_buf;		/* +0x016 Menuetexte 0x409 */
	char far * far *menu_txt;	/* +0x01a [0] Titel, [1] Notiz, [2] weiter, [3] Ende, [5]/[6] Tagebuchformate */
	void far *topic_buf[100];	/* +0x01e Zufallstexte je Thema (Datei g_conv_topic_tab[t - 7]) */
	char far * far *topic_txts[100];	/* +0x1ae */
	char far *answer;		/* +0x33e letzte Antwort (1000 Bytes, Tagebuchnotiz) */
	void far *bg;			/* +0x342 Hintergrund 64000 Bytes */
	char far *greeting;		/* +0x346 Begruessung aus dem Skript (Thema 0x62) */
	char far *farewell;		/* +0x34a Abschied (Thema 0x61) */
	char far *no_answer;		/* +0x34e Antwort ohne Eintrag (Thema 0x63) */
	struct conv_topic_txt far *topic_txt;	/* +0x352 Datei 0x1f2 */
	char far *menu_lines[5];	/* +0x356 Menuezeilen (set_menu) */
	signed char n_menu;		/* +0x36a */
	signed short script_id;		/* +0x36b Skript (Satz der Datei 0x203) */
	signed short callback;		/* +0x36d Rueckruf g_conv_callbacks[], 0 = keiner */
	signed char level;		/* +0x36f Stufe (Fragen bisher; >= script->tolerance: Abbruch) */
	signed char topic_state[100];	/* +0x370 je Thema 0 aus, 1 bekannt, 2 neu */
	signed char unlocked[100];	/* +0x3d4 im Skript freigeschaltet (relock nimmt es zurueck) */
	signed short list[100];		/* +0x438 angezeigte Themen (Menuezeile n = list[n - 1]) */
	struct conv_counter counters[10];	/* +0x500 */
	Script far *script;		/* +0x51e */
	Bmp bmp;			/* +0x522 Bildschirm (bg) */
	signed short refresh;		/* +0x52c Liste nach n Fragen aufraeumen (insert_topic: 2) */
	void far *saved_areas;		/* +0x52e Mausbereiche vor dem Gespraech */
	void far *scene_buf;		/* +0x532 Szenentexte XDS.LTX = Datei 0x1d8 (O259 conv_scene_*; frueher f532) */
	char far * far *scene_txt;	/* +0x536 (frueher f536) */

	/* O049 Gespraechsfuehrung */
	signed short run(signed short id);		/* O049_0000 conv_run: Gespraech fuehren, 1 = gefuehrt */
	void print(char far *s, signed short mode);	/* O049_0AE7 conv_print: mode 0 merkt den Text in answer */
	void mark_topic(signed short n);		/* O049_0B8E conv_mark_topic */
	signed short ask(signed short n);		/* O049_0CC3 conv_ask: Thema der Zeile n, 1 = neu zeichnen */
	void draw_topics(signed short clear);		/* O049_0FEF conv_draw_topics */
	char far *random_text(signed short t, signed char kind);	/* O049_10EF conv_random_text */
	void draw(signed short mode);			/* O049_124C conv_draw: 1/3 ganz, 2 Teil, sonst Hintergrund */
	void clear_new();				/* O049_151D conv_clear_new: neu (2) -> aus */
	void mouse_on();				/* O049_1546 conv_mouse_on: Mausbereiche/Rueckruf setzen */
	void mouse_off();				/* O049_158E conv_mouse_off */
	void dummy();					/* O049_15AF conv_dummy */
	void answer_frame();				/* O049_15B4 conv_answer_frame */
	/* O257 */
	Conv();						/* O257_0000 Conv_ctor */
	~Conv();					/* O257_0116 Conv_dtor */
	void greet();					/* O257_0164 conv_greet */
	void bye();					/* O257_020A conv_bye */
	void insert_topic(signed short t);		/* O257_0281 conv_insert_topic */
	void remove_temp();				/* O257_031D conv_remove_temp: nicht bekannte (neue) Themen entfernen */
	void script_topics();				/* O257_03A2 conv_script_topics */
	void relock();					/* O257_059E conv_relock */
	void place_topics();				/* O257_05C8 conv_place_topics: Themen je Stadt */
	signed short save();				/* O257_0BB3 conv_save ("XDIA") */
	signed short load();				/* O257_0C1A conv_load */
	signed char get_counter(signed short id);	/* O257_0C8B conv_get_counter */
	void set_counter(signed short id);		/* O257_0CC5 conv_set_counter */
	void dec_counter(signed short k);		/* O257_0D4E conv_dec_counter */
	void clr_counter(signed short id);		/* O257_0D89 conv_clr_counter */
	void compact_counters();			/* O257_0DD0 conv_compact_counters */
	signed char set_topic(signed short t, signed short v);	/* O257_0E49 conv_set_topic: alter Wert */
	void topic_on(signed short t);			/* O257_0E93 conv_topic_on */
	void unlock_topic(signed short t);		/* O257_0EA4 conv_unlock_topic */
	void build_list();				/* O257_102B conv_build_list */
	void refuse();					/* O257_110C conv_refuse: Gespraech schon beendet */
	void set_menu(signed short n, ...);		/* O257_11DB conv_set_menu */
	signed short menu(signed short n);		/* O257_1261 conv_menu: Zeile + 1 */
	void draw_menu();				/* O257_1538 conv_draw_menu */
	void note();					/* O257_164D conv_note: Antwort ins Tagebuch */
};

/* --- Diary: Tagebuch (Daten struct diary in s_struct.h; Code O103, O104; Druck/Schluessel in O105 als freie
 * Funktionen diary_*(DIARY_PTR)). Freie Namen der Methoden in s_proto.h (this = erstes Argument). */
class Diary : public diary {
public:
	Diary();					/* O103_0000 Diary_ctor */
	void run();					/* O103_0087 diary_run: Tagebuch-Schleife */
	signed short add(char far *s, signed short own);	/* O103_04E1 diary_add: Eintrag anhaengen, 1 = ok */
	void lock_inc();				/* O103_06AB diary_lock */
	void lock_dec();				/* O103_06B7 diary_unlock */
	void write();					/* O103_06C3 diary_write: eigener Eintrag (Edit-Feld) */
	void search();					/* O103_0887 diary_search */
	void print_menu();				/* O103_0F1E diary_print_menu */
	signed short append(char far *s, signed short own);	/* O104_0000 diary_append: Seite setzen, Laenge + 2 */
	void mark_names();				/* O104_0456 diary_mark_names */
	void mark_word(struct sel_rec far *list, char far *dst, char far *word, signed short far *pj);	/* O104_070C */
	void show_page(unsigned short page);		/* O104_08E3 diary_show_page */
	void draw(signed short mode, void far *dst);	/* O104_0D3D diary_draw: Seite zeigen (dst 0: eigener Puffer bzw. Bildschirm) */
	signed short load_page(unsigned short page);	/* O104_110C diary_load_page */
	void place_str(char far *buf);			/* O104_1245 diary_place_str */
	void date_str(char far *buf);			/* O104_12A5 diary_date_str */
	signed short count_lines(char far *s);		/* O104_133B diary_count_lines */
};

/* --- Automap: Kartenobjekt der 3D-Ansicht (0x28d Bytes, g_map ds:be36; Code O052, O053). C-Sicht struct map
 * (s_struct.h, nur die Felder fuer R08). Speicherblock handle (XMS bzw. EMS): Hintergrundbild 64000 Bytes, ab 0xfa00
 * 100 Notizsaetze zu 0x100 Bytes. Freie Namen automap_* bzw. Automap_ctor/_dtor in s_proto.h. ------------------- */
struct map_tile {			/* 2 Bytes, Feld der Automap-Karte (Kartendatei: w, h, w * h Felder) */
	unsigned char type;		/* +0 Feldtyp (Kachel ueber tile_idx) */
	unsigned char info;		/* +1 Feldinfo (Notiz nur bei 0 loeschbar) */
};

struct map_note {			/* 0x100 Bytes, Notizsatz der Automap */
	signed short x;			/* +0x00 Feld (-1 = frei) */
	signed short y;			/* +0x02 */
	char text[0xfc];		/* +0x04 */
};

class Automap {
public:
	void far *txt_buf;		/* +0x000 Texte 0x21e (txt_load_list) */
	char far * far *txt;		/* +0x004 */
	Pic pic;			/* +0x008 Bildobjekt (Hintergrund 0x222) */
	unsigned char far *img;		/* +0x0ae Kartenbild 320 x 200 */
	unsigned char far *tiles;	/* +0x0b2 Kachelbilder (ntiles * tile_size) */
	char far *note;			/* +0x0b6 Notiztext am Cursor (0x200 Bytes) */
	unsigned char far *tile_idx;	/* +0x0ba Feldtyp -> laufende Kachelnummer (256 Bytes) */
	signed short old_fg;		/* +0x0be gesicherte Schriftfarben */
	signed short old_bg;		/* +0x0c0 */
	signed short w;			/* +0x0c2 Breite der Karte */
	signed short h;			/* +0x0c4 Hoehe */
	signed short ntiles;		/* +0x0c6 Zahl der Kacheln */
	signed short zoom;		/* +0x0c8 1/2/4 Felder je Kachel */
	signed short tile_size;		/* +0x0ca tw * th */
	signed short tw;		/* +0x0cc Kachelbreite */
	signed short th;		/* +0x0ce Kachelhoehe */
	signed short vx;		/* +0x0d0 Ausschnitt (Feld links oben) */
	signed short vy;		/* +0x0d2 */
	signed short cx;		/* +0x0d4 Cursor (Feld) */
	signed short cy;		/* +0x0d6 */
	signed short cinfo;		/* +0x0d8 Feldinfo am Cursor */
	signed short vw;		/* +0x0da Ausschnitt in Pixeln */
	signed short vh;		/* +0x0dc */
	signed short ox;		/* +0x0de Bildversatz */
	signed short oy;		/* +0x0e0 */
	signed short last_x;		/* +0x0e2 letzte Cursorposition (Pixel) */
	signed short last_y;		/* +0x0e4 */
	signed short seen_id;		/* +0x0e6 geladene Aufdeckkarte (Datei 0x407), -1 = keine */
	signed long handle;		/* +0x0e8 Speicherblock (XMS bzw. EMS bei ems) */
	signed short is_open;		/* +0x0ec */
	signed short see_through[5];	/* +0x0ee durchsichtige Feldtypen fuer R08 reveal_map (Ende -1; set_town_type) */
	signed char swapped;		/* +0x0f8 Bildschirm ausgelagert (swap_save_file) */
	signed char arc_marked;		/* +0x0f9 arc_mark(g_file_3) gesetzt */
	signed char ems;		/* +0x0fa Speicherblock im EMS */
	signed short gx;		/* +0x0fb Marke der Gruppe (Pixel), -1 = keine */
	signed short gy;		/* +0x0fd */
	signed short other_x[6];	/* +0x0ff Marken der anderen Gruppen */
	signed short other_y[6];	/* +0x10b */
	signed short cur_x;		/* +0x117 Cursor in Pixeln, -1 = keiner */
	signed short cur_y;		/* +0x119 */
	struct map_tile far *cells;	/* +0x11b Karte w * h */
	Bmp far *mark_own[6];		/* +0x11f Animationsbilder der eigenen Gruppe (Bild 0x1a3 Nr. 12..17) */
	Bmp far *mark_other[6];		/* +0x137 andere Gruppen (Nr. 18..23) */
	Bmp far *bg_own;		/* +0x14f Hintergrund unter der eigenen Marke */
	Bmp far *bg_other[6];		/* +0x153 unter den anderen Marken */
	Bmp far *bg_cursor;		/* +0x16b unter dem Cursor */
	signed char (far *seen)[64];	/* +0x16f Aufdeckkarte 64 x 64 (R08) */
	Bmp far *b173;			/* +0x173 */
	signed short x;			/* +0x177 Reiseziel (travel) */
	signed short y;			/* +0x179 */
	signed short f17b;		/* +0x17b */
	signed short f17d;		/* +0x17d */
	signed short note_file;		/* +0x17f Notizdatei */
	char far *buf;			/* +0x181 Puffer 256 Bytes */
	Edit far *edit;			/* +0x185 Notizfeld */
	struct map_note rec;		/* +0x189 Notizsatz */
	void far *saved_areas;		/* +0x289 gesicherte Mausbereichsliste */

	/* --- O052 ---------------------------------------------------------------------------------------------- */
	Automap();						/* O052_0000 Automap_ctor */
	~Automap();						/* O052_0092 Automap_dtor */
	void open(signed short map_file, signed short note_file, signed short gfx);	/* O052_00F9 */
	void close();						/* O052_0596 */
	void load_cells(signed short id);			/* O052_07FE */
	void load_tiles(signed short id);			/* O052_08A3 */
	void make_bmps();					/* O052_09A2 */
	void load_tile_imgs(signed short id);			/* O052_0AF9 */
	void scroll_up();					/* O052_0D6B */
	void scroll_down();					/* O052_0F2C */
	void scroll_right();					/* O052_111E */
	void scroll_left();					/* O052_1321 */
	void set_town_type(signed short v);			/* O052_14E8 */
	void center();						/* O052_1566 */
	void show();						/* O052_16F7 */
	void draw();						/* O052_1771 */
	void reveal_all();					/* O052_1920 */
	void load_bg();						/* O052_194D */
	void blit(char far *src, signed long hd);		/* O052_1A6F */
	void load_seen();					/* O052_1B44 */
	signed short save();					/* O052_1C5E */
	signed short load();					/* O052_1CB3 */
	void run();						/* O052_1D1E */
	void frame();						/* O052_266D */
	void draw_tile(signed short x, signed short y, signed short t);	/* O052_26FC */
	void cursor_right();					/* O052_27D8 */
	void cursor_down();					/* O052_2830 */
	void cursor_left();					/* O052_2888 */
	void cursor_up();					/* O052_28C7 */
	void update_cursor();					/* O052_2906 */
	void edit_note();					/* O052_2C47 */
	void animate();						/* O052_2CBE */
	void erase_marks();					/* O052_3109 */
	void erase_cursor();					/* O052_326F */
	void clear_notes();					/* O052_3315 */
	void del_note();					/* O052_336F */
	void put_note(struct map_note far *r, signed short i);	/* O052_3416 */
	void get_note(struct map_note far *r, signed short i);	/* O052_3483 */
	char far *find_note();					/* O052_34F0 */
	signed short find_slot();				/* O052_355C */
	void save_note();					/* O052_35FC */
	void load_notes();					/* O052_369F */
	void save_notes();					/* O052_370F */
	/* --- O053 ---------------------------------------------------------------------------------------------- */
	void nop();						/* O053_0000 (leer) */
	signed short travel();					/* O053_0005 */
	signed short path_search(char far *walk, signed char far *buf, signed short far *px, signed short far *py,
		signed short all);				/* O053_0261 */
	void path_trace(char far *out, signed char far *p);	/* O053_04EE */
	void random_cell();					/* O053_05C4 */
	signed char cell_read(signed short id, signed short x, signed short y);	/* O053_091B */
	void cell_write(signed short id, signed short x, signed short y, signed char v);	/* O053_09A7 */
	void init(signed short id);				/* O053_0A38 */
	void free_cells();					/* O053_0A91 */
};

/* --- Travel: Reiseobjekt der Reisekarte (0x11c Bytes, g_travel ds:c0fb; Konstruktor/Destruktor und Rahmen in O038,
 * Reiseteil O209..O223, Ereignisse O164, O017/O113..O127). Die Reisefunktionen sind freie Funktionen mit dem Objekt
 * als erstem Argument (TRAVEL_PTR; C-Module benutzen struct travel gleichen Aufbaus). Begriffe: Weg = Eintrag der
 * Wegtabelle g_routes (zwischen zwei Orten, Wegpunkte points), Route = Ortsliste route[] (Ende 0xff).
 * Felder ohne gesicherte Bedeutung heissen f<offset>. --------------------------------------------------------- */
class Travel {
public:
	signed char _r00[8];
	void far *saved_areas;		/* +0x008 gesicherte Mausbereichsliste (g_mouse_areas) */
	signed char shown[4];		/* +0x00c angezeigte Werte: [0..2] wie status, [TSH_DAY] Tag/Nacht (travel_enter: -1 = neu) */
	signed char status[4];		/* +0x010 Zustandswerte TST_ROAD/TERRAIN/WEATHER/SPEED (Statusbild, Texte O216) */
	signed short bg_x1;		/* +0x014 Hintergrundebene 1 der Reiseanimation (travel_anim) */
	signed short bg_x2;		/* +0x016 Hintergrundebene 2 */
	signed short frame;		/* +0x018 Figur-Teilbild 0..7 */
	signed short anim_tick;		/* +0x01a Takt der Reiseanimation */
	signed short anim_rate;		/* +0x01c Takte je Bild (5 stehend, 2/1 je Tempo) */
	signed short anim_on;		/* +0x01e Reiseanimation laeuft (travel_anim_on/off; travelmap_draw, travel_redraw) */
	signed char _r20[0x4c - 0x20];
	signed char dir_sel;		/* +0x04c Richtungswahl 1..8 (O220, travelmap_select_dir) */
	signed char dir_def;		/* +0x04d Vorgabe (O220; travel_route_loop: Richtung des Weges am Startort) */
	signed short rose_x;		/* +0x04e Kompassrose (O220) */
	signed short rose_y;		/* +0x050 */
	signed short old_rose_x;	/* +0x052 */
	signed short old_rose_y;	/* +0x054 */
	struct dir_set place;		/* +0x056 Ortseintrag (Kopie aus g_places): freie Richtungen place.dirs (O220, O214) */
	signed short evtxt_file;	/* +0x059 Ereignistexte des Ortes: Datei (travel_enter -> g_tev_loc_txt) */
	signed short evtxt_n;		/* +0x05b Anzahl */
	void far *rose_bg_buf;		/* +0x05d Puffer fuer rose_bg (0xa29 Bytes, O220 rose_alloc) */
	void far *rose_buf;		/* +0x061 Puffer fuer rose */
	void far *rose_pics[17];	/* +0x065 Teilbilder der Rose (Datei 0x95): Grundbild, 8 Richtungen frei, 8 gewaehlt */
	Bmp rose_bg;			/* +0x0a9 Hintergrund der Kompassrose (O220) */
	Bmp rose;			/* +0x0b3 Kompassrose */
	signed short sub_scene;		/* +0x0bd Unterszene (Orts-/Reiseereignis, O220: Rose sichtbar; route_choose_dir wertet
					   Eingaben nur dann aus): Texte/Palette bleiben */
	signed char reenter;		/* +0x0bf 0 normal, 1 Kampf laeuft (O212/O217), 3: travel_enter/leave nur Rahmen */
	unsigned char from;		/* +0x0c0 Ort, von dem der aktuelle Weg ausgeht (= group.route_from) */
	unsigned char to;		/* +0x0c1 Ziel des aktuellen Weges (= group.route_to); from == to: am Ort */
	signed char prev_place;	/* +0x0c2 zuletzt erreichter Ort (travel_walk_planned/_simple: alter from; O017 Rueckweg) */
	signed char road;		/* +0x0c3 Wegzustand 0..8 (route.road; Tempofaktor travel_road_factor) */
	signed char weather;		/* +0x0c4 Wetter (Wx_get(g_weather); Tempofaktor travel_weather_factor) */
	unsigned long speed;		/* +0x0c5 Tempo: Strecke je Takt (travel_level: Stufe 0..4, Grenzen 50/200/330/450) */
	signed char hurry;		/* +0x0c9 Eilmarsch: speed x 13/10, g_rest_mod -3 (O217 travel_speed_hurry, nicht mit zu
					   vielen hilflosen Helden; O211) - frueher slow */
	unsigned char route_len;	/* +0x0ca Laenge des Weges (route.len; Strecke = route_len * 1000) */
	unsigned long route_pos;	/* +0x0cb zurueckgelegte Strecke (-> group.route_pos) */
	signed char terrain;		/* +0x0cf Gelaende 0..6 (route.terrain; Begegnungen 0/1/2/5, Naesse 4/5, Tempo 6) */
	unsigned char route[40];	/* +0x0d0 Route: Orte, Ende 0xff */
	signed char action;		/* +0x0f8 Reiseaktion: 1 umkehren (Kampf verloren O212, Fuehrer O164, Rastmenue O217),
					   2 Route ab Ort neu (O217), 3 Umkehr erfolgt (O217 travel_reverse), 4 ohne Rueckfrage weiter */
	signed char danger;		/* +0x0f9 Verfolger: waechst je Weg um route.danger (mit Gegenstand 0x117), ab 20 Ueberfall */
	signed short far *points;	/* +0x0fa Wegpunkte (x, y) des Weges (route_load_points, 500 Bytes) */
	signed short nr_points;		/* +0x0fe Index des letzten Wegpunkts (x) = (Zahl - 1) * 2 */
	signed char f100;		/* +0x100 */
	signed char resume_10;		/* +0x101 Weg bei 1/10 fortsetzen (travel_stage) */
	signed char resume_50;		/* +0x102 bei 1/2 */
	signed char resume_90;		/* +0x103 bei 9/10 */
	signed char mode;		/* +0x104 0 normal, 1 nach travel_leave, 2 Schiff (Tempo 800), 0x0d unterwegs (travel_main) */
	signed char rest;		/* +0x105 Rast angefordert (Menuepunkt bzw. Tageszeit) */
	signed short turn_pos;		/* +0x106 Strecke beim Umkehren (= group.turn_pos) */
	signed char turns;		/* +0x108 Zahl der Umkehrungen (ungerade: Gegenrichtung; = group.turns) */
	signed char f109;		/* +0x109 (O125 place_event_170 setzt 1 mit neuer Route) */
	signed short f10a;		/* +0x10a (O214 setzt; route_choose_dir O218: dann Meldung g_ttxt[26]) */
	signed char ev_half;		/* +0x10c Wegereignis nach der halben Strecke erledigt (O164: 0 bei Fuehrer) */
	signed char ev_start;		/* +0x10d Wegereignis nach 1/30 erledigt */
	signed char ev_end;		/* +0x10e Wegereignis nach 29/30 erledigt */
	signed char planned;		/* +0x10f geplante Route (route_plan) wird abgegangen: travel_walk_planned setzt 1,
					   travel_step setzt damit fort, Route in Farbe 0xbb zeigen (O213/O215) - frueher reverse */
	signed short map_pts[6];	/* +0x110 Kartenpunkte (travelmap_set_point1/2; die Rose O220 meidet sie) */

	Travel();					/* O038_0000 Travel_ctor */
	~Travel();					/* O038_014A Travel_dtor */
	void set_road(signed char v) { road = v; }	/* O038 travel_enter, O215, O216 (Parameterkopie) */
	void set_route_len(unsigned char v) { route_len = v; }
	void set_terrain(signed char v) { terrain = v; }
};

/* --- Town: Ereignisobjekt einer Stadt (0x12 Bytes, Code O183) - Gegenstueck zu struct area_events der Dungeons.
 * O046 legt es je Stadt an (g_town_events), O019 kurzzeitig; desc.handler (g_town_descs[id]) ist der
 * Kartenereignis-Rueckruf der Stadt, z. B. Town::harbor_event (O183) oder harbor2_event (O249). ------------ */
class Town {
public:
	void far *txt_buf;		/* +0x00 Texte (txt_load_list) */
	char far * far *txt;		/* +0x04 */
	signed short id;		/* +0x08 Stadt */
	struct area_desc desc;		/* +0x0a = g_town_descs[id] */

	Town(signed short id, signed short txt_file, signed short txt_n);	/* O183_0000 Town_ctor */
	~Town();						/* O183_0073 Town_dtor */
	void enter(signed short loctype, struct map_event far *ev);	/* O183_00BD Town_go: Ort betreten */
	signed short cell_dir(struct map_event far *ev);	/* O183_011D Drehung der Zelle - 1 */
	signed short harbor_event(struct map_event far *ev);	/* O183_015E Rueckruf der Hafenstadt */
	void market_stall(signed short k);			/* O183_03E0 Marktstand k einrichten */
	void ship_voyage();					/* O183_0460 */
	void show_anim3();					/* O183_0847 */
	void swim_check();					/* O183_08AB */
	/* Stadt mit den Sonderorten (Code O166) */
	signed short sites_event(struct map_event far *ev);	/* O166_0000 town_sites_event: Rueckruf g_town_descs[2] */
	void sites_shock();					/* O166_0435 */
	void sites_hint();					/* O166_0578 */
};

/* --- Sites: Sonderorte (Ereignisorte 1..4) der Stadt aus O166 (8 Bytes, lokale Objekte in O030/O178/O258) ---- */
class Sites {
public:
	void far *txt_buf;		/* +0x00 Texte 0x21a (txt_load_list) */
	char far * far *txt;		/* +0x04 */

	Sites();					/* O166_03A2 Sites_ctor */
	~Sites();					/* O166_03EB Sites_dtor */
	void inn_vision();				/* O166_05A5 (O178 Herberge) */
	void talk();					/* O166_0813 (O258) */
	void night_encounter();				/* O166_0A91 */
	void challenge();				/* O166_0D3B */
	void rematch();					/* O166_1313 */
	void evening_fight();				/* O166_1687 */
	void explore();					/* O166_1A37 */
};

/* --- Keylist: Schluesselliste fuer den CD-Cache (0x0c Bytes, Code O003; Schluessel = Bildnummer | Typ) ------------ */
class Keylist {				/* Code O003 */
public:
	signed short far *keys;		/* +0x00 Schluessel (max + 1, Ende -1) */
	unsigned short far *states;	/* +0x04 2-Bit-Zustaende (cache_state_set) */
	signed short count;		/* +0x08 */
	signed short max;		/* +0x0a */

	Keylist(signed short max);			/* O003_0EC8 Keylist_ctor */
	~Keylist();					/* O003_0F63 Keylist_dtor */
	signed short add(signed short nr, unsigned short type);	/* O003_0FAD keylist_add (0 = voll) */
	void load(void far *cache);			/* O003_1038 keylist_load: mit -1 abschliessen, in den Cache laden */
	void clear();					/* O003_107D keylist_clear */
};

/* --- Ev: Ereignisorte 5..28 der belagerten Stadt Lowangen (8 Bytes, Code O031; Texte LOWANGEN.LTX = Datei 0x21c, wie
 * die Ereignisroutinen in O184). O030 legt das Objekt beim Betreten eines Ereignisortes (Ortsart 17, g_loc_id = n)
 * an und ruft je n eine Methode; O186 benutzt offering_bowl/bridge_toll. Freie Namen ev_* in s_proto.h. -------- */
class Ev {
public:
	void far *txt_buf;		/* +0 Textpuffer (Datei 0x21c) */
	char far * far *txt;		/* +4 Texte */

	Ev();						/* O031_0000 Ev_ctor */
	~Ev();						/* O031_0049 Ev_dtor */
	void market_hall();				/* O031_0093 ev_market_hall: Ort 5 */
	void magistrate();				/* O031_00E0 ev_magistrate: Ort 6 */
	void eolan();					/* O031_0149 ev_eolan: Ort 7 */
	void house_low49();				/* O031_0212 ev_house_low49: Ort 8 */
	void academy();					/* O031_0230 ev_academy: Ort 9 */
	void smithy();					/* O031_024E ev_smithy: Ort 10 */
	void showhouse();				/* O031_0266 ev_showhouse: Ort 11 */
	void guardhouse();				/* O031_06A6 ev_guardhouse: Ort 12 */
	void city_gate1();				/* O031_0870 ev_city_gate1: Ort 13 */
	void city_gate2();				/* O031_08A7 ev_city_gate2: Ort 14 */
	void fortress_gate();				/* O031_08DE ev_fortress_gate: Ort 15 */
	void trade_office();				/* O031_0A2E ev_trade_office: Ort 16 */
	void altrogolan();				/* O031_0A73 ev_altrogolan: Ort 17 */
	void ailian();					/* O031_0AB1 ev_ailian: Ort 18 */
	void vindaria();				/* O031_0C56 ev_vindaria: Ort 19 */
	void jandora();					/* O031_0D1E ev_jandora: Ort 20 */
	void dragan();					/* O031_0D53 ev_dragan: Ort 21 */
	void talk_49();					/* O031_0F62 ev_talk_49: Ort 22 */
	void zaskia();					/* O031_0FD7 ev_zaskia: Ort 23 */
	void talk_4a();					/* O031_1039 ev_talk_4a: Ort 24 */
	void peddler_redhead();				/* O031_10AE ev_peddler_redhead: Ort 25 */
	void falkert();					/* O031_1130 ev_falkert: Ort 26 */
	void deringorn();				/* O031_11B9 ev_deringorn: Ort 27 */
	void offering_bowl();				/* O031_1216 ev_offering_bowl: Ort 28 (frueher Ev_f1216) */
	void bridge_toll();				/* O031_1350 ev_bridge_toll (O186; frueher Ev_f1350) */
	/* Ereignisroutine in O185 (aus O046 bei g_gavron_chase > 0) */
	void gavron_chase();				/* O185_0F25 ev_gavron_chase: Verfolgung Gavrons (LOWANGEN.LTX[89..110]) */
	/* Ereignisroutinen in O184 (Namen wie Welle N17) */
	void tribute();					/* O184_0000 ev_tribute */
	signed short story_13();			/* O184_0230 ev_story_13 */
	void talk_90();					/* O184_03FF ev_talk_90 */
	void show_pic_197();				/* O184_0479 ev_show_pic_197 */
	signed short count_sneak_fails(signed char mod);	/* O184_0560 ev_count_sneak_fails */
	void choice_49();				/* O184_05F7 ev_choice_49 */
	void choice_56();				/* O184_0778 ev_choice_56 */
	void wait_noon();				/* O184_08C3 ev_wait_noon */
};

/* --- Smith: Schmiede (O230, 0x56 Bytes; Reparatur O231, freie Funktionen smith_*), Zeiger g_smith (BE seg058 do_smith/
 * repair_screen): Oeffnungszeit 6..20 Uhr, Hausverbot g_smith_banned[g_loc_id], Gespraech, Gegenstand zur Reparatur
 * abgeben und spaeter abholen (g_smith_repairs). ---------------------------------------------------------------- */
class Smith {
public:
	signed short mouse_x;		/* +0x00 letzte Mauslage (grid_input) */
	signed short mouse_y;		/* +0x02 */
	signed short cur;		/* +0x04 markierter Held (Platz, 6 = NSC) */
	signed short old_cur;		/* +0x06 */
	signed short sel;		/* +0x08 gewaehltes Rasterfeld 0..14 */
	signed short old_sel;		/* +0x0a */
	signed short page;		/* +0x0c erster Listeneintrag der Seite (0, 15 ...) */
	signed short count;		/* +0x0e Helden mit reparierbaren Gegenstaenden */
	void far *txt_buf;		/* +0x10 Texte 0x1d (txt_load_list) */
	char far * far *txt;		/* +0x14 */
	signed short done;		/* +0x18 Schmiede verlassen */
	signed short price_mod;		/* +0x1a Preisaufschlag in % (Satz g_loc_id der Datei 0x202, 4 Bytes) */
	signed char f1c;		/* +0x1c */
	signed char female;		/* +0x1d Schmiedin (Bild 0x18a statt 0x2b, Texte 38..41) */
	struct smith_repair far *repair;	/* +0x1e = &g_smith_repairs[g_loc_id] */
	struct repair_item far *items;	/* +0x22 Auswahlliste (31 Eintraege, 0xd9 Bytes) */
	void far *icon_buf;		/* +0x26 Puffer 0x240 (Geldsymbol 24 x 24, read_record 0x12) */
	struct hero far *heroes[7];	/* +0x2a Helden mit reparierbaren Gegenstaenden */
	signed short cell_x[3];		/* +0x46 Raster: Spalten bzw. (draw_grid) Zeilen */
	signed short cell_y[5];		/* +0x4c */

	Smith();					/* O230_0000 Smith_ctor */
	~Smith();					/* O230_014D Smith_dtor */
	signed short check_open();			/* O230_01C7 smith_check_open: 0 = geschlossen/Hausverbot */
	void run();					/* O230_03C5 smith_run */
	void ban();					/* O230_05A7 smith_ban: Hausverbot, verlassen */
	void check_closing(signed long t);		/* O230_05BE smith_check_closing */
	void talk();					/* O230_0667 smith_talk */
	void pickup();					/* O230_073C smith_pickup */
	signed short grid_input(signed short far *redraw);	/* O230_087C smith_grid_input */
};

/* --- Tavern: Schenke (O240; Daten struct tavern in s_struct.h, weitere Methoden als freie Funktionen tavern_* in
 * O241/O242 mit struct tavern far *) ---------------------------------------------------------------------------- */
class Tavern : public tavern {
public:
	Tavern();					/* O240_0000 Tavern_ctor */
	~Tavern();					/* O240_00F6 Tavern_dtor */
	signed short admit();				/* O240_014F tavern_admit: Oeffnungszeit, Hausverbot, Zustand */
	void run();					/* O240_01A7 tavern_run */
	void set_crowd(signed long t);			/* O240_0574 tavern_set_crowd: g_tavern_crowd 1..3 */
	void check_ban();				/* O240_069E tavern_check_ban */
	void check_closing(signed long t);		/* O240_07CA tavern_check_closing: Sperrstunde */
	void drink_test(signed short drank);		/* O240_08FB tavern_drink_test: Zechprobe */
	void hourly_event();				/* O240_0AF3 tavern_hourly_event */
	signed short eat();				/* O240_0BB4 tavern_eat: Essen fuer alle bezahlen */
	void order();					/* O240_0E51 tavern_order */
	void add_bill(signed short kind);		/* O240_113B tavern_add_bill */
};

/* --- Bildfolgen mit Untertiteln O039 Intro, O040 Anim3, O041 Outro1/Outro2 (je 0x24 Bytes, lokale Objekte:
 * Konstruktor, run, Destruktor; O183/O222/O007). Basis struct text_screen (O181, tscr_*): Schrift sichern,
 * Bildschirm loeschen. Eigene Schrift font (new Font(1), Farbsatz 0xe2/0xba/0xff) und eigene Textliste. ------- */
class Intro : public text_screen {
public:
	Font far *font;			/* +0x18 Schrift der Untertitel */
	void far *texts_buf;		/* +0x1c Textliste 0xa9 (txt_load_list) */
	char far * far *texts;		/* +0x20 */

	Intro();					/* O039_004D Intro_ctor_o039 */
	~Intro();					/* O039_0137 Intro_dtor_o039 */
	void run();					/* O039_01D0 Intro_run_o039 */
};

class Anim3 : public text_screen {
public:
	Font far *font;			/* +0x18 Schrift der Untertitel */
	void far *texts_buf;		/* +0x1c Textliste 0x408 */
	char far * far *texts;		/* +0x20 */

	Anim3();					/* O040_0000 */
	~Anim3();					/* O040_00D0 */
	char far *cue_text(unsigned short n, signed short far *snd);	/* O040_014E: Text und Sprachnummer zur Marke n */
	void play(signed short k, signed short ani, unsigned char col, signed short pic, signed short x, signed short y,
		signed short mode);			/* O040_03FB: eine Animation mit Untertiteln */
	void blank(unsigned char far *pal);		/* O040_0BC4: Text loeschen, Grundbild */
	void run();					/* O040_0C7E */
};

class Outro1 : public text_screen {
public:
	Font far *font;			/* +0x18 */
	void far *texts_buf;		/* +0x1c (Outro1 laedt die Texte in run selbst) */
	char far * far *texts;		/* +0x20 */

	Outro1();					/* O041_0000 */
	~Outro1();					/* O041_00AB */
	void run();					/* O041_10DC */
	void plain();					/* O041_1106: ohne Sprachausgabe */
	void voice();					/* O041_16E1: mit Sprachausgabe */
};

class Outro2 : public text_screen {
public:
	Font far *font;			/* +0x18 */
	void far *texts_buf;		/* +0x1c Textliste 0xa9 */
	char far * far *texts;		/* +0x20 */

	Outro2();					/* O041_00F5 */
	~Outro2();					/* O041_018B */
	void run();					/* O041_0209 */
	void plain();					/* O041_0233 */
	void voice();					/* O041_08BB */
};

/* --- Init: Startobjekt (leer), Code in SEG005 (C++) -------------------------------------------------- */
class Init {
public:
	Init();						/* R05_01A5 Init_ctor: Speicher pruefen/reservieren, Archive laden */
	~Init();					/* R05_0588 Init_dtor: Temp-Dateien loeschen, Speicher freigeben */
	void setdir();					/* R05_0525 Init_setdir: Startverzeichnis merken, ins Datenverzeichnis */
};

/* --- CopyProt: Handbuchabfrage (Kopierschutz, 0x4b Bytes), Objekt g_copyprot (ds:c6ca), Code in SEG003 (C++).
 * Fragt nach Seite/Zeile/Wort im Handbuch (PROTECT.LTX, Saetze aus PASSWORD.DAT); in der CD-Fassung wird die
 * Abfrage selbst (query) nirgends aufgerufen, nur die Pruefung copyprot_check (game_idle) bleibt wirksam. ----- */
struct pw_rec {				/* Satz aus PASSWORD.DAT (25 Bytes; Datei: Wort Anzahl, dann die Saetze) */
	signed char word;		/* +0x00 Wort in der Zeile */
	signed char line;		/* +0x01 Zeile */
	signed char page;		/* +0x02 Handbuchseite */
	signed char key1;		/* +0x03 Schluessel 1 (Startwert der XOR-Kette) */
	signed char key2;		/* +0x04 Schluessel 2 (Startwert der Summe) */
	unsigned char code[20];		/* +0x05 verschluesselte Antwort (siehe CopyProt::encrypt) */
};

class CopyProt {
public:
	void far *txt_buf;		/* +0x00 Textpuffer PROTECT.LTX (txt_load_list) */
	char far * far *txt;		/* +0x04 Zeigerliste: [0] = "Bitte schlagen Sie ... Seite %d Zeile %d Wort %d" */
	signed char f8;			/* +0x08 Fuellwerte von set_f8 (nie gelesen) */
	signed char f9;			/* +0x09 */
	signed char fa;			/* +0x0a */
	signed char _r0b[3];
	unsigned char answer[20];	/* +0x0e Eingabe des Spielers, nach encrypt verschluesselt */
	signed short key1;		/* +0x22 = rec.key1 */
	signed short key2;		/* +0x24 = rec.key2 */
	struct pw_rec rec;		/* +0x26 gezogener Satz */
	signed short i;			/* +0x3f Schleifenzaehler von compare (Element!) */
	signed short rec_nr;		/* +0x41 Nummer des gezogenen Satzes */
	signed short tries;		/* +0x43 Versuche (hoechstens 3) */
	signed short nr_recs;		/* +0x45 Zahl der Saetze (Dateikopf) */
	signed short ok;		/* +0x47 != 0: Abfrage bestanden bzw. nicht noetig (Konstruktor: 3..21) */
	signed short ok2;		/* +0x49 zweiter Merker (init: 16, finish: 0) */

	CopyProt();					/* R03_0306 CopyProt_ctor: ok = Zufall 3..21 */
	~CopyProt();					/* R03_03FC CopyProt_dtor */
	void query(signed short chance);		/* R03_0041 Abfrage mit Wahrscheinlichkeit 1/chance (CD: unbenutzt) */
	void encrypt();					/* R03_0128 Eingabe verschluesseln, ok setzen */
	void compare();					/* R03_018C mit rec.code vergleichen */
	signed short get_ok();				/* R03_01F5 */
	void init();					/* R03_0203 Merker vorbelegen, Texte laden */
	signed short get_ok2();				/* R03_0256 */
	void fail();					/* R03_0264 Spiel beenden */
	void load_texts();				/* R03_0347 PROTECT.LTX laden */
	void finish();					/* R03_036D Texte frei, Datei zu */
	signed short rnd(signed short n);		/* R03_03D0 random_schweif(n) */
	signed short rnd_interval(signed short lo, signed short hi);	/* R03_03E2 random_interval */
	void seek_record();				/* R03_0421 zufaelligen Satz anfahren */
	void clear_answer();				/* R03_0465 */
	void ask();					/* R03_0480 Frage stellen (Eingabe 20 Zeichen) */
	void take_answer();				/* R03_04D4 Eingabe nach answer */
	void open_file();				/* R03_04EF PASSWORD.DAT oeffnen, Satzzahl lesen */
	void read_record();				/* R03_051C Satz lesen, Schluessel uebernehmen */
	void set_f8(signed short v);			/* R03_0558 Fuellwerte f8..fa (ohne Wirkung) */
};

#endif

/* --- Shop: Laden/Haendler (O226, 0x47 Bytes), Objekt *g_shop (ds:72cc); Verkaufen O227, Kaufen O228, Raster O229.
 * Datenteil als struct shop auch fuer C (O229); Prototypen der freien Funktionen mit SHOP_PTR. ------------------ */
struct shop {
	signed short mx;		/* +0x00 letzte Mausposition (Raster shop_grid_input) */
	signed short my;		/* +0x02 */
	signed short hero;		/* +0x04 Verkauf: gewaehlter Held 0..6 (6 = NSC) */
	signed short old_hero;		/* +0x06 Verkauf: zuletzt markierter Held (-1 = keiner) */
	signed short sel;		/* +0x08 Auswahl im Raster 0..14 */
	signed short old_sel;		/* +0x0a markierte Auswahl (-1 = keine) */
	signed short base;		/* +0x0c erste angezeigte Ware (Seiten zu 15) */
	signed short count;		/* +0x0e Zahl der Waren */
	signed short done;		/* +0x10 Laden verlassen */
	void far *txt_buf;		/* +0x12 Ladentexte 0x7d (txt_load_list) */
	char far * far *txt;		/* +0x16 */
	struct shop_info info;		/* +0x1a Ladendaten (13 Bytes) */
	struct ware far *wares;		/* +0x27 Warenliste (g_nr_items Eintraege) */
	signed short (far *sell_qty)[31];	/* +0x2b Verkauf: Menge je Held (7) und Inventarplatz (O227) */
	signed short far *buy_qty;	/* +0x2f Kauf: Menge je Gegenstand (O228) */
	void far *pic_buf;		/* +0x33 Bildpuffer 0x240 (Bild der Ladenleiste, read_record) */
	signed short grid_x[3];		/* +0x37 Raster: Spalten x */
	signed short grid_y[5];		/* +0x3d Raster: Zeilen y */
};

#ifdef __cplusplus
class Shop : public shop {
public:
	Shop();						/* O226_0000 Shop_ctor: Texte, Ladendaten, Puffer, g_shop = this */
	~Shop();					/* O226_0154 Shop_dtor */
	void leave();					/* O226_01CE shop_leave */
	signed short is_open();				/* O226_01DC shop_is_open: Uhrzeit, Hausverbot */
	void run();					/* O226_0314 shop_run: Hauptschleife */
	void check_time(signed long t);			/* O226_0573 shop_check_time: Ladenschluss */
	void talk();					/* O226_0602 shop_talk */
	void build_wares();				/* O226_07AC shop_build_wares */
	void add_ware(signed short id, signed short idx, signed char slot);	/* O226_0A0D shop_add_ware */
	void format_money(char far *buf, signed long m);	/* O226_0B0C shop_format_money */
	signed short trade(signed long price, signed char mode);	/* O226_0B52 shop_trade (1 Kauf, 2 Verkauf) */
	void steal();					/* O226_102A shop_steal */
};

/* --- Sel: Auswahlliste der Suchbegriffe des Tagebuchs (O225, 0x24 Bytes; new/delete in O103 Suche) ------------
 * Datei 0x334: struct sel_rec bis id == -1; Rahmenbild 0x333 (154 x 117). run liefert (Stufe << 8) + id bzw. -1. */
/* struct sel_rec (Datensatz der Datei 0x334) steht in s_struct.h (auch O104/O105 Tagebuch) */
struct sel_item {			/* 0x16 Bytes, Listeneintrag */
	char name[20];			/* +0x00 */
	unsigned char id;		/* +0x14 */
	signed char lvl;		/* +0x15 */
};

class Sel {
public:
	signed short n;			/* +0x00 Zahl der Eintraege */
	signed short cur;		/* +0x02 Auswahl */
	signed short top;		/* +0x04 erste angezeigte Zeile (10 Zeilen) */
	Bmp far *bmp;			/* +0x06 Rahmenbild 0x9a x 0x75 */
	struct sel_item far *items;	/* +0x0a Liste (350 Eintraege) */
	struct sel_rec far *recs;	/* +0x0e Datei 0x334 */
	signed short bar_h;		/* +0x12 Rollbalken: Hoehe */
	signed short bar_y;		/* +0x14 Rollbalken: Lage */
	char far *title;		/* +0x16 */
	char far *input;		/* +0x1a Tippsuche (20 Bytes) */
	void far *buf;			/* +0x1e Bildpuffer 0x9240 */
	signed short done;		/* +0x22 */

	Sel();						/* O225_0032 Sel_ctor */
	~Sel();						/* O225_0184 Sel_dtor */
	signed short run(char far *title, signed char lo, signed char hi);	/* O225_020A Sel_run */
	signed short fill(signed char lo, signed char hi);	/* O225_0268 sel_fill */
	signed short loop();				/* O225_037A sel_loop */
	void draw();					/* O225_08BB sel_draw */
	void scrollbar();				/* O225_0AEC sel_scrollbar */
	void flash();					/* O225_0CFA sel_flash */
	void type_search();				/* O225_0DDD sel_type_search */
};
#endif

/* --- Store: Lagerhaus (O012, 0xb2 Bytes, lokales Objekt in O030 bei LOC_WAREHOUSE). Drei Lagerhaeuser (g_loc_id
 * 1..3) teilen sich die Datei DEPOT.DAT in ..\TEMP: je ein struct depot und dessen Gegenstaende (inv_slot). Einlagern
 * O099 (store_deposit), Abholen O100 (store_fetch); fuer beide gilt ein Lagerschein ITEM_DEPOT_TICKET_1..3. -------- */
struct depot {				/* 0x16 Bytes, Lagerplatz eines Lagerhauses (Satz der Datei DEPOT.DAT) */
	signed short state;		/* +0x00 0 offen, 1/2 nach Diebstahl/Verlust geschlossen (store_lock) bis checked */
	signed long value;		/* +0x02 Wert der Gegenstaende (price * price_unit * Menge); Gebuehr je Tag value / 300 + 1 */
	signed short nr_items;		/* +0x06 Zahl der Gegenstaende (je 0x1b Bytes) */
	struct timestamp stored;	/* +0x08 zuletzt ein-/ausgelagert (Gebuehr und Verderb nach Tagen) */
	struct timestamp checked;	/* +0x0f Ende der Sperre bzw. Bezug der Diebstahlpruefung (store_lock: jetzt + 1..6 Monate; Pruefung 30 Tage danach) */
};

struct store {
	signed short mx;		/* +0x00 letzte Mausposition (store_grid_input) */
	signed short my;		/* +0x02 */
	signed short hero;		/* +0x04 Einlagern: gewaehlter Held 0..6 (6 = NSC) */
	signed short old_hero;		/* +0x06 Einlagern: zuletzt markierter Held */
	signed short sel;		/* +0x08 Auswahl im Raster 0..14 */
	signed short old_sel;		/* +0x0a markierte Auswahl (-1 = keine) */
	signed short base;		/* +0x0c erste angezeigte Ware (Seiten zu 15) */
	signed short count;		/* +0x0e Zahl der Waren */
	signed short grid_x[3];		/* +0x10 Raster: Spalten x */
	signed short grid_y[5];		/* +0x16 Raster: Zeilen y */
	signed short nr_sel;		/* +0x20 Zahl der gewaehlten Posten (Einlagern: in new_items) */
	signed long value;		/* +0x22 Einlagern: neuer Gesamtwert, Abholen: Wert des Rests (-> depot.value) */
	signed short done;		/* +0x26 Lagerhaus verlassen */
	void far *txt_buf;		/* +0x28 Texte 0x22/0x26 (txt_load_list) */
	char far * far *txt;		/* +0x2c */
	void far *pic_buf;		/* +0x30 Bildpuffer 0x240 (Gesicht, read_record 0x30/0x31) */
	signed char far *sel_qty[7];	/* +0x34 Einlagern: gewaehlte Menge je Held und Inventarplatz (je 31 Bytes) */
	signed char far *take_qty;	/* +0x50 Abholen: gewaehlte Menge je Gegenstand (256 Bytes, -1 = verloren) */
	struct ware far *wares;		/* +0x54 Warenliste (0x700 Bytes, 256 Eintraege; ware.slot = Platz) */
	struct inv_slot far *rest;	/* +0x58 Abholen: verbleibende Gegenstaende (store_save Art 1) */
	struct inv_slot far *new_items;	/* +0x5c Einlagern: neue Gegenstaende (store_save Art 0); O100 liest ab hier
					   (&new_items)[g_loc_id] = items[g_loc_id - 1] */
	struct inv_slot far *items[3];	/* +0x60 Gegenstaende je Lagerhaus (0x1b00 Bytes = 256 Plaetze) */
	struct depot far *depot;	/* +0x6c aktueller Lagerplatz = &depots[g_loc_id - 1] */
	struct depot depots[3];		/* +0x70 */
};

#ifdef __cplusplus
class Store : public store {
public:
	Store();					/* O012_0000 Store_ctor: Dateiname DEPOT.DAT, Texte, Puffer, Raster */
	~Store();					/* O012_010D Store_dtor */
	signed short is_open();				/* O012_01E6 store_is_open: Uhrzeit und Gebuehr (ohne Aufrufer) */
	void run();					/* O012_0271 store_run: Hauptschleife */
	signed short can_enter();			/* O012_0458 store_can_enter: Diebstahl/Sperre, Uhrzeit, Gebuehr */
	signed short check_time(signed long t);		/* O012_064D store_check_time: Ladenschluss -> done */
	void lock(signed short k);			/* O012_0695 store_lock: Sperre 1/2 (6 bzw. 3 Monate, sonst 1 Monat bis zur naechsten Pruefung) */
	signed short check_fee();			/* O012_06EF store_check_fee: Lagergebuehr anzeigen (zu wenig Geld: 0) */
	void load();					/* O012_0848 store_load: DEPOT.DAT lesen */
	void save(signed short mode);			/* O012_0981 store_save: 0 nach Einlagern, 1 nach Abholen, 2 alles */
	void create_file();				/* O012_0B49 store_create_file */
	void grid_mark();				/* O012_0BD8 store_grid_mark: Auswahlrahmen und Name */
	void talk();					/* O012_0D2A store_talk: Gespraech 0x9d..0x9f je Lagerhaus */
	signed short grid_input(signed short far *redraw);	/* O012_0D7B store_grid_input: Maus und Pfeiltasten */
};
#endif

/* --- Peddler: fahrender Haendler aus einem Gespraech (O069, 0x3a Bytes, lokales Objekt in O031 Ev::f10ae/f1130).
 * Ladendaten stehen nicht im Objekt, sondern in g_peddler..g_peddler_female (ds:c812..c823, O013 setzt sie je
 * Haendler 1..3, O031 nullt den Block nach dem Handel). Kaufen O068 peddler_buy_screen (Gegenstueck zu O228
 * shop_buy_screen); er verkauft hoechstens g_peddler_stock Plaetze, dann zieht er weiter. -------------------------- */
struct peddler {
	signed short mx;		/* +0x00 letzte Mausposition (peddler_grid_input) */
	signed short my;		/* +0x02 */
	signed char _r04[4];
	signed short sel;		/* +0x08 Auswahl im Raster 0..14 */
	signed short old_sel;		/* +0x0a markierte Auswahl (-1 = keine) */
	signed short base;		/* +0x0c erste angezeigte Ware (Seiten zu 15) */
	signed short count;		/* +0x0e Zahl der Waren */
	signed short sold;		/* +0x10 verkaufte Plaetze (Vergleich mit g_peddler_stock) */
	signed short done;		/* +0x12 Handel beendet */
	signed short sold_out;		/* +0x14 ausverkauft: run endet, g_peddler_away = 3 */
	struct ware far *wares;		/* +0x16 Warenliste (g_nr_items Eintraege) */
	signed short far *buy_qty;	/* +0x1a Kauf: Menge je Gegenstand (O068) */
	void far *pic_buf;		/* +0x1e Bildpuffer 0x240 (read_record 0x16) */
	void far *txt_buf;		/* +0x22 Ladentexte 0x7d (txt_load_list) */
	char far * far *txt;		/* +0x26 */
	signed short grid_x[3];		/* +0x2a Raster: Spalten x */
	signed short grid_y[5];		/* +0x30 Raster: Zeilen y */
};

#ifdef __cplusplus
class Peddler : public peddler {
public:
	Peddler();					/* O069_0000 Peddler_ctor: Texte, Warenliste, Puffer, Raster */
	~Peddler();					/* O069_00F2 Peddler_dtor */
	void run();					/* O069_0163 peddler_run: Menue Handeln/Ende */
	void build_wares();				/* O069_034A peddler_build_wares */
	void add_ware(signed short id, signed short idx);	/* O069_054A peddler_add_ware */
	signed short trade(signed long price);		/* O069_0633 peddler_trade: Feilschen, Waren verteilen, bezahlen */
	signed short grid_input(signed short far *redraw);	/* O069_0905 peddler_grid_input */
};
#endif

/* --- Lodging: Rasthaus/Gasthof LOC_LODGING (O070, 0x14 Bytes): Essen (1 Stunde) oder Uebernachten bis 8 Uhr; Preis
 * 10 Heller + Aufschlag je Held (Essen) bzw. das Zehnfache (Uebernachten). Datensatz g_loc_id der Datei 0x1fe. ---- */
struct lodging_info {			/* 4 Bytes, Datensatz der Datei 0x1fe */
	signed char regen;		/* +0 Zuschlag in % auf die Erholung beim Uebernachten */
	signed short markup;		/* +1 Aufschlag in % auf den Grundpreis 10 */
	signed char _r3;
};

struct lodging {
	void far *txt_buf;		/* +0x00 Texte 0x195 (txt_load_list) */
	char far * far *txt;		/* +0x04 */
	signed long price;		/* +0x08 Preis */
	signed short done;		/* +0x0c */
	signed short pic;		/* +0x0e Bild 0x191 */
	struct lodging_info info;	/* +0x10 */
};

#ifdef __cplusplus
class Lodging : public lodging {
public:
	Lodging();					/* O070_0000 Lodging_ctor: Texte, Datensatz g_loc_id aus Datei 0x1fe */
	~Lodging();					/* O070_00B2 Lodging_dtor */
	signed short is_open();				/* O070_00FC lodging_is_open: Hausverbot g_lodging_banned -> 0 */
	void run();					/* O070_016C lodging_run */
	void ask_price();				/* O070_02D3 lodging_ask_price: Preis nennen, Rueckfrage */
	void eat();					/* O070_042C lodging_eat: satt, 1 Stunde */
	void sleep();					/* O070_047D lodging_sleep: bis 8 Uhr, Erholung */
	void pay();					/* O070_0560 lodging_pay: bezahlen bzw. Hausverbot */
};
#endif

/* --- Temple: Tempel LOC_TEMPLE (O044, 0x11 Bytes, Zeiger g_temple; entspricht DSA1 seg061), Wunder O033. Die
 * Tempelfunktionen sind freie Funktionen temple_* mit dem Objekt als erstem Parameter. ------------------------- */
struct temple {
	signed short god;		/* +0x00 Gottheit 1..14 (temple_find_god ueber g_temple_god_locs) */
	BMP_PTR pic;			/* +0x02 Tempelbild 0x2b x 0x1d (NVF 0x11, Bild god - 1) */
	void far *txt_buf;		/* +0x06 Texte 0x16 (txt_load_list) */
	char far * far *txt;		/* +0x0a */
	signed short done;		/* +0x0e Tempel verlassen */
	signed char kind;		/* +0x10 Art (Byte g_loc_id der Datei 0x205): Bild 0x188/0x27, Texte txt[0x15 + kind],
					   txt[24]/[25], O033 txt[37]/[38] (vermutet Tempel/Schrein) */
};

#ifdef __cplusplus
class Temple : public temple {
public:
	Temple();					/* O044_0000 Temple_ctor: Bild, Texte, g_temple = this */
	~Temple();					/* O044_0084 Temple_dtor */
};

/* SaveMenu und CharGen: vollstaendig weiter unten (Welle N18) */

#endif

/* --- Healer: Heiler LOC_HEALER (O174, 0x0e Bytes; DSA1 seg062 Heiler). Datensatz g_loc_id der Datei 0x1ff. ---- */
struct healer_info {			/* 5 Bytes, Datensatz der Datei 0x1ff */
	signed short markup;		/* +0 Preisaufschlag in % */
	signed char skill;		/* +2 Koennen: Erfolg Krankheit W100 <= 120 - skill * 10 + Modifikator, Gift - skill * 5 */
	signed char kind;		/* +3 Art: 1 Bild 0x28, sonst 0x29 */
	signed char stone;		/* +4 heilt Versteinerung (Menuepunkt), Erfolg bei stone < W20 */
};

struct healer {
	unsigned char state;		/* +0x00 0 neu, 1 besucht, 2 nachts besucht (doppelter Preis) */
	struct healer_info info;	/* +0x01 */
	void far *txt_buf;		/* +0x06 Texte 0x19 (txt_load_list) */
	char far * far *txt;		/* +0x0a */
};

#ifdef __cplusplus
class Healer : public healer {
public:
	Healer();					/* O174_0000 Healer_ctor: Texte, Datensatz g_loc_id */
	~Healer();					/* O174_00A5 Healer_dtor */
	signed short is_open();				/* O174_00F0 healer_is_open: immer 1 */
	void run();					/* O174_00FA healer_run */
	void cure_wounds();				/* O174_0409 healer_cure_wounds: LE und le_mod */
	void cure_disease();				/* O174_06D0 healer_cure_disease (g_disease_prices) */
	void cure_poison();				/* O174_09D3 healer_cure_poison (g_poison_prices) */
	signed long price(signed long cost);		/* O174_0CBF healer_price: + markup %, nachts doppelt */
	void cure_stone();				/* O174_0D08 healer_cure_stone: Versteinerung (5000) */
	void talk();					/* O174_0F1F healer_talk: Gespraech je Heiler (script_load) */
};
#endif

/* --- Chest: Truhe (O078, 0x55 Bytes; +0..+0x33 = Datensatz nr der Datei 0x9d). Gegenstueck zu Lock (O108), aber
 * eigener Aufbau: Geld und zehn Gegenstaende. Chest(nr, 1) oeffnet sofort (Auswahl Aufbrechen/Dietrich/Foramen). - */
struct chest_item {			/* 4 Bytes */
	signed short id;		/* +0 Gegenstand (ITEM_MONEY: Geld in 100 Heller), -1 = Ende */
	unsigned char qty;		/* +2 Anzahl */
	signed char _r3;
};

#ifdef __cplusplus
class Chest;
typedef signed short (Chest::*CHEST_PMF)();	/* Methodenzeiger, 8 Bytes */

class Chest {
public:
	signed char _r00[2];
	unsigned short money;		/* +0x02 Geld in 10 Heller */
	signed short key;		/* +0x04 Schluessel (Gegenstand; nur Truhenart 11/13) */
	signed char special;		/* +0x06 Sonderbehandlung statt Auswahl (Index g_chest_special), -1 = keine */
	signed char fail_special;	/* +0x07 Folge eines Fehlschlags (Index g_chest_fail, Fallen), -1 = keine */
	unsigned char tries;		/* +0x08 Versuche (ab 10 Hinweis; Fallenprobe -tries) */
	signed char locked;		/* +0x09 verschlossen */
	signed char type;		/* +0x0a Truhenart (Index g_chest_types; 15 = immer Sonderbehandlung) */
	signed char msg;		/* +0x0b Meldung beim ersten Oeffnen (show_msg) */
	struct chest_item items[10];	/* +0x0c */
	signed short nr;		/* +0x34 Satznummer */
	signed char force;		/* +0x36 Aufbrechen: noetige KK (cur + mod) */
	signed char pick_mod;		/* +0x37 Zuschlag Schloesserknacken */
	signed char spell_mod;		/* +0x38 Zuschlag Foramen */
	CHEST_PMF special_func;		/* +0x39 Sonderbehandlung (g_chest_special_default bzw. [special]) */
	CHEST_PMF fail_func;		/* +0x41 Fehlschlag (g_chest_fail_default bzw. [fail_special]) */
	void far *txt_buf;		/* +0x49 Texte 0x9e (txt_load_list) */
	char far * far *txt;		/* +0x4d */
	struct hero far *hero;		/* +0x51 handelnder Held */

	signed short trap_2w6();			/* O078_0000 chest_trap_2w6: beim 1. Versuch 2W6 Schaden */
	signed short trap_3w6();			/* O078_0094 chest_trap_3w6: beim 1. Versuch 3W6 */
	signed short trap_always();			/* O078_0128 chest_trap_always: immer 2W6 */
	signed short trap_poison();			/* O078_0195 chest_trap_poison: Magier beim 1. Versuch vergiftet */
	signed short trap_5w6();			/* O078_0265 chest_trap_5w6: bei Versuch 1..2 5W6 */
	signed short special_key();			/* O078_02F9 chest_special_key: Schluessel 0x111 und Gegenstand 0x78 */
	signed short special_quest();			/* O078_04B7 chest_special_quest: nur bei g_eolan_state == 2 */
	Chest(signed short nr, signed short open);	/* O078_0585 Chest_ctor: Satz lesen, bei open == 1 oeffnen */
	~Chest();					/* O078_07E4 Chest_dtor: Satz zurueckschreiben */
	void init();					/* O078_088E chest_init */
	signed short try_open();			/* O078_09A1 chest_try_open: 1 offen, 0 misslungen, -1 abgebrochen */
	signed short pick();				/* O078_0BBC chest_pick: Dietrich, Schloesser-Probe */
	signed short count_items();			/* O078_0CB0 chest_count_items */
	signed short foramen();				/* O078_0CD9 chest_foramen */
	signed short force_open();			/* O078_0E13 chest_force_open: Aufbrechen (KK) */
	void take_items();				/* O078_0EBF chest_take_items: Auswahl, Menge, Geld */
	signed short list_items(char far *buf);		/* O078_117F chest_list_items: Zeilen zu 40 Bytes */
	void show_msg(signed char k);			/* O078_128C chest_show_msg */
	void set_item(signed short id, unsigned char qty, signed short k);	/* O078_12D7 chest_set_item */
};

extern CHEST_PMF g_chest_special[];	/* ds:59c3 Sonderbehandlungen (special_key, special_quest) */
extern CHEST_PMF g_chest_fail[];	/* ds:59d3 Fehlschlag-Folgen (Fallen) */
extern CHEST_PMF g_chest_special_default;	/* ds:59fb (0) */
extern CHEST_PMF g_chest_fail_default;	/* ds:5a03 (0) */
#endif

/* --- Inn: Herberge LOC_INN (O178, 0x16 Bytes, Zeiger g_inn; DSA1 seg068/069), Ortsereignisse O179. Buchung in
 * g_inn_room (Kategorie 1..3), g_inn_days, g_inn_town, g_inn_loc, g_inn_price. Datensatz g_loc_id der Datei 0x200. -- */
struct inn_info {			/* 6 Bytes, Datensatz der Datei 0x200 */
	signed short markup;		/* +0 Preisaufschlag in % */
	signed short quality;		/* +2 Qualitaet (Essen: Texte, Saettigung 21 - quality; Schlaf: Tage +-1) */
	signed char f04;		/* +4 */
	signed char kind;		/* +5 Art: Bild/Klang 0x189 bzw. 0x2a, txt[55]/[54] */
};

struct inn {
	void far *txt_buf;		/* +0x00 Texte 0x1a (txt_load_list) */
	char far * far *txt;		/* +0x04 */
	signed short nr_icons;		/* +0x08 Menuepunkte 4 bzw. 7 (mit Zimmer) */
	signed short special;		/* +0x0a Sonderfall Ort 2..3 (g_gashok_arrested, g_guide_avail) */
	signed short done;		/* +0x0c */
	signed short redraw_icons;	/* +0x0e Leiste neu */
	struct inn_info info;		/* +0x10 */
};

#ifdef __cplusplus
class Inn : public inn {
public:
	Inn();						/* O178_0000 Inn_ctor: g_inn = this, Texte, Datensatz g_loc_id */
	~Inn();						/* O178_00B7 Inn_dtor */
	signed short is_open();				/* O178_010A inn_is_open: geschlossen (g_inn_closed) -> Meldung, 0 */
	void run();					/* O178_01E1 inn_run */
	void eat();					/* O178_0679 inn_eat */
	void room();					/* O178_0985 inn_room: Zimmer buchen/stornieren */
	void sleep();					/* O178_0E82 inn_sleep: Naechte, Diebstahl, Sonderereignisse */
	signed short wait_midnight();			/* O178_1440 inn_wait_midnight: Zeit bis 0 Uhr, Abbruch per Taste */
	signed short wait_morning();			/* O178_1509 inn_wait_morning: Zeit bis 8 Uhr */
	void leave();					/* O178_15D2 inn_leave */
	/* O179 (Menuepunkte und Diebstahl; O178 ruft die freien Namen inn_talk(this) usw.) */
	void talk();					/* O179_0000 inn_talk: Gespraech mit dem Wirt je g_loc_id */
	void apply_skill();				/* O179_0115 inn_apply_skill: Talent anwenden (Hausverbot bei -1) */
	void apply_magic();				/* O179_0188 inn_apply_magic: Magie anwenden */
	void theft_morning();				/* O179_029C inn_theft_morning: bestohlen (einfache Herberge) */
	signed short theft_night();			/* O179_0403 inn_theft_night: Dieb in der Nacht, 1 = Verlust */
};
#endif

/* --- Camp: Lager/Rast (O071, 0x19 Bytes; aus O030 fuer LOC_CAMP_WILD/_TOWN/_DUNGEON als run(0/1/2), aus O173 per
 * new/run(1)/delete). Rast, Wachen, Ueberfall, Regeneration und Lagermenue in O072 (freie Funktionen camp_*, auch
 * mit dem Kontext der Reiselager O067/O073). Wachen: g_camp_watch_names[3], Auftraege g_camp_job_names[3]. -------- */
struct herb_find {			/* 4 Bytes, g_gather_herbs_table (ds:5826, BE gather_herbs_table) */
	unsigned char id;		/* +0 Kraut (Gegenstand), 0xff = Ende */
	unsigned char chance;		/* +1 Fundchance in % (Ortskraut g_loc_herb +10) */
	unsigned char max;		/* +2 hoechstens so viele (Ortskraut +1) */
	signed char diff;		/* +3 Zuschlag der Probe TA_PFLANZENKUNDE */
};

struct camp_encounter {			/* 8 Bytes, g_camp_encounters (ds:5884): naechtlicher Ueberfall (O072 camp_set_foes) */
	signed short id;		/* +0 Gegnerart */
	signed short count;		/* +2 Anzahl (x 10 / (W11 + 9) bzw. / (W10 + 10)) */
	signed short f4;		/* +4 */
	signed short f6;		/* +6 */
};

struct camp_foe {			/* 4 Bytes: Gegner einer naechtlichen Begegnung (g_camp_foes, O067) */
	signed short id;		/* +0 Gegneraufstellung (start_fight) */
	signed short count;		/* +2 Staerke: g_fight_max_enemies = count - W(count) * k */
};

struct camp {
	void far *txt_buf;		/* +0x00 Texte 0xae (txt_load_list) */
	char far * far *txt;		/* +0x04 */
	signed short searched;		/* +0x08 heute schon von Hand gesucht (dann keine Auftraege bei der Rast) */
	signed short job;		/* +0x0a laufender Auftrag: 1 Kraeuter, 2/3 Nahrung (camp_rest_jobs); O072: Wachen angefordert */
	signed char site;		/* +0x0c 99 = guter Lagerplatz (Suche +99); camp_rest setzt 1 */
	signed char food_tries;		/* +0x0d Nahrungssuchen (hoechstens 2) */
	signed short herbs_done;	/* +0x0e Kraeuter gesucht */
	signed char mode;		/* +0x10 0 Wildnis (LOC_CAMP_WILD), 1 Stadt, 2 Dungeon; O072: 0 mit Wachen/Ueberfall */
	signed short pic;		/* +0x11 Bild 0xaf/0xb0 (Tag/Nacht) bzw. 0xb1/0xb2 */
	signed short penalty;		/* +0x13 Folge der Rast (O072): 99 Krankheit (group_add_effect), -1..-3 Geldverlust */
	signed short done;		/* +0x15 */
	signed short watch_set;		/* +0x17 Wachen eingeteilt */
};

#ifdef __cplusplus
class Camp : public camp {
public:
	Camp();						/* O071_0000 Camp_ctor */
	~Camp();					/* O071_006C Camp_dtor: g_leave_location = 0 */
	signed short run(signed char mode);		/* O071_0097 camp_run */
	void menu_wild();				/* O071_027E camp_menu_wild: Hauptmenue in der Wildnis */
	void menu_rest();				/* O071_08E0 camp_menu_rest: Rasten in Stadt/Dungeon */
	void choose_watch();				/* O071_0C40 camp_choose_watch: drei Wachen */
	void find_food();				/* O071_0E0C camp_find_food: Wasser und Jagd */
	void gather_herbs();				/* O071_13AD camp_gather_herbs (BE gather_herbs) */
	void use_skill();				/* O071_19A4 camp_use_skill */
	void reset_watch(signed char n, signed char keep);	/* O071_19E3 camp_reset_watch */
	void nop();					/* O071_1AA8 camp_nop (leer) */
	void eat_drink();				/* O071_1AAD camp_eat_drink: Hungrige/Durstige essen und trinken */
	void menu_jobs();				/* O071_1D56 camp_menu_jobs: Wachen, Nahrung, Kraeuter, loeschen */
	void clear_jobs();				/* O071_1FA5 camp_clear_jobs */
	signed short rest_jobs();			/* O071_1FC3 camp_rest_jobs: Rast mit Auftraegen (ab 12 Uhr) */
};
#endif

/* ======================================================================================================
 * Kampf (Welle N7): Animationsobjekt FigAnim (O016/O112), Figurenliste FigList (O153), Kampfobjekt Fight (O018).
 * Fuer C-Module dieselben Daten als struct fig_anim / struct fig_list / struct fight; Prototypen benutzen die
 * Makros FIGANIM_PTR, FIGLIST_PTR und FIGHT_PTR (in C++ die Klassen, in C die Strukturen - gleicher Code).
 * ====================================================================================================*/
#ifdef __cplusplus
class FigAnim : public fig_anim {	/* 0xe0 Bytes, Code O016 (Methoden) und O112 (freie Funktionen fig_anim_*, seq_*) */
public:
	FigAnim(struct ani_seq far *src, signed short started);	/* O016_0000 Anim_ctor_o016: XMS-Platz belegen */
	~FigAnim();					/* O016_009E Anim_dtor_o016 */
	void load_seq();				/* O016_0195 fig_anim_load_seq: Folgenkopf des Schritts laden */
	void load_frame();				/* O016_02C4 fig_anim_load_frame: Einzelbild laden/entpacken */
	void find_seq(signed short n, signed short id);	/* O016_07E7 fig_anim_find_seq: Folgensatz id suchen */
	signed char next_frame();			/* O016_0869 fig_anim_next_frame: weiterschalten -> Ereignis */
	void unpack(Bmp far *b, void far *src);		/* O016_09C9 fig_anim_unpack: decomp nach b */
};
class FigList;
class Fight;
#define FIGANIM_PTR FigAnim far *
#define FIGLIST_PTR FigList far *
#define FIGHT_PTR Fight far *
#else
#define FIGANIM_PTR struct fig_anim far *
#define FIGLIST_PTR struct fig_list far *
#define FIGHT_PTR struct fight far *
#endif

struct fig_node {			/* 0x1f Bytes: Kampffigur in der Figurenliste (BE FIGHTER, dort 0x23 Bytes) */
	signed char cbx;		/* +0x00 Feld x */
	signed char cby;		/* +0x01 Feld y */
	signed short figure;		/* +0x02 Figurbild (Held fig_hero_pic, Gegner gfx_id + 0xb5, Objekt 0x91) (BE NVF_NO) */
	unsigned char obj_pic;		/* +0x04 Feldobjekt: Bildnummer (Index g_fig_obj_bmps); O144: Bildnummer im Zeichenprotokoll */
	signed char offsetx;		/* +0x05 Bildversatz bzw. Bildschirmlage beim Laufen */
	signed char offsety;		/* +0x06 */
	unsigned char id;		/* +0x07 Figur-ID (FigList.used) */
	signed char z;			/* +0x08 Sortierung bei gleichem Feld (0x63 Figuren, 0x32 Objekte, 10 Schwanz), -1 = vorn */
	signed char visible;		/* +0x09 */
	signed char kind;		/* +0x0a 0 Feldobjekt, 1 Gegner, 2 Held (BE IS_ENEMY) */
	unsigned char twofielded;	/* +0x0b Zweifeldfigur: Kopf n (<= 20), Schwanz n + 20; 0xff = keine */
	unsigned char obj_id;		/* +0x0c Feldinhalt vor dem Betreten (BE OBJ_ID) */
	FIGANIM_PTR anim;		/* +0x0d Animationsobjekt */
	BMP_PTR gfx;			/* +0x11 Standbild (Feldobjekte) */
	struct fig_node far *next;	/* +0x15 */
	struct fig_node far *prev;	/* +0x19 */
	signed char to_remove;		/* +0x1d beim naechsten fig_list_purge entfernen */
	signed char to_resort;		/* +0x1e beim naechsten fig_list_purge neu einsortieren (Lage geaendert) */
};

struct fig_list {			/* 0x0c Bytes: Figurenliste (Kopfknoten cbx = cby = -99), im Kampfobjekt +0x35 */
	struct fig_node far *head;	/* +0x00 */
	signed char _r04[4];
	unsigned char far *used;	/* +0x08 Belegung je ID (0xff Bytes) */
};

#ifdef __cplusplus
class FigList : public fig_list {	/* Code O153; freie Namen fig_list_* in s_proto.h (this = erstes Argument) */
public:
	FigList();					/* O153_0000 FigList_ctor */
	~FigList();					/* O153_00DC FigList_dtor */
	void clear();					/* O153_0134 */
	void mark_remove(unsigned char id);		/* O153_01AF */
	void mark_resort(unsigned char id);		/* O153_01F1 */
	struct fig_node far *last();			/* O153_0233 */
	void hide(unsigned char id);			/* O153_0267 */
	void show(unsigned char id);			/* O153_0303 */
	void resort();					/* O153_039F */
	void purge();					/* O153_03F0 */
	signed short exists(unsigned char id);		/* O153_0500 */
	struct fig_node far *find(unsigned char id);	/* O153_0541 */
	struct fig_node far *find_anim(void far *anim);	/* O153_0587 */
	struct fig_node far *find_twofield(unsigned char tf);	/* O153_05CF */
	void getpos(unsigned char id, signed short far *x, signed short far *y);	/* O153_0615 */
	unsigned char alloc_id();			/* O153_066A */
	void remove(unsigned char id, signed char keep);	/* O153_06A0 */
	unsigned char add(unsigned char id);		/* O153_076E */
};
#endif

#ifdef __cplusplus
/* --- FightMsgs: Meldungsliste des Kampfes (struct fight_msgs, Objekt g_fight_list), Code O175; freie Namen Lst_* in
 * s_proto.h (this = erstes Argument). Welle N15. */
class FightMsgs : public fight_msgs {
public:
	FightMsgs();					/* O175_0000 Lst_ctor */
	~FightMsgs();					/* O175_005C Lst_dtor */
	void set_screen(signed long p);			/* O175_00AB Lst_set (Zielpuffer, als long uebergeben) */
	void init();					/* O175_00BD Lst_init */
	void add(signed short id, signed short value, signed char type1, void far *obj1, signed char type2,
		void far *obj2);			/* O175_014B Lst_add */
	void add2(signed short id, signed short value, signed char type, void far *obj);	/* O175_01F7 Lst_add2 */
	void add0(signed short id, signed short value);	/* O175_02A0 Lst_add0 */
	void run();					/* O175_0344 Lst_run */
};
#endif

/* --- Kampfobjekt (0xfb1 Bytes): new Fight(nr, Seite) in start_fight (O018), Zeiger g_fight_obj; die Kampf-
 * module bekommen es als ersten Parameter. Unbenannte Bereiche: f<offset>/_r<offset>. -------------------- */
#ifdef __cplusplus
class Fight {
public:
#else
struct fight {
#endif
	void far *buf;			/* +0x000 Bildpuffer 0xfa00 (Kampffeld ohne Figuren; = g_fight_buf) */
	void far *txt_buf;		/* +0x004 Texte 0x25 */
	char far * far *txt;		/* +0x008 */
	char far *msg;			/* +0x00c Meldungspuffer 0xc8 Bytes (O158..O161: Zaubertext, nach der Animation ausgegeben) */
	unsigned char far *pal;		/* +0x010 Palette 0x300 Bytes (NVF 0x4a) */
	signed short swapped;		/* +0x014 Speicher knapp: Auslagerung aktiv (swap_save_file/swap_alloc) */
	struct fig_node tmpl;		/* +0x016 Vorlage fuer fig_list_add (g_fig_tmpl) */
#ifdef __cplusplus
	FigList list;			/* +0x035 Figurenliste (g_fig_list) */
#else
	struct fig_list list;
#endif
	struct ani_seq seq;		/* +0x041 Ablaufbeschreibung fuer neue Animationsobjekte */
	struct anim_view far *intro_ani;	/* +0x0e3 Start-/Schlussanimation (O051, Bild 0x8b/0x8c/0x8d) */
	BMP_PTR intro_bg;		/* +0x0e7 Hintergrund unter der Animation */
	BMP_PTR bar_bg[7];		/* +0x0eb Hintergrund je Portraetplatz (28 x 37) */
	BMP_PTR heads[7];		/* +0x107 Kopfbild je Held (16 x 16) */
	BMP_PTR pic_dead;		/* +0x123 Zustandsbild tot (16 x 16 aus NVF 5, Bild nvf5_count - 12; O148) */
	BMP_PTR pic_unable;		/* +0x127 Zustandsbild kampfunfaehig/blind (16 x 16 aus Datei 6, Versatz 0x487; O148) */
	BMP_PTR pic_fleeing;		/* +0x12b Zustandsbild flieht (hero.scared; Datei 6, Versatz 0x7747; O148) */
	BMP_PTR pic_unarmed;		/* +0x12f Zustandsbild ohne (heile) Waffe (Datei 6, Versatz 0x3847; O148) */
	BMP_PTR status_bg;		/* +0x133 Bildschirm ab (0x40, 0xa0), 200 x 14 */
	BMP_PTR bar_tmp;		/* +0x137 28 x 37: Portraetplatz der Heldenleiste (O148 zusammensetzen, ausgeben) */
	BMP_PTR sums_tmp;		/* +0x13b 200 x 14: Summenbalken der Gegner (O148) */
	BMP_PTR tmp32;			/* +0x13f Arbeitsbild 32 x 32 */
	BMP_PTR tmp24;			/* +0x143 Arbeitsbild 24 x 24 */
	signed short enemy_le_sum;	/* +0x147 Summe der Gegner-LE (O148_01CE) */
	signed short enemy_ae_sum;	/* +0x149 Summe der Gegner-AE (O148_021C) */
	signed short shown_le;		/* +0x14b angezeigte LE-Summe der Gegner (O148 fig_draw_hero_bar) */
	signed short shown_ae;		/* +0x14d angezeigte AE-Summe */
	signed short shown_le_max;	/* +0x14f angezeigter Hoechstwert (enemy_le_sum) */
	signed short shown_ae_max;	/* +0x151 */
	signed short shown_hero_le[7];	/* +0x153 angezeigte LE je Portraetplatz */
	signed short shown_hero_ae[7];	/* +0x161 angezeigte AE je Portraetplatz */
	signed short head_no[7];	/* +0x16f Kopfbildnummer je Held (O076_0B70) */
	BMP_PTR shown_status[7];	/* +0x17d angezeigtes Zustandsbild je Portraetplatz (pic_dead.., 0 = keins) */
	signed char _r199[0x19d - 0x199];
	signed short style_tmp;		/* +0x19d Zwischenwert des Kampfstils (O154 fig_menu_attack: hero.attack_style) */
	signed short pic_w;		/* +0x19f Breite (O134: auch als Hoehe uebergeben) */
	signed short old_dlg_mode;	/* +0x1a1 vorheriger Dialogmodus (dlg_set16, O022/O143) */
	signed short nvf5_count;	/* +0x1a3 Bildzahl der NVF-Datei 5 */
	signed char _r1a5[0x1ab - 0x1a5];
	signed short nr_dropped;	/* +0x1ab abgelegte Gegenstaende (BE FIG_DROPPED_COUNTER) */
	signed short dropped[30];	/* +0x1ad abgelegte Gegenstaende, nach dem Kampf zurueck (0 = Ende) (BE FIG_DROPPED_WEAPONS) */
	signed char _r1e9;
	signed short initiative;	/* +0x1ea wer beginnt: 1 Helden, 2 Gegner, sonst zufaellig (Argument von start_fight) */
	signed short id;		/* +0x1ec Kampfnummer (0x493/0x495 Sonderkaempfe, 0x494 ohne Gegner) */
	signed short round;		/* +0x1ee Kampfrunde */
	signed char atk_twice;		/* +0x1f0 Angriffsbild zweimal (O135/O136/O161) */
	signed char def_strikes;	/* +0x1f1 Verteidiger schlaegt zurueck: Angriffsbild nach der Parade (O135/O136) (vermutet) */
	signed char atk_hit;		/* +0x1f2 Angreifer wird getroffen: Bild 0x26 am Ende (O135/O136) (vermutet) */
	BMP_PTR pic_cursor;		/* +0x1f3 Feldcursor (22 x 11, NVF 0x47 Bild 0x7c; O145/O150) */
	BMP_PTR pic_mark;		/* +0x1f7 Markierung der Figur am Zug (Bild 0x7d; O148 fig_set_mark) */
	BMP_PTR pic_reach;		/* +0x1fb erreichbares Feld (Bild 0x80; O145 Umkreismarken) */
	void far *f1ff;			/* +0x1ff Inhalt der Datei 0x49 (O022) */
	struct hero far *top_hero;	/* +0x203 Anzeige oben links: Held am Zug */
	struct enemy far *top_enemy;	/* +0x207 bzw. Gegner am Zug */
	struct hero far *bottom_hero;	/* +0x20b Anzeige unten: Ziel-Held */
	struct enemy far *bottom_enemy;	/* +0x20f bzw. Ziel-Gegner */
	FIGANIM_PTR anims[30];		/* +0x213 Animationsobjekte der Figuren */
	FIGANIM_PTR action_anim;	/* +0x28b Animation der laufenden Aktion (Warteschleife bis seq_done; O021/O141/O158..O162) */
	FIGANIM_PTR action_anim2;	/* +0x28f zweite Animation der Aktion (Verteidiger; O158..O161) */
	signed char nr_anims;		/* +0x293 belegte anims[] */
	signed char anim_removed;	/* +0x294 Figur entfernt (O133 Aktion 32) */
	signed char mark_id;		/* +0x295 Markierungsfigur (vor jeder Aktion entfernt), -1 = keine */
	signed char mark_id2;		/* +0x296 Feldcursor bei der Ziel-/Bewegungsauswahl (O145/O150), -1 = keiner */
	signed char reach_marks[0x90];	/* +0x297 Figur-IDs der Umkreismarken (O145_0D7C), Ende -1 (O018: -1) */
	signed char path_marks[0x345 - 0x327];	/* +0x327 Figur-IDs der Bahnmarken (O145_0D7C, 30) */
	signed char f345;		/* +0x345 (O018: -1) - Endmarke hinter path_marks */
	signed char f346;		/* +0x346 (O018: -1) */
	signed char _r347[0x35c - 0x347];
	signed char nr_twofield;	/* +0x35c Zahl der Zweifeldgegner (BE FIG_TWOFIELDED_COUNT) */
	struct point los_line[30];	/* +0x35d Feldpunkte der Sichtlinie (bresenham_line), Ende (-1, -1) */
	unsigned char far *search_map;	/* +0x3d5 Kopie der Begehbarkeitskarte fuer die Wegsuche (O145/O146, Zielmarken) */
	signed char keys[7];		/* +0x3d9 Tasten gedrueckt (O144 fight_check_keys), [6] = Kampf automatisch */
	struct fig_rect draw_tab[255];	/* +0x3e0 zuletzt gezeichnete Figuren (Ende id 0xff) */
	/* +0xed5..+0xfac: Kampfsatz der Datei 0x8e (216 Bytes, fight_read_rec/fight_write_rec; BE FIGHT.LST) */
	char name[0x13];		/* +0xed5 Name des Kampfes */
	signed char intro_done;		/* +0xee8 Einleitung erledigt (aus fight_def; sonst einmal fight_intro_text) */
	signed short scenario;		/* +0xee9 Kampffeld: Satz der Datei 0x8f (fight_read_scenario; BE FIGHT_SCENARIO) */
	struct fight_monster monsters[20];	/* +0xeeb Gegner der Aufstellung */
	struct fight_hero_pos heroes[7];	/* +0xf4f Startplaetze der Helden ([6] NSC) */
	signed short loot[30];		/* +0xf6b Beute (O140 kopiert 20) */
	signed short ducats;		/* +0xfa7 Geld der Beute */
	signed short silver;		/* +0xfa9 */
	signed short heller;		/* +0xfab */
	struct fight_field far *field;	/* +0xfad Kampffeld (0x26d Bytes) */
#ifdef __cplusplus
	Fight(signed short id, signed short initiative);	/* O018_0000 Fight_ctor */
	~Fight();					/* O018_069D Fight_dtor */
	signed short run();				/* O018_09A5 fight_run */
#endif
};

/* --- Gegner (Welle N11): Klasse Enemy zu struct enemy (0x49 Bytes, g_enemies[20]), Code O192. Die Methoden
 * heissen frei enemy_* (s_proto.h, this = erstes Argument); Gegnerzauber als lokales Objekt Spell (O194). ------ */
#ifdef __cplusplus
class Enemy;
class Spell;
typedef void (Spell::*SPELL_PMF)();		/* Methodenzeiger, 8 Bytes */

/* Zauber eines Gegners (O194, 0x1e Bytes), lokal in enemy_cast_spell (O192); Gegenstueck zu Cast (O233).
 * cast(n) ruft die Zauberroutine g_mon_spell_handlers[n] (MSP_*, BE mspell_*), Ergebnis wie Cast (SPELL_RES_*). */
class Spell {
public:
	void far *txt_buf;		/* +0x00 Zaubertexte (Datei 0x1f, 300; Schnellkampf: g_b690) */
	char far * far *txt;		/* +0x04 */
	signed short result;		/* +0x08 SPELL_RES_* bzw. AE-Kosten */
	SPELL_PMF func;			/* +0x0a Zauberroutine (g_mon_spell_handlers[n]) */
	Enemy far *caster;		/* +0x12 zaubernder Gegner */
	Enemy far *target;		/* +0x16 Ziel-Gegner */
	struct hero far *hero;		/* +0x1a Ziel-Held */

	Spell(struct enemy far *e);			/* O194_0048 Spell_ctor */
	~Spell();					/* O194_0106 Spell_dtor */
	void damage(signed short le);			/* O194_0150 mon_spell_damage (BE MON_do_spell_damage) */
	signed short target_pa();			/* O194_0239 mon_spell_target_pa (BE MON_get_target_PA) */
	signed short target_rs();			/* O194_0295 mon_spell_target_rs (BE MON_get_target_RS) */
	signed short cast(signed short n);		/* O194_02ED mon_spell_cast (BE MON_cast_spell) */
	void mon_spell_verwandlung_beenden();		/* O194_0523  1 */
	void mon_spell_bannbaladin();			/* O194_0638  2 */
	void mon_spell_boeser_blick();			/* O194_06F0  3 */
	void mon_spell_horriphobus();			/* O194_0800  4 */
	void mon_spell_axxeleratus();			/* O194_08EE  5 */
	void mon_spell_balsam();			/* O194_0980  6 */
	void mon_spell_blitz();				/* O194_0A0E  7 */
	void mon_spell_eisenrost();			/* O194_0AC5  8 */
	void mon_spell_fulminictus();			/* O194_0C00  9 */
	void mon_spell_ignifaxius();			/* O194_0CBB 10 */
	void mon_spell_plumbumbarum();			/* O194_0FC3 11 */
	void mon_spell_saft_kraft();			/* O194_10D8 12 */
	void mon_spell_armatrutz();			/* O194_117E 13 */
	void mon_spell_paralue();			/* O194_11EA 14 */
};

extern SPELL_PMF g_mon_spell_handlers[];	/* ds:23ad Zauberroutine je Gegnerzauber MSP_* (15; [0] = 0) - kein eigenes
						   Public (_g_mon_spell_repertoire + 0x41) */

class Enemy : public enemy {
public:
	Enemy();					/* O192_0000 Enemy_ctor (leer) */
	~Enemy();					/* O192_0028 Enemy_dtor (leer) */
	char far *name();				/* O192_004D enemy_name */
	void load_pic(void far *dst);			/* O192_0074 enemy_load_pic */
	signed short is_gfx_19_3f();			/* O192_00AF Untote (Teil von g_gfx_undead), gegen Geisteszauber immun */
	signed short is_gfx_1b();			/* O192_00D1 (Untoter; Ignifaxius doppelt, O236) */
	signed short is_gfx_1f();			/* O192_00E9 (Untoter) */
	signed short is_gfx_15();			/* O192_0101 (ohne Aufrufer) */
	signed short is_gfx_17_29();			/* O192_0119 (Klang 0x67) */
	signed short is_gfx_24_18();			/* O192_013B (Trefferklang 0x66) */
	signed short is_gfx_25_35_36_23();		/* O192_015D (Trefferklang 0x7f) */
	signed short is_gfx_2a();			/* O192_0193 (Klang 0x68) */
	signed short is_gfx_1c();			/* O192_01AB (Klaenge 0x7d/0x76) */
	signed short is_gfx_42();			/* O192_01C3 (Klaenge 0x7e/0x8a) */
	signed short is_gfx_33_mon_7a();		/* O192_01DB (ohne Aufrufer) */
	signed short is_fire();				/* O192_01FC Feuerwesen (gfx 0x2b): RS zaehlt nicht, Feuerbann halbiert */
	signed short type_check();			/* O192_0214 enemy_type_check: Gegnerart 0x33 */
	signed short is_humanoid();			/* O192_022B Kampfgrafik in g_gfx_humanoid */
	signed short is_undead();			/* O192_0254 Kampfgrafik in g_gfx_undead */
	signed short is_dead();				/* O192_027D */
	void damage_range(signed short w, signed short far *lo, signed short far *hi);	/* O192_029B */
	struct hero far *target_hero();			/* O192_0355 enemy_target_hero */
	void set_target_hero(struct hero far *h);	/* O192_0387 */
	Enemy far *target_enemy();			/* O192_03B4 enemy_target_enemy */
	void set_target_enemy(struct enemy far *e);	/* O192_03E6 */
	void sub_le(signed short n, signed short keep_renegade);	/* O192_0413 enemy_sub_le */
	void sub_ae(signed short n);			/* O192_047B */
	signed short attack_damage(void far *target, signed short vs_enemy);	/* O192_0557 (BE FIG_get_enemy_attack_damage) */
	signed short range_damage(struct hero far *h);	/* O192_0741 Fernangriff (Schuss/Wurf) gegen einen Helden */
	signed short test_attrib3(signed short a1, signed short a2, signed short a3, signed char mod);	/* O192_084F */
	signed short test_spell(signed short n, signed char mod);	/* O192_08BB */
	signed short cast_spell(signed char mod);	/* O192_0998 enemy_cast_spell */
	signed short range_kind();			/* O192_0AEB enemy_range_kind */
	signed char at_value();				/* O192_0B09 enemy_attack_value */
	signed char pa_value();				/* O192_0BB1 enemy_parade_value */
	signed short blocks();				/* O192_0C6F enemy_blocks */
};
#endif

/* ===== Welle N20: Programmstart, Vorspann, Spielende, Klang, CD, Optionen ===================================== */

/* --- Opening: Vorspann beim Programmstart (O027, 0x0c Bytes; new/run/delete in main O032, ohne Destruktor). Bei
 * g_speech_ext_avail die FLI-Fassung opening_play_fli (O028, C: struct opening). Bildspeicher fuer scene5/scroll_text
 * im XMS bzw. EMS (store_*). ------------------------------------------------------------------------------------ */
struct opening {
	signed short done;		/* +0x00 abgebrochen (Taste) */
	signed short ems;		/* +0x02 Bildspeicher im EMS (sonst XMS; vom Konstruktor nicht gesetzt) */
	signed long handle;		/* +0x04 Handle des Bildspeichers, -1 = keins */
	signed short swapped;		/* +0x08 Archiv ausgelagert (swap_save_file, arc_mark) */
	signed short kb;		/* +0x0a Groesse des Bildspeichers in KB (0x140 bzw. 0x40) */
};
#ifdef __cplusplus
class Opening : public opening {
public:
	Opening();					/* O027_0000 Opening_ctor */
	void run();					/* O027_004C Opening_run */
	void play();					/* O027_00DC: scene1..scene8 bis done */
	void scene1();					/* O027_0190 Bild 0x41b */
	void scene2();					/* O027_030D Bild 0x41c */
	void scene3();					/* O027_04E0 Bild 0x41d mit Wellen (opening_scene3, O028) */
	void scene4();					/* O027_06A1 Titelbild 0x436 */
	void scene5();					/* O027_076A Animation 0x437..0x43b */
	void scene6();					/* O027_0A41 Bilder 0x41e..0x420/0x429 mit Schriftzuegen */
	void scene7();					/* O027_0CC9 Bild 0x421 */
	void scene8();					/* O027_0E69 Bilder 0x422..0x428/0x43c */
	void scroll_text(signed short n);		/* O027_10F7 Schriftzug 0x42a + n durchs Bild schieben */
	void waves(unsigned char far *src);		/* O027_1323 Wellen-Effekt */
	void copy_line(void far *dst, void far *src);	/* O027_149B eine Bildzeile (Assembler) */
	void store_put(unsigned long src, signed long n);	/* O027_14B3 Bild n in den Bildspeicher */
	void store_get(unsigned long dst, signed long n);	/* O027_150C Bild n aus dem Bildspeicher */
	void store_alloc();				/* O027_1565 Bildspeicher anfordern (XMS, sonst EMS) */
	void store_free();				/* O027_164D */
	void blend(unsigned char far *from, unsigned char far *to, signed short steps, signed short n,
		signed short f);			/* O027_16B5 pal_blend2, solange nicht done */
	void fade_in(unsigned char far *buf);		/* O027_16E0 dissolve_in (opening_fade_in, O028) */
	void wait(signed short n);			/* O027_16FC n Bildwechsel warten (poll_key) */
	signed short poll_key();			/* O027_1722 Taste: Alt-Q beendet, sonst done = 1 */
	signed short load_files();			/* O027_177A Titelbild, Dateiliste in den CD-Cache; 1 = Esc */
};
#endif

/* --- Finale: Spielende-Sequenz (O035, 0x22 Bytes; lokal in finale_play O045): Titelbild, neun Szenen bzw. das
 * Skript g_finale_script (O036, FLI-Fassung bei g_speech_ext_avail), Meldungen, Spielstandmenue. C-Sicht
 * struct finale (O036: txt, O045: lokales Objekt); Prototypen der freien Namen mit FINALE_PTR. -------------- */
#ifdef __cplusplus
class Finale {
public:
	void far *txt_buf;		/* +0x00 Texte 0x3b1 (load_list in run; vom Konstruktor nicht gesetzt) */
	char far * far *txt;		/* +0x04 */
	unsigned char far *pal_saved;	/* +0x08 beim Anlegen gesicherte Palette (get_palette) */
	unsigned char far *pal_from;	/* +0x0c Ausgangspalette der Blenden (nicht initialisiert) */
	unsigned char far *pal_pic;	/* +0x10 Palette des Szenenbilds */
	Pic far *pic;			/* +0x14 */
	Bmp bmp;			/* +0x18 Szenenbild */

	Finale();					/* O035_0000 Finale_ctor */
	~Finale();					/* O035_00AE Finale_dtor */
	void run();					/* O035_0175 Finale_run */
	void scenes();					/* O035_03C7 scene1..scene9 */
	void scene1();					/* O035_044A Bild 0x3a7, txt[0] */
	void scene2();					/* O035_0512 */
	void scene3();					/* O035_05DB */
	void scene4();					/* O035_06A4 */
	void scene5();					/* O035_076E */
	void scene6();					/* O035_0838 */
	void scene7();					/* O035_0900 */
	void scene8();					/* O035_09C9 */
	void scene9();					/* O035_0A92 Bild 0x3af, txt[8] */
	void fade_wait(signed short n);			/* O035_0B5B einblenden, warten (Marke n bzw. 1000), ausblenden */
	void title();					/* O035_0BFD Titelbild 0x436, Dateilisten in den CD-Cache */
};
#define FINALE_PTR Finale far *
#else
struct finale {
	void far *txt_buf;		/* +0x00 */
	char far * far *txt;		/* +0x04 Texte (O036 finale_play_script) */
	unsigned char far *pal_saved;	/* +0x08 */
	unsigned char far *pal_from;	/* +0x0c */
	unsigned char far *pal_pic;	/* +0x10 */
	void far *pic;			/* +0x14 Pic */
	struct Bmp bmp;			/* +0x18 */
};
#define FINALE_PTR struct finale far *
#endif

struct finale_step {			/* 6 Bytes, Schritt des Skripts der Spielende-Sequenz (O036) */
	unsigned short time;		/* +0 faellig ab g_finale_ticks (10 Hz) */
	unsigned short op;		/* +2 FINALE_OP_* */
	signed short arg;		/* +4 FLI-Nummer, Sprachnummer bzw. Textnummer */
};

/* --- Cache: Festplatten-Cache fuer Dateien von der CD (O003, 0x16 Bytes; g_cd_cache, Konstruktor in globals_ctor O032
 * mit 300 Eintraegen / 2000000 Bytes). Schluessel = Typ (Bits 13..15: CACHE_KEY_* -> Archiv g_file_main/g_file_fx/
 * g_file_speech/g_file_raw/g_file_cd/g_file_speech_ext) | Nummer (Bits 0..12; Typ 0xc000: Nummer - 1000). Zustaende
 * je Eintrag 2 Bit (cache_state_set/get; 2 = nach Gebrauch loeschbar, cache_purge). Cache-Dateien nach
 * g_file_names[5] ("..\CTEMP\%s"). Freie Namen cache_* bzw. load_pic_list/cache_purge in s_proto.h. ------------ */
struct cache_entry {			/* 4 Bytes */
	signed short key;		/* +0 Schluessel */
	signed short age;		/* +2 Alter (0 = zuletzt benutzt, bis -30000) */
};
#ifdef __cplusplus
class Cache {
public:
	struct cache_entry far *tab;	/* +0x00 Eintraege (max) */
	unsigned short far *states;	/* +0x04 2-Bit-Zustaende */
	signed short count;		/* +0x08 belegte Eintraege */
	signed short max;		/* +0x0a */
	signed long size;		/* +0x0c Platzbedarf auf der Platte */
	signed short music_off;		/* +0x10 Musik schon angehalten (einmal je load_list) */
	signed short result;		/* +0x12 Ergebnis von fetch (1 = da) */
	signed short esc;		/* +0x14 Esc erlaubt (1), -1 = abgebrochen */

	void fatal();					/* O003_009A cache_fatal */
	void split_key(unsigned short key, unsigned short far *type, unsigned short far *nr);	/* O003_01AC */
	signed short disk_full(signed long need);	/* O003_01DC */
	signed short find(signed short key);		/* O003_022F */
	signed short age(signed short i);		/* O003_0260 */
	void touch(signed short n);			/* O003_0282 */
	signed short victim();				/* O003_02EA */
	char far *file_name(unsigned short key);	/* O003_0358 */
	void copy_file(unsigned short key);		/* O003_0435 */
	signed short on_disk(unsigned short key);	/* O003_0628 */
	signed long file_size(unsigned short key);	/* O003_073C */
	void remove(signed short i);			/* O003_0858 */
	void insert(signed short key, signed short st);	/* O003_0930 */
	void fetch(signed short key, signed short st);	/* O003_0A32 */
	Cache(signed short max, signed long size);	/* O003_0ABA Cache_ctor */
	~Cache();					/* O003_0B72 Cache_dtor */
	signed short load_list(signed short far *list, unsigned short far *states, signed short mouse,
		signed short esc);			/* O003_0BCA load_pic_list */
	void purge(signed short mouse);			/* O003_0D58 cache_purge */
	void clear();					/* O003_0DD8 cache_clear */
	void init_dir();				/* O003_0DFA cache_init_dir */
};
#endif

/* --- CD: CD-Audio ueber MSCDEX (O004, 0x1f Bytes; *g_cd, angelegt in SndSetup::run O043 bei Musik-Port CD_PORT;
 * R04/O042 rufen CD_music/CD_update/CD_stop/CD_volume). Fehler err: CD_ERR_*. ------------------------------- */
struct cd_req {				/* 0x1a Bytes, MSCDEX-Geraeteanforderung (int 2fh, ax = 1510h) */
	unsigned char len;		/* +0x00 */
	unsigned char unit;		/* +0x01 */
	unsigned char cmd;		/* +0x02 Befehl (3 IOCTL-Eingabe, 0x0c IOCTL-Ausgabe, 0x84 spielen, 0x85 anhalten) */
	unsigned short status;		/* +0x03 Status (Bit 9 = spielt) */
	signed char _r05[0x0d - 0x05];
	unsigned char media;		/* +0x0d */
	void far *ptr;			/* +0x0e Uebergabepuffer bzw. Startsektor */
	unsigned short count;		/* +0x12 Laenge bzw. Sektorzahl (mit +0x14 als long) */
	unsigned short f14;		/* +0x14 */
	unsigned long f16;		/* +0x16 */
};
struct cd_diskinfo {			/* 7 Bytes, IOCTL 0x0a */
	unsigned char code;		/* +0 */
	unsigned char first;		/* +1 erster Titel */
	unsigned char last;		/* +2 letzter Titel */
	unsigned long leadout;		/* +3 */
};
struct cd_track {			/* 7 Bytes, IOCTL 0x0b */
	unsigned char code;		/* +0 */
	unsigned char no;		/* +1 Titel */
	unsigned long start;		/* +2 Start (MSF) */
	unsigned char ctrl;		/* +6 */
};
struct cd_music {			/* 2 Bytes, Eintrag der Musiktabelle (Datei 0x1cf, 71 Eintraege) */
	unsigned char track;		/* +0 Titel */
	unsigned char disc;		/* +1 CD 1/2, 0 = beliebig */
};
struct cd_volctl {			/* 9 Bytes, IOCTL 0x0c Lautstaerke (Vorlage g_cd_volctl) */
	unsigned char b[9];
};
#ifdef __cplusplus
class CD {
public:
	signed short drive;		/* +0x00 Laufwerk (0 = A:) */
	signed short track;		/* +0x02 laufender Titel, 0 = keiner */
	signed char disc;		/* +0x04 eingelegte CD 1/2, 0 = unbekannt */
	signed short busy;		/* +0x05 spielt */
	signed short paused;		/* +0x07 */
	signed short open;		/* +0x09 Tuer offen bzw. keine CD */
	signed short endtime;		/* +0x0b Ende des Titels (Zaehler g_random_seed2) */
	struct cd_diskinfo far *di;	/* +0x0d */
	struct cd_track far *tr;	/* +0x11 Titeltabelle (26) */
	struct cd_req far *rq;		/* +0x15 */
	struct cd_music far *mu;	/* +0x19 Musiktabelle (71) */
	signed short err;		/* +0x1d CD_ERR_*, 0 = gut */

	signed long msf2hsg(unsigned char far *msf);	/* O004_011A */
	signed short ndrives();				/* O004_015F */
	unsigned short version();			/* O004_016D */
	unsigned short firstdrive();			/* O004_017B */
	void request();					/* O004_018B */
	void status();					/* O004_01C2 */
	void read_disc();				/* O004_029F */
	CD(signed short drive);				/* O004_048A CD_ctor */
	~CD();						/* O004_0655 CD_dtor */
	void change();					/* O004_06C6 CD_change */
	void stop();					/* O004_071A CD_stop */
	void play(signed short t);			/* O004_077A CD_play */
	void music(signed short n);			/* O004_08E6 CD_music */
	void update();					/* O004_0ABF CD_update */
	signed short closed();				/* O004_0B1C CD_closed */
	void volume(signed short v);			/* O004_0B4A CD_volume */
};
#endif

/* --- Sound: Klangobjekt auf dem Miles-Treiber AIL (O042, 0x89 Bytes = struct sound; g_snd_music, g_snd_fx; Konstruktoren
 * in globals_ctor O032). Freie Namen Sound_* in s_proto.h (R04, C-Module). Speech: Sprachausgabe-Schalter g_snd_speech
 * (Wort +0 an/aus), Wiedergabe ueber g_snd_fx. ------------------------------------------------------------ */
#ifdef __cplusplus
typedef struct sound sound_base;	/* Basisname: dos.h deklariert die Funktion sound() */
class Sound : public sound_base {
public:
	Sound(File far *file, struct arc far *arch, File far *file_speech);	/* O042_0000 Sound_ctor */
	Sound(File far *file, File far *file_speech);	/* O042_00A6 Sound_ctor_files */
	Sound(struct arc far *arch);			/* O042_0140 Sound_ctor_arc */
	Sound(struct arc far *arch, File far *file);	/* O042_01DB Sound_ctor_o042 */
	~Sound();					/* O042_0275 Sound_dtor */
	void play(signed short n);			/* O042_0396 Sound_play */
	void far *load(char far *name);			/* O042_044A Sound_load: Treiberdatei laden */
	void unload();					/* O042_04F6 Sound_unload */
	signed short timbres();				/* O042_0524 Sound_timbres */
	signed short start(signed short loop);		/* O042_0668 Sound_start */
	void far *timbre(signed short bank, signed short patch);	/* O042_0700 Sound_timbre */
	signed short music(signed short n);		/* O042_07C6 Sound_music */
	void stop_music();				/* O042_086F Sound_stop_music */
	signed short init(char far *name, signed short type, signed short io, signed short irq);	/* O042_08CA */
	char far *drvname(char far *name, signed short type, signed short idx, struct ail_drv_desc far *out,
		char far *dst);				/* O042_0B07 Sound_drvname */
	void resume(signed short i);			/* O042_0C19 Sound_resume: AIL_resume_sequence/_digital_playback */
	void stop1(signed short i);			/* O042_0C7A Sound_stop1 */
	void stop();					/* O042_0C9A snd_obj_stop */
	void pause(signed short i);			/* O042_0D18 Sound_pause: AIL_stop_sequence/AIL_pause_digital_playback */
	void volume(signed short vol, signed short b, signed short i);	/* O042_0D79 Sound_volume */
	void volume2(signed short vol, signed short i);	/* O042_0E2F Sound_volume2 */
	void volume3(signed short vol, signed short b, signed short i);	/* O042_0E53 Sound_volume3 */
	void wait();					/* O042_0E78 Sound_wait */
	signed short mtype();				/* O042_0E93 Sound_mtype */
	void callback(void (far *cb)());		/* O042_0F0A Sound_callback */
	void nocallback();				/* O042_0F29 Sound_nocallback */
};

class Speech {
public:
	signed short on;		/* +0x00 Sprachausgabe an */

	void stop();					/* O042_0F42 Speech_stop */
	void volume(signed short vol);			/* O042_0F62 Speech_volume */
	void set(signed short v);			/* O042_0F89 Speech_set */
	signed short active();				/* O042_0F97 speech_active */
	void wait();					/* O042_0FBA Speech_wait */
	signed short get_pos();			/* O042_0FDA speech_get_pos: g_speech_pos & 0x3fff */
};
#endif

/* --- SndSetup: Klangeinstellungen beim Start (O043, 0xbaf Bytes; new/run/delete in main O032). Musik = SOUND.ADV
 * (AIL-Typ 3), Digital = DIGI.ADV (Typ 2); Konfigurationsdatei 0x13 (snd_cfg_*, O232), BLASTER-Erkennung (O232).
 * C-Sicht struct snd_setup (O232); Prototypen mit SNDSETUP_PTR. ----------------------------------------------- */
struct snd_drv_entry {			/* 0x5d Bytes, Eintrag der Treiberliste */
	char file[13];			/* +0x00 Treiberdatei (*.ADV) */
	char name[0x5d - 13];		/* +0x0d Geraetename */
};
struct snd_setup {
	char text[0x50];		/* +0x000 Geraetename (Sound::drvname) */
	char music_file[13];		/* +0x050 Treiberdatei Musik */
	signed short music_sel;		/* +0x05d gewaehlter Eintrag */
	signed short music_port;	/* +0x05f Port (CD_PORT = CD-Audio, 0 = keine Musik) */
	signed short music_irq;		/* +0x061 */
	char digi_file[13];		/* +0x063 Treiberdatei Digital */
	signed short digi_sel;		/* +0x070 */
	signed short digi_port;		/* +0x072 0 = keine Klaenge */
	signed short digi_irq;		/* +0x074 */
	signed short abort;		/* +0x076 Esc gedrueckt */
	signed char _r78[2];
	struct snd_drv_entry drivers[30];	/* +0x07a Treiberliste (scan) */
	char far *buf;			/* +0xb60 Puffer 8000 Bytes (Bilder, Palette) */
	void far *txt_buf;		/* +0xb64 Texte 0x18 */
	char far * far *txt;		/* +0xb68 ([6]/[7] Zieldateien der Treiber) */
	signed char ff[0x2b];		/* +0xb6c struct ffblk (findfirst/findnext in scan) */
	struct ail_drv_desc desc;	/* +0xb97 Beschreibung des gewaehlten Treibers */
};
#ifdef __cplusplus
class SndSetup : public snd_setup {
public:
	SndSetup();					/* O043_0000 SndSetup_ctor */
	~SndSetup();					/* O043_00BE SndSetup_dtor */
	void show(char far *s);				/* O043_00E3 Hintergrund und Text */
	void run();					/* O043_01B1 SndSetup_run */
	void save();					/* O043_0594 Treiberdateien kopieren */
	void write_driver(char far *file, char far *path, signed short port);	/* O043_0602 */
	void describe(signed short idx, signed short kind);	/* O043_070B Beschreibung nach desc */
	signed short scan(signed short kind);		/* O043_075D Treiberliste, Anzahl - 1 */
	signed short select(signed short max);		/* O043_0942 Auswahl, -1 = Esc */
	signed short input_hex(char far *s, signed short val);	/* O043_0B88 Hexwert eingeben */
	void music_menu();				/* O043_0DD0 */
	void digi_menu();				/* O043_1018 */
	signed short ask_port(char far *file, signed short port, signed short irq, signed short kind);	/* O043_11E5 */
	signed short ask_irq(char far *file, signed short port, signed short irq, signed short kind);	/* O043_12AB */
};
#define SNDSETUP_PTR SndSetup far *
#else
#define SNDSETUP_PTR struct snd_setup far *
#endif
struct snd_cfg {			/* 20 Bytes, Konfigurationsdatei 0x13 der Klangeinstellungen (O232, O043 liest 4 Worte) */
	signed short music_port;	/* +0x00 */
	signed short music_irq;		/* +0x02 */
	signed short digi_port;		/* +0x04 */
	signed short digi_irq;		/* +0x06 */
	signed char _r08[0x14 - 0x08];
};
struct timbre_entry {			/* 6 Bytes, Eintrag der Klangfarbenbibliothek SAMPLE.AD/SAMPLE.MT (Ende bank = -1) */
	signed char patch;		/* +0 */
	signed char bank;		/* +1 */
	signed long off;		/* +2 Lage in der Datei */
};
struct str2 { char s[2]; };		/* O007 g_scene_mark_on/_off (Strukturkopie) */
struct str7 { char s[7]; };		/* O007 g_scene_fmt "%s%s%s" (Strukturkopie) */
struct mouse_range {			/* 8 Bytes, ds:330b: Bewegungsbereich der Maus (= g_mouse_min_x .. g_mouse_max_y) */
	signed short min_x;		/* +0 */
	signed short min_y;		/* +2 */
	signed short max_x;		/* +4 */
	signed short max_y;		/* +6 */
};

/* --- Opt: Optionen-Menue (O199, 0xed Bytes; Pic als erstes Mitglied; new/run/delete in SEG002 ueber die freien
 * Namen Opt_ctor_o199/Opt_run_o199/Opt_dtor_o199). 17 Eintraege g_opt_entries (Bild OPTIONS.AIF, Teilbilder
 * OPTIONS.NVF), Vorgaben g_opt_defaults, Mausbereiche g_opt_areas. ----------------------------------------- */
struct opt_entry {			/* 14 Bytes, Eintrag des Optionen-Menues (ds:6e8e) */
	signed short x;			/* +0x00 */
	signed short y;			/* +0x02 */
	signed short w;			/* +0x04 Breite */
	signed char far *val;		/* +0x06 Wert (g_opt_music, g_opt_sound, g_cd_speech, g_subtitles, ..., Lautstaerken) */
	signed char img[4];		/* +0x0a Teilbild je Wert (-1 = nicht waehlbar; Regler: Leiste, Knopf) */
};
#ifdef __cplusplus
/* NvfMem ohne Fuellbyte (0x19 Bytes = struct nvf_desc) als Mitglied von Opt: gleiche Methoden R41 */
class NvfMem0 : public nvf_desc {
public:
	NvfMem0();					/* R41_000A nvf_new */
	~NvfMem0();					/* R41_0046 nvf_delete */
	signed long unpack(void far *dst, void far *src, signed short nr, signed short type, signed short far *w,
		signed short far *h, signed short stride);	/* R41_0715 nvf_unpack */
};

class Opt {
public:
	Pic pic;			/* +0x000 Hintergrundbild OPTIONS.AIF */
	void far *buf;			/* +0x0a6 Teilbilder OPTIONS.NVF */
	void far *buf_item;		/* +0x0aa Bildpuffer eines Eintrags (0x4b0) */
	void far *buf_popup_bg;		/* +0x0ae Hintergrund unter der Auswahlliste (0x10fe) */
	void far *buf_popup;		/* +0x0b2 Auswahlliste (0xff0) */
	signed short old_pp20;		/* +0x0b6 g_pp20_index beim Oeffnen */
	signed char _rb8[4];
	signed short first;		/* +0x0bc erstes Zeichnen (Regler setzen noch keine Lautstaerke) */
	NvfMem0 nvf;			/* +0x0be */
	void far *old_areas;		/* +0x0d7 gesicherte g_mouse_areas */
	void far *old_areas2;		/* +0x0db gesicherte g_mouse_areas2 */
	void far *pal;			/* +0x0df gesicherte Palette */
	signed short old_handler_on;	/* +0x0e3 gesicherter g_mouse_handler_on */
	void (far *old_handler)(short);	/* +0x0e5 gesicherter g_mouse_handler */
	void far *old_handler_areas;	/* +0x0e9 gesicherte g_mouse_handler_areas */

	Opt();						/* O199_0000 Opt_ctor_o199 */
	~Opt();						/* O199_0154 Opt_dtor_o199 */
	void run();					/* O199_026F Opt_run_o199 */
	void draw();					/* O199_049B Hintergrund und alle Eintraege */
	void draw_all();				/* O199_051D */
	void draw_ok();					/* O199_0640 OK-Knopf */
	void draw_entry(signed short n);		/* O199_06E1 */
	void popup(signed short n);			/* O199_07B7 Auswahlliste des Eintrags n */
	signed short popup_select(signed short n, signed short x, signed short y, signed short w, signed short cur);
							/* O199_0ACE Auswahl in der Liste, -1 = keine */
	void draw_slider(signed short n);		/* O199_0DDF Regler zeichnen, Lautstaerke setzen */
	void slider_mouse(signed short n);		/* O199_108E Regler mit der Maus */
	void slider_keys(signed short n);		/* O199_127A Regler mit den Pfeiltasten */
	void keyboard();				/* O199_1355 Tastensteuerung */
	void apply(signed short n);			/* O199_168F Einstellung n anwenden */
	void defaults();				/* O199_17ED Vorgaben */
};
#endif

#ifdef __cplusplus
/* --- CharGen: Heldenerschaffung (O026, 0x137 Bytes; Code O026, O167..O171) ------------------------------------------
 * Der Konstruktor laedt die Typtabellen aus g_file_main, die Texte 0x1e1 und die Symbole 0x1e2; chargen_run ist die
 * Hauptschleife (Seiten CHARGEN_PAGE_*). Freie Funktionen chargen_* in s_proto.h (Objekt als erstes Argument). Der Held
 * wird in einem eigenen Puffer (hero) aufgebaut und erst von chargen_save_hero in die Heldendateien geschrieben. */
class CharGen {
public:
	void far *txt_buf;		/* +0x000 Texte (Datei 0x1e1) */
	char far * far *txt;		/* +0x004 */
	signed char pic_type;		/* +0x008 Typ fuer die Portraetauswahl (Elfen gemeinsam: hoechstens 10) */
	signed short portrait;		/* +0x009 gewaehltes Portraet */
	signed short portrait_first;	/* +0x00b Bereich der Portraets je Typ/Geschlecht */
	signed short portrait_last;	/* +0x00d */
	signed char page;		/* +0x00f Seite CHARGEN_PAGE_* (0 Grundwerte, 1..3 Talente, 4 AT/PA, 5..10 Zauber) */
	signed short mu_bonus;		/* +0x010 Sternzeichen-Bonus MU +1 gegeben (bei Typwechsel zurueck) */
	signed short ch_bonus;		/* +0x012 Sternzeichen-Bonus CH +1 gegeben */
	unsigned char far *type_pal;	/* +0x014 Palette des Typbildes (32 Farben, Datei 0x1ef) */
	struct inc_counter far *skill_incs;	/* +0x018 Steigerungszaehler je Talent (52) */
	struct inc_counter far *spell_incs;	/* +0x01c je Zauber (86) */
	struct skill_tpl (far *auto_skills)[25];	/* +0x020 Talentvorlagen je Typ (Datei 0x9c) */
	signed char attr_moved[14];	/* +0x024 Eigenschaft verschoben: +1 gehoben, -1 gesenkt */
	signed short dummy;		/* +0x032 Breite/Hoehe von nvfile_load (unbenutzt) */
	signed short saved;		/* +0x034 Held gespeichert (Ergebnis von chargen_run) */
	struct type_req (far *type_reqs)[6];	/* +0x036 Typvoraussetzungen (Datei 0x1f5) */
	signed char (far *type_skills)[NR_SKILLS];	/* +0x03a Talent-Startwerte je Typ (Datei 0x1f6) */
	signed char (far *type_spells)[NR_SPELLS];	/* +0x03e Zauber-Startwerte je Zaubertyp 7..12 (Datei 0x1f7) */
	struct school_bonus far *schools;	/* +0x042 Akademien (Datei 0x1f8) */
	signed short (far *auto_spells)[45];	/* +0x046 Zauberlisten je Zaubertyp fuer die Automatik (Datei 0x1f9) */
	void far *bg;			/* +0x04a Hintergrundpuffer 320 x 200 */
	Nvf nvf;			/* +0x04e Bilddatei (Typbild, Symbole) */
	Pic pic;			/* +0x067 Hintergrundbild (0x1e3 + Seite) */
	struct hero far *hero;		/* +0x10d der neue Held (0x5ef Bytes) */
	Bmp far *bmp_bg;		/* +0x111 Bmp ueber bg */
	Bmp far *bmp_name;		/* +0x115 Hintergrund des Namens (0x5e x 9) */
	Bmp far *bmp_save1;		/* +0x119 gesicherte Hintergrundausschnitte je Seite (chargen_save_bg) */
	Bmp far *bmp_save2;		/* +0x11d */
	Bmp far *bmp_save3;		/* +0x121 */
	Bmp far *bmp_male;		/* +0x125 Symbol maennlich */
	Bmp far *bmp_female;		/* +0x129 Symbol weiblich */
	Bmp far *bmp_locked;		/* +0x12d Symbol ueber den Blaetterpfeilen (Anfaengermodus) */
	Bmp far *bmp_type;		/* +0x131 Typbild 0x80 x 0xb8 */
	signed short show_moved;	/* +0x135 Eigenschaften mit Farbe nach attr_moved zeigen */

	CharGen();			/* O026_0000 CharGen_ctor */
	~CharGen();			/* O026_0477 CharGen_dtor */
	signed short run();		/* O026_0A5D chargen_run: Hauptschleife, 1 = Held gespeichert (O044: cg->run()) */
};

/* --- SaveMenu: Spielstaende (0x6f7 Bytes; Code O106 savegame_*, O107 savemenu_*; new/delete in O044/O046/O071) ---
 * Ohne eigenen Konstruktor (das Pic-Mitglied wird automatisch konstruiert). Spielstand ..\GAMES\<name>.GAM: Kopf
 * "DSA VERSION" + 4 Versionsbytes, ab +0x10 "DESC" + Beschreibung 0x100, ab 0x114 Bloecke (Kennung, Laenge, Daten). */
struct savegame_entry {			/* 13 Bytes: Eintrag der Spielstandliste */
	char name[9];			/* +0 Name (auf 8 Zeichen mit ' ' aufgefuellt) */
	unsigned short time;		/* +9 Dateizeit (ff_ftime) */
	unsigned short date;		/* +0b Dateidatum (ff_fdate) */
};

class SaveMenu {
public:
	Pic pic;			/* +0x000 Hintergrundbild (0x89) */
	signed char fade[0x0c];		/* +0x0a6 Ueberblendung (fade_in_pic, vermutet) */
	Edit far *edit;			/* +0x0b2 Eingabefeld der Beschreibung */
	void far *txt_buf;		/* +0x0b6 Texte (Datei 0x9b) */
	char far * far *txt;		/* +0x0ba */
	char hdr[12];			/* +0x0be gelesene Kennung */
	signed char ver_a;		/* +0x0ca gelesene Versionsbytes (Datei: d, c, b, a) */
	signed char ver_b;		/* +0x0cb */
	signed char ver_c;		/* +0x0cc (mit g_version_c verglichen) */
	signed char ver_d;		/* +0x0cd */
	signed short line_h;		/* +0x0ce Zeilenhoehe */
	signed short sel;		/* +0x0d0 markierte Zeile 0..9 */
	signed short last;		/* +0x0d2 Zeile mit geaenderter Beschreibung, -1 = keine */
	signed short first;		/* +0x0d4 erste angezeigte Zeile */
	signed short fd;		/* +0x0d6 Dateinummer */
	char desc[0x100];		/* +0x0d8 Beschreibung */
	struct savegame_entry names[100];	/* +0x1d8 */
	char cur[9];			/* +0x6ec Name des zuletzt gespeicherten Spielstands */
	signed short count;		/* +0x6f5 Zahl der Spielstaende */
};

/* --- Save: Helden aus DSA-1-Spielstaenden uebernehmen (O176, 0x326 Bytes; Methoden O176/O177) ------------------ */
class Save {
public:
	struct hero far *hero;		/* +0x000 Arbeitspuffer (0x5ef Bytes) */
	struct dsa1_timer timers[100];	/* +0x004 Modifikations-Timer des Spielstands */
	signed short swap_timers;	/* +0x324 Timer noch in Dateibyteordnung (beim ersten Gebrauch drehen) */

	Save();							/* O176_018E Save_ctor */
	~Save();						/* O176_01CF Save_dtor */
	signed short load(signed short fd);			/* O176_0210 Save_load: 0 nicht lesbar, 1 gut, -1 keine Helden */
	void check_hero(struct hero far *h);			/* O176_066D save_check_hero: Ausruestung/Werte nachziehen */
	void undo_timers(struct dsa1_hero far *h, signed short swap);	/* O176_0B3C save_undo_timers */
	void undo_timers_v1(struct dsa1_hero_v1 far *h, signed short swap);	/* O176_0C2E save_undo_timers_v1 */
	signed short find_portrait(signed char far *pic, signed char sex, signed char type, signed short amiga);
								/* O176_0D20 save_find_portrait */
	signed short import_heroes(signed short fd, signed short amiga);	/* O177_044B save_import_heroes */
	signed short import_heroes_v1(signed short fd, signed short amiga);	/* O177_0DD8 save_import_heroes_v1 */
};
#endif

#ifdef __cplusplus
/* --- Bars: Balkenanzeigen (struct bars, Objekt g_bars; Code O057, freie Namen Bars_* in s_proto.h). Welle N19. */
class Bars : public bars {
public:
	Bars();						/* O057_0000 Bars_ctor: Farben, Blinkzaehler */
	void draw(signed short idx, signed short slot, signed short val, signed short max, signed short mode,
		void far *dst);				/* O057_0069 Bars_draw: LE- (idx 0) bzw. AE-Balken (1) */
	void tick();					/* O057_0211 Bars_tick: Hunger/Durst im Heldenbogen */
};
#endif

#ifdef __cplusplus
/* --- Ani: Hintergrundanimationen eines Bildes (struct ani, Objekt g_screen_pic; Laden O055, Abspielen O054; freie
 * Namen ani_... und anim_... in s_proto.h, this = erstes Argument). Welle N19. */
class Ani : public ani {
public:
	Ani();						/* O055_0000 ani_ctor: mode 5, pal_size 0x40, init */
	~Ani();						/* O055_0051 ani_dtor: free_all */
	void clear();					/* O055_0084 ani_clear: Teile, Flags, Phasen, Bilder loeschen */
	void pal_black();				/* O055_028A ani_pal_black */
	void set_pal(void far *p);			/* O055_02E6 ani_set_pal */
	void set_pos(signed short x, signed short y);	/* O055_0307 ani_set_pos */
	void set_pal_size(signed short n);		/* O055_0320 ani_set_pal_size */
	void halt();					/* O055_032F ani_halt: mode 5, run = 0 */
	void stop();					/* O055_0346 ani_stop: halt und freigeben */
	void play(signed short mode);			/* O055_0362 ani_play: bis zum Ende abspielen */
	void play_until(signed short mode, signed char code);	/* O055_0392 ani_play_until: bis Marke code */
	signed long calc_size(signed long hdr_size);	/* O055_03C4 ani_calc_size: Speicherbedarf aus dem Kopf */
	void load(signed short id);			/* O055_067A ani_load (BE load_ani) */
	void select_variant(signed short mode, signed char var);	/* O054_0000 anim_select_variant */
	void reset(signed short mode);			/* O054_00DE anim_reset: Grundbild, Start */
	void load_pic(signed short i, signed short n, Bmp far *dst);	/* O054_0290 anim_load_pic: Bild aus der Datei */
	void tick();					/* O054_052A anim_tick: Animationsschritt (Zeitgeber) */
	void load_reset(signed short id, signed short mode);	/* O054_0E85 anim_load */
	void load_variant(signed short id, signed short mode, signed char var);	/* O054_0EAB anim_load_variant */
	void free_all();				/* O054_0ED5 anim_free_all */
	void init();					/* O054_1075 anim_init: Grundeinstellung */
};
#endif

#ifdef __cplusplus
/* --- Puzzle: Legespiel des Phex-Tempels (O208, 0x29 Bytes; O204 legt es mit new an): 3 x 5 verdeckte Karten, zwoelf
 * Werte 3..14 und drei Karten mit Wert 2; immer die drei zuletzt aufgedeckten liegen offen - sind es die drei mit
 * Wert 2, ist das Spiel gewonnen. Grafik aus Datei 0x3b3 (NVF im Speicher: 0 Tisch, 1 Rueckseite, 2..14 Werte,
 * 15..19 Umdrehbilder, Palette = letzte 0x300 Bytes). Freie Namen puzzle_* in s_proto.h. ------------------------- */
class Puzzle {
public:
	char far *screen;		/* +0x00 Bildpuffer 64000 Bytes (Tisch) */
	char far *file_data;		/* +0x04 Inhalt der Grafikdatei 0x3b3 */
	signed char far *grid;		/* +0x08 Werte der 15 Karten (new signed char[15]) */
	char far *pal;			/* +0x0c Palette (file_data + Laenge - 0x300) */
	signed char shown[3];		/* +0x10 die drei offenen Karten, aelteste zuerst (-1 = keine) */
	signed short table_w;		/* +0x13 Tischbild */
	signed short table_h;		/* +0x15 */
	signed short card_w;		/* +0x17 Kartenbild */
	signed short card_h;		/* +0x19 */
	signed short sel;		/* +0x1b Karte unter dem Rahmen 0..14 */
	signed short last_sel;		/* +0x1d zuletzt umrahmte Karte */
	signed short mouse_x;		/* +0x1f letzte Mausposition */
	signed short mouse_y;		/* +0x21 */
	signed short moves;		/* +0x23 Zuege */
	signed short won;		/* +0x25 gewonnen (Rueckgabe von run) */
	signed short saved_handler;	/* +0x27 gesicherter g_mouse_handler_on */

	signed short run();		/* O208_0000 Puzzle_run: spielen bis Esc/rechte Maustaste oder gewonnen */
	void load_data();		/* O208_0119 puzzle_load_data */
	void shuffle();			/* O208_01A3 puzzle_shuffle */
	void draw_table();		/* O208_0245 puzzle_draw_table */
	void draw_cursor();		/* O208_03E3 puzzle_draw_cursor */
	void show_table();		/* O208_052E puzzle_show_table */
	void handle_keys();		/* O208_05B3 puzzle_handle_keys */
	void flip_card();		/* O208_072A puzzle_flip_card */
	void unflip_oldest();		/* O208_0A20 puzzle_unflip_oldest */
	void push_shown();		/* O208_0D08 puzzle_push_shown */
	void check_won();		/* O208_0D37 puzzle_check_won */
	void handle_mouse();		/* O208_0E12 puzzle_handle_mouse */
};
#endif

#ifdef __cplusplus
/* --- Welle N22 ---------------------------------------------------------------------------------------------- */
/* Wx: Wetter (O256, 4 Bytes, Objekt g_weather ds:be32; freie Namen Wx_* in s_proto.h). Zyklischer Zustand 0..9 mit
 * Zufallsschritt je Tag, daraus das Wetter WEATHER_* (Travel.weather, Wetterfolgen O213, Nahrungssuche O071). */
class Wx {
public:
	signed short season;		/* +0x00 Jahreszeit SEASON_* (date_season beim letzten next) */
	signed char level;		/* +0x02 Wetter WEATHER_HOT .. WEATHER_STORM */
	signed char cycle;		/* +0x03 Zustand 0..9 (zyklisch) */

	Wx();						/* O256_0000 Wx_ctor */
	~Wx();						/* O256_0060 Wx_dtor */
	void set(signed char v);			/* O256_0085 Wx_set */
	signed char get();				/* O256_0094 Wx_get */
	signed char next();				/* O256_00A2 Wx_next: naechster Tag */
};

/* Items: Gegenstandstabelle (O182, 12 Bytes, Objekt ds:b84a: desc = g_items, names = g_item_names). */
class Items {
public:
	struct item_desc far *desc;	/* +0x00 ITEMS.DAT (Datei 0x0c) */
	char far * far *names;		/* +0x04 ITEMS.LTX (Datei 0x0d) */
	void far *names_buf;		/* +0x08 */

	Items();					/* O182_0000 Items_ctor: Tabellen laden, g_nr_items, g_item_demand */
	~Items();					/* O182_00F3 Items_dtor */
	signed short change_price(signed short id, signed short pct);	/* O182_013A items_change_price */
};

/* Date: Spielzeit/Kalender (O250, 0x18 Bytes, Objekt g_date ds:b856; Zeitrechnung R21 timewarp_*). Freie Namen
 * Date_* / date_* in s_proto.h; Zeitpunkte struct timestamp (date_now/date_set/date_since). */
class Date : public game_date {
public:
	char far *sky_bg;		/* +0x0c Himmel (TIMEDISP.NVF Teilbild 0 Tag / 2 Nacht, 0x48 x 0x16) */
	char far *sky_marks;		/* +0x10 Sonne (3 Stellungen) und Mond, je 4 x 4 (Teilbilder 4..7) */
	char far *sky_fg;		/* +0x14 Vordergrund (Teilbild 1 Tag / 3 Nacht, maskiert) */

	Date();						/* O250_0000 Date_ctor: 15.12.17 Hal, 8 Uhr */
	signed char next_day();				/* O250_0078 Date_next_day: Tag nach day (unbenutzt) */
	void print();					/* O250_00A1 Date_print: Datum/Uhrzeit nach g_text_output_buf */
	signed short season();				/* O250_0231 date_season: SEASON_* */
	void draw(signed short force);			/* O250_0299 Date_draw: Himmelsanzeige (Uhr) */
	void alloc();					/* O250_05DA Date_alloc: Himmelsbilder laden */
	void release();					/* O250_06CF Date_free */
	signed short save();				/* O250_072E Date_save: "TIME", Version 11 */
	signed short load();				/* O250_084E Date_load */
};
#endif

#ifdef __cplusplus
/* House: Wohnhaus LOC_HOUSE (O173, 8 Bytes; aus O030 bei Ortsart LOC_HOUSE): abweisende Antworten je Hauslage, Bewohner
 * mit Rat (g_house_hints), in Finsterkoppen Nachtlager bei Zwergen. Texte GETOFF.LTX (Datei 0x19d). */
class House {
public:
	void far *txt_buf;		/* +0x00 */
	char far * far *txt;		/* +0x04 */

	House();					/* O173_0000 House_ctor */
	~House();					/* O173_0049 House_dtor */
	void run();					/* O173_0093 house_run */
	signed short hint();				/* O173_03F7 house_hint: Bewohner mit Rat, 1 = gefunden */
};
#endif

#ifdef __cplusplus
/* TMap: Kartenobjekt der Reisekarte (O187, 0xf8 Bytes, g_tmap ds:c217; Kartenbild ORKLAND.AIF = Datei 0x92 im EMS/XMS,
 * Ausschnitt in scr). Freie Namen tmap_* / TMap_* in s_proto.h (fuer Aufrufer mit g_tmap: O038, O209, O213..O223). */
class TMap {
public:
	signed char _r00[6];		/* +0x00 Dateikopf (0x1e Bytes) */
	signed short width;		/* +0x06 (get_size) */
	signed short height;		/* +0x08 */
	signed short nr_colors;		/* +0x0a Palettenfarben am Dateiende */
	signed short first_color;	/* +0x0c erste Farbe (set_pal) */
	signed char _r0e[0x1e - 0x0e];
	char far *pal;			/* +0x1e Palette */
	signed short view_x;		/* +0x22 Ausschnitt (Kartenkoordinate + 16) */
	signed short view_y;		/* +0x24 (+ 20) */
	signed short tile_x;		/* +0x26 view_x & ~15 */
	signed short tile_y;		/* +0x28 view_y / 20 * 20 */
	signed short last_x;		/* +0x2a zuletzt verfolgter Punkt (follow; -1 = keiner) */
	signed short last_y;		/* +0x2c */
	signed short fine_x;		/* +0x2e view_x % 16 */
	signed short fine_y;		/* +0x30 view_y % 20 */
	signed short file;		/* +0x32 Datei */
	signed short own_pal;		/* +0x34 Palette selbst angelegt */
	signed char _r36[2];
	signed long bytes_left;		/* +0x38 read_map: noch zu lesen */
	unsigned char rows[12];		/* +0x3c Reihenfolge der Kachelzeilen in scr */
	void far *mark2_bg_buf;		/* +0x48 */
	void far *mark1_bg_buf;		/* +0x4c */
	void far *mark2_bufs[4];	/* +0x50 Bilder Kartenpunkt 2 (CIRCLE.NVF 5..8) */
	signed char _r60[4];
	void far *mark1_bufs[4];	/* +0x64 Bilder Kartenpunkt 1 (CIRCLE.NVF 0..3) */
	signed char _r74[4];
	Bmp mark2_bg;			/* +0x78 Hintergrund unter Kartenpunkt 2 */
	Bmp mark1_bg;			/* +0x82 */
	Bmp mark2_pics[5];		/* +0x8c */
	Bmp mark1_pics[5];		/* +0xbe */
	char far *scr;			/* +0xf0 Ausschnitt (12 Kachelzeilen zu 0x120 x 20) */
	char far *cur;			/* +0xf4 */

	TMap();						/* O187_0000 TMap_ctor */
	~TMap();					/* O187_00B3 TMap_dtor */
	void new_pal();					/* O187_015E tmap_new_pal */
	signed short screen_x(signed short v);		/* O187_0182 tmap_screen_x */
	signed short screen_y(signed short v);		/* O187_0196 tmap_screen_y */
	void free_pal();				/* O187_01AA tmap_free_pal */
	signed short read_header();			/* O187_01E1 tmap_read_header */
	signed short read_pal();			/* O187_0227 tmap_read_pal */
	void set_pal();					/* O187_02A2 tmap_set_pal */
	void get_size(signed short far *w, signed short far *h);	/* O187_033A tmap_get_size */
	signed short load(signed short id);		/* O187_0359 tmap_load */
	signed short read_map();			/* O187_03D9 tmap_read_map */
	void set_pos(signed short x, signed short y);	/* O187_055A tmap_set_pos */
	void draw();					/* O187_0798 tmap_draw */
	void load_col(signed short a, unsigned short b);	/* O187_0804 tmap_load_col */
	void load_row(char far *dst, signed short dy);	/* O187_09E0 tmap_load_row */
	signed char scroll_right(signed short draw);	/* O187_0ADA tmap_scroll_right */
	signed char scroll_left(signed short draw);	/* O187_0BDC tmap_scroll_left */
	signed char scroll_down(signed short draw);	/* O187_0CD2 tmap_scroll_down */
	signed char scroll_up(signed short draw);	/* O187_0DD7 tmap_scroll_up */
	signed char plot(signed short x, signed short y, unsigned char c);	/* O187_0ED3 tmap_plot */
	signed char follow(signed short x, signed short y, signed char steps, unsigned char c);	/* O187_0FB9 tmap_follow */
	void plot_points(signed short far *p, unsigned char c);	/* O187_1167 tmap_plot_points */
	void mark1_init(signed short x, signed short y);	/* O187_11A1 */
	void mark2_init(signed short x, signed short y);	/* O187_12A4 */
	void mark1_free();				/* O187_13AC tmap_mark1_free */
	void mark2_free();				/* O187_1427 tmap_mark2_free */
	void mark1_anim(signed short x, signed short y);	/* O187_14A2 tmap_mark1_anim */
	void mark2_anim(signed short x, signed short y);	/* O187_1633 tmap_mark2_anim */
};
#endif

#endif
