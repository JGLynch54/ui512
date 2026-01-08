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

currnumerator	QWORD			16 dup (?)							; scratch working copy of dividend (numerator). could be 9 qwords, 16 declared for alignment
qdiv			QWORD			16 dup (?)							; scratch working copy of (trial) qhat * divisor. could be 9 qwords, 16 declared for alignment

quotient		QWORD			8 dup (?)							; working copy of quotient
normdivisor		QWORD			8 dup (?)							; working copy of normalized divisor
nDiv			QWORD			?									; first qword of normalized divisor

qHat			QWORD			?									; trial quotient 
rHat			QWORD			?									; trial remainder
									
mMSB			WORD			?									; indexes and dimensions of dividend (numerator) Note: dimensions are zero-based (0 to 7)
mDim			WORD			?
mIdx			WORD			?	
mllimit			WORD			?

nMSB			WORD			?									; indexes and dimensions of divisor (denominator)
nDim			WORD			?									
nIdx			WORD			?
nllimit			WORD			?

jDim			WORD			?
jIdx			WORD			?
jllimit			WORD			?

normf			WORD			?

;				WORD			3 dup (?)							; to get to 16 byte align for stack alloc (adjust as necessary)

div_u_Locals	ENDS

; Declare proc, save regs, set up frame
				Proc_w_Local	div_u, div_u_Locals, R12, R13, R14, R15, RDI
				MOV				RDXHome, RDX						; save the rest of parameter regs in callers reserved 'home' locations (RCX already home)
				MOV				R8Home, R8
				MOV				R9Home, R9

				CheckAlign		RCX, @ret							; (out) Quotient
				CheckAlign		RDX, @ret							; (out) Remainder
				CheckAlign		R8, @ret							; (in) Dividend
				CheckAlign		R9, @ret							; (in) Divisor

; clear callers quotient and remainder, and working memory (frame),
				Zero512			RCX									; zero callers quotient
				Zero512			RDX									; zero callers remainder

				XOR				RAX, RAX
				LEA				RDI, l_Ptr.currnumerator			; first declared variable in the reserved area is currnumerator
				MOV				ECX, sizeof(div_u_Locals) / 8		; length in QWORDS
				REP				STOSQ								; clear working area

; Examine divisor
; Note on msb_u: a returned zero means the significant bit is bit0 of the eighth word of the 512bit source parameter; (the right most bit)
; a returned 511 means bit63 of the first word (the left most bit), a returned -1 means no bits set (zero)
				MOV				RCX, R9								; address of divisor
				CALL			msb_u								; get Nr of most significant bit
				TEST			AX, AX								; 
				JL				divbyzero							; msb < 0?  -> divisor is zero, abort
				JE				divbyone							; msb == 0? -> divisor is one, exit with remainder = 0, quotient = dividend 
				CMP				AX, 64								; divisor only one 64-bit word?
				JGE				mbynDiv								; no, do divide of m QWORD by n QWORDs, both >= 2 qwords

; Divide of m 64-bit qwords by one 64 bit qword divisor, use the quicker divide routine (div_uT64), and return
				MOV				RCX, RCXHome						; set up parms for call to div by 64bit: RCX - addr of quotient
				MOV				RDX, RDXHome						; RDX - addr of remainder
				MOV				R8, R8Home							; R8 - addr of dividend
				MOV				RAX, R9Home							; RAX - addr of divisor
				MOV				R9, Q_PTR [ RAX ] [ 7 * 8 ]			; R9 - value of 64 bit divisor
				CALL			div_uT64							; call the divide by 64 bit routine
				MOV				RDX, RDXHome						; move 64 bit remainder to last word of 8 word remainder
				MOV				RCX, Q_PTR [ RDX ]					; get the one qword remainder
				Zero512			RDX									; clear the 8 qword callers remainder
				MOV				Q_PTR [ RDX ] [ 7 * 8 ], RCX		; put the one qword remainder in the least significant qword of the callers remainder
				JMP				cleanupret							; exit normally

