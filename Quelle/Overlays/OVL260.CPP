/* OVL260 - Overlay O260 (Segment 0x24b1 in SCHWEIF.EXE): Leser fuer Dialog-Skripte. Objekt Script (0xb4 Bytes):
 * STUFE D1 (wovd1, 19.10.2026): asm-strcat durch echtes strcat(.., "..") ersetzt - Code GLEICH, das Literal
 *   im eigenen _DATA bildet wovd1/cobjfix_d.py auf ds:78c4 ab (Bytes geprueft). Hinweise unten zum asm sind alt.
 * Konstruktor f_0000(id): Merker used[] loeschen, Zeigertabelle b0 (1000 Texte), f_0415 laedt die Themenliste
 * (Datei 0x50 ueber O037_057F/O037_0225 aus ds:35c6, Zeilen per split, Namen/Titel per word/quoted), f_05b7 die
 * Skriptdatei id (+8e), dann Zeilen zerlegen und parse() f_0bf5. Destruktor f_011f gibt Abschnitte (8 Bytes je
 * Thema: Anzahl, Listen von Eintraegen zu 10 Bytes mit Byte- und Wortfeldern, farcalloc/farrealloc) und alle
 * Puffer frei. find f_02dd: Themenname -> Index; count f_0560: Zeilen (CR) zaehlen; split f_0635: Text in Zeilen
 * zerlegen (strip = 1: Name endet am ersten Leer/Tab); cut f_06e3, f_0712 (Zeile ohne Vorspann nach dst), word
 * f_07a5, quoted f_0801 ("..."), keyword f_0851 (Liste *ds:7866 -> -1-k), isquote f_0905, is_used f_0944, add
 * f_0961(a, b, c): Eintrag zum aktuellen Thema. parse: Schluesselwoerter *ds:784a (TOLERANCE/TOPIC/END/NAME
 * u. a. -> +a0/+a2/Themawechsel/Ende/+a4), Zeilen "a b "Text"" bzw. "a b SCHLUESSELWORT", Fortsetzungszeilen
 * mit '"' werden mit " " angehaengt.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL260.CPP   -> 4032/4032 Bytes GLEICH (18.10.2026), cobjfix laeuft
 * - -Og noetig fuer keyword ([bp-4] = (int)c, [bp-6] = c*4) und is_used (zwei tote Epiloge); -Oi fuer
 *   strlen/strcmp/strncmp/strcpy/strcat/strchr/memset als Intrinsics.
 * - nb0 (+0) muss unsigned short sein: nur dann rechnet BCC bei b0[nb0++] und b0[nb0 - 1] zuerst den Index
 *   (mov ax/inc bzw. dec ax/shl) und laedt danach b0; mit signed short erst les b0, Index ueber es:si und
 *   -1 in der Verschiebung (-4) gefaltet. Auch die Reihenfolge Ziel vor quoted() beim zweiten strcat folgt daraus.
 * - strcat(b0[nb0 - 1], " ") mit 1-Zeichen-Literal (ds:78c4, mov cx,2 / rep movsb) nur als asm (wie OVL238);
 *   "extern char g_78c4[2]" reicht nicht.
 * - parse: lokale Puffer 0x80 und 0x50 Bytes (Rahmen 0xf2); atoi ist inline (Kopien [bp-1eh]/[bp-22h] fuer den
 *   far-Zeiger r, bei Array-Argument keine Kopie); add() bekommt b als int (push word), speichert aber ein Byte.
 * - farcalloc(n, sizeof(char)) -> cbw/movsx eax; farrealloc(p, n * sizeof(char)) -> cbw/movzx eax (unsigned).
 * - "if (p != 0)" -> cmp dword, "if (!p)" -> mov ax / or ax.
 * - Vorwaertsaufrufe aus dem Konstruktor sind far (Funktionen weiter unten), parse ruft near.
 * Auffaelliges: Konstruktor setzt buf/names/secs/lines/titles nicht auf 0; schlaegt f_0415/f_05b7 fehl, prueft der
 *   Destruktor uninitialisierte Zeiger (sofern R07_041C nicht nullt). memset loescht nur 100 der 0x80 Bytes used[].
 *   +9c (last) wird erst bei TOPIC gesetzt - Eintraege davor vergleichen mit einem undefinierten Wert.
 * (Namensfassung mit schweif.h, class Script in s_class.h: 19.10.2026 GLEICH, LINKGLEICH) Methoden: f_0393
 *   make_titles, f_0415 load_topics, count -> count_lines, f_05b7 load, f_0712 next_line, isquote -> is_quote.
 */
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <alloc.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:784a..78c6, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
char far *g_script_keywords[7] = { "#", "PIC", "TOLERANCE", "TOPIC", "END", "NAME", 0 };
char far *g_script_answer_kw[6] = { "NULL", "LIST_TRUE", "LIST_FALSE", "LIST", "NOTMORE", 0 };

