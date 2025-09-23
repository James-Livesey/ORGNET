.INCLUDE MOSVARS.INC
.INCLUDE MOSHEAD.INC
.INCLUDE MSWI.INC

.ORG $241B-25

xx:
	.BYTE	$6C		; Bootable; paged; RAM-based
	.BYTE	$04		; Size 32K
	.BYTE	$00		; Code only
	.BYTE	$41		; Device number
	.BYTE	$10		; Version 1.0
	.BYTE	$41		; Priority
	.WORD	%root-%xx	; Device overlay address
	.WORD	int(0x6C04+0x0041+0x1041+%root-%xx)	; Checksum
	.BYTE	$09
	.BYTE	$81
	.ASCII	"MAIN    "
	.BYTE	$90
	.BYTE	$02
	.BYTE	$80
	.WORD	%prgend-%root	; Size of code

.OVER root

start:
	.WORD	$0000
	.BYTE	$00
	.BYTE	$41		; Device number
	.BYTE	$10		; Version 1.0
	.BYTE	(endvec-vec)/2	; Number of vectors

vec:
	.WORD	install
	.WORD	remove
endvec:

install:
	jsr	comms_init
	bcs	install_ret	; Return error if comms could not init

	jsr	comms_begin	; Say hello over the comms buffer
	bcs	install_ret
	ldab	hello_buffer
	incb
	ldx	#hello_buffer
	jsr	comms_write
	bcs	install_ret

	ldaa	kbb_pkof	; Prevent auto pack switch-off
	staa	old_kbb_pkof
	ldaa	#0
	staa	kbb_pkof

	jsr	logo_show	; Show ORGNET logo

	ldx	#netcfg_item	; Add NETCFG item to top-level menu
	jsr	tl_add

	os	kb$getk		; Wait for keypress

	clc			; Return success signal

install_ret:
	rts

hello_buffer:
	.ASCIC	"HELLO "

remove:
	ldaa	#$0C		; Clear screen
	os	dp$emit

	ldab	remove_msg	; Print remove message to screen
	ldx	#remove_msg+1
	os	dp$prnt

	ldx	#netcfg_item	; Remove NETCFG item from main menu
	jsr	tl_remove

	ldaa	old_kbb_pkof	; Restore auto pack switch-off behaviour
	staa	kbb_pkof

	os	kb$getk		; Wait for keypress

	clc			; Return success signal
	rts

remove_msg:
	.ASCIC	"Remove vector"

old_kbb_pkof:
	.BYTE 0

.INCLUDE error.inc
.INCLUDE comms.inc
.INCLUDE udg.inc
.INCLUDE logo.inc
.INCLUDE tlmenu.inc
.INCLUDE netcfg.inc

.EOVER

.OVER prgend
.EOVER