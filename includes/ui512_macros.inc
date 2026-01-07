;
;			ui512 Macros
;
;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;
;			File:			ui512_macros.inc
;			Author:			John G. Lynch
;			Legal:			Copyright @2025, per MIT License included
;			Date:			October 24, 2025  (file creation)
;
IFNDEF							ui512_macros_INC
ui512_macros_INC				EQU		<1>

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;
; Some coding shortcuts
;
ZM_PTR			EQU				ZMMWORD PTR 
YM_PTR			EQU				YMMWORD PTR
XM_PTR			EQU				XMMWORD PTR
Q_PTR			EQU				QWORD PTR
D_PTR			EQU				DWORD PTR
W_PTR			EQU				WORD PTR
B_PTR			EQU				BYTE PTR
m32BCST			EQU				DWORD BCST
m64BCST			EQU				QWORD BCST
LPVOID			TYPEDEF			PTR VOID

; Mask codes (for compares using instructions like VPCMPUQ)
CPEQ			EQU				0
CPLT			EQU				1
CPLE			EQU				2
CPFALSE			EQU				3
CPNE			EQU				4
CPGE			EQU				5
CPGT			EQU				6
CPTRUE			EQU				7

; Masks commonly used: here in Record form, which can be used as immediate values. Example: OR	R8D, MASK kMask.b8
kMask			RECORD			b8:1, b7:1, b6:1, b5:1, b4:1, b3:1, b2:1, b1:1, b0:1

; Return codes commonly used.	
retcode_zero	EQU				0
retcode_one		EQU				1
retcode_neg_one	EQU				-1

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;           Notes on x64 calling conventions        specifically "fast call"
; ref: https://learn.microsoft.com/en-us/cpp/build/x64-calling-convention?view=msvc-170
; The callers first four parameters are passed in registers: RCX, RDX, R8, R9 if integer or address; if floating point XMM0L, XMM1L, XMM2L, XMM3L
; return (if any) is in EAX
;
; RAX, RCX, RDX, R8, R9, R10, R11 are considered volatile, and do not need to be saved
; XMM0, YMM0, ZMM0 and  ..1, ..2, ..3, ..4, and ..5 are considered volatile, and do not need to be saved
; ZMM16 to ZMM31: volatile, also do not need to be zeroed to resume full clock speeds
;
; R12, R13, R14, R15, RDI, RSI, RBX, RBP, RSP are non-volatile and if used, must be restored
; XMM, YMM, and ZMM ..6 thru 15 are non-volatile and if used, must be restored
;
; A "leaf" function is one that does not call and does not change non volatile registers
; leaf functionss therefore do not need frame, prolog or epilog
;

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
; Selected macros from "macamd64.inc" (c) Microsoft Corporation
;	These macros generate .xdata and .pdata entries in the executable image file.
;	The entries assist in exception and debugging; helping 'unwind' operations.
;	Only a few macros are included, and are reformatted to match coding style:;	indents and capitalization.
;
;--------------------------------------------------------------------------------------------------------------------------------------------------------------
; Leaf_Entry <Name>
;
; Macro Description:
;   This macro indicates the beginning of a leaf function.;
;
; Arguments:
;   Name - Supplies the name of the function
;   
Leaf_Entry		MACRO			Name
				DB				6 DUP (0cch)
				ALIGN			16
				PUBLIC			Name
Name			PROC
; Addresses of param regs "home" after reg pushs, and _allocspace adjust
RCXHome			EQU				Q_PTR [ RSP + ( 1 * 8 ) ]	
RDXHome			EQU				Q_PTR [ RSP + ( 2 * 8 ) ]
R8Home			EQU				Q_PTR [ RSP + ( 3 * 8 ) ]
R9Home			EQU				Q_PTR [ RSP + ( 4 * 8 ) ]
				MOV				RCXHome, RCX					; in 'fastcall' home param space is reserved, but regs not saved, so save at least RCX
				ENDM
;--------------------------------------------------------------------------------------------------------------------------------------------------------------
; Proc_w_Local	ProcName, LocalStructName, NV_Regs_Used:vararg
;
Proc_w_Local	MACRO			ProcName, LocalStructName, NV_Regs_Used:vararg

_shadowspace	EQU				40h								; stack space for caller (at top) and callee (at bottom)
_localspace		=				sizeof (LocalStructName)		; space needed (on stack) for local variables
_alignspace		EQU				64								; amount of space that might get truncated to get frame space aligned on specified boundary (64)
_allocspace		=				_localspace + ( 2 * _shadowspace) + _alignspace	; calculate total stack space to allocate.

				DB				6 DUP (0cch)
				ALIGN			16
				PUBLIC			ProcName						; Declare code section, public proc, no prolog, no frame, exceptions handled by caller
