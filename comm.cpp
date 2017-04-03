#include "stdafx.h"
#include "comm.h"
#include <Windows.h>

// This is a stripped-down (windows) serialcomm-wrapper derived from our full-featured async CComm-class
// we assume baud x-8-none-1, no handshake
// in the mid-run, this will be replaced by STL or boost:asio

bool simple_comm::open_comm(LPCSTR psz_port, int baudrate)
{
	char device[32];
	sprintf_s(device, "\\\\.\\%s", psz_port);
	hFile = ::CreateFileA(device,
					   GENERIC_READ | GENERIC_WRITE,
					   0,
					   NULL,
					   OPEN_EXISTING,
					   FILE_ATTRIBUTE_NORMAL,
					   NULL);
	if ( hFile != INVALID_HANDLE_VALUE ){
		if ( baudrate != -1 )
			setup_comm( (unsigned) baudrate);
		return true;
	}

	return false;
}

void simple_comm::close_comm(void)
{
	if ( hFile != INVALID_HANDLE_VALUE ){
		::FlushFileBuffers(hFile);
		::CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}
}

void simple_comm::setup_comm(unsigned baudrate)
{
	DCB dcb;
	::ZeroMemory(&dcb, sizeof(DCB));
	dcb.DCBlength			= sizeof(DCB);
	dcb.BaudRate			= baudrate;
	dcb.ByteSize			= 8;
	dcb.Parity				= 0;
	dcb.StopBits			= 1;
	dcb.XonChar				= 0x11;
	dcb.XoffChar			= 0x13;
	dcb.XonLim				= 100;
	dcb.XoffLim				= 100;
	dcb.fBinary				= TRUE;
	dcb.fAbortOnError		= FALSE;
	dcb.fBinary				= 1;

	dcb.fDtrControl			= DTR_CONTROL_ENABLE;
	dcb.fOutxDsrFlow		= FALSE;
	dcb.fRtsControl			= RTS_CONTROL_ENABLE;
	dcb.fOutX				= FALSE;
	dcb.fOutxCtsFlow		= FALSE;
	dcb.fTXContinueOnXoff	= TRUE;
	dcb.fInX				= FALSE;

	COMMTIMEOUTS timeouts;
	timeouts.ReadIntervalTimeout = MAXDWORD;
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.ReadTotalTimeoutConstant = 00;
	// avoids deadlocks in FlushFileBuffers
	timeouts.WriteTotalTimeoutMultiplier = 10000 / baudrate;	// tout/byte
	timeouts.WriteTotalTimeoutConstant = 1000;

	::SetCommState(hFile,		&dcb);
	::SetCommTimeouts(hFile,	&timeouts);
}
void simple_comm::clear_rcv_buffer(void)
{
	::PurgeComm(hFile, PURGE_RXCLEAR | PURGE_RXABORT);
}

size_t simple_comm::rcv_sync(unsigned char* buf, size_t size)
{
	DWORD read = 0;
	DWORD received;

	for (received = 0; ::ReadFile(hFile, &buf[received], (DWORD) (size - received), &read, nullptr) && read; received += read );
	return received;
}

void simple_comm::send_sync(LPCSTR psz_cmd)
{
	DWORD dwWritten;
	::WriteFile(hFile, psz_cmd, (DWORD) strlen(psz_cmd), &dwWritten, nullptr);
}