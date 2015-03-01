#pragma once

#include <module.hpp>

class core : public module {
public:
	core( bool& quit );

private:
	bool handle_channel_message( const std::string& user, const std::string& message ) override;
	bool handle_private_message( const std::string& user, const std::string& message ) override;

	bool& m_quit;
};
