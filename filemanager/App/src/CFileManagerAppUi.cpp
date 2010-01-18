/*
* Copyright (c) 2002-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  File Manager application UI class.
*
*/



// INCLUDE FILES
#include <akntitle.h>   // CAknTitlePane
#include <aknnavi.h>    // CAknNavigationControlContainer 
#include <aknnavide.h>  // CAknNavigationDecorator
#include <sendui.h>     // CSendAppUi
#include <sendnorm.rsg>
#include <avkon.rsg>
#include <barsread.h>
#include <hlplch.h>     // HlpLauncher
#include <eikapp.h>
#include <AknDef.h>     // For layout change event definitions
#include <utf.h>
#include <StringLoader.h>
#include <AknsUtils.h>
#include <centralrepository.h>
#include <AiwCommon.h>
#include <bautils.h>
#include <pathinfo.h>
#include <driveinfo.h>
#include <CFileManagerEngine.h>
#include <CFileManagerIRReceiver.h>
#include <FileManager.rsg>
#include <FileManagerView.rsg>
#include <FileManagerDebug.h>
#include <FileManagerUID.h>
#include <CFileManagerFeatureManager.h>
#include <CFileManagerUtils.h>
#include "CFileManagerAppUi.h"
#include "CFileManagerDocument.h"
#include "CFileManagerMainView.h"
#include "CFileManagerMemoryStoreView.h"
#ifdef RD_FILE_MANAGER_BACKUP
 #include <FileManagerSchDefinitions.h>
 #include "CFileManagerBackupView.h"
 #include "CFileManagerRestoreView.h"
 #include "CFileManagerTaskScheduler.h"
 #include "CFileManagerSchBackupHandler.h"
#endif // RD_FILE_MANAGER_BACKUP
#include "CFileManagerFoldersView.h"
#include "Cfilemanagersearchresultsview.h"
#include "CFileManagerIconArray.h"
#include "FileManager.hrh"


// CONSTANTS
const TUid CFileManagerAppUi::KFileManagerMainViewId = 
    { EFileManagerMainViewId };
const TUid CFileManagerAppUi::KFileManagerMemoryStoreViewId = 
    { EFileManagerMemoryStoreViewId };
const TUid CFileManagerAppUi::KFileManagerFoldersViewId = 
    { EFileManagerFoldersViewId };
const TUid CFileManagerAppUi::KFileManagerSearchResultsViewId = 
    { EFileManagerSearchResultsViewId };
const TUid CFileManagerAppUi::KFileManagerBackupViewId = 
    { EFileManagerBackupViewId };
const TUid CFileManagerAppUi::KFileManagerRestoreViewId = 
    { EFileManagerRestoreViewId };

const TUid KUidFileManager = { KFileManagerUID3 };
const TUid KUidGsRsfwPlugin = { 0x101F9778 };
const TUid KUidGsRsFwSettingsViewId = { 11 };


// ============================ LOCAL FUNCTIONS ================================

// ---------------------------------------------------------------------------
// ProcessExists
// ---------------------------------------------------------------------------
//
static TBool ProcessExists( const TSecureId& aSecureId )
    {
    _LIT( KFindPattern, "*" );
    TFindProcess finder( KFindPattern );
    TFullName processName;
    while( finder.Next( processName ) == KErrNone )
        {
        RProcess process;
        if ( process.Open( processName ) == KErrNone )
            {
            TSecureId processId( process.SecureId() );
            process.Close();
            if( processId == aSecureId )
                {
                return ETrue;
                }
            }
        }
    return EFalse;
    }

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CFileManagerAppUi::CFileManagerAppUi
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CFileManagerAppUi::CFileManagerAppUi() :
        iInitialFolderLevel( KErrNotFound )
    {
    FUNC_LOG
    }

// -----------------------------------------------------------------------------
// CFileManagerAppUi::~CFileManagerAppUi
// Destructor
// -----------------------------------------------------------------------------
// 
CFileManagerAppUi::~CFileManagerAppUi()
    {
    FUNC_LOG

    iCoeEnv->RemoveForegroundObserver( *this );
    delete iIRReceiver;
    delete iIRDeleteCB;
    delete iSendUi;
#ifdef RD_FILE_MANAGER_BACKUP
    delete iTaskScheduler;
    delete iSchBackupHandler;
#endif // RD_FILE_MANAGER_BACKUP
    CFileManagerEngine& engine =
        static_cast< CFileManagerDocument* >( Document() )->Engine();
    engine.SetObserver( NULL );
    }

