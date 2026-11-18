/* OVL022 - Overlay O022 (Segment 0x1f74 in SCHWEIF.EXE): Aufbau eines Kampfes (Kampf-Objekt p wie OVL018/OVL021,
 * 0xfb1 Bytes; Kampffeld p->ffad = 0x15 Kopfbytes + 24 Zeilen zu 25 Bytes, 0x26d Bytes).
 * fight_setup(p): Einstieg - Fenster O190_0A4F, Kartenspeicher 0x26d, Datei 0x49 (ds:35fe) komplett nach p->f1ff,
 *   Kampfdaten O023_0000/O023_0341 (id <= 0xaa) bzw. O140_0000, Gegnerliste ab ds:c68b leeren, Zaehler/Felder
 *   zuruecksetzen (ds:8bdb = ds:347d), dann fight_make_walkmap, O023_01AF, fight_draw_bg, fig_add_field_objs, fig_init_enemies, O148_01CE/021C, fig_init_heroes.
 * fill_enemy_sheet(p, idx, typ, x): Gegnerdatensatz typ (0x30 Bytes aus Datei 0x48) nach ds:c4f7[idx] (0x49 Bytes) umsetzen
 *   (Werte ueber O034_1A0D, Sonderfall Typ 0x33 LE >= 500, ds:395f 4/5). fig_add_enemy(p, e, x, y): Gegner e als Figur
 *   auf das Feld setzen (Sonderkaempfe 0x493 O141_034E, 0x494 nichts, 0x495 O021_01C8), Animationsobjekt
 *   O016_0000 nach p->f213[p->f293++], bei Doppelfeldgegnern (O153_0CD2) zweite Figur ueber ds:650c.
 * fig_init_enemies(p): alle 20 Gegnerplaetze loesen, Gegner aus p->e[20] (5 Bytes) laden, Klaenge O003_0FAD je Gegnerart
 *   (O192_*), Figuren O147_0933/O136_0268. fig_init_heroes(p): Helden auf ihre Startplaetze (p->ff4f), Figuren, Klaenge.
 * fight_make_walkmap(p): Begehbarkeitskarte ds:c99b (25x24) aus dem Kampffeld (O153_0904, Waende 0x59/0x5a je 7 Felder).
 * fig_add_field_objs(p): Feldobjekte (Bilder ds:c953 per Nvf/Bmp, Animation 0x5d mit O112_0101), fight_draw_bg(p): Hintergrund-
 *   bilder isometrisch in p->f0 zeichnen (Bildschirmpuffer ueber ds:c4db, R02_0EDA bzw. R38_0004).
 * fight_intro_text(p): bei ds:395f == 3 Text txt[0x65] ausgeben.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL022.CPP   -> 5842/5842 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (_g_347d -> _s_3343+0x13a). Ohne -Og 36 Bytes zu lang.
 *   (Namensfassung mit schweif.h, Welle N7: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * - C++ wegen inline-Settern der Gegnerstruktur (Welle P: mov al / mov [bp-x],al / les / mov al,[bp-x] / mov):
 *   jede Zuweisung in fill_enemy_sheet ist ein eigener Setter mit Parameterkopie; setat(i, v) { at[i].a = at[i].b = v; }
 *   (Kopie von i landet in DI, v in [bp-9]; Kette speichert +5 vor +4).
 * - fight_setup: "p->f1ff = R07_0002(len = R06_0c4d(..))" eingebettet (push eax direkt nach mov [bp-4],eax).
 * - "g_8bdb = g_347d" (extern char[]) -> mov [8bdd],ds / mov [8bdb],347dh; "(char near *)0x347d" faltet zu
 *   mov dword (falsch).
 * - "if (!r) ;" -> or di,di / jnz auf den naechsten Befehl (mit "if (r) ;" kommt jz).
 * - p->f23/p->f27 als signed long (= 0 -> mov dword), Bild als (unsigned long) nach f27 (mov eax).
 * - Tabellenzugriffe "*(g_c99b + y * 25 + x)" und "*(p->ffad->m[0] + y * 25 + x)" (les / add / add, +0x15 in
 *   der Verschiebung).
 * Originalfehler/Auffaelliges: fig_init_enemies und fig_init_heroes werten den Rueckgabewert r von O003_0FAD nur in einem leeren
 *   "if (!r) ;" aus (in fig_init_enemies kann r uninitialisiert sein); fig_init_enemies ruft O003_0BCA fuer jeden Gegner erneut auf
 *   (gehoert wohl vor die Schleife); fig_init_heroes "if (h->f93 != -1) h->f93 = -1;" (Test ueberfluessig).
 */
