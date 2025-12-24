//		ui512_unit_tests
// 
//		File:			ui512_unit_tests.cpp
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
#include <string>

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ui512_Unit_Tests
{
	const s32 test_run_count = 1000;
	const s32 reg_verification_count = 5000;
	const s32 timing_count = 1000000;

	const s32 warm_up_count = 10000;
	const s32 timing_count_short = 100000;
	const s32 timing_count_medium = 500000;
	const s32 timing_count_long = 5000000;

	const bool pipeline_test = true;


	//enum Perf_Tests { Comp, Comp64, Add, AddwC, Add64, Sub, Subwb, Sub64, Mul, Mul64, Div, Div64, And, Or, Xor, Not, Shl, Shr, msb, lsb };
	const string TestName [ ] = { "Compare: 512 <=> 512", "Compare 512 <=> 64",
		"Add: 512 + 512", "Add: 512 + 512 + carry", "Add: 512 + 64",
		"Subtract: 512 - 512", "Subtract: 512 - 512 - borrow", "Subtract: 512 - 64",
		"Multiply: 512 * 512", "Multiply: 512 * 64",
		"Divide: 512 / 512", "Divide: 512 / 64",
		"Logical bit AND", "Logical bit OR", "Logical bit XOR", "Logical bit NOT",
		"Shift Left", "Shift Right",
		"Most significant bit", "Least significant bit"
	};

	/// <summary>
	/// Random number generator
	/// uses linear congruential method 
	/// ref: Knuth, Art Of Computer Programming, Vol. 2, Seminumerical Algorithms, 3rd Ed. Sec 3.2.1
	/// </summary>
	/// <param name="seed">if zero, will supply with: 4294967291</param>
	/// <returns>Pseudo-random number from zero to ~2^63 (18446744073709551557)</returns>

#define		primeLT64	18446744073709551557ull			// greatest prime less that 2^64
#define		primeLT36	68719476721ull					// greatest prime less than 2^36
#define		primeLT32	4294967291ull					// greatest prime less than 2^32
#define		primeLT28	268435399ull					// greatest prime less than 2^28

	extern u64 RandomU64( u64* seed )
	{
		const u64 m = primeLT64;
		const u64 a = primeLT36;
		const u64 c = primeLT28;
		*seed = ( *seed == 0ull ) ? ( a * primeLT32 + c ) % m : ( a * *seed + c ) % m;
		return *seed;
	};

	/// <summary>
	/// Random fill of ui512 variable
	/// </summary>
	/// <param name="var">512 bit variable to be filled</param>
	/// <param name="seed">seed for random number generator</param>
	/// <returns>none</returns>
	extern void RandomFill( u64* var, u64* seed )
	{
		for ( int i = 0; i < 8; i++ )
		{
			var [ i ] = RandomU64( seed );
		};
	};

	/// <summary>
	/// 
	/// </summary>
	vector<perf_stats> Perf_Test_Parms
	{
		{timing_count_short,
		0.0, 1000000.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, NULL, NULL, NULL},
		{timing_count_medium,
		0.0, 1000000.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, NULL, NULL, NULL},
		{ timing_count_long,
		0.0, 1000000.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, NULL, NULL, NULL}
	};

	u64 seed = 0;

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	u64 DurationTest_Comp( )
	{
		_UI512( num1 ) { 7, 6, 5, 4, 3, 2, 1, 0 };
		_UI512( num2 ) { 0, 1, 2, 3, 4, 5, 6, 7 };
		s16 rc = 0;
		if ( !pipeline_test )
		{
			RandomFill( num1, &seed );
			RandomFill( num2, &seed );
		};
		u64 start = __rdtsc( );
		rc = compare_u( num1, num2 );
		return ( __rdtsc( ) - start );
	};

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	u64 DurationTest_Comp64( )
	{
		_UI512( num1 ) { 7, 6, 5, 4, 3, 2, 1, 0 };
		u64 num2 = 12340;
		s16 rc = 0;
		if ( !pipeline_test )
		{
			RandomFill( num1, &seed );
			num2 = RandomU64( &seed );
		}
		u64 start = __rdtsc( );
		rc = compare_uT64( num1, num2 );
		return ( __rdtsc( ) - start );
	};

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	u64 DurationTest_Add( )
	{

		_UI512( num1 ) { 7, 6, 5, 4, 3, 2, 1, 0 };
		_UI512( num2 ) { 0, 1, 2, 3, 4, 5, 6, 7 };
		_UI512( sum ) { 0 };
		s16 carry = 0;
		if ( !pipeline_test )
		{
			RandomFill( num1, &seed );
			RandomFill( num2, &seed );
		}
		u64 start = __rdtsc( );
		carry = add_u( sum, num1, num2 );
		return ( __rdtsc( ) - start );
	};

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	u64 DurationTest_AddwC( )
	{
		_UI512( num1 ) { 7, 6, 5, 4, 3, 2, 1, 0 };
		_UI512( num2 ) { 0, 1, 2, 3, 4, 5, 6, 7 };;
		_UI512( sum ) { 0 };
		s16 carry = 0;
		if ( !pipeline_test )
		{
			RandomFill( num1, &seed );
			RandomFill( num2, &seed );
		}
		u64 start = __rdtsc( );
		carry = add_u_wc( sum, num1, num2, carry );
		return ( __rdtsc( ) - start );
	};

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	u64 DurationTest_Add64( )
	{
		_UI512( num1 ) { 0, 1, 2, 3, 4, 5, 6, 7 };
		_UI512( sum ) { 0 };
		u64 num2 = 123456;
		s16 carry = 0;
		if ( !pipeline_test )
		{
			RandomFill( num1, &seed );
			num2 = RandomU64( &seed );
		}
		u64 start = __rdtsc( );
		carry = add_uT64( sum, num1, num2 );
		return ( __rdtsc( ) - start );
	};

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	u64 DurationTest_Sub( )
	{
		//u64 seed = 0;
		_UI512( num1 ) { 0 };
		_UI512( num2 ) { 0 };
		_UI512( diff ) { 0 };
		s16 borrow = 0;
		RandomFill( num1, &seed );
		RandomFill( num2, &seed );
		u64 start = __rdtsc( );
		borrow = sub_u( diff, num1, num2 );
		return ( __rdtsc( ) - start );
	};

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	u64 DurationTest_Subwb( )
	{
		//u64 seed = 0;
		_UI512( num1 ) { 0 };
		_UI512( num2 ) { 0 };
		_UI512( diff ) { 0 };
		s16 borrow = 0;
		RandomFill( num1, &seed );
		RandomFill( num2, &seed );
		u64 start = __rdtsc( );
		borrow = sub_u_wb( diff, num1, num2, 0 );
		return ( __rdtsc( ) - start );
	};

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	u64 DurationTest_Sub64( )
	{
		//u64 seed = 0;
		_UI512( num1 ) { 0 };
		_UI512( diff ) { 0 };
		u64 num2 = 0;
		s16 borrow = 0;
		RandomFill( num1, &seed );
		num2 = RandomU64( &seed );
		u64 start = __rdtsc( );
		borrow = sub_uT64( diff, num1, num2 );
		return ( __rdtsc( ) - start );
	};

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	u64 DurationTest_Mul( )
	{
		_UI512( num1 ) { 1, 2, 3, 4, 5, 6, 7, 8 };
		_UI512( num2 ) { 8, 7, 6, 5, 4, 3, 2, 1 };
		_UI512( product ) { 0 };
		_UI512( overflow ) { 0 };
		if ( !pipeline_test )
		{
			RandomFill( num1, &seed );
			RandomFill( num2, &seed );
		}
		u64 start = __rdtsc( );
		s16 rc = mult_u( product, overflow, num1, num2 );
		return ( __rdtsc( ) - start );
	};

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	u64 DurationTest_Mul64( )
	{
		_UI512( num1 ) { 1, 2, 3, 4, 5, 6, 7, 8 };
		_UI512( product ) { 0 };
		u64 num2 = 1230;
		u64 overflow = 0;
		if ( !pipeline_test )
		{
			RandomFill( num1, &seed );
			num2 = RandomU64( &seed );
		}
		u64 start = __rdtsc( );
		s16 rc = mult_uT64( product, &overflow, num1, num2 );
		return ( __rdtsc( ) - start );
	};

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	u64 DurationTest_Div( )
	{
		_UI512( num1 ) { 1, 2, 3, 4, 5, 6, 7, 8 };
		_UI512( num2 ) { 8, 7, 6, 5, 4, 3, 2, 1 };
		_UI512( quotient ) { 0 };
		_UI512( remainder ) { 0 };
		if ( !pipeline_test )
		{
			RandomFill( num1, &seed );
			RandomFill( num2, &seed );
		}
		u64 start = __rdtsc( );
		s16 rc = div_u( quotient, remainder, num1, num2 );
		return ( __rdtsc( ) - start );
	};

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	u64 DurationTest_Div64( )
	{
		_UI512( num1 ) { 1, 2, 3, 4, 5, 6, 7, 8 };
		_UI512( quotient ) { 0 };
		u64 num2 = 54760;
		u64 remainder = 0;
		if ( !pipeline_test )
		{
			RandomFill( num1, &seed );
			num2 = RandomU64( &seed );
		}
		u64 start = __rdtsc( );
		s16 rc = div_uT64( quotient, &remainder, num1, num2 );
		return ( __rdtsc( ) - start );
	};

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	u64 DurationTest_And( )
	{
		_UI512( num1 ) { 1, 2, 3, 4, 5, 6, 7, 8 };
		_UI512( num2 ) { 8, 7, 6, 5, 4, 3, 2, 1 };;
		_UI512( result ) { 0 };
		if ( !pipeline_test )
		{
			RandomFill( num1, &seed );
			RandomFill( num2, &seed );
		}
		u64 start = __rdtsc( );
		and_u( result, num1, num2 );
		return ( __rdtsc( ) - start );
	};

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	u64 DurationTest_Or( )
	{
		_UI512( num1 ) { 1, 2, 3, 4, 5, 6, 7, 8 };
		_UI512( num2 ) { 8, 7, 6, 5, 4, 3, 2, 1 };
		_UI512( result ) { 0 };
		if ( !pipeline_test )
		{
			RandomFill( num1, &seed );
			RandomFill( num2, &seed );
		}
		u64 start = __rdtsc( );
		or_u( result, num1, num2 );
		return ( __rdtsc( ) - start );
	};

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	u64 DurationTest_Xor( )
	{
		_UI512( num1 ) { 1, 2, 3, 4, 5, 6, 7, 8 };
		_UI512( num2 ) { 8, 7, 6, 5, 4, 3, 2, 1 };
		_UI512( result ) { 0 };
		if ( !pipeline_test )
		{
			RandomFill( num1, &seed );
			RandomFill( num2, &seed );
		}
		u64 start = __rdtsc( );
		xor_u( result, num1, num2 );
		return ( __rdtsc( ) - start );
	};

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	u64 DurationTest_Not( )
	{
		_UI512( num1 ) { 1, 2, 3, 4, 5, 6, 7, 8 };
		_UI512( result ) { 0 };
		if ( !pipeline_test )
		{
			RandomFill( num1, &seed );
		}
		u64 start = __rdtsc( );
		not_u( result, num1 );
		return ( __rdtsc( ) - start );
	};

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	u64 DurationTest_Shl( )
	{
		_UI512( num1 ) { 0, 1, 2, 3, 4, 5, 6, 7 };
		_UI512( result ) { 0 };
		u64 start = __rdtsc( );
		shl_u( result, num1, 180 );
		return ( __rdtsc( ) - start );
	};


	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	u64 DurationTest_Shr( )
	{
		_UI512( num1 ) { 0, 1, 2, 3, 4, 5, 6, 7 };
		_UI512( result ) { 0 };
		u64 start = __rdtsc( );
		shr_u( result, num1, 180 );
		return ( __rdtsc( ) - start );
	};

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	u64 DurationTest_msb( )
	{
		_UI512( num1 ) { 0, 1, 2, 3, 4, 5, 6, 7 };
		s16 result = 0;
		u64 start = __rdtsc( );
		result = msb_u( num1 );
		return ( __rdtsc( ) - start );
	};

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	u64 DurationTest_lsb( )
	{
		_UI512( num1 ) { 7, 6, 5, 4, 3, 2, 1, 0 };
		s16 result = 0;
		u64 start = __rdtsc( );
		result = lsb_u( num1 );
		return ( __rdtsc( ) - start );
	};

	/// <summary>
	/// 
	/// </summary>
	/// <param name="stat"></param>
	/// <param name="test_sel"></param>
	void RunStats( perf_stats* stat, Perf_Tests test_sel )
	{
		double duration = 0;
		stat->x_i = new std::vector<double>( stat->timing_count );
		stat->z_score = new std::vector<double>( stat->timing_count );
		stat->outliers = new std::vector<outlier>( 0 );
		// for reference: enum Perf_Tests { Comp, Comp64, Add, AddwC, Add64, Sub, Subwb, Sub64, Mul, Mul64, Div, Div64, And, Or, Xor, Not, Shl, Shr, msb, lsb };
		typedef u64( *targettest )( );
		targettest targets [ ] = {
			&DurationTest_Comp, &DurationTest_Comp64,
			&DurationTest_Add, &DurationTest_AddwC, &DurationTest_Add64,
			&DurationTest_Sub, &DurationTest_Subwb, &DurationTest_Sub64,
			&DurationTest_Mul, &DurationTest_Mul64,
			&DurationTest_Div, &DurationTest_Div64,
			&DurationTest_And, &DurationTest_Or,
			&DurationTest_Xor, &DurationTest_Not,
			&DurationTest_Shl, &DurationTest_Shr,
			&DurationTest_msb, &DurationTest_lsb
		};

		for ( int i = 0; i < warm_up_count; i++ ) {
			duration = ( double ) targets [ test_sel ]( );
		}
		// Run target function timing_count times, capturing each duration, also getting min, max, and total duration spent

		for ( int i = 0; i < stat->timing_count; i++ )
		{
			duration = ( double ) targets [ test_sel ]( );
			stat->min = ( duration < stat->min ) ? duration : stat->min;
			stat->max = ( duration > stat->max ) ? duration : stat->max;
			stat->total += duration;
			stat->x_i->at( i ) = duration;
		};

		// Calculate mean, population variance, standard deviation, coefficient of variation, and z-scores
		{
			stat->mean = stat->total / double( stat->timing_count );
			for ( int i = 0; i < stat->timing_count; i++ )
			{
				double deviation = stat->x_i->at( i ) - stat->mean;
				stat->sample_variance += deviation * deviation;
			};

			stat->sample_variance /= ( double( stat->timing_count ) - 1.0 );
			stat->stddev = sqrt( stat->sample_variance );
			stat->coefficient_of_variation = ( stat->mean != 0.0 ) ? ( stat->stddev / stat->mean ) * 100.0 : 0.0;
			for ( int i = 0; i < stat->timing_count; i++ )
			{
				stat->z_score->at( i ) = ( stat->stddev != 0.0 ) ? ( stat->x_i->at( i ) - stat->mean ) / stat->stddev : 0.0;
			};

			string test_message = "***\t\t\t" + TestName [ test_sel ] + "\t\t\t***\n";
			test_message += format( "Samples run:\t\t\t\t{:9d}\n", stat->timing_count );
			test_message += format( "Total target function (including c calling set - up) execution cycles :{:10.0f}\n", stat->total );
			test_message += format( "Average clock cycles per call: \t{:6.2f}\n", stat->mean );
			test_message += format( "Minimum in \t\t\t\t\t\t{:6.0f}\n", stat->min );
			test_message += format( "Maximum in \t\t\t\t\t\t{:6.0f}\n", stat->max );
			test_message += format( "Sample Variance: \t\t\t{:10.3f}\n", stat->sample_variance );
			test_message += format( "Standard Deviation :\t \t{:9.3f}\n", stat->stddev );
			test_message += format( "Coefficient of Variation: \t{:10.2f}\n\n", stat->coefficient_of_variation );

			Logger::WriteMessage( test_message.c_str( ) );
		};

		// Identify outliers, based on outlier_threshold
		stat->outlier_threshold = 3.0 * stat->stddev;
		for ( int i = 0; i < stat->timing_count; i++ )
		{
			double z_sc = stat->z_score->at( i );
			double abs_z_score = ( z_sc < 0.0 ) ? -z_sc : z_sc;

			if ( abs_z_score > 3.0 )
			{
				outlier o;
				o.iteration = i;
				o.duration = stat->x_i->at( i );
				o.z_score = z_sc;
				stat->outliers->push_back( o );
			};
		};

		// Report on outliers, if any
		// Note: in a normal distribution, 99.7% of all values will be within three standard deviations of the mean
		// Thus, any value outside that range is an outlier
		// In this test, we are looking for unexpected occurrences of outliers, which may indicate some kind of
		// external interference in the timing test (such as OS activity, etc)
		// If the number of outliers is small (say under 1% of total), then it is likely not a problem
		// If the number of outliers is larger (say over 5% of total), then there may be a problem with the test environment
		// We're determining 'average' (mean) performance as measured in clock cycles.
		// The relaibility of the user achieving that average is higher if the deviation from mean is low,
		// and the number of exceptional deviations (outliers) is low.

		// Note: some functions may have a bi-modal distribution, in which case you must temper the results of the outlier test.
		// for example, mult_u has a propagating carry loop, which may cause a bi-modal distribution, or at least a widening of the distribution
		// Further, the CPU clock penalty of starting AVX instructions on some processors may cause outliers
		//
		double outlier_percentage = ( double ) ( stat->outliers->size( ) * 100.0 ) / ( double ) stat->timing_count;
		double range_low = stat->mean - ( 3.0 * stat->stddev );
		range_low = ( range_low < 0.0 ) ? 0.0 : range_low;
		double range_high = stat->mean + ( 3.0 * stat->stddev );

		if ( stat->outliers->size( ) > 0 )
		{
			string test_message = _MSGA( "Identified " << stat->outliers->size( ) << " outlier(s), based on a threshold of "
				<< stat->outlier_threshold << " which is three standard deviations from the mean of " << stat->mean << " clock cycles.\n" );
			test_message += _MSGA( "Samples with cycles from " << range_low << " to " << range_high << ", are within that range.\n" );
			test_message += format( "Samples within this range are considered normal and contain {:6.3f}% of the samples.\n", ( 100.0 - outlier_percentage ) );
			test_message += "Samples outside this range are considered outliers. ";
			test_message += format( "This represents {:4.3f}% of the samples.", outlier_percentage );
			test_message += "\nTested via Assert that the percentage of outliers is below 2%\n";
			test_message += "\nUp to the first 20 are shown. z_score is the number of standards of deviation the outlier varies from the mean.\n\n";
			test_message += " Iteration | Clock Cycles  |      Z Score  |\n";
			test_message += "-----------|---------------|---------------|\n";

			s32 outlier_limit = 20;
			s32 count = 0;

			for ( int i = 0; i < stat->outliers->size( ) && i < outlier_limit; i++ ) {
				outlier o = stat->outliers->at( i );
				test_message += format( "{:10d} |", o.iteration );
				test_message += format( "{:13.0f}  |", o.duration );
				test_message += format( "{:13.3f}  |", o.z_score );
				test_message += "\n";
			};
			test_message += "\n";

			Assert::IsTrue( outlier_percentage < 2.0, L"Too many outliers, over 2% of total sample" );
			Logger::WriteMessage( test_message.c_str( ) );
		};

		// End of batch
		stat->x_i->clear( );
		delete stat->x_i;
		stat->z_score->clear( );
		delete stat->z_score;
		stat->outliers->clear( );
		delete stat->outliers;
		return;
	};

	TEST_CLASS( ui512_unit_tests )
	{
		TEST_METHOD( random_number_generator )
		{
			//	Check distribution of "random" numbers
			u64 seed = 0;
			const u32 dec = 10;
			u32 dist [ dec ] { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

			const u64 split = primeLT64 / dec;
			u32 distc = 0;
			float varsum = 0.0;
			float deviation = 0.0;
			float D = 0.0;
			float sumD = 0.0;
			float variance = 0.0;
			const u32 randomcount = 1000000;
			const s32 norm = randomcount / dec;
			for ( u32 i = 0; i < randomcount; i++ )
			{
				seed = RandomU64( &seed );
				dist [ u64( seed / split ) ]++;
			};

			string msgd = "Evaluation of pseudo-random number generator.\n\n";
			msgd += format( "Generated {0:*>8} numbers.\n", randomcount );
			msgd += format( "Counted occurrences of those numbers by decile, each decile {0:*>20}.\n", split );
			msgd += format( "Distribution of numbers across the deciles indicates the quality of the generator.\n\n" );
			msgd += "Distribution by decile:";
			string msgv = "Variance from mean:\t";
			string msgchi = "Variance ^2 (chi):\t";

			for ( int i = 0; i < 10; i++ )
			{
				deviation = float( abs( long( norm ) - long( dist [ i ] ) ) );
				D = ( deviation * deviation ) / float( long( norm ) );
				sumD += D;
				variance = float( deviation ) / float( norm ) * 100.0f;
				varsum += variance;
				msgd += format( "\t{:6d}", dist [ i ] );
				msgv += format( "\t{:5.3f}% ", variance );
				msgchi += format( "\t{:5.3f}% ", D );
				distc += dist [ i ];
			};

			msgd += "\t\tDecile counts sum to: " + to_string( distc ) + "\n";
			Logger::WriteMessage( msgd.c_str( ) );
			msgv += "\t\tVariance sums to: ";
			msgv += format( "\t{:6.3f}% ", varsum );
			msgv += '\n';
			Logger::WriteMessage( msgv.c_str( ) );
			msgchi += "\t\tChi-squared distribution: ";
			msgchi += format( "\t{:6.3f}% ", sumD );
			msgchi += '\n';
			Logger::WriteMessage( msgchi.c_str( ) );
		};
	};
};
