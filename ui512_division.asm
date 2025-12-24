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
				XOR				RAX, RAX
				LEA				RDI, l_Ptr.quotient					; clear working copy of contigous overflow/product, need to start as zero, results are accumulated
				MOV				ECX, 16 
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
				JMP				cleanupret
; Divide m digit by n digit
mbynDiv:
				MOV				l_Ptr.nMSB, AX						; save msb of divisor
				SHR				AX, 6
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