#include <mem.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:11aa..1222, dsana) - initialisierte Daten in Adressreihenfolge */
signed short g_sfight_pics[29] = {
	8227, 8254, 8255, 8351, 9074, 16520, 16490, 16527, 16479, 16518, 16519, 24586, 24587, 24585, 24581, 24582,
	24583, 24584, 24601, 24602, 24603, 24604, 24578, 24579, 24580, 24605, 24606, 24816, -1
};
char g_sfight_pic_states[8] = "";
signed short g_sfight2_pics[24] = {
	8256, 8257, 8258, 8259, 16490, 16513, -15373, -15372, -15371, -15370, -15369, -15368, -15367, -15366, -15365, -15364,
	-15363, -15362, -15361, -15360, -15359, -15358, -15357, -1
};
char g_sfight2_pic_states[6] = "";

/* Vorlage als long beschreiben (Original: Felder +0x23/+0x27 als long -> mov dword / mov eax) */
#define TMPL_ANIM_L	(*(signed long far *)&p->tmpl.anim)
#define TMPL_GFX_L	(*(unsigned long far *)&p->tmpl.gfx)

void fight_setup(Fight far *p)
{
	signed long len;
	register signed short i;

	p->old_dlg_mode = dlg_set16(g_dialog, 3);
	p->field = (struct fight_field far *)mem_alloc(0x26dL);
	arc_open(g_file_3, 0x49);
	p->f1ff = mem_alloc(len = arc_size(g_file_3));
	arc_read(g_file_3, p->f1ff, len);
	arc_close(g_file_3);
	if (p->id <= FIGHT_ID_MAX_DEF) {
		fight_read_rec(p);
		fight_read_scenario(p);
	} else
		fight_setup_env(p);
	if (g_fight_max_enemies > 0) {
		memset(&p->monsters[g_fight_max_enemies], 0, (20 - g_fight_max_enemies) * 5);
		g_fight_max_enemies = 0;
	}
	g_in_fight = g_refresh = 1;
	p->round = p->nr_dropped = g_clock_update = 0;
	g_mouse_areas = g_fig_mouse_areas;
	for (i = 0; i < 30; i++)
		p->dropped[i] = 0;
	fight_make_walkmap(p);
	fight_load_bg(p);
	fight_draw_bg(p);
	fig_add_field_objs(p);
	fig_init_enemies(p);
	p->enemy_le_sum = fig_enemies_le_sum(p);
	p->enemy_ae_sum = fig_enemies_ae_sum(p);
	fig_init_heroes(p);
}

