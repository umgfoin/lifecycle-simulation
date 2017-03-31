// main.cpp : Defines the entry point for the console application.
// (C) 2017 Armin Schöffmann & AEGAEON technologies GmbH


#include "stdafx.h"
#include <stdlib.h>  
#include <stdio.h>  
#include <limits.h> 

#include "gpd-x303s-control.h"
#include "files.h"
#include "main.h"

using namespace std;

bool		stop_flag = false;

state_file	states(L"state.cfg");
config_file config(L"config.cfg");
gpdx303s	power;


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

	if ( !power.open_comm(config.str_serialport.c_str(), 115200 )){
		wcout << "Failed to open serial-port (" << config.str_serialport.c_str() << ")." << endl;
		return 0;
	}

	SetConsoleCtrlHandler(HandlerRoutine, TRUE);
	
	try{

		power.set_online();
		power.set_Uout(config.u_range[MIN]);
		power.enable_output();

		// define period[ms] of jitter frequency
		const unsigned period = 200;

		LPCWSTR		status_line = L"cyc: %d, status: %s, t[s]: %3d/%3d, sig[h]: %.1f, U[V]: %4.1f, I[mA]: %4d\r";
		for(;;) {

			unsigned t_on = get_random(config.t_on_range[MIN], config.t_on_range[MAX]);
			unsigned t_off = get_random(config.t_off_range[MIN], config.t_off_range[MAX]);
			//t_on = get_random(3, 5);
			//t_off = get_random(1, 5);

			states.cycles++;

			for ( unsigned t = t_on; t--; ){
				// duty cycle
				for ( int i = 1000 / period; i--; ){
					if ( stop_flag )
						throw 0;

					// update jitter every 200ms
					DWORD t1 = GetTickCount();

					float jitter = (float) get_random(0, (unsigned) ((config.u_range[MAX] - config.u_range[MIN]) * 10.f)) / 10.f;

					float vadj = config.u_range[0] + jitter;
					power.set_Uout(vadj);

					float ICur = 0;
					power.get_Iout(ICur);

					wprintf(status_line,
							states.cycles,
							L" on",
							t,
							t_on,
							(float) states.sigma_t / 3600.f,
							vadj, (unsigned) (ICur * 1000.f)
					);

					// delay for reminder of period
					DWORD ticks = GetTickCount() - t1;
					if ( ticks < period )
						Sleep (period - ticks);
				}
				// count duty seconds
				states.sigma_t++;
			}

			// dark cycle
			printf("%79s\r", "");

			bool softoff;
			power.off(softoff = (get_random(0, 1) == 1));

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
						0.f
				);
				Sleep (1000);
			}

		}
	}

	catch (...)
	{
	}

	tidy_up();
  
	return 0;
}

void tidy_up(void)
{
	if ( !states.write() )
		wcout << "Failed writing statefile (" << states.psz_name << ")." << endl;
	
	power.close();
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