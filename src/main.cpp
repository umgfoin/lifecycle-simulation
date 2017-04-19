// main.cpp : Defines the entry point for the console application.
// (C) 2017 Armin Schöffmann & AEGAEON technologies GmbH


#include "stdafx.h"
#include <cstdlib>  
#include <limits> 
#include <locale> 
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <future>
#include <csignal>
#include "gpd-x303s-control.h"
#include "files.h"
#include <boost/random/discrete_distribution.hpp>
#include <boost/random/random_device.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include "main.h"

using namespace std;
using namespace std::chrono;
using namespace std::this_thread;
using namespace boost;

namespace fs = boost::filesystem;

atomic<bool>	      stop_flag(false);
state_file*           states = nullptr;
gpdx303s	          vsource;
random::random_device dev_random;



int main(int argc, char* argv[] )
{
	// set OS-defaults for character-conversion
	setlocale(LC_CTYPE, "");

	// constructs absolute path based on exe-dir
	fs::path base_path (fs::system_complete(argv[0]).parent_path());

	// using move-semantics
	state_file	states (fs::complete(L"state.cfg",  base_path) );
	config_file config (fs::complete(L"config.cfg", base_path) );
	log_file	logger (fs::complete(L"events.log", base_path) );

	// get persistant states and settings
	if ( !states.read() ){
		wcout << "Failed reading statefile (" << states.file_path.filename() << ")." << endl;
		return 0;
	}
	::states = &states;

	if ( !config.read() ){
		wcout << "Failed reading config-file (" << config.file_path.filename() << ")." << endl;
		return 0;
	}

	if ( !vsource.open_comm(config.str_serialport.c_str(), 115200 )){
		wcout << "Failed to open serial-port (" << config.str_serialport.c_str() << ")." << endl;
		return 0;
	}

	signal(SIGINT,   sig_handler);
	signal(SIGBREAK, sig_handler);
	signal(SIGTERM,  sig_handler);
	signal(SIGABRT,  sig_handler);
	
	try{

		vsource.set_online();
		vsource.set_Uout(config.u_range[MIN]);
		vsource.enable_output();

		std::stringstream log_event;
		log_event.setf(ios::left | ios::fixed);
		log_event.precision(1);
		
		logger << "Simulation started.";

		// define period[ms] of jitter frequency
		const milliseconds period(200);
		
		// init P to a value meeting p_range-constraints
		float P = config.p_range[MAX] - config.p_range[MIN] / 2;
		bool power_good = true;

		const wchar_t*	status_line = L"cycle: %d [%ls] t: %3d/%3ds, sigma: %.1fh, U: %4.1fV, I: %4dmA, P: %.2fW\r";

		for(;;) {

			unsigned t_on  = get_random(config.t_on_range[MIN], config.t_on_range[MAX]);
			unsigned t_off = get_random(config.t_off_range[MIN], config.t_off_range[MAX]);
			//t_on = get_random(3, 5);
			//t_off = get_random(1, 5);

			// only count duty-cycles if within power-constraints
			if (power_good)
				states.cycles++;

			for ( unsigned t = t_on; t--; ){
				// duty cycle
				float U, I;
				for ( int i = 1000 / (int) period.count(); i--; ){
					if ( stop_flag )
						throw 0;

					// update jitter every 200ms
					auto t1 = steady_clock::now();

					float jitter = (float) get_random(0, (unsigned) ((config.u_range[MAX] - config.u_range[MIN]) * 10.f)) / 10.f;

					U = config.u_range[MIN] + jitter;
					vsource.set_Uout(U);

					I = 0;
					vsource.get_Iout(I);

					P = I * U;

					wprintf(status_line,
							states.cycles,
							L" on",
							t,
							t_on,
							(float) states.sigma_t / 3600.f,
							U, (unsigned) (I * 1000.f),
							P
					);

					wcout.flush();
					// delay for remainder of period
					milliseconds ms_gone = duration_cast<milliseconds>(steady_clock::now() - t1);

					if ( ms_gone < period )
						sleep_for(period - ms_gone);
				}
				// update power_good-state
				bool last_power_good = power_good;
				power_good = (P > config.p_range[MIN] && P < config.p_range[MAX]);
				
				// only count duty-time if within power-constraints
				if (power_good)
					states.sigma_t++;

				else if(last_power_good){
					log_event << "Power violation: " << U << "V, " << setprecision(3) << I << "A, " << P << "W [U,I,P]" << setprecision(1);
					logger << log_event;
				}

			}
			log_event << "Duty cycle finished: T=" << t_on << "s, SigmaT=" << (float) states.sigma_t / 3600.f << "h";
			logger << log_event;

			// dark cycle
			bool softoff;
			vsource.off(softoff = (get_random(0, 1) == 1));

			std::printf("%79s\r", "");

			for ( unsigned t = t_off; t--; ){
				if ( stop_flag )
					throw 0;

				wprintf(status_line,
						states.cycles,
						softoff ? L"dim" : L"off",
						t,
						t_off,
						(float) states.sigma_t / 3600.f,
						0.f,
						0,
						0.f
				);
				wcout.flush();
				sleep_for(seconds(1));
			}
			log_event << "Dark cycle finished: T=" << t_off << "s";
			logger << log_event;
		}
	}

	catch (...)
	{
	}

	tidy_up();

 	logger << stringstream("Simulation stopped");
 
	return 0;
}

void tidy_up(void)
{

	if ( states && (!states->write()) )
		wcout << "Failed writing statefile (" << states->file_path.filename() << ")." << endl;
	
	vsource.close();
}

unsigned get_random(unsigned low, unsigned high)
{
	//boost::random::mt19937 generator;
	random::uniform_int_distribution<> distribution(low, high);
	return distribution(dev_random);
}

void sig_handler(int sig)
{
	switch ( sig ){
	// CTRL-C
	case SIGINT:
	case SIGABRT:
		stop_flag = true;
			break;

	case SIGTERM:
	// console-close
	case SIGBREAK:
		// on windows, signal-handlers run in seperate thread
		// leaving this function will remove the main-thread
		// thus waiting here, let the main thread exit safely
		stop_flag = true;
		sleep_for(seconds(2));
//		promise<void>().get_future().wait();
		break;
	default:
		break;
	}
}