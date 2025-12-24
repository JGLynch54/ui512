;
;			ui512_shift
;
;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;
;			File:			ui512_shift.asm
;			Author:			John G. Lynch
;			Legal:			Copyright @2025, per MIT License below
;			Date:			November 19, 2025  (file creation)

				INCLUDE			ui512_legalnotes.inc
				INCLUDE			ui512_compile_time_options.inc
				INCLUDE			ui512_macros.inc
				INCLUDE			ui512_externs.inc
.NOLISTIF
				OPTION			CASEMAP:NONE
ui512_shift		SEGMENT			PARA 'CODE'

	IF __UseZ		; Only need these tables if using zmm regs

; Note: storage of qwords in ZMM regs is in 'reverse' order, with the lowest index holding the most significant qword
;			so index 0 holds bits 511-448, index 7 holds bits 63-0
; But when the ZMM reg is stored to memory, the order is 'normal', with the lowest address holding the most significant qword

; table of indices for permuting words in a zmm reg to achieve right shifts by words
			;	PUBLIC			ShiftPermuteRt
				ALIGN			16
ShiftPermuteRt	QWORD			0, 1, 2, 3, 4, 5, 6, 7			; identity permute for shift left/right by words
				QWORD			0, 0, 1, 2, 3, 4, 5, 6			; shift right by one word
				QWORD			0, 0, 0, 1, 2, 3, 4, 5			; shift right by two words
				QWORD			0, 0, 0, 0, 1, 2, 3, 4			; shift right by three words
				QWORD			0, 0, 0, 0, 0, 1, 2, 3			; shift right by four words
				QWORD			0, 0, 0, 0, 0, 0, 1, 2			; shift right by five words
				QWORD			0, 0, 0, 0, 0, 0, 0, 1			; shift right by six words
				QWORD			0, 0, 0, 0, 0, 0, 0, 0			; shift right by seven words

; table of indices for permuting words in a zmm reg to achieve left shifts by words
			;	PUBLIC			ShiftPermuteLt
ShiftPermuteLt	QWORD			0, 1, 2, 3, 4, 5, 6, 7			; identity permute for shift left/right by words
				QWORD			1, 2, 3, 4, 5, 6, 7, 0			; shift left by one word
				QWORD			2, 3, 4, 5, 6, 7, 0, 0			; shift left by two words
				QWORD			3, 4, 5, 6, 7, 0, 0, 0			; shift left by three words
				QWORD			4, 5, 6, 7, 0, 0, 0, 0			; shift left by four words
				QWORD			5, 6, 7, 0, 0, 0, 0, 0			; shift left by five words
				QWORD			6, 7, 0, 0, 0, 0, 0, 0			; shift left by six words
				QWORD			7, 0, 0, 0, 0, 0, 0, 0			; shift left by seven words	

	
; When shifting, some words become zero,table of masks for zeroing words when shifting right
			;	PUBLIC			ShiftMaskRt
ShiftMaskRt		DB				0ffh, 0feh, 0fch, 0f8h, 0f0h, 0e0h, 0c0h, 080h

; When shifting, some words become zero,table of masks for zeroing words when shifting left
			;	PUBLIC			ShiftMaskLt
ShiftMaskLt		DB				0ffh, 07fh, 03fh, 01fh, 0fh, 07h, 03h, 01h	


	ENDIF		; __UseZ

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;	ShiftOrR	MACRO
;			Each word is shifted right, and the bits shifted out are ORd into the next (less significant) word.
;			RCX holds the number of bits to shift right, RBX holds the 64 bit complement for left shift.
;
ShiftOrR		MACRO			lReg, rReg
				SHLX			RDX, lReg, RBX					; shift 'bottom' bits to top
				SHRX			rReg, rReg, RCX					; shift target bits right (leaving zero filled bits at top)
				OR				rReg, RDX						; OR in new 'top' bits
				ENDM

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;	ShiftOrL	MACRO
;			Each word is shifted left, and the bits shifted out are ORd into the next (more significant) word.
;			RCX holds the number of bits to shift left, RBX holds the 64 bit complement for right shift.
;
ShiftOrL		MACRO			lReg, rReg
				SHRX			RDX, lReg, RBX					; shift 'top' bits to bottom
				SHLX			rReg, rReg, RCX					; shift target bits left (leaving zero filled bits at bottom)
				OR				rReg, RDX						; OR in new 'bottom' bits
				ENDM


