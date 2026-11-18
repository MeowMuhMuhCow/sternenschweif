/* OVL140 - Overlay O140 (Segment 0x2227 in SCHWEIF.EXE): Kampfaufstellung (Kampfstruktur wie O021: Kampfnummer
 * +1ec, Gegner +eeb (20 x 5 Bytes: Typ, x, y, Richtung, Wert), Helden +f4f (7 x 4 Bytes), Felder +f6b/+fa7..+fab,
 * Karte *(+fad) mit 24 x 25 Feldern ab +0x15). fight_setup_env(f): Sonderkaempfe 0x493 (O141_0000), 0x494 (nichts),
 * 0x495 (O021_0000); sonst Puffer aus Datei ds:6875 lesen (0x4000 Bytes), Datei ds:687b loeschen, Datensatz
 * (Gruppe->f22 - 1) * 50 aus Datei 0x1b, Bilddaten 0x1fd/0x1fb und sechs Teile 0x56 aus 0x1fc, Kampfdefinition
 * (0x45 Bytes, ds:3851[nr]) kopieren, Eintrittsseite fight_entry_side, Karte aufbauen (fight_make_field: O139_0213/0077/0A16),
 * Helden aufstellen (fight_place_hero, Held mit +9b auf Platz 6), Gegner aufstellen (fight_place_enemy, Anzahl je Gruppe aus der
 * Definition), Helden zum naechsten Gegner drehen (fight_heroes_face_enemies), Platzhalter 0xc9/0xca loeschen, Puffer frei.
 * fight_place_hero: Heldenfeld zufaellig im 11x11-Bereich um den Eintrittspunkt ds:6851[seite] (frei, Abstand < 5,
 * O139_0447), Blickrichtung (seite + 1) % 4 bzw. zufaellig bei Seite 4. fight_nearest_hero/fight_nearest_enemy: naechstes Feld mit
 * Held (0xc9) bzw. Gegner (0xca), Rueckgabe Abstand. fight_place_enemy: Gegnerfeld zufaellig, Abstand zum naechsten
 * Helden im Bereich ds:684b[k] (nach 50 Versuchen egal), Richtung zum Helden; Zweifeldgegner (flag) brauchen
 * das Nachbarfeld ds:650c[richtung]. fight_entry_side: freie Felder nach Norden/Sueden/Osten/Westen (O139_0000) zaehlen ->
 * Eintrittsseite ds:caba (0..4).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -rd -c -Y OVL140.CPP   -> 4536/4536 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h, Welle N7: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * - -rd (nur register-Variablen in Registern) war der Schluessel: fd (Block-Local um open/read/close) liegt sonst
 *   in DI; mit -rd bleibt es bei [bp-14h] hinter den -Og-Zwischenwerten, die register-Variablen j/k bekommen SI/DI.
 * - -Oi fuer memset (rep stosw) blockiert DI auf Funktionsebene -> "register k" im Block der Heldenschleife.
 * - Argument ok von fight_place_enemy als Variable mit if/else (zwei Stores nach [bp-0ch]), nicht ?: (xor ax / mov ax,1).
 * - -Og: (int)g_caba in [bp-0ch] und g_caba * 2 in DI/[bp-0eh], *px/*py als char- und int-Zwischenwerte
 *   ([bp-13h]/[bp-16h], [bp-0fh]/[bp-12h]), n * 5 / f + n * 5 / i * 25 in [bp-0eh]/[bp-10h]/[bp-12h].
 * - "struct fdef d = g_3851[f->f1ec];" -> SCOPY@ (cx = 0x45); "done = n1 = 0" -> [bp-2] zuerst, dann SI.
 * - RTL: _open (-> __open), read/close/unlink.
 * - Phase B (19.10.2026): eigene Daten im Quelltext. ds:6668..684b sind sieben Kampfdefinitionen (struct fight_def)
 *   g_fight_defs_ab fuer die Kampfnummern 0xab..0xb1: fight_setup_env liest g_fight_defs_ab[f->id - 0xab]; BCC faltet
 *   6668 - 0xab * 0x45 zur Basis ds:3851 (die ASM-Fassung nannte sie g_fight_defs, dort liegen unbenutzte Nullen von
 *   O032). Satz 0xab ist die Zufallsbegegnung (g_encounters = .groups von O072/O129, g_fight_ab_loot = .loot[0] von O131, O143
 *   loescht ihn).
 * Originalfehler: Feldgrenzen uneinheitlich - Zeilenschleifen 0..23, fight_place_hero erlaubt y bis 25, fight_place_enemy prueft
 *   y + dy > 24 und x + dx > 25 (Spalte 25 liegt schon in der naechsten Zeile). memset(f->ff6b) loescht 15
 *   Woerter, die Schleife danach kopiert 20. fight_setup_env: Merker +f52 (Bit +bc.1) wird mit i statt k indiziert -
 *   beim Helden mit +9b (Platz 6) landet er auf dem falschen Platz.
 */
