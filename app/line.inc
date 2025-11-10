; LINE-BASED INFORMATION RENDERER
; Subroutines used to display interactive information in a scrollable line-based
; format, such as lists or documents.

; Line reader callback: called when the contents of a line needs to be read. The
; contents of the line must be provided in a byte-counted buffer, and its
; address must be returned in X. Its maximum length must be 128 (including count
; byte).
; A line may have hotspots, indicating elements which can be selected to pick an
; option or activate a hyperlink. A list of hotspots must be provided in a byte-
; counted buffer, and its address must be returned in D. Its maximum length must
; be 128 (including count byte). This list must comprise of $01 bytes where
; there is to be a hotspot at the same position in the line, or $00 where there
; is to not be a hotspot.
; If a line at the given index does not exist, then C must be set. In this case,
; X and D do not need to be specified.
; INPUT:	D = Index of line to be read
; OUTPUT:	X = Address of byte-counted buffer containing line's contents
;		D = Address of byte-counted buffer containing hotspots
;		C = Set if line does not exist (reached end)
line_read_cbk:
	.WORD	$0000

; Scroll X position
line_scrlx:
	.BYTE	$00

; Scroll Y position
line_scrly:
	.WORD	$0000

; Cursor X position
line_curx:
	.BYTE	$00

; Cursor Y position
line_cury:
	.WORD	$0000

line_buf_cache:
	.BLKB	128

line_hsp_cache:
	.BLKB	128

; Set up line renderer callbacks and reset scrolling.
; INPUT:	X = Address of line reader callback routine
; OUTPUT:	None
;		D = Trashed
line_begin:
	stx	line_read_cbk	; Store line reader callback

	ldd	#0
	staa	line_scrlx	; Reset scroll X position
	std	line_scrly	; Reset scroll Y position
	staa	line_curx	; Reset cursor X position
	std	line_cury	; Reset cursor Y position

	rts

line_begin_vec:
	ldd	utw_s0
	bra	line_begin

; Draw a line given by the address of its buffer, at the given scroll X
; position.
; INPUT:	A = Scroll X position
;		B = Row of display to draw line on
;		X = Address of line buffer
; OUTPUT:	None
;		D = Trashed
;		X = Trashed
line_draw_scrlx:
	staa	line_dscrlx	; Store scroll X position for later

	ldaa	#16		; Set cursor pos to start of given row
	mul			; B = row * 16
	tba			; Move B into A
	os	dp$stat

	ldaa	0,x		; Get length of line
	ldab	line_dscrlx	; Get current scroll X position
	cba			; Don't do anything if scrolled past end of line
	bls	line_dscrlx_end	; (Length <= line_scrlx)

	suba	line_dscrlx	; Decrease line length by scroll X position
	abx			; Add current scroll X position to line address
	inx			; Skip over count byte

	cmpa	#16		; Don't cap length if line fits on display
	bls	line_dscrlx_ncp	; (Length <= 16)
	ldaa	#16		; Set length to 16

line_dscrlx_ncp:
	tab			; Move A (line length) into B
	os	dp$prnt		; Print the line
	tab			; Move A into B again

line_dscrlx_fll:
	cmpb	#16		; Don't fill rest of line if at end
	bcc	line_dscrlx_end	; (Column counter >= 16)

	ldaa	#$20		; Fill rest of line with spaces
	os	dp$emit

	incb			; Increment column counter

	bra	line_dscrlx_fll	; Loop to continue filling line

line_dscrlx_end:
	rts

line_dscrlx:
	.BYTE	$00

; Draw a single line at the current scroll position.
; INPUT:	B = Row of display to draw line on
; OUTPUT:	C = Set if line does not exist (reached end)
;		D = Trashed
;		X = Trashed
line_draw_sgl:
	stab	line_dsgl_row	; Save display row for later
	clra			; Get current scroll Y + display row offset
	addd	line_scrly
	std	line_dsgl_scrly	; Save scroll Y offset for later

	ldx	line_read_cbk
	jsr	0,x		; Get contents of line and hotspots
	bcs	line_dsgl_end	; Early exit if line does not exist
	stx	line_buf_ptr	; Store pointers to returned buffers
	std	line_hsp_ptr

	clra			; Clear scroll X pos for line_draw_scrlx
	ldx	line_cury	; Check if cursor is on this line
	cpx	line_dsgl_scrly	; If not, then don't cache it
	bne	line_dsgl_nocache

	ldx	line_buf_ptr	; Load line buffer's address into X
	ldab	0,x		; Read count byte in line buffer (A already 0)
	incb			; Increment to include count byte in length
	cmpb	#128		; Don't cache line if longer than 128 chars
	bhi	line_dsgl_end	; (Length > 128)
	std	utw_s0		; Store buffer length in utw_s0
	ldd	#line_buf_cache	; Destination is line buffer cache
	os	ut$cpyb		; Copy line into cache

	ldx	line_hsp_ptr	; Load line buffer's address into X
	clra			; Read first (count) byte of line buffer
	ldab	0,x
	incb			; Increment to include count byte in length
	cmpb	#128		; Don't cache line if longer than 128 chars
	bhi	line_dsgl_end	; (Length > 128)
	std	utw_s0		; Store buffer length in utw_s0
	ldd	#line_hsp_cache	; Destination is line hotspot cache
	os	ut$cpyb		; Copy line into cache

	ldaa	line_scrlx	; Load line scroll X into A