void fill_enemy_sheet(Fight far *p, signed short idx, signed char t, signed char x)
{
	struct enemy far *e;
	register signed short i;
	struct mon_rec r;

	arc_open(g_file_3, 0x48);
	arc_seek(g_file_3, t * sizeof(r), 0);
	arc_read(g_file_3, &r, sizeof(r));
	arc_close(g_file_3);
	g_mon_genders[t] = r.f2f;
	e = &g_enemies[idx];
	memset(e, 0, sizeof(*e));
	e->set_mon_id(r.mon_id);
	e->set_gfx_id(r.gfx_id);
	e->set_f03(r.f03);
	e->set_rs(r.rs);
	for (i = 0; i < 7; i++)
		e->set_attrib(i, dice_code_roll(r.attrib[i]));
	e->set_le_max(dice_code_roll(r.le));
	e->set_le(e->le_max);
	e->set_ae_max(dice_code_roll(r.ae));
	e->set_ae(e->ae_max);
	e->set_mr(dice_code_roll(r.mr));
	e->set_first_ap(r.first_ap);
	e->set_attacks(r.attacks);
	e->set_at(r.at);
	e->set_pa(r.pa);
	e->set_dam1(r.dam1);
	e->set_dam2(r.dam2);
	e->set_bp_max(r.bp);
	if (e->bp_max > 10)
		e->bp_max = 10;
	else
		;
	e->set_magic(r.magic);
	e->set_mag_id(r.mag_id);
	e->dead = 0;
	e->fighter_id = -1;
	e->set_level(r.level);
	e->set_size(r.size);
	e->set_is_animal(r.is_animal);
	e->set_round_appear(x);
	e->set_viewdir(p->monsters[idx].viewdir);
	e->set_shots(r.shots);
	e->set_shot_dam(r.shot_dam);
	e->set_throws(r.throws);
	e->set_throw_dam(r.throw_dam);
	e->set_le_flee(r.le_flee);
	e->set_f47(r.f2f);
	if (r.mon_id == 0x33 && e->le < 500) {
		e->le_max = 500;
		e->le = 500;
	}
	if (g_fight_end == 4)
		e->scared = 1;
	if (g_fight_end == 5)
		e->le_flee = 0;
}

void fig_add_enemy(Fight far *p, struct enemy far *b, signed short x, signed short y)
{
	if (p->id == FIGHT_ID_SPECIAL) {
		sfight_place_enemy(p, b);
		return;
	}
	if (p->id == FIGHT_ID_EMPTY)
		return;
	if (p->id == FIGHT_ID_SPECIAL2) {
		sfight2_place_enemy(p);
		return;
	}
	TMPL_ANIM_L = (signed long)(p->anims[p->nr_anims++] = (FIGANIM_PTR)FigAnim_ctor(0, &p->seq, 1));
	p->tmpl.cbx = p->tmpl.offsetx = x;
	p->tmpl.cby = p->tmpl.offsety = y;
	p->tmpl.obj_id = *(g_fight_walkmap + y * FIG_MAP_PITCH + x);
	if (enemy_is_twofielded(b))
		p->tmpl.twofielded = ++p->nr_twofield;
	else
		p->tmpl.twofielded = FIG_TWOFIELD_NONE;
	p->tmpl.kind = 1;
	p->tmpl.figure = fig_enemy_pic(b);
	TMPL_GFX_L = 0;
	p->tmpl.z = 0x63;
	p->tmpl.visible = !b->round_appear ? 1 : 0;
	b->set_fighter_id(fig_list_add(&p->list, -1));
	if (enemy_is_twofielded(b)) {
		p->tmpl.cbx = p->tmpl.offsetx = x + g_fig_tail_dxy[b->viewdir].dx;
		p->tmpl.cby = p->tmpl.offsety = y + g_fig_tail_dxy[b->viewdir].dy;
		p->tmpl.obj_id = *(g_fight_walkmap + p->tmpl.cby * FIG_MAP_PITCH + p->tmpl.cbx);
		p->tmpl.kind = 1;
		TMPL_ANIM_L = 0;
		p->tmpl.z = 10;
		p->tmpl.twofielded = p->nr_twofield + 20;
		fig_list_add(&p->list, -1);
	}
}

