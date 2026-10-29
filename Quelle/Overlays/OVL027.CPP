/* OVL027 - Overlay O027 (Segment 0x1f89 in SCHWEIF.EXE): Vorspann beim Programmstart, Klasse Opening (s_class.h,
 * 0x0c Bytes: abgebrochen done, EMS ems, Handle handle (long, -1 = keins), Archiv ausgelagert swapped, Groesse kb);
 * Aufruf aus main (O032: new/run/delete, ohne Destruktor). run(): load_files (Titelbild 0x436, Dateiliste
 * g_cl_opening in den CD-Cache, Esc bricht ab) -> Ende, sonst Musik 0x2d, bei g_speech_ext_avail die FLI-Fassung
 * opening_play_fli (O028), sonst play() mit den Szenen scene1 (Bild 0x41b), scene2 (0x41c), scene3 (0x41d mit
 * Wellen-Effekt waves ueber die Tabelle 0x3a6 aus g_file_main), scene4 (0x436), scene5 (Animation 0x437..0x43b
 * im EMS/XMS-Bildspeicher, Paletten-Ueberblendung pal_mix), scene6 (Bilder 0x41e..0x420/0x429 mit Schriftzuegen
 * scroll_text), scene7 (0x421), scene8 (Bilder 0x422..0x428/0x43c ueber pic_to_bmp).
 * Hilfen: scroll_text(n) Schriftzug 0x42a+n herein- und hinausschieben, waves Wellen, copy_line eine Zeile (asm rep
 * movsw), store_put/store_get Bild in/aus dem Bildspeicher (Auftrag g_xms_move, EMS_copy bzw. xms_move_any),
 * store_alloc/store_free Bildspeicher anfordern/freigeben (xms_free_kb/xms_alloc_kb/xms_free_block bzw. ems_alloc/
 * ems_free, Auslagern swap_save_file/swap_alloc, arc_mark/arc_release auf g_file_3), blend (pal_blend2), fade_in
 * (dissolve_in), wait(n) n Bildwechsel, poll_key (Alt-Q -> sys_exit/exit, sonst done = 1).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y -E..\bc\BIN\TASM.EXE OVL027.CPP   -> 6247/6247 Bytes GLEICH (18.10.2026),
 *   cobjfix laeuft. Ohne -Og fehlen die Epilogkopien in store_put/store_get; ohne -Oi (memcpy/memset als Aufrufe).
 *   (Namensfassung mit schweif.h, Welle N20: 19.10.2026 GLEICH, LINKGLEICH)
 * - Lokale Bild-Objekte (Pic) und Bmp: "Bmp b; Pic pic;" (Bmp 10 Bytes bleibt oben, Pic ans Ende);
 *   new Bmp(w, h) -> gb_new_alloc(0, w, h), delete -> gb_delete(b, 3); Puffer ueber new unsigned char[n].
 * - store_put/store_get: erster Parameter als unsigned long deklariert -> Aufrufer pushen far-Zeiger per push
 *   dword ((unsigned long)g_vga_buf bzw. buf), Rueckgabewerte von b->data() als push dx / push ax.
 * - waves: "t >> 0" ergibt wirklich sar ax,0; "tab[i] * s / c1" (char-Faktor zuerst geladen); p als huge-
 *   Zeiger mit unsigned-Versatz -> xor cx,cx / PADD@; "s -= 3" -> add al,0fdh.
 * - copy_line: im Original Assembler (push es/ds, les/lds, rep movsw), hier asm; BCC ergaenzt push si/di.
 * - Paletten als unsigned char far * (Puffer 0x300 Bytes), an die O034-Funktionen als struct srgb far * gecastet.
 * Auffaelligkeiten: der Konstruktor setzt ems nicht - store_put/store_get werten es in scene5/scroll_text aus,
 *   bevor store_alloc es nur im EMS-Fall setzt. scene6 legt ein unbenutztes lokales Bmp an.
 */
#include <mem.h>
#include <bios.h>
#include <stdlib.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:184a..1868, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
signed short g_cl_opening[13] = { -24574, -24573, -24572, -24571, -24570, -24569, -24568, -24567, -24566, -24565, 9245, 9126, -1 };

