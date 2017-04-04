// main.cpp : Defines the entry point for the console application.
// (C) 2017 Armin Schöffmann & AEGAEON technologies GmbH


#include "stdafx.h"
#include <stdlib.h>  
#include <limits.h> 
#include <iostream>
#include <iomanip>

#include "gpd-x303s-control.h"
#include "files.h"
#include "main.h"

using namespace std;

bool		stop_flag = false;

state_file	states(L"state.cfg");
config_file config(L"config.cfg");
log_file	logger(L"events.log");
gpdx303s	vsource;


int main()
{
	// get persistant states and settings
	if ( !states.read() ){
		wcout << "Failed reading statefile (" << states.psz_name << ")." << endl;
		return 0;
	}

	if ( !config.read() ){
		wcout << "Failed reading config-file (" << config.psz_name << ")." << endl;
		return 0;
	}

	if ( !vsource.open_comm(config.str_serialport.c_str(), 115200 )){
		wcout << "Failed to open serial-port (" << config.str_serialport.c_str() << ")." << endl;
		return 0;
	}

	SetConsoleCtrlHandler(HandlerRoutine, TRUE);
	
	try{

		vsource.set_online();
		vsource.set_Uout(config.u_range[MIN]);
		vsource.enable_output();

		stringstream log_event;
		log_event.setf(ios::left | ios::fixed);
		log_event.precision(1);
		
		log_event << "Simulation started.";
		logger << log_event;

		// define period[ms] of jitter frequency
		const unsigned period = 200;
		
		// init P to a value meeting p_range-constraints
		float P = config.p_range[MAX] - config.p_range[MIN] / 2;
		bool power_good = true;

		LPCWSTR	status_line = L"cycle: %d [%s] t: %3d/%3ds, sigma: %.1fh, U: %4.1fV, I: %4dmA, P: %.2fW\r";

		for(;;) {

			unsigned t_on = get_random(config.t_on_range[MIN], config.t_on_range[MAX]);
			unsigned t_off = get_random(config.t_off_range[MIN], config.t_off_range[MAX]);
			//t_on = get_random(3, 5);
			//t_off = get_random(1, 5);

			// only count duty-cycles if within power-constraints
			if (power_good)
				states.cycles++;

			for ( unsigned t = t_on; t--; ){
				// duty cycle
				float U, I;
				for ( int i = 1000 / period; i--; ){
					if ( stop_flag )
						throw 0;

					// update jitter every 200ms
					DWORD t1 = GetTickCount();

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

					// delay for remainder of period
					DWORD ticks = GetTickCount() - t1;
					if ( ticks < period )
						Sleep (period - ticks);
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
						0.f,
						0.f
				);
				Sleep (1000);
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

	if ( !states.write() )
		wcout << "Failed writing statefile (" << states.psz_name << ")." << endl;
	
	vsource.close();
}

unsigned get_random(unsigned low, unsigned high)
{
	unsigned urandom;

	rand_s(&urandom);
	urandom = (unsigned)( ((double) urandom / (double) UINT_MAX) * (double) (high - low + 1) + low);

	return urandom;
}

BOOL WINAPI HandlerRoutine(DWORD dwCtrlType)
{
	switch ( dwCtrlType ){
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		stop_flag = true;
		break;
	case CTRL_CLOSE_EVENT:
		stop_flag = true;
		tidy_up();
		break;
	default:
		break;
	}
	return TRUE;
}