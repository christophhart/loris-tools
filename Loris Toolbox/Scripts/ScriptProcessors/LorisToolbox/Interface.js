Content.makeFrontInterface(900, 500);

/** TODO:
- preview bar
- manifest, page handling
- UI skinning
- root note selector with detection
- lorisManager.createEnvelopePoints(complexData);
- envelope viewer (with lorisManager.getEnvelopePoints("frequency"), lorisManager.getEnvelopeAsBuffer("frequency"))

Pages:
- pitch shifter (fix and with table) and retain original noise
- frequency locker (retain original noise)
- noise remover (with table)
- dilation
- settings page

*/

include("OriginalWatcher.js");
include("MasterPeakMeter.js");

include("UI/RectHelpers.js");
include("UI/DilationRuler.js");
include("UI/BufferPreview.js");
include("UI/ZoomHandler.js");

BufferPreview.make("PreviewPanel");
DilationRuler.make("Ruler");
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
 