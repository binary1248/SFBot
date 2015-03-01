#include <core_module.hpp>
#include <algorithm>
#include <iostream>

namespace {

////////////////////////////////////////////////////////////////////////////////
// Settings
////////////////////////////////////////////////////////////////////////////////
std::string version_string = "SFBot version: 0.0.0 https://github.com/binary1248/SFBot";
std::vector<std::string> operators = { "binary1248" };
////////////////////////////////////////////////////////////////////////////////
// Settings
////////////////////////////////////////////////////////////////////////////////

bool contains( const std::string& haystack, const std::string& needle ) {
	auto pos = haystack.find( needle );

	if( pos == haystack.npos ) {
		return false;
	}

	if( ( pos > 0 ) && haystack[pos - 1] != ' ' ) {
		return false;
	}

	if( ( pos + needle.length() < haystack.length() ) && haystack[pos + needle.length() + 1] != ' ' ) {
		return false;
	}

	return true;
}

}

core::core( bool& quit ) :
	m_quit( quit )
{

}

bool core::handle_channel_message( const std::string& user, const std::string& message ) {
	if( contains( message, "!version" ) ) {
		send_channel_message( version_string );
		return true;
	}
	else if( contains( message, "!shutdown" ) && ( std::find( std::begin( operators ), std::end( operators ), user ) != std::end( operators ) ) ) {
		std::cout << "Shutdown command received from " << user << "\n";

		send_channel_message( user + " requested shutdown, shutting down..." );
		m_quit = true;
		return true;
	}
	else if( contains( message, "!commands" ) ) {
		std::string commands;

		for( const auto& c : get_commands() ) {
			commands += c + ", ";
		}

		if( !commands.empty() ) {
			commands.erase( commands.length() - 2 );
		}

		send_channel_message( commands );
		return true;
	}

	return false;
}

bool core::handle_private_message( const std::string& user, const std::string& message ) {
	if( contains( message, "!version" ) ) {
		send_private_message( user, version_string );
		return true;
	}
	else if( contains( message, "!shutdown" ) && ( std::find( std::begin( operators ), std::end( operators ), user ) != std::end( operators ) ) ) {
		std::cout << "Shutdown command received from " << user << "\n";

		send_private_message( user, user + " requested shutdown, shutting down..." );
		m_quit = true;
		return true;
	}
	else if( contains( message, "!commands" ) ) {
		std::string commands;

		for( const auto& c : get_commands() ) {
			commands += c + ", ";
		}

		if( !commands.empty() ) {
			commands.erase( commands.length() - 2 );
		}

		send_private_message( user, commands );
		return true;
	}

	return false;
}
