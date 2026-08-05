/*
Stratofier Stratux AHRS Display
(c) 2018 Allen K. Lair, Sky Fun
*/

#include <QApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QtDebug>
#include <QDir>
#include <QSettings>
#include <QFontDatabase>
#include <QStandardPaths>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

#include "AHRSMainWin.h"
#include "Keyboard.h"
#include "StreamReader.h"

QSettings *g_pSet = nullptr;
Keyboard  *g_pKeyboard = nullptr;
StreamReader *g_pStratuxStream = nullptr;

static void writeLog(const QString &msg)
{
#if defined(Q_OS_ANDROID)
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    QFile f(dir + "/stratofier_boot.log");
    if (f.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream ts(&f);
        ts << QDateTime::currentDateTime().toString(Qt::ISODate) << "  " << msg << "\n";
        f.close();
    }
#endif
    qInfo().noquote() << msg;
}

int main(int argc, char *argv[])
{
    QApplication guiApp(argc, argv);
    writeLog("main: QApplication created");

    QStringList qslArgs = guiApp.arguments();
    QString qsArg, qsToken, qsVal;
    bool bMax = true;
    QString qsIP;
    bool bPortrait = true;
    AHRSMainWin *pMainWin = nullptr;
    QString qsCurrWorkPath("/home/pi/Stratofier");
    bool bWindowed = false;

    foreach (qsArg, qslArgs) {
        QStringList qsl = qsArg.split('=');
        if (qsl.count() == 2) {
            qsToken = qsl.first();
            qsVal = qsl.last();
            if (qsToken == "initdisp")
                bMax = (qsVal == "max");
            else if (qsToken == "ip")
                qsIP = qsVal;
            else if (qsToken == "orient")
                bPortrait = (qsVal == "portrait");
            else if (qsToken == "home")
                qsCurrWorkPath = qsVal;
        } else if (qsArg == "windowed") {
            bWindowed = true;
        }
    }

#if defined(Q_OS_ANDROID)
    QScreen *pScreen = QGuiApplication::primaryScreen();
    if (pScreen) {
        const Qt::ScreenOrientation o = pScreen->orientation();
        bPortrait = (o == Qt::PortraitOrientation || o == Qt::InvertedPortraitOrientation);
    }
    writeLog(QString("main: portrait=%1").arg(bPortrait));
#else
    QDir::setCurrent(qsCurrWorkPath);
#endif

    QCoreApplication::setOrganizationName("Sky Fun");
    QCoreApplication::setOrganizationDomain("skyfun.space");
    QCoreApplication::setApplicationName("Stratofier");
    QGuiApplication::setApplicationDisplayName("Stratofier");

#if !defined(Q_OS_ANDROID)
    g_pSet = new QSettings("./config.ini", QSettings::IniFormat);
#else
    g_pSet = new QSettings;
#endif

    qsIP = g_pSet->value("StratuxIP", "192.168.10.1").toString();
    writeLog(QString("main: StratuxIP=%1").arg(qsIP));

    g_pStratuxStream = new StreamReader(qsIP);
    writeLog("main: StreamReader created");

    pMainWin = new AHRSMainWin(qsIP, bPortrait, g_pStratuxStream);
    writeLog("main: AHRSMainWin created");

#if defined(Q_OS_ANDROID)
    // En Android showMaximized a veces deja pantalla gris; usar show()
    pMainWin->show();
    writeLog("main: show()");
#else
    if (bMax)
        pMainWin->showMaximized();
    else {
        pMainWin->show();
        if (bPortrait)
            pMainWin->setGeometry(bWindowed ? 50 : 0, bWindowed ? 50 : 0, 562, 1000);
        else
            pMainWin->setGeometry(bWindowed ? 50 : 0, bWindowed ? 50 : 0, 1000, 562);
    }
#endif

    writeLog("main: entering event loop");
    const int rc = guiApp.exec();
    writeLog(QString("main: exit code %1").arg(rc));

    delete g_pStratuxStream;
    g_pStratuxStream = nullptr;
    return rc;
}
