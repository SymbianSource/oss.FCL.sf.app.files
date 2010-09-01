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


#include <e32std.h>
#include <ecom/implementationproxy.h>
#include <w32std.h>
#include <apgtask.h>
#include <apgcli.h>
#include <apgwgnam.h>
#include <apacmdln.h>
#include <centralrepository.h>
#include "FileManagerDebug.h"
#include "FileManagerPrivateCRKeys.h"
#include "filemanageraiwprovider.h"
#include "FileManagerUID.h"

const TImplementationProxy ImplementationTable[] =
    {
    IMPLEMENTATION_PROXY_ENTRY( 0x200110F9, CFileManagerAiwProvider::NewL )
    };
const TUid KUidDefaultFileManager = { KFileManagerUID3 };
const TInt KMaxSortMethodStr = 20;
_LIT( KRootMatch, "?:\\" );


// ======== LOCAL FUNCTIONS ========

// ---------------------------------------------------------------------------
// FindStandaloneAppL
// ---------------------------------------------------------------------------
//
static TBool FindStandaloneAppL(
        RWsSession& aWs, const TUid& aUid, TInt& aWgId )
    {
    FUNC_LOG;

    aWgId = 0; // Used window group id is always greater than zero
    RArray< RWsSession::TWindowGroupChainInfo > windowChain;
    User::LeaveIfError( aWs.WindowGroupList( &windowChain ) );
    CleanupClosePushL( windowChain );
    TInt count( windowChain.Count() );
    for( TInt i( 0 ); i < count; ++i )
        {
        const RWsSession::TWindowGroupChainInfo& entry( windowChain[ i ] );
        CApaWindowGroupName* app = CApaWindowGroupName::NewLC(
            aWs, entry.iId );
        TUid appUid( app->AppUid() );
        CleanupStack::PopAndDestroy( app );
        // Match the app's UID and the embedded status.
        // The app is standalone when there is no parent window group.
        if ( appUid == aUid && entry.iParentId <= 0 )
            {
            // Standalone application found
            aWgId = entry.iId;
            break;
            }
        }
    CleanupStack::PopAndDestroy( &windowChain );
    return aWgId > 0;
    }

// ---------------------------------------------------------------------------
// IsValidFolderToOpenPath
// ---------------------------------------------------------------------------
//
static TBool IsValidFolderToOpenPath( const TDesC& aFullPath )
    {
    FUNC_LOG;

    TInt len( aFullPath.Length() );
    if ( !len )
        {
        return ETrue; // Allow empty to open main view
        }
    // Check that at least root folder exists
    if ( aFullPath.Left( KRootMatch().Length() ).MatchF(
            KRootMatch ) == KErrNotFound )
        {
        return EFalse;
        }
    return ETrue;
    }

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CFileManagerAiwProvider::CFileManagerAiwProvider
// ---------------------------------------------------------------------------
//
CFileManagerAiwProvider::CFileManagerAiwProvider()
    {
    FUNC_LOG;
    }

// ---------------------------------------------------------------------------
// CFileManagerAiwProvider::NewL
// ---------------------------------------------------------------------------
//
CFileManagerAiwProvider* CFileManagerAiwProvider::NewL()
    {
    FUNC_LOG;

    return new ( ELeave ) CFileManagerAiwProvider();
    }

// ---------------------------------------------------------------------------
// CFileManagerAiwProvider::~CFileManagerAiwProvider
// ---------------------------------------------------------------------------
//
CFileManagerAiwProvider::~CFileManagerAiwProvider()
    {
    FUNC_LOG;

    delete iService;
    delete iInParamList;
    }

// ---------------------------------------------------------------------------
// CFileManagerAiwProvider::InitialiseL
// ---------------------------------------------------------------------------
//
void CFileManagerAiwProvider::InitialiseL(
        MAiwNotifyCallback& /*aFrameworkCallback*/,
        const RCriteriaArray& /*aInterest*/ )
    {
    FUNC_LOG;
    }

// ---------------------------------------------------------------------------
// CFileManagerAiwProvider::HandleServiceCmdL
// ---------------------------------------------------------------------------
//
void CFileManagerAiwProvider::HandleServiceCmdL(
        const TInt& aCmdId,
        const CAiwGenericParamList& aInParamList,
        CAiwGenericParamList& /*aOutParamList*/,
        TUint /*aCmdOptions*/,
        const MAiwNotifyCallback* aCallback )
    {
    FUNC_LOG;

    INFO_LOG2(
        "CFileManagerAiwProvider::HandleServiceCmdL-aCmdId=%d,aCallback=0x%x",
        aCmdId, aCallback );

    TInt err( KErrNone );
    switch ( aCmdId )
        {
        case KAiwCmdEdit:
            {
            TRAP( err, CmdEditL( aInParamList, aCallback ) );
            break;
            }
        default:
            {
            ERROR_LOG1(
                "CFileManagerAiwProvider::HandleServiceCmdL-InvalidCmd=%d",
                aCmdId );
            break;
            }
        }
    LOG_IF_ERROR2(
        err,
        "CFileManagerAiwProvider::HandleServiceCmdL-aCmdId=%d-err=%d",
        aCmdId, err );
    User::LeaveIfError( err );
    }

