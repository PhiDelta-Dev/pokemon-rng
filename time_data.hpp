/*
	Hi fellow RNG enthusiasts,

	this is an implementation of the algorithm used by
	most timers made to be used for RNG manipulation on
	the Nintendo DS in order to compute the time
	intervals needed to hit a certain delay at a certain
	time.

	Since I was unable to find a detailed explanation,
	I decided to study the behaviour and the source code
	of some of the most popular timers in order to be
	able to provide my own. I deliberately chose to use
	the same naming conventions used by EonTimer, for
	the sake of legibility.

	I plan on making my own tools for RNG manipulation,
	so I'll use this exact implementation in other
	projects, and you can do the same.

	Have fun!
	                                         -PhiDelta
	
	EonTimer by DasAmpharos:
	https://github.com/DasAmpharos/EonTimer

	RNG Timer by Teddy Katz:
	https://not-an-aardvark.github.io/rng-timer/
*/

/*
	Include guard
*/
#pragma once

/*
	Standard includes
*/
#include <cstdint>	//Data types
#include <cmath>	//std::fmod

/*
	Define the data types aliases
*/
using ui8  = std::uint8_t;
using ui16 = std::uint16_t;
using ui32 = std::uint32_t;
using ui64 = std::uint64_t;

using si8  = std::int8_t;
using si16 = std::int16_t;
using si32 = std::int32_t;
using si64 = std::int64_t;

using fp32 = float;
using fp64 = double;

/*
	Define the debug assertion
*/
#ifdef DEBUG
	#define ASSERT(exp) if(!exp) { __debugbreak(); }
#else
	#define ASSERT(exp) 
#endif

/*
	Define the constants needed to to compute the time data
*/
#define S_PER_MIN     60.0    //1 min = 60 s
#define NDS_FPS       59.8261 //Nintendo DS framerate
#define MIN_BOOT_TIME 14.0    //Minimum boot time

namespace rng::time
{
	/*
		Struct containing the data necessary to set the DS clock, boot the game and load the save file at the correct time
		The "offset" variable (a.k.a. the "minutes before target") is used to compensate for the total time when setting the clock
	*/
	struct TimeData
	{
		fp64 boot_time { 0.0 };	//Time elapsed between setting the clock and booting the game [s]
		fp64 load_time { 0.0 };	//Time elapsed between booting the game and loading the save file [s]
		ui8  offset    { 0 };	//Total time elapsed between setting the clock and loading the save file [min]
	};

	//Convert from delay [frames] to seconds
	inline const fp64 delay_to_second(const ui32& t_delay) { return static_cast<fp64>(t_delay) / NDS_FPS; }

	//Convert from seconds to delay [frames]
	inline const ui32 second_to_delay(const fp64& t_second) { return static_cast<ui32>(t_second * NDS_FPS); }

	//Get the time data from the provided calibrated and target delay and second
	const TimeData get_time_data
	(
		const ui32& t_calibrated_delay,
		const ui8&  t_calibrated_second,
		const ui32& t_target_delay,
		const ui8&  t_target_second
	)
	{
		//Initialise the resulting time data
		TimeData time_data {};

		//Calculate the load time
		time_data.load_time = delay_to_second(t_target_delay - t_calibrated_delay) + static_cast<fp64>(t_calibrated_second);

		//Break if the load time is invalid
		ASSERT(time_data.load_time > 0.0);

		//Calculate the boot time from the load time
		time_data.boot_time = std::fmod((static_cast<fp64>(t_target_second) - time_data.load_time), S_PER_MIN) + 0.2;

		//Adjust the boot time so that it's above the minumum threshold
		while (time_data.boot_time < MIN_BOOT_TIME) { time_data.boot_time += S_PER_MIN; }

		//Calculate the offset (that is, the minutes of the total time)
		time_data.offset = static_cast<ui8>((time_data.boot_time + time_data.load_time) / S_PER_MIN);

		//Return the resulting time data
		return time_data;
	}
}