;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;			shr_u		-	shift supplied source 512bit (8 QWORDS) right, put in destination
;			Prototype:		void shr_u( u64* destination, u64* source, u32 bits_to_shift)
;			destination	-	Address of 64 byte aligned array of 8 64-bit words (QWORDS) 512 bits (in RCX)
;			source		-	Address of 64 byte aligned array of 8 64-bit words (QWORDS) 512 bits (in RDX)
;			bits		-	Number of bits to shift. Will fill with zeros, truncate those shifted out (in R8W)
;			returns		-	nothing (0)
;			Note: unwound loop(s). More instructions, but fewer executed (no loop save, setup, compare loop), faster, fewer regs used

IF	__UseZ
				Leaf_Entry		shr_u				; Declare code section, public proc, no prolog, no frame, exceptions handled by caller
				CheckAlign		RCX								; (OUT) destination of shifted 8 QWORDs
				CheckAlign		RDX								; (IN)	source of 8 QWORDS

				CMP				R8W, 512						; handle edge case, shift 512 or more bits
				JL				@F
				Zero512			RCX								; zero destination, return
				RET

@@:				AND				R8, 511							; ensure no high bits above shift count
				JNZ				@@shift							; handle edge case, zero bits to shift
				CMP				RCX, RDX
				JE				@F								; destination is the same as the source: no copy needed
				Copy512			RCX, RDX						; no shift, just copy (destination, source already in regs), return
@@:				RET

@@shift:		VMOVDQA64		ZMM31, ZM_PTR [ RDX ]			; load the 8 qwords into zmm reg (note: word order)
				LEA				RAX, [ R8 ]
				AND				AX, 03fh						; limit shift count to 63 (shifting bits only here, not words)
				JZ				@F								; if true, must be multiple of 64 bits to shift, no bits, just words to shift
				VPBROADCASTQ	ZMM29, RAX						; Nr bits to shift right
				VPXORQ			ZMM28, ZMM28, ZMM28				; 
				VALIGNQ			ZMM30, ZMM31, ZMM28, 7			; shift copy of words left one word (to get low order bits aligned for shift)
				VPSHRDVQ		ZMM31, ZMM30, ZMM29				; shift, concatenating low bits of next word with each word to shift in

; with the bits shifted within the words (if needed), if the desired shift is more than 64 bits, word shifts are required
@@:				LEA				RAX, ShiftMaskRt	
				SHR				R8W, 6							; divide Nr bits to shift by 64 giving Nr words to shift (can only be 0-7 based on above validation)
				LEA				RAX,  [ RAX ] [ R8 ]			; Add index to base address of mask table
				KMOVB			K1, B_PTR [ RAX ]				; load mask for words to be zeroed
				LEA				RAX, ShiftPermuteRt				; address of permute table
				SHL				R8W, 6							; multiply by 64 to get offset into permute table		
				VMOVDQA64		ZMM29, ZM_PTR [R8] [ RAX ]		; load permute indices (with calculated offset)
				VPERMQ			ZMM31 {k1}{z}, ZMM29, ZMM31		; permute words in zmm31 to achieve word shift
				VMOVDQA64		ZM_PTR [ RCX ], ZMM31			; store result at callers destination
				RET
shr_u			ENDP

	ELSE
;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;			shr_u		-	shift supplied source 512bit (8 QWORDS) right, put in destination
;			Another version: this one without "z" support.
;			Enough different that a local frame is needed and a number of non-volatile regs
;			A local frame is set up with the 'locals' structure
;

shr_u_Locals	STRUCT
				QWORD			?
shr_u_Locals	ENDS

; Declare proc, save regs, set up frame
				Proc_w_Local	shr_u, shr_u_Locals, R12, R13, R14, R15, RDI, RBX
				MOV				RCXHome, RCX

				CMP				R8W, 512						; handle edge case, shift 512 or more bits
				JL				@F
				Zero512			RCX								; zero destination, return
				JMP				@@R

@@:				AND				R8, 511							; ensure no high bits above shift count
				JNZ				@@shift							; handle edge case, zero bits to shift
				CMP				RCX, RDX
				JE				@@R								; destination is the same as the source: no copy needed
				Copy512			RCX, RDX						; no shift, just copy (destination, source already in regs), return
				JMP				@@R

