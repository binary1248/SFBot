#include <repository.hpp>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>

namespace {

////////////////////////////////////////////////////////////////////////////////
// Settings
////////////////////////////////////////////////////////////////////////////////
const static auto poll_interval = std::chrono::seconds( 30 );
////////////////////////////////////////////////////////////////////////////////
// Settings
////////////////////////////////////////////////////////////////////////////////

std::string secret;

const std::string& get_secret() {
	if( secret.empty() ) {
		std::ifstream secret_file( "secret" );

		if( !secret_file.good() ) {
			std::cerr << "Could not open secret file" << "\n";
			return secret;
		}

		secret_file >> secret;
		if( secret_file.fail() ) {
			std::cerr << "Could not read secret file" << "\n";
			secret = "";
			return secret;
		}
	}

	return secret;
}

}

repository::repository( const std::string& uri ) :
	m_uri( uri )
{
	http_add_certificate( "api.github.com",
		"-----BEGIN CERTIFICATE-----\r\n"
		"MIIDxTCCAq2gAwIBAgIQAqxcJmoLQJuPC3nyrkYldzANBgkqhkiG9w0BAQUFADBs\r\n"
		"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\r\n"
		"d3cuZGlnaWNlcnQuY29tMSswKQYDVQQDEyJEaWdpQ2VydCBIaWdoIEFzc3VyYW5j\r\n"
		"ZSBFViBSb290IENBMB4XDTA2MTExMDAwMDAwMFoXDTMxMTExMDAwMDAwMFowbDEL\r\n"
		"MAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZMBcGA1UECxMQd3d3\r\n"
		"LmRpZ2ljZXJ0LmNvbTErMCkGA1UEAxMiRGlnaUNlcnQgSGlnaCBBc3N1cmFuY2Ug\r\n"
		"RVYgUm9vdCBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMbM5XPm\r\n"
		"+9S75S0tMqbf5YE/yc0lSbZxKsPVlDRnogocsF9ppkCxxLeyj9CYpKlBWTrT3JTW\r\n"
		"PNt0OKRKzE0lgvdKpVMSOO7zSW1xkX5jtqumX8OkhPhPYlG++MXs2ziS4wblCJEM\r\n"
		"xChBVfvLWokVfnHoNb9Ncgk9vjo4UFt3MRuNs8ckRZqnrG0AFFoEt7oT61EKmEFB\r\n"
		"Ik5lYYeBQVCmeVyJ3hlKV9Uu5l0cUyx+mM0aBhakaHPQNAQTXKFx01p8VdteZOE3\r\n"
		"hzBWBOURtCmAEvF5OYiiAhF8J2a3iLd48soKqDirCmTCv2ZdlYTBoSUeh10aUAsg\r\n"
		"EsxBu24LUTi4S8sCAwEAAaNjMGEwDgYDVR0PAQH/BAQDAgGGMA8GA1UdEwEB/wQF\r\n"
		"MAMBAf8wHQYDVR0OBBYEFLE+w2kD+L9HAdSYJhoIAu9jZCvDMB8GA1UdIwQYMBaA\r\n"
		"FLE+w2kD+L9HAdSYJhoIAu9jZCvDMA0GCSqGSIb3DQEBBQUAA4IBAQAcGgaX3Nec\r\n"
		"nzyIZgYIVyHbIUf4KmeqvxgydkAQV8GK83rZEWWONfqe/EW1ntlMMUu4kehDLI6z\r\n"
		"eM7b41N5cdblIZQB2lWHmiRk9opmzN6cN82oNLFpmyPInngiK3BD41VHMWEZ71jF\r\n"
		"hS9OMPagMRYjyOfiZRYzy78aG6A9+MpeizGLYAiJLQwGXFK3xPkKmNEVX58Svnw2\r\n"
		"Yzi9RKR/5CYrCsSXaQ3pjOLAEFe4yHYSkVXySGnYvCoCWw9E1CAx2/S6cCZdkGCe\r\n"
		"vEsXCS+0yx5DaMkHJ8HSXPfqIbloEpw8nL+e/IBcm2PN7EeqJSdnoDfzAIJ9VNep\r\n"
		"+OkuE6N36B9K\r\n"
		"-----END CERTIFICATE-----\r\n",
	"*github.com" );
}

bool repository::handle_channel_message( const std::string& /*user*/, const std::string& message ) {
	if( contains( message, "!repos" ) || contains( message, "!repo " + m_uri ) ) {
		auto delta = m_lead_star_count - m_star_count;
		delta = std::max( 0, delta );
		std::string reply;
		reply = m_uri + ": " + std::to_string( m_star_count ) + " stars, next: " + m_lead_repository + ": " + std::to_string( m_lead_star_count ) + " stars, (" + ( delta > 0 ? "+" : "" ) + std::to_string( delta ) + ")";
		send_channel_message( reply );
		reply = m_uri + ": Newest stargazer: " + m_newest_stargazer;
		send_channel_message( reply );
	}

	return false;
}

void repository::handle_tick( const std::chrono::milliseconds& elapsed ) {
	m_time_to_query -= elapsed;

	handle_repository();
	handle_stargazer();
	handle_lead();

	poll_repository();
}

