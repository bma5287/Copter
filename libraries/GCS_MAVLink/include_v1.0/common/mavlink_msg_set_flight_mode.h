// MESSAGE SET_FLIGHT_MODE PACKING

#define MAVLINK_MSG_ID_SET_FLIGHT_MODE 12

typedef struct __mavlink_set_flight_mode_t
{
 uint8_t target; ///< The system setting the mode
 uint8_t flight_mode; ///< The new navigation mode
} mavlink_set_flight_mode_t;

#define MAVLINK_MSG_ID_SET_FLIGHT_MODE_LEN 2
#define MAVLINK_MSG_ID_12_LEN 2



#define MAVLINK_MESSAGE_INFO_SET_FLIGHT_MODE { \
	"SET_FLIGHT_MODE", \
	2, \
	{  { "target", NULL, MAVLINK_TYPE_UINT8_T, 0, 0, offsetof(mavlink_set_flight_mode_t, target) }, \
         { "flight_mode", NULL, MAVLINK_TYPE_UINT8_T, 0, 1, offsetof(mavlink_set_flight_mode_t, flight_mode) }, \
         } \
}


/**
 * @brief Pack a set_flight_mode message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param target The system setting the mode
 * @param flight_mode The new navigation mode
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_set_flight_mode_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint8_t target, uint8_t flight_mode)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[2];
	_mav_put_uint8_t(buf, 0, target);
	_mav_put_uint8_t(buf, 1, flight_mode);

        memcpy(_MAV_PAYLOAD(msg), buf, 2);
#else
	mavlink_set_flight_mode_t packet;
	packet.target = target;
	packet.flight_mode = flight_mode;

        memcpy(_MAV_PAYLOAD(msg), &packet, 2);
#endif

	msg->msgid = MAVLINK_MSG_ID_SET_FLIGHT_MODE;
	return mavlink_finalize_message(msg, system_id, component_id, 2, 194);
}

/**
 * @brief Pack a set_flight_mode message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message was sent over
 * @param msg The MAVLink message to compress the data into
 * @param target The system setting the mode
 * @param flight_mode The new navigation mode
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_set_flight_mode_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint8_t target,uint8_t flight_mode)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[2];
	_mav_put_uint8_t(buf, 0, target);
	_mav_put_uint8_t(buf, 1, flight_mode);

        memcpy(_MAV_PAYLOAD(msg), buf, 2);
#else
	mavlink_set_flight_mode_t packet;
	packet.target = target;
	packet.flight_mode = flight_mode;

        memcpy(_MAV_PAYLOAD(msg), &packet, 2);
#endif

	msg->msgid = MAVLINK_MSG_ID_SET_FLIGHT_MODE;
	return mavlink_finalize_message_chan(msg, system_id, component_id, chan, 2, 194);
}

/**
 * @brief Encode a set_flight_mode struct into a message
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param set_flight_mode C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_set_flight_mode_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_set_flight_mode_t* set_flight_mode)
{
	return mavlink_msg_set_flight_mode_pack(system_id, component_id, msg, set_flight_mode->target, set_flight_mode->flight_mode);
}

/**
 * @brief Send a set_flight_mode message
 * @param chan MAVLink channel to send the message
 *
 * @param target The system setting the mode
 * @param flight_mode The new navigation mode
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_set_flight_mode_send(mavlink_channel_t chan, uint8_t target, uint8_t flight_mode)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[2];
	_mav_put_uint8_t(buf, 0, target);
	_mav_put_uint8_t(buf, 1, flight_mode);

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_SET_FLIGHT_MODE, buf, 2, 194);
#else
	mavlink_set_flight_mode_t packet;
	packet.target = target;
	packet.flight_mode = flight_mode;

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_SET_FLIGHT_MODE, (const char *)&packet, 2, 194);
#endif
}

#endif

// MESSAGE SET_FLIGHT_MODE UNPACKING


/**
 * @brief Get field target from set_flight_mode message
 *
 * @return The system setting the mode
 */
static inline uint8_t mavlink_msg_set_flight_mode_get_target(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  0);
}

/**
 * @brief Get field flight_mode from set_flight_mode message
 *
 * @return The new navigation mode
 */
static inline uint8_t mavlink_msg_set_flight_mode_get_flight_mode(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  1);
}

/**
 * @brief Decode a set_flight_mode message into a struct
 *
 * @param msg The message to decode
 * @param set_flight_mode C-struct to decode the message contents into
 */
static inline void mavlink_msg_set_flight_mode_decode(const mavlink_message_t* msg, mavlink_set_flight_mode_t* set_flight_mode)
{
#if MAVLINK_NEED_BYTE_SWAP
	set_flight_mode->target = mavlink_msg_set_flight_mode_get_target(msg);
	set_flight_mode->flight_mode = mavlink_msg_set_flight_mode_get_flight_mode(msg);
#else
	memcpy(set_flight_mode, _MAV_PAYLOAD(msg), 2);
#endif
}
