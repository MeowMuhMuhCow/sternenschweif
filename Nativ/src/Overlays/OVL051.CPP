/* OVL051 - Overlay O051 (Segment 0x2029 in SCHWEIF.EXE): Animationsobjekt Anim (0xaf Bytes, aktuelles Objekt
 * g_anim_cur) mit Palette (0x300 Bytes) und zwei Bildobjekten Bmp (bmp +0x27, bg +0x31): Konstruktor (Datei id,
 * Sequenz seq, from_file; laedt per load, O050) und Destruktor (Speicher freigeben, Puffer nur wenn nicht fremd:
 * bmp_ext/bg_ext), Abfragen size/area/frame_size/rect, Palette setzen (pal_set -> set_palette), Bild- bzw.
 * Hintergrundpuffer aus einem Bmp oder Zeiger uebernehmen (setbuf/setbg) bzw. loesen (clr_buf/clr_bg), start
 * (Puffer w * h anlegen, Abspielflags, g_anim_running = 1), stop, drei Warteschleifen auf running (wait_cue: Marke
 * cue, wait_frame: Bild >= n, next: Marke abwarten und loeschen).
 *
 * BCC -mlarge -3 -Ot -c -Y OVL051.CPP   -> 1668/1668 Bytes GLEICH (18.10.2026), cobjfix laeuft
 * - Member-Objekte: Default-Konstruktor gb_new0 und Destruktor gb_delete(.., 2) erzeugt BCC selbst
 *   (Reihenfolge +0x27, +0x31 bzw. umgekehrt); mem_free gibt hier einen Zeiger zurueck (p = free(p)).
 * - delete frame; -> mem_free_v ohne Nullpruefung (Zeiger auf Struktur ohne Destruktor).
 * - cur = nr_frames = 0; (Kette rechts nach links: +0x1d wird zuerst geschrieben).
 * - if (cue) -> cmp byte; if (!cue) / if (!rate) -> mov al / mov ah,0 / or ax,ax.
 * - Konstruktor: from_file im Original als unsigned char definiert; mit dem Wortparameter der Klasse (s_class.h,
 *   Sicht der Aufrufer) gleicher Code (nur das Low-Byte wird gespeichert).
 *   (Namensfassung mit schweif.h, Welle N19: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 */
#include "schweif.h"

Anim::Anim(signed short id_, signed short seq_, signed short from_file_)
{
	id = id_;
	seq = seq_;
	from_file = from_file_;
	bmp_ext = 0;
	bg_ext = 0;
	cur = nr_frames = 0;
	loop = 0;
	g_anim_running = 0;
	dst = g_vga_buf;
	data = 0;
	bmp_buf = 0;
	bg_buf = 0;
	frame = 0;
	pal = (unsigned char far *)mem_alloc(0x300);
	g_anim_cur = this;
	g_anim_no_skip = 0;
	buf_size = 0;
	load();
}

Anim::~Anim()
{
	if (pal)
		pal = (unsigned char far *)mem_free(pal);
	if (data)
		data = mem_free(data);
	if (bmp_buf && bmp_ext == 0)
		bmp_buf = mem_free(bmp_buf);
	if (bg_buf && bg_ext == 0)
		bg_buf = mem_free(bg_buf);
	delete frame;
	g_anim_running = 0;
	g_anim_cur = 0;
}

void Anim::size(signed short far *w_, signed short far *h_)
{
	*w_ = w;
	*h_ = h;
}

void Anim::area(signed short far *x_, signed short far *y_, signed short far *w_, signed short far *h_)
{
	*x_ = x;
	*y_ = y;
	*w_ = w;
	*h_ = h;
}

void Anim::frame_size(signed short far *w_, signed short far *h_)
{
	*w_ = frame->w;
	*h_ = frame->h;
}

void Anim::rect(signed short far *x_, signed short far *y_, signed short far *w_, signed short far *h_)
{
	*x_ = frame->x;
	*y_ = frame->y;
	*w_ = frame->w;
	*h_ = frame->h;
}

void Anim::pal_set(signed short first, signed short n)
{
	set_palette(pal + first * 3, first, n);
}

void Anim::setbuf(Bmp far *b)
{
	signed short w_;
	signed short h_;

	clr_buf();
	b->size(&w_, &h_);
	bmp_ext = 1;
	bmp_buf = b->data();
	bmp.init(w_, h_, bmp_buf);
}

void Anim::setbuf(signed short w_, signed short h_, void far *p)
{
	clr_buf();
	bmp_ext = 1;
	bmp_buf = p;
	bmp.init(w_, h_, bmp_buf);
}

void Anim::setbg(Bmp far *b)
{
	signed short w_;
	signed short h_;

	clr_bg();
	b->size(&w_, &h_);
	bg_ext = 1;
	bg_buf = b->data();
	bg.init(w_, h_, bg_buf);
}

void Anim::setbg(signed short w_, signed short h_, void far *p)
{
	clr_bg();
	bg_ext = 1;
	bg_buf = p;
	bg.init(w_, h_, bg_buf);
}

void Anim::clr_buf()
{
	bmp_ext = 0;
	bmp.free_lines();
}

void Anim::clr_bg()
{
	bg_ext = 0;
	bg.free_lines();
}

void Anim::start(signed short x_, signed short y_, signed short no_skip)
{
	if (x_ >= 0)
		x = x_;
	if (y_ >= 0)
		y = y_;
	if (bmp_buf == 0) {
		bmp_buf = mem_alloc((signed long)w * h);
		bmp.init(w, h, bmp_buf);
	}
	if (nr_frames) {
		if (!rate)
			rate = 1;
		rounds = 0;
		cue = 0;
		g_anim_timer = rate;
		f24 = 0;
		g_anim_running = 1;
		g_anim_no_skip = no_skip;
	}
}

void Anim::stop()
{
	g_anim_running = 0;
	bmp.free_lines();
}

void Anim::wait_cue()
{
	while (running() & 1) {
		poll_input();
		if (cue)
			return;
	}
}

void Anim::wait_frame(signed short n)
{
	while (running() & 1) {
		poll_input();
		if (cur >= n)
			return;
	}
}

void Anim::next()
{
	while (running() & 1) {
		poll_input();
		if (!cue)
			break;
	}
	cue = 0;
}
