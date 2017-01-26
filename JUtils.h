#pragma once
#include <iostream>
#include <chrono>
#include <vector>

//Iterations per Second
class JIpsManager {
public:
	
	JIpsManager() : second(1) {	}

	void update(bool print_debug = false) {
		++iteration_count;
		t_last = t_now;
		t_now = std::chrono::high_resolution_clock::now();
		
		time_span = std::chrono::duration<double, std::milli>(t_now - t_last);
		iterations_per_second = (iterations_per_second + (second / time_span)) / 2.f;
		if (print_debug) {
			if (std::chrono::duration<double>(t_now - t_out).count() > 1) {
				t_out = std::chrono::high_resolution_clock::now();
				std::cout << "Iterations per second: " << iterations_per_second << '\n';
			}
		}
	}
	
	double ips() {
		return iterations_per_second;
	}
	
private:
	long iteration_count;
	double iterations_per_second;
	std::chrono::high_resolution_clock::time_point t_last, t_now, t_out;
	std::chrono::duration<double> time_span;
	std::chrono::seconds second;
};

class JDurationManager {
public:
	void start() {
		t_laps.clear();
		stopped = false;
		t_start = std::chrono::high_resolution_clock::now();
	}
	
	void lap() {
		if (!stopped) {
			t_laps.push_back(std::chrono::high_resolution_clock::now());
		}
	}
	
	void stop() {
		if (!stopped) {
			t_stop = std::chrono::high_resolution_clock::now();
		}
		stopped = true;
	}
	
	double getTotalTime() {
		return std::chrono::duration<double>(t_stop - t_start).count();
	}
	
	void print() {
		std::cout << "total Time: " << std::chrono::duration<double>(t_stop - t_start).count() << "s\n"; 
	}
private:
	std::chrono::high_resolution_clock::time_point t_start, t_stop;
	std::vector<std::chrono::high_resolution_clock::time_point> t_laps;
	bool stopped;
};