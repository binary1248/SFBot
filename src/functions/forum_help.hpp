#pragma once

#include <function.hpp>
#include <chrono>

class forum_help : public function {
private:
	bool handle_channel_message( const std::string& user, const std::string& message ) override;
	void handle_tick( const std::chrono::milliseconds& elapsed ) override;

	int m_last_thread_id = 0;
	std::chrono::milliseconds m_time_to_query = std::chrono::milliseconds( 0 );
	std::future<std::string> m_current_query;
};
