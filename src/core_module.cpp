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

core::core( bool& quit, std::chrono::milliseconds tick_time, std::chrono::milliseconds send, unsigned int queue, unsigned int cps ) :
	m_quit( quit ),
	m_tick( tick_time ),
	m_send( send ),
	m_queue( queue ),
	m_cps( cps )
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
	else if( contains( message, "!config" ) ) {
		std::string config;

		config += "Tick interval: " + std::to_string( m_tick.count() ) + "ms, ";
		config += "Send interval: " + std::to_string( m_send.count() ) + "ms, ";
		config += "Queue size: " + std::to_string( m_queue ) + ", ";
		config += "Characters per second: " + std::to_string( m_cps );

		send_channel_message( config );
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
	else if( contains( message, "!config" ) ) {
		std::string config;

		config += "Tick interval: " + std::to_string( m_tick.count() ) + "ms, ";
		config += "Send interval: " + std::to_string( m_send.count() ) + "ms, ";
		config += "Queue size: " + std::to_string( m_queue ) + ", ";
		config += "Characters per second: " + std::to_string( m_cps );

		send_private_message( user, config );
		return true;
	}

	return false;
}
