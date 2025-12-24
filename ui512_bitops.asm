;
;			ui512_bitops
;
;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;
;			File:			ui512_bitops.asm
;			Author:			John G. Lynch
;			Legal:			Copyright @2025, per MIT License below
;			Date:			November 19, 2025  (file creation)

				INCLUDE			ui512_legalnotes.inc
				INCLUDE			ui512_compile_time_options.inc
				INCLUDE			ui512_macros.inc
				INCLUDE			ui512_externs.inc
.NOLISTIF
				OPTION			CASEMAP:NONE
ui512_bitops	SEGMENT			PARA 'CODE'

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;			and_u		-	logical 'AND' bits in lh_op, rh_op, put result in destination
;			Prototype:		void and_u( u64* destination, u64* lh_op, u64* rh_op);
;			destination	-	Address of 64 byte aligned array of 8 64-bit words (QWORDS) 512 bits (in RCX)
;			lh_op		-	Address of 64 byte aligned array of 8 64-bit words (QWORDS) 512 bits (in RDX)
;			rh_op		-	Address of 64 byte aligned array of 8 64-bit words (QWORDS) 512 bits (in R8)
;			returns		-	nothing (0)

				Leaf_Entry		and_u
				CheckAlign		RCX
				CheckAlign		RDX
				CheckAlign		R8

	IF __UseZ	
				VMOVDQA64		ZMM31, ZM_PTR [ RDX ]			; load lh_op	
				VPANDQ			ZMM31, ZMM31, ZM_PTR [ R8 ]		; 'AND' with rh_op
				VMOVDQA64		ZM_PTR [ RCX ], ZMM31			; store at destination address

	ELSEIF __UseY
				VMOVDQA64		YMM4, YM_PTR [ RDX + 0 * 8 ]
				VPANDQ			YMM5, YMM4, YM_PTR [ R8 + 0 * 8 ]
				VMOVDQA64		YM_PTR [ RCX + 0 * 8 ], YMM5
				VMOVDQA64		YMM2, YM_PTR [ RDX + 4 * 8 ]
				VPANDQ			YMM3, YMM2, YM_PTR [ R8 + 4 * 8]
				VMOVDQA64		YM_PTR [ RCX + 4 * 8 ], YMM3

	ELSEIF __UseX
				MOVDQA			XMM4, XM_PTR [ RDX + 0 * 8 ]
				PAND			XMM4, XM_PTR [ R8 + 0 * 8]
				MOVDQA			XM_PTR [ RCX + 0 * 8 ], XMM4
				MOVDQA			XMM5, XM_PTR [ RDX + 2 * 8 ]
				PAND			XMM5, XM_PTR [ R8 + 2 * 8]
				MOVDQA			XM_PTR [ RCX + 2 * 8 ], XMM5
				MOVDQA			XMM4, XM_PTR [ RDX + 4 * 8 ]
				PAND			XMM4, XM_PTR [ R8 + 4 * 8]
				MOVDQA			XM_PTR [ RCX + 4 * 8 ], XMM4
				MOVDQA			XMM5, XM_PTR [ RDX + 6 * 8 ]
				PAND			XMM5, XM_PTR [ R8 + 6 * 8]
				MOVDQA			XM_PTR [ RCX + 6 * 8 ], XMM5

	ELSE
; This looks like a runtime loop, but it generates (at compile time) and unwound repeated set of instructions
				FOR				idx, < 0, 1, 2, 3, 4, 5, 6, 7 >	
				MOV				RAX, Q_PTR [ RDX ] [ idx * 8 ]
				AND				RAX, Q_PTR [ R8 ] [ idx * 8 ]
				MOV				Q_PTR [ RCX ] [ idx * 8 ], RAX
				ENDM

	ENDIF
				RET	
