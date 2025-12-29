;
;			ui512_division
;
;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;
;			File:			ui512_division.asm
;			Author:			John G. Lynch
;			Legal:			Copyright @2025, per MIT License below
;			Date:			November 19, 2025  (file creation)

				INCLUDE			ui512_legalnotes.inc
				INCLUDE			ui512_compile_time_options.inc
				INCLUDE			ui512_macros.inc
				INCLUDE			ui512_externs.inc
.NOLISTIF
				OPTION			CASEMAP:NONE
ui512_division	SEGMENT			PARA 'CODE'

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;			EXTERNDEF		div_u:PROC					; s16 div_u( u64* quotient, u64* remainder, u64* dividend, u64* divisor)
;			div_u			-	divide 512 bit dividend by 512 bit divisor, giving 512 bit quotient and remainder
;			Prototype:		-	s16 div_u( u64* quotient, u64* remainder, u64* dividend, u64* divisor);
;			quotient		-	Address of 8 QWORDS to store resulting quotient (in RCX)
;			remainder		-	Address of 8 QWORDs for resulting remainder (in RDX)
;			dividend		-	Address of 8 QWORDS dividend (in R8)
;			divisor			-	Address of 8 QWORDs divisor (in R9)
;			returns			-	0 for success, -1 for attempt to divide by zero, (GP_Fault) for mis-aligned parameter address

div_u_Locals	STRUCT

currnumerator	QWORD			16 dup (?)							; scratch working copy of dividend (numerator)
qdiv			QWORD			16 dup (?)							; scratch working copy of (trial) qhat * divisor
quotient		QWORD			8 dup (?)							; working copy of quotient
normdivisor		QWORD			8 dup (?)							; working copy of normalized divisor
qHat			QWORD			?
rHat			QWORD			?									; trial quotient and remainder
nDiv			QWORD			?									; first qword of normalized divisor
addbackRDX		QWORD			?
addbackR11		QWORD			?									; saved pointers for add-back step
mIdx			WORD			?
mMSB			WORD			?
mDim			WORD			?									; indexes and dimensions of dividend (numerator) Note: dimensions are zero-based (0 to 7)										
nIdx			WORD			?
nMSB			WORD			?
nDim			WORD			?									; indexes and dimensions of divisor (denominator)
jIdx			WORD			?
jDim			WORD			?									; loop index and dimension of dividend (numerator) 
normf			WORD			?
sublen			WORD			?									; Nr bits to shift for normalization, length of subtraction
				WORD			2 dup (?)							; to get back to 16 byte align for stack alloc (adjust as necessary)
div_u_Locals	ENDS

; Declare proc, save regs, set up frame
				Proc_w_Local	div_u, div_u_Locals, R12
				MOV				RCXHome, RCX
				MOV				RDXHome, RDX
				MOV				R8Home, R8
				MOV				R9Home, R9

				CheckAlign		RCX, @ret							; (out) Quotient
				CheckAlign		RDX, @ret							; (out) Remainder
				CheckAlign		R8, @ret							; (in) Dividend
				CheckAlign		R9, @ret							; (in) Divisor

				XCHG			RDI, R10
				XOR				RAX, RAX
				LEA				RDI, l_Ptr.currnumerator
				MOV				ECX, sizeof(div_u_Locals)
				REP				STOSB
				XCHG			RDI, R10
				MOV				RCX, RCXHome
; Initialize; in frame / stack reserved memory, clear 16 qword area for working version of quotient; set up indexes for loop
				
				Zero512			RCX									; zero callers quotient
				Zero512			RDX									; zero callers remainder
	IF __UseQ
				VPXORQ			ZMM31, ZMM31, ZMM31
				VMOVDQA64		ZM_PTR l_Ptr.quotient, ZMM31
				VMOVDQA64		ZM_PTR l_Ptr.quotient + [ 8 * 8 ], ZMM31
	ELSE
				XCHG			RDI, R10
				XOR				RAX, RAX							; clear entire framed area
				LEA				RDI, RBP							; clear working copy of contigous quotient, remainder				
				MOV				ECX, 16								; need to start as zero, results are accumulated
				REP				STOSQ
				XCHG			RDI, R10
	ENDIF

; Examine divisor
				MOV				RCX, R9								; divisor
				CALL			msb_u								; get most significant bit
				TEST			AX, AX								; msb < 0? 
				JL				divbyzero							; divisor is zero, abort
				JE				divbyone							; divisor is one, exit with remainder = 0, quotient = dividend 
				CMP				AX, 64								; divisor only one 64-bit word?
				JGE				mbynDiv								; no, do divide of m digit by n digit

