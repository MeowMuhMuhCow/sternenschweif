/* OVL097 - Overlay O097 (Segment 0x2135 in SCHWEIF.EXE): 3D-Objekte der Ansicht laden.
 * view_load_objects: Objekte einer Stadt - Liste g_town_view_areas[group.town - 1].objs (struct view_obj {Platz, Datei}) bis
 *   -1/-2 abarbeiten (game_idle, obj_load), hinter -2 nur ohne Sparbetrieb g_mem_saving weiterlesen; Zusatzobjekt k
 *   je Stadt (2/6 -> 9, 3 -> 4, 4 -> 8, 5 -> 10, bei 5 und g_phex_finale_done hat Objekt 27 zwei Flaechen weniger), danach die
 *   Texturen der ersten vier Flaechen von Objekt 2 bzw. k nach g_tex_anim_a/b sichern und durch die der ersten
 *   ersetzen (Texturanimation O198); g_tex_anim_frames/rate/tick/frame = 4/0x1e/0/0.
 * view_load_dng_objects: dasselbe fuer Dungeons (g_dng_view_areas, group.dungeon - 1): Dungeon 4 Objekte 4/7/22 (Folgen a/b/c),
 *   6 Objekte 5 (3 Flaechen), 16, 17 (g_dng06_dogs_state -> Objekt 19 zwei Flaechen weniger), 5 Objekt 21, 1 nur
 *   g_tex_anim_rate = 0x14.
 * tex_load(tex, datei): Bild aus RAW.DAT (g_file_raw) laden, Breite/Hoehe/Farbzahl aus dem Kopf (+0x20..+0x25),
 *   Palette ab +0x26 per pal_set_rgb, freie Plaetze der Palette g_view_pal (bis 160 Farben) belegen, Rest per
 *   pal_find_nearest abbilden, Pixel umsetzen und in den Auslagerungsspeicher (swap_write); Rueckgabe 0/1/3.
 * tex_get(name): Bild-Cache (g_nr_textures Eintraege, Namen g_tex_names zu 12 Zeichen, g_textures, hoechstens 60).
 * obj_read_string(src, dst): verschluesselte Zeichenkette lesen. obj_load(idx, id): 0x2c-Byte-Objekt nach
 *   g_cell_objs[idx], obj_read, bei Erfolg obj3d_prepare (O010). obj_read(id, obj): Objektdatei (arc_file_size/
 *   arc_load, g_file_3) mit dem Schluessel g_obj_key (Start 0xf357) lesbar machen und Punkt-, Pfad- und
 *   Flaechentabellen fuellen (obj3d_alloc; bewegte Punkte g_vtx_paths[g_nr_vtx_paths]); Flaechen mit FACE_TEXTURED
 *   haben einen Bildnamen (tex_get), sonst eine 24-Bit-Farbe. unused_view_stub: leer.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL097.CPP   -> 6559/6559 Bytes GLEICH (18.10.2026), cobjfix laeuft
 * - Tabellenadresse "mov dx,BASIS / add dx,0eh / add ax,dx / mov bx,ax" (das UNGELOESTE Muster aus O014/O046)
 *   OHNE asm geloest (Welle AW): "typ * 18 + (unsigned)&((struct ent near *)(unsigned)g_4c25)->fe" - die
 *   Adresse als unsigned und zurueck in einen near-Zeiger gecastet ist fuer BCC keine faltbare Adresskonstante
 *   mehr: mov dx,offset g + add dx,Feld bleiben getrennt. Reihenfolge "typ * 18 + .." noetig (andersherum bzw.
 *   mit sizeof faltet/push-pop); ohne den inneren (unsigned)-Cast faltet BCC zu [bx+4c33h]. Vorstufe: -Op
 *   (Kopierfortpflanzung) mit "t = g_town_view_areas - 1; .. (unsigned)&t->objs" erzeugt das Muster auch, laesst aber den toten
 *   Store stehen. Gleiche Schreibweise in OVL014/OVL046 getestet (wovaw/t14, wovaw/t46): beide GLEICH ohne asm.
 * - C++ wegen inline: readb(p) mit Wort-Local r, "g_obj_key = 1 | (g_obj_key << 1)" (mov dx,1 / or dx,ax);
 *   readw(signed short far *p) braucht das Local "unsigned char far *q = (unsigned char far *)p", sonst fehlen
 *   die Kopien (Argument cur++ -> Temp, Kopie q mit mov eax, q++ -> Temp = readb-Parameter, readb(q) -> eigene
 *   Parameterkopie mit mov edx). Jede Expansion eigene 0x14 Bytes Stack; die Expansionen im Bit-3-Zweig liegen
 *   hinter denen des else-Zweigs, weil dort "char name[80]" deklariert ist (Block mit eigener Deklaration).
 * - 24-Bit-Wert: "(void far *)((signed long)readw(..) + ((signed long)readw(..) << 8) + (.. << 16))" (shld +
 *   zwei Wort-Stores). Schleifen "o->vl->n > i" (cmp ax,[i]), "j < n" mit j in DI.
 * - tex_load: swap_write-Argument "MK_FP(FP_SEG(buf), FP_OFF(buf) + cnt)" (push seg vor der Addition; buf + cnt
 *   rechnet zuerst); "pal.n = o->nr_colors = .."; "i <= o->nr_colors" (jnc, eine Farbe mehr als angegeben); "g_view_pal_n = g_view_pal_n +
 *   cnt"; -Og haelt &buf[i + cnt] in [bp-0ch]. -Oi fuer strncpy in tex_get.
 * - Flaechen als struct face3d (0x48 Bytes, Texturkoordinaten ueber struct uv_pair), Punkte struct vtx_list.
 *   (Namensfassung mit schweif.h, Welle N19: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * Auffaellig (Original): view_load_dng_objects setzt in Dungeon 6 fuer die Objekte 16 und 17 die Textur der ersten
 *   Flaeche auf sich selbst (wirkungslos); tex_load liest nr_colors + 1 Palettenfarben (Schleife bis einschliesslich
 *   n). Hinweis: der alte Kopf nannte view_load_objects Dungeon und view_load_dng_objects Orte (vertauscht).
 */
