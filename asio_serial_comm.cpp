#include "stdafx.h"
#include "asio_serial_comm.h"

asio_serial_comm::~asio_serial_comm()
{
	close_comm();
}

bool asio_serial_comm::open_comm(const char* port, unsigned linespeed)
{
	system::error_code error;

	p_serial = std::make_unique<asio::serial_port>(io_service);
	p_serial->open(port, error);

	if ( error )
		return false;
	
	if ( linespeed )
		setup_comm(linespeed);

	return true;
}

void asio_serial_comm::close_comm(void)
{
	if ( p_serial ){
		p_serial->cancel();
		p_serial->close();
		p_serial.reset();
	}
	io_service.stop();
	io_service.reset();
}

void asio_serial_comm::setup_comm(unsigned linespeed)
{
	p_serial->set_option(asio::serial_port_base::baud_rate(linespeed));
	p_serial->set_option(asio::serial_port_base::character_size(8));
	p_serial->set_option(asio::serial_port_base::stop_bits(asio::serial_port_base::stop_bits::one));
	p_serial->set_option(asio::serial_port_base::parity(asio::serial_port_base::parity::none));
	p_serial->set_option(asio::serial_port_base::flow_control(asio::serial_port_base::flow_control::none));
}

void asio_serial_comm::send_sync(const char* buf)
{
	system::error_code error;

	size_t size = strlen(buf);
	if ( p_serial ){
		p_serial->write_some(asio::buffer(buf, size), error);
	}
}

size_t asio_serial_comm::rcv_sync(unsigned char* buf, size_t size)
{
	system::error_code error;
	size_t read;

	if ( p_serial ){
		read = p_serial->read_some(asio::buffer (buf, size), error);
	}
	return read;
}

bool asio_serial_comm::is_open(void)
{
	return (bool) p_serial;
}

void asio_serial_comm::clear_rcv_buffer(void)
{
	clear_io_buffers(0);
}

void asio_serial_comm::clear_send_buffer(void)
{
	clear_io_buffers(1);
}

void asio_serial_comm::clear_io_buffers(unsigned method)
{
	// asio provides no implementation to clear io-buffers

#ifdef BOOST_WINDOWS
	// WIN32
	#include <windows.h>
	int purge_flags[] = {PURGE_RXCLEAR | PURGE_RXABORT,
						 PURGE_TXCLEAR | PURGE_TXABORT, 
						 PURGE_RXCLEAR | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_TXABORT };
	::PurgeComm(p_serial->native(), purge_flags[method]);

#else
	// POSIX
	#include <termios.h>
	int purge_flags[] = { TCIFLUSH, TCOFLUSH, TCIOFLUSH };
	::tcflush(sp_serial->native(), purge_flags[method]);
#endif
}