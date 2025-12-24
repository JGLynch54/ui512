;
;			ui512_addition
;
;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;
;			File:			ui512_addition.asm
;			Author:			John G. Lynch
;			Legal:			Copyright @2025, per MIT License below
;			Date:			October 29, 2025  (file creation)

				INCLUDE			ui512_legalnotes.inc
				INCLUDE			ui512_compile_time_options.inc
				INCLUDE			ui512_macros.inc
				INCLUDE			ui512_externs.inc
.NOLISTIF
				OPTION			CASEMAP:NONE
ui512_addition	SEGMENT			PARA 'CODE'

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
; Add_Prop_Carry MACRO
;		In lane SMD add(s), detect carries, propagate carries to next most significant lane, repeat until no more carries
;		Specify destination ZMM reg, the two addends, a K reg to track carries, and a reg with "1" in it
;		Uses and destroy contents of: k1, k2, and ZMM0
;		Returns in specified "carryK" a bit masks where carries occured, bit0 denotes carry overflow
;
Add_Prop_Carry	MACRO			dest, addend1, addend2, carryK, broad1
				VPADDQ			dest, addend1, addend2				; Initial sum, lane by lane add
				VPCMPUQ			k1, dest, addend1, CPLT				; Initial carries: sum[i] < lh_op[i] (unsigned carry detect, lane by lane)
@@:	; Carry propagation (loop until done) Note: tried unrolled here, but code size, hence instruction pipeline fetch, slower than tight loop
                KANDNB			k2, carryK, k1						; eliminate apparent carries if they had already been flagged (and processed) (by AND NOT previous carryK)
                KORB			carryK, carryK, k2					; new carries in K2, OR into carries already done (carryK)
                KSHIFTRB		k2, k2, 1							; new carries shift right to align which lane to add carry to
                KTESTB			k2, k2								; no new carries? exit
                JZ				@F
                VPBROADCASTQ	ZMM0 {k2}{z}, broad1				; in scratch reg, zero lanes, load shifted carry lanes with '1'
                VPADDQ			dest {k2}, dest, ZMM0				; add in the carries
                VPCMPUQ			k1 {k2}, dest, ZMM0, CPLT			; compare result, lane by lane, to see if less than orginal (indicating an overflow / carry)
				JMP				@B									; go back and check for additional (newly generated) carries
@@:
				ENDM

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;			add_u		-	unsigned add supplied 512bit (8 QWORDS) sources to supplied destination
;			Prototype:		extern "C" s32 add_u( u64* sum, u64* addend1, u64* addend2 )
;			sum			-	Address of 64 byte aligned array of 8 64-bit words (QWORDS) 512 bits (in RCX)
;			addend1		-	Address of  the 64 byte aligned array of 8 64-bit QWORDS (512 bits) in RDX
;			addend2		-	Address of  the 64 byte aligned array of 8 64-bit QWORDS (512 bits) in R8
;			returns		-	zero for no carry, 1 for carry (overflow)
;			Note: unrolled code instead of loop: faster, and no regs to save / setup / restore
;
								
				Leaf_Entry      add_u								; Declare code section, public proc, no prolog, no frame, exceptions handled by caller
				CheckAlign		RCX									; (OUT) 8 QWORD sum
				CheckAlign		RDX									; (IN) 8 QWORD LH addend
				CheckAlign		R8									; (IN) 8 QWORD RH addend

     IF __UseZ
; Load operands
                VMOVDQA64		ZMM30, ZM_PTR [ RDX ]				; Load addend1 (lh_op)
                VMOVDQA64		ZMM31, ZM_PTR [ R8 ]				; Load addend2 (rh_op)

; Initialize: R9=1 for carry-add; K7=0 for carry done mask
                XOR				R9, R9
                INC				R9
                KXORB			k7, k7, k7							; Clear k7 (what carries have been done mask)
				Add_Prop_Carry ZMM29, ZMM30, ZMM31, k7, R9			; add 30, 31; detect and propagate carries

