; Set a user-defined graphic for character A from the bitmap provided in X.
; INPUT:	A = ASCII character of user-defined graphic (0-7)
; 		X = Index pointing to start of bitmap (8 bytes)
; OUTPUT:	None
; 		D = Trashed
; 		X = Trashed
udg:
	staa	udg_char	; Store character value into variable
	clrb
	stab	udg_byte_idx	; Reset byte index variable

	jsr	udg_wait	; Wait for LCD controller

	ldaa	#8		; Command = 64 + (udg_char * 8)
	ldab	udg_char
	mul
	addb	#64
	stab	SCA_LCDCONTROL

udg_byte_loop:
	jsr	udg_wait	; Wait for LCD controller

	ldaa	0,x		; Load value of bitmap pointer
	staa	SCA_LCDDATA	; Send current byte to LCD controller

	inx			; Increment data pointer

	ldaa	udg_byte_idx	; Increment byte index
	inca
	staa	udg_byte_idx

	ldab	#7
	cba
	bls	udg_byte_loop	; udg_byte_idx <= 7

	rts

udg_char:
	.BYTE	0

udg_byte_idx:
	.BYTE	0

; Wait for LCD controller so that it is ready to accept a command or data value.
; INPUT:	None
; OUTPUT:	None
; 		A = Trashed
udg_wait:
	ldaa	SCA_LCDCONTROL	; Bit 7 indicates controller busy when set
	cmpa	#$80
	bcc	udg_wait	; a >= $80

	rts