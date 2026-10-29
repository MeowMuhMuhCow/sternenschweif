/* OVL004 - Overlay O004 (Segment 0x1f09 in SCHWEIF.EXE): CD-Audio ueber MSCDEX (int 2fh, ax = 15xxh), Klasse CD
 * (s_class.h, 0x1f Bytes; *g_cd). Freie Funktionen: cd_drive_reset(drv) (IOCTL-Ausgabe 0x0c mit Code 2),
 * cd_get_label(drv, label) Datentraegername lesen (1505h, 0x800 Bytes; bei Fehler Reset, delay(4000), zweiter
 * Versuch; Name +28h, 11 Zeichen), cd_check_drive(drv) Laufwerkspruefung (150bh, bx == 0adadh). Methoden:
 * msf2hsg ((m * 60 + s) * 75 + f - 150), ndrives/version/firstdrive (MSCDEX), request (1510h mit rq), status
 * (IOCTL 6: Tuer offen/keine CD -> open, IOCTL 0x0f: Pause -> paused, Status-Bit 0x200: spielt -> busy),
 * read_disc (CD 1/2 nach dem Datentraegernamen g_cd_label1/g_cd_label2, Disk-Info 0x0a, Titel-Info 0x0b je Titel
 * nach tr[], Ende mit 0xff und Lead-out), Konstruktor CD(drive) (Puffer 7/0xb6/0x1a/0x8e Bytes, Fehler
 * CD_ERR_NO_DRIVE/CD_ERR_MSCDEX_OLD/CD_ERR_NOT_CD, Musiktabelle aus Datei 0x1cf), Destruktor, change (CD-Wechsel,
 * CD_ERR_OPEN bei offener Tuer), stop (0x85 bis nicht mehr spielend/pausiert), play(t) (Lautstaerke g_vol_music,
 * 0x84 mit Start/Laenge, Ende endtime = g_random_seed2 + Laenge * 18 / 75 - 10, CD_ERR_TRACK/CD_ERR_PLAY),
 * music(n) Musikstueck n (Tabelle mu: Titel/CD-Nr.; falsche CD -> Meldung g_ttx[476] per dlg_choose, warten, bis
 * change die richtige meldet), update (Wiederholung nach Ablauf), closed (1 bei geschlossener Tuer), volume(v)
 * 0..100 -> 0..255 (IOCTL 0x0c mit Steuerblock g_cd_volctl, 9 Bytes).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y -E..\bc\BIN\TASM.EXE OVL004.CPP   -> 3050/3050 Bytes GLEICH (18.10.2026),
 *   cobjfix laeuft. Ohne -Og 14 Bytes zu lang, mit -Oi werden memset/strcpy/strcmp eingebaut (falsch).
 *   (Namensfassung mit schweif.h, Welle N20: 19.10.2026 GLEICH, LINKGLEICH)
 * - Inline-Assembler ist hier ORIGINAL (kein Notbehelf): die MSCDEX-Aufrufe sind TASM-kodiert (xor bx,bx =
 *   31 db, mov ax,bx = 89 d8; BCC selbst erzeugt 33/8b), mit push es / pop es und Kopien der Operanden in
 *   Locals (d, p, r), weil asm nicht an this->x kommt. Sprung "asm jc l1" auf eine C-Marke geht.
 * - asm-Funktionen mit Ergebnis in AX ohne return (Warnung "should return a value"): "return _AX;" ergibt mit
 *   -Og eine zusaetzliche tote Epilogkopie.
 * - MSF -> HSG: jedes Byte als (unsigned long) -> movzx eax/edx; ohne Cast mov dl / mov dh,0 / movsx edx,dx.
 * - ndrives() signed (cmp ax,1 / jnl), version() unsigned (cmp ax,20ah / jnc).
 * - "struct cd_volctl c = g_cd_volctl;" (9 Bytes) -> vier mov ax / mov [bp-x],ax und ein Byte (kein rep movsw).
 * - play: Start als (void far *)msf2hsg(..) in den Zeigerplatz rq->ptr (shld edx / zwei mov word), Laenge
 *   ueber Zeiger "*(unsigned long far *)&rq->count" (Local [bp-4]); len unsigned long (div ebx);
 *   &tr[t - 1].start ergibt t * 7 + tr - 5, &tr[t].start dagegen tr + t * 7 + 2 (push/pop um imul).
 * - closed(): "return open == 0 ? 1 : 0;" -> mov ax,1 / jmp / xor ax,ax, ein toter Epilog.
 * - Originalfehler: keine gefunden (new-Ergebnisse der Teilpuffer im Konstruktor werden nicht geprueft).
 */
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <mem.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:00ce..00f0, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
struct cd_volctl g_cd_volctl = { { 0x03, 0x00, 0x00, 0x01, 0x00, 0x02, 0x00, 0x03, 0x00 } };

