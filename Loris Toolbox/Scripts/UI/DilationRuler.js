/** Loris Example Toolbox - Dilation Ruler
	==================================================================================

	Licensed under the GPLv3 License.
	
	Author: Christoph Hart

	A UI element that allows to drag data points across the x-axis in order to create
	a distorted timeline for the *dilate* algorithm.
	
	The data used by the panel can directly be forwarded to the loris function as long 
	as you make sure that you're using the *0to1* timedomain property.
	
	==================================================================================
*/
namespace DilationRuler
{
const var NUM_STEPS = 200;

inline function rebuildPath(panel)
{
	panel.data.path.clear();
	
	local isFirst = true;
	
	for(d in panel.data.list)
	{
		if(isFirst)
		{
			panel.data.path.startNewSubPath(d[0], 1.0 - d[1]);
			isFirst = false;
		}
		else
		{
			panel.data.path.lineTo(d[0], 1.0 - d[1]);
		}
	}
	
	panel.repaint();
}

inline function reset(p)
{
	p.data.currentDragger = undefined;
	p.data.list = [[0.0, 0.0], [1.0, 1.0]];
	rebuildPath(p);
}

inline function make(panelName)
{
	local p = Content.getComponent(panelName);
	p.set("allowCallbacks", "All Callbacks");
	p.set("height", 200);
	
	p.data.path = Content.createPath();
	
	p.setPaintRoutine(function(g)
	{
		var xpos = 0.0;
		
		var h = 40;
		
		
		var stepSize = this.get("width") / NUM_STEPS;
		
		var gridIndex = 0;
		
		for(i = 0; i < this.data.list.length -1; i++)
		{
			var xdelta = this.data.list[i+1][0] - this.data.list[i][0];
			var ydelta = this.data.list[i+1][1] - this.data.list[i][1];
			var ratio = xdelta / ydelta;
			var numThisTime = ydelta * NUM_STEPS;
			var thisStepSize = stepSize * ratio;
			var isDragger = this.data.list[i] == this.data.currentDragger;
			var thisxpos = xpos;
			
			g.setColour(0x22FFFFFF);
			
			for(j = 0; j < numThisTime; j++)
			{
				var isTen = gridIndex % 20 == 0;
	
				var alpha = 0.2;
				if(isTen)
					alpha += 0.2;
				if(this.data.hover)
					alpha += 0.08;
	
				g.setColour(Colours.withAlpha(Colours.white, alpha));
	
				
				
				if(isTen)
					g.drawAlignedText(parseInt(gridIndex/20), [Math.range(xpos - 20, 0, this.get("width")-30), 0, 40, 15], gridIndex == 0 ? "left" : "centred");
	
				var isFive = gridIndex % 10 == 0;
	
				g.drawVerticalLine(xpos, isTen ? 17 : (isFive ? 20 :24), h - 4);
	
				xpos += thisStepSize;
				gridIndex++;
			}
			
			if(thisxpos != 0.0)
			{
				g.setColour(Colours.withMultipliedBrightness(0xFF521433, isDragger ? 1.8 : 1.5));
				g.fillRoundedRectangle([thisxpos-2, 13, 4, h-13], 2.0);
				
				if(isDragger)
				{
					g.setColour(0x44FFFFFF);;
					

					g.drawVerticalLine(thisxpos, 40.0, this.get("height"));
				}
			}
		}
		
		if(isDefined(this.data.currentDragger))
		{
			var pa = this.getLocalBounds(0);
			pa[1] += 40;
			pa[3] -= 40;
			
			g.setColour(Colours.withMultipliedBrightness(0xFF521433, isDragger ? 1.8 : 1.5));
			g.drawPath(this.data.path, pa, 2.0);
		}
	});
	
	p.setMouseCallback(function(event)
	{
		this.data.hover = event.hover;

		var normx = (event.x-2) / this.get("width");
	
		if(event.doubleClick)
		{
			reset(this);
			return;
		}
	
		if(event.clicked)
		{
			this.data.currentDragger = undefined;
	
			for(d in this.data.list)
			{
				if(this.data.list.indexOf(d) == 0 ||
				   this.data.list.indexOf(d) == this.data.list.length - 1)
			   {
				   continue;
			   }
	
				if(Math.abs(d[0] - normx) < 0.02)
				{
					this.data.currentDragger = d;
					break;
				}
			}
			
			if(!isDefined(this.data.currentDragger) &&
			   !event.rightClick)
			{
				this.data.currentDragger = [normx, normx];
	
				for(i = 0; i < this.data.list.length; i++)
				{
					if(this.data.list[i][0] > normx)
					{
						this.data.list.insert(i, this.data.currentDragger);
						break;
					}
				}
			}
		}
		
		if(event.drag)
		{
			if(isDefined(this.data.currentDragger))
			{
				var dragIndex = this.data.list.indexOf(this.data.currentDragger);
				
				

				this.data.currentDragger[0] = Math.range(normx, this.data.list[dragIndex-1][0] + 0.01, this.data.list[dragIndex+1][0]-0.01);
			}
				
			
			rebuildPath(this);
		}
		
		if(event.mouseUp)
		{
			if(event.rightClick && 
			   Math.abs(event.mouseDownX - event.x) < 10 &&
			   isDefined(this.data.currentDragger))
			{
				this.data.list.remove(this.data.currentDragger);
			}
			
			this.data.currentDragger = undefined;
		}
		
		this.repaint();
	});
	
	reset(p);
	
	return p;
}

}