#include <repository.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

namespace {

////////////////////////////////////////////////////////////////////////////////
// Settings
////////////////////////////////////////////////////////////////////////////////
register_function<repository> register_forum_help_sfml( { "!repo", "!repos" }, "LaurentGomila/SFML" );
register_function<repository> register_forum_help_sfgui( "TankOs/SFGUI" );
register_function<repository> register_forum_help_sfnul( "binary1248/SFNUL" );
const static auto poll_interval = std::chrono::seconds( 30 );
////////////////////////////////////////////////////////////////////////////////
// Settings
////////////////////////////////////////////////////////////////////////////////

std::string secret;

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

const std::string& get_secret() {
	if( secret.empty() ) {
		std::ifstream secret_file( "secret" );

		if( !secret_file.good() ) {
			std::cerr << "Could not open secret file" << std::endl;
			return secret;
		}

		secret_file >> secret;
		if( secret_file.fail() ) {
			std::cerr << "Could not read secret file" << std::endl;
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
	if( ( message == "!repos" ) || contains( message, "!repo " + m_uri ) ) {
		std::string reply;
		reply = m_uri + ": " + std::to_string( m_star_count ) + " stars";
		send_channel_message( reply );
		reply = m_uri + ": Newest stargazer: " + m_newest_stargazer;
		send_channel_message( reply );
	}

	return false;
}

void repository::handle_tick( const std::chrono::milliseconds& elapsed ) {
	m_time_to_query -= elapsed;

	if( m_current_query.valid() && ( m_current_query.wait_for( std::chrono::milliseconds( 0 ) ) == std::future_status::ready ) ) {
		auto body = m_current_query.get();

		auto stargazers_pos = body.find( "\"stargazers_count\":" );
		auto login_pos = body.rfind( "\"login\":" );
		if( stargazers_pos != body.npos ) {
			auto stargazers_start_pos = stargazers_pos + 19;
			auto stargazers_end_pos = body.find( ",", stargazers_start_pos );
			auto stargazers = body.substr( stargazers_start_pos, stargazers_end_pos - stargazers_start_pos );

			auto star_count = std::stoi( stargazers );

			if( !m_star_count ) {
				m_star_count = star_count;

				m_current_query = http_get( "api.github.com", 443, "/repos/" + m_uri + "/stargazers?" + get_secret() + "&per_page=1&page=" + std::to_string( m_star_count ), true );
				m_time_to_query = poll_interval;
			}
			else if( m_star_count != star_count ) {
				std::string message;
				message += m_uri + ": " + std::to_string( m_star_count ) + " -> " + std::to_string( star_count ) + " stars";
				send_channel_message( message );
				m_star_count = star_count;

				m_current_query = http_get( "api.github.com", 443, "/repos/" + m_uri + "/stargazers?" + get_secret() + "&per_page=1&page=" + std::to_string( m_star_count ), true );
				m_time_to_query = poll_interval;
			}
		}
		else if( login_pos != body.npos ) {
			auto login_start_pos = login_pos + 9;
			auto login_end_pos = body.find( "\"", login_start_pos );
			auto login = body.substr( login_start_pos, login_end_pos - login_start_pos );

			if( m_newest_stargazer.empty() ) {
				m_newest_stargazer = login;

				m_current_query = http_get( "api.github.com", 443, "/repos/" + m_uri + "?" + get_secret(), true );
				m_time_to_query = poll_interval;
			}
			else if( !login.empty() ) {
				std::string message;
				message += m_uri + ": Newest stargazer: " + login;
				send_channel_message( message );
				m_newest_stargazer = login;

				m_current_query = http_get( "api.github.com", 443, "/repos/" + m_uri + "?" + get_secret(), true );
				m_time_to_query = poll_interval;
			}
		}
	}

	if( m_time_to_query < std::chrono::milliseconds( 0 ) ) {
		if( m_current_query.valid() ) {
			std::cerr << "repository request still not complete" << std::endl;
			return;
		}

		m_current_query = http_get( "api.github.com", 443, "/repos/" + m_uri + "?" + get_secret(), true );
		m_time_to_query = poll_interval;
	}
}
