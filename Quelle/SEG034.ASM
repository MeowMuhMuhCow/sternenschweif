;===========================================================================
; SEG034.ASM - Zeilen und Spalten fuellen, Bildschirmraender sichern
; Sternenschweif (DSA 2), Attic 1994 - Rekonstruktion, bytegleich mit
; SCHWEIF.EXE C1.02 (TASM 3.1 /ml)
;
; Inhalt:    fill_line/fill_v_line (waagrechte/senkrechte Linie in einen
;            Puffer), vga_save_row/_col und vga_restore_row/_col(_part):
;            eine Bildschirmzeile (320 Bytes) oder -spalte (200 Bytes) in
;            A000 sichern bzw. zurueckschreiben (Fensterrahmen, Win::
;            save_border/restore_border).
; Aufrufer:  SEG018 (Linien), SEG044/O190 (Fensterraender), far
; Aufruf:    far, C-Konvention, Stackrahmen bp, Parameter ab [bp+6].
;            Die Kopierroutinen setzen DF = 0 voraus (kein cld in den
;            Zeilenroutinen).
; Register:  DS, ES, SI, DI gesichert; AX, CX veraendert.
; Daten:     keine.
; Herkunft:  aus SCHWEIF.EXE R34 (ab +0x8) mit asm2tasm.py zerlegt;
;            ;@nnnn = Offset im Segment R34.
;===========================================================================
	.386p
SCREEN_W	equ	320	; Bytes je Bildschirmzeile (Modus 13h)
SCREEN_H	equ	200
	locals
SEG034_TEXT	segment word public use16 'CODE'
	assume	cs:SEG034_TEXT, ds:nothing
	public	F_R34_0008
	public	F_R34_0022
	public	F_R34_0041
	public	F_R34_0060
	public	F_R34_007F
	public	F_R34_009F
	public	F_R34_00C4
	public	F_R34_00E9
; ---------------------------------------------------------------------------
; fill_line - n Bytes einer Zeile mit einer Farbe fuellen
; Ein:  [bp+6] dst far, [bp+0ah] n, [bp+0ch] Farbe (Byte)
; Aus:  -
; ---------------------------------------------------------------------------
F_R34_0008	label	far	; Linkname von fill_line
fill_line:
	push	bp	;@0008
	mov	bp,sp	;@0009
	push	ds	;@000b
	push	es	;@000c
	push	si	;@000d
	push	di	;@000e
	les	di,[bp+6]	;@000f
	mov	cx,[bp+0ah]	;@0012
	mov	al,[bp+0ch]	;@0015
	rep	stosb	;@0018 n Bytes Farbe
	pop	di	;@001a
	pop	si	;@001b
	pop	es	;@001c
	pop	ds	;@001d
	mov	sp,bp	;@001e
	pop	bp	;@0020
	retf	;@0021
; ---------------------------------------------------------------------------
; vga_save_row - Bildschirmzeile sichern
; Ein:  [bp+6] Offset in A000 (Zeilenanfang), [bp+8] Puffer far
; Aus:  -   (320 Bytes = 160 Worte von A000:ofs in den Puffer)
; ---------------------------------------------------------------------------
F_R34_0022	label	far	; Linkname von vga_save_row
vga_save_row:
	push	bp	;@0022
	mov	bp,sp	;@0023
	push	ds	;@0025
	push	es	;@0026
	push	si	;@0027
	push	di	;@0028
	mov	cx,SCREEN_W/2	;@0029
	mov	ax,0a000h	;@002c
	mov	ds,ax	;@002f
	mov	si,[bp+6]	;@0031
	les	di,[bp+8]	;@0034
	rep	movsw	;@0037 eine Zeile wortweise
	pop	di	;@0039
	pop	si	;@003a
	pop	es	;@003b
	pop	ds	;@003c
	mov	sp,bp	;@003d
	pop	bp	;@003f
	retf	;@0040
; ---------------------------------------------------------------------------
; vga_restore_row - Bildschirmzeile aus dem Puffer zurueckschreiben
; Ein:  [bp+6] Offset in A000, [bp+8] Puffer far
; Aus:  -   (320 Bytes)
; ---------------------------------------------------------------------------
F_R34_0041	label	far	; Linkname von vga_restore_row
vga_restore_row:
	push	bp	;@0041
	mov	bp,sp	;@0042
	push	ds	;@0044
	push	es	;@0045
	push	si	;@0046
	push	di	;@0047
	mov	cx,SCREEN_W/2	;@0048
	mov	ax,0a000h	;@004b
	mov	es,ax	;@004e
	mov	di,[bp+6]	;@0050
	lds	si,[bp+8]	;@0053
	rep	movsw	;@0056 eine Zeile wortweise
	pop	di	;@0058
	pop	si	;@0059
	pop	es	;@005a
	pop	ds	;@005b
	mov	sp,bp	;@005c
	pop	bp	;@005e
	retf	;@005f
