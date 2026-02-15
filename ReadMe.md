Project Description

	ui512 is a small project to provide basic operations for a variable type of unsigned 512 bit integer.

	It is written in assembly language, using the MASM (ml64) assembler provided as an option within Visual Studio.	(currently using VS Community 2026 18.3.0)
	It provides external signatures that allow linkage to C and C++ programs, where a shell/wrapper could encapsulate the methods as part of an object.
	It has assembly time options directing the use of Intel processor extensions: AVX4, AVX2, SIMD, or none:
	(Z (512), Y (256), or X (128) registers, or regular Q (64bit)).
	If processor extensions are used, the caller must align the variables declared and passed on the appropriate byte boundary (e.g. alignas 64 for 512)
	This module is very light-weight (less than 20K bytes) and relatively fast,	but is not intended for all processor types or all environments. 
	
	Intended use cases:
		1.) a "sum of primes" for primes up to 2^48. 
		2.) elliptical curve cryptography (ECC)

	The modules provide functions:
			mult_u			multiply 512 multiplicand by 512 multiplier, giving 512 product,512 overflow
			mult_uT64		multiply 512 bit multiplicand by 64 bit multiplier, giving 512 product, 64 bit overflow
			div_u			divide 512 bit dividend by 512 bit divisor, giving 512 bit quotient and remainder
			div_uT64		divide 512 bit dividend by 64 bit divisor, giving 512 bit quotient and 64 bit remainder
			add_u			add 512 bit addend to 512 bit augend, giving 512 bit sum and carry
			sub_u			subtract 512 bit subtrahend from 512 bit minuend, giving 512 bit difference and borrow
			add_uT64		add 64 bit addend to 512 bit augend, giving 512 bit sum and carry
			sub_uT64		subtract 64 bit subtrahend from 512 bit minuend, giving 512 bit difference and borrow
			add_u_wc		add 512 bit addend to 512 bit augend, with carry in, giving 512 bit sum and carry out
			sub_u_wb		subtract 512 bit subtrahend from 512 bit minuend, with borrow in, giving 512 bit difference and borrow out
			and_u			bitwise AND of 512 bit operand1 and 512 bit operand2, giving 512 bit result
			or_u			bitwise OR of 512 bit operand1 and 512 bit operand2, giving 512 bit result
			xor_u			bitwise XOR of 512 bit operand1 and 512 bit operand2, giving 512 bit result
			not_u			bitwise NOT of 512 bit operand, giving 512 bit result
			shr_u			logical right shift of 512 bit operand by specified number of bits, giving 512 bit result
			shl_u			logical left shift of 512 bit operand by specified number of bits, giving 512 bit result
			lsb_u			least significant bit of 512 bit operand, giving zero-based location of least significant bit set to 1, or -1 if operand is zero
			msb_u			most significant bit of 512 bit operand, giving zero-based location of most significant bit set to 1, or -1 if operand is zero
			copy_u			copy 512 bit source operand to 512 bit destination operand
			zero_u			zero out 512 bit destination operand
			set_uT64		set 512 bit destination operand to 64 bit source operand, zeroing upper bits
			compare_u		compare 512 bit operand1 to 512 bit operand2, giving 0 if equal, -1 if operand1 < operand2, +1 if operand1 > operand2
			compare_uT64	compare 512 bit operand1 to 64 bit operand2, giving 0 if equal, -1 if operand1 < operand2, +1 if operand1 > operand2	