#define MUSIC_TAB_FILE	0x1cf		/* Musiktabelle (71 Eintraege) im Hauptarchiv */

void cd_drive_reset(signed short drv)
{
	signed short d;
	struct cd_req far *p;
	unsigned char c = 2;
	struct cd_req r;

	memset(&r, 0, 0x1a);
	r.len = 0x1a;
	r.cmd = 0x0c;
	r.ptr = &c;
	r.count = 1;
	p = &r;
	d = drv;
	asm push es
	asm mov ax, 1510h
	asm mov cx, d
	asm les bx, p
	asm int 2fh
	asm pop es
}

signed short cd_get_label(register signed short drv, char far *label)
{
	signed short r;
	char far *buf;

	buf = new char[0x800];
	r = drv;
	asm push es
	asm mov ax, 1505h
	asm mov cx, r
	asm les bx, buf
	asm mov dx, 0
	asm int 2fh
	asm pop es
	asm mov word ptr r, 1
	asm jc l1
	asm dec word ptr r
l1:
	if (r != 0) {
		cd_drive_reset(drv);
		delay(4000);
		r = drv;
		asm push es
		asm mov ax, 1505h
		asm mov cx, r
		asm les bx, buf
		asm mov dx, 0
		asm int 2fh
		asm pop es
		asm mov word ptr r, 1
		asm jc l2
		asm dec word ptr r
	}
l2:
	if (r == 0) {
		buf[0x33] = 0;
		strcpy(label, buf + 0x28);
	} else
		*label = 0;
	delete buf;
	return r;
}

signed short cd_check_drive(signed short drv)
{
	asm mov ax, 150bh
	asm xor bx, bx
	asm mov cx, drv
	asm int 2fh
	asm cmp bx, 0adadh
	asm jz l3
	asm xor ax, ax
l3:
}

signed long CD::msf2hsg(unsigned char far *msf)
{
	signed long r;

	r = ((unsigned long)msf[2] * 60 + (unsigned long)msf[1]) * 75 + (unsigned long)msf[0] - 150;
	return r;
}

signed short CD::ndrives()
{
	asm mov ax, 1500h
	asm xor bx, bx
	asm int 2fh
	asm mov ax, bx
}

unsigned short CD::version()
{
	asm mov ax, 150ch
	asm xor bx, bx
	asm int 2fh
	asm mov ax, bx
}

unsigned short CD::firstdrive()
{
	asm mov ax, 1500h
	asm xor bx, bx
	asm xor cx, cx
	asm int 2fh
	asm mov ax, cx
}

void CD::request()
{
	struct cd_req far *p;
	signed short d;

	p = rq;
	d = drive;
	rq->status = 0;
	asm push es
	asm mov ax, 1510h
	asm mov cx, d
	asm les bx, p
	asm int 2fh
	asm pop es
}

void CD::status()
{
	unsigned char b[12];

	rq->cmd = 3;
	rq->ptr = b;
	b[0] = 6;
	rq->count = 5;
	request();
	if ((b[1] & 1) || (b[2] & 8))
		open = 1;
	else
		open = 0;
	if (open == 1) {
		busy = 0;
		paused = 0;
		return;
	}
	b[0] = 0x0f;
	rq->count = 0x0b;
	request();
	if (b[1] & 1)
		paused = 1;
	else
		paused = 0;
	if (rq->status & 0x200)
		busy = 1;
	else
		busy = 0;
}

void CD::read_disc()
{
	signed short i;
	char label[40];

	if (cd_get_label(drive, label)) {
		err = CD_ERR_DISC;
		disc = 0;
		return;
	}
	if (strcmp(label, "DSA2_CD1   ") == 0)
		disc = 1;
	else if (strcmp(label, "DSA2_CD2   ") == 0)
		disc = 2;
	else
		disc = 0;
	if (disc != 0) {
		rq->cmd = 3;
		rq->count = 7;
		rq->ptr = di;
		request();
		if (di->last > 25)
			di->last = 25;
		for (i = di->first - 1; i < di->last; i++) {
			tr[i].code = 0x0b;
			tr[i].no = i + 1;
			rq->ptr = &tr[i];
			request();
		}
		tr[di->last].code = 0x0b;
		tr[di->last].no = 0xff;
		tr[di->last].start = di->leadout;
		tr[di->last].ctrl = 0;
	}
}