#include <io.h>
#include <mem.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:6668..6882, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
struct fight_def g_fight_defs_ab[7] = {	/* Kaempfe 0xab..0xb1 (Index Kampfnummer - 0xab); 0xab: Zufallsbegegnung,
					   Gruppen von O072/O129, nach dem Kampf geloescht (O143) */
	{ 0, { 0, 0 }, { 0 },
	  { 0 }, 0, 0, 0 },	/* 0xab */
	{ 0, { 0, 0 }, { { 92, 1, 2, 0 } },
	  { 0 }, 0, 0, 0 },	/* 0xac */
	{ 0, { 0, 0 }, { { 102, 1, 1, 0 }, { 92, 1, 1, 0 }, { 63, 1, 2, 0 }, { 85, 1, 2, 0 }, { 88, 1, 0, 0 } },
	  { 0 }, 0, 0, 0 },	/* 0xad */
	{ 0, { 0, 0 }, { { 65, 5, 1, 0 }, { 45, 5, 2, 0 }, { 2, 5, 0, 0 }, { 3, 5, 1, 0 } },
	  { 0 }, 0, 0, 0 },	/* 0xae */
	{ 0, { 1, 0 }, { { 65, 5, 1, 0 }, { 45, 1, 2, 0 } },
	  { ITEM_OBSIDIAN_DAGGER, ITEM_TORCH_ON }, 6, 0, 0 },	/* 0xaf */
	{ 0, { 2, 0 }, { { 7, 1, 1, 0 }, { 11, 6, 2, 0 } },
	  { 0 }, 0, 0, 0 },	/* 0xb0 */
	{ 0, { 3, 0 }, { { 12, 3, 2, 0 }, { 13, 1, 0, 0 }, { 14, 1, 2, 0 } },
	  { ITEM_ARAX_POISON }, 45, 0, 0 } 	/* 0xb1 */
};
struct range8 g_fig_ranges[3] = { { 3, 6 }, { 6, 12 }, { 13, 99 } };
struct cxy g_fig_entry_pos[5] = { { 10, 17 }, { 18, 9 }, { 10, 2 }, { 3, 9 }, { 10, 10 } };
struct cxy g_env_cell_offs[12] = {
	{ 0, 3 },
	{ 1, 3 },
	{ 2, 3 },
	{ 3, 3 },
	{ 3, 2 },
	{ 3, 1 },
	{ 3, 0 },
	{ 2, 0 },
	{ 1, 0 },
	{ 0, 0 },
	{ 0, 1 },
	{ 0, 2 }
};
signed short g_fight_env_build = 0;
/* Phase B: eigenes BSS (ds:caba..cb00) in Adressreihenfolge */
signed char g_env_side;
signed short far *g_env_kind;
signed short far *g_env_side_kind;
signed char far *g_env_tab0;
signed char far *g_env_tab1;
signed char far *g_env_tab2;
signed char far *g_env_tab3;
signed char far *g_env_tab4;
struct cxy far *g_env_cell_pos;
signed char far *g_env_dirtabs[5];
unsigned char (far *g_env_wall_tab)[12];
signed char (far *g_env_neighbours)[9];
signed char g_env_wall_flag;
unsigned char far *g_env_cells;
signed char far *g_env_kinds;

