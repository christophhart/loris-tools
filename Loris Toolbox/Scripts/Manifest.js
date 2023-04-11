

namespace Manifest
{
	const var PAGE_COLOURS = [
		0xFFb8782b,
		0xFF0389ff,
		0xFF812ab8,
		0xFF508473
	]

	const var PAGE_BUTTONS = [
	{
		"text": "RESYNTHESIZE",
		"itemColour": PAGE_COLOURS[0],
		"tooltip": "Test the resynthesize algorithm with different parameters.",
		"width": 130
	},
	{
		"text": "PITCHLOCK",
		"itemColour": PAGE_COLOURS[1],
		"tooltip": "Flatten the pitch of the sample to the given root note.",
		"width": 110
	},
	{
		"text": "SPECTRAL",
		"itemColour": PAGE_COLOURS[2],
		"tooltip": "Change the harmonic spectrum of the sample.",
		"width": 125
	},
	{
		"text": "DILATE",
		"itemColour": PAGE_COLOURS[3],
		"tooltip": "Distort the timeline of the sample.",
		"width": 85
	}];
	
	const var INTERFACE_WIDTH = 900;
	const var INTERFACE_HEIGHT = 500;
	const var TOP_ROW_HEIGHT = 40;
	const var BOTTOM_ROW_HEIGHT = 28;
	
	const var defaultLaf = Content.createLocalLookAndFeel();
	
	defaultLaf.registerFunction("drawToggleButton", function(g, obj)
	{
		g.setColour(0x22000000);
		g.fillRect(obj.area);
		
		var alpha = 0.3;
		if(obj.over)
			alpha += 0.1;
			
		if(obj.value)
			alpha += 0.4;
		
		g.setColour(Colours.withAlpha(0xFFFFFFFF, alpha));
		
		g.setFontWithSpacing(obj.value ? "Lato Bold" : "Lato", 12.0, 0.03);
		g.drawAlignedText(obj.text.toUpperCase(), obj.area, "centred");
	});
	
	defaultLaf.registerFunction("drawComboBox", function(g, obj)
	{
		g.setColour(0x22000000);
		g.fillRect(obj.area);
		
		var alpha = 0.5;
		if(obj.hover)
			alpha += 0.1;
			
		
		
		g.setColour(Colours.withAlpha(0xFFFFFFFF, alpha));
		
		g.setFontWithSpacing("Lato Bold", 12.0, 0.03);
		
		Rect.removeFromLeft(obj.area, 10);
		
		g.drawAlignedText(obj.text.toUpperCase(), obj.area, "left");
		
		var t = Rect.removeFromRight(obj.area, obj.area[3]);
		g.fillTriangle(Rect.withSizeKeepingCentre(t, 8, 6), Math.PI);
		
	});
	
	
	
	defaultLaf.registerFunction("drawLinearSlider", function(g, obj)
	{
		var top = Rect.removeFromTop(obj.area, 5);
		
		g.setColour(0x22000000);
		
		g.fillRoundedRectangle(top, top[3]/2);
		
		var alpha = 0.7;
		
		if(obj.hover)
			alpha += 0.1;
		if(obj.clicked)
			alpha += 0.2;		
				
				
		g.setColour(Colours.withAlpha(obj.itemColour1, alpha));
		
		var value = Rect.removeFromLeft(top, Math.max(top[3], obj.valueNormalized * top[2]));
		
		g.fillRoundedRectangle(value, value[3]/2);
		
		g.setColour(Colours.withAlpha(Colours.white, alpha * 0.8));
		g.setFontWithSpacing(obj.clicked ? "Lato Bold" : "Lato", 12.0, 0.03);
		
		var text = obj.text.toUpperCase();
		
		if(obj.clicked)
			text = obj.valueAsText;
		
		g.drawAlignedText(text, obj.area, "centred");
	});
	
	const var iconLaf = Content.createLocalLookAndFeel();
	
