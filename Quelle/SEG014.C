/* SEG014 - "GURU Meditation": Absturzbericht mit den letzten 10 Funktionsnummern (g_func_trace) und der Zahl
 * der EMS-Handles (int 67h/4Bh; der Text sagt "XMS handles"), wahlweise Liste der Speicherbloecke nach
 * ALLOC.CAT (ptr == 1) oder Speicherauszug um ptr nach SCHWEIF.CAT; danach Tastendruck abwarten: Strg-Q
 * beendet, F5/Strg-J springt in den Debugger (int 3), sonst weiter im Grafikmodus. push_fc schreibt die
 * Funktionsnummern mit (seg014 in SCHWEIF.EXE)
 * BCC -mlarge -3 -Oi -Ot -c -Y -E..\bc\BIN\TASM.EXE SEG014.C   -> 1052/1052 Bytes GLEICH
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * - g_mem_blocks hat far-Zeiger; die Blocklaenge rechnet das Original huge (F_PSBP@) -> Casts.
 * - Originalfehler: der Speicherauszug schreibt ab ptr - 500 zweimal 0x7d00 Bytes, das zweite Stueck ab
 *   ptr - 500 + 0x7d00 (ohne Pruefung der Grenzen).
 */
/* Phase B: BCC (C-Modus) legt das BSS in umgekehrter Reihenfolge der ersten Deklaration an - Reihenfolge hier vor schweif.h */
extern void far *g_dbg_src;
extern void far *g_dbg_dest;
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:5ee8..605a, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
signed short g_func_trace[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
signed short g_dev_screen_on = 0;
char g_unused_5efe[2] = { 0 };   /* ds:5efe, unbenutzt */
signed short g_xms_free_kb = 0;
signed short g_min_stack = 32000;
signed short g_dbg_raster_mouse = 0;	/* ds:5f04 Fehlersuche: Maus-ISR faerbt Farbe 0 (SEG023) */
signed short g_mem_debug = 1;
signed short g_mem_debug2 = 1;
signed short g_func_trace_on = 0;
signed short g_dbg_raster_timer = 0;	/* ds:5f0c Fehlersuche: Zeitgeber faerben Farbe 0 (SEG023) */
char g_unused_5f0e[10] = { 0 };   /* ds:5f0e, unbenutzt */
void far *g_debug_txt_buf = 0;
char far * far *g_debug_txt = 0;
struct str3 g_str_crlf = { "\r\n" };
/* Phase B: eigenes BSS (ds:c910..c918) in Adressreihenfolge (Reihenfolge festgelegt durch die extern-Zeilen vor den #include) */
void far *g_dbg_dest;
void far *g_dbg_src;

void guru_meditation(char far *msg, unsigned char huge *ptr)
{
	signed short trace_bak;
	signed short ems_handles;
	unsigned char far *pal;
	signed short fd;
	signed short i;
	struct str3 crlf;
	char far *buf;

	mouse_hide(0);
	pal = mem_alloc(NR_COLORS * 3);
	get_palette(pal, 0, NR_COLORS);
	g_dialog_depth++;
	trace_bak = g_func_trace_on;
	g_func_trace_on = 0;
	ani_halt(g_screen_pic);
	restore_video_mode(&g_video_state);
	g_pp20_index = -1;
	textcolor(BLINK | LIGHTRED);
	cputs("                         STARTRAIL  --  GURU Meditation\r\n");
	textcolor(LIGHTRED);
	cputs(msg);

	if (g_ems_avail) {
		asm mov ah, 4bh
		asm int 67h
		asm mov ems_handles, bx
	} else {
		ems_handles = 1;
	}

	sprintf(g_portrait_buf, "\r\nFCs: %d %d %d %d %d %d %d %d %d %d\r\nXMS handles: %d\r\n", g_func_trace[0], g_func_trace[1], g_func_trace[2],
		g_func_trace[3], g_func_trace[4], g_func_trace[5], g_func_trace[6], g_func_trace[7],
		g_func_trace[8], g_func_trace[9], ems_handles);
	cputs(g_portrait_buf);
	sprintf(g_portrait_buf, "%s\r\nFCs: %d %d %d %d %d %d %d %d %d %d\r\nXMS handles: %d\r\n", msg, g_func_trace[0], g_func_trace[1], g_func_trace[2],
		g_func_trace[3], g_func_trace[4], g_func_trace[5], g_func_trace[6], g_func_trace[7],
		g_func_trace[8], g_func_trace[9], ems_handles);
	debug_log(g_portrait_buf);

	if (g_dbg_dest)
		cprintf("\r\nDest: 0x%lx\r\n", g_dbg_dest);
	if (g_dbg_src)
		cprintf("\r\nSource: 0x%lx\r\n", g_dbg_src);

	if ((signed long)ptr == 1) {
		/* Liste der Speicherbloecke */
		crlf = g_str_crlf;
		buf = mem_alloc(100);
		fd = _creat("ALLOC.CAT", 0);
		for (i = 0; i < NR_MEM_BLOCKS; i++) {
			sprintf(buf, "Allocentry %4d: Start: %p End: %p Len: %ld%s", i, g_mem_blocks[i].start, g_mem_blocks[i].end,
				(signed long)((unsigned char huge *)g_mem_blocks[i].end -
					(unsigned char huge *)g_mem_blocks[i].start), &crlf);
			write(fd, buf, strlen(buf));
		}
		close(fd);
		mem_free(buf);
	} else if ((signed long)ptr) {
		/* Speicherauszug um ptr */
		register signed short dump_fd;

		cprintf("\r\nAddress: 0x%lx\r\n", ptr);
		sprintf(g_portrait_buf, "\r\nAddress: 0x%lx\r\n", ptr);
		debug_log(g_portrait_buf);
		dump_fd = _creat("SCHWEIF.CAT", 0);
		ptr = ptr - 500;
		write(dump_fd, ptr, 0x7d00);
		ptr = ptr + 0x7d00;
		write(dump_fd, ptr, 0x7d00);
		close(dump_fd);
	}

	textcolor(LIGHTGRAY);
	g_action = g_key = g_key_ascii = 0;
	do {
		poll_input();
	} while (g_action == 0);

	if (g_key == KEY_CTRL_Q) {
		sys_exit(0);
		exit(0);
	} else if (g_key == KEY_F5 || g_key == KEY_CTRL_J) {
		asm nop
		asm int 3
	}

	g_action = g_key = g_key_ascii = 0;
	g_refresh = 1;
	set_video_mode(VIDEO_MODE_VGA);
	set_palette(pal, 0, NR_COLORS);
	mem_free(pal);
	mouse_show(0);
	g_func_trace_on = trace_bak;
}

/* Funktionsnummer an die Liste der letzten 10 anhaengen (nur bei g_func_trace_on), danach mem_check */
void push_fc(signed short id)
{
	if (g_func_trace_on) {
		g_func_trace[0] = g_func_trace[1];
		g_func_trace[1] = g_func_trace[2];
		g_func_trace[2] = g_func_trace[3];
		g_func_trace[3] = g_func_trace[4];
		g_func_trace[4] = g_func_trace[5];
		g_func_trace[5] = g_func_trace[6];
		g_func_trace[6] = g_func_trace[7];
		g_func_trace[7] = g_func_trace[8];
		g_func_trace[8] = g_func_trace[9];
		g_func_trace[9] = id;
		mem_check();
	}
}