CD::CD(register signed short d)
{
	err = 0;
	track = 0;
	disc = 0;
	endtime = 0;
	di = new struct cd_diskinfo;
	tr = new struct cd_track[26];
	rq = new struct cd_req;
	mu = new struct cd_music[71];
	di->code = 0x0a;
	rq->len = 0x1a;
	rq->unit = 0;
	rq->media = 0;
	rq->ptr = 0;
	rq->count = 0;
	rq->f14 = 0;
	rq->f16 = 0;
	if (ndrives() < 1) {
		err = CD_ERR_NO_DRIVE;
		return;
	}
	if (version() < 0x20a) {
		err = CD_ERR_MSCDEX_OLD;
		return;
	}
	if (d < 0)
		drive = firstdrive();
	else {
		drive = d;
		if (!cd_check_drive(drive))
			err = CD_ERR_NOT_CD;
	}
	if (err == 0) {
		file_open(g_file_main, MUSIC_TAB_FILE);
		file_read(g_file_main, mu, 0x8eL);
		file_close(g_file_main);
	}
}

CD::~CD()
{
	delete mu;
	delete rq;
	delete tr;
	delete di;
}

void CD::change()
{
	err = 0;
	track = 0;
	status();
	if (open == 1) {
		err = CD_ERR_OPEN;
		return;
	}
	stop();
	read_disc();
}

void CD::stop()
{
	err = 0;
	track = 0;
	status();
	while (busy == 1 || paused == 1) {
		rq->cmd = 0x85;
		request();
		status();
	}
}

void CD::play(register signed short t)
{
	unsigned long far *p;
	unsigned long len;

	err = 0;
	if (di->first > t || di->last < t) {
		err = CD_ERR_TRACK;
		return;
	}
	volume(g_vol_music);
	rq->cmd = 0x84;
	rq->ptr = (void far *)msf2hsg((unsigned char far *)&tr[t - 1].start);
	len = msf2hsg((unsigned char far *)&tr[t].start) - (signed long)rq->ptr;
	p = (unsigned long far *)&rq->count;
	*p = len;
	request();
	status();
	if (busy == 0)
		err = CD_ERR_PLAY;
	else
		track = t;
	len *= 18;
	len /= 75;
	endtime = g_random_seed2 + len - 10;
}

void CD::music(register signed short n)
{
	signed short old;
	char far *buf;
	char msg[70];

	err = 0;
	if (mu[n].track != track || mu[n].disc != 0 && mu[n].disc != disc) {
		stop();
		if (mu[n].disc != 0 && mu[n].disc != disc) {
			old = dlg_set16(g_dialog, 5);
			buf = new char[0x300];
			load_palette_arc3b(buf);
			set_palette(buf + 0x1e0, 0xa0, 0x60);
			sprintf(msg, g_ttx[476], mu[n].disc);
			do {
				dlg_choose(g_dialog, msg, 1, g_ttx[241]);
				wait_ticks_key(100);
				change();
				if (err == 0 && mu[n].disc != disc)
					err = CD_ERR_DISC;
			} while (err != 0);
			dlg_set16(g_dialog, old);
			delete buf;
		}
		play(mu[n].track);
	} else
		update();
}

void CD::update()
{
	err = 0;
	if (track > 0 && endtime <= g_random_seed2) {
		endtime = g_random_seed2 + 32;
		status();
		if (busy == 0)
			play(track);
	}
}

signed short CD::closed()
{
	err = 0;
	status();
	return open == 0 ? 1 : 0;
}

void CD::volume(signed short v)
{
	struct cd_volctl c = g_cd_volctl;

	err = 0;
	if (v < 0)
		v = 0;
	if (v > 100)
		v = 100;
	v *= 255;
	v /= 100;
	c.b[2] = v;
	c.b[4] = v;
	rq->cmd = 0x0c;
	rq->ptr = &c;
	rq->count = 9;
	request();
}
