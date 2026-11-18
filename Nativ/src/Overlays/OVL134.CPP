/* OVL134 - Overlay O134 (Segment 0x220d in SCHWEIF.EXE): Kampfbildschirm auf-/abbauen (Kampfstruktur wie O021/O148:
 * Bilder +0xe3..+0x143, Figurenliste +0x35, Animationspuffer struct anim +0x41, Animationsobjekte +0x213[30]).
 * fight_timer_stop/fight_timer_start: Zeitgeber stoppen/starten (R22, Handler R23:0264 alle 50, Zaehler ds:c521, ds:6644 = Nummer).
 * fight_quick_start(f): Aufbau - Texte freigeben, Bildreste +295/+296 (O153_06A0), Bildschirm sichern (O034_0551) bzw.
 * f->203 = 0 + O144_0000, alte Animationsobjekte (+213, ds:c961) loeschen, Bmp-Objekte 200x14/28x37/32x32/24x24,
 * je Held Portrait 28x37 + Kopf 16x16 (O076_0B70/0D8C), Symbole aus Datei 6 (Offsets 0x487/0x7747/0x3847, je
 * 0x240 Bytes), Hintergrund 0x8a (Palette ab +0xc0), Startanimation O051 0x8b (Ende-Modus 3 bzw. 2 nach ds:2541).
 * fight_quick_end(f, sieg): auf Ende der Animation warten (O144_1A30), Schlussanimation 0x8d/0x8c, alles freigeben,
 * Texte 0x25 neu laden. fight_quick_stop(f): Bilder freigeben, Figurenliste durchgehen und fuer Helden (Art 2, O135_0262),
 * Gegner (Art 1, nur Gegnertyp 0xff oder <= 20, O136_0268) und Objekte (Art 0 mit Bild: Folge 0x91/0xda)
 * neue Animationsobjekte anlegen, O153_03F0/039F, Texte neu, O144_0000. fight_new_extra_anim: freien Platz in ds:c961[10]
 * mit neuem Animationsobjekt fuellen (alle voll: Platz 0 ersetzen), Rueckgabe des Objekts.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL134.CPP   -> 4526/4526 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h, Welle N7: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * - -Oi fuer memset (rep stosw, blockiert DI; i daher in fight_quick_start/fight_new_extra_anim auf dem Stack), -Og fuer den Zwischenwert
 *   [bp-12h] (&f->f19f zweimal als Argument).
 * - C++ wegen inline-Setter ani::set21 (Parameterkopie [bp-0fh] bei "!g_fight_speed ? 3 : 2", Konstante 2 ohne Kopie);
 *   direkte Zuweisung ergibt kein Zwischen-Byte.
 * - "(char far *)f->f0 + (unsigned short)(h * w)": nur mit unsigned-Cast wird das Produkt vor dem les gerechnet
 *   (sonst push bx/es um imul).
 * - "if (g_opt_fight_keep_view == 0)" -> cmp byte; "!g_c521++" / "!(g_c521 -= 1)" -> mov al / inc|dec / cbw / or ax,ax.
 * - Kette "p->fd = f->f213[f->f293++] = O016_0000(..)" wie in O021 (push ax / pop dx).
 * Originalfehler: fight_quick_start uebergibt an O098_00D1 zweimal &f->f19f als Breite- und Hoehe-Ziel (vermutlich
 *   &f->f1a1 gemeint); R07_0140(f->f4/f->f8) ohne Ruecksetzen der Zeiger. fight_new_extra_anim: Parameter unbenutzt.
 */
#include <mem.h>
#include <dos.h>
#include "schweif.h"

void fight_timer_stop(void)
{
	if (g_fight_timer != -1) {
		AIL_release_timer_handle(g_fight_timer);
		if (!(g_ail_timer_users -= 1))
			AIL_shutdown(0, 0);
		g_fight_timer = -1;
	}
}