; load sequential regs with source 8 qwords
@@shift:		MOV				R9, Q_PTR [ RDX ] [ 0 * 8 ]		; R9 holds source at index [0], most significant qword
				MOV				R10, Q_PTR [ RDX ] [ 1 * 8 ]	; R10 <- [1]
				MOV				R11, Q_PTR [ RDX ] [ 2 * 8 ]	; R11 <- [2]
				MOV				R12, Q_PTR [ RDX ] [ 3 * 8 ]	; R12 <- [3]
				MOV				R13, Q_PTR [ RDX ] [ 4 * 8 ]	; R13 <- [4]
				MOV				R14, Q_PTR [ RDX ] [ 5 * 8 ]	; R14 <- [5]
				MOV				R15, Q_PTR [ RDX ] [ 6 * 8 ]	; R15 <- [6]
				MOV				RDI, Q_PTR [ RDX ] [ 7 * 8 ]	; RDI holds source at index [7], least significant qword

; determine if / how many bits to shift
				LEA				RCX, [ R8 ]						; R8 still carries users shift count.
				AND				RCX, 03Fh						; Mask down to Nr of bits to shift right -> RCX
				JZ				@@nobits						; might be word shifts, but no bit shifts required
				LEA				RBX, [ 64 ]
				SUB				RBX, RCX						; Nr to shift left -> RBX

; Using Macro for repetitive ops. Reduces chance of typo, easier to maintain, but not used anywhere else
				ShiftOrR		R15, RDI						; RDI is target to shift, but need bits from R15 to fill in high bits
				ShiftOrR		R14, R15						; now R15 is target, but need bits from R14
				ShiftOrR		R13, R14						; and on ...
				ShiftOrR		R12, R13
				ShiftOrR		R11, R12
				ShiftOrR		R10, R11
				ShiftOrR		R9, R10				
				SHRX			R9, R9, RCX						; no bits to OR in on the index 0 (high order) word, just shift it.

; with the bits shifted within the words, if the desired shift is more than 64 bits, word shifts are required
; verify Nr of word shift is zero to seven, use it as index into jump table; jump to appropriate shift
@@nobits:		SHR				R8W, 6							; divide bit shift count by 64 to get Nr words to shift
				AND				R8, 7							; mask out anything above seven (shouldnt happen, but . . . jump table, be sure)
				SHL				R8W, 3							; multiply by 8 to get offset into jump table
				LEA				RAX, jtbl						; base address of jump table
				ADD				R8, RAX							; add to offset
				XOR				EAX, EAX						; clear rax for use in zeroing words shifted "in"
				MOV				RCX, RCXHome					; restore RCX
				JMP				Q_PTR [ R8 ]
jtbl:
				QWORD			S0, S1, S2, S3, S4, S5, S6, S7
; no word shift, just bits, so store words in destination in the same order as they are in the regs
S0:				MOV				Q_PTR [ RCX ] [ 0 * 8 ], R9
				MOV				Q_PTR [ RCX ] [ 1 * 8 ], R10
				MOV				Q_PTR [ RCX ] [ 2 * 8 ], R11
				MOV				Q_PTR [ RCX ] [ 3 * 8 ], R12
				MOV				Q_PTR [ RCX ] [ 4 * 8 ], R13
				MOV				Q_PTR [ RCX ] [ 5 * 8 ], R14
				MOV				Q_PTR [ RCX ] [ 6 * 8 ], R15
				MOV				Q_PTR [ RCX ] [ 7 * 8 ], RDI	
				JMP				@@R
; one word shift, store from regs to callers destination offsetting one word (zeroing first, most significant, word)
S1:				MOV				Q_PTR [ RCX ] [ 0 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 1 * 8 ], R9
				MOV				Q_PTR [ RCX ] [ 2 * 8 ], R10
				MOV				Q_PTR [ RCX ] [ 3 * 8 ], R11
				MOV				Q_PTR [ RCX ] [ 4 * 8 ], R12
				MOV				Q_PTR [ RCX ] [ 5 * 8 ], R13
				MOV				Q_PTR [ RCX ] [ 6 * 8 ], R14
				MOV				Q_PTR [ RCX ] [ 7 * 8 ], R15	
				JMP				@@R
; two word shift
S2:				MOV				Q_PTR [ RCX ] [ 0 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 1 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 2 * 8 ], R9
				MOV				Q_PTR [ RCX ] [ 3 * 8 ], R10
				MOV				Q_PTR [ RCX ] [ 4 * 8 ], R11
				MOV				Q_PTR [ RCX ] [ 5 * 8 ], R12
				MOV				Q_PTR [ RCX ] [ 6 * 8 ], R13
				MOV				Q_PTR [ RCX ] [ 7 * 8 ], R14	
				JMP				@@R

