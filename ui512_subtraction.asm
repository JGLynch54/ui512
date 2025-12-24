;
;			ui512_subtraction
;
;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;
;			File:			ui512_subtraction.asm
;			Author:			John G. Lynch
;			Legal:			Copyright @2024, per MIT License below
;			Date:			October 29, 2025

				INCLUDE			ui512_legalnotes.inc
				INCLUDE			ui512_compile_time_options.inc
				INCLUDE			ui512_macros.inc
				INCLUDE			ui512_externs.inc
.NOLISTIF
				OPTION			CASEMAP:NONE

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
; Sub_Prop_Borrow MACRO
;		In lane SMD subtracts(s), detect borrows, propagate borrows to next most significant lane(s), repeat until no more borrows
;		Specify destination ZMM reg, the subtrahend and minuend, a K reg to track borrows, and a reg with "1" in it
;		Uses and destroy contents of: k1, k2, and ZMM0
;		Returns in specified "borrowK" bit masks where borrows occured, bit0 denotes borrow overflow
;
Sub_Prop_Borrow	MACRO			dest, subtrahend, minuend, borrowK, broad1
				VPSUBQ		    dest, subtrahend, minuend			; Initial subtraction, lane by lane subtract
				VPCMPUQ		    k1, dest, subtrahend, CPGT			; Detect initial borrows. Destination lane > subtrahend lane? must have borrowed in that lane
@@:             KANDNB			k2, borrowK, k1						; eliminate apparent borrows if they had already been flagged (and processed) (AND NOT previous K7)
                KORB			borrowK, borrowK, k2				; new borrows in K2, OR into borrows done (borrowK)
                KSHIFTRB		k2, k2, 1							; new borrows shift right to align which lane to add borrow from
                KTESTB			k2, k2								; no new borrows? exit
				JZ				@F									; If, after alignment shift, there are no borrows, save and exit
				VPBROADCASTQ	ZMM0 {k2}{z}, broad1				; Apply borrow-ins only where needed
				VPSUBQ			dest {k2}, dest, ZMM0				; subtract the borrows, possibly causing cascade of borrowing to next higher lane
				VPCMPUQ			k1 { k2 }, dest, ZMM0, CPGT			; detect new mask of borrows
				JMP				@B
@@:
				ENDM

ui512_subtract	SEGMENT			PARA 'CODE'

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;			sub_u		-	subtract supplied 512bit (8 QWORDS) RH OP from LH OP giving difference in destination
;			Prototype:		extern "C" s32 sub_u( u64* difference, u64* left operand, u64* right operand )
;			difference	-	Address of 64 byte aligned array of 8 64-bit words (QWORDS) 512 bits (in RCX)
;			lh_op		-	Address of the LHOP 8 64-bit QWORDS (512 bits) in RDX
;			rh_op		-	Address of the RHOP 8 64-bit QWORDS (512 bits) in R8
;			returns		-	zero for no borrow, 1 for borrow (underflow)
;			Note: unrolled code instead of loop: faster, and no regs to save / setup / restore
;
				Leaf_Entry		sub_u
				CheckAlign		RCX									; (OUT) 8 QWORD difference
				CheckAlign		RDX									; (IN) 8 QWORD left hand operand, minuend
				CheckAlign		R8									; (IN) 8 QWORD right hand operand, subtrahend

	IF __UseZ
; Load operands
				VMOVDQA64		ZMM30, ZM_PTR [RDX]					; Load lh_op
				VMOVDQA64		ZMM31, ZM_PTR [R8]					; Load rh_op

; Initialize loop variables: R9 for the targeted in-lane subtract of borrows; k7 for return code overall borrow flag
				XOR				R9, R9
				INC				R9
				KXORB			k7, k7, k7							; Clear k7 (what borrows have been done mask)

				Sub_Prop_Borrow ZMM29, ZMM30, ZMM31, k7, R9
                KMOVB			RAX, k7								; Move final borrows done to RAX
                AND				RAX, retcode_one					; bit 0 (MSB borrow-out)
				VMOVDQA64		ZM_PTR [RCX], ZMM29
				RET

	ELSE
				MOV				RAX, Q_PTR [ RDX ] [ 7 * 8 ]		; get last word of minuend (least significant word of the number we are subtracting from) (left-hand operand)
				SUB				RAX, Q_PTR [ R8 ] [ 7 * 8 ]			; subtract last word of subtrahend (the number to be subtracted) (right-hand operand)
				MOV				Q_PTR [ RCX ] [ 7 * 8 ], RAX		; store result in last word of difference, note: the flag 'carry' has been set to whether there has been a 'borrow'

; FOR EACH index 6 thru 0: Get minuend QWORD, subtract (with borrow), store at difference
				FOR				idx, < 6, 5, 4, 3, 2, 1, 0 >
				MOV				RAX, [ RDX ] [ idx * 8 ]					
				SBB				RAX, [ R8 ] [ idx * 8 ]
				MOV				[ RCX ] [ idx * 8 ], RAX
				ENDM

				LEA				RAX, [ retcode_zero ]				; return, set return code to zero if no remaining borrow, to one if there is a borrow
				CMOVC			EAX, ret_one
				RET

	ENDIF