; ---------------------------------------------------------------------------
; vga_restore_row_part - Teil einer Bildschirmzeile zurueckschreiben
; Ein:  [bp+6] Offset in A000, [bp+8] Puffer far, [bp+0ch] n Bytes
; Aus:  -
; ---------------------------------------------------------------------------
F_R34_0060	label	far	; Linkname von vga_restore_row_part
vga_restore_row_part:
	push	bp	;@0060
	mov	bp,sp	;@0061
	push	ds	;@0063
	push	es	;@0064
	push	si	;@0065
	push	di	;@0066
	mov	cx,[bp+0ch]	;@0067
	mov	ax,0a000h	;@006a
	mov	es,ax	;@006d
	mov	di,[bp+6]	;@006f
	lds	si,[bp+8]	;@0072
	rep	movsb	;@0075
	pop	di	;@0077
	pop	si	;@0078
	pop	es	;@0079
	pop	ds	;@007a
	mov	sp,bp	;@007b
	pop	bp	;@007d
	retf	;@007e
; ---------------------------------------------------------------------------
; fill_v_line - senkrechte Linie
; Ein:  [bp+6] dst far, [bp+0ah] n Punkte, [bp+0ch] Farbe,
;       [bp+0eh] Zeilenabstand in Bytes
; Aus:  -
; ---------------------------------------------------------------------------
F_R34_007F	label	far	; Linkname von fill_v_line
fill_v_line:
	push	bp	;@007f
	mov	bp,sp	;@0080
	push	ds	;@0082
	push	es	;@0083
	push	si	;@0084
	push	di	;@0085
	les	di,[bp+6]	;@0086
	mov	cx,[bp+0ah]	;@0089
	mov	al,[bp+0ch]	;@008c
@@next:
	mov	es:[di],al	;@008f
	add	di,[bp+0eh]	;@0092 naechste Zeile
	loop	@@next	;@0095
	pop	di	;@0097
	pop	si	;@0098
	pop	es	;@0099
	pop	ds	;@009a
	mov	sp,bp	;@009b
	pop	bp	;@009d
	retf	;@009e
; ---------------------------------------------------------------------------
; vga_save_col - Bildschirmspalte sichern
; Ein:  [bp+6] Offset in A000 (Spalte x in Zeile 0), [bp+8] Puffer far
; Aus:  -   (200 Bytes, Abstand 320 im Bildschirm, dicht im Puffer)
; ---------------------------------------------------------------------------
F_R34_009F	label	far	; Linkname von vga_save_col
vga_save_col:
	push	bp	;@009f
	mov	bp,sp	;@00a0
	push	ds	;@00a2
	push	es	;@00a3
	push	si	;@00a4
	push	di	;@00a5
	cld	;@00a6
	mov	ax,0a000h	;@00a7
	mov	ds,ax	;@00aa
	mov	si,[bp+6]	;@00ac
	les	di,[bp+8]	;@00af
	mov	cx,SCREEN_H	;@00b2
@@next:
	movsb	;@00b5
	add	si,SCREEN_W-1	;@00b6 movsb hat SI schon um 1 erhoeht
	loop	@@next	;@00ba
	pop	di	;@00bc
	pop	si	;@00bd
	pop	es	;@00be
	pop	ds	;@00bf
	mov	sp,bp	;@00c0
	pop	bp	;@00c2
	retf	;@00c3
; ---------------------------------------------------------------------------
; vga_restore_col - Bildschirmspalte zurueckschreiben
; Ein:  [bp+6] Offset in A000, [bp+8] Puffer far
; Aus:  -   (200 Bytes)
; ---------------------------------------------------------------------------
F_R34_00C4	label	far	; Linkname von vga_restore_col
vga_restore_col:
	push	bp	;@00c4
	mov	bp,sp	;@00c5
	push	ds	;@00c7
	push	es	;@00c8
	push	si	;@00c9
	push	di	;@00ca
	cld	;@00cb
	mov	ax,0a000h	;@00cc
	mov	es,ax	;@00cf
	mov	di,[bp+6]	;@00d1
	lds	si,[bp+8]	;@00d4
	mov	cx,SCREEN_H	;@00d7
@@next:
	movsb	;@00da
	add	di,SCREEN_W-1	;@00db movsb hat DI schon um 1 erhoeht
	loop	@@next	;@00df
	pop	di	;@00e1
	pop	si	;@00e2
	pop	es	;@00e3
	pop	ds	;@00e4
	mov	sp,bp	;@00e5
	pop	bp	;@00e7
	retf	;@00e8
; ---------------------------------------------------------------------------
; vga_restore_col_part - Teil einer Bildschirmspalte zurueckschreiben
; Ein:  [bp+6] Offset in A000, [bp+8] Puffer far, [bp+0ch] n Bytes
; Aus:  -
; ---------------------------------------------------------------------------
F_R34_00E9	label	far	; Linkname von vga_restore_col_part
vga_restore_col_part:
	push	bp	;@00e9
	mov	bp,sp	;@00ea
	push	ds	;@00ec
	push	es	;@00ed
	push	si	;@00ee
	push	di	;@00ef
	cld	;@00f0
	mov	ax,0a000h	;@00f1
	mov	es,ax	;@00f4
	mov	di,[bp+6]	;@00f6
	lds	si,[bp+8]	;@00f9
	mov	cx,[bp+0ch]	;@00fc
@@next:
	movsb	;@00ff
	add	di,SCREEN_W-1	;@0100
	loop	@@next	;@0104
	pop	di	;@0106
	pop	si	;@0107
	pop	es	;@0108
	pop	ds	;@0109
	mov	sp,bp	;@010a
	pop	bp	;@010c
	retf	;@010d
SEG034_TEXT	ends
	end
