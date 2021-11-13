project( set_mqtt_light )

cmake_minimum_required( VERSION 3.19 )

include( FetchContent )

FetchContent_Declare( json
	GIT_REPOSITORY
		https://github.com/nlohmann/json.git
)

FetchContent_Makeavailable( json )

set( WITH_CLIENTS OFF )
set( WITH_BROKER OFF )
set( WITH_APPS OFF )
set( WITH_PLUGINS OFF )
set( DOCUMENTATION OFF )

FetchContent_Declare( mosquitto 
	GIT_REPOSITORY
		https://github.com/eclipse/mosquitto.git
)

FetchContent_Makeavailable( mosquitto )

add_executable( set_mqtt_light
	main.cpp
)

target_compile_features( set_mqtt_light
	PRIVATE
		cxx_std_20
)

target_include_directories( set_mqtt_light
	PRIVATE
		${mosquitto_SOURCE_DIR}/include
)

target_link_libraries( set_mqtt_light
	PRIVATE
		libmosquitto
		nlohmann_json
)