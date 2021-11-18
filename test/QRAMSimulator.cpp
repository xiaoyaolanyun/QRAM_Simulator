#include <exception>
#include <typeinfo>

#include "data_structures.h"
#include "qram.h"
#include "logger.h"

logger _logger("test.out");

int main() {

	_logger.timer_start();	
	_logger.datetime();
	_logger.linesplit();
	try {
		profiler m("Main Process");
		QRAM_bb qram(3);
		qram.add_noise_models(
			{
				{OperationType::BitFlip, 0.01},
				// {OperationType::PhaseFlip, 0.01},
				// {OperationType::Damp, 0.01},
			}
		);
		qram.generate_QRAM_operations();
		qram.append_noise();
		qram.set_memory_random();
		qram.set_address_full();
		
		qram.run();
		double fidelity = qram.get_fidelity();
		qram.state_view_preparation();
		_logger << format("{}\n", qram.state_view_to_string());
	}
	catch (exception& e) {
		_logger << format("{} : {}\n", typeid(e).name(), e.what());
	}

	_logger.linesplit();

	_logger << format("time : {} ms\n", _logger.timer_end(millisec));

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