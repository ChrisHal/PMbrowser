.. _igor-export-infos-label:

**********************************************
Notes on Exporting Data for :program:`IgorPro`
**********************************************

The files created by :program:`PMbrowser` are compatible with :program:`IgorPro`
versions 5 and later.

When exporting traces as Igor binary waves as either single :file:`.ibw` files or as part of 
an experiment :file:`.pxp` file, metadata will be exported as a wave-note. Which metadata will
be exported is chosen via the :ref:`select-params-dlg-label` by selecting the 'export' checkbox for
the items you want to include.

When exporting several traces into a single :file:`.pxp` file, you have the option to have a
datafolder structure created which replicates the tree structure (group, series) found in
the :file:`.dat` file.
Additionally, a macro will be included to create graphs for the individual series. This macro
is accessible from the *Macro* menue (entry *Display Waves*) within :program:`IgorPro`.

You may want to restrict the traces you export using the :ref:`filter-dlg-label`
before invoking :ref:`Export -> all visible traces <export-all-visible-traces-label>`
from the *File* menue.
