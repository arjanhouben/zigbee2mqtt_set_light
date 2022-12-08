#include <iostream>
#include <chrono>
#include <thread>
#include <span>
#include <signal.h>

#include <arjan/mqttpp.hpp>

using namespace std::chrono_literals;

#include <nlohmann/json.hpp>

template < int Signal >
struct catch_signal
{
	explicit catch_signal()
	{
		static auto &ref = caught_;
		if ( signal( Signal, [](int){ ref = true; } ) == SIG_ERR )
		{
			throw std::system_error( errno, std::generic_category() );
		}
	}

	explicit operator bool() const
	{
		return caught_;
	}

	private:
		volatile bool caught_;
};
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

constexpr std::string_view homebridge = "homebridge";

int main( int argc, char *argv[] )
{
	try
	{
		arjan::mqttpp::host host;
		arjan::mqttpp::subscription zigbee(
			host,
			"zigbee2mqtt/#",
			[&]( const auto &m )
			{
				const auto topic = std::string_view( m.topic ).substr( 1 );
				if ( topic.ends_with( "set" ) ) return;
				if ( !topic.starts_with( "igbee2mqtt" ) ) return;
				const auto payload = std::string_view( static_cast< const char* >( m.payload ), m.payloadlen );
				std::copy( homebridge.begin(), homebridge.end(), m.topic + 1 );
				std::cout << "publish to: " << topic << '\n';
				arjan::mqttpp::publisher p( host );
				p.publish( std::string{ topic }, std::span{ payload }, arjan::mqttpp::retain::yes );
			}
		);

		arjan::mqttpp::subscription set(
			host,
			"homebridge/+/set",
			[&]( const mosquitto_message &m )
			{
				std::string_view topic( m.topic );
				topic.remove_prefix( homebridge.size() );
				std::cout << "publish to: " << topic << '\n';
				arjan::mqttpp::publisher p( host );
				p.publish( 
					std::string{ "zigbee2mqtt" } + std::string{ topic }, 
					std::span( static_cast< std::byte* >( m.payload ), m.payloadlen )
				);
			}
		);

		catch_signal< SIGINT > signal;
		while ( !signal )
		{
			zigbee.handle_events();
			set.handle_events();
			std::this_thread::yield();
		}
	}
	catch( std::exception &err )
	{
		std::cerr << err.what() << std::endl;
		return 1;
	}
	return 0;
}