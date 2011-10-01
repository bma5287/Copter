/*
             LUFA Library
     Copyright (C) Dean Camera, 2010.
              
  dean [at] fourwalledcubicle [dot] com
      www.fourwalledcubicle.com
*/

/*
  Copyright 2010  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this 
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in 
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting 
  documentation, and that the name of the author not be used in 
  advertising or publicity pertaining to distribution of the 
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *  \brief Common definitions and declarations for the library USB Audio Class driver.
 *
 *  Common definitions and declarations for the library USB Audio Class driver.
 *
 *  \note This file should not be included directly. It is automatically included as needed by the class driver
 *        dispatch header located in LUFA/Drivers/USB/Class/Audio.h.
 */

/** \ingroup Group_USBClassAudio
 *  @defgroup Group_USBClassAudioCommon  Common Class Definitions
 *
 *  \section Module Description
 *  Constants, Types and Enum definitions that are common to both Device and Host modes for the USB
 *  Audio Class.
 *
 *  @{
 */

#ifndef _AUDIO_CLASS_COMMON_H_
#define _AUDIO_CLASS_COMMON_H_

	/* Includes: */
		#include "../../USB.h"
		
		#include <string.h>

	/* Enable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			extern "C" {
		#endif

	/* Preprocessor Checks: */
		#if !defined(__INCLUDE_FROM_AUDIO_DRIVER)
			#error Do not include this file directly. Include LUFA/Drivers/Class/Audio.h instead.
		#endif

	/* Macros: */
		#if !defined(AUDIO_TOTAL_SAMPLE_RATES) || defined(__DOXYGEN__)
			/** Total number of discrete audio sample rates supported by the device. This value can be overridden by defining this
			 *  token in the project makefile to the desired value, and passing it to the compiler via the -D switch.
			 */
			#define AUDIO_TOTAL_SAMPLE_RATES    1
		#endif
		
		/** Descriptor header constant to indicate a Audio class interface descriptor. */
		#define DTYPE_AudioInterface         0x24

		/** Descriptor header constant to indicate a Audio class endpoint descriptor. */
		#define DTYPE_AudioEndpoint          0x25

		/** Audio class descriptor subtype value for a Audio class-specific header descriptor. */
		#define DSUBTYPE_Header              0x01

		/** Audio class descriptor subtype value for an Output Terminal Audio class-specific descriptor. */
		#define DSUBTYPE_InputTerminal       0x02

		/** Audio class descriptor subtype value for an Input Terminal Audio class-specific descriptor. */
		#define DSUBTYPE_OutputTerminal      0x03

		/** Audio class descriptor subtype value for a Feature Unit Audio class-specific descriptor. */
		#define DSUBTYPE_FeatureUnit         0x06

		/** Audio class descriptor subtype value for a general Audio class-specific descriptor. */
		#define DSUBTYPE_General             0x01

		/** Audio class descriptor subtype value for an Audio class-specific descriptor indicating the format of an audio stream. */
		#define DSUBTYPE_Format              0x02
		
		/** Supported channel mask for an Audio class terminal descriptor. See the Audio class specification for more details. */
		#define CHANNEL_LEFT_FRONT           (1 << 0)

		/** Supported channel mask for an Audio class terminal descriptor. See the Audio class specification for more details. */
		#define CHANNEL_RIGHT_FRONT          (1 << 1)

		/** Supported channel mask for an Audio class terminal descriptor. See the Audio class specification for more details. */
		#define CHANNEL_CENTER_FRONT         (1 << 2)

		/** Supported channel mask for an Audio class terminal descriptor. See the Audio class specification for more details. */
		#define CHANNEL_LOW_FREQ_ENHANCE     (1 << 3)

		/** Supported channel mask for an Audio class terminal descriptor. See the Audio class specification for more details. */
		#define CHANNEL_LEFT_SURROUND        (1 << 4)

		/** Supported channel mask for an Audio class terminal descriptor. See the Audio class specification for more details. */
		#define CHANNEL_RIGHT_SURROUND       (1 << 5)

		/** Supported channel mask for an Audio class terminal descriptor. See the Audio class specification for more details. */
		#define CHANNEL_LEFT_OF_CENTER       (1 << 6)

		/** Supported channel mask for an Audio class terminal descriptor. See the Audio class specification for more details. */
		#define CHANNEL_RIGHT_OF_CENTER      (1 << 7)

		/** Supported channel mask for an Audio class terminal descriptor. See the Audio class specification for more details. */
		#define CHANNEL_SURROUND             (1 << 8)

		/** Supported channel mask for an Audio class terminal descriptor. See the Audio class specification for more details. */
		#define CHANNEL_SIDE_LEFT            (1 << 9)

		/** Supported channel mask for an Audio class terminal descriptor. See the Audio class specification for more details. */
		#define CHANNEL_SIDE_RIGHT           (1 << 10)

		/** Supported channel mask for an Audio class terminal descriptor. See the Audio class specification for more details. */
		#define CHANNEL_TOP                  (1 << 11)

		/** Supported feature mask for an Audio class feature unit descriptor. See the Audio class specification for more details. */
		#define FEATURE_MUTE                 (1 << 0)

		/** Supported feature mask for an Audio class feature unit descriptor. See the Audio class specification for more details. */
		#define FEATURE_VOLUME               (1 << 1)

		/** Supported feature mask for an Audio class feature unit descriptor. See the Audio class specification for more details. */
		#define FEATURE_BASS                 (1 << 2)

		/** Supported feature mask for an Audio class feature unit descriptor. See the Audio class specification for more details. */
		#define FEATURE_MID                  (1 << 3)

		/** Supported feature mask for an Audio class feature unit descriptor. See the Audio class specification for more details. */
		#define FEATURE_TREBLE               (1 << 4)

		/** Supported feature mask for an Audio class feature unit descriptor. See the Audio class specification for more details. */
		#define FEATURE_GRAPHIC_EQUALIZER    (1 << 5)

		/** Supported feature mask for an Audio class feature unit descriptor. See the Audio class specification for more details. */

		/** Supported feature mask for an Audio class feature unit descriptor. See the Audio class specification for more details. */
		#define FEATURE_AUTOMATIC_GAIN       (1 << 6)

		/** Supported feature mask for an Audio class feature unit descriptor. See the Audio class specification for more details. */
		#define FEATURE_DELAY                (1 << 7)

		/** Supported feature mask for an Audio class feature unit descriptor. See the Audio class specification for more details. */
		#define FEATURE_BASS_BOOST           (1 << 8)

		/** Supported feature mask for an Audio class feature unit descriptor. See the Audio class specification for more details. */
		#define FEATURE_BASS_LOUDNESS        (1 << 9)

		/** Terminal type constant for an Audio class terminal descriptor. See the Audio class specification for more details. */		
		#define TERMINAL_UNDEFINED           0x0100

		/** Terminal type constant for an Audio class terminal descriptor. See the Audio class specification for more details. */		
		#define TERMINAL_STREAMING           0x0101

		/** Terminal type constant for an Audio class terminal descriptor. See the Audio class specification for more details. */		
		#define TERMINAL_VENDOR              0x01FF

		/** Terminal type constant for an Audio class terminal descriptor. See the Audio class specification for more details. */		
		#define TERMINAL_IN_UNDEFINED        0x0200

		/** Terminal type constant for an Audio class terminal descriptor. See the Audio class specification for more details. */		
		#define TERMINAL_IN_MIC              0x0201

		/** Terminal type constant for an Audio class terminal descriptor. See the Audio class specification for more details. */		
		#define TERMINAL_IN_DESKTOP_MIC      0x0202

		/** Terminal type constant for an Audio class terminal descriptor. See the Audio class specification for more details. */		
		#define TERMINAL_IN_PERSONAL_MIC     0x0203

		/** Terminal type constant for an Audio class terminal descriptor. See the Audio class specification for more details. */		
		#define TERMINAL_IN_OMNIDIR_MIC      0x0204

		/** Terminal type constant for an Audio class terminal descriptor. See the Audio class specification for more details. */		
		#define TERMINAL_IN_MIC_ARRAY        0x0205

		/** Terminal type constant for an Audio class terminal descriptor. See the Audio class specification for more details. */		
		#define TERMINAL_IN_PROCESSING_MIC   0x0206

		/** Terminal type constant for an Audio class terminal descriptor. See the Audio class specification for more details. */		
		#define TERMINAL_IN_OUT_UNDEFINED    0x0300

		/** Terminal type constant for an Audio class terminal descriptor. See the Audio class specification for more details. */		
		#define TERMINAL_OUT_SPEAKER         0x0301

		/** Terminal type constant for an Audio class terminal descriptor. See the Audio class specification for more details. */		
		#define TERMINAL_OUT_HEADPHONES      0x0302

		/** Terminal type constant for an Audio class terminal descriptor. See the Audio class specification for more details. */		
		#define TERMINAL_OUT_HEAD_MOUNTED    0x0303

		/** Terminal type constant for an Audio class terminal descriptor. See the Audio class specification for more details. */		
		#define TERMINAL_OUT_DESKTOP         0x0304

		/** Terminal type constant for an Audio class terminal descriptor. See the Audio class specification for more details. */		
		#define TERMINAL_OUT_ROOM            0x0305

		/** Terminal type constant for an Audio class terminal descriptor. See the Audio class specification for more details. */		
		#define TERMINAL_OUT_COMMUNICATION   0x0306

		/** Terminal type constant for an Audio class terminal descriptor. See the Audio class specification for more details. */		
		#define TERMINAL_OUT_LOWFREQ         0x0307

		/** Convenience macro, to fill a 24-bit AudioSampleFreq_t structure with the given sample rate as a 24-bit number.
		 *
		 *  \param[in] freq  Required audio sampling frequency in HZ
		 */
		#define AUDIO_SAMPLE_FREQ(freq)  {LowWord: ((uint32_t)freq & 0x00FFFF), HighByte: (((uint32_t)freq >> 16) & 0x0000FF)}
		
		/** Mask for the attributes parameter of an Audio class-specific Endpoint descriptor, indicating that the endpoint
		 *  accepts only filled endpoint packets of audio samples.
		 */
		#define EP_ACCEPTS_ONLY_FULL_PACKETS (1 << 7)

		/** Mask for the attributes parameter of an Audio class-specific Endpoint descriptor, indicating that the endpoint
		 *  will accept partially filled endpoint packets of audio samples.
		 */
		#define EP_ACCEPTS_SMALL_PACKETS     (0 << 7)
		
	/* Type Defines: */
		/** \brief Audio class-specific Interface Descriptor.
		 *
		 *  Type define for an Audio class-specific interface descriptor. This follows a regular interface descriptor to
		 *  supply extra information about the audio device's layout to the host. See the USB Audio specification for more
		 *  details.
		 */
		typedef struct
		{
			USB_Descriptor_Header_t   Header; /**< Regular descriptor header containing the descriptor's type and length. */
			uint8_t                   Subtype; /**< Sub type value used to distinguish between audio class-specific descriptors. */

			uint16_t                  ACSpecification; /**< Binary coded decimal value, indicating the supported Audio Class specification version. */
			uint16_t                  TotalLength; /**< Total length of the Audio class-specific descriptors, including this descriptor. */
			
			uint8_t                   InCollection; /**< Total number of audio class interfaces within this device. */
			uint8_t                   InterfaceNumbers[1]; /**< Interface numbers of each audio interface. */
		} USB_Audio_Interface_AC_t;
		
		/** \brief Audio class-specific Feature Unit Descriptor.
		 *
		 *  Type define for an Audio class-specific Feature Unit descriptor. This indicates to the host what features
		 *  are present in the device's audio stream for basic control, such as per-channel volume. See the USB Audio
		 *  specification for more details.
		 */
		typedef struct
		{
			USB_Descriptor_Header_t   Header; /**< Regular descriptor header containing the descriptor's type and length. */
			uint8_t                   Subtype; /**< Sub type value used to distinguish between audio class-specific descriptors. */
			
			uint8_t                   UnitID; /**< ID value of this feature unit - must be a unique value within the device. */
			uint8_t                   SourceID; /**< Source ID value of the audio source input into this feature unit. */
			
			uint8_t                   ControlSize; /**< Size of each element in the ChanelControlls array. */
			uint8_t                   ChannelControls[3]; /**< Feature masks for the control channel, and each separate audio channel. */
			
			uint8_t                   FeatureUnitStrIndex; /**< Index of a string descriptor describing this descriptor within the device. */
		} USB_Audio_FeatureUnit_t;

		/** \brief Audio class-specific Input Terminal Descriptor.
		 *
		 *  Type define for an Audio class-specific input terminal descriptor. This indicates to the host that the device
		 *  contains an input audio source, either from a physical terminal on the device, or a logical terminal (for example,
		 *  a USB endpoint). See the USB Audio specification for more details.
		 */
		typedef struct
		{
			USB_Descriptor_Header_t   Header; /**< Regular descriptor header containing the descriptor's type and length. */
			uint8_t                   Subtype; /**< Sub type value used to distinguish between audio class-specific descriptors. */
		
			uint8_t                   TerminalID; /**< ID value of this terminal unit - must be a unique value within the device. */
			uint16_t                  TerminalType; /**< Type of terminal, a TERMINAL_* mask. */
			uint8_t                   AssociatedOutputTerminal; /**< ID of associated output terminal, for physically grouped terminals
			                                                     *   such as the speaker and microphone of a phone handset.
			                                                     */
			uint8_t                   TotalChannels; /**< Total number of separate audio channels within this interface (right, left, etc.) */
			uint16_t                  ChannelConfig; /**< CHANNEL_* masks indicating what channel layout is supported by this terminal. */
			
			uint8_t                   ChannelStrIndex; /**< Index of a string descriptor describing this channel within the device. */
			uint8_t                   TerminalStrIndex; /**< Index of a string descriptor describing this descriptor within the device. */
		} USB_Audio_InputTerminal_t;

		/** \brief Audio class-specific Output Terminal Descriptor.
		 *
		 *  Type define for an Audio class-specific output terminal descriptor. This indicates to the host that the device
		 *  contains an output audio sink, either to a physical terminal on the device, or a logical terminal (for example,
		 *  a USB endpoint). See the USB Audio specification for more details.
		 */
		typedef struct
		{
			USB_Descriptor_Header_t   Header; /**< Regular descriptor header containing the descriptor's type and length. */
			uint8_t                   Subtype; /**< Sub type value used to distinguish between audio class-specific descriptors. */
		
			uint8_t                   TerminalID; /**< ID value of this terminal unit - must be a unique value within the device. */
			uint16_t                  TerminalType; /**< Type of terminal, a TERMINAL_* mask. */
			uint8_t                   AssociatedInputTerminal; /**< ID of associated input terminal, for physically grouped terminals
			                                                    *   such as the speaker and microphone of a phone handset.
			                                                    */
			uint8_t                   SourceID; /**< ID value of the unit this terminal's audio is sourced from. */
			
			uint8_t                   TerminalStrIndex; /**< Index of a string descriptor describing this descriptor within the device. */
		} USB_Audio_OutputTerminal_t;
		
		/** \brief Audio class-specific Streaming Audio Interface Descriptor.
		 *
		 *  Type define for an Audio class-specific streaming interface descriptor. This indicates to the host
		 *  how audio streams within the device are formatted. See the USB Audio specification for more details.
		 */
		typedef struct
		{
			USB_Descriptor_Header_t   Header; /**< Regular descriptor header containing the descriptor's type and length. */
			uint8_t                   Subtype; /**< Sub type value used to distinguish between audio class-specific descriptors. */
			
			uint8_t                   TerminalLink; /**< ID value of the output terminal this descriptor is describing. */
			
			uint8_t                   FrameDelay; /**< Delay in frames resulting from the complete sample processing from input to output. */
			uint16_t                  AudioFormat; /**< Format of the audio stream, see Audio Device Formats specification. */
		} USB_Audio_Interface_AS_t;
		
		/** \brief 24-Bit Audio Frequency Structure.
		 *
		 *  Type define for a 24bit audio sample frequency structure. GCC does not contain a built in 24bit datatype,
		 *  this this structure is used to build up the value instead. Fill this structure with the \ref AUDIO_SAMPLE_FREQ() macro.
		 */
		typedef struct
		{
			uint16_t                  LowWord; /**< Low 16 bits of the 24-bit value. */
			uint8_t                   HighByte; /**< Upper 8 bits of the 24-bit value. */
		} USB_Audio_SampleFreq_t;

		/** \brief Audio class-specific Format Descriptor.
		 *
		 *  Type define for an Audio class-specific audio format descriptor. This is used to give the host full details
		 *  about the number of channels, the sample resolution, acceptable sample frequencies and encoding method used
		 *  in the device's audio streams. See the USB Audio specification for more details.
		 */
		typedef struct
		{
			USB_Descriptor_Header_t   Header; /**< Regular descriptor header containing the descriptor's type and length. */
			uint8_t                   Subtype; /**< Sub type value used to distinguish between audio class-specific descriptors. */

			uint8_t                   FormatType; /**< Format of the audio stream, see Audio Device Formats specification. */
			uint8_t                   Channels; /**< Total number of discrete channels in the stream. */
			
			uint8_t                   SubFrameSize; /**< Size in bytes of each channel's sample data in the stream. */
			uint8_t                   BitResolution; /**< Bits of resolution of each channel's samples in the stream. */

			uint8_t                   SampleFrequencyType; /**< Total number of sample frequencies supported by the device. */			
			USB_Audio_SampleFreq_t    SampleFrequencies[AUDIO_TOTAL_SAMPLE_RATES]; /**< Sample frequencies supported by the device. */
		} USB_Audio_Format_t;
		
		/** \brief Audio class-specific Streaming Endpoint Descriptor.
		 *
		 *  Type define for an Audio class-specific endpoint descriptor. This contains a regular endpoint 
		 *  descriptor with a few Audio-class-specific extensions. See the USB Audio specification for more details.
		 */
		typedef struct
		{
			USB_Descriptor_Endpoint_t Endpoint; /**< Standard endpoint descriptor describing the audio endpoint. */

			uint8_t                   Refresh; /**< Always set to zero for Audio class devices. */
			uint8_t                   SyncEndpointNumber; /**< Endpoint address to send synchronization information to, if needed (zero otherwise). */
		} USB_Audio_StreamEndpoint_Std_t;
					
		/** \brief Audio class-specific Extended Endpoint Descriptor.
		 *
		 *  Type define for an Audio class-specific extended endpoint descriptor. This contains extra information
		 *  on the usage of endpoints used to stream audio in and out of the USB Audio device, and follows an Audio
		 *  class-specific extended endpoint descriptor. See the USB Audio specification for more details.
		 */
		typedef struct
		{
			USB_Descriptor_Header_t   Header; /**< Regular descriptor header containing the descriptor's type and length. */
			uint8_t                   Subtype; /**< Sub type value used to distinguish between audio class-specific descriptors. */
			
			uint8_t                   Attributes; /**< Audio class-specific endpoint attributes, such as ACCEPTS_SMALL_PACKETS. */

			uint8_t                   LockDelayUnits; /**< Units used for the LockDelay field, see Audio class specification. */
			uint16_t                  LockDelay; /**< Time required to internally lock endpoint's internal clock recovery circuitry. */
		} USB_Audio_StreamEndpoint_Spc_t;

	/* Disable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			}
		#endif
		
#endif

/** @} */
