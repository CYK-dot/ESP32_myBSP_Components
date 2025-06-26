/** @file
 *  @brief MAVLink comm protocol generated from esp_comm.xml
 *  @see http://mavlink.org
 */
#pragma once
#ifndef MAVLINK_ESP_COMM_H
#define MAVLINK_ESP_COMM_H

#ifndef MAVLINK_H
    #error Wrong include order: MAVLINK_ESP_COMM.H MUST NOT BE DIRECTLY USED. Include mavlink.h from the same directory instead or set ALL AND EVERY defines from MAVLINK.H manually accordingly, including the #define MAVLINK_H call.
#endif

#define MAVLINK_ESP_COMM_XML_HASH -277549981129410118

#ifdef __cplusplus
extern "C" {
#endif

// MESSAGE LENGTHS AND CRCS

#ifndef MAVLINK_MESSAGE_LENGTHS
#define MAVLINK_MESSAGE_LENGTHS {}
#endif

#ifndef MAVLINK_MESSAGE_CRCS
#define MAVLINK_MESSAGE_CRCS {{0, 50, 9, 9, 0, 0, 0}, {2, 137, 12, 12, 0, 0, 0}, {4, 237, 14, 14, 3, 12, 13}, {34, 237, 22, 22, 0, 0, 0}, {253, 83, 51, 54, 0, 0, 0}, {300, 217, 22, 22, 0, 0, 0}, {40000, 156, 5, 5, 0, 0, 0}, {50000, 3, 14, 14, 0, 0, 0}}
#endif

#include "../protocol.h"

#define MAVLINK_ENABLED_ESP_COMM

// ENUM DEFINITIONS


/** @brief Micro air vehicle / autopilot classes. This identifies the individual model. */
#ifndef HAVE_ENUM_MAV_AUTOPILOT
#define HAVE_ENUM_MAV_AUTOPILOT
typedef enum MAV_AUTOPILOT
{
   MAV_AUTOPILOT_GENERIC=0, /* Generic autopilot, full support for everything | */
   MAV_AUTOPILOT_RESERVED=1, /* Reserved for future use. | */
   MAV_AUTOPILOT_ENUM_END=2, /*  | */
} MAV_AUTOPILOT;
#endif

/** @brief MAVLINK component type reported in HEARTBEAT message. Flight controllers must report the type of the vehicle on which they are mounted (e.g. MAV_TYPE_OCTOROTOR). All other components must report a value appropriate for their type (e.g. a camera must use MAV_TYPE_CAMERA). */
#ifndef HAVE_ENUM_MAV_TYPE
#define HAVE_ENUM_MAV_TYPE
typedef enum MAV_TYPE
{
   MAV_TYPE_GENERIC=0, /* Generic micro air vehicle | */
   MAV_TYPE_FIXED_WING=1, /* Fixed wing aircraft. | */
   MAV_TYPE_GCS=6, /* Operator control unit / ground control station | */
   MAV_TYPE_ONBOARD_CONTROLLER=18, /* Onboard companion controller | */
   MAV_TYPE_CAMERA=30, /* Camera | */
   MAV_TYPE_LOG=38, /* Log | */
   MAV_TYPE_IMU=40, /* IMU | */
   MAV_TYPE_GPS=41, /* GPS | */
   MAV_TYPE_ENUM_END=42, /*  | */
} MAV_TYPE;
#endif

/** @brief These flags encode the MAV mode, see MAV_MODE enum for useful combinations. */
#ifndef HAVE_ENUM_MAV_MODE_FLAG
#define HAVE_ENUM_MAV_MODE_FLAG
typedef enum MAV_MODE_FLAG
{
   MAV_MODE_FLAG_CUSTOM_MODE_ENABLED=1, /* 0b00000001 system-specific custom mode is enabled. When using this flag to enable a custom mode all other flags should be ignored. | */
   MAV_MODE_FLAG_TEST_ENABLED=2, /* 0b00000010 system has a test mode enabled. This flag is intended for temporary system tests and should not be used for stable implementations. | */
   MAV_MODE_FLAG_AUTO_ENABLED=4, /* 0b00000100 autonomous mode enabled, system finds its own goal positions. Guided flag can be set or not, depends on the actual implementation. | */
   MAV_MODE_FLAG_GUIDED_ENABLED=8, /* 0b00001000 guided mode enabled, system flies waypoints / mission items. | */
   MAV_MODE_FLAG_STABILIZE_ENABLED=16, /* 0b00010000 system stabilizes electronically its attitude (and optionally position). It needs however further control inputs to move around. | */
   MAV_MODE_FLAG_HIL_ENABLED=32, /* 0b00100000 hardware in the loop simulation. All motors / actuators are blocked, but internal software is full operational. | */
   MAV_MODE_FLAG_MANUAL_INPUT_ENABLED=64, /* 0b01000000 remote control input is enabled. | */
   MAV_MODE_FLAG_SAFETY_ARMED=128, /* 0b10000000 MAV safety set to armed. Motors are enabled / running / can start. Ready to fly. Additional note: this flag is to be ignore when sent in the command MAV_CMD_DO_SET_MODE and MAV_CMD_COMPONENT_ARM_DISARM shall be used instead. The flag can still be used to report the armed state. | */
   MAV_MODE_FLAG_ENUM_END=129, /*  | */
} MAV_MODE_FLAG;
#endif

/** @brief These values encode the bit positions of the decode position. These values can be used to read the value of a flag bit by combining the base_mode variable with AND with the flag position value. The result will be either 0 or 1, depending on if the flag is set or not. */
#ifndef HAVE_ENUM_MAV_MODE_FLAG_DECODE_POSITION
#define HAVE_ENUM_MAV_MODE_FLAG_DECODE_POSITION
typedef enum MAV_MODE_FLAG_DECODE_POSITION
{
   MAV_MODE_FLAG_DECODE_POSITION_CUSTOM_MODE=1, /* Eighth bit: 00000001 | */
   MAV_MODE_FLAG_DECODE_POSITION_TEST=2, /* Seventh bit: 00000010 | */
   MAV_MODE_FLAG_DECODE_POSITION_AUTO=4, /* Sixth bit:   00000100 | */
   MAV_MODE_FLAG_DECODE_POSITION_GUIDED=8, /* Fifth bit:  00001000 | */
   MAV_MODE_FLAG_DECODE_POSITION_STABILIZE=16, /* Fourth bit: 00010000 | */
   MAV_MODE_FLAG_DECODE_POSITION_HIL=32, /* Third bit:  00100000 | */
   MAV_MODE_FLAG_DECODE_POSITION_MANUAL=64, /* Second bit: 01000000 | */
   MAV_MODE_FLAG_DECODE_POSITION_SAFETY=128, /* First bit:  10000000 | */
   MAV_MODE_FLAG_DECODE_POSITION_ENUM_END=129, /*  | */
} MAV_MODE_FLAG_DECODE_POSITION;
#endif

/** @brief  */
#ifndef HAVE_ENUM_MAV_STATE
#define HAVE_ENUM_MAV_STATE
typedef enum MAV_STATE
{
   MAV_STATE_UNINIT=0, /* Uninitialized system, state is unknown. | */
   MAV_STATE_BOOT=1, /* System is booting up. | */
   MAV_STATE_CALIBRATING=2, /* System is calibrating and not flight-ready. | */
   MAV_STATE_STANDBY=3, /* System is grounded and on standby. It can be launched any time. | */
   MAV_STATE_ACTIVE=4, /* System is active and might be already airborne. Motors are engaged. | */
   MAV_STATE_CRITICAL=5, /* System is in a non-normal flight mode (failsafe). It can however still navigate. | */
   MAV_STATE_EMERGENCY=6, /* System is in a non-normal flight mode (failsafe). It lost control over parts or over the whole airframe. It is in mayday and going down. | */
   MAV_STATE_POWEROFF=7, /* System just initialized its power-down sequence, will shut down now. | */
   MAV_STATE_FLIGHT_TERMINATION=8, /* System is terminating itself (failsafe or commanded). | */
   MAV_STATE_ENUM_END=9, /*  | */
} MAV_STATE;
#endif

/** @brief Legacy component ID values for particular types of hardware/software that might make up a MAVLink system (autopilot, cameras, servos, avoidance systems etc.).
      
        Components are not required or expected to use IDs with names that correspond to their type or function, but may choose to do so.
        Using an ID that matches the type may slightly reduce the chances of component id clashes, as, for historical reasons, it is less likely to be used by some other type of component.
        System integration will still need to ensure that all components have unique IDs.

        Component IDs are used for addressing messages to a particular component within a system.
        A component can use any unique ID between 1 and 255 (MAV_COMP_ID_ALL value is the broadcast address, used to send to all components).
        
        Historically component ID were also used for identifying the type of component.
        New code must not use component IDs to infer the component type, but instead check the MAV_TYPE in the HEARTBEAT message!
       */
#ifndef HAVE_ENUM_MAV_COMPONENT
#define HAVE_ENUM_MAV_COMPONENT
typedef enum MAV_COMPONENT
{
   MAV_COMP_ID_ALL=0, /* Target id (target_component) used to broadcast messages to all components of the receiving system. Components should attempt to process messages with this component ID and forward to components on any other interfaces. Note: This is not a valid *source* component id for a message. | */
   MAV_COMP_ID_AUTOPILOT1=1, /* System flight controller component ("autopilot"). Only one autopilot is expected in a particular system. | */
   MAV_COMP_ID_USER1=25, /* Id for a component on privately managed MAVLink network. Can be used for any purpose but may not be published by components outside of the private network. | */
   MAV_COMP_ID_USER2=26, /* Id for a component on privately managed MAVLink network. Can be used for any purpose but may not be published by components outside of the private network. | */
   MAV_COMP_ID_USER3=27, /* Id for a component on privately managed MAVLink network. Can be used for any purpose but may not be published by components outside of the private network. | */
   MAV_COMP_ID_USER4=28, /* Id for a component on privately managed MAVLink network. Can be used for any purpose but may not be published by components outside of the private network. | */
   MAV_COMP_ID_CAMERA=100, /* Camera #1. | */
   MAV_COMP_ID_CAMERA2=101, /* Camera #2. | */
   MAV_COMP_ID_SERVO1=140, /* Servo #1. | */
   MAV_COMP_ID_SERVO2=141, /* Servo #2. | */
   MAV_COMP_ID_SERVO3=142, /* Servo #3. | */
   MAV_COMP_ID_SERVO4=143, /* Servo #4. | */
   MAV_COMP_ID_SERVO14=153, /* Servo #14. | */
   MAV_COMP_ID_LOG=155, /* Logging component. | */
   MAV_COMP_ID_ADSB=156, /* Automatic Dependent Surveillance-Broadcast (ADS-B) component. | */
   MAV_COMP_ID_OSD=157, /* On Screen Display (OSD) devices for video links. | */
   MAV_COMP_ID_PERIPHERAL=158, /* Generic autopilot peripheral component ID. Meant for devices that do not implement the parameter microservice. | */
   MAV_COMP_ID_BATTERY=180, /* Battery #1. | */
   MAV_COMP_ID_ONBOARD_COMPUTER=191, /* Component that lives on the onboard computer (companion computer) and has some generic functionalities, such as settings system parameters and monitoring the status of some processes that don't directly speak mavlink and so on. | */
   MAV_COMP_ID_ONBOARD_COMPUTER2=192, /* Component that lives on the onboard computer (companion computer) and has some generic functionalities, such as settings system parameters and monitoring the status of some processes that don't directly speak mavlink and so on. | */
   MAV_COMP_ID_ONBOARD_COMPUTER3=193, /* Component that lives on the onboard computer (companion computer) and has some generic functionalities, such as settings system parameters and monitoring the status of some processes that don't directly speak mavlink and so on. | */
   MAV_COMP_ID_ONBOARD_COMPUTER4=194, /* Component that lives on the onboard computer (companion computer) and has some generic functionalities, such as settings system parameters and monitoring the status of some processes that don't directly speak mavlink and so on. | */
   MAV_COMP_ID_IMU=200, /* Inertial Measurement Unit (IMU) #1. | */
   MAV_COMP_ID_IMU_2=201, /* Inertial Measurement Unit (IMU) #2. | */
   MAV_COMP_ID_IMU_3=202, /* Inertial Measurement Unit (IMU) #3. | */
   MAV_COMP_ID_GPS=220, /* GPS #1. | */
   MAV_COMP_ID_GPS2=221, /* GPS #2. | */
   MAV_COMPONENT_ENUM_END=222, /*  | */
} MAV_COMPONENT;
#endif

// MAVLINK VERSION

#ifndef MAVLINK_VERSION
#define MAVLINK_VERSION 3
#endif

#if (MAVLINK_VERSION == 0)
#undef MAVLINK_VERSION
#define MAVLINK_VERSION 3
#endif

// MESSAGE DEFINITIONS
#include "./mavlink_msg_heartbeat.h"
#include "./mavlink_msg_system_time.h"
#include "./mavlink_msg_ping.h"
#include "./mavlink_msg_rc_channels_scaled.h"
#include "./mavlink_msg_statustext.h"
#include "./mavlink_msg_protocol_version.h"
#include "./mavlink_msg_comm_status_scaled.h"
#include "./mavlink_msg_comm_sensor_adc.h"

// base include



#if MAVLINK_ESP_COMM_XML_HASH == MAVLINK_PRIMARY_XML_HASH
# define MAVLINK_MESSAGE_INFO {MAVLINK_MESSAGE_INFO_HEARTBEAT, MAVLINK_MESSAGE_INFO_SYSTEM_TIME, MAVLINK_MESSAGE_INFO_PING, MAVLINK_MESSAGE_INFO_RC_CHANNELS_SCALED, MAVLINK_MESSAGE_INFO_STATUSTEXT, MAVLINK_MESSAGE_INFO_PROTOCOL_VERSION, MAVLINK_MESSAGE_INFO_COMM_STATUS_SCALED, MAVLINK_MESSAGE_INFO_COMM_SENSOR_ADC}
# define MAVLINK_MESSAGE_NAMES {{ "COMM_SENSOR_ADC", 50000 }, { "COMM_STATUS_SCALED", 40000 }, { "HEARTBEAT", 0 }, { "PING", 4 }, { "PROTOCOL_VERSION", 300 }, { "RC_CHANNELS_SCALED", 34 }, { "STATUSTEXT", 253 }, { "SYSTEM_TIME", 2 }}
# if MAVLINK_COMMAND_24BIT
#  include "../mavlink_get_info.h"
# endif
#endif

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // MAVLINK_ESP_COMM_H