// ---------------------------------------------------------------------------
// CFileManagerAiwProvider::HandleServerAppExit
// ---------------------------------------------------------------------------
//
void CFileManagerAiwProvider::HandleServerAppExit( TInt /*aReason*/ )
    {
    FUNC_LOG;

    TRAP_IGNORE( NotifyL( KAiwEventCompleted ) );
    iCallback = NULL; // No notifications are needed after app exit
    }

// ---------------------------------------------------------------------------
// CFileManagerAiwProvider::CmdEditL
// ---------------------------------------------------------------------------
//
void CFileManagerAiwProvider::CmdEditL(
        const CAiwGenericParamList& aInParamList,
        const MAiwNotifyCallback* aCallback )
    {
    FUNC_LOG;

    iCmd = KAiwCmdEdit;
    if ( IsFolderToOpenPathGiven( aInParamList ) )
        {
        if ( IsStandaloneLaunch( aInParamList ) )
            {
            LaunchStandaloneL( aInParamList );
            }
        else
            {
            LaunchEmbeddedL( aInParamList, aCallback );
            }
        }
    }

// ---------------------------------------------------------------------------
// CFileManagerAiwProvider::NotifyL
// ---------------------------------------------------------------------------
//
void CFileManagerAiwProvider::NotifyL( TInt aEvent )
    {
    FUNC_LOG;

    if ( iCallback && iInParamList )
        {
        CAiwGenericParamList* eventParamList = CAiwGenericParamList::NewL();
        CleanupStack::PushL( eventParamList );
        // Must cast this because of AIW design error
        const_cast< MAiwNotifyCallback* >( iCallback )->HandleNotifyL(
    	    iCmd, aEvent, *eventParamList, *iInParamList );
        CleanupStack::PopAndDestroy( eventParamList );
        }
    }

// ---------------------------------------------------------------------------
// CFileManagerAiwProvider::GetAppUidL
// ---------------------------------------------------------------------------
//
TUid CFileManagerAiwProvider::GetAppUidL()
    {
    FUNC_LOG;

    CRepository* cenRep = CRepository::NewLC( KCRUidFileManagerSettings );
    TInt uid( 0 );
    User::LeaveIfError( cenRep->Get( KFileManagerAppUidForAiwService, uid ) );
    CleanupStack::PopAndDestroy( cenRep );
    return TUid::Uid( uid );
    }

// ---------------------------------------------------------------------------
// CFileManagerAiwProvider::GetAppUid
// ---------------------------------------------------------------------------
//
TUid CFileManagerAiwProvider::GetAppUid()
    {
    FUNC_LOG;

    if ( !iAppUid.iUid )
        {
        // Read the application to start from CenRep
        TRAPD( err, iAppUid = GetAppUidL() );
        if ( err != KErrNone || !iAppUid.iUid )
            {
            // Use the default application
            iAppUid = KUidDefaultFileManager;
            }
        ERROR_LOG1(
            "CFileManagerAiwProvider::GetAppUid-Uid=0x%x", iAppUid.iUid );
        }
    return iAppUid;
    }

// ---------------------------------------------------------------------------
// CFileManagerAiwProvider::LaunchEmbeddedL
// ---------------------------------------------------------------------------
//
void CFileManagerAiwProvider::LaunchEmbeddedL(
        const CAiwGenericParamList& aInParamList,
        const MAiwNotifyCallback* aCallback )
    {
    FUNC_LOG;

    delete iService;
    iService = NULL;
    delete iInParamList;
    iInParamList = NULL;
    iCallback = NULL;

    // Copy the input params for the launcher and then do the launch
    iInParamList = CAiwGenericParamList::NewL();
    TInt count( aInParamList.Count() );
    for( TInt i( 0 ); i < count; ++i )
        {
        iInParamList->AppendL( aInParamList[ i ] );
        }
    iService = CAknLaunchAppService::NewL( GetAppUid(), this, iInParamList );
    iCallback = aCallback;
    }

