/* OVL200 - Overlay O200 (Segment 0x2359 in SCHWEIF.EXE): Gruppenklasse (struct group, 0x49 Bytes; g_group ds:c4c5
 * zeigt auf die aktive, g_groups[6] ds:c30f; weitere Methoden in O201/O202). Konstruktor group(): nr_heroes, money,
 * heroes[7] (memset), route_from/route_to = 1, route_pos/route_len/turn_pos/turns = 0. add_money(n): Geld hinzu, Anteil
 * money_share % fuer den NSC (group_get_npc, dessen money), danach money aller Helden mit npc_id == 0 = Gruppengeld;
 * Rueckgabe Geld. sub_money(n): Geld abziehen (bei <= 0 auf 0), Helden wie oben. money_share: Geld / nr_heroes.
 * sum_ap: Summe ap. count_check1/2/3: Anzahl Helden mit check_hero/check_hero2/check_hero3. Fuer alle Helden:
 * get_ill(ill, chance)/infect(ill, chance, except) hero_get_ill mit Wahrscheinlichkeit chance %, add_le(le)/
 * heal_full/add_le_range(lo, hi) add_hero_le (le, le_max - le, Zufall lo..hi), sub_le/sub_le_range sub_hero_le
 * (le bzw. lo..hi), sub_le_dice(dice, add, ignore_rs) sub_hero_le(W(dice) + add [- rs_bonus]), sub_le_keep(le, min_le)
 * sub_hero_le nur bis LE = min_le, add_le_max/sub_le_max/add_ae/sub_ae/add_ae_max/sub_ae_max (add_le_max/sub_le_max/
 * add_hero_ae/sub_ae_splash/add_hero_ae2/sub_ae_max), clr_hunger_thirst/clr_hunger/clr_thirst hunger/thirst = 0,
 * sub_hunger_thirst/sub_hunger_thirst_max(hunger, thirst) sub_hunger/sub_thirst (_max mit Grenze 100).
 * remove_npc(npc): NSC (npc_id != 0) aus der Gruppe entfernen (hero_pos -> Platz, g_group_sizes[Gruppe]--),
 * Heldendatei (sprintf(g_text_buf2, g_path_fmts[0], file_getname(g_file_main, g_npc_file_ids[npc_id])), _creat/
 * _write 0x5ef/close) schreiben, Struktur loeschen, heroes[Platz] = 0, compact, g_refresh = 1.
 *
 * BCC -mlarge -3 -Ot -Oi -c -Y -E..\bc\BIN\TASM.EXE OVL200.CPP   -> 3040/3040 Bytes GLEICH (18.10.2026),
 *   cobjfix laeuft. Mit zusaetzlich -Og ebenfalls GLEICH, ohne -Oi 4 Bytes zu lang.
 *   (Namensfassung mit schweif.h, Methoden an struct group: 19.10.2026 GLEICH, LINKGLEICH)
 * - C++: Konstruktor mit operator new (R07_041C mem_alloc_w, 0x49), Methoden ueber this; inline-Setter
 *   hero::set_money(long) erklaert die long-Locals [bp-0ch]/[bp-10h] (Parameterkopien, je Aufruf eine).
 *   group_get_npc(this) als freie Funktion ergibt dieselben Bytes wie der Methodenaufruf im Original.
 * - -Oi: memset(heroes, 0, sizeof(heroes)) -> rep stosw (cx = 0eh), memset(npc, 0, 0x5ef) -> rep stosw / stosb.
 * - Schleife "for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i])"; i ohne register: in Funktionen mit Aufrufen SI
 *   (Parameter, die in der Schleife gebraucht werden, DI), ohne Aufrufe CX; in sub_le_keep bekommt min_le SI, i DI.
 * - Geld rechnet O200 unsigned (group.money ist signed): "(unsigned long)(money -= n) <= 0" -> sub / mov eax /
 *   cmp eax,0 / ja; "(unsigned long)money / nr_heroes" -> movsx ebx / xor edx,edx / div ebx. "share = n * h->money_share"
 *   -> cbw / movsx eax / mov edx,n / imul edx,eax.
 * - "heroes[slot] = 0" (far-Zeiger) -> zwei mov word; g = g_groups -> mov [bp-6],ds / mov word [bp-8],0c30fh.
 * - Abschliessendes "return x;" hinterlaesst auch ohne -Og einen toten Epilog.
 * - Gleichnamige freie Funktionen (add_le_max, sub_le_max, sub_ae_max) in den Methoden mit "::" aufrufen.
 * - Originalfehler: sub_money prueft "Geld <= 0" auf einem unsigned long -> beim Abziehen von mehr als
 *   vorhanden laeuft das Geld ueber (riesiger Betrag statt 0); O120 vergleicht dasselbe Feld dagegen
 *   vorzeichenbehaftet (jl). remove_npc: ohne Treffer in g_groups wird g_group_sizes[6] erniedrigt.
 */
