; TOP-LEVEL MENU MANAGEMENT
; Subroutines to add and remove items from the top-level menu.

; Add an item to the top-level menu.
; INPUT:	X = Pointer to menu item struct
; OUTPUT:	None
;		D = Trashed
tl_add:
	stx	tl_item		; Store pointer to menu item

	clra			; Clear high byte of D for menu item length
	ldab	0,x		; Set low byte of D to length of item name
	addb	#3		; Increase length to include address pointer
	std	utw_s0		; Store length in utw_s0

	ldx	tl_item		; Copy item to runtime buffer, length utw_s0
	ldd	#rtb_bl
	os	ut$cpyb

	ldab	#$FF		; Position in menu ($FF for end)
	os	tl$addi		; Add item to main menu

	rts

tl_item:
	.WORD	$0000

; Remove an item from the top-level menu.
; INPUT:	X = Pointer to menu item struct
; OUTPUT:	None
tl_remove:
	os	tl$deli		; Remove item

	rts