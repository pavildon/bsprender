//
// Created by Patricio Villalobos on 9/25/15.
//

#ifndef TESSELLATION_BENCHMARKS_H
#define TESSELLATION_BENCHMARKS_H


#include <string>
#include <map>
#include <chrono>
#include <iostream>

using namespace std;

struct Benchmark {
    chrono::high_resolution_clock::time_point start;
    chrono::high_resolution_clock::time_point end;
    chrono::microseconds avg;
    chrono::microseconds acc;
    unsigned int count;
};

class Benchmarks {

public:
    static void start(const std::string &name) {
        map<string, Benchmark>::iterator bmi = benchmarks.find(name);

        if (bmi == benchmarks.end()) {
            Benchmark bm{};
            bm.start = chrono::high_resolution_clock::now();
            bm.count = 0;
            benchmarks.insert(std::make_pair(name, bm));
            return;
        }

        bmi->second.start = chrono::high_resolution_clock::now();

    }

    static void stop(const string &name) {
        auto bmi = benchmarks.find(name);

        if (bmi == benchmarks.end()) return;

        bmi->second.end = chrono::high_resolution_clock::now();

        bmi->second.count++;

        bmi->second.acc +=
                chrono::duration_cast<chrono::microseconds>
                        (bmi->second.end - bmi->second.start);

        bmi->second.avg = bmi->second.acc / bmi->second.count;

    }

    static void results() {
        for_each(benchmarks.begin(), benchmarks.end(), [] (const pair<const string, Benchmark> &p) {
            cout << "-- " << p.first << endl;
            cout << "AVG (ms) " << (p.second.avg.count()/1000.0f) << " COUNT :" << p.second.count << endl;
        });

    }

    static void reset() {
        for(auto &p: benchmarks) {
            p.second.count = 0;
            p.second.avg = chrono::microseconds(0);
            p.second.acc = chrono::microseconds(0);
        }
    }

private:

    static std::map<std::string, Benchmark> benchmarks;

};



#endif //TESSELLATION_BENCHMARKS_H
