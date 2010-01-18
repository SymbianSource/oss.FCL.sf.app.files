/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  This class offers AIW services
*
*/



#ifndef C_FILEMANAGERAIWPROVIDER_H
#define C_FILEMANAGERAIWPROVIDER_H

// INCLUDE FILES
#include <e32std.h>
#include <AiwServiceIfBase.h>
#include <AknLaunchAppService.h>


// CLASS DECLARATION
/**
*  This class offers AIW services
*
*  @since 5.0
*/
NONSHARABLE_CLASS( CFileManagerAiwProvider ) :
        public CAiwServiceIfBase,
        public MAknServerAppExitObserver
    {
public:

    /**
    * Two-phased constructor.
    */
    static CFileManagerAiwProvider* NewL();

    /**
    * Destructor.
    */
    ~CFileManagerAiwProvider();

public: // From CAiwServiceIfBase

    void InitialiseL(
        MAiwNotifyCallback& aFrameworkCallback,
        const RCriteriaArray& aInterest );

    void HandleServiceCmdL(
        const TInt& aCmdId,
        const CAiwGenericParamList& aInParamList,
        CAiwGenericParamList& aOutParamList,
        TUint aCmdOptions = 0,
        const MAiwNotifyCallback* aCallback = NULL );

public: // From MAknServerAppExitObserver
    void HandleServerAppExit( TInt aReason );

private: // New methods
    void CmdEditL(
        const CAiwGenericParamList& aInParamList,
        const MAiwNotifyCallback* aCallback );

    void NotifyL( TInt aEvent );

    TUid GetAppUidL();

    TUid GetAppUid();

    void LaunchEmbeddedL(
        const CAiwGenericParamList& aInParamList,
        const MAiwNotifyCallback* aCallback );

    TBool IsStandaloneLaunch(
        const CAiwGenericParamList& aInParamList );

    void LaunchStandaloneL(
        const CAiwGenericParamList& aInParamList );

    TBool IsFolderToOpenPathGiven(
        const CAiwGenericParamList& aInParamList );

private:
    /**
    * C++ default constructor.
    */
    CFileManagerAiwProvider();

private: // Data
    /**
     * For launching embedded file manager
     * Own.
     */
    CAknLaunchAppService* iService;

    /**
     * For client notifications
     * Not own.
     */
    const MAiwNotifyCallback* iCallback;

    /**
     * For storing the current input params
     * Own.
     */
    CAiwGenericParamList* iInParamList;

    /**
     * Id of the current command
     */
    TInt iCmd;

    /**
     * The uid of File Manager application to be started for this service
     */
    TUid iAppUid;

    };

#endif // C_FILEMANAGERAIWPROVIDER_H
