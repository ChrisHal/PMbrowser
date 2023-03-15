This folder ("demo") contains some python scripts that illustrate how npy files and the metadata exported
into json files can be used.

*You must keep the json files alongside their corresponding npy files to make sure the
metadata can be found and processed!*

**prerequisits**
- module *numpy* is installed
- module *matplotlib* is installed

**included demos**

*plot_trace*

The npy file given in the command line will be graphed.
This is the most basic demo.

Example: ``plot_trace.py PM_1_1_1_Imon.npy``

*multiplot_traces*

All npy files given in the command line will be displayed in a single graph.

Example: ``multiplot_trace.py PM_1_1_1_Imon.npy PM_1_1_2_Imon.npy``

*plot_traces*

All npy files the match the patterns given in the command line will be displayed in a single graph.

Example: ``plot_traces.py PM_1_1_*_Imon.npy``

*calc_mean*

The mean of a point range is calculated.

Example: ``calc_mean.py 0 100 PM_1_1_1_Imon.npy``
