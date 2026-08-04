// Stratofier Stratux AHRS Display
// (c) 2018 Allen K. Lair, Sky Fun
// Ported to Qt 6 / Android 16 (API 36) — androidextras (QAndroidJniObject) is
// Qt 5-only and no longer available. Qt 6 replaces it with QJniObject +
// QNativeInterface::QAndroidApplication, both in QtCore.

#include <QJniObject>
#include <QtCore/qnativeinterface.h>
#include <QtDebug>
#include <jni.h>
#include <cassert>

#include "ScreenLocker.h"

ScreenLocker::ScreenLocker()
{
    // Qt 5 looked up the Activity via the internal
    // "org/qtproject/qt5/android/QtNative" class, which no longer exists in
    // Qt 6 (and was never public API). Qt 6 exposes the Android context
    // directly through QNativeInterface::QAndroidApplication::context().
    QJniObject activity = QNativeInterface::QAndroidApplication::context();

    if( activity.isValid() )
    {
        QJniObject serviceName = QJniObject::getStaticObjectField<jstring>(
            "android/content/Context", "POWER_SERVICE" );

        if( serviceName.isValid() )
        {
            QJniObject powerMgr = activity.callObjectMethod(
                "getSystemService",
                "(Ljava/lang/String;)Ljava/lang/Object;",
                serviceName.object<jobject>() );

            if( powerMgr.isValid() )
            {
                jint levelAndFlags = QJniObject::getStaticField<jint>(
                    "android/os/PowerManager", "SCREEN_DIM_WAKE_LOCK" );
                QJniObject tag = QJniObject::fromString( "My Tag" );

                m_screenLock = powerMgr.callObjectMethod(
                    "newWakeLock",
                    "(ILjava/lang/String;)Landroid/os/PowerManager$WakeLock;",
                    levelAndFlags, tag.object<jstring>() );
            }
        }
    }

    if( m_screenLock.isValid() )
    {
        m_screenLock.callMethod<void>( "acquire", "()V" );
        qInfo() << "Screen stay on enabled";
    }
    else
        assert( false );
}


ScreenLocker::~ScreenLocker()
{
    if( m_screenLock.isValid() )
    {
        m_screenLock.callMethod<void>( "release", "()V" );
        qInfo() << "Screen stay on disabled";
    }
}
