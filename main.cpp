#include <iostream>

#include "mosquitto.h"
#include <nlohmann/json.hpp>

template < typename T >
struct scoped
{
	scoped( T t ) :
		destroy( std::move( t ) ) {}

	~scoped()
	{
		destroy();
	}
	const T destroy;
};

template < typename T, typename D >
struct mosquitto_ptr : std::unique_ptr< T, D >
{
	mosquitto_ptr( T*t, D d ) :
		std::unique_ptr< T, D >( t, d ) {}
};

void require( int what, int result )
{
	if ( what != result )
	{
		throw std::runtime_error( "unexpected result: " + std::to_string( result ) );
	}
}

int main( int argc, char *argv[] )
{
	try
	{
		if ( argc < 5 )
		{
			throw std::runtime_error( "please specify host, port, friendly name, value" );
		}
		mosquitto_lib_init();
		scoped de_init{ &mosquitto_lib_cleanup };

		mosquitto_ptr client{ mosquitto_new( nullptr, true, nullptr ), &mosquitto_destroy };

		mosquitto_log_callback_set( client.get(), []( auto, auto, auto level, auto str ) { std::cout << str << '\n'; } );

		constexpr auto keep_alive_interval = 60;
		int port = atoi( argv[ 2 ] );
		require( MOSQ_ERR_SUCCESS, mosquitto_connect( client.get(), argv[ 1 ], port, keep_alive_interval ) );
		mosquitto_ptr disconnect{ client.get(), &mosquitto_disconnect };

		int value = std::atoi( argv[ 4 ] );

		nlohmann::json message;
		message[ "state" ] = value ? "ON" : "OFF";
		message[ "brightness" ] = value;

		auto topic = std::string{ "zigbee2mqtt/" } + argv[ 3 ] + "/set";
		int message_id = 0;
		auto message_str = message.dump();
		require( MOSQ_ERR_SUCCESS, mosquitto_publish( client.get(), &message_id, topic.c_str(), message_str.size(), message_str.data(), 0, false ) );
	}
	catch( std::exception &err )
	{
		std::cerr << err.what() << std::endl;
		return 1;
	}
	return 0;
}