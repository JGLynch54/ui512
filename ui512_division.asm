;
;			ui512_division
; 
;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;
;			File:			ui512_division.asm
;			Author:			John G. Lynch
;			Legal:			Copyright @2026, per MIT License below
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
;
;		Regs with contents destroyed, not restored: RAX, RCX, RDX, R8, R9, R10, R11 (others are used and restored)
;
; 		Notes:			-	Algorithm is based on Knuths Algorithm D, with some modifications for efficiency and to avoid special cases. (e.g. one qword divide)
;							See Knuth Art of Computer Programming, Vol 2, section 4.3.1, pages 272-274.
;						-	All of the 512 vars are 8 QWORD arrays, or scalars that are used as 8 QWORD arrays, caller must ensure they are 64byte aligned
;						-	The 512 bit fields are "Big Endian" in the sense that the most significant qword is at the lowest address,
;							and the least significant qword is at the highest address, so that the leading bits of the field are in the first qword,
;							and trailing bits are in the last qword. This way, we can use the x86-64 instructions to operate on the low end of the field for efficiency,
;							and we can also use shifts to move bits between qwords as needed.

div_u_Locals	STRUCT
																	; Note: if qwords are added due to normalization, they are added at the beginning of the field, and the dimension is adjusted accordingly
currnumerator	QWORD			16 dup (?)							; scratch working copy of dividend (numerator). could be 9 qwords, 16 declared for alignment
qdiv			QWORD			16 dup (?)							; scratch working copy of (trial) qhat * divisor. could be 9 qwords, 16 declared for alignment

quotient		QWORD			8 dup (?)							; working copy of quotient
normdivisor		QWORD			8 dup (?)							; working copy of normalized divisor

nDiv1			QWORD			?									; first qword of normalized divisor
nDiv2			QWORD			?									; second qword of normalized divisor

qHat			QWORD			?									; trial quotient digit, referred to as qHat in Knuth
rHat			QWORD			?									; trial remainder referred to as rHat in Knuth

adjustcount		WORD			?									; counter for max adjustments to qHat
addbackcount	WORD			?									; counter for max number of add backs if overestimated qHat
									
mMSB			WORD			?									; indexes and dimensions of dividend (numerator) Note: dimensions are zero-based (0 to 7)
mDim			WORD			?
mIdx			WORD			?	
mllimit			WORD			?

nMSB			WORD			?									; indexes and dimensions of divisor (denominator)
nDim			WORD			?									
nIdx			WORD			?
nllimit			WORD			?

jDim			WORD			?									; indexes and dimensions of quotient
jIdx			WORD			?
jllimit			WORD			?

normf			WORD			?									; normalization factor (Nr bits shifted)
				WORD			2 dup (?)							; last padding to make struct size a multiple of 64 bytes

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
				MOV				RDI, RBP							; RBP points to stack reserved workspace "div_u_Locals"
				MOV				ECX, sizeof(div_u_Locals) / 8		; length in QWORDS, Note: structure must be multiple of 8 for this to work
				REP				STOSQ								; clear working area

; Examine divisor
; Note on msb_u: a returned zero means the most significant bit is bit0 of the eighth word of the 512bit source parameter; (the right most bit in the right most qword)
; a returned 511 means bit63 of the first word (the left most bit), a returned -1 means no bits set (the field is zero)
				MOV				RCX, R9								; address of divisor to RCX
				CALL			msb_u								; get Nr of most significant bit
				TEST			AX, AX								; 
				JL				divbyzero							; msb < 0?  -> divisor is zero, abort
				JE				divbyone							; msb == 0? -> divisor is one, exit with remainder = 0, quotient = dividend 
				CMP				AX, 64								; divisor only one 64-bit word?
				JGE				mbynDiv								; no, more than one qword divisor, do divide of m QWORD by n QWORDs, both >= 2 qwords

