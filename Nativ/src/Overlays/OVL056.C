/* OVL056 - Overlay O056 (Segment 0x2052 in SCHWEIF.EXE): Grammatik der Gegenstands-/Monsternamen (DSA1 seg096
 * GUI_names_grammar / GUI_name_plural / GUI_name_singular / GUI_2f2 / GUI_get_ptr / GUI_get_ptr2) - Wortgruppe mit
 * Artikel per sprintf in 4 rotierende 40-Byte-Puffer, Plural-/Kasusendung (neu ggue. DSA1: Namen mit Klammerzusatz
 * "X (Y)" bekommen die Endung vor der Klammer, Dativ-n per isupper), Pronomen/Artikel je Genus/Kasus (+Plural).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL056.C   -> 1528/1528 Bytes GLEICH (18.10.2026), cobjfix laeuft
 * - -Oi: strchr/strcpy/strcat als Intrinsics (repne scasb ...), -Og: *s im letzten Kopierschleifen-Kopf als
 *   Zwischenwert [bp-9].
 * - f_01f9: Merker f nur im Klammer-Block deklariert (signed short f = 0;) -> landet in DX; als Funktions-
 *   Local (auch register) immer auf dem Stack. q ebenfalls im inneren Block (liegt dann hinter dem -Og-Wert).
 * - tmp = p[-1]; und if (tmp == ')') getrennt (sonst cmp al statt cmp byte [bp-5]).
 * - f_0000: flag &= 0xffff; ist im Original wirklich da (and si,-1); l4 = Puffernummer-Kette wie DSA1.
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH) f_0000 grammar_article (BE GUI_names_grammar),
 *   f_01f9 grammar_plural (GUI_name_plural), f_0449 grammar_name (GUI_name_singular), f_04ad grammar_pronoun_of
 *   (GUI_2f2), f_0501 grammar_pronoun (GUI_get_ptr, frueher grammar_item_name), f_0594 grammar_def_article
 *   (GUI_get_ptr2). Die Zeile "ohne Artikel" ds:563b hat kein eigenes Public: g_grammar_indef_table[8].
 */
