/* OVL176 - Overlay O176 (Segment 0x22cb in SCHWEIF.EXE): Helden-Import aus Spielstaenden von DSA 1
 * (Schicksalsklinge / Blade of Destiny). class Save (s_class.h, 0x326 Bytes): Arbeitspuffer hero (0x5ef Bytes,
 * mem_alloc), 100 Modifikations-Timer timers[] (struct dsa1_timer: long Zeit, Versatz im Heldensatz, Kennung,
 * Betrag) und swap_timers. dsa1_heroes_pos/_roa(fd): Dateiposition hinter der Tabelle mit 0x109 bzw. 0x10a
 * Blockgroessen (ab 0x1a+0x1543 bzw. 0x1a+0x1615, je Eintrag swap32) - Datei bleibt, wo sie war. Save::load(fd):
 * Dateilaenge pruefen, Kennung "DSA VERSION" (g_dsa_version_string, mit 'V' -> Amiga-Fassung) oder "ROA GAME" an +2
 * (g_str_roa_game, englisch) oder alter Stand ohne Kopf, Timer-Tabelle von +0xfb/+0xf8 einlesen, je Fassung (kind
 * 1/0x63, fmt 1 bzw. 2/3) mit einem lokalen SaveMenu-Objekt den Grundzustand laden (savegame_read) und
 * import_heroes_v1 bzw. import_heroes den Helden umsetzen lassen. check_hero(h): nach dem Import Ausruestung
 * pruefen (unbenutzbare Gegenstaende am Koerper in den Rucksack zu einem Gegenstand gleicher Art bzw. ablegen
 * drop_item, Sonderfaelle 0xb7/0xc4/0xb6/0xd7/0xd9 von der Hand in Platz 7/0/8), Werte neu setzen (set_attrib,
 * change_attrib, add_mr_mod), Gegenstand 0xd5 als magisch markieren. undo_timers/undo_timers_v1(h, swap):
 * Timer-Betraege vom DSA-1-Helden abziehen (Kennung timer_id), dann Bits seen_phantom/encouraged loeschen.
 * find_portrait(pic, sex, type, amiga): Pruefsumme eines Portraets (0x300/0x400 Bytes) in der Tabelle 0x7b
 * (g_file_main) im Bereich je Typ (g_dsa1_pics_m/_f) suchen -> Bildnummer (g_portrait_index_m/_f + k).
 * hero_clear_slot(h, slot): Gegenstand aus dem Platz entfernen (nr_items, load, unequip am Koerper).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL176.CPP   -> 4032/4032 Bytes GLEICH (18.10.2026), cobjfix laeuft
 * - -Og (DI-Zwischenwert in check_hero, [bp-14h] = (int)type in find_portrait, [bp-4]/[bp-6] in
 *   hero_clear_slot), -Oi (strcmp,
 *   strncmp, memset als Intrinsics).
 * - load: zwei switch (kind mit case 1/0x63, darin fmt) - mit if (kind == 1 || kind == 0x63) kommt cmp byte statt
 *   mov al/cbw. Das new/delete-Paar je case in eigenem Block, sonst bekommt das zweite new einen neuen
 *   Zwischenwert-Platz (Rahmen 0x1c statt 0x18).
 * - check_hero: "inv[i].magic = 1" als inline hero::set_inv_magic(i) (Parameterkopie [bp-0ch]); direkt
 *   geschrieben landet die Kopie in CX bzw. faellt weg.
 * - find_portrait: lokale Tabellen als Strukturkopie aus DS (12 Bytes rep movsw, 11 Bytes rep movsw + movsb);
 *   Schleifenbedingung "f.v[type] > i" (Tabellenwert links).
 * - Bitwort item_desc +2 einmal als Ganzes verglichen (check_hero: ITEM_FLAGS), einmal als Bitfeld (stackable,
 *   hero_clear_slot).
 * - Grundzustand ueber ein lokales SaveMenu (0x6f7 Bytes, Pic-Mitglied, ohne eigenen Konstruktor).
 * Originalfehler/Auffaelliges (siehe Bericht): check_hero prueft 0xd7 -> Slot 8 doppelt (zweiter Block wohl fuer
 *   einen anderen Gegenstand gedacht); load: "len < 0x6da && len < 0x6ca" (erste Bedingung wirkungslos);
 *   der gelesene Wert c3 wird nie benutzt.
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 */
#include <io.h>
#include <string.h>
#include "schweif.h"

