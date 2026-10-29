;===========================================================================
; SEG033.ASM - PowerPacker-Entpacker (PP20)
; Sternenschweif (DSA 2), Attic 1994 - Rekonstruktion, bytegleich mit
; SCHWEIF.EXE C1.02 (TASM 3.1 /ml)
;
; Inhalt:    ppDecrunch, der Entpacker fuer PowerPacker-Daten (PP20, Nico
;            Francois, vom Amiga). Gepackt wird von hinten nach vorn: der
;            Bitstrom wird vom Ende der gepackten Daten rueckwaerts gelesen,
;            die Ausgabe vom Ende des Zielpuffers rueckwaerts geschrieben.
;            Am Ende der gepackten Daten stehen 3 Bytes Ausgabelaenge (big
;            endian) und 1 Byte Zahl der zu ueberspringenden Bits.
;            Bitstrom: 1 = Verweis, 0 = Literale (Zahl in 2-Bit-Gruppen,
;            Gruppe 3 = weiter), nach Literalen folgt immer ein Verweis.
;            Verweis: 2 Bit Laengenklasse n (Laenge n + 2), Offsetbreite aus
;            der Effizienztabelle; bei n = 3 waehlt ein Bit 7 Bit oder die
;            volle Breite, die Laenge waechst in 3-Bit-Gruppen (7 = weiter).
;            Zeiger sind huge: bei Ueberlauf wird das Segment um 1000h
;            (64 KB) verschoben.
; Aufrufer:  SEG029 unpack_pp20 (Bilder, NVF), far
; Aufruf:    far, C: void ppDecrunch(unsigned char far *src, unsigned char
;            far *dst, unsigned short eff_off, unsigned short eff_seg,
;            unsigned long size). [bp+6] src, [bp+0ah] dst, [bp+0eh]
;            Effizienztabelle (4 Bytes, hinter "PP20"), [bp+12h] size =
;            Laenge der gepackten Daten. [bp+0ah] wird als lineare Adresse
;            des Zielanfangs (Endetest) ueberschrieben.
; Register:  DS, ES, SI, DI gesichert; AX, BX, CX, DX veraendert.
;            DX = Bitpuffer (Markierungsbit oben), DS:SI = Lesezeiger,
;            ES:DI = Schreibzeiger.
; Daten:     keine.
; Herkunft:  aus SCHWEIF.EXE R33 (ab +0xe) mit asm2tasm.py zerlegt;
;            ;@nnnn = Offset im Segment R33. Die nop hinter Vorwaerts-
;            spruengen sind Fuellbytes aus Attics Uebersetzung.
;===========================================================================
	.386p
; GET_BIT - naechstes Bit des gepackten Stroms nach CF holen
; DX ist der Bitpuffer: shr liefert das unterste Bit; wird DX dabei 0, war
; das das Markierungsbit, und das naechste Wort (big endian) wird von DS:SI
; rueckwaerts gelesen (stc/rcr setzt die neue Markierung oben ein).
GET_BIT	macro
	local	@@have_bit, @@same_seg
	shr	dx,1
	jnz	@@have_bit
	nop
	sub	si,2
	jnc	@@same_seg
	nop
	push	ax
	mov	ax,ds
	sub	ax,01000h
	mov	ds,ax
	pop	ax
@@same_seg:
	mov	dx,[si]
	xchg	dh,dl
	stc
	rcr	dx,1
@@have_bit:
	endm
	locals
SEG033_TEXT	segment byte public use16 'CODE'
	assume	cs:SEG033_TEXT, ds:nothing
	public	F_R33_000E
; ---------------------------------------------------------------------------
; ppDecrunch - PowerPacker-Daten entpacken
; Ein:  src, dst, Effizienztabelle, Laenge der gepackten Daten (s. Kopf)
; Aus:  -
; ---------------------------------------------------------------------------
F_R33_000E	label	far	; Linkname von ppDecrunch
ppDecrunch:
	push	bp	;@000e
	mov	bp,sp	;@000f
	push	ds	;@0011
	push	es	;@0012
	push	si	;@0013
	push	di	;@0014
	mov	ax,[bp+6]	;@0015 DX:AX = src + size: Ende der gepackten Daten
	mov	dx,[bp+8]	;@0018
	add	ax,[bp+012h]	;@001b
	jnc	@@src_end	;@001e
	nop	;@0022
	add	dx,01000h	;@0023
