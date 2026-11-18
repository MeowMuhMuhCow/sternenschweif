/* OVL250 - Overlay O250 (Segment 0x2475 in SCHWEIF.EXE): Spielzeit/Kalender-Objekt Date (0x18 Bytes, global g_date
 * ds:b856; class Date : struct game_date in s_class.h): Ticks seit Mitternacht ticks (TICKS_PER_DAY = 129600 je Tag,
 * 5400 je Stunde), Wochentag weekday, Woche week, Tag day (negativ = Namenlose Tage), Monat month, Jahr year,
 * besonderer Tag special und drei Grafikpuffer sky_bg/sky_marks/sky_fg (Himmelsanzeige mit Sonne bzw. Mond).
 * Konstruktor (15.12.17 Hal, 8 Uhr, g_clock_update = 1), next_day (Folgetag, -5 nach dem 30.12.), print (Datum/Uhrzeit
 * nach g_text_output_buf aus GLOBLTXT.LTX, besonderer Tag per sprintf nach g_text_buf2 + strcat), season (Monat in den
 * Listen g_months_winter/_summer/_spring per is_in_byte_array -> SEASON_*), draw(force) (Himmelsbild je 853 Ticks:
 * Tag/Nacht-Wechsel laedt die Bilder per nvfile_load aus TIMEDISP.NVF, Sonne/Mond mit Bmp-Objekten in einen
 * 0x1b80-Puffer, Maus kurz aus, Ausgabe bei (0xf4, 0x1e) - nur bei Vollbild 4 bzw. Reisekarte), alloc/free der
 * Puffer, save/load (Kennung "TIME", Version 11, Datei g_save_fd per _write/_read). Freie Funktionen fuer
 * Zeitpunkte (struct timestamp, 7 Bytes): date_now(t) = jetzt, date_set(t, rel, ticks, d, m, y) = jetzt + Abstand
 * (rel == 1) bzw. absolut, date_since(t) = Abstand jetzt - t in Ticks (long).
 * Texte (GLOBLTXT.LTX = g_ttx): [265..271] Wochentage "Rohalstag" .. "Praiostag", [5..16] Monate (= Goetter "Praios"
 *   .. "Rahja"), [322..325] Jahreszeiten "Winter", "Fruehling", "Sommer", "Herbst", [274..306] besondere Tage;
 *   GLOBLTXT.LTX[272]: "Es ist %s der %d.^%s (%s) im Jahre %d^Hal, %d^Uhr."
 *   GLOBLTXT.LTX[307]: "Es ist %s,@einer der namenlosen Tage@Im Jahre %d^Hal@@%d^Uhr"
 *   GLOBLTXT.LTX[273]: "@@Heute ist <F3>%s<F0>"
 *
 * BCC -mlarge -3 -Ot -Oi -c -Y OVL250.CPP   -> 3094/3094 Bytes GLEICH (18.10.2026), cobjfix laeuft (mit -Og ebenso)
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - C++: Konstruktor mit automatischem "if (this == 0) this = new(0x18)" + totem Epilog; "new Bmp(w, h, p)" ->
 *   gb_new(0, w, h, p), "delete b" -> gb_delete(b, 3). -Oi noetig fuer strcat(g_text_output_buf, g_text_buf2).
 * - Epilogkopien bei return (season, save, load, date_set "return;" im then-Zweig) kommen hier auch OHNE -Og.
 * - date_set: char-Parameter day/month als register -> DL/CL (Blattfunktion); "t->day = t->day - 5" ->
 *   mov al / add al,0fbh / mov (kein add mem).
 * - date_since: register signed char d1 -> CL (keine Aufrufe), Tausch ueber tmp [bp-6]; "(signed long)(a - b) *
 *   0x1fa40" -> cbw/push/cbw/pop dx/sub/movsx eax,dx/imul eax,eax,imm.
 * - mem_alloc mit unsigned long: 0x40 -> "66 6a 40" (push dword byte), 0x630 -> "66 68 ..".
 * - is_in_byte_array bekommt den Monat als signed char -> mov al / push ax (ohne cbw).
 * - nvfile_load: zwei Nullworte als (void far *)0.
 * Originalfehler: date_set (rel == 1) - Tagesueberlauf durch d setzt "t->day = 30 - t->day" (statt t->day - 30,
 *   ergibt 0 oder negativ); beim Tick-Ueberlauf (> 0x1fa40, nicht >=) wird t->ticks auf 0 gesetzt statt um
 *   0x1fa40 vermindert (Rest geht verloren); nach Monat 12 wird beim Ueberlauf durch die Ticks nur day = -5
 *   gesetzt, der Monat bleibt 13.
 */
