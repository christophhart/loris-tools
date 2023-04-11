namespace LorisProcessor
{
	const var ProgressPanel = Content.getComponent("ProgressPanel");
	
	ProgressPanel.setPaintRoutine(function(g)
	{
		if(this.data.alpha != 0.0)
		{
			var p = Content.createPath();
			
			p.startNewSubPath(0.0, 0.0);
			p.startNewSubPath(1.0, 1.0);
			
			p.addArc([0.0, 0.0, 1.0, 1.0], 0, Math.PI * 2.0 * this.data.progress);
			
			g.setColour(Colours.withMultipliedAlpha(0x11FFFFFF, this.data.alpha));
			g.drawEllipse(this.getLocalBounds(19), 3);
			g.setColour(Colours.withMultipliedAlpha(0x88FFFFFF, this.data.alpha));
			g.drawPath(p, this.getLocalBounds(19), 3);
			g.setFontWithSpacing("Lato", 12.0, 0.05);
			g.drawAlignedText(Engine.getPreloadMessage(), Rect.removeFromBottom(this.getLocalBounds(0), 18), "centred");
			g.addDropShadowFromAlpha(Colours.white, 5);
		}
	});
	
	ProgressPanel.setLoadingCallback(function(isPreloading)
	{
		this.data.preloading = isPreloading;
		this.data.progress = 0.0;
		
		if(isPreloading)
		{
			this.data.alpha = 0.0;
			this.data.alphaDelta = 0.1;
			this.startTimer(30);
		}
		else
		{
			this.data.alphaDelta = -0.1;
		}
			
		this.repaint(); 
	});
	
	
	
	ProgressPanel.setTimerCallback(function() 
	{
		var target = Engine.getPreloadProgress();
	
		if(this.data.alphaDelta < 0.0)
			target = 1.0;
			

		this.data.progress = this.data.progress * 0.7 + target * 0.3;
		this.repaint();
		
		this.data.alpha = Math.range(this.data.alpha + this.data.alphaDelta, 0.0, 1.0);
		
		if(this.data.alphaDelta < 0.0)
		{
			if(this.data.alpha == 0.0)
			{
				this.stopTimer();
			}
		}
	});

	const var worker = Engine.createBackgroundTask("Loris Processor");
	
	worker.setTimeOut(5000);
	worker.setForwardStatusToLoadingThread(true);
	
	reg CURRENT_FILE;
	reg CURRENT_FUNCTION;
	reg CURRENT_SETUP;
	reg PENDING = false;
	
	const var RetainNoiseButton = Content.getComponent("RetainNoiseButton");
	const var PreviewDiffButton = Content.getComponent("PreviewDiffButton");
	const var NoiseAmount = Content.getComponent("NoiseAmount");
	
	PreviewDiffButton.setLocalLookAndFeel(Manifest.defaultLaf);
	
	
	
	const var lorisManager = Engine.getLorisManager();
	
	lorisManager.set("timedomain", "0to1");
	lorisManager.set("enablecache", "false");
	
	const var rootNote = Content.getComponent("RootNote");
	
	reg ooo;
	
	function rebuild(thread)
	{
		PENDING = true;

		var rootFreq = Engine.getFrequencyForMidiNoteNumber(rootNote.getValue() - 1);
		var preserveNoise = RetainNoiseButton.getValue();
		var previewDiff = PreviewDiffButton.getValue();

		Console.print("Root Freq: " + rootFreq);

		if(worker.shouldAbort())
		{
			worker.setProgress(0.0);
			worker.setStatusMessage("Cancel");
			PENDING = false;
			return;
		}
		
		worker.setStatusMessage("Analysing...");
		worker.setProgress(0.05);

		var original;
		var noise;

		

		lorisManager.analyse(CURRENT_FILE, rootFreq);
		
		if(preserveNoise || previewDiff)
		{
			original = CURRENT_FILE.loadAsAudioFile();
			
			var isMultichannel = isDefined(original[0].length);
			
			var resyn = lorisManager.synthesise(CURRENT_FILE);
			
			if(isMultichannel)
			{
				for(i = 0; i < original.length; i++)
				{
					original[i] -= resyn[i];
				}
			}
			else 
			{
				original -= resyn;
			}
		}
		
		worker.setStatusMessage("Processing...");
		worker.setProgress(0.33);
		
		if(worker.shouldAbort())
		{
			worker.setProgress(0.0);
			worker.setStatusMessage("Cancel");
			PENDING = false;
			return;
		}
		
		if(Manifest.pageBroadcaster.pageIndex == 3)
		{
			var list = Content.getComponent("Ruler").data.list;
			
			var dilatePoints = [[], []];
			
			for(e in list)
			{
				dilatePoints[1].push(e[0] * DilateScaleSlider.getValue() );	
				dilatePoints[0].push(e[1]);
			}
			
			lorisManager.process(CURRENT_FILE, "dilate", dilatePoints);		
		}
		else if(isDefined(CURRENT_FUNCTION))
		{
			if(isDefined(CURRENT_SETUP))
				CURRENT_SETUP();

			lorisManager.processCustom(CURRENT_FILE, CURRENT_FUNCTION);
		}
		
		worker.setStatusMessage("Resynthesizing...");
		worker.setProgress(0.66);
		
		if(worker.shouldAbort())
		{
			worker.setProgress(0.0);
			worker.setStatusMessage("Cancel");
			PENDING = false;
			return;
		}
		
		var buffer = lorisManager.synthesise(CURRENT_FILE);
		
		ooo = buffer;
		
		worker.setStatusMessage("Done...");
		worker.setProgress(0.9);
		
		if(isDefined(original))
		{
			if(previewDiff)
				buffer = original;
			else
			{
				var isMultichannel = isDefined(original[0].length);
						
				var gain = Engine.getGainFactorForDecibels(NoiseAmount.getValue());
							
				if(isMultichannel)
				{
					for(i = 0; i < original.length; i++)
					{
						original[i] *= gain;

						buffer[i] += original[i];
					}
				}
				else 
				{
					original *= gain;
					buffer[0] += original;
				}	
			}
		}
		
		
		
		BufferPreview.setResynthesisedBuffer(Content.getComponent("PreviewPanel"), buffer, CURRENT_FILE.loadAudioMetadata().SampleRate);
		
		worker.setProgress(1.0);
		PENDING = false;
		
	}
	
	
	
	
	function detectPitch()
	{
		var data = CURRENT_FILE.loadAsAudioFile();
					
		var isMultiChannel = isDefined(data[0].length);
		
		if(isMultiChannel)
			data = data[0];
			
		var pitch = data.detectPitch(CURRENT_FILE.loadAudioMetadata().SampleRate, data.length * 0.2, data.length * 0.6);
		
		if(pitch != 0.0)
		{
			for(i = 0; i < 127; i++)
			{
				var thisPitch = Engine.getFrequencyForMidiNoteNumber(i);
				
				var ratio = Math.abs(1.0 - thisPitch / pitch);
				
				if(ratio < 0.05)
				{
					Console.print("FOUND");
					

					rootNote.setValue(i+1);
					rootNote.sendRepaintMessage();
					return;
				}
			}
		}
	}
	
	OriginalWatcher.broadcaster.addListener(worker, "update", function(unused, unused, file)
	{
		CURRENT_FILE = FileSystem.fromReferenceString(file, FileSystem.AudioFiles);
		
		detectPitch();
		
		if(isDefined(CURRENT_FILE))
		{
			this.callOnBackgroundThread(rebuild);
		}
	});
	
	inline function onDetectPitchButtonControl(component, value)
	{
		if(value && isDefined(CURRENT_FILE))
		{
			detectPitch();
		}
	};
	
	Content.getComponent("DetectPitchButton").setControlCallback(onDetectPitchButtonControl);
	
	inline function onRefresh(component, value)
	{
		if(PENDING && value)
		{
			worker.sendAbortSignal(false);
			return;
		}
		
		if(value)
		{
			OriginalWatcher.broadcaster.resendLastMessage(false);
		}
	}
	
	const var refreshButton = Content.getComponent("RefreshButton");
	
	refreshButton.setControlCallback(onRefresh);
	
}