; FILL.COM - Speicher fuellen/ausgeben (Untersuchung BCC-Restspeicher)
; FILL A pp..        konventionell (ab cs+1000h) und ganzes freies XMS mit Muster pp.. (Hex-Bytes)
; FILL C pp..        nur konventionell
; FILL X ssss llll pp..  XMS ab KB ssss, llll KB (hex) mit Muster
; FILL R ssss        alles mit Pseudozufall (Startwert ssss)
; FILL D             ganzes freies XMS nach DUMP.BIN, konventionell (ab cs+1000h) nach CONV.BIN
; FILL L ssss llll   XMS ab KB ssss, llll KB aus LOAD.BIN laden (Dateioffset 0)
	.model tiny
	.286
	.code
	org 100h
start:
	cld
	mov	si,81h
	call	skip
	lodsb
	or	al,20h
	mov	[mode],al
	cmp	al,'x'
	je	rng
	cmp	al,'l'
	je	rng
	cmp	al,'r'
	jne	pat0
	call	getw
	mov	[seed],ax
	mov	byte ptr [rnd],1
	mov	byte ptr [mode],'a'
	jmp	short go
rng:	call	getw
	mov	[xs],ax
	call	getw
	mov	[xl],ax
pat0:	xor	di,di
parse:	call	gethex
	jc	pdone
	mov	[pat+di],al
	inc	di
	cmp	di,32
	jb	parse
pdone:	or	di,di
	jnz	havepat
	inc	di
havepat:
	mov	[plen],di
go:	call	mkbuf
	cmp	byte ptr [mode],'d'
	je	doconv
	cmp	byte ptr [mode],'x'
	je	doxms
	cmp	byte ptr [mode],'l'
	je	doxms
; --- konventionell: ab cs+1000h bis Speicherende (PSP:2)
doconv:	cmp	byte ptr [mode],'d'
	jne	cfill
	mov	dx,offset fconv
	call	fcreate
cfill:	mov	dx,cs
	add	dx,1000h
	mov	bx,ds:[2]
conv:	cmp	dx,bx
	jae	convdone
	mov	ax,bx
	sub	ax,dx
	cmp	ax,800h
	jbe	csz
	mov	ax,800h			; max 32 KB je Schritt
csz:	mov	cx,ax
	shl	cx,4			; Bytes (0 = 64K nicht moeglich, max 8000h)
	push	bx
	push	dx
	cmp	byte ptr [mode],'d'
	jne	cf2
	push	ds
	mov	ds,dx
	xor	dx,dx
	mov	bx,cs:[fh]
	mov	ah,40h
	int	21h
	pop	ds
	jmp	short cf3
cf2:	mov	es,dx
	xor	di,di
	mov	si,offset buf
	rep	movsb
	push	cs
	pop	es
cf3:	pop	dx
	pop	bx
	add	dx,800h
	jmp	short conv
convdone:
	cmp	byte ptr [mode],'d'
	jne	cnoclose
	call	fclose
cnoclose:
	cmp	byte ptr [mode],'c'
	jne	doxms
	jmp	ende
; --- XMS
doxms:	mov	ax,4300h
	int	2fh
	cmp	al,80h
	je	x1
	jmp	ende
x1:	mov	ax,4310h
	int	2fh
	mov	word ptr [xms],bx
	mov	word ptr [xms+2],es
	push	cs
	pop	es
	mov	ah,08h
	call	dword ptr [xms]
	mov	[kb],ax
	or	ax,ax
	jnz	x2
	jmp	ende
x2:	mov	dx,ax
	mov	ah,09h
	call	dword ptr [xms]
	or	ax,ax
	jnz	x3
	jmp	ende
x3:	mov	[hdl],dx
	cmp	byte ptr [mode],'x'
	je	x4
	cmp	byte ptr [mode],'l'
	je	x4
	mov	word ptr [xs],0
	mov	ax,[kb]
	mov	[xl],ax
x4:	cmp	byte ptr [mode],'d'
	jne	x5
	mov	dx,offset fdump
	call	fcreate
x5:	cmp	byte ptr [mode],'l'
	jne	x6
	mov	dx,offset fload
	mov	ax,3d00h
	int	21h
	mov	[fh],ax
x6:	mov	ax,[xs]			; Start-KB -> Byteoffset
	mov	dx,1024
	mul	dx
	mov	word ptr [xoff],ax
	mov	word ptr [xoff+2],dx
	mov	cx,[xl]
xloop:	jcxz	xfree0
	jmp	short xbody
