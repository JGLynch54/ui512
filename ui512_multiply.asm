;
;			ui512_multiply
;
;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;
;			File:			ui512_multiply.asm
;			Author:			John G. Lynch
;			Legal:			Copyright @2024, per MIT License below
;			Date:			October 29, 2025

				INCLUDE			ui512_legalnotes.inc
				INCLUDE			ui512_compile_time_options.inc
				INCLUDE			ui512_macros.inc
				INCLUDE			ui512_externs.inc
.NOLISTIF
				OPTION			CASEMAP:NONE
ui512_multiply	SEGMENT			PARA 'CODE'

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;			EXTERNDEF		mult_u:PROC					; s16 mult_u( u64* product, u64* overflow, u64* multiplicand, u64* multiplier)
;			mult_u			-	multiply 512 multiplicand by 512 multiplier, giving 512 product, 512 overflow
;			Prototype:		-	s16 mult_u( u64* product, u64* overflow, u64* multiplicand, u64* multiplier);
;			product			-	Address of 8 QWORDS to store resulting product (in RCX)
;			overflow		-	Address of 8 QWORDS to store resulting overflow (in RDX)
;			multiplicand	-	Address of 8 QWORDS multiplicand (in R8)
;			multiplier		-	Address of 8 QWORDS multiplier (in R9)
;			returns			-	(0) for success, (GP_Fault) for mis-aligned parameter address
;
				
mult_u_Locals	STRUCT
product			QWORD			16 dup (?)
mult_u_Locals	ENDS

; Declare proc, save regs, set up frame
				Proc_w_Local	mult_u, mult_u_Locals, R12, R13, R14, R15
				MOV				RCXHome, RCX
				MOV				RDXHome, RDX

; Check passed parameters alignment, since this is checked within frame, need to specify exit / cleanup / unwrap label
				CheckAlign		RCX, @@exit							; (out) Product
				CheckAlign		RDX, @@exit							; (out) Overflow
				CheckAlign		R8, @@exit							; (in) Multiplicand
				CheckAlign		R9, @@exit							; (in) Multiplier

; Examine multiplicand, save dimensions, handle edge cases of zero or one
				MOV				RCX, R8								; examine multiplicand
				CALL			msb_u								; get count to most significant bit (-1 if no bits)
				TEST			EAX, EAX								
				JL				@@zeroandexit						; msb < 0? multiplicand = 0; exit with product = 0
				LEA				RDX, [ R9 ]							; multiplicand = 1?	exit with product = multiplier -> address of multiplier (to be copied to product)
				JE				@@copyandexit						; msb = 0 means lowest bit, or multiplicand == 1 -> copy multiplier to product and exit
				SHR				EAX, 6								; divide msb by 64 to get Nr words
				LEA				R14D, [ 7 ]							; subtract from 7 to get starting (high order, left-most) beginning index
				SUB				R14D, EAX							; save in scratch reg (R14) as multiplicand index lower limit (eliminate multiplying leading zero words)	

; Examine multiplier, save dimensions, handle edge cases of zero or one
				MOV				RCX, R9								; examine multiplier
				CALL			msb_u								; get count to most significant bit (-1 if no bits)
				TEST			EAX, EAX							; 
				JL				@@zeroandexit						; return -1? means multiplier == 0 -> exit with product = 0
				LEA				RDX, [ R8 ]							; 
				JE				@@copyandexit						; multiplier = 1? exit with product = multiplicand -> address of multiplicand (to be copied to product)
				SHR				EAX, 6								; divide msb by 64 to get Nr words
				LEA				R15D, [ 7 ]							; subtract from 7 to get starting (high order, left-most) beginning index
				SUB				R15D, EAX							; save off multiplier index lower limit (eliminate multiplying leading zero words)	(R15)					

