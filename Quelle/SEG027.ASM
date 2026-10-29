;===========================================================================
; SEG027.ASM - Reisekarte: sichtbaren Ausschnitt nach A000 kopieren
; Sternenschweif (DSA 2), Attic 1994 - Rekonstruktion, bytegleich mit
; SCHWEIF.EXE C1.02 (TASM 3.1 /ml)
;
; Inhalt:    tmap_blit_rows. Der Kartenpuffer der Reisekarte (O187) haelt
;            12 Kachelzeilen zu 20 Bildzeilen, je 288 Bytes breit, als
;            Ringpuffer. rows[] sagt, welche Pufferzeile auf welcher
;            Bildschirmzeile steht. Gezeigt werden 240 x 200 Punkte: oben
;            20 - fine_y Zeilen der ersten Kachelzeile, dann 9 volle, unten
;            fine_y Zeilen der elften.
; Aufrufer:  O187 (Travel-Karte: draw, scroll_*), far
; Aufruf:    far, C: void tmap_blit_rows(char far *buf, unsigned char far
;            *rows, short fine_y). Stackrahmen bp: [bp+6] buf, [bp+0ah] rows,
;            [bp+0eh] fine_y.
; Register:  DS, ES, SI, DI gesichert; AX, BX, CX, DX, EAX veraendert.
; Daten:     g_tmap_src, g_tmap_row, g_tmap_row_h, g_tmap_row_ofs (eigenes
;            Datenstueck DMR27, ds:8668..8692).
; Herkunft:  aus SCHWEIF.EXE R27 (ab +0xa) mit asm2tasm.py zerlegt;
;            ;@nnnn = Offset im Segment R27.
;===========================================================================
	.386p
; Daten im Datensegment (Stueck DMR27); Adressen als Konstanten, damit der
; Code ohne Fixups bleibt wie im Original
g_tmap_src	equ	08668h	; dd  Quellzeiger der aktuellen Bildzeile (normalisiert)
g_tmap_row	equ	0866ch	; dw  laufende Bildschirm-Kachelzeile 0..10
g_tmap_row_h	equ	0866eh	; db 11 dup  sichtbare Zeilen je Kachelzeile (20, [0] = 20 - fine_y, [10] = fine_y)
g_tmap_row_ofs	equ	0867ah	; dw 12 dup  Offset der Kachelzeile k im Puffer (k * 20 * 288)
TMAP_SRC_W	equ	288	; Pufferbreite in Bytes
TMAP_VIEW_W	equ	240	; sichtbare Breite
	locals
SEG027_TEXT	segment word public use16 'CODE'
	assume	cs:SEG027_TEXT, ds:nothing
	public	F_R27_000A
; ---------------------------------------------------------------------------
; tmap_blit_rows - sichtbaren Kartenausschnitt auf den Bildschirm
; Ein:  [bp+6]   char far *buf    Kartenpuffer, schon um view_x verschoben
;       [bp+0ah] unsigned char far *rows  Pufferzeile je Bildschirmzeile
;                (benutzt werden rows[1..11])
;       [bp+0eh] short fine_y     Feinverschiebung 0..19 in Bildzeilen
; Aus:  -
; Fuer jede der 11 Bildschirm-Kachelzeilen r: Quelle = buf + ofs[rows[r+1]],
; bei r < 10 um (20 - h[r]) Zeilen weiter (oben abgeschnittene Zeilen),
; dann h[r] Zeilen zu 240 Bytes nach A000:di (Zielzeile 320, Quelle 288).
; Zeiger werden mit EMS_norm_ptr (R36) normalisiert.
; ---------------------------------------------------------------------------
F_R27_000A	label	far	; Linkname von tmap_blit_rows
tmap_blit_rows:
	push	bp	;@000a
	mov	bp,sp	;@000b
	push	ds	;@000d
	push	es	;@000e
	push	si	;@000f
	push	di	;@0010
	push	dword ptr [bp+6]	;@0011 buf
	push	cs	;@0015
	db	0e8h,083h,0ah	; call 0xa9c	;@0016 call EMS_norm_ptr (R36:00BC, near ueber den gemeinsamen Rahmen; als Bytes, weil das Ziel in einem anderen Modul liegt)
	add	sp,4	;@0019
	mov	[bp+6],ax	;@001c buf normalisiert zurueckschreiben
	mov	[bp+8],dx	;@001f
	mov	bx,[bp+0eh]	;@0022
	mov	ds:[g_tmap_row_h+10],bl	;@0025 unterste Kachelzeile: fine_y Zeilen
	mov	ax,014h	;@0029
	sub	ax,bx	;@002c
	mov	ds:[g_tmap_row_h],al	;@002e oberste Kachelzeile: 20 - fine_y Zeilen
	mov	word ptr ds:[g_tmap_row],0	;@0031
	xor	di,di	;@0037 DI = Bildschirmoffset in A000