void fight_setup_env(Fight far *f)
{
	signed short i = 0;
	signed short n;
	signed short size;
	struct hero far *h;
	signed short ok;
	struct fight_def d;
	register signed short j;

	if (f->id == FIGHT_ID_SPECIAL) {
		sfight_setup(f);
		return;
	}
	if (f->id == FIGHT_ID_EMPTY)
		return;
	if (f->id == FIGHT_ID_SPECIAL2) {
		sfight2_setup(f);
		return;
	}
	g_env_cells = (unsigned char far *)mem_alloc(0x4000L);
	{
		signed short fd;

		fd = _open("t.$$$", 0x8001);
		read(fd, g_env_cells, 0x4000);
		close(fd);
	}
	unlink("t.$$$");
	g_env_kinds = (signed char far *)mem_alloc(50L);
	arc_open(g_file_3, 0x1b);
	arc_seek(g_file_3, (signed long)(g_group->dungeon * 50 - 50), 0);
	arc_read(g_file_3, g_env_kinds, 50L);
	arc_close(g_file_3);
	g_env_kind = (signed short far *)mem_alloc(0x56L);
	g_env_side_kind = (signed short far *)mem_alloc(0x56L);
	g_env_wall_tab = (unsigned char (far *)[12])mem_alloc(0xc0L);
	arc_load(g_file_3, 0x1fd, g_env_wall_tab);
	g_env_neighbours = (signed char (far *)[9])mem_alloc(0x5aL);
	arc_load(g_file_3, 0x1fb, g_env_neighbours);
	size = 0x56;
	g_env_tab0 = (signed char far *)mem_alloc(size);
	g_env_tab1 = (signed char far *)mem_alloc(size);
	g_env_tab2 = (signed char far *)mem_alloc(size);
	g_env_tab3 = (signed char far *)mem_alloc(size);
	g_env_tab4 = (signed char far *)mem_alloc(size);
	g_env_cell_pos = (struct cxy far *)mem_alloc(size);
	arc_open(g_file_3, 0x1fc);
	arc_read(g_file_3, g_env_tab0, size);
	arc_read(g_file_3, g_env_tab1, size);
	arc_read(g_file_3, g_env_tab2, size);
	arc_read(g_file_3, g_env_tab3, size);
	arc_read(g_file_3, g_env_tab4, size);
	arc_read(g_file_3, g_env_cell_pos, size);
	arc_close(g_file_3);
	g_env_dirtabs[0] = g_env_tab2;
	g_env_dirtabs[1] = g_env_tab3;
	g_env_dirtabs[2] = g_env_tab0;
	g_env_dirtabs[3] = g_env_tab1;
	g_env_dirtabs[4] = g_env_tab4;
	d = g_fight_defs_ab[f->id - 0xab];
	fight_entry_side(f);
	f->field->type = 0;
	if (g_group->dungeon == 4)
		f->field->type = 2;
	fight_make_field(f);
	f->intro_done = d.intro_done;
	memset(f->heroes, 0, sizeof(f->heroes));
	for (j = 0; j < 7; j++)
		f->heroes[j].x = -1;
	h = get_first_hero(g_group);
	do {
		register signed short k;

		if (h->npc_id)
			k = 6;
		else
			k = i;
		fight_place_hero(f, &f->heroes[k].x, &f->heroes[k].y, &f->heroes[k].viewdir);
#if FIX_0098
		/* Fix #98: Schlafmerker am Platz k des Helden (NSC: 6) */
		if ((signed char)h->asleep)
			f->heroes[k].asleep = 1;
#else
		if ((signed char)h->asleep)
			f->heroes[i].asleep = 1;
#endif
		h = get_next_hero(g_group, h);
		i++;
	} while (get_first_hero(g_group) != h && get_first_hero(g_group) != 0);
	n = 0;
	memset(f->monsters, 0, sizeof(f->monsters));
	for (i = 0; i < 5; i++) {
		if (d.groups[i].count != 0) {
			for (j = 0; j < d.groups[i].count; j++) {
				f->monsters[n].mon_id = d.groups[i].mon_id;
				f->monsters[n].round_appear = d.groups[i].round_appear;
				if (is_in_byte_array2(f->monsters[n].mon_id, g_twofield_mons) == -1)
					ok = 0;
				else
					ok = 1;
				fight_place_enemy(f, d.groups[i].range, &f->monsters[n].x, &f->monsters[n].y, &f->monsters[n].viewdir, ok);
				n++;
			}
		}
	}
	fight_heroes_face_enemies(f);
	for (i = 0; i < 24; i++) {
		for (j = 0; j < 25; j++) {
			if (f->field->cells[i][j] == 0xc9 || f->field->cells[i][j] == 0xca)
				f->field->cells[i][j] = 0;
		}
	}
	memset(f->loot, 0, 30);		/* Originalfehler: nur 15 Eintraege, kopiert werden 20 */
	for (i = 0; i < 20; i++)
		f->loot[i] = d.loot[i];
	f->ducats = d.ducats;
	f->silver = d.silver;
	f->heller = d.heller;
	mem_free(g_env_cell_pos);
	mem_free(g_env_tab4);
	mem_free(g_env_tab3);
	mem_free(g_env_tab2);
	mem_free(g_env_tab1);
	mem_free(g_env_tab0);
	mem_free(g_env_neighbours);
	mem_free(g_env_wall_tab);
	mem_free(g_env_side_kind);
	mem_free(g_env_kind);
	mem_free(g_env_kinds);
	mem_free(g_env_cells);
}

