/*
 * This file is part of the HISE loris_library codebase (https://github.com/christophhart/loris-tools).
 * Copyright (c) 2023 Christoph Hart
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "src/loris.h"

#include "src/Breakpoint.h"
#include "src/PartialUtils.h"

#include <JuceHeader.h>

namespace loris2hise
{
using namespace juce;


enum class TimeDomainType
{
	Seconds,
	Samples,
	Normalised,
	Frequency,
	numTimeDomainTypes
};

struct Options
{
	var toJSON() const;

	static StringArray getTimeDomainOptions();

	void initLorisParameters();

	bool update(const Identifier& id, const var& value);

	TimeDomainType currentDomainType = TimeDomainType::Seconds;
	double freqfloor;
	double ampfloor = 90.0;
	double sidelobes = 90.0;
	double freqdrift;
	double hoptime = 0.0129;
	double croptime = 0.0129;
	double bwregionwidth = 1.0;
	bool enablecache = true;
	double windowwidth = 1.0;
};

struct Helpers
{
	struct FunctionPOD
	{
		// Constants
		int channelIndex = 0;
		int partialIndex = 0;
		double sampleRate = 44100.0;
		double rootFrequency = 0.0;
		void* obj = nullptr;

		// Variable properties
		double time = 0.0;
		double frequency = 0.0;
		double phase = 0.0;
		double gain = 1.0;
		double bandwidth = 0.0;

	};

	struct CustomFunctionArgs
	{
		CustomFunctionArgs(void* obj_, const Breakpoint& b, int channelIndex_,
			int partialIndex_,
			double sampleRate_,
			double time_,
			double rootFrequency_) :
			channelIndex(channelIndex_),
			partialIndex(partialIndex_),
			sampleRate(sampleRate_),
			rootFrequency(rootFrequency_),
			obj(obj_),
			time(time_)
		{
			static_assert(sizeof(CustomFunctionArgs) == sizeof(FunctionPOD), "not the same size");

			frequency = b.frequency();
			phase = b.phase();
			gain = b.amplitude();
			bandwidth = b.bandwidth();
		};

		// Constants
		const int channelIndex = 0;
		const int partialIndex = 0;
		const double sampleRate = 44100.0;
		const double rootFrequency = 0.0;
		void* obj = nullptr;

		// Variable properties
		double time = 0.0;
		double frequency = 0.0;
		double phase = 0.0;
		double gain = 1.0;
		double bandwidth = 0.0;


	};

	using CustomFunctionType = bool(*)(CustomFunctionArgs&);
	using CustomFunction = std::function<bool(CustomFunctionArgs&)>;

	static void reportError(const char* msg);

	static void logMessage(const char* msg);
};

}