void fight_timer_start(void)
{
	if (!g_ail_timer_users++)
		AIL_startup();
	g_fight_timer = AIL_register_timer(MK_FP(FP_SEG(r23_000f), 0x264));
	AIL_set_timer_frequency(g_fight_timer, 50L);
	AIL_start_timer(g_fight_timer);
}

void fight_quick_start(Fight far *f)
{
	signed short w;
	signed short h;
	signed short i;
	struct hero far *hero;
	BMP_PTR b;

	mem_free(f->txt_buf);
	mem_free(f->txt);
	if (f->mark_id != -1) {
		fig_list_remove(&f->list, f->mark_id, 0);
		f->mark_id = -1;
	}
	if (f->mark_id2 != -1) {
		fig_list_remove(&f->list, f->mark_id2, 0);
		f->mark_id2 = -1;
	}
	if (g_opt_fight_keep_view == 0) {
		clear_screen_buf(g_vga_buf);
	} else {
		f->top_hero = 0;
		g_fig_redraw_all = 1;
		fight_draw(f);
	}
	g_fight_screen_on = 0;
	for (i = 0; i < 30; i++) {
		if (f->anims[i]) {
			FigAnim_dtor(f->anims[i], 3);
			f->anims[i] = 0;
		}
	}
	f->nr_anims = 0;
	for (i = 0; i < 10; i++) {
		if (g_fig_extra_anims[i]) {
			FigAnim_dtor(g_fig_extra_anims[i], 3);
			g_fig_extra_anims[i] = 0;
		}
	}
	f->status_bg = (BMP_PTR)gb_new_alloc(0, 200, 14);
	gb_save(f->status_bg, g_vga_buf, 0x40, 0xa0);
	f->bar_tmp = (BMP_PTR)gb_new_alloc(0, 28, 37);
	f->sums_tmp = (BMP_PTR)gb_new_alloc(0, 200, 14);
	f->tmp32 = (BMP_PTR)gb_new_alloc(0, 32, 32);
	f->tmp24 = (BMP_PTR)gb_new_alloc(0, 24, 24);
	hero = get_first_hero(g_group);
	i = 0;
	do {
		f->bar_bg[i] = (BMP_PTR)gb_new_alloc(0, 28, 37);
		gb_save(f->bar_bg[i], g_vga_buf, i * 28 + 0x3f, 6);
		f->heads[i] = (BMP_PTR)gb_new_alloc(0, 16, 16);
		f->head_no[i] = portrait_file(hero);
		load_portrait_bmp(hero, f->tmp32);
		gb_scale(f->tmp32, f->heads[i]);
		hero = get_next_hero(g_group, hero);
		i++;
	} while (get_first_hero(g_group) != hero);
	f->pic_dead = (BMP_PTR)gb_new_alloc(0, 16, 16);
	nvfile_load(g_nvf_heads, 5, gb_data(f->tmp32), 0, f->nvf5_count - 12, 0, &f->pic_w, &f->pic_w, 0x140);
	gb_scale(f->tmp32, f->pic_dead);
	f->pic_unable = (BMP_PTR)gb_new_alloc(0, 16, 16);
	f->pic_fleeing = (BMP_PTR)gb_new_alloc(0, 16, 16);
	f->pic_unarmed = (BMP_PTR)gb_new_alloc(0, 16, 16);
	arc_open(g_file_3, 6);
	arc_seek(g_file_3, 0x487L, 0);
	arc_read(g_file_3, gb_data(f->tmp24), 0x240L);
	gb_scale(f->tmp24, f->pic_unable);
	arc_seek(g_file_3, 0x7747L, 0);
	arc_read(g_file_3, gb_data(f->tmp24), 0x240L);
	gb_scale(f->tmp24, f->pic_fleeing);
	arc_seek(g_file_3, 0x3847L, 0);
	arc_read(g_file_3, gb_data(f->tmp24), 0x240L);
	gb_scale(f->tmp24, f->pic_unarmed);
	arc_close(g_file_3);
	f->shown_le = f->shown_ae = f->shown_le_max = f->shown_ae_max = 0;
	memset(f->shown_hero_le, 0, sizeof(f->shown_hero_le));
	memset(f->shown_hero_ae, 0, sizeof(f->shown_hero_ae));
	memset(f->shown_status, 0, sizeof(f->shown_status));
	/* Hintergrund 0x8a und Startanimation 0x8b */
	nvfile_pal(g_nvf_heads, 0x8a, f->pal, 0x60);
	set_palette(f->pal + 0xc0, 0x40, 0x20);
	nvfile_load(g_nvf_heads, 0x8a, f->buf, 0, 0, 0, &w, &h, 0x140);
	b = (BMP_PTR)gb_new_alloc(0, w, h);
	gb_load_mem(b, f->buf);
	gb_blit_m(b, g_vga_buf, (320 - w) / 2, (200 - h) / 2, 4);
	gb_delete(b, 3);
	f->intro_ani = (struct anim_view far *)Anim_ctor(0, 0x8b, 0, 0);
	Anim_size(f->intro_ani, &w, &h);
	f->intro_bg = (BMP_PTR)gb_new(0, w, h, (char far *)f->buf + (unsigned short)(h * w));
	gb_save(f->intro_bg, g_vga_buf, (320 - w) / 2, (200 - h) / 2);
	Anim_setbg(f->intro_ani, f->intro_bg);
	Anim_setbuf(f->intro_ani, w, h, f->buf);
	f->intro_ani->set_rate(!g_fight_speed ? 3 : 2);
	Anim_start(f->intro_ani, (320 - w) / 2, (200 - h) / 2, 0);
	snd_play(g_snd_music, 0x1a);
	if (g_mem_saving) {
		Anim_tick(f->intro_ani);
		Anim_wait(f->intro_ani, 1);
		g_anim_running = 0;
	}
	g_quick_fight = 1;
}

