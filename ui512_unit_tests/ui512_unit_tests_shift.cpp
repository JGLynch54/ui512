//		ui512_unit_tests_shift
// 
//		File:			ui512_unit_tests_shift.cpp
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
	TEST_CLASS(ui512_unit_tests_shift)
	{
		TEST_METHOD(ui512bits_01_shr)
		{
			_UI512(pattern) { 2, 4, 6, 8, 10, 12, 14, 16 };
			_UI512(num1) { 0, 0, 0, 0, 0, 0, 0, 0 };
			_UI512(num2) { 0, 0, 0, 0, 0, 0, 0, 0 };
			regs r_before{};
			regs r_after{};
			u64 seed = 0;

			reg_verify((u64*)&r_before);
			shr_u(num1, pattern, 0);
			reg_verify((u64*)&r_after);
			Assert::IsTrue(r_before.AreEqual(&r_after), L"Register validation failed");			
			for (int j = 0; j < 8; j++)
			{
				Assert::AreEqual(pattern[j], num1[j]);
			};
			{
				string test_message = "Shift right function testing. Edge case. Shift zero, should be copy. Tested true by assert. Verified reg integrity.\n";
				Logger::WriteMessage(test_message.c_str());
			};

			reg_verify((u64*)&r_before);
			shr_u(num1, pattern, 512);
			reg_verify((u64*)&r_after);
			Assert::IsTrue(r_before.AreEqual(&r_after), L"Register validation failed");
			for (int j = 0; j < 8; j++)
			{
				Assert::AreEqual(0ull, num1[j]);
			};
			{
				string test_message = "Shift right function testing. Edge case. Shift 512, should be all zero. Tested true by assert. Verified reg integrity.\n";
				Logger::WriteMessage(test_message.c_str());
			};

			reg_verify((u64*)&r_before);
			shr_u(num1, pattern, 1);
			reg_verify((u64*)&r_after);
			Assert::IsTrue(r_before.AreEqual(&r_after), L"Register validation failed");
			for (int j = 0; j < 8; j++)
			{
				Assert::AreEqual(pattern[j] >> 1, num1[j]);
			};
			{
				string test_message = "Shift right function testing. Shift one, should be each word halved. Tested true by assert. Verified reg integrity.\n";
				Logger::WriteMessage(test_message.c_str());
			};

			reg_verify((u64*)&r_before);
			shr_u(num1, pattern, 64);
			reg_verify((u64*)&r_after);
			Assert::IsTrue(r_before.AreEqual(&r_after), L"Register validation failed");
			Assert::AreEqual(0ull, num1[0]);	// most significant word now zero
			for (int j = 1; j < 6; j++)
			{
				Assert::AreEqual(pattern[j - 1], num1[j]);
			};

			{
				string test_message = "Shift right function testing. Shift 64, should be each word to next lower word. Tested true by assert. Verified reg integrity.\n";
				Logger::WriteMessage(test_message.c_str());
			};

			u16 shftcnt = 0;
			for (int j = 0; j < 8; j++)
			{
				num1[j] = 0xFF00000000000000ull;
			};

			// shift within each word to the end of the word
			shftcnt = 64 - 8;
			reg_verify((u64*)&r_before);
			shr_u(num2, num1, shftcnt);
			reg_verify((u64*)&r_after);
			Assert::IsTrue(r_before.AreEqual(&r_after), L"Register validation failed");
			for (int j = 0; j < 8; j++)
			{
				Assert::AreEqual(0x00000000000000FFull, num2[j]);
			};
			{
				string test_message = "Shift right function testing. Shift ff within each word to the end of each word. Passed.\n Tested expected values via assert. Verified reg integrity.\n";
				Logger::WriteMessage(test_message.c_str());
			};

			// shift (same ffh at high end of each word) into same place in next word. Note: start validation at word 2 as most significant (index 0) word now zero
			shftcnt = 64;
			reg_verify((u64*)&r_before);
			shr_u(num2, num1, shftcnt);
			reg_verify((u64*)&r_after);
			Assert::IsTrue(r_before.AreEqual(&r_after), L"Register validation failed");

			Assert::AreEqual(num2[0], 0x0000000000000000ull);
			for (int j = 7; j > 0; j--)
			{
				Assert::AreEqual(0xFF00000000000000ull, num2[j]);
			};
			{
				string test_message = "Shift right function testing. Shift ff into next word. Passed. Tested expected values via assert. Verified reg integrity.\n";
				Logger::WriteMessage(test_message.c_str());
			};

			// run same tests, with destination same as source, note: second test works on results of first now
			for (int j = 0; j < 8; j++)
			{
				num1[j] = 0xFF00000000000000ull;
			};

			shftcnt = 64 - 8;
			reg_verify((u64*)&r_before);
			shr_u(num1, num1, shftcnt);
			reg_verify((u64*)&r_after);
			Assert::IsTrue(r_before.AreEqual(&r_after), L"Register validation failed");
			for (int j = 0; j < 8; j++)
			{
				Assert::AreEqual(0x00000000000000FFull, num1[j]);
			};
			{
				string test_message = "Shift right function testing. Shift ff into next word. Destination same as source. Passed.\n Tested expected values via assert. Verified reg integrity.\n";
				Logger::WriteMessage(test_message.c_str());
			};

			// shift into next word. Note: start at word 2 as first word now zero
			shftcnt = 64;
			reg_verify((u64*)&r_before);
			shr_u(num1, num1, shftcnt);
			reg_verify((u64*)&r_after);
			Assert::IsTrue(r_before.AreEqual(&r_after), L"Register validation failed");
			Assert::AreEqual(0x0000000000000000ull, num1[0]);
			for (int j = 7; j >= 1; j--)
			{
				Assert::AreEqual(0x00000000000000FFull, num1[j]);
			};

			// walk a bit from most significant to least
			_UI512(num3) { 0x8000000000000000ull, 0, 0, 0, 0, 0, 0, 0 };
			_UI512(wlk1) { 0x8000000000000000ull, 0, 0, 0, 0, 0, 0, 0 };
			_UI512(wlk2) { 0, 0, 0, 0, 0, 0, 0, 0 };

			reg_verify((u64*)&r_before);
			shr_u(wlk2, num3, 0); // essentially a copy: wlk2 now equal to num3
			reg_verify((u64*)&r_after);
			Assert::IsTrue(r_before.AreEqual(&r_after), L"Register validation failed");
			for (int i = 1; i < 512; i++)
			{
				for (int j = 7; j >= 0; j--)
				{
					if (wlk1[j] != wlk2[j])
					{
						string errmsg = "Shift right walk failed. shift count: "
							+ to_string(i)
							+ "; word index: " + to_string(j)
							+ "; wlk1: " + to_string(wlk1[j])
							+ "; wlk2: " + to_string(wlk2[j])
							+ ":\n";

						Logger::WriteMessage(errmsg.c_str());
					};
					Assert::AreEqual(wlk1[j], wlk2[j]);
				};
				reg_verify((u64*)&r_before);
				shr_u(wlk1, num3, i);	// shift from original source 'n' bits
				reg_verify((u64*)&r_after);
				Assert::IsTrue(r_before.AreEqual(&r_after), L"Register validation failed");
				reg_verify((u64*)&r_before);
				shr_u(wlk2, wlk2, 1);	// shift from last shift one more bit (should be equal)
				reg_verify((u64*)&r_after);
				Assert::IsTrue(r_before.AreEqual(&r_after), L"Register validation failed");
			};

			Assert::AreEqual(1ull, wlk2[7]);		// end of bit by bit walk, should be one
			Assert::AreEqual(1ull, wlk1[7]);		// end of a 511 shift, should be one

			string test_message5 = "Shift right function testing. Walk a bit from msb to lsb. Verify values each step. Passed.\nTested expected values via assert. Verified reg integrity.\n";
			Logger::WriteMessage(test_message5.c_str());
			string test_message = "Shift right function testing. Ran tests " + to_string(4 + 512 + 511) + " times, with selected bit values. Passed.\nTested expected values via assert. Verified reg integrity.\n";
			Logger::WriteMessage(test_message.c_str());
		};

		TEST_METHOD(ui512bits_02_shr_performance)
		{
			// Performance timing tests.
			// Ref: "Essentials of Modern Business Statistics", 7th Ed, by Anderson, Sweeney, Williams, Camm, Cochran. South-Western, 2015
			// Sections 3.2, 3.3, 3.4
			// Note: these tests are not pass/fail, they are informational only

			Logger::WriteMessage(L"Shift Right function performance timing test.\n\n");

			Logger::WriteMessage(L"First run.\n");
			perf_stats No1 = Perf_Test_Parms[0];
			ui512_Unit_Tests::RunStats(&No1, Shr);

			Logger::WriteMessage(L"Second run.\n");
			perf_stats No2 = Perf_Test_Parms[1];
			RunStats(&No2, Shr);

			Logger::WriteMessage(L"Third run.\n");
			perf_stats No3 = Perf_Test_Parms[2];
			RunStats(&No3, Shr);
		};

		TEST_METHOD(ui512bits_03_shl)
		{
			_UI512(pattern) { 2, 4, 6, 8, 10, 12, 14, 16 };
			_UI512(num1) { 0 };
			_UI512(num2) { 0 };
			regs r_before{};
			regs r_after{};
			u64 seed = 0;
			u16 shftcnt = 0;
			for (int j = 0; j < 8; j++)
			{
				num1[j] = 0x00000000000000FFull;
			};

			reg_verify((u64*)&r_before);
			shl_u(num1, pattern, 0);
			reg_verify((u64*)&r_after);
			Assert::IsTrue(r_before.AreEqual(&r_after), L"Register validation failed");
			for (int j = 0; j < 8; j++)
			{
				Assert::AreEqual(pattern[j], num1[j]);
			};
			{
				string test_message = "Shift left function testing. Edge case. Shift zero, should be copy. Tested true by assert. Verified reg integrity.\n";
				Logger::WriteMessage(test_message.c_str());
			};

			reg_verify((u64*)&r_before);
			shl_u(num1, pattern, 512);
			reg_verify((u64*)&r_after);
			Assert::IsTrue(r_before.AreEqual(&r_after), L"Register validation failed");
			for (int j = 0; j < 8; j++)
			{
				Assert::AreEqual(0ull, num1[j]);
			};
			{
				string test_message = "Shift left function testing. Edge case. Shift 512, should be all zero. Tested true by assert. Verified reg integrity.\n";
				Logger::WriteMessage(test_message.c_str());
			};

			reg_verify((u64*)&r_before);
			shl_u(num1, pattern, 1);
			reg_verify((u64*)&r_after);
			Assert::IsTrue(r_before.AreEqual(&r_after), L"Register validation failed");
			for (int j = 0; j < 8; j++)
			{
				Assert::AreEqual(pattern[j] << 1, num1[j]);
			};
			{
				string test_message = "Shift left function testing. Shift one, should be each word doubled. Tested true by assert. Verified reg integrity.\n";
				Logger::WriteMessage(test_message.c_str());
			};

			reg_verify((u64*)&r_before);
			shl_u(num1, pattern, 64);
			reg_verify((u64*)&r_after);
			Assert::IsTrue(r_before.AreEqual(&r_after), L"Register validation failed");
			Assert::AreEqual(0ull, num1[7]);	// least significant word now zero
			for (int j = 1; j < 6; j++)
			{
				Assert::AreEqual(pattern[j + 1], num1[j]);
			};
			{
				string test_message = "Shift left function testing. Shift 64, should be each word to next higher word. Tested true by assert. Verified reg integrity.\n";
				Logger::WriteMessage(test_message.c_str());
			};

			// shift left within each word to the beginning of the word
			for (int j = 0; j < 8; j++)
			{
				num1[j] = 0x00000000000000FFull;
			};

			shftcnt = 64 - 8;
			reg_verify((u64*)&r_before);
			shl_u(num2, num1, shftcnt);
			reg_verify((u64*)&r_after);
			Assert::IsTrue(r_before.AreEqual(&r_after), L"Register validation failed");
			for (int j = 0; j < 8; j++)
			{
				Assert::AreEqual(num2[j], 0xFF00000000000000ull);
			};
			{
				string test_message = "Shift left function testing. Shift ff within each word to the beginning of each word. Passed. Tested expected values via assert. Verified reg integrity.\n";
				Logger::WriteMessage(test_message.c_str());
			};

			// shift into next word. Note: start validation at word 2 as least significant (index 7) word now zero
			shftcnt = 64;
			reg_verify((u64*)&r_before);
			shl_u(num2, num1, shftcnt);
			reg_verify((u64*)&r_after);
			Assert::IsTrue(r_before.AreEqual(&r_after), L"Register validation failed");
			Assert::AreEqual(num2[7], 0x0000000000000000ull);
			for (int j = 6; j >= 0; j--)
			{
				Assert::AreEqual(num2[j], 0x00000000000000FFull);
			};
			{
				string test_message = "Shift left function testing. Shift ff into next word. Passed. Tested expected values via assert. Verified reg integrity.\n";
				Logger::WriteMessage(test_message.c_str());
			};

			// run same tests, with destination same as source, note: second test works on results of first now
			for (int j = 0; j < 8; j++)
			{
				num1[j] = 0x00000000000000FFull;
			};
			shftcnt = 64 - 8;
			reg_verify((u64*)&r_before);
			shl_u(num1, num1, shftcnt);
			reg_verify((u64*)&r_after);
			Assert::IsTrue(r_before.AreEqual(&r_after), L"Register validation failed");
			for (int j = 0; j < 8; j++)
			{
				Assert::AreEqual(num1[j], 0xFF00000000000000ull);
			};
			{
				string test_message = "Shift left function testing. Shift ff into next word. Destination same as source. Passed. Tested expected values via assert. Verified reg integrity.\n";
				Logger::WriteMessage(test_message.c_str());
			};

			// shift into next word. Note: start at word 2 as first word now zero
			shftcnt = 64;
			reg_verify((u64*)&r_before);
			shl_u(num1, num1, shftcnt);
			reg_verify((u64*)&r_after);
			Assert::IsTrue(r_before.AreEqual(&r_after), L"Register validation failed");
			Assert::AreEqual(num1[7], 0x0000000000000000ull);
			for (int j = 6; j >= 0; j--)
			{
				Assert::AreEqual(num1[j], 0xFF00000000000000ull);
			};
			{
				string test_message = "Shift left function testing. Shift ff into next word. Destination same as source. Passed. Tested expected values via assert. Verified reg integrity.\n";
				Logger::WriteMessage(test_message.c_str());
			};

			// walk a bit from least significant to most
			alignas (64) u64 num3[8]{ 0, 0, 0, 0, 0, 0, 0, 1 };
			alignas (64) u64 wlk1[8]{ 0, 0, 0, 0, 0, 0, 0, 1 };
			alignas (64) u64 wlk2[8]{ 0, 0, 0, 0, 0, 0, 0, 0 };
			u16 shiftwalkcount = 1;
			for (int i = 0; i < 512; i++)
			{
				reg_verify((u64*)&r_before);
				shl_u(wlk2, num3, i);
				reg_verify((u64*)&r_after);
				Assert::IsTrue(r_before.AreEqual(&r_after), L"Register validation failed");
				for (int j = 7; j >= 0; j--)
				{
					if (wlk1[j] != wlk2[j])
					{
						string errmsg = "Shift left walk failed. shift count: "
							+ to_string(i)
							+ "; wlk1: " + to_string(wlk1[j])
							+ "; wlk2: " + to_string(wlk2[j])
							+ "\n";

						Logger::WriteMessage(errmsg.c_str());
					};

					Assert::AreEqual(wlk1[j], wlk2[j]);
				};

				if (i != 511)
				{
					reg_verify((u64*)&r_before);
					shl_u(wlk1, wlk1, shiftwalkcount);
					reg_verify((u64*)&r_after);
					Assert::IsTrue(r_before.AreEqual(&r_after), L"Register validation failed");
				};
			};

			Assert::AreEqual(0x8000000000000000ull, wlk2[0]);
			Assert::AreEqual(0x8000000000000000ull, wlk1[0]);

			string test_message = "Shift left function testing. Walk a bit from 0 to 511. Ran tests " + to_string(4 + 512 + 511) + " times, with selected bit values. Passed.\nTested expected values via assert. Verified reg integrity.\n";
			Logger::WriteMessage(test_message.c_str());
		};

		TEST_METHOD(ui512bits_04_shl_performance)
		{
			// Performance timing tests.
			// Ref: "Essentials of Modern Business Statistics", 7th Ed, by Anderson, Sweeney, Williams, Camm, Cochran. South-Western, 2015
			// Sections 3.2, 3.3, 3.4
			// Note: these tests are not pass/fail, they are informational only

			Logger::WriteMessage(L"Shift Left function performance timing test.\n\n");

			Logger::WriteMessage(L"First run.\n");
			perf_stats No1 = Perf_Test_Parms[0];
			ui512_Unit_Tests::RunStats(&No1, Shl);

			Logger::WriteMessage(L"Second run.\n");
			perf_stats No2 = Perf_Test_Parms[1];
			RunStats(&No2, Shl);

			Logger::WriteMessage(L"Third run.\n");
			perf_stats No3 = Perf_Test_Parms[2];
			RunStats(&No3, Shl);
		};		
	};
};