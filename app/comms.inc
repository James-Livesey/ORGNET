; COMMUNICATIONS MANAGER
; Manages communications between the RP2350 and the Organiser. This is achieved
; by having some extra space reserved in the datapack that's emulated by the
; RP2350. The Organiser can write to that extra space in order to send commands
; and other data to the RP2350.

COMMS_LOCATION	.EQU	$2000

comms_pack_id:
	.BYTE $00

comms_cur:
	.BYTE $00

; Set if not on real ORGNET hardware - used for testing features on dummy
; hardware (such as normal datapack in emulator) only.
comms_is_dummy:
	.BYTE $00

; Initialise the communications manager.
; INPUT:	None
; OUTPUT:	None
;		D = Trashed
;		C = Set on error
comms_init:
	ldaa	pkb_curp	; Copy ORGNET's pack ID to global variable
	staa	comms_pack_id

	cmpa	#1		; Pack must be either B: or C:
	bcs	comms_err	; A < 1
	cmpa	#2
	bhi	comms_err	; A > 2

	jsr	comms_begin	; Determine if on dummy hardware
	bcs	comms_err
	jsr	comms_read_byte	; B = byte read
	ldaa	#$7F		; First byte is equal to $7F on real HW
	cba
	bne	comms_set_dummy

	jsr	comms_begin	; Set cursor back to beginning again

	clc			; Return success signal
	rts

comms_err:
	sec			; Return error signal
	rts

comms_set_dummy:
	ldaa	#1		; Indicate that current ORGNET HW is dummy
	staa	comms_is_dummy

	jsr	comms_begin	; Set cursor back to beginning again

	clc			; Return success signal
	rts

; Set comms cursor to the beginning to read data from the comms buffer.
; INPUT:	None
; OUTPUT:	None
;		D = Trashed
;		X = Trashed
;		C = Set on error
comms_begin:
	clra			; Set current pack first
	ldab	comms_pack_id
	os	pk$setp

	clrb			; Return error if error when reading from pack
	cmpb	#0
	bne	comms_err

	clrb			; Set pack address back to start
	ldx	#COMMS_LOCATION
	os	pk$sadd

	clra			; Set comms cursor back to start
	staa	comms_cur

	clc			; Return success signal
	rts

; Read bytes from the comms buffer at the current cursor location.
; INPUT:	B = Number of bytes to read from comms buffer
;		X = Address of buffer to copy bytes to
; OUTPUT:	None
;		D = Trashed
;		C = Set if read would result in cursor exceeding buffer length
comms_read:
	clc			; Check if cursor would exceed $FF
	ldaa	comms_cur
	inca
	aba
	bcs	comms_err	; (B + comms_cur + 1) > $FF

	deca			; Store new cursor position
	staa	comms_cur

	clra			; Read bytes from pack
	os	pk$read

	clc			; Return success signal
	rts

; Read a single byte from the comms buffer at the current cursor location.
; INPUT:	None
; OUTPUT:	A = Trashed
;		B = Byte read
;		C = Set if read would result in cursor exceeding buffer length
comms_read_byte:
	ldaa	comms_cur
	inca
	cmpa	#$FF
	beq	comms_err	; (comms_cur + 1) == $FF

	staa	comms_cur	; Store new cursor position

	os	pk$rbyt		; Read byte from pack

	clc			; Return success signal
	rts

; Write bytes to the comms buffer at the current cursor location.
; INPUT:	B = Number of bytes to write to comms buffer
;		X = Address of buffer to copy bytes from
; OUTPUT:	None
;		D = Trashed
;		C = Set if read would result in cursor exceeding buffer length
comms_write:
	clc			; Check if cursor would exceed $FF
	ldaa	comms_cur
	inca
	aba
	bcs	comms_err	; (B + comms_cur + 1) > $FF

	deca			; Store new cursor position
	staa	comms_cur

	clra			; Write bytes to pack
	os	pk$save

	clrb			; Return error if error when writing to pack
	cmpb	#0
	bne	comms_err

	clc			; Return success signal
	rts