#include <stdio.h>
#include <string.h>
#include <io.h>
#include "schweif.h"

group::group()
{
	nr_heroes = 0;
	money = 0;
	memset(heroes, 0, sizeof(heroes));
	route_from = 1;
	route_to = 1;
	route_pos = 0;
	route_len = 0;
	turn_pos = 0;
	turns = 0;
}

unsigned long group::add_money(signed long n)
{
	signed long share;
	struct hero far *h;
	signed short i;

	if ((h = group_get_npc(this)) != 0) {
		share = n * h->money_share;
		share /= 100;
		n -= share;
		h->set_money(share + h->money);
	}
	money += n;
	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0 && !h->npc_id)
			h->set_money(money);
	}
	return money;
}

unsigned long group::sub_money(signed long n)
{
	struct hero far *h;
	signed short i;

#if FIX_0025
	/* Fix #25: mit Vorzeichen pruefen: mehr abziehen als vorhanden ergibt 0 statt ~42,9 Mio. Dukaten */
	if ((signed long)(money -= n) <= 0)
		money = 0;
#else
	if ((unsigned long)(money -= n) <= 0)	/* Originalfehler: unsigned, Unterlauf statt 0 */
		money = 0;
#endif
	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0 && !h->npc_id)
			h->set_money(money);
	}
	return money;
}

unsigned long group::money_share()
{
#if FIX_0356
	/* Fix #356: Anteil nur ueber die Helden ohne NSC rechnen */
	signed short n = nr_heroes;

	if (group_get_npc(this) != 0)
		n--;
	if (n <= 0)
		n = 1;
	return (unsigned long)money / n;
#else
	return (unsigned long)money / nr_heroes;
#endif
}

signed long group::sum_ap()
{
	struct hero far *h;
	signed long sum;
	signed short i;

	sum = 0;
	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0)
			sum += h->ap;
	}
	return sum;
}

signed short group::count_check1()
{
	struct hero far *h;
	signed short n;
	signed short i;

	n = 0;
	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0 && check_hero(h))
			n++;
	}
	return n;
}

signed short group::count_check2()
{
	struct hero far *h;
	signed short n;
	signed short i;

	n = 0;
	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0 && check_hero2(h))
			n++;
	}
	return n;
}

signed short group::count_check3()
{
	struct hero far *h;
	signed short n;
	signed short i;

	n = 0;
	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0 && check_hero3(h))
			n++;
	}
	return n;
}

void group::get_ill(signed short ill, signed short chance)
{
	struct hero far *h;
	signed short i;

	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0 && random_schweif(100) <= chance)
			hero_get_ill(h, ill);
	}
}

void group::infect(signed short ill, signed short chance, struct hero far *except)
{
	struct hero far *h;
	signed short i;

	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0 && h != except && random_schweif(100) <= chance)
			hero_get_ill(h, ill);
	}
}

void group::add_le(signed short le)
{
	struct hero far *h;
	signed short i;

	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0)
			add_hero_le(h, le);
	}
}

void group::heal_full()
{
	struct hero far *h;
	signed short i;

	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0)
			add_hero_le(h, h->le_max - h->le);
	}
}

void group::add_le_range(signed short lo, signed short hi)
{
	struct hero far *h;
	signed short i;

	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0)
			add_hero_le(h, lo + random_schweif(hi - lo + 1) - 1);
	}
}

