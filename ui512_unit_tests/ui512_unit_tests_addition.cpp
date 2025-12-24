//		ui512_unit_tests_addition
// 
//		File:			ui512_unit_tests_addition.cpp
//		Author:			John G.Lynch
//		Legal:			Copyright @2024, per MIT License below
//		Date:			October 29, 2025
//
//		ui512 is a small project to provide basic operations for a variable type of unsigned 512 bit integer.
//		The basic operations : zero, copy, compare, add, subtract.
//		Other optional modules provide bit ops and multiply / divide.
//		It is written in assembly language, using the MASM ( ml64 ) assembler provided as an option within Visual Studio.
//		( currently using VS Community 2022 17.14.10)
//		It provides external signatures that allow linkage to C and C++ programs,
//		where a shell / wrapper could encapsulate the methods as part of an object.
//		It has assembly time options directing the use of Intel processor extensions : AVX4, AVX2, SIMD, or none :
//		( Z ( 512 ), Y ( 256 ), or X ( 128 ) registers, or regular Q ( 64bit ) ).
//		If processor extensions are used, the caller must align the variables declared and passed
//		on the appropriate byte boundary ( e.g. alignas 64 for 512 )
//		These modules (in total) are very light-weight ( less than 10K bytes ) and relatively fast,
//		but is not intended for all processor types or all environments.
// 
//		Intended use cases :
//			1.) a "sum of primes" for primes up to 2 ^ 48.
//			2.) elliptical curve cryptography(ECC)
//
//		This sub - project: ui512aTests, is a unit test project that invokes each of the routines in the ui512a assembly.
//		It runs each assembler proc with pseudo-random values.
//		It validates ( asserts ) expected and returned results.
//		It also runs each repeatedly for comparative timings.
//		It provides a means to invoke and debug.
//		It illustrates calling the routines from C++.

