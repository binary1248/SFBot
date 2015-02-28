#pragma once

#include <function.hpp>

class social : public function {
private:
	bool handle_channel_message( const std::string& user, const std::string& message ) override;
	bool handle_private_message( const std::string& user, const std::string& message ) override;
};
