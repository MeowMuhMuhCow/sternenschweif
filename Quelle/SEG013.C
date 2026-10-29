/* SEG013 - 3D-Kollision: Wandprofile je Kartenzelle aus gedrehten Polygonkanten, Kameradrehung,
 * Bewegung mit Kollisionspruefung (seg013 in SCHWEIF.EXE)
 * cell_profile(x, y): Profil der Zelle (Cache g_prof_cells[4], Platz g_prof_next): fuer jede Flaeche mit Bit
 * FACE_SOLID des Zellobjekts g_cell_objs[get_cell_a(x, y)] (Bit get_cell_b & 0x2000 macht Flaeche 0 zur Wand)
 * die Kanten gedreht (Winkel get_cell_c) per edge in die Tabellen g_prof_zmin/zmax/ymin/ymax[Platz][Flaeche]
 * [32 Spalten] eintragen. can_stand(x, z): 16 x 16-Rand des Standorts gegen die Profile pruefen. cam_set_angles/
 * cam_set_speed: Bewegungsvektor g_cam_dir = g_cam_vec gedreht um g_cam_ang. cam_move: Kamera (Festkomma
 * g_cam_fx/g_cam_fz, << 8) bewegen; freie Bewegung (g_free_move) oder schrittweise bis zur Zellmitte mit
 * Abprallen (g_cam_bounced, g_move_flags = 1). cam_drop: so lange weiterschieben, bis die Kamera steht.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y SEG013.C   -> 4681/4681 Bytes GLEICH (18.10.2026)
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * ROT ist ein Makro mit eigenem Block und eigenen long-Variablen (X,Y,Z,s0..c2,t1..t3): darum liegen
 *   sie im Stack hinter den -Og-Zwischenwerten, und aufeinanderfolgende ROT-Bloecke teilen sich die Plaetze.
 * Ebenso der Tausch in edge(): Hilfszeiger im inneren Block -> [bp-28h].
 * cell_profile: dieselbe Registervariable fuer die Cache-Schleife und die Flaechenzahl (SI), j in DI.
 * can_stand: j und xx ausdruecklich register (sonst vergibt BCC SI/DI an andere Variablen).
 * Sinustabelle 1024 Eintraege, Kosinus = Sinus(a + 0x100), Festkomma 1.15.
 */
#include <stdlib.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:5ee4..5ee8, dsana) - initialisierte Daten in Adressreihenfolge */
signed short g_cam_speed_set = 32767;
signed short g_cam_bounced = 0;

#define ROT(src, ang, dst) { \
	long X, Y, Z, s0, s1, s2, c0, c1, c2, t1, t2, t3; \
	s0 = g_sin_table[(ang)[0] & 0x3ff]; \
	s1 = g_sin_table[(ang)[1] & 0x3ff]; \
	s2 = g_sin_table[(ang)[2] & 0x3ff]; \
	c0 = g_sin_table[((ang)[0] + 0x100) & 0x3ff]; \
	c1 = g_sin_table[((ang)[1] + 0x100) & 0x3ff]; \
	c2 = g_sin_table[((ang)[2] + 0x100) & 0x3ff]; \
	t1 = ((X = (src).x) * c0 - (Y = (src).y) * s0) >> 15; \
	t2 = (Y * c0 + X * s0) >> 15; \
	(dst).y = (t2 * c1 - (Z = (src).z) * s1) >> 15; \
	t3 = (Z * c1 + t2 * s1) >> 15; \
	(dst).z = (t3 * c2 - t1 * s2) >> 15; \
	(dst).x = (t1 * c2 + t3 * s2) >> 15; \
}

void edge(signed short slot, signed short k, struct vtx far *a, struct vtx far *b)
{
	signed long y, dy, z, dz;
	signed short n, x2;
	signed short x;

	if (a->x > b->x) {
		struct vtx far *t = a;
		a = b;
		b = t;
	}
	x = a->x * 34 / 168 + 17;
	x2 = b->x * 34 / 168 + 17;
	n = x2 - x;
	if (n == 0)
		return;
	dy = (((long)b->y - (long)a->y) << 16) / n;
	dz = (((long)b->z - (long)a->z) << 16) / n;
	y = (long)a->y << 16;
	z = (long)a->z << 16;
	while (n != 0) {
		if ((unsigned)x < 32) {
			if (g_prof_zmin[slot][k][x] > (z >> 16)) {
				g_prof_zmin[slot][k][x] = z >> 16;
				g_prof_ymin[slot][k][x] = y >> 16;
			}
			if (g_prof_zmax[slot][k][x] < (z >> 16)) {
				g_prof_zmax[slot][k][x] = z >> 16;
				g_prof_ymax[slot][k][x] = y >> 16;
			}
		}
		y += dy;
		z += dz;
		x++;
		n--;
	}
}

