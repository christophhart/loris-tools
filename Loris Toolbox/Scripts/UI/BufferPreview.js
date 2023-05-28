namespace BufferPreview
{
inline function setBuffer(unused, unused, file)
{
	/*

	local sf = FileSystem.fromReferenceString(file, FileSystem.AudioFiles);
				
	local newData = [];
	
	if(isDefined(sf))
		newData = sf.loadAsAudioFile();
	
	

	local isMultiChannel = isDefined(newData[0].length);
	
	if(!isMultiChannel)
	{
		this.data.channels = [newData];
	}
	else
	{
		this.data.channels = newData;
	}
	
	rebuild(this);
	*/
}


inline function setEnvelope(panel, env)
{
	panel.data.envelope = env;
	panel.repaint();
}

inline function setResynthesisedBuffer(panel, bufferData, samplerate)
{
	local isMultiChannel = isDefined(bufferData[0].length);
		
	if(!isMultiChannel)
	{
		panel.data.channels = [bufferData];
	}
	else
	{
		panel.data.channels = bufferData;
	}
	
	panel.data.samplerate = samplerate;
	
	rebuild(panel);
}

inline function rebuild(panel)
{
	panel.data.paths = [];
	
	local w = 0.0;
	
	for(c in panel.data.channels)
	{
		local np = Content.createPath();
		np.startNewSubPath(0.0, 1.0);
		np.startNewSubPath(0.0, -1.0);
		np.startNewSubPath(0.0, 0.0);
		
		local samplesPerPixel = c.length / panel.get("width") * 0.8;
		
		
		
		local points = [];
		points.reserve(panel.get("width"));
		
		local useHighRes = samplesPerPixel < 5;
		
		
		
		for(i = 0.0; i < c.length; i += samplesPerPixel)
		{
			local v;
			
			if(useHighRes)
				v = -1.0 * c[parseInt(i)];
			else
			 	v = c.getMagnitude(i, Math.min(c.length -i, samplesPerPixel * 1.5));
			 	
			points.push([i, v]);
		}
		
		for(p in points)
		{
			np.lineTo(p[0], p[1]);
		}
		
		
		
		
		np.lineTo(c.length, 0.0);
		
		if(!useHighRes)
		{
			for(i = points.length-1; i >= 0; i--)
				np.lineTo(points[i][0], -1.0 * points[i][1]);
		}
		
		np.closeSubPath();
		
		panel.data.paths.push(np);
	}
	
	panel.repaint();
}



inline function make(name)
{
	const var p = Content.getComponent("PreviewPanel");
		
	p.data.paths = [];
	p.data.channels = [];
	p.data.samplerate = 44100.0;
		
	p.set("allowCallbacks", "Clicks & Hover");
		
	p.setMouseCallback(function(event)
	{
		this.data.hover = event.hover;

		if(event.clicked)
		{
			Engine.playBuffer(this.data.channels, function[this](isPlaying, pos)
			{
				this.data.previewPos = pos;
				this.data.isPlaying = isPlaying;
				
				this.repaint();
			}, this.data.samplerate);
		}
		if(event.mouseUp)
		{
			Engine.playBuffer("", "", 0.0);
			
		}
		
		this.repaint();
	});
		
	p.setPaintRoutine(function(g)
	{
		var b = this.getLocalBounds(0);
		
		g.setColour(0x05FFFFFF);
		g.fillRect(b);
		g.setColour(0x15FFFFFF);
		g.drawRect(b, 1);
		
		
		for(p in this.data.paths)
		{
			var pb = Rect.removeFromTop(b, this.get("height") / this.data.paths.length);
			
			g.setColour(Colours.withAlpha(this.get("itemColour"), 0.1));
			
			g.fillPath(p, pb);
			g.setColour(0x5aFFFFFF);
			g.drawPath(p, pb, 1.0);
		}
		
		if(this.data.isPlaying)
		{
			var h = this.get("height");

			g.setColour(0x44FFFFFF);
			
			var x = this.data.previewPos * this.get("width");
			
			g.fillRect([x-2, 0, 4, h]);
			
			g.setColour(0x99FFFFFF);
			g.drawVerticalLine(x, 0.0, h);
			
			
			
		}
		
		if(this.data.envelope)
		{
			g.setColour(this.get("itemColour"));
			g.drawPath(this.data.envelope, this.getLocalBounds(0), 2.0);
			
		}
	});
	
	OriginalWatcher.registerAtBroadcaster(p, "update preview", setBuffer);
	
	Manifest.pageBroadcaster.addComponentPropertyListener(p, "itemColour", "setItemColour", function(index, value)
	{
		return Manifest.PAGE_COLOURS[value];
	});
	
	Manifest.pageBroadcaster.addComponentRefreshListener(p, "repaint", "repaint");
	
	return p;
}
	
}
