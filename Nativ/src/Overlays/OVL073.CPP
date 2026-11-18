/* OVL073 - Overlay O073 (Segment 0x20a9 in SCHWEIF.EXE): die vier Besucher am Lagerfeuer (spaeter Gast), von O067
 * camp_visitor gerufen (Lagerobjekt c: Texte CAMP.LTX ueber c->txt, Ergebnis c->penalty; Bild 32x32 aus Datei 0x0a
 *   per
 * nvfile_load, Fensterstil dlg_set16 gesichert/zurueckgesetzt, Merker g_camp_*_met = 1). Ablauf jeweils: Gast weckt
 * die Gruppe (txt[51]), Beschreibung txt[52]/[53] (mehr als ein Gruppenmitglied) + txt[54] + Art (txt[55..58]) +
 * Zusatz per strcat, Frage txt[59] (verjagen / erst fragen / Platz anbieten; bei "fragen" Rueckfrage -> 2 = verjagen,
 * penalty = 1), Platz angeboten: zweite Frage (Rumpfreihenfolge 3, 2, 1): 3 verjagen, 2 nicht zu gastfreundlich
 * (txt[70]), 1 bewirten -> Umgebung beschreiben (camp_reveal_area(c, n)) und Tagebuch (diary_add, txt[105..108]).
 * camp_visitor_vagabond (Streuner, Bild 0x48): Essbares suchen (group_find_food), abgeben (drop_item: Lakritze 5,
 *   Bonbons 10, sonst 1), Ergebnis als signed char res (-3/-2, bei penalty == 1 -> -1). camp_visitor_hunter (Jaeger,
 *   Bild 0x88): wie der Streuner ohne res. camp_visitor_druid (Druide, Bild 0xc6): Kraeutersuppe kochen lassen und
 *   mitessen -> penalty = 99 (O072: Alptraum, Kopfschmerz). camp_visitor_juggler (Gaukler, Bild 0x8d): Blick auf
 *   seine Karte gegen Essen oder 1 Dukaten (Geld der Gruppe >= 100) -> penalty = -1, dann txt[90] und
 *   camp_reveal_area(c, 3), sonst txt[94].
 *
 * BCC -mlarge -3 -Ot -Oi -c -Y OVL073.CPP   -> 4984/4984 Bytes GLEICH (18.10.2026), cobjfix laeuft (mit -Og
 *   ebenso GLEICH). -Oi: strcat(g_text_output_buf, c->txt[n]) mit far-Quelle als Intrinsic
 *   (repne scasb / xchg si,di ..). (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Bmp als letztes Local deklariert (10 Bytes, liegt dann hinter dem far-Zeiger), Konstruktor trotzdem zuerst.
 * - camp_visitor_hunter/camp_visitor_juggler: unbenutztes char-Local (res) haelt den Platz [bp-2] wie in
 *   camp_visitor_vagabond (sub sp,1ah).
 * - "c->penalty = res = 0" -> mov al,0 / mov [bp-2],al / cbw / mov es:[13],ax; switch-Rumpfreihenfolge 3, 2, 1, der
 *   letzte Fall mit break (jmp short auf das direkt folgende Ende).
 * - pos wird nach group_find_food als Anzahl wiederverwendet ([bp-8] = 5/10/1).
 * Auffaellig: camp_visitor_juggler zieht beim Bezahlen (Geld >= 100) hier kein Geld ab (setzt nur c->penalty = -1).
 */
#include <stdio.h>
#include <string.h>

#include "schweif.h"