#include <string.h>
#include <dos.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:5eb2..5ee4, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */

#define UV(f, j) (((struct uv_pair far *)(f)->uv)[j])	/* Texturkoordinaten der Ecke j einer Flaeche */

inline unsigned short readb(unsigned char far *p)
{
	unsigned short r = (unsigned char)(*p ^ g_obj_key);
	if (g_obj_key & 0x8000)
		g_obj_key = 1 | (g_obj_key << 1);
	else
		g_obj_key <<= 1;
	return r;
}

inline signed short readw(signed short far *p)
{
	unsigned char far *q = (unsigned char far *)p;

	return readb(q++) + (readb(q) << 8);
}

void view_load_objects(void)
{
	struct obj3d far *q;
	signed short k;
	struct view_obj far *p;
	struct texture far *save;
	register signed short i;
	register signed short typ;

	k = 0;
	typ = g_group->town;
	p = *(struct view_obj far * near *)(typ * 18 + (unsigned)&((struct view_area near *)(unsigned)(g_town_view_areas - 1))->objs);
	g_nr_vtx_paths = 0;
	while (p->idx != -1 && p->idx != -2) {
		game_idle();
		obj_load(p->idx, p->file);
		p++;
	}
	if (p->idx == -2 && g_mem_saving == 0) {
		p++;
		while (p->idx != -1) {
			game_idle();
			obj_load(p->idx, p->file);
			p++;
		}
	}
	if (typ == 2)
		k = 9;
	else if (typ == 3)
		k = 4;
	else if (typ == 4)
		k = 8;
	else if (typ == 6)
		k = 9;
	else if (typ == 5) {
		k = 10;
		if (g_phex_finale_done != 0 && !g_mem_saving) {
			q = g_cell_objs[27];
			q->nfaces -= 2;
		}
	}
	if (typ != 3) {
		g_obj_cur = g_cell_objs[2];
		save = g_obj_cur->faces[0]->tex;
		for (i = 0; i < 4; i++) {
			g_tex_anim_a[i] = g_obj_cur->faces[i]->tex;
			g_obj_cur->faces[i]->tex = save;
		}
	}
	if (k) {
		g_obj_cur = g_cell_objs[k];
		save = g_obj_cur->faces[0]->tex;
		for (i = 0; i < 4; i++) {
			g_tex_anim_b[i] = g_obj_cur->faces[i]->tex;
			g_obj_cur->faces[i]->tex = save;
		}
	}
	g_tex_anim_frames = 4;
	g_tex_anim_rate = 0x1e;
	g_tex_anim_frame = g_tex_anim_tick = 0;
}

