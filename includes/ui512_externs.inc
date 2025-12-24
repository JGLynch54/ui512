;
;			ui512 Externs
;
;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;
;			File:			ui512_externs.inc
;			Author:			John G. Lynch
;			Legal:			Copyright @2025, per MIT License included
;			Date:			October 24, 2025
;
IFNDEF							ui512_externs_INC
ui512_externs_INC				EQU		<1>

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
;   header file equivalent extern declarations
;			EXTERN "C" signatures 

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
; from ui512_clear_copy_set.asm:
;
;	// void zero_u ( u64* destarr ); 
;	// fill supplied 512bit (8 QWORDS) with zero
EXTERNDEF		zero_u:PROC

;	// void copy_u ( u64* destarr, u64* srcarr );
;	// copy supplied 512bit (8 QWORDS) source to supplied destination
EXTERNDEF		copy_u:PROC

;	// void set_uT64 ( u64* destarr, u64 value );
;	// set supplied destination 512 bit to supplied u64 value
EXTERNDEF		set_uT64:PROC

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
; from ui512_compare.asm
;
;	// s16 compare_u ( u64* lh_op, u64* rh_op );
;	// compare supplied 512bit (8 QWORDS) LH operand to supplied RH operand
;	// returns: (0) for equal, -1 for less than, 1 for greater than (logical, unsigned compare)
EXTERNDEF		compare_u:PROC

;	// s16 compare_uT64 ( u64* lh_op, u64 rh_op );
;	// compare supplied 512bit (8 QWORDS) LH operand to supplied 64bit RH operand (value)
;	// returns: (0) for equal, -1 for less than, 1 for greater than (logical, unsigned compare)
EXTERNDEF		compare_uT64:PROC

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
; from ui512_addition.asm
;
;	// s16 add_u ( u64* sum, u64* addend1, u64* addend2 );
;	// add supplied 512bit (8 QWORDS) sources, place in supplied destination
;	// returns: zero for no carry, 1 for carry (overflow)
EXTERNDEF		add_u:PROC

;	// s16 add_u_wc ( u64* sum, u64* addend1, u64* addend2, s16 carry );
;	// add supplied 512bit (8 QWORDS) sources, with passed-in carry, place in supplied destination
;	// returns: zero for no carry, 1 for carry (overflow)
EXTERNDEF		add_u_wc:PROC

;	// s16 add_uT64 ( u64* sum, u64* addend1, u64 addend2 );
;	// add 64bit QWORD (value) to supplied 512bit (8 QWORDS), place in supplied destination
;	// returns: zero for no carry, 1 for carry (overflow)
EXTERNDEF		add_uT64:PROC

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
; from ui512_subtraction.asm
;
;	// s16 sub_u ( u64* difference, u64* left operand, u64* right operand );
;	// subtract supplied 512bit (8 QWORDS) RH OP from LH OP giving difference in destination
;	// returns: zero for no borrow, 1 for borrow (underflow)
EXTERNDEF		sub_u:PROC

;	// s16 sub_u_wc ( u64* difference, u64* left operand, u64* right operand, s16 borrow );
;	// subtract supplied 512bit (8 QWORDS) RH OP from LH OP, with passed-in borrow giving difference in destination
;	// returns: zero for no borrow, 1 for borrow (underflow)
EXTERNDEF		sub_u_wb:PROC

;	// s16 sub_uT64( u64* difference, u64* left operand, u64 right operand );
;	// subtract supplied 64 bit right hand (64 bit value) op from left hand (512 bit) giving difference
;	// returns: zero for no borrow, 1 for borrow (underflow)
EXTERNDEF		sub_uT64:PROC

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
; from ui512_multiply.asm
;
; //			mult_uT64		-	multiply 512 bit multiplicand by 64 bit multiplier, giving 512 product, 64 bit overflow
; //			Prototype:		-	s16 mult_uT64( u64* product, u64* overflow, u64* multiplicand, u64 multiplier);
EXTERNDEF		mult_uT64:PROC	;	s16 mult_uT64( u64* product, u64* overflow, u64* multiplicand, u64 multiplier);