void fig_init_enemies(Fight far *p)
{
	signed short i;
	signed short a;
	signed short b;
	struct enemy far *e;
	void far *t;
	signed short r;

	for (i = 0; i < 20; i++) {
		e = &g_enemies[i];
		if (e->fighter_id != -1) {
			fig_list_remove(&p->list, e->fighter_id, 0);
			e->fighter_id = -1;
		}
		e->dead = 1;
	}
	g_nr_enemies = 0;
	for (i = 0; i < 20; i++) {
		if (p->monsters[i].mon_id != 0) {
			fill_enemy_sheet(p, i, p->monsters[i].mon_id, p->monsters[i].round_appear);
			g_nr_enemies++;
		}
	}
	t = Keylist_ctor(0, 0x28);
	for (i = 0; i < g_nr_enemies; i++) {
		e = &g_enemies[i];
		a = p->monsters[i].x;
		b = p->monsters[i].y;
		fig_add_enemy(p, e, a, b);
		if (p->id == FIGHT_ID_SPECIAL)
			load_pic_list(g_cd_cache, g_sfight_pics, g_sfight_pic_states, 1, 0);
		else if (p->id == FIGHT_ID_SPECIAL2)
			load_pic_list(g_cd_cache, g_sfight2_pics, g_sfight2_pic_states, 1, 0);
		if (e->gfx_id != 0x45)
			r = keylist_add(t, fig_enemy_pic(e), 0x2000);
		if (enemy_is_gfx_25_35_36_23(e))
			r = keylist_add(t, 0x7f, 0x4000);
		else if (enemy_is_gfx_24_18(e))
			r = keylist_add(t, 0x66, 0x4000);
		else if (enemy_is_gfx_2a(e))
			r = keylist_add(t, 0x68, 0x4000);
		else if (enemy_is_gfx_17_29(e))
			r = keylist_add(t, 0x67, 0x4000);
		else if (enemy_is_fire(e))
			r = keylist_add(t, 0x85, 0x4000);
		else if (e->gfx_id == 0x29) {
			r = keylist_add(t, 0x89, 0x4000);
			r = keylist_add(t, 0x8b, 0x4000);
		} else if (enemy_is_gfx_1c(e)) {
			r = keylist_add(t, 0x7d, 0x4000);
			r = keylist_add(t, 0x76, 0x4000);
		} else if (enemy_is_gfx_42(e)) {
			r = keylist_add(t, 0x7e, 0x4000);
			r = keylist_add(t, 0x8a, 0x4000);
		}
		if (!p->monsters[i].round_appear)
			place_obj_on_cb(p, a, b, i + 10, p->monsters[i].mon_id, p->monsters[i].viewdir);
		fig_stand_seq(p, e, &p->seq);
	}
#if FIX_0168
	/* Fix #168: leeres "if (!r) ;" auf das (evtl. uninitialisierte) r entfernt */
	keylist_load(t, g_cd_cache);
#else
	if (!r)
		;
	keylist_load(t, g_cd_cache);
#endif
	Keylist_dtor(t, 3);
}

void fig_init_heroes(Fight far *p)
{
	struct hero far *cur;
	struct hero far *first;
	signed short x;
	signed short y;
	void far *t;
	signed short k;
	signed short r;

	first = cur = get_first_hero(g_group);
	do {
		if (cur->fighter_id != -1)
			cur->fighter_id = -1;
		cur = get_next_hero(g_group, cur);
	} while (cur != first);
	t = Keylist_ctor(0, 0x0f);
	k = -1;
	do {
		k++;
		cur->action_id = FIG_ACTION_WAIT;
		cur->enemy_id = -1;
		if (cur->npc_id && p->id != FIGHT_ID_SPECIAL2) {
			x = p->heroes[6].x;
			y = p->heroes[6].y;
			cur->set_viewdir(p->heroes[6].viewdir);
			if (p->heroes[6].asleep && !(signed char)cur->dead)
				set_asleep(cur, 1);
		} else {
			x = p->heroes[k].x;
			y = p->heroes[k].y;
			cur->set_viewdir(p->heroes[k].viewdir);
			if (p->heroes[k].asleep && !(signed char)cur->dead)
				set_asleep(cur, 1);
		}
		place_obj_on_cb(p, x, y, get_hero_index(cur) + FIG_ID_HERO, cur->type, cur->viewdir);
		TMPL_ANIM_L = (signed long)(p->anims[p->nr_anims++] = (FIGANIM_PTR)FigAnim_ctor(0, &p->seq, 1));
		p->tmpl.cbx = p->tmpl.offsetx = x;
		p->tmpl.cby = p->tmpl.offsety = y;
		p->tmpl.kind = 2;
		p->tmpl.figure = fight_hero_pic(cur);
		TMPL_GFX_L = 0;
		p->tmpl.obj_id = 0;
		p->tmpl.z = 0x63;
		p->tmpl.visible = 1;
		p->tmpl.twofielded = FIG_TWOFIELD_NONE;
		cur->set_fighter_id(fig_list_add(&p->list, -1));
		fig_stand_pic(p, cur, &p->seq);
		if (cur->poison[POISON_ANGSTGIFT].state == ILL_STATE_SICK)
			cur->scared = 1;
		cur = get_next_hero(g_group, cur);
		r = keylist_add(t, fight_hero_pic(cur), 0x2000);
		if (cur->is_mage()) {
			if (cur->spells[SP_HEPTAGON] >= -5 || cur->spells[SP_FUROR_BLUT] >= -5) {
				r = keylist_add(t, 0xde, 0x2000);
				r = keylist_add(t, 0xcc, 0x2000);
				r = keylist_add(t, 0x89, 0x4000);
				r = keylist_add(t, 0x8b, 0x4000);
				r = keylist_add(t, 0x67, 0x4000);
			}
			if (cur->spells[SP_ELEMENTARE_BESCHWOEREN] >= -5) {
				r = keylist_add(t, 0xe0, 0x2000);
				r = keylist_add(t, 0x67, 0x4000);
				r = keylist_add(t, 0x85, 0x4000);
			}
		}
	} while (cur != first);
#if FIX_0168
	/* Fix #168: leeres "if (!r) ;" auf das (evtl. uninitialisierte) r entfernt */
	keylist_load(t, g_cd_cache);
#else
	if (!r)
		;
	keylist_load(t, g_cd_cache);
#endif
	Keylist_dtor(t, 3);
}

