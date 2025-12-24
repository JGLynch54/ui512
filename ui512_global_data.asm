;
;			ui512_global_data
;
;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;
;			File:			ui512_global_data.asm
;			Author:			John G. Lynch
;			Legal:			Copyright @2024, per MIT License below
;			Date:			November 7, 2025

				INCLUDE			ui512_legalnotes.inc
				INCLUDE			ui512_compile_time_options.inc
				INCLUDE			ui512_macros.inc

				OPTION			CASEMAP:NONE

ui512_global	SEGMENT READONLY PARA 'CONST'
				ALIGN			16
; ---- data segment for constants ----

				PUBLIC			qOnes
qOnes			QWORD			8 DUP ( 0FFFFFFFFFFFFFFFFh )

				PUBLIC			qZero
qZero			QWORD			0

; common return codes
				PUBLIC			ret_zero
				PUBLIC			ret_one
				PUBLIC			ret_neg_one
				PUBLIC			ret_GPFault
ret_zero		DD				retcode_zero
ret_one			DD				retcode_one
ret_neg_one		DD				retcode_neg_one
ret_GPFault		DD				0C0000005h						; Windows code for General Protection Fault


; masks commonly used
				PUBLIC			mskB0
				PUBLIC			mskB1
				PUBLIC			mskB2
				PUBLIC			mskB3
				PUBLIC			mskB4
				PUBLIC			mskB5
				PUBLIC			mskB6
				PUBLIC			mskB7
				PUBLIC			mskAll8
mskB0			DB				1
mskB1			DB				2
mskB2			DB				4
mskB3			DB				8
mskB4			DB				16
mskB5			DB				32
mskB6			DB				64
mskB7			DB				128
mskAll8			DB				255

ui512_global	ENDS

				END