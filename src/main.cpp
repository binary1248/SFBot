#include <module.hpp>
#include <SFNUL.hpp>
#include <algorithm>
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

	auto send_line = [&irc_socket]( const std::string& line ) {
		irc_socket->Send( line.c_str(), line.length() );
		std::cout << "Sent line to server: " << line;
	};

	std::string connect_command = "NICK " + bot_name + "\r\nUSER " + bot_name + " 0 * :" + bot_name + "\r\n";
	send_line( connect_command );

	std::string recv_string;

	while( !irc_socket->RemoteHasShutdown() ) {
		std::array<char, 1024> recv_data;
		recv_data.fill( 0 );

		auto reply_size = irc_socket->Receive( recv_data.data(), recv_data.size() );

		recv_string.append( recv_data.data(), reply_size );

		auto pos = recv_string.find( "\r\n" );

		if( pos != recv_string.npos ) {
			std::string line = recv_string.substr( 0, pos + 2 );
			recv_string = recv_string.substr( pos + 2 );

			if( line.size() ) {
				std::cout << "Got line from server: " << line;
				if( line.size() > 4 ) {
					if( line.substr( 0, 4 ) == "PING" ) {
						line[1] = 'O';
						send_line( line );
					}
					else if( line.find( "MODE " + bot_name + " :+x" ) != line.npos ) {
						std::string join_command = "JOIN :#" + channel_name + "\r\n";
						send_line( join_command );
						break;
					}
				}
			}
		}
	}

	module::register_send_channel_message_handler( [&]( const std::string& message ) {
		auto full_line = "PRIVMSG #" + channel_name + " :" + message + "\r\n";
		irc_socket->Send( full_line.c_str(), full_line.length() );
		std::cout << "Sent message to server: " << full_line;
	} );

	module::register_send_private_message_handler( [&]( const std::string& user, const std::string& message ) {
		auto full_line = "PRIVMSG " + user + " :" + message + "\r\n";
		irc_socket->Send( full_line.c_str(), full_line.length() );
		std::cout << "Sent message to server: " << full_line;
	} );

	{
		auto functions = module::instantiate_all();
		auto previous_tick = std::chrono::system_clock::now();

		module::add_commands( { "!version", "!shutdown", "!commands" } );

		while( !irc_socket->RemoteHasShutdown() ) {
			module::tick( std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::system_clock::now() - previous_tick ) );
			previous_tick = std::chrono::system_clock::now();

			std::array<char, 1024> recv_data;
			recv_data.fill( 0 );

			auto reply_size = irc_socket->Receive( recv_data.data(), recv_data.size() );

			recv_string.append( recv_data.data(), reply_size );

			auto pos = recv_string.find( "\r\n" );

			if( pos != recv_string.npos ) {
				std::string line = recv_string.substr( 0, pos + 2 );
				recv_string = recv_string.substr( pos + 2 );

				if( line.size() ) {
					std::cout << "Got line from server: " << line;
					if( line.size() > 4 ) {
						if( line.substr( 0, 4 ) == "PING" ) {
							line[1] = 'O';
							send_line( line );
						}
						else if( !line.empty() ) {
							std::stringstream sstr( line );

							// Check if the message contains a prefix and extract it
							std::string prefix;
							if( line[0] == ':' ) {
								sstr >> prefix;
							}

							// Check if the prefix contains a valid username and extract it
							std::string username;
							if( prefix.length() > 2 ) {
								auto username_end_pos = prefix.find( '!' );
								if( username_end_pos != prefix.npos ) {
									username = prefix.substr( 1, username_end_pos - 1 );
								}
							}

							// Extract the command
							std::string command;
							sstr >> command;

							line.erase( 0, prefix.length() + 1 );
							line.erase( 0, command.length() );

							if( command == "PRIVMSG" ) {
								std::string target;
								sstr >> target;
								line.erase( 0, target.length() + 2 );

								if( !line.empty() && ( line[0] == ':' ) ) {
									line.erase( 0, 1 );

									line.erase( std::remove( std::begin( line ), std::end( line ), '\r' ), std::end( line ) );
									line.erase( std::remove( std::begin( line ), std::end( line ), '\n' ), std::end( line ) );

									if( target == ( "#" + channel_name ) ) {
										// Channel message
										if( contains( line, "!version" ) ) {
											module::send_channel_message( version_string );
										}
										else if( contains( line, "!shutdown" ) && ( std::find( std::begin( operators ), std::end( operators ), username ) != std::end( operators ) ) ) {
											module::send_channel_message( username + " requested shutdown, shutting down..." );
											break;
										}
										else if( contains( line, "!commands" ) ) {
											std::string commands;

											for( const auto& c : module::get_commands() ) {
												commands += c + ", ";
											}

											if( !commands.empty() ) {
												commands.erase( commands.length() - 2 );
											}

											module::send_channel_message( commands );
										}
										else {
											module::receive_channel_message( username, line );
										}
									}
									else if( target == bot_name ) {
										// Private message
										if( contains( line, "!version" ) ) {
											module::send_private_message( username, version_string );
										}
										else if( contains( line, "!shutdown" ) && ( std::find( std::begin( operators ), std::end( operators ), username ) != std::end( operators ) ) ) {
											module::send_private_message( username, username + " requested shutdown, shutting down..." );
											break;
										}
										else if( contains( line, "!commands" ) ) {
											std::string commands;

											for( const auto& c : module::get_commands() ) {
												commands += c + ", ";
											}

											if( !commands.empty() ) {
												commands.erase( commands.length() - 2 );
											}

											module::send_private_message( username, commands );
										}
										else {
											module::receive_private_message( username, line );
										}
									}
								}
							}
						}
					}
				}
			}

			std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
		}
	}

	irc_socket->Shutdown();
	irc_socket->Close();

	sfn::Stop();
}
