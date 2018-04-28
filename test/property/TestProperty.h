#pragma once

#include <QObject>

class TestPropertyClass : public QObject {

    Q_OBJECT

public:
    Q_SIGNAL void aSignal();

};
