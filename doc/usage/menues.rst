Usage
#####

The Menues
==========

"File"
******

'Open'
------

Select a :program:`PATCHMASTER` or :program:`PATCHMASTER NEXT` :file:`.dat` file. (Alternatively,
you can open a file by dragging it onto the :program:`PMbrowser` window. Non-bundled files, which consist
of a collection of various files, usually :file:`.pul` and :file:`.pgf` and possibly others
are also present.)

'Close'
-------

Close currently open file.

'Export Subtree for Igor'
-------------------------

All traces, that are children of the node currently selected in the **tree-view** are selected for export
as either individual :file:`.ibw`-files or as one packaged experiment file (extension :file:`.pxp`)
which can be read by
:program:`IgorPro`.

The export dialog will appear, which allows you to select paths and filenames as well as several options
that pertain to exports to packed experiment files (see :ref:`export-dlg-label` and
:ref:`igor-export-infos-label` for additional information on exports).

This function also available from the context menue of the **tree-view**.


'Export All Visible Traces for Igor'
------------------------------------

Similar to `'Export Subtree for Igor'`_, but all traces that are currently visible (i.e. not hidden)
in the **tree-view** are selected for export. Traces can be hidden either by *filtering* (see
:ref:`filter-menue-label` and :ref:`filter-dlg-label`) or by using the context menue of the **tree-view**.

'Export All Traces as IBW Files'
--------------------------------

Similar to `'Export All Visible Traces for Igor'`_, but all (hidden and visible) traces will be
exported as individual :file:`.ibw`-files. Export to a packed experiment file is not support by
this option.

'Select Parameters'
-------------------

Opens the :ref:`select-params-dlg-label`. There you can select which parameters from the PatchMaster-file
will be either *printed* to the **text-area** or *exported* in *wave-notes*.


"Edit"
******

'Clear Text'
------------

Clears the **text-area**.

"Tree"
******

.. _filter-menue-label:

'Filter'
--------

Allows you to hide unwanted nodes and traces from the **tree-view**, see :ref:`filter-dlg-label`.

'Show All'
----------

Un-hides all nodes and traces in the **tree-view**.

'Print All Parameters'
----------------------

Print all available parameters - including those not selected for printing - from the currently
selected **tree-view** item (trace or node) and its parents to the **text-area**.

(Also available from the context menue of the **tree-view**.)

"Graph"
*******

'Auto Scale'
------------

Set scaling of graph axes to extents of most recently loaded trace.

'Do Autoscale on Load'
----------------------

Toggle to enable or disable automatic scaling when a new trace is loaded
into the graph. Also avaible from the context menue of the **graph area**.

'YX mode (cur. trace as X)'
---------------------------

Enter *YX mode*. The currently selected trace will be used a *X* reference for 
subsequent drawing operations. This is especially usefull if you are 
working with ramp stimuli.

'YT mode'
---------

Enter *YT mode*, which is the default mode. *Time* will be used as the abscissa for the graph.

'Wipe All'
----------

Clear the **graph area**.

'Wipe Background Traces'
------------------------

Delete all displayed traces from the internal buffer of the graph. Only the current trace
will remain visible in the **graph area**.

'Settings and Ranges'
---------------------

Opens dialog which allows altering certain settings of the **graph area** - see :ref:`ranges-dlg-label`.


"Help"
******

'About'
-------

Displays 'About'-Dialog with various bits of information, including 
version of Qt-library against which the executable was build.

'About Qt'
----------

Displays dialog with information on Qt-library, including version of 
library actually used at runtime.


The Dialogs
===========

.. _export-dlg-label:

Igor Export: 'Choose Path & Prefix' Dialog
******************************************

.. _filter-dlg-label:

'Tree-Filter' Dialog
********************

.. _select-params-dlg-label:

'Select Parameters' Dialog
**************************


.. _ranges-dlg-label:

'Setting and Ranges' Dialog
***************************

