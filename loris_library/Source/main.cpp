#include <loris.h>
#include <loris/Partial.h>
#include <loris/Breakpoint.h>
#include <loris/SdifFile.h>
#include <loris/PartialUtils.h>
#include <JuceHeader.h>

#if JUCE_WINDOWS
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT __attribute__((visibility("default")))
#endif

#define DECLARE_ID(x) static const juce::Identifier x(#x);

namespace OptionIds
{
DECLARE_ID(timedomain);
DECLARE_ID(freqfloor);
DECLARE_ID(ampfloor);
DECLARE_ID(sidelobes);
DECLARE_ID(freqdrift);
DECLARE_ID(hoptime);
DECLARE_ID(croptime);
DECLARE_ID(bwregionwidth);

static juce::Array<juce::Identifier> getAllIds()
{
    return {timedomain, freqfloor, ampfloor, sidelobes, freqdrift, hoptime, croptime, bwregionwidth};
}
}

namespace ProcessIds
{
DECLARE_ID(reset);
DECLARE_ID(shiftTime);
DECLARE_ID(shiftPitch);
DECLARE_ID(applyFilter);
DECLARE_ID(scaleFrequency);
DECLARE_ID(dilate);
DECLARE_ID(custom);


static juce::Array<juce::Identifier> getAllIds()
{
    return { reset, shiftTime, shiftPitch, scaleFrequency, dilate, applyFilter, custom};
}

}
#undef DECLARE_ID



struct LorisState;

static LorisState* currentState = nullptr;

/** A state that holds all information. Create it with createLorisState and delete it with deleteLorisState().
 
    This avoids having a global context, however the error reporting will only work with the
    state that was created last (because the loris error handling is global).
 */
struct LorisState
{
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
        juce::var toJSON() const
        {
            juce::DynamicObject::Ptr obj = new juce::DynamicObject();
            
            obj->setProperty(OptionIds::timedomain,
                             getTimeDomainOptions()[(int)currentDomainType]);
            
            obj->setProperty(OptionIds::freqfloor, freqfloor);
            obj->setProperty(OptionIds::ampfloor, ampfloor);
            obj->setProperty(OptionIds::sidelobes, sidelobes);
            obj->setProperty(OptionIds::freqdrift, freqdrift);
            obj->setProperty(OptionIds::hoptime, hoptime);
            obj->setProperty(OptionIds::croptime, croptime);
            obj->setProperty(OptionIds::bwregionwidth, bwregionwidth);
            
            return juce::var(obj.get());
        }
        
        static juce::StringArray getTimeDomainOptions()
        {
            static const juce::StringArray options =
            {
                "seconds",
                "samples",
                "0to1"
            };
            
            return options;
        }
        
        void initLorisParameters()
        {
            hoptime = analyzer_getHopTime();
            croptime = analyzer_getCropTime();
            freqfloor = analyzer_getFreqFloor();
            freqdrift = analyzer_getFreqDrift();
            ampfloor = analyzer_getAmpFloor();
            sidelobes = analyzer_getSidelobeLevel();
            bwregionwidth = analyzer_getBwRegionWidth();
        }
        
        bool update(const juce::Identifier& id, const juce::var& value)
        {
            if(id == OptionIds::timedomain)
            {
                auto x = value.toString().trim().unquoted();
                
                auto idx = getTimeDomainOptions().indexOf(x);
                
                if(idx == -1)
                    throw juce::Result::fail("unknown time domain option: " + value.toString());
                
                currentDomainType = (TimeDomainType)idx;
                
                return true;
            }
            if(id == OptionIds::freqfloor) { freqfloor = (double)value; analyzer_setFreqFloor(freqfloor); return true; }
            if(id == OptionIds::ampfloor) { ampfloor = (double)value; analyzer_setAmpFloor(ampfloor); return true; }
            if(id == OptionIds::sidelobes) { sidelobes = (double)value; analyzer_setSidelobeLevel(sidelobes); return true; }
            if(id == OptionIds::freqdrift) { freqdrift = (double)value; analyzer_setFreqDrift(freqdrift); return true; }
            if(id == OptionIds::hoptime) { hoptime = (double)value; analyzer_setHopTime(hoptime); return true; }
            if(id == OptionIds::croptime) { croptime = (double)value; analyzer_setCropTime(croptime); return true; }
            if(id == OptionIds::bwregionwidth) { bwregionwidth = (double)value; analyzer_setBwRegionWidth(bwregionwidth); return true; }
            
            
            throw juce::Result::fail("Invalid option: " + id.toString());
            
        }
        