void fight_place_hero(Fight far *f, signed char far *px, signed char far *py, signed char far *pdir)
{
	signed short x0;
	signed short x1;
	signed short y0;
	signed short y1;
	signed char save;

	x0 = g_fig_entry_pos[g_env_side].x >= 5 ? g_fig_entry_pos[g_env_side].x - 5 : 0;
	x1 = g_fig_entry_pos[g_env_side].x <= 19 ? g_fig_entry_pos[g_env_side].x + 5 : 24;
	y0 = g_fig_entry_pos[g_env_side].y >= 5 ? g_fig_entry_pos[g_env_side].y - 5 : 0;
#if FIX_0098
	/* Fix #98: Zeilen nur bis 23 (Gitter 24 Zeilen) */
	y1 = g_fig_entry_pos[g_env_side].y <= 18 ? g_fig_entry_pos[g_env_side].y + 5 : 23;
#else
	y1 = g_fig_entry_pos[g_env_side].y <= 20 ? g_fig_entry_pos[g_env_side].y + 5 : 24;
#endif
	save = f->field->cells[g_fig_entry_pos[g_env_side].y][g_fig_entry_pos[g_env_side].x];
	f->field->cells[g_fig_entry_pos[g_env_side].y][g_fig_entry_pos[g_env_side].x] = 0xcb;
	do {
		*px = random_interval(x0, x1);
		*py = random_interval(y0, y1);
	} while (f->field->cells[*py][*px] != 0 || manhattan_dist(*px, *py, g_fig_entry_pos[g_env_side].x, g_fig_entry_pos[g_env_side].y) >= 5 ||
		!fight_path_to_start(f, *px, *py));
	f->field->cells[g_fig_entry_pos[g_env_side].y][g_fig_entry_pos[g_env_side].x] = save;
	f->field->cells[*py][*px] = 0xc9;
	*pdir = 0;
	if (g_env_side == 4) {
		*pdir = random_schweif(4) - 1;
		return;
	}
	*pdir = (g_env_side + 1) % 4;
}

