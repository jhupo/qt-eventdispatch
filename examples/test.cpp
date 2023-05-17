#include "test.h"

#include <QDebug>

#include <qteventdispatch.h>

Test::Test(QObject* parent)
    : QObject(parent)
{
    REGISTER_EVENT(0x0001);
}

Test::~Test()
{
    UNREGISTER_EVENT_ALL;
}

void Test::customEvent(QEvent *event)
{
    HANDLE_EVENT_BEGIN(pEvent);
    switch (pEvent->customEvent()) {
    case 0x0001:
    {
        qDebug()<<pEvent->customData().toString();
        break;
    }
    case 0x0002:
    {
        qDebug()<<"internal value = "<<pEvent->customData().toString();
        break;
    }
    default:
        break;
    }
    HANDLE_EVENT_END;
}
