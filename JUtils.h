#pragma once
#include <iostream>
#include <chrono>
#include <vector>
#include <thread>

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
			t_laps.emplace_back(std::chrono::high_resolution_clock::now());
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
		std::cout << "----- total Time: " << std::chrono::duration<double>(t_stop - t_start).count() << "s -----\n"; 
    if (t_laps.size()>0) {
      std::cout << "lap 1: " << std::chrono::duration<double>(t_laps[0] - t_start).count() << "s\n";
      for (int i=2; i<t_laps.size(); ++i) {
        std::cout << "lap " << i << ": " << std::chrono::duration<double>(t_laps[i] - t_laps[i-1]).count() << "s\n";
      }
      std::cout << "lap " << t_laps.size() << ": " << std::chrono::duration<double>(t_stop - t_laps[t_laps.size()-1]).count() << "s\n";
    }
	}
private:
	std::chrono::high_resolution_clock::time_point t_start, t_stop;
	std::vector<std::chrono::high_resolution_clock::time_point> t_laps;
	bool stopped;
};

class JTimedIterationManager {
public:
  JTimedIterationManager() = delete;
  JTimedIterationManager(double ms) : counter(0), interval(ms) {}
  
  void set_interval(double ms) {
    interval = ms;
  }
  
  void start() {
    t_start = std::chrono::high_resolution_clock::now();
  }
  
  void wait() {
    //std::cout << "arrived at:              " << std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t_start).count()*1000 << '\n';
    //std::cout << "counter:                 " << counter << '\n';
    //std::cout << "should be released at:   " << (counter+1) * interval << '\n';
    time_till_next = interval - std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t_start).count()*1000 + interval*counter;
    //std::cout << "calculated waiting time: " << time_till_next << '\n';
    if (time_till_next>0) {
      std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(time_till_next*1000)));
    }
    ++counter;
    //std::cout << "released at:             " << std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t_start).count()*1000 << "\n --------------------- \n";
  }
private:
  std::chrono::high_resolution_clock::time_point t_start;
  unsigned long long counter;
  double interval, time_till_next;
};