void repository::poll_repository() {
	if( m_time_to_query < std::chrono::milliseconds( 0 ) ) {
		if( m_current_repository_query.valid() ) {
			std::cerr << "repository request still not complete" << "\n";
		}
		else {
			m_current_repository_query = http_get( "api.github.com", 443, "/repos/" + m_uri + "?" + get_secret(), true );
			m_time_to_query = poll_interval;
		}
	}
}

void repository::handle_repository() {
	if( m_current_repository_query.valid() && ( m_current_repository_query.wait_for( std::chrono::milliseconds( 0 ) ) == std::future_status::ready ) ) {
		auto body = m_current_repository_query.get();

		auto stargazers_pos = body.find( "\"stargazers_count\":" );

		if( stargazers_pos == body.npos ) {
			return;
		}

		{
			auto stargazers_start_pos = stargazers_pos + 19;
			auto stargazers_end_pos = body.find( ",", stargazers_start_pos );
			auto stargazers = body.substr( stargazers_start_pos, stargazers_end_pos - stargazers_start_pos );

			auto star_count = std::stoi( stargazers );

			if( m_star_count && ( m_star_count != star_count ) ) {
				std::string message;
				message += m_uri + ": " + std::to_string( m_star_count ) + " -> " + std::to_string( star_count ) + " stars";
				send_channel_message( message );
			}

			m_star_count = star_count;

			if( m_current_stargazer_query.valid() ) {
				std::cerr << "stargazer request still not complete" << "\n";
			}
			else {
				m_current_stargazer_query = http_get( "api.github.com", 443, "/repos/" + m_uri + "/stargazers?" + get_secret() + "&per_page=1&page=" + std::to_string( m_star_count ), true );
			}
		}

		auto language_pos = body.find( "\"language\":" );

		if( m_star_count && ( language_pos != body.npos ) ) {
			auto language_start_pos = language_pos + 11;
			auto language_end_pos = body.find( ",", language_start_pos );
			auto language = body.substr( language_start_pos, language_end_pos - language_start_pos );

			language.erase( std::remove( std::begin( language ), std::end( language ), '"' ), std::end( language ) );
			auto encoded_language = url_encode( language );

			if( m_current_lead_query.valid() ) {
				std::cerr << "lead request still not complete" << "\n";
			}
			else {
				m_current_lead_query = http_get( "api.github.com", 443, "/search/repositories?q=language:" + encoded_language + "+stars:>" + std::to_string( m_star_count ) + "&sort=stars&order=asc&page=1&per_page=1", true );
			}
		}
	}
}

void repository::handle_stargazer() {
	if( m_current_stargazer_query.valid() && ( m_current_stargazer_query.wait_for( std::chrono::milliseconds( 0 ) ) == std::future_status::ready ) ) {
		auto body = m_current_stargazer_query.get();

		auto login_pos = body.rfind( "\"login\":" );

		if( login_pos != body.npos ) {
			auto login_start_pos = login_pos + 9;
			auto login_end_pos = body.find( "\"", login_start_pos );
			auto login = body.substr( login_start_pos, login_end_pos - login_start_pos );

			if( !m_newest_stargazer.empty() && ( m_newest_stargazer != login ) ) {
				std::string message;
				message += m_uri + ": Newest stargazer: " + login;
				send_channel_message( message );
			}

			m_newest_stargazer = login;
		}
	}
}

void repository::handle_lead() {
	if( m_current_lead_query.valid() && ( m_current_lead_query.wait_for( std::chrono::milliseconds( 0 ) ) == std::future_status::ready ) ) {
		auto body = m_current_lead_query.get();

		auto fullname_pos = body.find( "\"full_name\":" );

		if( fullname_pos == body.npos ) {
			return;
		}

		auto fullname_start_pos = fullname_pos + 12;
		auto fullname_end_pos = body.find( ",", fullname_start_pos );
		auto fullname = body.substr( fullname_start_pos, fullname_end_pos - fullname_start_pos );

		fullname.erase( std::remove( std::begin( fullname ), std::end( fullname ), '"' ), std::end( fullname ) );

		m_lead_repository = fullname;

		auto stargazers_pos = body.find( "\"stargazers_count\":" );

		if( stargazers_pos != body.npos ) {
			auto stargazers_start_pos = stargazers_pos + 19;
			auto stargazers_end_pos = body.find( ",", stargazers_start_pos );
			auto stargazers = body.substr( stargazers_start_pos, stargazers_end_pos - stargazers_start_pos );

			auto star_count = std::stoi( stargazers );

			if( m_lead_star_count && ( m_lead_star_count != star_count ) ) {
				auto delta = star_count - m_star_count;
				std::string message;
				message += m_uri + ": " + std::to_string( m_star_count ) + " stars, next: " + m_lead_repository + ": " + std::to_string( star_count ) + " stars, (" + ( delta > 0 ? "+" : "" ) + std::to_string( delta ) + ")";
				send_channel_message( message );
			}

			m_lead_star_count = star_count;
		}
	}
}
