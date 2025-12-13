; COMMUNICATIONS MANAGER MOCK
; Mocked version of communications manager for use when testing in the emulator.

comms_pack_id:
	.BYTE	$01

comms_cur:
	.BYTE	$00

comms_buf:
	.BLKB	256

comms_buf_ofst:
	.BYTE	$00

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
	staa	comms_buf_ofst	; Store current cursor position as read offset
	inca
	aba
	bcs	comms_read_err	; (B + comms_cur + 1) > $FF

	deca			; Store new cursor position
	staa	comms_cur

	clra
	std	utw_s0		; Store length in utw_s0

	stx	comms_read_dst	; Store destination buffer address from X

	ldx	#comms_buf	; Load source (comms) buffer address into X
	ldab	comms_buf_ofst
	abx			; Add read offset to source buffer address

	ldd	comms_read_dst	; Load destination buffer address into D

	os	ut$cpyb		; Copy comms buffer into destination buffer

	ldx	comms_read_dst	; Restore source buffer address into X

	clc			; Return success signal
	rts

comms_read_err:
	sec			; Return error signal
	rts

comms_read_dst:
	.WORD	$0000

comms_read_vec:
	ldd	utw_s0
	bra	comms_read

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
	staa	comms_buf_ofst	; Store current cursor position as write offset
	inca
	aba
	bcs	comms_write_err	; (B + comms_cur + 1) > $FF

	deca			; Store new cursor position
	staa	comms_cur

	clra			; Set utw_s0 to number of bytes to copy
	std	utw_s0

	ldab	comms_buf_ofst	; Destination is comms buffer + offset
	ldd	#comms_buf

	os	ut$cpyb		; Copy source buffer into comms buffer

	jsr	comms_mock	; Generate mock response

	clc			; Return success signal
	rts

comms_write_err:
	sec			; Return error signal
	rts

comms_write_vec:
	ldd	utw_s0
	bra	comms_write

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

; Take the current command from the comms buffer and generate an appropriate
; mock response.
; INPUT:	None
; OUTPUT:	None
; 		D = Trashed
; 		X = Trashed
comms_mock:
	ldab	$00
	stab	comms_mock_idx

comms_mock_loop:
	ldx	#comms_mock_cmds
	ldab	comms_mock_idx	; Get command address to compare from table
	abx

	ldx	0,x		; Load command address pointed to by table entry
	cpx	#$0000		; Check if at end of table
	beq	comms_mock_ret	; If so, return

	clra
	ldab	0,x		; Get command length
	inx			; Skip over count byte in command
	stx	utw_s0		; Store as second string to compare

	ldx	#comms_buf	; Load comms buffer first string to compare
	ldaa	0,x		; Get buffer contents length
	inx			; Skip over count byte in buffer contents

	os	ut$icpb		; Perform string comparison
	beq	comms_mock_copy	; Copy response if match

	ldab	comms_mock_idx	; Move onto next command address in table
	addb	#4		; Skipping over 2 words (command and response)
	stab	comms_mock_idx

	bra	comms_mock_loop

comms_mock_copy:
	ldx	#comms_mock_cmds
	ldab	comms_mock_idx	; Get address of command
	abx

	ldx	2,x		; Load response address from table (next word)
	clra
	ldab	0,x		; Get response length
	incb			; Include count byte in length
	std	utw_s0		; Store response length in utw_s0
	ldd	#comms_buf	; Destination is comms buffer
	os	ut$cpyb		; Copy response to comms buffer

comms_mock_ret:
	rts

comms_mock_idx:
	.BYTE	$00

comms_mock_wistat_cmd:
	.ASCIC	"WISTAT "

comms_mock_wistat_res:
	.BYTE	12		; Response length
	.BYTE	$01		; Response status
	.BYTE	$01		; Connection status
	.BYTE	$02		; Connection strength
	.ASCII	"Mock Test"	; SSID

comms_mock_wilist_0_cmd:
	.BYTE	9		; Command length
	.ASCII	"WILIST "	; Command
	.WORD	$0000		; Index parameter

comms_mock_wilist_0_res:
	.BYTE	13		; Response length
	.BYTE	$01		; Response status
	.BYTE	$02		; Connection strength
	.BYTE	$00		; Security type
	.ASCII	"First Mock"	; SSID

comms_mock_wilist_1_cmd:
	.BYTE	9		; Command length
	.ASCII	"WILIST "	; Command
	.WORD	$0001		; Index parameter

comms_mock_wilist_1_res:
	.BYTE	36		; Response length
	.BYTE	$01		; Response status
	.BYTE	$02		; Connection strength
	.BYTE	$00		; Security type
	.ASCII	"Second Mock With Really Long Name"	; SSID

comms_mock_wilist_2_cmd:
	.BYTE	9		; Command length
	.ASCII	"WILIST "	; Command
	.WORD	$0002		; Index parameter

comms_mock_wilist_2_res:
	.BYTE	13		; Response length
	.BYTE	$01		; Response status
	.BYTE	$02		; Connection strength
	.BYTE	$00		; Security type
	.ASCII	"Third Mock"	; SSID

comms_mock_wilist_3_cmd:
	.BYTE	9		; Command length
	.ASCII	"WILIST "	; Command
	.WORD	$0003		; Index parameter

comms_mock_wilist_3_res:
	.BYTE	1		; Response length
	.BYTE	$01		; Response status

comms_mock_cmds:
	.WORD	comms_mock_wistat_cmd
	.WORD	comms_mock_wistat_res
	.WORD	comms_mock_wilist_0_cmd
	.WORD	comms_mock_wilist_0_res
	.WORD	comms_mock_wilist_1_cmd
	.WORD	comms_mock_wilist_1_res
	.WORD	comms_mock_wilist_2_cmd
	.WORD	comms_mock_wilist_2_res
	.WORD	comms_mock_wilist_3_cmd
	.WORD	comms_mock_wilist_3_res
	.WORD	$0000