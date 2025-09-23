; ERROR HANDLING
; Allows error messages to be displayed to users.

ERROR_COMMS	.EQU	$01

; Shows an error message based on the error code provided. This should be called
; as a branch instead of as a subroutine call if an early exit from the calling
; subroutine is desired. Otherwise, this subroutine can be called as normal to
; continue executing the calling subroutine.
; INPUTS:	A = Error code
; OUTPUTS:	D = Trashed
;		X = Trashed
error_show:
	; TODO: Customise error messages
	ldab	error_msg	; Print error message to screen
	ldx	#error_msg+1
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

error_msg:
	.ASCIC	"ORGNET ERROR    press space key"

error	.MACRO	error_code
	ldaa	#error_code
	jmp	error_show
.ENDM