/* Bitwort item_desc +2 als Ganzes (Art des Gegenstands fuer den Vergleich gleichartiger Stuecke) */
/* swap32/swap16 (O177, void far *) mit typisiertem Zeiger: so wie im Original berechnet BCC die Adresse vor push ss */
#define swap32_l ((void (*)(signed long far *))swap32)
#define swap16_w ((void (*)(unsigned short far *))swap16)

#define ITEM_FLAGS(id) (((signed short far *)&g_items[id])[1])

signed long dsa1_heroes_pos(register signed short fd)
{
	signed long n = 0x109;
	signed long pos = 0;
	signed long save;
	signed long tab[0x109];
	register signed short i;

	save = tell(fd);
	pos += 0x1a;
	pos += 0x1543;
	lseek(fd, pos, SEEK_SET);
	_read(fd, tab, n * 4);
	pos += n * 4;
	for (i = 0; i < n; i++) {
		swap32_l(&tab[i]);
		pos += tab[i];
	}
	lseek(fd, save, SEEK_SET);
	return pos;
}

signed long dsa1_heroes_pos_roa(register signed short fd)
{
	signed long n = 0x10a;
	signed long pos = 0;
	signed long save;
	signed long tab[0x10a];
	register signed short i;

	save = tell(fd);
	pos += 0x1a;
	pos += 0x1615;
	lseek(fd, pos, SEEK_SET);
	_read(fd, tab, n * 4);
	pos += n * 4;
	for (i = 0; i < n; i++) {
		swap32_l(&tab[i]);
		pos += tab[i];
	}
	lseek(fd, save, SEEK_SET);
	return pos;
}

Save::Save()
{
	hero = (struct hero far *)mem_alloc(sizeof(struct hero));
}

Save::~Save()
{
	hero = (struct hero far *)mem_free(hero);
}

signed short Save::load(signed short fd)
{
	signed char mark;
	signed char kind;
	signed char c3;
	signed char fmt;
	signed long pos;
	signed long len;
	signed short ret = 1;
	signed short amiga = 0;

	len = filelength(fd);
	if (len < 0x6da && len < 0x6ca) {
		ret = 0;
	} else {
		_read(fd, g_text_buf2, 12);
		if (!strcmp(g_text_buf2, g_dsa_version_string)) {
			_read(fd, &mark, 1);
			if (mark == 'V') {
				amiga = 1;
				_read(fd, g_text_buf2, 5);
				_read(fd, &c3, 1);
				_read(fd, &fmt, 1);
				_read(fd, &kind, 1);
				_read(fd, &mark, 1);
				_read(fd, &pos, 4);
				pos = dsa1_heroes_pos(fd);
			} else {
				_read(fd, &kind, 1);
				_read(fd, &fmt, 1);
				_read(fd, &c3, 1);
				_read(fd, &pos, 4);
			}
		} else if (!strncmp(g_text_buf2 + 2, g_str_roa_game, 8)) {
			amiga = 1;
			_read(fd, g_text_buf2, 6);
			_read(fd, &c3, 1);
			_read(fd, &fmt, 1);
			_read(fd, &kind, 1);
			_read(fd, &mark, 1);
			_read(fd, &pos, 4);
			pos = dsa1_heroes_pos_roa(fd);
		} else {
			lseek(fd, 0L, SEEK_SET);
			kind = 0x63;
			fmt = 1;
			_read(fd, &pos, 4);
		}
		swap_timers = 1;
		lseek(fd, amiga ? 0xfb : 0xf8, SEEK_CUR);
		_read(fd, timers, sizeof(timers));
		lseek(fd, pos, SEEK_SET);
		switch (kind) {
		case 1:
		case 0x63:
			switch (fmt) {
			case 1:
				{
				SaveMenu far *p = new SaveMenu;
				savegame_read(p, file_getname(g_file_main, 0x80));
				g_group = &g_groups[g_group_no];
				delete p;
				}
				ret = import_heroes_v1(fd, amiga);
				break;
			case 2:
			case 3:
				{
				SaveMenu far *p = new SaveMenu;
				savegame_read(p, file_getname(g_file_main, 0x80));
				g_group = &g_groups[g_group_no];
				delete p;
				}
				ret = import_heroes(fd, amiga);
				break;
			default:
				ret = 0;
				break;
			}
			break;
		default:
			ret = 0;
			break;
		}
	}
	return ret;
}

