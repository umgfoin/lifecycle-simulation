#pragma once
#include <iostream> 
#include <fstream> 
#include <sstream> 
#include <string>

using namespace std;

class base_file
{
public:
	base_file(const wchar_t* psz_filename) : psz_name(psz_filename){};
	base_file(void) = delete;

	virtual bool read(void);
	virtual void parse_line(istringstream& stream) abstract;

protected:
	bool	parse_key_value_pair(istringstream& stream, string& value);
	bool	parse_value(istringstream& stream, unsigned& val);
	bool	parse_value(istringstream& stream, float& val);
	bool	parse_value(istringstream& stream, string& val);
	string	get_timestamp(time_t* = nullptr);

public:
	const wchar_t*		 psz_name;
	static 	const char*  psz_equal;
};

class state_file : public base_file
{
public:
	state_file(const wchar_t* psz_filename) : base_file(psz_filename) {};

	virtual bool write(void);
	virtual void parse_line(istringstream& stream);

	unsigned cycles  = 0;		// total number of on/off cycles
	unsigned sigma_t = 0;		// accumulated duty-cycle-time [s]

private:
	static const char* option_names[];
};

class config_file : public base_file
{
public:
	config_file(const wchar_t* psz_filename) : base_file(psz_filename) {};

	virtual bool write(void);
	virtual void parse_line(istringstream& stream);

	string		str_serialport;						// serial-port power-supply data-connection
	float		u_range[2]		= { 09.0f, 13.f };	// U-range for jitter simulation [V]
	unsigned	t_on_range[2]	= { 30, 600 };		// range of random duty-cycle-time[s]
	unsigned	t_off_range[2]	= { 20, 250 };		// range of random dark-cycle-time [s]
	float		p_range[2]		= { 0.5f, 10.f };	// Allowed power-range [W]

private:
	static const char* option_names[];
};

class log_file : public base_file
{
public:
	log_file(const wchar_t* psz_filename) : base_file(psz_filename) {};

	virtual bool write(string& event);
	virtual void parse_line(istringstream& stream);

	void operator << (stringstream& str);
};