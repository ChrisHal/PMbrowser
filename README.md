# PMbrowser
Copyright 2020 Christian R. Halaszovich

Licensing information: see file COPYING

## Purpose
This is a tool to browse the contents of PatchMaster(tm) .dat-files. PatchMaster is a trademark of Heka GmbH.
"Big Endian" and "Little Endian" files are supported.
You can display individual traces, review some of the metadata stored in the file and you can export traces as ibw-files that can be read by IgorPro(tm). IgorPro is a trademark of Wavemetrics Inc.

## Usage
### Opening and Browsing
Open a file using either the menu File -> open or drag the file onto the application window.
Use the tree-view to navigate the file. If a "trace" item is selected in the tree, the corresponding trace is displayed.

### Exporting Traces as IBW Files
If you want the export all traces that are children of the selected item (e.g. a series is selected and you want to export all traces belonging to that series), select File->Export IBW File. Id you want to export all traces in the file, select File->Export All as IBW.
In the dialog that will pop up, you can choose a directory to which the exported files will be saved. You can also change the prefix which will preceed the names of the exported files / waves. The wavenames will be of the form  `<prefix>_<group#>_<series#>_<sweep#>_<trace#>`. Where appropiate, `<trace#>` will be replaced by "Vmon", "Imon", or "Leak".

### The Text-Area
In the text-area usefull information gets displayed, e.g. Vhold, Rseries. If the area gets to cluttered, use Edit->Clear Text to clear it.
You can copy the text to the clipboard by selecting the area with a mouse click and using `Ctrl-A` followed by `Ctrl-C` (at least on Windows systems).
