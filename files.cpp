#include "stdafx.h"
#include "files.h"
#include <ctime>
#include <iomanip>

const char* base_file::psz_equal = " = ";

bool base_file::read(void)
{
	ifstream file(psz_name);
	if ( !file )
		return false;

	string line;
	size_t hit;

	while ( getline(file, line) ){
		// remove leading blanks
		hit = line.find_first_not_of(' ');
		if( hit && hit != string::npos )
			line.erase(0, hit);
		// ignore comments
		if ( line.substr(0, 2) == "//" || line[0] == '#' )
			continue;

		// normalize assignment-syntax to "option = value"
		hit = line.find_first_of('=');
		if ( hit && hit != string::npos){
			if ( line[hit + 1] != ' ' )
				line.insert(hit + 1, 1, ' ');

			if ( line[hit - 1] != ' ' )
				line.insert(hit, 1, ' ');
		}

		parse_line(istringstream(line));
	}
	return true;
}

bool base_file::parse_value(istringstream& stream, unsigned& val)
{
	string value;
	bool res;
	
	if ( res = parse_key_value_pair(stream, value) )
		val = stoi (value);
	
	return res;
}

bool base_file::parse_value(istringstream& stream, string& val)
{
	return parse_key_value_pair(stream, val);
}

bool base_file::parse_value(istringstream& stream, float& val)
{
	string value;
	bool res;

	if ( res = parse_key_value_pair(stream, value) )
		val = stof (value);

	return res;
}

bool base_file::parse_key_value_pair(istringstream& stream, string& value)
{
	return (stream >> value && value == "=" && stream >> value);
}

string base_file::get_timestamp(time_t* timedata)
{
	time_t current_t = time(nullptr);
	tm	tm;

	localtime_s(&tm, timedata ? timedata : &current_t);
	
	stringstream stream;
	stream << std::put_time(&tm, "%F-%T");

	return move(stream.str());
}

const char* config_file::option_names[] = {
	"device",
	"u_min",
	"u_max",
	"t_on_min",
	"t_on_max",
	"t_off_min",
	"t_off_max",
	"p_min",
	"p_max"
};

void config_file::parse_line(istringstream& stream)
{
	string token;
	while ( stream >> token ){
		// make lowercase
		for ( auto character : token ) tolower(character);
		unsigned o = 0;

		if ( token == option_names[o++] ){
			parse_value(stream, str_serialport);
			// skip remainder
			break;
		}

		for ( int i = 0; i < 2; i++ )
			if ( token == option_names[o++] ){
				parse_value(stream, u_range[i]);
				// skip remainder
				goto end_func;
			}

		for ( int i = 0; i < 2; i++ )
			if ( token == option_names[o++] ){
				parse_value(stream, t_on_range[i]);
				// skip remainder
				goto end_func;
			}

		for ( int i = 0; i < 2; i++ )
			if ( token == option_names[o++] ){
				parse_value(stream, t_off_range[i]);
				// skip remainder
				goto end_func;
			}

		for ( int i = 0; i < 2; i++ )
			if ( token == option_names[o++] ){
				parse_value(stream, p_range[i]);
				goto end_func;
			}
	}
	end_func:;
}

bool config_file::write(void)
{
	ofstream file(psz_name, ios_base::out | ios_base::trunc);
	if ( !file )
		return false;

	file << option_names[0] << psz_equal << str_serialport	<< endl;
	file << option_names[1] << psz_equal << u_range[0]		<< endl;
	file << option_names[2] << psz_equal << u_range[1]		<< endl;
	file << option_names[3] << psz_equal << t_on_range[0]	<< endl;
	file << option_names[4] << psz_equal << t_on_range[1]	<< endl;
	file << option_names[5] << psz_equal << t_off_range[0]	<< endl;
	file << option_names[6] << psz_equal << t_off_range[1]	<< endl;
	file << option_names[7] << psz_equal << p_range[0]		<< endl;
	file << option_names[8] << psz_equal << p_range[1]		<< endl;

	return true;
}

const char* state_file::option_names[] = {
	"runtime",
	"cycles",
	"last_power_violation",
	"last_update"
};

void state_file::parse_line(istringstream& stream)
{
	string token;

	while ( stream >> token ){
			// make lowercase
			for ( auto character : token ) tolower(character);

			if ( token == option_names[0] ){
				parse_value(stream, sigma_t);
				// skip remainder
				break;
			}

			if ( token == option_names[1] ){
				parse_value(stream, cycles);
				// skip remainder
				break;
			}
	}
}

bool state_file::write(void)
{
	ofstream file(psz_name, ios_base::out | ios_base::trunc);
	if ( !file )
		return false;

	file << option_names[0] << psz_equal << sigma_t << endl;
	file << option_names[1] << psz_equal << cycles  << endl;
	file << option_names[2] << psz_equal << get_timestamp()  << endl;

	return true;
}

void log_file::parse_line(istringstream& stream)
{
	// this is not an input-file
}

bool log_file::write(string& event)
{
	ofstream file(psz_name, ios_base::out | ios_base::app);
	if ( !file )
		return false;

	time_t current_t = time(nullptr);
	tm	tm;
	localtime_s(&tm, &current_t);

	file << std::put_time(&tm, "%F-%T ") << event << endl;
	
	return true;
}

void log_file::operator << (stringstream& stream)
{
	ofstream file(psz_name, ios_base::out | ios_base::app);
	if ( file )
		file << get_timestamp() << " " << stream.str() << endl;

	stream.str("");
}
