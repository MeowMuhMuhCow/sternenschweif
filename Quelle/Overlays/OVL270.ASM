;===========================================================================
; OVL270.ASM - FLI-Entpacker (Autodesk Animator), Strahlruecklauf, Palette
; Sternenschweif (DSA 2), Attic 1994 - Rekonstruktion, bytegleich mit
; SCHWEIF.EXE C1.02 (TASM 3.1 /ml)
;
; Inhalt:    Die Chunk-Routinen fuer FLI-Filme (320 x 200, 256 Farben):
;            FLI_COLOR (Palette, direkt in den DAC oder in einen Puffer),
;            FLI_LC (Zeilenaenderungen), FLI_BRUN (Lauflaengen, Vollbild),
;            FLI_BLACK (loeschen), FLI_COPY (unkomprimiert), dazu Warten auf
;            den Strahlruecklauf und Normalisieren eines huge-Zeigers.
; Aufrufer:  O025 (FLI-Spieler), far
; Aufruf:    far, C-Konvention, Stackrahmen bp (fli_wait_retrace ohne).
;            Chunkdaten jeweils ohne den 6-Byte-Chunkkopf.
; Register:  die Routinen sichern alles ausser AX, DX (und BX/CX, wo sie
;            sie selbst pushen); DF wird geloescht (cld).
; Daten:     keine.
; Herkunft:  aus SCHWEIF.EXE O270 (ab +0x0) mit asm2tasm.py zerlegt;
;            ;@nnnn = Offset im Overlay O270.
;===========================================================================
	.386p
VGA_STATUS	equ	03dah	; Eingabestatus 1, Bit 3 = senkrechter Ruecklauf
DAC_WRITE_INDEX	equ	03c8h
SCREEN_W	equ	320
	locals
OVL270_TEXT	segment byte public use16 'CODE'
	assume	cs:OVL270_TEXT, ds:nothing
	public	F_O270_0009
	public	F_O270_0000
	public	F_O270_0047
	public	F_O270_0141
	public	F_O270_007F
	public	F_O270_015C
	public	F_O270_00DD
	public	F_O270_012C
; ---------------------------------------------------------------------------
; fli_wait_retrace - warten, bis der senkrechte Strahlruecklauf laeuft
; Ein:  -
; Aus:  AL = Status. Wartet nur auf den Beginn (Bit 3 = 1), nicht auf das
;       Ende eines schon laufenden Ruecklaufs.
; ---------------------------------------------------------------------------
F_O270_0000	label	far	; Linkname von fli_wait_retrace
fli_wait_retrace:
	mov	dx,VGA_STATUS	;@0000
@@wait:
	in	al,dx	;@0003
	test	al,8	;@0004
	jz	@@wait	;@0006
	retf	;@0008
; ---------------------------------------------------------------------------
; fli_color_dac - FLI_COLOR-Chunk direkt in den DAC
; Ein:  [bp+6] Chunkdaten far: Paketzahl (Wort), je Paket: Farben
;       ueberspringen (Byte), Anzahl (Byte, 0 = 256), Anzahl * RGB (6 Bit)
; Aus:  -
; ---------------------------------------------------------------------------
F_O270_0009	label	far	; Linkname von fli_color_dac
fli_color_dac:
	push	bp	;@0009
	mov	bp,sp	;@000a
	push	ds	;@000c
	push	si	;@000d
	push	di	;@000e
	push	cx	;@000f
	push	bx	;@0010
	cld	;@0011
	lds	si,[bp+6]	;@0012
	mov	di,0	;@0015
	lodsw	;@0018 BX = Paketzahl
	mov	bx,ax	;@0019
	test	bx,bx	;@001b
	jmp	@@test_packets	;@001d ueber den Test unten einsteigen
@@next_packet:
	lodsb	;@0020
	add	di,ax	;@0021 Farben ueberspringen
	lodsb	;@0023
	mov	cx,ax	;@0024 Anzahl
	or	cx,cx	;@0026
	jnz	short @@next_color	;@0028
	mov	cx,0100h	;@002a 0 bedeutet 256
@@next_color:
	mov	dx,DAC_WRITE_INDEX	;@002d
	mov	ax,di	;@0030
	out	dx,al	;@0032
	inc	di	;@0033 naechste Farbe
	inc	dx	;@0034
	lodsb	;@0035 R, G, B
	out	dx,al	;@0036
	lodsb	;@0037
	out	dx,al	;@0038
	lodsb	;@0039
	out	dx,al	;@003a
	loop	@@next_color	;@003b
	dec	bx	;@003d
@@test_packets:
	jnz	@@next_packet	;@003e
	pop	bx	;@0040
	pop	cx	;@0041
	pop	di	;@0042
	pop	si	;@0043
	pop	ds	;@0044
	pop	bp	;@0045
	retf	;@0046
