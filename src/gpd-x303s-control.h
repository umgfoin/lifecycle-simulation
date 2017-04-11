#pragma once
//#include "win_serial_comm.h"
#include "asio_serial_comm.h"

class gpdx303s : public asio_serial_comm /*win_serial_comm*/
{
public:
	~gpdx303s(){ close(); };

	void off(bool bSoftoff = false);
	void set_Uout(float uout);
	bool get_Iout(float& iout);
	void enable_output(bool enable = true);
	void set_online(bool enable = true);
	void close(void);
	
public:
	float u_out = 0;
};