signed short fight_nearest_hero(Fight far *f, signed char x, signed char y, signed char far *px, signed char far *py)
{
	signed short col;
	signed short row;
	register signed short best;
	register signed short d;

	best = 99;
	for (row = 0; row < 24; row++) {
		for (col = 0; col < 25; col++) {
			if (f->field->cells[row][col] == 0xc9) {
				if ((d = manhattan_dist(x, y, col, row)) <= best && (d < best || x == col || y == row)) {
					best = d;
					*px = col;
					*py = row;
				}
			}
		}
	}
	return best;
}

void fight_place_enemy(Fight far *f, signed short k, signed char far *px, signed char far *py, signed char far *pdir,
	signed short flag)
{
	signed short found = 0;
	signed short tries = 0;
	signed short dist;
	signed char tx;
	signed char ty;
	signed char save;
	signed short free;

	save = f->field->cells[g_fig_entry_pos[g_env_side].y][g_fig_entry_pos[g_env_side].x];
	f->field->cells[g_fig_entry_pos[g_env_side].y][g_fig_entry_pos[g_env_side].x] = 0xcb;
	do {
		do {
			*px = random_schweif(24) - 1;
			*py = random_schweif(24) - 1;
		} while (f->field->cells[*py][*px] != 0);
		free = fight_path_to_start(f, *px, *py);
		tries++;
		dist = fight_nearest_hero(f, *px, *py, &tx, &ty);
		if (free && (tries > 50 || (g_fig_ranges[k].lo <= dist && g_fig_ranges[k].hi >= dist))) {
			found = 1;
			if (*px == tx) {
				if (*py > ty)
					*pdir = 1;
				else
					*pdir = 3;
			} else if (*py == ty) {
				if (*px > tx)
					*pdir = 2;
				else
					*pdir = 0;
			} else if (*px > tx) {
				if (*py > ty) {
					if (*py - ty > *px - tx)
						*pdir = 1;
					else
						*pdir = 2;
				} else {
					if (ty - *py > *px - tx)
						*pdir = 3;
					else
						*pdir = 2;
				}
			} else {
				if (*py > ty) {
					if (*py - ty > tx - *px)
						*pdir = 1;
					else
						*pdir = 0;
				} else {
					if (ty - *py > tx - *px)
						*pdir = 3;
					else
						*pdir = 0;
				}
			}
			if (flag) {
#if FIX_0098
				/* Fix #98: Feldgrenzen 0..23/0..24 und vor dem Zugriff pruefen */
				if (*py + g_fig_tail_dxy[*pdir].dy < 0 || *py + g_fig_tail_dxy[*pdir].dy > 23 ||
					*px + g_fig_tail_dxy[*pdir].dx < 0 || *px + g_fig_tail_dxy[*pdir].dx > 24 ||
					f->field->cells[*py + g_fig_tail_dxy[*pdir].dy][*px + g_fig_tail_dxy[*pdir].dx] != 0)
#else
				if (f->field->cells[*py + g_fig_tail_dxy[*pdir].dy][*px + g_fig_tail_dxy[*pdir].dx] != 0 ||
					*py + g_fig_tail_dxy[*pdir].dy < 0 || *py + g_fig_tail_dxy[*pdir].dy > 24 ||
					*px + g_fig_tail_dxy[*pdir].dx < 0 || *px + g_fig_tail_dxy[*pdir].dx > 25)
#endif
					found = 0;
				else
					f->field->cells[*py + g_fig_tail_dxy[*pdir].dy][*px + g_fig_tail_dxy[*pdir].dx] = 0xca;
			}
		}
	} while (!found);
	f->field->cells[g_fig_entry_pos[g_env_side].y][g_fig_entry_pos[g_env_side].x] = save;
	f->field->cells[*py][*px] = 0xca;
}

