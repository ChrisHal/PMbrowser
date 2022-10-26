###############################
Installing :program:`PMbrowser`
###############################

You can install from source or use one of the provided binaries. Currently,
for Linux, an executable is available as well as a DEB package.
For Windows (64bit only) and macOS (minimum version 10.14) installers are available.

Alternatively, you can build from source.


Installing on Windows
=====================

Download the installer :file:`PMbrowser-<version>-win64.msi` from the GitHub repository release
page: https://github.com/ChrisHal/PMbrowser/releases .

There is no need to install any libraries of the Qt framework, since the installer comes with all files necessary
for installing and running the program.

An entry will be created in the start menu.

Inside the program folder :file:`PM browser` under :file:`Program Files` you will find an offline version of
the documentation in the :file:`share/doc` folder. 


Installing on Linux
===================

using DEB package
*****************

Download the installer :file:`PMbrowser-<version>-win64.deb` from the GitHub repository release
page: https://github.com/ChrisHal/PMbrowser/releases . Use the command `sudo dpkg -i `PMbrowser-<version>-win64.deb`
to install.

Your distribution needs to provide packages for the Qt libraries, version 6. If it does not, you can
use the selfcontained *AppImage* (see :ref:`appimage`) or the flatpak (see :ref:`flatpak`).

Documentation will be installed in :file:`/usr/share/doc/QtPMbrowser` by default.
The executable :file:`QtPMbrowser` in :file:`/usr/bin` .

Now, you can start :program:`PMbrowser` with ``QtPMbrowser &`` from a terminal.

Starting with Version 2.2.1, you should find :program:`PM browser`
under *Applications*, too.

.. _appimage:

using the *AppImage*
********************

Dowload the AppImage file from https://github.com/ChrisHal/PMbrowser/releases. Make
the file executable (usually a right-click in you file manager is involved in this).
Double-clicking the file should start the application.

The *AppImage contains* all necessary Qt libraries, even when your distro does not provide
them.

.. _flatpak:

using *flatpak*
***************

A self-contained :file:`flatpak` file is provided on https://github.com/ChrisHal/PMbrowser/releases.
If your system is set up accordingly,
you can intall the flatpak by double-clicking the file after download. (See https://flatpak.org/setup/ 
for inforamtion on setting up flatpak.)

You might need to install the necessay runtime, which is provided by the flathub
repository.


Installing on macOS
===================

Download the DragN'Dropinstaller :file:`PMbrowser-<version>-Darwin.dmg` from
the GitHub repository release
page: https://github.com/ChrisHal/PMbrowser/releases .
After opening the :file:`dmg` image by double-clicking,
drag the application icon *QtPMbrowser* to your Applications directory.

Building from Source
====================

Since version 2.1, the build process has been unified such that :command:`cmake` is used
on all target systems.

Prerequisits
************

You will need to install the usual build tools including :program:`cmake` and
Qt-Library version 6.2 or newer. To build the documentation, you need :program:`sphinx` 
and :program:`sphinx_rtd_theme`. (Usually, these can be installed via :program:`pip`, 
if :program:`python` is installed.)

On most :program:`Linux` distributions , Qt is provided as a package,
e.g. :file:`qt6-base-dev` for Ubuntu.

For :program:`Windows` and :program:`macOS` the Qt developer tools need to be installed.
You can get them from `Qt <https://www.qt.io/>`_.

On :program:`Linux`, there is an obscure bug in the Qt-libraries that can lead to the error
  
  ``error while loading shared libraries: libQt6Core.so.6: cannot open shared object file: No such file or directory``
  
This can be solved by this command:
 
  ``sudo strip --remove-section=.note.ABI-tag /usr/lib/x86_64-linux-gnu/libQt6Core.so.6``
  
Build commands
**************

Usually, these commands should work to build and install from the terminal:
  
.. code-block:: bash

	git clone https://github.com/ChrisHal/PMbrowser.git
	mkdir PMbrowser_build
	cd PMbrowser_build
	cmake ../PMbrowser -DCMAKE_BUILD_TYPE=Release
	cmake --build . --config Release
	cmake --install . --config Release

You might need to set the :command:`cmake` variable `CMAKE_PREFIX_PATH` to your :file:`Qt` directory.
You might find this easier to do using :program:`cmake-gui` .

By default, the *documentation* will not be build / installed. You have to set the `BUILD_DOCS` option
for this when configuring `cmake`:

.. code-block:: bash

	cmake ../PMbrowser -DCMAKE_BUILD_TYPE=Release -DBUILD_DOCS=on

