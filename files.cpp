#include "stdafx.h"
#include "files.h"


bool base_file::parse_value(istringstream& stream, unsigned& val)
{
	string token;

	if ( stream >> token && token == "=" && stream >> token ){
		val = stoi(token);
		return true;
	}
	return false;
}

bool base_file::parse_value(istringstream& stream, string& val)
{
	string token;

	if ( stream >> token && token == "=" && stream >> token ){
		val = token;
		return true;
	}
	return false;
}

bool base_file::parse_value(istringstream& stream, float& val)
{
	string token;

	if ( stream >> token && token == "=" && stream >> token ){
		val = stof(token);
		return true;
	}
	return false;
}

LPCSTR config_file::option_names[] = {
	"device",
	"u_min",
	"u_max",
	"t_on_min",
	"t_on_max",
	"t_off_min",
	"t_off_max"
};

bool config_file::read()
{
	ifstream file(psz_name);
	if ( !file )
		return false;

	string line;
	string token;

	while ( getline(file, line) ){

		istringstream stream(line);

		while ( stream >> token ){
			// make lowercase
			for ( auto character : token ) tolower(character);

			if ( token == option_names[0] ){
				parse_value(stream, str_serialport);
				// skip remainder
				break;
			}
			if ( token == option_names[1] ){
				parse_value(stream, u_range[0]);
				// skip remainder
				break;
			}

			if ( token == option_names[2] ){
				parse_value(stream, u_range[1]);
				// skip remainder
				break;
			}
			if ( token == option_names[3] ){
				parse_value(stream, t_on_range[0]);
				// skip remainder
				break;
			}

			if ( token == option_names[4] ){
				parse_value(stream, t_on_range[1]);
				// skip remainder
				break;
			}
			if ( token == option_names[5] ){
				parse_value(stream, t_off_range[0]);
				// skip remainder
				break;
			}

			if ( token == option_names[6] ){
				parse_value(stream, t_off_range[1]);
				// skip remainder
				break;
			}
		}
	}
	return true;
}

bool config_file::write(void)
{
	ofstream file(psz_name, ios_base::out | ios_base::trunc);
	if ( !file )
		return false;

	file << option_names[0] << " = " << str_serialport	<< endl;
	file << option_names[1] << " = " << u_range[0]		<< endl;
	file << option_names[2] << " = " << u_range[1]		<< endl;
	file << option_names[3] << " = " << t_on_range[0]	<< endl;
	file << option_names[4] << " = " << t_on_range[1]	<< endl;
	file << option_names[5] << " = " << t_off_range[0]	<< endl;
	file << option_names[6] << " = " << t_off_range[1]	<< endl;

	return true;
}

LPCSTR state_file::option_names[] = {
	"runtime",
	"cycles"
};

bool state_file::read()
{
	ifstream file(psz_name);
	if ( !file )
		return false;

	string line;
	string token;

	while ( getline(file, line) ){

		istringstream stream(line);

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
	return true;
}

bool state_file::write(void)
{
	ofstream file(psz_name, ios_base::out | ios_base::trunc);
	if ( !file )
		return false;

	file << option_names[0] << " = " << sigma_t << endl;
	file << option_names[1] << " = " << cycles  << endl;

	return true;
}