void fight_quick_end(Fight far *f, signed short won)
{
	signed short w;
	signed short h;
	struct hero far *hero;
	register signed short i;

	if (g_mem_saving)
		g_anim_running = 1;
	while (!(i = f->intro_ani->rounds))
		fight_check_keys(f);
	Anim_step(f->intro_ani);
	Anim_dtor(f->intro_ani, 3);
	f->intro_ani = (struct anim_view far *)Anim_ctor(0, won ? 0x8d : 0x8c, 0, 0);
	Anim_size(f->intro_ani, &w, &h);
	Anim_setbuf(f->intro_ani, w, h, f->buf);
	Anim_setbg(f->intro_ani, f->intro_bg);
	f->intro_ani->set_rate(2);
	Anim_start(f->intro_ani, (320 - w) / 2, (200 - h) / 2, 0);
	Anim_step(f->intro_ani);
	fig_draw_hero_bar(f);
	wait_ticks_key(50);
	Anim_dtor(f->intro_ani, 3);
	gb_delete(f->intro_bg, 3);
	gb_delete(f->pic_unarmed, 3);
	gb_delete(f->pic_fleeing, 3);
	gb_delete(f->pic_unable, 3);
	gb_delete(f->pic_dead, 3);
	hero = get_first_hero(g_group);
	i = 0;
	do {
		gb_delete(f->bar_bg[i], 3);
		gb_delete(f->heads[i], 3);
		hero = get_next_hero(g_group, hero);
		i++;
	} while (get_first_hero(g_group) != hero);
	gb_delete(f->tmp24, 3);
	gb_delete(f->tmp32, 3);
	gb_delete(f->sums_tmp, 3);
	gb_delete(f->bar_tmp, 3);
	gb_delete(f->status_bg, 3);
	screensaver_reset(&g_saver);
	txt_load_list(g_font, 0x25, 0x6c, &f->txt_buf, &f->txt);
	g_quick_fight = 0;
}

