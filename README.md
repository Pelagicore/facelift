# QMLCppAPI

Project folder structure:
 - lib : contains the C++ library which the generated code relies on
 - generator : contains the code generator based on the qface framework
 - qface : contains the qface framework (IDL parser, code generation frontend)
 - examples : contains example applications
     - AddressBook
         - interface : contains the interface definition of the model
         - models : contains various implementations of the model interfaces
         - ui : contains the UI code of the application

# Build

Check the qface INSTALL.md file located in the "qface" folder. Note that qface is a
submodule which you can fetch using:
```
$ git submodule init && git submodule update
```
## Build dependencies
* Cmake >= 3.1
* Python 3 (for qface)

Build the project with:
```
$ mkdir build
$ cd build
$ cmake -DCMAKE_PREFIX_PATH=/path/to/Qt/gcc_64/lib/cmake .. && make
```

Start the example application with:
$ /path/to/Qt/gcc_64/bin/qmlscene ../examples/AddressBook/ui/AddressBookApp.qml  -I imports

## License and Copyright

This code is copyright (c) 2017 Pelagicore AB
Licensed under the LGPL 2.1, please see LICENSE file for more.