; Divide of m 64-bit qwords by one 64 bit qword divisor, use the quicker divide routine (div_uT64), and return
				MOV				RCX, RCXHome						; set up parms for call to div by 64bit: RCX - addr of quotient
				MOV				RDX, RDXHome						; RDX - addr of remainder
				MOV				R8, R8Home							; R8 - addr of dividend
				MOV				RAX, R9Home							; RAX - addr of divisor from caller
				MOV				R9, Q_PTR [ RAX ] [ 7 * 8 ]			; R9 - value of 64 bit divisor (de reference to get value)
				CALL			div_uT64							; call the divide by 64 bit routine
				MOV				RDX, RDXHome						; move 64 bit remainder to last word of 8 word callers remainder
				MOV				RCX, Q_PTR [ RDX ]					; get the one qword remainder
				MOV				Q_PTR [ RDX ], 0					; clear the first word, where div_uT64 put the remainder
				MOV				Q_PTR [ RDX ] [ 7 * 8 ], RCX		; and  put it in the least significant qword of the callers remainder
				JMP				cleanupret							; exit normally

; Divide an m digit (qword) dividend by an n digit (qword) divisor, both >= 2 qwords
mbynDiv:
				MOV				l_Ptr.nMSB, AX						; save msb of divisor
				SHR				AX, 6								; divide Nr bits by 64 to get qword count
				MOV				l_Ptr.nDim, AX						; Dimension (Nr Qwords) of divisor (n)

; examine dividend
				MOV				RCX, R8Home							; retrieve address of dividend
				CALL			msb_u								; get msb of dividend
				CMP				AX, l_Ptr.nMSB						; msb of dividend < msb of divisor? -> answer is zero with dividend going to remainder
				JL				numtoremain							;
				MOV				l_Ptr.mMSB, AX						; save msb of dividend
				SHR				AX, 6
				MOV				l_Ptr.mDim, AX						; save dimension (Nr Qwords) of dividend (m)

; So far: we have checked (and processed) edge cases (div by zero, div by one, num < denom)
; and we have m >= 2, n >= 2, and m >= n

; Normalize divisor (if needed), and copy to working area
				MOV				AX, l_Ptr.nMSB						; Nr bits in divisor
				AND				AX, 63								; masked down modulo 64 (within one qword)
				MOV				R8W, 63								; max bits in qword
				SUB				R8W, AX								; calculate shift count
				JNZ				@F									; if zero, no normalization needed (means highest bit already on)
				LEA				RCX, l_Ptr.normdivisor				; destination of normalized divisor
				MOV				RDX, R9Home							; using callers divisor
				Copy512			RCX, RDX							; copy divisor to normalized divisor working area
				LEA				RCX, l_Ptr.currnumerator [ 8 * 8 ]	; destination for working copy of numerator
				MOV				RDX, R8Home							; callers passed dividend
				Copy512			RCX, RDX							; copy it - no normalization needed, but still need to copy to working area
				JMP				normdivdone							; skip normalization (could go thruogh with zero bit shift, but copy faster this way)
@@:				MOV				l_Ptr.normf, R8W					; save normalization factor
				ADD				l_Ptr.nMSB, R8W						; new MSB of normalized divisor
				; Note: the shift is insufficient to shift out the msb bit, so dimension of divisor unchanged
				LEA				RCX, l_Ptr.normdivisor				; destination of normalized divisor
				MOV				RDX, R9Home							; using callers divisor
				CALL			shl_u								; shifting left so msb is in high bit position

; Normalize dividend aka numerator, or current numerator or currnumerator. Shift same Nr bits as divisor. Might increase dimension
				LEA				RCX, l_Ptr.currnumerator [ 8 * 8 ]	; put normalized dividend here
				MOV				RDX, R8Home							; using callers dividend
				MOV				R8W, l_Ptr.normf					; get normalization factor
				CALL			shl_u								; the same number of bits that the divisor was shifted
				; the shift, even if within existing 8 qwords, may have increased dimension of dividend
				MOV				AX, l_Ptr.normf						; get normalization factor	
				ADD				AX, l_Ptr.mMSB						; add to msb of dividend
				MOV				l_Ptr.mMSB, AX						; save new msb of dividend
				MOV				CX, AX								; save total bit count for possible later shift
				SHR				AX, 6								; get new dimension of dividend
				MOV				l_Ptr.mDim, AX						; save new dimension of dividend