S3:				MOV				Q_PTR [ RCX ] [ 0 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 1 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 2 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 3 * 8 ], R9
				MOV				Q_PTR [ RCX ] [ 4 * 8 ], R10
				MOV				Q_PTR [ RCX ] [ 5 * 8 ], R11
				MOV				Q_PTR [ RCX ] [ 6 * 8 ], R12
				MOV				Q_PTR [ RCX ] [ 7 * 8 ], R13	
				JMP				@@R

S4:				MOV				Q_PTR [ RCX ] [ 0 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 1 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 2 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 3 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 4 * 8 ], R9
				MOV				Q_PTR [ RCX ] [ 5 * 8 ], R10
				MOV				Q_PTR [ RCX ] [ 6 * 8 ], R11
				MOV				Q_PTR [ RCX ] [ 7 * 8 ], R12			
				JMP				@@R

S5:				MOV				Q_PTR [ RCX ] [ 0 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 1 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 2 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 3 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 4 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 5 * 8 ], R9
				MOV				Q_PTR [ RCX ] [ 6 * 8 ], R10
				MOV				Q_PTR [ RCX ] [ 7 * 8 ], R11	
				JMP				@@R

S6:				MOV				Q_PTR [ RCX ] [ 0 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 1 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 2 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 3 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 4 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 5 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 6 * 8 ], R9
				MOV				Q_PTR [ RCX ] [ 7 * 8 ], R10	
				JMP				@@R

S7:				MOV				Q_PTR [ RCX ] [ 0 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 1 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 2 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 3 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 4 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 5 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 6 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 7 * 8 ], R9		

; restore non-volatile regs to as-called condition
@@R:
				Local_Exit		RBX, RDI, R15, R14, R13, R12

shr_u			ENDP
	
ENDIF
;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;			shl_u		-	shift supplied source 512bit (8 QWORDS) left, put in destination
;			Prototype:		void shl_u( u64* destination, u64* source, u16 bits_to_shift);
;			destination	-	Address of 64 byte aligned array of 8 64-bit words (QWORDS) 512 bits (in RCX)
;			source		-	Address of 64 byte aligned array of 8 64-bit words (QWORDS) 512 bits (in RDX)
;			bits		-	Number of bits to shift. Will fill with zeros, truncate those shifted out (in R8W)
;			returns		-	nothing (0)

IF	__UseZ
				Leaf_Entry		shl_u				; Declare code section, public proc, no prolog, no frame, exceptions handled by caller
				CheckAlign		RCX								; (OUT) destination of shifted 8 QWORDs
				CheckAlign		RDX								; (IN)	source of 8 QWORDS

				CMP				R8W, 512						; handle edge case, shift 512 or more bits
				JL				@F
				Zero512			RCX								; zero destination
				RET
@@:				AND				R8, 511							; mask out high bits above shift count, test for 0
				JNE				@F								; handle edge case, shift zero bits
				CMP				RCX, RDX						; destination same as source?
				JE				@@r								; no copy needed
				Copy512			RCX, RDX						; no shift, just copy (destination, source already in regs)
@@r:			RET
@@:				VMOVDQA64		ZMM31, ZM_PTR [ RDX ]			; load the 8 qwords into zmm reg (note: word order)
				LEA				RAX, [ R8 ]
				AND				AX, 03fh
				JZ				@F								; must be multiple of 64 bits to shift, no bits, just words to shift

; Do the shift of bits within the 64 bit words
				VPBROADCASTQ	ZMM29, RAX						; Nr bits to shift left
				VPXORQ			ZMM28, ZMM28, ZMM28				; 
				VALIGNQ			ZMM30, ZMM28, ZMM31, 1			; shift copy of words right one word (to get low order bits aligned for shift)
				VPSHLDVQ		ZMM31, ZMM30, ZMM29				; shift, concatenating low bits of next word with each word to shift in

; with the bits shifted within the words, if the desired shift is more than 64 bits, word shifts are required
@@:				LEA				RAX, ShiftMaskLt	
				SHR				R8W, 6							; divide Nr bits to shift by 64 giving Nr words to shift (can only be 0-7 based on above validation)
				LEA				RAX, [ RAX ] [ R8 ]				; Add index to base address of mask table
				KMOVB			K1, B_PTR [ RAX ]				; create mask for words to be zeroed
				LEA				RAX, ShiftPermuteLt				; address of permute table
				SHL				R8W, 6							; multiply by 64 to get offset into permute table		
				VMOVDQA64		ZMM29, ZM_PTR [ R8 ] [ RAX ]	; load permute indices (with calculated offset)
				VPERMQ			ZMM31 {k1}{z}, ZMM29, ZMM31		; permute words in zmm31 to achieve word shift
				VMOVDQA64		ZM_PTR [ RCX ], ZMM31			; store result at callers destination
				RET
