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
    HbApplication app( argc, argv );
    app.setApplicationName( "filemanager" );

    QTranslator translator;
    QString lang = QLocale::system().name(); 
    QString path = "z:/resource/qt/translations/"; 
    translator.load( path + "filemanager_" + lang ); 
    app.installTranslator(&translator);


    FmMainWindow mw;

    mw.show(); 


    return app.exec();
}
