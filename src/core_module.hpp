#pragma once

#include <module.hpp>

class core : public module {
public:
	core( bool& quit, std::chrono::milliseconds tick_time, std::chrono::milliseconds send, unsigned int queue, unsigned int cps );

private:
	bool handle_channel_message( const std::string& user, const std::string& message ) override;
	bool handle_private_message( const std::string& user, const std::string& message ) override;

	bool& m_quit;
	std::chrono::milliseconds m_tick;
	std::chrono::milliseconds m_send;
	unsigned int m_queue;
	unsigned int m_cps;
};