; Complete, extract carry out (overflow) for return code, store result sum at callers sum
                KMOVB			RAX, k7								; Move final carries done to RAX
                AND				RAX, retcode_one					; bit 0 (MSB carry-out)
                VMOVDQA64		ZM_PTR [ RCX ], ZMM29				; Store sum
                RET

 	ELSE
; if not using "Z" SIMD, then use Q regs, roughly 27 instructions counting return code, return				
				MOV				RAX, Q_PTR [ RDX ] [ 7 * 8 ]
				ADD				RAX, Q_PTR [ R8 ] [ 7 * 8 ]
				MOV				Q_PTR [ RCX ] [ 7 * 8 ], RAX

; FOR EACH index of 6 thru 0 : fetch qword of addend1 (RDX), add (with carry) to qword of addend2; store at callers sum			
				FOR				idx, < 6, 5, 4, 3, 2, 1, 0 >
				MOV				RAX, Q_PTR [ RDX ] [ idx * 8 ]
				ADCX			RAX, Q_PTR [ R8 ] [ idx * 8 ]		; Note: some CPUs dont support ADCX (64bit unsigned add) An Add with checking sign change could be used
				MOV				Q_PTR [ RCX ] [ idx * 8 ] , RAX
				ENDM

; Complete. Carry to return code
				LEA				RAX, [ retcode_zero ]				; return carry flag as overflow
				CMOVC			EAX, ret_one
				RET	

	ENDIF
add_u			ENDP

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;			add_u_wc	-	unsigned add supplied 512bit (8 QWORDS) sources to supplied destination, with carry
;			Prototype:		extern "C" s32 add_u( u64* sum, u64* addend1, u64* addend2 )
;			sum			-	Address of 64 byte aligned array of 8 64-bit words (QWORDS) 512 bits (in RCX)
;			addend1		-	Address of  the 64 byte aligned array of 8 64-bit QWORDS (512 bits) in RDX
;			addend2		-	Address of  the 64 byte aligned array of 8 64-bit QWORDS (512 bits) in R8
;			carry		-	value of carry in (usually 0 or 1), but any non-zero results in adding 1
;			returns		-	zero for no carry, 1 for carry (overflow)
;			Note: unrolled code instead of loop: faster, and no regs to save / setup / 
;			Note: add with carry allows "laddering" adds to build much larger than 512bit variables
;
				Leaf_Entry		add_u_wc
				CheckAlign		RCX									; (OUT) 8 QWORD sum
				CheckAlign		RDX									; (IN) 8 QWORD LH addend
				CheckAlign		R8									; (IN) 8 QWORD RH addend

     IF __UseZ
; Load operands
                VMOVDQA64		ZMM30, ZM_PTR [ RDX ]				; Load addend1 (lh_op)
				VMOVDQA64		ZMM31, ZM_PTR [ R8 ]				; Load addend2 (rh_op)
				XOR				R8, R8
				TEST			R9W, R9W							; carry-in non-zero?
				JZ				@@nocarryin
				XOR				R9, R9
                INC				R9
				KMOVB			k1, B_PTR [ mskB7 ]					; mask for least significant word
				VPBROADCASTQ	ZMM28 {k1}{z}, R9					; ZMM28 now 512 bit version of carry-in
                KXORB			k7, k7, k7							; Clear k7 (what carries have been done mask)
				Add_Prop_Carry	ZMM29, ZMM30, ZMM28, k7, R9			; add 30 (lh_op), 28 (carry-in); detect and propagate carries
				KMOVB			R8, k7								; adding carry cause carry?
				AND				R8, retcode_one						; save this carry, if present, for return
				VMOVDQA64		ZMM30, ZMM29						; move carried in sum to addend 1
@@nocarryin:    XOR				R9, R9
                INC				R9
                KXORB			k7, k7, k7							; Clear k7 (what carries have been done mask)
				Add_Prop_Carry	ZMM29, ZMM30, ZMM31, k7, R9			; add 30 (lh_op + carry-in), 31 (rh_op); detect and propagate carries

; Complete, extract carry out (overflow) for return code, store result sum at callers sum
                KMOVB			RAX, k7								; Move final carries done to RAX
                AND				RAX, retcode_one					; bit 0 (MSB carry-out)
				OR				RAX, R8								; if either adding passed in carry, or the addition causes carry, return carry code
                VMOVDQA64		ZM_PTR [ RCX ], ZMM29				; Store sum
                RET

 	ELSE