; Divide an m digit (qword) by dividend by an n digit (qword) divisor, both >= 2 qwords
mbynDiv:
				MOV				l_Ptr.nMSB, AX						; save msb of divisor
				SHR				AX, 6								; divide bits by 64 to get qword count
				MOV				l_Ptr.nDim, AX						; Dimension (Nr Qwords) of divisor (n)

; examine dividend
				MOV				RCX, R8Home							; retrieve address of dividend
				CALL			msb_u								; get msb of dividend
				TEST			AX, AX								; zero?
				JL				numtoremain							; dividend == zero -> answer is zero with remainder
				CMP				AX, l_Ptr.nMSB						; msb of dividend < msb of divisor? -> answer is zero with dividend going to remainder
				JL				numtoremain							;
				MOV				l_Ptr.mMSB, AX						; save msb of dividend
				SHR				AX, 6
				MOV				l_Ptr.mDim, AX						; save dimension (Nr Qwords) of dividend (m)

; So far: we have checked ( and processed) edge cases (div by zero, div by one, num < denom)
; and we have m >= 2, n >= 2, and m >= n

; The dimensions (mDim, nDim) are zero-based (0 to 7), and are a minimum of 2.
; The actual number of qwords is dimension + 1, and the most significant qword is at index = (7 - dimension)

; thus for a dimension of 5, the number of qwords is 6, and the most significant qword is at index 2 (7 - 5),
; least significant at index 7. 

; Normalize divisor 
				MOVZX			RAX, l_Ptr.nMSB						; Nr bits in divisor
				AND				RAX, 63								; masked down modulo 64
				MOV				R8, 63								; max bits in qword
				SUB				R8W, AX								; calculate shift count
				MOV				l_Ptr.normf, R8W					; save normalization factor	
				LEA				RCX, l_Ptr.normdivisor				; destination of normalized divisor
				MOV				RDX, R9Home							; using callers divisor
				CALL			shl_u								; shifting left so msb is in high bit position

; The working copy of the dividend (currnumerator) is up to 9 qwords long, with the most significant qword at index 15 - dimension, least significant at index 15. 
; The base is currnumerator [ 7 * 8 ] if there was a qword added due to normalization.
; Put another way, currnumerator goes from (15 - dim)) being most significant to 15 least significant. 

; Normalize dividend aka numerator, or current numerator or currnumerator
				LEA				RCX, l_Ptr.currnumerator [ 8 * 8 ]	; put normalized dividend here
				MOV				RDX, R8Home							; using callers dividend
				MOV				R8W, l_Ptr.normf					; get normalization factor
				CALL			shl_u								; the same number of bits that the divisor was shifted
; shift, even if within existing 8 qwords, may have increased dimension of dividend
				MOV				AX, l_Ptr.normf						; get normalization factor	
				ADD				AX, l_Ptr.mMSB						; add to msb of dividend
				MOV				CX, AX								; save total bit count for possible later shift
				SHR				AX, 6								; get new dimension of dividend
				MOV				l_Ptr.mDim, AX						; save new dimension of dividend

; Check: did we shift out msb bits of dividend? We shifted left normf bits, so if (original msb + normf) >= 512, we shifted out bits
; if so, need to increment dimension of dividend (already done above), and put shifted out bits into new msb qword of currnumerator (at inxes = 7)
; the shifted out bits are the high bits of the high word of the original dividend shifted left by normf bits

				CMP				CX, 511								; did we shift out bits?
				JLE				normdivdone							; no
				MOV				AX, CX
				AND				AX, 63								; get bit position within qword
				LEA				ECX, [ 63 ]
				SUB				CL, AL								; get count of bits to shift right to get shifted out bits
				MOV				RAX, R8Home
				MOV				RAX, Q_PTR [ RAX ]					; get most significant qword of original dividend
				SHR				RAX, CL								; shifted out bits now in low part of RAX
				MOV				l_Ptr.currnumerator [ 7 * 8 ], RAX	; put the shifted out bits at the 'front' of the currnumerator
normdivdone:														; putting low into new msb ninth word of currnumerator	

