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

// This file contains the public function API for calling into the dynamic library
// It's implemented as pure C API to avoid the usual C++ hassles

#include <JuceHeader.h>

#if JUCE_WINDOWS
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT __attribute__((visibility("default")))
#endif

extern "C" {

/** Creates a new instance of a loris state that holds all analysed files. 

	While it's theoretically possible to use multiple instances, it's not recommended
	as there are a few functions (mainly logging) that rely on the internal loris
	logging functions which use a global state.

	Returns an opaque pointer that you need to pass into all other functions.

	Call destroyLorisState to deallocate it when you're done.
*/
DLL_EXPORT void* createLorisState();

/** Deallocates the state provided by createLorisState(). */
DLL_EXPORT void destroyLorisState(void* stateToDestroy);

/** Returns the Library version to check for mismatches. */
DLL_EXPORT const char* getLibraryVersion();

/** Returns the Loris version. */
DLL_EXPORT const char* getLorisVersion();



DLL_EXPORT bool loris_analyze(void* state, char* file, double rootFrequency);

DLL_EXPORT bool loris_process(void* state, const char* file,
                              const char* command, const char* json);

DLL_EXPORT bool loris_process_custom(void* state, const char* file, void* obj, void* function);

DLL_EXPORT bool loris_config(void* state, const char* setting, const char* value);

/** Returns the number of bytes that you need to allocate before calling synthesise. */
DLL_EXPORT size_t getRequiredBytes(void* state, const char* file);

/** Synthesises the partial list for the given file. 

	state - the state context
	file - the full path name
	dst - a preallocated buffer that will be written to as float array. Use getRequiredBytes
	      in order to obtain the size of the buffer.
	numChannels - will be set to the number of channels
	numSamples - will be set to the number of samples
*/
DLL_EXPORT bool loris_synthesize(void* state, const char* file, float* dst, int& numChannels, int& numSamples);

DLL_EXPORT bool getLastMessage(void* state, char* buffer, int maxlen);

DLL_EXPORT void getIdList(char* buffer, int maxlen, bool getOptions);

DLL_EXPORT const char* getLastError(void* state);

} // extern "C"