#include <stdio.h>
#include <string.h>
#include <io.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:76ba..7718, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
signed char g_months_spring[4] = { MONTH_PERAINE, MONTH_INGERIMM, MONTH_RAHJA, -1 };
signed char g_months_summer[4] = { MONTH_PRAIOS, MONTH_RONDRA, MONTH_EFFERD, -1 };
signed char g_months_winter[4] = { MONTH_FIRUN, MONTH_TSA, MONTH_PHEX, -1 };
signed char g_sky_arc_y[74] = {
	18, 17, 16, 15, 14, 13, 12, 11, 11, 10, 9, 9, 8, 8, 7, 7,
	6, 6, 5, 5, 4, 4, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2,
	2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
	3, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9,
	9, 10, 11, 11, 12, 13, 14, 15, 16, 17
};
signed short g_unused_7710 = 4626;	/* ds:7710 (0x1212), unbenutzt; dieselbe Adresse ist die gefaltete Basis g_conv_topic_tab - 7 (O254/O257) */
signed char g_sky_pos = 0;

#define DATE		((Date far *)g_date)
#define FILE_TIMEDISP	0x96		/* TIMEDISP.NVF: Himmel Tag/Nacht (0/1 bzw. 2/3), Sonne/Mond (4..7) */
#define SKY_DAY_START	0x93a8		/* 7 Uhr */
#define SKY_DAY_END	0x190c8		/* 19 Uhr */
#define SKY_STEP	853		/* Ticks je Position der Sonne bzw. des Mondes */
#define PP20_MAIN	4		/* Vollbild 4 (Spielbildschirm mit Uhr) */
#define DATE_SAVE_VERSION	11

Date::Date()
{
	g_clock_update = 1;
	g_date_flag = 0;
	special = 0;
	ticks = 0xa8c0;			/* 8 Uhr */
	weekday = 0;
	day = 15;
	week = 3;
	month = 12;
	year = 17;
	week_reset = 0;
}

signed char Date::next_day()
{
	register signed char d;

	d = day + 1;
	if (d > 30) {
		if (month > 12)
			d = -5;
		else
			d = 1;
	}
	return d;
}

void Date::print()
{
	if (day < 0)
		sprintf(g_text_output_buf, g_ttx[307], g_ttx[weekday + 265], (signed short)year, ticks / 5400);
	else
		sprintf(g_text_output_buf, g_ttx[272], g_ttx[weekday + 265], (signed short)day, g_ttx[month + 4],
			g_ttx[season() + 322], (signed short)year, ticks / 5400);
	if (special != 0) {
		sprintf(g_text_buf2, g_ttx[273], g_ttx[special + 273]);
		strcat(g_text_output_buf, g_text_buf2);
	}
}

signed short Date::season()
{
	register signed short r;

	if (is_in_byte_array(month, g_months_winter) != -1)
		r = SEASON_WINTER;
	else if (is_in_byte_array(month, g_months_summer) != -1)
		r = SEASON_SUMMER;
	else if (is_in_byte_array(month, g_months_spring) != -1)
		r = SEASON_SPRING;
	else
		r = SEASON_AUTUMN;
	return r;
}