	iconLaf.registerFunction("drawToggleButton", function(g, obj)
	{
		var alpha = 0.3;
		if(obj.over)
			alpha += 0.1;
			
		var margin = 1;
			
		if(obj.down)
		{
			alpha += 0.2;
			margin = 2;
		}
			
		
		
		

		g.setColour(Colours.withAlpha(Colours.white, alpha));
		g.fillPath(Paths.icons[obj.text], Rect.reduced(obj.area, margin));
		
	});
	
	Content.getComponent("SettingsButton").setLocalLookAndFeel(iconLaf);
	
	
	const var SaveButton = Content.getComponent("SaveButton");
	SaveButton.setLocalLookAndFeel(iconLaf);
	
	
	Content.getComponent("NoiseAmount").setLocalLookAndFeel(defaultLaf);
	
	
	const var rootNote = Content.getComponent("RootNote");
	
	rootNote.set("items", "");
	
	for(i = 0; i < 127; i++)
		rootNote.addItem(Engine.getMidiNoteName(i));
		
	rootNote.setLocalLookAndFeel(defaultLaf);
	
	const var exampleLoad = Content.getComponent("ExampleLoad");
	
	Content.getComponent("RetainNoiseButton").setLocalLookAndFeel(defaultLaf);
	Content.getComponent("DetectPitchButton").setLocalLookAndFeel(defaultLaf);
	exampleLoad.setLocalLookAndFeel(defaultLaf);
	
	exampleLoad.set("items", "Custom");
	
	for(d in Engine.loadAudioFilesIntoPool())
		exampleLoad.addItem(d.split("/")[1]);
		
	const var Original = Synth.getAudioSampleProcessor("Original");
		
	inline function onExampleLoad(component, value)
	{
		if(value > 1)
		{
			local fileToLoad = "{PROJECT_FOLDER}sound_examples/" + component.getItemText();
			Original.setFile(fileToLoad);
		}
	}
		
	exampleLoad.setControlCallback(onExampleLoad);
	
	
	
	
	const var pageButtonLaf = Content.createLocalLookAndFeel();
	
	pageButtonLaf.registerFunction("drawToggleButton", function(g, obj)
	{
		g.setColour(0x22000000);
		g.fillRoundedRectangle(obj.area, obj.area[3]/2);
		g.setColour(obj.textColour);
		
		var circle = Rect.removeFromLeft(obj.area, obj.area[3]);
		
		var alpha = 0.3;
		if(obj.over)
			alpha += 0.1;
			
		if(obj.value)
			alpha += 0.4;
		
		g.setColour(Colours.withAlpha(obj.itemColour1, alpha));
		
		g.fillEllipse(Rect.reduced(circle, 9));
		
		
		
		g.setColour(Colours.withAlpha(Colours.white, alpha));
		
		
		g.setFontWithSpacing(obj.value ? "Lato Bold" : "Lato", 12.0, 0.05);
		g.drawAlignedText(obj.text, obj.area, "left");
	});
	
	
	
	const var pageBroadcaster = Engine.createBroadcaster({
		"id": "pageBroadcaster",
		"args": ["pageIndex"],
		"tags": ["page-handling"]
	});
	
	
	
	
	
	inline function initPageButtons()
	{
		local pageIndex = 1;
		local pageButtons = [];
		local x = 5;
		
		for(d in PAGE_BUTTONS)
		{
			local id = "PageButton"+pageIndex++;

			local b = Content.addButton(id, x, 5);
			
			b.set("radioGroup", 9000);
			b.set("parentComponent", "BG");
			b.set("saveInPreset", false);
			
			b.setLocalLookAndFeel(pageButtonLaf);
			
			Content.setPropertiesFromJSON(id, d);
			
			x += d.width + 5;
			pageButtons.push(b);
		}
		
		pageBroadcaster.attachToRadioGroup(9000, "page group");
	}
	
	const var Pages = ["ResynthesisePage", "PitchLockPage",
	   				   "LoopBuilderPage", "DilatePage"];
	
	pageBroadcaster.addComponentPropertyListener(Pages, "visible", "set visible", function(index, value)
	{
		return index == value;
	});
	
	initPageButtons();
	
