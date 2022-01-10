###############################
Installing :program:`PMbrowser`
###############################

You can install from source or use one of the provided binaries. Currently,
a Linux executable is available (but no installation package)
and a installer for 64bit Windows. Alternatively, you can build
from source.

Since :program:`PMbrowser` is created using the `Qt-framework <https://www.qt.io/>`_, building
should work on macOS, too.

Installing on Windows
=====================

Download the installer :file:`PMbrowserSetup_win_x64.msi` from the GitHub repository release
page: https://github.com/ChrisHal/PMbrowser/releases .

You might need to download and install the
`Visual C++ Redistributable <https://aka.ms/vs/17/release/vc_redist.x64.exe>`_ for a successful installation.
There is a good chance that this is installed already. Anyway, if you re-install or update :program:`PMbrowser`
you will not need to install the redistributable again.

There is no need to install any libraries of the Qt framework, since the installer comes with all files necessary
for installing and running the program.


Installing on Linux
===================

The executable can be found at the GitHub repository release page: https://github.com/ChrisHal/PMbrowser/releases

Save to file to an appropiate location, e.g. :file:`/usr/local/bin/` and make sure it is marked executable.
An easy way to achive this is to use the :command:`install` command.

A possible command sequence to get and install v2.0 of :program:`PMbrowser` could look like this

.. code-block:: bash

 wget https://github.com/ChrisHal/PMbrowser/releases/download/v2.0/QtPMbrowser
 sudo install QtPMbrowser /usr/local/bin/


For this to work, :command:`wget` must be installed.

To run :program:`PMbrowser`, you will need the Qt5 libraries installed. On Debian and Ubuntu this is easily done
with this command: ``sudo apt install qt5-default``

Now, start :program:`PMbrowser` with ``QtPMbrowser &``.


Building from Source
====================

Linux
*****


You will need to install the usual build tools and Qt-Library version 5.xx.
On most distributions, Qt is provided as a package, e.g. :file:`qt5-default` for Ubuntu.

Download /clone the files from the GitHub repository.
Use the provided :file:`.pro`-file. Usually, all you need to do is

.. code-block:: bash

 git clone https://github.com/ChrisHal/PMbrowser.git
 cd PMbrowser/QtPMbrowser
 qmake QtPMbrowser.pro
 make
 
There is an obscure bug in the Qt-libraries that can lead to the error
  
  ``error while loading shared libraries: libQt5Core.so.5: cannot open shared object file: No such file or directory``
  
This can be solved by this command:
 
  ``sudo strip --remove-section=.note.ABI-tag /usr/lib/x86_64-linux-gnu/libQt5Core.so.5``
  

Optionally, to install to the default location, e.g. :file:`/opt/QtPMbrowser/bin/`:

.. code-block:: bash

 make install

Windows
*******

Your best option is to use the provided solution file :file:`QtPMbrowser.sln` to build using Visual Studio 2019.
The Qt developer tools and the corresponding VS extension needs to be installed. You can get them from `Qt <https://www.qt.io/>`_.
The solution file contains two targets: the excutable :file:`QtPMbrowser.exe` and an installer / setup package.

It is recommended to build the installer. After a sucessful build, you find the :file:`msi` file in the folder :file:`PMbrowserSetup\\Release\\`.
This can be used to install :program:`PMbrowser` with all dependencies.


other, e.g. macOS
*****************

Unfortunately, no build instructions are available for other platforms. As long as the Qt5 framework is avilable
on a platform, it should be possible to build :program:`PMbrowser`.

