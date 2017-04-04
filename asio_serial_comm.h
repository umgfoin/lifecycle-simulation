#pragma once

#define ASIO_STANDALONE

#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
//#include <boost/bind.hpp>
//#include <boost/thread.hpp>

using namespace boost;
using namespace std;

class asio_serial_comm
{
public:
	~asio_serial_comm();

	bool open_comm(const char* port, unsigned linespeed = 0);
	void setup_comm(unsigned linespeed);
	void close_comm(void);
	bool is_open(void);
	void send_sync(const char* psz_cmd);
	size_t rcv_sync(unsigned char* buf, size_t size);
	void clear_rcv_buffer(void);
	void clear_send_buffer(void);
	void clear_io_buffers(unsigned method /*0: RX-queue, 1: TX-queue, 2: both queues*/);

protected:
	asio::io_service					io_service;
	std::unique_ptr<asio::serial_port>  p_serial;
};

