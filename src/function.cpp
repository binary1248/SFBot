#include <function.hpp>
#include <SFNUL.hpp>
#include <tuple>

namespace {

struct impl{

sfn::HTTPClient http_client;
std::vector<std::tuple<sfn::HTTPRequest, std::string, unsigned short, std::promise<std::string>>> requests;

};

std::unique_ptr<impl> instance;
int ref_count = 0;

std::vector<std::function<std::unique_ptr<function>(void)>> function_constructors;
std::vector<std::unique_ptr<function>>* functions;

std::function<void(const std::string&)> send_channel_message_handler;
std::function<void(const std::string&, const std::string&)> send_private_message_handler;

std::vector<std::string> command_list;

}

function::function() {
	if( ref_count < 1 ) {
		instance.reset( new impl );
	}

	++ref_count;
}

function::~function() {
	--ref_count;

	if( ref_count < 1 ) {
		instance.reset();
	}
}

std::unique_ptr<std::vector<std::unique_ptr<function>>> function::instantiate_all() {
	auto unique_functions = std::unique_ptr<std::vector<std::unique_ptr<function>>>( new std::vector<std::unique_ptr<function>> );

	for( const auto& f : function_constructors ) {
		unique_functions->emplace_back( f() );
	}

	functions = unique_functions.get();
	return unique_functions;
}

void function::receive_channel_message( const std::string& user, const std::string& message ) {
	for( auto& f : *functions ) {
		if( f->handle_channel_message( user, message ) ) {
			return;
		}
	}
}

void function::receive_private_message( const std::string& user, const std::string& message ) {
	for( auto& f : *functions ) {
		if( f->handle_private_message( user, message ) ) {
			return;
		}
	}
}

void function::send_channel_message( const std::string& message ) {
	if( send_channel_message_handler ) {
		send_channel_message_handler( message );
	}
}

void function::send_private_message( const std::string& user, const std::string& message ) {
	if( send_private_message_handler ) {
		send_private_message_handler( user, message );
	}
}

void function::tick( const std::chrono::milliseconds& elapsed ) {
	if( instance ) {
		instance->http_client.Update();

		for( auto iter = std::begin( instance->requests ); iter != std::end( instance->requests ); ) {
			const auto& request = std::get<0>( *iter );
			const auto& host = std::get<1>( *iter );
			const auto& port = std::get<2>( *iter );

			auto response = instance->http_client.GetResponse( request, host, port );

			if( response.IsComplete() ) {
				auto& promise = std::get<3>( *iter );
				promise.set_value( response.GetBody() );

				iter = instance->requests.erase( iter );
				continue;
			}

			++iter;
		}
	}

	for( auto& f : *functions ) {
		f->handle_tick( elapsed );
	}
}

void function::register_send_channel_message_handler( std::function<void(const std::string&)> handler ) {
	send_channel_message_handler = handler;
}

void function::register_send_private_message_handler( std::function<void(const std::string&, const std::string&)> handler ) {
	send_private_message_handler = handler;
}

void function::http_add_certificate( const std::string& host, const std::string& certificate, const std::string& common_name ) {
	instance->http_client.LoadCertificate( host, sfn::TlsCertificate::Create( certificate ), common_name );
}

std::future<std::string> function::http_get( const std::string& host, unsigned short port, const std::string& uri, bool secure ) {
	sfn::HTTPRequest request{};

	request.SetMethod( "GET" );
	request.SetHeaderValue( "Host", host );
	request.SetHeaderValue( "User-Agent", "SFBot" );
	request.SetURI( uri );

	instance->http_client.SendRequest( request, host, port, secure );

	auto promise = std::promise<std::string>();
	auto future = promise.get_future();

	instance->requests.emplace_back( std::make_tuple( std::move( request ), host, port, std::move( promise ) ) );

	return future;
}

bool function::handle_channel_message( const std::string& /*user*/, const std::string& /*message*/ ) {
	return false;
}

bool function::handle_private_message( const std::string& /*user*/, const std::string& /*message*/ ) {
	return false;
}

void function::handle_tick( const std::chrono::milliseconds& /*elapsed*/ ) {
}

void function::add_function_constructor( std::function<std::unique_ptr<function>(void)> constructor ) {
	function_constructors.push_back( std::move( constructor ) );
}

void function::add_commands( std::initializer_list<std::string> commands ) {
	command_list.insert( std::end( command_list ), std::begin( commands ), std::end( commands ) );
	std::sort( std::begin( command_list ), std::end( command_list ) );
}

const std::vector<std::string>& function::get_commands() {
	return command_list;
}