; We have normalized divisor and dividend, and set up dimensions of each
; The leading bit of the normalized divisor is in bit 63 of qword ( 7 - nDim ), thus the first qword of the normalized divisor is >= 0x8000000000000000

; The dividend is in currnumerator, and may be up to one qword longer than before normalization, with the leading bit in bit 62 of qword ( 15 - mDim ),
; thus the first qword of the normalized dividend is < 0x800000000000000

; Therefor, the first qword of the normalized dividend is always less than the first qword of the normalized divisor, and the first divide
; will be of the form (at most) {0x7FFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF} / 0x8000000000000000,
; yielding a qHat of 0x0FFFFFFFFFFFFFFF which fits in 64 bits.

; Progressing from most significant to least, as in the divide, indexes are started at the most significant word or (limit - dimension),
; and are incremented until reaching the limit.

; Progressing from least signicant to most, as in the multiply, add, and subtract - the index starts at the limit (i.e. 7),
; and is decremented until it reaches (limit - dimension) aka llimit.

; Main divide loop, initializxe 

				MOV				AX, l_Ptr.mDim
				SUB				AX, l_Ptr.nDim						; since nDim <= mDim, this will be from 0 (one qword) to 6 as mDim is 2->8, nDim 2->7
				MOV				l_Ptr.jDim, AX						; the Nr digits (QWORDS) of quotient is <= mDim - n?Dim + 1. Set jDim
				LEA				RCX, [ 7 ]
				SUB				CX, AX
				MOV				l_Ptr.jIdx, CX						; Initialize jIdx
				MOV				l_Ptr.jllimit, CX					; and lower limit (first, most significant QWORD index)

				LEA				RAX, [ 15 ]
				SUB				AX, l_Ptr.mDim
				MOV				l_Ptr.mIdx, AX						; initialize mIdx
				MOV				l_Ptr.mllimit, AX					; and lower limit (first, most significant QWORD index)

				LEA				RAX, [ 7 ]
				SUB				AX, l_Ptr.nDim
				MOV				l_Ptr.nIdx, AX						; initialize nIdx
				MOV				l_Ptr.nllimit, AX					; and lower limit (first, most significant QWORD index)

				MOV				RDX, l_Ptr.normdivisor [ RAX * 8 ]	; get indexed word of divisor (leading non-zero)
				MOV				l_Ptr.nDiv, RDX						; will be using repeatedly to determine qHat

				LEA				R15, [ 5 ]							; max adjustments to qHat (shouldnt be needed, but the code just looks like endless loop possible)

; mainloop, the loop, until jIDX reaches limit
maindivloop:

; compute qHat and rHat
				MOVZX			R8, l_Ptr.mIdx						; get mIdx. It is calculated from mDim, which in turn was adjusted for normalization
				DEC				R8									; mIdx - 1 for high qword of currnumerator for divide
				MOV				RDX, l_Ptr.currnumerator [ R8 * 8 ]	; the more significant qword of the 128bit dividend for divide
				MOV				RAX, l_Ptr.currnumerator + 8 [ R8 * 8 ]	; mIdx to get low qword of currnumerator for divide
		IF __DEBUG_DIVIDE_ESTIMATE__
		; Debug output of qHat estimate
		CMP RDX, l_Ptr.nDiv
		JB @ok
		INT 3  ; Or jump to error
		@ok:
		ENDIF ;__DEBUG_DIVIDE_ESTIMATE__
				DIV				l_Ptr.nDiv							; first qword of normalized divisor
				MOV				l_Ptr.qHat, RAX						; our "trial" digit of quotient
				MOV				l_Ptr.rHat, RDX

; Adjust qHat and rHat if necessary
checkqhat:
				MOVZX			R8, l_Ptr.nllimit					; get nllimit
				MOV				R10, l_Ptr.normdivisor + 8 [R8 * 8]	; get n second qword of normalized divisor