void Date::draw(signed short force)
{
	signed short pos;
	signed long t;
	signed short was_day;
	signed short hidden;
	signed short dummy;
	Bmp far *sky;
	Bmp far *b;
	void far *buf;
	register signed short mark;

	if ((g_pp20_index == PP20_MAIN || g_pp20_index == PP20_TRAVELMAP) && g_silent_mode == 0 && g_clock_update != 0) {
		t = ticks;
		was_day = g_is_day;
		hidden = 0;
		if (t >= SKY_DAY_START && t < SKY_DAY_END) {
			g_is_day = 1;
			t -= SKY_DAY_START;
			pos = t / SKY_STEP;
		} else {
			g_is_day = 0;
			t -= SKY_DAY_END;
			if (t < 0)
				t += 0x1fa40;
			pos = t / SKY_STEP;
		}
		if (force || g_sky_pos != pos) {
			buf = mem_alloc(0x1b80);
			g_sky_pos = pos;
			mark = 0x30;		/* Mond */
			if (g_is_day != 0) {
				mark = 0;	/* Sonne: Morgen, Mittag, Abend */
				if (pos > 0x44)
					mark = 0x20;
				else if (pos > 5)
					mark = 0x10;
			}
			if (force || g_is_day != was_day) {
				if (g_is_day != 0) {
					nvfile_load(g_nvf_heads, FILE_TIMEDISP, sky_bg, (void far *)0, 0, 3, &dummy, &dummy, 0x140);
					nvfile_load(g_nvf_heads, FILE_TIMEDISP, sky_fg, (void far *)0, 1, 3, &dummy, &dummy, 0x140);
				} else {
					nvfile_load(g_nvf_heads, FILE_TIMEDISP, sky_bg, (void far *)0, 2, 3, &dummy, &dummy, 0x140);
					nvfile_load(g_nvf_heads, FILE_TIMEDISP, sky_fg, (void far *)0, 3, 3, &dummy, &dummy, 0x140);
				}
			}
			sky = new Bmp(0x48, 0x16, sky_bg);
			sky->put(buf, 0, 0, BLIT_COPY);
			b = new Bmp(4, 4, sky_marks + mark);
			b->put(buf, pos, g_sky_arc_y[pos], BLIT_MASKED);
			delete b;
			sky->set_base(sky_fg);
			sky->put(buf, 0, 0, BLIT_MASKED);
			delete sky;
			b = new Bmp(0x48, 0x16);
			b->get(buf, 0, 0);
			mem_free(buf);
			if (mouse_overlaps_rect(0xf0, 0x1e, 0x13f, 0x37)) {
				hidden = 1;
				mouse_hide(0);
			}
			b->put(g_vga_buf, 0xf4, 0x1e, BLIT_COPY);
			delete b;
			if (hidden)
				mouse_show(0);
		}
	}
}

void Date::alloc()
{
	signed short dummy;
	register signed short i;

	sky_bg = (char far *)mem_alloc(0x630);
	sky_marks = (char far *)mem_alloc(0x40);
	sky_fg = (char far *)mem_alloc(0x630);
	nvfile_load(g_nvf_heads, FILE_TIMEDISP, sky_bg, (void far *)0, 0, 3, &dummy, &dummy, 0x140);
	nvfile_load(g_nvf_heads, FILE_TIMEDISP, sky_fg, (void far *)0, 1, 3, &dummy, &dummy, 0x140);
	for (i = 0; i < 4; i++)
		nvfile_load(g_nvf_heads, FILE_TIMEDISP, sky_marks + (i << 4), (void far *)0, i + 4, 3, &dummy, &dummy, 0x140);
}

void Date::release()
{
	sky_bg = (char far *)mem_free(sky_bg);
	sky_marks = (char far *)mem_free(sky_marks);
	sky_fg = (char far *)mem_free(sky_fg);
}

