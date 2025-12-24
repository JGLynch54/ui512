;
;			ui512_clear_copy_set
;
;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;
;			File:			ui512_clear_copy_set.asm
;			Author:			John G. Lynch
;			Legal:			Copyright @2024, per MIT License below
;			Date:			October 29, 2025

				INCLUDE			ui512_legalnotes.inc
				INCLUDE			ui512_compile_time_options.inc
				INCLUDE			ui512_macros.inc
				INCLUDE			ui512_externs.inc
.NOLISTIF
				OPTION			CASEMAP:NONE

IF				__VerifyRegs	
Verify_Regs		SEGMENT			PARA 'CODE'				
				VerifyRegs
Verify_Regs		ENDS
ENDIF

clear_copy_set	SEGMENT			PARA 'CODE'

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;			zero_u		-	fill supplied 512bit (8 QWORDS) with zero
;			Prototype:		extern "C" void zero_u ( u64* destarr );
;			destarr		-	Address of destination 64 byte aligned array of 8 64-bit words (QWORDS) 512 bits (in RCX)
;			returns		-	nothing
;
				Leaf_Entry		zero_u				
				Zero512			RCX									; Zero 512 bit space addressed in RCX (the parameter)
				RET	
zero_u			ENDP

;
;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;			copy_u		-	copy supplied 512bit (8 QWORDS) source to supplied destination
;			Prototype:		extern "C" void copy_u( u64* destarr, u64* srcarr )
;			destarr		-	Address of destination 64 byte aligned array of 8 64-bit words (QWORDS) 512 bits (in RCX)
;			srcarr		-	Address of source 64 byte aligned array of 8 64-bit QWORDS (512 bits) in RDX
;			returns		-	nothing
;
				Leaf_Entry		copy_u
				Copy512			RCX, RDX							; Copy 512 bit space from src array (address in RDX) to dest (RCX)
				RET
copy_u			ENDP

;
;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;			set_uT64	-	set supplied destination 512 bit to supplied u64 value
;			Prototype:		extern "C" void set_uT64( u64* destarr, u64 value )
;			destarr		-	Address of destination 64 byte aligned array of 8 64-bit words (QWORDS) 512 bits (in RCX)
;			src			-	u64 value in RDX
;			returns		-	nothing
;
				Leaf_Entry		set_uT64
				CheckAlign		RCX									; (OUT) destination array to be set

	IF __UseZ
				KMOVB			k1, B_PTR [ mskB7 ]					; Mask for least significant word (Note: ZMM regs store least first, while mem stores least last. Beware.)
				VPBROADCASTQ 	ZMM31 {k1}{z}, RDX					; load parameter, zeroing all other qwords
				VMOVDQA64		ZM_PTR [ RCX ], ZMM31				; store at destination

	ELSE
				Zero512			RCX									; Zero destination array	
				MOV				Q_PTR [ RCX ] [ 7 * 8 ], RDX		; Move given 64 bit qword to least significant qword of array

	ENDIF
				RET	
set_uT64		ENDP
clear_copy_set	ENDS												; end of section
				END													; end of module