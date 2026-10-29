/* OVL243 - Overlay O243 (Segment 0x2457 in SCHWEIF.EXE): Arbeitsdatei-Klasse TempFile (4 Bytes: fd, access; s_class.h,
 * Objekt g_ofile ds:b635) ueber das Archiv g_file_main (O037) und DOS-Dateien mit Namen sprintf(g_path_fmts[0]
 * = "..\TEMP\%s", file_getname(id)). extract(id): Archiveintrag in Bloecken (0xa000 bzw. 0x2800 Bytes je nach
 * farcoreleft) per _creat/_write auf Platte schreiben (Fehlertext g_ttx[455]), sonst leere Datei; Ergebnis = Datei
 * laesst sich oeffnen. open(id) (O_RDONLY | O_BINARY) / open(id, access) / open(id, access, mode): Datei oeffnen,
 * bei Fehlschlag erst extrahieren; load = oeffnen + ganz lesen (read_big mit filelength) + schliessen; read/write
 * (write_big, Fehlertext bei Kurzschreiben); close mit DOS-Fn 0Dh (Puffer leeren), wenn access Bit O_RDONLY fehlt;
 * length()/length(id) ueber filelength; seek ueber lseek.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL243.CPP   -> 1506/1506 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h, Klasse TempFile in s_class.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Konstruktor mit leerem Rumpf (new(4), jz auf das naechste Label, toter Epilog).
 * - Kein Inline-Assembler: mov ah,0dh / int 21h = _AH = 0x0d; geninterrupt(0x21).
 * - long-Werte mit -3: shl eax,10h / shrd eax,edx,10h nach Aufrufen, shld edx,eax,10h bei return.
 * - unsigned long fuer got/remain/chunk (ja/jna), written unsigned (movzx eax,di); h = SI, written = DI.
 * - fd = access = 0 als Kette (access zuerst), danach return fd.
 * - -Og noetig fuer extract() (Epilogkopien return 1 / else return 0) und die drei open-Varianten.
 * - _close (__close) in close(), close (_close) in extract; _creat/_write mit Unterstrich; RTL-open/close mit
 *   "::" wegen der gleichnamigen Elemente.
 */
#include <stdio.h>
#include <io.h>
#include <alloc.h>
#include <dos.h>
#include <fcntl.h>
#include "schweif.h"

#define FMT_TEMP_PATH	g_path_fmts[0]		/* "..\TEMP\%s" */
#define TTX_WRITE_ERROR	455			/* g_ttx: Meldung "Schreibfehler" */

TempFile::TempFile()
{
}

signed short TempFile::extract(signed short id)
{
	void far *ptr;
	unsigned long got;
	unsigned long remain;
	unsigned long chunk;
	char name[40];
	register signed short h;
	register unsigned short written;

	if (id != 0 && file_open(g_file_main, id) > 0) {
		chunk = farcoreleft() > 0xc800L ? 0xa000L : 0x2800L;
		remain = file_length2(g_file_main);
		ptr = mem_alloc(chunk);
		sprintf(name, FMT_TEMP_PATH, file_getname(g_file_main, id));
		h = _creat(name, 0);
		do {
			got = file_read(g_file_main, ptr, chunk);
			written = _write(h, ptr, (unsigned short)got);
			if (written != got) {
				dlg_message(g_dialog, g_ttx[TTX_WRITE_ERROR]);
				break;
			}
			remain -= got;
		} while (remain > 0);
		file_close(g_file_main);
		::close(h);
		mem_free(ptr);
	} else {
		sprintf(name, FMT_TEMP_PATH, file_getname(g_file_main, id));
		h = _creat(name, 0);
		::close(h);
	}
	h = ::open(name, O_RDONLY);
	if (h > 0) {
		::close(h);
		return 1;
	} else
		return 0;
}

signed short TempFile::open(signed short id)
{
	char name[40];
	register signed short r;

	r = -1;
	if (fd == 0) {
		sprintf(name, FMT_TEMP_PATH, file_getname(g_file_main, id));
		if ((fd = ::open(name, O_RDONLY | O_BINARY)) <= 0) {
			if (extract(id) == 1) {
				fd = ::open(name, O_RDONLY | O_BINARY);
				r = fd;
			} else {
				fd = 0;
				r = -1;
			}
		} else
			r = fd;
	}
	if (r != -1)
		access = O_RDONLY;
	return r;
}

signed short TempFile::open(signed short id, signed short acc)
{
	char name[40];
	register signed short r;

	r = -1;
	if (fd == 0) {
		sprintf(name, FMT_TEMP_PATH, file_getname(g_file_main, id));
		if ((fd = ::open(name, acc)) <= 0) {
			if (extract(id) == 1) {
				fd = ::open(name, acc);
				r = fd;
			} else {
				fd = 0;
				r = -1;
			}
		} else
			r = fd;
	}
	if (r != -1)
		access = acc;
	return r;
}

signed short TempFile::open(signed short id, signed short acc, unsigned short mode)
{
	char name[40];
	register signed short r;

	r = -1;
	if (fd == 0) {
		sprintf(name, FMT_TEMP_PATH, file_getname(g_file_main, id));
		if ((fd = ::open(name, acc, mode)) <= 0) {
			if (extract(id) == 1) {
				fd = ::open(name, acc, mode);
				r = fd;
			} else {
				fd = 0;
				r = -1;
			}
		} else
			r = fd;
	}
	if (r != -1)
		access = acc;
	return r;
}

signed long TempFile::load(signed short id, void far *buf)
{
	signed long n;

	if (fd == 0) {
		if ((fd = open(id)) != -1) {
			n = read_all(buf);
			close();
			return n;
		} else
			fd = 0;
	}
	return 0;
}

signed long TempFile::read_all(void far *buf)
{
	return read_big(fd, buf, filelength(fd));
}

signed long TempFile::read(void far *buf, signed long len)
{
	return read_big(fd, buf, len);
}

signed long TempFile::write(void far *buf, signed long len)
{
	signed long r;

	r = write_big(fd, buf, len);
	if (r != len)
		dlg_message(g_dialog, g_ttx[TTX_WRITE_ERROR]);
	return r;
}

signed short TempFile::close()
{
	_close(fd);
	if (!(access & O_RDONLY)) {
		_AH = 0x0d;		/* DOS: Puffer zurueckschreiben */
		geninterrupt(0x21);
	}
	fd = access = 0;
	return fd;
}

signed long TempFile::length()
{
	if (fd == 0)
		return 0;
	return filelength(fd);
}

signed long TempFile::length(signed short id)
{
	signed long n;

	open(id);
	n = length();
	close();
	return n;
}

signed long TempFile::seek(signed long off, signed short whence)
{
	return lseek(fd, off, whence);
}
