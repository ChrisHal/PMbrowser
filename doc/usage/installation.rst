###############################
Installing :program:`PMbrowser`
###############################

You can install from source or use one of the provided binaries. Currently,
for Linux, an executable is available as well as a DEB package.
For Windows (64bit only) and macOS (minimum version 10.13) installers are available.

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

To run :program:`PMbrowser`, you will need the Qt5 libraries installed. On Debian and Ubuntu this is easily done
with this command: ``sudo apt install qt5-default``

using DEB package
*****************

Download the installer :file:`PMbrowser-<version>-win64.deb` from the GitHub repository release
page: https://github.com/ChrisHal/PMbrowser/releases . Use the command `sudo dpkg -i `PMbrowser-<version>-win64.deb`
to install.

Documentation will be installed in :file:`/usr/share/doc/QtPMbrowser` by default.
The executable :file:`QtPMbrowser` in :file:`/usr/bin` .

Now, start :program:`PMbrowser` with ``QtPMbrowser &``.

direct instalation of the executable
*************************************

The executable can be found at the GitHub repository release page: https://github.com/ChrisHal/PMbrowser/releases

Save to file to an appropiate location, e.g. :file:`/usr/local/bin/` and make sure it is marked executable.
An easy way to achive this is to use the :command:`install` command.

A possible command sequence to get and install v2.0 of :program:`PMbrowser` could look like this

.. code-block:: bash

 wget https://github.com/ChrisHal/PMbrowser/releases/download/v2.1/QtPMbrowser
 sudo install QtPMbrowser /usr/local/bin/


For this to work, :command:`wget` must be installed.

Now, start :program:`PMbrowser` with ``QtPMbrowser &``.

Installing on macOS
===================

Download the installer :file:`PMbrowser-<version>-Darwin.pkg` from the GitHub repository release
page: https://github.com/ChrisHal/PMbrowser/releases . The installer will guide you through the
instalation process.

On :program:`macOS` no offline documentation will be installed.


Building from Source
====================

Since version 2.1, the build process has been unified such that :command:`cmake` is used
on all target systems.

Prerequisits
************

You will need to install the usual build tools including :program:`cmake` and
Qt-Library version 5.12 or newer. To build the documention, you need :program:`Sphinx` .

On most :program:`Linux` distributions , Qt is provided as a package,
e.g. :file:`qt5-default` for Ubuntu.

For :program:`Windows` and :program:`macOS` the Qt developer tools need to be installed.
You can get them from `Qt <https://www.qt.io/>`_.

On :program:`Linux`, there is an obscure bug in the Qt-libraries that can lead to the error
  
  ``error while loading shared libraries: libQt5Core.so.5: cannot open shared object file: No such file or directory``
  
This can be solved by this command:
 
  ``sudo strip --remove-section=.note.ABI-tag /usr/lib/x86_64-linux-gnu/libQt5Core.so.5``
  
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
for this.

