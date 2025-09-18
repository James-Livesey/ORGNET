; Display ORGNET logo to screen.
; INPUT:	None
; OUTPUT:	None
; 		D = Trashed
; 		X = Trashed
logo_show:
	jsr	logo_udg_setup	; Set up user-defined graphics first

        ldaa	#$0C		; Clear screen
	os	dp$emit

	ldaa	#5		; Set cursor pos on first line
	ldab	#0
	os	dp$stat

	ldab	#6		; Print logo to screen
	ldx	#logo_str
	os	dp$prnt

	ldaa	#16+5		; Set cursor pos on second line
	ldab	#0
	os	dp$stat

	ldab	logo_brand	; Print brand name to screen
	ldx	#logo_brand+1
	os	dp$prnt

        rts

; Set up user-defined graphics for custom characters used in logo.
; INPUT:	None
; OUTPUT:	None
; 		D = Trashed
; 		X = Trashed
logo_udg_setup:
	ldaa	#0
	ldx	#logo_udg_0
	jsr	udg

	ldaa	#1
	ldx	#logo_udg_1
	jsr	udg

	ldaa	#2
	ldx	#logo_udg_2
	jsr	udg

	rts

logo_str:
	.BYTE	0
	.BYTE	1
	.BYTE	1
	.BYTE	1
	.BYTE	1
	.BYTE	2

logo_udg_0:
	.BYTE	0
	.BYTE	0
	.BYTE	0
	.BYTE	14
	.BYTE	16
	.BYTE	14
	.BYTE	0
	.BYTE	0

logo_udg_1:
	.BYTE	0
	.BYTE	0
	.BYTE	0
	.BYTE	0
	.BYTE	14
	.BYTE	0
	.BYTE	0
	.BYTE	0

logo_udg_2:
	.BYTE	0
	.BYTE	0
	.BYTE	0
	.BYTE	14
	.BYTE	1
	.BYTE	14
	.BYTE	0
	.BYTE	0

logo_brand:
	.ASCIC	"ORGNET"