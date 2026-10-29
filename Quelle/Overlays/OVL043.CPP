/* OVL043 - Overlay O043 (Segment 0x1ff9 in SCHWEIF.EXE): Klangeinstellungen beim Start, Klasse SndSetup (s_class.h,
 * 0xbaf Bytes; new/run/delete in main O032); Musik = SOUND.ADV (AIL-Typ 3, music_*), Digital = DIGI.ADV (Typ 2,
 * digi_*). Konstruktor: Puffer 8000 Bytes, Texte 0x18 (0x11 Stueck), Treibernamen leer, Ports/IRQs 0. show(s):
 * Hintergrundbild (NvfMem Nr. 0 in Bmp 0xb4 x 0x79) bei 0x49/0x22 und Text. run(): Konfigurationsdatei 0x13
 * (file_getname) lesen; fehlt sie: Treiberdateien loeschen (snd_cfg_unlink_drivers), Fenster/Farben sichern,
 * Palette aus Datei 0x12, music_menu, digi_menu, snd_cfg_write, save; danach Texte frei; Musik-Port CD_PORT =
 * CD-Audio: CD-Objekt (O004) fuer das Laufwerk aus g_file_names[4], bei Fehler CD_ERR_NO_DRIVE..CD_ERR_NOT_CD
 * Meldung und Programmende, sonst warten, bis die richtige CD liegt; sonst Treiber SOUND.ADV bzw. DIGI.ADV mit
 * Port/IRQ initialisieren (Sound::init). save/write_driver: Treiberdatei ..\DRIVERS\<Treiber> in die Zieldatei
 * txt[6]/[7] kopieren bzw. bei Port 0 loeschen. describe: Beschreibung des gewaehlten Treibers nach desc.
 * scan(kind): Treiberliste (findfirst *.ADV, je Datei alle Geraetenamen per Sound::drvname) in drivers[30], bei
 * kind 3 zuerst txt[16] (CD), zuletzt txt[9] (kein Treiber); Rueckgabe Anzahl - 1. select: Auswahl mit Pfeiltasten
 * (g_action, Mausbereiche g_sndsetup_areas_sel), Esc -> abort = 1 und -1. input_hex: Hexwert-Eingabe (Leertaste ->
 * Eingabefeld Input O180, strtol(.., 16), itoa). music_menu/digi_menu: Auswahl, Treiberbeschreibung (Port/IRQ),
 * bei "SB..."-Treibern blaster_detect (O232), Port/IRQ erfragen (ask_port/ask_irq), Taste abwarten; Musik-Auswahl
 * 0 = CD (CD_PORT), letzter Eintrag = keiner (0).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL043.CPP   -> 4977/4977 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (ohne -Og 2 Bytes zu lang). Keine Intrinsics (strcpy/strcat/memset/strncmp sind Aufrufe) -> ohne -Oi.
 *   (Namensfassung mit schweif.h, Welle N20: 19.10.2026 GLEICH, LINKGLEICH)
 * - C++-Klasse mit nicht-inline Konstruktor/Destruktor: Konstruktor "if (this == 0) new" mit jmp near (E9) und
 *   totem Epilog, Destruktor nur "(flags & 1) -> mem_free_v" (flags in SI) - entsteht von selbst.
 * - "music_port = music_irq = digi_port = digi_irq = 0" (Speicherreihenfolge rechts -> links: 74, 72, 61, 5f).
 * - Lokale Objekte: Initialisierung in Deklarationsreihenfolge (select: NvfMem-Ktor, sel = 0, dann die Bmp);
 *   NvfMem (0x1a) und Input (0x28) liegen als grosse Aggregate hinten, Bmp und char[6] in Reihenfolge; Rueckgabewert
 *   ueber Stack-Zwischenwert vor den Destruktoren.
 * - music_menu: n bekommt SI (port/irq adressgenommen fuer blaster_detect); digi_menu: port/irq zuerst deklariert
 *   -> SI/DI, n auf dem Stack. music_sel nach der Zuweisung als -Og-Zwischenwert [bp-0ah] (cmp [bp-0ah],si).
 * - font_set_colors(g_font, c1, c2) mit gesicherten Farben: Aufruf ueber gecasteten Funktionszeiger (push word,
 *   der Prototyp hat unsigned char).
 * - "port = irq = CD_PORT" speichert irq zuerst. write_driver: "void far *data = 0" -> mov dword; "*g_names_star_idx
 *   = path" -> mov eax; "data != 0" -> cmp dword.
 * - g_sndsetup_areas_sel/_input/_key liegen in der Tabelle ds:426e (cobjfix ordnet sie als _s_426e+n zu).
 * Originalfehler: ask_port/ask_irq verwerfen im Zweig "Treiber beginnt mit "SB" und blaster_detect == 0" den
 *   eingegebenen Wert (input_hex(...) ohne Zuweisung) - es bleibt der Wert aus der Treiberbeschreibung.
 *   digi_menu prueft "digi_sel != -1" ein zweites Mal (immer wahr; music_menu prueft dort != 0). scan schreibt
 *   ohne Grenze in die 30 Eintraege.
 */