void face_profile(signed short slot, signed short k, signed short n, struct vtx far *v,
	signed short far *idx, signed short far *ang)
{
	struct vtx p1, p2;
	struct vtx far *p;
	signed short i;

	for (i = 0; i < 32; i++) {
		g_prof_zmin[slot][k][i] = 0x7fff;
		g_prof_zmax[slot][k][i] = -0x7fff;
	}
	for (i = 0; i < n - 1; i++) {
		p = &v[idx[i]];
		ROT(*p, ang, p1);
		p = &v[idx[i + 1]];
		ROT(*p, ang, p2);
		edge(slot, k, &p1, &p2);
	}
	p = &v[idx[n - 1]];
	ROT(*p, ang, p1);
	p = &v[idx[0]];
	ROT(*p, ang, p2);
	edge(slot, k, &p1, &p2);
}

signed short cell_profile(signed short x, signed short y)
{
	signed short cell;
	struct cell_rot q;
	struct obj3d far *o;
	struct face3d far *f;
	signed short slot;
	signed short saved;
	register signed short i, j;

	for (i = 0; i < 4; i++) {
		if (g_prof_cells[i].x == x && g_prof_cells[i].y == y)
			return i;
	}
	slot = g_prof_next & 3;
	g_prof_next = (g_prof_next + 1) & 3;
	g_prof_cells[slot].x = x;
	g_prof_cells[slot].y = y;
	g_prof_count[slot] = 0;
	cell = get_cell_a(x, y);
	q = get_cell_c(x, y);
	o = g_cell_objs[cell];
	i = o->nfaces;
	saved = o->faces[0]->flags;
	if (get_cell_b(x, y) & 0x2000)
		o->faces[0]->flags |= FACE_SOLID;
	for (j = 0; j < i; j++) {
		f = o->faces[j];
		if (f->flags & FACE_SOLID)
			face_profile(slot, g_prof_count[slot]++, f->nverts, o->vl->v, f->idx, &q.ax);
	}
	o->faces[0]->flags = saved;
	return slot;
}

void cell_forget(signed short x, signed short y)
{
	signed short i;

	for (i = 0; i < 4; i++) {
		if (g_prof_cells[i].x == x && g_prof_cells[i].y == y)
			g_prof_cells[i].x = g_prof_cells[i].y = -1;
	}
}

void cam_set_angles(signed short far *ang)
{
	if (ang[0] != g_cam_ang[0] || ang[1] != g_cam_ang[1] || ang[2] != g_cam_ang[2]) {
		g_cam_ang[0] = ang[0];
		g_cam_ang[1] = ang[1];
		g_cam_ang[2] = ang[2];
		ROT(g_cam_vec, g_cam_ang, g_cam_dir);
		g_cam_neg[0] = -ang[0];
		g_cam_neg[1] = -ang[1];
		g_cam_neg[2] = -ang[2];
	}
}

void cam_set_speed(signed short v)
{
	if (g_cam_speed_set != v) {
		g_cam_speed_set = v;
		g_cam_vec.x = 0;
		g_cam_vec.y = 0;
		g_cam_vec.z = v << 8;
		ROT(g_cam_vec, g_cam_ang, g_cam_dir);
	}
}

