#include "qteventdispatch.h"

#include <QMap>
#include <QDebug>
#include <QThread>
#include <QCoreApplication>
#include <QReadWriteLock>

namespace Event {

    class QtEventDispatchPrivate
    {
        Q_DISABLE_COPY(QtEventDispatchPrivate)
        Q_DECLARE_PUBLIC(QtEventDispatch)

    public:

        QtEventDispatchPrivate(QtEventDispatch* q);
        virtual~QtEventDispatchPrivate();

        void init();

        void sendPostedEvents(QObject* obj, QtEvent* event, Qt::ConnectionType type);

        void listenerCollects(const int event, QList<QObject*>& objs);

        QtEventDispatch*const                    q_ptr;
        QReadWriteLock                                          _lock;
        QMap<int,QMap<Qt::EventPriority, QSet<QObject*> > >     _listeners;
    };

    QtEventDispatchPrivate::QtEventDispatchPrivate(QtEventDispatch *q)
        : q_ptr(q)
    {
        Q_ASSERT(q);
    }

    QtEventDispatchPrivate::~QtEventDispatchPrivate()
    {

    }

    void QtEventDispatchPrivate::init()
    {

    }

    void QtEventDispatchPrivate::sendPostedEvents(QObject *obj, QtEvent *event, Qt::ConnectionType type)
    {
#ifdef LOGGER_DETAILS
        qInfo()<<obj<<" recv "<<event->customEvent()<<" connect type "<<type;
#endif
        switch (type)
        {
        case Qt::AutoConnection:
        {
            if(obj->thread() != QThread::currentThread())
            {
                QCoreApplication::postEvent(obj,event);
            }
            else
            {
                QCoreApplication::sendEvent(obj,event);
#ifdef LOGGER_DETAILS
                qInfo()<<obj<<" The same thread";
#endif
                delete event;
            }
            break;
        }
        case Qt::DirectConnection:
        {
            QCoreApplication::sendEvent(obj,event);
            delete event;
            break;
        }
        case Qt::BlockingQueuedConnection:
        {
            if(QMetaObject::invokeMethod(obj,"invokeCustomEvent",type,Q_ARG(QEvent*, event)))
                delete event;
            else
                QCoreApplication::postEvent(obj,event);
            break;
        }
        case Qt::UniqueConnection:
        case Qt::QueuedConnection:
        {
            QCoreApplication::postEvent(obj,event);
            break;
        }
        default:break;
        }
    }

    void QtEventDispatchPrivate::listenerCollects(const int event, QList<QObject *> &objs)
    {
        objs = _listeners[event][Qt::HighEventPriority].values();
        objs.append(_listeners[event][Qt::NormalEventPriority].values());
        objs.append(_listeners[event][Qt::LowEventPriority].values());
    }

    QtEventDispatch::QtEventDispatch(QObject *parent)
        : QObject(parent)
        , d_ptr(new QtEventDispatchPrivate(this))
    {
        d_func()->init();
    }

    QtEventDispatch::~QtEventDispatch()
    {

    }

    Q_GLOBAL_STATIC(QtEventDispatch,_event_dispatch_inst_)
    QtEventDispatch *QtEventDispatch::inst()
    {
        return _event_dispatch_inst_;
    }

    void QtEventDispatch::publishEvent(const int event, const QVariant &val, const Qt::ConnectionType conn)
    {
        Q_D(QtEventDispatch);
        QReadLocker lock(&d->_lock);
        QList<QObject*> listeners;
        d->listenerCollects(event,listeners);
        foreach(QObject* obj, listeners)
        {
            d->sendPostedEvents(obj,new QtEvent(event,val),conn);
        }
    }

    void QtEventDispatch::sendPostedEvents(QObject *obj, const int event, const QVariant &var, const bool block)
    {
        if(block){
            if(QMetaObject::invokeMethod(obj,"invokeCustomEvent",Qt::BlockingQueuedConnection,Q_ARG(QEvent*, &QtEvent(event,var))))
            {
#ifdef LOGGER_DETAILS
                qInfo()<<obj<<" exec invokeCustomEvent";
#endif
            }
            else
            {
#ifdef LOGGER_DETAILS
                qWarning()<<obj<<"exec invokeCustomEvent error";
#endif
            }
        }else{
            if(obj->thread() != QThread::currentThread())
                QCoreApplication::postEvent(obj,new QtEvent(event,var));
            else
                QCoreApplication::sendEvent(obj,&QtEvent(event,var));
#ifdef LOGGER_DETAILS
            qInfo()<<obj<<" "<<event<<" event";
#endif
        }
    }

    bool QtEventDispatch::registerEvent(const int event, QObject *listener, const Qt::EventPriority priority)
    {
        Q_D(QtEventDispatch);
        QWriteLocker lock(&d->_lock);
        d->_listeners[event][priority].insert(listener);
#ifdef LOGGER_DETAILS
        qInfo()<< listener <<" register "<<event<<" event "<<priority<<" level";
#endif
        return true;
    }

    bool QtEventDispatch::unRegisterEvent(QObject *listener, const int event)
    {
        Q_D(QtEventDispatch);
        QWriteLocker lock(&d->_lock);
        if(-1 == event)
        {
            QList<int> keys = d->_listeners.keys();
            foreach(int key, keys)
            {
                d->_listeners[key][Qt::HighEventPriority].remove(listener);
                d->_listeners[key][Qt::NormalEventPriority].remove(listener);
                d->_listeners[key][Qt::LowEventPriority].remove(listener);
            }
#ifdef LOGGER_DETAILS
            qInfo()<<listener << " unregister all event";
#endif
        }
        else
        {
            d->_listeners[event][Qt::HighEventPriority].remove(listener);
            d->_listeners[event][Qt::NormalEventPriority].remove(listener);
            d->_listeners[event][Qt::LowEventPriority].remove(listener);
#ifdef LOGGER_DETAILS
            qInfo()<<listener<<" unregister "<<event<<" event";
#endif
        }
        return true;
    }

}

