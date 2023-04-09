namespace OriginalWatcher
{
const var broadcaster = Engine.createBroadcaster({
	"id": "originalLoader",
	"args": ["processor", "index", "value"]
});

broadcaster.attachToComplexData("AudioFile.Content", "Original", 0, "original file");

const var clickpreview = Engine.createBroadcaster({
	"id": "original_preview",
	"args": ["component", "event"]
});

clickpreview.attachToComponentMouseEvents("OriginalWaveform", "Clicks Only", "preview on click");

reg lastId = -1;

clickpreview.addListener("play note", "play note", function(component, event)
{
	if(event.clicked)
		lastId = Synth.playNote(60, 127);
	else
		Synth.noteOffByEventId(lastId);
});

inline function registerAtBroadcaster(p, title, callback)
{
	broadcaster.addListener(p, title, callback);
}

}