/* OVL182 - Overlay O182 (Segment 0x22eb in SCHWEIF.EXE): Gegenstandstabelle Items (12 Bytes, Objekt ds:b84a = g_items;
 * class Items in s_class.h). Konstruktor laedt Datei 0x0c ITEMS.DAT (struct item_desc, 14 Bytes je Gegenstand) ueber
 * das Dateiobjekt g_file_main (O037) nach g_items, die Namen Datei 0x0d ITEMS.LTX per txt_load_list (Eintraege + 10)
 * nach names_buf/g_item_names und legt die Nachfragezaehler g_item_demand (g_nr_items Bytes) an; Destruktor gibt
 * Tabelle und Zaehler frei. change_price(id, pct): price += price * price_unit / 100 * pct / price_unit (long-
 * Rechnung), liefert den neuen Preis - Haendler aendern so den Preis bei Nachfrage (O069, O226: +-10 %).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL182.CPP   -> 455/455 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - mem_alloc((unsigned short)g_nr_items): g_nr_items ist im Header signed (sonst movsx statt movzx).
 * Schluessel: [bp-0ah] in change_price() ist KEINE Variable, sondern der -Og-Zwischenwert fuer e->price_unit
 *   (zweimal benutzt); dadurch "push ax / mov ax,e->price / pop dx / imul dx". Ohne -Og bzw. mit eigener Variable
 *   nie gleich. Ein Konstruktor mit "if (this==0) this = operator new(12)" und totem Epilog = gewoehnlicher
 *   C++-Konstruktor. (signed short)l beim += ist noetig (sonst long-Addition).
 */
#include "schweif.h"

#define FILE_ITEMS_DAT	0x0c		/* ITEMS.DAT: struct item_desc je Gegenstand */
#define FILE_ITEMS_LTX	0x0d		/* ITEMS.LTX: Gegenstandsnamen */

Items::Items()
{
	signed long len;
	signed short n;

	file_open(g_file_main, FILE_ITEMS_DAT);
	len = file_length2(g_file_main);
	desc = (struct item_desc far *)mem_alloc(len);
	len = File_read_all(g_file_main, desc);
	file_close(g_file_main);
	n = len / sizeof(struct item_desc) + 10;
	txt_load_list(g_font, FILE_ITEMS_LTX, n, &names_buf, &names);
	g_nr_items = n - 9;
	g_item_demand = (signed char far *)mem_alloc((unsigned short)g_nr_items);
}

Items::~Items()
{
	mem_free(desc);
	mem_free(g_item_demand);
}

signed short Items::change_price(signed short id, signed short pct)
{
	signed long l;
	struct item_desc far *e;

	e = desc + id;
	l = e->price * e->price_unit;
	l /= 100;
	l *= pct;
	l /= e->price_unit;
	e->price += (signed short)l;
	return e->price;
}