void view_load_dng_objects(void)
{
	struct obj3d far *q;
	struct texture far *save;
	struct view_obj far *p;
	register signed short i;
	register signed short typ;

	typ = g_group->dungeon;
	p = *(struct view_obj far * near *)(typ * 18 + (unsigned)&((struct view_area near *)(unsigned)(g_dng_view_areas - 1))->objs);
	g_nr_vtx_paths = 0;
	while (p->idx != -1) {
		game_idle();
		obj_load(p->idx, p->file);
		p++;
	}
	if (typ == 4) {
		g_obj_cur = g_cell_objs[4];
		save = g_obj_cur->faces[0]->tex;
		for (i = 0; i < 4; i++) {
			g_tex_anim_a[i] = g_obj_cur->faces[i]->tex;
			g_obj_cur->faces[i]->tex = save;
		}
		g_obj_cur = g_cell_objs[7];
		save = g_obj_cur->faces[0]->tex;
		for (i = 0; i < 4; i++) {
			g_tex_anim_b[i] = g_obj_cur->faces[i]->tex;
			g_obj_cur->faces[i]->tex = save;
		}
		g_obj_cur = g_cell_objs[22];
		save = g_obj_cur->faces[0]->tex;
		for (i = 0; i < 4; i++) {
			g_tex_anim_c[i] = g_obj_cur->faces[i]->tex;
			g_obj_cur->faces[i]->tex = save;
		}
		g_tex_anim_frames = 4;
		g_tex_anim_rate = 0x23;
		g_tex_anim_frame = g_tex_anim_tick = 0;
		return;
	}
	if (typ == 6) {
		if (g_dng06_dogs_state) {
			q = g_cell_objs[19];
			q->nfaces -= 2;
		}
		g_obj_cur = g_cell_objs[5];
		save = g_obj_cur->faces[0]->tex;
		for (i = 0; i < 3; i++) {
			g_tex_anim_a[i] = g_obj_cur->faces[i]->tex;
			g_obj_cur->faces[i]->tex = save;
		}
		g_obj_cur = g_cell_objs[16];
		save = g_obj_cur->faces[0]->tex;
		g_obj_cur->faces[0]->tex = save;
		g_obj_cur = g_cell_objs[17];
		save = g_obj_cur->faces[0]->tex;
		g_obj_cur->faces[0]->tex = save;
		g_tex_anim_frames = 3;
		g_tex_anim_rate = 0x14;
		g_tex_anim_frame = g_tex_anim_tick = 0;
		return;
	}
	if (typ == 5) {
		g_obj_cur = g_cell_objs[21];
		save = g_obj_cur->faces[0]->tex;
		for (i = 0; i < 4; i++) {
			g_tex_anim_a[i] = g_obj_cur->faces[i]->tex;
			g_obj_cur->faces[i]->tex = save;
		}
		g_tex_anim_frames = 4;
		g_tex_anim_rate = 0x14;
		g_tex_anim_frame = g_tex_anim_tick = 0;
		return;
	}
	if (typ == 1) {
		g_tex_anim_rate = 0x14;
		g_tex_anim_frame = g_tex_anim_tick = 0;
	}
}

signed short tex_load(struct texture far *o, signed short file)
{
	unsigned short len;
	unsigned char far *buf;
	unsigned short i;
	unsigned short size;
	struct palette pal;
	unsigned char map[256];
	register unsigned short cnt;

	if (file_open(g_file_raw, file) == -1)
		return 1;
	len = file_length2(g_file_raw);
	if ((buf = (unsigned char far *)mem_alloc((unsigned long)len)) == 0)
		return 3;
	file_read(g_file_raw, buf, (unsigned long)len);
	file_close(g_file_raw);
	o->w = buf[0x20] + (buf[0x21] << 8) + 1;
	o->h = buf[0x22] + (buf[0x23] << 8) + 1;
	pal.n = o->nr_colors = buf[0x24] + (buf[0x25] << 8);
#if FIX_0202
	/* Fix #202: genau nr_colors Palettenfarben lesen */
	for (i = 0; i < o->nr_colors; i++)
		pal_set_rgb(&pal, i, buf[i * 3 + 0x26], buf[i * 3 + 0x27], buf[i * 3 + 0x28]);
#else
	for (i = 0; i <= o->nr_colors; i++)
		pal_set_rgb(&pal, i, buf[i * 3 + 0x26], buf[i * 3 + 0x27], buf[i * 3 + 0x28]);
#endif
	cnt = 0xa0 - g_view_pal_n;
	if (cnt < pal.n) {
		for (i = cnt; i < pal.n; i++)
			map[i] = pal_find_nearest(&pal.c[i], 0);
		pal.n = cnt;
	} else
		cnt = pal.n;
	for (i = 0; i < cnt; i++) {
		g_view_pal[g_view_pal_n + i] = pal.c[i];
		map[i] = g_view_pal_n + i;
	}
	g_view_pal_n = g_view_pal_n + cnt;
	cnt = o->nr_colors * 3 + 0x26;
	size = o->w * o->h;
	o->ref.seg = 0;
	o->ref.off = g_swap_pos;
	for (i = 0; i < size; i++)
		buf[i + cnt] = map[buf[i + cnt]];
	swap_write((unsigned char far *)MK_FP(FP_SEG(buf), FP_OFF(buf) + cnt), (unsigned long)size);
	g_swap_used_kb = g_swap_pos / 1024;
	mem_free(buf);
	return 0;
}