void fight_quick_stop(Fight far *f)
{
	struct hero far *hero;
	struct fig_node far *p;
	struct enemy far *e;
	register signed short i;

	Anim_dtor(f->intro_ani, 3);
	gb_delete(f->intro_bg, 3);
	gb_delete(f->pic_unarmed, 3);
	gb_delete(f->pic_fleeing, 3);
	gb_delete(f->pic_unable, 3);
	gb_delete(f->pic_dead, 3);
	hero = get_first_hero(g_group);
	i = 0;
	do {
		gb_delete(f->bar_bg[i], 3);
		gb_delete(f->heads[i], 3);
		hero = get_next_hero(g_group, hero);
		i++;
	} while (get_first_hero(g_group) != hero);
	gb_delete(f->tmp24, 3);
	gb_delete(f->tmp32, 3);
	gb_delete(f->sums_tmp, 3);
	gb_delete(f->bar_tmp, 3);
	gb_delete(f->status_bg, 3);
	for (i = 0; i < 30; i++) {
		if (f->anims[i]) {
			FigAnim_dtor(f->anims[i], 3);
			f->anims[i] = 0;
		}
	}
	f->nr_anims = 0;
	for (i = 0; i < 10; i++) {
		if (g_fig_extra_anims[i]) {
			FigAnim_dtor(g_fig_extra_anims[i], 3);
			g_fig_extra_anims[i] = 0;
		}
	}
	g_quick_fight = 0;
	/* Animationsobjekte fuer alle Figuren neu anlegen */
	p = f->list.head;
	do {
		if (!p->to_remove && (p->kind == FIG_KIND_ENEMY || p->kind == FIG_KIND_HERO ||
			(p->kind == FIG_KIND_OBJ && p->anim != 0))) {
			if (p->kind == FIG_KIND_ENEMY) {
				if (p->twofielded == FIG_TWOFIELD_NONE || p->twofielded <= 20) {
					e = fig_get_enemy(p->id);
					p->anim = f->anims[f->nr_anims++] = (FIGANIM_PTR)FigAnim_ctor(0, &f->seq, 1);
					fig_stand_seq(f, e, &f->seq);
				}
			} else if (p->kind == FIG_KIND_OBJ) {
				p->anim = f->anims[f->nr_anims++] = (FIGANIM_PTR)FigAnim_ctor(0, &f->seq, 1);
				memset(&f->seq, 0, sizeof(f->seq));
				f->seq.figure = 0x91;
				f->seq.step[0].frame = 0xda;
				f->seq.step[0].type = 3;
				f->seq.step[0].count = 0xff;
				f->seq.step[1].frame = 0xff;
				seq_set(p->anim, &f->seq);
				p->anim->delay_fix = 0x0f;
			} else {
				hero = fig_get_hero(g_group, p->id);
				p->anim = f->anims[f->nr_anims++] = (FIGANIM_PTR)FigAnim_ctor(0, &f->seq, 1);
				fig_stand_pic(f, hero, &f->seq);
			}
		}
	} while ((p = p->next) != 0);
	fig_list_purge(&f->list);
	fig_list_resort(&f->list);
	Lst_init(g_fight_list);
	screensaver_reset(&g_saver);
	txt_load_list(g_font, 0x25, 0x6c, &f->txt_buf, &f->txt);
	g_fight_screen_on = 1;
	g_fig_redraw_all = 1;
	fight_draw(f);
}

FIGANIM_PTR fight_new_extra_anim(void far *unused)
{
	signed short i;
	struct ani_seq seq;

	memset(&seq, 0, sizeof(seq));
	for (i = 0; i < 10; i++) {
		if (!g_fig_extra_anims[i]) {
			g_fig_extra_anims[i] = (FIGANIM_PTR)FigAnim_ctor(0, &seq, 0);
			break;
		} else if (i == 9) {
			/* alle belegt: Platz 0 ersetzen */
			i = 0;
			FigAnim_dtor(g_fig_extra_anims[0], 3);
			g_fig_extra_anims[i] = (FIGANIM_PTR)FigAnim_ctor(0, &seq, 0);
			break;
		}
	}
	return g_fig_extra_anims[i];
}
