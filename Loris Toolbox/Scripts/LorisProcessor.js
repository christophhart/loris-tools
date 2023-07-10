namespace LorisProcessor
{
	const var ProgressPanel = Content.getComponent("ProgressPanel");
	
	const var Sampler1 = Synth.getSampler("Sampler1");
	
	
	const var SampleMapList = Content.getComponent("SampleMapList");
	
	SampleMapList.set("items", Sampler1.getSampleMapList().join("\n"));
	
	reg outputDirectory = FileSystem.getFolder(FileSystem.Desktop);
	
	inline function onDirectoryButtonControl(component, value)
	{
		if(!value)
			return;

		FileSystem.browseForDirectory(outputDirectory, function(newDir)
		{
			outputDirectory = newDir;
			DirectoryButton.set("text", outputDirectory.toString(0));
		});
	};
	
	const var DirectoryButton = Content.getComponent("DirectoryButton");
	
	DirectoryButton.setControlCallback(onDirectoryButtonControl);
	
	
	for(c in [Content.getComponent("SampleMapList"), Content.getComponent("UseSampleMaps"), Content.getComponent("SampleSelection")])
	{
		c.setLocalLookAndFeel(Manifest.defaultLaf);
	}
	
	const var OriginalWaveform = Content.getComponent("OriginalWaveform");
	
	inline function onUseSampleMapsControl(component, value)
	{
		local l = [[Content.getComponent("ExampleLoad"),
          Content.getComponent("RootNote"),
          Content.getComponent("DetectPitchButton"),
          Content.getComponent("OriginalPitchSlider")],
         [Content.getComponent("SampleMapList"),
          Content.getComponent("SampleSelection")]];

		OriginalWaveform.set("processorId", value ? "Sampler1" : "Original");

		if(!value)
			OriginalWaveform.set("sampleIndex", 0);
		

		for(c in l[0])
			c.set("visible", value == 0);
		for(c in l[1])
			c.set("visible", value == 1);
	};
	
	const var UseSampleMaps = Content.getComponent("UseSampleMaps");
	
	UseSampleMaps.setControlCallback(onUseSampleMapsControl);
	
	reg CURRENT_SAMPLE_LIST = [];
	reg ALL_SAMPLES_MODE = false;
	
	inline function onSampleMapListControl(component, value)
	{
		if(value)
		{
			Sampler1.loadSampleMap(component.getItemText());
			Content.getComponent("SampleSelection").setValue(0);
		}
	};
	
	Content.getComponent("SampleMapList").setControlCallback(onSampleMapListControl);
	
	
	inline function onSampleSelectionControl(component, value)
	{
		if(value == 0)
			return;
		else if(value == 1) // All Samples
		{
			OriginalWaveform.set("sampleIndex", -1);
			ALL_SAMPLES_MODE = true;
		}
		else
		{
			ALL_SAMPLES_MODE = false;
			CURRENT_FILE = FileSystem.fromReferenceString(component.getItemText(), FileSystem.Samples);
			Console.print(CURRENT_FILE.toString(0));
			
			if(CURRENT_SAMPLE_LIST.length)
			{
				OriginalWatcher.CURRENT_NOTE = CURRENT_SAMPLE_LIST[value-2].get(Sampler1.Root);
				
				OriginalWaveform.set("sampleIndex", value - 2);
				OriginalWaveform.sendRepaintMessage();
				
				if(isDefined(CURRENT_FILE))
				{
					//worker.callOnBackgroundThread(rebuild);
				}
			}
		}
	};
	
	Content.getComponent("SampleSelection").setControlCallback(onSampleSelectionControl);
	
	
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
			
			var list = ["All Samples"];
			
			CURRENT_SAMPLE_LIST = Sampler1.createSelection(".*");
			
			for(s in CURRENT_SAMPLE_LIST)
				list.push(s.get(Sampler1.FileName));
				
			var c = Content.getComponent("SampleSelection");
				
			c.set("items", list.join("\n"));
			c.setValue(1);
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
	reg CURRENT_POST;
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
	
	inline function saveBuffer(data, suffix)
	{
		if(!Manifest.SaveButton.getValue())
			return;

		local suffixToUse = suffix;

		if(PitchLock.AutomatePitch.getValue())
		{
			Console.print(rootNote.getValue());

			local thisRoot = rootNote.getValue() + parseInt(PitchLock.PitchLockSliders[1].getValue());

			Console.print(suffixToUse);

			suffixToUse += "_" + Engine.getMidiNoteName(thisRoot);
			
			
		}

		local sr = CURRENT_FILE.loadAudioMetadata().SampleRate;
		local prefix = CURRENT_FILE.toString(CURRENT_FILE.NoExtension);		
		local ext = CURRENT_FILE.toString(CURRENT_FILE.Extension);
		local target = outputDirectory.getChildFile(prefix + suffixToUse + ext);
		
		target.writeAudioFile(data, sr, 24);
		
		
		Console.print("SAVE TO " + target.toString(0));
	}
	
	function rebuildFile(f)
	{
		var rootToUse = UseSampleMaps.getValue() ? OriginalWatcher.CURRENT_NOTE : rootNote.getValue() - 1;
		
		var rootFreq = Engine.getFrequencyForMidiNoteNumber(rootToUse);
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
		var originalTone;

		

		lorisManager.analyse(CURRENT_FILE, rootFreq);
		
		if(preserveNoise || previewDiff)
		{
			original = CURRENT_FILE.loadAsAudioFile();
			
			var isMultichannel = isDefined(original[0].length);
			
			originalTone = lorisManager.synthesise(CURRENT_FILE);
			
			//saveBuffer(originalTone, "_tone");
			
			if(isMultichannel)
			{
				for(i = 0; i < original.length; i++)
				{
					original[i] -= originalTone[i];
				}
			}
			else 
			{
				original -= originalTone[0];
			}
			
			//saveBuffer(original, "_noise");
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
		
		if(isDefined(CURRENT_POST))
		{
			CURRENT_POST();
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
		
		saveBuffer(buffer, Content.getComponent("SuffixLabel").get("text"));
		
		worker.setProgress(1.0);
	}
	
	function rebuild(thread)
	{
		PENDING = true;

		if(ALL_SAMPLES_MODE)
		{
			Console.print("ALL SAMPLES");
			

			for(s in CURRENT_SAMPLE_LIST)
			{
				OriginalWatcher.CURRENT_NOTE = s.get(Sampler1.Root);
				CURRENT_FILE = FileSystem.fromReferenceString(s.get(Sampler1.FileName), FileSystem.Samples);

				Console.print("Rebuild " + CURRENT_FILE.toString(0) + " with root " + OriginalWatcher.CURRENT_NOTE);
				rebuildFile(CURRENT_FILE);
			}
		}
		else
		{
			if(PitchLock.AutomatePitch.getValue())
			{
				for(i = -12; i <= 12; i++)
				{
					PitchLock.PitchLockSliders[1].setValue(i);
					PitchLock.PitchLockSliders[1].changed();
					rebuildFile(CURRENT_FILE);
				}
			}
			else
			{
				rebuildFile(CURRENT_FILE);	
			}
		}

		PENDING = false;		
	}
	
	function detectPitch()
	{
		if(isDefined(CURRENT_FILE) && CURRENT_FILE.isFile())
		{
			var data = CURRENT_FILE.loadAsAudioFile();
								
			var isMultiChannel = isDefined(data[0].length);
			
			if(isMultiChannel)
				data = data[0];
			
			Console.print(data[0]);
			
				
			var pitch = data.detectPitch(CURRENT_FILE.loadAudioMetadata().SampleRate, data.length * 0.2, data.length * 0.6);
			
			Console.print(pitch);
			
			if(pitch != 0.0)
			{
				for(i = 0; i < 127; i++)
				{
					var thisPitch = Engine.getFrequencyForMidiNoteNumber(i);
					
					var ratio = Math.abs(1.0 - thisPitch / pitch);
					
					if(ratio < 0.05)
					{
						Console.print("FOUND");
						Console.print(thisPitch);
	
						rootNote.setValue(i+1);
						rootNote.sendRepaintMessage();
						return;
					}
				}
			}
		}
	}
	
	OriginalWatcher.broadcaster.addListener(worker, "update", function(unused, unused, file)
	{
		if(UseSampleMaps.getValue())
			return;

		CURRENT_FILE = FileSystem.fromReferenceString(file, FileSystem.AudioFiles);
		
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
			worker.callOnBackgroundThread(rebuild);
		}
	}
	
	const var refreshButton = Content.getComponent("RefreshButton");
	
	refreshButton.setControlCallback(onRefresh);
	
}