#include <string.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <dir.h>
#include <dos.h>
#include <process.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:4272..43dc, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
char g_sndsetup_areas_sel[40] = {
	0x64, 0x00, 0x59, 0x00, 0xbc, 0x00, 0x84, 0x00, 0x0d, 0x1c, 0xc6, 0x00, 0x6d, 0x00, 0xdf, 0x00,
	0x79, 0x00, 0x00, 0x50, 0xc6, 0x00, 0x7b, 0x00, 0xdf, 0x00, 0x87, 0x00, 0x00, 0x48, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};
char g_sndsetup_areas_input[30] = {
	0x61, 0x00, 0x69, 0x00, 0x7a, 0x00, 0x75, 0x00, 0x20, 0x39, 0xc6, 0x00, 0x7b, 0x00, 0xdf, 0x00,
	0x87, 0x00, 0x20, 0x39, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};
char g_sndsetup_areas_key[20] = {
	0x00, 0x00, 0x00, 0x00, 0x3f, 0x01, 0xc7, 0x00, 0x0d, 0x1c, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff
};

#define SND_CFG_FILE	0x13		/* Konfigurationsdatei der Klangeinstellungen */
#define SND_PAL_FILE	0x12		/* Palette der Einstellungsbilder */
#define SND_TXT		0x18		/* Texte (0x11 Stueck) */

#define FF		((struct ffblk *)ff)
#define CDP		((CD far *)g_cd)
#define SND_MUSIC	((Sound far *)g_snd_music)
#define SND_FX		((Sound far *)g_snd_fx)

SndSetup::SndSetup()
{
	buf = (char far *)mem_alloc(8000L);
	txt_load_list(g_font, SND_TXT, 0x11, &txt_buf, &txt);
	abort = 0;
	strcpy(music_file, "");
	strcpy(digi_file, "");
	music_port = music_irq = digi_port = digi_irq = 0;
}

SndSetup::~SndSetup()
{
}

void SndSetup::show(char far *s)
{
	signed short w;
	signed short h;
	Bmp b(0xb4, 0x79);
	NvfMem nvf;

	nvf.unpack(b.data(), buf, 0, 0, &w, &h, 0x140);
	font_set_box(g_font, 0x65, 0x64, 0x35, -1);
	mouse_hide(0);
	b.put(g_vga_buf, 0x49, 0x22);
	print_text(s);
	mouse_show(0);
}

