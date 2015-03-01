#pragma once

#include <module.hpp>
#include <chrono>

class repository : public module {
public:
	repository( const std::string& uri );

private:
	bool handle_channel_message( const std::string& user, const std::string& message ) override;
	void handle_tick( const std::chrono::milliseconds& elapsed ) override;

	std::string m_uri;
	int m_star_count = 0;
	std::string m_newest_stargazer;
	std::chrono::milliseconds m_time_to_query = std::chrono::milliseconds( 0 );
	std::future<std::string> m_current_query;
};