@@src_end:
	mov	bx,[bp+014h]	;@0027 Hochwort der Laenge * 1000h aufs Segment
	shl	bx,0ch	;@002a
	add	dx,bx	;@002d
	mov	si,ax	;@002f
	mov	ds,dx	;@0031
	xor	dx,dx	;@0033
	sub	si,4	;@0035 SI auf die letzten 4 Bytes
	jnc	@@read_tail	;@0038
	nop	;@003c
	push	ax	;@003d
	mov	ax,ds	;@003e
	sub	ax,01000h	;@0040
	mov	ds,ax	;@0043
	pop	ax	;@0045
@@read_tail:
	mov	al,[si+3]	;@0046 AL = zu ueberspringende Bits
	mov	cl,[si]	;@0049 CL:BH:BL = Ausgabelaenge (big endian)
	mov	bh,[si+1]	;@004b
	mov	bl,[si+2]	;@004e
	and	al,al	;@0051
	jz	@@skip_done	;@0053
	nop	;@0057
@@skip_bits:
	GET_BIT		;@0058 Fuellbits verwerfen
	dec	al	;@0077
	jnz	@@skip_bits	;@0079
@@skip_done:
	mov	ch,0	;@007b
	push	dx	;@007d
	mov	ax,[bp+0ah]	;@007e ES:DI = dst + Ausgabelaenge
	mov	dx,[bp+0ch]	;@0081
	add	ax,bx	;@0084
	jnc	@@dst_end	;@0086
	nop	;@008a
	add	dx,01000h	;@008b
@@dst_end:
	shl	cx,0ch	;@008f
	add	dx,cx	;@0092
	mov	di,ax	;@0094
	mov	es,dx	;@0096
	mov	ax,[bp+0ah]	;@0098 [bp+0ah]/[bp+0ch] = lineare Adresse von dst (Endetest)
	mov	cx,[bp+0ch]	;@009b
	mov	bl,ch	;@009e
	shr	bl,4	;@00a0
	mov	bh,0	;@00a3
	shl	cx,4	;@00a5
	add	ax,cx	;@00a8
	mov	cx,0	;@00aa
	adc	cx,bx	;@00ad
	mov	[bp+0ah],ax	;@00af
	mov	[bp+0ch],cx	;@00b2
	pop	dx	;@00b5
@@main_loop:
	GET_BIT		;@00b6 Hauptschleife: 1 = Verweis, 0 = Literale
	jnc	short @@literals	;@00d5
	jmp	near ptr @@match	;@00d7
@@literals:
	mov	cx,1	;@00da Literale: Zahl = 1 + Summe der 2-Bit-Gruppen
@@lit_count:
	xor	ax,ax	;@00dd
	GET_BIT		;@00df
	rcl	al,1	;@00fe
	GET_BIT		;@0100
	rcl	al,1	;@011f
	add	cx,ax	;@0121
	cmp	al,3	;@0123 Gruppe 3: weiter zaehlen
	jz	@@lit_count	;@0125
@@lit_byte:
	mov	al,0	;@0127 ein Literal: 8 Bit
	GET_BIT		;@0129
	rcl	al,1	;@0148
	GET_BIT		;@014a
	rcl	al,1	;@0169
	GET_BIT		;@016b
	rcl	al,1	;@018a
	GET_BIT		;@018c
	rcl	al,1	;@01ab
	GET_BIT		;@01ad
	rcl	al,1	;@01cc
	GET_BIT		;@01ce
	rcl	al,1	;@01ed
	GET_BIT		;@01ef
	rcl	al,1	;@020e
	GET_BIT		;@0210
	rcl	al,1	;@022f
	sub	di,1	;@0231 rueckwaerts schreiben
	jnc	@@lit_store	;@0234
	nop	;@0238
	push	ax	;@0239
	mov	ax,es	;@023a
	sub	ax,01000h	;@023c
	mov	es,ax	;@023f
	pop	ax	;@0241
@@lit_store:
	mov	es:[di],al	;@0242
	loop	@@lit_next	;@0245
	jmp	short @@lit_done	;@0247
@@lit_next:
	jmp	near ptr @@lit_byte	;@0249
