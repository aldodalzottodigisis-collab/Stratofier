#ifndef __SCREENLOCKER_H__
#define __SCREENLOCKER_H__

#include <QJniObject>

class ScreenLocker
{
public:
    ScreenLocker();
    virtual ~ScreenLocker();

private:
    QJniObject m_screenLock;
};
#endif // __SCREENLOCKER_H__