void Save::check_hero(struct hero far *h)
{
	signed short id;
	signed short id2;
	signed short slot;
	signed short i;
	signed short kind;
	register signed short j;

	g_quiet = 1;
	for (i = 0; i < INV_SLOT_KNAPSACK; i++) {
		id = h->inv[i].item_id;
		kind = ITEM_FLAGS(id);
		if (!can_use_item(h, id)) {
			slot = -1;
			for (j = INV_SLOT_KNAPSACK; j <= 30; j++) {
				if (h->inv[j].item_id == 0) {
					if (slot == -1)
						slot = j;
				} else if (ITEM_FLAGS(h->inv[j].item_id) == kind && can_use_item(h, h->inv[j].item_id)) {
					slot = -2;
					move_item(h, i, j);
					break;
				}
			}
			if (slot == -1)
				drop_item(h, i, 1);
			else if (slot >= INV_SLOT_KNAPSACK)
				move_item(h, i, slot);
		}
	}
	/* Gegenstaende, die in DSA 2 an einen anderen Platz gehoeren */
	id = h->inv[INV_SLOT_RIGHT_HAND].item_id;
	id2 = h->inv[INV_SLOT_LEFT_HAND].item_id;
	if (id == 0xb7 && h->inv[7].item_id == 0) move_item(h, INV_SLOT_RIGHT_HAND, 7);
	if (id2 == 0xb7 && h->inv[7].item_id == 0) move_item(h, INV_SLOT_LEFT_HAND, 7);
	if (id == 0xc4 && h->inv[0].item_id == 0) move_item(h, INV_SLOT_RIGHT_HAND, 0);
	if (id2 == 0xc4 && h->inv[0].item_id == 0) move_item(h, INV_SLOT_LEFT_HAND, 0);
	if (id == 0xb6 && h->inv[7].item_id == 0) move_item(h, INV_SLOT_RIGHT_HAND, 7);
	if (id2 == 0xb6 && h->inv[7].item_id == 0) move_item(h, INV_SLOT_LEFT_HAND, 7);
	if (id == 0xd7 && h->inv[8].item_id == 0) move_item(h, INV_SLOT_RIGHT_HAND, 8);
	if (id2 == 0xd7 && h->inv[8].item_id == 0) move_item(h, INV_SLOT_LEFT_HAND, 8);
	if (id == 0xd9 && h->inv[8].item_id == 0) move_item(h, INV_SLOT_RIGHT_HAND, 8);
	if (id2 == 0xd9 && h->inv[8].item_id == 0) move_item(h, INV_SLOT_LEFT_HAND, 8);
	if (id == 0xd7 && h->inv[8].item_id == 0) move_item(h, INV_SLOT_RIGHT_HAND, 8);
	if (id2 == 0xd7 && h->inv[8].item_id == 0) move_item(h, INV_SLOT_LEFT_HAND, 8);
	if (h->mr_mod)
		add_mr_mod(h, -h->mr_mod);
	for (j = ATTRIB_MU; j <= ATTRIB_JZ; j++)
		set_attrib(h, j, h->attrib[j].normal);
	/* Wirkungen der getragenen Gegenstaende neu */
	if (h->inv[7].item_id == 0xb7) change_attrib(h, ATTRIB_KK, 5);
	if (h->inv[0].item_id == 0xc4) change_attrib(h, ATTRIB_CH, -1);
	if (h->inv[0].item_id == 0xd9) add_mr_mod(h, 2);
	if (h->inv[4].item_id == 0xa5 || h->inv[13].item_id == 0xa5) add_mr_mod(h, 2);
	if (h->inv[7].item_id == 0xb6) change_attrib(h, ATTRIB_TA, -4);
	if (h->inv[8].item_id == 0xd7) change_attrib(h, ATTRIB_TA, -2);
	if (h->inv[2].item_id == 0x143 || h->inv[11].item_id == 0x143) change_attrib(h, ATTRIB_HA, -3);
	if (h->inv[2].item_id == 0x144 || h->inv[11].item_id == 0x144) change_attrib(h, ATTRIB_CH, 5);
	if (get_item_pos(h, 0xa3) != -1)
		add_mr_mod(h, 5);
	add_mr_mod(h, 0);
	calc_atpa_base(h);
	for (i = 0; i < 30; i++) {
		id = h->inv[i].item_id;
		if (id == 0xd5) {
			h->set_inv_magic(i);
		}
	}
	g_quiet = 0;
}