line_dsgl_nocache:
	ldab	line_dsgl_row	; Load row into B
	ldx	line_buf_ptr	; Load line buffer's address into X
	jsr	line_draw_scrlx	; Draw line with scroll X offset

	clc
	rts

line_dsgl_end:
	rts

line_dsgl_row:
	.BYTE	$00

line_dsgl_scrly:
	.WORD	$0000

line_buf_ptr:
	.WORD	$0000

line_hsp_ptr:
	.WORD	$0000

; Draw all lines at the current scroll position.
; INPUT:	None
; OUTPUT:	None
;		D = Trashed
;		X = Trashed
line_draw_all:
	ldaa	#$0C		; Clear screen
	os	dp$emit

	ldab	#0		; Draw top row
	jsr	line_draw_sgl

	ldab	#1		; Draw bottom row
	jsr	line_draw_sgl

	rts

; Show lines interactively and wait for an action. Must already be set up using
; line_begin before calling.
; INPUT:	None
; OUTPUT:	None
;		D = Trashed
;		X = Trashed
line_show:
	jsr	line_draw_all	; Draw all lines

line_show_getk:
	os	kb$getk		; Wait for keypress
	cmpb	#3		; Scroll up if UP key is pressed
	beq	line_show_up
	cmpb	#4		; Scroll down if DOWN key is pressed
	beq	line_show_down
	cmpb	#5		; Scroll left if LEFT key is pressed
	beq	line_show_left
	cmpb	#6		; Scroll right if RIGHT key is pressed
	beq	line_show_right

	clc			; Otherwise, return to caller
	rts

line_show_up:
	ldx	line_cury	; Get current cursor Y position
	cpx	#0		; Don't do anything if already at top
	beq	line_show_getk	; (line_cury = 0)
	dex			; Otherwise, decrement it
	stx	line_cury	; Store it

	clr	line_curx	; Reset cursor X position back to start
	clr	line_scrlx	; Reset scroll X position back to start

	cpx	line_scrly	; Decrement scroll Y position if required
	bcc	line_show	; Skip if past scroll (line_cury >= line_scrly)
	ldx	line_scrly	; Otherwise, scroll up one line
	dex
	stx	line_scrly

	bra	line_show	; Redraw and get next key

line_show_down:
	ldx	line_cury	; Get current cursor Y position
	inx			; Increment it
	stx	line_cury	; Store it

	clr	line_curx	; Reset cursor X position back to start
	clr	line_scrlx	; Reset scroll X position back to start

	ldd	line_scrly	; Increment scroll Y position if required
	addd	#2
	std	line_show_scrly
	cpx	line_show_scrly
	bcs	line_show	; Skip if not past (cury < scrly + rows)
	ldx	line_scrly
	inx
	stx	line_scrly

	bra	line_show	; Redraw and get next key

line_show_scrly:
	.WORD	$0000

line_show_left:
	ldaa	line_curx	; Get current cursor X position
	cmpa	#0		; Don't do anything if already at start
	beq	line_show_getk	; (line_curx = 0)
	deca			; Otherwise, decrement it
	staa	line_curx	; Store it

	cmpa	line_scrlx	; Decrement scroll X position if required
	bcc	line_show_cache	; Skip if past scroll (line_curx >= line_scrlx)
	dec	line_scrlx	; Otherwise, scroll left one char

	bra	line_show_cache	; Redraw and get next key

line_show_right:
	ldaa	line_curx	; Get current cursor X position
	inca			; Increment it
	staa	line_curx	; Store it

	ldab	line_scrlx	; Increment scroll X position if required
	addb	#16
	cba
	bcs	line_show_cache	; Skip if not past (curx < cury + cols)
	inc	line_scrlx	; Otherwise, scroll right one char

	bra	line_show_cache	; Redraw and get next key

line_show_cache:
	jmp	line_show	; TODO: Call line_draw_sgl instead