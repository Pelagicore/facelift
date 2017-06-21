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

## Dependencies

* Cmake >= 3.1
* Python 3 (for qface)
* Qt >= 5.5
* Antlr4 python3 runtime

Ubuntu/Debian packages:
The required packages can be installed using the following commands:
```
$ sudo apt-get install python3-click python3-path python3-pip python3-jinja2 python3-yaml cmake qtdeclarative5-dev qml-module-qtquick-controls qtdeclarative5-private-dev
```

In addition, since no deb package is available for it, you need to install the antlr4 runtime using the following command line (for example):
```
$ pip3 install antlr4-python3-runtime
```

# Build

We use QFace as a submodule, which you need to fetch using the following command line:
```
$ git submodule init && git submodule update
```

You are now ready to build the package using the following commands:
```
$ mkdir build
$ cd build
$ cmake .. && make
```

If you want to build using your own version of Qt, you can specify its location using the following command instead:
```
$ cmake -DCMAKE_PREFIX_PATH=/path/to/Qt/gcc_64/lib/cmake .. && make
```

Start the example application with:
$ /path/to/Qt/gcc_64/bin/qmlscene ../examples/AddressBook/ui/AddressBookApp.qml  -I imports

## License and Copyright

This code is copyright (c) 2017 Pelagicore AB
Licensed under the LGPL 2.1, please see LICENSE file for more.
