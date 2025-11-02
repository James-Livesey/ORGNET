; APP LOADER
; Dynamically loads ORGNET apps into memory from their overlays as required.

; Load an app into from the given pack address and execute it.
; INPUT:	X = Address of app overlay
;		D = Length of app overlay
; OUTPUT:	None
;		D = Trashed
;		X = Trashed
;		C = Set on error
appldr_load:
	stx	appldr_load_adr	; Store parameters for later
	std	appldr_load_len

	ldd	rta_sp		; Decrease stack pointer by app overlay length
	subd	appldr_load_len
	std	rta_sp		; This is where we'll load the app's code

	ldx	ala_free	; Check if enough space to load app onto stack
	ldab	#$FF		; Get top of allocator and add $FF
	abx
	cpx	rta_sp		; Check if collides with stack pointer
	bcc	appldr_load_mer	; Throw error if so (ala_free + $FF >= rta_sp)

	clra			; Set current pack first
	ldab	comms_pack_id
	os	pk$setp
	bcs	appldr_load_err

	ldx	appldr_load_adr	; Address of app's relocatable code on pack
	ldd	rta_sp		; Address to load code into memory
	std	utw_s0		; Fixup address (same as load address)
	os	dv$load		; Load app into memory and apply fixups
	bcs	appldr_load_err

	ldx	rta_sp		; Get app code's address in memory (on stack)
	ldx	0,x		; Read first word in code (entry point vector)
	jsr	0,x		; Execute code pointed to by entry point vector

	clc
	rts

appldr_load_mer:
	ldab	#254		; Message to show is 'NO MORE ROOM'
	bra	appldr_load_err

appldr_load_err:
	ldd	rta_sp		; Restore stack pointer to original position
	addd	appldr_load_len
	std	rta_sp

	os	er$mess		; Display error message

	sec			; Return error signal
	rts

appldr_load_adr:
	.WORD	$0000

appldr_load_len:
	.WORD	$0000

; Unload the currently loaded app.
; INPUT:	None
; OUTPUT:	None
;		D = Trashed
;		X = Trashed
appldr_unload:
	ldd	rta_sp		; Increase stack pointer by app overlay length
	addd	appldr_load_len
	std	rta_sp

	ldd 	#0		; Clear app overlay length for further calls
	std	appldr_load_len

	clc
	rts