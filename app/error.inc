; ERROR HANDLING
; Allows error messages to be displayed to users.

ERROR_GENERIC	.EQU	$00
ERROR_COMMS	.EQU	$01

; Shows an error message based on the error code provided and waits for the user
; to dismiss it. This should be called as a branch instead of as a subroutine
; call if an early exit from the calling subroutine is desired. Otherwise, this
; subroutine can be called as normal to continue executing the calling
; subroutine.
; INPUTS:	B = Error code
; OUTPUTS:	D = Trashed
;		X = Trashed
error_show:
	ldaa	#$0C		; Clear screen
	os	dp$emit

	ldx	#error_msgs
	abx			; Add error code as word offset to table
	abx
	ldx	0,x		; Load address pointed to by table entry
	ldab	0,x		; Get error message length
	inx			; Skip over count byte in string
	os	dp$prnt		; Print current error message to screen

	ldaa	#16		; Set cursor pos on second line
	clrb
	os	dp$stat

	ldab	error_prompt	; Print error prompt to screen
	ldx	#error_prompt+1
	os	dp$prnt

error_key:
	os	kb$getk		; Wait for keypress
	cmpb	#$01		; If ON/CLEAR, then return
	beq	error_ret
	cmpb	#$20		; If space, then return
	beq	error_ret
	
	bra	error_key	; Otherwise, wait for another keypress

error_ret:
	rts

error_prompt:
	.ASCIC	"press space key"

error_generic:
	.ASCIC	"ORGNET ERROR"

error_comms:
	.ASCIC	"PACK COMMS FAIL"

error_msgs:
	.WORD	error_generic
	.WORD	error_comms

error_show_vec:
	ldd	utw_s0
	jmp	error_show