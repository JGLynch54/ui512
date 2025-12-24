#pragma once
#ifndef ui512_unit_test_h
#define ui512_unit_test_h

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
//
//		ui512_unit_test.h
// 
//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// 
//		File:			ui512_unit_test.h
//		Author:			John G.Lynch
//		Legal:			Copyright @2025, per MIT License below
//		Date:			October 24, 2025
//


#include "pch.h"
#include "CppUnitTest.h"
#include "CommonTypeDefs.h"
#include "ui512_externs.h"

#include <string>
#include <cstring>
#include <sstream>
#include <format>
#include <chrono>


//--------------------------------------------------------------------------------------------------------------------------------------------------------------

namespace ui512_Unit_Tests
{
	struct outlier
	{
		int iteration;
		double duration;
		double variance;
		double z_score;
	};

	struct perf_stats
	{
		int timing_count;
		double total;
		double min;
		double max;
		double mean;
		double sample_variance;
		double stddev;
		double coefficient_of_variation;
		double outlier_threshold;
		std::vector<double> *x_i;
		std::vector<double> *z_score;
		std::vector<outlier> *outliers;
	};

	enum Perf_Tests { Comp, Comp64, Add, AddwC, Add64, Sub, Subwb, Sub64, Mul, Mul64, Div, Div64, And, Or, Xor, Not, Shl, Shr, msb, lsb };
	
	extern const s32 test_run_count;
	extern const s32 reg_verification_count;
	extern const s32 timing_count;

	extern const s32 timing_count_short;
	extern const s32 timing_count_medium;
	extern const s32 timing_count_long;

	extern std::vector<perf_stats> Perf_Test_Parms;

	extern const std::string TestName[]; // use perf_test enum as index

	extern u64 DurationTest_Mul();

	extern u64 RandomU64(u64* seed);
	extern void RandomFill(u64* var, u64* seed);

	extern void RunStats(perf_stats* stat, Perf_Tests test_sel);
};

#endif // ui512_unit_test_h
