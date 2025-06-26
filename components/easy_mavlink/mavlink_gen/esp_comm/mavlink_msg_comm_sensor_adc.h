#pragma once
// MESSAGE COMM_SENSOR_ADC PACKING

#define MAVLINK_MSG_ID_COMM_SENSOR_ADC 50000


typedef struct __mavlink_comm_sensor_adc_t {
 uint16_t adc1; /*<  adc channel 1*/
 uint16_t adc2; /*<  adc channel 2*/
 uint16_t adc3; /*<  adc channel 3*/
 uint16_t adc4; /*<  adc channel 4*/
 uint16_t adc5; /*<  adc channel 5*/
 uint16_t adc6; /*<  adc channel 6*/
 uint16_t adc7; /*<  adc channel 7*/
} mavlink_comm_sensor_adc_t;

#define MAVLINK_MSG_ID_COMM_SENSOR_ADC_LEN 14
#define MAVLINK_MSG_ID_COMM_SENSOR_ADC_MIN_LEN 14
#define MAVLINK_MSG_ID_50000_LEN 14
#define MAVLINK_MSG_ID_50000_MIN_LEN 14

#define MAVLINK_MSG_ID_COMM_SENSOR_ADC_CRC 3
#define MAVLINK_MSG_ID_50000_CRC 3



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_COMM_SENSOR_ADC { \
    50000, \
    "COMM_SENSOR_ADC", \
    7, \
    {  { "adc1", NULL, MAVLINK_TYPE_UINT16_T, 0, 0, offsetof(mavlink_comm_sensor_adc_t, adc1) }, \
         { "adc2", NULL, MAVLINK_TYPE_UINT16_T, 0, 2, offsetof(mavlink_comm_sensor_adc_t, adc2) }, \
         { "adc3", NULL, MAVLINK_TYPE_UINT16_T, 0, 4, offsetof(mavlink_comm_sensor_adc_t, adc3) }, \
         { "adc4", NULL, MAVLINK_TYPE_UINT16_T, 0, 6, offsetof(mavlink_comm_sensor_adc_t, adc4) }, \
         { "adc5", NULL, MAVLINK_TYPE_UINT16_T, 0, 8, offsetof(mavlink_comm_sensor_adc_t, adc5) }, \
         { "adc6", NULL, MAVLINK_TYPE_UINT16_T, 0, 10, offsetof(mavlink_comm_sensor_adc_t, adc6) }, \
         { "adc7", NULL, MAVLINK_TYPE_UINT16_T, 0, 12, offsetof(mavlink_comm_sensor_adc_t, adc7) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_COMM_SENSOR_ADC { \
    "COMM_SENSOR_ADC", \
    7, \
    {  { "adc1", NULL, MAVLINK_TYPE_UINT16_T, 0, 0, offsetof(mavlink_comm_sensor_adc_t, adc1) }, \
         { "adc2", NULL, MAVLINK_TYPE_UINT16_T, 0, 2, offsetof(mavlink_comm_sensor_adc_t, adc2) }, \
         { "adc3", NULL, MAVLINK_TYPE_UINT16_T, 0, 4, offsetof(mavlink_comm_sensor_adc_t, adc3) }, \
         { "adc4", NULL, MAVLINK_TYPE_UINT16_T, 0, 6, offsetof(mavlink_comm_sensor_adc_t, adc4) }, \
         { "adc5", NULL, MAVLINK_TYPE_UINT16_T, 0, 8, offsetof(mavlink_comm_sensor_adc_t, adc5) }, \
         { "adc6", NULL, MAVLINK_TYPE_UINT16_T, 0, 10, offsetof(mavlink_comm_sensor_adc_t, adc6) }, \
         { "adc7", NULL, MAVLINK_TYPE_UINT16_T, 0, 12, offsetof(mavlink_comm_sensor_adc_t, adc7) }, \
         } \
}
#endif

/**
 * @brief Pack a comm_sensor_adc message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param adc1  adc channel 1
 * @param adc2  adc channel 2
 * @param adc3  adc channel 3
 * @param adc4  adc channel 4
 * @param adc5  adc channel 5
 * @param adc6  adc channel 6
 * @param adc7  adc channel 7
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_comm_sensor_adc_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint16_t adc1, uint16_t adc2, uint16_t adc3, uint16_t adc4, uint16_t adc5, uint16_t adc6, uint16_t adc7)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_COMM_SENSOR_ADC_LEN];
    _mav_put_uint16_t(buf, 0, adc1);
    _mav_put_uint16_t(buf, 2, adc2);
    _mav_put_uint16_t(buf, 4, adc3);
    _mav_put_uint16_t(buf, 6, adc4);
    _mav_put_uint16_t(buf, 8, adc5);
    _mav_put_uint16_t(buf, 10, adc6);
    _mav_put_uint16_t(buf, 12, adc7);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_COMM_SENSOR_ADC_LEN);
#else
    mavlink_comm_sensor_adc_t packet;
    packet.adc1 = adc1;
    packet.adc2 = adc2;
    packet.adc3 = adc3;
    packet.adc4 = adc4;
    packet.adc5 = adc5;
    packet.adc6 = adc6;
    packet.adc7 = adc7;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_COMM_SENSOR_ADC_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_COMM_SENSOR_ADC;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_COMM_SENSOR_ADC_MIN_LEN, MAVLINK_MSG_ID_COMM_SENSOR_ADC_LEN, MAVLINK_MSG_ID_COMM_SENSOR_ADC_CRC);
}

/**
 * @brief Pack a comm_sensor_adc message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 *
 * @param adc1  adc channel 1
 * @param adc2  adc channel 2
 * @param adc3  adc channel 3
 * @param adc4  adc channel 4
 * @param adc5  adc channel 5
 * @param adc6  adc channel 6
 * @param adc7  adc channel 7
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_comm_sensor_adc_pack_status(uint8_t system_id, uint8_t component_id, mavlink_status_t *_status, mavlink_message_t* msg,
                               uint16_t adc1, uint16_t adc2, uint16_t adc3, uint16_t adc4, uint16_t adc5, uint16_t adc6, uint16_t adc7)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_COMM_SENSOR_ADC_LEN];
    _mav_put_uint16_t(buf, 0, adc1);
    _mav_put_uint16_t(buf, 2, adc2);
    _mav_put_uint16_t(buf, 4, adc3);
    _mav_put_uint16_t(buf, 6, adc4);
    _mav_put_uint16_t(buf, 8, adc5);
    _mav_put_uint16_t(buf, 10, adc6);
    _mav_put_uint16_t(buf, 12, adc7);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_COMM_SENSOR_ADC_LEN);
#else
    mavlink_comm_sensor_adc_t packet;
    packet.adc1 = adc1;
    packet.adc2 = adc2;
    packet.adc3 = adc3;
    packet.adc4 = adc4;
    packet.adc5 = adc5;
    packet.adc6 = adc6;
    packet.adc7 = adc7;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_COMM_SENSOR_ADC_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_COMM_SENSOR_ADC;
#if MAVLINK_CRC_EXTRA
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_COMM_SENSOR_ADC_MIN_LEN, MAVLINK_MSG_ID_COMM_SENSOR_ADC_LEN, MAVLINK_MSG_ID_COMM_SENSOR_ADC_CRC);
#else
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_COMM_SENSOR_ADC_MIN_LEN, MAVLINK_MSG_ID_COMM_SENSOR_ADC_LEN);
#endif
}

/**
 * @brief Pack a comm_sensor_adc message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param adc1  adc channel 1
 * @param adc2  adc channel 2
 * @param adc3  adc channel 3
 * @param adc4  adc channel 4
 * @param adc5  adc channel 5
 * @param adc6  adc channel 6
 * @param adc7  adc channel 7
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_comm_sensor_adc_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint16_t adc1,uint16_t adc2,uint16_t adc3,uint16_t adc4,uint16_t adc5,uint16_t adc6,uint16_t adc7)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_COMM_SENSOR_ADC_LEN];
    _mav_put_uint16_t(buf, 0, adc1);
    _mav_put_uint16_t(buf, 2, adc2);
    _mav_put_uint16_t(buf, 4, adc3);
    _mav_put_uint16_t(buf, 6, adc4);
    _mav_put_uint16_t(buf, 8, adc5);
    _mav_put_uint16_t(buf, 10, adc6);
    _mav_put_uint16_t(buf, 12, adc7);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_COMM_SENSOR_ADC_LEN);
#else
    mavlink_comm_sensor_adc_t packet;
    packet.adc1 = adc1;
    packet.adc2 = adc2;
    packet.adc3 = adc3;
    packet.adc4 = adc4;
    packet.adc5 = adc5;
    packet.adc6 = adc6;
    packet.adc7 = adc7;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_COMM_SENSOR_ADC_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_COMM_SENSOR_ADC;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_COMM_SENSOR_ADC_MIN_LEN, MAVLINK_MSG_ID_COMM_SENSOR_ADC_LEN, MAVLINK_MSG_ID_COMM_SENSOR_ADC_CRC);
}

/**
 * @brief Encode a comm_sensor_adc struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param comm_sensor_adc C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_comm_sensor_adc_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_comm_sensor_adc_t* comm_sensor_adc)
{
    return mavlink_msg_comm_sensor_adc_pack(system_id, component_id, msg, comm_sensor_adc->adc1, comm_sensor_adc->adc2, comm_sensor_adc->adc3, comm_sensor_adc->adc4, comm_sensor_adc->adc5, comm_sensor_adc->adc6, comm_sensor_adc->adc7);
}

/**
 * @brief Encode a comm_sensor_adc struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param comm_sensor_adc C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_comm_sensor_adc_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_comm_sensor_adc_t* comm_sensor_adc)
{
    return mavlink_msg_comm_sensor_adc_pack_chan(system_id, component_id, chan, msg, comm_sensor_adc->adc1, comm_sensor_adc->adc2, comm_sensor_adc->adc3, comm_sensor_adc->adc4, comm_sensor_adc->adc5, comm_sensor_adc->adc6, comm_sensor_adc->adc7);
}

/**
 * @brief Encode a comm_sensor_adc struct with provided status structure
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 * @param comm_sensor_adc C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_comm_sensor_adc_encode_status(uint8_t system_id, uint8_t component_id, mavlink_status_t* _status, mavlink_message_t* msg, const mavlink_comm_sensor_adc_t* comm_sensor_adc)
{
    return mavlink_msg_comm_sensor_adc_pack_status(system_id, component_id, _status, msg,  comm_sensor_adc->adc1, comm_sensor_adc->adc2, comm_sensor_adc->adc3, comm_sensor_adc->adc4, comm_sensor_adc->adc5, comm_sensor_adc->adc6, comm_sensor_adc->adc7);
}

/**
 * @brief Send a comm_sensor_adc message
 * @param chan MAVLink channel to send the message
 *
 * @param adc1  adc channel 1
 * @param adc2  adc channel 2
 * @param adc3  adc channel 3
 * @param adc4  adc channel 4
 * @param adc5  adc channel 5
 * @param adc6  adc channel 6
 * @param adc7  adc channel 7
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_comm_sensor_adc_send(mavlink_channel_t chan, uint16_t adc1, uint16_t adc2, uint16_t adc3, uint16_t adc4, uint16_t adc5, uint16_t adc6, uint16_t adc7)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_COMM_SENSOR_ADC_LEN];
    _mav_put_uint16_t(buf, 0, adc1);
    _mav_put_uint16_t(buf, 2, adc2);
    _mav_put_uint16_t(buf, 4, adc3);
    _mav_put_uint16_t(buf, 6, adc4);
    _mav_put_uint16_t(buf, 8, adc5);
    _mav_put_uint16_t(buf, 10, adc6);
    _mav_put_uint16_t(buf, 12, adc7);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_COMM_SENSOR_ADC, buf, MAVLINK_MSG_ID_COMM_SENSOR_ADC_MIN_LEN, MAVLINK_MSG_ID_COMM_SENSOR_ADC_LEN, MAVLINK_MSG_ID_COMM_SENSOR_ADC_CRC);
#else
    mavlink_comm_sensor_adc_t packet;
    packet.adc1 = adc1;
    packet.adc2 = adc2;
    packet.adc3 = adc3;
    packet.adc4 = adc4;
    packet.adc5 = adc5;
    packet.adc6 = adc6;
    packet.adc7 = adc7;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_COMM_SENSOR_ADC, (const char *)&packet, MAVLINK_MSG_ID_COMM_SENSOR_ADC_MIN_LEN, MAVLINK_MSG_ID_COMM_SENSOR_ADC_LEN, MAVLINK_MSG_ID_COMM_SENSOR_ADC_CRC);
#endif
}

/**
 * @brief Send a comm_sensor_adc message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_comm_sensor_adc_send_struct(mavlink_channel_t chan, const mavlink_comm_sensor_adc_t* comm_sensor_adc)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_comm_sensor_adc_send(chan, comm_sensor_adc->adc1, comm_sensor_adc->adc2, comm_sensor_adc->adc3, comm_sensor_adc->adc4, comm_sensor_adc->adc5, comm_sensor_adc->adc6, comm_sensor_adc->adc7);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_COMM_SENSOR_ADC, (const char *)comm_sensor_adc, MAVLINK_MSG_ID_COMM_SENSOR_ADC_MIN_LEN, MAVLINK_MSG_ID_COMM_SENSOR_ADC_LEN, MAVLINK_MSG_ID_COMM_SENSOR_ADC_CRC);
#endif
}

#if MAVLINK_MSG_ID_COMM_SENSOR_ADC_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This variant of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_comm_sensor_adc_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint16_t adc1, uint16_t adc2, uint16_t adc3, uint16_t adc4, uint16_t adc5, uint16_t adc6, uint16_t adc7)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint16_t(buf, 0, adc1);
    _mav_put_uint16_t(buf, 2, adc2);
    _mav_put_uint16_t(buf, 4, adc3);
    _mav_put_uint16_t(buf, 6, adc4);
    _mav_put_uint16_t(buf, 8, adc5);
    _mav_put_uint16_t(buf, 10, adc6);
    _mav_put_uint16_t(buf, 12, adc7);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_COMM_SENSOR_ADC, buf, MAVLINK_MSG_ID_COMM_SENSOR_ADC_MIN_LEN, MAVLINK_MSG_ID_COMM_SENSOR_ADC_LEN, MAVLINK_MSG_ID_COMM_SENSOR_ADC_CRC);
#else
    mavlink_comm_sensor_adc_t *packet = (mavlink_comm_sensor_adc_t *)msgbuf;
    packet->adc1 = adc1;
    packet->adc2 = adc2;
    packet->adc3 = adc3;
    packet->adc4 = adc4;
    packet->adc5 = adc5;
    packet->adc6 = adc6;
    packet->adc7 = adc7;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_COMM_SENSOR_ADC, (const char *)packet, MAVLINK_MSG_ID_COMM_SENSOR_ADC_MIN_LEN, MAVLINK_MSG_ID_COMM_SENSOR_ADC_LEN, MAVLINK_MSG_ID_COMM_SENSOR_ADC_CRC);
#endif
}
#endif

#endif

// MESSAGE COMM_SENSOR_ADC UNPACKING


/**
 * @brief Get field adc1 from comm_sensor_adc message
 *
 * @return  adc channel 1
 */
static inline uint16_t mavlink_msg_comm_sensor_adc_get_adc1(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  0);
}

/**
 * @brief Get field adc2 from comm_sensor_adc message
 *
 * @return  adc channel 2
 */
static inline uint16_t mavlink_msg_comm_sensor_adc_get_adc2(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  2);
}

/**
 * @brief Get field adc3 from comm_sensor_adc message
 *
 * @return  adc channel 3
 */
static inline uint16_t mavlink_msg_comm_sensor_adc_get_adc3(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  4);
}