// -----------------------------------------------------------------------------
// CFileManagerAppUi::ConstructL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerAppUi::ConstructL()
    {
    FUNC_LOG

    // Construct only must have stuff required for app start here.
    // Other stuff is constructed later to decrease app start time.
    INFO_LOG( "CFileManagerAppUi::ConstructL()-BaseConstructL" )
    BaseConstructL( EAknEnableSkin | EAknEnableMSK | EAknSingleClickCompatible );

    INFO_LOG( "CFileManagerAppUi::ConstructL()-Setup panes" )
    CEikStatusPane* sp = StatusPane();
    iTitlePane = static_cast< CAknTitlePane* >
        ( sp->ControlL( TUid::Uid( EEikStatusPaneUidTitle ) ) );

    INFO_LOG( "CFileManagerAppUi::ConstructL()-Create main view" )
    CAknView* view = CFileManagerMainView::NewLC();
    AddViewL( view ); // Takes ownership
    CleanupStack::Pop( view );
    SetDefaultViewL( *view );
    iActiveView = view->Id();

#ifdef RD_FILE_MANAGER_BACKUP
    CFileManagerDocument* doc =
        static_cast< CFileManagerDocument* >( Document() );
    iSchBackupStandalone = doc->IsScheduledBackup();
    if ( iSchBackupStandalone )
        {
        // Do standalone scheduled backup startup
        INFO_LOG( "CFileManagerAppUi::ConstructL-Launch scheduled backup" )
        iEikonEnv->WsSession().ComputeMode( RWsSession::EPriorityControlDisabled ); 
        RThread().SetProcessPriority( EPriorityForeground );
        sp->MakeVisible( EFalse );
        sp->SwitchLayoutL( R_AVKON_STATUS_PANE_LAYOUT_EMPTY );
        StartSchBackupL();
        }
    else
        {
#endif // RD_FILE_MANAGER_BACKUP

        CFileManagerEngine& engine =
            static_cast< CFileManagerDocument* >( Document() )->Engine();
        iWaitingForParams = engine.FeatureManager().IsEmbedded();

        // Do normal application startup
        INFO_LOG( "CFileManagerAppUi::ConstructL()-Add foreground observer" )
        iCoeEnv->AddForegroundObserverL( *this );

#ifdef RD_FILE_MANAGER_BACKUP
        }
#endif // RD_FILE_MANAGER_BACKUP

    }