struct texture far *tex_get(char far *name)
{
	signed short i;
	signed short h;

	str_toupper(name);
	if (g_nr_textures) {
		for (i = 0; i < g_nr_textures; i++)
			if (!strnicmp(g_tex_names[i], name, 12))
				return &g_textures[i];
	}
	if (g_nr_textures == 60)
		guru_meditation("TOO MANY BITMAPS", 0);
	h = File_find(g_file_raw, name);
	if (tex_load(&g_textures[g_nr_textures], h))
		guru_meditation("CAN'T LOAD BITMAP", 0);
	strncpy(g_tex_names[g_nr_textures], name, 12);
	g_nr_textures++;
	return &g_textures[g_nr_textures - 1];
}

unsigned char far *obj_read_string(unsigned char far *src, char far *dst)
{
	unsigned char far *p = src;
	register unsigned short c;

	do {
		c = readb(p++);
		*dst++ = c;
	} while (c);
	return p;
}

void obj_load(register signed short idx, signed short id)
{
	if ((g_cell_objs[idx] = (struct obj3d far *)mem_alloc(0x2cL)) == 0)
		guru_meditation("OUT OF MEMORY", 0);
	if (obj_read(id, g_cell_objs[idx]) == 1)
		obj3d_prepare(g_cell_objs[idx]);
}

signed short obj_read(signed short id, struct obj3d far *o)
{
	signed short i;
	signed short n;
	signed short m;
	signed short nverts;
	signed short nfaces;
	struct vtx_path far *tab;
	unsigned char far *buf;
	signed short far *cur;
	register signed short j;

	g_obj_key = 0xf357;
	buf = (unsigned char far *)mem_alloc(arc_file_size(g_file_3, id));
	arc_load(g_file_3, id, buf);
	cur = (signed short far *)buf;
	nverts = readw(cur++);
	nfaces = readw(cur++);
	obj3d_alloc(o, nverts, nfaces);
	o->vl->n = nverts;
	o->nfaces = nfaces;
	for (i = 0; o->vl->n > i; i++) {
		o->vl->v[i].x = readw(cur++);
		if (o->vl->v[i].x != 0x7fff) {
			o->vl->v[i].y = readw(cur++);
			o->vl->v[i].z = readw(cur++);
		} else {
			o->vl->v[i].y = g_nr_vtx_paths;
			n = readw(cur++);
			m = readw(cur++);
			tab = g_vtx_paths[g_nr_vtx_paths] = (struct vtx_path far *)mem_alloc(n * 8U + 4);
			if (tab == 0)
				return 0;
			g_nr_vtx_paths++;
			tab->n = n;
			tab->m = m;
			for (j = 0; j < n; j++) {
				tab->e[j].x = readw(cur++);
				tab->e[j].y = readw(cur++);
				tab->e[j].z = readw(cur++);
			}
		}
	}
	for (i = 0; o->nfaces > i; i++) {
		o->faces[i]->nverts = readw(cur++);
		o->faces[i]->flags = readw(cur++);
		if (o->faces[i]->flags & FACE_TEXTURED) {
			char name[80];

			cur = (signed short far *)obj_read_string((unsigned char far *)cur, name);
			o->faces[i]->tex = tex_get(name);
			for (j = 0; o->faces[i]->nverts > j; j++) {
				o->faces[i]->idx[j] = readw(cur++);
				UV(o->faces[i], j).u = readw(cur++);
				UV(o->faces[i], j).v = readw(cur++);
			}
		} else {
			o->faces[i]->tex = (struct texture far *)((signed long)readw(cur++)
				+ ((signed long)readw(cur++) << 8)
				+ ((signed long)readw(cur++) << 16));
			for (j = 0; o->faces[i]->nverts > j; j++)
				o->faces[i]->idx[j] = readw(cur++);
		}
	}
	mem_free(buf);
	return 1;
}

void unused_view_stub(void)
{
}