        TimeDomainType currentDomainType = TimeDomainType::Seconds;
        double freqfloor;
        double ampfloor;
        double sidelobes;
        double freqdrift;
        double hoptime;
        double croptime;
        double bwregionwidth;
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
                               double rootFrequency_):
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
        
        static void resetState(void* state)
        {
            if(state != currentState)
                currentState = (LorisState*)state;
            
            ((LorisState*)state)->lastError = juce::Result::ok();
        }

        static void reportError(const char* msg)
        {
            if(currentState != nullptr)
                currentState->reportError(msg);
        }

        static void logMessage(const char* msg)
        {
            if(currentState != nullptr)
                currentState->messages.add(juce::String(msg));
        }
        
        
    };
    
    LorisState():
       lastError(juce::Result::ok())
    {
        setExceptionHandler(Helpers::reportError);
        setNotifier(Helpers::logMessage);
        
        
    }
    
    ~LorisState()
    {
        analysedFiles.clear();
        messages.clear();
    }
    
    struct ScopedPartialList
    {
        double convertTimeToSeconds(double timeInput) const
        {
            if(options.currentDomainType == TimeDomainType::Seconds)
                return timeInput;
            if(options.currentDomainType == TimeDomainType::Samples)
            {
                return timeInput / sampleRate;
            }
            if(options.currentDomainType == TimeDomainType::Normalised)
            {
                return timeInput * numSamples / sampleRate;
            }
            if(options.currentDomainType == TimeDomainType::Frequency)
            {
                return timeInput;
            }
            
            jassertfalse;
            return timeInput;
        }
        
        double convertSecondsToTime(double timeInput) const
        {
            if(options.currentDomainType == TimeDomainType::Seconds)
                return timeInput;
            if(options.currentDomainType == TimeDomainType::Samples)
            {
                return timeInput * sampleRate;
            }
            if(options.currentDomainType == TimeDomainType::Normalised)
            {
                return timeInput * sampleRate / numSamples;
            }
            if(options.currentDomainType == TimeDomainType::Frequency)
            {
                return timeInput;
            }
            
            jassertfalse;
            return timeInput;
        }
        
        ScopedPartialList(const juce::String& name, int numChannels):
          filename(name)
        {
            for(int i = 0; i < numChannels; i++)
            {
                list.add(createPartialList());
            }
        }
        
        ~ScopedPartialList()
        {
            for(auto& p: list)
                destroyPartialList(p);
        }
        
        PartialList* get(int index) { return list[index]; }
        
        void setMetadata(juce::AudioFormatReader* r, double root)
        {
            numSamples = (int)r->lengthInSamples;
            sampleRate = r->sampleRate;
            rootFrequency = root;
        }
        
        LinearEnvelope* createEnvelopeFromJSON(const juce::var& data)
        {
            auto env = createLinearEnvelope();
            
            auto cleanup = [env]() { destroyLinearEnvelope(env); };
            
            checkArgs(data.isArray(), "must be a a list of [x, y] points", cleanup);
            
            if(data.isArray())
            {
                for(const auto& s: *data.getArray())
                {
                    checkArgs(s.isArray() &&
                              s.size() == 2, "point element must be an array with two elements", cleanup);
                    
                    auto timeValue = convertTimeToSeconds(s[0]);
                    
                    linearEnvelope_insertBreakpoint(env, timeValue, s[1]);
                }
            }
            
            return env;
        }
        
        void saveAsOriginal()
        {
            for(const auto l: list)
            {
                auto newO = createPartialList();
                partialList_copy(newO, l);
                original.add(newO);
            }
            
            jassert(list.size() == original.size());
        }
        
        void checkArgs(bool condition, const juce::String& error, const std::function<void()>& additionalCleanupFunction={})
        {
            if(!condition)
            {
                if(additionalCleanupFunction)
                    additionalCleanupFunction();
                
                throw juce::Result::fail("process args error: " + error);
            }
        }
        
        bool processCustom(void* obj, const Helpers::CustomFunction& f)
        {
            int channelIndex = 0;
            
            for(auto l: list)
            {
                int partialIndex = 0;
                
                for(auto& p: *l)
                {
                    for ( Partial::iterator iter = p.begin(); iter != p.end(); ++iter )
                    {
                        auto t =  convertSecondsToTime(iter.time());
                        auto& b = iter.breakpoint();
                        
                        Helpers::CustomFunctionArgs a(obj, b, channelIndex, partialIndex, sampleRate, t, rootFrequency);
                        
                        if(f(a))
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
        
        bool process(const juce::Identifier& command, const juce::var& data)
        {
            juce::String msg;
            msg << "Process " << filename << " with command " << command << " and JSON argument " << juce::JSON::toString(data);
            
            Helpers::logMessage(msg.getCharPointer().getAddress());
            
            if(command == ProcessIds::reset)
            {
                checkArgs(data.isObject() &&
                          data.getDynamicObject()->getProperties().isEmpty(),
                          "must be an empty object");
                
                for(int i = 0; i < list.size(); i++)
                {
                    partialList_clear(list[i]);
                    partialList_copy(list[i], original[i]);
                }
                
                return true;
            }
            if(command == ProcessIds::applyFilter)
            {
                juce::ScopedValueSetter<TimeDomainType> svs(options.currentDomainType, TimeDomainType::Frequency);
                
                auto env = createEnvelopeFromJSON(data);
                
                for(auto& l: list)
                {
                    for(auto& p: *l)
                    {
                        
                        for(auto& b: p)
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
            if(command == ProcessIds::shiftTime)
            {
                checkArgs(data.hasProperty("offset"), "must be a JSON object with a offset property");
                
                auto offset = convertTimeToSeconds(data["offset"]);
                
                for(auto l: list)
                    shiftTime(l, offset);
                
                return true;
            }
            if(command == ProcessIds::shiftPitch)
            {
                if(data.isArray())
                {
                    auto env = createEnvelopeFromJSON(data);
                    
                    for(auto l: list)
                        shiftPitch(l, env);
                    
                    destroyLinearEnvelope(env);
                }
                else if(data.isObject())
                {
                    checkArgs(data.hasProperty("offset"), "shiftPitch with a constant value needs a JSON with an offset property");
                    
                    auto s = data["offset"];
                    
                    auto env = createLinearEnvelope();
                    
                    linearEnvelope_insertBreakpoint(env, 0.0, s);
                    
                    for(auto l: list)
                    {
                        shiftPitch(l, env);
                    }
                    
                    destroyLinearEnvelope(env);
                }
                
                
                return true;
            }
            if(command == ProcessIds::scaleFrequency)
            {
                auto env = createEnvelopeFromJSON(data);
                
                for(auto l: list)
                    scaleFrequency(l, env);
                
                destroyLinearEnvelope(env);
                
                return true;
            }
            
            if(command == ProcessIds::dilate)
            {
                checkArgs(data.isArray(), "must be an array with two list of data points");
                
                if(data.isArray())
                {
                    juce::Array<double> itimes;
                    juce::Array<double> ttimes;
                    
                    auto iv = data[0];
                    auto tv = data[1];
                    
                    checkArgs(iv.isArray(), "first element must be a list of numbers");
                    checkArgs(tv.isArray(), "second element must be a list of numbers");
                    
                    for(int i = 0; i < iv.size(); i++)
                    {
                        checkArgs(iv[i].isDouble(),
                                  "in[" + juce::String(i) + "] must be a double number");
                        
                        checkArgs(tv[i].isDouble(),
                                  "out[" + juce::String(i) + "] must be a double number");
                        
                        itimes.add(convertTimeToSeconds(iv[i]));
                        ttimes.add(convertTimeToSeconds(tv[i]));
                    }
                    
                    for(auto l: list)
                        dilate(l, itimes.getRawDataPointer(), ttimes.getRawDataPointer(), itimes.size());
                }
                
                return true;
            }
            
            throw juce::Result::fail("Invalid command: " + command);
        }
        
        size_t getRequiredBytes() const
        {
            return list.size() * numSamples * sizeof(float);
        }
        
        juce::AudioSampleBuffer synthesize()
        {
            juce::String msg;
            msg << "Synthesize " << filename << "...";
            Helpers::logMessage(msg.getCharPointer().getAddress());
            juce::AudioSampleBuffer output(list.size(), numSamples);
            
            juce::HeapBlock<double> buffer;
            buffer.allocate(numSamples, true);
            
            for(int i = 0; i < list.size(); i++)
            {
                ::synthesize(list[i], buffer, numSamples, sampleRate);
                
                for(int s = 0; s < numSamples; s++)
                {
                    output.setSample(i, s, (float)buffer[s]);
                }
            }
            
            Helpers::logMessage("...Synthesize OK");
            return output;
        }
        
        bool matches(const juce::File& f) const
        {
            return f.getFullPathName() == filename;
        }
        
        int getNumSamples() const
        {
            return numSamples;
        }
        
        int getNumChannels() const
        {
            return list.size();
        }
        
        void setOptions(const Options& newOptions)
        {
            options = newOptions;
        }
        
    private:
        
        Options options;
        
        juce::String filename;
        
        int numSamples = 0;
        double sampleRate = 0.0;
        double rootFrequency;
        
        juce::Array<PartialList*> list;
        
        juce::Array<PartialList*> original;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ScopedPartialList);
    };
    
    void reportError(const char* msg)
    {
        lastError = juce::Result::fail(msg);
    }
    
    bool analyse(const juce::File& audioFile, double rootFrequency)
    {
        for(const auto& af: analysedFiles)
        {
            if(af->matches(audioFile))
            {
                messages.add("Skip " + audioFile.getFileName());
                return true;
            }
        }
        juce::AudioFormatManager m;
        m.registerBasicFormats();
        
        analyzer_configure(rootFrequency * 0.8, rootFrequency);
        analyzer_setFreqDrift(0.2 * rootFrequency);
        
        currentOption.initLorisParameters();
        
        if(juce::ScopedPointer<juce::AudioFormatReader> r = m.createReaderFor(audioFile))
        {
            messages.add("Analyse " + audioFile.getFileName());
            
            auto newEntry = new ScopedPartialList(audioFile.getFullPathName(), r->numChannels);
            
            newEntry->setMetadata(r, rootFrequency);
            newEntry->setOptions(currentOption);
            
            juce::AudioSampleBuffer bf(r->numChannels, (int)r->lengthInSamples);
            
            r->read(&bf, 0, (int)r->lengthInSamples, 0, true, true);
            
            juce::HeapBlock<double> buffer;
            
            buffer.allocate(bf.getNumSamples(), true);
            
            for(int c = 0; c < bf.getNumChannels(); c++)
            {
                for(int i = 0; i < bf.getNumSamples(); i++)
                {
                    buffer[i] = bf.getSample(c, i);
                }
                
                auto list = newEntry->get(c);
                
                analyze(buffer, bf.getNumSamples(), r->sampleRate, list);
                
            }
            
            newEntry->saveAsOriginal();
            
            analysedFiles.add(newEntry);
            
            messages.add("... Analysed OK");
            return true;
        }
        
        return false;
    }
    
    bool setOption(const juce::Identifier& id, const juce::var& data)
    {
        juce::String msg;
        msg << "Set option " << id << " with value " << data.toString().quoted();
        
        Helpers::logMessage(msg.getCharPointer().getAddress());
        
        try
        {
            if(!currentOption.update(id, data))
                return false;
            
            for(auto& s: analysedFiles)
                s->setOptions(currentOption);
        }
        catch(juce::Result& r)
        {
            lastError = r;
            return false;
        }
        
        msg = "Updated options to: ";
        msg << "\n" << juce::JSON::toString(currentOption.toJSON());
        
        Helpers::logMessage(msg.getCharPointer().getAddress());
        
        return true;
    }
    
    Options currentOption;
    
    juce::Result lastError;
    
    juce::OwnedArray<ScopedPartialList> analysedFiles;
    
    juce::StringArray messages;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LorisState);
};


static LorisState::ScopedPartialList* getExisting(void* state, const char* file)
{
    auto typed = (LorisState*)state;
    
    juce::File f(file);
    
    for(auto af: typed->analysedFiles)
    {
        if(af->matches(f))
            return af;
    }
    
    return nullptr;
}

extern "C" {

DLL_EXPORT void* createLorisState()
{
    currentState = new LorisState();
    return currentState;
}

DLL_EXPORT void destroyLorisState(void* stateToDestroy)
{
    delete (LorisState*)stateToDestroy;
}

DLL_EXPORT const char* getLibraryVersion()
{
    return ProjectInfo::versionString;
}

DLL_EXPORT const char* getLorisVersion()
{
    
    return LORIS_VERSION_STR;
}

DLL_EXPORT size_t getRequiredBytes(void* state, const char* file)
{
    LorisState::Helpers::resetState(state);
    
    if(auto s = getExisting(state, file))
        return s->getRequiredBytes();
    
    return 0;
}

DLL_EXPORT bool loris_analyze(void* state, char* file, double rootFrequency)
{
    LorisState::Helpers::resetState(state);
    
    auto typed = (LorisState*)state;
    
    juce::File f(file);
    
    return typed->analyse(f, rootFrequency);
}

DLL_EXPORT bool loris_process(void* state, const char* file,
                              const char* command, const char* json)
{
    LorisState::Helpers::resetState(state);
    
    auto data = juce::JSON::fromString(juce::StringRef(json));
    
    if(auto s = getExisting(state, file))
    {
        try
        {
            return s->process(juce::Identifier(command), data);
        }
        catch(juce::Result& r)
        {
            auto m = r.getErrorMessage();
            
            ((LorisState*)state)->reportError(m.getCharPointer().getAddress());
        }
    }
    
    return false;
}

DLL_EXPORT bool loris_process_custom(void* state, const char* file, void* obj, void* function)
{
    LorisState::Helpers::CustomFunction f = (LorisState::Helpers::CustomFunctionType)function;
    
    LorisState::Helpers::resetState(state);
    
    if(auto s = getExisting(state, file))
    {
        return s->processCustom(obj, f);
    }
    
    return false;
}

DLL_EXPORT bool loris_config(void* state, const char* setting, const char* value)
{
    LorisState::Helpers::resetState(state);
    
    auto typed = (LorisState*)state;
    
    juce::String v(value);
    
    return typed->setOption(juce::Identifier(setting), juce::var(v));
}

DLL_EXPORT bool loris_synthesize(void* state, const char* file, float* dst, int& numChannels, int& numSamples)
{
    LorisState::Helpers::resetState(state);
    
    numSamples = 0;
    numChannels = 0;
    
    if(auto s = getExisting(state, file))
    {
        jassert(s->getRequiredBytes() > 0);
        
        auto buffer = s->synthesize();
        
        for(int i = 0; i < buffer.getNumChannels(); i++)
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

DLL_EXPORT bool getLastMessage(void* state, char* buffer, int maxlen)
{
    auto typed = ((LorisState*)state);
    
    auto hasSomething = !typed->messages.isEmpty();
    
    if(hasSomething)
    {
        auto m = typed->messages[0];
        
        memset(buffer, 0, maxlen);
        memcpy(buffer, m.getCharPointer().getAddress(), m.length());
        
        typed->messages.remove(0);
    }
    
    return hasSomething;
}

DLL_EXPORT void getIdList(char* buffer, int maxlen, bool getOptions)
{
    juce::String allCommands;
    
    auto idList = getOptions ? OptionIds::getAllIds() : ProcessIds::getAllIds();
    
    for(const auto& id: idList)
        allCommands << id.toString() << ";";
    
    memset(buffer, 0, maxlen);
    
    memcpy(buffer, allCommands.getCharPointer().getAddress(), allCommands.length());
}

DLL_EXPORT char* getLastError(void* state)
{
    return ((LorisState*)state)->lastError.getErrorMessage().getCharPointer().getAddress();
}





} // extern "C"