void camp_visitor_vagabond(Camp far *c)
{
	signed char r;
	signed char res;
	signed short old;
	signed short dummy;
	signed short pos;
	signed short slot;
	signed short id;
	struct hero far *h;
	Bmp b(0x20, 0x20);

	nvfile_load(g_nvf_heads,0x0a, b.data(), 0, 0x48, 0, &dummy, &dummy, 0x140);
	c->penalty = res = 0;
	g_camp_vagabond_met = 1;
	old = dlg_set16(g_dialog, 3);
	/* CAMP.LTX[51]: "<Gegruesset seid Ihr in Travias Namen, der Goettin des Herdfeuers und der Gastfreundschaft.>" */
	dlg_box_bmp(g_dialog, &b, (char far *)0, c->txt[51], 0);
	dlg_set16(g_dialog, 4);
	if (group_count_check1(g_group) > 1)
		/* CAMP.LTX[52]: "Nachdem Ihr die Traeume aus Euren Koepfen vertrieben habt, koennt Ihr den spaeten Gast etwas
		 * genauer in Augenschein nehmen.@Es" */
		sprintf(g_text_output_buf, c->txt[52]);
	else
		/* CAMP.LTX[53]: "Der spaete Gast" */
		sprintf(g_text_output_buf, c->txt[53]);
	/* CAMP.LTX[54]: " ist offensichtlich ein" */
	strcat(g_text_output_buf, c->txt[54]);
	/* CAMP.LTX[55]: "Streuner" */
	strcat(g_text_output_buf, c->txt[55]);
	/* CAMP.LTX[101]: ", der mit seiner wenigen Habe und zerschlissenen Kleidern durch die Gegend streift." */
	strcat(g_text_output_buf, c->txt[101]);
	dlg_box_bmp(g_dialog, &b, (char far *)0, g_text_output_buf, 0);
	/* CAMP.LTX[59]: "<Kann ich mich an Eurem Feuer aufwaermen?>, fragt der Besucher, bevor Ihr Zeit habt Euch ein
	 * genaueres Bild von ihm zu machen. Wollt Ihr" */
	/* CAMP.LTX[63]: "Ihm einen Platz anbieten" */
	/* CAMP.LTX[61]: "Ihn davonjagen, da er bestimmt nichts Gutes im Schilde fuehrt" */
	/* CAMP.LTX[62]: "Ihn zuerst fragen, warum er so spaet unterwegs ist" */
	r = dlg_box_bmp(g_dialog, &b, (char far *)0, c->txt[59], 3, c->txt[63], c->txt[61], c->txt[62]);
	if (r == 3)
		/* CAMP.LTX[65]: "<Seit die Orks ueberall herumstreifen ist man seines Lebens nicht mehr sicher. Da ist es besser
		 * nachts zu reisen, und tagsueber in der Sicherheit einer Stadt zu sein.>@Wollt Ihr" */
		/* CAMP.LTX[71]: "ihm jetzt einen Platz anbieten" */
		/* CAMP.LTX[72]: "ihm kein Wort glauben und ihn davonjagen" */
		r = dlg_box_bmp(g_dialog, &b, (char far *)0, c->txt[65], 2, c->txt[71], c->txt[72]);
	if (r == 2)
		c->penalty = 1;
	if (r == 1) {
		/* CAMP.LTX[66]: "Der Mann laesst sich am Feuer nieder und erzaehlt Euch von seiner heutigen Reisestrecke.@Als
		 * jedoch sein Magen zu knurren beginnt unterbricht er seinen Bericht. <Habt Ihr wohl noch etwas zu Essen fuer
		 * einen einsamen Wanderer?> Wollt Ihr" */
		/* CAMP.LTX[67]: "nachsehen, ob Ihr noch etwas abgeben koennt" */
		/* CAMP.LTX[68]: "es mit der Gastfreundschaft nicht uebertreiben" */
		/* CAMP.LTX[69]: "den unverschaemten Kerl jetzt davonjagen" */
		r = dlg_box_bmp(g_dialog, &b, (char far *)0, c->txt[66], 3, c->txt[67], c->txt[68], c->txt[69]);
		switch (r) {
		case 3:
			c->penalty = 1;
			break;
		case 2:
			/* CAMP.LTX[70]: "Schweigend sitzt der %s noch einige Minuten am Feuer, bis er sich grusslos wieder auf den Weg
			 * macht." */
			sprintf(g_text_output_buf, c->txt[70], c->txt[55]);
			dlg_box_bmp(g_dialog, &b, (char far *)0, g_text_output_buf, 0);
			res = -2;
			break;
		case 1:
			if (group_find_food(g_group, (struct hero far *)0, &pos, &slot, &id) != 0) {
				/* CAMP.LTX[73]: "Nachdem Ihr noch etwas Essbares in einem Rucksack gefunden habt, laesst es sich Euer Gast
				 * schmecken. Unter seinem Schmatzen koennt Ihr verstehen, wie er etwas von dem <alten Karrenweg> und dem
				 * <Oedenweg> erzaehlt, und mit einem Dolch mehrere Wege in den Boden kritzelt. Schliesslich macht er sich
				 * wieder auf den Weg." */
				dlg_message(g_dialog, c->txt[73]);
				h = g_group->heroes[pos];
				switch (id) {
				case ITEM_LIQUORICE:
					pos = 5;
					break;
				case ITEM_SWEETS:
					pos = 10;
					break;
				default:
					pos = 1;
					break;
				}
				drop_item(h, slot, pos);
			} else
				/* CAMP.LTX[74]: "Trotz genauer Suche koennt Ihr keine Vorraete mehr finden. Euer Gast erzaehlt noch etwas von
				 * dem <alten Karrenweg> und dem <Oedenweg>, und kritzelt dabei mit einem Dolch mehrere Wege in den Boden, bis
				 * er sich schliesslich wieder auf den Weg macht." */
				dlg_message(g_dialog, c->txt[74]);
			/* CAMP.LTX[75]: "Mit diesen Beschreibungen duerfte es ein Leichtes sein, sich die naechsten Tage
			 * zurechtzufinden." */
			dlg_message(g_dialog, c->txt[75]);
			res = -3;
			camp_reveal_area(c, 1);
			/* CAMP.LTX[105]: "Ein Streuner ist auf unser Lager aufmerksam geworden. Er gesellte sich zu uns und beschrieb
			 * uns ein wenig die umliegende Gegend." */
			diary_add(g_diary, c->txt[105], 0);
			break;
		}
	}
	if (c->penalty == 1)
		res = -1;
	c->penalty = res;
	dlg_set16(g_dialog, old);
}