// ---------------------------------------------------------------------------
// CFileManagerAiwProvider::IsStandaloneLaunch
// ---------------------------------------------------------------------------
//
TBool CFileManagerAiwProvider::IsStandaloneLaunch(
        const CAiwGenericParamList& aInParamList )
    {
    FUNC_LOG;

    TBool ret( EFalse );
    TInt i( 0 );
    // Get sort method first
    const TAiwGenericParam* param = aInParamList.FindFirst(
        i, EGenericParamDir, EVariantTypeTInt32 );
    if ( i != KErrNotFound && param )
        {
        // Then, check if standalone is requested
        param = aInParamList.FindNext( i, EGenericParamDir, EVariantTypeTInt32 );
        if ( i != KErrNotFound && param )
            {
            ret = param->Value().AsTInt32();
            }
        }
    return ret;
    }

// ---------------------------------------------------------------------------
// CFileManagerAiwProvider::LaunchStandaloneL
// ---------------------------------------------------------------------------
//
void CFileManagerAiwProvider::LaunchStandaloneL(
        const CAiwGenericParamList& aInParamList )
    {
    FUNC_LOG;

    TPtrC folderToOpen( KNullDesC );
    TInt sortMethod( KErrNotFound );
    TInt i( 0 );
    // Get folder path
    const TAiwGenericParam* param = aInParamList.FindFirst(
        i, EGenericParamDir, EVariantTypeDesC );
    if ( i != KErrNotFound && param )
        {
        folderToOpen.Set( param->Value().AsDes() );
        }
    // Get sort method
    param = aInParamList.FindFirst( i, EGenericParamDir, EVariantTypeTInt32 );
    if ( i != KErrNotFound && param )
        {
        sortMethod = param->Value().AsTInt32();
        }

    RWsSession wsSession;
    User::LeaveIfError( wsSession.Connect() );
    CleanupClosePushL( wsSession );
    TInt wgId( 0 );
    TUid appUid( GetAppUid() );
    if ( FindStandaloneAppL( wsSession, appUid, wgId ) )
        {
        // Bring the existing standalone app to foreground
        TApaTask apaTask( wsSession );
        apaTask.SetWgId( wgId );
        apaTask.BringToForeground();
        }
    else
        {
        // Start new standalone app
        TApaAppInfo appInfo;
        RApaLsSession apaLsSession;
        User::LeaveIfError( apaLsSession.Connect() );
        CleanupClosePushL( apaLsSession );
        User::LeaveIfError( apaLsSession.GetAppInfo( appInfo, appUid ) );
        CApaCommandLine* apaCmdLine = CApaCommandLine::NewLC();
        apaCmdLine->SetExecutableNameL( appInfo.iFullName );
        apaCmdLine->SetCommandL( EApaCommandOpen );
        apaCmdLine->SetDocumentNameL( folderToOpen );
        if ( sortMethod != KErrNotFound )
            {
            TBuf8< KMaxSortMethodStr > sortMethodStr;
            sortMethodStr.AppendNum( sortMethod );
            apaCmdLine->SetTailEndL( sortMethodStr );
            }
        TThreadId dummy;
        User::LeaveIfError( apaLsSession.StartApp( *apaCmdLine, dummy ) );
        CleanupStack::PopAndDestroy( apaCmdLine );
        CleanupStack::PopAndDestroy( &apaLsSession );
        }
    CleanupStack::PopAndDestroy( &wsSession );
    }

// ---------------------------------------------------------------------------
// CFileManagerAiwProvider::IsFolderToOpenPathGiven
// ---------------------------------------------------------------------------
//
TBool CFileManagerAiwProvider::IsFolderToOpenPathGiven(
        const CAiwGenericParamList& aInParamList )
    {
    FUNC_LOG;

    TBool ret( EFalse );
    TPtrC folderToOpen( KNullDesC );
    TInt i( 0 );
    // Ensure first that no files are defined, because only folder service
    // is offered by this provider
    const TAiwGenericParam* param = aInParamList.FindFirst(
        i, EGenericParamFile, EVariantTypeAny );
    if ( i == KErrNotFound )
        {
        // Get folder path and check it
        i = 0;
        param = aInParamList.FindFirst(
            i, EGenericParamDir, EVariantTypeDesC );
        if ( i != KErrNotFound && param )
            {
            folderToOpen.Set( param->Value().AsDes() );
            ret = IsValidFolderToOpenPath( folderToOpen );
            }
        }
    if ( !ret )
        {
        ERROR_LOG2(
            "CFileManagerAiwProvider::IsFolderToOpenPathGiven-ret=%d,given=%S",
            ret, &folderToOpen );
        }
    return ret;
    }

// ======== GLOBAL FUNCTIONS ========

// ---------------------------------------------------------------------------
// ImplementationGroupProxy.
// ---------------------------------------------------------------------------
//
EXPORT_C const TImplementationProxy* ImplementationGroupProxy( TInt& aTableCount )
    {
    FUNC_LOG;

    aTableCount = sizeof( ImplementationTable ) / sizeof( TImplementationProxy );
    return ImplementationTable;
    }
