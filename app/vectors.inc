; CORE ROUTINE VECTORS
; Allows ORGNET apps to call common subroutines that reside in the permanent
; allocator cell from their own arbitrary memory location.

ERROR_SHOW	.EQU	3
COMMS_BEGIN	.EQU	4
COMMS_READ	.EQU	5
COMMS_READ_BYTE	.EQU	6
COMMS_WRITE	.EQU	7
COMMS_WAIT	.EQU	8

jvec	.MACRO	vector
	std	utw_s0		; Store D in utw_s0

	ldaa	#$41		; Device number
	ldab	#vector		; Vector number to call
	os	dv$vect		; Call vector
.ENDM