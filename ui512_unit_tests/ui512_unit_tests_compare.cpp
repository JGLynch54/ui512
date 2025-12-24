//		ui512_unit_tests_compare
// 
//		File:			ui512_unit_tests_compare.cpp
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
	TEST_CLASS( ui512_unit_tests_compare )
	{
		TEST_METHOD( ui512_01_compare )
		{
			u64 seed = 0;
			s16 eval;
			_UI512( num1 ) { 0, 0, 0, 0, 0, 0, 0, 0 };
			_UI512( num2 ) { 0, 0, 0, 0, 0, 0, 0, 0 };

			for ( int i = 0; i < test_run_count; i++ )
			{
				for ( int j = 0; j < 8; j++ )
				{
					num1 [ j ] = num2 [ j ] = RandomU64( &seed );
				};

				for ( int j = 0; j <= 7; j++ )
				{
					num2 [ j ]++;
					eval = compare_u( num1, num2 );		// num1 < num2
					if ( ( s16 ) -1 != eval )
					{
						Logger::WriteMessage( ToString( j ).c_str( ) );
					}
					Assert::AreEqual( ( s16 ) -1, eval );
					num2 [ j ] -= 2;
					eval = compare_u( num1, num2 );		// num1 > num2
					Assert::AreEqual( ( s16 ) 1, eval );
					num2 [ j ]++;
					eval = compare_u( num1, num2 );		// num1 == num2
					Assert::AreEqual( ( s16 ) 0, eval );
				};
			};

			string runmsg = "Compare function testing. Ran tests " + to_string( 3 * 8 * test_run_count ) + " times, each with pseudo random values.\n";
			Logger::WriteMessage( runmsg.c_str( ) );
			Logger::WriteMessage( L"Passed. Tested expected values via assert.\n" );
		};

		TEST_METHOD( ui512_01_compare_performance )
		{
			// Performance timing tests.
			// Ref: "Essentials of Modern Business Statistics", 7th Ed, by Anderson, Sweeney, Williams, Camm, Cochran. South-Western, 2015
			// Sections 3.2, 3.3, 3.4
			// Note: these tests are not pass/fail, they are informational only

			Logger::WriteMessage( L"Compare function performance timing test.\n\n" );

			Logger::WriteMessage( L"First run.\n" );
			perf_stats No1 = Perf_Test_Parms [ 0 ];
			ui512_Unit_Tests::RunStats( &No1, Comp );

			Logger::WriteMessage( L"Second run.\n" );
			perf_stats No2 = Perf_Test_Parms [ 1 ];
			RunStats( &No2, Comp );

			Logger::WriteMessage( L"Third run.\n" );
			perf_stats No3 = Perf_Test_Parms [ 2 ];
			RunStats( &No3, Comp );
		};

		TEST_METHOD( ui512_01_compare_reg )
		{
			// Compare function register verification.
			//	Check register before call, verify non-volatile register remain unchanged after call.
			regs r_before {};
			regs r_after {};
			u64 seed = 0;
			_UI512( num1 ) { };
			_UI512( num2 ) { };
			for ( int i = 0; i < reg_verification_count; i++ )
			{
				for ( int j = 0; j < 8; j++ )
				{
					num1 [ j ] = RandomU64( &seed );
					num2 [ j ] = RandomU64( &seed );
				};
				u64 val = RandomU64( &seed );
				r_before.Clear( );
				reg_verify( ( u64* ) &r_before );
				s16 result = compare_u( num1, num2 );
				r_after.Clear( );
				reg_verify( ( u64* ) &r_after );
				Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
			};

			string runmsg = "Compare function register validation. Ran " + to_string( reg_verification_count ) + " times.\n";
			Logger::WriteMessage( runmsg.c_str( ) );
		};

		TEST_METHOD( ui512_02_compare64 )
		{
			u64 seed = 0;
			_UI512( num1 ) { 0, 0, 0, 0, 0, 0, 0, 8 };
			// test that higher order words are checked
			u64 num2 = 8;
			for ( int i = 0; i < 7; i++ )
			{
				num1 [ i ] = RandomU64( &seed );
				s32 eval;
				eval = compare_uT64( num1, num2 );
				Assert::AreEqual( 1, eval );
			};

			for ( int i = 0; i < test_run_count; i++ )
			{
				alignas ( 64 ) u64 num [ 8 ] { 0, 0, 0, 0, 0, 0, 0, 0 };
				num [ 7 ] = num2 = RandomU64( &seed );
				for ( int j = 0; j <= 7; j++ )
				{
					s32 eval;
					num2++;
					eval = compare_uT64( num, num2 );
					Assert::AreEqual( -1, eval );
					num2 -= 2;
					eval = compare_uT64( num, num2 );
					Assert::AreEqual( 1, eval );
					num2++;
					eval = compare_uT64( num, num2 );
					Assert::AreEqual( 0, eval );
				};
			};

			string runmsg = "Compare (x64) function testing. Ran tests " + to_string( 3 * 8 * test_run_count ) + " times, each with pseudo random values.\n";
			Logger::WriteMessage( runmsg.c_str( ) );
			Logger::WriteMessage( L"Passed. Tested expected values via assert.\n" );
		};

		TEST_METHOD( ui512_02_compare64_performance )
		{
			// Performance timing tests.
			// Ref: "Essentials of Modern Business Statistics", 7th Ed, by Anderson, Sweeney, Williams, Camm, Cochran. South-Western, 2015
			// Sections 3.2, 3.3, 3.4
			// Note: these tests are not pass/fail, they are informational only

			Logger::WriteMessage( L"Compare x64 function performance timing test.\n\n" );

			Logger::WriteMessage( L"First run.\n" );
			perf_stats No1 = Perf_Test_Parms [ 0 ];
			ui512_Unit_Tests::RunStats( &No1, Comp64 );

			Logger::WriteMessage( L"Second run.\n" );
			perf_stats No2 = Perf_Test_Parms [ 1 ];
			RunStats( &No2, Comp64 );

			Logger::WriteMessage( L"Third run.\n" );
			perf_stats No3 = Perf_Test_Parms [ 2 ];
			RunStats( &No3, Comp64 );
		};

		TEST_METHOD( ui512_02_compare64_reg )
		{
			// Compare64 function register verification.
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
				s16 result = compare_uT64( num1, val );
				r_after.Clear( );
				reg_verify( ( u64* ) &r_after );
				Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
			};

			string runmsg = "Compare64 function register validation. Ran " + to_string( reg_verification_count ) + " times.\n";
			Logger::WriteMessage( runmsg.c_str( ) );
		};

		TEST_METHOD( ui512_03_compare_edge_cases )
		{
			_UI512( zero ) { 0 };
			_UI512( allones );
			std::fill_n( allones, 8, u64_Max );

			Assert::AreEqual( ( s16 ) 0, compare_u( zero, zero ) );
			Assert::AreEqual( ( s16 ) 0, compare_u( allones, allones ) );
			Assert::AreEqual( ( s16 ) 1, compare_u( allones, zero ) );
			Assert::AreEqual( ( s16 ) -1, compare_u( zero, allones ) );

			Assert::AreEqual( ( s16 ) -1, compare_uT64( zero, 1 ) );
			Assert::AreEqual( ( s16 ) 1, compare_uT64( allones, 0 ) );
			Assert::AreEqual( ( s16 ) 0, compare_uT64( zero, 0 ) );
			Logger::WriteMessage( L"Edge case testing.\n\n" );
			Logger::WriteMessage( L"Compare_U: zero to zero, all ones to all ones, all ones to zero, zero to all ones. Passed.\n" );
			Logger::WriteMessage( L"Compare_UT64: zero to one, all ones to zero, zero to zero. Passed.\n" );
		}

	};	// test_class
};	// namespace