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

; Draw a line given by the address of its buffer, at the current scroll X
; position.
; INPUT:	B = Row of display to draw line on
;		X = Address of line buffer
; OUTPUT:	None
;		D = Trashed
;		X = Trashed
line_draw_scrlx:
	ldaa	#16		; Set cursor pos to start of given row
	mul			; B = row * 16
	tba			; Move B into A
	os	dp$stat

	; TODO: Clear the line

	ldaa	0,x		; Get length of line
	ldab	line_curx	; Get current scroll X position
	cba			; Don't do anything if scrolled past end of line
	bls	line_dscrlx_end	; (length <= line_scrlx)

	; TODO: Cap line length if > 16
	suba	line_curx	; Decrease line length by scroll X position
	abx			; Add current scroll X position to line address
	inx			; Skip over count byte
	tab			; Move A into B
	os	dp$prnt		; Print the line

line_dscrlx_end:
	rts

; Draw a single line at the current scroll position.
; INPUT:	B = Row of display to draw line on
; OUTPUT:	C = Set if line does not exist (reached end)
;		D = Trashed
;		X = Trashed
line_draw_sgl:
	stab	line_dsgl_row	; Save row for later
	clra			; Get current scroll Y + display row offset
	addd	line_scrly
	std	line_dsgl_scrly	; Save scroll Y offset for later

	ldx	line_read_cbk
	jsr	0,x		; Get contents of line and hotspots
	bcs	line_dsgl_end	; Early exit if line does not exist
	stx	line_buf_ptr	; Store pointers to returned buffers
	std	line_hsp_ptr

	ldx	line_cury	; Check if cursor is on this line
	cpx	line_dsgl_scrly	; If not, then don't cache it
	bne	line_dsgl_nocache

	ldx	line_buf_ptr	; Load line buffer's address into X
	clra			; Read first (count) byte of line buffer
	ldab	0,x
	incb			; Increment to include count byte in length
	std	utw_s0		; Store buffer length in utw_s0
	ldd	#line_buf_cache	; Destination is line buffer cache
	os	ut$cpyb		; Copy line into cache
	; TODO: Maybe add length check to prevent buffer overflow

	ldx	line_hsp_ptr	; Load line buffer's address into X
	clra			; Read first (count) byte of line buffer
	ldab	0,x
	incb			; Increment to include count byte in length
	std	utw_s0		; Store buffer length in utw_s0
	ldd	#line_hsp_cache	; Destination is line hotspot cache
	os	ut$cpyb		; Copy line into cache
	; TODO: Maybe add length check to prevent buffer overflow

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
	jsr	line_draw_all

	os	kb$getk

	rts