void camp_visitor_hunter(Camp far *c)
{
	signed char r;
	signed char res;
	signed short old;
	signed short dummy;
	signed short pos;
	signed short slot;
	signed short id;
	struct hero far *h;
	Bmp b(0x20, 0x20);

	nvfile_load(g_nvf_heads,0x0a, b.data(), 0, 0x88, 0, &dummy, &dummy, 0x140);
	g_camp_hunter_met = 1;
	c->penalty = 0;
	old = dlg_set16(g_dialog, 4);
	/* CAMP.LTX[51]: "<Gegruesset seid Ihr in Travias Namen, der Goettin des Herdfeuers und der Gastfreundschaft.>" */
	dlg_box_bmp(g_dialog, &b, (char far *)0, c->txt[51], 0);
	if (group_count_check1(g_group) > 1)
		/* CAMP.LTX[52]: "Nachdem Ihr die Traeume aus Euren Koepfen vertrieben habt, koennt Ihr den spaeten Gast etwas
		 * genauer in Augenschein nehmen.@Es" */
		sprintf(g_text_output_buf, c->txt[52]);
	else
		/* CAMP.LTX[53]: "Der spaete Gast" */
		sprintf(g_text_output_buf, c->txt[53]);
	/* CAMP.LTX[54]: " ist offensichtlich ein" */
	strcat(g_text_output_buf, c->txt[54]);
	/* CAMP.LTX[57]: "Jaeger" */
	strcat(g_text_output_buf, c->txt[57]);
	/* CAMP.LTX[103]: ", der noch keine Beute gemacht hat." */
	strcat(g_text_output_buf, c->txt[103]);
	dlg_box_bmp(g_dialog, &b, (char far *)0, g_text_output_buf, 0);
	/* CAMP.LTX[59]: "<Kann ich mich an Eurem Feuer aufwaermen?>, fragt der Besucher, bevor Ihr Zeit habt Euch ein
	 * genaueres Bild von ihm zu machen. Wollt Ihr" */
	/* CAMP.LTX[63]: "Ihm einen Platz anbieten" */
	/* CAMP.LTX[61]: "Ihn davonjagen, da er bestimmt nichts Gutes im Schilde fuehrt" */
	/* CAMP.LTX[62]: "Ihn zuerst fragen, warum er so spaet unterwegs ist" */
	r = dlg_box_bmp(g_dialog, &b, (char far *)0, c->txt[59], 3, c->txt[63], c->txt[61], c->txt[62]);
	if (r == 3)
		/* CAMP.LTX[96]: "<Normalerweise kann man um diese Zeit gut Wiesel und aehnliches Getier fangen, fuer deren Felle
		 * man gute Preise erzielt, aber heute war ich noch gluecklos.> Wollt Ihr" */
		/* CAMP.LTX[71]: "ihm jetzt einen Platz anbieten" */
		/* CAMP.LTX[72]: "ihm kein Wort glauben und ihn davonjagen" */
		r = dlg_box_bmp(g_dialog, &b, (char far *)0, c->txt[96], 2, c->txt[71], c->txt[72]);
	if (r == 2)
		c->penalty = 1;
	if (r == 1) {
		/* CAMP.LTX[97]: "Der Mann laesst sich am Feuer nieder und erzaehlt Euch von seinen heutigen Bemuehungen, wobei
		 * er das umliegende Gelaende sehr genau beschreibt.@Als jedoch sein Magen zu knurren beginnt unterbricht er
		 * seinen Bericht. <Habt Ihr wohl noch etwas zu Essen fuer mich?> Wollt Ihr" */
		/* CAMP.LTX[67]: "nachsehen, ob Ihr noch etwas abgeben koennt" */
		/* CAMP.LTX[68]: "es mit der Gastfreundschaft nicht uebertreiben" */
		/* CAMP.LTX[69]: "den unverschaemten Kerl jetzt davonjagen" */
		r = dlg_box_bmp(g_dialog, &b, (char far *)0, c->txt[97], 3, c->txt[67], c->txt[68], c->txt[69]);
		switch (r) {
		case 3:
			c->penalty = 1;
			break;
		case 2:
			/* CAMP.LTX[70]: "Schweigend sitzt der %s noch einige Minuten am Feuer, bis er sich grusslos wieder auf den Weg
			 * macht." */
			sprintf(g_text_output_buf, c->txt[70], c->txt[57]);
			dlg_box_bmp(g_dialog, &b, (char far *)0, g_text_output_buf, 0);
			break;
		case 1:
			if (group_find_food(g_group, (struct hero far *)0, &pos, &slot, &id) != 0) {
				/* CAMP.LTX[100]: "Nachdem Ihr noch etwas Essbares in einem Rucksack gefunden habt, laesst es sich Euer Gast
				 * schmecken. Waehrend er genuesslich sein Mahl verzehrt, faehrt er mit der Beschreibung der umliegenden Wege
				 * fort, wobei er haeufig von dem <grossen Thaschpass> spricht. Schliesslich hat er sein Mahl beendet und macht
				 * sich wieder auf den Weg." */
				dlg_message(g_dialog, c->txt[100]);
				h = g_group->heroes[pos];
				switch (id) {
				case ITEM_LIQUORICE:
					pos = 5;
					break;
				case ITEM_SWEETS:
					pos = 10;
					break;
				default:
					pos = 1;
					break;
				}
				drop_item(h, slot, pos);
			} else
				/* CAMP.LTX[74]: "Trotz genauer Suche koennt Ihr keine Vorraete mehr finden. Euer Gast erzaehlt noch etwas von
				 * dem <alten Karrenweg> und dem <Oedenweg>, und kritzelt dabei mit einem Dolch mehrere Wege in den Boden, bis
				 * er sich schliesslich wieder auf den Weg macht." */
				dlg_message(g_dialog, c->txt[74]);
			camp_reveal_area(c, 4);
			/* CAMP.LTX[106]: "Ein Jaeger hat unser Lager entdeckt. Er blieb eine Weile bei uns und wir konnten von ihm
			 * einiges ueber die naehere Umgebung erfahren." */
			diary_add(g_diary, c->txt[106], 0);
			break;
		}
	}
	dlg_set16(g_dialog, old);
}