@@lit_done:
	mov	ax,di	;@024c Ziel ganz geschrieben?
	mov	cx,es	;@024e
	mov	bl,ch	;@0250
	shr	bl,4	;@0252
	mov	bh,0	;@0255
	shl	cx,4	;@0257
	add	ax,cx	;@025a
	mov	cx,0	;@025c
	adc	cx,bx	;@025f
	cmp	cx,[bp+0ch]	;@0261
	ja	@@match	;@0264
	nop	;@0268
	jnc	short @@lit_cmp_low	;@0269
	jmp	near ptr @@done	;@026b
@@lit_cmp_low:
	cmp	ax,[bp+0ah]	;@026e
	ja	@@match	;@0271
	nop	;@0275
	jmp	near ptr @@done	;@0276
@@match:
	mov	bl,0	;@0279 Verweis: 2 Bit Laengenklasse
	GET_BIT		;@027b
	rcl	bl,1	;@029a
	GET_BIT		;@029c
	rcl	bl,1	;@02bb
	mov	bh,0	;@02bd
	push	es	;@02bf
	push	di	;@02c0
	les	di,[bp+0eh]	;@02c1 CL = Offsetbreite aus der Effizienztabelle
	mov	cl,es:[bx+di]	;@02c4
	pop	di	;@02c7
	pop	es	;@02c8
	cmp	bl,3	;@02c9
	jz	short @@match_long	;@02cc
	jmp	near ptr @@match_short	;@02ce
@@match_long:
	GET_BIT		;@02d1 Klasse 3: ein Bit waehlt 7 Bit oder volle Breite
	jc	@@match_offset	;@02f0
	nop	;@02f4
	mov	cl,7	;@02f5
@@match_offset:
	xor	ax,ax	;@02f7
	mov	ch,0	;@02f9
@@long_offset_bit:
	GET_BIT		;@02fb Offset lesen
	rcl	ax,1	;@031a
	loop	@@long_offset_bit	;@031c
	push	ax	;@031e
@@len_ext:
	xor	cx,cx	;@031f Laengenerweiterung in 3-Bit-Gruppen
	GET_BIT		;@0321
	rcl	cl,1	;@0340
	GET_BIT		;@0342
	rcl	cl,1	;@0361
	GET_BIT		;@0363
	rcl	cl,1	;@0382
	add	bx,cx	;@0384
	cmp	cl,7	;@0386
	jz	@@len_ext	;@0389
	jmp	@@match_len	;@038b
@@match_short:
	mov	ch,0	;@038e Klasse 0..2: Offset mit voller Breite
	xor	ax,ax	;@0390
@@short_offset_bit:
	GET_BIT		;@0392
	rcl	ax,1	;@03b1
	loop	@@short_offset_bit	;@03b3
	push	ax	;@03b5
@@match_len:
	add	bx,2	;@03b6 CX = Laenge = Klasse + Erweiterung + 2, BX = Offset
	mov	cx,bx	;@03b9
	pop	bx	;@03bb
@@copy:
	mov	al,es:[bx+di]	;@03bc Byte aus der schon geschriebenen Ausgabe
	sub	di,1	;@03bf
	jnc	@@copy_store	;@03c2
	nop	;@03c6
	push	ax	;@03c7
	mov	ax,es	;@03c8
	sub	ax,01000h	;@03ca
	mov	es,ax	;@03cd
	pop	ax	;@03cf
@@copy_store:
	mov	es:[di],al	;@03d0
	loop	@@copy	;@03d3
	mov	ax,di	;@03d5 Ziel ganz geschrieben?
	mov	cx,es	;@03d7
	mov	bl,ch	;@03d9
	shr	bl,4	;@03db
	mov	bh,0	;@03de
	shl	cx,4	;@03e0
	add	ax,cx	;@03e3
	mov	cx,0	;@03e5
	adc	cx,bx	;@03e8
	cmp	cx,[bp+0ch]	;@03ea
	jna	short @@cmp_low	;@03ed
	jmp	near ptr @@main_loop	;@03ef
@@cmp_low:
	jc	@@done	;@03f2
	nop	;@03f6
	cmp	ax,[bp+0ah]	;@03f7
	jna	short @@done	;@03fa
	jmp	near ptr @@main_loop	;@03fc
@@done:
	pop	di	;@03ff
	pop	si	;@0400
	pop	es	;@0401
	pop	ds	;@0402
	mov	sp,bp	;@0403
	pop	bp	;@0405
	retf	;@0406
SEG033_TEXT	ends
	end
