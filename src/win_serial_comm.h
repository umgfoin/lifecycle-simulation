#pragma once
#include "stdafx.h"
#include <Windows.h>

class win_serial_comm
{
public:
	 ~win_serial_comm(){ close_comm(); };

	void setup_comm(unsigned baudrate);
	bool open_comm(LPCSTR psz_port, unsigned linespeed = 0);
	void close_comm(void);
	bool is_open(void);
	void clear_rcv_buffer(void);
	void send_sync(LPCSTR psz_cmd);
	size_t rcv_sync(unsigned char* buf, size_t size);

protected:
	HANDLE hFile = INVALID_HANDLE_VALUE;
};