void fight_make_walkmap(Fight far *p)
{
	signed short y;
	unsigned char c;
	signed short x;
	signed short k;

	memset(g_fight_walkmap, 0, 600);
	for (y = 0; y < 24; y++) {
		for (x = 0; x < 24; x++) {
			c = *(p->field->cells[0] + y * FIG_MAP_PITCH + x);
			if (c >= 0xfd) {
				fight_set_cell(p, x, y, c);
			} else if (fig_is_cell_89_8c(c)) {
				c += 0xce;
				if (c == 0x59) {
					for (k = 1; k < 8; k++)
						fight_set_cell(p, x, y + k, 0xfe);
				} else if (c == 0x5a) {
					for (k = 1; k < 8; k++)
						fight_set_cell(p, x + k, y, 0xfe);
				}
				fight_set_cell(p, x, y, 0xfe);
			} else if (c == 0x9c || c == 0x9d)
				fight_set_cell(p, x, y, 0xcd);
		}
	}
	for (y = 8, x = 23, k = 0; k < 16; y++, x--, k++)
		if (!*(g_fight_walkmap + y * FIG_MAP_PITCH + x))
			fight_set_cell(p, x, y, 0xfe);
	for (y = 0, x = 16, k = 0; k < 8; y++, x++, k++)
		if (!*(g_fight_walkmap + y * FIG_MAP_PITCH + x))
			fight_set_cell(p, x, y, 0xfe);
}

