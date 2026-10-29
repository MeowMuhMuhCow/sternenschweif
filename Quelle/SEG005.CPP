/* SEG005 - Startphase: Spielobjekt Init (Konstruktor: Ctrl-Break, Verzeichnis, XMS/EMS-Speicher
 * pruefen und reservieren, Overlay-Puffer, Datenarchive laden; Destruktor: Temp-Dateien loeschen,
 * Speicher freigeben), Archiv-Namenslisten und Patchtabellen laden, Auslagerungsspeicher der 3D-Ansicht
 * in EMS bzw. XMS anlegen (seg005 in SCHWEIF.EXE)
 * BCC -mlarge -3 -Ot (C++)   -> 4145/4145 Bytes GLEICH
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 *
 * Funktion F_R05_0012 cd_check: CD-Pruefung (Kopierschutz der CD-Fassung), seit Welle KS (19.10.2026) als C
 *   (vorher Byteblock). Die Texte sind echte Literale (Phase B: Literalpool direkt hinter g_ltx_count_init, ds:445b..4537); der Abbruch per
 *   Sprung auf PSP:0000 ist im Original Inline-asm (TASM-Kodierung xor ax,ax = 31 C0).
 *
 * Funktionen:
 *   F_R05_0008  ctrl_break_handler  Ctrl-Break-Handler: liefert 1 (= weiterlaufen)
 *   F_R05_0012  cd_check            CD-Pruefung beim Start (PATH.CFG, MSCDEX, Datentraegername DSA2_CD1/2)
 *   F_R05_01A5  Init::Init          Konstruktor (Init_ctor)
 *   F_R05_0525  Init::setdir        Startverzeichnis merken, ins Datenverzeichnis wechseln
 *   F_R05_0588  Init::~Init         Destruktor (Init_dtor)
 *   F_R05_067D  load_archives       Datenarchive (Einzeldatei oder Namensliste) und Patchtabellen laden
 *   F_R05_0DD9  load_ltx            Namensliste *.LTX laden (Daten + Zeigertabelle)
 *   F_R05_0EF8  swap_mem_init       Auslagerungsspeicher der 3D-Ansicht in EMS bzw. XMS anlegen
 *                                   (frueher "sound_mem"; O010 lagert dorthin aus, R12 liest Texturen)
 *
 * Auffaelligkeiten des Originals (bewusst nachgebaut):
 *   - Init::Init: die XMS-Schleife reserviert 16-MB-Bloecke ohne Obergrenze in g_xms_handles[10];
 *     ein elfter Block wuerde g_xms_reserved (= g_xms_handles[10]) ueberschreiben (praktisch nur bei > 160 MB XMS).
 *   - Init::Init: if (EMS-KB != XMS-KB) ; else ems_is_xms = 1;  (leerer then-Zweig wie im Original;
 *     gleiche Werte = EMS wird aus XMS emuliert, dann gilt die strengere Speichergrenze)
 *   - load_archives: nach "speechcd.dat" wird fd zweimal auf -1 geprueft; size = 8 im selben
 *     Zweig ist tot (wird sofort durch size = 7 ueberschrieben).
 *   - load_ltx: Rueckgabe von open() wird nicht geprueft; die Suche nach dem Nullbyte vom
 *     Dateiende rueckwaerts hat keine Abbruchgrenze (ohne Nullbyte laeuft sie vor den Dateianfang).
 *   - load_ltx: atol() liest aus einem 5-Byte-Puffer ohne Endnull (Anzahl hoechstens 4 Ziffern
 *     sicher, sonst liest atol ueber den Puffer hinaus).
 * - Klasse Init in s_class.h; Vorbelegungen der Puffer (g_path_init, g_ltx_count_init) als Strukturkopie.
 * - g_file_names[4] (= g_fmt_cd_path) wird wie im Original ueber die Tabelle angesprochen.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <io.h>
#include <dos.h>
#include <dir.h>
#include <bios.h>
#include <fcntl.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:43dc..49c0, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
signed long g_xms_handles[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
signed short g_xms_reserved = 0;
struct str80 g_path_init = { "" };
struct str5 g_ltx_count_init = { "" };

#define NAMES_DUMMY_SIZE	20	/* Namensliste ohne .LTX (Einzeldatei): leere Daten */
#define NAMES_DUMMY_IDX_SIZE	8	/* und leere Zeigertabelle */