ProcName		PROC			FRAME				
				MOV				[ RSP + ( 1 * 8) ], RCX			; in 'fastcall' home param space is reserved, but regs not saved, so save at least RCX
				push_reg        RBP								; standard prologue code

	nRegs = 1
				FOR				reg, <NV_Regs_Used>				; save specified non-volatile regs on stack (in top shadowspace)
				push_reg		reg
	nRegs = nRegs + 1
				ENDM

				SUB				RSP, _allocspace				; set stack pointer down by space needed (locals+ (2 * shadowspace))
				.ALLOCSTACK		_allocspace
	
; set frame pointer within new space, but above shadowspace
				LEA				RBP, ( _shadowspace + _alignspace ) [ RSP ]
				AND				RBP, -_alignspace				; truncate low bits of base pointer to yield aligned pointer

				.ENDPROLOG
; Addresses of param regs "home" after reg pushs, and _allocspace adjust
RCXHome			EQU				Q_PTR  [ RSP + ( _allocspace + ( ( nRegs + 1 ) * 8 ) ) ]
RDXHome			EQU				Q_PTR  [ RSP + ( _allocspace + ( ( nRegs + 2 ) * 8 ) ) ] 
R8Home			EQU				Q_PTR  [ RSP + ( _allocspace + ( ( nRegs + 3 ) * 8 ) ) ]
R9Home			EQU				Q_PTR  [ RSP + ( _allocspace + ( ( nRegs + 4 ) * 8 ) ) ]
l_Ptr			EQU				[RBP] + LocalStructName			; set up shortcut for accessing local data structure
				ENDM

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
; Local_Exit	NV_Regs_Used:vararg
;
Local_Exit		MACRO			NV_Regs_Used:vararg
				ADD				RSP,  _allocspace
				FOR				reg, <NV_Regs_Used>
				POP				reg
				ENDM 
				POP				RBP
				RET
				ENDM

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
; push_reg	reg (non-volatile) to push
;
push_reg		MACRO			reg
				PUSH			reg
				.PUSHREG		reg
				ENDM

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
; CheckAlign <RAddr>
;
;			Test passed variable addresses for 64 byte alignment
;			Note: Better performance if this is off, but for debugging, maybe have it on
;
CheckAlign		MACRO			Raddr:REQ, ExitLbl:VARARG
	IF	__CheckAlign
				TEST			Raddr, 63							; Is specified param aligned 64?
				JZ				@F									; Yes, passes test, continue
				MOV				EAX, 0C0000005h						; Windows code for General Protection Fault	
		IFNB	<ExitLbl>
				JMP				ExitLbl								; Return to caller, with GPFault code in EAX
		ELSE
				RET													; No, fault return with err code
		ENDIF
@@:
	ENDIF
				ENDM