signed short can_stand(signed short x, signed short z)
{
	signed short lo, hi, h, i, slot;
	register signed short j, xx;

	x = ((long)x << 5) / 168;
	z = ((long)z << 5) / 168;
	x -= 8;
	z -= 8;
	for (i = 0; i < 16; i++) {
		slot = cell_profile(x >> 5, z >> 5);
		xx = x & 0x1f;
		h = ((long)((z & 0x1f) - 16) * 168) >> 5;
		for (j = 0; j < g_prof_count[slot]; j++) {
			lo = g_prof_zmin[slot][j][xx];
			hi = g_prof_zmax[slot][j][xx];
			if (lo <= h && hi >= h)
				goto ok1;
		}
		return 0;
ok1:		x++;
	}
	for (i = 0; i < 16; i++) {
		slot = cell_profile(x >> 5, z >> 5);
		xx = x & 0x1f;
		h = ((long)((z & 0x1f) - 16) * 168) >> 5;
		for (j = 0; j < g_prof_count[slot]; j++) {
			lo = g_prof_zmin[slot][j][xx];
			hi = g_prof_zmax[slot][j][xx];
			if (lo <= h && hi >= h)
				goto ok2;
		}
		return 0;
ok2:		z++;
	}
	for (i = 0; i < 16; i++) {
		slot = cell_profile(x >> 5, z >> 5);
		xx = x & 0x1f;
		h = ((long)((z & 0x1f) - 16) * 168) >> 5;
		for (j = 0; j < g_prof_count[slot]; j++) {
			lo = g_prof_zmin[slot][j][xx];
			hi = g_prof_zmax[slot][j][xx];
			if (lo <= h && hi >= h)
				goto ok3;
		}
		return 0;
ok3:		x--;
	}
	for (i = 0; i < 16; i++) {
		slot = cell_profile(x >> 5, z >> 5);
		xx = x & 0x1f;
		h = ((long)((z & 0x1f) - 16) * 168) >> 5;
		for (j = 0; j < g_prof_count[slot]; j++) {
			lo = g_prof_zmin[slot][j][xx];
			hi = g_prof_zmax[slot][j][xx];
			if (lo <= h && hi >= h)
				goto ok4;
		}
		return 0;
ok4:		z--;
	}
	return 1;
}

signed short cam_move(void)
{
	signed long save;
	signed short v1, v2;
	signed short rem, ok;

	ok = 1;
	if (g_free_move) {
		if (g_cam_dir.x) {
			save = g_cam_fx;
			g_cam_fx += g_cam_dir.x;
			if (!can_stand(g_cam_fx >> 8, g_cam_z))
				g_cam_fx = save;
		}
		if (g_cam_dir.z) {
			save = g_cam_fz;
			g_cam_fz += g_cam_dir.z;
			if (!can_stand(g_cam_x, g_cam_fz >> 8))
				g_cam_fz = save;
		}
	} else {
		if (g_cam_dir.x) {
			v1 = g_cam_dir.x;
			rem = abs(v1);
			save = g_cam_fx;
			while (rem) {
				if (g_cam_dir.x > 0)
					g_cam_fx += (rem > 0x100 ? 0x100 : rem);
				else
					g_cam_fx -= (rem > 0x100 ? 0x100 : rem);
				if ((rem -= 0x100) < 0)
					rem = 0;
				if (((g_cam_fx >> 8) - 84) % 168 == 0) {
					g_cam_fx &= 0xffffff00L;
					ok = 0;
					break;
				}
			}
			if (!can_stand(g_cam_fx >> 8, g_cam_z)) {
				g_cam_fx = save;
				if (g_cam_bounced) {
					g_cam_bounced = ok = 0;
					g_cam_dir.x = 0;
				} else {
					g_cam_dir.x = -g_cam_dir.x;
					g_move_flags = 1;
					g_cam_bounced = 1;
				}
			}
		}
		if (g_cam_dir.z) {
			v2 = g_cam_dir.z;
			rem = abs(v2);
			save = g_cam_fz;
			while (rem) {
				if (g_cam_dir.z > 0)
					g_cam_fz += (rem > 0x100 ? 0x100 : rem);
				else
					g_cam_fz -= (rem > 0x100 ? 0x100 : rem);
				if ((rem -= 0x100) < 0)
					rem = 0;
				if (((g_cam_fz >> 8) - 84) % 168 == 0) {
					g_cam_fz &= 0xffffff00L;
					ok = 0;
					break;
				}
			}
			if (!can_stand(g_cam_x, g_cam_fz >> 8)) {
				g_cam_fz = save;
				if (g_cam_bounced) {
					g_cam_bounced = ok = 0;
					g_cam_dir.z = 0;
				} else {
					g_cam_dir.z = -g_cam_dir.z;
					g_move_flags = 1;
					g_cam_bounced = 1;
				}
			}
		}
		if (!ok)
			g_cam_bounced = 0;
	}
	g_cam_x = g_cam_fx >> 8;
	g_cam_y = 0;
	g_cam_z = g_cam_fz >> 8;
	return ok;
}

void cam_drop(void)
{
	while (!can_stand(g_cam_fx >> 8, g_cam_fz >> 8)) {
		g_cam_fx += g_cam_dir.x;
		g_cam_fz += g_cam_dir.z;
	}
	g_cam_x = g_cam_fx >> 8;
	g_cam_y = 0;
	g_cam_z = g_cam_fz >> 8;
}