void SndSetup::run()
{
	signed short x1;
	signed short y1;
	signed short x2;
	signed short y2;
	signed short c1;
	signed short c2;
	signed long len;
	char far *pal;
	signed short fd;

	fd = open(file_getname(g_file_main, SND_CFG_FILE), O_RDONLY | O_BINARY);
	if (fd < 0) {
		mouse_show(0);
		snd_cfg_unlink_drivers(this);
		font_get_box(g_font, &x1, &y1, &x2, &y2);
		font_get_colors(g_font, &c1, &c2);
		font_set_colors(g_font, 0x0f, -1);
		len = file_load(g_file_main, SND_PAL_FILE, buf);
		set_palette(buf + len - 0x300, 0, 0x100);
		clear_kbd();
		music_menu();
		if (abort == 0) {
			clear_kbd();
			digi_menu();
		}
		if (abort == 0) {
			snd_cfg_write(this);
			save();
		}
		mouse_hide(0);
		set_video_mode(0x13);
		mouse_show(0);
		font_set_box(g_font, x1, y1, x2, y2);
		((void (*)(void far *, signed short, signed short))font_set_colors)(g_font, c1, c2);
		mouse_hide(0);
	} else {
		read(fd, &music_port, 2);
		read(fd, &music_irq, 2);
		read(fd, &digi_port, 2);
		read(fd, &digi_irq, 2);
		close(fd);
	}
	mem_free(buf);
	mem_free(txt_buf);
	mem_free(txt);
	if (music_port != 0) {
		if (music_port == (signed short)CD_PORT) {
			pal = (char far *)mem_alloc_w(0x300);
			load_palette_arc3b(pal);
			g_cd = new CD(*g_file_names[4] - 'A');
			if (CDP->err == 0) {
				g_cd_music = 1;
				do {
					CDP->change();
					if (CDP->err != 0) {
						set_palette(pal + 0x1e0, 0xa0, 0x60);
						dlg_choose(g_dialog, "Bitte Sternenschweif-CD einlegen und Taste dr\x81""cken!", 1, "Weiter");
						wait_ticks_key(100);
					}
				} while (CDP->err != 0);
			} else {
				set_palette(pal + 0x1e0, 0xa0, 0x60);
				switch (CDP->err) {
				case CD_ERR_NO_DRIVE:
					dlg_choose(g_dialog, "Kein CD-Laufwerk gefunden!", 1, "Weiter");
					break;
				case CD_ERR_MSCDEX_OLD:
					dlg_choose(g_dialog, "MSCDEX-Version ist \x84lter als 2.10!", 1, "Weiter");
					break;
				case CD_ERR_NOT_CD:
					dlg_choose(g_dialog, "Angegebenes Laufwerk ist kein CD-Laufwerk!", 1, "Weiter");
					break;
				}
				g_cd_music = 0;
				delete CDP;
				mem_free_v(pal);
				sys_exit(0);
				exit(0);
			}
		} else {
			SND_MUSIC->init("SOUND.ADV", AIL_XMIDI_DRVR, music_port, music_irq);
		}
	}
	if (digi_port != 0)
		SND_FX->init("DIGI.ADV", AIL_DSP_DRVR, digi_port, digi_irq);
}

void SndSetup::save()
{
	if (music_port != (signed short)CD_PORT)
		write_driver(music_file, txt[6], music_port);
	write_driver(digi_file, txt[7], digi_port);
}

void SndSetup::write_driver(char far *file, char far *path, register signed short port)
{
	signed long len;
	void far *data = 0;
	char tmp[40];

	if (port != 0) {
		strcpy(tmp, "..\\DRIVERS\\");
		strcat(tmp, file);
		*(char far * far *)g_names_star_idx = tmp;
		len = file_length(g_file_main, 0);
		data = mem_alloc(len);
		len = file_load(g_file_main, 0, data);
	}
	*(char far * far *)g_names_star_idx = path;
	if (port == 0)
		unlink(*(char far * far *)g_names_star_idx);
	else {
		file_open_mode(g_file_main, 0, 0x8300);
		File_write(g_file_main, data, len);
		file_close(g_file_main);
	}
	if (data != 0)
		mem_free(data);
}

void SndSetup::describe(signed short idx, signed short kind)
{
	chdir("..\\DRIVERS");
	SND_MUSIC->drvname(drivers[idx].file, kind, 0, &desc, text);
	chdir("..\\DATA");
}

