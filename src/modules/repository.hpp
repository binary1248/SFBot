#pragma once

#include <module.hpp>
#include <chrono>

class repository : public module {
public:
	repository( const std::string& uri );

private:
	bool handle_channel_message( const std::string& user, const std::string& message ) override;
	void handle_tick( const std::chrono::milliseconds& elapsed ) override;

	void poll_repository();
	void handle_repository();
	void handle_stargazer();
	void handle_lead();

	std::string m_uri;

	int m_star_count = 0;
	int m_lead_star_count = 0;
	std::string m_newest_stargazer;
	std::string m_lead_repository;
	std::chrono::milliseconds m_time_to_query = std::chrono::milliseconds( 0 );
	std::future<std::string> m_current_repository_query;
	std::future<std::string> m_current_stargazer_query;
	std::future<std::string> m_current_lead_query;
};