; Check if carry-in set, set flag carry for addition if so, clear if not
				CLC
				TEST			R9W, R9W
				JZ				@@nocarryin
				STC

; FOR EACH index of 7 thru 0 : fetch qword of addend1 (RDX), add (with carry) to qword of addend2; store at callers sum			
@@nocarryin:	FOR				idx, < 7, 6, 5, 4, 3, 2, 1, 0 >
				MOV				RAX, Q_PTR [ RDX ] [ idx * 8 ]
				ADCX			RAX, Q_PTR [ R8 ] [ idx * 8 ]		; Note: some CPUs dont support ADCX (64bit unsigned add) An Add with checking sign change could be used
				MOV				Q_PTR [ RCX ] [ idx * 8 ] , RAX
				ENDM

; Complete. Carry to return code
				LEA				RAX, [ retcode_zero ]				; return carry flag as overflow
				CMOVC			EAX, ret_one
				RET	

	ENDIF
add_u_wc		ENDP

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;			add_uT64	-	add supplied 64bit QWORD (value) to 512bit (8 QWORDS), place in supplied destination
;			Prototype:		extern "C" s32 add_uT64( u64* sum, u64* addend1, u64 addend2 )
;			sum			-	Address of 64 byte aligned array of 8 64-bit words (QWORDS) 512 bits (in RCX)
;			addend1		-	Address of  the 64 byte aligned array of 8 64-bit QWORDS (512 bits) in RDX
;			addend2		-	The 64-bit value in R8
;			returns		-	zero for no carry, 1 for carry (overflow)
;			Note: unrolled code instead of loop: faster, and no regs to save / setup / restore
;
				Leaf_Entry		add_uT64
				CheckAlign		RCX									; (OUT) 8 QWORD sum
				CheckAlign		RDX									; (IN) 8 QWORD LH addend

	IF __UseZ
; Load operands				
				VMOVDQA64		ZMM30, ZM_PTR [RDX]					; ZMM30 = addend1 (8 QWORDs)
				KMOVB			k1, B_PTR [ mskB7 ]					; mask for least significant word
				VPBROADCASTQ	ZMM31 {k1}{z}, R8					; ZMM31 now 512 bit version of passed addend2

; Initialize: R9=1 for carry-add; K7=0 for carries done mask
                XOR				R9, R9
                INC				R9
                KXORB			k7, k7, k7							; Clear k7 (what carries have been done mask)
				Add_Prop_Carry ZMM29, ZMM30, ZMM31, k7, R9			; add 30, 31; detect and propagate carries

; Complete, extract carry out (overflow) for return code, store result sum at callers sum
@@saveexit:     KMOVB			RAX, k7								; Move final carries done to RAX
                AND				RAX, retcode_one					; bit 0 (MSB carry-out)
                VMOVDQA64		ZM_PTR [ RCX ], ZMM29				; Store sum
                RET													; EAX carries return code (from carry computation above)

	ELSE
; First Addition, Get Least significant QWORD of addend, Add passed QWORD to it
				MOV				RAX, Q_PTR [ RDX ] [ 7 * 8 ]
				ADD				RAX, R8 
				MOV				Q_PTR [ RCX ] [ 7 * 8 ], RAX

; FOR EACH index 6 thru 0: Get addend QWORD, add zero, but with carry if any from previous add
				FOR				idx, < 6, 5, 4, 3, 2, 1, 0 >
				MOV				RAX, Q_PTR [ RDX ] [ idx * 8 ]
				ADCX			RAX, qZero							; Note: some CPUs dont support ADCX (64bit unsigned add) An Add with checking sign change could be used
				MOV				Q_PTR [ RCX ] [ idx * 8 ], RAX
				ENDM

; return zero unless carry still exists from addition
				LEA				EAX, [ retcode_zero ]
				CMOVC			EAX, ret_one
				RET	

	ENDIF				
add_uT64		ENDP												; end of proc
ui512_addition	ENDS												; end of section
				END													; end of module