void Save::undo_timers(struct dsa1_hero far *h, signed short swap)
{
	unsigned char huge *q;
	struct dsa1_timer far *t;
	register signed short i;

	if (swap && swap_timers) {
		for (i = 0; i < 100; i++) {
			swap32_l(&timers[i].time);
			swap16_w(&timers[i].offset);
		}
	}
	swap_timers = 0;
	for (i = 0; i < 100; i++) {
		t = &timers[i];
		if (t->time && t->target && t->target == h->timer_id) {
			q = (unsigned char huge *)h;
			q += t->offset;
			*q -= t->value;
		}
	}
	h->flags2 &= 0xef;	/* ~DSA1_FLAGS2_SEEN_PHANTOM */
	h->flags2 &= 0x7f;	/* ~DSA1_FLAGS2_ENCOURAGED */
}

void Save::undo_timers_v1(struct dsa1_hero_v1 far *h, signed short swap)
{
	unsigned char huge *q;
	struct dsa1_timer far *t;
	register signed short i;

	if (swap && swap_timers) {
		for (i = 0; i < 100; i++) {
			swap32_l(&timers[i].time);
			swap16_w(&timers[i].offset);
		}
	}
	swap_timers = 0;
	for (i = 0; i < 100; i++) {
		t = &timers[i];
		if (t->time && t->target && t->target == h->timer_id) {
			q = (unsigned char huge *)h;
			q += t->offset;
			*q -= t->value;
		}
	}
	h->flags2 &= 0xef;	/* ~DSA1_FLAGS2_SEEN_PHANTOM */
	h->flags2 &= 0x7f;	/* ~DSA1_FLAGS2_ENCOURAGED */
}

signed short Save::find_portrait(signed char far *pic, signed char sex, signed char type, signed short amiga)
{
	signed long sum = 0;
	signed long far *t;
	signed short i;
	signed short k;
	signed short found = 0;
	signed short res = 0;
	signed short n = amiga ? 0x300 : 0x400;
	struct portrait_idx12 m = g_dsa1_pics_m;
	struct portrait_idx11 f = g_dsa1_pics_f;

	if (type > HERO_TYPE_GREEN_ELF)
		type = HERO_TYPE_GREEN_ELF;
	t = (signed long far *)mem_alloc(0x100L);
	for (i = 0; i < n; i++) {
		sum += *pic;
		pic++;
	}
	file_open(g_file_main, 0x7b);
	file_read(g_file_main, t, 0x100L);
	file_close(g_file_main);
	if (!sex) {
		k = 0;
		for (i = m.v[type]; f.v[type] > i; i++, k++) {
			if (t[i] == sum) {
				found = 1;
				res = g_portrait_index_m[type] + k;
				break;
			}
		}
	} else {
		k = 0;
		for (i = f.v[type]; m.v[type + 1] > i; i++, k++) {
			if (t[i] == sum) {
				found = 1;
				res = g_portrait_index_f[type] + k;
				break;
			}
		}
	}
	if (!found)
		res = sex ? g_portrait_index_f[type] : g_portrait_index_m[type];
	t = (signed long far *)mem_free(t);
	return res;
}

void hero_clear_slot(struct hero far *h, register signed short slot)
{
	signed short id;

	id = h->inv[slot].item_id;
	if (id != 0) {
		if (g_items[id].stackable) {
			h->nr_items--;
			h->load -= g_items[id].weight * h->inv[slot].quantity;
			memset(&h->inv[slot], 0, sizeof(struct inv_slot));
			return;
		}
		if (slot < INV_SLOT_KNAPSACK)
			unequip(h, id, slot);
		h->nr_items--;
		memset(&h->inv[slot], 0, sizeof(struct inv_slot));
		h->load -= g_items[id].weight;
	}
}
