; APP DEFINITIONS
; Includes top-level menu entries and launch vectors for each ORGNET app.

; Adds all ORGNET apps to top-level menu.
; INPUT:	None
; OUTPUT:	None
;		D = Trashed
;		X = Trashed
apps_add_tl:
	ldx	#apps_netcfg_tl	; Add NETCFG item to top-level menu
	jsr	tl_add

	clc
	rts

; Removes all ORGNET apps from top-level menu.
; INPUT:	None
; OUTPUT:	None
;		X = Trashed
apps_rm_tl:
	ldx	#apps_netcfg_tl	; Remove NETCFG item from main menu
	jsr	tl_remove

	clc
	rts

apps_netcfg_tl:
	.ASCIC	"NETCFG"
	.WORD	apps_netcfg_vec

apps_netcfg_vec:
	ldx	#%netcfg-%xx
	ldd	#%netcfgend-%netcfg
	jsr	appldr_load

	jsr	appldr_unload

	clc
	rts