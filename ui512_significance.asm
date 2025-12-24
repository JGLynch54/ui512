;
;			ui512_significance
;
;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;
;			File:			ui512_significaance.asm
;			Author:			John G. Lynch
;			Legal:			Copyright @2025, per MIT License below
;			Date:			November 19, 2025  (file creation)

				INCLUDE			ui512_legalnotes.inc
				INCLUDE			ui512_compile_time_options.inc
				INCLUDE			ui512_macros.inc
				INCLUDE			ui512_externs.inc
.NOLISTIF
				OPTION			CASEMAP:NONE
ui512_significance SEGMENT		PARA 'CODE'

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;			msb_u		-	find most significant bit in supplied source 512bit (8 QWORDS)
;			Prototype:		s16 msb_u( u64* source );
;			source		-	Address of 64 byte aligned array of 8 64-bit words (QWORDS) 512 bits (in RCX)
;			returns		-	-1 if no most significant bit, bit number otherwise, bits numbered 0 to 511 inclusive
;			Note:	a returned zero means the significant bit is bit0 of the eighth word of the 512bit source parameter; (the right most bit)
;					a returned 511 means bit63 of the first word (the left most bit)

				Leaf_Entry		msb_u							; Declare public proc, no prolog, no frame, exceptions handled by caller
				CheckAlign		RCX								; (IN) source to scan 

	IF __UseZ
				VMOVDQA64		ZMM31, ZM_PTR [RCX]				; Load source 
				VPTESTMQ		k1, ZMM31, ZMM31				; find non-zero words (if any)
				KMOVB			EAX, k1							; ZMM regs in least significant word to most ([0] lsw to [7] msw)
				TZCNT			ECX, EAX						; determine index of word from last (trailing) non-zero bit in mask
				JNC				@F								; all words zero?
				LEA				EAX, [ retcode_neg_one ]		; exit with -1 if all eight qwords are zero (no significant bit)
				RET

@@:				LEA				EAX, [ 7 ]						; numbering of words in Z regs (and hence in k1 mask) is reverse in significance order
				SUB				EAX, ECX						; so 7 minus leading k bit index becomes index to our ui512 bit qword
				SHL				EAX, 6							; convert index to offset
				VPCOMPRESSQ		ZMM0 {k1}{z}, ZMM31				; compress it into first word of ZMM0, which is also XMM0
				VMOVQ			RCX, XMM0						; extract the non-zero word (k1 still has index to it)
				LZCNT			RCX, RCX						; get the index of the non-zero bit within the word
				ADD				EAX, 63							; LZCNT counts leading non-zero bits. Subtract from 63 to get our bit index
				SUB				EAX, ECX						; Word index * 64 + bit index becomes bit index to first non-zero bit (0 to 511, where )

	ELSE
				LEA				R10, [ -1 ]						; Initialize loop counter (and index)
@@NextWord:
				INC				R10D
				CMP				R10D, 8
				JNZ				@F								; Loop through values 0 to 7, then exit
				LEA				EAX,  [ retcode_neg_one ]
				RET

@@:				LZCNT			R11, Q_PTR [ RCX ] [ R10 * 8 ]	; Leading zero count to find significant bit for index 
				JC				@@NextWord						; None found, loop to next word
				LEA				EAX, [ 7 ]
				SUB				EAX, R10D						; calculate seven minus the word index (which word has the msb?)
				SHL				EAX, 6							; times 64 for each word
				LEA				ECX, [ 63 ]
				SUB				ECX, R11D
				ADD				EAX, ECX						; plus the found bit position within the word yields the bit position within the 512 bit source

	ENDIF
				RET
msb_u			ENDP

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;			lsb_u		-	find least significant bit in supplied source 512bit (8 QWORDS)
;			Prototype:		s16 lsb_u( u64* source );
;			source		-	Address of 64 byte aligned array of 8 64-bit words (QWORDS) 512 bits (in RCX)
;			returns		-	-1 if no least significant bit, bit number otherwise, bits numbered 0 to 511 inclusive
;			Note:	a returned zero means the significant bit is bit0 of the eighth word of the 512bit source parameter; (the right most bit)
;					a returned 511 means bit63 of the first word (the left most bit)

				Leaf_Entry		lsb_u							; Declare public proc, no prolog, no frame, exceptions handled by caller
				CheckAlign		RCX								; (IN) source to scan

	IF __UseZ
				VMOVDQA64		ZMM31, ZM_PTR [ RCX ]			; Load source 
				VPTESTMQ		k1, ZMM31, ZMM31				; find non-zero words (if any)
				KMOVB			EAX, k1
				LZCNT			R10D, EAX						; ZMM regs in least significant word to most ([0] lsw to [7] msw)
				JNC				@F
				LEA				EAX, [ retcode_neg_one ]		; exit with -1 if all eight qwords are zero (no significant bit)
				RET

@@:				AND				R10, 7							; mask out all but 0 -> 7
				LEA				EAX, [ 7 ]						; numbering of words in Z regs (and hence in k1 mask) is reverse in significance order
				SUB				EAX, R10D						; so 7 minus leading k bit index becomes index to our ui512 bit qword
				XOR				R9D, R9D
				INC				R9D
				MOV				CL, AL
				SHL				R9D, CL
				KMOVB			k1, R9D
				VPCOMPRESSQ		ZMM0 {k1}{z}, ZMM31
				VMOVQ			RAX, XMM0						; extract the non-zero word
				SHL				R10D, 6							; convert index to offset
				TZCNT			RAX, RAX						; get the index of the non-zero bit within the word
				ADD				EAX, R10D						; Word index * 64 + bit index becomes bit index to first non-zero bit (0 to 511, where )
				RET
	ELSE
				LEA				R10D, [ 8 ]		 				; Initialize loop counter (and index)
@@NextWord:
				DEC				R10D
				CMP				R10D, -1
				JNE				@F								; Loop through values 7 to 0, then exit
				LEA				EAX, [ retcode_neg_one ]
				RET

@@:				TZCNT			RAX, Q_PTR [ RCX ] [ R10 * 8 ]	; Scan indexed word for significant bit
				JC				@@NextWord						; None found, loop to next word
				LEA				R11D, [ 7 ]						;  
				SUB				R11D, R10D						; calculate seven minus the word index (which word has the msb?)
				SHL				R11D, 6							; times 64 for each word
				ADD				EAX, R11D						; plus the BSF found bit position within the word yields the bit position within the 512 bit source
				RET

	ENDIF
lsb_u			ENDP
ui512_significance ENDS											; end of section
				END												; end of module
