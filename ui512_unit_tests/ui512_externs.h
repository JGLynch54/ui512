#pragma once
#ifndef ui512_externs_h
#define ui512_externs_h

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
//
//		ui512_externs.h
// 
//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// 
//		File:			ui512_externs.h
//		Author:			John G.Lynch
//		Legal:			Copyright @2025, per MIT License below
//		Date:			October 24, 2025
//

#include "CommonTypeDefs.h"

extern "C"
{
	// Note:  Unless assembled with "__UseQ", all of the u64* arguments passed must be 64 byte aligned (alignas 64); GP fault will occur if not 
//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// 
//	from ui512_clear_copy_set.asm:

	// void zero_u ( u64* destarr ); 
	// fill supplied 512bit (8 QWORDS) with zero
	void zero_u(const u64*);

	// void copy_u ( u64* destarr, u64* srcarr );
	// copy supplied 512bit (8 QWORDS) source to supplied destination
	void copy_u(const u64*, const u64*);

	// void set_uT64 ( u64* destarr, u64 value );
	// set supplied destination 512 bit to supplied u64 value
	void set_uT64(const u64*, const u64);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// 
// 	 from ui512_compare.asm
// 
	// s16 compare_u ( u64* lh_op, u64* rh_op );
	// compare supplied 512bit (8 QWORDS) LH operand to supplied RH operand
	// returns: (0) for equal, -1 for less than, 1 for greater than (logical, unsigned compare)
	s16 compare_u(const u64*, const u64*);

	// s16 compare_uT64 ( u64* lh_op, u64 rh_op );
	// compare supplied 512bit (8 QWORDS) LH operand to supplied 64bit RH operand (value)
	// returns: (0) for equal, -1 for less than, 1 for greater than (logical, unsigned compare)
	s16 compare_uT64(const u64*, const u64);

	//--------------------------------------------------------------------------------------------------------------------------------------------------------------
	//
	//	from ui512_addition.asm
	//

	// s16 add_u ( u64* sum, u64* addend1, u64* addend2 );
	// add supplied 512bit (8 QWORDS) sources to supplied destination
	// returns: zero for no carry, 1 for carry (overflow)
	s16 add_u(const u64*, const u64*, const u64*);

	// s16 add_u_wc ( u64* sum, u64* addend1, u64* addend2, s16 carry_in );
	// add supplied 512bit (8 QWORDS) sources to supplied destination
	// returns: zero for no carry, 1 for carry (overflow)
	s16 add_u_wc(const u64*, const u64*, const u64*, s16);

	// s16 add_uT64 ( u64* sum, u64* addend1, u64 addend2 );
	// add 64bit QWORD (value) to supplied 512bit (8 QWORDS), place in supplied destination
	// returns: zero for no carry, 1 for carry (overflow)
	s16 add_uT64(const u64*, const u64*, const u64);

	//--------------------------------------------------------------------------------------------------------------------------------------------------------------
	//
	//	from ui512_subtraction.asm
	//

	// s16 sub_u ( u64* difference, u64* left operand, u64* right operand );
	// subtract supplied 512bit (8 QWORDS) RH OP from LH OP giving difference in destination
	// returns: zero for no borrow, 1 for borrow (underflow)
	s16 sub_u(const u64*, const u64*, const u64*);

	// s16 sub_u_wb ( u64* difference, u64* left operand, u64* right operand, s16 borrow );
	// subtract supplied 512bit (8 QWORDS) RH OP from LH OP, with passed-in borrow giving difference in destination
	// returns: zero for no borrow, 1 for borrow (underflow)
	s16 sub_u_wb(const u64*, const u64*, const u64*, const s16);

	// s16 sub_uT64( u64* difference, u64* left operand, u64 right operand );
	// subtract supplied 64 bit right hand (64 bit value) op from left hand (512 bit) giving difference
	// returns: zero for no borrow, 1 for borrow (underflow)
	s16 sub_uT64(const u64*, const u64*, const u64);

	//--------------------------------------------------------------------------------------------------------------------------------------------------------------
	//
	//	from ui512_multiply.asm
	//

	//	EXTERNDEF	mult_uT64 : PROC
	//	mult_uT64	multiply 512 bit multiplicand by 64 bit multiplier, giving 512 product, 64 bit overflow
	//	Prototype:	s16 mult_uT64 ( u64 * product, u64 * overflow, u64 * multiplicand, u64 multiplier );
	s16 mult_uT64(const u64*, const u64*, const u64*, const u64);

	//	EXTERNDEF	mult_u : PROC
	//	mult_u		multiply 512 multiplicand by 512 multiplier, giving 512 product, overflow
	//	Prototype:	s16 mult_u ( u64 * product, u64 * overflow, u64 * multiplicand, u64 * multiplier );
	s16 mult_u(const u64*, const u64*, const u64*, const u64*);

	//--------------------------------------------------------------------------------------------------------------------------------------------------------------
	//
	//	from ui512_divide.asm
	//

	//	EXTERNDEF	div_uT64 : PROC
	//	div_uT64	divide 512 bit dividend by 64 bit divisor, giving 512 bit quotient and 64 bit remainder
	//	Prototype:	s16 div_uT64 ( u64 * quotient, u64 * remainder, u64 * dividend, u64 divisor );
	s16 div_uT64(const u64*, const u64*, const u64*, const u64);

	//	EXTERNDEF	div_u : PROC
	//	div_u		divide 512 bit dividend by 512 bit divisor, giving 512 bit quotient and remainder
	//	Prototype:	s16 div_u ( u64 * quotient, u64 * remainder, u64 * dividend, u64 * divisor );
	s16 div_u(const u64*, const u64*, const u64*, const u64*);

	//--------------------------------------------------------------------------------------------------------------------------------------------------------------
	//
	//	from ui512_significance.asm
	//

	// EXTERNDEF	msb_u : PROC
	// find most significant bit in supplied source 512bit (8 QWORDS)
	// returns: -1 if no most significant bit, bit number otherwise, bits numbered 0 to 511 inclusive	
	s16 msb_u(const u64*);

	// EXTERNDEF	lsb_u : PROC
	// find least significant bit in supplied source 512bit (8 QWORDS)
	// returns: -1 if no least significant bit, bit number otherwise, bits numbered 0 to 511 inclusive
	s16 lsb_u(const u64*);

	//--------------------------------------------------------------------------------------------------------------------------------------------------------------
	//
	//	from ui512_shift.asm
	//
	
	// void shr_u ( u64* destination, u64* source, u32 bits_to_shift );
	// shift supplied source 512bit (8 QWORDS) right, put in destination
	// EXTERNDEF	shr_u : PROC
	void shr_u(const u64*, const u64*, const u16);

	// void shl_u ( u64* destination, u64* source, u16 bits_to_shift );
	// shift supplied source 512bit (8 QWORDS) left, put in destination
	// EXTERNDEF	shl_u : PROC
	void shl_u(const u64*, const u64*, const u16);

	//--------------------------------------------------------------------------------------------------------------------------------------------------------------
	//
	//	from ui512_bitops.asm
	//

	// void and_u ( u64* destination, u64* lh_op, u64* rh_op );
	// logical 'AND' bits in lh_op, rh_op, put result in destination
	// EXTERNDEF	and_u : PROC
	void and_u(const u64*, const u64*, const u64*);

	// void or_u ( u64* destination, u64* lh_op, u64* rh_op );
	// logical 'OR' bits in lh_op, rh_op, put result in destination	
	// EXTERNDEF	or_u : PROC
	void or_u(const u64*, const u64*, const u64*);

	// void xor_u ( u64* destination, u64* lh_op, u64* rh_op );
	// logical 'XOR' bits in lh_op, rh_op, put result in destination	
	// EXTERNDEF	xor_u : PROC
	void xor_u(const u64*, const u64*, const u64*);

	// void not_u ( u64* destination, u64* source );
	// logical 'NOT' bits in source, put result in destination
	// EXTERNDEF	not_u : PROC
	void not_u(const u64*, const u64*);

	//--------------------------------------------------------------------------------------------------------------------------------------------------------------
	//
	//	optional function for debugging
	//

	// void reg_verify(u64* regs);
	// reg_verify - copy non-volatile regs into callers struct of nine qwords; intended for unit tests to verify non-volatile regs are not changed
	void reg_verify(const u64*);

};

#endif	//ui512_externs_h