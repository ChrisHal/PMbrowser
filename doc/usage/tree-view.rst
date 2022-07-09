.. _tree-view-label:

Tree-View
#########

Here, the the pulse tree stored in the :file:`dat` file is displayed.

When you click on any item, the parameters marked *print* in the :ref:`select-params-dlg-label`
of the selected item are printed to the **text area**.

If the selected item is a trace, it will be displayed in the :ref:`grapharea-label`.

Double-clicking on an item while display all traces that are (direct or indirect) children of 
this item consecutively the :ref:`grapharea-label`.
(On macOS, because of technical limitations of the platform, all traces will be displayed at
once.)
It is recommended to use this function in conjunction with the *overlay* feature of the
:ref:`Graph Area <overlay-feature-label>`.
If *overlay* is disabled, only the last trace will persist in the display.

context menu
============

Right-click (macOS: option click) on an item to open the context menu.

export subtree
--------------

Export all currently displayed (i.e. not :index:`hidden`) child traced to igor.
Opens the dialog :ref:`export-dlg-label`.

hide subtree
------------

:index:`Hide <hide>` selected item and all its children.

show all children
-----------------

:index:`Un-hide <un-hide>` all children of the selected item.

print all parameters
--------------------

Print all parameters, regardless whether they are marked *print* or not,
of the selected item *and all its parents* to the **text area**.

set as time reference
---------------------

Set the time of the selected item as the reference for the calculation of
:ref:`relative time <relative-time-info-label>` parameters.

amplifier state
---------------

This function is only available for *series* items.

Prints the *amplifier state* recorded for the *series* to the **text area**.

draw stimuli
------------

This function is only available for *series* items.

Draws the calculated stimuli for all child sweeps in the **display area**.
It is recommended to use this function in conjunction with the *overlay* feature of the
:ref:`Graph Area <overlay-feature-label>`.
If *overlay* is disabled, only the last stimulus will persist in the **display area**.

show stimulus
-------------

This function is only available for *sweep* items.

Draws the :index:`stimulus` for the selected *sweep* in the **display area**.

**Note:** Currently only *const*, *ramp*, and *continous* stimulus segments are supported. Also,
certain time and voltage increment modes are :index:`not supported <unsupported>`.


use stim. as x trace
--------------------

This function is only available for *sweep* items.

Sets the display *YX mode*, using the stimulus of the selected *sweep* as X-trace.

Use :ref:`YT-mode` from *Graph* menu to return to normal display mode.


buttons
=======

Filter...
---------

Opens :ref:`filter-dlg-label`.

undo filter
-----------

Undo filter by un-hiding all items.

