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
 * 
 * Description:
 *     The source file of File Manager main window
 */

#include "fmmainwindow.h"

#include <hbaction.h>

FmMainWindow::FmMainWindow() : mViewManager( 0 ), mFirstViewLoaded( false )
{
    connect(this, SIGNAL(viewReady()), this, SLOT(delayedLoading()));
}

FmMainWindow::~FmMainWindow()
{
    FmViewManager::RemoveViewManager();
}

void FmMainWindow::onOrientationChanged( Qt::Orientation orientation )
{
	Q_UNUSED( orientation );
	/*
    if ( orientation == Qt::Vertical ) {
        activateDriverView();
    } else {
        activateSplitView();
    }
	*/
}

void FmMainWindow::init()
{
    FmLogger::log("FmMainWindow::init start");
    mViewManager = FmViewManager::CreateViewManager( this );
    mViewManager->createDriverView();
    connect( this, SIGNAL( orientationChanged( Qt::Orientation ) ),
             this, SLOT( onOrientationChanged( Qt::Orientation ) ) );
    
    FmLogger::log("FmMainWindow::init end");
//    if ( orientation() == Qt::Vertical ) {
//        createDriverView();
//    } else {
//        createSplitView();
//    }
}

void FmMainWindow::delayedLoading()
{
    FmLogger::log("FmMainWindow::delayedLoading start");
    if( mFirstViewLoaded ) {
        return;
    }
    init();
    mFirstViewLoaded = true;
    FmLogger::log("FmMainWindow::delayedLoading end");
}

