//		ui512_unit_tests_significance
// 
//		File:			ui512_unit_tests_significance.cpp
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
	TEST_CLASS(ui512_unit_tests_significance)
	{
		TEST_METHOD(ui512bits_01_msb)
		{
			u64 seed = 0;
			s16 bitloc = 0;
			regs r_before{};
			regs r_after{};
			s16 expectedbitloc = 0;
			int adjtest_run_count = test_run_count / 64;
			_UI512(num1) { 0 };
			_UI512(nobit) { 0 };
			_UI512(lowbit) { 0, 0, 0, 0, 0, 0, 0, 1 };
			_UI512(b2bit) { 0, 0, 0, 0, 0, 0, 0, 2 };
			_UI512(highbit) { 0x8000000000000000, 0, 0, 0, 0, 0, 0, 0 };
			_UI512(twobitsdiffwords) { 0, 1, 0, 0, 0, 0, 0, 1 };
			_UI512(b2bitsw) { 0, 0, 0, 0, 0, 0, 0, 17 };
			_UI512(choosebit) { 0, 9, 0, 0, 0, 0x8000000000000000, 0xff, 0 };

			s16 nobitloc = msb_u(nobit);
			Assert::AreEqual(s16(-1), nobitloc);
			{
				string test_message = "Most significant bit function testing. Find 'no bit'. Location returned: " + to_string(nobitloc) + ". Assert validated.\n";
				Logger::WriteMessage(test_message.c_str());
			}

			s16 lowbitloc = msb_u(lowbit);
			Assert::AreEqual(s16(0), lowbitloc);
			{
				string test_message = "Most significant bit function testing. Find 'Low bit'. Location returned: " + to_string(lowbitloc) + ". Assert validated.\n";
				Logger::WriteMessage(test_message.c_str());
			}

			s16 b2bitloc = msb_u(b2bit);
			Assert::AreEqual(s16(1), b2bitloc);
			{
				string test_message = "Most significant bit function testing. Find 'bit two bit'. Location returned: " + to_string(b2bitloc) + ". Assert validated.\n";
				Logger::WriteMessage(test_message.c_str());
			}

			s16 highbitloc = msb_u(highbit);
			Assert::AreEqual(s16(511), highbitloc);
			{
				string test_message = "Most significant bit function testing. Find 'High bit'. Location returned: " + to_string(highbitloc) + ". Assert validated.\n";
				Logger::WriteMessage(test_message.c_str());
			}

			s16 twobitbitloc = msb_u(twobitsdiffwords);  // find / select msb bit of the two from different words
			Assert::AreEqual(s16(384), twobitbitloc);
			{
				string test_message = "Most significant bit function testing. Select msb from two different words. Location returned: " + to_string(twobitbitloc) + ". Assert validated.\n";
				Logger::WriteMessage(test_message.c_str());
			}

			s16 b2bitswloc = msb_u(b2bitsw);	// find / select msb bit of the two from the same word
			Assert::AreEqual(s16(4), b2bitswloc);
			{
				string test_message = "Most significant bit function testing.Select msb from two within a word. Location returned: " + to_string(b2bitswloc) + ". Assert validated.\n";
				Logger::WriteMessage(test_message.c_str());
			}

			s16 choosebitloc = msb_u(choosebit);
			Assert::AreEqual(s16(387), choosebitloc);
			{
				string test_message = "Most significant bit function testing. Choose a bit. Location returned: " + to_string(choosebitloc) + ". Assert validated.\n";
				Logger::WriteMessage(test_message.c_str());
			}

			for (int i = 0; i < adjtest_run_count; i++)
			{
				for (int j = 0; j < 8; j++)
				{
					u64 val = 0x8000000000000000ull;
					for (int k = 0; k < 64; k++)
					{
						num1[j] = val;
						expectedbitloc = 511 - ((j * 64) + k);
						reg_verify((u64*)&r_before);
						bitloc = msb_u(num1);
						reg_verify((u64*)&r_after);
						Assert::AreEqual(expectedbitloc, bitloc);
						Assert::IsTrue(r_before.AreEqual(&r_after), L"Register validation failed");
						val = val >> 1;
					};

					num1[j] = 0;
				};

			};

			string test_message = "Most significant bit function testing. Ran tests " + to_string(test_run_count) + " times, each with shifted values. Every bit location checked.\n";
			Logger::WriteMessage(test_message.c_str());
			Logger::WriteMessage(L"Passed. Register imegrity checked, tested expected values via assert.\n");
		};

		TEST_METHOD(ui512bits_02_msb_performance)
		{
			// Performance timing tests.
			// Ref: "Essentials of Modern Business Statistics", 7th Ed, by Anderson, Sweeney, Williams, Camm, Cochran. South-Western, 2015
			// Sections 3.2, 3.3, 3.4
			// Note: these tests are not pass/fail, they are informational only

			Logger::WriteMessage(L"MSB function performance timing test.\n\n");

			Logger::WriteMessage(L"First run.\n");
			perf_stats No1 = Perf_Test_Parms[0];
			ui512_Unit_Tests::RunStats(&No1, msb);

			Logger::WriteMessage(L"Second run.\n");
			perf_stats No2 = Perf_Test_Parms[1];
			RunStats(&No2, msb);

			Logger::WriteMessage(L"Third run.\n");
			perf_stats No3 = Perf_Test_Parms[2];
			RunStats(&No3, msb);
		};

		TEST_METHOD(ui512bits_03_lsb)
		{
			_UI512(num1) { 0 };
			_UI512(nobit) { 0 };
			_UI512(lowbit) { 0, 0, 0, 0, 0, 0, 0, 1 };
			_UI512(b2bit) { 0, 0, 0, 0, 0, 0, 0, 2 };
			_UI512(highbit) { 0x8000000000000000, 0, 0, 0, 0, 0, 0, 0 };
			_UI512(twobitsdiffwords) { 0, 1, 0, 0, 0, 0, 0, 1 };
			_UI512(b2bitsw) { 0, 0, 0, 0, 0, 0, 0, 17 };
			_UI512(choosebit) { 0, 9, 0, 0, 0, 0x8000000000000000, 0xff, 0 };
			u64 seed = 0;
			regs r_before{};
			regs r_after{};
			s16 bitloc = 0;
			s16 expectedbitloc = 0;
			int adjtest_run_count = test_run_count / 64;

			s16 nobitloc = lsb_u(nobit);
			Assert::AreEqual(s16(-1), nobitloc);
			{
				string test_message = "Least significant bit function testing. Find 'no bit'. Location returned: " + to_string(nobitloc) + ". Assert validated.\n";
				Logger::WriteMessage(test_message.c_str());
			}

			s16 lowbitloc = lsb_u(lowbit);
			Assert::AreEqual(s16(0), lowbitloc);
			{
				string test_message = "Least significant bit function testing. Find 'Low bit'. Location returned: " + to_string(lowbitloc) + ". Assert validated.\n";
				Logger::WriteMessage(test_message.c_str());
			}

			s16 b2bitloc = lsb_u(b2bit);
			Assert::AreEqual(s16(1), b2bitloc);
			{
				string test_message = "Least significant bit function testing. Find 'Bit 2 bit'. Location returned: " + to_string(b2bitloc) + ". Assert validated.\n";
				Logger::WriteMessage(test_message.c_str());
			}

			s16 highbitloc = lsb_u(highbit);
			Assert::AreEqual(s16(511), highbitloc);
			{
				string test_message = "Least significant bit function testing. Find 'High bit'. Location returned: " + to_string(highbitloc) + ". Assert validated.\n";
				Logger::WriteMessage(test_message.c_str());
			}

			s16 twobitbitloc = lsb_u(twobitsdiffwords);  // find / select lsb bit of the two from different words
			Assert::AreEqual(s16(0), twobitbitloc);
			{
				string test_message = "Least significant bit function testing. Select from two bits in different words. Location returned: " + to_string(twobitbitloc) + ". Assert validated.\n";
				Logger::WriteMessage(test_message.c_str());
			}

			s16 b2bitswloc = lsb_u(b2bitsw);	// find / select lsb bit of the two from the same word
			Assert::AreEqual(s16(0), b2bitswloc);
			{
				string test_message = "Least significant bit function testing. Select from two bits in same word. Location returned: " + to_string(b2bitswloc) + ". Assert validated.\n";
				Logger::WriteMessage(test_message.c_str());
			}

			s16 choosebitloc = lsb_u(choosebit);
			Assert::AreEqual(s16(64), choosebitloc);
			{
				string test_message = "Least significant bit function testing. Find 'Chhose bit'. Location returned: " + to_string(choosebitloc) + ". Assert validated.\n";
				Logger::WriteMessage(test_message.c_str());
			}

			for (int i = 0; i < adjtest_run_count; i++)
			{
				for (int j = 7; j >= 0; j--)
				{
					u64 val = 1;
					for (int k = 63; k >= 0; k--)
					{
						num1[j] = val;
						expectedbitloc = 511 - (j * 64 + k);
						reg_verify((u64*)&r_before);
						bitloc = lsb_u(num1);						
						reg_verify((u64*)&r_after);
						Assert::AreEqual(expectedbitloc, bitloc);
						Assert::IsTrue(r_before.AreEqual(&r_after), L"Register validation failed");
						val = val << 1;
					};

					num1[j] = 0;
				};
			};

			string test_message = "Least significant bit function testing. Ran tests " + to_string(test_run_count) + " times, each with shifted values. Every bit location checked.\n";
			Logger::WriteMessage(test_message.c_str());
			Logger::WriteMessage(L"Passed. Register imegrity checked, tested expected values via assert.\n");
		};

		TEST_METHOD(ui512bits_04_lsb_performance)
		{
			// Performance timing tests.
			// Ref: "Essentials of Modern Business Statistics", 7th Ed, by Anderson, Sweeney, Williams, Camm, Cochran. South-Western, 2015
			// Sections 3.2, 3.3, 3.4
			// Note: these tests are not pass/fail, they are informational only

			Logger::WriteMessage(L"LSB function performance timing test.\n\n");

			Logger::WriteMessage(L"First run.\n");
			perf_stats No1 = Perf_Test_Parms[0];
			ui512_Unit_Tests::RunStats(&No1, lsb);

			Logger::WriteMessage(L"Second run.\n");
			perf_stats No2 = Perf_Test_Parms[1];
			RunStats(&No2, lsb);

			Logger::WriteMessage(L"Third run.\n");
			perf_stats No3 = Perf_Test_Parms[2];
			RunStats(&No3, lsb);
		};
	};
};
