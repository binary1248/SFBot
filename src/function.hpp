#pragma once

#include <string>
#include <chrono>
#include <future>
#include <vector>
#include <memory>

template<typename T>
class register_function;

class function {
public:
	function();
	virtual ~function();

	static std::unique_ptr<std::vector<std::unique_ptr<function>>> instantiate_all();

	static void receive_channel_message( const std::string& user, const std::string& message );
	static void receive_private_message( const std::string& user, const std::string& message );
	static void send_channel_message( const std::string& message );
	static void send_private_message( const std::string& user, const std::string& message );
	static void tick( const std::chrono::milliseconds& elapsed );

	static void register_send_channel_message_handler( std::function<void(const std::string&)> handler );
	static void register_send_private_message_handler( std::function<void(const std::string&, const std::string&)> handler );

	static void add_commands( std::initializer_list<std::string> commands );
	static const std::vector<std::string>& get_commands();

protected:
	void http_add_certificate( const std::string& host, const std::string& certificate, const std::string& common_name );
	std::future<std::string> http_get( const std::string& host, unsigned short port, const std::string& uri, bool secure = false );

private:
	virtual bool handle_channel_message( const std::string& user, const std::string& message );
	virtual bool handle_private_message( const std::string& user, const std::string& message );
	virtual void handle_tick( const std::chrono::milliseconds& elapsed );

	static void add_function_constructor( std::function<std::unique_ptr<function>(void)> constructor );

	template<typename T>
	friend class register_function;
};

template<typename T>
class register_function {
public:
	template<typename... Args>
	register_function( Args&&... args ) {
		function::add_function_constructor( [args...](){ return std::unique_ptr<function>( new T( std::forward<Args>( args )... ) ); } );
	}

	register_function( std::initializer_list<std::string> commands ) {
		function::add_function_constructor( [](){ return std::unique_ptr<function>( new T ); } );
		function::add_commands( commands );
	}

	template<typename... Args>
	register_function( std::initializer_list<std::string> commands, Args&&... args ) {
		function::add_function_constructor( [args...](){ return std::unique_ptr<function>( new T( std::forward<Args>( args )... ) ); } );
		function::add_commands( commands );
	}
};