; In frame / stack reserved memory, clear 16 qword area for working version of overflow/product; set up indexes for loop				
	IF __UseQ
				VPXORQ			ZMM31, ZMM31, ZMM31
				VMOVDQA64		ZM_PTR l_Ptr.product, ZMM31
				VMOVDQA64		ZM_PTR l_Ptr.product + [ 8 * 8 ], ZMM31
	ELSE
				XCHG			RDI, R10
				XOR				RAX, RAX
				LEA				RDI, l_Ptr.product					; clear working copy of contigous overflow/product, need to start as zero, results are accumulated
				MOV				ECX, 16 
				REP				STOSQ
				XCHG			RDI, R10
	ENDIF
				LEA				R11	, [ 7 ] 						; index for multiplier (reduced until less than saved multiplier lower limit (R15W) (outer loop)
				MOV				R12, R11							; index for multiplicand (reduced until less than saved multiplicand lower limit (R14W) (inner loop)

; multiply loop: an outer loop for each non-leading-zero qword of multiplicand,
; with an inner loop for each non-leading-zero qword of multiplier, results accumulated in 'overflow/product' local copy 
				ALIGN												; start (both inner and outer) loop aligned
@@multloop:		LEA				R10,  1 [ R11 ] [ R12]				; R10 now holds index for overflow / product work area (results)
				MOV				RAX, Q_PTR [ R8 ] [ R12 * 8 ]		; get qword of multiplicand
				MUL				Q_PTR [ R9 ] [ R11 * 8 ]			; multiply by qword of multiplier
				ADD				l_Ptr.product [ R10 * 8 ], RAX			; accummulate in product [R10], this is low-order 64 bits of result of mul
				DEC				R10									; index for overflow / product area, decrement and preserve carry flag
@@:
				ADC				l_Ptr.product [ R10 * 8 ], RDX		; high-order result of 64bit multiply, plus the carry (if any)
				JNC				@F									; if adding caused carry, propagate it, else next 
				LEA				RDX, [ 0 ]							; if propagating, add zero plus carry. preserve carry flag
				DEC				R10									; propagating carry
				JGE				@B
@@:																	
				DEC				R12D								; index to next qword of multiplicand
				CMP				R12D, R14D							; Done with inner loop? R14W has multiplicand index lower limit 
				JGE				@@multloop							; no -> do it again
				LEA				R12D, [ 7 ]							; yes, reset inner loop (multiplicand) index
				DEC				R11D								; decrement index for outer loop
				CMP				R11D, R15D							; done with outer loop? R15W has multiplier index lower limit
				JGE				@@multloop							; no, do it again with next qword of multiplier

; finished: copy working product/overflow to callers product/overflow
				MOV				RCX, RCXHome						; parameter passed as addr of callers product
				LEA				RDX, l_Ptr.product [ 8 * 8 ]
				Copy512			RCX, RDX							; copy working product to callers product
				MOV				RCX, RDXHome						; parameter passed as addr of callers overflow
				LEA				RDX, l_Ptr.product [ 0 ]
				Copy512			RCX, RDX							; copy working overflow to callers overflow

; restore regs, release frame, return
@@exit:			XOR				RAX, RAX							; return zero
				Local_Exit		R15, R14, R13, R12				

; multiplying by 0: zero callers product and overflow
@@zeroandexit:	MOV				RCX, RCXHome						; reload address of callers product
				Zero512			RCX									; zero it
				MOV				RCX, RDXHome						; reload address of caller overflow
				Zero512			RCX									; zero it
				JMP				@@exit

; multiplying by 1: zero overflow, copy the non-one (multiplier or multiplicand) to the product
@@copyandexit:	MOV				RCX, RDXHome						; address of passed overflow
				Zero512			RCX 								; zero it
				MOV				RCX, RCXHome						; copy (whichever: multiplier or multiplicand) to callers product
				Copy512			RCX, RDX							; RDX "passed" here from the jump here (either &multiplier, or &multiplicand in RDX)
				JMP				@@exit								; and exit				
mult_u			ENDP				

;
;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;			EXTERNDEF		mult_uT64:PROC				;	s16 mult_uT64( u64* product, u64* overflow, u64* multiplicand, u64 multiplier);
;			mult_uT64		-	multiply 512 bit multiplicand by 64 bit multiplier, giving 512 product, 64 bit overflow
;			Prototype:		-	s16 mult_uT64( u64* product, u64* overflow, u64* multiplicand, u64 multiplier);
;			product			-	Address of 8 QWORDS to store resulting product (in RCX)
;			overflow		-	Address of QWORD for resulting overflow (in RDX)
;			multiplicand	-	Address of 8 QWORDS multiplicand (in R8)
;			multiplier		-	multiplier QWORD (in R9)
;			returns			-	(0) for success, (GP_Fault) for mis-aligned parameter address

; Declare structure of local variables (will become part of stack-based frame) (thread-safe)
mult64_Locals	STRUCT
multiplicand	QWORD			8 dup (?)
mult64_Locals	ENDS

; Declare proc, save regs, set up frame
				Proc_w_Local	mult_uT64, mult64_Locals

; Check passed parameters alignment, since this is checked within frame, need to specify exit / cleanup / unwrap label
				CheckAlign		RCX, @@exit							; (out) Product
				CheckAlign		R8, @@exit							; (in) Multiplicand

; caller might be doing multiply 'in-place', so need to save the original multiplicand, prior to clearing callers product (A = A * x), or (A *= x)
				LEA				R10, l_Ptr.multiplicand
				Copy512			R10, R8

; clear callers product and overflow
;	Note: if caller used multiplicand and product as the same variable (memory space),
;	this would wipe the multiplicand. Hence the saving of the multiplicand on the stack. (above)
				XOR				RAX, RAX
				Zero512			RCX		   							; clear callers product (multiply uses an addition with carry, so it needs to start zeroed)
				MOV				Q_PTR [ RDX ], RAX					; clear callers overflow
 				MOV				R10, RDX							; RDX (pointer to callers overflow) gets used in the MUL: save it in R10 (a volatile reg)

; FOR EACH index of 7 thru 1 (omiting 0): fetch qword of multiplicand, multiply, add 128 bit result (RAX, RDX) to running working product
				FOR				idx, < 7, 6, 5, 4, 3, 2, 1 >		; Note: this is not a 'real' for statement, this is a macro that generates an unwound loop
				MOV				RAX, l_Ptr.multiplicand + [ idx * 8 ] ; multiplicand [ idx ] qword -> RAX
				MUL				R9									; times multiplier -> RAX, RDX
				ADD				Q_PTR [ RCX ] [ idx * 8 ], RAX		; add RAX to working product [ idx ] qword
				ADC				Q_PTR [ RCX ] [ (idx - 1) * 8 ], RDX ; and add RDX with carry to [ idx - 1 ] qword of working product
				ENDM

; Most significant (idx=0), the high order result of the multiply in RDX, goes to the overflow of the caller
				MOV				RAX, l_Ptr.multiplicand
				MUL				R9
				ADD				Q_PTR [ RCX ] [ 0 * 8 ], RAX
				ADC				Q_PTR [ R10 ], RDX					; last qword overflow is also the operation overflow
				XOR				RAX, RAX							; return zero
@@exit:			Local_Exit

mult_uT64		ENDP		
ui512_multiply	ENDS
				END													; end of module