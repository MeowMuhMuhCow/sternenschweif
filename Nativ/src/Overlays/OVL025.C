/* OVL025 - Overlay O025 (Segment 0x1f81 in SCHWEIF.EXE): FLI-Animationen - Dateikopf pruefen (fli_open, FLI_MAGIC),
 * Rahmen lesen (fli_read_frame, FLI_FRAME_MAGIC) und die Chunks an die Entpacker in O270 verteilen
 * (fli_do_chunks: FLI_COLOR -> Palettenpuffer und bei show auch DAC, FLI_LC, FLI_BLACK, FLI_BRUN, FLI_COPY), Bild
 * loeschen (fli_clear_screen). Fehler in g_fli_error (FLI_ERR_*). Ziel ist g_fli_screen (struct fli_screen).
 *
 * BCC -mlarge -3 -Ot -c -Y OVL025.C   -> 711/711 Bytes GLEICH (18.10.2026)
 * - Chunk-Verteiler = switch ueber Typ 11..16 (Sprungtabelle im Code, Typ 14 = default).
 * - if (fr.size -= 0x10) ergibt sub dword/jnz NUR mit signed long; mit unsigned long laedt BCC den Wert
 *   neu (mov eax/cmp eax,0). -O darf nicht an (entfernt das jmp hinter dem letzten case und die toten Epiloge).
 * - Puffer-Zeiger ist huge (Weiterschalten per PADD@), danach Normalisierung durch fli_normalize.
 * - mem_free liefert einen Zeiger zurueck, der wieder in buf landet (buf = mem_free(buf)).
 * - fli_open: zweite Pruefung als else-if (totes jmp hinter dem ersten return).
 *   (Namensfassung mit schweif.h, Welle N19: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * - Phase B (19.10.2026): eigene Daten im Quelltext. g_fli_screen ist eine struct fli_screen (0x12 Bytes); die
 *   MAP-Publics g_fli_vga (ds:152c) und g_fli_pal sind ihre Felder scr und pal, pal zeigt auf den Palettenpuffer
 *   g_fli_palette (ds:1222, 0x300 Bytes mit Anfangspalette). Dahinter 8 Bytes ohne Verweis (00 fa ...).
 */
#include <stddef.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:1222..153c, dsana) - initialisierte Daten in Adressreihenfolge */
unsigned char g_fli_palette[0x300] = { 0 };
struct fli_screen g_fli_screen = { 0, 0, 0, 0, 0, g_fli_palette };
char g_unused_1534[8] = { 0 };	/* ds:1534, unbenutzt (0xfa00 = 320 x 200) */

/* Chunks eines Rahmens auspacken */
void fli_do_chunks(struct fli_screen far *scr, struct fli_frame far *fr, unsigned char huge *buf, signed short show)
{
	struct fli_chunk far *c;
	unsigned char far *d;
	signed short i;

	if (show) {
		fli_wait_retrace();
	}
	for (i = 0; i < fr->chunks; i++) {
		c = (struct fli_chunk far *)buf;
		d = (unsigned char far *)c + 6;
		switch (c->type) {
		case FLI_COLOR:
			if (show) {
				fli_color_dac(d);
			}
			fli_color_pal(d, scr->pal);
			break;
		case FLI_LC:
			fli_lc(d, scr->scr);
			break;
		case FLI_BLACK:
			fli_black(scr->scr, FLI_SCREEN_SIZE);
			break;
		case FLI_BRUN:
			fli_brun(d, scr->scr, scr->height);
			break;
		case FLI_COPY:
			fli_copy(d, scr->scr, FLI_SCREEN_SIZE);
			break;
		}
		buf = fli_normalize(buf + c->size);
	}
}



/* naechsten Rahmen aus g_file_cd lesen und auspacken; 0 = ok, -1 = Fehler (g_fli_error) */
signed short fli_read_frame(struct fli_screen far *scr, signed short show)
{
	unsigned char far *buf;
	struct fli_frame fr;

	g_fli_error = 0;
	if (file_read(g_file_cd, &fr, 0x10L) != 0x10) {
		g_fli_error = FLI_ERR_READ;
		return -1;
	}
	if (fr.magic != FLI_FRAME_MAGIC) {
		g_fli_error = FLI_ERR_MAGIC;
		return -1;
	}
	if (fr.size -= 0x10) {
		if ((buf = mem_alloc(fr.size)) == NULL) {
			g_fli_error = FLI_ERR_MEM;
			return -1;
		}
		if (file_read(g_file_cd, buf, (unsigned short)fr.size) != fr.size) {
			buf = mem_free(buf);
			g_fli_error = FLI_ERR_READ;
			return -1;
		}
		fli_do_chunks(scr, &fr, buf, show);
		buf = mem_free(buf);
	}
	return 0;
}

void fli_clear_screen(void)
{
	fli_black(g_fli_screen.scr, FLI_SCREEN_SIZE);
}

/* Datei h oeffnen und den Kopf pruefen; 0 = ok, -1 = Fehler (g_fli_error) */
signed short fli_open(signed short h, struct fli_header far *hdr)
{
	g_fli_error = 0;
	if (file_open(g_file_cd, h) < 0) {
		g_fli_error = FLI_ERR_OPEN;
		return -1;
	}
	if (file_read(g_file_cd, hdr, 0x80L) != 0x80) {
		file_close(g_file_cd);
		g_fli_error = FLI_ERR_READ;
		return -1;
	} else if (hdr->magic != FLI_MAGIC) {
		file_close(g_file_cd);
		g_fli_error = FLI_ERR_MAGIC;
		return -1;
	}
	return 0;
}