void group::sub_le(signed short le)
{
	struct hero far *h;
	signed short i;

	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0)
			sub_hero_le(h, le);
	}
}

void group::sub_le_range(signed short lo, signed short hi)
{
	struct hero far *h;
	signed short i;

	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0)
			sub_hero_le(h, lo + random_schweif(hi - lo + 1) - 1);
	}
}

void group::sub_le_dice(signed char dice, signed char add, signed short ignore_rs)
{
	struct hero far *h;
	signed short i;

	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0) {
			if (ignore_rs == 0)
				sub_hero_le(h, random_schweif(dice) + add - h->rs_bonus);
			else
				sub_hero_le(h, random_schweif(dice) + add);
		}
	}
}

void group::sub_le_keep(signed short le, signed short min_le)
{
	struct hero far *h;
	signed short cur;
	signed short i;

	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0 && (cur = h->le) > min_le) {
			if (h->le >= le + min_le)
				sub_hero_le(h, le);
			else
				sub_hero_le(h, cur - min_le);
		}
	}
}

void group::add_le_max(signed short v)
{
	struct hero far *h;
	signed short i;

	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0)
			::add_le_max(h, v);
	}
}

void group::sub_le_max(signed short v)
{
	struct hero far *h;
	signed short i;

	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0)
			::sub_le_max(h, v);
	}
}

void group::add_ae(signed short v)
{
	struct hero far *h;
	signed short i;

	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0)
			add_hero_ae(h, v);
	}
}

void group::sub_ae(signed short v)
{
	struct hero far *h;
	signed short i;

	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0)
			sub_ae_splash(h, v);
	}
}

void group::add_ae_max(signed short v)
{
	struct hero far *h;
	signed short i;

	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0)
			add_hero_ae2(h, v);
	}
}

void group::sub_ae_max(signed short v)
{
	struct hero far *h;
	signed short i;

	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0)
			::sub_ae_max(h, v);
	}
}

void group::clr_hunger_thirst()
{
	struct hero far *h;
	signed short i;

	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0) {
			h->hunger = 0;
			h->thirst = 0;
		}
	}
}

void group::sub_hunger_thirst(signed short hunger, signed short thirst)
{
	struct hero far *h;
	signed short i;

	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0) {
			sub_hunger(h, hunger);
			sub_thirst(h, thirst);
		}
	}
}

void group::sub_hunger_thirst_max(signed short hunger, signed short thirst)
{
	struct hero far *h;
	signed short i;

	if (hunger > 100)
		hunger = 100;
	if (thirst > 100)
		thirst = 100;
	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0) {
			sub_hunger(h, hunger);
			sub_thirst(h, thirst);
		}
	}
}

void group::clr_hunger()
{
	struct hero far *h;
	signed short i;

	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0)
			h->hunger = 0;
	}
}

void group::clr_thirst()
{
	struct hero far *h;
	signed short i;

	for (i = 0, h = heroes[0]; i <= 6; h = heroes[++i]) {
		if (h != 0)
			h->thirst = 0;
	}
}

void group::remove_npc(struct hero far *npc)
{
	signed short slot;
	signed short fd;
	struct group far *g;
	signed short i;

	if (npc->npc_id != 0) {
		for (i = 0, g = g_groups; i < NR_GROUPS; i++, g++) {
			if (g == this)
				break;
		}
		slot = hero_pos(npc);
		nr_heroes--;
#if FIX_0026
		/* Fix #26: nur eine gefundene Gruppe verkleinern (nicht g_group_sizes[6]) */
		if (i < NR_GROUPS)
			g_group_sizes[i]--;
		npc->group_no = 0;
#else
		g_group_sizes[i]--;
		npc->group_no = 0;
#endif
		npc->group_pos = 0;
		sprintf(g_text_buf2, g_path_fmts[0], file_getname(g_file_main, g_npc_file_ids[npc->npc_id]));
		fd = _creat(g_text_buf2, 0);
		_write(fd, npc, sizeof(struct hero));
		close(fd);
		memset(npc, 0, sizeof(struct hero));
		heroes[slot] = 0;
		compact();
		g_refresh = 1;
	}
}