; Divide of m 64-bit qwords by one 64 bit qword divisor, use the quicker divide routine (div_uT64), and return
				MOV				RCX, RCXHome						; set up parms for call to div by 64bit: RCX - addr of quotient
				MOV				RDX, RDXHome						; RDX - addr of remainder
				MOV				R8, R8Home							; R8 - addr of dividend
				MOV				RAX, R9Home
				MOV				R9, Q_PTR [ RAX ] [ 7 * 8 ]			; R9 - value of 64 bit divisor
				CALL			div_uT64
				MOV				RDX, RDXHome						; move 64 bit remainder to last word of 8 word remainder
				MOV				RCX, Q_PTR [ RDX ]					; get the one qword remainder
				Zero512			RDX									; clear the 8 qword callers remainder
				MOV				Q_PTR [ RDX ] [ 7 * 8 ], RCX		; put the one qword remainder in the least significant qword of the callers remainder
				JMP				cleanupret							; exit normally

; Divide m digit by n digit
mbynDiv:
				MOV				l_Ptr.nMSB, AX						; save msb of divisor
				SHR				AX, 6								;
				MOV				l_Ptr.nDim, AX						; Dimension (Nr Qwords) of divisor (n)
				MOV				RCX, R8
				CALL			msb_u								; get msb of dividend
				TEST			AX, AX
				JL				numtoremain							; dividend == zero > answer is zero with remainder
				CMP				AX, l_Ptr.nMSB						; msb of dividend < msb of divisor? -> answer is zero with dividend going to remainder
				JL				numtoremain							;
				MOV				l_Ptr.mMSB, AX						; save msb of dividend
				SHR				AX, 6
				MOV				l_Ptr.mDim, AX						; save dimension (Nr Qwords) of dividend (m)

; Normalize divisor 
				MOV				R8W, l_Ptr.nMSB						; get msb of divisor
				NOT				R8W									; complement
				AND				R8W, 63								; get bits to shift (max 63)
				MOV				l_Ptr.normf, R8W					; save normalization factor	
				LEA				RCX, l_Ptr.normdivisor				; put normalized divisor here
				MOV				RDX, R9Home							; using callers divisor
				CALL			shl_u								; shifting left until msb is in high bit position

; Normalize dividend
				LEA				RCX, l_Ptr.currnumerator [ 8 * 8 ]	; put normalized dividend here
				MOV				RDX, R8Home							; using callers dividend
				MOV				R8W, l_Ptr.normf					; get normalization factor
				CALL			shl_u								; shifting left until msb is in high bit position

; Check: did we shift  out msb bits of dividend?
				MOV				AX, l_Ptr.mMSB						; get msb of dividend
				ADD				AX, R8W								; add in shift count
				CMP				AX, 511								; did we shift out bits?
				JLE				normdivdone							; no
				INC				l_Ptr.mDim							; yes, increment dimension of dividend
				CMP				l_Ptr.mDim, 8						; did we overflow dimension?
				JLE				normdivdone							; no
				MOV				R8W, l_Ptr.nMSB
				LEA				RCX, l_Ptr.currnumerator [ 0 * 8 ]	;
				MOV				RDX, R8Home							; using callers dividend
				Call			shl_u								; left to get shifted out bits Note: we shift entire 512 bits - msb
																	; putting low into new msb ninth word of currnumerator	
normdivdone:
; To recap: we have checked edge cases (div by zero, div by one, num < denom)
; We have normalized divisor and dividend, and set up dimensions of each
; The leading bit of the normalized divisor is in bit 63 of qword nDim, thus the first qword of the normalized divisor is >= 0x8000000000000000
; The dividend is in currnumerator, and may be up to one qword longer than before normalization, with the leading bit in bit 62 of qword mDim, thus
; the first qword of the normalized dividend is < 0x800000000000000
; thus the first qword of the normalized dividend is always less than the first qword of the normalized divisor, and the first divide
; will be of the form (at most) 0x7FFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF / 0x8000000000000000, yielding a qHat of 0x0FFFFFFFFFFFFFFF
; which fits in 64 bits.