; ---------------------------------------------------------------------------
; fli_color_pal - FLI_COLOR-Chunk in einen Palettenpuffer
; Ein:  [bp+6] Chunkdaten far, [bp+0ah] Palette far (256 * 3 Bytes)
; Aus:  -
; ---------------------------------------------------------------------------
F_O270_0047	label	far	; Linkname von fli_color_pal
fli_color_pal:
	push	bp	;@0047
	mov	bp,sp	;@0048
	push	ds	;@004a
	push	si	;@004b
	push	di	;@004c
	push	cx	;@004d
	push	bx	;@004e
	cld	;@004f
	lds	si,[bp+6]	;@0050
	les	di,[bp+0ah]	;@0053
	lodsw	;@0056
	mov	bx,ax	;@0057
	test	bx,bx	;@0059
	jmp	@@test_packets	;@005b
@@next_packet:
	lodsb	;@005e
	add	di,ax	;@005f Farben ueberspringen (je 3 Bytes)
	add	di,ax	;@0061
	add	di,ax	;@0063
	lodsb	;@0065
	or	al,al	;@0066
	jnz	short @@copy	;@0068
	mov	ax,0100h	;@006a 0 bedeutet 256
@@copy:
	mov	cx,ax	;@006d Anzahl * 3 Bytes kopieren
	add	cx,ax	;@006f
	add	cx,ax	;@0071
	rep	movsb	;@0073
	dec	bx	;@0075
@@test_packets:
	jnz	@@next_packet	;@0076
	pop	bx	;@0078
	pop	cx	;@0079
	pop	di	;@007a
	pop	si	;@007b
	pop	ds	;@007c
	pop	bp	;@007d
	retf	;@007e
; ---------------------------------------------------------------------------
; fli_lc - FLI_LC-Chunk (Zeilenaenderungen) entpacken
; Ein:  [bp+6] Chunkdaten far, [bp+0ah] Bild far (320 Bytes je Zeile)
; Aus:  -
; Daten: erste Zeile (Wort), Zeilenzahl (Wort); je Zeile Paketzahl (Byte);
; je Paket Punkte ueberspringen (Byte), Groesse (Byte, vorzeichenbehaftet):
; > 0 Bytes kopieren, < 0 das naechste Byte -n mal wiederholen.
; Locals: [bp-2] verbleibende Zeilen.
; ---------------------------------------------------------------------------
F_O270_007F	label	far	; Linkname von fli_lc
fli_lc:
	push	bp	;@007f
	mov	bp,sp	;@0080
	sub	sp,4	;@0082
	push	es	;@0085
	push	ds	;@0086
	push	si	;@0087
	push	di	;@0088
	push	bx	;@0089
	push	cx	;@008a
	cld	;@008b
	lds	si,[bp+6]	;@008c
	les	di,[bp+0ah]	;@008f
	lodsw	;@0092 erste Zeile
	mov	dx,SCREEN_W	;@0093
	mul	dx	;@0096
	add	di,ax	;@0098 DI = Bild + erste Zeile * 320
	lodsw	;@009a Zeilenzahl
	mov	[bp-2],ax	;@009b
	mov	dx,di	;@009e DX = Zeilenanfang
	xor	ah,ah	;@00a0
@@next_line:
	mov	di,dx	;@00a2
	lodsb	;@00a4 Paketzahl der Zeile
	mov	bl,al	;@00a5
	test	bl,bl	;@00a7
	jmp	@@test_packets	;@00a9
@@next_packet:
	lodsb	;@00ac
	add	di,ax	;@00ad ueberspringen
	lodsb	;@00af
	test	al,al	;@00b0
	js	short @@run	;@00b2 negativ: Wiederholung
	mov	cx,ax	;@00b4 kopieren
	rep	movsb	;@00b6
	dec	bl	;@00b8
	jnz	@@next_packet	;@00ba
	jmp	@@line_done	;@00bc
@@run:
	neg	al	;@00bf Wiederholung: -n mal das naechste Byte
	mov	cx,ax	;@00c1
	lodsb	;@00c3
	rep	stosb	;@00c4
	dec	bl	;@00c6
@@test_packets:
	jnz	@@next_packet	;@00c8
@@line_done:
	add	dx,SCREEN_W	;@00ca naechste Zeile
	dec	word ptr [bp-2]	;@00ce
	jnz	@@next_line	;@00d1
	pop	cx	;@00d3
	pop	bx	;@00d4
	pop	di	;@00d5
	pop	si	;@00d6
	pop	ds	;@00d7
	pop	es	;@00d8
	mov	sp,bp	;@00d9
	pop	bp	;@00db
	retf	;@00dc