signed short Date::save()
{
	signed short ver = DATE_SAVE_VERSION;
	signed long t;
	signed char c;

	_write(g_save_fd, "TIME", 4);
	_write(g_save_fd, &ver, 2);
	t = ticks;
	_write(g_save_fd, &t, 4);
	c = special;
	_write(g_save_fd, &c, 1);
	c = weekday;
	_write(g_save_fd, &c, 1);
	c = day;
	_write(g_save_fd, &c, 1);
	c = month;
	_write(g_save_fd, &c, 1);
	c = year;
	_write(g_save_fd, &c, 1);
	c = week;
	_write(g_save_fd, &c, 1);
	c = week_reset;
	_write(g_save_fd, &c, 1);
	return 1;
}

signed short Date::load()
{
	signed long magic;
	signed short ver;
	signed long t;
	signed char c;

	_read(g_save_fd, &magic, 4);
	if (magic == 0x454d4954L) {	/* "TIME" */
		_read(g_save_fd, &ver, 2);
		_read(g_save_fd, &t, 4);
		ticks = t;
		_read(g_save_fd, &c, 1);
		special = c;
		_read(g_save_fd, &c, 1);
		weekday = c;
		_read(g_save_fd, &c, 1);
		day = c;
		_read(g_save_fd, &c, 1);
		month = c;
		_read(g_save_fd, &c, 1);
		year = c;
		_read(g_save_fd, &c, 1);
		week = c;
		_read(g_save_fd, &c, 1);
		week_reset = c;
		return 1;
	}
	return 0;
}

void date_now(struct timestamp far *t)
{
	t->ticks = DATE->ticks;
	t->day = DATE->day;
	t->month = DATE->month;
	t->year = DATE->year;
}

void date_set(struct timestamp far *t, signed short rel, signed long ticks, register signed char day,
	register signed char month, signed char year)
{
	if (rel == 1) {
		t->ticks = DATE->ticks;
		t->day = DATE->day;
		t->month = DATE->month;
		t->year = DATE->year;
		t->ticks += ticks;
#if FIX_0034
		/* Fix #34: Tagesueberlauf ab einem vollen Tag, Rest behalten */
		if (t->ticks >= 0x1fa40) {
			t->ticks -= 0x1fa40;
#else
		if (t->ticks > 0x1fa40) {
			t->ticks = 0;
#endif
			t->day++;
			if (t->day > 30) {
				t->day = 1;
				t->month++;
				if (t->month > 12)
					t->day = -5;
			} else if (t->day == 0) {
				t->day = 1;
				t->month = 1;
				t->year++;
			}
		}
		t->day += day;
		if (t->day > 30) {
#if FIX_0033
			/* Fix #33: Tagesueberlauf: Rest in den naechsten Monat */
			t->day -= 30;
#else
			t->day = 30 - t->day;
#endif
			t->month++;
			if (t->month > 12) {
				t->day = t->day - 5;
				t->month = 1;
				t->year++;
			}
		}
		t->month += month;
		if (t->month > 12) {
			t->day = t->day - 5;
			t->month = t->month - 12;
			t->year++;
			if (t->day <= 0 && t->month != 1) {
				t->day = t->day + 30;
				t->month--;
			}
		}
		t->year += year;
		return;
	}
	t->ticks = ticks;
	t->day = day;
	t->month = month;
	t->year = year;
}

signed long date_since(struct timestamp far *t)
{
	signed long r;
	signed char d2;
	signed char tmp;
	register signed char d1;

	d1 = DATE->day;
	d2 = t->day;
	if (d1 < 0 && d2 < 0) {
		tmp = -d1;
		d1 = -d2;
		d2 = tmp;
	} else if (d1 > 0 && d2 < 0) {
		d2 = -d2;
		d1 = d1 - 4;
	} else if (d1 < 0 && d2 > 0) {
		d1 = 1 - d1 - 5;
	}
	r = DATE->ticks - t->ticks;
	r += (signed long)(d1 - d2) * 0x1fa40;
	r += (signed long)(DATE->month - t->month) * 0x3b5380;	/* 30 Tage */
	r += (signed long)(DATE->year - t->year) * 0x2d1cd40;	/* 365 Tage */
	return r;
}