Installation Instructions

    A.) Set up Visual Studio environment.
		Ref: https://www.wikihow.com/Use-MASM-in-Visual-Studio-2022
		Ref: https://learn.microsoft.com/en-us/cpp/assembler/masm/masm-for-x64-ml64-exe?view=msvc-170
		Ref: https://programminghaven.home.blog/2020/02/16/setup-an-assembly-project-on-visual-studio-2019/

		I also use ASMDude2 from https://marketplace.visualstudio.com/items?itemName=Henk-JanLebbink.AsmDude2
		You can install that through "Extensions, Manage Extensions, Online, MarketPlace, Tools, search "AsmDude2".

		It is not necessary, but provides syntax highlighting and mnemonic assistance with asm code.
		Also instruction performance characteristics.

	B.) Set up or copy project directories, add as project and or solution to new, blank project under Visual Studio
		solution explorer.

		Get VS to recognize .asm in general, and your files in particular:

			Right click project name.
			Select "Build Dependencies, Build Customization"
			Select (check) masm (.tagets, .props), Click OK
			Project can now include assembler code.

		Select all .asm source files. Right click, Select "Properties"
			Under "Configuration Properties", Select "Microsoft Macro Assembler"
			Under "General", Select "Include Paths", Click Edit
			Add:	$(ProjectDir)Include
			Click OK, Click Apply.

		The source code file is now recognized as partipating in the build, and of type assembler.
		Note: Add "$(ProjectDir)Include" to your project additional include directories

