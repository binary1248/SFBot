#pragma once

#include <string>
#include <chrono>
#include <future>
#include <vector>
#include <memory>

template<typename T>
class register_module;

class module {
public:
	module();
	virtual ~module();

	static std::unique_ptr<std::vector<std::unique_ptr<module>>> instantiate_all();

	static void receive_channel_message( const std::string& user, const std::string& message );
	static void receive_private_message( const std::string& user, const std::string& message );
	static void send_channel_message( const std::string& message );
	static void send_private_message( const std::string& user, const std::string& message );
	static void tick( const std::chrono::milliseconds& elapsed );

	static void register_send_channel_message_handler( std::function<void(const std::string&)> handler );
	static void register_send_private_message_handler( std::function<void(const std::string&, const std::string&)> handler );

	static void add_module( module* the_module );

	static void add_commands( std::initializer_list<std::string> commands );
	static const std::vector<std::string>& get_commands();

protected:
	void http_add_certificate( const std::string& host, const std::string& certificate, const std::string& common_name );
	std::future<std::string> http_get( const std::string& host, unsigned short port, const std::string& uri, bool secure = false );

	static bool contains( std::string haystack, const std::string& needle );
	static std::string url_encode( const std::string& str );

private:
	virtual bool handle_channel_message( const std::string& user, const std::string& message );
	virtual bool handle_private_message( const std::string& user, const std::string& message );
	virtual void handle_tick( const std::chrono::milliseconds& elapsed );

	static void add_function_constructor( std::function<std::unique_ptr<module>(void)> constructor );

	template<typename T>
	friend class register_module;
};

template<typename T>
class register_module {
public:
	template<typename... Args>
	register_module( Args&&... args ) {
		module::add_function_constructor( [&](){ return std::unique_ptr<module>( new T( std::forward<Args>( args )... ) ); } );
	}

	register_module( std::initializer_list<std::string> commands ) {
		module::add_function_constructor( [](){ return std::unique_ptr<module>( new T ); } );
		module::add_commands( commands );
	}

	template<typename... Args>
	register_module( std::initializer_list<std::string> commands, Args&&... args ) {
		module::add_function_constructor( [&](){ return std::unique_ptr<module>( new T( std::forward<Args>( args )... ) ); } );
		module::add_commands( commands );
	}
};
