#include "operators.h"
#include "logger.h"
#include "qram.h"
#include "stl.h"
#include "stl_bind.h"
#include "pytypes.h"
#include "pybind11.h"
#include "complex.h"

using namespace std;
namespace py = pybind11;

#define classfunc(classname, funcname) .def(#funcname, &classname::funcname)
#define exportenum(classname, enumname) .value(#enumname, classname::enumname)
#define strdefine(classname) .def("__str__", &classname::to_string).def("__repr__", &classname::to_string)
#define strdefine_hassimple(classname) .def("__str__", &classname::to_string).def("__repr__", &classname::to_string_simple)

PYBIND11_MODULE(pyqram, m)
{
    m.doc() = "";

// #define loggerdef(funcname) classfunc(Logger, funcname)    
//     py::class_<Logger>(m, "Logger")
//         .def(py::init<string>())
//         .def("__lshift__", &Logger::operator<<, py::is_operator())
//         loggerdef(linesplit)
//         loggerdef(datetime)
//         loggerdef(timer_start)
//         loggerdef(timer_end)
//         ;
// #undef loggerdef
    
#define timeslicedef(funcname) classfunc(TimeSlices, funcname)
    py::class_<TimeSlices>(m, "TimeSlices")
        timeslicedef(to_string)
        strdefine(TimeSlices)
        ;

#undef treedef

#define treedef(funcname) classfunc(Tree, funcname)
    py::class_<Tree>(m, "Tree")
        treedef(get_nlayer)
        treedef(is_same)
        treedef(less_than)
        treedef(to_string)
        strdefine(Tree)
        .def("__eq__", &Tree::operator==)
        .def("__lt__", &Tree::operator<)
        ;
#undef treedef

#define branchdef(funcname) classfunc(Branch, funcname)
    py::class_<Branch>(m, "Branch")
        branchdef(to_string)
        branchdef(get_state)
        strdefine_hassimple(Branch)
        .def("__eq__", &Branch::operator==)
        .def("__lt__", &Branch::operator<)
        ;
#undef branchdef

#define qramdef(funcname) classfunc(QRAM_bb, funcname)
    py::class_<QRAM_bb>(m, "QRAM_bb")
        .def(py::init<int>())
        qramdef(set_address)
        qramdef(set_address_sample)
        qramdef(set_address_full)
        qramdef(set_memory)
        qramdef(set_memory_random)
        qramdef(get_qubit_num)
        qramdef(set_seed)
        qramdef(get_seed)
        qramdef(get_address)
        qramdef(clear_address)
        qramdef(get_memory)
        qramdef(asz)
        qramdef(get_branches)
        qramdef(get_operations)
        qramdef(clear_noise)
        qramdef(add_noise_model)
        qramdef(add_noise_models)        
        qramdef(generate_QRAM_operations)
        // qramdef(state_view)
        qramdef(get_global_coef)
        qramdef(get_fidelity)
        qramdef(append_noise)
        qramdef(run)
        qramdef(to_string)
        qramdef(state_view_preparation)
        qramdef(state_view_to_string)
        qramdef(get_state_tree_view)
        strdefine(QRAM_bb)
        ;
#undef qramdef

#define profilerdef(funcname) classfunc(profiler, funcname)
    py::class_<profiler>(m, "profiler")
        .def(py::init<string>())
        ;
#undef profilerdef

    m.def("get_all_profiles_v2", []() {return profiler::get_all_profiles_v2();});
    m.def("get_all_profiles", []() {return profiler::get_all_profiles();});
    m.def("start_profiler", []() {profiler::start_profiler();});
    m.def("close_profiler", []() {profiler::close_profiler();});
    m.def("init_profiler", []() {return profiler::init_profiler();});
    m.def("get_time", [](string name) {return profiler::get_time(name);});
    m.def("get_ncalls", [](string name) {return profiler::get_ncalls(name);});

#define operationtypedef(enumname) exportenum(OperationType, enumname)
    py::enum_<OperationType>(m, "OperationType")
        operationtypedef(Begin)
        operationtypedef(ControlSwap)
        operationtypedef(HadamardData)
        operationtypedef(SwapInternal)
        operationtypedef(FirstCopy)
        operationtypedef(FetchData)
        operationtypedef(SetZero)
        operationtypedef(Damp)
        operationtypedef(BitFlip)
        operationtypedef(End)
        .export_values()
        ;
#undef operationtype
    
}


