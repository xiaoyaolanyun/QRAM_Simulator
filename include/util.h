#pragma once
#include <complex>
#include <type_traits>
#include <algorithm>
#include <numeric>
#include <map>
#include <vector>
#include <random>
#include <chrono>
#include <set>
using namespace std;

template<typename Ty>
auto abs_sqr(const complex<Ty>& c) -> Ty {
    return c.real() * c.real() + c.imag() * c.imag();
}

constexpr double epsilon = 1.e-7;

constexpr bool ignorable(const double v) {
    if (v > -epsilon && v < epsilon) return true;
    else return false;
}

template<typename Ty>
constexpr bool ignorable(const complex<Ty>& v) {
    Ty value = abs_sqr(v);
    if (ignorable(value)) return true;
    else return false;
}

template<typename T>
struct remove_cvref {
    typedef remove_cv_t<remove_reference_t<T>> type;
};

template<typename T>
using remove_cvref_t = typename remove_cvref<T>::type;

template<typename Ty>
void* to_voidptr(Ty ptr) {
    using T_ptr_t = remove_cvref_t<Ty>;
    using T = remove_cvref_t<remove_pointer_t<T_ptr_t>>;
    using clear_pointer_type = T*;
    return reinterpret_cast<void*>(const_cast<clear_pointer_type>(ptr));
}

template<typename KeyTy, typename ValTy>
void map2vec(vector<pair<void*, void*>>& vec, const map<KeyTy, ValTy>& map1) {
    vec.clear();
    vec.reserve(map1.size());
    for (const auto& item : map1) {
        void* keyptr = to_voidptr(&(item.first));
        void* valptr = to_voidptr(&(item.second));
        vec.push_back({ keyptr, valptr });
    }
}

template<typename rng_t = default_random_engine>
void random_memory(vector<bool>& memory, rng_t& reng) {
    size_t size = memory.size();
    uniform_int_distribution ud(0, 1);
    for (size_t i = 0; i < memory.size(); ++i) {
        memory[i] = ud(reng);
    }
}

template <typename FwdIt, typename Pred, typename Func>
FwdIt unique_and_merge(FwdIt first, FwdIt last, Pred pred, Func fn)
{
    if (first == last) return last;

    FwdIt result = first;
    while (++first != last)
    {
        if (!pred(*result, *first))
            *(++result) = *first;
        else
            fn(*result, *first);
    }
    return ++result;
}

template<typename Rng>
void choice_from(set<size_t>& samples, int size, size_t n_samples, Rng& g)
{
    samples.clear();
    uniform_int_distribution<size_t> ud(0, 1ull << size);
    while (n_samples > 0) {
        if (samples.insert(ud(g)).second) { n_samples--; };
    }
}

template<typename T>
bool vector_less(const vector<T>& v1, const vector<T>& v2)
{
    if (v1.size() < v2.size()) return true;
    if (v1.size() > v2.size()) return false;

    size_t size = v1.size();
    for (size_t i = 0; i < size; ++i) {
        if (v1[i] < v2[i]) return true;
        if (v2[i] < v1[i]) return false;
    }
    return false;
}

inline vector<double> linspace(double min, double max, size_t points) {
	double delta = (max - min) / (points - 1);
	vector<double> ret;
	ret.reserve(points);
	for (size_t i = 0; i < points; ++i) {
		ret.push_back(min + delta * i);
	}
	return ret;
}

inline pair<double, double> mean_std(vector<double> m) {
	auto sq = [](double m, double y) {
		return m + y * y;
	};

	double sum = accumulate(m.begin(), m.end(), 0.0);
	double sumsq = accumulate(m.begin(), m.end(), 0.0, sq);
	double mean = sum / m.size();
	double meansq = sumsq / m.size();
	return { mean, sqrt(meansq - mean * mean) };

}