Usage Guidelines

    For this project, build a library to be included in another build such as a unit test program,
	a "C" program, or a "C++" program.
		Right click project name. Select properties.
		Under General Properties, Configuration Type, Select drop down, select "Static Library (.lib)",
		Click "Apply"
		Under Librarian, General, Select Output FIle, CLick Edit,
		and type:	$(SolutionDir)$(TargetName)$(TargetExt)
	This places your compiled library file (to include as an additional library
	to link in your host project), in the solution directory.

	Get a listing of your assembly. This is not necessary, but I like to look at the assembled code
	(old school tendency, where we debugged from listings and opcodes).
		Right click project name, Select Properties.
		Select Microsoft Macro Assembler, Expand it (the little arrow), Select Listing file.
		Select Enable Assembly Generated Code Listing, click drop down selector, select "YES (/Sg)",
		Click Apply.
		Select Assembled Code Listing File. Use drop down arrow, select "Edit .."
		Type in "$(IntDir)%(FileName).cod" Click OK, Click Apply.
		
	Click on the solution name in solution explorer..
		Click on the Visual Studio main menu: Build, Select Rebuild.
		Hopefully all the settings, copying, and environment setup results in a clean build. If not,
		go back and see what you missed.

		In solution explorer, right click on the project name. Select "Open Folder in File Explorer"
		In file explorer, navigate to project name, x64, debug.
		Double click on ui512.cod (your assembled code listing)

	Your .lib file is ready to be included in whatever project. It is under your project file directory, x64, debug (or release) , projectname.lib
	You can copy it, or refer to it in your other project build by directoryname/filename in the link section of the other build. 
	In the other build: Properties, Linker, Input, Additional dependencies, Edit: Add path,	filename to your new library.

	Don't forget to put something in a header file that looks something like this:
	//--------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Apologies to purists, but I want simpler, clearer, shorter variable
	// declarations (no "unsigned long long", etc.) 
	// Type aliases:

	typedef unsigned _int64 u64;
	typedef unsigned int u32;
	typedef unsigned long u32l;
	typedef unsigned short u16;
	typedef char u8;
	typedef _int64 s64;
	typedef int s32;
	typedef short s16;

	// Useful constants:

	#define u64_Max UINT64_MAX
	#define u32_Max UINT32_MAX
	#define u16_Max UINT16_MAX

	// 64 byte alignment macro and 512 bit (8 QWORD) aligned variable declaration
	#define ALIGN64 __declspec(alignas(64))
	#define _UI512(name) alignas(64) u64 name[8] /* Big-endian: name[0]=MSB qword, name[7]=LSB */


	// void zero_u ( u64* destarr ); 
	// fill supplied 512bit (8 QWORDS) with zero
	void zero_u( const u64* );

	// void copy_u ( u64* destarr, u64* srcarr );
	// copy supplied 512bit (8 QWORDS) source to supplied destination
	void copy_u( const u64*, const u64* );

	// void set_uT64 ( u64* destarr, u64 value );
	// set supplied destination 512 bit to supplied u64 value
	void set_uT64( const u64*, const u64 );

	//--------------------------------------------------------------------------------------------------------------------------------------------------------------
	// 
	// 	 from ui512_compare.asm
	// 
		// s16 compare_u ( u64* lh_op, u64* rh_op );
		// compare supplied 512bit (8 QWORDS) LH operand to supplied RH operand
		// returns: (0) for equal, -1 for less than, 1 for greater than (logical, unsigned compare)
	s16 compare_u( const u64*, const u64* );

	// s16 compare_uT64 ( u64* lh_op, u64 rh_op );
	// compare supplied 512bit (8 QWORDS) LH operand to supplied 64bit RH operand (value)
	// returns: (0) for equal, -1 for less than, 1 for greater than (logical, unsigned compare)
	s16 compare_uT64( const u64*, const u64 );

	//--------------------------------------------------------------------------------------------------------------------------------------------------------------
	//
	//	from ui512_addition.asm
	//

	// s16 add_u ( u64* sum, u64* addend1, u64* addend2 );
	// add supplied 512bit (8 QWORDS) sources to supplied destination
	// returns: zero for no carry, 1 for carry (overflow)
	s16 add_u( const u64*, const u64*, const u64* );

	// s16 add_u_wc ( u64* sum, u64* addend1, u64* addend2, s16 carry_in );
	// add supplied 512bit (8 QWORDS) sources to supplied destination
	// returns: zero for no carry, 1 for carry (overflow)
	s16 add_u_wc( const u64*, const u64*, const u64*, s16 );

	// s16 add_uT64 ( u64* sum, u64* addend1, u64 addend2 );
	// add 64bit QWORD (value) to supplied 512bit (8 QWORDS), place in supplied destination
	// returns: zero for no carry, 1 for carry (overflow)
	s16 add_uT64( const u64*, const u64*, const u64 );

	//--------------------------------------------------------------------------------------------------------------------------------------------------------------
	//
	//	from ui512_subtraction.asm
	//

	// s16 sub_u ( u64* difference, u64* left operand, u64* right operand );
	// subtract supplied 512bit (8 QWORDS) RH OP from LH OP giving difference in destination
	// returns: zero for no borrow, 1 for borrow (underflow)
	s16 sub_u( const u64*, const u64*, const u64* );

	// s16 sub_u_wb ( u64* difference, u64* left operand, u64* right operand, s16 borrow );
	// subtract supplied 512bit (8 QWORDS) RH OP from LH OP, with passed-in borrow giving difference in destination
	// returns: zero for no borrow, 1 for borrow (underflow)
	s16 sub_u_wb( const u64*, const u64*, const u64*, const s16 );

	// s16 sub_uT64( u64* difference, u64* left operand, u64 right operand );
	// subtract supplied 64 bit right hand (64 bit value) op from left hand (512 bit) giving difference
	// returns: zero for no borrow, 1 for borrow (underflow)
	s16 sub_uT64( const u64*, const u64*, const u64 );

	//--------------------------------------------------------------------------------------------------------------------------------------------------------------
	//
	//	from ui512_multiply.asm
	//

	//	EXTERNDEF	mult_uT64 : PROC
	//	mult_uT64	multiply 512 bit multiplicand by 64 bit multiplier, giving 512 product, 64 bit overflow
	//	Prototype:	s16 mult_uT64 ( u64 * product, u64 * overflow, u64 * multiplicand, u64 multiplier );
	s16 mult_uT64( const u64*, const u64*, const u64*, const u64 );

	//	EXTERNDEF	mult_u : PROC
	//	mult_u		multiply 512 multiplicand by 512 multiplier, giving 512 product, overflow
	//	Prototype:	s16 mult_u ( u64 * product, u64 * overflow, u64 * multiplicand, u64 * multiplier );
	s16 mult_u( const u64*, const u64*, const u64*, const u64* );

	//--------------------------------------------------------------------------------------------------------------------------------------------------------------
	//
	//	from ui512_divide.asm
	//

	//	EXTERNDEF	div_uT64 : PROC
	//	div_uT64	divide 512 bit dividend by 64 bit divisor, giving 512 bit quotient and 64 bit remainder
	//	Prototype:	s16 div_uT64 ( u64 * quotient, u64 * remainder, u64 * dividend, u64 divisor );
	s16 div_uT64( const u64*, const u64*, const u64*, const u64 );

	//	EXTERNDEF	div_u : PROC
	//	div_u		divide 512 bit dividend by 512 bit divisor, giving 512 bit quotient and remainder
	//	Prototype:	s16 div_u ( u64 * quotient, u64 * remainder, u64 * dividend, u64 * divisor );
	s16 div_u( const u64*, const u64*, const u64*, const u64* );

	//--------------------------------------------------------------------------------------------------------------------------------------------------------------
	//
	//	from ui512_significance.asm
	//

	// EXTERNDEF	msb_u : PROC
	// find most significant bit in supplied source 512bit (8 QWORDS)
	// returns: -1 if no most significant bit, bit number otherwise, bits numbered 0 to 511 inclusive	
	s16 msb_u( const u64* );

	// EXTERNDEF	lsb_u : PROC
	// find least significant bit in supplied source 512bit (8 QWORDS)
	// returns: -1 if no least significant bit, bit number otherwise, bits numbered 0 to 511 inclusive
	s16 lsb_u( const u64* );

	//--------------------------------------------------------------------------------------------------------------------------------------------------------------
	//
	//	from ui512_shift.asm
	//

	// void shr_u ( u64* destination, u64* source, u32 bits_to_shift );
	// shift supplied source 512bit (8 QWORDS) right, put in destination
	// EXTERNDEF	shr_u : PROC
	void shr_u( const u64*, const u64*, const u16 );

	// void shl_u ( u64* destination, u64* source, u16 bits_to_shift );
	// shift supplied source 512bit (8 QWORDS) left, put in destination
	// EXTERNDEF	shl_u : PROC
	void shl_u( const u64*, const u64*, const u16 );

	//--------------------------------------------------------------------------------------------------------------------------------------------------------------
	//
	//	from ui512_bitops.asm
	//

	// void and_u ( u64* destination, u64* lh_op, u64* rh_op );
	// logical 'AND' bits in lh_op, rh_op, put result in destination
	// EXTERNDEF	and_u : PROC
	void and_u( const u64*, const u64*, const u64* );

	// void or_u ( u64* destination, u64* lh_op, u64* rh_op );
	// logical 'OR' bits in lh_op, rh_op, put result in destination	
	// EXTERNDEF	or_u : PROC
	void or_u( const u64*, const u64*, const u64* );

	// void xor_u ( u64* destination, u64* lh_op, u64* rh_op );
	// logical 'XOR' bits in lh_op, rh_op, put result in destination	
	// EXTERNDEF	xor_u : PROC
	void xor_u( const u64*, const u64*, const u64* );

	// void not_u ( u64* destination, u64* source );
	// logical 'NOT' bits in source, put result in destination
	// EXTERNDEF	not_u : PROC
	void not_u( const u64*, const u64* );

Contributing

    I'm interested in ways to improve the code, feel free to suggest, revise.


License

	MIT License

		Copyright (c) 2026 John G. Lynch
			Permission is hereby granted, free of charge, to any person obtaining a copy
			of this software and associated documentation files (the "Software"), to deal
			in the Software without restriction, including without limitation the rights
			to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
			copies of the Software, and to permit persons to whom the Software is
			furnished to do so, subject to the following conditions:

			The above copyright notice and this permission notice shall be included in all
			copies or substantial portions of the Software.

			THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
			IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
			FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
			AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
			LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
			OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
			SOFTWARE.


Contact Information

    This project is posted at Github, User Name JGLynch54.