@@next_row:
	push	dword ptr [bp+6]	;@0039 Quelle = buf
	pop	dword ptr ds:[g_tmap_src]	;@003d
	mov	bx,ds:[g_tmap_row]	;@0042
	les	si,[bp+0ah]	;@0046
	mov	bl,es:[bx+si+1]	;@0049 BL = rows[r + 1]
	mov	bh,0	;@004d
	add	bx,bx	;@004f
	movzx	eax,word ptr [bx+g_tmap_row_ofs]	;@0051 + Offset dieser Pufferzeile
	add	ds:[g_tmap_src],eax	;@0057
	push	dword ptr ds:[g_tmap_src]	;@005c
	push	cs	;@0061
	db	0e8h,037h,0ah	; call 0xa9c	;@0062 call EMS_norm_ptr (R36:00BC)
	add	sp,4	;@0065
	mov	ds:[g_tmap_src],ax	;@0068
	mov	ds:[g_tmap_src+2],dx	;@006b
	mov	bx,ds:[g_tmap_row]	;@006f
	cmp	bx,0ah	;@0073 bei r < 10 ...
	jnl	@@no_skip	;@0076
	mov	bl,[bx+g_tmap_row_h]	;@007a
	mov	ax,014h	;@007e
	sub	al,bl	;@0081
	mov	bx,0120h	;@0083 ... (20 - h) * 288 Bytes ueberspringen
	mul	bx	;@0086
	add	ds:[g_tmap_src],ax	;@0088
@@no_skip:
	mov	bx,ds:[g_tmap_row]	;@008c
	xor	dx,dx	;@0090
	mov	dl,[bx+g_tmap_row_h]	;@0092 DX = h[r] = Zeilenzahl
	cmp	dx,0	;@0096 keine sichtbare Zeile?
	jz	@@row_done	;@0099
	push	ds	;@009d
	lds	si,ds:[g_tmap_src]	;@009e DS:SI = Quelle
	push	0a000h	;@00a2
	pop	es	;@00a5
@@copy_line:
	cld	;@00a6
	mov	cx,TMAP_VIEW_W/4	;@00a7 240 Bytes als Doppelworte
	rep	movsd	;@00aa
	add	di,050h	;@00ad Ziel: naechste Bildschirmzeile (320 - 240)
	add	si,030h	;@00b0 Quelle: naechste Pufferzeile (288 - 240)
	dec	dx	;@00b3
	jnz	@@copy_line	;@00b4
	pop	ds	;@00b6
@@row_done:
	inc	word ptr ds:[g_tmap_row]	;@00b7
	cmp	word ptr ds:[g_tmap_row],0bh	;@00bb 11 Kachelzeilen
	jnz	@@next_row	;@00c0
	pop	di	;@00c4
	pop	si	;@00c5
	pop	es	;@00c6
	pop	ds	;@00c7
	mov	sp,bp	;@00c8
	pop	bp	;@00ca
	retf	;@00cb
SEG027_TEXT	ends
	end