; Reminder: the dimensions (mDim, nDim) are zero-based (0 to 7), so the actual number of qwords is dimension + 1, and the most significant
; qword is at index = (7 - dimension); thus for a dimension of 3, the number of qwords is 4, and the most significant qword is at index 4 (7 - 3), least significant
; at index 7. Remember also that the working copy of the dividend (currnumerator) is up to 9 qwords long, with the most significant qword at index 8 - dimension,
; least significant at index 8. And the base is currnumerator [ 7 * 8 ] (if there was a qword added due to normalization. Put another way, 
; currnumerator goes from 7 to 15, with 7 being most significant, 15 least significant. Indexes are started 

; Set up for main divide loop



; Main divide loop: for j = mDim - nDim downto 0

maindivloop:

				CALL			compute_qhat						; compute qHat and rHat
				CALL			multiply_and_subtract				; multiply qHat * divisor, subtract from currnumerator
				CALL			check_and_addback					; check if we need to add back


; Store quotient
				LEA				R11, l_Ptr.quotient
				XOR				R8, R8		
				MOV				R8W, l_Ptr.jIdx
				MOV				RAX, l_Ptr.qHat
				MOV				 Q_PTR [ R11 ] [ R8 * 8 ], RAX		; store qHat in quotient working copy
				JGE				maindivloop							; loop until jDim < 0
; Unnormalize remainder
				LEA				RCX, RDXHome						; put remainder at callers remainder
				LEA				RDX, l_Ptr.currnumerator			; using working copy of currnumerator
				MOV				R8W, l_Ptr.normf					; get normalization factor
				CALL			shr_u								; shifting right to unnormalize

; Normal exit
cleanupret:
				XOR				RAX, RAX							; return zero

; Either fall-thru normal exit, or from exception handling
cleanupwretcode:			
				Local_Exit		R12
; Flat exit if exception found before frame setup, or fall thru normal exit
@ret:
				RET

; Exception handling, divide by zero
divbyzero:
				LEA				EAX, [ retcode_neg_one ]
				JMP				cleanupwretcode

compute_qhat:
				; TO DO: implement compute qhat routine
				RET
multiply_and_subtract:
				; TO DO: implement multiply and subtract routine
				RET
check_and_addback:
				; TO DO: implement check and add-back routine
				RET
; Exception handling, divide by one
divbyone:
				MOV				RCX, RCXHome						; callers quotient
				MOV				R8,  R8Home							; callers dividend
				Copy512			RCX, R8								; copy dividend to quotient
				MOV				RDX, RDXHome						; callers remainder	
				Zero512			RDX									; remainder is zero
				JMP				cleanupret

; Exception handling, If dimension of numerator (m) is less than dimension of denominator (n), result is zero, remainder is numerator
numtoremain:
				MOV				R8, R8Home							; callers dividend
				MOV				RDX, RDXHome						; callers remainder
				Copy512			RDX, R8
				JMP				cleanupret

div_u			ENDP

;
;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;			EXTERNDEF		div_uT64:PROC				; s16 div_uT64( u64* quotient, u64* remainder, u64* dividend, u64 divisor)
;			div_uT64		-	divide 512 bit dividend by 64 bit divisor, giving 512 bit quotient and 64 bit remainder
;			Prototype:		-	s16 div_u( u64* quotient, u64* remainder, u64* dividend, u64 divisor);
;			quotient		-	Address of 8 QWORDS to store resulting quotient (in RCX)
;			remainder		-	Address of QWORD for resulting remainder (in RDX)
;			dividend		-	Address of 8 QWORDS dividend (in R8)
;			divisor			-	Value of 64 bit divisor (in R9)
;			returns			-	0 for success, -1 for attempt to divide by zero, (GP_Fault) for mis-aligned parameter address
;
;			Regs with contents destroyed, not restored: RAX, RDX, R10 (each considered volitile, but caller might optimize on other regs)

				Leaf_Entry		div_uT64							; Declare code section, public proc, no prolog, no frame, exceptions handled by caller
				CheckAlign		RCX									; (out) Quotient
				CheckAlign		R8									; (in) Dividend

; Test divisor for divide by zero				
				TEST			R9, R9
				JZ				@@DivByZero

; DIV instruction (64-bit) uses RAX and RDX. Need to move RDX (addr of remainder) out of the way; start it off with zero
				MOV				R10, RDX							; save addr of callers remainder
				XOR				RDX, RDX

; FOR EACH index of 0 thru 7: get qword of dividend, divide by divisor, store qword of quotient
				FOR				idx, < 0, 1, 2, 3, 4, 5, 6, 7 >
				MOV				RAX, Q_PTR [ R8 ] [ idx * 8 ]		; dividend [ idx ] -> RAX
				DIV				R9									; divide by divisor in R9 (as passed)
				MOV				Q_PTR [ RCX ] [ idx * 8 ], RAX		; quotient [ idx ] <- RAX ; Note: remainder in RDX for next divide
				ENDM

; Last (least significant qword) divide leaves a remainder, store it at callers remainder
				MOV				Q_PTR [ R10 ], RDX					; remainder to callers remainder
				XOR				RAX, RAX							; return zero
@@exit:			
				RET

; Exception handling, divide by zero
@@DivByZero:
				Zero512			RCX									; Divide by Zero. Could throw fault, but returning zero quotient, zero remainder
				XOR				RAX, RAX
				MOV				Q_PTR [ R10 ] , RAX
				LEA				EAX, [ retcode_neg_one ]			; return error (div by zero)
				JMP				@@exit

div_uT64		ENDP

ui512_division	ENDS												; end of section

				END													; end of module