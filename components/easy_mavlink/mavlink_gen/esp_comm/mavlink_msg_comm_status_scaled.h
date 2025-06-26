#pragma once
// MESSAGE COMM_STATUS_SCALED PACKING

#define MAVLINK_MSG_ID_COMM_STATUS_SCALED 40000


typedef struct __mavlink_comm_status_scaled_t {
 uint16_t volt; /*<  battery voltage,0~1024*/
 uint8_t info; /*<  led blink times*/
 uint8_t warn; /*<  led blink times*/
 uint8_t err; /*<  led blink times*/
} mavlink_comm_status_scaled_t;

#define MAVLINK_MSG_ID_COMM_STATUS_SCALED_LEN 5
#define MAVLINK_MSG_ID_COMM_STATUS_SCALED_MIN_LEN 5
#define MAVLINK_MSG_ID_40000_LEN 5
#define MAVLINK_MSG_ID_40000_MIN_LEN 5

#define MAVLINK_MSG_ID_COMM_STATUS_SCALED_CRC 156
#define MAVLINK_MSG_ID_40000_CRC 156



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_COMM_STATUS_SCALED { \
    40000, \
    "COMM_STATUS_SCALED", \
    4, \
    {  { "info", NULL, MAVLINK_TYPE_UINT8_T, 0, 2, offsetof(mavlink_comm_status_scaled_t, info) }, \
         { "warn", NULL, MAVLINK_TYPE_UINT8_T, 0, 3, offsetof(mavlink_comm_status_scaled_t, warn) }, \
         { "err", NULL, MAVLINK_TYPE_UINT8_T, 0, 4, offsetof(mavlink_comm_status_scaled_t, err) }, \
         { "volt", NULL, MAVLINK_TYPE_UINT16_T, 0, 0, offsetof(mavlink_comm_status_scaled_t, volt) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_COMM_STATUS_SCALED { \
    "COMM_STATUS_SCALED", \
    4, \
    {  { "info", NULL, MAVLINK_TYPE_UINT8_T, 0, 2, offsetof(mavlink_comm_status_scaled_t, info) }, \
         { "warn", NULL, MAVLINK_TYPE_UINT8_T, 0, 3, offsetof(mavlink_comm_status_scaled_t, warn) }, \
         { "err", NULL, MAVLINK_TYPE_UINT8_T, 0, 4, offsetof(mavlink_comm_status_scaled_t, err) }, \
         { "volt", NULL, MAVLINK_TYPE_UINT16_T, 0, 0, offsetof(mavlink_comm_status_scaled_t, volt) }, \
         } \
}
#endif

/**
 * @brief Pack a comm_status_scaled message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param info  led blink times
 * @param warn  led blink times
 * @param err  led blink times
 * @param volt  battery voltage,0~1024
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_comm_status_scaled_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint8_t info, uint8_t warn, uint8_t err, uint16_t volt)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_COMM_STATUS_SCALED_LEN];
    _mav_put_uint16_t(buf, 0, volt);
    _mav_put_uint8_t(buf, 2, info);
    _mav_put_uint8_t(buf, 3, warn);
    _mav_put_uint8_t(buf, 4, err);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_COMM_STATUS_SCALED_LEN);
#else
    mavlink_comm_status_scaled_t packet;
    packet.volt = volt;
    packet.info = info;
    packet.warn = warn;
    packet.err = err;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_COMM_STATUS_SCALED_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_COMM_STATUS_SCALED;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_COMM_STATUS_SCALED_MIN_LEN, MAVLINK_MSG_ID_COMM_STATUS_SCALED_LEN, MAVLINK_MSG_ID_COMM_STATUS_SCALED_CRC);
}

/**
 * @brief Pack a comm_status_scaled message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 *
 * @param info  led blink times
 * @param warn  led blink times
 * @param err  led blink times
 * @param volt  battery voltage,0~1024
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_comm_status_scaled_pack_status(uint8_t system_id, uint8_t component_id, mavlink_status_t *_status, mavlink_message_t* msg,
                               uint8_t info, uint8_t warn, uint8_t err, uint16_t volt)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_COMM_STATUS_SCALED_LEN];
    _mav_put_uint16_t(buf, 0, volt);
    _mav_put_uint8_t(buf, 2, info);
    _mav_put_uint8_t(buf, 3, warn);
    _mav_put_uint8_t(buf, 4, err);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_COMM_STATUS_SCALED_LEN);
#else
    mavlink_comm_status_scaled_t packet;
    packet.volt = volt;
    packet.info = info;
    packet.warn = warn;
    packet.err = err;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_COMM_STATUS_SCALED_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_COMM_STATUS_SCALED;
#if MAVLINK_CRC_EXTRA
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_COMM_STATUS_SCALED_MIN_LEN, MAVLINK_MSG_ID_COMM_STATUS_SCALED_LEN, MAVLINK_MSG_ID_COMM_STATUS_SCALED_CRC);
#else
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_COMM_STATUS_SCALED_MIN_LEN, MAVLINK_MSG_ID_COMM_STATUS_SCALED_LEN);
#endif
}

/**
 * @brief Pack a comm_status_scaled message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param info  led blink times
 * @param warn  led blink times
 * @param err  led blink times
 * @param volt  battery voltage,0~1024
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_comm_status_scaled_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint8_t info,uint8_t warn,uint8_t err,uint16_t volt)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_COMM_STATUS_SCALED_LEN];
    _mav_put_uint16_t(buf, 0, volt);
    _mav_put_uint8_t(buf, 2, info);
    _mav_put_uint8_t(buf, 3, warn);
    _mav_put_uint8_t(buf, 4, err);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_COMM_STATUS_SCALED_LEN);
#else
    mavlink_comm_status_scaled_t packet;
    packet.volt = volt;
    packet.info = info;
    packet.warn = warn;
    packet.err = err;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_COMM_STATUS_SCALED_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_COMM_STATUS_SCALED;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_COMM_STATUS_SCALED_MIN_LEN, MAVLINK_MSG_ID_COMM_STATUS_SCALED_LEN, MAVLINK_MSG_ID_COMM_STATUS_SCALED_CRC);
}

/**
 * @brief Encode a comm_status_scaled struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param comm_status_scaled C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_comm_status_scaled_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_comm_status_scaled_t* comm_status_scaled)
{
    return mavlink_msg_comm_status_scaled_pack(system_id, component_id, msg, comm_status_scaled->info, comm_status_scaled->warn, comm_status_scaled->err, comm_status_scaled->volt);
}

/**
 * @brief Encode a comm_status_scaled struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param comm_status_scaled C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_comm_status_scaled_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_comm_status_scaled_t* comm_status_scaled)
{
    return mavlink_msg_comm_status_scaled_pack_chan(system_id, component_id, chan, msg, comm_status_scaled->info, comm_status_scaled->warn, comm_status_scaled->err, comm_status_scaled->volt);
}

/**
 * @brief Encode a comm_status_scaled struct with provided status structure
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 * @param comm_status_scaled C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_comm_status_scaled_encode_status(uint8_t system_id, uint8_t component_id, mavlink_status_t* _status, mavlink_message_t* msg, const mavlink_comm_status_scaled_t* comm_status_scaled)
{
    return mavlink_msg_comm_status_scaled_pack_status(system_id, component_id, _status, msg,  comm_status_scaled->info, comm_status_scaled->warn, comm_status_scaled->err, comm_status_scaled->volt);
}

/**
 * @brief Send a comm_status_scaled message
 * @param chan MAVLink channel to send the message
 *
 * @param info  led blink times
 * @param warn  led blink times
 * @param err  led blink times
 * @param volt  battery voltage,0~1024
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_comm_status_scaled_send(mavlink_channel_t chan, uint8_t info, uint8_t warn, uint8_t err, uint16_t volt)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_COMM_STATUS_SCALED_LEN];
    _mav_put_uint16_t(buf, 0, volt);
    _mav_put_uint8_t(buf, 2, info);
    _mav_put_uint8_t(buf, 3, warn);
    _mav_put_uint8_t(buf, 4, err);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_COMM_STATUS_SCALED, buf, MAVLINK_MSG_ID_COMM_STATUS_SCALED_MIN_LEN, MAVLINK_MSG_ID_COMM_STATUS_SCALED_LEN, MAVLINK_MSG_ID_COMM_STATUS_SCALED_CRC);
#else
    mavlink_comm_status_scaled_t packet;
    packet.volt = volt;
    packet.info = info;
    packet.warn = warn;
    packet.err = err;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_COMM_STATUS_SCALED, (const char *)&packet, MAVLINK_MSG_ID_COMM_STATUS_SCALED_MIN_LEN, MAVLINK_MSG_ID_COMM_STATUS_SCALED_LEN, MAVLINK_MSG_ID_COMM_STATUS_SCALED_CRC);
#endif
}

/**
 * @brief Send a comm_status_scaled message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_comm_status_scaled_send_struct(mavlink_channel_t chan, const mavlink_comm_status_scaled_t* comm_status_scaled)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_comm_status_scaled_send(chan, comm_status_scaled->info, comm_status_scaled->warn, comm_status_scaled->err, comm_status_scaled->volt);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_COMM_STATUS_SCALED, (const char *)comm_status_scaled, MAVLINK_MSG_ID_COMM_STATUS_SCALED_MIN_LEN, MAVLINK_MSG_ID_COMM_STATUS_SCALED_LEN, MAVLINK_MSG_ID_COMM_STATUS_SCALED_CRC);
#endif
}

#if MAVLINK_MSG_ID_COMM_STATUS_SCALED_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This variant of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_comm_status_scaled_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint8_t info, uint8_t warn, uint8_t err, uint16_t volt)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint16_t(buf, 0, volt);
    _mav_put_uint8_t(buf, 2, info);
    _mav_put_uint8_t(buf, 3, warn);
    _mav_put_uint8_t(buf, 4, err);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_COMM_STATUS_SCALED, buf, MAVLINK_MSG_ID_COMM_STATUS_SCALED_MIN_LEN, MAVLINK_MSG_ID_COMM_STATUS_SCALED_LEN, MAVLINK_MSG_ID_COMM_STATUS_SCALED_CRC);
#else
    mavlink_comm_status_scaled_t *packet = (mavlink_comm_status_scaled_t *)msgbuf;
    packet->volt = volt;
    packet->info = info;
    packet->warn = warn;
    packet->err = err;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_COMM_STATUS_SCALED, (const char *)packet, MAVLINK_MSG_ID_COMM_STATUS_SCALED_MIN_LEN, MAVLINK_MSG_ID_COMM_STATUS_SCALED_LEN, MAVLINK_MSG_ID_COMM_STATUS_SCALED_CRC);
#endif
}
#endif

#endif

// MESSAGE COMM_STATUS_SCALED UNPACKING


/**
 * @brief Get field info from comm_status_scaled message
 *
 * @return  led blink times
 */
static inline uint8_t mavlink_msg_comm_status_scaled_get_info(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  2);
}

/**
 * @brief Get field warn from comm_status_scaled message
 *
 * @return  led blink times
 */
static inline uint8_t mavlink_msg_comm_status_scaled_get_warn(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  3);
}

/**
 * @brief Get field err from comm_status_scaled message
 *
 * @return  led blink times
 */
static inline uint8_t mavlink_msg_comm_status_scaled_get_err(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  4);
}

/**
 * @brief Get field volt from comm_status_scaled message
 *
 * @return  battery voltage,0~1024
 */
static inline uint16_t mavlink_msg_comm_status_scaled_get_volt(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  0);
}

/**
 * @brief Decode a comm_status_scaled message into a struct
 *
 * @param msg The message to decode
 * @param comm_status_scaled C-struct to decode the message contents into
 */
static inline void mavlink_msg_comm_status_scaled_decode(const mavlink_message_t* msg, mavlink_comm_status_scaled_t* comm_status_scaled)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    comm_status_scaled->volt = mavlink_msg_comm_status_scaled_get_volt(msg);
    comm_status_scaled->info = mavlink_msg_comm_status_scaled_get_info(msg);
    comm_status_scaled->warn = mavlink_msg_comm_status_scaled_get_warn(msg);
    comm_status_scaled->err = mavlink_msg_comm_status_scaled_get_err(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_COMM_STATUS_SCALED_LEN? msg->len : MAVLINK_MSG_ID_COMM_STATUS_SCALED_LEN;
        memset(comm_status_scaled, 0, MAVLINK_MSG_ID_COMM_STATUS_SCALED_LEN);
    memcpy(comm_status_scaled, _MAV_PAYLOAD(msg), len);
#endif
}
