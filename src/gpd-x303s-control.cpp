#include "stdafx.h"
#define __STDC_WANT_LIB_EXT1__1
#include <cstdio>
#include <chrono>
#include <thread>
#include "gpd-x303s-control.h"

using namespace std;
using namespace std::chrono;
using namespace std::this_thread;

void gpdx303s::set_online(bool enable)
{
	send_sync(enable ? "REMOTE\n" : "LOCAL\n");
	if ( enable ){
		sleep_for(milliseconds(500));
		clear_rcv_buffer();
	}
}

void gpdx303s::enable_output(bool enable)
{
	char psz_cmd[10];
	sprintf(psz_cmd, "OUT%d\n", enable ? 1 : 0);
	
	send_sync(psz_cmd);
}


void gpdx303s::set_Uout(float u_new)
{
	u_out = u_new;
	char psz_cmd[32];
	// safety limit 14.0V
	sprintf(psz_cmd, "VSET1:%.1f\n", min(u_out, 14.f));

	send_sync(psz_cmd);
	sleep_for(milliseconds(10));
}

void gpdx303s::off(bool bSoftoff)
{
	if ( bSoftoff ){
		float step = .1f;

		for ( float u = u_out; u > 0.; u -= step ){
			set_Uout(u);
			sleep_for(milliseconds(50));
		}
	}
	set_Uout(0.);
}

bool gpdx303s::get_Iout(float& iout)
{
	unsigned char buf[11];
	const char* psz_cmd = "IOUT1?\n";

	send_sync(psz_cmd);

	for ( int retry = 10; retry--; ){
		sleep_for(milliseconds(10));
		size_t inbytes = rcv_sync(buf, sizeof(buf) - 1);
		if ( inbytes ){
			buf[inbytes] = 0;
			sscanf((char*) buf, "%fA", &iout);
			//printf("%d\r\n", retry);
			return true;
		}
	}

	return false;
}

void gpdx303s::close(void)
{
	if ( is_open() ){
		off();
		enable_output(false);
		set_online(false);
		close_comm();
	}
}
