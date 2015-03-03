#include <social.hpp>
#include <iostream>
#include <sstream>

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
