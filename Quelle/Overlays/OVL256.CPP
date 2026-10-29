/* OVL256 - Overlay O256 (Segment 0x248c in SCHWEIF.EXE): Wetterobjekt Wx (4 Bytes, global g_weather ds:be32; class Wx in
 * s_class.h). Jahreszeit season aus date_season(g_date), zyklischer Zustand cycle 0..9 (Zufallsschritt -2..+2 per
 * random_schweif(5), im Sommer -1, im Winter +1) und daraus das Wetter level WEATHER_HOT..WEATHER_STORM (Tabelle per
 * switch; Anzeigetexte MMASK.LTX[15..29], g_ttxt2[level * 3 + 0..2 + 15] in O216). Konstruktor (Startzustand
 * zufaellig 1..10), leerer Destruktor, set/get des Wetters, next() einmal je Tag (O005 timers_daily).
 * Abfrage Wx_get(g_weather): O038 (Travel.weather), O071 (Nahrungssuche), O213 (Wetterfolgen), O215/O216 (Reise).
 *
 * BCC -mlarge -3 -Ot -c -Y OVL256.CPP   -> 462/462 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * Schluessel: cycle -= 1 / cycle += 1 (load, dec/inc al, store) - "cycle--" ergibt dec byte ptr und passt nicht.
 *   switch-Sprungtabelle: Faelle in Quellreihenfolge 6,8 / 7 / 0,4,5,9 / 1,3 / 2.
 *   next() steht hinter dem Konstruktor -> far call (TLINK macht nop/push cs/call near daraus).
 */
#include "schweif.h"

Wx::Wx()
{
	season = date_season(g_date);
	cycle = random_schweif(10);
	level = next();
}

Wx::~Wx()
{
}

void Wx::set(signed char v)
{
	level = v;
}

signed char Wx::get()
{
	return level;
}

signed char Wx::next()
{
	signed char old;

	old = cycle;
	season = date_season(g_date);
	cycle = old + random_schweif(5) - 2;
	if (season == SEASON_SUMMER)
		cycle -= 1;
	if (season == SEASON_WINTER)
		cycle += 1;
	if (cycle > 9)
		cycle -= 10;
	if (cycle < 0)
		cycle += 10;
	switch (cycle) {
	case 6:
	case 8:
		level = WEATHER_HOT;
		break;
	case 7:
		level = WEATHER_FINE;
		break;
	case 0:
	case 4:
	case 5:
	case 9:
		level = WEATHER_CLOUDY;
		break;
	case 1:
	case 3:
		level = WEATHER_RAIN;
		break;
	case 2:
		level = WEATHER_STORM;
		break;
	}
	if (level < WEATHER_HOT)
		level = WEATHER_HOT;
	if (level > WEATHER_STORM)
		level = WEATHER_STORM;
	return level;
}
