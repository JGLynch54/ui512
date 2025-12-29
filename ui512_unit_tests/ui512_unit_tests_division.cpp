//		ui512_unit_tests_division
// 
//		File:			ui512_unit_tests_division.cpp
//		Author:			John G.Lynch
//		Legal:			Copyright @2024, per MIT License below
//		Date:			November 24, 2025
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
//		This sub - project: ui512_unit_tests_division, is a unit test project that invokes each of the routines in the ui512a assembly.
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
	TEST_CLASS( ui512_unit_tests_division )
	{
		TEST_METHOD( ui512_01_div_pt1 )
		{
			u64 seed = 0;
			regs r_before {};
			regs r_after {};

			_UI512( num1 ) { 0 };
			_UI512( num2 ) { 0 };
			_UI512( dividend ) { 0 };
			_UI512( divisor ) { 0 };
			_UI512( expectedquotient ) { 0 };
			_UI512( expectedremainder ) { 0 };
			_UI512( quotient ) { 0 };
			_UI512( remainder ) { 0 };

			// Edge case tests

			// 1. zero divided by random
			zero_u( dividend );
			for ( int i = 0; i < test_run_count; i++ )
			{
				zero_u( dividend );
				RandomFill( divisor, &seed );
				zero_u( expectedquotient );
				zero_u( expectedremainder );
				reg_verify( ( u64* ) &r_before );
				s16 retcode = div_u( quotient, remainder, dividend, divisor );
				reg_verify( ( u64* ) &r_after );
				Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
				for ( int j = 0; j < 8; j++ )
				{
					Assert::AreEqual( expectedquotient [ j ], quotient [ j ], _MSGW( L"Quotient at word #" << j << " failed zero divided by random on run #" << i ) );
					Assert::AreEqual( expectedremainder [ j ], remainder [ j ], _MSGW( L"Remainder at word #" << j << " failed zero divided by random on run #" << i ) );
				};
				Assert::AreEqual( s16( 0 ), retcode, L"Return code failed zero divided by random" );
			};

			// 2.random divided by zero
			zero_u( dividend );
			for ( int i = 0; i < test_run_count; i++ )
			{
				RandomFill( dividend, &seed );
				zero_u( divisor );
				zero_u( expectedquotient );
				zero_u( expectedremainder );
				reg_verify( ( u64* ) &r_before );
				s16 retcode = div_u( quotient, remainder, dividend, divisor );
				reg_verify( ( u64* ) &r_after );
				Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
				for ( int j = 0; j < 8; j++ )
				{
					Assert::AreEqual( expectedquotient [ j ], quotient [ j ], _MSGW( L"Quotient at word #" << j << " failed random divided by zero on run #" << i ) );
					Assert::AreEqual( expectedremainder [ j ], remainder [ j ], _MSGW( L"Remainder at word #" << j << " failed random divided by zero on run #" << i ) );
				};
				Assert::AreEqual( s16( -1 ), retcode, L"Return code failed random divided by zero" );
			};

			// 3. random divided by one
			for ( int i = 0; i < test_run_count; i++ )
			{
				RandomFill( dividend, &seed );
				set_uT64( divisor, 1 );
				copy_u( expectedquotient, dividend );
				zero_u( expectedremainder );
				reg_verify( ( u64* ) &r_before );
				s16 retcode = div_u( quotient, remainder, dividend, divisor );
				reg_verify( ( u64* ) &r_after );
				Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
				for ( int j = 0; j < 8; j++ )
				{
					Assert::AreEqual( expectedquotient [ j ], quotient [ j ], _MSGW( L"Quotient at word #" << j << " failed random divided by one on run #" << i ) );
					Assert::AreEqual( expectedremainder [ j ], remainder [ j ], _MSGW( L"Remainder at word #" << j << " failed random divided by one on run #" << i ) );
				};
				Assert::AreEqual( s16( 0 ), retcode, L"Return code failed random divided by one" );
			};

			// 4. one divided by random
			for ( int i = 0; i < test_run_count; i++ )
			{
				zero_u( dividend );
				set_uT64( dividend, 1 );
				RandomFill( divisor, &seed );
				zero_u( expectedquotient );
				copy_u( expectedremainder, dividend );
				reg_verify( ( u64* ) &r_before );
				s16 retcode = div_u( quotient, remainder, dividend, divisor );
				reg_verify( ( u64* ) &r_after );
				Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
				Assert::AreEqual( s16( 0 ), retcode, L"Return code failed one divided by random" );
				for ( int j = 0; j < 8; j++ )
				{
					Assert::AreEqual( expectedquotient [ j ], quotient [ j ], _MSGW( L"Quotient at word #" << j << " failed one divided by random on run #" << i ) );
					Assert::AreEqual(expectedremainder[j], remainder[j], _MSGW(L"Remainder at word #" << j << " failed one divided by random on run #" << i));
				};
			};

			// 5. random divided by single word divisor, random bit 0->63
			// expected quotient is a shift right, expected remainder is a shift left
			for ( int i = 0; i < test_run_count; i++ )
			{
				RandomFill( dividend, &seed );
				u16 bitno = RandomU64( &seed ) % 63; // bit 0 to 63
				u64 divby = 1ull << bitno;
				set_uT64( divisor, divby );
				shr_u( expectedquotient, dividend, bitno );
				shl_u( expectedremainder, dividend, 512 - bitno );
				shr_u( expectedremainder, expectedremainder, 512 - bitno );
				reg_verify( ( u64* ) &r_before );
				s16 retcode = div_u( quotient, remainder, dividend, divisor );
				reg_verify( ( u64* ) &r_after );
				Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
				Assert::AreEqual( s16( 0 ), retcode, L"Return code failed one divided by random" );
				for ( int j = 0; j < 8; j++ )
				{
					Assert::AreEqual( expectedquotient [ j ], quotient [ j ], _MSGW( L"Quotient at word #" << j << " failed random divided by one word of random bit during run #" << i ) );
					Assert::AreEqual( expectedremainder [ j ], remainder [ j ], _MSGW( L"Remainder at word #" << j << " failed random divided by one word of random bit during run #" << i ) );
				};
			};

			{
				string test_message = _MSGA( "Divide function testing.\n Edge cases:\n\tzero divided by random,\n\trandom divided by zero,\n\trandom divided by one"
					"\n\tone divided by random,\n\trandom divided by one word of random bit.\n"
					<< test_run_count << " times each, with pseudo random values. \n" );
				Logger::WriteMessage( test_message.c_str( ) );
				Logger::WriteMessage( L"Passed. Non-volatile registers verified. Return code verified. Quotient and remainder verified; each via assert.\n\n" );
			};
		};


		TEST_METHOD( ui512_01_div_pt2 )
		{
			u64 seed = 0;
			_UI512( num1 ) { 0 };
			_UI512( num2 ) { 0 };
			_UI512( dividend ) { 0 };
			_UI512( divisor ) { 0 };
			_UI512( expectedquotient ) { 0 };
			_UI512( expectedremainder ) { 0 };
			_UI512( quotient ) { 0 };
			_UI512( remainder ) { 0 };
			regs r_before {};
			regs r_after {};

			num1[0] = 0;
			num1[1] = 0;
			num1[2] = 0;
			num1[3] = 0;
			num1[4] = 0;
			num1[5] = 4;	//; 0xFFFFFFFFFFFFFFFFull;	// bit 320 to 383 set
			num1[6] = 6;	// 0xFFFFFFFFFFFFFFFFull;	// bit 384 to 447 set
			num1[7] = 4;	// 0xFFFFFFFFFFFFFFFFull;	// bit 448 to 511 set
			num2[0] = 0;
			num2[1] = 0;
			num2[2] = 0;
			num2[3] = 0;
			num2[4] = 0;
			num2[5] = 0;
			num2[6] = 1;	// 0x8FFFFFFFFFFFFFFFull;	// bit 384 to 447 set
			num2[7] = 2;	// 0xFFFFFFFFFFFFFFFFull;	// bit 448 to 511 set
			copy_u(dividend, num1);
			copy_u(divisor, num2);
			zero_u(expectedquotient);
			expectedquotient[5] = 0;// 1;
			expectedquotient[6] = 0;// 0xFFFFFFFFFFFFFFFFull;
			expectedquotient[7] = 2;// 0xFFFFFFFFFFFFFFFFull;
			zero_u(expectedremainder);
			reg_verify((u64*)&r_before);
			s16 retcode = div_u(quotient, remainder, dividend, divisor);
			reg_verify((u64*)&r_after);
			Assert::IsTrue(r_before.AreEqual(&r_after), L"Register validation failed");
			Assert::AreEqual(s16(0), retcode, L"Return code failed specific value test");
			//for (int j = 0; j < 8; j++)
			//{
			//	Assert::AreEqual(expectedquotient[j], quotient[j], _MSGW(L"Quotient at word #" << j << " failed specific value test"));
			//	Assert::AreEqual(expectedremainder[j], remainder[j], _MSGW(L"Remainder at word #" << j << " failed specific value test"));
			//};
			{
				string test_message = _MSGA("Divide function testing.\n Specific value test.\n");
				Logger::WriteMessage(test_message.c_str());
				Logger::WriteMessage(L"Passed. Non-volatile registers verified. Return code verified. Quotient and remainder verified; each via assert.\n\n");
			};





			//	//	//	Pre-test: various sizes of dividend / divisor
			//	//	//	Just to exercise various paths through the code

			//	//	s16 retval = 0;
			//	//	//	Pre-testing, various sizes of dividend / divisor
			//	//	for (int i = 7; i >= 0; i--)
			//	//	{
			//	//		for (int j = 7; j >= 0; j--)
			//	//		{
			//	//			zero_u(dividend);
			//	//			zero_u(divisor);
			//	//			dividend[i] = RandomU64(&seed);
			//	//			divisor[j] = RandomU64(&seed);
			//	//			if ((i == 5 && j == 6) || (i == 6 && j == 7)) {
			//	//				break;
			//	//			}
			//	//			reg_verify((u64*)&r_before);
			//	//			retval = div_u(quotient, remainder, dividend, divisor);
			//	//			reg_verify((u64*)&r_after);
			//	//			Assert::IsTrue(r_before.AreEqual(&r_after), L"Register validation failed");
			//	//		};
			//	//	};

			//	//	// First test, a simple divide by two. 
			//	//	// Easy to check as the expected answer is a shift right,
			//	//	// and expected remainder is a shift left

			//	//	for (int i = 0; i < test_run_count; i++)
			//	//	{
			//	//		RandomFill(dividend, &seed);
			//	//		zero_u(quotient);
			//	//		set_uT64(divisor, 2);
			//	//		shr_u(expectedquotient, dividend, u16(1));
			//	//		shl_u(expectedremainder, dividend, 511);
			//	//		shr_u(expectedremainder, expectedremainder, 511);

			//	//		div_u(quotient, remainder, dividend, divisor);

			//	//		for (int j = 0; j < 8; j++)
			//	//		{
			//	//			Assert::AreEqual(expectedquotient[j], quotient[j], _MSGW(L"Quotient at " << j << " failed " << i));
			//	//			Assert::AreEqual(expectedremainder[j], remainder[j], _MSGW(L"Remainder failed " << i));
			//	//		};
			//	//	};

			//	//	{
			//	//		string test_message = _MSGA("Divide function testing. Simple divide by 2 " << test_run_count << " times, each with pseudo random values.\n");
			//	//		Logger::WriteMessage(test_message.c_str());
			//	//		Logger::WriteMessage(L"Passed. Tested expected values via assert.\n\n");
			//	//	}
			//	//	// Second test, a simple divide by sequential powers of two. 
			//	//	// Still relatively easy to check as expected answer is a shift right,
			//	//	// and expected remainder is a shift left

			//	//	for (u16 nrShift = 0; nrShift < 512; nrShift++)	// rather than a random bit, cycle thru all 64 bits 
			//	//	{
			//	//		for (int i = 0; i < test_run_count / 512; i++)
			//	//		{
			//	//			RandomFill(dividend, &seed);
			//	//			set_uT64(divisor, 1);
			//	//			shl_u(divisor, divisor, nrShift);
			//	//			shr_u(expectedquotient, dividend, nrShift);
			//	//			if (nrShift == 0)
			//	//			{
			//	//				zero_u(expectedremainder);
			//	//			}
			//	//			else
			//	//			{
			//	//				u16 shft = 512 - nrShift;
			//	//				shl_u(expectedremainder, dividend, shft);
			//	//				shr_u(expectedremainder, expectedremainder, shft);
			//	//			}

			//	//			div_u(quotient, remainder, dividend, divisor);

			//	//			for (int j = 0; j < 8; j++)
			//	//			{
			//	//				Assert::AreEqual(expectedquotient[j], quotient[j], _MSGW(L"Quotient at " << j << " failed " << nrShift << " at " << i));
			//	//				Assert::AreEqual(expectedremainder[j], remainder[j], _MSGW(L"Remainder failed at " << j << " on " << nrShift << " at " << i));
			//	//			}

			//	//		};
			//	//	}
			//	//	{
			//	//		string test_message = _MSGA("Divide function testing. Divide by sequential powers of 2 " << test_run_count << " times, each with pseudo random values.\n");
			//	//		Logger::WriteMessage(test_message.c_str());
			//	//		Logger::WriteMessage(L"Passed. Tested expected values via assert.\n\n");
			//	//	}
			//	//	//	Use case testing
			//	//	//		Divide number by common use case examples

			//	//	int adjtest_run_count = test_run_count / 64;
			//	//	for (int i = 0; i < adjtest_run_count; i++)
			//	//	{
			//	//		for (int m = 7; m >= 0; m--)
			//	//		{
			//	//			for (int j = 7; j >= 0; j--)
			//	//			{
			//	//				for (int l = 0; l < 8; l++)
			//	//				{
			//	//					num1[l] = RandomU64(&seed);
			//	//					num2[l] = 0;
			//	//					quotient[l] = 0;
			//	//					remainder[l] = 0;
			//	//				};
			//	//				num2[m] = 1;
			//	//				;
			//	//				div_u(quotient, remainder, num1, num2);

			//	//				for (int v = 7; v >= 0; v--)
			//	//				{
			//	//					int qidx, ridx = 0;
			//	//					u64 qresult, rresult = 0;

			//	//					qidx = v - (7 - m);
			//	//					qresult = (qidx >= 0) ? qresult = (v >= (7 - m)) ? num1[qidx] : 0ull : qresult = 0;
			//	//					rresult = (v > m) ? num1[v] : 0ull;

			//	//					Assert::AreEqual(quotient[v], qresult, L"Quotient incorrect");
			//	//					Assert::AreEqual(remainder[v], rresult, L" Remainder incorrect");
			//	//				};

			//	//				num2[m] = 0;
			//	//			};
			//	//		};
			//	//	};
			{
				string test_message = _MSGA( "Divide function testing. Ran tests " << test_run_count << " times, each with pseudo random values.\n" );
				Logger::WriteMessage( test_message.c_str( ) );
				Logger::WriteMessage( L"Passed. Tested expected values via assert.\n\n" );
			};
		};


		TEST_METHOD( ui512_02_div64 )
		{
			u64 seed = 0;
			regs r_before {};
			regs r_after {};

			_UI512( dividend ) { 0 };
			_UI512( quotient ) { 0 };
			_UI512( expectedquotient ) { 0 };

			u64 divisor = 0;
			u64 remainder = 0;
			u64 expectedremainder = 0;

			// Edge case tests
			// 1. zero divided by random
			zero_u( dividend );
			for ( int i = 0; i < test_run_count; i++ )
			{
				zero_u( dividend );
				divisor = RandomU64( &seed );
				zero_u( expectedquotient );
				expectedremainder = 0;
				reg_verify( ( u64* ) &r_before );
				s16 retcode = div_uT64( quotient, &remainder, dividend, divisor );
				reg_verify( ( u64* ) &r_after );
				Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
				Assert::AreEqual( s16( 0 ), retcode, L"Return code failed one divided by random" );
				for ( int j = 0; j < 8; j++ )
				{
					Assert::AreEqual( expectedquotient [ j ], quotient [ j ],
						_MSGW( L"Quotient at word #" << j << " failed zero divided by random on run #" << i ) );
				};
				Assert::AreEqual( expectedremainder, remainder,
					_MSGW( L"Remainder failed zero divided by random on run #" << i ) );
			};
			// 2. random divided by one
			for ( int i = 0; i < test_run_count; i++ )
			{
				RandomFill( dividend, &seed );
				divisor = 1;
				copy_u( expectedquotient, dividend );
				expectedremainder = 0;
				reg_verify( ( u64* ) &r_before );
				s16 retcode = div_uT64( quotient, &remainder, dividend, divisor );
				reg_verify( ( u64* ) &r_after );
				Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
				Assert::AreEqual( s16( 0 ), retcode, L"Return code failed one divided by random" );
				for ( int j = 0; j < 8; j++ )
				{
					Assert::AreEqual( expectedquotient [ j ], quotient [ j ],
						_MSGW( L"Quotient at word #" << j << " failed random divided by one on run #" << i ) );
				};
				Assert::AreEqual( expectedremainder, remainder,
					_MSGW( L"Remainder failed random divided by one " << i ) );
			};
			// 3. random divided by self
			for ( int i = 0; i < test_run_count; i++ )
			{
				zero_u( dividend );
				dividend [ 7 ] = RandomU64( &seed );
				divisor = dividend [ 7 ];
				set_uT64( expectedquotient, 1 );
				expectedremainder = 0;
				reg_verify( ( u64* ) &r_before );
				s16 retcode = div_uT64( quotient, &remainder, dividend, divisor );
				reg_verify( ( u64* ) &r_after );
				Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
				Assert::AreEqual( s16( 0 ), retcode, L"Return code failed one divided by random" );
				for ( int j = 0; j < 8; j++ )
				{
					Assert::AreEqual( expectedquotient [ j ], quotient [ j ],
						_MSGW( L"Quotient at word #" << j << " failed random divided by self on run #" << i ) );
				};
				Assert::AreEqual( expectedremainder, remainder,
					_MSGW( L"Remainder failed random divided by self " << i ) );
			};
			{
				string test_message = _MSGA( "Divide (u64) function testing.\n\n Edge cases:\n\tzero divided by random,\n\trandom divided by one,\n\trandom divided by self.\n "
					<< test_run_count << " times each, with pseudo random values.\n";);
				Logger::WriteMessage( test_message.c_str( ) );
				Logger::WriteMessage( L"Passed. Non-volatile registers verified. Return code verified. Quotient and remainder verified; each via assert.\n\n" );
			};

			// First test, a simple divide by two. 
			// Easy to check as the expected answer is a shift right,
			// and expected remainder is a shift left

			for ( int i = 0; i < test_run_count; i++ )
			{
				RandomFill( dividend, &seed );
				zero_u( quotient );
				divisor = 2;
				shr_u( expectedquotient, dividend, u16( 1 ) );
				expectedremainder = ( dividend [ 7 ] << 63 ) >> 63;
				reg_verify( ( u64* ) &r_before );
				s16 retcode = div_uT64( quotient, &remainder, dividend, divisor );
				reg_verify( ( u64* ) &r_after );
				Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
				Assert::AreEqual( s16( 0 ), retcode, L"Return code failed one divided by random" );
				for ( int j = 0; j < 8; j++ )
				{
					Assert::AreEqual( expectedquotient [ j ], quotient [ j ],
						_MSGW( L"Quotient at word #" << j << " failed on run #" << i ) );
				};
				Assert::AreEqual( expectedremainder, remainder, _MSGW( L"Remainder failed " << i ) );
			};
			{
				string test_message = _MSGA( "Divide (u64) function testing. Simple divide by 2 "
					<< test_run_count << " times, each with pseudo random values.\n" );
				Logger::WriteMessage( test_message.c_str( ) );
				Logger::WriteMessage( L"Passed. Non-volatile registers verified. Return code verified. Quotient and remainder verified; each via assert.\n\n" );
			};

			// Second test, a simple divide by sequential powers of two. 
			// Still relatively easy to check as expected answer is a shift right,
			// and expected remainder is a shift left

			for ( u16 nrShift = 0; nrShift < 64; nrShift++ )	// rather than a random bit, cycle thru all 64 bits 
			{
				for ( int i = 0; i < test_run_count / 64; i++ )
				{
					RandomFill( dividend, &seed );
					divisor = 1ull << nrShift;
					shr_u( expectedquotient, dividend, nrShift );
					expectedremainder = ( nrShift == 0 ) ? 0 : ( dividend [ 7 ] << ( 64 - nrShift ) ) >> ( 64 - nrShift );
					reg_verify( ( u64* ) &r_before );
					s16 retcode = div_uT64( quotient, &remainder, dividend, divisor );
					reg_verify( ( u64* ) &r_after );
					Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
					Assert::AreEqual( s16( 0 ), retcode, L"Return code failed one divided by random" );
					for ( int j = 0; j < 8; j++ )
					{
						Assert::AreEqual( expectedquotient [ j ], quotient [ j ],
							_MSGW( L"Quotient at word #" << j << " failed shifting: " << nrShift << " on run #" << i ) );
					}
					Assert::AreEqual( expectedremainder, remainder, _MSGW( L"Remainder failed shifting: " << nrShift << " on run #" << i ) );
				};
			}
			{
				string test_message = _MSGA( "Divide function testing. Divide by sequential powers of 2 "
					<< test_run_count << " times, each with pseudo random values.\n" );
				Logger::WriteMessage( test_message.c_str( ) );
				Logger::WriteMessage( L"Passed. Non-volatile registers verified. Return code verified. Quotient and remainder verified; each via assert.\n\n" );
			}

			// Third test, Use case tests, divide out to get decimal digits. Do whole with random,
			// and a knowable sample 
			{
				string digits = "";
				RandomFill( dividend, &seed );
				int comp = compare_uT64( dividend, 0ull );
				int cnt = 0;
				while ( comp != 0 )
				{
					reg_verify( ( u64* ) &r_before );
					s16 retcode = div_uT64( dividend, &remainder, dividend, 10ull );
					reg_verify( ( u64* ) &r_after );
					Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
					Assert::AreEqual( s16( 0 ), retcode, L"Return code failed one divided by random" );
					char digit = 0x30 + char( remainder );
					digits.insert( digits.begin( ), digit );
					comp = compare_uT64( dividend, 0ull );
					if ( comp != 0 )
					{
						cnt++;
						if ( cnt % 30 == 0 )
						{
							digits.insert( digits.begin( ), '\n' );
						}
						else
						{
							if ( cnt % 3 == 0 )
							{
								digits.insert( digits.begin( ), ',' );
							};
						};
					}
				}
				Logger::WriteMessage( L"Use case: Divide to extract decimal digits:\n" );
				Logger::WriteMessage( digits.c_str( ) );
				Logger::WriteMessage( L"\nPassed. Non-volatile registers verified. Return code verified; each via assert.\n" );
			}

			{
				string digits = "";
				u64 num = 12345678910111213ull;
				set_uT64( dividend, num );
				int comp = compare_uT64( dividend, 0ull );
				int cnt = 0;
				while ( comp != 0 )
				{
					reg_verify( ( u64* ) &r_before );
					s16 retcode = div_uT64( dividend, &remainder, dividend, 10ull );
					reg_verify( ( u64* ) &r_after );
					Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
					Assert::AreEqual( s16( 0 ), retcode, L"Return code failed one divided by random" );
					char digit = 0x30 + char( remainder );
					digits.insert( digits.begin( ), digit );
					comp = compare_uT64( dividend, 0ull );
					if ( comp != 0 )
					{
						cnt++;
						if ( cnt % 3 == 0 )
						{
							digits.insert( digits.begin( ), ',' );
						};
					}

				}
				string expected = "12,345,678,910,111,213";
				Assert::AreEqual( expected, digits );
				Logger::WriteMessage( L"\nUse case: Divide to extract known decimal digits:\n" );
				Logger::WriteMessage( digits.c_str( ) );
				Logger::WriteMessage( L"\nPassed.\n\tNon-volatile registers verified.\n\tReturn code verified.\n\tExtracted decimal digits verified.\nEach via assert.\n\n" );
			}
		}

		TEST_METHOD( ui512_02_div64_performance )
		{
			// Performance timing tests.
			// Ref: "Essentials of Modern Business Statistics", 7th Ed, by Anderson, Sweeney, Williams, Camm, Cochran. South-Western, 2015
			// Sections 3.2, 3.3, 3.4
			// Note: these tests are not pass/fail, they are informational only

			Logger::WriteMessage( L"Divide x64 function performance timing test.\n\n" );

			Logger::WriteMessage( L"First run.\n" );
			perf_stats No1 = Perf_Test_Parms [ 0 ];
			ui512_Unit_Tests::RunStats( &No1, Div64 );

			Logger::WriteMessage( L"Second run.\n" );
			perf_stats No2 = Perf_Test_Parms [ 1 ];
			RunStats( &No2, Div64 );

			Logger::WriteMessage( L"Third run.\n" );
			perf_stats No3 = Perf_Test_Parms [ 2 ];
			RunStats( &No3, Div64 );
		};
	};
};
