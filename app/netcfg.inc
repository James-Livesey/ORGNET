; CONFIGURATION APPLICATION
; Configures various settings for ORGNET. Shown as 'NETCFG' on top-level menu.

WISTATUS_DISCON	.EQU	$00
WISTATUS_CON	.EQU	$01

.OVER netcfg

netcfg_entry:
	.WORD	netcfg_main

; Main subroutine for NETCFG application. Shows a menu of network configuration
; options.
; INPUT:	None
; OUTPUT:	None
;		D = Trashed
;		X = Trashed
netcfg_main:
	ldx	#netcfg_menu
	ldd	#$1001
	os	mn$disp		; Open main menu

	cmpb	#1		; Exit if ON is pressed
	beq	netcfg_main_exit

	jsr	0,x		; Otherwise, visit selected area

	bra	netcfg_main

netcfg_main_exit:
	rts			; Exit application

netcfg_menu:
	.ASCIC	"WIFI"
	.WORD	netcfg_wifi
	.ASCIC	"APPS"
	.WORD	netcfg_apps
	.ASCIC	"ABOUT"
	.WORD	netcfg_about
	.BYTE	$00

; Displays Wi-Fi status and configuration options.
; INPUT:	None
; OUTPUT:	None
;		D = Trashed
;		X = Trashed
netcfg_wifi:
	jvec	COMMS_BEGIN	; Begin comms to check Wi-Fi status
	bcs	netcfg_wifi_err

	ldab	netcfg_wistat_cmd
	incb
	ldx	#netcfg_wistat_cmd
	jvec	COMMS_WRITE	; Invoke WISTAT command

	jvec	COMMS_WAIT	; Wait for reply and store SSID length from A
	bcs	netcfg_wifi_err
	decb			; Discount connection status byte in length
	decb			; Discount signal strength byte in length
	andb	#$0F		; Limit length to 15 chars to fit buffer
	stab	netcfg_wissid_disp

	jvec	COMMS_READ_BYTE	; Store connection status
	bcs	netcfg_wifi_err
	stab	netcfg_wistatus

	jvec	COMMS_READ_BYTE	; Store connection strength
	bcs	netcfg_wifi_err
	stab	netcfg_wistrength

	ldab	netcfg_wissid_disp
	ldx	#netcfg_wissid_disp+1
	jvec	COMMS_READ	; Read SSID into its buffer
	bcs	netcfg_wifi_err

	bra	netcfg_wifi_draw

netcfg_wifi_err:
	ldab	#ERROR_COMMS
	jvec	ERROR_SHOW	; Show error message and exit subroutine
	rts

netcfg_wifi_draw:
	ldaa	#60		; Set poll interval to 3 seconds
	staa	netcfg_wipoll_timer

	ldaa	#$0C		; Clear screen
	os	dp$emit

	ldaa	netcfg_wistatus	; Show disconnected message if disconnected
	cmpa	#0
	beq	netcfg_widiscon

	ldab	netcfg_wissid_disp
	ldx	#netcfg_wissid_disp+1
	os	dp$prnt		; Print SSID to screen

	ldaa	#16		; Set cursor pos on second line
	clrb
	os	dp$stat

	ldx	#netcfg_wistatus_msgs
	ldab	netcfg_wistatus
	abx			; Add status index as word offset to table
	abx
	ldx	0,x		; Load address pointed to by table entry
	ldab	0,x		; Get status message length
	inx			; Skip over count byte in string
	os	dp$prnt		; Print current status message to screen

	bra	netcfg_wifi_poll

netcfg_widiscon:
	ldab	netcfg_widiscon_msg
	ldx	#netcfg_widiscon_msg+1
	os	dp$prnt		; Print disconnected message to screen

netcfg_wifi_poll:
	os	kb$brek		; Exit if ON is pressed
	bcs	netcfg_wifi_exit

	os	kb$test		; Get last pressed key
	; TODO: Make MODE key open connection options menu

	os	kb$uget		; Unget the key

	ldd	#1		; Brief wait as part of timer
	os	tm$wait

	ldaa	netcfg_wipoll_timer
	cmpa	#0		; If current timer value is 0, update status
	beq	netcfg_wifi_update
	deca			; Otherwise, decrease timer value
	staa	netcfg_wipoll_timer

	bra	netcfg_wifi_poll

netcfg_wifi_update:
	jmp	netcfg_wifi

netcfg_wifi_exit:
	rts			; Exit area

netcfg_wistat_cmd:
	.ASCIC	"WISTAT "

netcfg_widiscon_msg:
	.ASCIC	"DISCONNECTED    press MODE key"

; SSID display - length limited to 15 chars (first byte is char count)
netcfg_wissid_disp:
	.BLKB	16

netcfg_wistatus:
	.BYTE	WISTATUS_DISCON

netcfg_wistatus_con_msg:
	.ASCIC	"CONNECTED"

; Wi-Fi status message table to be indexed into using netcfg_wistatus
netcfg_wistatus_msgs:
	.WORD	$0000
	.WORD	netcfg_wistatus_con_msg

netcfg_wistrength:
	.BYTE	$00

netcfg_wipoll_timer:
	.BYTE	$00

; Displays options to configure which built-in ORGNET apps are shown.
; INPUT:	None
; OUTPUT:	None
netcfg_apps:
	rts

; Displays about information.
; INPUT:	None
; OUTPUT:	None
;		D = Trashed
;		X = Trashed
netcfg_about:
	ldaa	#$0C		; Clear screen
	os	dp$emit

	ldab	netcfg_about_msg1
	ldx	#netcfg_about_msg1+1
	os	dp$prnt		; Print first message to screen

	os	kb$getk		; Wait for keypress; exit if ON is pressed
	cmpb	#1
	beq	netcfg_about_exit

	ldaa	#$0C		; Clear screen
	os	dp$emit

	ldab	netcfg_about_msg2
	ldx	#netcfg_about_msg2+1
	os	dp$prnt		; Print second message to screen

	os	kb$getk		; Wait for keypress

netcfg_about_exit:
	rts			; Exit area

netcfg_about_msg1:
	.ASCIC	"ORGNET FW V1.0  orgnet.jamesl.me"

netcfg_about_msg2:
	.ASCIC	"C Copyright 2025James Livesey"

.EOVER

.OVER netcfgend
.EOVER