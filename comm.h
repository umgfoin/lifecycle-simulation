#pragma once
#include "stdafx.h"

class simple_comm
{
public:
	 ~simple_comm(){ close_comm(); };

	void setup_comm(unsigned baudrate);
	bool open_comm(LPCSTR psz_port, int baud_rate = -1);
	void close_comm(void);
	void clear_rcv_buffer(void);
	void send_sync(LPCSTR psz_cmd);
	size_t rcv_sync(unsigned char* buf, size_t size);

public:
	HANDLE hFile = INVALID_HANDLE_VALUE;
};