adjustqhat:
				MOV				RAX, l_Ptr.qHat
				MUL				R10									; times multiplicand -> RAX, RDX

				MOVZX			R8, l_Ptr.mIdx
				ADD				R8, 2                               ; mIdx + 2 for u[j+2]
				MOV				RCX, l_Ptr.currnumerator [R8 * 8]   ; u[j+2]
				MOV				R11, l_Ptr.rHat                     ; Load rHat for comparison
				CMP				RDX, R11                            ; Compare high part first
				JA				overestimate
				JB				qhatok
				; RDX == rHat, now compare low: RAX > u[j+2]?
				CMP				RAX, RCX
				JBE				qhatok
overestimate:
				DEC				R15                                 ; Adjustment counter
				JZ				divbyzero							; Too many (safety)
				DEC				l_Ptr.qHat							; Decrement qHat
				MOV				RDX, l_Ptr.rHat
				ADD				RDX, l_Ptr.nDiv						; add back nDiv to rHat
				JC				adjustqhat							; If carry (rHat overflow), re-test (rare)
				JMP				adjustqhat							; Re-MUL and test
qhatok:

; Multiply and subtract

				CALL			multiply_and_subtract				; multiply qHat * divisor, subtract from currnumerator
				JNC				no_addback							; if no borrow from subtract, skip add back

				LEA				R14, [ 3 ]							; max adjustments to add back (shouldnt be needed, but the code just looks like endless loop possible)
@addback:

; from multiply and subtract, have base addresses of currnumerator (R10) and subtracted product (R11), and length of add in R12
				MOV				R9, R12								; length of add
				CLC
@@:				MOV				RAX, [ R10 ][ R9 * 8 ]				; currnumerator [ idx ] -> RAX
				ADC				RAX, [ R11 ][ R9 * 8 ]				; qdiv [ idx ]
				MOV				[ R10 ][ R9 * 8 ], RAX				; store sum back to currnumerator
				DEC				R9
				JGE				@B
				DEC				l_Ptr.qHat							; decrement qHat
				DEC				R14									; adjustment counter
				JZ				divbyzero							; too many (safety)
				JC				@addback							; if borrow, need to add back again

no_addback:
; Store digit of quotient
				MOV				RAX, l_Ptr.qHat
				MOVZX			R8, l_Ptr.jIdx
				MOV				l_Ptr.quotient [ R8 * 8 ], RAX		; store qHat in quotient working copy
				; Increment indexes
				INC				l_Ptr.mIdx							; increment mIdx
				INC				l_Ptr.jIdx							; increment jIdx
				CMP				l_Ptr.jIdx, 7
				JLE				maindivloop							; loop until jDim > limit (7)

; Unnormalize remainder
				MOV				RCX, RDXHome						; put remainder at callers remainder
				LEA				RDX, l_Ptr.currnumerator [ 8 * 8 ]	; using working copy of currnumerator
				MOV				R8W, l_Ptr.normf					; get normalization factor
				CALL			shr_u								; shifting right to unnormalize
; Store quotient to callers area
				MOV				RCX, RCXHome						; callers quotient
				LEA				RDX, l_Ptr.quotient					; working copy of quotient
				Copy512			RCX, RDX							; copy quotient to callers area	
; Normal exit
cleanupret:
				XOR				RAX, RAX							; return zero

; Either fall-thru normal exit, or from exception handling
cleanupwretcode:			
				Local_Exit		RDI, R15, R14, R13, R12
; Flat exit if exception found before frame setup, or fall thru normal exit
@ret:
				RET

; Exception handling, divide by zero
divbyzero:
				LEA				EAX, [ retcode_neg_one ]
				JMP				cleanupwretcode

multiply_and_subtract:

; clear product work area
				XOR				RAX, RAX							
				LEA				RDI, l_Ptr.qdiv						; clear, every time, product of qHat * divisor (qdiv)				
				MOV				ECX, 16								; need to start as zero, as results are accumulated
				REP				STOSQ

