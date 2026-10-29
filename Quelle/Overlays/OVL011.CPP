/* OVL011 - Overlay O011 (Segment 0x1f36 in SCHWEIF.EXE): Entwickler-Infobildschirm (DEBUG.LTX) mit Befehlseingabe.
 * dev_info_screen: nur wenn nicht schon aktiv (g_dev_screen_on), Palette sichern (get_palette), Texte 0x1f0 (0x1cc
 * Stueck) ggf. nach g_debug_txt_buf/g_debug_txt laden, Textmodus (restore_video_mode) und Speicher-/Systemwerte
 * ausgeben (g_mem_free_start/g_mem_min_free, arc_info, XMS g_xms_free_kb, EMS g_ems_free_pages bzw. int 67h/4Bh,
 * xms_free_kb, Texturen g_nr_textures/g_swap_used_kb/g_mem_saving, offene Dateien dos_open_files, SP, g_min_stack,
 * farcoreleft, Heapzustand heap_free_total). Dann Eingabezeile (bioskey/toupper, Ende mit Enter) und Befehle
 * txt[0x10..0x13] (MEMORY/PROTOCOL: g_mem_debug/g_func_trace_on an/aus, mit Tonfolge dev_beep), VIEW/SET und
 * PVIEW/PSET (Bytefelder g_flags und g_playflags), ALLOCTABLE (Speicherbloecke g_mem_blocks), GODACCOUNT
 * (g_god_favor), CHECKSUM (Pruefsumme eines Codebereichs im Segment von r03_0006); zum Schluss Grafikmodus 0x13,
 * Palette zurueck, Texte freigeben, cam_set_from_group. dev_beep: Tonfolge 440/587/659/880 Hz. heap_free_total:
 * freier Speicher (farcoreleft + freie Heapbloecke per farheapwalk; -1 = Heap leer, -2 = defekt).
 * unused_dev_stub1/0a8d/0a97 und debug_log (aus R14 guru_meditation mit dem Bericht) leer. dos_open_files: offene
 * Dateihandles (JFT aus PSP:34h, Laenge PSP:32h, Eintrag != 0xff).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL011.CPP   -> 2785/2785 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h, Welle N20: 19.10.2026 GLEICH, LINKGLEICH)
 * - C++ wegen atoi: in C++ ist atoi in STDLIB.H inline ("return (int) atol(s)") -> jede der 7 Aufrufstellen
 *   hat eine eigene Parameterkopie (mov eax,[p] / mov [bp-2ch..-44h],eax).
 * - Ganzer Rumpf in "if (!g_dev_screen_on) { .. }" (cmp / jz short / jmp near ans Ende, asm2tasm: db 0e9h); mit
 *   "if (g_dev_screen_on) return;" erzeugt -Og eine Epilogkopie.
 * - Stack: Funktionsebene i, x, mem, n, v, p, spv, loaded; dann switch-Kopie [bp-1eh] und -Og-Zwischenwerte
 *   [bp-20h..-24h]; pal liegt dahinter ([bp-28h]) -> im if-Block deklariert; dann inline-Kopien, Block der
 *   Pruefsumme, zuletzt buf[40] ([bp-78h]). Arrays/Objekte ueber 10 Bytes (12 getestet; 8/10 nicht) legt BCC
 *   unabhaengig von der Deklarationsstelle ans Ende ihres Blocks, nach inline-Kopien und inneren Bloecken.
 * - switch auf long (mem): Vergleichstabelle im Code (cx = 2, loop, jmp cs:[bx+4]), Faelle -1/-2/default.
 * - Pruefsummenbereich als MK_FP(FP_SEG(r03_0006), 6) bzw. 0x278 -> nur Segment-Relokation, Offsets literal;
 *   Vergleich q < e nur ueber den Offset (jc). "sum += *q" -> mov ah,0 / movsx eax,ax.
 * - "(i >> 2) + 14" (sar) im ersten, "i / 4 + 13" (cwd/idiv) im zweiten Anzeigeblock - so im Original.
 * - Zuweisungsketten rechts zuerst: "g_key_ascii = g_action = g_key = 0" bzw. am Ende "g_action = g_key = g_key_ascii = 0".
 * - dos_open_files ohne register: n -> DI, cnt -> SI, i -> CX (mit register-Angaben andere Verteilung).
 * - Originalfehler: Eingabe ohne Laengenpruefung in buf[40] (Stackueberlauf ab 40 Zeichen); Anzeige-/Setzbefehle
 *   pruefen den Index n nicht.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>
#include <bios.h>
#include <dos.h>
#include <alloc.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:059c..0626, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */

#define DEBUG_TXT	0x1f0		/* DEBUG.LTX (0x1cc Texte) */

void dev_info_screen(void)
{
	signed char i;
	signed short x[4];
	signed long mem;
	signed short n;
	signed short v;
	char far *p;
	signed short spv;
	signed short loaded;
	char buf[40];

	i = 0;
	p = buf;
	loaded = 0;
	if (!g_dev_screen_on) {
		unsigned char far *pal;

		g_dev_screen_on = 1;
		g_dialog_depth++;
		pal = (unsigned char far *)mem_alloc(0x300L);
		get_palette(pal, 0, 0x100);
		if (g_debug_txt_buf == 0) {
			loaded = 1;
			((Font far *)g_font)->load_list(DEBUG_TXT, 0x1cc, &g_debug_txt_buf, &g_debug_txt);
		}
		mouse_hide(0);
		ani_halt(g_screen_pic);
		g_pp20_index = -1;
		restore_video_mode(&g_video_state);
		gotoxy(1, 1);
		textcolor(12);
		cputs(g_debug_txt[0]);
		printf(g_version_str);
		textcolor(7);
		printf(g_debug_txt[1], g_mem_free_start);
		printf("\n");
		printf(g_debug_txt[2], g_mem_min_free);
		printf("\n");
		arc_info(g_file_3, x);
		printf("\n");
		printf(g_debug_txt[3], (signed long)g_xms_free_kb << 10);
		if (g_ems_avail)
			printf("\nBasic EMS amount available to program:             %ld", ((signed long)g_ems_free_pages << 4) << 10);
		printf("\n");
		printf(g_debug_txt[4], ((signed long)g_xms_free_kb << 10) - ((signed long)xms_free_kb() << 10));
		printf("\n");
		printf(g_debug_txt[5], g_nr_textures, (signed long)g_swap_used_kb << 10, g_mem_saving == 1 ? 2 : 4);
		printf("\n");
		if (g_ems_avail) {
			_AH = 0x4b;
			geninterrupt(0x67);
			v = _BX;
			printf(g_debug_txt[0x145], v);
			printf("\n");
		}
		printf("Used File handles by this process:                %2d\n", dos_open_files());
		spv = _SP;
		printf(g_debug_txt[6], spv);
		printf("\n");
		printf(g_debug_txt[0x111], g_min_stack);
		printf("\n");
		printf(g_debug_txt[7], farcoreleft());
		printf("\n");
		mem = heap_free_total();
		switch (mem) {
		case -1:
			printf(g_debug_txt[8]);
			break;
		case -2:
			printf(g_debug_txt[9]);
			break;
		default:
			printf(g_debug_txt[10], mem);
			break;
		}
		gotoxy(1, 25);
		cprintf(g_debug_txt[11]);
		do {
			screensaver_reset(&g_saver);
			g_key_ascii = g_action = g_key = 0;
			if (bioskey(1)) {
				g_key = g_action = bioskey(0);
				g_key_ascii = g_key & 0xff;
			}
#if FIX_0032
			/* Fix #32: Eingabe auf buf[40] begrenzen (Enter passt immer noch) */
			if (g_key_ascii && (i < 39 || (i < 40 && g_action == KEY_RETURN))) {
				buf[i] = toupper(g_key_ascii);
				cprintf("%c", buf[i++]);
			}
#else
			if (g_key_ascii) {
				buf[i] = toupper(g_key_ascii);
				cprintf("%c", buf[i++]);
			}
#endif
		} while (g_action != KEY_RETURN);
		buf[i - 1] = 0;
		if (!strcmp(buf, g_debug_txt[0x10])) {
			dev_beep();
			g_mem_debug = 0;
		}
		if (!strcmp(buf, g_debug_txt[0x11])) {
			dev_beep();
			g_mem_debug = 1;
		}
		if (!strcmp(buf, g_debug_txt[0x12])) {
			dev_beep();
			g_func_trace_on = 0;
		}
		if (!strcmp(buf, g_debug_txt[0x13])) {
			dev_beep();
			g_func_trace_on = 1;
		}
		if (!strncmp(buf, g_debug_txt[0x16], 4)) {
			p += 5;
			n = atoi(p);
			for (i = 0; i < 40; i += 4) {
				gotoxy(1, (i >> 2) + 14);
				cprintf(g_debug_txt[0x18], n + i, g_flags[n + i], n + i + 1, g_flags[n + i + 1],
					n + i + 2, g_flags[n + i + 2], n + i + 3, g_flags[n + i + 3]);
			}
			getch();
		}
		if (!strncmp(buf, g_debug_txt[0x17], 3)) {
			p += 4;
			n = atoi(p);
			while (*p != 0 && *p != ' ')
				p++;
			v = atoi(p);
			g_flags[n] = v;
			dev_beep();
		}
		if (!strncmp(buf, g_debug_txt[0xd7], 5)) {
			p += 6;
			n = atoi(p);
			for (i = 0; i < 40; i += 4) {
				gotoxy(1, i / 4 + 13);
				cprintf(g_debug_txt[0xd9], n + i, g_playflags[n + i], n + i + 1, g_playflags[n + i + 1],
					n + i + 2, g_playflags[n + i + 2], n + i + 3, g_playflags[n + i + 3]);
			}
			getch();
		}
		if (!strncmp(buf, g_debug_txt[0xd8], 4)) {
			p += 5;
			n = atoi(p);
			while (*p != 0 && *p != ' ')
				p++;
			v = atoi(p);
			g_playflags[n] = v;
			dev_beep();
		}
		if (!strncmp(buf, g_debug_txt[0x141], 10)) {
			p += 11;
			n = atoi(p);
			for (i = 0; i < 30; i += 3) {
				gotoxy(1, i / 3 + 14);
				cprintf(g_debug_txt[0x142], n + i, g_mem_blocks[n + i].start, n + i + 1, g_mem_blocks[n + i + 1].start,
					n + i + 2, g_mem_blocks[n + i + 2].start);
			}
			getch();
		}
		if (!strncmp(buf, g_debug_txt[0x1b3], 10)) {
			for (i = 0; i < 14; i += 3) {
				gotoxy(1, i / 3 + 14);
				cprintf(g_debug_txt[0x1b4], i, g_god_favor[i], i + 1, g_god_favor[i + 1], i + 2, g_god_favor[i + 2]);
			}
			getch();
		}
		if (!strcmp(buf, g_debug_txt[0x1cb])) {
			signed long sum;
			unsigned char far *q;
			unsigned char far *e;

			sum = 0;
			q = (unsigned char far *)MK_FP(FP_SEG(r03_0006), 6);
			e = (unsigned char far *)MK_FP(FP_SEG(r03_0006), 0x278);
			for (; q < e; q++)
				sum += *q;
			gotoxy(1, 16);
			cprintf("%ld", sum);
			getch();
		}
		g_action = g_key = g_key_ascii = 0;
		g_refresh = 1;
		set_video_mode(0x13);
		set_palette(pal, 0, 0x100);
		mem_free(pal);
		mouse_show(0);
		if (loaded) {
			g_debug_txt_buf = mem_free(g_debug_txt_buf);
			g_debug_txt = (char far * far *)mem_free(g_debug_txt);
		}
		g_dialog_depth--;
		g_dev_screen_on = 0;
		cam_set_from_group();
	}
}