#define PAL_SIZE	0x300
#define SCREEN_SIZE	0xfa00		/* 320 x 200 */
#define KEY_ALT_Q	0x1011
#define MUSIC_OPENING	0x2d
#define PIC_TITLE	0x436		/* Titelbild */
#define PIC_TEXT0	0x42a		/* Schriftzuege 0x42a.. (scroll_text) */
#define WAVE_TAB	0x3a6		/* Wellentabelle (0x200 Worte) in g_file_main */

#define SRGB(p)		((struct srgb far *)(p))

Opening::Opening()
{
#if FIX_0172
	/* Fix #172: EMS-Merker im Konstruktor setzen */
	done = swapped = 0;
	ems = 0;
	handle = -1L;
#else
	done = swapped = 0;
	handle = -1L;
#endif
	kb = 0x140;
}

void Opening::run()
{
	if (load_files()) {
		cache_purge(g_cd_cache, 0);
		return;
	}
	snd_play(g_snd_music, MUSIC_OPENING);
	if (g_speech_ext_avail)
		opening_play_fli(this);
	else
		play();
	cache_purge(g_cd_cache, 0);
	clear_screen_buf(g_vga_buf);
	snd_obj_stop(g_snd_music);
	Sound_stop_music(g_snd_music);
}

void Opening::play()
{
	scene1();
	if (!done)
		scene2();
	if (!done)
		scene3();
	if (!done)
		scene4();
	if (!done)
		scene5();
	if (!done)
		scene6();
	if (!done)
		scene7();
	if (!done)
		scene8();
}

void Opening::scene1()
{
	Pic pic;
	unsigned char far *p1 = new unsigned char[PAL_SIZE];
	unsigned char far *p2 = new unsigned char[PAL_SIZE];

	memset(p1, 0, PAL_SIZE);
	set_palette(p1, 0, 0x100);
	pic.load(0x41b, g_vga_buf);
	memcpy(p2, pic.get_palette(), 0x2f7);
	blend(p1, p2, 0x32, 0xfd, 1);
	wait(0x50);
	memcpy(p1, pic.get_palette(), PAL_SIZE);
	blend(p2, p1, 0x32, 0x100, 1);
	wait(0x104);
	memset(p2, 0x3f, PAL_SIZE);
	blend(p1, p2, 0x19, 0x100, 1);
	delete p2;
	delete p1;
}

void Opening::scene2()
{
	Pic pic;
	unsigned char far *p1 = new unsigned char[0x36];
	unsigned char far *p2 = new unsigned char[0x36];

	memset(p1, 0x3f, 0x36);
	set_palette(p1, 0, 0x12);
	pic.load(0x41c, g_vga_buf);
	memcpy(p2, pic.get_palette(), 0x36);
	memset(p2, 0, 0x30);
	blend(p1, p2, 0x19, 0x12, 1);
	wait(0x78);
	memcpy(p1, p2, 0x36);
	memset(p2, 0, 0x36);
	memcpy(p2, pic.get_palette(), 0x30);
	blend(p1, p2, 0x32, 0x12, 1);
	wait(0x104);
	memcpy(p1, p2, 0x36);
	memset(p2, 0, 0x36);
	blend(p1, p2, 0x32, 0x12, 1);
	clear_screen_buf(g_vga_buf);
	delete p2;
	delete p1;
}

void Opening::scene3()
{
	Pic pic;
	unsigned char far *p1 = new unsigned char[0x66];
	unsigned char far *p2 = new unsigned char[0x66];
	unsigned char far *buf = new unsigned char[SCREEN_SIZE];

	pic.load(0x41d, buf);
	memcpy(p2, pic.get_palette(), 0x66);
	memcpy(p1, p2, 0x60);
	memset(p1, 0, 0x30);
	set_palette(p1, 0, 0x22);
	waves(buf);
	blend(p1, p2, 0x19, 0x20, 1);
	wait(0x12c);
	memcpy(p1, p2, 0x30);
	blend(p1, p2, 0x32, 0x22, 1);
	wait(0x12c);
	memset(p1, 0, 0x66);
	blend(p2, p1, 0x32, 0x22, 1);
	clear_screen_buf(g_vga_buf);
	delete buf;
	delete p2;
	delete p1;
}

