//		ui512_stub_testing
// 
//		File:			ui512_stub_testing.cpp
//		Author:			John G.Lynch
//		Legal:			Copyright @2024, per MIT License below
//		Date:			October 29, 2025
//

#include "pch.h"
#include "CppUnitTest.h"
#include "ui512_externs.h"
#include "ui512_stubexterns.h"
#include "ui512_unit_tests.h"

#include "intrin.h"
#include <chrono>
#include <cstring>
#include <format>
#include <sstream>
#include <string>

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ui512_Unit_Tests
{
	TEST_CLASS( ui512_stub_testing )
	{
		struct div_u_Locals
		{
			alignas( 64 ) u64 currnumerator [ 16 ];		// scratch working copy of dividend( numerator ).could be 9 qwords, 16 declared for alignment
			u64 qdiv [ 16 ];							// scratch working copy of( trial ) qhat* divisor.could be 9 qwords, 16 declared for alignment
			u64 quotient [ 8 ];							// working copy of quotient
			u64	normdivisor [ 8 ];						// working copy of normalized divisor

			u64	nDiv;									// first qword of normalized divisor
			u64	qHat;									// trial quotient
			u64	rHat;									// trial remainder

			u16	mMSB;									// indexes and dimensions of dividend ( numerator ) Note: dimensions are zero - based( 0 to 7 )
			u16	mDim;
			u16	mIdx;
			u16	mllimit;

			u16	nMSB;									// indexes and dimensions of divisor ( denominator )
			u16	nDim;
			u16	nIdx;
			u16	nllimit;

			u16	jDim;
			u16	jIdx;
			u16	jllimit;

			u16	normf;									// normalization factor ( bits to shift left )

			u16 filler [ 3 ];							// to get to 16 byte align for stack alloc( adjust as necessary )
		};

		TEST_METHOD( ui512_01 )
		{
			string test_message = string( "***\t\t\t" ) + "Stub Test 01" + "\t\t\t***\n";
			//test_message += format( "Samples run:\t\t\t\t{:9d}\n", stat->timing_count );
			//test_message += format( "Total target function (including c calling set - up) execution cycles :{:10.0f}\n", stat->total );
			//test_message += format( "Average clock cycles per call: \t{:6.2f}\n", stat->mean );
			//test_message += format( "Minimum in \t\t\t\t\t\t{:6.0f}\n", stat->min );
			//test_message += format( "Maximum in \t\t\t\t\t\t{:6.0f}\n", stat->max );
			//test_message += format( "Sample Variance: \t\t\t{:10.3f}\n", stat->sample_variance );
			//test_message += format( "Standard Deviation :\t \t{:9.3f}\n", stat->stddev );
			//test_message += format( "Coefficient of Variation: \t{:10.2f}\n\n", stat->coefficient_of_variation );

			Logger::WriteMessage( test_message.c_str( ) );
		};


	};	// test_class
};	// namespace