xfree0:	jmp	xfree
xbody:
	push	cx
	mov	word ptr [mv],1024
	mov	word ptr [mv+2],0
	cmp	byte ptr [mode],'d'
	je	xdump
	cmp	byte ptr [mode],'l'
	jne	xput
	mov	bx,[fh]
	mov	dx,offset buf
	mov	cx,1024
	mov	ah,3fh
	int	21h
xput:	; konventionell -> Handle
	mov	word ptr [m_sh],0
	mov	word ptr [m_so],offset buf
	mov	word ptr [m_so+2],cs
	mov	ax,[hdl]
	mov	[m_dh],ax
	mov	ax,word ptr [xoff]
	mov	word ptr [m_do],ax
	mov	ax,word ptr [xoff+2]
	mov	word ptr [m_do+2],ax
	mov	si,offset mv
	mov	ah,0bh
	call	dword ptr [xms]
	cmp	byte ptr [rnd],0
	je	xnext
	call	mkbuf
	jmp	short xnext
xdump:	; Handle -> konventionell, schreiben
	mov	ax,[hdl]
	mov	[m_sh],ax
	mov	ax,word ptr [xoff]
	mov	word ptr [m_so],ax
	mov	ax,word ptr [xoff+2]
	mov	word ptr [m_so+2],ax
	mov	word ptr [m_dh],0
	mov	word ptr [m_do],offset buf
	mov	word ptr [m_do+2],cs
	mov	si,offset mv
	mov	ah,0bh
	call	dword ptr [xms]
	mov	bx,[fh]
	mov	dx,offset buf
	mov	cx,1024
	mov	ah,40h
	int	21h
xnext:	add	word ptr [xoff],1024
	adc	word ptr [xoff+2],0
	pop	cx
	dec	cx
	jmp	xloop
xfree:	cmp	byte ptr [mode],'d'
	je	xcl
	cmp	byte ptr [mode],'l'
	jne	xfr
xcl:	call	fclose
xfr:	mov	dx,[hdl]
	mov	ah,0ah
	call	dword ptr [xms]
ende:	mov	ax,4c00h
	int	21h

fcreate	proc
	mov	ah,3ch
	xor	cx,cx
	int	21h
	mov	[fh],ax
	ret
fcreate	endp
fclose	proc
	mov	bx,[fh]
	mov	ah,3eh
	int	21h
	ret
fclose	endp

; Puffer (32 KB) mit Muster bzw. Zufall fuellen
mkbuf	proc
	push	cs
	pop	es
	mov	di,offset buf
	mov	cx,8000h
	cmp	byte ptr [rnd],0
	jne	mkr
	xor	bx,bx
mkp:	mov	al,[pat+bx]
	stosb
	inc	bx
	cmp	bx,[plen]
	jb	mkp2
	xor	bx,bx
mkp2:	loop	mkp
	ret
mkr:	mov	ax,[seed]
	mov	dx,25173
	mul	dx
	add	ax,13849
	mov	[seed],ax
	mov	al,ah
	stosb
	loop	mkr
	ret
mkbuf	endp

skip	proc
sk0:	lodsb
	cmp	al,' '
	je	sk0
	dec	si
	ret
skip	endp
; vier Hex-Ziffern -> ax
getw	proc
	call	gethex
	mov	bh,al
	call	gethex
	mov	ah,bh
	ret
getw	endp
; zwei Hex-Ziffern ab ds:si (fuehrende Leerzeichen ueberspringen) -> al, CF bei Ende
gethex	proc
gh0:	lodsb
	cmp	al,' '
	je	gh0
	cmp	al,13
	je	gherr
	call	nib
	jc	gherr
	mov	ah,al
	lodsb
	call	nib
	jc	gherr
	shl	ah,4
	or	al,ah
	clc
	ret
gherr:	dec	si
	stc
	ret
gethex	endp
nib	proc
	cmp	al,'0'
	jb	nberr
	cmp	al,'9'
	jbe	nbd
	or	al,20h
	cmp	al,'a'
	jb	nberr
	cmp	al,'f'
	ja	nberr
	sub	al,'a'-10
	clc
	ret
nbd:	sub	al,'0'
	clc
	ret
nberr:	stc
	ret
nib	endp

fdump	db	'DUMP.BIN',0
fconv	db	'CONV.BIN',0
fload	db	'LOAD.BIN',0
xms	dd	0
xoff	dd	0
kb	dw	0
hdl	dw	0
fh	dw	0
xs	dw	0
xl	dw	0
plen	dw	1
seed	dw	0
rnd	db	0
mode	db	0
pat	db	32 dup (0)
mv	dd	1024		; Laenge
m_sh	dw	0		; Quell-Handle 0 = konventionell
m_so	dd	0
m_dh	dw	0
m_do	dd	0
buf	label byte
	end	start