signed short fight_nearest_enemy(Fight far *f, signed char x, signed char y, signed char far *px, signed char far *py)
{
	signed short col;
	signed short row;
	register signed short best;
	register signed short d;

	best = 99;
	for (row = 0; row < 24; row++) {
		for (col = 0; col < 25; col++) {
			if (f->field->cells[row][col] == 0xca) {
				if ((d = manhattan_dist(x, y, col, row)) <= best && (d < best || x == col || y == row)) {
					best = d;
					*px = col;
					*py = row;
				}
			}
		}
	}
	return best;
}

void fight_heroes_face_enemies(Fight far *f)
{
	signed char tx;
	signed char ty;
	signed char x;
	signed char y;
	signed char dir;
	register signed short i;

	for (i = 0; i < 7; i++) {
		if (f->heroes[i].x != -1) {
			x = f->heroes[i].x;
			y = f->heroes[i].y;
			fight_nearest_enemy(f, x, y, &tx, &ty);
			if (tx == x) {
				if (ty < y)
					dir = 1;
				else
					dir = 3;
			} else if (ty == y) {
				if (tx < x)
					dir = 2;
				else
					dir = 0;
			} else if (tx < x) {
				if (ty < y) {
					if (y - ty > x - tx)
						dir = 1;
					else
						dir = 2;
				} else {
					if (ty - y > x - tx)
						dir = 3;
					else
						dir = 2;
				}
			} else {
				if (ty < y) {
					if (y - ty > tx - x)
						dir = 1;
					else
						dir = 0;
				} else {
					if (ty - y > tx - x)
						dir = 3;
					else
						dir = 0;
				}
			}
			f->heroes[i].viewdir = dir;
		}
	}
}

void fight_make_field(Fight far *f)
{
	g_fight_env_build = 1;
	fight_load_grid(f);
	fight_env_scan(f);
	fight_build_field(f);
	g_fight_env_build = 0;
}

void fight_entry_side(Fight far *f)
{
	signed short n1;
	signed short n3;
	signed short n2;
	signed short n4;
	signed short y;
	signed short dir;
	signed char c;
	register signed short done;
	register signed short x;

	done = n1 = 0;
	y = g_group->y;
	do {
		y--;
		c = fight_env_cell(f, g_group->x, y);
		if (y < 0 || c == 2 || c == 5 || c == 8 || c == 7 || !c)
			done = 1;
		else
			n1++;
	} while (!done);
	done = n2 = 0;
	y = g_group->y;
	do {
		y++;
		c = fight_env_cell(f, g_group->x, y);
		if (y == 64 || c == 2 || c == 5 || c == 8 || c == 7 || !c)
			done = 1;
		else
			n2++;
	} while (!done);
	done = n3 = 0;
	x = g_group->x;
	do {
		x++;
		c = fight_env_cell(f, x, g_group->y);
		if (x == g_map_w || c == 2 || c == 5 || c == 8 || c == 7 || !c)
			done = 1;
		else
			n3++;
	} while (!done);
	done = n4 = 0;
	x = g_group->x;
	do {
		x--;
		c = fight_env_cell(f, x, g_group->y);
		if (x < 0 || c == 2 || c == 5 || c == 8 || c == 7 || !c)
			done = 1;
		else
			n4++;
	} while (!done);
	if (n1 - n2 > 2) {
		if (n3 - n4 > 3)
			dir = 3;
		else if (n4 - n3 > 3)
			dir = 1;
		else
			dir = 2;
	} else if (n2 - n1 > 2) {
		if (n3 - n4 > 3)
			dir = 3;
		else if (n4 - n3 > 3)
			dir = 1;
		else
			dir = 0;
	} else {
		if (n3 - n4 > 3)
			dir = 3;
		else if (n4 - n3 > 3)
			dir = 1;
		else
			dir = 4;
	}
	g_env_side = dir;
}
