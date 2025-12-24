//		ui512_unit_tests_clear_copy_set
// 
//		File:			ui512_unit_tests_clear_copy_set.cpp
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
	TEST_CLASS( ui512_unit_tests_clear_copy_set )
	{
		TEST_METHOD( ui512_01_zero )
		{
			// Test zero function. Set var to some random number, call function. Test to see if zero. Repeat. 
			u64 seed = 0;
			_UI512( num1 ) { 0, 0, 0, 0, 0, 0, 0, 0 };

			for ( int i = 0; i < test_run_count; i++ )
			{
				RandomFill( num1, &seed );
				string m = "\tZero field at=0x%llx\n";
				const char* mc = "\tZero field at=0x%llx\n";
				zero_u( num1 );
				for ( int j = 0; j < 8; j++ )
				{
					Assert::AreEqual( 0ull, num1 [ j ] );
				};
			};

			string runmsg = "Zero function testing. Ran tests " + to_string( test_run_count ) + " times, each with pseudo random values.\n";
			Logger::WriteMessage( runmsg.c_str( ) );
			Logger::WriteMessage( L"Passed. Tested expected values via assert.\n" );
		};

		TEST_METHOD( ui512_01_zero_performance )
		{
			// Zero function timing. Run function a bunch of times. See if coding changes improve/reduce overall timing.
			// Eliminate everything possible except just repeatedly calling the function.
			// I'm not using chrono timing here as the point isn't to get absolute timing, but differences in bulk timing, 
			// for example over 10 million runs is it faster slower with this change or not? if run as "Z" is it faster than "Q"?
			u64 seed = 0;
			_UI512( num1 ) { };
			RandomFill( num1, &seed );
			u64 cycles = 0;
			u64 start = __rdtsc( );
			for ( int i = 0; i < timing_count; i++ )
			{
				zero_u( num1 );
			};
			cycles = __rdtsc( ) - start;
			string runmsg = "Zero function timing. Ran " + to_string( timing_count ) + " times.\n";
			Logger::WriteMessage( runmsg.c_str( ) );
			Logger::WriteMessage( std::format( "Cycles per call: {:.2f}\n", double( cycles ) / timing_count ).c_str( ) );
		};

		TEST_METHOD( ui512_01_zero_reg )
		{
			// Zero function register verification.
			//	Save registers before call, verify non-volatile registers remain unchanged after call.
			regs r_before {};
			regs r_after {};
			u64 seed = 0;
			_UI512( num1 ) { };

			for ( int i = 0; i < reg_verification_count; i++ )
			{
				RandomFill( num1, &seed );
				r_before.Clear( );
				reg_verify( ( u64* ) &r_before );
				zero_u( num1 );
				r_after.Clear( );
				reg_verify( ( u64* ) &r_after );
				Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
			};

			string runmsg = "Zero function register validation. Ran " + to_string( reg_verification_count ) + " times.\n";
			Logger::WriteMessage( runmsg.c_str( ) );
		};

		TEST_METHOD( ui512_02_copy )
		{
			// Test copy function. Set var to some random number, copy it to another (zeroed) var. Test second var to see if equal. Repeat.
			u64 seed = 0;
			_UI512( num1 ) { 0, 0, 0, 0, 0, 0, 0, 0 };
			_UI512( num2 ) { 0, 0, 0, 0, 0, 0, 0, 0 };

			for ( int i = 0; i < test_run_count; i++ )
			{
				for ( int j = 0; j < 8; j++ )
				{
					num1 [ j ] = RandomU64( &seed );
					num2 [ j ] = 0;
				};

				copy_u( num2, num1 );
				for ( int j = 0; j < 8; j++ )
				{
					Assert::AreEqual( num1 [ j ], num2 [ j ] );
				};
			};

			string runmsg = "Copy function testing. Ran tests " + to_string( test_run_count ) + " times, each with pseudo random values.\n";
			Logger::WriteMessage( runmsg.c_str( ) );
			Logger::WriteMessage( L"Passed. Tested expected values via assert.\n" );
		};

		TEST_METHOD( ui512_02_copy_performance )
		{
			// Copy function timing. Run function a bunch of times. See if coding changes improve/reduce overall timing.
			// Eliminate everything possible except just repeatedly calling the function.
			u64 seed = 0;
			_UI512( num1 ) { };
			RandomFill( num1, &seed );
			alignas ( 64 ) u64 num2 [ 8 ] { 8, 7, 6, 5, 4, 3, 2, 1 };

			u64 cycles = 0;
			u64 start = __rdtsc( );
			for ( int i = 0; i < timing_count; i++ )
			{
				copy_u( num2, num1 );
			};
			cycles = __rdtsc( ) - start;
			string runmsg = "Copy function timing. Ran " + to_string( timing_count ) + " times.\n";
			Logger::WriteMessage( runmsg.c_str( ) );
			Logger::WriteMessage( std::format( "Cycles per call: {:.2f}\n", double( cycles ) / timing_count ).c_str( ) );
		};

		TEST_METHOD( ui512_02_copy_reg )
		{
			// Copy function register verification.
			//	Check register before call, verify non-volatile register remain unchanged after call.
			regs r_before {};
			regs r_after {};
			u64 seed = 0;
			_UI512( num1 ) { };
			_UI512( num2 ) { };
			for ( int i = 0; i < reg_verification_count; i++ )
			{
				RandomFill( num1, &seed );
				RandomFill( num2, &seed );
				r_before.Clear( );
				reg_verify( ( u64* ) &r_before );
				copy_u( num1, num2 );
				r_after.Clear( );
				reg_verify( ( u64* ) &r_after );
				Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
			};

			string runmsg = "Copy function register validation. Ran " + to_string( reg_verification_count ) + " times.\n";
			Logger::WriteMessage( runmsg.c_str( ) );
		};

		TEST_METHOD( ui512_03_set64 )
		{
			// Test set function. Set var to some random number, set it to some random (64bit) var. Test first var to see if equal. Repeat.
			u64 seed = 0;
			_UI512( num1 ) { 0, 0, 0, 0, 0, 0, 0, 0 };
			u64 val = 0;

			for ( int i = 0; i < test_run_count; i++ )
			{
				RandomFill( num1, &seed );
				val = RandomU64( &seed );
				set_uT64( num1, val );

				for ( int j = 0; j < 7; j++ )
				{
					Assert::AreEqual( ( u64 ) 0, num1 [ j ] );
				};

				Assert::AreEqual( val, num1 [ 7 ] );
			};
			string runmsg = "Set value function testing. Ran tests " + to_string( test_run_count ) + " times, each with pseudo random values.\n";
			Logger::WriteMessage( runmsg.c_str( ) );
			Logger::WriteMessage( L"Passed. Tested expected values via assert.\n" );
		};

		TEST_METHOD( ui512_03_set64_performance )
		{
			// Set function timing. Run function a bunch of times. See if coding changes improve/reduce overall timing.
			// Eliminate everything possible except just repeatedly calling the function.
			u64 seed = 0;
			_UI512( num1 ) { };
			RandomFill( num1, &seed );
			u64 val = RandomU64( &seed );
			u64 cycles = 0;
			u64 start = __rdtsc( );
			for ( int i = 0; i < timing_count; i++ )
			{
				set_uT64( num1, val );
			};
			cycles = __rdtsc( ) - start;
			string runmsg = "Set value (x64) function timing. Ran " + to_string( timing_count ) + " times.\n";
			Logger::WriteMessage( runmsg.c_str( ) );
			Logger::WriteMessage( std::format( "Cycles per call: {:.2f}\n", double( cycles ) / timing_count ).c_str( ) );
		};

		TEST_METHOD( ui512_03_set64_reg )
		{
			// Set64 function register verification.
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
				set_uT64( num1, val );
				r_after.Clear( );
				reg_verify( ( u64* ) &r_after );
				Assert::IsTrue( r_before.AreEqual( &r_after ), L"Register validation failed" );
			};

			string runmsg = "Set64 function register validation. Ran " + to_string( reg_verification_count ) + " times.\n";
			Logger::WriteMessage( runmsg.c_str( ) );
		};

	};	// test_class
};	// namespace