;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;			Zero a 512 bit destination, conditional assembly based on configuration parameters
;
Zero512			MACRO			dest:REQ
	IF		__UseZ
				CheckAlign		dest
				VPXORQ			ZMM31, ZMM31, ZMM31
				VMOVDQA64		ZM_PTR [ dest ], ZMM31
	ELSEIF	__UseY
				CheckAlign		dest
				VPXORQ			YMM4, YMM4, YMM4
				FOR				idx, < 0, 4 >
				VMOVDQA64		YM_PTR [ dest ] [ idx * 8 ], YMM4
				ENDM
	ELSEIF	__UseX
				CheckAlign		dest
				PXOR			XMM4, XMM4
				FOR				idx, < 0, 2, 4, 6 >
				MOVDQA			XM_PTR [ dest ] [ idx * 8 ], XMM4
				ENDM		
	ELSE
				XOR				RAX, RAX
				FOR				idx,  < 0, 1, 2, 3, 4, 5, 6, 7 >
				MOV				Q_PTR [ dest ] [ idx * 8 ], RAX
				ENDM
	ENDIF
				ENDM

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;			Zero a 512 bit destination, always use Q_PTR, avoids clock penalty from using SIMD
;
Zero512Q		MACRO			dest:REQ
				XOR				RAX, RAX
				FOR				idx,  < 0, 1, 2, 3, 4, 5, 6, 7 >
				MOV				Q_PTR [ dest ] [ idx * 8 ], RAX
				ENDM

				ENDM

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;			Copy a 512 bit source to destination, conditional assembly based on configuration parameters
;
Copy512			MACRO			dest:REQ, src:REQ
	IF		__UseZ 
				CheckAlign		dest
				CheckAlign		src
				VMOVDQA64		ZMM31, ZM_PTR [ src ]
				VMOVDQA64		ZM_PTR [ dest ], ZMM31
	ELSEIF	__UseY
				CheckAlign		dest
				CheckAlign		src
				VMOVDQA64		YMM4, YM_PTR [ src + 0 * 8 ]
				VMOVDQA64		YM_PTR [ dest ] [ 0 * 8 ], YMM4	; alternate ymm regs in case pipeline can execute next without waiting for this.
				VMOVDQA64		YMM5, YM_PTR [ src ] [ 4 * 8 ]
				VMOVDQA64		YM_PTR [ dest ] [ 4 * 8 ], YMM5
	ELSEIF	__UseX
				CheckAlign		dest
				CheckAlign		src
				MOVDQA			XMM4, XM_PTR [ src ] [ 0 * 8 ]
				MOVDQA			XM_PTR [ dest ] [ 0 * 8 ], XMM4
				MOVDQA			XMM3, XM_PTR [ src ] [ 2 * 8 ]
				MOVDQA			XM_PTR [ dest ] [ 2 * 8 ], XMM3
				MOVDQA			XMM4, XM_PTR [ src ] [ 4 * 8 ]
				MOVDQA			XM_PTR [ dest ] [ 4 * 8 ], XMM4
				MOVDQA			XMM3, XM_PTR [ src ] [ 6 * 8 ]
				MOVDQA			XM_PTR [ dest ] [ 6 * 8 ], XMM3
	ELSE
				FOR				idx, < 0, 1, 2, 3, 4, 5, 6, 7 >
				MOV				RAX, Q_PTR [ src ] [ idx * 8 ]
				MOV				Q_PTR [ dest ] [ idx * 8 ], RAX
				ENDM
	ENDIF
				ENDM

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;			Copy a 512 bit source to destination, always use Q_PTR, avoids clock penalty from using SIMD
;
Copy512Q		MACRO			dest:REQ, src:REQ
				FOR				idx, < 0, 1, 2, 3, 4, 5, 6, 7 >
				MOV				RAX, Q_PTR [ src ] [ idx * 8 ]
				MOV				Q_PTR [ dest ] [ idx * 8 ], RAX
				ENDM

				ENDM

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;	VerifyRegs <none>
;
;			If option is on, generate a function, callable by unit test routines, 
;				to save non-volatile registers in passed structure.
;
IF	__VerifyRegs
;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;			EXTERNDEF		reg_verify:PROC	;	void reg_verify ( u64* regstruct)
;			reg_verify		-	copy non-volatile regs into callers struct of nine qwords) intended for unit tests to verify non-volatile regs are not changed
;			Prototype:		-	void reg_verify( uu64* regstruct);
;			regstruct		-	Address of 9 QWORDS in a struct where regs will be copied (in RCX)
; //			reg_verify		-	save non-volatile regs for verification (debug)
; //			Prototype		-	void reg_verify ( u64* reg struct)

EXTERNDEF		reg_verify:PROC	;	void reg_verify ( u64* reg struct)

VerifyRegs		MACRO
				Leaf_Entry		reg_verify
				MOV				Q_PTR [ RCX ] [ 0 * 8 ], R12
				MOV				Q_PTR [ RCX ] [ 1 * 8 ], R13
				MOV				Q_PTR [ RCX ] [ 2 * 8 ], R14
				MOV				Q_PTR [ RCX ] [ 3 * 8 ], R15
				MOV				Q_PTR [ RCX ] [ 4 * 8 ], RDI
				MOV				Q_PTR [ RCX ] [ 5 * 8 ], RSI
				MOV				Q_PTR [ RCX ] [ 6 * 8 ], RBX
				MOV				Q_PTR [ RCX ] [ 7 * 8 ], RBP
				MOV				Q_PTR [ RCX ] [ 8 * 8 ], RSP
				RET
reg_verify		ENDP
				ENDM
ENDIF

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;			Get a GP reg QWORD from within a Z register as specified by mask
;			Note: RAX, ZMM0 and k1 are used and not restored
;			Example usage: GetZatIdx R11, ZMM1, MaskBit2 or SetZatIdx ZMM1, R12, [ R9 ]  (where R9 is a bit mask, not an integer index)
;			Note: These are req to reg ops; no memory fetches (other than instructions from pipeline)
;
GetZatMask		MACRO			dest, src, mask
				LEA				RAX,  mask
				KMOVB			k1, RAX
				VPCOMPRESSQ		ZMM0 {k1}{z}, src
				VMOVQ			dest, XMM0
				ENDM

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;			Set a GP Reg QWORD within a Z register as specified by mask
;			Note: RAX and k1 are used and not restored
;			Example usage: SetZatIdx ZMM1, R8, MaskBit2
;			Note: These are req to reg ops; no memory fetches (other than instructions from pipeline)
;
SetZatMask		MACRO			dest, src, mask
				LEA				RAX, mask
				KMOVB			k1, RAX
				VPBROADCASTQ 	dest {k1}, src
				ENDM

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
ENDIF			; ui512_macros_INC
