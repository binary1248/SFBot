#include <forum_help.hpp>
#include <iostream>
#include <sstream>

namespace {

////////////////////////////////////////////////////////////////////////////////
// Settings
////////////////////////////////////////////////////////////////////////////////
const static auto poll_interval = std::chrono::seconds( 30 );
////////////////////////////////////////////////////////////////////////////////
// Settings
////////////////////////////////////////////////////////////////////////////////

}

bool forum_help::handle_channel_message( const std::string& /*user*/, const std::string& message ) {
	if( contains( message, "!helpforum" ) ) {
		std::string reply;
		if(!m_last_thread_poster.empty()) {
			reply = "Latest help request on SFML forum by " + m_last_thread_poster + " at: ";
		}
		else {
			reply = "Latest help request on SFML forum at: ";
		}
		reply += "http://en.sfml-dev.org/forums/index.php?topic=" + std::to_string( m_last_thread_id ) + ".0";

		send_channel_message( reply );

		return true;
	}

	return false;
}

void forum_help::handle_tick( const std::chrono::milliseconds& elapsed ) {
	m_time_to_query -= elapsed;

	handle_forum();

	poll_forum();
}

void forum_help::poll_forum() {
	if( m_time_to_query < std::chrono::milliseconds( 0 ) ) {
		if( m_current_forum_query.valid() ) {
			std::cerr << "forum_help request still not complete" << "\n";
			return;
		}

		m_current_forum_query = http_get( "en.sfml-dev.org", 80, "/forums/index.php?type=xml;action=.xml;sa=news;limit=1;c=3" );

		m_time_to_query = poll_interval;
	}
}

void forum_help::handle_forum() {
	if( m_current_forum_query.valid() && ( m_current_forum_query.wait_for( std::chrono::milliseconds( 0 ) ) == std::future_status::ready ) ) {
		auto body_orig = m_current_forum_query.get();
		auto body = body_orig;
		std::string poster_head( "<poster>" );
		auto poster_start = body.find( poster_head );

		auto poster = std::string("");

		if( poster_start != body.npos ) {
			body = body.substr( poster_start + poster_head.length() );

			std::string name( "<name><![CDATA[" );
			auto name_start = body.find( name );

			if( name_start != body.npos ) {
				body = body.substr( name_start + name.length() );

				auto name_end = body.find( "]]></name>" );

				if( name_end != body.npos ) {
					body = body.substr( 0, name_end );

					poster = body;
				}
			}
		}

		body = body_orig;

		std::string head( "<topic>" );
		auto id_start = body.find( head );

		if( id_start != body.npos ) {
			body = body.substr( id_start + head.length() );

			auto id_end = body.find( "</topic>" );

			if( id_end != body.npos ) {
				body = body.substr( 0, id_end );

				auto id = std::stod( body );
				auto int_id = static_cast<int>( id );

				if( int_id > m_last_thread_id ) {
					if( m_last_thread_id ) {
						std::string message;
						if(!poster.empty()) {
							message += "New help request by " + poster + " detected on SFML forum at: ";
						}
						else {
							message += "New help request detected on SFML forum at: ";
						}

						message += "http://en.sfml-dev.org/forums/index.php?topic=";
						message += body;
						send_channel_message( message );
					}

					m_last_thread_poster = poster;

					m_last_thread_id = int_id;
				}
			}
		}
	}
}
