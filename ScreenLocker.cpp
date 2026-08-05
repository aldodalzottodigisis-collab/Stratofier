#include <QJniObject>
#include <QtDebug>
#include "ScreenLocker.h"

ScreenLocker::ScreenLocker()
{
    // Qt 6: clase actualizada (sin el "5")
    QJniObject activity = QJniObject::callStaticObjectMethod(
        "org/qtproject/qt/android/QtNative",
        "activity",
        "()Landroid/app/Activity;");

    if (!activity.isValid()) {
        qWarning() << "ScreenLocker: activity not valid";
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
