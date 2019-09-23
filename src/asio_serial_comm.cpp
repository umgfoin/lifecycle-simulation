#include "stdafx.h"
#include "asio_serial_comm.h"

#ifdef _WIN32
	// WIN32
	#include <windows.h>
#else
	// POSIX
	#include <termios.h>
#endif

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
		system::error_code error;
		p_serial->cancel(error);
		p_serial->close(error);
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
	set_comm_timeouts();
}

void asio_serial_comm::set_comm_timeouts(void)
{
#ifdef _WIN32
	COMMTIMEOUTS to;
	HANDLE h = p_serial->native_handle();
	::GetCommTimeouts(h, &to);
	to.ReadIntervalTimeout = MAXDWORD;
	to.ReadTotalTimeoutMultiplier = 0;
	to.ReadTotalTimeoutConstant = 0;
	::SetCommTimeouts(h, &to);
#else
	termios tio;
	int fd = p_serial->native_handle();
	::tcgetattr(fd, &tio); /* get current port settings */
	tio.c_cc[VTIME] = 0;   /* inter-character timer unused */
	tio.c_cc[VMIN] = 0;    /* blocking read until 5 chars received */
	::tcflush(fd, TCIFLUSH);
	::tcsetattr(fd, TCSANOW, &tio);
#endif
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

	if ( p_serial )
		read = p_serial->read_some(asio::buffer (buf, size), error);
	else
		read = 0;

	return read;
}

bool asio_serial_comm::is_open(void) const
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

#ifdef _WIN32
	// WIN32
	int purge_flags[] = {PURGE_RXCLEAR | PURGE_RXABORT,
						 PURGE_TXCLEAR | PURGE_TXABORT, 
						 PURGE_RXCLEAR | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_TXABORT };
	::PurgeComm(p_serial->native_handle(), purge_flags[method]);

#else
	// POSIX
	int purge_flags[] = { TCIFLUSH, TCOFLUSH, TCIOFLUSH };
	::tcflush(p_serial->native_handle(), purge_flags[method]);
#endif
}
