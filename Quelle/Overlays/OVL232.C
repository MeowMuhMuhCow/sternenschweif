/* OVL232 - Overlay O232 (Segment 0x2408 in SCHWEIF.EXE): Klangkonfiguration zu SndSetup (O043, C-Sicht struct
 * snd_setup): Ports/IRQs fuer Musik und Digital (music_port/music_irq/digi_port/digi_irq) aus der offenen
 * g_file_main lesen (snd_cfg_read, 20 Bytes struct snd_cfg) bzw. in die Datei 0x13 schreiben (snd_cfg_write),
 * die beiden Zieldateien der Treiber txt[6]/txt[7] loeschen (snd_cfg_unlink_drivers), BLASTER-Umgebungsvariable
 * zerlegen (blaster_parse: Kennbuchstaben g_blaster_keys "AIDT", Zahlenbasis g_blaster_bases, Ziele g_blaster_dest
 * in g_blaster_desc bzw. g_blaster_type; je nach Kartentyp 1..4 Treiber Musik/Digital und Kartenname kopieren);
 * blaster_detect liefert den Treibernamen (Musik bei kind 3, sonst Digital) und Port/IRQ aus g_blaster_result.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL232.C   -> 1040/1040 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h, Welle N20: 19.10.2026 GLEICH, LINKGLEICH)
 * - -Oi: strncpy/strlen/strcpy/memset inline (rep movsw/stosw), Strukturkopie aus far-Zeiger nach DS-Variable
 *   ueber SCOPY@ (g_blaster_desc = *desc), zurueck inline (rep movsw).
 * - -Og noetig fuer die Epilogkopien bei return 0 und den doppelten Epilog nach if/else-return in blaster_detect.
 * - Locals in Deklarationsreihenfolge k, m, c, i, d, val, j, env, buf[0x80] ([bp-2]..[bp-92h]); keine
 *   Registervariablen (SI/DI gehen an die String-Intrinsics).
 * - isalnum(d) aus ctype.h (_ctype+1, Maske 0x0e); i += (buf[i] == ' ') materialisiert 1/0.
 * - blaster_detect: Puffer music[16], digi[16], card[64] in dieser Deklarationsreihenfolge.
 * - Phase B (19.10.2026): eigene Daten im Quelltext. g_blaster_dest hat vier Ziele (A/I/D/T, das vierte ist
 *   &g_blaster_type); die Tabellen je Kartentyp heissen jetzt g_blaster_music_drivers/_digi_drivers/_card_names[4]
 *   und werden mit type - 1 indiziert (die MAP-Namen g_blaster_music_drv usw. liegen je 4 Bytes davor). Die
 *   Treibernamen sind Literale der Tabelleninitialisierer (Literalpool in Auftretensreihenfolge).
 */
/* Phase B: BCC (C-Modus) legt das BSS in umgekehrter Reihenfolge der ersten Deklaration an - Reihenfolge hier vor schweif.h */
extern signed short g_blaster_type;
extern struct ail_drv_desc g_blaster_desc;
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dos.h>
#include "schweif.h"

#define SND_CFG_FILE	0x13		/* Konfigurationsdatei der Klangeinstellungen */
#define NR_BLASTER_KEYS	4
#define NR_BLASTER_TYPES 4

/* Phase B: eigene Daten des Moduls (ds:731e..747a, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
char g_str_empty_731e[2] = "";
char g_blaster_keys[5] = "AIDT";
signed short g_blaster_bases[4] = { 16, 10, 10, 10 };
signed short far *g_blaster_dest[NR_BLASTER_KEYS] = {	/* Ziel je Kennbuchstabe A, I, D, T */
	&g_blaster_desc.io, &g_blaster_desc.irq, &g_blaster_desc.dma, &g_blaster_type
};
char far *g_blaster_music_drivers[NR_BLASTER_TYPES] = {	/* Musiktreiber je Kartentyp 1..4 (Index type - 1) */
	"SBFM.ADV", "SBP1FM.ADX", "SBFM.ADV", "SBP2FM.ADX"
};
char far *g_blaster_digi_drivers[NR_BLASTER_TYPES] = {	/* Digitaltreiber je Kartentyp 1..4 */
	"SBDIG.ADV", "SBPDIG.ADV", "SBDIG.ADV", "SBPDIG.ADV"
};
char far *g_blaster_card_names[NR_BLASTER_TYPES] = {	/* Kartenname je Kartentyp 1..4 */
	"Sound Blaster V1.5 or earlier",
	"Sound Blaster Pro (Yamaha YM3812 version)",
	"Sound Blaster V2.0",
	"Sound Blaster Pro (Yamaha YMF262/OPL3 version)"
};
struct ail_drv_desc g_blaster_result = { 0, 0, "", 0, 0, 0, 0, 0, 0, 0x0000 };
/* Phase B: eigenes BSS (ds:cb34..cb4e) in Adressreihenfolge (Reihenfolge festgelegt durch die extern-Zeilen vor den #include) */
struct ail_drv_desc g_blaster_desc;
signed short g_blaster_type;

