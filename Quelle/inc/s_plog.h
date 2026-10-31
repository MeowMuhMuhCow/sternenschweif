/* s_plog.h - Probenlog (Zusatzfunktion der Bugfix-Fassung, keine Fehlerbehebung; Schalter FIX_PROBENLOG in s_fix.h).
 *
 * Haengt je Probe eine Zahlenzeile an PROBEN.CSV im Spielordner an, aber nur, wenn die Datei schon da ist
 * (anlegen = Log an, loeschen = aus; die Startdatei ..._MIT_PROBENLOG legt sie an). Kein neuer Text im
 * Datensegment: der Dateiname entsteht Zeichen fuer Zeichen, geschrieben werden nur Zahlen und der Heldenname.
 * Zeile: Art;Tag;Monat;Jahr;Minute des Tages;Name;Wert1;...;WertN
 *   Art 1 Eigenschaftsprobe  (Eigenschaft, Wert, Zuschlag, W20, Ergebnis)
 *   Art 3 Dreifachprobe      (3 Eigenschaften, 3 Werte, Zuschlag, 3 W20 (0 = nicht mehr gewuerfelt), Ergebnis)
 *   Art 4 Talent             (Talent, Talentwert bzw. Zielwert, Zuschlag, 1 = Fernkampf, W20, Ergebnis);
 *                            bei 0 folgt die Dreifachprobe in der naechsten Zeile
 *   Art 5 Zauber             (Zauber, Zauberfertigkeit, Zuschlag mit MR); die Dreifachprobe folgt
 *   Art 6 Attacke, 7 Parade  (Wert, W20, Gegnerart (-1 = Held), Richtung 0 Held greift an, 1 Gegner greift Held an,
 *                            2 Gegner gegen Gegner)
 * Klartext daraus machen Probenlog/probenlog_fenster.ps1, probenlog.py und Probenlog_Sternenschweif.html.
 *
 * Die Funktion ist static: jedes Overlay, das sie braucht, bekommt seine eigene Kopie (ein Aufruf in ein anderes
 * Overlay waere ein neuer Overlay-Einsprung, die bleiben unveraendert). */
#if FIX_PROBENLOG
#ifndef S_PLOG_H
#define S_PLOG_H
#include <io.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

static char far *plog_zahl(char far *p, signed short v)
{
	itoa(v, p, 10);
	p += strlen(p);
	*p++ = ';';
	return p;
}

static void probenlog(signed short art, const char far *name, signed short n, const signed short far *w)
{
	char z[160];
	char far *p;
	signed short i;
	signed short fd;

	/* Das Spiel arbeitet im Unterordner DATA (Init::setdir), PROBEN.CSV liegt im Startordner: Pfad aus
	 * g_start_dir ("X:\" + Startpfad) bauen */
	for (i = 0; i < 100 && g_start_dir[i] != '\0'; i++)
		z[i] = g_start_dir[i];
	if (i == 0 || z[i - 1] != '\\')
		z[i++] = '\\';
	z[i++] = 'P'; z[i++] = 'R'; z[i++] = 'O'; z[i++] = 'B'; z[i++] = 'E'; z[i++] = 'N';
	z[i++] = '.'; z[i++] = 'C'; z[i++] = 'S'; z[i++] = 'V'; z[i] = '\0';
	if ((fd = open(z, O_BINARY | O_WRONLY)) == -1)
		return;
	p = plog_zahl(z, art);
	p = plog_zahl(p, (signed char)g_date_day);
	p = plog_zahl(p, (signed char)g_date_month);
	p = plog_zahl(p, (signed char)g_date_year);
	p = plog_zahl(p, (signed short)(g_daytime / TICKS_PER_MINUTE));
	for (i = 0; name != 0 && i < 16 && name[i] != '\0'; i++)
		*p++ = name[i];
	*p++ = ';';
	for (i = 0; i < n; i++)
		p = plog_zahl(p, w[i]);
	p[-1] = '\r';
	*p++ = '\n';
	lseek(fd, 0L, SEEK_END);
	write(fd, z, (unsigned)(p - z));
	close(fd);
}
#endif
#endif
