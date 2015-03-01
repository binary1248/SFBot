#include <forum_help.hpp>
#include <iostream>
#include <sstream>

namespace {

////////////////////////////////////////////////////////////////////////////////
// Settings
////////////////////////////////////////////////////////////////////////////////
register_module<forum_help> register_forum_help( { "!helpforum" } );
const static auto poll_interval = std::chrono::seconds( 30 );
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

bool forum_help::handle_channel_message( const std::string& /*user*/, const std::string& message ) {
	if( contains( message, "!helpforum" ) ) {
		std::string reply = "Latest help request on SFML forum at: ";
		reply += "http://en.sfml-dev.org/forums/index.php?topic=" + std::to_string( m_last_thread_id ) + ".0";

		send_channel_message( reply );

		return true;
	}


	return false;
}

void forum_help::handle_tick( const std::chrono::milliseconds& elapsed ) {
	m_time_to_query -= elapsed;

	if( m_current_query.valid() && ( m_current_query.wait_for( std::chrono::milliseconds( 0 ) ) == std::future_status::ready ) ) {
		auto body = m_current_query.get();
		std::string head( "rdf:resource=\"http://en.sfml-dev.org/forums/index.php?topic=" );
		auto id_start = body.find( head );

		if( id_start != body.npos ) {
			body = body.substr( id_start + head.length() );

			auto id_end = body.find( "\"" );

			if( id_end != body.npos ) {
				body = body.substr( 0, id_end );
				std::stringstream sstr( body );

				auto id = 0.0;
				sstr >> id;
				auto int_id = static_cast<int>( id );

				if( int_id > m_last_thread_id ) {
					if( m_last_thread_id ) {
						std::string message = "New help request detected on SFML forum at: ";

						message += "http://en.sfml-dev.org/forums/index.php?topic=";
						message += body;
						message += " Don't know how to compute a reply yet :(";

						send_channel_message( message );
					}

					m_last_thread_id = int_id;
				}
			}
		}
	}

	if( m_time_to_query < std::chrono::milliseconds( 0 ) ) {
		if( m_current_query.valid() ) {
			std::cerr << "forum_help request still not complete" << std::endl;
			return;
		}

		m_current_query = http_get( "en.sfml-dev.org", 80, "/forums/index.php?type=rdf;action=.xml;sa=news;limit=1;c=3" );

		m_time_to_query = poll_interval;
	}
}