;===========================================================================
; SEG046.ASM - Bilder mit Zeilenzeigern zeichnen, holen und skalieren
; Sternenschweif (DSA 2), Attic 1994 - Rekonstruktion, bytegleich mit
; SCHWEIF.EXE C1.02 (TASM 3.1 /ml)
;
; Inhalt:    Die Bildklasse Bmp (SEG042) haelt ein Bild als Feld von far-
;            Zeigern auf seine Zeilen. blit_lines zeichnet es in einen
;            Puffer mit 320 Bytes je Zeile (geclippt an g_clip_rect, sieben
;            Zeichenarten BLIT_*), grab_lines holt einen Ausschnitt, und
;            scale_lines skaliert mit selbst erzeugtem Code: fuer eine Zeile
;            wird eine Folge aus lodsb/stosb (Bresenham) nach g_scale_code
;            geschrieben und je Zielzeile far aufgerufen.
; Aufrufer:  SEG042 (gb_blit*, gb_save*, gb_scale), far
; Aufruf:    far, C-Konvention, Stackrahmen bp mit lokalen Kopien.
; Register:  DS, ES, SI, DI (scale_lines auch FS) gesichert; AX, BX, CX, DX,
;            EAX veraendert. blit_lines laesst nach dem Rueckwaertskopieren
;            DF = 1 stehen (Fehler #302).
; Daten:     g_clip_rect (ds:3303, x0/y0/x1/y1), Stueck DMR46: g_scale_src,
;            g_scale_dst, g_scale_op_lodsb/_stosb/_retf, g_scale_code.
; Herkunft:  aus SCHWEIF.EXE R46 (ab +0xc) mit asm2tasm.py zerlegt;
;            ;@nnnn = Offset im Segment R46.
;===========================================================================
	.386p
	include	..\names\inc\s_fix.inc	; Schalter der Fehlerbehebungen (FIX_nnnn, tools/mkfix.py)
; Daten im Datensegment; Adressen als Konstanten (Code ohne Fixups wie im Original)
g_clip_x0	equ	03303h	; dw  g_clip_rect.x0
g_clip_y0	equ	03305h	; dw  g_clip_rect.y0
g_clip_x1	equ	03307h	; dw  g_clip_rect.x1
g_clip_y1	equ	03309h	; dw  g_clip_rect.y1
g_scale_src	equ	086a6h	; dd  Zeilenzeiger der Quelle (scale_lines)
g_scale_dst	equ	086aah	; dd  Zeilenzeiger des Ziels (scale_lines)
g_scale_op_lodsb	equ	086aeh	; db 0ach  Opcode lodsb
g_scale_op_stosb	equ	086afh	; db 0aah  Opcode stosb
g_scale_op_retf	equ	086b0h	; db 0cbh  Opcode retf
g_scale_code	equ	086b1h	; dd  Puffer fuer den erzeugten Skaliercode (R42 gb_scale)
SCREEN_W	equ	320
; Zeichenarten (s_const.h)
BLIT_COPY	equ	0
BLIT_OR		equ	1
BLIT_XOR	equ	2
BLIT_AND	equ	3
BLIT_MASKED	equ	4	; Farbe 0 durchsichtig
BLIT_BEHIND	equ	5	; nur wo das Ziel eine Farbe < 0a0h hat
BLIT_MASKED_FF	equ	6	; Farbe 0ffh durchsichtig
	locals
SEG046_TEXT	segment byte public use16 'CODE'
	assume	cs:SEG046_TEXT, ds:nothing
	public	F_R46_000C
	public	F_R46_01BD
	public	F_R46_023E
; ---------------------------------------------------------------------------
; blit_lines - Bild aus Zeilenzeigern in einen Puffer zeichnen
; Ein:  [bp+6]   x, [bp+8] y      Zielposition
;       [bp+0ah] w, [bp+0ch] h    Breite, Hoehe
;       [bp+0eh] (unbenutzt)
;       [bp+10h] lines far        Feld von h far-Zeigern auf die Zeilen
;       [bp+14h] mode (Byte)      BLIT_*
;       [bp+16h] dst far          Zielpuffer, 320 Bytes je Zeile
;       [bp+1ah] backwards        != 0: Zeile rueckwaerts kopieren (std)
; Aus:  -
; Locals: [bp-2] x, [bp-4] y (laeuft mit), [bp-6] w (geclippt), [bp-8] mode,
;       [bp-0ah] links abgeschnitten, [bp-0eh] Zeilenzeiger, [bp-12h] dst.
; Je Zeile: y ausserhalb g_clip_y0..y1 oder w = 0 -> ueberspringen; x an
; g_clip_x0/x1 clippen, dann je nach mode kopieren.
; Originalfehler #301: BLIT_OR springt nach dem ersten Wort zurueck in die
; Kopierschleife (loop auf rep movsd statt auf @@or_word): der Rest der
; Zeile wird kopiert statt geodert, mit Laenge in Doppelworten statt Worten.
; Originalfehler #302: Rueckwaerts (backwards != 0) zeigen SI/DI hinter das
; Zeilenende statt auf das letzte Byte (Versatz bis 4 Bytes), und DF bleibt
; nach dem Ende gesetzt. Beide Faelle benutzt das Spiel nicht.
; ---------------------------------------------------------------------------
F_R46_000C	label	far	; Linkname von blit_lines
blit_lines:
	push	bp	;@000c
	mov	bp,sp	;@000d
	sub	sp,012h	;@000f
	push	ds	;@0012
	push	es	;@0013
	push	si	;@0014
	push	di	;@0015
	mov	eax,[bp+016h]	;@0016 [bp-12h] = dst
	mov	[bp-012h],eax	;@001a
	les	di,[bp+010h]	;@001e
	mov	ax,[bp+6]	;@0021
	mov	bx,[bp+8]	;@0024
	mov	dl,[bp+014h]	;@0027
	mov	[bp-2],ax	;@002a
	mov	[bp-4],bx	;@002d
	mov	ax,[bp+0ah]	;@0030
	mov	[bp-6],ax	;@0033
	mov	[bp-8],dl	;@0036
	mov	cx,[bp+0ch]	;@0039
@@next_line:
	mov	eax,es:[di]	;@003c
	mov	[bp-0eh],eax	;@0040 [bp-0eh] = Zeiger auf diese Zeile
	push	cx	;@0044
	push	es	;@0045
	push	di	;@0046
	mov	ax,[bp-2]	;@0047
	mov	bx,[bp-4]	;@004a
	mov	cx,[bp-6]	;@004d
	cmp	bx,ds:[g_clip_y0]	;@0050 y < y0?
	jc	@@skip_line_far	;@0054
	cmp	bx,ds:[g_clip_y1]	;@0058 y > y1?
	ja	@@skip_line_far	;@005c
	or	cx,cx	;@0060 w = 0?
	jz	@@skip_line_far	;@0062
	cmp	ax,ds:[g_clip_x1]	;@0066 x > x1?
	jg	@@skip_line_far	;@006a
	add	ax,cx	;@006e x + w <= x0?
	cmp	ax,ds:[g_clip_x0]	;@0070
	jng	@@skip_line_far	;@0074
	sub	ax,cx	;@0078
	xor	si,si	;@007a
	cmp	ax,ds:[g_clip_x0]	;@007c links abschneiden: SI = x0 - x
	jnl	@@left_done	;@0080
	mov	si,ds:[g_clip_x0]	;@0084
	sub	si,ax	;@0088
	sub	cx,si	;@008a
@@left_done:
	mov	[bp-0ah],si	;@008c
	add	si,ax	;@008f
	add	si,cx	;@0091
	dec	si	;@0093
	cmp	si,ds:[g_clip_x1]	;@0094 rechts abschneiden
	jng	@@clipped	;@0098
	mov	cx,ds:[g_clip_x1]	;@009c
	sub	cx,ax	;@00a0
	sub	cx,[bp-0ah]	;@00a2
	inc	cx	;@00a5
@@clipped:
	push	word ptr [bp-6]	;@00a6 altes w sichern, geclipptes w setzen
	mov	[bp-6],cx	;@00a9
	jmp	@@draw	;@00ac
@@skip_line_far:
	jmp	near ptr @@skip_line	;@00af
@@draw:
	mov	ax,[bp-4]	;@00b2 DI = y * 320 + x + dst + links abgeschnitten
	mov	bx,SCREEN_W	;@00b5
	mul	bx	;@00b8
	mov	dl,[bp-8]	;@00ba
	add	ax,[bp-2]	;@00bd
	add	ax,[bp-012h]	;@00c0
	mov	di,ax	;@00c3
	mov	ax,[bp-0ah]	;@00c5
	add	di,ax	;@00c8
	mov	cx,[bp-6]	;@00ca
	push	ds	;@00cd
	mov	bx,[bp-010h]	;@00ce
	mov	es,bx	;@00d1
	lds	si,[bp-0eh]	;@00d3 DS:SI = Zeile + links abgeschnitten
	add	si,ax	;@00d6
	cld	;@00d8
	cmp	word ptr [bp+01ah],0	;@00d9 rueckwaerts?
	jz	@@mode_copy	;@00dd
	add	si,cx	;@00e1 SI/DI auf das Ende (Fehler #302: eins zu weit)
	add	di,cx	;@00e3
	std	;@00e5
@@mode_copy:
	cmp	dl,0	;@00e6 BLIT_COPY
	jnz	@@mode_or	;@00e9
	shr	cx,1	;@00ed ungerades Byte
	jnc	@@copy_word	;@00ef
	movsb	;@00f3
@@copy_word:
	shr	cx,1	;@00f4 ungerades Wort
	jnc	@@copy_dwords	;@00f6
	movsw	;@00fa
@@copy_dwords:
	rep	movsd	;@00fb Rest als Doppelworte
	jmp	near ptr @@line_done	;@00fe
@@mode_or:
	cmp	dl,1	;@0101 BLIT_OR
	jnz	@@mode_xor	;@0104
	shr	cx,1	;@0108
	jnc	@@or_word	;@010a
	lodsb	;@010e
	or	es:[di],al	;@010f
	inc	di	;@0112
@@or_word:
	lodsw	;@0113
	or	es:[di],ax	;@0114
	add	di,2	;@0117
IF FIX_0301
	loop	@@or_word	; Fix #301: Oder-Schleife fortsetzen
ELSE
	loop	@@copy_dwords	;@011a Fehler #301: richtig waere @@or_word
ENDIF
	jmp	near ptr @@line_done	;@011c
@@mode_xor:
	cmp	dl,2	;@011f BLIT_XOR
	jnz	@@mode_and	;@0122
	shr	cx,1	;@0126
	jnc	@@xor_word	;@0128
	lodsb	;@012c
	xor	es:[di],al	;@012d
	inc	di	;@0130
@@xor_word:
	lodsw	;@0131
	xor	es:[di],ax	;@0132
	add	di,2	;@0135
	loop	@@xor_word	;@0138
	jmp	@@line_done	;@013a
@@next_line_far:
	jmp	near ptr @@next_line	;@013d
@@mode_and:
	cmp	dl,3	;@0140 BLIT_AND
	jnz	@@mode_masked	;@0143
	shr	cx,1	;@0147
	jnc	@@and_word	;@0149
	lodsb	;@014d
	and	es:[di],al	;@014e
	inc	di	;@0151
@@and_word:
	lodsw	;@0152
	and	es:[di],ax	;@0153
	add	di,2	;@0156
	loop	@@and_word	;@0159
	jmp	@@line_done	;@015b
@@mode_masked:
	cmp	dl,4	;@015e BLIT_MASKED: 0 durchsichtig
	jnz	@@mode_behind	;@0161
@@masked_pixel:
	lodsb	;@0165
	and	al,al	;@0166
	jz	@@masked_skip	;@0168
	stosb	;@016c
	loop	@@masked_pixel	;@016d
	jmp	@@line_done	;@016f
@@masked_skip:
	inc	di	;@0172
	loop	@@masked_pixel	;@0173
@@mode_behind:
	cmp	dl,5	;@0175 BLIT_BEHIND: nur auf Hintergrund (Farbe < 0a0h)
	jnz	@@mode_masked_ff	;@0178
@@behind_pixel:
	mov	al,es:[di]	;@017c
	cmp	al,0a0h	;@017f
	jnc	@@behind_skip	;@0181
	movsb	;@0185
	loop	@@behind_pixel	;@0186
	jmp	@@line_done	;@0188
@@behind_skip:
	inc	di	;@018b
	inc	si	;@018c
	loop	@@behind_pixel	;@018d
@@mode_masked_ff:
	cmp	dl,6	;@018f BLIT_MASKED_FF: 0ffh durchsichtig
	jnz	@@line_done	;@0192
@@masked_ff_pixel:
	lodsb	;@0196
	cmp	al,0ffh	;@0197
	jz	@@masked_ff_skip	;@0199
	stosb	;@019d
	loop	@@masked_ff_pixel	;@019e
	jmp	@@line_done	;@01a0
@@masked_ff_skip:
	inc	di	;@01a3
	loop	@@masked_ff_pixel	;@01a4
@@line_done:
	pop	ds	;@01a6
	pop	word ptr [bp-6]	;@01a7 w zuruecksetzen
@@skip_line:
	pop	di	;@01aa
	pop	es	;@01ab
	pop	cx	;@01ac
	add	di,4	;@01ad naechster Zeilenzeiger
	inc	word ptr [bp-4]	;@01b0
	loop	@@next_line_far	;@01b3
	pop	di	;@01b5
	pop	si	;@01b6
	pop	es	;@01b7
	pop	ds	;@01b8
	mov	sp,bp	;@01b9
	pop	bp	;@01bb
	retf	;@01bc
; ---------------------------------------------------------------------------
; grab_lines - Ausschnitt eines Puffers (320 Bytes je Zeile) in Zeilen holen
; Ein:  [bp+6] x, [bp+8] y, [bp+0ah] w, [bp+0ch] h,
;       [bp+0eh] lines far (Zielzeilen), [bp+12h] src far (Puffer)
; Aus:  -
; Zeilen ausserhalb g_clip_y0..y1 werden uebersprungen; x wird nicht geclippt.
; ---------------------------------------------------------------------------
F_R46_01BD	label	far	; Linkname von grab_lines
grab_lines:
	push	bp	;@01bd
	mov	bp,sp	;@01be
	sub	sp,0eh	;@01c0
	push	ds	;@01c3
	push	es	;@01c4
	push	si	;@01c5
	push	di	;@01c6
	mov	eax,[bp+012h]	;@01c7
	mov	[bp-0eh],eax	;@01cb
	les	di,[bp+0eh]	;@01cf
	mov	ax,[bp+0ah]	;@01d2
	mov	bx,[bp+0ch]	;@01d5
	mov	cx,[bp+6]	;@01d8
	mov	dx,[bp+8]	;@01db
	mov	[bp-2],cx	;@01de
	mov	[bp-4],dx	;@01e1
	mov	[bp-6],ax	;@01e4
	mov	cx,bx	;@01e7
@@next_line:
	mov	eax,es:[di]	;@01e9
	mov	[bp-0ah],eax	;@01ed
	push	cx	;@01f1
	push	es	;@01f2
	push	di	;@01f3
	mov	ax,[bp-4]	;@01f4
	cmp	ax,ds:[g_clip_y0]	;@01f7 y < y0?
	jc	@@skip_line	;@01fb
	cmp	ax,ds:[g_clip_y1]	;@01ff y > y1?
	ja	@@skip_line	;@0203
	mov	bx,SCREEN_W	;@0207 SI = y * 320 + x + src
	mul	bx	;@020a
	add	ax,[bp-2]	;@020c
	add	ax,[bp-0eh]	;@020f
	mov	si,ax	;@0212
	les	di,[bp-0ah]	;@0214
	mov	cx,[bp-6]	;@0217
	push	ds	;@021a
	mov	ax,[bp-0ch]	;@021b
	mov	ds,ax	;@021e
	cld	;@0220
	shr	cx,1	;@0221 ungerades Byte, dann Worte
	jnc	@@copy_words	;@0223
	movsb	;@0227
@@copy_words:
	rep	movsw	;@0228
	pop	ds	;@022a
@@skip_line:
	pop	di	;@022b
	pop	es	;@022c
	pop	cx	;@022d
	add	di,4	;@022e
	inc	word ptr [bp-4]	;@0231
	loop	@@next_line	;@0234
	pop	di	;@0236
	pop	si	;@0237
	pop	es	;@0238
	pop	ds	;@0239
	mov	sp,bp	;@023a
	pop	bp	;@023c
	retf	;@023d
; ---------------------------------------------------------------------------
; scale_lines - Bild skalieren
; Ein:  [bp+6] src lines far, [bp+0ah] sw, [bp+0ch] sh,
;       [bp+0eh] dst lines far, [bp+12h] dw, [bp+14h] dh
; Aus:  -
; 1. Code fuer eine Zeile erzeugen (in g_scale_code): Bresenham ueber die
;    Breite, je Quellpunkt ein lodsb, je Zielpunkt ein stosb, am Ende retf.
; 2. Bresenham ueber die Hoehe: je Zielzeile den erzeugten Code mit
;    DS:SI = Quellzeile, ES:DI = Zielzeile far aufrufen.
; ---------------------------------------------------------------------------
F_R46_023E	label	far	; Linkname von scale_lines
scale_lines:
	push	bp	;@023e
	mov	bp,sp	;@023f
	sub	sp,0eh	;@0241
	push	ds	;@0244
	push	es	;@0245
	push	fs	;@0246
	push	si	;@0248
	push	di	;@0249
	mov	ax,[bp+0ah]	;@024a
	mov	bx,[bp+0ch]	;@024d
	mov	[bp-2],ax	;@0250
	mov	[bp-4],bx	;@0253
	les	di,[bp+6]	;@0256
	mov	ds:[g_scale_src],di	;@0259
	mov	ds:[g_scale_src+2],es	;@025d
	mov	ax,[bp+012h]	;@0261
	mov	bx,[bp+014h]	;@0264
	mov	[bp-6],ax	;@0267
	mov	[bp-8],bx	;@026a
	les	di,[bp+0eh]	;@026d
	mov	ds:[g_scale_dst],di	;@0270
	mov	ds:[g_scale_dst+2],es	;@0274
	les	di,ds:[g_scale_code]	;@0278 ES:DI = Codepuffer
	push	ds	;@027c
	pop	fs	;@027d FS = Datensegment (DS wechselt unten)
	cld	;@027f
	xor	dx,dx	;@0280
	xor	bx,bx	;@0282
	mov	cx,[bp-2]	;@0284 CX = Fehlerterm (sw)
@@emit_lodsb:
	sub	cx,[bp-6]	;@0287 Quellpunkt: lodsb erzeugen
	mov	al,ds:[g_scale_op_lodsb]	;@028a
	stosb	;@028d
	mov	al,ds:[g_scale_op_stosb]	;@028e
@@emit_stosb:
	cmp	cx,0	;@0291 Zielpunkte, solange der Fehlerterm <= 0
	jg	@@next_src_x	;@0294
	cmp	dx,[bp-6]	;@0298
	jnl	@@emit_done	;@029b
	stosb	;@029f stosb erzeugen
	inc	dx	;@02a0
	add	cx,[bp-2]	;@02a1
	jmp	short @@emit_stosb	;@02a4
@@next_src_x:
	inc	bx	;@02a6
	cmp	bx,[bp-2]	;@02a7
	jl	@@emit_lodsb	;@02aa
@@emit_done:
	mov	al,ds:[g_scale_op_retf]	;@02ac retf erzeugen
	stosb	;@02af
	mov	ax,[bp-4]	;@02b0
	mov	[bp-0ah],ax	;@02b3
	mov	ax,[bp-8]	;@02b6
	mov	[bp-0ch],ax	;@02b9
	les	di,fs:[g_scale_dst]	;@02bc
	lds	si,fs:[g_scale_src]	;@02c1
	mov	dx,[bp-4]	;@02c6
@@next_src_line:
	sub	dx,[bp-8]	;@02c9
@@dst_line:
	cmp	dx,0	;@02cc
	jg	@@src_line_done	;@02cf
	push	ds	;@02d3
	push	si	;@02d4
	push	es	;@02d5
	push	di	;@02d6
	les	di,es:[di]	;@02d7 ES:DI = Zielzeile, DS:SI = Quellzeile
	lds	si,[si]	;@02da
	call	dword ptr fs:[g_scale_code]	;@02dc erzeugten Code aufrufen
	pop	di	;@02e1
	pop	es	;@02e2
	pop	si	;@02e3
	pop	ds	;@02e4
	add	di,4	;@02e5
	dec	word ptr [bp-0ch]	;@02e8
	jz	@@done	;@02eb
	add	dx,[bp-4]	;@02ef
	jmp	short @@dst_line	;@02f2
@@src_line_done:
	add	si,4	;@02f4 naechste Quellzeile
	dec	word ptr [bp-0ah]	;@02f7
	jnz	@@next_src_line	;@02fa
@@done:
	pop	di	;@02fc
	pop	si	;@02fd
	pop	fs	;@02fe
	pop	es	;@0300
	pop	ds	;@0301
	mov	sp,bp	;@0302
	pop	bp	;@0304
	retf	;@0305
SEG046_TEXT	ends
	end
