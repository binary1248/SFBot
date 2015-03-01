#include <social.hpp>
#include <iostream>
#include <sstream>

namespace {

////////////////////////////////////////////////////////////////////////////////
// Settings
////////////////////////////////////////////////////////////////////////////////
register_module<social> register_social( { "!hi" } );
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

bool social::handle_channel_message( const std::string& user, const std::string& message ) {
	if( contains( message, "!hi" ) ) {
		send_channel_message( "Hi " + user + "!" );
		return true;
	}

	return false;
}

bool social::handle_private_message( const std::string& user, const std::string& message ) {
	if( contains( message, "!hi" ) ) {
		send_private_message( user, "Hi " + user + "! Why so secretive? :D" );
		return true;
	}

	return false;
}