void fig_add_field_objs(Fight far *p)
{
	signed short x;
	signed short y;
	signed short w;
	signed short h;
	unsigned char c;
	Bmp far *bmp;
	struct fig_node far *s;

	for (x = 0; x < 24; x++) {
		for (y = 0; y < 24; y++) {
			c = *(p->field->cells[0] + y * FIG_MAP_PITCH + x);
			if ((c >= 0x32 && c < 0x89) || (c >= 0x8f && c < 0xae && c != 0x9c && c != 0x9d)) {
				c -= 0x32;
				if (c == 0x5d) {
					TMPL_ANIM_L = (signed long)(p->anims[p->nr_anims++] = (FIGANIM_PTR)FigAnim_ctor(0, &p->seq, 1));
					p->tmpl.cbx = p->tmpl.offsetx = x;
					p->tmpl.cby = p->tmpl.offsety = y;
					p->tmpl.kind = 0;
					p->tmpl.figure = 0x91;
					TMPL_GFX_L = 0;
					p->tmpl.obj_id = 0;
					p->tmpl.z = 0x32;
					p->tmpl.visible = 1;
					p->tmpl.twofielded = FIG_TWOFIELD_NONE;
					g_fig_obj_ids[g_fig_nr_objs] = fig_list_add(&p->list, -1);
					g_fig_nr_objs++;
					memset(&p->seq, 0, 0xa2);
					p->seq.figure = 0x91;
					p->seq.step[0].frame = 0xda;
					p->seq.step[0].type = 3;
					p->seq.step[0].count = -1;
					p->seq.step[1].frame = -1;
					s = fig_list_find(&p->list, g_fig_obj_ids[g_fig_nr_objs - 1]);
					seq_set(s->anim, &p->seq);
					s->anim->delay_fix = 15;
				} else {
					if (g_fig_obj_bmps[c]) {
						bmp = g_fig_obj_bmps[c];
						bmp->size(&w, &h);
					} else {
						nvfile_load(g_nvf_heads, 0x47, p->buf, 0, c, 0, &w, &h, 0x140);
						bmp = g_fig_obj_bmps[c] = new Bmp(w, h);
						bmp->load(p->buf);
					}
					p->tmpl.obj_pic = c;
					p->tmpl.cbx = x;
					p->tmpl.cby = y;
					p->tmpl.offsetx = g_fig_obj_dxy[c].dx;
					p->tmpl.offsety = g_fig_obj_dxy[c].dy;
					p->tmpl.kind = 0;
					TMPL_ANIM_L = 0;
					TMPL_GFX_L = (unsigned long)bmp;
					p->tmpl.z = 0x32;
					p->tmpl.visible = 1;
					p->tmpl.twofielded = FIG_TWOFIELD_NONE;
					g_fig_obj_ids[g_fig_nr_objs] = fig_list_add(&p->list, -1);
					g_fig_nr_objs++;
				}
				place_obj_on_cb(p, x, y, c + 0x32, c, 0);
			} else if (c == 0xcc || c == 0xcd || c == 0xfe)
				fight_set_cell(p, x, y, c);
		}
	}
}

void fight_draw_bg(Fight far *p)
{
	signed short x;
	signed short y;
	signed short w;
	signed short h;
	signed short sx;
	signed short sy;
	Bmp far *bmp;
	void far *buf;
	signed short c;

	buf = mem_alloc(10000L);
	g_xms_move.len = 0xfa00L;
	g_xms_move.dst_handle = 0;
	g_xms_move.dst = (unsigned long)p->buf;
	g_xms_move.src_handle = g_fight_xms;
	g_xms_move.src = 0;
	if (g_fight_use_ems == 1)
		EMS_copy(&g_xms_move);
	else
		xms_move_any(&g_xms_move);
	for (y = 0; y < 24; y++) {
		for (x = 0; x < 24; x++) {
			c = *(p->field->cells[0] + y * FIG_MAP_PITCH + x);
			if (fig_is_cell_89_8c(c) || c == 0x9c || c == 0x9d) {
				c -= 0x32;
				if (g_fig_obj_bmps[c]) {
					bmp = g_fig_obj_bmps[c];
					bmp->size(&w, &h);
				} else {
					nvfile_load(g_nvf_heads, 0x47, buf, 0, c, 0, &w, &h, 0x140);
					bmp = g_fig_obj_bmps[c] = new Bmp(w, h);
					bmp->load(buf);
				}
				sx = g_fig_origin_x - w / 2 + (x + y) * 10;
				sy = g_fig_origin_y - h + (x - y) * 5;
				sx += g_fig_obj_dxy[c].dx;
				sy += g_fig_obj_dxy[c].dy;
				bmp->put(p->buf, sx, sy, BLIT_MASKED);
			}
		}
	}
	g_xms_move.len = 0xfa00L;
	g_xms_move.src_handle = 0;
	g_xms_move.src = (unsigned long)p->buf;
	g_xms_move.dst_handle = g_fight_xms;
	g_xms_move.dst = 0;
	if (g_fight_use_ems == 1)
		EMS_copy(&g_xms_move);
	else
		xms_move_any(&g_xms_move);
	mem_free(buf);
}

void fight_intro_text(Fight far *p)
{
	if (g_fight_end == 3)
		dlg_message(g_dialog, p->txt[0x65]);
}
