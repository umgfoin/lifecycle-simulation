#pragma once
#include "comm.h"

class gpdx303s : public simple_comm
{
public:
	void off(bool bSoftoff = false);
	void set_Uout(float uout);
	bool get_Iout(float& iout);
	void enable_output(bool enable = true);
	void set_online(bool enable = true);
	void close(void);
	
public:
	float u_out = 0;
};