
namespace PitchLock
{
	// UI Setup

	const var PitchLockSliders = [Content.getComponent("PitchLockAmount"),
	                              Content.getComponent("PitchShift"),
	                              Content.getComponent("NoiseGain")];
	
	
	reg pitchLockAmount = 0.0;
	reg pitchFactor = 1.0;
	reg noiseGain = 1.0;
	
	inline function onPitchLockSlider(component, value)
	{
		local idx = PitchLockSliders.indexOf(component);
		
		if(idx == 0)
		{
			pitchLockAmount = value;
		}
		else if (idx == 1)
		{
			pitchFactor = Engine.getPitchRatioFromSemitones(value);
		}
		else
		{
			noiseGain = Engine.getGainFactorForDecibels(value);
		}
	}
	
	for(s in PitchLockSliders)
	{
		s.setLocalLookAndFeel(Manifest.defaultLaf);
		s.set("itemColour", Manifest.PAGE_COLOURS[1]);
		s.setControlCallback(onPitchLockSlider);
	}
	
	
	
	inline function pitchLock(obj)
	{
		obj.frequency *= pitchFactor;
		
		obj.bandwidth *= noiseGain;
		
		local ratio = Math.round(obj.frequency / obj.rootFrequency);
		
		local lockedFrequency = ratio * obj.rootFrequency;
		
		obj.frequency = pitchLockAmount * lockedFrequency + (1.0 - pitchLockAmount) * obj.frequency;
		
		
	}
	
	Manifest.pageBroadcaster.addListener(pitchLock, "set function", function(index)
	{
		if(index == 1)
		{
			LorisProcessor.CURRENT_FUNCTION = this;
		}
	});
	
	
}