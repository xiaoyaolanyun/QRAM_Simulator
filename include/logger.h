#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <time.h>
#include <chrono>
#include <map>
#include <fmt/core.h>
#include "util.h"

using namespace std;
using fmt::print;
using fmt::format;

constexpr double nanosec = 1;
constexpr double microsec = nanosec / 1e3;
constexpr double millisec = microsec / 1e3;
constexpr double sec = millisec / 1e3;
constexpr double minute = sec / 60;
constexpr double hour = minute / 60;
constexpr double day = hour / 24;

template<typename Ty>
string vec2str(vector<Ty> v, string lb = "[", string rb = "]", string sep = ", ") {
    if (v.size() == 0) {
        return lb + rb;
    }
    stringstream ret;
    for (size_t i = 0; i < v.size() - 1;++i) {
        ret << v[i] << sep;
    }
    ret << v.back();
    return lb + ret.str() + rb;
}

template<typename Ty>
string num2str(Ty num) {
    stringstream ret;
    ret << num;
    return ret.str();
}

inline string _datetime() {
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
    return buf;
}

struct timer {
    chrono::time_point<chrono::steady_clock> startpoint;
    timer() {
        startpoint = chrono::steady_clock::now();
    }
    double get(double unit) {
        chrono::nanoseconds m = chrono::steady_clock::now() - startpoint;
        return chrono::duration_cast<chrono::nanoseconds>(m).count() * unit;
    }
};

struct logger {
    ofstream out;    
    bool on = true;
    vector<timer> timers;
    logger(string filename) {
        out = ofstream(filename);
    }
    
    inline void set_on() { on = true; }
    
    inline void set_off() { on = false; }

    inline logger& info(string str) {
        if (on) out << str;
        return *this;
    }
    inline logger& operator<<(string str) {
        return info(str);
    }
    inline logger& linesplit() {
        return info( "----------------------------\n");        
    }   
    inline logger& datetime() {
        return info(_datetime());
    }
    inline void timer_start() {
        timers.push_back(timer());
    }
    double timer_end(double unit = sec) {
        if (timers.size() == 0) {
            return 0.0;
        }            
        double ret = timers.back().get(unit);
        timers.pop_back();
        return ret;
    }
};

struct profile {
    size_t ncalls = 0;
    double time = 0;
    vector<timer> timers;
    size_t max_depth = 100;
    profile(){
        enter();
    }
    void enter() {
        if (timers.size() == max_depth)
            throw runtime_error("Exceed max depth.");
        timers.push_back(timer());
        ncalls++;
    }
    void exit() {
        if (timers.size() == 0)
            throw runtime_error("Why profiler has 0 timer?");
        time += timers.back().get(millisec);
        timers.pop_back();
    }

};

struct profiler {
    static map<string, profile*> profiles;
    static bool on;
    string current_identifier;
    profile* current_profile;

    profiler(string function_identifier) {
        if (!on) { return; }
        if (function_identifier.size() > 25) {
            current_identifier.assign(function_identifier.begin(), function_identifier.begin() + 15);
            current_identifier += "...";
        }
        else {
            current_identifier = function_identifier;            
        }
        auto iter = profiles.find(function_identifier);
        if (iter == profiles.end()) {
            current_profile = new profile();
            profiles.insert({ current_identifier, current_profile });
        }
        else {
            current_profile = iter->second;
            current_profile->enter();
        }
    }

    ~profiler() {
        current_profile->exit();
    }

    inline static void init_profiler() {
        profiles.clear();
    }

    inline static void close_profiler() {
        profiler::on = false;
    }

    inline static void start_profiler(){
        profiler::on = true;
    }

    inline static double get_time(string profilename) {
        auto iter = profiles.find(profilename);
        if (iter == profiles.end()) return -1.0;
        else return iter->second->time;
    }

    inline static size_t get_ncalls(string profilename) {  
        auto iter = profiles.find(profilename);
        if (iter == profiles.end()) return -1;
        else return iter->second->ncalls;
    }

    inline static string get_all_profiles() {
        if (profiles.empty()) {
            return "No profiles.";
        }
        string ret;
        for (auto profile : profiles) {
            ret += format("[{:^28s}] Calls = {:^3d} Time = {:^4f} ms\n",
                profile.first, profile.second->ncalls, profile.second->time);
        }
        return ret;
    }
    inline static string get_all_profiles_v2() {
        string ret;
        if (profiles.empty()) {
            return "No profiles.";
        }
        else {
            ret += format("Item: {}\n", profiles.size());
        }
        vector<pair<void*, void*>> profvec;
        map2vec(profvec, profiles);

        auto get_time = [](const pair<void*, void*>& item) {
            return (*static_cast<profile**>(item.second))->time;
        };
        auto get_name = [](const pair<void*, void*>& item) {
            return (*static_cast<string*>(item.first));
        };
        auto get_profile = [](const pair<void*, void*>& item) {
            return (*static_cast<profile**>(item.second));
        };

        sort(profvec.begin(), profvec.end(), [&get_time](
            const pair<void*, void*>& item1,
            const pair<void*, void*>& item2)
            {
                return get_time(item1) > get_time(item2);
            }
        );

        for (const auto& profile : profvec) {
            ret += format("[{:^28s}] Calls = {:^3d} Time = {:^4f} ms\n",
                get_name(profile), get_profile(profile)->ncalls, get_profile(profile)->time);
        }
        return ret;
    }
};

#define FunctionProfiler volatile profiler _profilehelper_(__FUNCTION__)

extern logger _logger;