;===========================================================================
; SEG031.ASM - Tastaturpuffer leeren
; Sternenschweif (DSA 2), Attic 1994 - Rekonstruktion, bytegleich mit
; SCHWEIF.EXE C1.02 (TASM 3.1 /ml)
;
; Inhalt:    clear_kbd (alle wartenden Tasten verwerfen) und wait_key_clear
;            (Puffer leeren, dann auf eine neue Taste warten).
; Aufrufer:  clear_kbd: viele C-Module (13 far-Aufrufer);
;            wait_key_clear: O032 (Speichermeldung beim Start)
; Aufruf:    far, keine Parameter, kein Stackrahmen.
; Register:  AX (Rueckgabe von bioskey), sonst wie Borland bioskey.
; Daten:     keine. Benutzt die Borland-Laufzeit _bioskey (int 16h).
; Herkunft:  aus SCHWEIF.EXE R31 (ab +0x5) mit asm2tasm.py zerlegt;
;            ;@nnnn = Offset im Segment R31.
;===========================================================================
	.386p
	locals
	extrn	_bioskey:far
SEG031_TEXT	segment byte public use16 'CODE'
	assume	cs:SEG031_TEXT, ds:nothing
	public	F_R31_0005
	public	F_R31_0020
; ---------------------------------------------------------------------------
; clear_kbd - Tastaturpuffer leeren
; Ein:  -
; Aus:  AX = 0
; Solange bioskey(1) eine wartende Taste meldet, wird sie mit bioskey(0)
; abgeholt und verworfen.
; ---------------------------------------------------------------------------
F_R31_0005	label	far	; Linkname von clear_kbd
clear_kbd:
	jmp	short @@test_key	;@0005
@@take_key:
	push	0	;@0007 bioskey(0): Taste abholen
	call	far ptr _bioskey	;@0009
	add	sp,2	;@000e
@@test_key:
	push	1	;@0011 bioskey(1): Taste wartend?
	call	far ptr _bioskey	;@0013
	add	sp,2	;@0018
	or	ax,ax	;@001b
	jnz	@@take_key	;@001d
	retf	;@001f
; ---------------------------------------------------------------------------
; wait_key_clear - Puffer leeren und auf eine Taste warten
; Ein:  -
; Aus:  AX = Tastencode (bioskey(0): Scancode << 8 | ASCII)
; ---------------------------------------------------------------------------
F_R31_0020	label	far	; Linkname von wait_key_clear
wait_key_clear:
	push	cs	;@0020
	call	near ptr clear_kbd	;@0021
	push	0	;@0024 bioskey(0) wartet auf die naechste Taste
	call	far ptr _bioskey	;@0026
	add	sp,2	;@002b
	retf	;@002e
SEG031_TEXT	ends
	end
