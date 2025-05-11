/*
 * This header creates shorter types for the automatically generated protobuf classes.
 * Since this library is only used in the context of the SwarmBots, we can assume there will be no conflict.
 * Still want to keep the package defined in the proto files though, for clarity.
 * This header also serves as a shorthand for importing all other protobuf-related headers.
 */
#ifndef Protocol_h
#define Protocol_h

#include <pb_common.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include <wrapper.pb.h>

// Shorter type names
typedef at_htlw10_swarmbots_EncoderData EncoderData;
typedef at_htlw10_swarmbots_ImuData     ImuData;
typedef at_htlw10_swarmbots_LedCmd      LedCmd;
typedef at_htlw10_swarmbots_LidarData   LidarData;
typedef at_htlw10_swarmbots_MoveCmd     MoveCmd;
typedef at_htlw10_swarmbots_Wrapper     Wrapper;
// Shorter type names for subtypes
typedef _at_htlw10_swarmbots_LidarData_LidarMeasurement LidarData_LidarMeasurement;

// Shorter init defines
#define EncoderData_init_zero   at_htlw10_swarmbots_EncoderData_init_zero
#define ImuData_init_zero       at_htlw10_swarmbots_ImuData_init_zero
#define LedCmd_init_zero        at_htlw10_swarmbots_LedCmd_init_zero
#define LidarData_init_zero     at_htlw10_swarmbots_LidarData_init_zero
#define MoveCmd_init_zero       at_htlw10_swarmbots_MoveCmd_init_zero
#define Wrapper_init_zero       at_htlw10_swarmbots_Wrapper_init_zero


#endif