#ifndef __TEST_H__
#define __TEST_H__

#include <QObject>

class Test : public QObject
{
    Q_OBJECT

public:

    Test(QObject *parent = Q_NULLPTR);
    virtual~Test();

protected:

    virtual void customEvent(QEvent *event);

};


#endif