void Opening::scene4()
{
	Pic pic;
	unsigned char far *p1 = new unsigned char[PAL_SIZE];
	unsigned char far *p2 = new unsigned char[PAL_SIZE];

	set_palette(p1, 0, 0x100);
	pic.load(PIC_TITLE, g_vga_buf, (struct rgb far *)p2);
	blend(p1, p2, 0x1e, 0x100, 1);
	wait(0x190);
	delete p2;
	delete p1;
}

void Opening::scene5()
{
	Pic pic;
	Bmp far *b = new Bmp(0x140, 0xc8);
	unsigned char far *p1 = new unsigned char[PAL_SIZE];
	unsigned char far *p2 = new unsigned char[PAL_SIZE];
	unsigned char far *p3 = new unsigned char[PAL_SIZE];
	register signed short i;
	register signed short v;

	store_alloc();
	for (i = 0; i < 5; i++) {
		pic.load(i + 0x437, b->data());
		store_put((unsigned long)b->data(), (signed long)i);
	}
	memcpy(p3, pic.get_palette(), PAL_SIZE);
	memset(p1, 0x3f, PAL_SIZE);
	blend(p2, p1, 0x1e, 0x100, 1);
	memcpy(p2, p3, PAL_SIZE);
	for (i = 0; i <= 250; i++) {
		store_get((unsigned long)b->data(), (signed long)(i % 5));
		b->put(g_vga_buf, 0, 0);
		v = i < 50 ? i * 2 : i < 200 ? 100 : (250 - i) * 2;
		if (i == 150)
			memset(p1, 0, PAL_SIZE);
		else
			memset(p3, 0, PAL_SIZE);
		pal_mix(SRGB(p1), SRGB(p2), SRGB(p3), v, 0x100);
		if (i < 200)
			set_palette(p3, 0, 0x100);
		else
			set_palette(p3, 0, 0xc0);
		wait(3);
		if (done)
			break;
	}
	get_palette(p2, 0, 0x100);
	blend(p2, p1, 0x30, 0x100, 1);
	clear_screen_buf(g_vga_buf);
	store_free();
	delete p3;
	delete p2;
	delete p1;
	delete b;
}

void Opening::scene6()
{
	Bmp b;
	Pic pic;
	unsigned char far *p1 = new unsigned char[PAL_SIZE];
	unsigned char far *p2 = new unsigned char[PAL_SIZE];

	set_palette(p1, 0, 0x100);
	clear_screen_buf(g_vga_buf);
	pic.load(0x41e, g_vga_buf, (struct rgb far *)p2);
	blend(p1, p2, 0x19, 0x100, 1);
	scroll_text(0);
	blend(p2, p1, 0x19, 0x100, 1);
	if (!done) {
		clear_screen_buf(g_vga_buf);
		pic.load(0x41f, g_vga_buf, (struct rgb far *)p2);
		blend(p1, p2, 0x19, 0x100, 1);
		scroll_text(1);
		blend(p2, p1, 0x19, 0x100, 1);
	}
	if (!done) {
		clear_screen_buf(g_vga_buf);
		pic.load(0x420, g_vga_buf, (struct rgb far *)p2);
		blend(p1, p2, 0x19, 0x100, 1);
		scroll_text(2);
		blend(p2, p1, 0x19, 0x100, 1);
	}
	if (!done) {
		clear_screen_buf(g_vga_buf);
		pic.load(0x429, g_vga_buf, (struct rgb far *)p2);
		blend(p1, p2, 0x19, 0x100, 1);
	}
	delete p2;
	delete p1;
}