// -----------------------------------------------------------------------------
// CFileManagerAppUi::HandleCommandL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerAppUi::HandleCommandL( TInt aCommand )
    {
    switch( aCommand )
        {
        case EEikCmdExit: // FALLTHROUGH
        case EAknSoftkeyExit:
            {
            CFileManagerEngine& engine = 
                static_cast< CFileManagerDocument* >( Document() )->Engine();
            engine.CancelRefresh();
            Exit();
            break;
            }
        case EAknCmdHelp:
            {
            HlpLauncher::LaunchHelpApplicationL( iEikonEnv->WsSession(), 
                AppHelpContextL());
            break;
            }
        default:
            break;
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerAppUi::HandleViewDeactivation
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerAppUi::HandleViewDeactivation( 
                            const TVwsViewId& aViewIdToBeDeactivated, 
                            const TVwsViewId& aNewlyActivatedViewId )
    {
    if ( aNewlyActivatedViewId.iAppUid == Application()->AppDllUid() )
        {
        if ( IsFileManagerView( aNewlyActivatedViewId.iViewUid ) &&
        		View( aNewlyActivatedViewId.iViewUid ) )
            {
            iActiveView = aNewlyActivatedViewId.iViewUid;
            }
        }
    CAknViewAppUi::HandleViewDeactivation(
        aViewIdToBeDeactivated, aNewlyActivatedViewId );
    }

// -----------------------------------------------------------------------------
// CFileManagerAppUi::ActivateFoldersViewL
// 
// -----------------------------------------------------------------------------
// 
TBool CFileManagerAppUi::ActivateFoldersViewL()
    {
    if ( iActiveView != KFileManagerFoldersViewId )
        {
        if ( iSearchViewOpen )
            {
            CreateAndActivateLocalViewL( KFileManagerFoldersViewId,
                CFileManagerFoldersView::KOpenFromSearchResultsView,
                KNullDesC8 );
            }
        else
            {
            CreateAndActivateLocalViewL( KFileManagerFoldersViewId );
            }
        return ETrue;
        }
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CFileManagerAppUi::CloseFoldersViewL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerAppUi::CloseFoldersViewL()
    {
    if ( iSearchViewOpen )
        {
        CreateAndActivateLocalViewL( KFileManagerSearchResultsViewId );
        iActiveView = KFileManagerSearchResultsViewId;
        }
    else 
        {
        ExitEmbeddedAppIfNeededL();
        ActivateMemoryStoreViewL();
        iActiveView = KFileManagerMemoryStoreViewId;
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerAppUi::ActivateSearchResultsViewL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerAppUi::ActivateSearchResultsViewL()
    {
    if ( iActiveView != KFileManagerSearchResultsViewId )
        {
        // Activate with refresh
        CreateAndActivateLocalViewL(
            KFileManagerSearchResultsViewId,
            KNullUid,
            KFileManagerSearchViewRefreshMsg );
        iSearchViewOpen = ETrue;
        }

    CFileManagerEngine& engine = 
        static_cast< CFileManagerDocument* >( Document() )->Engine();
    SetTitleL( engine.SearchString() );
    }

// -----------------------------------------------------------------------------
// CFileManagerAppUi::CloseSearchResultsViewL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerAppUi::CloseSearchResultsViewL()
    {
    CFileManagerEngine& engine =
        static_cast< CFileManagerDocument* >( Document() )->Engine();
    TInt level( engine.FolderLevel() );

    // If embedded, then ensure that the return folder is not below given level
    // or on different drive
    if ( engine.FeatureManager().IsEmbedded() )
        {
        TPtrC dir( engine.CurrentDirectory().Left(
            iFolderToOpenAtStartup.Length() ) );
        if ( engine.FolderLevel() < iInitialFolderLevel ||
             dir.CompareF( iFolderToOpenAtStartup ) )
            {
            engine.SetDirectoryWithBackstepsL( iFolderToOpenAtStartup );
            level = iInitialFolderLevel;
            }
        }

    if ( level > 0 )
        {
        CreateAndActivateLocalViewL( KFileManagerFoldersViewId );
        iActiveView = KFileManagerFoldersViewId;
        }
    else if ( !level )
        {
        CreateAndActivateLocalViewL( KFileManagerMemoryStoreViewId );
        iActiveView = KFileManagerMemoryStoreViewId;
        }
    else
        {
        CreateAndActivateLocalViewL( KFileManagerMainViewId );
        iActiveView = KFileManagerMainViewId;
        }
    iSearchViewOpen = EFalse;
    }

// -----------------------------------------------------------------------------
// CFileManagerAppUi::RestoreDefaultTitleL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerAppUi::RestoreDefaultTitleL()
    {
    TResourceReader reader;
    iCoeEnv->CreateResourceReaderLC( reader, R_FILEMANAGER_TITLE_PANE );
    iTitlePane->SetFromResourceL( reader ); // restore title pane
    CleanupStack::PopAndDestroy(); // reader
    }

// -----------------------------------------------------------------------------
// CFileManagerAppUi::SendUiL
// 
// -----------------------------------------------------------------------------
// 
CSendUi& CFileManagerAppUi::SendUiL()
    {
    if ( !iSendUi )
        {
        iSendUi = CSendUi::NewL();
        }
    return *iSendUi;
    }

// ------------------------------------------------------------------------------
// CFileManagerAppUi::HandleResourceChangeL
//
// ------------------------------------------------------------------------------
//
void CFileManagerAppUi::HandleResourceChangeL( TInt aType )
    {
    if( aType == KAknsMessageSkinChange ||
        aType == KEikDynamicLayoutVariantSwitch )
        {
        CEikStatusPane* sp = StatusPane();
        sp->HandleResourceChange( aType );
        CFileManagerDocument* doc =
            static_cast< CFileManagerDocument* >( Document() );
        doc->ClearStringCache();
        doc->IconArray()->UpdateIconsL();

        if ( iActiveView == KFileManagerFoldersViewId )
            {
            CFileManagerFoldersView* view =
                static_cast< CFileManagerFoldersView* >(
                    View( KFileManagerFoldersViewId ) );
            view->HandleResourceChangeL( KAknsMessageSkinChange );
            }
        NotifyViewScreenDeviceChanged();
        }

    CAknViewAppUi::HandleResourceChangeL( aType );
    }

// ------------------------------------------------------------------------------
// CFileManagerAppUi::HandleGainingForeground
//
// ------------------------------------------------------------------------------
//
void CFileManagerAppUi::HandleGainingForeground()
    {
    if ( !iForeGround ) 
        {
        iForeGround = ETrue;

        CFileManagerViewBase* view =
            static_cast< CFileManagerViewBase* >( View( iActiveView ) );

        CFileManagerEngine& engine =
            static_cast< CFileManagerDocument* >( Document() )->Engine();

        engine.SetObserver( view );
        view->NotifyForegroundStatusChange( iForeGround );

        if ( view->CurrentProcess() ==
            MFileManagerProcessObserver::ENoProcess )
            {
            engine.FileSystemEvent( EFalse );
            }
        }
    }

// ------------------------------------------------------------------------------
// CFileManagerAppUi::HandleLosingForeground
//
// ------------------------------------------------------------------------------
//
void CFileManagerAppUi::HandleLosingForeground()
    {
    iForeGround = EFalse;

    CFileManagerViewBase* view =
        static_cast< CFileManagerViewBase* >( View( iActiveView ) );

    CFileManagerEngine& engine =
        static_cast< CFileManagerDocument* >( Document() )->Engine();

    engine.SetObserver( view );
    view->StoreIndex();
    view->NotifyForegroundStatusChange( iForeGround );

    if ( view->CurrentProcess() ==
        MFileManagerProcessObserver::ENoProcess )
        {
        engine.FileSystemEvent( ETrue );
        }
    }

// ------------------------------------------------------------------------------
// CFileManagerAppUi::IsFmgrForeGround
//
// ------------------------------------------------------------------------------
//
TBool CFileManagerAppUi::IsFmgrForeGround()
    {
    return iForeGround;
    }

// -----------------------------------------------------------------------------
// CFileManagerAppUi::HandleScreenDeviceChangedL()
//
// -----------------------------------------------------------------------------
//
void CFileManagerAppUi::HandleScreenDeviceChangedL()
    {
    CAknViewAppUi::HandleScreenDeviceChangedL();
    NotifyViewScreenDeviceChanged();
    }

// -----------------------------------------------------------------------------
// CFileManagerAppUi::HandleError()
//
// -----------------------------------------------------------------------------
//
TErrorHandlerResponse CFileManagerAppUi::HandleError( TInt aError,
        const SExtendedError& aExtErr, TDes& aErrorText, TDes& aContextText )
    {
    ERROR_LOG1( "CFileManagerAppUi::HandleError()-aError=%d", aError )
    switch ( aError )
        {
        case KErrCancel: // FALLTHROUGH
        case KErrNotReady: 
            {
            // Suppress notes for canceled operations.
            // Suppress notes for not ready when caused by hotswap.
            if ( View( iActiveView ) )
                {
                return ENoDisplay;
                }
            break;
            }
        case KErrNotFound: // Fall through
        case KErrPathNotFound:
            {
            CFileManagerViewBase* view =
                static_cast< CFileManagerViewBase* >( View( iActiveView ) );
            TBool handled( EFalse );
            TRAPD( err, handled = view->HandleFileNotFoundL( aError ) );
            if ( err == KErrNone && handled )
                {
                // Suppress not found when handled by refresh.
                return ENoDisplay;
                }
            break;
            }
        default:
            {
            break;
            }
        }
    return CAknViewAppUi::HandleError(
        aError, aExtErr, aErrorText, aContextText );
    }

// -----------------------------------------------------------------------------
// CFileManagerAppUi:ActivateMainViewL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerAppUi::ActivateMainViewL()
    {
    if ( iActiveView != KFileManagerMainViewId )
        {
        CreateAndActivateLocalViewL( KFileManagerMainViewId );
        iSearchViewOpen = EFalse;
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerAppUi:ActivateMemoryStoreViewL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerAppUi::ActivateMemoryStoreViewL()
    {
    if ( iActiveView != KFileManagerMemoryStoreViewId )
        {
        CreateAndActivateLocalViewL( KFileManagerMemoryStoreViewId );
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerAppUi::CloseMemoryStoreViewL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerAppUi::CloseMemoryStoreViewL()
    {
    if ( iSearchViewOpen )
        {
        CreateAndActivateLocalViewL( KFileManagerSearchResultsViewId );
        }
    else 
        {
        ExitEmbeddedAppIfNeededL();
        CreateAndActivateLocalViewL( KFileManagerMainViewId );
                
        //Fix for EDMO-7JMH7V
        //File manager:The phone displays "no data" in File manager 
        //when cancel installing ".sis" file
        iActiveView = KFileManagerMainViewId;
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerAppUi::DriveInfo
// 
// -----------------------------------------------------------------------------
// 
TFileManagerDriveInfo& CFileManagerAppUi::DriveInfo()
    {
    return iDriveInfo;
    }

// -----------------------------------------------------------------------------
// CFileManagerAppUi::ActivateRemoteDriveSettingsViewL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerAppUi::ActivateRemoteDriveSettingsViewL( const TDesC& aDriveName )
    {
    CFileManagerDocument* doc =
        static_cast< CFileManagerDocument* >( Document() );
    CFileManagerFeatureManager& featureManager( doc->Engine().FeatureManager() );

    if ( !featureManager.IsRemoteStorageFwSupported() )
        {
        return;
        }

    if ( !View( KUidGsRsfwPlugin ) )
        {
        TUid plugin_ID_Key;
        TAny* ptr = REComSession::CreateImplementationL(
            KUidGsRsfwPlugin, plugin_ID_Key );
        CAknView* view = reinterpret_cast< CAknView* >( ptr );

        TRAPD( err,
            {
            doc->StorePluginL( plugin_ID_Key );
            AddViewL( view ); // Takes ownership
            } );
        if ( err != KErrNone )
            {
            REComSession::DestroyedImplementation( plugin_ID_Key );
            User::Leave( err );
            }
        }

    if ( aDriveName.Length() )
        {
        // 16bit Unicode conversion to UTF8 takes max 3 bytes per char
        const TInt KUtf8Factor = 3;
        HBufC8* buffer8 = HBufC8::NewLC( aDriveName.Length() * KUtf8Factor );
        TPtr8 ptr8( buffer8->Des() );

        CnvUtfConverter::ConvertFromUnicodeToUtf8( ptr8, aDriveName );
        CreateActivateViewEventL(
            TVwsViewId( KUidFileManager, KUidGsRsfwPlugin ),
            KUidGsRsFwSettingsViewId,
            *buffer8 );
        CleanupStack::PopAndDestroy( buffer8 );
        }
    else
        {
        CreateActivateViewEventL(
            TVwsViewId( KUidFileManager, KUidGsRsfwPlugin ),
            KUidGsRsFwSettingsViewId,
            KNullDesC8() );
        }
    }

#ifdef RD_FILE_MANAGER_BACKUP
// -----------------------------------------------------------------------------
// CFileManagerAppUi:ActivateBackupViewL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerAppUi::ActivateBackupViewL()
    {
    if ( iActiveView != KFileManagerBackupViewId )
        {
        CAknView* view = View( KFileManagerBackupViewId );
        if ( !view )
            {
            view = CFileManagerBackupView::NewLC();
            AddViewL( view ); // Takes ownership
            CleanupStack::Pop( view );
            }
        CreateAndActivateLocalViewL( KFileManagerBackupViewId );
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerAppUi:CloseBackupViewL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerAppUi::CloseBackupViewL()
    {
    CreateAndActivateLocalViewL( KFileManagerMainViewId );
    }

// -----------------------------------------------------------------------------
// CFileManagerAppUi:ActivateRestoreViewL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerAppUi::ActivateRestoreViewL( TBool aDeleteBackup )
    {
    if ( iActiveView != KFileManagerRestoreViewId )
        {
        if ( aDeleteBackup )
            {
            CreateAndActivateLocalViewL(
                KFileManagerRestoreViewId,
                CFileManagerRestoreView::KDeleteBackups,
                KNullDesC8 );
            }
        else
            {
            CreateAndActivateLocalViewL( KFileManagerRestoreViewId );
            }
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerAppUi:CloseRestoreViewL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerAppUi::CloseRestoreViewL()
    {
    CreateAndActivateLocalViewL( KFileManagerBackupViewId );
    }

// -----------------------------------------------------------------------------
// CFileManagerAppUi::ProcessCommandParametersL
// 
// -----------------------------------------------------------------------------
// 
TBool CFileManagerAppUi::ProcessCommandParametersL(
        TApaCommand aCommand,
        TFileName& aDocumentName,
        const TDesC8& aTail )
    {
    switch( aCommand )
        {
        case EApaCommandOpen:
            {
            TLex8 lex8( aTail );
            TInt32 sortMode( 0 );
            if ( lex8.Val( sortMode, EDecimal ) != KErrNone )
                {
                sortMode = 0; // Use the default on error
                }
            SetFolderToOpenAtStartup( aDocumentName, sortMode );
            break;
            }
        default:
            {
            break;
            }
        }
    aDocumentName.Zero();
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CFileManagerAppUi:ProcessMessageL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerAppUi::ProcessMessageL(
        TUid /*aUid*/, const TDesC8& aParams )
    {
    if ( aParams.Length() )
        {
        // Allow the backup only if scheduled backup starter exists
        const TSecureId KFileManagerSchBkupUid( KFileManagerSchBkupUID3 );
        if( ProcessExists( KFileManagerSchBkupUid ) )
            {
            HBufC* cmd = HBufC::NewLC( aParams.Length() );
            TPtr ptr( cmd->Des() );
            ptr.Copy( aParams );
            if ( !ptr.Compare( KSchBackupTaskName ) )
                {
                StartSchBackupL();
                }
            CleanupStack::PopAndDestroy( cmd );
            }
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerAppUi:StartSchBackupL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerAppUi::StartSchBackupL()
    {
    CFileManagerViewBase* view =
        static_cast< CFileManagerViewBase* >( View( iActiveView ) );
    view->StartSchBackupL();
    }

// -----------------------------------------------------------------------------
// CFileManagerAppUi::TaskSchedulerL
// 
// -----------------------------------------------------------------------------
//
CFileManagerTaskScheduler& CFileManagerAppUi::TaskSchedulerL()
    {
    if ( !iTaskScheduler )
        {
        CFileManagerDocument* doc =
            static_cast< CFileManagerDocument* >( Document() );
        iTaskScheduler = CFileManagerTaskScheduler::NewL(
            doc->Engine() );
        }
    return *iTaskScheduler;
    }

// -----------------------------------------------------------------------------
// CFileManagerAppUi::SchBackupHandlerL
// 
// -----------------------------------------------------------------------------
//
CFileManagerSchBackupHandler& CFileManagerAppUi::SchBackupHandlerL()
    {
    if ( !iSchBackupHandler )
        {
        CFileManagerDocument* doc =
            static_cast< CFileManagerDocument* >( Document() );
        iSchBackupHandler = CFileManagerSchBackupHandler::NewL(
            doc->Engine() );
        iSchBackupHandler->SetObserver( this );
        }
    return *iSchBackupHandler;
    }

#endif // RD_FILE_MANAGER_BACKUP

// -----------------------------------------------------------------------------
// CFileManagerAppUi::StartIRReceiveL()
//
// -----------------------------------------------------------------------------
//
void CFileManagerAppUi::StartIRReceiveL(
        MFileManagerProcessObserver& aObserver )
    {
    FUNC_LOG

    delete iIRDeleteCB;
    iIRDeleteCB = NULL;

    delete iIRReceiver;
    iIRReceiver = NULL;

    CFileManagerEngine& engine = 
        static_cast< CFileManagerDocument* >( Document() )->Engine();
    iIRReceiver = CFileManagerIRReceiver::NewL(
        aObserver,
        engine.CurrentDirectory(),
        engine );
    iIRReceiver->ReceiveFileL();
    }

// -----------------------------------------------------------------------------
// CFileManagerAppUi::StopIRReceive()
//
// -----------------------------------------------------------------------------
//
void CFileManagerAppUi::StopIRReceive()
    {
    FUNC_LOG

    if ( iIRReceiver )
        {
        iIRReceiver->StopReceiving();

        delete iIRDeleteCB;
        iIRDeleteCB = NULL;

        iIRDeleteCB = new CAsyncCallBack(
            TCallBack( DeleteIRReceiveCB, this ),
            CActive::EPriorityStandard );
        if ( iIRDeleteCB )
            {
            iIRDeleteCB->CallBack();
            }
        }
    }

// ------------------------------------------------------------------------------
// CFileManagerAppUi::DeleteIRReceiveCB
//
// ------------------------------------------------------------------------------
//
TInt CFileManagerAppUi::DeleteIRReceiveCB( TAny* aPtr )
    {
    FUNC_LOG
        
    CFileManagerAppUi* self = static_cast< CFileManagerAppUi* >( aPtr );

    delete self->iIRReceiver;
    self->iIRReceiver = NULL;

    delete self->iIRDeleteCB;
    self->iIRDeleteCB = NULL;

    return KErrNone;
    }

// ------------------------------------------------------------------------------
// CFileManagerAppUi::NotifyViewScreenDeviceChanged
//
// ------------------------------------------------------------------------------
//
void CFileManagerAppUi::NotifyViewScreenDeviceChanged()
    {
    CAknView* activeView = View( iActiveView );
    if ( activeView )
        {
        CFileManagerViewBase* view =
            static_cast< CFileManagerViewBase* > ( activeView );
        view->ScreenDeviceChanged();
        }
    }

// ------------------------------------------------------------------------------
// CFileManagerAppUi::SetTitleL
//
// ------------------------------------------------------------------------------
//
void CFileManagerAppUi::SetTitleL( const TDesC& aTitle )
    {
    iTitlePane->SetTextL( aTitle );
    }

// ------------------------------------------------------------------------------
// CFileManagerAppUi::SetTitleL
//
// ------------------------------------------------------------------------------
//
void CFileManagerAppUi::SetTitleL( const TInt aTitle )
    {
    HBufC* title = StringLoader::LoadLC( aTitle );
    iTitlePane->SetTextL( *title );
    CleanupStack::PopAndDestroy( title );
    }

// ------------------------------------------------------------------------------
// CFileManagerAppUi::ScheduledBackupFinishedL
//
// ------------------------------------------------------------------------------
// 
#ifdef RD_FILE_MANAGER_BACKUP
void CFileManagerAppUi::SchBackupFinishedL( TInt /*aError*/ )
    {
    CFileManagerViewBase* view =
        static_cast< CFileManagerViewBase* >( View( iActiveView ) );
    if ( view->CurrentProcess() == MFileManagerProcessObserver::ESchBackupProcess )
        {
        view->SchBackupFinishedL();
        }
    if ( iSchBackupStandalone )
        {
        ProcessCommandL( EAknCmdExit );
        }
    }
#else // RD_FILE_MANAGER_BACKUP
void CFileManagerAppUi::SchBackupFinishedL( TInt /*aError*/ )
    {
    }
#endif // RD_FILE_MANAGER_BACKUP


// ------------------------------------------------------------------------------
// CFileManagerAppUi::IsFileManagerView
//
// ------------------------------------------------------------------------------
// 
TBool CFileManagerAppUi::IsFileManagerView( const TUid aViewUid )
    {
    return ( aViewUid == KFileManagerFoldersViewId ||
             aViewUid == KFileManagerSearchResultsViewId ||
             aViewUid == KFileManagerMainViewId ||
             aViewUid == KFileManagerMemoryStoreViewId ||
             aViewUid == KFileManagerBackupViewId ||
             aViewUid == KFileManagerRestoreViewId );
    }

// ------------------------------------------------------------------------------
// CFileManagerAppUi::NotifyCbaUpdate
//
// ------------------------------------------------------------------------------
//
void CFileManagerAppUi::NotifyCbaUpdate()
    {
    CAknView* activeView = View( iActiveView );
    if ( activeView )
        {
        CFileManagerViewBase* view =
            static_cast< CFileManagerViewBase* > ( activeView );
        TRAP_IGNORE( view->UpdateCbaL() );
        }
    }

// ------------------------------------------------------------------------------
// CFileManagerAppUi::SetFolderToOpenAtStartup
//
// ------------------------------------------------------------------------------
//
void CFileManagerAppUi::SetFolderToOpenAtStartup(
        const TDesC& aFullPath, TInt aSortMode )
    {
    FUNC_LOG

    INFO_LOG2(
        "CFileManagerAppUi::SetFolderToOpenAtStartup-%S-%d",
        &aFullPath, aSortMode )

    // Validate given path
    _LIT( KWild1, "*" );
    _LIT( KWild2, "?" );
    const TInt KWildLen = 1;
    CFileManagerEngine& engine =
        static_cast< CFileManagerDocument* >( Document() )->Engine();
    RFs& fs( iEikonEnv->FsSession() );
    TInt err( KErrNone );
    if ( !aFullPath.Length() ||
         !aFullPath.Left( KWildLen ).CompareF( KWild1 ) ||
         !aFullPath.Left( KWildLen ).CompareF( KWild2 ) )
        {
        err = KErrBadName; // Ignore bad drives
        }
    else
        {
        TParsePtrC parse( aFullPath );
        if ( !parse.DrivePresent() || !parse.PathPresent() )
            {
            err = KErrBadName; // Ignore bad paths
            }
        TUint driveStatus( 0 );
        if ( err == KErrNone )
            {
            TInt drive = TDriveUnit( aFullPath );
            err = DriveInfo::GetDriveStatus( fs, drive, driveStatus );
            if ( err != KErrNone ||
                !( driveStatus & DriveInfo::EDriveUserVisible ) )
                {
                err = KErrAccessDenied; // Ignore hidden drives
                }
            else
                {
                TFileName rootPath;
                err = PathInfo::GetRootPath( rootPath, drive );
                if ( err == KErrNone &&
                     rootPath.CompareF( aFullPath.Left( rootPath.Length() ) ) )
                    {
                    err = KErrAccessDenied; // Ignore paths below root level
                    }
                }
            }
        if ( err == KErrNone &&
             !( driveStatus & DriveInfo::EDriveRemote ) )
            {
            if ( engine.IsSystemFolder( parse.DriveAndPath() ) )
                {
                err = KErrAccessDenied; // Ignore system folders
                }
            else if ( !( driveStatus & DriveInfo::EDriveReadOnly ) )
                {
                TRAP( err, BaflUtils::EnsurePathExistsL(
                    fs, parse.DriveAndPath() ) );
                }
            }
        if ( err == KErrNone )
            {
            iFolderToOpenAtStartup.Copy( parse.DriveAndPath() );
            engine.SetSortMethod(
                static_cast< CFileManagerEngine::TSortMethod >( aSortMode ) );
            engine.SetCurrentItemName(parse.NameAndExt());
            TRAP( err, OpenFolderViewAtStartupL() );
            }
        }
    if ( err != KErrNone )
        {
        ERROR_LOG1( "CFileManagerAppUi::SetFolderToOpenAtStartup-err=%d", err )
        iWaitingForParams = EFalse;

        // Refresh only if embedded app
        if ( engine.FeatureManager().IsEmbedded() )
            {
            engine.RefreshDirectory();
            }
        }
    }

// ------------------------------------------------------------------------------
// CFileManagerAppUi::ProcessAiwParamListL
//
// ------------------------------------------------------------------------------
//
void CFileManagerAppUi::ProcessAiwParamListL(
        const CAiwGenericParamList& aInParams )
    {
    FUNC_LOG

    iWaitingForParams = EFalse;

    TPtrC folder( KNullDesC );
    TInt folderSortMode( KErrNotFound );
    TInt i( 0 );
    // Get folder path
    const TAiwGenericParam* param = aInParams.FindFirst(
        i, EGenericParamDir, EVariantTypeDesC );
    if ( i != KErrNotFound && param )
        {
        folder.Set( param->Value().AsDes() );
        }
    // Get folder sort mode
    param = aInParams.FindFirst(
        i, EGenericParamDir, EVariantTypeTInt32 );
    if ( i != KErrNotFound && param )
        {
        folderSortMode = param->Value().AsTInt32();
        }
    SetFolderToOpenAtStartup( folder, folderSortMode );
    }

// ------------------------------------------------------------------------------
// CFileManagerAppUi::OpenFolderViewAtStartupL
//
// ------------------------------------------------------------------------------
//
void CFileManagerAppUi::OpenFolderViewAtStartupL()
    {
    FUNC_LOG

    CFileManagerEngine& engine =
        static_cast< CFileManagerDocument* >( Document() )->Engine();
    engine.SetDirectoryWithBackstepsL( iFolderToOpenAtStartup );
    if( engine.NavigationLevel() <= 0 )
        {
        ActivateMemoryStoreViewL();
        // Ensure that view change gets updated when used during contruction
        iActiveView = KFileManagerMemoryStoreViewId;
        }
    else
        {
        ActivateFoldersViewL();
        // Ensure that view change gets updated when used during contruction
        iActiveView = KFileManagerFoldersViewId;
        }
    // Store initial folder level when embedded
    if ( engine.FeatureManager().IsEmbedded() )
        {
        iInitialFolderLevel = engine.FolderLevel();
        }
    }

// ------------------------------------------------------------------------------
// CFileManagerAppUi::ExitEmbeddedAppIfNeededL
//
// ------------------------------------------------------------------------------
//
void CFileManagerAppUi::ExitEmbeddedAppIfNeededL()
    {
    FUNC_LOG

    CFileManagerEngine& engine =
        static_cast< CFileManagerDocument* >( Document() )->Engine();
    // Exit embedded app if the initial folder level is crossed
    if ( engine.FeatureManager().IsEmbedded() &&
         engine.FolderLevel() < iInitialFolderLevel )
        {
        ProcessCommandL( EAknCmdExit );
        User::Leave( KErrCancel ); // To abort current operation
        }
    }

// ------------------------------------------------------------------------------
// CFileManagerAppUi::WaitingForInputParams
//
// ------------------------------------------------------------------------------
//
TBool CFileManagerAppUi::WaitingForInputParams() const
    {
    return iWaitingForParams;
    }

// ------------------------------------------------------------------------------
// CFileManagerAppUi::CreateAndActivateLocalViewL
//
// ------------------------------------------------------------------------------
//
void CFileManagerAppUi::CreateAndActivateLocalViewL(
        TUid aViewId, TUid aCustomMessageId, const TDesC8& aCustomMessage )
    {
    FUNC_LOG

    CAknView* view = View( aViewId );
    if ( !view )
        {
        if ( aViewId == KFileManagerMainViewId )
            {
            view = CFileManagerMainView::NewLC();
            }
        else if ( aViewId == KFileManagerMemoryStoreViewId )
            {
            view = CFileManagerMemoryStoreView::NewLC();
            }
        else if ( aViewId == KFileManagerFoldersViewId )
            {
            view = CFileManagerFoldersView::NewLC();
            }
        else if ( aViewId == KFileManagerSearchResultsViewId )
            {
            view = CFileManagerSearchResultsView::NewLC();
            }
        else if ( aViewId == KFileManagerBackupViewId )
            {
            view = CFileManagerBackupView::NewLC();
            }
        else if ( aViewId == KFileManagerRestoreViewId )
            {
            view = CFileManagerRestoreView::NewLC();
            }
        else
            {
            ERROR_LOG1(
                "CFileManagerAppUi::CreateAndActivateLocalViewL-InvalidView=0x%x",
                aViewId.iUid )
            User::Leave( KErrNotFound );
            }
        AddViewL( view ); // Takes ownership
        CleanupStack::Pop( view );
        }
    ActivateLocalViewL( aViewId, aCustomMessageId, aCustomMessage );
    }

// ------------------------------------------------------------------------------
// CFileManagerAppUi::CreateAndActivateLocalViewL
//
// ------------------------------------------------------------------------------
//
void CFileManagerAppUi::CreateAndActivateLocalViewL( TUid aViewId )
    {
    FUNC_LOG

    CreateAndActivateLocalViewL( aViewId, KNullUid, KNullDesC8 );
    }

// ------------------------------------------------------------------------------
// CFileManagerAppUi::IsSearchViewOpen
//
// ------------------------------------------------------------------------------
//
TBool CFileManagerAppUi::IsSearchViewOpen() const
    {
    return iSearchViewOpen;
    }
// ------------------------------------------------------------------------------
// CFileManagerAppUi::BackupOrRestoreStarted
//
// ------------------------------------------------------------------------------
//
void CFileManagerAppUi::BackupOrRestoreStarted()
    {
    iManualBackupOrRestoreStarted.HomeTime();
    }

// ------------------------------------------------------------------------------
// CFileManagerAppUi::BackupOrRestoreEnded
//
// ------------------------------------------------------------------------------
//
void CFileManagerAppUi::BackupOrRestoreEnded()
    {
    iManualBackupOrRestoreEnded.HomeTime();
    }

// ------------------------------------------------------------------------------
// CFileManagerAppUi::BackupOrRestoreStartTime
//
// ------------------------------------------------------------------------------
//
TTime CFileManagerAppUi::BackupOrRestoreStartTime() const
    {
    return iManualBackupOrRestoreStarted;
    }

// ------------------------------------------------------------------------------
// CFileManagerAppUi::BackupOrRestoreEndTime
//
// ------------------------------------------------------------------------------
//
TTime CFileManagerAppUi::BackupOrRestoreEndTime() const
    {
    return iManualBackupOrRestoreEnded;
    }
// ------------------------------------------------------------------------------
// CFileManagerAppUi::ResetBackupOrRestoreEndTime
//
// ------------------------------------------------------------------------------
//
void CFileManagerAppUi::ResetBackupOrRestoreEndTime()
    {
    iManualBackupOrRestoreStarted = 0;
    iManualBackupOrRestoreEnded = 0;
    }

// End of File  

