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
* 
* Description:
*     The header file of service utils handler
*/


#ifndef FMSERVICEUTILSHANDLER_H
#define FMSERVICEUTILSHANDLER_H

#include <e32def.h> 
#include <e32cmn.h> 
#include <QString>


// INCLUDES
#include <e32base.h>
#include <f32file.h>

/*!
    \class MServiceUtilsObserver 
    \brief The class MServiceUtilsObserver provide event to notify CloseApp complete
*/
class MServiceUtilsObserver
{
public:
    // called when CloseAppsL operation complete
    virtual void handleCloseAppCompleteL( TInt err ) = 0;
};

// FORWARD DECLARATIONS
class CBaBackupSessionWrapper;

/*!
    \class CFmServiceUtilsHandler 
    \brief The class CFmServiceUtilsHandler provide implement of service utils APIs
*/
NONSHARABLE_CLASS(CFmServiceUtilsHandler) :
        public CActive
    {

public:
    static CFmServiceUtilsHandler* NewL();
    virtual ~CFmServiceUtilsHandler();

public: // New methods
    void CloseAppsL();
    void RestartAppsL();
    void setObserver( MServiceUtilsObserver *observer );
    
private: // From CActive
    void DoCancel();
    void RunL();
    TInt RunError( TInt aError );

private:

    CFmServiceUtilsHandler();
    void ConstructL();
    
private:    // Data
    /**
     * Last process error
     */
    TInt iLastError;

    /**
     * For closing and restarting open files while formatting
     */
    CBaBackupSessionWrapper* iBSWrapper;

    /**
     * Target drive of the process
     */
    TInt iDrive;

    /**
     * Wait is used to change asynchronous function to synchronous function
     */
    CActiveSchedulerWait iWait;
    
    MServiceUtilsObserver *iObserver;
    };

#endif
            
// End of File