/* Phase B: BCC (C-Modus) legt das BSS in umgekehrter Reihenfolge der ersten Deklaration an - Reihenfolge hier vor schweif.h */
extern signed char far *g_mon_genders;
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:555c..56fc, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
char g_str_der[4] = "der";
char g_str_die[4] = "die";
char g_str_das[4] = "das";
char g_str_des[4] = "des";
char g_str_den[4] = "den";
char g_str_dem[4] = "dem";
char g_str_ein[4] = "ein";
char g_str_eine[5] = "eine";
char g_str_eines[6] = "eines";
char g_str_einer[6] = "einer";
char g_str_einen[6] = "einen";
char g_str_eine2[5] = "eine";
char g_str_einem[6] = "einem";
char g_str_no_article[1] = "";	/* Leereintrag 13 der Artikeltabelle (ohne Artikel) */
char g_str_von[4] = "von";
char far *g_grammar_articles[15] = {
	(char far *)&g_str_der, (char far *)&g_str_die, (char far *)&g_str_das, (char far *)&g_str_des, (char far *)&g_str_den, (char far *)&g_str_dem, (char far *)&g_str_ein, (char far *)&g_str_eine, (char far *)&g_str_eines, (char far *)&g_str_einer, (char far *)&g_str_einen, (char far *)&g_str_eine2, (char far *)&g_str_einem, (char far *)&g_str_no_article, (char far *)&g_str_von
};
struct grammar_row g_grammar_def_table[8] = {
	{ { 0, 1, 2 } },
	{ { 3, 0, 3 } },
	{ { 4, 1, 2 } },
	{ { 5, 0, 5 } },
	{ { 1, 1, 1 } },
	{ { 0, 0, 0 } },
	{ { 1, 1, 1 } },
	{ { 4, 4, 4 } }
};
struct grammar_row g_grammar_indef_table[16] = {
	{ { 6, 7, 6 } },
	{ { 8, 9, 8 } },
	{ { 10, 11, 6 } },
	{ { 12, 9, 12 } },
	{ { 13, 13, 13 } },
	{ { 13, 13, 13 } },
	{ { 13, 13, 13 } },
	{ { 13, 13, 13 } },
	{ { 13, 13, 13 } },
	{ { 13, 13, 13 } },
	{ { 13, 13, 13 } },
	{ { 13, 13, 13 } },
	{ { 13, 13, 13 } },
	{ { 13, 13, 13 } },
	{ { 13, 13, 13 } },
	{ { 13, 13, 13 } }
};
char far *g_fmt_s_s = "%s %s";
char far *g_fmt_von_s_s = "von%s %s";
signed short g_mon_no_article[2] = { 51, -1 };
signed short g_grammar_buf_no = 0;
struct grammar_row g_grammar_gender_bits = { { 4096, 8192, 12288 } };
char g_str_er[3] = "er";
char g_str_sie[4] = "sie";
char g_str_es[3] = "es";
char g_str_sein[5] = "sein";
char g_str_ihr[4] = "ihr";
char g_str_ihn[4] = "ihn";
char g_str_ihm[10] = "ihm\0ihnen";	/* "ihm", dahinter "ihnen" (ohne eigenen Verweis) */
char g_str_seine[6] = "seine";
char g_str_ihre[5] = "ihre";
char g_str_seinen[7] = "seinen";
char g_str_ihren[6] = "ihren";
char far *g_grammar_pronouns[7] = {
	(char far *)&g_str_er, (char far *)&g_str_sie, (char far *)&g_str_es, (char far *)&g_str_sein, (char far *)&g_str_ihr, (char far *)&g_str_ihn, (char far *)&g_str_ihm
};
signed char g_grammar_pronoun_idx[24] = { 0, 1, 2, 3, 4, 3, 5, 1, 2, 6, 4, 6, 1, 1, 1, 4, 4, 4, 1, 1, 1, 7, 7, 7 };
/* Phase B: eigenes BSS (ds:c806..c80a) in Adressreihenfolge (Reihenfolge festgelegt durch die extern-Zeilen vor den #include) */
signed char far *g_mon_genders;

#define TYPE_ITEM		0		/* type: index ist ein Gegenstand */
#define TYPE_MONSTER		1		/* type: index ist eine Gegnerart */
#define GRAMMAR_NO_ARTICLE	(g_grammar_indef_table[8].v)	/* ds:563b ohne Artikel (kein eigenes Public) */
#define GRAMMAR_BUF_LEN		40		/* vier rotierende Puffer g_grammar_bufs */

char far *grammar_article(signed short flag, signed short index, signed short type)
{
	signed short far *row;
	signed short von = 0;
	char far *name;
	signed short l4;
	struct grammar_row gender_bits = g_grammar_gender_bits;

	if (type == TYPE_ITEM) {
		name = g_item_names[index];
		flag += gender_bits.v[g_items[index].gender];
		row = g_items_plural2;
		while (((l4 = *row++) != -1) && (l4 != index));
		if (l4 == index) {
			flag += 4;
			flag &= 0x7fff;
			flag |= 0x4000;
			if (flag & 1)
				von = 1;
		}
	} else {
		if (is_in_word_array(index, g_mon_no_article) != -1) {
			flag &= 0x7fff;
			flag &= 0xffff;
			flag |= 0x4000;
		}
		name = g_mon_names[index];
		flag += gender_bits.v[g_mon_genders[index]];
	}

	row = (flag & 0x8000) ? g_grammar_def_table[flag & 0xf].v :
		((flag & 0x4000) ? GRAMMAR_NO_ARTICLE : g_grammar_indef_table[flag & 0xf].v);

	sprintf(g_grammar_bufs + g_grammar_buf_no * GRAMMAR_BUF_LEN,
		(von == 0) ? g_fmt_s_s : g_fmt_von_s_s,
		g_grammar_articles[row[((flag & 0x3000) - 1) >> 12]],
		grammar_plural(flag, name, type));

	name = g_grammar_bufs + g_grammar_buf_no * GRAMMAR_BUF_LEN;

	if (*name == ' ') {				/* fuehrendes Leerzeichen (leerer Artikel) entfernen */
		do {
			l4 = *++name;
			name[-1] = l4;
		} while (l4 != 0);
	}

	l4 = g_grammar_buf_no;
	if (++g_grammar_buf_no == 4)
		g_grammar_buf_no = 0;

	return g_grammar_bufs + l4 * GRAMMAR_BUF_LEN;
}