void camp_visitor_druid(Camp far *c)
{
	signed char r;
	signed short old;
	signed short dummy;
	Bmp b(0x20, 0x20);

	nvfile_load(g_nvf_heads,0x0a, b.data(), 0, 0xc6, 0, &dummy, &dummy, 0x140);
	c->penalty = 0;
	g_camp_druid_met = 1;
	old = dlg_set16(g_dialog, 4);
	/* CAMP.LTX[51]: "<Gegruesset seid Ihr in Travias Namen, der Goettin des Herdfeuers und der Gastfreundschaft.>" */
	dlg_box_bmp(g_dialog, &b, (char far *)0, c->txt[51], 0);
	if (group_count_check1(g_group) > 1)
		/* CAMP.LTX[52]: "Nachdem Ihr die Traeume aus Euren Koepfen vertrieben habt, koennt Ihr den spaeten Gast etwas
		 * genauer in Augenschein nehmen.@Es" */
		sprintf(g_text_output_buf, c->txt[52]);
	else
		/* CAMP.LTX[53]: "Der spaete Gast" */
		sprintf(g_text_output_buf, c->txt[53]);
	/* CAMP.LTX[54]: " ist offensichtlich ein" */
	strcat(g_text_output_buf, c->txt[54]);
	/* CAMP.LTX[56]: "Druide" */
	strcat(g_text_output_buf, c->txt[56]);
	/* CAMP.LTX[102]: ", der mit einer Sichel und einem kleinen Beutelchen mit Kraeutern ausgestattet ist." */
	strcat(g_text_output_buf, c->txt[102]);
	dlg_box_bmp(g_dialog, &b, (char far *)0, g_text_output_buf, 0);
	/* CAMP.LTX[59]: "<Kann ich mich an Eurem Feuer aufwaermen?>, fragt der Besucher, bevor Ihr Zeit habt Euch ein
	 * genaueres Bild von ihm zu machen. Wollt Ihr" */
	/* CAMP.LTX[63]: "Ihm einen Platz anbieten" */
	/* CAMP.LTX[61]: "Ihn davonjagen, da er bestimmt nichts Gutes im Schilde fuehrt" */
	/* CAMP.LTX[62]: "Ihn zuerst fragen, warum er so spaet unterwegs ist" */
	r = dlg_box_bmp(g_dialog, &b, (char far *)0, c->txt[59], 3, c->txt[63], c->txt[61], c->txt[62]);
	if (r == 3)
		/* CAMP.LTX[76]: "<Bei manchen Kraeutern ist es einfach notwendig, dass man sie nachts schneidet, weil sie sonst
		 * ihre Wirkung verlieren. Heute bin ich sogar schon fuendig geworden.> Wollt Ihr" */
		/* CAMP.LTX[71]: "ihm jetzt einen Platz anbieten" */
		/* CAMP.LTX[72]: "ihm kein Wort glauben und ihn davonjagen" */
		r = dlg_box_bmp(g_dialog, &b, (char far *)0, c->txt[76], 2, c->txt[71], c->txt[72]);
	if (r == 2)
		c->penalty = 1;
	if (r == 1) {
		/* CAMP.LTX[77]: "<Wenn Ihr erlaubt wuerde ich mir mit den frischen Kraeutern gleich ein Sueppchen kochen. Wollt
		 * Ihr" */
		/* CAMP.LTX[78]: "dem Alten seinen Wunsch erfuellen" */
		/* CAMP.LTX[68]: "es mit der Gastfreundschaft nicht uebertreiben" */
		/* CAMP.LTX[69]: "den unverschaemten Kerl jetzt davonjagen" */
		r = dlg_box_bmp(g_dialog, &b, (char far *)0, c->txt[77], 3, c->txt[78], c->txt[68], c->txt[69]);
		switch (r) {
		case 3:
			c->penalty = 1;
			break;
		case 2:
			/* CAMP.LTX[70]: "Schweigend sitzt der %s noch einige Minuten am Feuer, bis er sich grusslos wieder auf den Weg
			 * macht." */
			sprintf(g_text_output_buf, c->txt[70], c->txt[56]);
			dlg_box_bmp(g_dialog, &b, (char far *)0, g_text_output_buf, 0);
			break;
		case 1:
			/* CAMP.LTX[79]: "Sogleich macht sich der Druide an die Arbeit und bald dampft eine warme Suppe ueber dem Feuer.
			 * <Nehmt doch auch eine Kleinigkeit.> Wollt Ihr" */
			/* CAMP.LTX[80]: "sein Angebot annehmen" */
			/* CAMP.LTX[81]: "dankend ablehnen" */
			r = dlg_box_bmp(g_dialog, &b, (char far *)0, c->txt[79], 2, c->txt[80], c->txt[81]);
			if (r == 1) {
				/* CAMP.LTX[82]: "Die Suppe schmeckt wirklich ausgezeichnet und bald ist der Topf leergegessen. Der Druide
				 * macht sich wieder auf den Weg und Ihr koennt weiter traeumen." */
				dlg_message(g_dialog, c->txt[82]);
				c->penalty = 99;
				camp_reveal_area(c, 2);
				/* CAMP.LTX[107]: "Ein Druide ist auf unser Lager aufmerksam geworden. Er gesellte sich zu uns und beschrieb
				 * uns ein wenig die umliegende Gegend." */
				diary_add(g_diary, c->txt[107], 0);
			} else
				/* CAMP.LTX[99]: "Der Druide loeffelt genuesslich seine Suppe. Als er keinen Appetit mehr hat schuettet er den
				 * Rest neben einen Strauch und macht sich wieder auf den Weg." */
				dlg_message(g_dialog, c->txt[99]);
			break;
		}
	}
	dlg_set16(g_dialog, old);
}