and_u			ENDP

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;			or_u		-	logical 'OR' bits in lh_op, rh_op, put result in destination
;			Prototype:		void or_u( u64* destination, u64* lh_op, u64* rh_op);
;			destination	-	Address of 64 byte aligned array of 8 64-bit words (QWORDS) 512 bits (in RCX)
;			lh_op		-	Address of 64 byte aligned array of 8 64-bit words (QWORDS) 512 bits (in RDX)
;			rh_op		-	Address of 64 byte aligned array of 8 64-bit words (QWORDS) 512 bits (in R8)
;			returns		-	nothing (0)

				Leaf_Entry		or_u
				CheckAlign		RCX
				CheckAlign		RDX
				CheckAlign		R8

	IF __UseZ	
				VMOVDQA64		ZMM31, ZM_PTR [ RDX ]			
				VPORQ			ZMM31, ZMM31, ZM_PTR [ R8 ]
				VMOVDQA64		ZM_PTR [ RCX ], ZMM31

	ELSEIF __UseY
				VMOVDQA64		YMM4, YM_PTR [ RDX + 0 * 8 ]
				VPORQ			YMM5, YMM4, YM_PTR [ R8 + 0 * 8 ]
				VMOVDQA64		YM_PTR [ RCX + 0 * 8 ], YMM5
				VMOVDQA64		YMM2, YM_PTR [ RDX + 4 * 8 ]
				VPORQ			YMM3, YMM2, YM_PTR [ R8 + 4 * 8]
				VMOVDQA64		YM_PTR [ RCX + 4 * 8 ], YMM3

	ELSEIF __UseX
				MOVDQA			XMM4, XM_PTR [ RDX + 0 * 8 ]
				POR				XMM4, XM_PTR [ R8 + 0 * 8]
				MOVDQA			XM_PTR [ RCX + 0 * 8 ], XMM4
				MOVDQA			XMM5, XM_PTR [ RDX + 2 * 8 ]
				POR				XMM5, XM_PTR [ R8 + 2 * 8]
				MOVDQA			XM_PTR [ RCX + 2 * 8 ], XMM5
				MOVDQA			XMM4, XM_PTR [ RDX + 4 * 8 ]
				POR				XMM4, XM_PTR [ R8 + 4 * 8]
				MOVDQA			XM_PTR [ RCX + 4 * 8 ], XMM4
				MOVDQA			XMM5, XM_PTR [ RDX + 6 * 8 ]
				POR				XMM5, XM_PTR [ R8 + 6 * 8]
				MOVDQA			XM_PTR [ RCX + 6 * 8 ], XMM5

	ELSE
				FOR				idx, < 0, 1, 2, 3, 4, 5, 6, 7 >
				MOV				RAX, Q_PTR [ RDX ] [ idx * 8 ]	; get qword from callers lh_op
				OR				RAX,  Q_PTR [ R8 ] [ idx * 8 ]	; "OR" woth qword from callers RH_op
				MOV				Q_PTR [ RCX ] [ idx * 8 ], RAX	; store at callers destination
				ENDM

	ENDIF
				RET 
or_u			ENDP

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;			xor_u		-	logical 'XOR' bits in lh_op, rh_op, put result in destination
;			Prototype:		void xor_u( u64* destination, u64* lh_op, u64* rh_op);
;			destination	-	Address of 64 byte aligned array of 8 64-bit words (QWORDS) 512 bits (in RCX)
;			lh_op		-	Address of 64 byte aligned array of 8 64-bit words (QWORDS) 512 bits (in RDX)
;			rh_op		-	Address of 64 byte aligned array of 8 64-bit words (QWORDS) 512 bits (in R8)
;			returns		-	nothing (0)

				Leaf_Entry		xor_u
				CheckAlign		RCX
				CheckAlign		RDX
				CheckAlign		R8

	IF __UseZ	
				VMOVDQA64		ZMM31, ZM_PTR [ RDX ]			
				VPXORQ			ZMM31, ZMM31, ZM_PTR [ R8 ]
				VMOVDQA64		ZM_PTR [ RCX ], ZMM31

	ELSEIF __UseY
				VMOVDQA64		YMM4, YM_PTR [ RDX + 0 * 8 ]
				VPXORQ			YMM5, YMM4, YM_PTR [ R8 + 0 * 8 ]
				VMOVDQA64		YM_PTR [ RCX + 0 * 8 ], YMM5
				VMOVDQA64		YMM2, YM_PTR [ RDX + 4 * 8 ]
				VPXORQ			YMM3, YMM2, YM_PTR [ R8 + 4 * 8]
				VMOVDQA64		YM_PTR [ RCX + 4 * 8 ], YMM3

	ELSEIF __UseX
				MOVDQA			XMM4, XM_PTR [ RDX + 0 * 8 ]
				PXOR			XMM4, XM_PTR [ R8 + 0 * 8]
				MOVDQA			XM_PTR [ RCX + 0 * 8 ], XMM4
				MOVDQA			XMM5, XM_PTR [ RDX + 2 * 8 ]
				PXOR			XMM5, XM_PTR [ R8 + 2 * 8]
				MOVDQA			XM_PTR [ RCX + 2 * 8 ], XMM5
				MOVDQA			XMM4, XM_PTR [ RDX + 4 * 8 ]
				PXOR			XMM4, XM_PTR [ R8 + 4 * 8]
				MOVDQA			XM_PTR [ RCX + 4 * 8 ], XMM4
				MOVDQA			XMM5, XM_PTR [ RDX + 6 * 8 ]
				PXOR			XMM5, XM_PTR [ R8 + 6 * 8]
				MOVDQA			XM_PTR [ RCX + 6 * 8 ], XMM5

	ELSE
				FOR				idx, < 0, 1, 2, 3, 4, 5, 6, 7 >
				MOV				RAX, Q_PTR [ RDX ] [ idx * 8 ]
				XOR				RAX,  Q_PTR [ R8 ] [ idx * 8 ]
				MOV				Q_PTR [ RCX ] [ idx * 8 ], RAX
				ENDM

	ENDIF
				RET 