shl_u			ENDP			
	ELSE
;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;			shl_u		-	shift supplied source 512bit (8 QWORDS) left, put in destination
;			Another version: this one without "z" support.
;			Enough different that a local frame is needed and a number of non-volatile regs
;			A local frame is set up with the 'locals' structure
;


shl_u_Locals	STRUCT
				QWORD			?
shl_u_Locals	ENDS

; Declare proc, save regs, set up frame
				Proc_w_Local	shl_u, shl_u_Locals, R12, R13, R14, R15, RDI, RBX
				MOV				RCXHome, RCX

				CMP				R8W, 512						; handle edge case, shift 512 or more bits
				JL				@F
				Zero512			RCX								; zero destination, return
				JMP				@@R

@@:				AND				R8, 511							; ensure no high bits above shift count
				JNZ				@@shift							; handle edge case, zero bits to shift
				CMP				RCX, RDX
				JE				@@R								; destination is the same as the source: no copy needed
				Copy512			RCX, RDX						; no shift, just copy (destination, source already in regs), return
				JMP				@@R

; load sequential regs with source 8 qwords
@@shift:		MOV				R9, Q_PTR [ RDX ] [ 0 * 8 ]		; R9 holds source at index [0], most significant qword
				MOV				R10, Q_PTR [ RDX ] [ 1 * 8 ]	; R10 <- [1]
				MOV				R11, Q_PTR [ RDX ] [ 2 * 8 ]	; R11 <- [2]
				MOV				R12, Q_PTR [ RDX ] [ 3 * 8 ]	; R12 <- [3]
				MOV				R13, Q_PTR [ RDX ] [ 4 * 8 ]	; R13 <- [4]
				MOV				R14, Q_PTR [ RDX ] [ 5 * 8 ]	; R14 <- [5]
				MOV				R15, Q_PTR [ RDX ] [ 6 * 8 ]	; R15 <- [6]
				MOV				RDI, Q_PTR [ RDX ] [ 7 * 8 ]	; RDI holds source at index [7], least significant qword

; determine if / how many bits to shift
				LEA				RCX, [ R8 ]						; R8 still carries users shift count.
				AND				RCX, 03Fh						; Mask down to Nr of bits to shift left -> RCX
				JZ				@@nobits						; might be word shifts, but no bit shifts required
				LEA				RBX, [ 64 ]
				SUB				RBX, RCX						; Nr to shift right -> RBX

; Macro for repetitive ops. Reduces chance of typo, easier to maintain, but not used anywhere else
				ShiftOrL		R10, R9							; R9 is target to shift, but need bits from R10 to fill in low bits
				ShiftOrL		R11, R10
				ShiftOrL		R12, R11
				ShiftOrL		R13, R12
				ShiftOrL		R14, R13
				ShiftOrL		R15, R14
				ShiftOrL		RDI, R15				
				SHLX			RDI, RDI, RCX					; no bits to OR in on the index 0 (high order) word, just shift it.

; with the bits shifted within the words, if the desired shift is more than 64 bits, word shifts are required
; verify Nr of word shift is zero to seven, use it as index into jump table; jump to appropriate shift
@@nobits:		SHR				R8W, 6
				AND				R8, 07h 
				SHL				R8W, 3
				LEA				RAX, @@jtbl
				ADD				R8, RAX
				XOR				EAX, EAX						; clear rax for use as zeroing words shifted "in"
				MOV				RCX, RCXHome					; restore RCX, destination address
				JMP				Q_PTR [ R8 ]

@@jtbl:
				QWORD			@@0, @@1, @@2, @@3, @@4, @@5, @@6, @@7

; no word shift, just bits, so store words in destination in the same order as they are
@@0:			MOV				Q_PTR [ RCX ] [ 0 * 8 ], R9
				MOV				Q_PTR [ RCX ] [ 1 * 8 ], R10
				MOV				Q_PTR [ RCX ] [ 2 * 8 ], R11
				MOV				Q_PTR [ RCX ] [ 3 * 8 ], R12
				MOV				Q_PTR [ RCX ] [ 4 * 8 ], R13
				MOV				Q_PTR [ RCX ] [ 5 * 8 ], R14
				MOV				Q_PTR [ RCX ] [ 6 * 8 ], R15
				MOV				Q_PTR [ RCX ] [ 7 * 8 ], RDI	
				JMP				@@R

