#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 2.8.11)

compileAsC99()

set(IOTHUB_MANAGER_H_FILES
    ./inc/iothub_client_manager/iothub_client_manager.h
)

set(AUTH_CLIENT_C_FILES
    ./src/iothub_client_manager.c
)

include_directories(${DEV_AUTH_MODULES_CLIENT_INC_FOLDER})
include_directories(${SHARED_UTIL_INC_FOLDER})
include_directories(${IOTHUB_CLIENT_INC_FOLDER})
include_directories(${CMAKE_CURRENT_LIST_DIR}/inc)
    
target_link_libraries(${whatExeIsBuilding} prov_auth_client)
target_link_libraries(${whatExeIsBuilding} hsm_security_client)

#set(DEV_AUTH_MODULES_CLIENT_INC_FOLDER "${CMAKE_CURRENT_LIST_DIR}/inc" "${CMAKE_CURRENT_LIST_DIR}/inc/internal" CACHE INTERNAL "this is what needs to be included if using iothub_client lib" FORCE)

if(${memory_trace})
    add_definitions(-DGB_MEASURE_MEMORY_FOR_THIS -DGB_DEBUG_ALLOC)
endif()

if (${use_http})
    add_definitions(-DUSE_HTTP)
endif()
if (${use_mqtt})
    add_definitions(-DUSE_MQTT)
endif()
if (${use_amqp})
    add_definitions(-DUSE_AMQP)
endif()

if(WIN32)
    #windows needs this define
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)

    if(WINCE) # Be lax with WEC 2013 compiler
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W3")
    ENDIF(WINCE)
ENDIF(WIN32)

add_library(iothub_client_manager ${AUTH_CLIENT_C_FILES} ${IOTHUB_MANAGER_H_FILES})
setSdkTargetBuildProperties(iothub_client_manager)
linkSharedUtil(iothub_client_manager)

if (${use_http})
    target_link_libraries(iothub_client_manager prov_http_transport)
endif()
if (${use_mqtt})
    target_link_libraries(iothub_client_manager prov_mqtt_transport prov_mqtt_ws_transport iothub_client_mqtt_transport)
endif()
if (${use_amqp})
    target_link_libraries(iothub_client_manager prov_amqp_ws_transport prov_amqp_transport iothub_client_amqp_transport)
    linkUAMQP(iothub_client_manager)
endif()

if (NOT ${skip_samples})
  add_subdirectory(samples)
endif()