	pageBroadcaster.pageIndex = 0;
}


	const var BG = Content.getComponent("BG");
	
	
	
	BG.setPaintRoutine(function(g)
	{
		g.setGradientFill([0xFF333333, 0.0, 0.0,
						  0xFF303030, 0.0, 500, false]);
					
		var c1 = Colours.mix(Manifest.PAGE_COLOURS[Manifest.pageBroadcaster.pageIndex], 0xFF303030, 0.96);
		var c2 = 0xFF282828;
		
		g.setGradientFill([c1, this.get("width")/2, this.get("height")/2,
						   c2, 0.0, 0, true]);
					
		var b = this.getLocalBounds(0);
						  
		g.fillRect(b);
		
		g.setColour(0xFF282828);
		
		var topRow = Rect.removeFromTop(b, Manifest.TOP_ROW_HEIGHT);
		
		g.fillRect(topRow);
		
		g.setColour(0x11FFFFFF);
		g.setFontWithSpacing("Lato Bold", 24.0, 0.05);
		Rect.removeFromRight(topRow, 40);
		
		g.drawAlignedText("LORIS TOOLBOX", topRow, "right");
		
		
		g.setColour(0x44000000);
		g.fillRect(Rect.removeFromTop(b, 1));
		
		g.setColour(0xFF282828);
		g.fillRect(Rect.removeFromBottom(b, Manifest.BOTTOM_ROW_HEIGHT));
		g.setColour(0x44000000);
		g.fillRect(Rect.removeFromBottom(b, 1));
		
		
		
		g.setColour(0x22FFFFFF);
		g.setFontWithSpacing("Lato", 12.0, 0.03);
		g.drawAlignedText("Resynthesized Version", [740, 225, 120, 20], "right");
		g.drawAlignedText("Original Sample", [740, 510, 120, 20], "right");
	});
	
	const var refreshLaf = Content.createLocalLookAndFeel();
		
		refreshLaf.registerFunction("drawToggleButton", function(g, obj)
		{
			var margin = obj.value ? 11 : 10;

			var p = Paths.icons[obj.text];

			g.drawDropShadowFromPath(p, Rect.reduced(obj.area, margin), Colours.black, 10, [0, 2]);

			

			var c = Colours.mix(obj.textColour, Colours.white, obj.over ? 0.2 : 0.0);

			g.setGradientFill([Colours.withMultipliedBrightness(c, 1.5), 0.0, 0.0,
							   c, 0.0, obj.area[3], false]);
			g.fillPath(p, Rect.reduced(obj.area, margin-5));
			
			g.setColour(0x55ffffff);
			g.drawPath(p, Rect.reduced(obj.area, margin-5), 1);
			
		});
		
		const var refreshButton = Content.getComponent("RefreshButton");
		
		Manifest.pageBroadcaster.addComponentPropertyListener(refreshButton, "textColour", "update refresh colour", function(index, value)
		{
			return Manifest.PAGE_COLOURS[value];
		});
		
		Manifest.pageBroadcaster.addComponentRefreshListener(refreshButton, "repaint", "repaint refresh button");
		Manifest.pageBroadcaster.addComponentRefreshListener(BG, "repaint", "repaint refresh button");
		
		
		refreshButton.setLocalLookAndFeel(refreshLaf);
	
	
	const var SettingsPopup = Content.getComponent("SettingsPopup");
	
	inline function onSettingsButtonControl(component, value)
	{
		SettingsPopup.showControl(value);
	};
	
	
	Content.getComponent("SettingsButton").setControlCallback(onSettingsButtonControl);
	
	
	Content.getComponent("SettingsTile").setLocalLookAndFeel(Manifest.defaultLaf);
	
	Content.getComponent("OriginalPitchSlider").setLocalLookAndFeel(Manifest.defaultLaf);
	
	const var Constant1 = Synth.getModulator("Constant1");
	
	inline function onOriginalPitchSliderControl(component, value)
	{
		Constant1.setIntensity(value);
		
	};
	
	Content.getComponent("OriginalPitchSlider").setControlCallback(onOriginalPitchSliderControl);
	