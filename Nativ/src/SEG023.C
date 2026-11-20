/* SEG023 - Zeitgeber und Maus: BIOS-Zeitgeber-Rueckruf INT 1Ch (timer_isr: Klangdienste, Zufallssaat,
 * Spielzeit, Doppelklick-Frist), AIL-Zeitgeber-Rueckrufe der 3D-Ansicht (view_timer_cb, O010) und des Kampfs
 * (fight_timer_cb, O134; mit Pruefung des Kopierschutzes), Maus-Ereignis-ISR (mouse_isr, O193 haengt sie per
 * INT 33h/0Ch ein) und die ganzzahlige Quadratwurzel long_sqrt (O010).
 * (seg023 in SCHWEIF.EXE, Offsets R23_000F..R23_06AF)
 *
 * BCC -mlarge -3 -Ot -c -Y SEG023.C   (#pragma inline: ueber TASM wie im Original, s. TASM-Kodierung 2B C0 in
 *   long_sqrt; die Reihenfolge der GRP-Fixups der ISR-Prologe gleicht cobjfix an, s. LEITFADEN Welle KS)
 *
 * Funktionen (Offset Name):
 *   000F timer_init       INT 1Ch auf timer_isr umbiegen, timer_restore per atexit (O032 main)
 *   0043 timer_restore    alten INT-1Ch-Vektor zurueck
 *   005A timer_isr        interrupt: Speicherwaechter alle 20 Takte, Zufallssaat, Bildschirmschoner-Zaehler,
 *                         Anzeigedauer der Portraets, Spielzeit (time_tick), Doppelklick-Frist (4 Takte),
 *                         Klangdienste anstossen, alter Vektor
 *   01B0 view_timer_cb    AIL-Rueckruf der 3D-Ansicht (O010: MK_FP(FP_SEG(timer_init), 0x1b0)): Bewegungseingabe,
 *                         Drehung (g_teleport_spin), Texturanimation
 *   0264 fight_timer_cb   AIL-Rueckruf des Kampfs (O134: ... 0x264): alle 250 Takte Pruefung des Kopierschutzes:
 *                         Pruefsumme ueber SEG003 (copyprot_code_begin..copyprot_code_end) gegen g_copyprot_sum,
 *                         markierter Aufruf von copyprot_check in game_idle und von copyprot_stub in savegame_op
 *                         (Muster COPYPROT_MARKER, dann 9A = call far, dann Ziel); Fehlbefund -> g_tamper_count++
 *                         (> 50: LE-Verlust in R10, Kampfbild nach O144). Zaehlt g_anim_ticks.
 *   0437 mouse_isr        interrupt (Maus-Ereignis, Ereignismaske in AX; BE: mouse_isr): Tasten, Doppelklick links,
 *                         Bewegung (Position holen, begrenzen, setzen), Zeiger nachzeichnen
 *   068F long_sqrt        Quadratwurzel (abgerundet) durch Abziehen der ungeraden Zahlen (Inline-asm)
 *
 * Die vier Rueckrufe/ISRs sichern zusaetzlich die 32-Bit-Register (PUSH_E32 direkt nach dem Prolog, POP_E32 am
 * Ende), weil sie mitten in 386-Code (AIL, 3D-Ansicht) zuschlagen koennen.
 * Bei g_dbg_raster_timer / g_dbg_raster_mouse faerben sie Farbe 0 beim Eintritt (gruen/blau/rot) und Austritt
 * (schwarz) - Laufzeitmessung am Bildschirmrand.
 */
#pragma inline
/* Phase B: BCC (C-Modus) legt das BSS in umgekehrter Reihenfolge der ersten Deklaration an - Reihenfolge hier vor schweif.h */
extern signed short g_random_seed2;
extern void interrupt (far *g_old_timer_isr)();
extern signed char g_fight_timer_busy;
extern signed char g_view_timer_busy;
#include <dos.h>
#include <stdlib.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:78d2..78da, dsana) - initialisierte Daten in Adressreihenfolge */
unsigned long g_anim_ticks = 0;
signed short g_fight_timer_ticks = 0;
signed char g_mouse_isr_lock = 0;
/* Phase B: eigenes BSS (ds:cb6e..cb76) in Adressreihenfolge (Reihenfolge festgelegt durch die extern-Zeilen vor den #include) */
signed char g_view_timer_busy;
signed char g_fight_timer_busy;
void interrupt (far *g_old_timer_isr)();
signed short g_random_seed2;