/* F_R05_0008: Ctrl-Break-Handler */
int ctrl_break_handler(void)
{
	return 1;
}

/* F_R05_0012: CD-Pruefung beim Start (Kopierschutz der CD-Fassung; O032 globals_ctor -> g_data_on_cd).
 * PATH.CFG nennt im ersten Zeichen das Laufwerk der CD. Das Laufwerk muss ein CD-ROM-Laufwerk sein (MSCDEX,
 * cd_check_drive) und eine Sternenschweif-CD mit dem Datentraegernamen "DSA2_CD1" oder "DSA2_CD2" enthalten;
 * sonst Aufforderung "Bitte Sternenschweif-CD in Laufwerk %c: einlegen und Taste druecken." und erneuter
 * Versuch, Esc bricht ab. Fehlt PATH.CFG, ist das Laufwerk kein CD-ROM oder bricht der Spieler ab, endet das
 * Programm sofort per Sprung auf PSP:0000 (dort steht INT 20h; ohne exit(), atexit-Funktionen laufen nicht).
 * Rueckgabe 1 = CD gefunden. */
signed short cd_check(void)
{
	signed short fd;
	register signed short quit = 0;
	register signed short found = 0;
	char far *msg_insert = "Bitte Sternenschweif-CD in Laufwerk %c: einlegen und Taste dr\x81""cken.";
	char far *msg_esc = "\n<ESC>";
	char cfg[40];			/* [0] Laufwerksbuchstabe aus PATH.CFG, ab [1] Datentraegername */

	fd = _open("path.cfg", O_RDONLY | O_BINARY);
	if (fd >= 0) {
#if FIX_0230
		/* Fix #230: leere PATH.CFG: kein Laufwerk statt Zufallswert (Vorbelegung statt Pruefung, spart Platz) */
		cfg[0] = 0;
		_read(fd, cfg, 1);
#else
		_read(fd, cfg, 1);
#endif
		_close(fd);
		if (cfg[0] >= 'a' && cfg[0] <= 'z')
			cfg[0] -= 'a' - 'A';
		if (cd_check_drive(cfg[0] - 'A')) {
			while (!quit && !found) {
				if (cd_get_label(cfg[0] - 'A', &cfg[1])) {
					/* keine CD im Laufwerk */
					printf(msg_insert, cfg[0]);
					if (bioskey(0) == KEY_ESC) {
						puts(msg_esc);
						quit = 1;
					} else
						puts("");
				} else if (!strcmp(&cfg[1], "DSA2_CD1   ") || !strcmp(&cfg[1], "DSA2_CD2   ")) {
					found = 1;
				} else {
					/* falsche CD */
					printf(msg_insert, cfg[0]);
					if (bioskey(0) == KEY_ESC) {
						puts(msg_esc);
						quit = 1;
					} else
						puts("");
				}
			}
		} else {
			puts("\nDas in PATH.CFG angegeben Laufwerk ist kein CD-ROM-Laufwerk.\n");
			quit = 1;
		}
	} else {
		puts("\nPATH.CFG nicht gefunden.\n");
		quit = 1;
	}
	if (quit) {
#if FIX_0231
		/* Fix #231: regulaeres Programmende (atexit-Funktionen laufen) */
		exit(1);
#else
		/* sofortiges Programmende: far-Sprung auf PSP:0000 (INT 20h) */
		asm mov bx, _psp
		asm xor ax, ax
		asm push bx
		asm push ax
		asm retf
#endif
	}
	return 1;
}

