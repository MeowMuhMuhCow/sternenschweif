/* OVL193 - Overlay O193 (Segment 0x2335 in SCHWEIF.EXE): Maus - Mausvariablen zuruecksetzen und Zeigerbilder
 * laden (mouse_init_vars), Maustreiber pruefen und Ereignis-Handler (int 33h Fn 0Ch ueber Vektor 79h) setzen
 * (mouse_init/mouse_irq_init), Abbau per atexit (disable_mouse/mouse_reset_ehandler), Mauszeigerbild neu
 * auspacken (mouse_reset) - wie mouse_init/mouse_irq_init/mouse_reset_ehandler in DSA 1 (BE seg002).
 *
 * BCC -mlarge -3 -O -Ot -c -Y OVL193.C   -> 697/697 Bytes GLEICH (18.10.2026)
 *   (Namensfassung mit schweif.h, Welle N20: 19.10.2026 GLEICH, LINKGLEICH)
 * - mouse_action(&ax,&bx,&cx,&dx,&es), Locals l1..l5 in Deklarationsreihenfolge [bp-2]..[bp-0Ah].
 * - Adressen ohne eigenes Public im Original: Maus-ISR R23:0437 und Ereignis-Handler R18:016A werden als
 *   (char far *)r23_000f + 0x428 bzw. mouse_show + 0x161 geschrieben (die ASM-Fassung hat dort nur einen
 *   Segment-Fixup auf F_R23_000F/F_R18_0009 -> cobjfix behaelt den Namen, der Inline-Wert ergibt die Adresse).
 * - Ketten a = b = c = 0 speichern von rechts nach links (Speicherreihenfolge im Original).
 * - g_mouse_locked == 0 statt !g_mouse_locked (sonst mov al/cbw/or ax,ax statt cmp byte).
 * - nvf_new/nvf_delete(this, 2) = Konstruktor/Destruktor eines lokalen NvfMem (0x1a Bytes), hier als C
 *   ausgeschrieben.
 */
#include <dos.h>
#include <stdlib.h>
#include "schweif.h"

#define CURSOR_FILE	2		/* Datei der Mauszeigerbilder im Hauptarchiv */
#define MOUSE_IRQ	0x1f		/* Ereignismaske fuer int 33h Fn 0Ch */
#define MOUSE_VEC	0x79		/* Vektor fuer den Ereignis-Handler */

void mouse_init_vars(void)
{
	register signed short len;

	g_mouse_busy = g_mouse_moved = g_mouse_irq_init = g_mouse_flag = 0;
	mouse_clear_left();
	mouse_clear_right();
	g_mouse_ldclick = g_mouse_rdclick = 0;
	g_cursor_hot_x = g_cursor_hot_y = 1;
	g_mouse_handler_on = 0;
	g_mouse_show_cnt = -1;
	g_mouse_posx = 0xf0;
	g_mouse_posy = 0x64;
	file_open(g_file_main, CURSOR_FILE);
	len = file_length2(g_file_main);
	g_cursor_nvf = mem_alloc((signed long)len);
	file_read(g_file_main, g_cursor_nvf, (signed long)len);
	file_close(g_file_main);
	mouse_set_cursor(0, 1);
}

void mouse_init(void)
{
	signed short l1, l2, l3, l4, l5;

	mouse_init_vars();
	l1 = 0;
	mouse_action(&l1, &l2, &l3, &l4, &l5);
	if (l1) {
		l1 = 4;
		l3 = g_mouse_posx;
		l4 = g_mouse_posy;
		mouse_action(&l1, &l2, &l3, &l4, &l5);
		mouse_irq_init(MOUSE_IRQ, (void interrupt (*)(void))((char far *)r23_000f + 0x428));
		g_mouse_locked = 0;
		atexit(disable_mouse);
	} else {
		g_mouse_locked = 1;
	}
}

void disable_mouse(void)
{
	if (g_mouse_locked == 0) {
		mouse_reset_ehandler();
		g_mouse_locked = 1;
		if (g_cursor_nvf) {
			mem_free(g_cursor_nvf);
		}
	}
}

void mouse_reset(void)
{
	signed short w, h;
	unsigned char nvf[0x1a];		/* lokales NvfMem */
	register signed short nr;

	nvf_new(nvf);
	if (g_mouse_locked == 0 && (nr = g_cursor_nr_prev) >= 0) {
		g_cursor_nr_prev = g_cursor_nr;
		g_cursor_nr = nr;
		nvf_unpack(nvf, gb_data(g_cursor_bmp), g_cursor_nvf, nr, 0, &w, &h, 0x140);
		g_cursor_w = g_mouse_bg_cw = w;
		g_cursor_h = g_mouse_bg_ch = h;
		g_cursor_hot_x = g_cursor_hot_y = 1;
		g_mouse_moved = 1;
		mouse_update();
	}
	nvf_delete(nvf, 2);
}

void mouse_irq_init(signed short irq, void interrupt (*isr)(void))
{
	signed short l1, l2, l3, l4, l5;

	l1 = 0x0c;
	l3 = irq;
	l4 = FP_OFF((char far *)mouse_show + 0x161);
	l5 = FP_SEG(mouse_show);
	g_mouse_handler_bak = getvect(MOUSE_VEC);
	setvect(MOUSE_VEC, isr);
	mouse_action(&l1, &l2, &l3, &l4, &l5);
	g_mouse_irq_init = 1;
}

void mouse_reset_ehandler(void)
{
	signed short l1, l2, l3, l4, l5;

	setvect(MOUSE_VEC, g_mouse_handler_bak);
	l1 = 0x0c;
	l3 = l4 = l5 = 0;
	mouse_action(&l1, &l2, &l3, &l4, &l5);
	g_mouse_irq_init = 0;
}