#define FILE_TOPICS	0x50		/* Themenliste */
#define MAX_TEXTS	1000		/* texts[] */

/* Schluesselwoerter am Zeilenanfang (g_script_keywords) */
#define KW_PIC		1
#define KW_TOLERANCE	2
#define KW_TOPIC	3
#define KW_END		4
#define KW_NAME		5

Script::Script(signed short id)
{
#if FIX_0227
	/* Fix #227: last vorbelegen (wie nach TOPIC) */
	n_texts = cur = 0;
	last = -1;
#else
	n_texts = cur = 0;
#endif
	texts = (char far * far *)mem_alloc(MAX_TEXTS * sizeof(char far *));
#if FIX_0067
	/* Fix #67: Zeiger nullen, ganzes Feld used[] loeschen */
	topics_raw = raw = 0;
	topic_names = lines = titles = 0;
	topics = 0;
	memset(used, 0, sizeof(used));
#else
	memset(used, 0, 100);
#endif
	if (load_topics() && load(id)) {
		n_lines = count_lines(raw);
		lines = (char far * far *)mem_alloc(n_lines * sizeof(char far *));
		split(raw, lines, n_lines, 0);
		parse();
	}
}

Script::~Script()
{
	signed short i;
	struct script_topic far *s;
	struct script_ent far *e;

	if (raw != 0)
		mem_free(raw);
	for (i = 0, s = topics; n_topics > i; i++, s++) {
		if (s->list != 0) {
			for (signed short j = 0; s->n > j; j++) {
				e = s->list[j];
				farfree(e->vals);
				farfree(e->answers);
				farfree(e);
			}
			farfree(s->list);
		}
	}
	if (topics_raw != 0) mem_free(topics_raw);
	if (topics != 0) mem_free(topics);
	if (lines != 0) mem_free(lines);
	if (topic_names != 0) mem_free(topic_names);
	if (titles != 0) mem_free(titles);
	if (texts != 0) mem_free(texts);
}

signed short Script::find(char far *name)
{
	signed short i;
	signed short len;
	char tmp[0x50];

	word(tmp, name);
	len = strlen(tmp);
	for (i = 0; n_topics > i; i++) {
		if (strlen(topic_names[i]) == len && !strcmp(tmp, topic_names[i]))
			return i;
	}
	return -1;
}

void Script::make_titles()
{
	char far *p;
	char tmp[0x50];
	register signed short i;

	for (i = 0; n_topics > i; i++) {
		p = topic_names[i];
		p = word(tmp, p);
		titles[i] = quoted(p + 1);
	}
}

signed short Script::load_topics()
{
	signed long size;

	if ((size = file_length(g_file_main, FILE_TOPICS)) != 0 && (topics_raw = (char far *)mem_alloc(size + 2)) != 0) {
		file_load(g_file_main, FILE_TOPICS, topics_raw);
		n_topics = count_lines(topics_raw);
		topic_names = (char far * far *)mem_alloc(n_topics * sizeof(char far *));
		titles = (char far * far *)mem_alloc(n_topics * sizeof(char far *));
		split(topics_raw, topic_names, n_topics, 1);
		make_titles();
		topics = (struct script_topic far *)mem_alloc((n_topics + 1) * sizeof(struct script_topic));
		return 1;
	}
	return 0;
}

signed short Script::count_lines(char far *s)
{
	signed short n = 0;
	char far *p = s;

	do {
		p = strchr(p, '\r');
		n++;
	} while (p != 0 && *++p != 0);
	return --n;
}

signed short Script::load(register signed short id)
{
	signed long size;

	if ((raw_size = size = file_length(g_file_main, id)) != 0 && (raw = (char far *)mem_alloc(size + 2)) != 0) {
		file_load(g_file_main, id, raw);
		return 1;
	}
	return 0;
}

void Script::split(char far *s, char far * far *arr, signed short n, signed short strip)
{
	register signed short i = 0;
	char far *p = s;
	char far *q;
	char far *r;

	do {
		arr[i++] = p;
		q = cut(p, '\r');
		p += strlen(p) + 1;
		if (*p == '\n' || *p == '\r') {
			*p = 0;
			p++;
		}
		if (strip == 1) {
			register char c;
			r = q;
			do
				c = *r++;
			while (c != ' ' && c != '\t' && c != 0);
			r--;
			*r = 0;
		}
	} while (i < n);
}

char far *Script::cut(char far *p, char c)
{
	char far *start = p;

	while (*p != c)
		p++;
	*p = 0;
	return start;
}

