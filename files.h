#pragma once
#include <iostream> 
#include <fstream> 
#include <sstream> 
#include <string>

using namespace std;

class base_file
{
public:
	base_file(LPCWSTR psz_filename) : psz_name(psz_filename){};
	base_file(void) = delete;

	virtual bool read(void) abstract;
	virtual bool write(void) abstract;
	bool		 parse_value(istringstream& stream, unsigned& val);
	bool		 parse_value(istringstream& stream, float& val);
	bool		 parse_value(istringstream& stream, string& val);

	LPCWSTR		psz_name;
};

class state_file : public base_file
{
public:
	state_file(LPCWSTR psz_filename) : base_file(psz_filename) {};

	virtual bool read(void);
	virtual bool write(void);

	unsigned cycles  = 0;
	unsigned sigma_t = 0;

private:
	static LPCSTR option_names[];
};

class config_file : public base_file
{
public:
	config_file(LPCWSTR psz_filename) : base_file(psz_filename) {};

	virtual bool read(void);
	virtual bool write(void);

	string str_serialport;
	float		u_range[2]		= { 09.0f, 13.f };
	unsigned	t_off_range[2]	= { 20, 250 };
	unsigned	t_on_range[2]	= { 30, 600 };

private:
	static LPCSTR option_names[];
};