sub_u			ENDP

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;			sub_u_wb	-	subtract supplied 512bit (8 QWORDS) RH OP from LH OP, including borrow giving difference in destination
;			Prototype:		extern "C" s32 sub_u( u64* difference, u64* left operand, u64* right operand )
;			difference	-	Address of 64 byte aligned array of 8 64-bit words (QWORDS) 512 bits (in RCX)
;			lh_op		-	Address of the LHOP 8 64-bit QWORDS (512 bits) in RDX
;			rh_op		-	Address of the RHOP 8 64-bit QWORDS (512 bits) in R8
;			s16			-	borrow indicator zero for none, anything else for one
;			returns		-	zero for no borrow, 1 for borrow (underflow)
;			Note: unrolled code instead of loop: faster, and no regs to save / setup / restore
;
				Leaf_Entry		sub_u_wb
				CheckAlign		RCX									; (OUT) 8 QWORD difference
				CheckAlign		RDX									; (IN) 8 QWORD left hand operand, minuend
				CheckAlign		R8									; (IN) 8 QWORD right hand operand, subtrahend

	IF __UseZ
; Load operands
				VMOVDQA64		ZMM30, ZM_PTR [RDX]					; Load lh_op
				VMOVDQA64		ZMM31, ZM_PTR [R8]					; Load rh_op
				XOR				R8, R8
				TEST			R9W, R9W							; borrow-in non-zero?
				JZ				@@noborrowin
				XOR				R9, R9
                INC				R9
				KMOVB			k1,  B_PTR mskB7				; mask for least significant word
				VPBROADCASTQ	ZMM28 {k1}{z}, R9					; ZMM28 now 512 bit version of carry-in
                KXORB			k7, k7, k7							; Clear k7 (what carries have been done mask)
				Sub_Prop_Borrow	ZMM29, ZMM30, ZMM28, k7, R9			; add 30 (lh_op), 28 (carry-in); detect and propagate carries
				KMOVB			R8, k7								; adding carry cause carry?
				AND				R8, retcode_one						; save this carry, if present, for return
				VMOVDQA64		ZMM30, ZMM29						; move carried in sum to addend 1

; Initialize loop variables: R9 for the targeted in-lane subtract of borrows; k7 for return code overall borrow flag
@@noborrowin: 	XOR				R9, R9
				INC				R9
				KXORB			k7, k7, k7							; Clear k7 (what borrows have been done mask)

				Sub_Prop_Borrow ZMM29, ZMM30, ZMM31, k7, R9
                KMOVB			RAX, k7								; Move final borrows done to RAX
                AND				RAX, retcode_one					; bit 0 (MSB borrow-out)
				OR				RAX, R8
				VMOVDQA64		ZM_PTR [RCX], ZMM29
				RET

	ELSE
				; Check if carry-in set, set flag carry for addition if so, clear if not
				CLC
				TEST			R9W, R9W
				JZ				@@noborrowin
				STC

; FOR EACH index 6 thru 0: Get minuend QWORD, subtract (with borrow), store at difference
@@noborrowin:	FOR				idx, < 7, 6, 5, 4, 3, 2, 1, 0 >
				MOV				RAX, [ RDX ] [ idx * 8 ]					
				SBB				RAX, [ R8 ] [ idx * 8 ]
				MOV				[ RCX ] [ idx * 8 ], RAX
				ENDM

				LEA				RAX, [ retcode_zero ]				; return, set return code to zero if no remaining borrow, to one if there is a borrow
				CMOVC			EAX, ret_one
				RET

	ENDIF
sub_u_wb		ENDP

;
;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;			sub_uT64	-	subtract supplied 64 bit right hand (64 bit value) op from left hand (512 bit) giving difference
;			Prototype:		extern "C" s32 sub_uT64( u64* difference, u64* left operand, u64 right operand )
;			difference	-	Address of 64 byte aligned array of 8 64-bit words (QWORDS) 512 bits (in RCX)
;			lh_op		-	Address of  the 64 byte aligned array of 8 64-bit QWORDS (512 bits) in RDX
;			rh_op		-	64-bitvalue in R8
;			returns		-	zero for no borrow, 1 for borrow (underflow)
;			Note: unrolled code instead of loop: faster, and no regs to save / setup / restore
;
				Leaf_Entry		sub_uT64
				CheckAlign		RCX									; (OUT) 8 QWORD difference
				CheckAlign		RDX									; (IN) 8 QWORD left hand operand, minuend

	IF __UseZ
; Load operands
				VMOVDQA64		ZMM30, ZM_PTR [RDX]			        ; Load lh_op
				KMOVB			k1, B_PTR mskB7						; mask for least significant word
				VPBROADCASTQ	ZMM31 {k1}{z}, R8					; ZMM31 now 512 bit version of passed rh_op

; Initialize loop variables: R9 for the targeted in-lane subtract of borrows; k7 for return code overall borrow flag
				XOR				R9, R9
				INC				R9
				KXORB			k7, k7, k7							; Clear k7 (what borrows have been done mask)
				Sub_Prop_Borrow ZMM29, ZMM30, ZMM31, k7, R9
                KMOVB			RAX, k7								; Move final borrows done to RAX
                AND				RAX, retcode_one					; bit 0 (MSB borrow-out)
				VMOVDQA64		ZM_PTR [RCX], ZMM29
				RET

	ELSE
				MOV				RAX, [ RDX ] [ 7 * 8 ]				; 
				SUB				RAX, R8
				MOV				[ RCX ] [ 7 * 8 ], RAX

; FOR EACH index 6 thru 0: Get minuend QWORD, subtract borrow (if any), store at difference
				FOR				idx, < 6, 5, 4, 3, 2, 1, 0 >
				MOV				RAX, [ RDX ] [ idx * 8 ]					
				SBB				RAX, 0
				MOV				[ RCX ] [ idx * 8 ], RAX
				ENDM

				LEA				RAX, [ retcode_zero ]
				CMOVC			EAX, ret_one
				RET

	ENDIF
sub_uT64		ENDP
ui512_subtract	ENDS											; end of section
				END													; end of module