#include "BenchmarkingPlugin.h"
#include "facelift/tests/benchmarking/Module.h"
#include "BenchmarkingImplementation.h"

using namespace facelift::tests::benchmarking;

void BenchmarkingPlugin::registerTypes(const char *uri)
{
    Module::registerQmlTypes(uri);
    facelift::registerQmlComponent<BenchmarkingImplementation> (uri , "Benchmarking");
}