/* 32-Bit-Register sichern bzw. zuruecksetzen (Inline-asm, ueber TASM: #pragma inline) */
#define PUSH_E32() ((void)0)
#define POP_E32() ((void)0)

/* R23_000F */
void timer_init(void)
{
	g_old_timer_isr = getvect(TIMER_INT);
	setvect(TIMER_INT, timer_isr);
	atexit(timer_restore);
}

/* R23_0043 */
void timer_restore(void)
{
	setvect(TIMER_INT, g_old_timer_isr);
}

/* R23_005A: BIOS-Zeitgeber (18,2 Hz) */
void interrupt timer_isr(void)
{
	struct rgb col_in;
	struct rgb col_out;
	register signed short i;

	PUSH_E32();

	if (g_dbg_raster_timer) {
		col_in.r = 0;
		col_in.g = 0x3f;
		col_in.b = 0;
		set_color(&col_in, 0);
	}
	if (g_timer_isr_busy == 0) {
		if (!*(signed char far *)g_indos_flag) {
			signed short loctype;

			g_timer_isr_busy = 1;
			if (--g_mem_check_countdown <= 0) {
				g_mem_check_countdown = 20;
				if (g_func_trace_on == 0)
					mem_check();
			}
			g_random_seed2++;
			if (g_random_seed2 < 0)
				g_random_seed2 = 0;
			g_idle_ticks++;
			for (i = 0; i <= 6; i++) {
				if (g_menu_tail[0x4e + i] > 0) {
					if (--g_menu_tail[0x4e + i] == 0)
						g_refresh_portraits = 1;
				}
			}
			if (g_game_state != 0 && (loctype = g_group->loctype) != 0 && loctype != 0)
				g_leave_location = 1;
			time_tick(g_date);
			if (g_mouse_lpending != 0 && g_random_seed2 - g_mouse_lclick_time > 4) {
				/* Doppelklick-Frist abgelaufen: einfacher Klick */
				g_mouse_ldclick = 0;
				g_mouse_lpending = 0;
				g_mouse_lclick = 1;
				g_mouse_event = 1;
			}
			g_timer_isr_busy = 0;
		}
	}
	g_snd_service_due = 1;
	g_anim_timer++;
	g_old_timer_isr();
	if (g_dbg_raster_timer) {
		col_out.r = 0;
		col_out.g = 0;
		col_out.b = 0;
		set_color(&col_out, 0);
	}

	POP_E32();
}

/* R23_01B0: AIL-Zeitgeber der 3D-Ansicht (O010 view_timer_on) */
void view_timer_cb(void)
{
	struct rgb col_in;
	struct rgb col_out;

	PUSH_E32();

	if (g_view_timer_busy == 0) {
		g_view_timer_busy = 1;
		if (g_dbg_raster_timer) {
			col_in.r = 0;
			col_in.g = 0;
			col_in.b = 0x3f;
			set_color(&col_in, 0);
		}
		if (g_view_input_on)
			move_input();
		if (g_teleport_spin != -1) {
			g_teleport_spin++;
			g_cam_angle += 8;
		}
		if (g_tex_anim_rate) {
			if (++g_tex_anim_tick >= g_tex_anim_rate) {
				g_tex_anim_tick = 0;
				g_tex_anim_due = 1;
			}
		}
		if (g_dbg_raster_timer) {
			col_out.r = 0;
			col_out.g = 0;
			col_out.b = 0;
			set_color(&col_out, 0);
		}
		g_view_timer_busy = 0;
	}

	POP_E32();
}