xor_u			ENDP

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;			not_u		-	logical 'NOT' bits in source, put result in destination
;			Prototype:		void not_u( u64* destination, u64* source);
;			destination	-	Address of 64 byte aligned array of 8 64-bit words (QWORDS) 512 bits (in RCX)
;			source		-	Address of 64 byte aligned array of 8 64-bit words (QWORDS) 512 bits (in RDX)
;			returns		-	nothing (0)

				Leaf_Entry		not_u
				CheckAlign		RCX
				CheckAlign		RDX

	IF __UseZ	
				VMOVDQA64		ZMM31, ZM_PTR [RDX]			
				VPANDNQ			ZMM31, ZMM31, qOnes					; qOnes (declared in the data section of this module) is 8 QWORDS, binary all ones
				VMOVDQA64		ZM_PTR [RCX], ZMM31

	ELSEIF __UseY
				VMOVDQA64		YMM4, YM_PTR [ RDX + 0 * 8 ]
				VPANDNQ			YMM5, YMM4, qOnes
				VMOVDQA64		YM_PTR [ RCX + 0 * 8 ], YMM5
				VMOVDQA64		YMM4, YM_PTR [ RDX + 4 * 8 ]
				VPANDNQ			YMM5, YMM4, qOnes
				VMOVDQA64		YM_PTR [ RCX + 4 * 8 ], YMM5

	ELSEIF __UseX
				MOVDQA			XMM4, XM_PTR [ RDX + 0 * 8 ]
				PANDN			XMM4, XM_PTR qOnes
				MOVDQA			XM_PTR [ RCX + 0 * 8 ], XMM4
				MOVDQA			XMM4, XM_PTR [ RDX + 2 * 8 ]
				PANDN			XMM4, XM_PTR qOnes
				MOVDQA			XM_PTR [ RCX + 2 * 8 ], XMM4
				MOVDQA			XMM4, XM_PTR [ RDX + 4 * 8 ]
				PANDN			XMM4, XM_PTR qOnes
				MOVDQA			XM_PTR [ RCX + 4 * 8 ], XMM4
				MOVDQA			XMM4, XM_PTR [ RDX + 6 * 8 ]
				PANDN			XMM4, XM_PTR qOnes
				MOVDQA			XM_PTR [ RCX + 6 * 8 ], XMM4

	ELSE
				FOR				idx, < 0, 1, 2, 3, 4, 5, 6, 7 >
				MOV				RAX, Q_PTR [ RDX ] [ idx * 8 ]
				NOT				RAX
				MOV				Q_PTR [ RCX ] [ idx * 8 ], RAX
				ENDM

	ENDIF
				RET	

not_u			ENDP
ui512_bitops	ENDS												; end of section
				END													; end of module