; //			mult_u			-	multiply 512 multiplicand by 512 multiplier, giving 512 product, overflow
; //			Prototype:		-	s16 mult_u( u64* product, u64* overflow, u64* multiplicand, u64* multiplier);
EXTERNDEF		mult_u:PROC		;	s16 mult_u( u64* product, u64* overflow, u64* multiplicand, u64* multiplier);

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
; from ui512_division.asm
;
; //			div_uT64		-	divide 512 bit dividend by 64 bit bit divisor, giving 512 bit quotient and 64 bit remainder
; //			Prototype:		-	s16 div_u( u64* quotient, u64* remainder, u64* dividend, u64 divisor,);
EXTERNDEF		div_uT64:PROC	;	s16 div_uT64( u64* quotient, u64* remainder, u64* dividend, u64 divisor);

; //			div_u			-	divide 512 bit dividend by 512 bit divisor, giving 512 bit quotient and remainder
; //			Prototype:		-	s16 div_u( u64* quotient, u64* remainder, u64* dividend, u64* divisor);
EXTERNDEF		div_u:PROC		;	s16 div_u( u64* quotient, u64* remainder, u64* dividend, u64* divisor);

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
; from ui512_significance.asm

;   // find most significant bit in supplied source 512bit (8 QWORDS)
;	// s16 msb_u( u64* );
;   // returns: -1 if no most significant bit, bit number otherwise, bits numbered 0 to 511 inclusive
;	//	Note:	a returned zero means the significant bit is bit0 of the eighth word of the 512bit source parameter; (the right most bit).
;	//			a returned 511 means bit63 of the first word; (the left most bit).	
EXTERNDEF		msb_u:PROC

;   // find least significant bit in supplied source 512bit (8 QWORDS)
;	// s16 lsb_u( u64* );
;   // returns: -1 if no least significant bit, bit number otherwise, bits numbered 0 to 511 inclusive
;	//	Note:	a returned zero means the significant bit is bit0 of the eighth word of the 512bit source parameter; (the right most bit).
;	//			a returned 511 means bit63 of the first word; (the left most bit).	
EXTERNDEF		lsb_u:PROC

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
; from ui512_shift.asm
;
;   // void shr_u ( u64* destination, u64* source, u16 bits_to_shift )
;   // shift supplied source 512bit (8 QWORDS) right, put in destination
EXTERNDEF		shr_u:PROC

;   // void shl_u ( u64* destination, u64* source, u16 bits_to_shift );
;   // shift supplied source 512bit (8 QWORDS) left, put in destination
EXTERNDEF		shl_u:PROC

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
; from ui512_bitops
;
;   // void and_u ( u64* destination, u64* lh_op, u64* rh_op );
;   // logical 'AND' bits in lh_op, rh_op, put result in destination
EXTERNDEF		and_u:PROC

;   // logical 'OR' bits in lh_op, rh_op, put result in destination
;   // void or_u( u64* destination, u64* lh_op, u64* rh_op);
EXTERNDEF		or_u:PROC

;   // logical 'XOR' bits in lh_op, rh_op, put result in destination
;   // void xor_u( u64* destination, u64* lh_op, u64* rh_op);
EXTERNDEF		xor_u:PROC

;   // logical 'NOT' bits in source, put result in destination
;	// void not_u( u64* destination, u64* source);
EXTERNDEF		not_u:PROC

;--------------------------------------------------------------------------------------------------------------------------------------------------------------
; from ui512_global_data
;
EXTERNDEF		qOnes:QWORD
EXTERNDEF		qZero:QWORD

EXTERNDEF		ret_zero:DWORD
EXTERNDEF		ret_one:DWORD
EXTERNDEF		ret_neg_one:DWORD
EXTERNDEF		ret_GPFault:DWORD

EXTERNDEF		mskB0:DB
EXTERNDEF		mskB1:DB
EXTERNDEF		mskB2:DB
EXTERNDEF		mskB3:DB
EXTERNDEF		mskB4:DB
EXTERNDEF		mskB5:DB
EXTERNDEF		mskB6:DB
EXTERNDEF		mskB7:DB
EXTERNDEF		mskAll8:DB

ENDIF			; ui512_externs_INC

