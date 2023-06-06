namespace Resynthesize
{
	const var ConfigSliders = [Content.getComponent("ConfigSlider1"),
	                           Content.getComponent("ConfigSlider2"),
	                           Content.getComponent("ConfigSlider3"),
	                           Content.getComponent("ConfigSlider4"),
	                           Content.getComponent("ConfigSlider5"),
	                           Content.getComponent("ConfigSlider6")];
	
	inline function onConfigSlider(component, value)
	{
		getOptions();
	}
	
	const var ids = ["windowwidth",
	 "sidelobes",
	 "freqdrift",
	 "ampfloor",
	 "freqfloor",
	 "hoptime",
	 "freqdrift"
	];
	
	reg v = 0.0;
	reg idx = 0;
	
	for(s in ConfigSliders)
	{
		s.setLocalLookAndFeel(Manifest.defaultLaf);
		s.setControlCallback(onConfigSlider);
		
		v = LorisProcessor.lorisManager.get(ids[idx++]);
		Console.print(v);
		//s.setValue(v);
	}
	
	inline function getOptions()
	{
		local obj = {};
		
		obj.window = ConfigSliders[0].getValue();
		obj.sidelobes = -1.0 * ConfigSliders[1].getValue();
		obj.ampfloor = 1.0 * ConfigSliders[2].getValue();
		obj.freqfloor = ConfigSliders[3].getValue();
		//obj.hoptime = ConfigSliders[4].getValue();
		
		for(b in obj)
		{
			//LorisProcessor.lorisManager.set(b, obj[b]);
		}
	}
}