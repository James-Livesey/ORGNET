; CONFIGURATION APPLICATION
; Configures various settings for ORGNET. Shown as 'NETCFG' on top-level menu.

netcfg_item:
	.ASCIC	"NETCFG"
	.WORD	netcfg_main

; Main subroutine for NETCFG application.
; INPUT:	None
; OUTPUT:	None
;		D = Trashed
;		X = Trashed
netcfg_main:
	ldaa	#$0C		; Clear screen
	os	dp$emit

	ldab	netcfg_msg	; Print hello message to screen
	ldx	#netcfg_msg+1
	os	dp$prnt

	os	kb$getk		; Wait for keypress

	rts			; Exit application

netcfg_msg:
	.ASCIC	"Network config  goes here"