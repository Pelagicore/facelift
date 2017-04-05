#include "TunerModelPlugin.h"

#include <qqml.h>

#include "tuner/TunerModuleDummy.h"
#include "tuner/TunerModule.h"
#include "tuner/TunerViewModelQML.h"

#include "models/TunerViewModelCpp.h"

void TunerModelPlugin::registerTypes(const char *uri) {

    TunerModule::registerTypes();
    TunerModule::registerQmlTypes(uri);

    TunerModuleDummy::registerQmlTypes(uri);
    registerQmlComponent<TunerViewModelCpp>(uri);
//    TunerViewModelQMLImplementation::registerTypes(uri, STRINGIFY(TUNERVIEWMODEL_QML_LOCATION));

}