char far *grammar_plural(signed short flag, char far *s, signed short type)
{
	char far *p;
	char tmp;
	signed short kasus;
	char rest[20];

	p = g_grammar_name_buf;

	while ((tmp = *s++) && (tmp != '.'))
		*p++ = tmp;

	if (flag & 4) {
		while ((tmp = *s++) && (tmp != '.'));
	} else if ((kasus = flag & 0x0f) == 2 && type == TYPE_MONSTER) {
		while ((tmp = *s++) && (tmp != '.'));
		while ((tmp = *s++) && (tmp != '.'));
	}

	while ((tmp = *s) && (tmp != '.'))
		*p++ = *s++;

	if ((kasus = flag & 0x0f) == 1 && (flag & 0x3000) != 0x2000) {
		signed short bracket = 0;
		tmp = p[-1];
		if (tmp == ')') {
			char far *q;
			q = strchr(g_grammar_name_buf, ' ');
			strcpy(rest, q);
			p = q;
			tmp = p[-1];
			bracket = 1;
		}
		if (tmp == 'B' || tmp == 'D')
			*p++ = 'E';
		if (tmp == 'b' || tmp == 'd')
			*p++ = 'e';
		if (tmp >= 'a')
			*p++ = 's';
		else
			*p++ = 'S';
		if (bracket) {
			*p = 0;
			strcat(p, rest);
			while (*p++);
		}
	} else if (kasus == 7) {
		tmp = p[-1];
		if (isupper(tmp)) {
			if (tmp != 'N' && tmp != 'S')
				*p++ = 'N';
		} else {
			if (tmp != 'n' && tmp != 's')
				*p++ = 'n';
		}
	}

	*p = 0;
	return g_grammar_name_buf;
}

char far *grammar_name(char far *s)
{
	char far *p;
	char tmp;

	p = g_grammar_name_buf;

	while ((tmp = *s++) && (tmp != '.'))
		*p++ = tmp;

	while ((tmp = *s) && (tmp != '.'))
		*p++ = *s++;

	*p = 0;
	return g_grammar_name_buf;
}

char far *grammar_pronoun_of(signed short v1, signed short word_id, signed short type)
{
	signed short genus;

	genus = (type == TYPE_ITEM) ? g_items[word_id].gender : g_mon_genders[word_id];

	return g_grammar_pronouns[g_grammar_pronoun_idx[v1 * 3 + genus]];
}

char far *grammar_pronoun(signed short genus, signed short kasus, signed short pl)
{
	if (genus == 0) {
		return (kasus == 0) ? g_str_er :
			((kasus == 1) ? (pl ? g_str_seine : g_str_sein) :
			((kasus == 3) ? (pl ? g_str_seinen : g_str_ihm) : g_str_ihn));
	} else {
		return (kasus == 0) ? g_str_sie :
			((kasus == 1) ? (pl ? g_str_ihre : g_str_ihr) :
			((kasus == 3) ? (pl ? g_str_ihren : g_str_ihr) : g_str_sie));
	}
}

char far *grammar_def_article(signed short genus, signed short kasus)
{
	if (genus == 0) {
		return (kasus == 0) ? g_str_der :
			((kasus == 1) ? g_str_des :
			((kasus == 3) ? g_str_dem : g_str_den));
	} else {
		return (kasus == 0) ? g_str_die :
			((kasus == 1) ? g_str_der :
			((kasus == 3) ? g_str_der : g_str_die));
	}
}
