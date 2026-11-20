/* SEG021 - Spielzeit: Zeitsprung, Tick, Kalender, besondere Tage (seg021 in SCHWEIF.EXE)
 * 1 Tick = 1/1,5 s Spielzeit: 90 Ticks = 1 Minute, 5400 = 1 Stunde, 129600 = 1 Tag (TICKS_PER_*).
 * Datumsobjekt struct game_date (g_date): Tageszeit, Wochentag, Woche, Tag (1..30, Namenlose Tage -5..-1),
 * Monat, Jahr. time_tick zaehlt die Zeitzaehler g_timer_* hoch (R10 do_timers wertet sie aus und setzt sie
 * zurueck) und setzt die Tageszeit-Merker g_at_*.
 * BCC -mlarge -3 -Ot -c -Y SEG021.C   -> 1031/1031 Bytes GLEICH
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * - Objektparameter wie in s_proto.h als void far *, Zugriff ueber den Cast-Makro DATE (gleicher Code).
 * - timewarp: g_clock_update (Zeitanzeige nachfuehren) wird gesetzt, solange je Tick eine weitere
 *   Viertelstunde vergeht (schneller Zeitsprung); danach der alte Wert.
 */
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:7638..76ba, dsana) - initialisierte Daten in Adressreihenfolge */
struct special_day g_special_days[43] = { 0 };

#define DATE	((struct game_date far *)date)

void timewarp_until(void far *date, signed long target)
{
	signed long diff;
	signed long now;

	if ((now = DATE->ticks) > target)
		diff = TICKS_PER_DAY - (now - target);
	else
		diff = target - DATE->ticks;
	timewarp(date, diff);
}

void timewarp(void far *date, signed long ticks)
{
	signed long i;
	signed short bak;
	signed char quarters;

	bak = g_clock_update;
	quarters = 0;
	g_clock_update = 0;
	for (i = 0; i < ticks; i++) {
		time_tick(date);
		if (g_timer_15min != 0)
			quarters++;
		if (quarters >= 2)
			g_clock_update = 1;
		else
			g_clock_update = 0;
		game_idle();
		if (g_timer_15min == 0)
			quarters = 0;
	}
	do_timers(&g_timers_lock);
	g_clock_update = bak;
}

void timewarp_minutes(void far *date, signed long n) { timewarp(date, n * TICKS_PER_MINUTE); }
void timewarp_hours(void far *date, signed long n) { timewarp(date, n * TICKS_PER_HOUR); }
void timewarp_days(void far *date, signed long n) { timewarp(date, n * TICKS_PER_DAY); }
void timewarp_months(void far *date, signed long n) { timewarp(date, n * (DAYS_PER_MONTH * TICKS_PER_DAY)); }
void timewarp_years(void far *date, signed long n) { timewarp(date, n * (365 * TICKS_PER_DAY)); }

void time_tick(void far *date)
{
	signed short pm;
	signed short i;

	pm = 0;
	if (g_dialog_depth == 0) {
		DATE->ticks++;
		g_timer_ticks++;

		/* Teiler je Halbtag (nachmittags vorher 12 Stunden abziehen) */
		if (DATE->ticks >= TICKS_PER_HALF_DAY) {
			DATE->ticks -= TICKS_PER_HALF_DAY;
			pm = 1;
		}
		if (DATE->ticks % TICKS_PER_MINUTE == 0) g_timer_minutes++;
		if (DATE->ticks % (5 * TICKS_PER_MINUTE) == 0) g_timer_5min++;
		if (DATE->ticks % (15 * TICKS_PER_MINUTE) == 0) g_timer_15min++;
		if (DATE->ticks % TICKS_PER_HOUR == 0) g_timer_hours++;
		if (DATE->ticks % (4 * TICKS_PER_HOUR) == 0) g_timer_4hours++;
		if (pm == 1)
			DATE->ticks += TICKS_PER_HALF_DAY;

		if (DATE->ticks == 8 * TICKS_PER_HOUR) g_at_0800 = 1;
		if (DATE->ticks == 10 * TICKS_PER_HOUR) g_at_1000 = 1;
		if (DATE->ticks == 14 * TICKS_PER_HOUR) g_at_1400 = 1;
		if (DATE->ticks == 20 * TICKS_PER_HOUR) g_at_2000_2200 = 1;
		if (DATE->ticks == 22 * TICKS_PER_HOUR) g_at_2000_2200 = 0;
		if (DATE->ticks == 6 * TICKS_PER_HOUR) g_at_0600_0900 = 1;
		if (DATE->ticks == 9 * TICKS_PER_HOUR) g_at_0600_0900 = 0;

		if (DATE->ticks >= TICKS_PER_DAY) {
			/* neuer Tag */
			DATE->ticks = 0;
			g_timer_days++;

			DATE->weekday++;
			if (DATE->weekday > DAYS_PER_WEEK - 1) {
				DATE->weekday = 0;
				if (DATE->week_reset != 0) {
					DATE->week_reset = 0;
					DATE->week = 1;
				} else {
					DATE->week++;
				}
			}

			DATE->day++;
			if (DATE->day > DAYS_PER_MONTH) {
				DATE->day = 1;
				DATE->month++;
				if (DATE->weekday == 0)
					DATE->week++;
				else
					DATE->week_reset = 1;
				g_timer_months++;
				if (DATE->month > MONTHS_PER_YEAR)
					DATE->day = -NAMELESS_DAYS;
			} else if (DATE->day == 0) {
				/* nach den Namenlosen Tagen: neues Jahr */
				DATE->month = 1;
				DATE->year++;
				DATE->day = 1;
				g_timer_years++;
			}

			for (i = 0, DATE->special = 0; g_special_days[i].month != -1; i++) {
				if (g_special_days[i].month == DATE->month && g_special_days[i].day == DATE->day) {
					DATE->special = g_special_days[i].id;
					return;
				}
			}
		}
	}
}
