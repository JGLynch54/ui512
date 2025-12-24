;
;			ui512_compare
;
;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;
;			File:			ui512_compare.asm
;			Author:			John G. Lynch
;			Legal:			Copyright @2024, per MIT License below
;			Date:			October 29, 2025

				INCLUDE			ui512_legalnotes.inc
				INCLUDE			ui512_compile_time_options.inc
				INCLUDE			ui512_macros.inc
				INCLUDE			ui512_externs.inc
.NOLISTIF
				OPTION			CASEMAP:NONE
ui512_compare	SEGMENT			PARA 'CODE'

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;			compare_u	-	unsigned compare supplied 512bit (8 QWORDS) LH operand to supplied RH operand
;			Prototype:		extern "C" s32 compare_u( u64* lh_op, u64* rh_op )
;			lh_op		-	Address of LH 64 byte aligned array of 8 64-bit words (QWORDS) 512 bits (in RCX)
;			rh_op		-	Address of RH 64 byte aligned array of 8 64-bit QWORDS (512 bits) in RDX
;			returns		-	(0) for equal, -1 for lh_op is less than rh_op, 1 for lh_op is greater than rh_op 
;			Note: unrolled code instead of loop: faster, and no regs to save / setup / restore
;

				Leaf_Entry		compare_u							; Declare public proc, no prolog, no frame, exceptions handled by caller
				CheckAlign		RCX									; (IN) left hand (LH) operand to compare
				CheckAlign		RDX									; (IN) right hand (RH) operand in the compare 

	IF __UseZ
				VMOVDQA64		ZMM30, ZM_PTR [ RCX ]				; Load parameters
				VMOVDQA64		ZMM31, ZM_PTR [ RDX ]
				VPCMPUQ			k1, ZMM30, ZMM31, CPLT				; in-lane compare 8 words for 'less than'
				VPCMPUQ			k2, ZMM30, ZMM31, CPGT				; do the same for 'greater than' (interleave these two compares to hide latencies)
				KMOVW			R8D, k1
				KMOVW			EAX, k2
				OR				R8D, MASK kMask.b8					; OR in a high bit to make an equal compare not zero	
				OR				EAX, MASK kMask.b8
				SHL				R8D, 1								; shift to get bits 2 through 8
				SHL				EAX, 1
		IF __UseBMI2
				TZCNT			R8D, R8D							; get bit number of right-most (most significant) 1 thru 8
				TZCNT			EAX, EAX

		ELSE		
				BSF				R8D, R8D							; get bit number of right-most (most significant) 1 thru 8					
				BSF				EAX, EAX

		ENDIF
				CMP				R8D, EAX							; compare: which is most significant? LT or GT? (or zero - equal)
				LEA				EAX, [ retcode_zero ]
				CMOVA			EAX, ret_one
				CMOVB			EAX, ret_neg_one
				RET

	ELSEIF	__UseY
				VMOVDQA64		YMM0, YM_PTR [ RCX ] [ 4 * 8 ]		; load most significant 4 qwords of parameters
				VMOVDQA64		YMM2, YM_PTR [ RDX ] [ 4 * 8 ]
				VPCMPUQ			k1, YMM0, YMM2, CPLT				; in-lane compare, this one for 'LT'
				VPCMPUQ			k2, YMM0, YMM2, CPGT				; repeat for "GT"
				KMOVB			R8D, k1								; LT compare result to R8D
				KMOVB			EAX, k2								; GT compare result to EAX
				OR				R8D, MASK kMask.b8					; OR in a high bit to make an equal compare not zero
				OR				EAX, MASK kMask.b8
				SHL				R8D, 1								; shift so zero bit is one bit 
				SHL				EAX, 1

		IF __UseBMI2
				TZCNT			R8D, R8D							; get bit number of right-most (most significant) 1 thru 8
				TZCNT			EAX, EAX

		ELSE
				BSF				R8D, R8D							; find most significant "LT" word
				BSF				EAX, EAX							; same for 'GT' word

		ENDIF
				CMP				R8D, EAX							; most significant (either LT or GT), else fall through to look at least significant 4 qwords
				JNE				@F
				VMOVDQA64		YMM1, YM_PTR [ RCX ] [ 0 * 8 ]		; if the most significant 4 qwords were equal, have to look at least significant 4 qwords
				VMOVDQA64		YMM3, YM_PTR [ RDX ] [ 0 * 8 ]
				VPCMPUQ			k1, YMM1, YMM3, CPLT
				VPCMPUQ			k2, YMM1, YMM3, CPGT
				KMOVB			R8D, k1
				KMOVB			EAX, k2
				OR				R8D, MASK kMask.b8					; OR in a high bit to make an equal compare not zero	
				OR				EAX, MASK kMask.b8
				SHL				R8D, 1								; shift so zero bit is one bit
				SHL				EAX, 1

		IF __UseBMI2
				TZCNT			R8D, R8D							; get bit number of right-most (most significant) 1 thru 8
				TZCNT			EAX, EAX

		ELSE
				BSF				R8D, R8D							; find most significant "LT" word
				BSF				EAX, EAX							; same for 'GT' word

		ENDIF
				CMP				R8D, EAX	
