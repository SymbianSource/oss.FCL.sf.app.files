/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*     Zhiqiang Yang <zhiqiang.yang@nokia.com>
*     Steven Yao <steven.yao@nokia.com>
*     Yong Zhang <yong.5.zhang@nokia.com>
* 
* Description:
*     The main application file for file manager
*
*/


#include "fmmainwindow.h"

#include <hbapplication.h>

#include <QTranslator>
#include <QLocale>

int main( int argc, char *argv[] )
{
    FmLogger::log( "main start" );
    HbApplication app( argc, argv );

    QTranslator translator;
    QString lang = QLocale::system().name(); 
    QString path = "z:/resource/qt/translations/"; 
    translator.load( path + "filemanager_" + lang );
    app.installTranslator(&translator);

    app.setApplicationName( hbTrId("txt_fmgr_title_file_manager") );
    
    FmLogger::log( "main_createMainwindow start" );
    FmMainWindow mw;
    FmLogger::log( "main_createMainwindow end" );

    mw.show(); 
    FmLogger::log( "main end" );
    return app.exec();
}
