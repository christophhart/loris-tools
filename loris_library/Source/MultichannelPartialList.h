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

#include "src/loris.h"
#include "src/Partial.h"

#include "Properties.h"
#include "Helpers.h"

#pragma once

namespace loris2hise {
using namespace juce;

struct MultichannelPartialList
{
	MultichannelPartialList(const juce::String& name, int numChannels);

	~MultichannelPartialList();

	double convertTimeToSeconds(double timeInput) const;

	double convertSecondsToTime(double timeInput) const;

	PartialList* get(int index) { return list[index]; }

	void setMetadata(juce::AudioFormatReader* r, double root);

	LinearEnvelope* createEnvelopeFromJSON(const juce::var& data);

	void checkArgs(bool condition, const juce::String& error, const std::function<void()>& additionalCleanupFunction = {});

	bool processCustom(void* obj, const Helpers::CustomFunction& f);
	bool process(const juce::Identifier& command, const juce::var& data);
	juce::AudioSampleBuffer synthesize();

	bool matches(const juce::File& f) const;

	size_t getRequiredBytes() const;
	int getNumSamples() const;
	int getNumChannels() const;

	void setOptions(const Options& newOptions);

	void saveAsOriginal();

private:

	Options options;

	juce::String filename;

	int numSamples = 0;
	double sampleRate = 0.0;
	double rootFrequency;

	juce::Array<PartialList*> list;

	juce::Array<PartialList*> original;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MultichannelPartialList);
};

}