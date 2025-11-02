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
	.WORD	language
	.WORD	error_show_vec
	.WORD	comms_begin
	.WORD	comms_read_vec
	.WORD	comms_read_byte
	.WORD	comms_write_vec
	.WORD	comms_wait
endvec:

install:
	jsr	comms_init
	bcs	install_ret	; Return error if comms could not init

	ldaa	kbb_pkof	; Prevent auto pack switch-off
	staa	old_kbb_pkof
	ldaa	#0
	staa	kbb_pkof

	jsr	logo_show	; Show ORGNET logo

	jsr	apps_add_tl	; Add ORGNET apps to top-level menu

	os	kb$getk		; Wait for keypress

	clc			; Return success signal

install_ret:
	rts

remove:
	ldaa	#$0C		; Clear screen
	os	dp$emit

	ldab	remove_msg	; Print remove message to screen
	ldx	#remove_msg+1
	os	dp$prnt

	jsr	apps_rm_tl	; Remove ORGNET apps from top-level menu

	ldaa	old_kbb_pkof	; Restore auto pack switch-off behaviour
	staa	kbb_pkof

	os	kb$getk		; Wait for keypress

	clc			; Return success signal
	rts

remove_msg:
	.ASCIC	"Remove vector"

language:
	clc			; Return success signal
	rts

old_kbb_pkof:
	.BYTE 0

.INCLUDE vectors.inc
.INCLUDE error.inc
.INCLUDE comms.inc
.INCLUDE apps.inc
.INCLUDE appldr.inc
.INCLUDE udg.inc
.INCLUDE logo.inc
.INCLUDE tlmenu.inc

.EOVER

.INCLUDE netcfg.inc

.OVER prgend
.EOVER