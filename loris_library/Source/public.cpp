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

#include "LorisState.h"
#include "public.h"


static loris2hise::MultichannelPartialList* getExisting(void* state, const char* file)
{
	auto typed = (loris2hise::LorisState*)state;

	juce::File f(file);

	for (auto af : typed->analysedFiles)
	{
		if (af->matches(f))
			return af;
	}

	return nullptr;
}

extern "C"
{

void* createLorisState()
{
	return loris2hise::LorisState::getCurrentInstance(true);
}

void destroyLorisState(void* stateToDestroy)
{
	delete (loris2hise::LorisState*)stateToDestroy;
}

const char* getLibraryVersion()
{
	return ProjectInfo::versionString;
}

const char* getLorisVersion()
{
	return LORIS_VERSION_STR;
}

size_t getRequiredBytes(void* state, const char* file)
{
	loris2hise::LorisState::resetState(state);

	if (auto s = getExisting(state, file))
		return s->getRequiredBytes();

	return 0;
}

bool loris_analyze(void* state, char* file, double rootFrequency)
{
	loris2hise::LorisState::resetState(state);

	auto typed = (loris2hise::LorisState*)state;

	juce::File f(file);

	return typed->analyse(f, rootFrequency);
}

bool loris_process(void* state, const char* file, const char* command, const char* json)
{
	loris2hise::LorisState::resetState(state);

	auto data = juce::JSON::fromString(juce::StringRef(json));

	if (auto s = getExisting(state, file))
	{
		try
		{
			return s->process(juce::Identifier(command), data);
		}
		catch (juce::Result& r)
		{
			auto m = r.getErrorMessage();

			((loris2hise::LorisState*)state)->reportError(m.getCharPointer().getAddress());
		}
	}

	return false;
}

bool loris_process_custom(void* state, const char* file, void* obj, void* function)
{
	loris2hise::Helpers::CustomFunction f = (loris2hise::Helpers::CustomFunctionType)function;

	loris2hise::LorisState::resetState(state);

	if (auto s = getExisting(state, file))
	{
		return s->processCustom(obj, f);
	}

	return false;
}

bool loris_config(void* state, const char* setting, const char* value)
{
	loris2hise::LorisState::resetState(state);

	auto typed = (loris2hise::LorisState*)state;

	juce::String v(value);

	return typed->setOption(juce::Identifier(setting), juce::var(v));
}

bool loris_synthesize(void* state, const char* file, float* dst, int& numChannels, int& numSamples)
{
	loris2hise::LorisState::resetState(state);

	numSamples = 0;
	numChannels = 0;

	if (auto s = getExisting(state, file))
	{
		jassert(s->getRequiredBytes() > 0);

		auto buffer = s->synthesize();

		for (int i = 0; i < buffer.getNumChannels(); i++)
		{
			juce::FloatVectorOperations::copy(dst, buffer.getReadPointer(i), buffer.getNumSamples());

			dst += buffer.getNumSamples();
		}

		numSamples = s->getNumSamples();
		numChannels = s->getNumChannels();

		return true;
	}

	return false;
}

bool getLastMessage(void* state, char* buffer, int maxlen)
{
	auto typed = ((loris2hise::LorisState*)state);

	auto hasSomething = !typed->messages.isEmpty();

	if (hasSomething)
	{
		auto m = typed->messages[0];

		memset(buffer, 0, maxlen);
		memcpy(buffer, m.getCharPointer().getAddress(), m.length());

		typed->messages.remove(0);
	}

	return hasSomething;
}

void getIdList(char* buffer, int maxlen, bool getOptions)
{
	juce::String allCommands;

	auto idList = getOptions ? loris2hise::OptionIds::getAllIds() : loris2hise::ProcessIds::getAllIds();

	for (const auto& id : idList)
		allCommands << id.toString() << ";";

	memset(buffer, 0, maxlen);

	memcpy(buffer, allCommands.getCharPointer().getAddress(), allCommands.length());
}

const char* getLastError(void* state)
{
	return ((loris2hise::LorisState*)state)->lastError.getErrorMessage().getCharPointer().getAddress();
}

}