; ---------------------------------------------------------------------------
; fli_brun - FLI_BRUN-Chunk (Lauflaengen) entpacken
; Ein:  [bp+6] Chunkdaten far, [bp+0ah] Bild far, [bp+0eh] h Zeilen
; Aus:  -
; Je Zeile Paketzahl (Byte); je Paket Groesse (Byte, vorzeichenbehaftet):
; > 0 das naechste Byte n mal wiederholen, < 0 -n Bytes kopieren.
; ---------------------------------------------------------------------------
F_O270_00DD	label	far	; Linkname von fli_brun
fli_brun:
	push	bp	;@00dd
	mov	bp,sp	;@00de
	sub	sp,4	;@00e0
	push	es	;@00e3
	push	ds	;@00e4
	push	si	;@00e5
	push	di	;@00e6
	push	bx	;@00e7
	push	cx	;@00e8
	cld	;@00e9
	lds	si,[bp+6]	;@00ea
	les	di,[bp+0ah]	;@00ed
	mov	dx,di	;@00f0
	xor	ah,ah	;@00f2
@@next_line:
	mov	di,dx	;@00f4
	lodsb	;@00f6
	mov	bl,al	;@00f7
	test	bl,bl	;@00f9
	jmp	@@test_packets	;@00fb
@@next_packet:
	lodsb	;@00fe
	test	al,al	;@00ff
	js	short @@copy	;@0101 negativ: kopieren
	mov	cx,ax	;@0103 Wiederholung
	lodsb	;@0105
	rep	stosb	;@0106
	dec	bl	;@0108
	jnz	@@next_packet	;@010a
	jmp	@@line_done	;@010c
@@copy:
	neg	al	;@010f -n Bytes kopieren
	mov	cx,ax	;@0111
	rep	movsb	;@0113
	dec	bl	;@0115
@@test_packets:
	jnz	@@next_packet	;@0117
@@line_done:
	add	dx,SCREEN_W	;@0119
	dec	word ptr [bp+0eh]	;@011d
	jnz	@@next_line	;@0120
	pop	cx	;@0122
	pop	bx	;@0123
	pop	di	;@0124
	pop	si	;@0125
	pop	ds	;@0126
	pop	es	;@0127
	mov	sp,bp	;@0128
	pop	bp	;@012a
	retf	;@012b
; ---------------------------------------------------------------------------
; fli_black - FLI_BLACK: Bild loeschen
; Ein:  [bp+6] Bild far, [bp+0ah] n Worte (O025: FLI_SCREEN_SIZE = 32000)
; Aus:  -   (setzt DF nicht; erwartet DF = 0)
; ---------------------------------------------------------------------------
F_O270_012C	label	far	; Linkname von fli_black
fli_black:
	push	bp	;@012c
	mov	bp,sp	;@012d
	push	es	;@012f
	push	di	;@0130
	push	cx	;@0131
	les	di,[bp+6]	;@0132
	mov	cx,[bp+0ah]	;@0135
	xor	ax,ax	;@0138
	rep	stosw	;@013a
	pop	cx	;@013c
	pop	di	;@013d
	pop	es	;@013e
	pop	bp	;@013f
	retf	;@0140
; ---------------------------------------------------------------------------
; fli_copy - FLI_COPY: unkomprimiertes Bild kopieren
; Ein:  [bp+6] Quelle far, [bp+0ah] Ziel far, [bp+0eh] n Worte
; Aus:  -
; ---------------------------------------------------------------------------
F_O270_0141	label	far	; Linkname von fli_copy
fli_copy:
	push	bp	;@0141
	mov	bp,sp	;@0142
	push	es	;@0144
	push	di	;@0145
	push	ds	;@0146
	push	si	;@0147
	push	cx	;@0148
	lds	si,[bp+6]	;@0149
	les	di,[bp+0ah]	;@014c
	mov	cx,[bp+0eh]	;@014f
	cld	;@0152
	rep	movsw	;@0153
	pop	cx	;@0155
	pop	si	;@0156
	pop	ds	;@0157
	pop	di	;@0158
	pop	es	;@0159
	pop	bp	;@015a
	retf	;@015b
; ---------------------------------------------------------------------------
; fli_normalize - huge-Zeiger normalisieren
; Ein:  [bp+6] Offset, [bp+8] Segment
; Aus:  DX:AX = (Segment + Offset / 16):(Offset & 0fh)
; ---------------------------------------------------------------------------
F_O270_015C	label	far	; Linkname von fli_normalize
fli_normalize:
	push	bp	;@015c
	mov	bp,sp	;@015d
	push	cx	;@015f
	mov	ax,[bp+6]	;@0160
	mov	dx,[bp+8]	;@0163
	mov	cl,4	;@0166
	shr	ax,cl	;@0168
	add	dx,ax	;@016a
	mov	ax,[bp+6]	;@016c
	and	ax,0fh	;@016f
	pop	cx	;@0172
	pop	bp	;@0173
	retf	;@0174
OVL270_TEXT	ends
	end