/* F_R05_01A5 */
Init::Init()
{
	signed short ovr_ok;
	signed long ovr_kb;
	signed short ems_is_xms;
	signed long handle;
	char buf[100];
	union REGS regs;
	register signed short i;
	register signed short xms_kb;

	xms_kb = 0;
	ems_is_xms = 0;
	g_ems_avail = cpu_v86_active();
	ctrlbrk(ctrl_break_handler);
	regs.x.ax = 0x3301;		/* DOS: Ctrl-Break-Pruefung aus */
	regs.x.dx = 0;
	intdos(&regs, &regs);
	setdir();
	ovr_ok = 0;
	g_mem_too_low = 0;
	if (xms_present()) {
		/* alles XMS in 16-MB-Bloecken reservieren ... */
#if FIX_0209
		/* Fix #209: hoechstens 10 Bloecke (Groesse von g_xms_handles) */
		for (i = 0; i < 10 && (handle = xms_alloc_kb(0x4000)) > 0; i++) {
#else
		for (i = 0; (handle = xms_alloc_kb(0x4000)) > 0; i++) {
#endif
			g_xms_reserved = 1;
			g_xms_handles[i] = handle;
		}
		xms_kb = xms_free_kb();
		/* ... den letzten Block durch einen kleineren ersetzen, wenn weniger als 15000 KB frei sind */
		if (g_xms_reserved != 0 && xms_kb < 15000) {
			i--;
			xms_free_block(g_xms_handles[i]);
			g_xms_handles[i] = xms_alloc_kb(xms_kb + 100);
		}
		xms_kb = xms_free_kb();
		g_xms_free_kb = xms_kb;
		if (g_ems_avail == 1) {
			ems_check();
			g_ems_free_pages = EMS_get_num_pages_unalloced();
			if (((signed long)g_ems_free_pages << 4) != (signed long)xms_kb) ;
			else ems_is_xms = 1;
		} else {
			g_ems_free_pages = 0;
		}
		/* Mindestbedarf: EMS 38 Seiten + 64 KB XMS bzw. 672 KB XMS */
		if (g_ems_avail == 1 && (g_ems_free_pages < 0x26 || xms_kb < 0x40)) {
			g_mem_too_low = 1;
		} else if (g_ems_avail == 0 && xms_kb < 0x2a0) {
			g_mem_too_low = 1;
		} else if (g_ems_avail != 0 && ems_is_xms != 0 && xms_kb < 0x2a0) {
			g_mem_too_low = 1;
		}
		if (g_mem_too_low == 0) {
			signed short left_kb;
			left_kb = 0;
			swap_mem_init();
			left_kb = xms_free_kb();
			if (left_kb < 0x40) g_mem_too_low = 1;
			/* Rest als Overlay-Puffer */
			if (g_mem_too_low == 0 && xms_free_kb() > 200) {
				if (xms_free_kb() > 2000) ovr_kb = 500;
				else if (xms_free_kb() > 500) ovr_kb = 200;
				else ovr_kb = 100;
				ovr_ok = 1;
			}
		}
	} else {
		g_mem_too_low = 1;
	}
	if (g_mem_too_low == 1) {
		puts("\nSternenschweif ben\x94tigt einen installierten XMS/EMS Treiber,");
		puts("und mindestens   688128 Byte freien XMS/EMS Speicher.");
		if (g_ems_avail == 1) {
			puts("Wovon mindestens 622592 Bytes als EMS Speicher und");
			puts("                  65536 Bytes als XMS Speicher ansprechbar sein m\x81ssen!");
			sprintf(buf, "\nIhr System hat nur %8ld Byte freien EMS Speicher und", ((signed long)g_ems_free_pages << 4) << 10);
			puts(buf);
			sprintf(buf, "                   %8ld Byte freien XMS Speicher.", (signed long)xms_kb << 10);
			puts(buf);
			puts("\nSollten beide Werte in Ihrer DOS-Anzeige den Anforderungen entsprechen,");
			puts("so nutzt Ihr Speichermanager den Speicher doppelt!");
			puts("\nDa Ihre momentane Systemkonfiguration einen stabilen Spielverlauf");
			puts("von Sternenschweif verhindert, bitten wir Sie vor einem erneuten Start");
			puts("die Hinweise im beiliegenden Faltblatt und die Beispielkonfigurationen");
			puts("in der Datei CONFIG.BSP zu ber\x81""cksichtigen.");
		} else {
			sprintf(buf, "\nIhr System hat nur %8ld Byte freien XMS/EMS Speicher!", (signed long)g_xms_free_kb << 10);
			puts(buf);
		}
		g_mem_too_low = 1;
	}
	if (ovr_ok) {
		if (_OvrInitExt(0L, ovr_kb << 10) == 0) {
			sprintf(buf, "\nXMS (%ld KByte) for C++ Overlay handling utilized.", ovr_kb);
			puts(buf);
		}
	} else if (g_mem_too_low == 0) {
		puts("\nNo memory available to C++ Overlay handler.");
	}
	load_archives();
}

/* F_R05_0525 */
void Init::setdir()
{
	char path[80];

	strcpy(g_start_dir, "X:\\");
	g_start_dir[0] = getdisk() + 'A';
	getcurdir(0, g_start_dir_path);
	strcpy(path, g_start_dir);
	strcat(path, g_path_fmts[2]);
	chdir(path);
}

/* F_R05_0588 */
Init::~Init()
{
	signed short res;
	struct ffblk ff;
	signed short i;

	/* Temp-Dateien loeschen */
	sprintf(g_text_output_buf, g_path_fmts[0], "*.*");
	res = findfirst(g_text_output_buf, &ff, 0);
	if (res == 0) do {
		sprintf(g_text_output_buf, g_path_fmts[0], ff.ff_name);
		unlink(g_text_output_buf);
		res = findnext(&ff);
	} while (res == 0);
	chdir(g_start_dir);
	for (i = 0; i < 10; i++) {
		if (g_xms_handles[i] != 0) xms_free_block(g_xms_handles[i]);
	}
	game_free_all();
	::operator delete(g_palette);
}

/* F_R05_067D: je Archiv entweder Einzeldatei (*.dat vorhanden: leere Namensliste, Patchtabelle laden)
 * oder Namensliste *.ltx; STARCD/SPEECHCD auch ueber das CD-Laufwerk aus ..\path.cfg */
void load_archives(void)
{
	signed short fd;
	signed short size;
	signed short disk;
	struct str80 path = g_path_init;

	fd = open("star.dat", O_RDONLY | O_BINARY);
	if (fd != -1) {
		close(fd);
		puts("\nONEFILE Version");
		g_names_star = mem_alloc(NAMES_DUMMY_SIZE);
		g_names_star_idx = mem_alloc(NAMES_DUMMY_IDX_SIZE);
		size = 0x9c;
		g_patch_star = mem_alloc(size);
		memset(g_patch_star, 0, size);
		fd = open("dpatch.dat", O_RDONLY | O_BINARY);
		if (fd != -1) {
			read(fd, g_patch_star, size);
			close(fd);
		}
	} else {
		load_ltx("filename.ltx", &g_names_star, &g_names_star_idx);
	}
	fd = open("fx.dat", O_RDONLY | O_BINARY);
	if (fd != -1) {
		close(fd);
		g_names_fx = mem_alloc(NAMES_DUMMY_SIZE);
		g_names_fx_idx = mem_alloc(NAMES_DUMMY_IDX_SIZE);
		size = 0x17;
		g_patch_fx = mem_alloc(size);
		memset(g_patch_fx, 0, size);
		fd = open("fpatch.dat", O_RDONLY | O_BINARY);
		if (fd != -1) {
			read(fd, g_patch_fx, size);
			close(fd);
		}
	} else {
		load_ltx("fxname.ltx", &g_names_fx, &g_names_fx_idx);
	}
	fd = open("speech.dat", O_RDONLY | O_BINARY);
	if (fd != -1) {
		close(fd);
		g_names_speech = mem_alloc(NAMES_DUMMY_SIZE);
		g_names_speech_idx = mem_alloc(NAMES_DUMMY_IDX_SIZE);
		size = 0x1f;
		g_patch_speech = mem_alloc(size);
		memset(g_patch_speech, 0, size);
		fd = open("spatch.dat", O_RDONLY | O_BINARY);
		if (fd != -1) {
			read(fd, g_patch_speech, size);
			close(fd);
		}
	} else {
		fd = open("pakname.ltx", O_RDONLY | O_BINARY);
		if (fd != -1) {
			close(fd);
			load_ltx("pakname.ltx", &g_names_speech, &g_names_speech_idx);
		}
	}
	fd = open("raw.dat", O_RDONLY | O_BINARY);
	if (fd != -1) {
		close(fd);
		g_names_raw = mem_alloc(NAMES_DUMMY_SIZE);
		g_names_raw_idx = mem_alloc(NAMES_DUMMY_IDX_SIZE);
		size = g_nr_raw_files / 8 + 1;
		g_patch_raw = mem_alloc(size);
		memset(g_patch_raw, 0, size);
		fd = open("rpatch.dat", O_RDONLY | O_BINARY);
		if (fd != -1) {
			read(fd, g_patch_raw, size);
			close(fd);
		}
	} else {
		load_ltx("rawname.ltx", &g_names_raw, &g_names_raw_idx);
	}
	fd = open(g_file_names[0], O_RDONLY | O_BINARY);
	if (fd != -1) {
		/* STARCD.DAT im Verzeichnis */
		close(fd);
		g_speech_ext_avail = 1;
		g_names_starcd = mem_alloc(NAMES_DUMMY_SIZE);
		g_names_starcd_idx = mem_alloc(NAMES_DUMMY_IDX_SIZE);
		size = 7;
	} else {
		fd = open("..\\path.cfg", O_RDONLY | O_BINARY);
		if (fd != -1) {
			/* CD-Laufwerk aus ..\path.cfg in "X:\DATA\%s" eintragen */
			disk = getdisk();
			read(fd, g_file_names[4], 1);
			close(fd);
			if (*g_file_names[4] >= 'a' && *g_file_names[4] <= 'z') *g_file_names[4] = *g_file_names[4] - 0x20;
			setdisk(*g_file_names[4] - 'A');
			if (getdisk() == disk) {
				g_speech_ext_avail = 0;
				*g_file_names[4] = disk + 'A';
			} else {
				setdisk(disk);
				sprintf(path.s, g_file_names[4], g_file_names[0]);
				fd = open(path.s, O_RDONLY | O_BINARY);
				if (fd != -1) {
					close(fd);
					g_speech_ext_avail = 1;
					g_data_on_cd = 1;
				} else {
					sprintf(path.s, g_file_names[4], "attic.fli");
					fd = open(path.s, O_RDONLY | O_BINARY);
					if (fd != -1) {
						close(fd);
						g_speech_ext_avail = 1;
						g_data_on_cd = 1;
						load_ltx("cdfname.ltx", &g_names_starcd, &g_names_starcd_idx);
					}
				}
			}
		} else {
			fd = open("cdfname.ltx", O_RDONLY | O_BINARY);
			if (fd != -1) {
				close(fd);
				load_ltx("cdfname.ltx", &g_names_starcd, &g_names_starcd_idx);
			} else {
				g_names_starcd = mem_alloc(NAMES_DUMMY_SIZE);
				g_names_starcd_idx = mem_alloc(NAMES_DUMMY_IDX_SIZE);
			}
			fd = open("attic.fli", O_RDONLY | O_BINARY);
			if (fd != -1) {
				close(fd);
				g_speech_ext_avail = 1;
			}
		}
	}
	fd = open("speechcd.dat", O_RDONLY | O_BINARY);
	if (fd == -1) {
		sprintf(path.s, g_file_names[4], "speechcd.dat");
		fd = open(path.s, O_RDONLY | O_BINARY);
	}
	if (fd != -1) {
#if FIX_0209 || FIX_0230 || FIX_0231
		/* Platz fuer die Fehlerbehebungen in diesem residenten Segment (es darf nicht laenger werden): doppelte
		 * Pruefung und das tote "size = 8" (sofort durch size = 7 ueberschrieben) entfallen - gleiches Verhalten */
		close(fd);
		g_names_speechcd = mem_alloc(NAMES_DUMMY_SIZE);
		g_names_speechcd_idx = mem_alloc(NAMES_DUMMY_IDX_SIZE);
#else
		if (fd != -1) close(fd);
		g_names_speechcd = mem_alloc(NAMES_DUMMY_SIZE);
		g_names_speechcd_idx = mem_alloc(NAMES_DUMMY_IDX_SIZE);
		size = 8;
#endif
	} else {
		fd = open("cdpname.ltx", O_RDONLY | O_BINARY);
		if (fd != -1) {
			close(fd);
			load_ltx("cdpname.ltx", &g_names_speechcd, &g_names_speechcd_idx);
		} else {
			g_names_speechcd = mem_alloc(NAMES_DUMMY_SIZE);
			g_names_speechcd_idx = mem_alloc(NAMES_DUMMY_IDX_SIZE);
		}
	}
	size = 7;
	g_patch_starcd = mem_alloc(size);
	memset(g_patch_starcd, 0, size);
	fd = open("dcdpatch.dat", O_RDONLY | O_BINARY);
	if (fd != -1) {
		read(fd, g_patch_starcd, size);
		close(fd);
	}
	size = 8;
	g_patch_speechcd = mem_alloc(size);
	memset(g_patch_speechcd, 0, size);
	fd = open("scdpatch.dat", O_RDONLY | O_BINARY);
	if (fd != -1) {
		read(fd, g_patch_speechcd, size);
		close(fd);
	}
}

/* F_R05_0DD9: Namensliste laden; am Dateiende steht nach einem Nullbyte die Zahl der Namen (Text) */
void load_ltx(char far *name, void far **data, void far **idx)
{
	signed long len;
	signed long pos;
	struct str5 count_txt;
	signed short count;
	signed short fd;

	fd = open(name, O_RDONLY | O_BINARY);
	len = filelength(fd);
	pos = 0;
	count_txt = g_ltx_count_init;
	do {
		pos--;
		lseek(fd, pos, SEEK_END);
		read(fd, &count_txt, 1);
	} while (count_txt.s[0] != 0);
	len -= read(fd, &count_txt, 5);
	count = atol(count_txt.s);
	*data = mem_alloc(len);
	*idx = mem_alloc(count * 4);
	lseek(fd, 0L, SEEK_SET);
	read(fd, *data, len);
	close(fd);
	txt_build_index(*idx, *data, len);
}

/* F_R05_0EF8: 700 KB (bei knappem Speicher 600 KB, Sparbetrieb) in EMS bzw. XMS;
 * zweiter Block: 351 KB XMS bzw. im Sparbetrieb derselbe Block */
void swap_mem_init(void)
{
	signed short avail;

	g_swap_kb = 700;
	if (g_ems_avail == 1) {
		avail = EMS_get_num_pages_unalloced();
		if (avail >= 0x70) {
			ems_alloc((((signed long)g_swap_kb << 10) + 0x3fff) >> 14, &g_swap_handle);
			g_swap_pos = 0;
			g_ems_frame = ems_frame_seg();
			g_swap_handle2 = xms_alloc_kb(0x15f);
			g_mem_saving = 0;
		} else if (avail >= 0x26) {
			g_swap_kb = 600;
			ems_alloc((((signed long)g_swap_kb << 10) + 0x3fff) >> 14, &g_swap_handle);
			g_swap_pos = 0;
			g_ems_frame = ems_frame_seg();
			g_swap_handle2 = g_swap_handle;
			g_mem_saving = 1;
		} else {
			g_mem_too_low = 1;
		}
		return;
	}
	avail = xms_free_kb();
	if (avail >= 1800) {
		g_swap_handle2 = xms_alloc_kb(0x15f);
		g_swap_handle = xms_alloc_kb(g_swap_kb);
		g_swap_pos = 0;
		g_mem_saving = 0;
		return;
	}
	if (avail >= 600) {
		g_swap_kb = 600;
		g_swap_handle2 = g_swap_handle = xms_alloc_kb(g_swap_kb);
		g_swap_pos = 0;
		g_mem_saving = 1;
		return;
	}
	g_mem_too_low = 1;
}
