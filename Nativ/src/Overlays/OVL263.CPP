/* OVL263 - Overlay O263 (Segment 0x24c3 in SCHWEIF.EXE): Gefangennahme - Gegenstaende der Gruppe einsammeln.
 * group_confiscate_herbs/_armor/_weapons/_rest: fuer jeden der 7 Helden (g_group) und jeden der 31 Plaetze
 *   Gegenstaende mit bestimmten Eigenschaften (g_items: herb_potion oder magic / armor / weapon / alle), die nicht in
 *   g_never_confiscated stehen (is_in_word_array == -1), in die Ablage Stash_ctor(0, 0x34/0x35/0x36/0x33, 1) eintragen
 *   (stash_set) und beim Helden entfernen (hero_remove_item; Nahrung wird nur entfernt), Ablage mit Endmarke -1
 *   abschliessen und schreiben (Stash_dtor(p, 3)).
 * group_give_clothes: lebende Helden (check_hero3) bekommen Hose und Schuhe (Heldinnen auch ein Hemd) und legen sie an
 *   (give_hero_new_item, move_item aus dem ersten Rucksackplatz).
 * group_confiscate: alles zwischen diary_unlock/diary_lock(g_diary) (Aufrufer O264 mit seinem Ereignisobjekt).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL263.CPP   -> 1474/1474 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h, Welle N15: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * - C++ wegen inline-Memberfunktionen: group::get_hero(n) erzeugt die Parameterkopie (DI in group_give_clothes,
 *   [bp-0eh] sonst) beim Weiterschalten; der Startwert h = g_group->heroes[0] wird mit -3 als mov eax kopiert.
 * - item_desc::is_magic() { return magic; } liefert mov al/cbw/or ax,ax (direkt im || gibt es cmp byte); is_armor()
 *   usw. ueber die Bitfelder.
 * - inv_slot::get_quantity() als Argument fuer den unsigned-char-Parameter von stash_set ergibt
 *   mov ax,es:[..] / push ax (ohne inline: mov al; mit int-Parameter: push word es:[..]).
 * - -Og noetig: Zwischenwert [bp-10h] = Offset von &g_items[id] (Segment danach aus ds:b84c nachgeladen).
 * - i = n = 0 (n bleibt 0, wird so an stash_set gegeben - alle Eintraege landen auf Platz 0).
 * Originalfehler: n wird nie erhoeht - jede Ablage enthaelt danach nur die Endmarke -1 auf Platz 0, die
 * eingesammelten Gegenstaende sind verloren (Stash::set schreibt genau Platz i).
 */
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:78c6..78cc, dsana) - initialisierte Daten in Adressreihenfolge */
signed short g_never_confiscated[3] = { 133, 140, -1 };

/* jedem lebenden Helden Hose und Schuhe (Heldinnen auch ein Hemd) geben und anlegen */
void group_give_clothes(void far *ctx)
{
	struct hero far *h;
	register signed short i;

	for (i = 0, h = g_group->heroes[0]; i <= 6; h = g_group->get_hero(++i)) {
		if (h && check_hero3(h)) {
			give_hero_new_item(h, ITEM_TROUSERS, 1, 1);
			move_item(h, 5, INV_SLOT_KNAPSACK);
			give_hero_new_item(h, ITEM_SHOES, 1, 1);
			move_item(h, 14, INV_SLOT_KNAPSACK);
			if (h->sex != 0) {
				give_hero_new_item(h, ITEM_SHIRT, 1, 1);
				move_item(h, 10, INV_SLOT_KNAPSACK);
			}
		}
	}
}

/* Gefangennahme: alle Gegenstaende der Gruppe (ausser g_never_confiscated) in die Ablagen 0x33..0x36, danach neue
 * Kleidung (Aufrufer O264 mit seinem Ereignisobjekt, hier unbenutzt) */
void group_confiscate(void far *ctx)
{
	diary_unlock(g_diary);
	group_confiscate_herbs(ctx);
	group_confiscate_armor(ctx);
	group_confiscate_weapons(ctx);
	group_confiscate_rest(ctx);
	group_give_clothes(ctx);
	diary_lock(g_diary);
}