; Check: did we shift out msb bits of dividend? We shifted left normf bits, so if (original msb + normf) >= 512, we shifted out bits
				CMP				CX, 511								; did we shift out bits?
				JLE				normdivdone							; no
				MOV				AX, CX
				AND				AX, 63								; get bit position within qword
				LEA				CX, [ 63 ]
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
; yielding a qHat of at most 0x0FFFFFFFFFFFFFFF which fits in 64 bits.

; Main divide loop, initialize. first v[n] related stuff
				LEA				RAX, [ 7 ]
				SUB				AX, l_Ptr.nDim
				MOV				l_Ptr.nIdx, AX						; initialize nIdx
				MOV				l_Ptr.nllimit, AX					; and lower limit (first, most significant QWORD index)
				MOV				RDX, l_Ptr.normdivisor [ RAX * 8 ]	; get indexed word of divisor (leading non-zero)
				MOV				l_Ptr.nDiv1, RDX					; will be using repeatedly to determine qHat
				MOV				RDX, l_Ptr.normdivisor + 8 [ RAX * 8 ]	; get next word of divisor (leading non-zero)
				MOV				l_Ptr.nDiv2, RDX					; will be using repeatedly to determine qHat

; next, u[m] related stuff.  verify u[mIdx] (currnumerator [ mIdx ]) < v[nIdx] (normdivisor [ nIdx ]), which should be true with proper normalization,
; but normalization may not have been necessary, leaving the first qword large. If so, need to add a new leading word (zero) for initial qHat calculation
				LEA				RAX, [ 15 ]
				SUB				AX, l_Ptr.mDim
				MOV				RDX, l_Ptr.currnumerator [ RAX * 8 ]	
				CMP				RDX, l_Ptr.nDiv1					; compare to leading qword of divisor, should be less (if equal, qHat will be 0xFFFFFFFFFFFFFFFF,
				JBE				@F
				INC				l_Ptr.mDim							; if not less, we need to adjust the dimension of the dividend up by one, and adjust indices and limits accordingly
				DEC				AX
@@:				MOV				l_Ptr.mIdx, AX						; initialize mIdx
				MOV				l_Ptr.mllimit, AX					; and lower limit (first, most significant QWORD index)

; finally, initialize j related stuff. The quotient is at most mDim - nDim + 1 qwords,
; and the first qword of the quotient is generated from the divide of the first mDim qwords of the dividend by the first nDim qwords of the divisor,
; so we set jIdx to start at the index corresponding to that first quotient word, which is mIdx + 1
				MOVZX			RAX, l_Ptr.mDim
				SUB				AX, l_Ptr.nDim						; since nDim <= mDim, this will be from 0 (one qword) to 6 as mDim is 2->8, nDim 2->7
				MOV				l_Ptr.jDim, AX						; the Nr digits (QWORDS) of quotient is <= mDim - nDim + 1. Set jDim
				LEA				CX, [ 7 ]
				SUB				CX, AX
				MOV				l_Ptr.jIdx, CX						; Initialize jIdx
				MOV				l_Ptr.jllimit, CX					; and lower limit (first, most significant QWORD index)

; At this point, we have normalized divisor and dividend, set up dimensions of each, and verified that the leading qword of the dividend is less than the leading qword of the divisor
; We are ready for the main divide loop, which will generate each digit (qword) of the quotient in turn, starting with the most significant digit (qHat), and working down to the least significant

; mainloop, the loop, until jIDX reaches limit
maindivloop:

; compute qHat and rHat
				MOVZX			R8, l_Ptr.mIdx						; get mIdx. It is calculated from mDim, which in turn was adjusted for normalization
				MOV				RDX, l_Ptr.currnumerator [ R8 * 8 ]	; the more significant qword of the 128bit dividend for divide
				MOV				RAX, l_Ptr.currnumerator + 8 [ R8 * 8 ]	; mIdx + 1 to get low qword of currnumerator for divide
				DIV				l_Ptr.nDiv1							; first qword of normalized divisor
				MOV				l_Ptr.qHat, RAX						; our "trial" digit of quotient. referred to as qHat in Knuth
				MOV				l_Ptr.rHat, RDX						; and remainder. referred to as rHat in Knuth

; Adjust qHat and rHat if necessary 
; **Note: comments on indexing differ from knuth, who numbers 1 descending, while we use from 0 ascending. Hence u[j-2] (in comments) is currnumerator [ mIdx + 2 ]
				LEA				R14W, [ 3 ]							; max adjustments per qhat generation (shouldnt be needed, but the code just looks like endless loop possible)
				MOV				l_Ptr.adjustcount, R14W				; save adjustment counter
checkqhat:
				MOV				RAX, l_Ptr.qHat						; qHat in RAX
				MUL				l_Ptr.nDiv2							; RDX:RAX is now (qhat * v[n-2])
				; Compare (rHat << 64) + u[j-2] to (qhat * v[n-2]). Note: both are 2 qwords (128 bits) (if > then qHat is too big, adjust
				CMP				RDX, l_Ptr.rHat						; Compare high part of (qhat * v[n-2]) with rhat
				JA				overestimate						; rHat*b + u[j-2] > qhat * v[n-2] on high word compare, so adjustment needed
				JB				qhatok								; rHat*b + u[j-2] < qhat * v[n-2] on high word compare, so no adjustment needed
				; if upper part compare says equal, compare low parts: u[j-2] > (qhat * v[n-2]) low part in RAX
				XOR				RCX, RCX							; value of u[j-2], default zero
				MOVZX			R8, l_Ptr.mIdx
				ADD				R8W, 2                              ; mIdx + 2 for u[j-2]
				CMP				R8W, 15						        ; limit of dividend index?
				; if beyond limit, there is no u[j-2], therefore compare to zero
				CMOVLE			RCX, l_Ptr.currnumerator [R8 * 8]   ; else get u[j-2]
				CMP				RAX, RCX							; compare low: (qhat * v[n-2]) > u[j-2]?
				JBE				qhatok
overestimate:
				DEC				l_Ptr.adjustcount                   ; Adjustment counter
				JZ				toomanyadjust						; Too many adjustments, something wrong (safety check only)
				DEC				l_Ptr.qHat							; Decrement qHat
				; Add back v[n-1] to rHat (rHat was too small, so we are effectively moving one 'unit' of divisor back from the quotient to the remainder)
				MOV				RCX, l_Ptr.nDiv1
				ADD				l_Ptr.rHat, RCX					
				JNC				checkqhat							; If carry (rHat overflow), re-test (rare)
qhatok:

; Multiply and subtract:  multiply qHat * divisor (n), subtract from currnumerator (at mIdx), and if borrow,
; add back divisor (adjust qHat down by one, and add back divisor to currnumerator), and repeat until no borrow. Note: in the rare case that we need to adjust qHat down,
; we might need to repeat this process, but it should not need to be repeated more than once, as we only overestimate qHat by at most one 'unit' of divisor.
	
; clear product (of qhat * normdivisor) work area (qdiv)
				XOR				RAX, RAX							
				LEA				RDI, l_Ptr.qdiv						; clear, every time, product of qHat * divisor (qdiv)				
				MOV				ECX, 16								; need values to start as zero, as results are accumulated
				REP				STOSQ

; compute length of and starting point for multiply	(and subtract and add-back)
				MOVZX			R8, l_Ptr.mIdx						; calculate begining of where product will go (within qdiv)
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
; and length of multiply in R9 (decrement to zero) perform multiply of qHat * divisor with product into qdiv in qwords corresponding to currnumerator 
				MOV				R13, l_Ptr.qHat
@@:				MOV				RAX, [ R11 ] [ R9 * 8 ]				; multiplicand [ idx ] qword -> RAX
				MUL				R13									; times multiplier -> RAX, RDX
				ADD				[ R10 ][ 1 * 8 ][ R9 * 8 ], RAX		; add RAX to working product [ idx + 1 ] qword
				ADC				[ R10 ][ R9 * 8 ], RDX				; and add RDX with carry to [ idx ] qword of working product
				DEC				R9
				JGE				@B

; done with multiply, initialize for subtracting the product from currnumerator
				MOV				R9, R12								; length of subtract
				MOVZX			R8, l_Ptr.mIdx						; calculate begining of the current numerator
				CMP				R8W, 15								; mIdx vs limit?
				JE				@F									; if equal, no offset needed
				INC				R9W									; and length of subtract
@@:				LEA				R10, l_Ptr.currnumerator [ R8 * 8 ]	;
				LEA				R11, l_Ptr.qdiv [ R8 * 8 ]			; base of product to subtract

; base addresses of currnumerator (R10) and product (R11), length of subtract in R9 (decrement to zero)
				CLC
@@:				MOV				RAX, [ R11 ][ R9 * 8 ]				; qdiv [ idx ] -> RAX
				SBB				[ R10 ][ R9 * 8 ],	RAX				; subtract product qdiv [ idx ] from currnumerator [ idx ]
				DEC				R9
				JGE				@B

; If borrow from subtract, need to add back divisor (product) once to correct qHat, and adjust qHat down by one.
; In the rare case that the adjusted qHat is still too large, may need to repeat this process,
; but it should not need to be repeated more than once,
; as we only overestimate qHat by at most one 'unit' of divisor. Note: if we needed to adjust qHat down,
; then we know that the current qHat * divisor was too large for the current dividend,
; therefore we know that the current dividend is less than the current product,
; therefore if we subtract the product from the dividend we will get a borrow,
; and if we add back the product to correct for the overestimate, we will get a carry (no borrow),
; so we can use the carry flag to determine whether we need to add back again.
				JNC				no_addback							; if no borrow from subtract, skip add back
				LEA				R14W, [ 3 ]							; max Nr of add back (shouldnt be needed, but the code just looks like endless loop possible)
				MOV				l_Ptr.addbackcount, R14W			; save addback count count
@addback:
				DEC				l_Ptr.addbackcount					; addback counter
				JZ				toomanyadjust						; too many (safety) Note: DEC doesnt affect the carry flag
; from multiply and subtract, have base addresses of currnumerator (R10) and subtracted product (R11), and length of add in R12
				MOV				R9, R12								; length of add
				MOVZX			R8, l_Ptr.mIdx						; calculate begining of the current numerator
				CMP				R8W, 15								; mIdx vs limit?
				JE				@F									; if equal, no offset needed
				INC				R9W									; and length of add
@@:				LEA				R10, l_Ptr.currnumerator [ R8 * 8 ]	;
				LEA				R11, l_Ptr.qdiv [ R8 * 8 ]			; base of product to add back
				CLC
@@:				MOV				RAX, [ R11 ][ R9 * 8 ]				; qdiv [ idx ]
				ADC				[ R10 ][ R9 * 8 ], RAX				; added (back) to currnumerator [idx]
				DEC				R9
				JGE				@B
				DEC				l_Ptr.qHat							; decrement qHat
				JC				@addback							; if borrow (carry), need to add back again
no_addback:

; At this point, have valid qHat for this digit of quotient. Store digit of quotient
				MOV				RAX, l_Ptr.qHat
				MOVZX			R8, l_Ptr.jIdx
				INC				R8W									; quotient index is mIdx + 1
				MOV				l_Ptr.quotient [ R8 * 8 ], RAX		; store qHat in quotient working copy

; Increment indices
				INC				l_Ptr.mIdx							; increment mIdx
				INC				l_Ptr.jIdx							; increment jIdx
				CMP				l_Ptr.jIdx, 7
				JL				maindivloop							; loop until jDim > limit (7)

; Unnormalize remainder, move it to caller space
				MOV				RCX, RDXHome						; put remainder at callers remainder
				LEA				RDX, l_Ptr.currnumerator [ 8 * 8 ]	; using working copy of currnumerator
				MOV				R8W, l_Ptr.normf					; get normalization factor
				CALL			shr_u								; shifting right to unnormalize

; Store quotient to callers space
				MOV				RCX, RCXHome						; callers quotient
				LEA				RDX, l_Ptr.quotient					; working copy of quotient
				Copy512			RCX, RDX							; copy quotient to callers area	

; Normal exit
cleanupret:		XOR				RAX, RAX							; return zero
cleanupwretcode:Local_Exit		RDI, R15, R14, R13, R12
@ret:			RET

; Exception handling, divide by zero
divbyzero:
				LEA				EAX, [ retcode_neg_one ]
				JMP				cleanupwretcode

; Exception handling, too many adjustments to qHat
toomanyadjust:
				LEA				EAX, [ 0707h ]
				JMP				cleanupwretcode

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
;			Regs with contents destroyed, not restored: RAX, RDX, R10 (each considered volatile).

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



