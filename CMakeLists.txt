project( mqtt_homebridge_proxy )

cmake_minimum_required( VERSION 3.19 )

include( FetchContent )

option( WITH_CJSON OFF )

FetchContent_Declare( json
	GIT_REPOSITORY
		https://github.com/nlohmann/json.git
)

FetchContent_Makeavailable( json )

option( WITH_CLIENTS OFF )
option( WITH_BROKER OFF )
option( WITH_APPS OFF )
option( WITH_PLUGINS OFF )
option( DOCUMENTATION OFF )

FetchContent_Declare( mqttpp 
	GIT_REPOSITORY
		https://github.com/arjanhouben/mqttpp
	GIT_TAG
		main
)

FetchContent_Makeavailable( mqttpp )

add_executable( mqtt_homebridge_proxy
	main.cpp
)

target_compile_features( mqtt_homebridge_proxy
	PRIVATE
		cxx_std_20
)

target_include_directories( mqtt_homebridge_proxy
	PRIVATE
		${mosquitto_SOURCE_DIR}/include
)

target_link_libraries( mqtt_homebridge_proxy
	PRIVATE
		mqttpp
		nlohmann_json
)