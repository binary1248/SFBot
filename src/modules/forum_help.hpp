#pragma once

#include <module.hpp>
#include <chrono>
#include <string>

class forum_help : public module {
private:
	bool handle_channel_message( const std::string& user, const std::string& message ) override;
	void handle_tick( const std::chrono::milliseconds& elapsed ) override;

	void poll_forum();
	void handle_forum();

	int m_last_thread_id = 0;
	std::string m_last_thread_poster;
	std::chrono::milliseconds m_time_to_query = std::chrono::milliseconds( 0 );
	std::future<std::string> m_current_forum_query;
};
