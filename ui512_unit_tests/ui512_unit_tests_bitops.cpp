//		ui512_unit_tests_bitops
// 
//		File:			ui512_unit_tests_bitops.cpp
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
	TEST_CLASS(ui512_unit_tests_bitops)
	{
		TEST_METHOD(ui512bits_01_and)
		{
			u64 seed = 0;
			_UI512(num1) { 0 };
			_UI512(num2) { 0 };
			_UI512(result) { 0 };
			regs r_before{};
			regs r_after{};

			for (int i = 0; i < test_run_count; i++)
			{
				for (int j = 0; j < 8; j++)
				{
					num1[j] = RandomU64(&seed);
					num2[j] = ~num1[j];
					result[j] = 0;
				};

				reg_verify((u64*)&r_before);
				and_u(result, num1, num2);
				reg_verify((u64*)&r_after);
				Assert::IsTrue(r_before.AreEqual(&r_after), L"Register validation failed");

				for (int j = 0; j < 8; j++)
				{
					Assert::AreEqual(0x0ull, result[j]);
				};
			};

			string test_message = "'AND' function testing. Ran tests " + to_string(test_run_count) + " times, each with pseudo random values.\n"
				+ "and_u function register validation included in each run of the test, validated by assert.\n";
			Logger::WriteMessage(test_message.c_str());
			Logger::WriteMessage(L"Passed. Tested expected values via assert.\n");
		};

		TEST_METHOD(ui512bits_01_and_performance)
		{
			// Performance timing tests.
			// Ref: "Essentials of Modern Business Statistics", 7th Ed, by Anderson, Sweeney, Williams, Camm, Cochran. South-Western, 2015
			// Sections 3.2, 3.3, 3.4
			// Note: these tests are not pass/fail, they are informational only

			Logger::WriteMessage(L"AND function performance timing test.\n\n");

			Logger::WriteMessage(L"First run.\n");
			perf_stats No1 = Perf_Test_Parms[0];
			ui512_Unit_Tests::RunStats(&No1, And);

			Logger::WriteMessage(L"Second run.\n");
			perf_stats No2 = Perf_Test_Parms[1];
			RunStats(&No2, And);

			Logger::WriteMessage(L"Third run.\n");
			perf_stats No3 = Perf_Test_Parms[2];
			RunStats(&No3, And);
		};

		TEST_METHOD(ui512bits_02_or)
		{
			u64 seed = 0;
			_UI512(num1) { 0 };
			_UI512(num2) { 0 };
			_UI512(result) { 0 };
			regs r_before{};
			regs r_after{};

			for (int i = 0; i < test_run_count; i++)
			{
				for (int j = 0; j < 8; j++)
				{
					num1[j] = RandomU64(&seed);
					num2[j] = ~num1[j];
					result[j] = 0;
				};

				reg_verify((u64*)&r_before);
				or_u(result, num1, num2);
				reg_verify((u64*)&r_after);
				Assert::IsTrue(r_before.AreEqual(&r_after), L"Register validation failed");

				for (int j = 0; j < 8; j++)
				{
					Assert::AreEqual(0xFFFFFFFFFFFFFFFFull, result[j]);
				};
			};
			string test_message = "'OR' function testing. Ran tests " + to_string(test_run_count) + " times, each with pseudo random values.\n"
				+ "or_u function register validation included in each run of the test, validated by assert.\n";
			Logger::WriteMessage(test_message.c_str());
			Logger::WriteMessage(L"Passed. Tested expected values via assert.\n");
		};


		TEST_METHOD(ui512bits_02_or_performance)
		{
			// Performance timing tests.
			// Ref: "Essentials of Modern Business Statistics", 7th Ed, by Anderson, Sweeney, Williams, Camm, Cochran. South-Western, 2015
			// Sections 3.2, 3.3, 3.4
			// Note: these tests are not pass/fail, they are informational only

			Logger::WriteMessage(L"OR function performance timing test.\n\n");

			Logger::WriteMessage(L"First run.\n");
			perf_stats No1 = Perf_Test_Parms[0];
			ui512_Unit_Tests::RunStats(&No1, Or);

			Logger::WriteMessage(L"Second run.\n");
			perf_stats No2 = Perf_Test_Parms[1];
			RunStats(&No2, Or);

			Logger::WriteMessage(L"Third run.\n");
			perf_stats No3 = Perf_Test_Parms[2];
			RunStats(&No3, Or);
		};

		TEST_METHOD(ui512bits_03_xor)
		{
			u64 seed = 0;
			_UI512(num1) { 0 };
			_UI512(num2) { 0 };
			_UI512(result) { 0 };
			regs r_before{};
			regs r_after{};

			for (int i = 0; i < test_run_count; i++)
			{
				for (int j = 0; j < 8; j++)
				{
					num1[j] = num2[j] = RandomU64(&seed);
					result[j] = 0;
				};

				reg_verify((u64*)&r_before);
				xor_u(result, num1, num2);
				reg_verify((u64*)&r_after);
				Assert::IsTrue(r_before.AreEqual(&r_after), L"Register validation failed");

				for (int j = 0; j < 8; j++)
				{
					Assert::AreEqual(0x0ull, result[j]);
				};
			};

			string test_message = "'XOR' function testing. Ran tests " + to_string(test_run_count) + " times, each with pseudo random values.\n"
				+ "xor_u function register validation included in each run of the test, validated by assert.\n";
			Logger::WriteMessage(test_message.c_str());
			Logger::WriteMessage(L"Passed. Tested expected values via assert.\n");
		};

		TEST_METHOD(ui512bits_03_xor_performance)
		{
			// Performance timing tests.
			// Ref: "Essentials of Modern Business Statistics", 7th Ed, by Anderson, Sweeney, Williams, Camm, Cochran. South-Western, 2015
			// Sections 3.2, 3.3, 3.4
			// Note: these tests are not pass/fail, they are informational only

			Logger::WriteMessage(L"XOR function performance timing test.\n\n");

			Logger::WriteMessage(L"First run.\n");
			perf_stats No1 = Perf_Test_Parms[0];
			ui512_Unit_Tests::RunStats(&No1, Xor);

			Logger::WriteMessage(L"Second run.\n");
			perf_stats No2 = Perf_Test_Parms[1];
			RunStats(&No2, Xor);

			Logger::WriteMessage(L"Third run.\n");
			perf_stats No3 = Perf_Test_Parms[2];
			RunStats(&No3, Xor);
		};

		TEST_METHOD(ui512bits_04_not)
		{
			u64 seed = 0;
			_UI512(num1) { 0 };
			_UI512(num2) { 0 };
			_UI512(result) { 0 };
			regs r_before{};
			regs r_after{};

			for (int i = 0; i < test_run_count; i++)
			{

				for (int j = 0; j < 8; j++)
				{
					num1[j] = RandomU64(&seed);
					num2[j] = ~num1[j];
					result[j] = 0;
				};

				reg_verify((u64*)&r_before);
				not_u(result, num1);
				reg_verify((u64*)&r_after);
				Assert::IsTrue(r_before.AreEqual(&r_after), L"Register validation failed");
				
				for (int j = 0; j < 8; j++)
				{
					Assert::AreEqual(num2[j], result[j]);
				};
			};
			string test_message = "'NOT' function testing. Ran tests " + to_string(test_run_count) + " times, each with pseudo random values.\n"
				+ "not_u function register validation included in each run of the test, validated by assert.\n";
			Logger::WriteMessage(test_message.c_str());
			Logger::WriteMessage(L"Passed. Tested expected values via assert.\n");
		};

		TEST_METHOD(ui512bits_05_not_performance)
		{
			// Performance timing tests.
			// Ref: "Essentials of Modern Business Statistics", 7th Ed, by Anderson, Sweeney, Williams, Camm, Cochran. South-Western, 2015
			// Sections 3.2, 3.3, 3.4
			// Note: these tests are not pass/fail, they are informational only

			Logger::WriteMessage(L"NOT function performance timing test.\n\n");

			Logger::WriteMessage(L"First run.\n");
			perf_stats No1 = Perf_Test_Parms[0];
			ui512_Unit_Tests::RunStats(&No1, Not);

			Logger::WriteMessage(L"Second run.\n");
			perf_stats No2 = Perf_Test_Parms[1];
			RunStats(&No2, Not);

			Logger::WriteMessage(L"Third run.\n");
			perf_stats No3 = Perf_Test_Parms[2];
			RunStats(&No3, Not);
		};
	};
};
