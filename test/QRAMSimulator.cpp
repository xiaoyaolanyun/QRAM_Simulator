#include <exception>
#include <typeinfo>

#include "data_structures.h"
#include "qram.h"
#include "logger.h"

void test_distribution(
	int size,
	int n_trials,
	int n_samples,
	unsigned int &init_seed,
	// vector<QRAM_bb::noise_type> noise_types,
	double noise,
	bool verbose
) 
{
	/* Sample Test Code */
	/*
		double noise_min = -6;
		double noise_max = -2;
		double noise_step = 0.5;

		int size_min = 3;
		int size_max = 12;
		int size_step = 1;

		int n_trials = 10000;
		size_t n_sample = 10000;

		unsigned int seed = 16161;
		bool verbose = true;
		int size;
		double noise;

		for (noise = noise_min; noise <= noise_max; noise += noise_step) {
			double actual_noise = pow(10, noise);

			for (size = size_min; size <= size_max; size += size_step) {
				test_distribution(size, n_trials, n_sample, seed, actual_noise, verbose);
			}
		}
	*/
	timer t;
	vector<QRAM_bb::noise_type> noise_types	= { {OperationType::BitFlip, noise } };
	vector<double> res = qram_fid_sample_address(
		size, n_trials, noise_types, n_samples, init_seed, verbose);
	double mean, std; 
	tie(mean, std) = mean_std(res);

	print_and_log("Size = {:3d} Noise = {:.2e} | Mean = {:.4e} Std = {:.4e} Sigma = {:.4e}",
		size, noise, mean, std, std / sqrt(n_trials));

	double time = t.get(millisec);
	string unt = "ms";
	if (time > 100000) { time /= 1000; unt = "s"; }

	print_and_log("(duration = {} {})\n", time, unt);
}

int main() {
	logger.newfile_auto();
	logger.timer_start();	
	logger.datetime();
	logger.linesplit();
	try {
		profiler m("Main Process");
		double noise_min = -4.5;
		double noise_max = -2.0;
		double noise_step = 0.5;

		int size_min = 3;
		int size_max = 10;
		int size_step = 1;

		int n_trials = 10000;
		int n_sample = 10000;

		unsigned int seed = 16161;
		bool verbose = false;
		int size;
		double noise;

		for (noise = noise_min; noise <= noise_max; noise += noise_step) {
			double actual_noise = pow(10, noise);

			for (size = size_min; size <= size_max; size += size_step) {
				print("{:.2f} ({:.2e}) {:3d} | ", noise, actual_noise, size);
				test_distribution(size, n_trials, n_sample, seed, actual_noise, verbose);				
			}
		}
	}
	catch (exception& e) {
		logger << format("{} : {}\n", typeid(e).name(), e.what());
	}

	logger.linesplit();

	logger << format("time : {} ms\n", logger.timer_end(millisec));

	print("{} {} \n", _datetime(), " ** Success ** ");
	print("Profiles : \n");
	print("{}", profiler::get_all_profiles_v2());

	return 0;
}

// int main() {
// 	Tree tree1(2);
// 	Tree tree2(2);

// 	tree2.root()->addr.NOT();

// 	print(" {} \n {} \n", tree1.to_string(), tree2.to_string());

// 	print("{} ; {} ; {}", tree1 < tree2, tree2 < tree1, tree1 == tree2);

// 	return 0;

// }

// int main() {
// 	vector<pair<int, int>> a
// 		= {

// 		{1,1},
// 		{1,1},
// 		{1,1},
// 		{1,1},
// 		{1,1},
// 		{2,1},
// 		{2,1},
// 		{2,-1},
// 		{2,-1},
// 		{3,1},
// 		{3,1},
// 		{3,1},
// 		{3,1},
// 		{3,1},
// 	};
	
// 	auto iter = unique_and_merge(a.begin(), a.end(),
// 		[](pair<int, int>& m1, pair<int, int>& m2) {
// 			return m1.first == m2.first;
// 		},
// 		[](pair<int, int>& m1, pair<int, int>& m2) {
// 			return m1.second += m2.second;
// 		}
// 		);
//     auto remove = [](const pair<int, int>& b)
//     {
//         return ignorable(b.second);
//     };


// 	iter = remove_if(a.begin(), iter, remove);
// 	a.resize(distance(a.begin(), iter));

// 	for (auto m : a) {
// 		printf("(%d,%d)", m.first, m.second);
// 	}

// 	return 0;
	
// }