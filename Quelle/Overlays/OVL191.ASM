;===========================================================================
; OVL191.ASM - Vordergrundbild mit Durchsicht ueber ein Hintergrundbild
; Sternenschweif (DSA 2), Attic 1994 - Rekonstruktion, bytegleich mit
; SCHWEIF.EXE C1.02 (TASM 3.1 /ml)
;
; Inhalt:    blit_masked_screen: zwei Vollbilder (320 x 200) Punkt fuer Punkt
;            nach A000 mischen: Vordergrund, wo er nicht 0 ist, sonst
;            Hintergrund.
; Aufrufer:  keiner (Overlay ohne Aufruf im Spiel; Einsprung bleibt erhalten)
; Aufruf:    far, C: void blit_masked_screen(unsigned char far *fg,
;            unsigned char far *bg). Stackrahmen bp, [bp+6] fg, [bp+0ah] bg.
; Register:  DS, ES, SI, DI gesichert; AX, BX, CX, DX, EAX veraendert.
; Daten:     keine.
; Herkunft:  aus SCHWEIF.EXE O191 (ab +0x0) mit asm2tasm.py zerlegt;
;            ;@nnnn = Offset im Overlay O191.
;===========================================================================
	.386p
SCREEN_W	equ	320
SCREEN_H	equ	200
	locals
OVL191_TEXT	segment byte public use16 'CODE'
	assume	cs:OVL191_TEXT, ds:nothing
	public	F_O191_0000
; ---------------------------------------------------------------------------
; blit_masked_screen - Vordergrund (0 = durchsichtig) ueber Hintergrund
; Ein:  [bp+6] fg far, [bp+0ah] bg far (je 320 x 200, Zeilen dicht)
; Aus:  -
; Locals: [bp-2] Zeile, [bp-6] Zielzeiger A000:0, [bp-0ah] fg, [bp-0eh] bg.
; Die Zeilenzeiger werden mit add dword um 320 weitergezaehlt (ein
; Offsetueberlauf liefe ins Segment); bei normalisierten 64000-Byte-
; Bildern kommt das nicht vor.
; ---------------------------------------------------------------------------
F_O191_0000	label	far	; Linkname von blit_masked_screen
blit_masked_screen:
	push	bp	;@0000
	mov	bp,sp	;@0001
	sub	sp,0eh	;@0003
	push	ds	;@0006
	push	es	;@0007
	push	si	;@0008
	push	di	;@0009
	mov	eax,[bp+6]	;@000a
	mov	[bp-0ah],eax	;@000e
	mov	eax,[bp+0ah]	;@0012
	mov	[bp-0eh],eax	;@0016
	mov	dword ptr [bp-6],0a0000000h	;@001a Ziel A000:0000
	mov	word ptr [bp-2],0	;@0022
@@next_row:
	mov	ax,[bp-2]	;@0027
	mov	bx,SCREEN_W	;@002a
	mul	bx	;@002d
	mov	di,ax	;@002f DI = Zeile * 320
	mov	cx,SCREEN_W	;@0031
	push	ds	;@0034
	mov	bx,[bp-4]	;@0035
	mov	es,bx	;@0038
	xor	dx,dx	;@003a
	cld	;@003c
@@next_pixel:
	lds	si,[bp-0ah]	;@003d
	add	si,dx	;@0040
	lodsb	;@0042
	and	al,al	;@0043
	jz	@@from_bg	;@0045 0 = durchsichtig
	stosb	;@0049 Vordergrundpunkt
	inc	dx	;@004a
	loop	@@next_pixel	;@004b
	jmp	@@row_done	;@004d
@@from_bg:
	lds	si,[bp-0eh]	;@0050
	add	si,dx	;@0053
	movsb	;@0055 Hintergrundpunkt
	inc	dx	;@0056
	loop	@@next_pixel	;@0057
@@row_done:
	pop	ds	;@0059
	add	dword ptr [bp-0ah],0140h	;@005a
	add	dword ptr [bp-0eh],0140h	;@0062
	inc	word ptr [bp-2]	;@006a
	cmp	word ptr [bp-2],SCREEN_H	;@006d
	jnz	@@next_row	;@0072
	pop	di	;@0074
	pop	si	;@0075
	pop	es	;@0076
	pop	ds	;@0077
	mov	sp,bp	;@0078
	pop	bp	;@007a
	retf	;@007b
OVL191_TEXT	ends
	end