/* Kraeuter/Traenke und magische Gegenstaende in Ablage 0x34, Nahrung wegwerfen */
void group_confiscate_herbs(void far *ctx)
{
	signed short i;
	signed short n;
	struct hero far *h;
	void far *lst;
	register signed short k;
	register signed short id;

	lst = Stash_ctor(0, 0x34, 1);
	i = n = 0;
	for (h = g_group->heroes[0]; i <= 6; h = g_group->get_hero(++i)) {
		if (h) {
			for (k = 0; k < 31; k++) {
				id = h->inv[k].item_id;
				if (id && (g_items[id].is_herb_potion() || g_items[id].is_magic()) && is_in_word_array(id, g_never_confiscated) == -1) {
#if FIX_0212
					/* Fix #212: naechsten Platz der Ablage benutzen (n erhoehen) - die Gegenstaende bleiben erhalten */
					if (n < 0xd9) {
						stash_set(lst, n++, id, h->inv[k].get_quantity());
						hero_remove_item(h, k);
					}
#else
					stash_set(lst, n, id, h->inv[k].get_quantity());
					hero_remove_item(h, k);
#endif
				} else if (id && g_items[id].is_food())
					hero_remove_item(h, k);
			}
		}
	}
	if (n < 0xd9)
		stash_set(lst, n, -1, 0);
	Stash_dtor(lst, 3);
}

/* Ruestungen in Ablage 0x35 */
void group_confiscate_armor(void far *ctx)
{
	signed short i;
	signed short n;
	struct hero far *h;
	void far *lst;
	register signed short k;
	register signed short id;

	lst = Stash_ctor(0, 0x35, 1);
	i = n = 0;
	for (h = g_group->heroes[0]; i <= 6; h = g_group->get_hero(++i)) {
		if (h) {
			for (k = 0; k < 31; k++) {
				id = h->inv[k].item_id;
				if (id && g_items[id].is_armor() && is_in_word_array(id, g_never_confiscated) == -1) {
#if FIX_0212
					/* Fix #212: naechsten Platz der Ablage benutzen (n erhoehen) - die Gegenstaende bleiben erhalten */
					if (n < 0xd9) {
						stash_set(lst, n++, id, h->inv[k].get_quantity());
						hero_remove_item(h, k);
					}
#else
					stash_set(lst, n, id, h->inv[k].get_quantity());
					hero_remove_item(h, k);
#endif
				}
			}
		}
	}
	if (n < 0xd9)
		stash_set(lst, n, -1, 0);
	Stash_dtor(lst, 3);
}

/* Waffen in Ablage 0x36 */
void group_confiscate_weapons(void far *ctx)
{
	signed short i;
	signed short n;
	struct hero far *h;
	void far *lst;
	register signed short k;
	register signed short id;

	lst = Stash_ctor(0, 0x36, 1);
	i = n = 0;
	for (h = g_group->heroes[0]; i <= 6; h = g_group->get_hero(++i)) {
		if (h) {
			for (k = 0; k < 31; k++) {
				id = h->inv[k].item_id;
				if (id && g_items[id].is_weapon() && is_in_word_array(id, g_never_confiscated) == -1) {
#if FIX_0212
					/* Fix #212: naechsten Platz der Ablage benutzen (n erhoehen) - die Gegenstaende bleiben erhalten */
					if (n < 0xd9) {
						stash_set(lst, n++, id, h->inv[k].get_quantity());
						hero_remove_item(h, k);
					}
#else
					stash_set(lst, n, id, h->inv[k].get_quantity());
					hero_remove_item(h, k);
#endif
				}
			}
		}
	}
	if (n < 0xd9)
		stash_set(lst, n, -1, 0);
	Stash_dtor(lst, 3);
}

/* alle uebrigen Gegenstaende in Ablage 0x33 */
void group_confiscate_rest(void far *ctx)
{
	signed short i;
	signed short n;
	struct hero far *h;
	void far *lst;
	register signed short k;
	register signed short id;

	lst = Stash_ctor(0, 0x33, 1);
	i = n = 0;
	for (h = g_group->heroes[0]; i <= 6; h = g_group->get_hero(++i)) {
		if (h) {
			for (k = 0; k < 31; k++) {
				id = h->inv[k].item_id;
				if (id && is_in_word_array(id, g_never_confiscated) == -1) {
#if FIX_0212
					/* Fix #212: naechsten Platz der Ablage benutzen (n erhoehen) - die Gegenstaende bleiben erhalten */
					if (n < 0xd9) {
						stash_set(lst, n++, id, h->inv[k].get_quantity());
						hero_remove_item(h, k);
					}
#else
					stash_set(lst, n, id, h->inv[k].get_quantity());
					hero_remove_item(h, k);
#endif
				}
			}
		}
	}
	if (n < 0xd9)
		stash_set(lst, n, -1, 0);
	Stash_dtor(lst, 3);
}
