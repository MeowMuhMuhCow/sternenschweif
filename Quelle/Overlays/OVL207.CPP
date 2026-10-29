/* OVL207 - Overlay O207 (Segment 0x238d in SCHWEIF.EXE): Charakterbogen drucken (Aufrufe aus O206; Ausgabe
 * STUFE D1 (wovd1, 19.10.2026): asm-strcat durch echtes strcat(.., "..") ersetzt - Code GLEICH, das Literal
 *   im eigenen _DATA bildet wovd1/cobjfix_d.py auf ds:70b0 ab (Bytes geprueft). Hinweise unten zum asm sind alt.
 * zeilenweise ueber print_line(h, s), in Datei bei g_print_to_file, sonst Drucker; Texte h->txt).
 * print_spells(h): auf dem Drucker zuerst Hinweis dlg_choose(txt[0], txt[109]); Kopf txt[54], Name/Typ (txt[4] mit
 *   Typname g_ttx[(sex ? 0x3e : 0x32) + type] und Geschlecht txt[116/117]), dann alle Zauberwerte h->spells[SP_*]
 *   als sprintf(g_text_buf2, txt[56..107], ..) + print_line, Leerzeilen g_str_space_7096..70aa; Schluss: Datei ->
 *   g_str_space_70ac, txt[111], g_str_space_70ae; Drucker -> Seitenvorschub biosprint(0, 12, 0).
 * print_items(h, a, b): zwei Inventarplaetze nebeneinander (print_get_item liefert Name + 6 Werte), Format txt[48]
 *   (keiner), txt[46] (nur b, dann nach links geschoben), txt[47] (nur a), txt[44] (beide); Werte -1 ->
 *   txt[51]/txt[53]; Namen per strncpy auf 14 Zeichen. print_file_name(h, dst): Dateiname aus Vorlage
 *   g_print_path_init ("..\\", 20 Bytes) + Heldenname, Zeichen 3..10 ausser Buchstaben/Ziffern/Oe/Ae/Ue/ss -> '_',
 *   11 Zeichen + ".TXT".
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y -E..\bc\BIN\TASM.EXE OVL207.CPP -> 5292/5292 Bytes GLEICH (18.10.2026), cobjfix
 *   laeuft, Ohne -Og 2 Bytes zu lang (Epilogkopie print_spells, print_file_name).
 * - print_spells-Rumpf (ca. 50 sprintf/print_line-Paare) urspruenglich per gen/g207.py aus der ASM erzeugt.
 * - strcat(dst, ".CHR") mit Literal ds:70b0 (Laenge 5 als Konstante, mov cx,5 / rep movsb) nur als Inline-asm
 *   (wie O238/O260; xor ax,ax per db 33h,0c0h). ACHTUNG: mit dem Literal meldet ccmp GLEICH und cobjfix laeuft
 *   durch, das OBJ hat aber ein eigenes _DATA (5 Bytes) - nur die Segmentliste (omfinfo) verraet es.
 * - Lokales char-Feld mit Initialisierung: "struct str20 buf = g_print_path_init;" (rep movsw cx=0ah); isalnum() aus ctype.h
 *   (_ctype + 1 = ds:86bd, test ax,0eh); Umlaute als Zeichenkonstanten '\x99' usw. (cmp byte).
 * - print_items: die 12 Werte-Locals in Stack-Reihenfolge deklariert (a4, a1, a5, a6, a2, a3, b4, ...); Format per
 *   verschachteltem ?: mit gemeinsamem Store; memset/strncpy/sprintf-Puffer als lokale Felder (10/10/6/6/20/20).
 * - Typname: "(h->sex ? 0x3e : 0x32) + h->type" (type zuerst gepusht), Geschlecht "!h->sex ? .. : ..".
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <bios.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:7082..70b6, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
struct str20 g_print_path_init = { "..\\" };

void print_spells(struct hero far *h)
{
	if (!g_print_to_file)
		dlg_choose(g_dialog, h->txt[0], 1, h->txt[109]);
	print_line(h, h->txt[54]);
	print_line(h, " ");
	sprintf(g_text_buf2, h->txt[4], h->name, g_ttx[(h->sex ? 0x3e : 0x32) + h->type],
		!h->sex ? h->txt[116] : h->txt[117]);
	print_line(h, g_text_buf2);
	print_line(h, " ");
	print_line(h, " ");
	print_line(h, h->txt[55]);
	print_line(h, " ");
	sprintf(g_text_buf2, h->txt[56], h->spells[SP_BEHERRSCHUNG_BRECHEN], h->spells[SP_BALSAM]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[57], h->spells[SP_DESTRUCTIBO], h->spells[SP_HEXENSPEICHEL]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[58], h->spells[SP_GARDIANUM], h->spells[SP_KLARUM_PURUM]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[59], h->spells[SP_ILLUSIONEN_ZERSTOEREN], h->spells[SP_RUHE_KOERPER]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[60], h->spells[SP_VERWANDLUNG_BEENDEN], h->spells[SP_TIERE_BESPRECHEN]);
	print_line(h, g_text_buf2);
	print_line(h, " ");
	print_line(h, h->txt[61]);
	print_line(h, " ");
	sprintf(g_text_buf2, h->txt[62], h->spells[SP_FUROR_BLUT], h->spells[SP_AXXELERATUS]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[63], h->spells[SP_GEISTER_AUSTREIBEN], h->spells[SP_FORAMEN]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[64], h->spells[SP_GEISTER_BESCHWOEREN], h->spells[SP_MOTORICUS]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[65], h->spells[SP_HEPTAGON], h->spells[SP_SPURLOS]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[66], h->spells[SP_KRAEHENRUF], h->spells[SP_TRANSVERSALIS]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[67], h->spells[SP_SKELETTARIUS], h->spells[SP_UEBER_EIS]);
	print_line(h, g_text_buf2);
	print_line(h, " ");
	print_line(h, h->txt[68]);
	print_line(h, " ");
	sprintf(g_text_buf2, h->txt[69], h->spells[SP_ABVENENUM], h->spells[SP_CHAMAELIONI]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[70], h->spells[SP_AEOLITUS], h->spells[SP_DUPLICATUS]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[71], h->spells[SP_BRENNE_TOTER_STOFF], h->spells[SP_HARMLOSE_GESTALT]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[72], h->spells[SP_CLAUDIBUS], h->spells[SP_HEXENKNOTEN]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[73], h->spells[SP_FLIM_FLAM]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[74], h->spells[SP_DUNKELHEIT]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[75], h->spells[SP_HARTES_SCHMELZE]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[76], h->spells[SP_SILENTIUM], h->spells[SP_ELEMENTARE_BESCHWOEREN]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[77], h->spells[SP_STURMGEBRUELL], h->spells[SP_NIHILATIO_GRAVITAS]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[78], h->spells[SP_WEICHES_ERSTARRE], h->spells[SP_SOLIDIRID]);
	print_line(h, g_text_buf2);
	print_line(h, " ");
	print_line(h, h->txt[79]);
	print_line(h, " ");
	sprintf(g_text_buf2, h->txt[80], h->spells[SP_BAND_UND_FESSEL], h->spells[SP_ADLER_WOLF]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[81], h->spells[SP_BANNBALADIN], h->spells[SP_ARCANO_PSYCHOSTABILIS]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[82], h->spells[SP_BOESER_BLICK], h->spells[SP_ARMATRUTZ]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[83], h->spells[SP_GROSSE_GIER], h->spells[SP_MU_STEIGERN]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[84], h->spells[SP_GROSSE_VERWIRRUNG], h->spells[SP_KL_STEIGERN]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[85], h->spells[SP_HERR_UEBER_DAS_TIERREICH], h->spells[SP_CH_STEIGERN]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[86], h->spells[SP_HORRIPHOBUS], h->spells[SP_FF_STEIGERN]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[87], h->spells[SP_MAGISCHER_RAUB], h->spells[SP_GE_STEIGERN]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[88], h->spells[SP_RESPONDAMI], h->spells[SP_IN_STEIGERN]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[89], h->spells[SP_SANFTMUT], h->spells[SP_KK_STEIGERN]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[90], h->spells[SP_SOMNIGRAVIS], h->spells[SP_FEUERBANN]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[91], h->spells[SP_ZWINGTANZ], h->spells[SP_IN_SEE_UND_FLUSS]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[92], h->spells[SP_PARALUE]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[93], h->spells[SP_SALANDER]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[94], h->spells[SP_VISIBILI]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[95], h->spells[SP_BLITZ], h->spells[SP_MUTABILI]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[96], h->spells[SP_ECLIPTIFACTUS]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[97], h->spells[SP_EISENROST]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[98], h->spells[SP_FULMINICTUS]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[99], h->spells[SP_IGNIFAXIUS], h->spells[SP_ADLERAUG]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[100], h->spells[SP_PLUMBUMBARUM], h->spells[SP_ANALUES]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[101], h->spells[SP_RADAU], h->spells[SP_EIGENSCHAFTEN]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[102], h->spells[SP_SAFT_KRAFT], h->spells[SP_EXPOSAMI]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[103], h->spells[SP_SCHARFES_AUGE], h->spells[SP_ODEM_ARCANUM]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[104], h->spells[SP_PENETRIZZEL]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[105], h->spells[SP_SENSIBAR]);
	print_line(h, g_text_buf2);
	print_line(h, " ");
	sprintf(g_text_buf2, h->txt[106], h->spells[SP_HEXENBLICK]);
	print_line(h, g_text_buf2);
	sprintf(g_text_buf2, h->txt[107], h->spells[SP_NEKROPATHIA]);
	print_line(h, g_text_buf2);
	if (g_print_to_file) {
		print_line(h, " ");
		print_line(h, h->txt[111]);
		print_line(h, " ");
	} else {
		biosprint(0, 12, 0);
	}
}

void print_items(struct hero far *h, signed short a, signed short b)
{
	signed short a4;
	signed short a1;
	signed short a5;
	signed short a6;
	signed short a2;
	signed short a3;
	signed short b4;
	signed short b1;
	signed short b5;
	signed short b6;
	signed short b2;
	signed short b3;
	char far *n1;
	char far *n2;
	char far *fmt;
	char s1[10];
	char t1[10];
	char s2[6];
	char t2[6];
	char m1[20];
	char m2[20];

	n1 = print_get_item(h, a, &a1, &a2, &a3, &a4, &a5, &a6);
	n2 = print_get_item(h, b, &b1, &b2, &b3, &b4, &b5, &b6);
	fmt = a1 == 0 && b1 == 0 ? h->txt[48] : a1 == 0 ? h->txt[46] : b1 == 0 ? h->txt[47] : h->txt[44];
	if (a1 == 0 && b1 != 0) {
		a1 = b1;
		a2 = b2;
		a3 = b3;
		a4 = b4;
		a5 = b5;
		a6 = b6;
		n1 = n2;
	}
	sprintf(s1, a5 != -1 ? h->txt[50] : h->txt[51], a5, a6);
	sprintf(t1, b5 != -1 ? h->txt[50] : h->txt[51], b5, b6);
	sprintf(s2, a4 != -1 ? h->txt[52] : h->txt[53], a4);
	sprintf(t2, b4 != -1 ? h->txt[52] : h->txt[53], b4);
	memset(m1, 0, 20);
	memset(m2, 0, 20);
	strncpy(m1, n1, 14);
	strncpy(m2, n2, 14);
	sprintf(g_text_buf2, fmt, a2, m1, a3, s2, s1, b2, m2, b3, t2, t1);
	print_line(h, g_text_buf2);
}

void print_file_name(struct hero far *h, char far *dst)
{
	struct str20 buf = g_print_path_init;
	signed short i;
	char c;

	strcat(buf.s, h->name);
	for (i = 3; i < 11; i++) {
		c = buf.s[i];
		if (c != 0 && !isalnum(c) && c != '\x99' && c != '\x8e' && c != '\x9a' && c != '\xe1')
			buf.s[i] = '_';
	}
	strncpy(dst, buf.s, 11);
	dst[11] = 0;
	/* Literal ds:70b0 = ".TXT" (nicht ".CHR") */
	strcat(dst, ".TXT");
}