signed short SndSetup::scan(signed short kind)
{
	signed short n = 0;
	signed short k = 0;
	char far *p;

	chdir("..\\DRIVERS");
	memset(drivers, 0, sizeof(drivers));
	if (kind == AIL_XMIDI_DRVR) {
		strcpy(drivers[n].name, txt[16]);
		n++;
	}
	if (findfirst("*.ADV", FF, 0) == 0) {
#if FIX_0127
		/* Fix #127: Treiberliste auf 30 Eintraege begrenzen (einer bleibt fuer den Schluss) */
		while (n < 29 && (p = SND_MUSIC->drvname(FF->ff_name, kind, k++, &desc, text)) != 0) {
			strcpy(drivers[n].name, text);
			strcpy(drivers[n++].file, FF->ff_name);
		}
		while (n < 29 && findnext(FF) == 0) {
			k = 0;
			while (n < 29 && (p = SND_MUSIC->drvname(FF->ff_name, kind, k++, &desc, text)) != 0) {
#else
		while ((p = SND_MUSIC->drvname(FF->ff_name, kind, k++, &desc, text)) != 0) {
			strcpy(drivers[n].name, text);
			strcpy(drivers[n++].file, FF->ff_name);
		}
		while (findnext(FF) == 0) {
			k = 0;
			while ((p = SND_MUSIC->drvname(FF->ff_name, kind, k++, &desc, text)) != 0) {
#endif
				strcpy(drivers[n].name, text);
				strcpy(drivers[n++].file, FF->ff_name);
			}
		}
		strcpy(drivers[n++].name, txt[9]);
	}
	chdir("..\\DATA");
	n--;
	return n;
}

signed short SndSetup::select(signed short max)
{
	signed short w;
	signed short h;
	NvfMem nvf;
	signed short sel = 0;
	Bmp b1(0x5f, 0x30);
	Bmp b2(0x19, 0x0c);
	signed short redraw;

	g_mouse_areas = g_sndsetup_areas_sel;
	redraw = 1;
	do {
		if (g_action == KEY_UP || g_action == KEY_RIGHT) {
			redraw = 1;
			if (++sel > max)
				sel = 0;
		} else if (g_action == KEY_DOWN || g_action == KEY_LEFT) {
			redraw = 1;
			if (--sel < 0)
				sel = max;
		}
		if (redraw) {
			mouse_hide(0);
			font_set_box(g_font, 0x65, 0x5a, 0x5a, -1);
			nvf.unpack(b1.data(), buf, 1, 0, &w, &h, 0x140);
			b1.put(g_vga_buf, 0x61, 0x57);
			font_print_wrapped(g_font, drivers[sel].name);
			redraw = 0;
			nvf.unpack(b2.data(), buf, 4, 0, &w, &h, 0x140);
			b2.put(g_vga_buf, 0xc6, 0x6d);
			nvf.unpack(b2.data(), buf, 5, 0, &w, &h, 0x140);
			b2.put(g_vga_buf, 0xc6, 0x7b);
			mouse_show(0);
		}
		poll_input();
	} while (g_action != KEY_RETURN && g_action != KEY_ESC);
	g_mouse_areas = 0;
	if (g_action == KEY_ESC)
		abort = 1;
	return g_action == KEY_ESC ? -1 : sel;
}

signed short SndSetup::input_hex(char far *s, register signed short val)
{
	signed short w;
	signed short h;
	NvfMem nvf;
	Bmp b1(0x19, 0x0c);
	Bmp b2(0x19, 0x0c);
	Input in(3);
	char tmp[6];
	signed short redraw;

	nvf.unpack(b1.data(), buf, 2, 0, &w, &h, 0x140);
	g_mouse_areas = g_sndsetup_areas_input;
	redraw = 1;
	do {
		poll_input();
		if (g_action == KEY_SPACE) {
			strcpy(tmp, "");
			in.run(tmp, 0x65, 0x6b, 0);
			val = strtol(tmp, 0, 16);
			redraw = 1;
			g_action = KEY_RETURN;
		}
		if (redraw == 1) {
			mouse_hide(0);
			font_set_box(g_font, 0x65, 0x64, 0x61, -1);
			font_set_colors(g_font, 0x0f, -1);
			itoa(val, tmp, 16);
			print_text(s);
			b1.put(g_vga_buf, 0x61, 0x69);
			font_set_box(g_font, 0x65, 0x64, 0x6b, -1);
			font_set_colors(g_font, 0x0f, 0x23);
			print_text(tmp);
			nvf.unpack(b2.data(), buf, 3, 0, &w, &h, 0x140);
			b2.put(g_vga_buf, 0xc6, 0x7b);
			mouse_show(0);
			redraw = 0;
		}
	} while (g_action != KEY_RETURN && g_action != KEY_ESC);
	g_mouse_areas = 0;
	return val;
}

void SndSetup::music_menu()
{
	signed short n;
	signed short port;
	signed short irq;
	char far *file;

	n = scan(AIL_XMIDI_DRVR);
	font_set_colors(g_font, 0x0f, -1);
	font_set_box(g_font, 0x65, 0x64, 0x35, -1);
	show(txt[0]);
	if ((music_sel = select(n)) != -1) {
		file = drivers[music_sel].file;
		if (music_sel != 0 && music_sel != n) {
			describe(music_sel, AIL_XMIDI_DRVR);
			strcpy(music_file, file);
			port = desc.io;
			irq = desc.irq;
			if (strncmp(file, "SB", 2) == 0 && blaster_detect(this, file, &port, &irq, AIL_XMIDI_DRVR) == 1)
				strcpy(music_file, file);
			if (port != -1)
				port = ask_port(file, port, irq, AIL_XMIDI_DRVR);
			if (irq != -1)
				irq = ask_irq(file, port, irq, AIL_XMIDI_DRVR);
			mouse_hide(0);
			font_set_colors(g_font, 0x0f, -1);
			font_set_box(g_font, 0x65, 0x64, 0x35, -1);
			show(txt[5]);
			mouse_show(0);
			g_mouse_areas = g_sndsetup_areas_key;
			do
				poll_input();
			while (g_action == 0);
			g_mouse_areas = 0;
			if (g_action == KEY_ESC)
				abort = 1;
		} else if (music_sel == 0)
			port = irq = (signed short)CD_PORT;
		else
			port = irq = 0;
		music_port = port;
		music_irq = irq;
	}
}

void SndSetup::digi_menu()
{
	signed short port;
	signed short irq;
	signed short n;
	char far *file;

	n = scan(AIL_DSP_DRVR);
	font_set_colors(g_font, 0x0f, -1);
	font_set_box(g_font, 0x65, 0x64, 0x35, -1);
	show(txt[1]);
	if ((digi_sel = select(n)) != -1) {
		file = drivers[digi_sel].file;
		if (digi_sel != -1 && digi_sel != n) {
			describe(digi_sel, AIL_DSP_DRVR);
			strcpy(digi_file, file);
			port = desc.io;
			irq = desc.irq;
			if (port != -1)
				port = ask_port(file, port, irq, AIL_DSP_DRVR);
			if (irq != -1)
				irq = ask_irq(file, port, irq, AIL_DSP_DRVR);
			mouse_hide(0);
			font_set_colors(g_font, 0x0f, -1);
			font_set_box(g_font, 0x65, 0x64, 0x35, -1);
			show(txt[5]);
			mouse_show(0);
			g_mouse_areas = g_sndsetup_areas_key;
			do
				poll_input();
			while (g_action == 0);
			g_mouse_areas = 0;
			if (g_action == KEY_ESC)
				abort = 1;
		} else
			port = irq = 0;
		digi_port = port;
		digi_irq = irq;
	}
}

signed short SndSetup::ask_port(char far *file, signed short port, signed short irq, signed short kind)
{
	if (strncmp(file, "SB", 2) == 0) {
		if (blaster_detect(this, file, &port, &irq, kind) == 0) {
#if FIX_0127
			/* Fix #127: eingegebenen Port uebernehmen */
			show(txt[3]);
			port = input_hex(txt[4], port);
#else
			show(txt[3]);
			input_hex(txt[4], port);
#endif
		}
	} else {
		show(txt[3]);
		port = input_hex(txt[4], port);
	}
	return port;
}

signed short SndSetup::ask_irq(char far *file, signed short port, signed short irq, signed short kind)
{
	if (strncmp(file, "SB", 2) == 0) {
		if (blaster_detect(this, file, &port, &irq, kind) == 0) {
#if FIX_0127
			/* Fix #127: eingegebenen IRQ uebernehmen */
			show(txt[10]);
			irq = input_hex(txt[11], irq);
#else
			show(txt[10]);
			input_hex(txt[11], irq);
#endif
		}
	} else {
		show(txt[10]);
		irq = input_hex(txt[11], irq);
	}
	return irq;
}