#include "pch.h"
#include "CppUnitTest.h"
#include "ui512_externs.h"
#include "ui512_unit_tests.h"

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ui512_Unit_Tests
{
	TEST_CLASS( ui512_unit_tests_addition )
	{
		TEST_METHOD( ui512_01_add )
		{
			_UI512( num1 ) { 0, 0, 0, 0, 0, 0, 0, 0 };
			_UI512( num2 ) { 0, 0, 0, 0, 0, 0, 0, 0 };
			_UI512( sum ) { 0, 0, 0, 0, 0, 0, 0, 0 };
			_UI512( one ) { 0, 0, 0, 0, 0, 0, 0, 1 };
			u64 seed = 0;
			s16 overflow = 0;

			// cascade testing (cascading lane by lane carry(ies))
			_UI512( cascade1 ) { 0xFFFFFFFFFFFFFFFF, 0, 0, 0xFFFFFFFFFFFFFFFF, 0, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 1 };
			_UI512( cascade2 ) { 10, 0, 0, 2, 0, 0, 5, 1 };
			// should: lane by lane from the right: add yielding 2; add yield 4 with carry ; carry in cascade to 0 with carry; add to 1
			//		add yield 1 with carry, carry in to 1, add to 0, add to 9 with overflow
			//		first lane by lane gives three carries (including overflow) lanes 0, 3, 6
			//		causes second iteration of carry in adds, causing carry in lane 5
			overflow = add_u( sum, cascade1, cascade2 );
			Assert::AreEqual( ( s16 ) 1, overflow );
			Assert::AreEqual( ( u64 ) 9, sum [ 0 ] );
			Assert::AreEqual( ( u64 ) 0, sum [ 1 ] );
			Assert::AreEqual( ( u64 ) 1, sum [ 2 ] );
			Assert::AreEqual( ( u64 ) 1, sum [ 3 ] );
			Assert::AreEqual( ( u64 ) 1, sum [ 4 ] );
			Assert::AreEqual( ( u64 ) 0, sum [ 5 ] );
			Assert::AreEqual( ( u64 ) 4, sum [ 6 ] );
			Assert::AreEqual( ( u64 ) 2, sum [ 7 ] );

			for ( int i = 0; i < test_run_count; i++ )
			{
				for ( int j = 0; j < 8; j++ )
				{
					num1 [ j ] = RandomU64( &seed );
					num2 [ j ] = ~num1 [ j ];
					sum [ j ] = 0;
				};

				// add test: "random" number plus ones complement should equal 0xfff..., no carries or overflow
				overflow = add_u( sum, num1, num2 );
				Assert::AreEqual( ( s16 ) 0, overflow );
				for ( int j = 0; j < 8; j++ )
				{
					Assert::AreEqual( 0xFFFFFFFFFFFFFFFFull, sum [ j ] );
				};

				// now add one, should cascade carries through all eight, making them each zero, and overflow
				overflow = add_u( sum, sum, one );		// Note:  Destination (sum) is also an operand
				Assert::AreEqual( ( s16 ) 1, overflow );
				for ( int j = 0; j < 8; j++ )
				{
					Assert::AreEqual( 0x0000000000000000ull, sum [ j ] );
				};

				// run same tests, with destination being one of the sources
				for ( int j = 0; j < 8; j++ )
				{
					num1 [ j ] = RandomU64( &seed );
					num2 [ j ] = ~num1 [ j ];
				};

				// add test: "random" number plus ones complement should equal 0xfff..., no carries or overflow
				overflow = add_u( num1, num1, num2 );
				Assert::AreEqual( ( s16 ) 0, overflow );
				for ( int j = 0; j < 8; j++ )
				{
					Assert::AreEqual( 0xFFFFFFFFFFFFFFFFull, num1 [ j ] );
				};

				// now add one, should cascade carries through all eight, making them each zero, and overflow
				overflow = add_u( num1, num1, one );		// Note:  Destination (sum) is also an operand
				Assert::AreEqual( ( s16 ) 1, overflow );
				for ( int j = 0; j < 8; j++ )
				{
					Assert::AreEqual( 0x0000000000000000ull, num1 [ j ] );
				};

				zero_u( num1 );
				zero_u( num2 );
				for ( int j = 0; j < 8; j++ )
				{

					num1 [ j ] = RandomU64( &seed );
					num2 [ j ] = RandomU64( &seed );
					sum [ j ] = 0;
				};

				u16 nRzeros = RandomU64( &seed ) % 4;
				for ( u16 k = 0; k < nRzeros; k++ ) {
					s16 kk = RandomU64( &seed ) % 7;
					num1 [ kk ] = 0;
				};

				nRzeros = RandomU64( &seed ) % 4;
				for ( u16 k = 0; k < nRzeros; k++ ) {
					s16 kk = RandomU64( &seed ) % 7;
					num2 [ kk ] = 0;
				}

				overflow = add_u( sum, num1, num2 );
				s16 chkovr = compare_u( sum, num1 );
				if ( overflow )
				{
					if ( chkovr != -1 )
					{
						Logger::WriteMessage( "Overflow error: false positive\n" );
					};
				}

				if ( !overflow )
				{
					if ( chkovr == -1 )
					{
						Logger::WriteMessage( "Overflow error: false negative\n" );
					};
				};
			};

			string runmsg = "Add function testing. Ran tests " + to_string( test_run_count * 5 ) + " times, each with pseudo random values.\n";
			Logger::WriteMessage( runmsg.c_str( ) );
			Logger::WriteMessage( L"Passed. Tested expected values via assert.\n" );
		};

		TEST_METHOD( ui512_01_add_performance )
		{
			// Performance timing tests.
			// Ref: "Essentials of Modern Business Statistics", 7th Ed, by Anderson, Sweeney, Williams, Camm, Cochran. South-Western, 2015
			// Sections 3.2, 3.3, 3.4
			// Note: these tests are not pass/fail, they are informational only

			Logger::WriteMessage( L"Add function performance timing test.\n\n" );

			Logger::WriteMessage( L"First run.\n" );
			perf_stats No1 = Perf_Test_Parms [ 0 ];
			ui512_Unit_Tests::RunStats( &No1, Add );

			Logger::WriteMessage( L"Second run.\n" );
			perf_stats No2 = Perf_Test_Parms [ 1 ];
			RunStats( &No2, Add );

			Logger::WriteMessage( L"Third run.\n" );
			perf_stats No3 = Perf_Test_Parms [ 2 ];
			RunStats( &No3, Add );
		};

		TEST_METHOD( ui512_01_add_reg )
		{
			// add_u function register verification.
			//	Check register before call, verify non-volatile register remain unchanged after call.
			regs r_before {};
			regs r_after {};
			u64 seed = 0;
			_UI512( num1 ) { };
			_UI512( num2 ) { };
			_UI512( num3 ) { };
			for ( int i = 0; i < reg_verification_count; i++ )
			{
				for ( int j = 0; j < 8; j++ )
				{
					num1 [ j ] = RandomU64( &seed );
					num2 [ j ] = RandomU64( &seed );
					num3 [ j ] = RandomU64( &seed );
				};
				u64 val = RandomU64( &seed );
				r_before.Clear( );
				reg_verify( ( u64* ) &r_before );
				add_u( num1, num2, num3 );
				r_after.Clear( );
				reg_verify( ( u64* ) &r_after );
				Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
			};

			string runmsg = "add_u function register validation. Ran " + to_string( reg_verification_count ) + " times.\n";
			Logger::WriteMessage( runmsg.c_str( ) );
		};

		TEST_METHOD( ui512_03_add_u_wc )
		{
			u64 seed = 0;
			_UI512( num1 ) { 0 };  // All zero
			_UI512( num2 ) { 0 };
			_UI512( sum ) { 0 };
			_UI512( expected ) { 0, 0, 0, 0, 0, 0, 0, 1 };  // LSB=1
			s16 carry = 1;

			// Basic carry-in
			s16 overflow = add_u_wc( sum, num1, num2, carry );
			Assert::AreEqual( ( s16 ) 0, overflow );
			for ( int j = 0; j < 8; j++ ) {
				Assert::AreEqual( expected [ j ], sum [ j ] );
			}

			_UI512( one ) { 0, 0, 0, 0, 0, 0, 0, 1 };
			overflow = 0;

			_UI512( cascade1 ) { 0xFFFFFFFFFFFFFFFF, 0, 0, 0xFFFFFFFFFFFFFFFF, 0, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 1 };
			_UI512( cascade2 ) { 10, 0, 0, 2, 0, 0, 5, 1 };
			// should: lane by lane from the right: add yielding 2; add yield 4 with carry ; carry in cascade to 0 with carry; add to 1
			//		add yield 1 with carry, carry in to 1, add to 0, add to 9 with overflow
			//		first lane by lane gives three carries (including overflow) lanes 0, 3, 6
			//		causes second iteration of carry in adds, causing carry in lane 5
			overflow = add_u_wc( sum, cascade1, cascade2, 0 );
			Assert::AreEqual( ( s16 ) 1, overflow );
			Assert::AreEqual( ( u64 ) 9, sum [ 0 ] );
			Assert::AreEqual( ( u64 ) 0, sum [ 1 ] );
			Assert::AreEqual( ( u64 ) 1, sum [ 2 ] );
			Assert::AreEqual( ( u64 ) 1, sum [ 3 ] );
			Assert::AreEqual( ( u64 ) 1, sum [ 4 ] );
			Assert::AreEqual( ( u64 ) 0, sum [ 5 ] );
			Assert::AreEqual( ( u64 ) 4, sum [ 6 ] );
			Assert::AreEqual( ( u64 ) 2, sum [ 7 ] );

			for ( int i = 0; i < test_run_count; i++ )
			{
				for ( int j = 0; j < 8; j++ )
				{
					num1 [ j ] = RandomU64( &seed );
					num2 [ j ] = ~num1 [ j ];
					sum [ j ] = 0;
				};

				// add test: "random" number plus ones complement should equal 0xfff..., no carries or overflow
				overflow = add_u_wc( sum, num1, num2, 0 );
				Assert::AreEqual( ( s16 ) 0, overflow );
				for ( int j = 0; j < 8; j++ )
				{
					Assert::AreEqual( 0xFFFFFFFFFFFFFFFFull, sum [ j ] );
				};

				// now add one, should cascade carries through all eight, making them each zero, and overflow
				overflow = add_u_wc( sum, sum, one, 0 );		// Note:  Destination (sum) is also an operand
				Assert::AreEqual( ( s16 ) 1, overflow );
				for ( int j = 0; j < 8; j++ )
				{
					Assert::AreEqual( 0x0000000000000000ull, sum [ j ] );
				};

				// run same tests, with destination being one of the sources
				for ( int j = 0; j < 8; j++ )
				{
					num1 [ j ] = RandomU64( &seed );
					num2 [ j ] = ~num1 [ j ];
				};

				// add test: "random" number plus ones complement should equal 0xfff..., no carries or overflow
				overflow = add_u_wc( num1, num1, num2, 0 );
				Assert::AreEqual( ( s16 ) 0, overflow );
				for ( int j = 0; j < 8; j++ )
				{
					Assert::AreEqual( 0xFFFFFFFFFFFFFFFFull, num1 [ j ] );
				};

				// now add one, should cascade carries through all eight, making them each zero, and overflow
				overflow = add_u_wc( num1, num1, one, 0 );		// Note:  Destination (sum) is also an operand
				Assert::AreEqual( ( s16 ) 1, overflow );
				for ( int j = 0; j < 8; j++ )
				{
					Assert::AreEqual( 0x0000000000000000ull, num1 [ j ] );
				};

				zero_u( num1 );
				zero_u( num2 );
				for ( int j = 0; j < 8; j++ )
				{

					num1 [ j ] = RandomU64( &seed );
					num2 [ j ] = RandomU64( &seed );
					sum [ j ] = 0;
				};

				u16 nRzeros = RandomU64( &seed ) % 4;
				for ( u16 k = 0; k < nRzeros; k++ ) {
					s16 kk = RandomU64( &seed ) % 7;
					num1 [ kk ] = 0;
				};

				nRzeros = RandomU64( &seed ) % 4;
				for ( u16 k = 0; k < nRzeros; k++ ) {
					s16 kk = RandomU64( &seed ) % 7;
					num2 [ kk ] = 0;
				}

				overflow = add_u_wc( sum, num1, num2, 0 );
				s16 chkovr = compare_u( sum, num1 );
				if ( overflow )
				{
					if ( chkovr != -1 )
					{
						Logger::WriteMessage( "Overflow error: false positive\n" );
					};
				}

				if ( !overflow )
				{
					if ( chkovr == -1 )
					{
						Logger::WriteMessage( "Overflow error: false negative\n" );
					};
				};
			};

			string runmsg = "Add with carry function testing. Ran tests " + to_string( test_run_count * 5 ) + " times, each with pseudo random values.\n";
			Logger::WriteMessage( runmsg.c_str( ) );
			Logger::WriteMessage( L"Passed. Tested expected values via assert.\n" );
		}

		TEST_METHOD( ui512_03_addwc_performance )
		{
			// Performance timing tests.
			// Ref: "Essentials of Modern Business Statistics", 7th Ed, by Anderson, Sweeney, Williams, Camm, Cochran. South-Western, 2015
			// Sections 3.2, 3.3, 3.4
			// Note: these tests are not pass/fail, they are informational only

			Logger::WriteMessage( L"Add with carry function performance timing test.\n\n" );

			Logger::WriteMessage( L"First run.\n" );
			perf_stats No1 = Perf_Test_Parms [ 0 ];
			RunStats( &No1, AddwC );

			Logger::WriteMessage( L"Second run.\n" );
			perf_stats No2 = Perf_Test_Parms [ 1 ];
			RunStats( &No2, AddwC );

			Logger::WriteMessage( L"Third run.\n" );
			perf_stats No3 = Perf_Test_Parms [ 2 ];
			RunStats( &No3, AddwC );
		};

		TEST_METHOD( ui512_03_addwc_reg )
		{
			// add_u function register verification.
			//	Check register before call, verify non-volatile register remain unchanged after call.
			regs r_before {};
			regs r_after {};
			u64 seed = 0;
			_UI512( num1 ) { };
			_UI512( num2 ) { };
			_UI512( num3 ) { };
			for ( int i = 0; i < reg_verification_count; i++ )
			{
				for ( int j = 0; j < 8; j++ )
				{
					num1 [ j ] = RandomU64( &seed );
					num2 [ j ] = RandomU64( &seed );
					num3 [ j ] = RandomU64( &seed );
				};
				u64 val = RandomU64( &seed );
				r_before.Clear( );
				reg_verify( ( u64* ) &r_before );
				add_u_wc( num1, num2, num3, ( s16 ) 0 );
				r_after.Clear( );
				reg_verify( ( u64* ) &r_after );
				Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
			};

			string runmsg = "add_u_wc function register validation. Ran " + to_string( reg_verification_count ) + " times.\n";
			Logger::WriteMessage( runmsg.c_str( ) );
		};
		TEST_METHOD( ui512_02_add64 )
		{
			_UI512( num1 ) { 0, 0, 0, 0, 0, 0, 0, 0 };
			_UI512( sum ) { 0, 0, 0, 0, 0, 0, 0, 0 };
			_UI512( one ) { 0, 0, 0, 0, 0, 0, 0, 1 };

			u64 seed = 0;
			u64 num2 = 0;
			s32 overflow = 0;

			for ( int i = 0; i < test_run_count; i++ )
			{
				for ( int j = 0; j < 8; j++ )
				{
					num1 [ j ] = 0xFFFFFFFFFFFFFFFFull;
					sum [ j ] = 0;
				};
				num2 = RandomU64( &seed );
				num1 [ 7 ] = ~num2;
				// add test: "random" number plus ones complement should equal 0xfff..., no carries or overflow
				overflow = add_uT64( sum, num1, num2 );
				Assert::AreEqual( 0, overflow );
				for ( int j = 0; j < 8; j++ )
				{
					Assert::AreEqual( 0xFFFFFFFFFFFFFFFFull, sum [ j ] );
				};
				// now add one, should cascade carries through all eight, making them each zero, and overflow
				overflow = add_uT64( sum, sum, 0x0000000000000001ull );		// Note:  Destination (sum) is also an operand
				Assert::AreEqual( 1, overflow );
				for ( int j = 0; j < 8; j++ )
				{
					Assert::AreEqual( 0x0000000000000000ull, sum [ j ] );
				};
			};

			string runmsg = "Add (x64) function testing. Ran tests " + to_string( test_run_count * 2 ) + " times, each with pseudo random values.\n";
			Logger::WriteMessage( runmsg.c_str( ) );
			Logger::WriteMessage( L"Passed. Tested expected values via assert.\n" );
		};

		TEST_METHOD( ui512_02_add64_performance )
		{
			// Performance timing tests.
			// Ref: "Essentials of Modern Business Statistics", 7th Ed, by Anderson, Sweeney, Williams, Camm, Cochran. South-Western, 2015
			// Sections 3.2, 3.3, 3.4
			// Note: these tests are not pass/fail, they are informational only

			Logger::WriteMessage( L"Add x64 function performance timing test.\n\n" );

			Logger::WriteMessage( L"First run.\n" );
			perf_stats No1 = Perf_Test_Parms [ 0 ];
			ui512_Unit_Tests::RunStats( &No1, Add64 );

			Logger::WriteMessage( L"Second run.\n" );
			perf_stats No2 = Perf_Test_Parms [ 1 ];
			RunStats( &No2, Add64 );

			Logger::WriteMessage( L"Third run.\n" );
			perf_stats No3 = Perf_Test_Parms [ 2 ];
			RunStats( &No3, Add64 );
		};


		TEST_METHOD( ui512_02_add64_reg )
		{
			// Add64 function register verification.
			//	Check register before call, verify non-volatile register remain unchanged after call.
			regs r_before {};
			regs r_after {};
			u64 seed = 0;
			_UI512( num1 ) { };
			for ( int i = 0; i < reg_verification_count; i++ )
			{
				RandomFill( num1, &seed );
				u64 val = RandomU64( &seed );
				r_before.Clear( );
				reg_verify( ( u64* ) &r_before );
				add_uT64( num1, num1, val );
				r_after.Clear( );
				reg_verify( ( u64* ) &r_after );
				Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
			};

			string runmsg = "add_uT64 function register validation. Ran " + to_string( reg_verification_count ) + " times.\n";
			Logger::WriteMessage( runmsg.c_str( ) );
		};

	};	// test_class
};	// namespace
