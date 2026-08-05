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

#include "AHRSMainWin.h"
#include "Keyboard.h"
// #if defined( Q_OS_ANDROID )
// #include "ScreenLocker.h"
// #endif
#include "StreamReader.h"


QSettings *g_pSet = nullptr;
Keyboard  *g_pKeyboard = nullptr;


// This needs to stay global so it's not destroyed when the orientation changes
StreamReader *g_pStratuxStream = nullptr;


int main( int argc, char *argv[] )
{
#if defined( Q_OS_ANDROID )
    // En Qt 6 ya no hace falta AA_EnableHighDpiScaling (está deprecado)
    // QGuiApplication::setAttribute( Qt::AA_EnableHighDpiScaling );
#endif

    QApplication guiApp( argc, argv );
    QStringList  qslArgs = guiApp.arguments();
    QString      qsArg;
    QString      qsToken, qsVal;
    bool         bMax = true;
    QString      qsIP;
    bool         bPortrait = true;
    AHRSMainWin *pMainWin = 0;
    QString      qsCurrWorkPath( "/home/pi/Stratofier" );
    bool         bWindowed = false;

#if defined( Q_OS_ANDROID )
    // ScreenLocker desactivado temporalmente para diagnosticar el crash
    // ScreenLocker locker;
#endif

    foreach( qsArg, qslArgs )
    {
        QStringList qsl = qsArg.split( '=' );

        if( qsl.count() == 2 )
        {
            qsToken = qsl.first();
            qsVal = qsl.last();

            if( qsToken == "initdisp" )
                bMax = (qsVal == "max");
            else if( qsToken == "ip" )
                qsIP = qsVal;
            else if( qsToken == "orient" )
                bPortrait = (qsVal == "portrait");
            else if( qsToken == "home" )
                qsCurrWorkPath = qsVal;
            else if( qsArg == "windowed" )
                bWindowed = true;
        }
    }

#if defined( Q_OS_ANDROID )
    QScreen *pScreen = QGuiApplication::primaryScreen();
    if( pScreen )
        bPortrait = ((pScreen->orientation() == Qt::PortraitOrientation) ||
                     (pScreen->orientation() == Qt::InvertedPortraitOrientation));
#else
    QDir::setCurrent( qsCurrWorkPath );
#endif

    QCoreApplication::setOrganizationName( "Sky Fun" );
    QCoreApplication::setOrganizationDomain( "skyfun.space" );
    QCoreApplication::setApplicationName( "Stratofier" );
    QGuiApplication::setApplicationDisplayName( "Stratofier" );

#if !defined( Q_OS_ANDROID )
    g_pSet = new QSettings( "./config.ini", QSettings::IniFormat );
#else
    g_pSet = new QSettings;
#endif

    qsIP = g_pSet->value( "StratuxIP", "192.168.10.1" ).toString();

    qInfo() << "Iniciando Stratofier";
    g_pStratuxStream = new StreamReader( qsIP );
    pMainWin = new AHRSMainWin( qsIP, bPortrait, g_pStratuxStream );

    if( bMax )
        pMainWin->showMaximized();
    else
    {
        pMainWin->show();
        if( bPortrait )
            pMainWin->setGeometry( bWindowed ? 50 : 0, bWindowed ? 50 : 0, 562, 1000 );
        else
            pMainWin->setGeometry( bWindowed ? 50 : 0, bWindowed ? 50 : 0, 1000, 562 );
    }

    guiApp.exec();

    delete g_pStratuxStream;
    g_pStratuxStream = nullptr;

    return 0;
}