void Opening::scene7()
{
	unsigned char far *p1 = new unsigned char[PAL_SIZE];
	unsigned char far *p2 = new unsigned char[PAL_SIZE];

	if (!done) {
		Pic pic;
		signed short w;
		signed short h;
		Bmp far *b;
		unsigned char far *buf;

		buf = new unsigned char[SCREEN_SIZE];
		pic.get_size(0x421, &w, &h);
		b = new Bmp(w, h);
		pic.load(0x421, b->data());
		memcpy(p2, pic.get_palette(), PAL_SIZE);
		set_palette(p2, 0, 0xd0);
		memcpy(buf, g_vga_buf, SCREEN_SIZE);
		b->put(buf, 0x2b, 0x13, 4);
		delete b;
		fade_in(buf);
		delete buf;
	}
	scroll_text(3);
	blend(p2, p1, 0x19, 0xd0, 1);
	delete p2;
	delete p1;
}

void Opening::scene8()
{
	Bmp b;
	Pic pic;
	unsigned char far *buf;
	unsigned char far *p1 = new unsigned char[PAL_SIZE];
	unsigned char far *p2 = new unsigned char[PAL_SIZE];
	register signed short i;

	for (i = 1; i < 8; i++) {
		pic_to_bmp(&pic, i + 0x421, &b, SRGB(p2), 0xc0);
		b.put(g_vga_buf, 0x2b, 0x13, 4);
		bmp_free_data(&b);
		blend(p1, p2, 0x19, 0xc0, 1);
		scroll_text(i + 3);
		if (i == 3) {
			buf = new unsigned char[SCREEN_SIZE];
			memcpy(buf, g_vga_buf, SCREEN_SIZE);
			pic_to_bmp(&pic, 0x43c, &b, SRGB(p2), 0x100);
			b.put(buf, 0x48, 0xa5, 4);
			bmp_free_data(&b);
			set_palette(p2 + 0x240, 0xc0, 0x10);
			fade_in(buf);
			delete buf;
		}
		if (i < 7)
			blend(p2, p1, 0x19, 0xc0, 1);
		if (done)
			break;
	}
	buf = new unsigned char[SCREEN_SIZE];
	pic.load(0x429, buf);
	fade_in(buf);
	delete buf;
	scroll_text(0x0b);
	buf = new unsigned char[SCREEN_SIZE];
	clear_screen_buf(buf);
	fade_in(buf);
	delete buf;
	delete p2;
	delete p1;
}

void Opening::scroll_text(signed short n)
{
	signed short y_mid;
	signed short y_end;
	signed short w;
	signed short h;
	Bmp far *b;
	unsigned char far *buf;
	register signed short y;
	register signed short x;

	if (!done) {
		Pic pic;

		buf = new unsigned char[SCREEN_SIZE];
		kb = 0x40;
		store_alloc();
		store_put((unsigned long)g_vga_buf, 0L);
		pic.get_size(n + PIC_TEXT0, &w, &h);
		b = new Bmp(w, h);
		pic.load(n + PIC_TEXT0, b->data());
		x = 0xa0 - w / 2;
		y_mid = 100 - h / 2;
		y_end = -h - 8;
		for (y = 199; y >= y_mid; y -= 8) {
			store_get((unsigned long)buf, 0L);
			b->put(buf, x, y, 4);
			wait(1);
			memcpy(g_vga_buf, buf, SCREEN_SIZE);
			if (done)
				break;
		}
		wait(h * 28 / 16 + 120);
		if (!done) {
			for (y = y_mid; y >= y_end; y -= 8) {
				store_get((unsigned long)buf, 0L);
				b->put(buf, x, y, 4);
				wait(1);
				memcpy(g_vga_buf, buf, SCREEN_SIZE);
				if (done)
					break;
			}
		}
		store_free();
		delete buf;
		delete b;
	}
}

