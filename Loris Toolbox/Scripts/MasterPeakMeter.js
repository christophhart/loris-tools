const var MasterPeak = Content.addFloatingTile("MasterPeak", 870, 30);

MasterPeak.setContentData({
  "Type": "MatrixPeakMeter",
  "ProcessorId": "toolbox_project",
  "SegmentLedSize": 2.0,
  "UpDecayTime": 0.0,
  "DownDecayTime": 1100.0,
  "UseSourceChannels": true,
  "SkewFactor": 0.35,
  "PaddingSize": 1.0,
  "ShowMaxPeak": 1.0,
  "ChannelIndexes": [
    0,
    1
  ]
});

Content.setPropertiesFromJSON("MasterPeak", {
    "x": 870.0,
    "y": 30.0,
    "width": 20.0,
    "height": 412.0,
    "bgColour": 0xFF333333,
    "itemColour": 0xFF888888,
    "itemColour2": 0xFF404040,
});