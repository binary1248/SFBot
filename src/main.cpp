#include <module.hpp>
#include <core_module.hpp>
#include <SFNUL.hpp>
#include <iostream>
#include <sstream>
#include <thread>

namespace {

////////////////////////////////////////////////////////////////////////////////
// Settings
////////////////////////////////////////////////////////////////////////////////
std::string server_address = "irc.boxbox.org";
std::string channel_name = "sfgui";
std::string bot_name = "SFBot";
const auto tick_interval = std::chrono::milliseconds( 100 );
////////////////////////////////////////////////////////////////////////////////
// Settings
////////////////////////////////////////////////////////////////////////////////

}

int main()
{
	auto irc_socket = sfn::TcpSocket::Create();

	sfn::Start();

	auto addresses = sfn::IpAddress::Resolve( server_address );

	if( addresses.empty() ) {
		std::cout << "Could not resolve hostname \"" << server_address << "\" to an address.\n";
		return 1;
	}

	irc_socket->Connect( sfn::Endpoint{ addresses.front(), 6667 } );

	auto send_message = [&irc_socket]( const std::string& message ) {
		auto line = message + "\r\n";
		irc_socket->Send( line.c_str(), line.length() );
	};

	module::register_send_channel_message_handler( [&]( const std::string& message ) {
		send_message( "PRIVMSG #" + channel_name + " :" + message );
		std::cout << "Bot: " << "PRIVMSG #" + channel_name + " :" + message << "\n";
	} );

	module::register_send_private_message_handler( [&]( const std::string& user, const std::string& message ) {
		send_message( "PRIVMSG " + user + " :" + message );
		std::cout << "Bot: " << "PRIVMSG " + user + " :" + message << "\n";
	} );

	send_message( "NICK " + bot_name );
	send_message( "USER " + bot_name + " 0 * :" + bot_name );

	{
		bool quit = false;
		register_module<core>( { "!version", "!shutdown", "!commands" }, quit );

		auto modules = module::instantiate_all();
		auto previous_tick = std::chrono::system_clock::now();

		while( !quit && !irc_socket->RemoteHasShutdown() ) {
			auto start_time = std::chrono::system_clock::now();
			module::tick( std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::system_clock::now() - previous_tick ) );
			previous_tick = std::chrono::system_clock::now();

			static std::string recv_string;
			std::array<char, 1024> recv_data;
			auto reply_size = irc_socket->Receive( recv_data.data(), recv_data.size() );
			recv_string.append( recv_data.data(), reply_size );

			auto pos = recv_string.find( "\r\n" );

			while( pos != recv_string.npos ) {
				std::string line = recv_string.substr( 0, pos );
				recv_string = recv_string.substr( pos + 2 );

				if( line.size() ) {
					if( !line.empty() ) {
						std::stringstream sstr( line );

						std::string token;
						sstr >> token;

						std::string prefix;
						std::string command;
						std::string name;

						if( token[0] == ':' ) {
							prefix = token.substr( 1 );

							if( prefix.length() > 1 ) {
								auto name_end_pos = prefix.find( '!' );
								if( name_end_pos != prefix.npos ) {
									name = prefix.substr( 0, name_end_pos );
								}
								else {
									name = prefix;
								}
							}

							sstr >> command;
						}
						else {
							command = token;
						}

						if( command == "MODE" ) {
							std::string target;
							sstr >> target;

							if( target == bot_name ) {
								std::string permission;
								sstr >> permission;

								if( permission == ":+x" ) {
									send_message( "JOIN :#" + channel_name );
								}
							}
						}
						else if( command == "PING" ) {
							std::string message;
							sstr >> message;
							send_message( "PONG " + message );
						}
						else if( command == "PRIVMSG" ) {
							std::string target;
							std::string message;
							sstr >> target >> message;

							if( !target.empty() && !message.empty() && ( message[0] == ':' ) ) {
								message.erase( 0, 1 );

								if( target == ( "#" + channel_name ) ) {
									// Channel message
									module::receive_channel_message( name, message );
								}
								else if( target == bot_name ) {
									// Private message
									module::receive_private_message( name, message );
								}
							}
						}
						else if( ( command == "001" ) || ( command == "002" ) || ( command == "003" ) || ( command == "004" ) ) {
							// Server greeting
						}
						else if( command == "005" ) {
							// Supported commands
						}
						else if( ( command == "251" ) || ( command == "252" ) || ( command == "253" ) || ( command == "254" ) || ( command == "255" ) ) {
							// Network information
						}
						else if( ( command == "265" ) || ( command == "266" ) ) {
							// More network information
						}
						else if( ( command == "332" ) || ( command == "333" ) ) {
							// Channel topic information
						}
						else if( ( command == "353" ) || ( command == "366" ) ) {
							// Channel user list
						}
						else if( ( command == "372" ) || ( command == "375" ) || ( command == "376" ) ) {
							// MOTD
						}
						else {
							std::cout << "Server: " << line << "\n";
						}
					}
				}

				pos = recv_string.find( "\r\n" );
			}

			std::cout.flush();
			std::this_thread::sleep_until( start_time + tick_interval );
		}
	}

	irc_socket->Shutdown();
	irc_socket->Close();

	sfn::Stop();
}
