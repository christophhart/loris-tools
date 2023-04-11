Content.makeFrontInterface(900, 600);

/** TODO:
- preview bar ok
- manifest, page handling ok
- UI skinning
- root note selector with detection
- lorisManager.createEnvelopePoints(complexData);
- envelope viewer (with lorisManager.getEnvelope("frequency"))

Pages:
- pitch shifter (fix and with table)
- frequency locker
- noise remover (with table)
- dilation
- settings page

*/

include("OriginalWatcher.js");
include("MasterPeakMeter.js");

include("UI/Paths.js");

include("Manifest.js");




include("UI/RectHelpers.js");
include("UI/DilationRuler.js");
include("UI/BufferPreview.js");
include("UI/ZoomHandler.js");

BufferPreview.make("PreviewPanel");
DilationRuler.make("Ruler");

include("LorisProcessor.js");

include("PitchLock.js");

include("Config.js");


namespace HarmonicScaler
{
	const var Harmonics = Content.getComponent("Harmonics");

	const var randHarm = [];
	
	
	const var randAmount = 0.7;
	
	inline function setup()
	{
		randHarm.clear();
		
		for(i = 0; i < Harmonics.getNumSliders(); i++)
		{
			randHarm.push((1.0 - randAmount/2) + randAmount * Math.random());
		}
	}

	inline function scale(obj)
	{
		local idx = parseInt(Math.round(obj.frequency / obj.rootFrequency)-1);
		
		local harmonicGain = Harmonics.getSliderValueAt(idx);
		
		if(idx >= 0 && idx < Harmonics.getNumSliders())
		{
			harmonicGain *= randHarm[idx];
			obj.gain *= harmonicGain;
		}
	}
	
	Manifest.pageBroadcaster.addListener(scale, "set scale", function(index)
	{
		if(index == 2)
		{
			LorisProcessor.CURRENT_SETUP = setup;

			LorisProcessor.CURRENT_FUNCTION = this;
		}
	});
}

function onNoteOn()
{
	
}
 function onNoteOff()
{
	
}
 function onController()
{
	
}
 function onTimer()
{
	
}
 function onControl(number, value)
{
	
}
 