@@:
				LEA				EAX, [ retcode_zero ]
				CMOVA			EAX, ret_one
				CMOVB			EAX, ret_neg_one
				RET

	ELSE
; FOR EACH index of 0 thru 7 : fetch qword of lh_op, compare to qword of rh_op; jump to exit if not equal
				FOR				idx, < 0, 1, 2, 3, 4, 5, 6, 7 >
				MOV				RAX, Q_PTR [ RCX ] [ idx * 8 ]
				CMP				RAX, Q_PTR [ RDX ] [ idx * 8 ]
				JNZ				@F
				ENDM
@@:
				LEA				RAX, [ retcode_zero ]
				CMOVA			EAX, ret_one						; 'above' is greater than for an unsigned integer
				CMOVB			EAX, ret_neg_one					; 'below' is less than for an unsigned integer
				RET

	ENDIF
compare_u		ENDP

;
;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;			compare_uT64-	unsigned compare supplied 512bit (8 QWORDS) LH operand to supplied 64bit RH operand
;			Prototype:		extern "C" s32 compare_uT64( u64* lh_op, u64 rh_op )
;			lh_op		-	Address of LH 64 byte aligned array of 8 64-bit words (QWORDS) 512 bits (in RCX)
;			rh_op		-	The RH 64-bit value in RDX
;			returns		-	(0) for equal, -1 for less than, 1 for greater than
;			Note: unrolled code instead of loop: faster, and no regs to save / setup / restore
;

				Leaf_Entry		compare_uT64
				CheckAlign		RCX									; (IN) left hand (LH) operand to compare 

	IF		__UseZ
				VMOVDQA64		ZMM30, ZM_PTR [ RCX ]				; Load lh-op parameter
				KMOVB			k1, B_PTR [ mskB7 ]
				VPBROADCASTQ 	ZMM31 {k1}{z}, RDX					; load rh_op parameter (both now in Z regs)
				VPCMPUQ			k1, ZMM30, ZMM31, CPLT				; in-lane compare for LT
				VPCMPUQ			k2, ZMM30, ZMM31, CPGT				; do the same for 'greater than'
				KMOVW			R8D, k1
				KMOVW			EAX, k2
				OR				R8D, MASK kMask.b8					; OR in a high bit to make an equal compare not zero	
				OR				EAX, MASK kMask.b8
				SHL				R8D, 1								; shift to get bits 2 through 8
				SHL				EAX, 1

		IF __UseBMI2
				TZCNT			R8D, R8D							; get bit number of right-most (most significant) 1 thru 8
				TZCNT			EAX, EAX

		ELSE														; TZCNT/BSF: source guaranteed non-zero because of sentinel bit
				BSF				R8D, R8D							; find most significant "LT" word
				BSF				EAX, EAX							; same for 'GT' word

		ENDIF
				CMP				R8D, EAX							; compare: which is most significant? LT or GT? (or zero - equal)
				LEA				EAX, [ retcode_zero ]
				CMOVA			EAX, ret_one
				CMOVB			EAX, ret_neg_one
				RET

	ELSE
				XOR				RAX, RAX
; FOR EACH index 0 thru 6: Get QWORD, compare for zero 
				FOR				idx, < 0, 1, 2, 3, 4, 5, 6 >
				CMP				Q_PTR [ RCX ] [ idx * 8 ], RAX
				JNZ				@F
				ENDM

				MOV				RAX, Q_PTR [ RCX ] [ 7 * 8 ]
				CMP				RAX, RDX 
				JNZ				@F
				XOR				EAX, EAX
@@:				CMOVA			EAX, ret_one						; 'above' is greater than for an unsigned integer
				CMOVB			EAX, ret_neg_one					; 'below' is less than for an unsigned integer
				RET

	ENDIF
compare_uT64	ENDP												; end of proc
ui512_compare	ENDS												; end of section
				END													; end of module