/* R23_0264: AIL-Zeitgeber des Kampfs (O134 fight_timer_start); alle 250 Takte Pruefung des Kopierschutzes */
void fight_timer_cb(void)
{
	unsigned char far *q;		/* Suchzeiger im Code von SEG001 */
	unsigned char far *q_end;
	unsigned char far *p;		/* Pruefsummenzeiger im Code von SEG003 */
	unsigned char far *p_end;
	signed char done;
	register signed short found_check;
	register signed short found_stub;
	void far *target;		/* erwartetes Ziel des markierten call far */
	unsigned long w;
	signed long sum;
	struct rgb col_in;
	struct rgb col_out;

	PUSH_E32();

	done = 0;
	found_check = 0;
	found_stub = 0;
	sum = 0;
	if (g_fight_timer_busy == 0) {
		g_fight_timer_busy = 1;
		g_fight_timer_ticks++;
		if (g_fight_timer_ticks >= 250) {
			g_fight_timer_ticks = 0;
			if (g_dbg_raster_timer) {
				col_in.r = 0x3f;
				col_in.g = 0;
				col_in.b = 0;
				set_color(&col_in, 0);
			}
			/* 1. Pruefsumme ueber den Code der Handbuchabfrage (SEG003 0x0006..0x0277) */
			for (p = (unsigned char far *)copyprot_code_begin, p_end = (unsigned char far *)copyprot_code_end;
				p < p_end; p++)
				sum += *p;

			/* 2. markierter Aufruf copyprot_check() in game_idle */
			target = (void far *)copyprot_check;
			q = (unsigned char far *)copyprot_idle_begin;
			q_end = (unsigned char far *)copyprot_idle_end;
			while (!done) {
				w = *(unsigned long far *)q;
				if (w == COPYPROT_MARKER) {
					q += 4;
					if (*q == 0x9a) {		/* call far ptr */
						q++;
						if (*(void far * far *)q == target) {
							found_check = 1;
							break;
						}
					}
				} else
					q++;
				if (q > q_end)
					done = 1;
			}

			/* 3. markierter Aufruf copyprot_stub() in savegame_op */
			target = (void far *)copyprot_stub;
			q = (unsigned char far *)copyprot_save_begin;
			q_end = (unsigned char far *)copyprot_save_end;
			done = 0;
			while (!done) {
				w = *(unsigned long far *)q;
				if (w == COPYPROT_MARKER) {
					q += 4;
					if (*q == 0x9a) {
						q++;
						if (*(void far * far *)q == target) {
							found_stub = 1;
							break;
						}
					}
				} else
					q++;
				if (q > q_end)
					done = 1;
			}

			if (sum != g_copyprot_sum || !found_check || !found_stub)
				g_tamper_count++;
			if (g_dbg_raster_timer) {
				col_out.r = 0;
				col_out.g = 0;
				col_out.b = 0;
				set_color(&col_out, 0);
			}
		}
		g_anim_ticks++;
		g_fight_timer_busy = 0;
	}

	POP_E32();
}

/* R23_0437: Maus-Ereignis-ISR (BE: mouse_isr); Ereignismaske in AX: 1 bewegt, 2/4 links gedrueckt/losgelassen,
 * 8/0x10 rechts gedrueckt/losgelassen */
void interrupt mouse_isr(void)
{
    /* Im SDL2-Port ueber dsa_platform_poll_events behandelt */
}

/* R23_068F: ganzzahlige Quadratwurzel (abgerundet): zaehlt, wie viele ungerade Zahlen 1, 3, 5, ... sich von n
 * abziehen lassen (Summe der ersten k ungeraden Zahlen = k*k). Ergebnis in DX:AX (DX = 0). */
#pragma warn -rvl
signed long long_sqrt(signed long n)
{
    unsigned long res = 0;
    unsigned long odd = 1;
    long rem = n;
    while ((rem -= (long)odd) >= 0) {
        res++;
        odd += 2;
    }
    return (signed long)res;
}
#pragma warn .rvl