void camp_visitor_juggler(Camp far *c)
{
	signed char r;
	signed char res;
	signed short old;
	signed short dummy;
	signed short pos;
	signed short slot;
	signed short id;
	struct hero far *h;
	Bmp b(0x20, 0x20);

	nvfile_load(g_nvf_heads,0x0a, b.data(), 0, 0x8d, 0, &dummy, &dummy, 0x140);
	old = dlg_set16(g_dialog, 4);
	g_camp_juggler_met = 1;
	c->penalty = 0;
	/* CAMP.LTX[51]: "<Gegruesset seid Ihr in Travias Namen, der Goettin des Herdfeuers und der Gastfreundschaft.>" */
	dlg_box_bmp(g_dialog, &b, (char far *)0, c->txt[51], 0);
	if (group_count_check1(g_group) > 1)
		/* CAMP.LTX[52]: "Nachdem Ihr die Traeume aus Euren Koepfen vertrieben habt, koennt Ihr den spaeten Gast etwas
		 * genauer in Augenschein nehmen.@Es" */
		sprintf(g_text_output_buf, c->txt[52]);
	else
		/* CAMP.LTX[53]: "Der spaete Gast" */
		sprintf(g_text_output_buf, c->txt[53]);
	/* CAMP.LTX[54]: " ist offensichtlich ein" */
	strcat(g_text_output_buf, c->txt[54]);
	/* CAMP.LTX[58]: "Musikant" */
	strcat(g_text_output_buf, c->txt[58]);
	/* CAMP.LTX[104]: ", wie man an seinen bunten Kleidern und einer Laute erkennt." */
	strcat(g_text_output_buf, c->txt[104]);
	dlg_box_bmp(g_dialog, &b, (char far *)0, g_text_output_buf, 0);
	/* CAMP.LTX[59]: "<Kann ich mich an Eurem Feuer aufwaermen?>, fragt der Besucher, bevor Ihr Zeit habt Euch ein
	 * genaueres Bild von ihm zu machen. Wollt Ihr" */
	/* CAMP.LTX[63]: "Ihm einen Platz anbieten" */
	/* CAMP.LTX[61]: "Ihn davonjagen, da er bestimmt nichts Gutes im Schilde fuehrt" */
	/* CAMP.LTX[62]: "Ihn zuerst fragen, warum er so spaet unterwegs ist" */
	r = dlg_box_bmp(g_dialog, &b, (char far *)0, c->txt[59], 3, c->txt[63], c->txt[61], c->txt[62]);
	if (r == 3)
		/* CAMP.LTX[84]: "<Ich habe es sehr eilig, weil meine Tante aus Lowangen im Sterben liegt.> Wollt Ihr" */
		/* CAMP.LTX[71]: "ihm jetzt einen Platz anbieten" */
		/* CAMP.LTX[72]: "ihm kein Wort glauben und ihn davonjagen" */
		r = dlg_box_bmp(g_dialog, &b, (char far *)0, c->txt[84], 2, c->txt[71], c->txt[72]);
	if (r == 2)
		c->penalty = 1;
	if (r == 1) {
		/* CAMP.LTX[66]: "Der Mann laesst sich am Feuer nieder und erzaehlt Euch von seiner heutigen Reisestrecke.@Als
		 * jedoch sein Magen zu knurren beginnt unterbricht er seinen Bericht. <Habt Ihr wohl noch etwas zu Essen fuer
		 * einen einsamen Wanderer?> Wollt Ihr" */
		/* CAMP.LTX[67]: "nachsehen, ob Ihr noch etwas abgeben koennt" */
		/* CAMP.LTX[68]: "es mit der Gastfreundschaft nicht uebertreiben" */
		/* CAMP.LTX[69]: "den unverschaemten Kerl jetzt davonjagen" */
		r = dlg_box_bmp(g_dialog, &b, (char far *)0, c->txt[66], 3, c->txt[67], c->txt[68], c->txt[69]);
		switch (r) {
		case 3:
			c->penalty = 1;
			break;
		case 2:
			/* CAMP.LTX[70]: "Schweigend sitzt der %s noch einige Minuten am Feuer, bis er sich grusslos wieder auf den Weg
			 * macht." */
			sprintf(g_text_output_buf, c->txt[70], c->txt[58]);
			dlg_box_bmp(g_dialog, &b, (char far *)0, g_text_output_buf, 0);
			break;
		case 1:
			if (group_find_food(g_group, (struct hero far *)0, &pos, &slot, &id) != 0) {
				/* CAMP.LTX[85]: "Ihr findet etwas Essbares in einem der Rucksaecke und gebt dem Gaukler davon." */
				dlg_box_bmp(g_dialog, &b, (char far *)0, c->txt[85], 0);
				h = g_group->heroes[pos];
				switch (id) {
				case ITEM_LIQUORICE:
					pos = 5;
					break;
				case ITEM_SWEETS:
					pos = 10;
					break;
				default:
					pos = 1;
					break;
				}
				drop_item(h, slot, pos);
				/* CAMP.LTX[87]: "<Ihr seid nicht von hier, oder?@Ich habe eine Karte von der Gegend dabei und wuerde Euch %s
				 * einen Blick darauf werfen lassen.>" */
				/* CAMP.LTX[88]: "gerne" */
				sprintf(g_text_output_buf, c->txt[87], c->txt[88]);
				dlg_box_bmp(g_dialog, &b, (char far *)0, g_text_output_buf, 0);
				c->penalty = -1;
			} else {
				/* CAMP.LTX[86]: "Ihr muesst feststellen, dass Ihr selbst nichts mehr uebrig habt." */
				dlg_box_bmp(g_dialog, &b, (char far *)0, c->txt[86], 0);
				/* CAMP.LTX[89]: "fuer 1 Dukaten" */
				sprintf(g_text_output_buf, c->txt[87], c->txt[89]);
				/* CAMP.LTX[91]: "Wollt Ihr dem Gaukler den Dukaten geben?" */
				strcat(g_text_output_buf, c->txt[91]);
				/* CAMP.LTX[92]: "ja" */
				/* CAMP.LTX[93]: "nein" */
				r = dlg_box_bmp(g_dialog, &b, (char far *)0, g_text_output_buf, 2, c->txt[92], c->txt[93]);
				if (r == 1) {
					if (g_group->money < 100)
						/* CAMP.LTX[95]: "Ihr muesst gestehen, dass Ihr gar nicht genug Geld dabei habt, worauf der Gaukler seine
						 * Reise fortsetzt, und dabei etwas von Verrueckten murmelt." */
						dlg_message(g_dialog, c->txt[95]);
					else
						c->penalty = -1;
				}
			}
			if (c->penalty != 0) {
				if (c->penalty == -1) {
					/* CAMP.LTX[90]: "Nachdem Ihr Euch die naeheren Strecken eingepraegt habt, macht sich der Reisende wieder auf
					 * den Weg." */
					dlg_message(g_dialog, c->txt[90]);
					camp_reveal_area(c, 3);
					/* CAMP.LTX[108]: "Ein Gaukler hat unser Lager entdeckt. Er blieb eine Weile bei uns und wir konnten von ihm
					 * einiges ueber die naehere Umgebung erfahren." */
					diary_add(g_diary, c->txt[108], 0);
					c->penalty = 0;
				}
			} else
				/* CAMP.LTX[94]: "<Na gut, dann eben nicht!> Mit diesen Worten setzt der Gaukler seine naechtliche Reise fort." */
				dlg_message(g_dialog, c->txt[94]);
			break;
		}
	}
	dlg_set16(g_dialog, old);
}
