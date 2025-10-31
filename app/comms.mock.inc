; COMMUNICATIONS MANAGER MOCK
; Mocked version of communications manager for use when testing in the emulator.

comms_pack_id:
	.BYTE	$01

comms_cur:
	.BYTE	$00

comms_buf:
	; Currently just an example return value for WISTAT command
	.BYTE	12
	.BYTE	$01
	.BYTE	$01
	.BYTE	$03
	.ASCII	"Mock Test"
	.BLKB	243
	; .BLKB	256

; Initialise the communications manager.
; INPUT:	None
; OUTPUT:	None
;		D = Trashed
;		C = Set on error
comms_init:
	jsr	comms_begin	; Set cursor to beginning

	clc			; Return success signal
	rts

; Set comms cursor to the beginning to read data from the comms buffer.
; INPUT:	None
; OUTPUT:	None
;		D = Trashed
;		X = Trashed
;		C = Set on error
comms_begin:
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
	staa	comms_read_ofst	; Store current cursor position as read offset
	inca
	aba
	bcs	comms_read_err	; (B + comms_cur + 1) > $FF

	deca			; Store new cursor position
	staa	comms_cur

	clra
	std	utw_s0		; Store length in utw_s0

	stx	comms_read_dst	; Store destination buffer address from X

	ldx	#comms_buf	; Load source (comms) buffer address into X
	ldab	comms_read_ofst
	abx			; Add read offset to source buffer address

	ldd	comms_read_dst	; Load destination buffer address into D

	os	ut$cpyb		; Copy comms buffer into destination buffer

	ldx	comms_read_dst	; Restore source buffer address into X

	clc			; Return success signal
	rts

comms_read_err:
	sec			; Return error signal
	rts

comms_read_ofst:
	.BYTE	$00

comms_read_dst:
	.WORD	$0000

; Read a single byte from the comms buffer at the current cursor location.
; INPUT:	None
; OUTPUT:	B = Byte read
;		C = Set if read would result in cursor exceeding buffer length
comms_read_byte:
	ldab	comms_cur
	incb
	cmpb	#$FF
	beq	comms_readb_err	; (comms_cur + 1) == $FF

	stab	comms_cur	; Store new cursor position

	ldx	#comms_buf	; Get address of byte in comms buffer
	decb
	abx			; Offset address by current cursor position

	ldab	0,x		; Read byte from comms buffer

	clc			; Return success signal
	rts

comms_readb_err:
	sec			; Return error signal
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
	bcs	comms_write_err	; (B + comms_cur + 1) > $FF

	deca			; Store new cursor position
	staa	comms_cur

	; TODO: Copy bytes into comms buffer and generate mock response

	clc			; Return success signal
	rts

comms_write_err:
	sec			; Return error signal
	rts

; Wait for a reply from the command already invoked using comms_write.
; INPUT:	None
; OUTPUT:	A = Trashed
;		B = Length of reply, excluding count and status bytes
;		C = Set if command execution was unsuccessful
comms_wait:
	ldaa	#20		; Set timeout to 1 second
	staa	comms_wait_timeout

comms_wait_poll:
	ldd	#1		; Brief wait for a response
	os	tm$wait

	jsr	comms_begin	; Begin reading response
	bcs	comms_wait_err

	jsr	comms_read_byte	; If count byte is 0, then continue waiting
	cmpb	#0
	beq	comms_wait_poll
	stab	comms_wait_rlen	; Store response length

	jsr	comms_read_byte	; Read status byte
	cmpb	#1		; If 1, then we have a successful response
	beq	comms_wait_success
	cmpb	#2		; If 2, then command execution was unsuccessful
	beq	comms_wait_err

	ldaa	comms_wait_timeout
	cmpa	#0		; If current timeout value is 0, throw error
	beq	comms_wait_err
	deca			; Otherwise, decrease timeout value
	staa	comms_wait_timeout

	bra	comms_wait_poll	; Continue waiting

comms_wait_success:
	ldab	comms_wait_rlen	; Load response length
	decb			; Skip status byte

	clc
	rts

comms_wait_err:
	sec			; Return error signal
	rts

comms_wait_timeout:
	.BYTE	$00

comms_wait_rlen:
	.BYTE	$00