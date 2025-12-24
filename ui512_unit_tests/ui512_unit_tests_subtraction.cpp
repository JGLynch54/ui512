//		ui512_unit_tests_subtraction
// 
//		File:			ui512_unit_tests_subtraction.cpp
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
	TEST_CLASS(ui512_unit_tests_subtraction)
	{
		TEST_METHOD(ui512_01_subtract)
		{
			_UI512(num1) { 0 };
			_UI512(num2) { 0 };
			_UI512(diff) { 0 };
			_UI512(one) { 0, 0, 0, 0, 0, 0, 0, 1 };
			regs r_before{};
			regs r_after{};
			u64 seed = 0;
			s32 borrow = 0;

			for (int i = 0; i < test_run_count; i++)
			{
				for (int j = 0; j < 8; j++)
				{
					num1[j] = num2[j] = RandomU64(&seed);
					diff[j] = 0;
				};
				// subtract test: "random" number minus same number equals zero..., no borrow

				reg_verify((u64*)&r_before);
				borrow = sub_u(diff, num1, num2);
				reg_verify((u64*)&r_after);
				Assert::IsTrue(r_before.AreEqual(&r_after), L"Register validation failed");
				Assert::AreEqual(borrow, 0);
				for (int j = 0; j < 8; j++)
				{
					Assert::AreEqual(0x0000000000000000ull, diff[j]);
				};
				// now subtract one, should cascade borrows through all eight, making them each 0xFFF... , and overflow to borrow
				reg_verify((u64*)&r_before);
				borrow = sub_u(diff, diff, one);		// Note:  Destination (diff) is also an operand
				reg_verify((u64*)&r_after);
				Assert::IsTrue(r_before.AreEqual(&r_after), L"Register validation failed");
				Assert::AreEqual(1, borrow);
				for (int j = 0; j < 8; j++)
				{
					Assert::AreEqual(0xFFFFFFFFFFFFFFFFull, diff[j]);
				};
			};

			string runmsg = "Subtract function testing. Ran tests " + to_string(test_run_count * 2) + " times, each with pseudo random values.\n";
			Logger::WriteMessage(runmsg.c_str());
			Logger::WriteMessage(L"Passed. Register imegrity checked, tested expected values via assert.\n");
		};

		TEST_METHOD(ui512_02_subtract_performance)
		{
			// Performance timing tests.
			// Ref: "Essentials of Modern Business Statistics", 7th Ed, by Anderson, Sweeney, Williams, Camm, Cochran. South-Western, 2015
			// Sections 3.2, 3.3, 3.4
			// Note: these tests are not pass/fail, they are informational only

			Logger::WriteMessage(L"Subtract function performance timing test.\n\n");

			Logger::WriteMessage(L"First run.\n");
			perf_stats No1 = Perf_Test_Parms[0];
			ui512_Unit_Tests::RunStats(&No1, Sub);

			Logger::WriteMessage(L"Second run.\n");
			perf_stats No2 = Perf_Test_Parms[1];
			RunStats(&No2, Sub);

			Logger::WriteMessage(L"Third run.\n");
			perf_stats No3 = Perf_Test_Parms[2];
			RunStats(&No3, Sub);
		};

		TEST_METHOD(ui512_03_subtract_wb)
		{
			_UI512(num1) { 0 };
			_UI512(num2) { 0 };
			_UI512(diff) { 0 };
			_UI512(one) { 0, 0, 0, 0, 0, 0, 0, 1 };
			regs r_before{};
			regs r_after{};
			u64 seed = 0;
			s16 borrowout = 0;
			s16 borrowin = 0;

			for (int i = 0; i < test_run_count; i++)
			{
				for (int j = 0; j < 8; j++)
				{
					num1[j] = num2[j] = RandomU64(&seed);
					diff[j] = 0;
				};

				// subtract test: "random" number minus same number equals zero..., no borrow
				reg_verify((u64*)&r_before);
				borrowout = sub_u_wb(diff, num1, num2, borrowin);
				reg_verify((u64*)&r_after);
				Assert::IsTrue(r_before.AreEqual(&r_after), L"Register validation failed");
				Assert::AreEqual(borrowout, (s16)0);
				for (int j = 0; j < 8; j++)
				{
					Assert::AreEqual(0x0000000000000000ull, diff[j]);
				};

				// now subtract one, should cascade borrows through all eight, making them each 0xFFF... , and overflow to borrow
				reg_verify((u64*)&r_before);
				borrowin = 0;
				borrowout = sub_u_wb(diff, diff, one, borrowin);		// Note:  Destination (diff) is also an operand
				reg_verify((u64*)&r_after);
				Assert::IsTrue(r_before.AreEqual(&r_after), L"Register validation failed");
				Assert::AreEqual((s16)1, borrowout);
				for (int j = 0; j < 8; j++)
				{
					Assert::AreEqual(0xFFFFFFFFFFFFFFFFull, diff[j]);
				};
			};

			string runmsg = "Subtract (with borrow) function testing. Ran tests " + to_string(test_run_count * 2) + " times, each with pseudo random values.\n";
			Logger::WriteMessage(runmsg.c_str());
			Logger::WriteMessage(L"Passed. Register integrity checked, tested expected values via assert.\n");
		};

		TEST_METHOD(ui512_04_subtractwb_performance)
		{
			// Performance timing tests.
			// Ref: "Essentials of Modern Business Statistics", 7th Ed, by Anderson, Sweeney, Williams, Camm, Cochran. South-Western, 2015
			// Sections 3.2, 3.3, 3.4
			// Note: these tests are not pass/fail, they are informational only

			Logger::WriteMessage(L"Subtract with borrow function performance timing test.\n\n");

			Logger::WriteMessage(L"First run.\n");
			perf_stats No1 = Perf_Test_Parms[0];
			ui512_Unit_Tests::RunStats(&No1, Subwb);

			Logger::WriteMessage(L"Second run.\n");
			perf_stats No2 = Perf_Test_Parms[1];
			RunStats(&No2, Subwb);

			Logger::WriteMessage(L"Third run.\n");
			perf_stats No3 = Perf_Test_Parms[2];
			RunStats(&No3, Subwb);
		};

		TEST_METHOD(ui512_05_subtract64)
		{
			_UI512(num1) { 0 };
			_UI512(diff) { 0 };
			regs r_before{};
			regs r_after{};
			u64 num2 = 0;
			u64 one = 1;
			u64 seed = 0;
			s32 borrow = 0;

			for (int i = 0; i < test_run_count; i++)
			{
				for (int j = 0; j < 8; j++)
				{
					num1[j] = 0;
					diff[j] = 0;
				};
				// subtract test: "random" number minus same number equals zero..., no borrow
				num1[7] = num2 = RandomU64(&seed);
				num2 = num1[7];
				reg_verify((u64*)&r_before);
				borrow = sub_uT64(diff, num1, num2);
				reg_verify((u64*)&r_after);
				Assert::IsTrue(r_before.AreEqual(&r_after), L"Register validation failed");
				Assert::AreEqual(0, borrow);
				for (int j = 0; j < 8; j++)
				{
					Assert::AreEqual(0x0000000000000000ull, diff[j]);
				};
				// now subtract one, should cascade borrows through all eight, making them each 0xFFF... , and overflow to borrow
				reg_verify((u64*)&r_before);
				borrow = sub_uT64(diff, diff, one);		// Note:  Destination (sum) is also an operand
				reg_verify((u64*)&r_after);
				Assert::IsTrue(r_before.AreEqual(&r_after), L"Register validation failed");
				Assert::AreEqual(1, borrow);
				for (int j = 0; j < 8; j++)
				{
					Assert::AreEqual(0xFFFFFFFFFFFFFFFFull, diff[j]);
				};
			};

			string runmsg = "Subtract (T64) function testing. Ran tests " + to_string(test_run_count * 2) + " times, each with pseudo random values.\n";
			Logger::WriteMessage(runmsg.c_str());
			Logger::WriteMessage(L"Passed. Register imegrity checked, tested expected values via assert.\n");
		};

		TEST_METHOD(ui512_06_subtract64_performance)
		{
			// Performance timing tests.
			// Ref: "Essentials of Modern Business Statistics", 7th Ed, by Anderson, Sweeney, Williams, Camm, Cochran. South-Western, 2015
			// Sections 3.2, 3.3, 3.4
			// Note: these tests are not pass/fail, they are informational only

			Logger::WriteMessage(L"Subtract x64 function performance timing test.\n\n");

			Logger::WriteMessage(L"First run.\n");
			perf_stats No1 = Perf_Test_Parms[0];
			ui512_Unit_Tests::RunStats(&No1, Sub64);

			Logger::WriteMessage(L"Second run.\n");
			perf_stats No2 = Perf_Test_Parms[1];
			RunStats(&No2, Sub64);

			Logger::WriteMessage(L"Third run.\n");
			perf_stats No3 = Perf_Test_Parms[2];
			RunStats(&No3, Sub64);
		};
	};	// test_class
};	// namespace