/**
 * @brief Get field adc4 from comm_sensor_adc message
 *
 * @return  adc channel 4
 */
static inline uint16_t mavlink_msg_comm_sensor_adc_get_adc4(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  6);
}

/**
 * @brief Get field adc5 from comm_sensor_adc message
 *
 * @return  adc channel 5
 */
static inline uint16_t mavlink_msg_comm_sensor_adc_get_adc5(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  8);
}

/**
 * @brief Get field adc6 from comm_sensor_adc message
 *
 * @return  adc channel 6
 */
static inline uint16_t mavlink_msg_comm_sensor_adc_get_adc6(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  10);
}

/**
 * @brief Get field adc7 from comm_sensor_adc message
 *
 * @return  adc channel 7
 */
static inline uint16_t mavlink_msg_comm_sensor_adc_get_adc7(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  12);
}

/**
 * @brief Decode a comm_sensor_adc message into a struct
 *
 * @param msg The message to decode
 * @param comm_sensor_adc C-struct to decode the message contents into
 */
static inline void mavlink_msg_comm_sensor_adc_decode(const mavlink_message_t* msg, mavlink_comm_sensor_adc_t* comm_sensor_adc)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    comm_sensor_adc->adc1 = mavlink_msg_comm_sensor_adc_get_adc1(msg);
    comm_sensor_adc->adc2 = mavlink_msg_comm_sensor_adc_get_adc2(msg);
    comm_sensor_adc->adc3 = mavlink_msg_comm_sensor_adc_get_adc3(msg);
    comm_sensor_adc->adc4 = mavlink_msg_comm_sensor_adc_get_adc4(msg);
    comm_sensor_adc->adc5 = mavlink_msg_comm_sensor_adc_get_adc5(msg);
    comm_sensor_adc->adc6 = mavlink_msg_comm_sensor_adc_get_adc6(msg);
    comm_sensor_adc->adc7 = mavlink_msg_comm_sensor_adc_get_adc7(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_COMM_SENSOR_ADC_LEN? msg->len : MAVLINK_MSG_ID_COMM_SENSOR_ADC_LEN;
        memset(comm_sensor_adc, 0, MAVLINK_MSG_ID_COMM_SENSOR_ADC_LEN);
    memcpy(comm_sensor_adc, _MAV_PAYLOAD(msg), len);
#endif
}
