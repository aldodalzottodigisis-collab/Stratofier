#include <QJniObject>
#include <QCoreApplication>
#include <QNativeInterface>
#include <QtDebug>
#include "ScreenLocker.h"

ScreenLocker::ScreenLocker()
{
    // Qt 6: obtener el contexto de la Activity
    QJniObject activity = QNativeInterface::QAndroidApplication::context();

    if (!activity.isValid()) {
        qWarning() << "ScreenLocker: activity/context not valid";
        return;
    }

    QJniObject serviceName = QJniObject::getStaticObjectField(
        "android/content/Context",
        "POWER_SERVICE",
        "Ljava/lang/String;");

    if (!serviceName.isValid())
        return;

    QJniObject powerMgr = activity.callObjectMethod(
        "getSystemService",
        "(Ljava/lang/String;)Ljava/lang/Object;",
        serviceName.object<jobject>());

    if (!powerMgr.isValid())
        return;

    jint levelAndFlags = QJniObject::getStaticField<jint>(
        "android/os/PowerManager",
        "SCREEN_BRIGHT_WAKE_LOCK");

    QJniObject tag = QJniObject::fromString("Stratofier");

    m_screenLock = powerMgr.callObjectMethod(
        "newWakeLock",
        "(ILjava/lang/String;)Landroid/os/PowerManager$WakeLock;",
        levelAndFlags,
        tag.object<jstring>());

    if (m_screenLock.isValid()) {
        m_screenLock.callMethod<void>("acquire", "()V");
        qInfo() << "Screen stay-on enabled";
    } else {
        qWarning() << "Failed to acquire WakeLock";
    }
}

ScreenLocker::~ScreenLocker()
{
    if (m_screenLock.isValid()) {
        m_screenLock.callMethod<void>("release", "()V");
        qInfo() << "Screen stay-on disabled";
    }
}