void dev_beep(void)
{
	sound(440);
	delay(240);
	sound(587);
	delay(240);
	sound(659);
	delay(240);
	sound(880);
	delay(720);
	nosound();
}

signed long heap_free_total(void)
{
	signed long total;
	struct farheapinfo hi;

	total = farcoreleft();
	switch (farheapcheck()) {
	case _HEAPEMPTY:
		total = -1;
		break;
	case _HEAPCORRUPT:
		total = -2;
		break;
	case _HEAPOK:
		hi.ptr = 0;
		while (farheapwalk(&hi) != _HEAPEND) {
			if (hi.in_use == 0)
				total = total + hi.size;
		}
		break;
	}
	return total;
}

void unused_dev_stub1(void)
{
}

void unused_dev_stub2(void)
{
}

void debug_log(char far *report)
{
}

void unused_dev_stub3(void)
{
}

signed short dos_open_files(void)
{
	unsigned char far *jft;
	signed short cnt;
	signed short n;
	signed short i;

	jft = *(unsigned char far * far *)MK_FP(_psp, 0x34);
	n = *(signed short far *)MK_FP(_psp, 0x32);
	cnt = 0;
	for (i = 0; i < n; i++) {
		if (jft[i] != 0xff)
			cnt++;
	}
	return cnt;
}
