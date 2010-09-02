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
* 
* Description:
*     The source file of filemanager view base
*
*/

#ifndef FMVIEWBASE_H
#define FMVIEWBASE_H

#include <HbView>

class HbAction;

class FmViewBase : public HbView
{
Q_OBJECT
public:
    enum TViewType
    {
        ENone = 0,
        EDriverView,
        EFileView,
        EFindView,
        EBackupView,
        ERestoreView,
        ESplitView,
        EDeleteBackupView
    };


    explicit FmViewBase( TViewType viewType );
    virtual ~FmViewBase(void);
    
    TViewType viewType()
    {
        return mViewType;
    }
    virtual void aboutToClose();
private slots:
    void on_backAction_triggered();
	
signals:
    void tryToExit();   
    
private:
    void init();

private:
    TViewType mViewType;
    
    HbAction *mBackAction;
};

#endif