char far *Script::next_line(char far *src, char far *dst)
{
	char c;

	do
		c = *src++;
	while (c == ' ' || c == '\r' || c == '\n' || c == '\t');
	strcpy(dst, --src);
	src += strlen(dst);
	if (!*++src)
		src++;
	return src;
}

char far *Script::word(char far *dst, char far *src)
{
	register char c;

	do
		c = *src++;
	while (c == ' ' || c == '\t');
	src--;
	do
		*dst++ = c = *src++;
	while (c != 0 && c != ' ' && c != '\t' && c != '\n' && c != '\r');
	dst--;
	*dst = 0;
	return --src;
}

char far *Script::quoted(char far *p)
{
	char far *start;

	while (*p != '"' && *p != 0)
		p++;
	p++;
	start = p;
	while (*p != '"' && *p != 0)
		p++;
	*p = 0;
	return start;
}

signed short Script::keyword(char far *p)
{
	char c;

	do
		c = *p++;
	while (c != 0 && (c == ' ' || c == '\t' || c == '\n' || c == '\r'));
	p--;
	for (c = 0; g_script_answer_kw[c]; c++) {
		if (!strncmp(g_script_answer_kw[c], p, strlen(g_script_answer_kw[c])))
			return -c - 1;
	}
	return 0;
}

signed short Script::is_quote(char far *p)
{
	register char c;

	do
		c = *p++;
	while (c != 0 && (c == ' ' || c == '\t' || c == '\n' || c == '\r'));
	return *--p == '"' ? 1 : 0;
}

signed short Script::is_used(signed short i)
{
	if (used[i])
		return 1;
	else
		return 0;
}

void Script::add(signed short a, signed short b, signed short c)
{
	struct script_topic far *s = &topics[cur];
	struct script_ent far *e;
	signed short j;

	if (s->list == 0) {
		s->list = (struct script_ent far * far *)farcalloc(1, sizeof(struct script_ent far *));
		s->cap++;
	} else if (last != a) {
		s->list = (struct script_ent far * far *)farrealloc(s->list, (s->cap + 1) * sizeof(struct script_ent far *));
		s->list[s->cap] = 0;
		s->cap++;
	}
	if (last != a)
		s->n++;
	j = s->n - 1;
	if (last != a) {
		if (!s->list[j])
			s->list[j] = (struct script_ent far *)farcalloc(1, sizeof(struct script_ent));
	} else {
		s->list[j] = (struct script_ent far *)farrealloc(s->list[j], (s->list[j]->n + 1) * sizeof(struct script_ent));
	}
	last = a;
	e = s->list[j];
	e->n++;
	if (e->vals == 0)
		e->vals = (signed char far *)farcalloc(e->n, sizeof(char));
	else
		e->vals = (signed char far *)farrealloc(e->vals, e->n * sizeof(char));
	if (e->answers == 0)
		e->answers = (signed short far *)farcalloc(e->n, sizeof(signed short));
	else
		e->answers = (signed short far *)farrealloc(e->answers, e->n * sizeof(signed short));
	e->vals[e->n - 1] = b;
	e->answers[e->n - 1] = c;
}

void Script::parse()
{
	signed short a;
	signed short b;
	signed short k;
	signed short line = 0;
	signed short done = 0;
	char far *p = raw;
	char far *q;
	char far *kw;
	char far *r;
	char tmp[0x80];
	char tmp2[0x50];

	a = 0;
	do {
		p = next_line(q = p, tmp);
		for (k = 0; (kw = g_script_keywords[k]) != 0; k++) {
			if (!strncmp(tmp, kw, strlen(kw))) {
				r = tmp + strlen(kw) + 1;
				switch (k) {
				case KW_PIC:
					pic = atoi(r);
					f9e = 0;
					break;
				case KW_TOLERANCE:
					tolerance = atoi(r);
					f9e = 0;
					break;
				case KW_TOPIC:
					word(tmp2, r);
					used[cur = find(tmp2)] = 1;
					last = -1;
					f9e = 0;
					break;
				case KW_END:
					done = 1;
					f9e = 0;
					break;
				case KW_NAME:
					word(tmp, q);
					name = quoted(q);
					f9e = 0;
					break;
				}
				break;
			} else if (isdigit(tmp[0]) || tmp[0] == '-') {
				r = word(tmp2, tmp);
				a = atoi(tmp2);
				r = word(tmp2, r);
				b = atoi(tmp2);
				if (is_quote(r)) {
					texts[n_texts] = quoted(q);
					add(a, b, n_texts++);
				} else {
					texts[n_texts++] = 0;
					add(a, b, keyword(r));
				}
				break;
			} else if (tmp[0] == '"') {
				strcat(texts[n_texts - 1], " ");
				strcat(texts[n_texts - 1], quoted(q));
				break;
			}
		}
	} while (n_lines > line++ && !done);
}