; compute length of and starting point for multiply	(and subtract and add-back)
; Note: the first (most significant) word of the qhat * divisor product goes into qdiv [ (mIdx - 1) * 8 ]
; so the subtract (and add-back) is "lined up" with currnumerator [ (mIdx - 1) * 8 ]
; Three cases possible:
;		1. full length multiply of nDim + 1 words (divisor length) fits into qdiv starting at (mIdx - 1)
;		2. partial length multiply of nDim + 1 words (divisor length) minus the remaining lengtth of qdiv starting at (mIdx - 1)
;		3. partial length multiply of remaining words in qdiv starting at (mIdx - 1) fits into qdiv starting at (mIdx - 1)
;
; In case 1, length of multiply is nDim + 1, the qdiv starting point (least significant word, working toward most) is (mIdx + nDim)
; verfiy mIdx + nDim <= 15, the divisor starting point is 7.
; in case 2, the remaining length of qdiv is less than the length of the divisor, so length of multiply is remaining length of qdiv,
; and the start point for qdiv is 15, while the start point for divisor is (nDim - remaining length of qdiv + 1)
; in case 3, the remaining length of divisor is less than the remaining length of qdiv, so length of multiply is remaining length of divisor,
; and the start point for qdiv is (mIdx + remaining length of divisor - 1), while the start point for divisor is 7.
; 
; Netting all of that out: The base address of the destination for the product is always qdiv [ (mIdx - 1) * 8 ];
; The base address of the source (divisor) is the most significant of normdivisor [nllimit * 8 ]
; both are indexed starting at their bases plus the length of the multiply, indexed down from there by -1, down to zero
				MOVZX			R8, l_Ptr.mIdx						; calculate begining of where product will go (within qdiv)
				DEC				R8									; mIdx - 1 is where product starts
				LEA				R10, l_Ptr.qdiv [ R8 * 8 ]			;
				LEA				R9, [ 15 ]							; calculate remaining space in qdiv for product
				SUB				R9W, R8W							; (as divide loops, there is less space in qdiv)				
				MOVZX			R11, l_Ptr.nDim						; divisor length (does not change through loop)
				CMP				R11, R9
				CMOVLE			R9, R11								; remaining space is min( remaining qdiv, divisor )
				MOV				R12, R9								; save this length for later use
				MOVZX			R8, l_Ptr.nllimit					; index of divisor start
				LEA				R11, l_Ptr.normdivisor [ R8 * 8 ]	; base of divisor at nllimit

; at this point, have base addresses of product (R10) and divisor (R11) indexed to first qword of each,
; and length of multiply in R9 (decrement to zero)
; perform multiply of qHat * divisor with product into qdiv in qwords corresponding to currnumerator 
				MOV				R13, l_Ptr.qHat
@@:				MOV				RAX, [ R11 ] [ R9 * 8 ]				; multiplicand [ idx ] qword -> RAX
				MUL				R13									; times multiplier -> RAX, RDX
				ADD				[ R10 ][ 1 * 8][ R9 * 8 ], RAX		; add RAX to working product [ idx + 1 ] qword
				ADC				[ R10 ][ R9 * 8 ], RDX				; and add RDX with carry to [ idx ] qword of working product
				DEC				R9
				JGE				@B

; subtract product from currnumerator
				MOV				R9, R12								; length of subtract
				MOVZX			R8, l_Ptr.mIdx						; calculate begining of the current numerator 
				DEC				R8									; mIdx - 1 is where subtract starts
				LEA				R10, l_Ptr.currnumerator [ R8 * 8 ]	;
				LEA				R11, l_Ptr.qdiv [ R8 * 8 ]			; base of product to subtract

; base addresses of currnumerator (R10) and product (R11), length of subtract in R9 (decrement to zero)
				CLC
@@:				MOV				RAX, [ R10 ][ R9 * 8 ]				; currnumerator [ idx ] -> RAX
				SBB				RAX, [ R11 ][ R9 * 8 ]				; subtract product qdiv [ idx ]
				MOV				[ R10 ][ R9 * 8 ], RAX				; store difference back to currnumerator
				DEC				R9
				JGE				@B

; return with borrow flag in AX
				LEA				EAX, [ retcode_zero ]				; return zero
				CMOVC			EAX, ret_one 						; if borrow out, return 1
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