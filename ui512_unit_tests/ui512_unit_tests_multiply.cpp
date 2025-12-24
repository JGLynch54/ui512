//		ui512_unit_tests_multiply
// 
//		File:			ui512_unit_tests_multiply.cpp
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

#include <cstring>
#include <sstream>
#include <format>
#include <chrono>
#include "intrin.h"

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ui512_Unit_Tests
{
	TEST_CLASS( ui512_unit_tests_multiply )
	{
		TEST_METHOD( ui512md_01_mul )
		{
			// mult_u tests
			// multistage testing, part for use as debugging, progressively "real" testing
			// Note: the ui512a module must pass testing before these tests, as zero, add, and set are used in this test
			// Note: the ui512b module must pass testing before these tests, as 'or' and shifts are used in this test

			u64 seed = 0;
			regs r_before {};
			regs r_after {};
			_UI512( num1 ) { 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul };;
			_UI512( num2 ) { 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul };;
			_UI512( num3 ) { 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul };;
			_UI512( product ) { 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul };;
			_UI512( overflow ) { 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul };;
			_UI512( intermediateprod ) { 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul };;
			_UI512( intermediateovrf ) { 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul };;
			_UI512( expectedproduct ) { 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul };;
			_UI512( expectedoverflow ) { 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul };;
			_UI512( work ) { 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul };;
			__m512i a { 1,2,3,4,5,6,7,8 };
			__m512i b { 0,0,0,0,0,0,0, 2 };
			__m512i result = _mm512_mul_epu32( a, b ); // VPMULUDQ

			// Edge case tests

			// 1. zero times zero (don't loop and repeat, just once)
			zero_u( num1 );
			zero_u( num2 );
			zero_u( expectedproduct );
			zero_u( expectedoverflow );
			reg_verify( ( u64* ) &r_before );
			s16 ret = mult_u( product, overflow, num1, num2 );
			reg_verify( ( u64* ) &r_after );
			Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
			Assert::AreEqual( s16( 0 ), ret, L"Return code failed zero times zero test." ); // Only exception possible is parameter alignment
			for ( int j = 0; j < 8; j++ )
			{
				Assert::AreEqual( expectedproduct [ j ], product [ j ], _MSGW( L"Product at word #" << j << " failed zero times zero test." ) );
				Assert::AreEqual( expectedoverflow [ j ], overflow [ j ], _MSGW( L"Overflow at word #" << j << " failed zero times zero test." ) );
			};

			// 2. zero times random
			for ( int i = 0; i < test_run_count; i++ )
			{
				zero_u( num1 );
				RandomFill( num2, &seed );
				zero_u( expectedproduct );
				zero_u( expectedoverflow );
				reg_verify( ( u64* ) &r_before );
				s16 ret = mult_u( product, overflow, num1, num2 );
				reg_verify( ( u64* ) &r_after );
				Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
				Assert::AreEqual( s16( 0 ), ret, L"Return code failed zero times random test." ); // Only exception possible is parameter alignment
				for ( int j = 0; j < 8; j++ )
				{
					Assert::AreEqual( expectedproduct [ j ], product [ j ], _MSGW( L"Product at word #" << j << " failed zero times random test on run #" << i ) );
					Assert::AreEqual( expectedoverflow [ j ], overflow [ j ], _MSGW( L"Overflow at word #" << j << " failed zero times random test on run #" << i ) );
				};
			};

			// 3. random times zero
			for ( int i = 0; i < test_run_count; i++ )
			{
				zero_u( num2 );
				RandomFill( num1, &seed );
				zero_u( expectedproduct );
				zero_u( expectedoverflow );
				reg_verify( ( u64* ) &r_before );
				s16 ret = mult_u( product, overflow, num1, num2 );
				reg_verify( ( u64* ) &r_after );
				Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
				Assert::AreEqual( s16( 0 ), ret, L"Return code failed random times zero test." ); // Only exception possible is parameter alignment
				for ( int j = 0; j < 8; j++ )
				{
					Assert::AreEqual( expectedproduct [ j ], product [ j ], _MSGW( L"Product at word #" << j << " failed random times zero test on run #" << i ) );
					Assert::AreEqual( expectedoverflow [ j ], overflow [ j ], _MSGW( L"Overflow at word #" << j << " failed random times zero test on run #" << i ) );
				};
			};

			// 4. one times random
			for ( int i = 0; i < test_run_count; i++ )
			{
				set_uT64( num1, 1ull );
				RandomFill( num2, &seed );
				copy_u( expectedproduct, num2 );
				zero_u( expectedoverflow );
				reg_verify( ( u64* ) &r_before );
				s16 ret = mult_u( product, overflow, num1, num2 );
				reg_verify( ( u64* ) &r_after );
				Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
				Assert::AreEqual( s16( 0 ), ret, L"Return code failed one times random test." ); // Only exception possible is parameter alignment
				for ( int j = 0; j < 8; j++ )
				{
					Assert::AreEqual( expectedproduct [ j ], product [ j ], _MSGW( L"Product at word #" << j << " failed one times random test on run #" << i ) );
					Assert::AreEqual( expectedoverflow [ j ], overflow [ j ], _MSGW( L"Overflow at word #" << j << " failed one times random test on run #" << i ) );
				};
			};

			// 5. random times one
			for ( int i = 0; i < test_run_count; i++ )
			{
				set_uT64( num2, 1ull );
				RandomFill( num1, &seed );
				copy_u( expectedproduct, num1 );
				zero_u( expectedoverflow );
				reg_verify( ( u64* ) &r_before );
				s16 ret = mult_u( product, overflow, num1, num2 );
				reg_verify( ( u64* ) &r_after );
				Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
				Assert::AreEqual( s16( 0 ), ret, L"Return code failed random times one test." ); // Only exception possible is parameter alignment
				for ( int j = 0; j < 8; j++ )
				{
					Assert::AreEqual( expectedproduct [ j ], product [ j ], _MSGW( L"Product at word #" << j << " failed random times one test on run #" << i ) );
					Assert::AreEqual( expectedoverflow [ j ], overflow [ j ], _MSGW( L"Overflow at word #" << j << " failed random times one test on run #" << i ) );
				};
			};

			{
				string test_message = _MSGA( "Multiply function testing.\n\nEdge cases:\n\tzero times zero,\n\tzero times random,\n\trandom times zero,\n\tone times random,\n\trandom times one.\n"
					<< test_run_count << " times each, with pseudo random values.\n" );
				Logger::WriteMessage( test_message.c_str( ) );
				Logger::WriteMessage( L"Passed. Tested expected values, return value, and volatile register integrity: each via assert.\n\n" );
			};


			// Now the real tests, progressively more complex

			// First test, a simple multiply by two. 
			// Easy to check as the expected answer is a shift left, expected overflow is a shift right
			for ( int i = 0; i < test_run_count; i++ )
			{
				RandomFill( num1, &seed );					//	random initialize multiplicand				
				set_uT64( num2, 2ull );						//	initialize multiplier				
				shl_u( expectedproduct, num1, u16( 1 ) );		// calculate expected product				
				shr_u( expectedoverflow, num1, u16( 511 ) );	// calculate expected overflow				
				reg_verify( ( u64* ) &r_before );
				s16 ret = mult_u( product, overflow, num1, num2 );
				reg_verify( ( u64* ) &r_after );
				Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
				Assert::AreEqual( s16( 0 ), ret, L"Return code failed simple times two test." ); // Only exception possible is parameter alignment

				//	check actual vs. expected
				for ( int j = 0; j < 8; j++ )
				{
					Assert::AreEqual( expectedproduct [ j ], product [ j ], _MSGW( L"Product at word #" << j << " failed on run #" << i ) );
					Assert::AreEqual( expectedoverflow [ j ], overflow [ j ], _MSGW( L"Overflow at word #" << j << " failed on run #" << i ) );
				}
			};
			{
				string test_message = _MSGA( "First test, a simple multiply by two.  " << test_run_count << " times, each with pseudo random values.\n" );
				Logger::WriteMessage( test_message.c_str( ) );
				Logger::WriteMessage( L"Passed. Tested expected values, return value, and volatile register integrity: all via assert.\n\n" );
			};

			// Second test, a simple multiply by a random power of two. 
			// Still relatively easy to check as the expected answer is a shift left,
			// expected overflow is a shift right
			for ( int i = 0; i < test_run_count; i++ )
			{
				RandomFill( num1, &seed );
				set_uT64( num2, 1ull );
				u16 nrShift = RandomU64( &seed ) % 512;
				shl_u( num2, num2, nrShift );
				shl_u( expectedproduct, num1, nrShift );
				shr_u( expectedoverflow, num1, ( 512 - nrShift ) );
				reg_verify( ( u64* ) &r_before );
				s16 ret = mult_u( product, overflow, num1, num2 );
				reg_verify( ( u64* ) &r_after );
				Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
				Assert::AreEqual( s16( 0 ), ret, L"Return code failed random power of 2 test." ); // Only exception possible is parameter alignment
				for ( int j = 0; j < 8; j++ )
				{
					Assert::AreEqual( expectedproduct [ j ], product [ j ], _MSGW( L"Product at word #" << j << " failed on run #" << i ) );
					Assert::AreEqual( expectedoverflow [ j ], overflow [ j ], _MSGW( L"Overflow at word #" << j << " failed on run #" << i ) );
				}
			};

			{
				string test_message = _MSGA( "Second test, a simple multiply by a random power of two. " << test_run_count << " times, each with pseudo random values.\n" );
				Logger::WriteMessage( test_message.c_str( ) );
				Logger::WriteMessage( L"Passed. Tested expected values, return value, and volatile register integrity: all via assert.\n\n" );
			};

			// Third test, a simple multiply by a random 64 bit integer. Checked via a 64 bit multiply (which is tested before this, or they both fail)
			for ( int i = 0; i < test_run_count; i++ )
			{
				RandomFill( num1, &seed );
				u64 num2_64 = RandomU64( &seed );
				u64 expectedovfl_64 = 0;
				set_uT64( num2, num2_64 );
				mult_uT64( expectedproduct, &expectedovfl_64, num1, num2_64 );
				set_uT64( expectedoverflow, expectedovfl_64 );
				reg_verify( ( u64* ) &r_before );
				s16 ret = mult_u( product, overflow, num1, num2 );
				reg_verify( ( u64* ) &r_after );
				Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
				Assert::AreEqual( s16( 0 ), ret, L"Return code failed random 64 test." ); // Only exception possible is parameter alignment

				for ( int j = 0; j < 8; j++ )
				{
					Assert::AreEqual( expectedproduct [ j ], product [ j ], _MSGW( L"Product at word #" << j << " failed on run #" << i ) );
					Assert::AreEqual( expectedoverflow [ j ], overflow [ j ], _MSGW( L"Overflow at word #" << j << " failed on run #" << i ) );
				}
			};

			{
				string test_message = _MSGA( "Third test, a multiply by a random 64 bit value. " << test_run_count << " times, each with pseudo random values.\n" );
				Logger::WriteMessage( test_message.c_str( ) );
				Logger::WriteMessage( L"Passed. Tested expected values, return value, and volatile register integrity: all via assert.\n\n" );
			};

			// Fourth test, a multiply by sums of random powers of two into a 64 bit value. 
			// Building "expected" is a bit more complicated. This test is more about correctly building "expected"
			for ( int i = 0; i < test_run_count; i++ )
			{
				bool rbits [ 64 ];
				std::memset( rbits, 0, sizeof( rbits ) );

				for ( int j = 0; j < 63; j++ )
				{
					rbits [ j ] = ( RandomU64( &seed ) % 2 ) == 1;
				};

				// random initialize multiplicand
				RandomFill( num1, &seed );

				// build multiplier as 'N' number of random bits (avoid duplicate bits)
				// simultaneously build expected result and overflow

				zero_u( num2 );
				zero_u( expectedproduct );
				zero_u( expectedoverflow );

				u16 nrBits = ( RandomU64( &seed ) % 36 ) + 2; //  Nr Bits somewhere between 2 and 36? 
				for ( int j = 0; j < nrBits; j++ )
				{
					zero_u( intermediateprod );
					zero_u( intermediateovrf );
					// use selected bit to build a random number (through shift/or)
					// then build expected result of multiply (through shift / add)
					bool found = false;
					u16 nrShift = 0;
					while ( ( nrShift == 0 ) && !found )
					{
						u16 rBit = RandomU64( &seed ) % 64;
						for ( u16 idx = rBit; idx < 64; idx++ )
						{
							u16 bt = idx % 64;
							u64 msk = 1ull << bt;
							if ( rbits [ idx ] && !( num2 [ 7 ] & msk ) )
							{
								nrShift = idx;
								found = true;
								break;
							}
						};
						if ( !found )
						{
							break;
						};
					};

					if ( found )
					{
						// Multiplier:
						set_uT64( num3, 1ull );
						shl_u( num3, num3, nrShift );
						or_u( num2, num2, num3 );

						// Calculate expected product and overflow:
						shl_u( intermediateprod, num1, nrShift );
						s16 prad = add_u( expectedproduct, expectedproduct, intermediateprod );
						shr_u( intermediateovrf, num1, 512 - nrShift );
						if ( prad == 1 )
						{
							add_uT64( intermediateovrf, intermediateovrf, 1ull );
						};
						s16 ofad = add_u( expectedoverflow, expectedoverflow, intermediateovrf );
					};
				};

				_UI512( prod64 );
				_UI512( ovfl64 );
				zero_u( prod64 );
				zero_u( ovfl64 );
				reg_verify( ( u64* ) &r_before );
				s16 ret = mult_u( prod64, ovfl64, num1, num2 );
				reg_verify( ( u64* ) &r_after );
				Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
				Assert::AreEqual( s16( 0 ), ret, L"Return code failed random 64 test." ); // Only exception possible is parameter alignment

				// Compare 64bit results to calculated expected results (aborts test if they don't match)
				for ( int j = 0; j < 8; j++ )
				{
					if ( expectedproduct [ j ] != prod64 [ j ] ) {
						Logger::WriteMessage( ToString( j ).c_str( ) );
					};
					Assert::AreEqual( expectedproduct [ j ], prod64 [ j ], _MSGW( L"64 bit Product at word #" << j << " failed on run #" << i ) );
					if ( expectedoverflow [ j ] != ovfl64 [ j ] ) {
						Logger::WriteMessage( ToString( j ).c_str( ) );
					};
					Assert::AreEqual( expectedoverflow [ j ], ovfl64 [ j ], _MSGW( L"64 bit Overflow at word #" << j << " failed on run #" << i ) );
				}

				// Got a random multiplicand and multiplier. Execute function under test
				reg_verify( ( u64* ) &r_before );
				s16 ret2 = mult_u( product, overflow, num1, num2 );
				reg_verify( ( u64* ) &r_after );
				Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
				Assert::AreEqual( s16( 0 ), ret2, L"Return code failed random 64 test." ); // Only exception possible is parameter alignment

				// Compare results to expected (aborts test if they don't match)
				for ( int j = 0; j < 8; j++ )
				{
					if ( expectedproduct [ j ] != product [ j ] ) {
						Logger::WriteMessage( ToString( j ).c_str( ) );
					};
					Assert::AreEqual( expectedproduct [ j ], product [ j ], _MSGW( L"Product at word #" << j << " failed on run #" << i ) );
					if ( expectedoverflow [ j ] != overflow [ j ] ) {
						Logger::WriteMessage( ToString( j ).c_str( ) );
					};
					Assert::AreEqual( expectedoverflow [ j ], overflow [ j ], _MSGW( L"Overflow at word #" << j << " failed on run #" << i ) );
				}
			};

			{
				string test_message = _MSGA( "Fourth test. Multiply by sums of random powers of two, building ""expected"" 64 bit only; " << test_run_count << " times, each with pseudo random values.\n" );
				Logger::WriteMessage( test_message.c_str( ) );
				Logger::WriteMessage( L"Passed. Tested expected values, return value, and volatile register integrity: all via assert.\n\n" );
			}

			// Fifth test, a multiply by sums of random powers of two into a 512 bit value. 
			// Building "expected" is a bit more complicated. 
			for ( int i = 0; i < test_run_count; i++ )
			{
				bool rbits [ 512 ];
				std::memset( rbits, 0, sizeof( rbits ) );

				for ( int j = 0; j < 510; j++ )
				{
					rbits [ j ] = ( RandomU64( &seed ) % 2 ) == 1;
				};

				RandomFill( num1, &seed );

				// build multiplier as 'N' number of random bits (avoid duplicate bits)
				// simultaneously build expected result and overflow

				u16 nrBits = ( RandomU64( &seed ) % 128 ) + 2; //  Nr Bits somewhere between 2 and 510? 
				for ( int j = 0; j < nrBits; j++ )
				{
					zero_u( num2 );
					zero_u( expectedproduct );
					zero_u( expectedoverflow );
					zero_u( intermediateprod );
					zero_u( intermediateovrf );

					// use selected bit to build a random number (through shift/or)
					// then build expected result of multiply (through shift / add)
					bool found = false;
					u16 nrShift = 0;
					while ( ( nrShift == 0 ) && !found )
					{
						u16 rwrd = RandomU64( &seed ) % 8;
						u16 rBit = RandomU64( &seed ) % 64;
						for ( u16 idx = rwrd * 64 + rBit; idx < 512; idx++ )
						{
							u16 wt = idx / 64;
							u16 bt = idx % 64;
							u64 msk = 1ull << bt;
							if ( rbits [ idx ] && !( num2 [ wt ] & msk ) )
							{
								nrShift = idx;
								found = true;
								break;
							}
						};
						if ( !found )
						{
							break;
						};
					};

					if ( found )
					{
						// Multiplier:
						set_uT64( num3, 1ull );
						shl_u( num3, num3, nrShift );
						or_u( num2, num2, num3 );

						// Calculate expected product and overflow:
						shl_u( intermediateprod, num1, nrShift );
						add_u( expectedproduct, expectedproduct, intermediateprod );
						shr_u( intermediateovrf, num1, 512 - nrShift );
						add_u( expectedoverflow, expectedoverflow, intermediateovrf );
					};
				};

				// Got a random multiplicand and multiplier. Execute function under test
				reg_verify( ( u64* ) &r_before );
				s16 ret = mult_u( product, overflow, num1, num2 );
				reg_verify( ( u64* ) &r_after );
				Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
				Assert::AreEqual( s16( 0 ), ret, L"Return code failed zero times zero test." ); // Only exception possible is parameter alignment

				// Compare results to expected (aborts test if they don't match)
				for ( int j = 0; j < 8; j++ )
				{
					if ( expectedproduct [ j ] != product [ j ] ) {
						Logger::WriteMessage( ToString( j ).c_str( ) );
					};
					Assert::AreEqual( expectedproduct [ j ], product [ j ], _MSGW( L"Product at word #" << j << " failed on run #" << i ) );
					if ( expectedoverflow [ j ] != overflow [ j ] ) {
						Logger::WriteMessage( ToString( j ).c_str( ) );
					};
					Assert::AreEqual( expectedoverflow [ j ], overflow [ j ], _MSGW( L"Overflow at word #" << j << " failed on run #" << i ) );
				}
			};
			{
				string test_message = _MSGA( "Fifth test. Multiply by sums of random powers of two, building ""expected"" full 512 bit; " << test_run_count << " times, each with pseudo random values.\n" );
				Logger::WriteMessage( test_message.c_str( ) );
				Logger::WriteMessage( L"Passed. Tested expected values, return value, and volatile register integrity: each via assert.\n\n" );
			};
		};


		TEST_METHOD( ui512md_01_mul_performance_timing )
		{
			// Performance timing tests.
			// Ref: "Essentials of Modern Business Statistics", 7th Ed, by Anderson, Sweeney, Williams, Camm, Cochran. South-Western, 2015
			// Sections 3.2, 3.3, 3.4
			// Note: these tests are not pass/fail, they are informational only

			Logger::WriteMessage( L"Multiply function performance timing test.\n\n" );

			Logger::WriteMessage( L"First run.\n" );
			perf_stats No1 = Perf_Test_Parms [ 0 ];
			ui512_Unit_Tests::RunStats( &No1, Mul );

			Logger::WriteMessage( L"Second run.\n" );
			perf_stats No2 = Perf_Test_Parms [ 1 ];
			RunStats( &No2, Mul );

			Logger::WriteMessage( L"Third run.\n" );
			perf_stats No3 = Perf_Test_Parms [ 2 ];
			RunStats( &No3, Mul );

		};

		TEST_METHOD( ui512md_02_mul64 )
		{
			// mult_uT64 tests
			// multistage testing, part for use as debugging, progressively "real" testing
			// Note: the ui512a module must pass testing before these tests, as adds are used in this test
			// Note: the ui512b module must pass testing before these tests, as 'or' and shifts are used in this test

			u64 seed = 0;
			regs r_before {};
			regs r_after {};
			_UI512( num1 ) { 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul };

			_UI512( product ) { 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul };
			_UI512( product2 ) { 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul };
			_UI512( intermediateprod ) { 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul };
			_UI512( expectedproduct ) { 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul };
			_UI512( overflow2 ) { 0 };
			u64 num2 = 0;
			u64 num3 = 0;
			u64 overflow = 0;
			u64 intermediateovrf = 0;
			u64 expectedoverflow = 0;

			// Code walk-thru tests. Curated data to validate in-lane splits, shifts, and singular multiply operations 
			// during code walk-thru
			{
				_UI512( num2 ) { 0 };
				_UI512( testn )
				{
					0x0000000200000002ul, 0x0000000300000004ul, 0x0000000500000006ul, 0x0000000700000008ul,
						0x000000090000000Aul, 0x0000000B0000000Cul, 0x00000000000000Eul, 0x0000000F00000010ul
				};
				u64 testm = 0x0000000200000001ul;
				s16 ret = mult_uT64( product, &overflow, testn, testm );

				set_uT64( num2, testm );
				mult_u( product2, overflow2, testn, num2 );

				for ( int j = 0; j < 8; j++ )
				{
					Assert::AreEqual( product2 [ j ], product [ j ], _MSGW( L"Product at word #" << j << " failed zero times zero" ) );
				};

				Assert::AreEqual( overflow2 [ 7 ], overflow, _MSGW( L"Overflow failed " ) );
			}
			// Edge case tests
			// 
			// 1. zero times zero
			{
				zero_u( num1 );
				zero_u( expectedproduct );
				num2 = 0;
				expectedoverflow = 0;
				reg_verify( ( u64* ) &r_before );
				s16 ret = mult_uT64( product, &overflow, num1, num2 );
				reg_verify( ( u64* ) &r_after );
				Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
				Assert::AreEqual( s16( 0 ), ret, L"Return code failed zero times zero test." );
				Assert::AreEqual( expectedoverflow, overflow, _MSGW( L"Overflow failed zero times zero" ) );
				for ( int j = 0; j < 8; j++ )
				{
					Assert::AreEqual( expectedproduct [ j ], product [ j ], _MSGW( L"Product at word #" << j << " failed zero times zero" ) );
				};
			}

			// 2. zero times random
			for ( int i = 0; i < test_run_count; i++ )
			{
				zero_u( num1 );
				zero_u( expectedproduct );
				num2 = RandomU64( &seed );
				expectedoverflow = 0;
				reg_verify( ( u64* ) &r_before );
				s16 ret = mult_uT64( product, &overflow, num1, num2 );
				reg_verify( ( u64* ) &r_after );
				Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
				Assert::AreEqual( s16( 0 ), ret, L"Return code failed zero times random test." );
				Assert::AreEqual( expectedoverflow, overflow, _MSGW( L"Overflow  failed zero times random " << i ) );
				for ( int j = 0; j < 8; j++ )
				{
					Assert::AreEqual( expectedproduct [ j ], product [ j ], _MSGW( L"Product at word #" << j << " failed zero times random on run #" << i ) );
				};
			};

			// 3. random times zero
			for ( int i = 0; i < test_run_count; i++ )
			{
				num2 = 0;
				RandomFill( num1, &seed );
				zero_u( expectedproduct );
				expectedoverflow = 0;
				reg_verify( ( u64* ) &r_before );
				s16 ret = mult_uT64( product, &overflow, num1, num2 );
				reg_verify( ( u64* ) &r_after );
				Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
				Assert::AreEqual( s16( 0 ), ret, L"Return code failed random times zero test." );
				Assert::AreEqual( expectedoverflow, overflow, _MSGW( L"Overflow failed random times zero " << i ) );
				for ( int j = 0; j < 8; j++ )
				{
					Assert::AreEqual( expectedproduct [ j ], product [ j ], _MSGW( L"Product at word #" << j << " failed random times zero on run #" << i ) );
				};
			};

			// 4. one times random
			for ( int i = 0; i < test_run_count; i++ )
			{
				set_uT64( num1, 1ull );
				num2 = RandomU64( &seed );
				set_uT64( expectedproduct, num2 );
				expectedoverflow = 0;
				reg_verify( ( u64* ) &r_before );
				s16 ret = mult_uT64( product, &overflow, num1, num2 );
				reg_verify( ( u64* ) &r_after );
				Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
				Assert::AreEqual( s16( 0 ), ret, L"Return code failed one times random test." );
				Assert::AreEqual( expectedoverflow, overflow, _MSGW( L"Overflow failed one times random on run #" << i ) );
				for ( int j = 0; j < 8; j++ )
				{
					Assert::AreEqual( expectedproduct [ j ], product [ j ], _MSGW( L"Product at word #" << j << " failed one times random on run #" << i ) );
				};
			};

			// 5. random times one
			for ( int i = 0; i < test_run_count; i++ )
			{
				num2 = 1ull;
				RandomFill( num1, &seed );
				copy_u( expectedproduct, num1 );
				expectedoverflow = 0;
				reg_verify( ( u64* ) &r_before );
				s16 ret = mult_uT64( product, &overflow, num1, num2 );
				reg_verify( ( u64* ) &r_after );
				Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
				Assert::AreEqual( s16( 0 ), ret, L"Return code failed random times one test." );
				Assert::AreEqual( expectedoverflow, overflow, _MSGW( L"Overflow failed random times one on run #" << i ) );
				for ( int j = 0; j < 8; j++ )
				{
					Assert::AreEqual( expectedproduct [ j ], product [ j ], _MSGW( L"Product at word #" << j << " failed random times one on run #" << i ) );
				};
			};

			{
				string test_message = _MSGA( "Multiply (x64) function testing.\n\nEdge cases:\n\tzero times zero,\n\tzero times random,\n\trandom times zero,\n\tone times random,\n\trandom times one.\n"
					<< test_run_count << " times each, with pseudo random values.\n" );
				Logger::WriteMessage( test_message.c_str( ) );
				Logger::WriteMessage( L"Passed. Tested expected values, return value, and volatile register integrity: each via assert.\n\n" );
			};

			// Now the real tests, progressively more complex
			// First test, a simple multiply by two. 
			// Easy to check as the expected answer is a shift left,
			// and expected overflow is a shift right
			for ( int i = 0; i < test_run_count; i++ )
			{
				for ( int j = 0; j < 8; j++ )
				{
					num1 [ j ] = RandomU64( &seed );
				};

				num2 = 2;
				shl_u( expectedproduct, num1, u16( 1 ) );
				expectedoverflow = num1 [ 0 ] >> 63;
				reg_verify( ( u64* ) &r_before );
				s16 ret = mult_uT64( product, &overflow, num1, num2 );
				reg_verify( ( u64* ) &r_after );
				Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
				Assert::AreEqual( s16( 0 ), ret, L"Return code failed random times one test." );
				Assert::AreEqual( expectedoverflow, overflow, _MSGW( L"Overflow failed " << i ) );
				for ( int j = 0; j < 8; j++ )
				{
					Assert::AreEqual( expectedproduct [ j ], product [ j ], _MSGW( L"Product at word #" << j << " failed on ru #" << i ) );
				};
			};

			string runmsg1 = "Multiply (u64) function testing. First test. Simple multiply by 2 " +
				to_string( test_run_count ) + " times, each with pseudo random values.\n";;
			Logger::WriteMessage( runmsg1.c_str( ) );
			Logger::WriteMessage( L"Passed. Tested expected values, return value, and volatile register integrity: each via assert.\n\n" );

			// Second test, a simple multiply by sequential powers of two. 
			// Still relatively easy to check as expected answer is a shift left,
			// and expected overflow is a shift right
			for ( u16 nrShift = 0; nrShift < 64; nrShift++ )	// rather than a random bit, cycle thru all 64 bits 
			{
				for ( int i = 0; i < test_run_count / 64; i++ )
				{
					RandomFill( num1, &seed );
					num2 = 1ull << nrShift;
					shl_u( expectedproduct, num1, nrShift );
					expectedoverflow = ( nrShift == 0 ) ? 0 : num1 [ 0 ] >> ( 64 - nrShift );
					reg_verify( ( u64* ) &r_before );
					s16 ret = mult_uT64( product, &overflow, num1, num2 );
					reg_verify( ( u64* ) &r_after );
					Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
					Assert::AreEqual( s16( 0 ), ret, L"Return code failed random times one test." );
					for ( int j = 0; j < 8; j++ )
					{
						Assert::AreEqual( expectedproduct [ j ], product [ j ], _MSGW( L"Product at word #" << j << " failed shift: " << nrShift << " on run #" << i ) );
					}
					Assert::AreEqual( expectedoverflow, overflow, _MSGW( L"Overflow failed shift: " << nrShift << " on run #" << i ) );
				};
			}

			string runmsg2 = "Multiply (u64) function testing. Second test. Multiply by sequential powers of 2 "
				+ to_string( test_run_count ) + " times, each with pseudo random values.\n";
			Logger::WriteMessage( runmsg2.c_str( ) );
			Logger::WriteMessage( L"Passed. Tested expected values, return value, and volatile register integrity: each via assert.\n\n" );

			// Third test, a multiply by random sums of powers of two. 
			// Building "expected" is a bit more complicated
			const u16 nrBits = 24;
			u16 BitsUsed [ nrBits ] = { 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul, 0ul };;
			for ( int i = 0; i < test_run_count; i++ )
			{
				u16 bitcnt = 0;
				fill_n( BitsUsed, nrBits, 0 );

				for ( int j = 0; j < 8; j++ )
				{
					num1 [ j ] = RandomU64( &seed );
					product [ j ] = 0;
					expectedproduct [ j ] = 0;
				};

				expectedoverflow = 0;
				overflow = 0;
				num2 = 0;
				num3 = 0;

				// build multiplier as 'N' number of random bits (avoid duplicate bits)
				// simultaneously build expected result

				u16 nrShift = 0;
				for ( int j = 0; j < nrBits; j++ )
				{
					bool nubit = false;
					zero_u( intermediateprod );
					intermediateovrf = 0;

					// Find a bit (0 -> 63) that hasn't already been used in this random number
					nrShift = RandomU64( &seed ) % 64;
					u16 k = 0;
					while ( !nubit )
					{
						u16 j2 = 0;
						for ( ; j2 < bitcnt; j2++ )
						{
							if ( nrShift == BitsUsed [ j2 ] )
							{
								break;
							}
						};
						if ( nrShift == BitsUsed [ j2 ] )
						{
							nrShift = RandomU64( &seed ) % 64;
						}
						else
						{
							nubit = true;
							BitsUsed [ bitcnt++ ] = nrShift;
						};
					};

					// use selected bit to build a random number (through shift/add)
					// then build / project expected result of multiply (also through shift / add)

					// Multiplier:
					num3 = 1ull << nrShift;
					num2 += num3;

					// Expected:
					shl_u( intermediateprod, num1, nrShift );
					s32 carry = add_u( expectedproduct, expectedproduct, intermediateprod );
					intermediateovrf = ( nrShift == 0 ) ? 0 : num1 [ 0 ] >> ( 64 - nrShift );
					expectedoverflow += intermediateovrf;
					if ( carry != 0 )
					{
						expectedoverflow++;
					};
				};

				reg_verify( ( u64* ) &r_before );
				s16 ret = mult_uT64( product, &overflow, num1, num2 );
				reg_verify( ( u64* ) &r_after );
				Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
				Assert::AreEqual( s16( 0 ), ret, L"Return code failed random times one test." );
				// Now compare results
				for ( int j = 0; j < 8; j++ )
				{
					Assert::AreEqual( expectedproduct [ j ], product [ j ], _MSGW( L"Product at word #" << j << " failed on run #" << i << " num2: " << num2 ) );
				}
				Assert::AreEqual( expectedoverflow, overflow, _MSGW( L"Overflow failed " << i ) );
			};
			{
				string test_message = _MSGA( "Multiply (x64) function testing. Third test. Multiply by sums of random powers of two, building ""expected""; "
					<< test_run_count << " times, each with pseudo random values.\n" );
				Logger::WriteMessage( test_message.c_str( ) );
				Logger::WriteMessage( L"Passed. Tested expected values, return value, and volatile register integrity: each via assert.\n\n" );
			}
		};

		TEST_METHOD( ui512md_02_mul64_performance_timing )
		{
			// Performance timing tests.
			// Ref: "Essentials of Modern Business Statistics", 7th Ed, by Anderson, Sweeney, Williams, Camm, Cochran. South-Western, 2015
			// Sections 3.2, 3.3, 3.4
			// Note: these tests are not pass/fail, they are informational only

			Logger::WriteMessage( L"Multiply x64 function performance timing test.\n\n" );

			Logger::WriteMessage( L"First run.\n" );
			perf_stats No1 = Perf_Test_Parms [ 0 ];
			ui512_Unit_Tests::RunStats( &No1, Mul64 );

			Logger::WriteMessage( L"Second run.\n" );
			perf_stats No2 = Perf_Test_Parms [ 1 ];
			RunStats( &No2, Mul64 );

			Logger::WriteMessage( L"Third run.\n" );
			perf_stats No3 = Perf_Test_Parms [ 2 ];
			RunStats( &No3, Mul64 );
		};

	};	// test_class
};	// namespace