//		ui51mdTests
// 
//		File:			ui512mdTests.cpp
//		Author:			John G.Lynch
//		Legal:			Copyright @2024, per MIT License below
//		Date:			June 19, 2024
//
//		This sub - project: ui512mdTests, is a unit test project that invokes each of the routines in the ui512md assembly.
//		It runs each assembler proc with pseudo - random values.
//		It validates ( asserts ) expected and returned results.
//		It also runs each repeatedly for comparative timings.
//		It provides a means to invoke and debug.
//		It illustrates calling the routines from C++.

#include "pch.h"
#include "CppUnitTest.h"

#include "CommonTypeDefs.h"
#include "ui512_externs.h"
//#include "ui512_unit_tests.h"
#include <cstring>
#include <sstream>
#include <format>
#include <chrono>


using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ui512_Unit_Tests_m
{



	TEST_CLASS(ui512mdTests)
	{
	public:

		 const s32 test_run_count = 1000;
		 const s32 reg_verification_count = 5000;
		 const s32 timing_count = 1000000;

		 const s32 timing_count_short = 10000;
		 const s32 timing_count_medium = 100000;
		 const s32 timing_count_long = 1000000;

		u64 RandomU64(u64* seed)
		{
			const u64 m = 18446744073709551557ull;			// greatest prime below 2^64
			const u64 a = 68719476721ull;					// closest prime below 2^36
			const u64 c = 268435399ull;						// closest prime below 2^28
			// suggested seed: around 2^32, 4294967291
			*seed = (*seed == 0ull) ? (a * 4294967291ull + c) % m : (a * *seed + c) % m;
			return *seed;
		};

		/// <summary>
		/// Random fill of ui512 variable
		/// </summary>
		/// <param name="var">512 bit variable to be filled</param>
		/// <param name="seed">seed for random number generator</param>
		/// <returns>none</returns>
		void RandomFill(u64* var, u64* seed)
		{
			for (int i = 0; i < 8; i++)
			{
				var[i] = RandomU64(seed);
			};
		}

		TEST_METHOD(random_number_generator)
		{
			//	Check distribution of "random" numbers
			u64 seed = 0;
			const u32 dec = 10;
			u32 dist[dec]{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
			const u64 split = 18446744073709551557ull / dec;
			u32 distc = 0;
			float varsum = 0.0;
			float deviation = 0.0;
			float D = 0.0;
			float sumD = 0.0;
			float variance = 0.0;
			const u32 randomcount = 1000000;
			const s32 norm = randomcount / dec;

			// generate random numbers, count distribution
			for (u32 i = 0; i < randomcount; i++)
			{
				seed = RandomU64(&seed);
				dist[u64(seed / split)]++;
			};

			string msgd = "Evaluation of pseudo-random number generator.\n\n";
			msgd += format("Generated {0:*>8} numbers.\n", randomcount);
			msgd += format("Counted occurrences of those numbers by decile, each decile {0:*>20}.\n", split);
			msgd += format("Distribution of numbers across the deciles indicates the quality of the generator.\n\n");
			msgd += "Distribution by decile:";
			string msgv = "Variance from mean:\t";
			string msgchi = "Variance ^2 (chi):\t";

			// evaluate distribution
			for (int i = 0; i < 10; i++)
			{
				deviation = float(abs(long(norm) - long(dist[i])));
				D = (deviation * deviation) / float(long(norm));
				sumD += D;
				variance = float(deviation) / float(norm) * 100.0f;
				varsum += variance;
				msgd += format("\t{:6d}", dist[i]);
				msgv += format("\t{:5.3f}% ", variance);
				msgchi += format("\t{:5.3f}% ", D);
				distc += dist[i];
			};

			msgd += "\t\tDecile counts sum to: " + to_string(distc) + "\n";
			Logger::WriteMessage(msgd.c_str());
			msgv += "\t\tVariance sums to: ";
			msgv += format("\t{:6.3f}% ", varsum);
			msgv += '\n';
			Logger::WriteMessage(msgv.c_str());
			msgchi += "\t\tChi-squared distribution: ";
			msgchi += format("\t{:6.3f}% ", sumD);
			msgchi += '\n';
			Logger::WriteMessage(msgchi.c_str());
		};

		TEST_METHOD(ui512md_04_div64_performance_timing)
		{
			// Performance timing tests.
			// Ref: "Essentials of Modern Business Statistics", 7th Ed, by Anderson, Sweeney, Williams, Camm, Cochran. South-Western, 2015
			// Sections 3.2, 3.3, 3.4
			// Note: these tests are not pass/fail, they are informational only

			_UI512(num1) { 0 };
			_UI512(dividend) { 0 };
			u64 remainder = 0;
			u64 num2 = 0;
			u64 seed = 0;

			double total_short = 0.0;
			double min_short = 1000000.0;
			double max_short = 0.0;
			double mean_short = 0.0;
			double sample_variance_short = 0.0;
			double stddev_short = 0.0;
			double coefficient_of_variation_short = 0.0;

			double total_medium = 0.0;
			double min_medium = 1000000.0;
			double max_medium = 0.0;
			double mean_medium = 0.0;
			double sample_variance_medium = 0.0;
			double stddev_medium = 0.0;
			double coefficient_of_variation_medium = 0.0;

			double total_long = 0.0;
			double min_long = 1000000.0;
			double max_long = 0.0;
			double mean_long = 0.0;
			double sample_variance_long = 0.0;
			double stddev_long = 0.0;
			double coefficient_of_variation_long = 0.0;

			double outlier_threshold = 0.0;

			struct outlier
			{
				int iteration;
				double duration;
				double variance;
				double z_score;
			};
			vector<outlier> outliers;

			// First batch, short run
			{
				std::vector<double> x_i_short(timing_count_short);
				std::vector<double> z_scores_short(timing_count_short);

				//Run target function timing_count_short times, capturing each time, also getting min, max, and total time spent
				for (int i = 0; i < timing_count_short; i++)
				{
					RandomFill(num1, &seed);
					num2 = RandomU64(&seed);
					auto countStart = std::chrono::steady_clock::now();
					div_uT64(dividend, &remainder, num1, num2);
					auto countEnd = std::chrono::steady_clock::now();
					std::chrono::duration<double, std::micro> countDur = countEnd - countStart;
					double duration = countDur.count();
					min_short = (duration < min_short) ? duration : min_short;
					max_short = (duration > max_short) ? duration : max_short;
					total_short += duration;
					x_i_short[i] = duration;
				};

				// Calculate mean, population variance, standard deviation, coefficient of variation, and z-scores
				{
					mean_short = total_short / double(timing_count_short);
					for (int i = 0; i < timing_count_short; i++)
					{
						double deviation = x_i_short[i] - mean_short;
						sample_variance_short += deviation * deviation;
					};

					sample_variance_short /= (double(timing_count_short) - 1.0);
					stddev_short = sqrt(sample_variance_short);
					coefficient_of_variation_short = (mean_short != 0.0) ? (stddev_short / mean_short) * 100.0 : 0.0;
					for (int i = 0; i < timing_count_short; i++)
					{
						z_scores_short[i] = (stddev_short != 0.0) ? (x_i_short[i] - mean_short) / stddev_short : 0.0;
					};

					string test_message = _MSGA("Divide (x64) function performance timing test.\n\nFirst batch. \nRan for "
						<< timing_count_short << " samples.\nTotal target function (including c calling set-up) execution time: "
						<< total_short << " microseconds. \nAverage time per call : "
						<< mean_short << " microseconds.\nMinimum in "
						<< min_short << "\nMaximum in "
						<< max_short << "\n");

					test_message += _MSGA("Sample Variance: "
						<< sample_variance_short << "\nStandard Deviation: "
						<< stddev_short << "\nCoefficient of Variation: "
						<< coefficient_of_variation_short << "%\n\n");

					Logger::WriteMessage(test_message.c_str());
				};

				// Identify outliers, based on outlier_threshold
				for (int i = 0; i < timing_count_short; i++)
				{
					double z_sc = z_scores_short[i];
					double abs_z_score = (z_sc < 0.0) ? -z_sc : z_sc;
					outlier_threshold = 3.0 * stddev_short;
					if (abs_z_score > 3.0)
					{
						outlier o;
						o.iteration = i;
						o.duration = x_i_short[i];
						o.z_score = z_sc;
						outliers.push_back(o);
					};
				};

				double outlier_percentage = (double)(outliers.size() * 100.0) / (double)timing_count_short;
				double range_low = mean_short - (3.0 * stddev_short);
				double range_high = mean_short + (3.0 * stddev_short);
				range_low = (range_low < 0.0) ? 0.0 : range_low;
				if (outliers.size() > 0)
				{
					string test_message = _MSGA("Identified " << outliers.size() << " outlier(s), based on a threshold of "
						<< outlier_threshold << " which is three standard deviations from the mean of " << mean_short << " microseconds (us).\n");
					test_message += _MSGA("Samples with execution times from " << range_low << " us to " << range_high << " us, are within that range.\n");
					test_message += format("Samples within this range are considered normal and contain {:6.3f}% of the samples.\n", (100.0 - outlier_percentage));
					test_message += "Samples outside this range are considered outliers. ";
					test_message += format("This represents {:6.3f}% of the samples.", outlier_percentage);
					test_message += "\nTested via Assert that the percentage of outliers is below 1%\n";
					test_message += "\nUp to the first 20 are shown. z_score is the number of standards of deviation the outlier varies from the mean.\n\n";
					test_message += " Iteration | Duration (us) | Z Score (us)  | \n";
					test_message += "-----------|---------------|---------------|\n";
					s32 outlier_limit = 20;
					s32 count = 0;
					for (auto& o : outliers) {
						if (count++ >= outlier_limit) break;
						test_message += format("{:10d} |", o.iteration);
						test_message += format("{:13.2f}  |", o.duration);
						test_message += format("{:13.4f}  |", o.z_score);
						test_message += "\n";
					}
					test_message += "\n";
					Assert::IsTrue(outlier_percentage < 1.0, L"Too many outliers, over 1% of total sample");
					Logger::WriteMessage(test_message.c_str());
				};

				// End of first batch
				x_i_short.clear();
				z_scores_short.clear();
				outliers.clear();
			};

			// Second batch, medium run
			{
				std::vector<double> x_i_medium(timing_count_medium);
				std::vector<double> z_scores_medium(timing_count_medium);
				outliers.clear();

				//Run target function timing_count_medium times, capturing each time, also getting min, max, and total time spent
				for (int i = 0; i < timing_count_medium; i++)
				{
					RandomFill(num1, &seed);
					num2 = RandomU64(&seed);
					auto countStart = std::chrono::steady_clock::now();
					div_uT64(dividend, &remainder, num1, num2);
					auto countEnd = std::chrono::steady_clock::now();
					std::chrono::duration<double, std::micro> countDur = countEnd - countStart;
					double duration = countDur.count();
					min_medium = (duration < min_medium) ? duration : min_medium;
					max_medium = (duration > max_medium) ? duration : max_medium;
					total_medium += duration;
					x_i_medium[i] = duration;
				};

				// Calculate mean, population variance, standard deviation, coefficient of variation, and z-scores
				{
					mean_medium = total_medium / double(timing_count_medium);
					for (int i = 0; i < timing_count_medium; i++)
					{
						double deviation = x_i_medium[i] - mean_medium;
						sample_variance_medium += deviation * deviation;
					};
					sample_variance_medium /= (double(timing_count_medium) - 1.0);
					stddev_medium = sqrt(sample_variance_medium);
					coefficient_of_variation_medium = (mean_medium != 0.0) ? (stddev_medium / mean_medium) * 100.0 : 0.0;
					for (int i = 0; i < timing_count_medium; i++)
					{
						z_scores_medium[i] = (stddev_medium != 0.0) ? (x_i_medium[i] - mean_medium) / stddev_medium : 0.0;
					};

					string test_message = _MSGA("\nSecond batch. \nRan for "
						<< timing_count_medium << " samples.\nTotal target function (including c calling set-up) execution time: "
						<< total_medium << " microseconds. \nAverage time per call : "
						<< mean_medium << " microseconds.\nMinimum in "
						<< min_medium << " \nMaximum in "
						<< max_medium << " \n");

					test_message += _MSGA("Sample Variance: "
						<< sample_variance_medium << "\nStandard Deviation: "
						<< stddev_medium << "\nCoefficient of Variation: "
						<< coefficient_of_variation_medium << "%\n\n");

					Logger::WriteMessage(test_message.c_str());
				};

				// Identify outliers, based on outlier_threshold
				for (int i = 0; i < timing_count_medium; i++)
				{
					double z_sc = z_scores_medium[i];
					double abs_z_score = (z_sc < 0.0) ? -z_sc : z_sc;
					outlier_threshold = 3.0 * stddev_medium;
					if (abs_z_score > 3.0)
					{
						outlier o;
						o.iteration = i;
						o.duration = x_i_medium[i];
						o.z_score = z_sc;
						outliers.push_back(o);
					};
				};

				// Report on outliers, if any
				double outlier_percentage = (double)(outliers.size() * 100.0) / (double)timing_count_medium;
				double range_low = mean_medium - (3.0 * stddev_medium);
				double range_high = mean_medium + (3.0 * stddev_medium);
				range_low = (range_low < 0.0) ? 0.0 : range_low;

				if (outliers.size() > 0)
				{
					string test_message = _MSGA("Identified " << outliers.size() << " outlier(s), based on a threshold of "
						<< outlier_threshold << " which is three standard deviations from the mean of " << mean_medium << " microseconds (us).\n");
					test_message += _MSGA("Samples with execution times from " << range_low << " us to " << range_high << " us, are within that range.\n");
					test_message += format("Samples within this range are considered normal and contain {:6.3f}% of the samples.\n", (100.0 - outlier_percentage));
					test_message += "Samples outside this range are considered outliers. ";
					test_message += format("This represents {:6.3f}% of the samples.", outlier_percentage);
					test_message += "\nTested via Assert that the percentage of outliers is below 1%\n";
					test_message += "\nUp to the first 20 are shown. z_score is the number of standards of deviation the outlier varies from the mean.\n\n";
					test_message += " Iteration | Duration (us) | Z Score (us)  | \n";
					test_message += "-----------|---------------|---------------|\n";
					s32 outlier_limit = 20;
					s32 count = 0;
					for (auto& o : outliers) {
						if (count++ >= outlier_limit) break;
						test_message += format("{:10d} |", o.iteration);
						test_message += format("{:13.2f}  |", o.duration);
						test_message += format("{:13.4f}  |", o.z_score);
						test_message += "\n";
					}
					test_message += "\n";
					Assert::IsTrue(outlier_percentage < 1.0, L"Too many outliers, over 1% of total sample");
					Logger::WriteMessage(test_message.c_str());
				};

				// End of second batch
				x_i_medium.clear();
				z_scores_medium.clear();
			};
			// Third batch, long run
			{
				std::vector<double> x_i_long(timing_count_long);
				std::vector<double> z_scores_long(timing_count_long);
				outliers.clear();
				//Run target function timing_count_long times, capturing each time, also getting min, max, and total time spent
				for (int i = 0; i < timing_count_long; i++)
				{
					RandomFill(num1, &seed);
					num2 = RandomU64(&seed);
					auto countStart = std::chrono::steady_clock::now();
					div_uT64(dividend, &remainder, num1, num2);
					auto countEnd = std::chrono::steady_clock::now();
					std::chrono::duration<double, std::micro> countDur = countEnd - countStart;
					double duration = countDur.count();
					min_long = (duration < min_long) ? duration : min_long;
					max_long = (duration > max_long) ? duration : max_long;
					total_long += duration;
					x_i_long[i] = duration;
				};
				// Calculate mean, population variance, standard deviation, coefficient of variation, and z-scores
				{
					mean_long = total_long / double(timing_count_long);
					for (int i = 0; i < timing_count_long; i++)
					{
						double deviation = x_i_long[i] - mean_long;
						sample_variance_long += deviation * deviation;
					};
					sample_variance_long /= (double(timing_count_long) - 1.0);
					stddev_long = sqrt(sample_variance_long);
					coefficient_of_variation_long = (mean_long != 0.0) ? (stddev_long / mean_long) * 100.0 : 0.0;
					for (int i = 0; i < timing_count_long; i++)
					{
						z_scores_long[i] = (stddev_long != 0.0) ? (x_i_long[i] - mean_long) / stddev_long : 0.0;
					};
					string test_message = _MSGA("\nThird batch.\nRan for "
						<< timing_count_long << " samples.\nTotal target function (including c calling set-up) execution time: "
						<< total_long << " microseconds. \nAverage time per call : "
						<< mean_long << " microseconds.\nMinimum in "
						<< min_long << "\nMaximum in "
						<< max_long << "\n");

					test_message += _MSGA("Sample Variance: "
						<< sample_variance_long << "\nStandard Deviation: "
						<< stddev_long << "\nCoefficient of Variation: "
						<< coefficient_of_variation_long << "%\n\n");

					Logger::WriteMessage(test_message.c_str());
				};

				// Identify outliers, based on outlier_threshold
				for (int i = 0; i < timing_count_long; i++)
				{
					double z_sc = z_scores_long[i];
					double abs_z_score = (z_sc < 0.0) ? -z_sc : z_sc;
					outlier_threshold = 3.0 * stddev_long;
					if (abs_z_score > 3.0)
					{
						outlier o;
						o.iteration = i;
						o.duration = x_i_long[i];
						o.z_score = z_sc;
						outliers.push_back(o);
					};
				};

				// Report on outliers, if any
				double outlier_percentage = (double)(outliers.size() * 100.0) / (double)timing_count_long;
				double range_low = mean_long - (3.0 * stddev_long);
				double range_high = mean_long + (3.0 * stddev_long);
				range_low = (range_low < 0.0) ? 0.0 : range_low;
				if (outliers.size() > 0)
				{
					string test_message = _MSGA("Identified " << outliers.size() << " outlier(s), based on a threshold of "
						<< outlier_threshold << " which is three standard deviations from the mean of " << mean_long << " microseconds (us).\n");
					test_message += _MSGA("Samples with execution times from " << range_low << " us to " << range_high << " us, are within that range.\n");
					test_message += format("Samples within this range are considered normal and contain {:6.3f}% of the samples.\n", (100.0 - outlier_percentage));
					test_message += "Samples outside this range are considered outliers. ";
					test_message += format("This represents {:6.3f}% of the samples.", outlier_percentage);
					test_message += "\nTested via Assert that the percentage of outliers is below 1%\n";
					test_message += "\nUp to the first 20 are shown. z_score is the number of standards of deviation the outlier varies from the mean.\n\n";
					test_message += " Iteration | Duration (us) | Z Score (us)  | \n";
					test_message += "-----------|---------------|---------------|\n";
					s32 outlier_limit = 20;
					s32 count = 0;
					for (auto& o : outliers) {
						if (count++ >= outlier_limit) break;
						test_message += format("{:10d} |", o.iteration);
						test_message += format("{:13.2f}  |", o.duration);
						test_message += format("{:13.4f}  |", o.z_score);
						test_message += "\n";
					}
					test_message += "\n";
					Assert::IsTrue(outlier_percentage < 1.0, L"Too many outliers, over 1% of total sample");
					Logger::WriteMessage(test_message.c_str());
				};

				// End of third batch
				x_i_long.clear();
				z_scores_long.clear();
				outliers.clear();
			};
		}
	};
};