void Opening::waves(unsigned char far *src)
{
	signed char s_old;
	unsigned short src_off;
	unsigned short dst_off;
	unsigned char huge *p = src;
	signed short far *tab = (signed short far *)new unsigned char[0x400];
	signed short t = 0;
	signed char s = 0x1e;
	register signed short i;
	register signed short y;

	file_open(g_file_main, WAVE_TAB);
	file_read(g_file_main, tab, 0x400L);
	file_close(g_file_main);
	do {
		t++;
		if (t > 0xbd) {
			t = 0xbd;
			if (s > 0) {
				s_old = s;
				s -= 3;
			}
			for (i = 0; i < 0x200; i++)
				tab[i] = tab[i] * s / s_old;
		}
		wait(2);
		if (done)
			break;
		for (i = 0; i < 200; i++) {
			y = i + (t >> 0) + tab[(t * 22 + i * 9) & 0x1ff] - 0xbd;
			if (y < 0 || y > 0xbd)
				y = 0xbe;
			src_off = y * 320;
			dst_off = i * 320;
			copy_line((unsigned char far *)g_vga_buf + dst_off, p + src_off);
		}
	} while (s != 0);
	delete tab;
}

void Opening::copy_line(void far *dst, void far *src)
{
	asm push es
	asm push ds
	asm les di,dst
	asm lds si,src
	asm mov cx,0a0h
	asm rep movsw
	asm pop ds
	asm pop es
}

void Opening::store_put(unsigned long src, signed long n)
{
	g_xms_move.len = (signed long)SCREEN_SIZE;
	g_xms_move.src_handle = 0;
	g_xms_move.src = src;
	g_xms_move.dst_handle = handle;
	g_xms_move.dst = n * (signed long)SCREEN_SIZE;
	if (ems == 1)
		EMS_copy(&g_xms_move);
	else
		xms_move_any(&g_xms_move);
}

void Opening::store_get(unsigned long dst, signed long n)
{
	g_xms_move.len = (signed long)SCREEN_SIZE;
	g_xms_move.src_handle = handle;
	g_xms_move.src = n * (signed long)SCREEN_SIZE;
	g_xms_move.dst_handle = 0;
	g_xms_move.dst = dst;
	if (ems == 1)
		EMS_copy(&g_xms_move);
	else
		xms_move_any(&g_xms_move);
}

void Opening::store_alloc()
{
	signed short h;

	if (xms_free_kb() <= kb) {
		swap_save_file();
		swapped = 1;
	}
	handle = xms_alloc_kb(kb);
	if (handle < 0) {
		swap_save_file();
		swapped = 1;
		handle = xms_alloc_kb(kb);
	}
	if (swapped == 1)
		arc_mark(g_file_3);
	if (handle < 0 && g_ems_avail == 1) {
		ems = 1;
		ems_alloc((signed short)((((signed long)kb << 10) + 0x3fff) >> 14), &h);
		g_ems_frame = ems_frame_seg();
		handle = h;
	}
}

void Opening::store_free()
{
	if (ems == 1)
		ems_free(handle);
	else
		xms_free_block(handle);
	handle = -1L;
	ems = 0;
	if (swapped == 1) {
		arc_release(g_file_3);
		swapped = 0;
		swap_alloc();
	}
}

void Opening::blend(unsigned char far *from, unsigned char far *to, signed short steps, signed short n,
	signed short f)
{
	if (!done)
		pal_blend2(SRGB(from), SRGB(to), steps, n, f);
}

void Opening::fade_in(unsigned char far *buf)
{
	if (!done)
		dissolve_in(buf);
}

void Opening::wait(register signed short n)
{
	while (!done && --n >= 0)
		poll_key();
}

signed short Opening::poll_key()
{
	register signed short key = 0;

	if (bioskey(1)) {
		key = bioskey(0);
		if (key == KEY_ALT_Q) {
			sys_exit(0);
			exit(0);
		}
		if (key)
			done = 1;
	}
	game_idle();
	wait_for_vsync();
	return key;
}

signed short Opening::load_files()
{
	Pic pic;
	unsigned char far *p1 = new unsigned char[PAL_SIZE];
	unsigned char far *p2 = new unsigned char[PAL_SIZE];
	signed short aborted;

	set_palette(p1, 0, 0x100);
	pic.load(PIC_TITLE, g_vga_buf, (struct rgb far *)p2);
	blend(p1, p2, 0x1e, 0x100, 1);
	aborted = load_pic_list(g_cd_cache, g_cl_opening, "\xaa\xaa\xaa", 0, 1);
	pal_blend(SRGB(p2), 0x19);
	delete p2;
	delete p1;
	return aborted;
}