void snd_cfg_read(SNDSETUP_PTR s)
{
	struct snd_cfg cfg;

	file_read(g_file_main, &cfg, 0x14L);
	file_close(g_file_main);
	s->music_port = cfg.music_port;
	s->music_irq = cfg.music_irq;
	s->digi_port = cfg.digi_port;
	s->digi_irq = cfg.digi_irq;
}

void snd_cfg_unlink_drivers(SNDSETUP_PTR s)
{
	unlink(s->txt[6]);
	unlink(s->txt[7]);
}

void snd_cfg_write(SNDSETUP_PTR s)
{
	struct snd_cfg cfg;

	memset(&cfg, 0, sizeof(cfg));
	cfg.music_port = s->music_port;
	cfg.music_irq = s->music_irq;
	cfg.digi_port = s->digi_port;
	cfg.digi_irq = s->digi_irq;
	file_open_mode(g_file_main, SND_CFG_FILE, 0x8300);
	File_write(g_file_main, &cfg, 0x14L);
	file_close(g_file_main);
}

signed short blaster_parse(SNDSETUP_PTR s, struct ail_drv_desc far *desc, char far *music, char far *digi,
	char far *card)
{
	signed short k;
	signed short m;
	signed short c;
	signed short i;
	signed short d;
	signed short val;
	signed short j;
	char far *env;
	char buf[0x80];

	env = getenv("BLASTER");
	if (env == 0)
		return 0;
	strncpy(buf, env, 0x7f);
	if (strlen(buf) == 0)
		return 0;
	strupr(buf);
	g_blaster_desc = *desc;
	for (i = 0; i < strlen(buf); i++) {
		if (i != 0 && buf[i] != ' ')
			continue;
		i += (buf[i] == ' ');
		c = buf[i];
		for (k = 0; k < NR_BLASTER_KEYS; k++) {
			if (g_blaster_keys[k] == c) {
				j = i + 1;
				val = 0;
				do {
					d = buf[j];
					j++;
					for (m = 0; m < g_blaster_bases[k]; m++)
						if (toupper(d) == "0123456789ABCDEF"[m])
							val = val * g_blaster_bases[k] + m;
				} while (isalnum(d));
				*g_blaster_dest[k] = val;
				break;
			}
		}
	}
	*desc = g_blaster_desc;
	if (g_blaster_type == 0)
		return 0;
	if (g_blaster_type > NR_BLASTER_TYPES)
		g_blaster_type = NR_BLASTER_TYPES;
	if (music)
		strcpy(music, g_blaster_music_drivers[g_blaster_type - 1]);
	if (digi)
		strcpy(digi, g_blaster_digi_drivers[g_blaster_type - 1]);
	if (card)
		strcpy(card, g_blaster_card_names[g_blaster_type - 1]);
	return g_blaster_type;
}

signed short blaster_detect(SNDSETUP_PTR s, char far *file, signed short far *port, signed short far *irq,
	signed short kind)
{
	char digi[0x10];
	char music[0x10];
	char card[0x40];

	if (blaster_parse(s, &g_blaster_result, music, digi, card)) {
		strcpy(file, kind == 3 ? music : digi);
		*port = g_blaster_result.io;
		*irq = g_blaster_result.irq;
		return 1;
	} else
		return 0;
}
