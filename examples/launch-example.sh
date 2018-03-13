#!/bin/sh -x

set -x

@_qt5Core_install_prefix@/bin/qmlscene  @QML_FILE@ -I @CMAKE_BINARY_DIR@/imports
