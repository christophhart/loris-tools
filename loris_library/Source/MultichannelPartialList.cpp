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

#include "MultichannelPartialList.h"

namespace loris2hise {
using namespace juce;


MultichannelPartialList::MultichannelPartialList(const juce::String& name, int numChannels) :
	filename(name)
{
	for (int i = 0; i < numChannels; i++)
	{
		list.add(createPartialList());
	}
}

MultichannelPartialList::~MultichannelPartialList()
{
	for (auto& p : list)
		destroyPartialList(p);
}

double MultichannelPartialList::convertTimeToSeconds(double timeInput) const
{
	if (options.currentDomainType == TimeDomainType::Seconds)
		return timeInput;
	if (options.currentDomainType == TimeDomainType::Samples)
	{
		return timeInput / sampleRate;
	}
	if (options.currentDomainType == TimeDomainType::Normalised)
	{
		return timeInput * numSamples / sampleRate;
	}
	if (options.currentDomainType == TimeDomainType::Frequency)
	{
		return timeInput;
	}

	jassertfalse;
	return timeInput;
}

double MultichannelPartialList::convertSecondsToTime(double timeInput) const
{
	if (options.currentDomainType == TimeDomainType::Seconds)
		return timeInput;
	if (options.currentDomainType == TimeDomainType::Samples)
	{
		return timeInput * sampleRate;
	}
	if (options.currentDomainType == TimeDomainType::Normalised)
	{
		return timeInput * sampleRate / numSamples;
	}
	if (options.currentDomainType == TimeDomainType::Frequency)
	{
		return timeInput;
	}

	jassertfalse;
	return timeInput;
}

void MultichannelPartialList::setMetadata(juce::AudioFormatReader* r, double root)
{
	numSamples = (int)r->lengthInSamples;
	sampleRate = r->sampleRate;
	rootFrequency = root;
}

LinearEnvelope* MultichannelPartialList::createEnvelopeFromJSON(const juce::var& data)
{
	auto env = createLinearEnvelope();

	auto cleanup = [env]() { destroyLinearEnvelope(env); };

	checkArgs(data.isArray(), "must be a a list of [x, y] points", cleanup);

	if (data.isArray())
	{
		for (const auto& s : *data.getArray())
		{
			checkArgs(s.isArray() &&
				s.size() == 2, "point element must be an array with two elements", cleanup);

			auto timeValue = convertTimeToSeconds(s[0]);

			linearEnvelope_insertBreakpoint(env, timeValue, s[1]);
		}
	}

	return env;
}

void MultichannelPartialList::saveAsOriginal()
{
	for (const auto l : list)
	{
		auto newO = createPartialList();
		partialList_copy(newO, l);
		original.add(newO);
	}

	jassert(list.size() == original.size());
}

void MultichannelPartialList::checkArgs(bool condition, const juce::String& error, const std::function<void()>& additionalCleanupFunction /*= {}*/)
{
	if (!condition)
	{
		if (additionalCleanupFunction)
			additionalCleanupFunction();

		throw juce::Result::fail("process args error: " + error);
	}
}

bool MultichannelPartialList::processCustom(void* obj, const Helpers::CustomFunction& f)
{
	int channelIndex = 0;

	for (auto l : list)
	{
		int partialIndex = 0;

		for (auto& p : *l)
		{
			for (Partial::iterator iter = p.begin(); iter != p.end(); ++iter)
			{
				auto t = convertSecondsToTime(iter.time());
				auto& b = iter.breakpoint();

				Helpers::CustomFunctionArgs a(obj, b, channelIndex, partialIndex, sampleRate, t, rootFrequency);

				if (f(a))
					return true;

				iter.time();
				breakpoint_setAmplitude(&b, a.gain);
				breakpoint_setPhase(&b, a.phase);
				breakpoint_setFrequency(&b, a.frequency);
				breakpoint_setBandwidth(&b, a.bandwidth);
			}

			partialIndex++;
		}

		channelIndex++;
	}

	return false;
}

bool MultichannelPartialList::process(const juce::Identifier& command, const juce::var& data)
{
	juce::String msg;
	msg << "Process " << filename << " with command " << command << " and JSON argument " << juce::JSON::toString(data);

	Helpers::logMessage(msg.getCharPointer().getAddress());

	if (command == ProcessIds::reset)
	{
		checkArgs(data.isObject() &&
			data.getDynamicObject()->getProperties().isEmpty(),
			"must be an empty object");

		for (int i = 0; i < list.size(); i++)
		{
			partialList_clear(list[i]);
			partialList_copy(list[i], original[i]);
		}

		return true;
	}
	if (command == ProcessIds::applyFilter)
	{
		juce::ScopedValueSetter<TimeDomainType> svs(options.currentDomainType, TimeDomainType::Frequency);

		auto env = createEnvelopeFromJSON(data);

		for (auto& l : list)
		{
			for (auto& p : *l)
			{

				for (auto& b : p)
				{
					auto freq = breakpoint_getFrequency(&b);
					auto gain = linearEnvelope_valueAt(env, freq);

					gain *= breakpoint_getAmplitude(&b);

					breakpoint_setAmplitude(&b, gain);
				}
			}
		}

		destroyLinearEnvelope(env);
		return true;
	}
	if (command == ProcessIds::shiftTime)
	{
		checkArgs(data.hasProperty("offset"), "must be a JSON object with a offset property");

		auto offset = convertTimeToSeconds(data["offset"]);

		for (auto l : list)
			shiftTime(l, offset);

		return true;
	}
	if (command == ProcessIds::shiftPitch)
	{
		if (data.isArray())
		{
			auto env = createEnvelopeFromJSON(data);

			for (auto l : list)
				shiftPitch(l, env);

			destroyLinearEnvelope(env);
		}
		else if (data.isObject())
		{
			checkArgs(data.hasProperty("offset"), "shiftPitch with a constant value needs a JSON with an offset property");

			auto s = data["offset"];

			auto env = createLinearEnvelope();

			linearEnvelope_insertBreakpoint(env, 0.0, s);

			for (auto l : list)
			{
				shiftPitch(l, env);
			}

			destroyLinearEnvelope(env);
		}


		return true;
	}
	if (command == ProcessIds::scaleFrequency)
	{
		auto env = createEnvelopeFromJSON(data);

		for (auto l : list)
			scaleFrequency(l, env);

		destroyLinearEnvelope(env);

		return true;
	}

	if (command == ProcessIds::dilate)
	{
		checkArgs(data.isArray(), "must be an array with two list of data points");

		if (data.isArray())
		{
			juce::Array<double> itimes;
			juce::Array<double> ttimes;

			auto iv = data[0];
			auto tv = data[1];

			checkArgs(iv.isArray(), "first element must be a list of numbers");
			checkArgs(tv.isArray(), "second element must be a list of numbers");

			for (int i = 0; i < iv.size(); i++)
			{
				checkArgs(iv[i].isDouble(),
					"in[" + juce::String(i) + "] must be a double number");

				checkArgs(tv[i].isDouble(),
					"out[" + juce::String(i) + "] must be a double number");

				itimes.add(convertTimeToSeconds(iv[i]));
				ttimes.add(convertTimeToSeconds(tv[i]));
			}

			for (auto l : list)
				dilate(l, itimes.getRawDataPointer(), ttimes.getRawDataPointer(), itimes.size());
		}

		return true;
	}

	throw juce::Result::fail("Invalid command: " + command);
}

size_t MultichannelPartialList::getRequiredBytes() const
{
	return list.size() * numSamples * sizeof(float);
}

juce::AudioSampleBuffer MultichannelPartialList::synthesize()
{
	juce::String msg;
	msg << "Synthesize " << filename << "...";
	Helpers::logMessage(msg.getCharPointer().getAddress());
	juce::AudioSampleBuffer output(list.size(), numSamples);

	juce::HeapBlock<double> buffer;
	buffer.allocate(numSamples, true);

	for (int i = 0; i < list.size(); i++)
	{
		juce::FloatVectorOperations::clear(buffer, numSamples);
		::synthesize(list[i], buffer, numSamples, sampleRate);

		for (int s = 0; s < numSamples; s++)
		{
			output.setSample(i, s, (float)buffer[s]);
		}
	}

	Helpers::logMessage("...Synthesize OK");
	return output;
}

bool MultichannelPartialList::matches(const juce::File& f) const
{
	return f.getFullPathName() == filename;
}

int MultichannelPartialList::getNumSamples() const
{
	return numSamples;
}

int MultichannelPartialList::getNumChannels() const
{
	return list.size();
}

void MultichannelPartialList::setOptions(const Options& newOptions)
{
	options = newOptions;
}

}