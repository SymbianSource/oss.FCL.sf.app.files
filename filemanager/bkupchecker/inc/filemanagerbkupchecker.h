/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0""
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  Declaration of CFileManagerBkupChecker class
*
*/


#ifndef C_FILEMANAGERBKUPCHECKER_H
#define C_FILEMANAGERBKUPCHECKER_H

#include <aplaunchchecker.h>
#include <connect/sbdefs.h>

class CResourceFile;

using namespace conn;

/**
*  Plugin for application launch check
*
*  This class implements rule based application check Plug-in API.
*  Application launch is restricted during backup and restore.
*  
*  @since S60 3.2
*/
NONSHARABLE_CLASS( CFileManagerBkupChecker ) : public CAppLaunchChecker
    {

public:

    /**
     * Two-phased constructor.
     */
	static CFileManagerBkupChecker* NewL();

    /**
     * Destructor
     */
	~CFileManagerBkupChecker();

    /**
     * From CApplaunchChecker
     * Two-phased constructor.
     *
     * @since S60 3.2
     * @param aAppToLaunch Uid of application to launch
     * @param TApaTaskList List of applications running in device
     * @return Launch code for apparc
     */
	TAppLaunchCode virtual OkayToLaunchL(const TUid aAppToLaunch, TApaTaskList& aTaskList);

private:

    CFileManagerBkupChecker();
    
    void ConstructL();
    
    TBool ValidateBUROngoing();
    
    static TInt ThreadFunction( TAny* ptr );
    
    void ThreadFunctionL();

private: // data

    /**
     * Array of allowed uids to run.
     */
    RArray<TUint32> iUids;
    
    /**
     * Notes shown during backup or restore.
     * Own
     */
    HBufC* iBackupNote;
    HBufC* iRestoreNote;

    /**
     * Ongoing backup or restore.
     */
    TBool iIsBackup;
    };

#endif // C_FILEMANAGERBKUPCHECKER_H
