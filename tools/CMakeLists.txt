cmake_minimum_required(VERSION 3.1)

project(QMLCppApiTools)

find_package(Multimake)

set(qt_GIT_COMMIT v5.8.0)
add_qt_external_git_project(qt "https://code.qt.io/qt/qt5.git" "" "--module-subset=qtbase,qtdeclarative,qtmultimedia,qtquickcontrols,qttools" "-nomake;tests;-nomake;examples;-no-compile-examples;-opensource;-confirm-license")

set(gammaray_GIT_COMMIT 79aff446d0274563b56569c76a57dfd940edcc9e)
add_cmake_external_git_project(gammaray "https://github.com/KDAB/GammaRay.git" "qt" "" "")

set(qmllive_GIT_COMMIT 5.8)
add_qmake_external_git_project(qmllive "http://code.qt.io/cgit/qt-apps/qmllive.git" "qt" "" "")
