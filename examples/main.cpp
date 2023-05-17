#include <QApplication>

#include "test.h"

#include <qteventdispatch.h>

int main(int argc, char** argv)
{
    QApplication a(argc, argv);

    Test t;

    PUBLISH_EVENT_VALUE(0x0001,QString("test"));

    PUBLISH_INTERNAL_EVENT_VALUE(&t,0x0002,QString("test 1"));

    return a.exec();
}