; one word shift, shifting one word (64+ bits) so store words in destination shifted left one, fill with zero
@@1:			MOV				Q_PTR [ RCX ] [ 0 * 8 ], R10
				MOV				Q_PTR [ RCX ] [ 1 * 8 ], R11
				MOV				Q_PTR [ RCX ] [ 2 * 8 ], R12
				MOV				Q_PTR [ RCX ] [ 3 * 8 ], R13
				MOV				Q_PTR [ RCX ] [ 4 * 8 ], R14
				MOV				Q_PTR [ RCX ] [ 5 * 8 ], R15
				MOV				Q_PTR [ RCX ] [ 6 * 8 ], RDI
				MOV				Q_PTR [ RCX ] [ 7 * 8 ], RAX	
				JMP				@@R

; two word shift
@@2:			MOV				Q_PTR [ RCX ] [ 0 * 8], R11
				MOV				Q_PTR [ RCX ] [ 1 * 8], R12
				MOV				Q_PTR [ RCX ] [ 2 * 8], R13
				MOV				Q_PTR [ RCX ] [ 3 * 8], R14
				MOV				Q_PTR [ RCX ] [ 4 * 8], R15
				MOV				Q_PTR [ RCX ] [ 5 * 8], RDI
				MOV				Q_PTR [ RCX ] [ 6 * 8], RAX
				MOV				Q_PTR [ RCX ] [ 7 * 8], RAX	
				JMP				@@R

; three word shift
@@3:			MOV				Q_PTR [ RCX ] [ 0 * 8 ], R12
				MOV				Q_PTR [ RCX ] [ 1 * 8 ], R13
				MOV				Q_PTR [ RCX ] [ 2 * 8 ], R14
				MOV				Q_PTR [ RCX ] [ 3 * 8 ], R15
				MOV				Q_PTR [ RCX ] [ 4 * 8 ], RDI
				MOV				Q_PTR [ RCX ] [ 5 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 6 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 7 * 8 ], RAX	
				JMP				@@R

; four word shift
@@4:			MOV				Q_PTR [ RCX ] [ 0 * 8 ], R13
				MOV				Q_PTR [ RCX ] [ 1 * 8 ], R14
				MOV				Q_PTR [ RCX ] [ 2 * 8 ], R15
				MOV				Q_PTR [ RCX ] [ 3 * 8 ], RDI
				MOV				Q_PTR [ RCX ] [ 4 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 5 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 6 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 7 * 8 ], RAX	
				JMP				@@R

; five word shift
@@5:			MOV				Q_PTR [ RCX ] [ 0 * 8 ], R14
				MOV				Q_PTR [ RCX ] [ 1 * 8 ], R15
				MOV				Q_PTR [ RCX ] [ 2 * 8 ], RDI
				MOV				Q_PTR [ RCX ] [ 3 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 4 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 5 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 6 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 7 * 8 ], RAX	
				JMP				@@R

; six word shift
@@6:			MOV				Q_PTR [ RCX ] [ 0 * 8], R15
				MOV				Q_PTR [ RCX ] [ 1 * 8], RDI
				MOV				Q_PTR [ RCX ] [ 2 * 8], RAX
				MOV				Q_PTR [ RCX ] [ 3 * 8], RAX
				MOV				Q_PTR [ RCX ] [ 4 * 8], RAX
				MOV				Q_PTR [ RCX ] [ 5 * 8], RAX
				MOV				Q_PTR [ RCX ] [ 6 * 8], RAX
				MOV				Q_PTR [ RCX ] [ 7 * 8], RAX	
				JMP				@@R

; seven word shift
@@7:			MOV				Q_PTR [ RCX ] [ 0 * 8 ], RDI
				MOV				Q_PTR [ RCX ] [ 1 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 2 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 3 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 4 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 5 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 6 * 8 ], RAX
				MOV				Q_PTR [ RCX ] [ 7 * 8 ], RAX

;	restore non-volatile regs to as-called condition
@@R:			Local_Exit		RBX, RDI, R15, R14, R13, R12
				RET
shl_u			ENDP
	ENDIF

ui512_shift		ENDS												; end of section
				END													; end of module