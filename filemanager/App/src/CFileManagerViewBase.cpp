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
* Description:  Base class for all file manager views
*
*/



// INCLUDE FILES
#include <aknlists.h>
#include <eikmenup.h> // CEikMenuPane
#include <eikmenub.h> // CEikMenuBar
#include <StringLoader.h>
#include <AknCommonDialogs.h> // Common File Dialogs
#include <CAknMemorySelectionDialog.h>
#include <CAknFileSelectionDialog.h>
#include <sendui.h>
#include <Sendnorm.rsg>
#include <SenduiMtmUids.h>
#include <AknProgressDialog.h>
#include <eikprogi.h>
#include <AknWaitNoteWrapper.h>
#include <aknnotewrappers.h>
#include <AknWaitDialog.h>
#include <cmemstatepopup.h>
#include <f32file.h>
#include <aknmessagequerydialog.h>
#include <CMessageData.h>
#include <DRMHelper.h>
#include <bautils.h>
#include <AknCommonDialogsDynMem.h>
#include "CFileManagerViewBase.h"
#include "CFileManagerContainerBase.h"
#include "CFileManagerAppUi.h"
#include "CFileManagerDocument.h"
#include "CFileManagerFileSelectionFilter.h"
#include "FileManager.hrh"
#ifdef RD_FILE_MANAGER_BACKUP
 #include "CFileManagerSchBackupHandler.h"
 #include "CFileManagerBackupSettings.h"
 #include "CFileManagerTaskScheduler.h"
#endif // RD_FILE_MANAGER_BACKUP
#include <CFileManagerEngine.h>
#include <CFileManagerUtils.h>
#include <CFileManagerCommonDefinitions.h>
#include <CFileManagerItemProperties.h>
#include <CFileManagerActiveExecute.h>
#include <Cfilemanageractivedelete.h>
#include <FileManager.rsg>
#include <FileManagerView.rsg>
#include <FileManagerDebug.h>
#include <FileManagerDlgUtils.h>
#include <CFileManagerFeatureManager.h>
#include <FileManagerPrivateCRKeys.h>
#include <DataSyncInternalPSKeys.h>
#include <connect/sbdefs.h>
#include <e32property.h>
#include <caf/caf.h>
#include <drmagents.h>

using namespace conn;

// CONSTANTS
const TUint KMessageSize = 1024;
const TUint KMaxPercentage = 100;
const TUint KProgressBarUpdateInterval = 1000000; // microseconds
const TUint KDriveLetterSize = 1;
const TUint KRefreshProgressStartDelay = 1000000; // microseconds
const TInt KFmgrMSK = 3;
const TInt KEstimateUpperLimit = 90; // User selectable continuation
const TInt KEstimateLowerLimit = 10; // Backup will be interrupted
const TUint32 KDefaultFolderMask = CFileManagerItemProperties::EFolder |
                                  CFileManagerItemProperties::EDefault;
const TInt64 KMSecToMicroSecMultiplier = 1000000;
const TInt64 KMinToMicroSecMultiplier = KMSecToMicroSecMultiplier * 60;
const TInt64 KHourToMicroSecMultiplier = KMinToMicroSecMultiplier * 60;
const TUint KProgressBarAsyncStartDelay = 1500000; // microseconds
const TInt KMinificationFactor = 1024; // Used to zoom processbar's proportion

// ============================ LOCAL FUNCTIONS ================================

// -----------------------------------------------------------------------------
// IsWmDrmFile
// 
// -----------------------------------------------------------------------------
// 
static TBool IsWmDrmFile( const TDesC& aFullPath )
    {
    TBool ret( EFalse );
    ContentAccess::CContent* content = NULL;
    TRAPD( err, content = ContentAccess::CContent::CContent::NewL(
        aFullPath, EContentShareReadWrite ) );
    if ( err != KErrNone )
        {
        TRAP( err, content = ContentAccess::CContent::CContent::NewL(
            aFullPath, EContentShareReadOnly ) );
        }
    if ( err == KErrNone )
        {
        TInt isProtected( 0 );
        err = content->GetAttribute( EIsProtected, isProtected );
        if ( err == KErrNone && isProtected )
            {
            TInt fileType( 0 );
            err = content->GetAttribute( DRM::EDrmFileType, fileType );
            ret = ( err == KErrNone && fileType == DRM::EDrmWMFile );
            }
        delete content;
        }
    return ret;
    }

// -----------------------------------------------------------------------------
// Int64ToInt
// 
// -----------------------------------------------------------------------------
// 
static TInt Int64ToInt( const TInt64& aInt64 )
    {
    if ( aInt64 > KMaxTInt )
        {
        return KMaxTInt;
        }
    return I64INT( aInt64 );
    }

// -----------------------------------------------------------------------------
// EmptyPwd
// 
// -----------------------------------------------------------------------------
// 
//static void EmptyPwd( TDes& aPwd )
//    {
//    aPwd.FillZ( aPwd.MaxLength( ) );
//    aPwd.Zero();
//    }

// -----------------------------------------------------------------------------
// ConvertCharsToPwd
// 
// -----------------------------------------------------------------------------
// 
//static void ConvertCharsToPwd( const TDesC& aWord, TDes8& aConverted )
//    {
//    // Make sure the target password is empty ( can't use the function here )
//    aConverted.FillZ( aConverted.MaxLength() );
//    aConverted.Zero();
//    TInt size( aWord.Size() );
//    if ( size )
//        {
//        if ( size > aConverted.MaxLength() )
//            {
//            size = aConverted.MaxLength();
//            }
//        aConverted.Copy( (TUint8*)aWord.Ptr(), size );
//        }
//    }

// -----------------------------------------------------------------------------
// IsSystemProcess
// 
// -----------------------------------------------------------------------------
// 
static TBool IsSystemProcess(
        MFileManagerProcessObserver::TFileManagerProcess aProcess )
    {
    switch ( aProcess )
        {
        case MFileManagerProcessObserver::EFormatProcess:
        case MFileManagerProcessObserver::EBackupProcess:
        case MFileManagerProcessObserver::ERestoreProcess:
        case MFileManagerProcessObserver::ESchBackupProcess:
            {
            return ETrue;
            }
        default:
            {
            break;
            }
        }
    return EFalse;
    }

// -----------------------------------------------------------------------------
// GetDeleteQueryPromptLC
// Chooses correct string for the delete note
// -----------------------------------------------------------------------------
// 
static HBufC* GetDeleteQueryPromptLC( CFileManagerItemProperties& aProp, TInt aCount )
    {

    HBufC* prompt = NULL;
    if ( aCount == 0 && aProp.ContainsAnyFilesOrFolders() )
        {
        prompt = StringLoader::LoadLC( R_QTN_FLDR_DEL_FULL_FLDRS_QUERY );
        }
    else if ( aCount <= 1 )
        {
        prompt = StringLoader::LoadLC( R_QTN_QUERY_COMMON_CONF_DELETE, aProp.NameAndExt() );
        }
    else // aCount > 1
        {
        prompt = StringLoader::LoadLC( R_QTN_FLDR_DEL_ITEMS_QUERY, aCount );
        }

    return prompt;
    }

// -----------------------------------------------------------------------------
// MinIndex
// 
// -----------------------------------------------------------------------------
// 
static TInt MinIndex( CArrayFixFlat<TInt>& aIndexArray )
    {
    TInt count( aIndexArray.Count() );
    if ( !count )
        {
        return 0;
        }
    // Find min index
    TInt index( 0 );
    TInt i( 0 );
    TInt ret( aIndexArray.At( i ) );
    ++i;
    for( ; i < count; ++i )
        {
        index = aIndexArray.At( i );
        if ( index < ret )
            {
            ret = index;
            }
        }
    return ret;
    }

// -----------------------------------------------------------------------------
// SetCurrentYearMonthAndDay
// 
// -----------------------------------------------------------------------------
// 
static TTime SetCurrentYearMonthAndDay( const TTime& aTime )
    {    
    TTime timeNow;
    timeNow.HomeTime();
    TDateTime dateTimeNow( timeNow.DateTime() );
    TInt64 ret( timeNow.Int64() );
    // Replace hours, minutes and seconds using given ones.
    ret -= static_cast< TInt64 >( dateTimeNow.Hour() ) * KHourToMicroSecMultiplier;
    ret -= static_cast< TInt64 >( dateTimeNow.Minute() ) * KMinToMicroSecMultiplier;
    ret -= static_cast< TInt64 >( dateTimeNow.Second() ) * KMSecToMicroSecMultiplier;
    ret -= dateTimeNow.MicroSecond();
    TDateTime dateTime( aTime.DateTime() );
    ret += static_cast< TInt64 >( dateTime.Hour() ) * KHourToMicroSecMultiplier;
    ret += static_cast< TInt64 >( dateTime.Minute() ) * KMinToMicroSecMultiplier;
    ret += static_cast< TInt64 >( dateTime.Second() ) * KMSecToMicroSecMultiplier;
    return ret;
    }

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CFileManagerViewBase::CFileManagerViewBase
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CFileManagerViewBase::CFileManagerViewBase() :
    iEngine( static_cast< CFileManagerDocument* >( AppUi()->Document() )->Engine() )
    {
    }


// -----------------------------------------------------------------------------
// CFileManagerViewBase::~CFileManagerViewBase
// Destructor
// -----------------------------------------------------------------------------
// 
CFileManagerViewBase::~CFileManagerViewBase()
    {
    delete iWaitNoteWrapper;
    delete iActiveDelete;
    delete iPeriodic;
    delete iMarkedArray;
    delete iContainer;
    delete iActiveExec;
    delete iRefreshProgressDelayedStart;
    delete iEjectQueryDialog;
    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::GetSendFilesLC
// 
// -----------------------------------------------------------------------------
// 
CArrayFixFlat<TInt>* CFileManagerViewBase::GetSendFilesLC( TInt& aSize )
    {
    // Get index array and remove folders and play lists
    CArrayFixFlat< TInt >* ret = MarkedArrayLC();
    TInt i( ret->Count() );
    while ( i > 0 )
        {
        --i;
        // IconIdL() is slow if the icon is not cached yet.
        // However, it is faster than FileTypeL().
        switch ( iEngine.IconIdL( ret->At( i ) ) )
            {
            case EFileManagerFolderIcon: // FALLTHROUGH
            case EFileManagerFolderSubIcon: // FALLTHROUGH
            case EFileManagerFolderEmptyIcon: // FALLTHROUGH
            case EFileManagerPlaylistFileIcon:
                {
                ret->Delete( i );
                break;
                }
            default:
                {
                break;
                }
            }
        }
    aSize = Int64ToInt( iEngine.GetFileSizesL( *ret ) );
    return ret;
    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::ConstructL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerViewBase::ConstructL( TInt aResId )
    {
    BaseConstructL( aResId );
    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::HandleCommandL
// 
// -----------------------------------------------------------------------------
//
void CFileManagerViewBase::HandleCommandL( TInt aCommand )
    {
    if ( !iContainer ) return;

    TBool updateCba( !iContainer->SelectionModeEnabled() );

    switch( aCommand )
        {
        case EFileManagerOpen:
            {
            CmdOpenL();
            break;
            }
        case EFileManagerDelete:
            {
            CmdDeleteL();
            break;
            }
        case EFileManagerMoveToFolder:
            {
            CmdMoveToFolderL();
            break;
            }
        case EFileManagerCopyToFolder:
            {
            CmdCopyToFolderL();
            break;
            }
        case EFileManagerNewFolder:
            {
            CmdNewFolderL();
            break;
            }
        case EFileManagerMarkOne:   // FALLTHROUGH
        case EFileManagerUnmarkOne: // FALLTHROUGH
        case EFileManagerToggleMark:
            {
            CmdToggleMarkL();
            break;
            }
        case EFileManagerMarkAll:
            {
            CmdMarkAllL();
            break;
            }
        case EFileManagerUnmarkAll:
            {
            CmdUnmarkAllL();
            break;
            }
        case EFileManagerRename:
            {
            CmdRenameL();
            break;
            }
        case EFileManagerFindFile:
            {
            CmdFindL();
            break;
            }
        case EFileManagerFileDetails: // FALLTHROUGH
        case EFileManagerFolderDetails: // FALLTHROUGH
        case EFileManagerViewInfo:
            {
            CmdViewInfoL();
            break;
            }
//        case EFileManagerMemoryState:
//            {
//            CmdMemoryStateL();
//            break;
//            }
        case EFileManagerReceiveViaIR:
            {
            CmdReceiveViaIRL();
            break;
            }
        case EFileManagerCheckMark: // Suppress
            {
            break;
            }
        case EAknSoftkeyContextOptions: // FALLTHROUGH
        case EFileManagerSelectionKey:
            {
            TInt count( iContainer->ListBoxSelectionIndexesCount() );
            if ( !count )
                {
                HandleCommandL( EFileManagerOpen );
                }
            else if ( count > 0 )
                {
                ShowContextSensitiveMenuL();
                }
            break;
            }
        case EFileManagerSend:
            {
            if ( !iSendUiPopupOpened )
            {
                SendUiQueryL();
            }
            break;
            }
        case EFileManagerMoreInfoOnline:
            {
            OpenInfoUrlL( iContainer->ListBoxCurrentItemIndex() );
            break;
            }
        case EFileManagerUnlockMemoryCard:
            {
            CmdUnlockDriveL();
            break;
            }
//        case EFileManagerMemoryCardName:
//        case EFileManagerMemoryCardRename: // Fall through
//            {
//            CmdRenameDriveL();
//            break;
//            }
        //case EFileManagerMemoryCardFormat:
        case EFileManagerMemoryStorageFormat:
        case EFileManagerFormatMassStorage: // Fall through
            {
            CmdFormatDriveL();
            break;
            }
//        case EFileManagerMemoryCardPasswordSet:
//            {
//            CmdSetDrivePasswordL();
//            break;
//            }
//        case EFileManagerMemoryCardPasswordChange:
//            {
//            CmdChangeDrivePasswordL();
//            break;
//            }
//        case EFileManagerMemoryCardPasswordRemove:
//            {
//            CmdRemoveDrivePasswordL();
//            break;
//            }
//        case EFileManagerMemoryCardDetails:
//            {
//            CmdMemoryCardDetailsL();
//            break;
//            }
        case EFileManagerConnectRemoveDrive:
            {
            SetRemoteDriveConnectionStateL( ETrue );
            break;
            }
        case EFileManagerDisconnectRemoveDrive:
            {
            SetRemoteDriveConnectionStateL( EFalse );
            break;
            }
        case EFileManagerRefreshRemoteDrive:
            {
            CmdRefreshDirectoryL();
            break;
            }
	    case EFileManagerSortByName:
	    case EFileManagerSortByType: // Fall through
	    case EFileManagerSortMostRecentFirst: // Fall through
	    case EFileManagerSortLargestFirst: // Fall through
	    case EFileManagerSortByMatch: // Fall through
            {
            CmdSortL( aCommand );
            break;
            }
        default:
            {
            AppUi()->HandleCommandL( aCommand );
            break;
            }
        }

    if ( updateCba )
        {
        UpdateCbaL();
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::SendUiQueryL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerViewBase::SendUiQueryL()
    {
    //iSendUiPopupOpened = ETrue;	
    	
    CSendUi& sendUi( static_cast< CFileManagerAppUi* >( AppUi() )->SendUiL() );
    CMessageData* msgData = CMessageData::NewL();
    CleanupStack::PushL( msgData );
    TInt msgSize( KMessageSize );
    CArrayFixFlat< TInt >* files = GetSendFilesLC( msgSize );
    TInt count( files->Count() );
    if ( count )
        {
        // Set dimmed services specified for FileManager by Send UI spec
        const TInt KDimmedServices = 4;
        CArrayFixFlat< TUid >* servicesToDim =
            new ( ELeave ) CArrayFixFlat< TUid >( KDimmedServices );
        CleanupStack::PushL( servicesToDim );

        servicesToDim->AppendL( KSenduiMtmAudioMessageUid );
        servicesToDim->AppendL( KMmsDirectUpload );
        servicesToDim->AppendL( KMmsIndirectUpload );
        servicesToDim->AppendL( KSenduiMtmPostcardUid );

        TSendingCapabilities caps(
            0, msgSize, TSendingCapabilities::ESupportsAttachments );
        for( TInt i( 0 ); i < count ; i++ )
            {
            HBufC* fullPath = iEngine.IndexToFullPathLC( files->At( i ) );
            msgData->AppendAttachmentL( *fullPath );
            CleanupStack::PopAndDestroy( fullPath );
            }
        // Let SendUi handle protected files, queries and filtering
        sendUi.ShowQueryAndSendL( msgData, caps, servicesToDim );
        CleanupStack::PopAndDestroy( servicesToDim );
        }
    CleanupStack::PopAndDestroy( files );
    CleanupStack::PopAndDestroy( msgData );
    
    iSendUiPopupOpened = EFalse;
    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::MarkMenuFilteringL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerViewBase::MarkMenuFilteringL( CEikMenuPane& aMenuPane )
    {
    TInt index( iContainer->ListBoxCurrentItemIndex() );

    if ( iEngine.IsFolder( index ) )
        {
        aMenuPane.SetItemDimmed( EFileManagerMarkOne, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerUnmarkOne, ETrue );
        }
    else
        {
        if ( iContainer->ListBoxIsItemSelected( index ) )
            {
            aMenuPane.SetItemDimmed( EFileManagerMarkOne, ETrue );
            }
        else
            {
            aMenuPane.SetItemDimmed( EFileManagerUnmarkOne, ETrue );
            }
        }
    TInt files( 0 );
    if( iContainer->IsSearchFieldVisible() )
        {
        files = FilesCountInSearchField();
        }
    else
        {
        files = iEngine.FilesInFolderL();
        }  
    TInt count( iContainer->ListBoxSelectionIndexesCount() );
    if ( count == files )
        {
        aMenuPane.SetItemDimmed( EFileManagerMarkAll, ETrue );
        }

    if ( !count )
        {
        aMenuPane.SetItemDimmed( EFileManagerUnmarkAll, ETrue );
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::CmdOpenL
// 
// -----------------------------------------------------------------------------
// 
CFileManagerViewBase::TFileManagerOpenResult CFileManagerViewBase::CmdOpenL()
    {
    if ( !iContainer || iActiveProcess != ENoProcess )
        {
        return EOpenError; // Ignore to avoid mess up
        }
    TInt index( iContainer->ListBoxCurrentItemIndex() );
    TInt err( KErrNone );

    if ( index < 0 )
        {
        return EOpenError;
        }
    CFileManagerAppUi* appUi = 
        static_cast< CFileManagerAppUi* >( AppUi() );
    TBool isFolder( iEngine.IsFolder( index ) );
    StoreIndex();
    TRAP( err, iEngine.OpenL( index ) );
    if ( err == KErrNone )
        {
        if ( isFolder )
            {
            if ( !appUi->ActivateFoldersViewL() )
                {
                // Folders view is already open
                // Refresh if this view is folders view
                if ( Id() == CFileManagerAppUi::KFileManagerFoldersViewId )
                    {
                    iEngine.SetObserver( this );
                    iEngine.RefreshDirectory();
                    }
                }
            return EFolderOpened;
            }
        return EFileOpened;
        }
    if ( !HandleFileNotFoundL( err ) )
        {
        if ( !isFolder )
            {
            FileManagerDlgUtils::ShowErrorNoteL(
                R_QTN_FMGR_ERROR_CANT_OPEN );
            }
        }
    return EOpenError;
    }

// ----------------------------------------------------------------------------
// CFileManagerViewBase::CmdDeleteL
// 
// ----------------------------------------------------------------------------
// 
void CFileManagerViewBase::CmdDeleteL()
    {

    if ( !iContainer->ListBoxNumberOfItems() )
        {
        // List box is empty, nothing to delete
        return;
        }

    const TInt selectionCount(iContainer->ListBoxSelectionIndexesCount() );
    TInt index( iContainer->ListBoxCurrentItemIndex() );

    if ( selectionCount == 1 )
        {
        // One item marked
        const CArrayFix< TInt >* items = iContainer->ListBoxSelectionIndexes();
        index = items->At( 0 );
        }

    CFileManagerItemProperties* prop = iEngine.GetItemInfoL( index );
    CleanupStack::PushL( prop );

    if ( DeleteStatusNotOkL( *prop, selectionCount ) ) 
        {
        // It is not possible to continue delete operation
        CleanupStack::PopAndDestroy( prop );
        return;
        }

    HBufC* prompt = GetDeleteQueryPromptLC( *prop, selectionCount );

    TBool ret( EFalse );
    DenyDirectoryRefresh( ETrue );
    TRAPD( err, ret = FileManagerDlgUtils::ShowConfirmQueryWithYesNoL( *prompt ) );
    DenyDirectoryRefresh( EFalse );
    User::LeaveIfError( err );
    if ( ret )
        {
        if( IsDriveAvailable( DriveInfo().iDrive ) )
            {
            DeleteItemsL( index );
            }
        else
            {
            FileManagerDlgUtils::ShowInfoNoteL( R_QTN_MEMC_NOT_AVAILABLE );
            CheckPostponedDirectoryRefresh();
            }
        }
    else
        {
        CheckPostponedDirectoryRefresh();
        }
    CleanupStack::PopAndDestroy( prompt );
    CleanupStack::PopAndDestroy( prop );

    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::CmdMoveToFolderL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerViewBase::CmdMoveToFolderL()
    {

    if ( DriveReadOnlyMmcL( iEngine.CurrentDirectory() ) )
        {
        return;
        }

    // double KMaxFileName is needed if both source and target are KMaxFileName
    HBufC* fileName = HBufC::NewLC( KFmgrDoubleMaxFileName );
    TPtr ptrFileName = fileName->Des();
    CFileManagerFileSelectionFilter* filter =
        new( ELeave ) CFileManagerFileSelectionFilter( iEngine );
    CleanupStack::PushL( filter );

    TInt memType(
        AknCommonDialogsDynMem::EMemoryTypePhone |
        AknCommonDialogsDynMem::EMemoryTypeMMC );

    if ( FeatureManager().IsRemoteStorageFwSupported() )
        {
        memType |= AknCommonDialogsDynMem::EMemoryTypeRemote;
        }

    DenyDirectoryRefresh( ETrue );
    TBool ret( AknCommonDialogsDynMem::RunMoveDlgLD( 
        memType,
        ptrFileName,
        R_FILEMANAGER_MOVE_MEMORY_SELECTIONDIALOG,
        filter ) );
    DenyDirectoryRefresh( EFalse );
    CleanupStack::PopAndDestroy( filter );

    if ( ret && ptrFileName.Length() )
        {
        if ( !DriveReadOnlyMmcL( ptrFileName ) )
            {
            RunOperationL( 
                MFileManagerProcessObserver::EMoveProcess, ptrFileName );
            }
        }
    if (!ret )
        {
        CheckPostponedDirectoryRefresh();
        }

    CleanupStack::PopAndDestroy( fileName );
    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::CmdCopyToFolderL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerViewBase::CmdCopyToFolderL()
    {
    // double KMaxFileName is needed if both source and target are KMaxFileName
    HBufC* fileName = HBufC::NewLC( KFmgrDoubleMaxFileName );
    TPtr ptrFileName = fileName->Des();
    CFileManagerFileSelectionFilter* filter =
        new( ELeave ) CFileManagerFileSelectionFilter( iEngine );
    CleanupStack::PushL( filter );

    TInt memType(
        AknCommonDialogsDynMem::EMemoryTypePhone |
        AknCommonDialogsDynMem::EMemoryTypeMMC );

    if ( FeatureManager().IsRemoteStorageFwSupported() )
        {
        memType |= AknCommonDialogsDynMem::EMemoryTypeRemote;
        }

    DenyDirectoryRefresh( ETrue );
    TBool ret( AknCommonDialogsDynMem::RunCopyDlgLD( 
        memType,
        ptrFileName,
        R_FILEMANAGER_COPY_MEMORY_SELECTIONDIALOG,
        filter ) );
    DenyDirectoryRefresh( EFalse );
    CleanupStack::PopAndDestroy( filter );

    if ( ret && ptrFileName.Length() )
        {
        if ( !DriveReadOnlyMmcL( ptrFileName ) )
            {
            RunOperationL(
                MFileManagerProcessObserver::ECopyProcess, ptrFileName );
            }
        }
    if (!ret )
        {
        CheckPostponedDirectoryRefresh();
        }
    CleanupStack::PopAndDestroy( fileName );
    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::CmdNewFolderL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerViewBase::CmdNewFolderL()
    {

    if ( DriveReadOnlyMmcL( iEngine.CurrentDirectory() ) )
        {
        return;
        }

    StoreIndex();

    if ( !iEngine.EnoughSpaceL(
            iEngine.CurrentDirectory(),
            0, 
            MFileManagerProcessObserver::ENoProcess ) )
        {
        User::Leave( KErrDiskFull );
        }
    HBufC* folderNameBuf = HBufC::NewLC( KMaxFileName );
    TPtr folderName( folderNameBuf->Des() );

    if ( FileManagerDlgUtils::ShowFolderNameQueryL(
         R_QTN_FLDR_NAME_PRMPT, folderName, iEngine, ETrue ) )
        {
        TBuf<KMaxPath> fullFolderName( iEngine.CurrentDirectory() );
        fullFolderName.Append( folderName );
        CFileManagerUtils::EnsureFinalBackslash( fullFolderName );
        
        if ( iEngine.IsSystemFolder( fullFolderName ) )
            {
            FileManagerDlgUtils::ShowInfoNoteL( R_QTN_FLDR_NAME_ALREADY_USED, folderName );
            }
        else
            {
        iEngine.NewFolderL( folderName );
            }
        iEngine.SetObserver( this );
        iEngine.RefreshDirectory();
        }
    else
        {
        if ( iContainer && iContainer->IsSearchFieldVisible() )
            {
            iContainer->DrawDeferred();
            }
        }
    CleanupStack::PopAndDestroy( folderNameBuf );
    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::CmdToggleMarkL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerViewBase::CmdToggleMarkL()
    {
    const TInt index( iContainer->ListBoxCurrentItemIndex() );
    if ( iEngine.IsFolder( index ) )
        {
        iContainer->ListBoxDeselectItem( index );
        }
    else
        {
        iContainer->ListBoxToggleItemL( index );
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::CmdMarkAllL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerViewBase::CmdMarkAllL()
    {
    iContainer->ListBoxSelectAllL();
    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::CmdUnmarkAllL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerViewBase::CmdUnmarkAllL()
    {
    iContainer->ListBoxClearSelection();
    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::CmdRenameL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerViewBase::CmdRenameL()
    {
    TInt index( iContainer->ListBoxCurrentItemIndex() );
    if ( index >= 0 )
        {
        StoreIndex();
        CFileManagerItemProperties* prop = 
            iEngine.GetItemInfoL( index );
        CleanupStack::PushL( prop );

        if ( DriveReadOnlyMmcL( prop->FullPath() ) )
            {
            CleanupStack::PopAndDestroy( prop );
            return;
            }

        HBufC* itemNameBuf = HBufC::NewLC( KMaxFileName );
        TPtr itemName( itemNameBuf->Des() );
        itemName.Append( prop->NameAndExt() );

        TInt err( KErrNone );
        TBool ret( EFalse );
        if ( prop->TypeL() & CFileManagerItemProperties::EFolder )
            {
            DenyDirectoryRefresh( ETrue );
            TRAP( err, ret = FileManagerDlgUtils::ShowFolderNameQueryL(
                R_QTN_FLDR_ITEM_NAME_PRMPT, itemName, iEngine  ) );
            DenyDirectoryRefresh( EFalse );
            User::LeaveIfError( err );
            if ( ret )
                {
                if ( itemName.Length() > 1 )
                	{
                	if ( itemName[0] == '.' )
                		{
                		TInt j = 1;
                		for ( j; j < itemName.Length(); j++ )
                			{
                			if ( !( (itemName[j] <= 'Z') && (itemName[j] >= 'A') ) )
                				{
                				break;
                				}
                			}
                		if ( j == itemName.Length() )
                			{
                			itemName.Delete(0, 1);
                			}
                		}
                	}
                TRAP( err, iEngine.RenameL( index, itemName ) );
                if ( err == KErrAccessDenied || 
                    err == KErrInUse || 
                    err == KErrBadName || 
                    err == KErrAlreadyExists ||
                    err == KErrNotReady )
                    {
                    err = KErrNone; // Set error as handled
                    FileManagerDlgUtils::ShowInfoNoteL(
                        R_QTN_FLDR_CANT_RENAME_ITEM,
                        prop->NameAndExt() );
                    }
                }
            }
        else 
            {
            DenyDirectoryRefresh( ETrue );
            TRAP( err, ret = FileManagerDlgUtils::ShowFileNameQueryL(
                R_QTN_FLDR_ITEM_NAME_PRMPT, prop->FullPath(), itemName, iEngine ) );
            DenyDirectoryRefresh( EFalse );
            User::LeaveIfError( err );
            if ( ret )
                {
                TRAP( err, iEngine.RenameL( index, itemName ) );
                if ( err == KErrAccessDenied || 
                    err == KErrInUse || 
                    err == KErrBadName || 
                    err == KErrAlreadyExists ||
                    err == KErrNotReady )
                    {
                    err = KErrNone; // Set error as handled
                    FileManagerDlgUtils::ShowInfoNoteL(
                        R_QTN_FLDR_CANT_RENAME_ITEM,
                        prop->NameAndExt() );
                    }
                }
            }

        CleanupStack::PopAndDestroy( itemNameBuf );
        CleanupStack::PopAndDestroy( prop );
        User::LeaveIfError( err );
        }
    iEngine.SetObserver( this );
    iEngine.RefreshDirectory();
    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::CmdFindL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerViewBase::CmdFindL()
    {
    HBufC* path = HBufC::NewLC( KMaxFileName );
    TPtr ptrPath( path->Des() );
    if( AskPathL( ptrPath, R_QTN_FMGR_FIND_PRTX ) )
        {
        HBufC* searchStringBuf = HBufC::NewLC( KMaxFileName );
        TPtr searchString( searchStringBuf->Des() );
        HBufC* prompt = StringLoader::LoadLC( R_QTN_FMGR_FIND_DATAQ_PRTX );
        CAknTextQueryDialog *textQuery = 
            new( ELeave ) CAknTextQueryDialog( searchString, *prompt );
        if ( textQuery->ExecuteLD( R_FILEMANAGER_SEARCH_QUERY ) )
            {
            iEngine.SetSearchStringL( searchString );
            iEngine.SetSearchFolderL( ptrPath );
            if ( Id() == CFileManagerAppUi::KFileManagerSearchResultsViewId )
                {
                // Start new search in the existing view
                iIndex = 0;
                iEngine.SetObserver( this );
                iEngine.RefreshDirectory();
                }
            else
                {
                // Open search view and start new search
                StoreIndex();
                // Ensure that current directory is set correctly.
                // If current view was opened from previous search results view,
                // backstep stack to current directory may be incomplete.
                iEngine.SetDirectoryWithBackstepsL( iEngine.CurrentDirectory() );
                CFileManagerAppUi* appUi = 
                    static_cast< CFileManagerAppUi* >( AppUi() );
                appUi->ActivateSearchResultsViewL();
                }
            }
        CleanupStack::PopAndDestroy( prompt );
        CleanupStack::PopAndDestroy( searchStringBuf );
        }
    CleanupStack::PopAndDestroy( path );
    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::CmdViewInfoL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerViewBase::CmdViewInfoL()
    {
    TInt index( iContainer->ListBoxCurrentItemIndex() );
    if ( index >= 0 )
        {
        CFileManagerItemProperties* prop = iEngine.GetItemInfoL( index );
        CleanupStack::PushL( prop );
        FileManagerDlgUtils::ShowItemInfoPopupL( *prop, FeatureManager() );
        CleanupStack::PopAndDestroy( prop );
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::CmdMemoryStateL
// 
// -----------------------------------------------------------------------------
// 
//void CFileManagerViewBase::CmdMemoryStateL()
//    {
//    TInt drv( iEngine.CurrentDrive() );
//    if ( drv != KErrNotFound )
//        {
//        HBufC* title = StringLoader::LoadLC( R_QTN_FMGR_MSTATE_HEADING );
//        CMemStatePopup::RunLD(
//            static_cast< TDriveNumber >( drv ), *title );
//        CleanupStack::PopAndDestroy( title );
//        }
//    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::CmdReceiveViaIR
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerViewBase::CmdReceiveViaIRL()
    {

    if ( DriveReadOnlyMmcL( iEngine.CurrentDirectory() ) )
        {
        return;
        }

    iEngine.SetObserver( this );

    ClearProgressBarL();

    iProgressDialog = new( ELeave ) CAknProgressDialog( 
        reinterpret_cast< CEikDialog** >( &iProgressDialog ), ETrue );
    iProgressDialog->PrepareLC( R_FILE_RECEIVE_DIALOG );
    iProgressInfo = iProgressDialog->GetProgressInfoL();
    if ( iProgressInfo )
        {
        // final value is 100 percent
        iProgressInfo->SetFinalValue( KMaxPercentage ); 
        }
    iProgressDialog->RunLD();
    iProgressDialog->SetCallback( this );

    HBufC* label = StringLoader::LoadLC( R_QTN_IR_CONNECTING );
    iProgressDialog->SetTextL( *label );
    CleanupStack::PopAndDestroy( label );

    CFileManagerAppUi* appUi = static_cast< CFileManagerAppUi* >( AppUi() );
    TRAPD( err, appUi->StartIRReceiveL( *this ) );

    if ( err == KErrNone )
        {
        iActiveProcess = MFileManagerProcessObserver::EIRReceiveProcess;
        }
    else
        {
        ClearProgressBarL();
        User::Leave( err );
        }
    }


// -----------------------------------------------------------------------------
// CFileManagerViewBase::DynInitMenuPaneL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerViewBase::DynInitMenuPaneL( TInt aResourceId, 
                                             CEikMenuPane* aMenuPane)
    {
    TBool isHandled( ETrue );

    switch( aResourceId )
        {
        // These menus are used by memory store and folders views
        case R_FILEMANAGER_MEMORY_STORE_VIEW_MENU:
            {
            MemoryStoreMenuFilteringL( *aMenuPane );
            break;
            }
        case R_FILEMANAGER_MARK_UNMARK_MENU:
        case R_FILEMANAGER_CONTEXT_SENSITIVE_MARK_UNMARK_MENU:
            {
            MarkMenuFilteringL( *aMenuPane );
            break;
            }
        case R_FILEMANAGER_ORGANISE_MENU:
            {
            OrganiseMenuFilteringL( *aMenuPane );
            break;
            }
        case R_FILEMANAGER_DETAILS_MENU:
            {
            DetailsMenuFilteringL( *aMenuPane );
            break;
            }
//        case R_FILEMANAGER_MEMORY_CARD_MENU:
//            {
//            MemoryCardMenuFilteringL( *aMenuPane );
//            break;
//            }
//        case R_FILEMANAGER_MEMORY_CARD_PASSWORD_MENU:
//            {
//            MemoryCardPasswordMenuFilteringL( *aMenuPane );
//            break;
//            }
        case R_FILEMANAGER_CONTEXT_SENSITIVE_MENU:
            {
            ContextSensitiveMenuFilteringL( *aMenuPane );
            break;
            }
        case R_FILEMANAGER_SORT_MENU:
        case R_FILEMANAGER_SEARCH_SORT_MENU: // Fall through
            {
            SortMenuFilteringL( *aMenuPane );
            break;
            }
        default:
            {
            isHandled = EFalse;
            break;
            }
        }

    TBool isContextMenu( aResourceId == R_FILEMANAGER_CONTEXT_SENSITIVE_MENU );
    if ( isHandled || isContextMenu )
        {
        CEikMenuBar* menuBar = MenuBar();
        if ( menuBar )
            {
            if ( isContextMenu )
                {
                menuBar->SetMenuType( CEikMenuBar::EMenuContext );
                }
            else
                {
                menuBar->SetMenuType( CEikMenuBar::EMenuOptions );
                }
            }
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::DoActivateL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerViewBase::DoActivateL( const TVwsViewId& /*aPrevViewId*/,
                                        TUid /*aCustomMessageId*/,
                                        const TDesC8& /*aCustomMessage*/ )
    {
    if ( !iContainer )
        {
        iContainer = CreateContainerL();
        iContainer->SetMopParent( this );
        AppUi()->AddToStackL( *this, iContainer );
        iEngine.SetObserver( this );
        iContainer->ActivateL();
        }

    if ( iContainer )
        {
        iContainer->SetListEmptyL();
        }

    // Set container to observe MSK commands
    CEikButtonGroupContainer* bgc = Cba();
    if ( bgc )
        {
        CEikCba* cba = static_cast< CEikCba* >( bgc->ButtonGroup() );
        cba->SetMSKCommandObserver( iContainer );
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::DoDeactivate
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerViewBase::DoDeactivate()
    {
    if ( iContainer )
        {
        AppUi()->RemoveFromStack( iContainer );
        delete iContainer;
        iContainer = NULL;
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::MarkedArrayLC
// 
// -----------------------------------------------------------------------------
// 
CArrayFixFlat<TInt>* CFileManagerViewBase::MarkedArrayLC()
    {
    TInt count( iContainer->ListBoxSelectionIndexesCount() );
    CArrayFixFlat<TInt>* ret = 
        new( ELeave ) CArrayFixFlat<TInt>( count ? count : 1 );
    
    CleanupStack::PushL( ret );

    if ( !count )
        {
        if ( iContainer->ListBoxNumberOfItems() > 0)
            {
            ret->AppendL( iContainer->ListBoxCurrentItemIndex() );
            }
        return ret;
        }

    const CArrayFix< TInt >* items = iContainer->ListBoxSelectionIndexes();
    for( TInt i( 0 ); i < count; ++i )
        {
        ret->AppendL( items->At( i ) );
        }
    return ret;
    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::DialogDismissedL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerViewBase::DialogDismissedL( TInt aButtonId  )
    {
    FUNC_LOG

    if ( aButtonId == EAknSoftkeyCancel )
        {
        TBool isHandled( ETrue );
        switch( iActiveProcess )
            {
            case ENoProcess:
                {
                if ( IsRefreshInProgress() )
                    {
                    // Already freed, just set to NULL
                    iProgressDialogRefresh = NULL;
                    iEngine.CancelRefresh();
                    DirectoryChangedL(); // Ensure that view gets updated
                    }
                break;
                }
            case EIRReceiveProcess:
                {
                // Already freed, just set to NULL
                iProgressDialog = NULL;
                iProgressInfo = NULL;

                static_cast< CFileManagerAppUi* >( AppUi() )->StopIRReceive();
                break;
                }
            case ECopyProcess: // FALLTHROUGH
            case EMoveProcess:
                {
                // Already freed, just set to NULL
                iProgressDialog = NULL;
                iProgressInfo = NULL;

                delete iPeriodic;
                iPeriodic = NULL;
                if ( iActiveExec )
                    {
                    iActiveExec->CancelExecution();
                    }
                break;
                }
            case EFileOpenProcess: // FALLTHROUGH
            case EBackupProcess: // FALLTHROUGH
            case ERestoreProcess:
                {
                // Already freed, just set to NULL
                iProgressDialog = NULL;
                iProgressInfo = NULL;

                iEngine.CancelProcess( iActiveProcess );
                if ( iActiveProcess == EBackupProcess ||
                     iActiveProcess == ERestoreProcess )
                    {
                    CFileManagerAppUi* appUi = static_cast< CFileManagerAppUi* >( AppUi() );
                    appUi->BackupOrRestoreEnded();
                    }
                break;
                }
            case EFormatProcess: // FALLTHROUGH
            case EEjectProcess:
                {
                // Already freed, just set to NULL
                iProgressDialog = NULL;
                iProgressInfo = NULL;
                break;
                }
            default:
                {
                isHandled = EFalse;
                break;
                }
            }
        if ( isHandled )
            {
            iEikonEnv->SetSystem( EFalse );
            iActiveProcess = ENoProcess;
            }

#ifdef RD_FILE_MANAGER_BACKUP
        if ( iSchBackupPending )
            {
            StartSchBackupL();
            }
#endif // RD_FILE_MANAGER_BACKUP

        }
    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::ProcessFinishedL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerViewBase::ProcessFinishedL( TInt aError, const TDesC& aName )
    {
    FUNC_LOG

    TRAPD( err, DoProcessFinishedL( aError, aName ) );
    if( err != KErrNone )
        {
        // Clean up the active process before forwarding leave
        ERROR_LOG2(
            "CFileManagerViewBase::ProcessFinishedL-iActiveProcess=%d,err=%d",
            iActiveProcess, err )
        iEikonEnv->SetSystem( EFalse );
        iActiveProcess = ENoProcess;
        User::Leave( err );
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::DoProcessFinishedL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerViewBase::DoProcessFinishedL( TInt aError, const TDesC& aName )
    {
    FUNC_LOG

    TBool isHandled( ETrue );
    TBool doRefresh( ETrue );

    LOG_IF_ERROR2( aError, "CFileManagerViewBase::DoProcessFinishedL-iActiveProcess=%d,aError=%d",
        iActiveProcess, aError )

    if ( iPeriodic && iProgressInfo && iTotalTransferredBytes )
        {
        iProgressInfo->SetAndDraw( iTotalTransferredBytes );
        }
    if ( IsSystemProcess( iActiveProcess ) )
        {
        // Remove system status to allow app close from task switcher
        iEikonEnv->SetSystem( EFalse );
        }

    ClearProgressBarL();

    switch( iActiveProcess )
        {
        case EIRReceiveProcess: // FALLTHROUGH
            {
            static_cast< CFileManagerAppUi* >( AppUi() )->StopIRReceive();
            if ( aError != KErrNone && aError != KErrCancel )
                {
                if ( aError == KErrDiskFull )
                    {
                    ShowDiskSpaceErrorL( iEngine.CurrentDirectory() );
                    }
                else
                    {
                    // Show general error note
                    Error( aError );
                    }
                }
            break;
            }
        case ECopyProcess: // FALLTHROUGH
        case EMoveProcess: 
            {
            if ( aError != KErrNone && aError != KErrCancel && !aName.Length() )
                {
                // Show general error note if item name is unavailable
                if ( iActiveExec && aError == KErrDiskFull )
                    {
                    ShowDiskSpaceErrorL( iActiveExec->ToFolder() );
                    }
                else
                    {
                    Error( aError );
                    }
                }
            else if ( aError != KErrNone )
                {
                // If the copy process is cancelled, no error notes should be displayed
                if( aError != KErrCancel )
                    {
                    // Show more informative note first
                    if ( iActiveExec && aError == KErrDiskFull )
                        {
                        ShowDiskSpaceErrorL( iActiveExec->ToFolder() );
                        }
                    else if ( aError == KErrNoMemory ||
                              aError == KErrDiskFull ||
                              aError == KErrDirFull )
                        {
                        Error( aError );
                        }
                    if ( iActiveProcess == EMoveProcess )
                        {
                        FileManagerDlgUtils::ShowErrorNoteL(
                            R_QTN_FLDR_ITEM_CANNOT_BE_MOVED, aName );
                        }
                    else
                        {
                        FileManagerDlgUtils::ShowErrorNoteL(
                            R_QTN_FLDR_ITEM_CANNOT_BE_COPIED, aName );
                        }
                    }
                delete iActiveExec;
                iActiveExec = NULL;
                }
            else if ( iActiveProcess == EMoveProcess && iMarkedArray )
                {
                // Set focus to the item after selection
                TInt newIndex( MinIndex( *iMarkedArray ) );
                if ( iContainer )
                    {
                    iContainer->SetIndex( newIndex );
                    }
                StoreIndex();
                }
                
            break;
            }
        case EFileOpenProcess:
            {
            if ( aError != KErrNone && aError != KErrCancel )
                {
                if ( aError == KErrNoMemory || aError == KErrDiskFull )
                    {
                    Error( aError );
                    }
                else if ( aError == KErrNotSupported )
                    {
                    FileManagerDlgUtils::ShowErrorNoteL(
                        R_QTN_FMGR_ERROR_UNSUPPORT );
                    }
                else if ( aError == KErrFmgrNotSupportedRemotely )
                    {
                    FileManagerDlgUtils::ShowConfirmQueryWithOkL(
                        FileManagerDlgUtils::EInfoIcons,
                        R_QTN_FMGR_INFONOTE_UNABLE_OPEN_REMOTELY );
                    }
                else if ( !HandleFileNotFoundL( aError ) )
                    {
                    FileManagerDlgUtils::ShowErrorNoteL(
                        R_QTN_FMGR_ERROR_CANT_OPEN );
                    }
                }
            else
                {
                // No refresh needed if open was successful or canceled
                doRefresh = EFalse;
                }
            break;
            }
        case EFormatProcess:
            {
            TFileManagerDriveInfo drvInfo;
            DriveInfoAtCurrentPosL( drvInfo );
            if ( aError == KErrNone )
                {
#ifdef RD_MULTIPLE_DRIVE
                if ( drvInfo.iState & TFileManagerDriveInfo::EDriveMassStorage )
                    {
                    FileManagerDlgUtils::ShowInfoNoteL(
                        R_QTN_FMGR_MASS_FORMAT_COMPLETED );
                    }
                else
                    {
#endif // RD_MULTIPLE_DRIVE
                    FileManagerDlgUtils::ShowInfoNoteL( R_QTN_FORMAT_COMPLETED );

                    // After formatting a name to the card can be given
                    RenameDriveL( ETrue );
#ifdef RD_MULTIPLE_DRIVE
                    }
#endif // RD_MULTIPLE_DRIVE
                }
            else if ( aError == KErrInUse || aError > 0 )
                {
                FileManagerDlgUtils::ShowErrorNoteL(
                    R_QTN_FORMAT_FILES_IN_USE );
                }
            else if ( aError != KErrCancel )
                {
                FileManagerDlgUtils::ShowErrorNoteL(
                    R_QTN_CRITICAL_ERROR );
                }
            break;
            }
        case EBackupProcess:
            {
            CFileManagerAppUi* appUi = static_cast< CFileManagerAppUi* >( AppUi() );
            appUi->BackupOrRestoreEnded();
            if ( aError == KErrNone )
                {
                FileManagerDlgUtils::ShowInfoNoteL(
                    R_QTN_BACKUP_COMPLETED );
                }
            else if ( aError == KErrDiskFull )
                {
#ifdef RD_FILE_MANAGER_BACKUP

                CFileManagerBackupSettings& settings(
                    iEngine.BackupSettingsL() );

                FileManagerDlgUtils::ShowConfirmQueryWithOkL(
                    FileManagerDlgUtils::EErrorIcons,
                    R_QTN_FMGR_BACKUP_DESTINATION_FULL,
                    iEngine.DriveName( settings.TargetDrive() ) );

#else // RD_FILE_MANAGER_BACKUP

                FileManagerDlgUtils::ShowErrorNoteL(
                    R_QTN_BACKUP_NO_SPACE );

#endif // RD_FILE_MANAGER_BACKUP
                }
            else if ( aError > 0 )
                {
                // No critical error, but some files not handled
                if ( aError > 1 )
                    {
                    FileManagerDlgUtils::ShowConfirmQueryWithOkL(
                        FileManagerDlgUtils::EInfoIcons,
                        R_QTN_FILES_NOT_BACKUPPED,
                        aError );
                    }
                else
                    {
                    FileManagerDlgUtils::ShowConfirmQueryWithOkL(
                        FileManagerDlgUtils::EInfoIcons,
                        R_QTN_ONE_FILE_NOT_BACKUPPED );
                    }
                }
            else if ( aError != KErrCancel )
                {
                if ( aError == KErrNoMemory || aError == KErrDirFull )
                    {
                    // Show more informative note first
                    Error( aError );
                    }
                FileManagerDlgUtils::ShowErrorNoteL(
                    R_QTN_CRITICAL_ERROR );
                }
            break;
            }
        case ERestoreProcess:
            {
            CFileManagerAppUi* appUi = static_cast< CFileManagerAppUi* >( AppUi() );
            appUi->BackupOrRestoreEnded();
            if ( aError == KErrNone )
                {
                FileManagerDlgUtils::ShowInfoNoteL(
                    R_QTN_RESTORE_COMPLETED );
                }
            else if ( aError == KErrDiskFull )
                {
                FileManagerDlgUtils::ShowErrorNoteL(
#ifdef RD_FILE_MANAGER_BACKUP
                    R_QTN_FMGR_RESTORE_SPACE_ERROR
#else // RD_FILE_MANAGER_BACKUP
                    R_QTN_RESTORE_NO_SPACE
#endif // RD_FILE_MANAGER_BACKUP
                     );
                }
#ifdef RD_FILE_MANAGER_BACKUP
            else if ( aError == KErrCorrupt )
                {
                FileManagerDlgUtils::ShowErrorNoteL(
                    R_QTN_FMGR_ERROR_CORRUPTED_BACKUP_FILE );
                }
#endif // RD_FILE_MANAGER_BACKUP
            else if ( aError > 0 )
                {
                // No critical error, but some files not handled
                if ( aError > 1 )
                    {
                    FileManagerDlgUtils::ShowInfoNoteL(
                        R_QTN_FILES_NOT_RESTORED, aError );
                    }
                else
                    {
                    FileManagerDlgUtils::ShowInfoNoteL(
                        R_QTN_ONE_FILE_NOT_RESTORED );
                    }
                }
            else
                {
                if ( aError == KErrNoMemory || aError == KErrDirFull )
                    {
                    // Show more informative note first
                    Error( aError );
                    }
                FileManagerDlgUtils::ShowErrorNoteL(
                    R_QTN_CRITICAL_ERROR );
                }
            break;
            }
        case EEjectProcess:
            {
            TRAP_IGNORE( ShowEjectQueryL() );
            break;
            }
#ifdef RD_FILE_MANAGER_BACKUP
        case ESchBackupProcess:
            {
            CFileManagerAppUi* appUi =
                static_cast< CFileManagerAppUi* >( AppUi() );
            appUi->SchBackupHandlerL().ProcessFinishedL( aError, aName );
            // No refresh needed, done by view activation
            doRefresh = EFalse;
            break;
            }
#endif // RD_FILE_MANAGER_BACKUP
        default:
            {
            isHandled = EFalse;
            break;
            }
        }

    if ( isHandled )
        {
        iEikonEnv->SetSystem( EFalse );
        iActiveProcess = ENoProcess;

        if ( doRefresh )
            {
            iEngine.SetObserver( this );
            iEngine.RefreshDirectory();
            }
        }

#ifdef RD_FILE_MANAGER_BACKUP
    if ( iSchBackupPending )
        {
        StartSchBackupL();
        }
#endif // RD_FILE_MANAGER_BACKUP

    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::ProcessAdvanceL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerViewBase::ProcessAdvanceL( TInt aValue )
    {
    FUNC_LOG

    switch( iActiveProcess )
        {
        case EIRReceiveProcess:
            {
            if ( iProgressDialog )
                {
                HBufC* label = StringLoader::LoadLC( 
                    R_QTN_FMGR_NOTE_RECEIVE_IR, aValue );
                iProgressDialog->SetTextL( *label );
                CleanupStack::PopAndDestroy( label );
                // Incrementing progress of the process: 
                if ( iProgressInfo )
                    {
                    iProgressInfo->SetAndDraw( aValue ); 
                    }
                }
            break;
            }
        case EBackupProcess: // FALLTHROUGH
        case ERestoreProcess: // FALLTHROUGH
        case EFormatProcess:
            {
#ifdef RD_FILE_MANAGER_BACKUP
            if ( iActiveProcess == EBackupProcess && iProgressDialog )
                {
                HBufC* label = StringLoader::LoadLC(
                    R_QTN_BACKUP_INPROGRESS );
                iProgressDialog->SetTextL( *label );
                CleanupStack::PopAndDestroy( label );
                
                iProgressDialog->ButtonGroupContainer().SetCommandSetL(R_AVKON_SOFTKEYS_CANCEL);
                iProgressDialog->ButtonGroupContainer().DrawDeferred();
                }
            else if ( iActiveProcess == ERestoreProcess && iProgressDialog )
                {
                HBufC* label = StringLoader::LoadLC(
                    R_QTN_RESTORE_INPROGRESS );
                iProgressDialog->SetTextL( *label );
                CleanupStack::PopAndDestroy( label );
                }                
#endif // RD_FILE_MANAGER_BACKUP
            if ( iProgressInfo )
                {
                iProgressInfo->SetAndDraw( aValue );
                }
            break;
            }
#ifdef RD_FILE_MANAGER_BACKUP
        case ESchBackupProcess:
            {
            CFileManagerAppUi* appUi =
                static_cast< CFileManagerAppUi* >( AppUi() );
            appUi->SchBackupHandlerL().ProcessAdvanceL( aValue );
            break;
            }
#endif // RD_FILE_MANAGER_BACKUP
        default:
            {
            break;
            }
        }
    iTotalTransferredBytes = static_cast<TUint>(aValue);  // to avoid over 2GB files looks likes minus value 
    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::ProcessStartedL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerViewBase::ProcessStartedL(
    MFileManagerProcessObserver::TFileManagerProcess aProcess,
    TInt aFinalValue )
    {
    FUNC_LOG

    // For preventing shutter to close app during system process
    iEikonEnv->SetSystem( IsSystemProcess( aProcess ) );

    switch( aProcess )
        {
        case EIRReceiveProcess:
            {
            if ( iProgressDialog )
                {
                HBufC* label = StringLoader::LoadLC(
                    R_QTN_FMGR_NOTE_RECEIVE_IR, 0 );
                iProgressDialog->SetTextL( *label );
                CleanupStack::PopAndDestroy( label );
                }
            break;
            }
        case EFileOpenProcess:
            {
            ClearProgressBarL();
            LaunchProgressDialogL( 0, 0, aProcess );
            iActiveProcess = aProcess;
            break;
            }
        case ERestoreProcess:
            {
            CEikButtonGroupContainer* cba = Cba();
            cba->SetCommandSetL( R_AVKON_SOFTKEYS_EMPTY );
            cba->DrawDeferred();
            // FALLTHROUGH
            }
        case EBackupProcess: // FALLTHROUGH
        case EFormatProcess:
            {
            if ( iProgressDialog )
                {
                if ( !iProgressDialog->IsVisible() )
                    {
                    iProgressDialog->MakeVisible( ETrue );
                    }
                }
            if ( iProgressInfo )
                {
                iProgressInfo->SetFinalValue( aFinalValue );
                }    
            break;
            }
#ifdef RD_FILE_MANAGER_BACKUP
        case ESchBackupProcess:
            {
            CFileManagerAppUi* appUi =
                static_cast< CFileManagerAppUi* >( AppUi() );
            appUi->SchBackupHandlerL().ProcessStartedL( aFinalValue );
            break;
            }
#endif // RD_FILE_MANAGER_BACKUP
        default:
            {
            break;
            }
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::RunOperationL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerViewBase::RunOperationL
    ( MFileManagerProcessObserver::TFileManagerProcess aOperation,
      const TDesC& aToFolder )
    {

    StoreIndex();
    delete iMarkedArray;
    iMarkedArray = NULL;
    iMarkedArray = MarkedArrayLC();
    CleanupStack::Pop( iMarkedArray );

    // Check if marked source and destination folder are available
    if ( !iMarkedArray->Count() || !IsDriveAvailable( aToFolder ) )
        {
        return;
        }

    CFileManagerItemProperties* prop =
        iEngine.GetItemInfoLC( iMarkedArray->At( 0 ) );

#ifdef __KEEP_DRM_CONTENT_ON_PHONE
    // When this flag is on all the selected items have to be gone through and checked
    // whether they are protected and the user has to be notified when moving or
    // copying file(s) is impossible. This only applies to processes from phone to MMC.
    TBool process( ETrue );
    if ( CFileManagerUtils::IsFromInternalToRemovableDrive(
            iEikonEnv->FsSession(), iEngine.CurrentDirectory(), aToFolder ) )
        {
        TInt fileAmount (iMarkedArray->Count());

        // Only one folder can be selected at a time
        if (iEngine.IsFolder(iMarkedArray->At( 0 )))
            {
            if ( prop->FilesContainedL() == 0 && prop->FoldersContainedL() == 0)
                {
                process = ETrue;
                }
            else if (AreChosenFilesProtectedL( ETrue ))
                {
                if ( aOperation == EMoveProcess )
                    {
                    FileManagerDlgUtils::ShowInfoNoteL(
                        R_QTN_DRM_INFO_MOVE_FOLDER_FORBID );
                    }
                else
                    {
                    FileManagerDlgUtils::ShowInfoNoteL(
                        R_QTN_DRM_INFO_COPY_FOLDER_FORBID );
                    }
                process = EFalse;
                }
            else if (AreChosenFilesProtectedL( EFalse ))
                {
                TInt textId( 0 );
                if ( aOperation == EMoveProcess )
                    {
                    textId = R_QTN_DRM_QUERY_MOVE_FORBIDDEN;
                    }
                else
                    {
                    textId = R_QTN_DRM_QUERY_COPY_FORBIDDEN;
                    }
                if ( FileManagerDlgUtils::ShowConfirmQueryWithYesNoL(
                        textId ) )
                    {
                    // Engine will not touch protected objects anyway
                    process = ETrue;
                    }
                else
                    {
                    process = EFalse;
                    }
                }
            }
        else if ( fileAmount == 1 && AreChosenFilesProtectedL( ETrue ))
            {
            if ( aOperation == EMoveProcess )
                {
                FileManagerDlgUtils::ShowInfoNoteL(
                    R_QTN_DRM_INFO_MOVE_ONE_FORBID );
                }
            else
                {
                FileManagerDlgUtils::ShowInfoNoteL(
                    R_QTN_DRM_INFO_COPY_ONE_FORBID );
                }
            process= EFalse;
            }
        else if ( fileAmount > 1 && AreChosenFilesProtectedL( EFalse ))
            {
            if (AreChosenFilesProtectedL( ETrue ))
                {
                if ( aOperation == EMoveProcess )
                    {
                    FileManagerDlgUtils::ShowInfoNoteL(
                        R_QTN_DRM_INFO_MOVE_MANY_FORBID );
                    }
                else
                    {
                    FileManagerDlgUtils::ShowInfoNoteL(
                        R_QTN_DRM_INFO_COPY_MANY_FORBID );
                    }
                process= EFalse;
                }
            else
                {
                TInt textId( 0 );
                if ( aOperation == EMoveProcess )
                    {
                    textId = R_QTN_DRM_QUERY_MOVE_FORBIDDEN;
                    }
                else
                    {
                    textId = R_QTN_DRM_QUERY_COPY_FORBIDDEN;
                    }
                if ( FileManagerDlgUtils::ShowConfirmQueryWithYesNoL(
                        textId ) )
                    {
                    // Engine will not touch protected objects anyway
                    process = ETrue;
                    }
                else
                    {
                    process = EFalse;
                    }
                }
            }
        }
    if ( process )
        {
#endif // __KEEP_DRM_CONTENT_ON_PHONE

        TInt64 size( 0 );
        // Skip remote folder size counting because it may last very long time.
        // The content may also change during the operation what makes 
        // the counting needless.
        if ( !( prop->IsRemoteDrive() &&
                ( prop->TypeL() & CFileManagerItemProperties::EFolder ) ) )
            {
            size = iEngine.GetFileSizesL( *iMarkedArray ) ;
            }
        if ( size == KErrNotFound )
            {
            // User has cancelled size calculation, do nothing
            }
        else if ( iEngine.EnoughSpaceL( aToFolder, size, aOperation ))
            {
            iTotalTransferredBytes = 0;            
            iEngine.SetObserver( this );
            if ( aOperation == EMoveProcess && 
                aToFolder.Left( KDriveLetterSize ) == 
                prop->FullPath().Left( KDriveLetterSize ) )
                {
                // If operation is move and it happens inside drive
                // set size to file amount
                // CFileMan is not calling notify if those conditions apply
                if ( iMarkedArray->Count() > 0 )
                    {   
                    size = iMarkedArray->Count();
                    }
                else
                    {
                    // Folder move time we cannot predict, so setting size to
                    // 0 to show wait note, one file moves so fast that it
                    // won't show wait note anyway
                    size = 0;
                    }
                }

            if ( prop->IsRemoteDrive() ||
                CFileManagerUtils::IsRemoteDrive(
                    iEikonEnv->FsSession(), aToFolder ) )
                {
                // Use wait note for remote drives 
                // because real progress information is unavailable
                size = 0;
                }

            LaunchProgressDialogL( size, 0, aOperation );
            delete iActiveExec;
            iActiveExec = NULL;
            iActiveExec = CFileManagerActiveExecute::NewL(
                iEngine, aOperation, *this, *iMarkedArray, aToFolder );
            iActiveProcess = aOperation;
            TRAPD( err, iActiveExec->ExecuteL( CFileManagerActiveExecute::ENoOverWrite ) );
            if ( err != KErrNone )
                {
                // Clean up the active process before forwarding leave
                ERROR_LOG2(
                    "CFileManagerViewBase::RunOperationL-aOperation=%d,err=%d",
                    aOperation, err )
                iActiveProcess = ENoProcess;
                User::Leave( err );
                }
            }
        else
            {
            ShowDiskSpaceErrorL( aToFolder );
            }

#ifdef  __KEEP_DRM_CONTENT_ON_PHONE
        }
#endif // __KEEP_DRM_CONTENT_ON_PHONE

    CleanupStack::PopAndDestroy( prop );

    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::ProcessQueryOverWriteL
// 
// -----------------------------------------------------------------------------
// 
TBool CFileManagerViewBase::ProcessQueryOverWriteL
    ( const TDesC& aOldName, TDes& aNewName, TFileManagerProcess aOperation )
    {
    
    TParsePtrC name( aOldName );

    // Stop progress note before showing the query to be restarted later. 
    // Note that progress note may still exist after stop (to fill min time on screen) 
    // and it gets deleted later by AVKON. Asynchronous restart is needed to prevent 
    // mess up (if note still exists). Otherwise starting and stopping progress note too 
    // quickly multiple times leads to mess up in AVKON's note handling.
    StopProgressDialogAndStoreValues();

    TBool overWrite( FileManagerDlgUtils::ShowConfirmQueryWithYesNoL(
            R_QTN_FLDR_OVERWRITE_QUERY, name.NameAndExt() ) );
    if ( !overWrite )
        {
        if ( !FileManagerDlgUtils::ShowFileNameQueryL(
            R_QTN_FLDR_ITEM_NAME_PRMPT, aOldName, aNewName, iEngine ) )
            {
            aNewName.Zero();
            }
        }

    if ( iActiveProcess == aOperation &&
         ( aOperation == MFileManagerProcessObserver::ECopyProcess ||
           aOperation == MFileManagerProcessObserver::EMoveProcess ) )
        {
        // Progress note needs asynchronous start because of AVKON's note handling.
        delete iRefreshProgressDelayedStart;
        iRefreshProgressDelayedStart = NULL;
        iRefreshProgressDelayedStart = CPeriodic::NewL( CActive::EPriorityUserInput );
        iRefreshProgressDelayedStart->Start(
            KProgressBarAsyncStartDelay, KProgressBarAsyncStartDelay,
            TCallBack( LaunchProgressDialogAsync, this ) );
        }
    
    return overWrite;
    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::ProcessQueryRenameL
// 
// -----------------------------------------------------------------------------
//
TBool CFileManagerViewBase::ProcessQueryRenameL
    ( const TDesC& aOldName, TDes& aNewName, TFileManagerProcess aOperation )
    {
    TParsePtrC typeCheck( aOldName );
    TParse oldName;
    TBool folderRename( EFalse );

    // Check is item file or folder
    if ( !typeCheck.NameOrExtPresent() )
        {
        oldName.Set( aOldName.Left( aOldName.Length() - 1 ), NULL, NULL );
        folderRename = ETrue;
        }
    else
        {
        oldName.Set( aOldName , NULL, NULL );
        }

    // Stop progress note before showing the query to be restarted later. 
    // Note that progress note may still exist after stop (to fill min time on screen) 
    // and it gets deleted later by AVKON. Asynchronous restart is needed to prevent 
    // mess up (if note still exists). Otherwise starting and stopping progress note too 
    // quickly multiple times leads to mess up in AVKON's note handling.
    StopProgressDialogAndStoreValues();

    TBool rename( FileManagerDlgUtils::ShowConfirmQueryWithOkCancelL(
            R_QTN_FLDR_RENAME_QUERY, oldName.NameAndExt() ) );
    if ( rename )
        {
        TBool done( 0 );
        if ( folderRename )
            {
            aNewName.Copy( aOldName );
            done = FileManagerDlgUtils::ShowFolderNameQueryL(
                R_QTN_FLDR_ITEM_NAME_PRMPT, aNewName, iEngine );
            }
        else
            {
            done = FileManagerDlgUtils::ShowFileNameQueryL(
                R_QTN_FLDR_ITEM_NAME_PRMPT, aOldName, aNewName, iEngine );
            }

        if ( !done )
            {
            // User cancelled rename
            aNewName.Zero();
            }
        else
            {
            //file server doesn't support the space in the end of the folder/file name
            aNewName.TrimRight();
            }
        }

    if ( iActiveProcess == aOperation &&
         ( aOperation == MFileManagerProcessObserver::ECopyProcess ||
           aOperation == MFileManagerProcessObserver::EMoveProcess ) )
        {
        // Progress note needs asynchronous start because of AVKON's note handling.
        delete iRefreshProgressDelayedStart;
        iRefreshProgressDelayedStart = NULL;
        iRefreshProgressDelayedStart = CPeriodic::NewL( CActive::EPriorityUserInput );
        iRefreshProgressDelayedStart->Start(
            KProgressBarAsyncStartDelay, KProgressBarAsyncStartDelay,
            TCallBack( LaunchProgressDialogAsync, this ) );
        }

    return rename;
    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::LaunchProgressDialogL  
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerViewBase::LaunchProgressDialogL( 
    TInt64 aFinalValue, 
    TInt64 aInitialValue, 
    MFileManagerProcessObserver::TFileManagerProcess aOperation,
    TBool aImmediatelyVisible )
    {
    TInt dialogId( 0 );
    TInt textId( 0 );
    TBool isPeriodic( EFalse );
    TInt value;
    value=Int64ToInt(aFinalValue);
    switch ( aOperation )
        {
        case ECopyProcess:
            {
            isPeriodic = ( value > 1 );
            if ( isPeriodic )
                {
                dialogId = R_FILEMANAGER_PROGRESS_NOTE_COPY;
                }
            else
                {
                dialogId = R_FILEMANAGER_WAIT_NOTE_COPY;
                }
            break;
            }
        case EMoveProcess:
            {
            isPeriodic = ( value > 1 );
            if ( isPeriodic )
                {
                dialogId = R_FILEMANAGER_PROGRESS_NOTE_MOVE;
                }
            else
                {
                dialogId = R_FILEMANAGER_WAIT_NOTE_MOVE;
                }
            break;
            }
        case EFormatProcess:
            {
            dialogId = R_FILEMANAGER_PROGRESS_NOTE;
			TFileManagerDriveInfo drvInfo;
			DriveInfoAtCurrentPosL( drvInfo );
#ifdef RD_MULTIPLE_DRIVE
			if ( drvInfo.iState & TFileManagerDriveInfo::EDriveMassStorage
					|| drvInfo.iState & TFileManagerDriveInfo::EDriveUsbMemory )
                {
                textId = R_QTN_FMGR_MASS_FORMAT_INPROGRESS;
                }
            else
                {
#endif // RD_MULTIPLE_DRIVE
                textId = R_QTN_FORMAT_INPROGRESS;
#ifdef RD_MULTIPLE_DRIVE
                }
#endif // RD_MULTIPLE_DRIVE
            break;
            }
        case EBackupProcess:
            {
            dialogId = R_FILEMANAGER_PROGRESS_NOTE_WITH_CANCEL;
#ifdef RD_FILE_MANAGER_BACKUP
            textId = R_QTN_FMGR_PROGRESS_PREPARING_BACKUP;
#else // RD_FILE_MANAGER_BACKUP
            textId = R_QTN_BACKUP_INPROGRESS;
#endif // RD_FILE_MANAGER_BACKUP
            aImmediatelyVisible = ETrue;
            break;
            }
        case ERestoreProcess:
            {
            dialogId = R_FILEMANAGER_PROGRESS_NOTE;
#ifdef RD_FILE_MANAGER_BACKUP
            textId = R_QTN_FMGR_PROGRESS_PREPARING_RESTORE;
#else // RD_FILE_MANAGER_BACKUP
            textId = R_QTN_RESTORE_INPROGRESS;
#endif // RD_FILE_MANAGER_BACKUP
            break;
            }
        case EEjectProcess:
            {
            dialogId = R_FILEMANAGER_WAIT_NOTE;
            textId = R_QTN_WAIT_EJECT;
            break;
            }
        case EFileOpenProcess:
            {
            dialogId = R_FILEMANAGER_WAIT_NOTE_OPEN_WITH_CANCEL;
            break;
            }
        default:
            {
            dialogId = R_FILEMANAGER_WAIT_NOTE_OPEN;
            break;
            }
        }
    LaunchProgressBarL(
        dialogId, textId, aFinalValue, aInitialValue, isPeriodic, aImmediatelyVisible );
    
#ifdef RD_FILE_MANAGER_BACKUP
    if ( aOperation == EBackupProcess && iProgressDialog )
        {
        iProgressDialog->ButtonGroupContainer().SetCommandSetL(R_AVKON_SOFTKEYS_EMPTY);
        iProgressDialog->ButtonGroupContainer().DrawDeferred();
        }
#endif // RD_FILE_MANAGER_BACKUP
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::ShowWaitDialogL
//
// ------------------------------------------------------------------------------
//
void CFileManagerViewBase::ShowWaitDialogL( MAknBackgroundProcess& aProcess)
    {
    CAknWaitNoteWrapper* waitNoteWrapper = CAknWaitNoteWrapper::NewL();
    CleanupDeletePushL( waitNoteWrapper );
    waitNoteWrapper->ExecuteL( R_FILEMANAGER_WAIT_NOTE_PROCESS, aProcess );
    CleanupStack::PopAndDestroy( waitNoteWrapper );
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::DoUpdateProgressBar
//
// ------------------------------------------------------------------------------
//
void CFileManagerViewBase::DoUpdateProgressBar()
    {
    // Update progress indicator
    if ( iProgressDialog && iProgressInfo )
        {
          iProgressInfo->SetAndDraw( iTotalTransferredBytes / KMinificationFactor );
        }
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::UpdateProgressBar
//
// ------------------------------------------------------------------------------
//
TInt CFileManagerViewBase::UpdateProgressBar(TAny* aPtr )
    {
    static_cast< CFileManagerViewBase* >( aPtr )->DoUpdateProgressBar();
    return KErrNone;
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::RefreshStartedL
//
// ------------------------------------------------------------------------------
//
void CFileManagerViewBase::RefreshStartedL()
    {
    FUNC_LOG

    DenyDirectoryRefresh( EFalse );

    if ( static_cast< CFileManagerAppUi* >( AppUi() )->IsFmgrForeGround() )
        {
        if ( iContainer )
            {
            iContainer->SetListEmptyL();
            }

        ClearProgressBarL();
        if ( Id() == CFileManagerAppUi::KFileManagerSearchResultsViewId )
            {
            // On remote drives local find progress note is sometimes 
            // totally blocked if the note is not started directly here.
            iProgressDialogRefresh = new( ELeave ) CAknProgressDialog(
                reinterpret_cast< CEikDialog** >(
                    &iProgressDialogRefresh ), EFalse );
            iProgressDialogRefresh->SetCallback( this );
            iProgressDialogRefresh->ExecuteLD( R_FILEMANAGER_FIND_WAIT_DIALOG );
            }
        else
            {
            // Start progress dialog using own timer, otherwise progress dialog
            // burns sometimes quite a lot CPU time even if it is not
            // visible at all.
            iRefreshProgressDelayedStart = CPeriodic::NewL(
                CActive::EPriorityUserInput );
            iRefreshProgressDelayedStart->Start(
                KRefreshProgressStartDelay,
                KRefreshProgressStartDelay,
                TCallBack( RefreshProgressDelayedStart, this ) );
            }
        }
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::RefreshStoppedL
//
// ------------------------------------------------------------------------------
//
void CFileManagerViewBase::RefreshStoppedL()
    {
    FUNC_LOG

    ClearProgressBarL();

    if( iContainer )
        {
        TInt index( iContainer->ListBoxCurrentItemIndex() );
        if ( index > 0 && index < iContainer->ListBoxNumberOfItems() )
            {
            iIndex = index;
            }
        DirectoryChangedL();
        UpdateCbaL();
        }
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::AreChosenFilesProtectedL
//
// ------------------------------------------------------------------------------
//
TBool CFileManagerViewBase::AreChosenFilesProtectedL( TBool aMode )
    {
    TBool ret = aMode;
    CArrayFixFlat<TInt>* indexArray = MarkedArrayLC();

    TInt i( 0 );

#ifdef __KEEP_DRM_CONTENT_ON_PHONE
    TBool protectedFile( EFalse );
#endif // __KEEP_DRM_CONTENT_ON_PHONE

    while( ( ret == aMode ) && i < indexArray->Count() )
        {
        CFileManagerItemProperties* prop = iEngine.GetItemInfoL( indexArray->At( i ));
        CleanupStack::PushL( prop );

#ifdef __KEEP_DRM_CONTENT_ON_PHONE
        // Only one folder can be selected at a time
        if (iEngine.IsFolder(indexArray->At( i )))
            {

            CDirScan *dirScan = CDirScan::NewLC( iEikonEnv->FsSession() );
            CDir *currentDir = NULL;
            // Go through the files only
            dirScan->SetScanDataL( prop->FullPath(), KEntryAttNormal, ESortNone );

            dirScan->NextL( currentDir );
            while ( ( ret == aMode ) && currentDir )
                {
                CleanupStack::PushL( currentDir );  // currentDir won't be null
                                                    // due to while loop conditional
                TInt j( 0 );
                while ( ( ret == aMode ) && j < currentDir->Count() )
                    {
                    const TEntry &currentFile( ( *currentDir )[ j ] );
                    TPtrC currentPath (dirScan->FullPath());
                    HBufC* currentFilePath = HBufC::NewLC( KMaxFileName );
                    TPtr completeFilePath = currentFilePath->Des();
                    completeFilePath.Append(currentPath);
                    completeFilePath.Append(currentFile.iName);


                    // the following could leave if file is opened in exclusive
                    // mode by another app- will cause 'in use' error dialog
                    //to be displayed
                    User::LeaveIfError( iEngine.IsDistributableFile( completeFilePath,
                                                                      protectedFile ));
                      if( protectedFile == !aMode )
                        {
                        ret = !aMode;
                        }
                    ++j;
                    CleanupStack::PopAndDestroy( currentFilePath );
                    currentFilePath = NULL;
                    }
                CleanupStack::PopAndDestroy( currentDir );
                currentDir=NULL;
                dirScan->NextL( currentDir );
                }
            CleanupStack::PopAndDestroy( dirScan );
            dirScan = NULL;
            }
        else
            {

            // the following could leave if file is opened in exclusive
            // mode by another app- will cause 'in use' error dialog
            //to be displayed
            User::LeaveIfError( iEngine.IsDistributableFile( prop->FullPath(),
                                                              protectedFile ));
            if( protectedFile == !aMode )
                {
                ret = !aMode;
                }
            }

#else // __KEEP_DRM_CONTENT_ON_PHONE
        if ( ( (prop->TypeL() & CFileManagerItemProperties::EForwardLocked)
                 == CFileManagerItemProperties::EForwardLocked ) == !aMode )
            {
            ret = !aMode;
            }
#endif // __KEEP_DRM_CONTENT_ON_PHONE
        CleanupStack::PopAndDestroy( prop );
        ++i;
        }
    CleanupStack::PopAndDestroy( indexArray );
    return ret;
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::ShowContextSensitiveMenuL
//
// ------------------------------------------------------------------------------
//
void CFileManagerViewBase::ShowContextSensitiveMenuL()
    {
    CEikMenuBar* menu = MenuBar();
    // set context sensitive menu
    menu->SetMenuTitleResourceId( R_FILEMANAGER_CONTEXT_SENSITIVE_MENUBAR );
    // show context sensitive menu
    TRAPD( err, menu->TryDisplayMenuBarL() );
    menu->SetMenuTitleResourceId( R_FILEMANAGER_MEMORY_STORE_MENUBAR );
    User::LeaveIfError( err );
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::ClearProgressBarL
//
// ------------------------------------------------------------------------------
//
void CFileManagerViewBase::ClearProgressBarL()
    {
    FUNC_LOG

    iProgressInfo = NULL;

    if ( iProgressDialog )
        {
        iProgressDialog->ProcessFinishedL();
        iProgressDialog = NULL;
        }
    if ( iProgressDialogRefresh )
        {
        iProgressDialogRefresh->ProcessFinishedL();
        iProgressDialogRefresh = NULL;
        }
    delete iPeriodic;
    iPeriodic = NULL;

    delete iRefreshProgressDelayedStart;
    iRefreshProgressDelayedStart = NULL;

    iTotalTransferredBytes = 0;
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::StoreIndex
//
// ------------------------------------------------------------------------------
//
TBool CFileManagerViewBase::StoreIndex()
    {
    if ( iContainer )
        {
        TInt index( iContainer->ListBoxCurrentItemIndex() );
        if ( index >= 0 &&
            index < iContainer->ListBoxNumberOfItems() )
            {
            iIndex = index; // Store view's internal index

            // Store navigation index
            TUid viewId( Id() );
            if ( viewId == CFileManagerAppUi::KFileManagerMemoryStoreViewId ||
                 viewId == CFileManagerAppUi::KFileManagerFoldersViewId )
                {
                iEngine.SetCurrentIndex( index );
                }
            }
        }
    else
        {
        return EFalse;
        }
    return ETrue;
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::DriveReadOnlyMmcL
//
// ------------------------------------------------------------------------------
//
TBool CFileManagerViewBase::DriveReadOnlyMmcL( const TInt aDrive ) const
    {
    TBool ret( EFalse );
    TUint32 drvState( 0 );
    TInt err( iEngine.DriveState( drvState, aDrive ) );
    if ( err == KErrNone &&
        ( drvState & TFileManagerDriveInfo::EDriveWriteProtected ) )
        {
        ret = ETrue;
        }
    if ( ret )
        {
#ifdef RD_MULTIPLE_DRIVE
        HBufC* text = iEngine.GetFormattedDriveNameLC(
            aDrive,
            R_QTN_MEMC_MULTIPLE_MEMC_READ_ONLY );
        FileManagerDlgUtils::ShowErrorNoteL( *text );
        CleanupStack::PopAndDestroy( text );
#else // RD_MULTIPLE_DRIVE
        FileManagerDlgUtils::ShowErrorNoteL(
            R_QTN_MEMC_MEMORYCARD_READ_ONLY );
#endif // RD_MULTIPLE_DRIVE
        }

    return ret;
    }


// ------------------------------------------------------------------------------
// CFileManagerViewBase::DriveReadOnlyMmcL
//
// ------------------------------------------------------------------------------
//
TBool CFileManagerViewBase::DriveReadOnlyMmcL( const TDesC& aFullPath ) const
    {
    TBool ret( EFalse );
    if ( aFullPath.Length() )
        {
        TInt drive = TDriveUnit( aFullPath );
        ret = DriveReadOnlyMmcL( drive );
        }
    return ret;
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::CurrentProcess
//
// ------------------------------------------------------------------------------
//
MFileManagerProcessObserver::TFileManagerProcess CFileManagerViewBase::CurrentProcess()
    {
    return iActiveProcess;
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::Error
//
// ------------------------------------------------------------------------------
//
void CFileManagerViewBase::Error( TInt aError )
    {
    if ( aError != KErrNone )
        {
        ERROR_LOG1( "CFileManagerViewBase::Error()-aError=%d", aError )
        iEikonEnv->HandleError( aError );
        }
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::AddSendOptionL
//
// ------------------------------------------------------------------------------
//
void CFileManagerViewBase::AddSendOptionL(
        CEikMenuPane& aMenuPane,
        const TInt aCommandIdAfter )
    {
    CSendUi& sendUi( static_cast< CFileManagerAppUi* >( AppUi() )->SendUiL() );
    TInt pos( 0 );
    aMenuPane.ItemAndPos( aCommandIdAfter, pos );
    CArrayFixFlat< TInt >* indexArray = MarkedArrayLC();
    TInt msgSize( KMessageSize );
    if ( indexArray->Count() == 1 && 
         !iEngine.IsFolder( indexArray->At( 0 ) ) )
        {
        msgSize = Int64ToInt( iEngine.GetFileSizesL( *indexArray ) );
        }
    CleanupStack::PopAndDestroy( indexArray );
    TSendingCapabilities caps(
        0, msgSize, TSendingCapabilities::ESupportsAttachments );
    sendUi.AddSendMenuItemL( aMenuPane, pos, EFileManagerSend, caps );
    aMenuPane.SetItemSpecific(EFileManagerSend, ETrue);
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::DeleteStatusNotOk
//
// ------------------------------------------------------------------------------
//
TBool CFileManagerViewBase::DeleteStatusNotOkL(
        CFileManagerItemProperties& aProp, TInt aSelectionCount ) const
    {
    if ( DriveReadOnlyMmcL( aProp.FullPath() ) )
        {
        // Can't delete from read-only MMC card
        return ETrue;
        }

    TUint32 itemType( aProp.TypeL() );
    if ( !aSelectionCount &&
        ( itemType & KDefaultFolderMask ) == KDefaultFolderMask )
        {
        // Can't delete default folder
        FileManagerDlgUtils::ShowErrorNoteL(
            R_QTN_FMGR_ERROR_DEL_DEF_FLDR );
        return ETrue;
        }
    if ( aSelectionCount <= 1 &&
        ( itemType & CFileManagerItemProperties::EOpen ) )
        {
        // Can't delete open file
        FileManagerDlgUtils::ShowErrorNoteL(
            R_QTN_FMGR_ERROR_DELETE_FILE_OPEN );
        return ETrue;
        }

    return EFalse;
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::DeleteItemsL
//
// ------------------------------------------------------------------------------
//
void CFileManagerViewBase::DeleteItemsL( TInt aIndex )
    {
    
    CArrayFixFlat<TInt>* deleteArray = MarkedArrayLC();
    TInt newIndex( MinIndex( *deleteArray ) );
    delete iActiveDelete;
    iActiveDelete = NULL;
    iActiveDelete = iEngine.CreateActiveDeleteL( *deleteArray );
    delete iWaitNoteWrapper;
    iWaitNoteWrapper = NULL;
    iWaitNoteWrapper = CAknWaitNoteWrapper::NewL();
    iActiveProcess = EDeleteProcess;
    TRAPD( err, iWaitNoteWrapper->ExecuteL(
        R_FILEMANAGER_DELETE_WAIT_DIALOG, *iActiveDelete ) );
    iActiveProcess = ENoProcess;
    User::LeaveIfError( err );

    HBufC* fileNameBuf = HBufC::NewLC( KMaxFileName );
    TPtr fileName( fileNameBuf->Des() );

    err = iActiveDelete->GetError( fileName );

    switch ( err )
        {
        case KErrInUse:
        case KErrFmgrSeveralFilesInUse:
            {
            ERROR_LOG1( "CFileManagerViewBase::DeleteItemsL()-err=%d", err )
            if ( iEngine.IsFolder( aIndex ) )
                {
                if ( err == KErrFmgrSeveralFilesInUse )
                    {
                    FileManagerDlgUtils::ShowErrorNoteL(
                        R_QTN_FMGR_ERROR_DEL_FLDR_OPEN_SE );
                    }
                else
                    {
                    FileManagerDlgUtils::ShowErrorNoteL(
                        R_QTN_FMGR_ERROR_DEL_FLDR_OPEN_1 );
                    }
                }
            else
                {
                FileManagerDlgUtils::ShowErrorNoteL(
                    R_QTN_FMGR_ERROR_DELETE_FILE_OPEN );
                }
            break;
            }
        case KErrNone:
            {
            if ( iContainer )
                {
                //CEikListBox& listBox( iContainer->ListBox() );
                //AknSelectionService::HandleItemRemovalAndPositionHighlightL(  
                //    &listBox, listBox.CurrentItemIndex(), *deleteArray);

#ifndef RD_DRM_RIGHTS_MANAGER_REMOVAL
                if ( FeatureManager().IsDrmFullSupported() )
                    {
                    TInt deletedItems( 0 );
                    TInt deletedDrmItems( iActiveDelete->DeletedDrmItems( deletedItems ) );
                    if( deletedDrmItems )
                        {
                        if( deletedDrmItems > 1 )
                            {
                            FileManagerDlgUtils::ShowInfoQueryL(
                                R_QTN_DRM_MOS_DELETED, deletedDrmItems );
                            }
                        else
                            {
                            FileManagerDlgUtils::ShowInfoQueryL(
                                R_QTN_DRM_MO_DELETED, fileName );
                            }
                        }
                    }
#endif // RD_DRM_RIGHTS_MANAGER_REMOVAL

                // Set focus to the item after selection
                iContainer->SetIndex( newIndex );
                }
            break;
            }
        default:
            {
            ERROR_LOG1( "CFileManagerViewBase::DeleteItemsL()-err=%d", err )
            FileManagerDlgUtils::ShowErrorNoteL(
                R_QTN_FLDR_CANT_DELETE_ITEM, fileName );
            break;
            }
        }
    CleanupStack::PopAndDestroy( fileNameBuf );
    CleanupStack::PopAndDestroy( deleteArray );
    StoreIndex();
    iEngine.SetObserver( this );
    iEngine.RefreshDirectory();
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::HasInfoUrlL
//
// ------------------------------------------------------------------------------
//
TBool CFileManagerViewBase::HasInfoUrlL( TInt aIndex )
    {
    if ( iEngine.IsFolder( aIndex ) )
        {
        return EFalse;
        }
    TBool hasUrl( EFalse );
    HBufC8* url = NULL;
    HBufC* fullPath = iEngine.IndexToFullPathLC( aIndex );
    CDRMHelper* drmHelper = CDRMHelper::NewLC( *iEikonEnv );

    TRAPD( err, hasUrl = drmHelper->HasInfoUrlL( *fullPath, url ) );
    if ( hasUrl && url && err == KErrNone )
        {
        hasUrl = url->Length() > 0;
        }
    else
        {
        hasUrl = EFalse;
        }

    ERROR_LOG2( "CFileManagerViewBase::HasInfoUrlL()-hasUrl=%d,err=%d", hasUrl, err )

    delete url;
    CleanupStack::PopAndDestroy( drmHelper );
    CleanupStack::PopAndDestroy( fullPath );
    return hasUrl;
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::OpenInfoUrlL
//
// ------------------------------------------------------------------------------
//
void CFileManagerViewBase::OpenInfoUrlL( TInt aIndex )
    {
    if ( iEngine.IsFolder( aIndex ) )
        {
        return;
        }
    HBufC* fullPath = iEngine.IndexToFullPathLC( aIndex );
    CDRMHelper* drmHelper = CDRMHelper::NewLC( *iEikonEnv );

    // Call returns after browser has been closed
#ifdef FILE_MANAGER_ERROR_LOG_ENABLED
    TRAPD( err, drmHelper->OpenInfoUrlL( *fullPath ) );
    ERROR_LOG1( "CFileManagerViewBase::OpenInfoUrlL()-err=%d", err )
#else // FILE_MANAGER_ERROR_LOG_ENABLED
    TRAP_IGNORE( drmHelper->OpenInfoUrlL( *fullPath ) );
#endif // FILE_MANAGER_ERROR_LOG_ENABLED

    CleanupStack::PopAndDestroy( drmHelper );
    CleanupStack::PopAndDestroy( fullPath );
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::CheckFileRightsAndInformIfExpiredL
//
// ------------------------------------------------------------------------------
//
TBool CFileManagerViewBase::CheckFileRightsAndInformIfExpiredL(
        const TDesC& aFullPath )
    {
    if ( !FeatureManager().IsDrmFullSupported() )
        {
        return ETrue;
        }
    TBool expired( EFalse );
    TBool wmDrm( IsWmDrmFile( aFullPath ) );
    if ( !wmDrm ) // Ignore WM DRM files
        {
        TBool dummy( EFalse );
        CDRMHelperRightsConstraints* dummy2 = NULL;
        CDRMHelperRightsConstraints* dummy3 = NULL;
        CDRMHelperRightsConstraints* dummy4 = NULL;
        CDRMHelperRightsConstraints* dummy5 = NULL;
        CDRMHelper* drmHelper = CDRMHelper::NewLC( *iEikonEnv );
        TRAPD( err, drmHelper->GetRightsDetailsL(
            aFullPath, 0, expired, dummy, dummy2, dummy3, dummy4, dummy5 ) );
        delete dummy2;
        delete dummy3;
        delete dummy4;
        delete dummy5;
        if ( expired )
            {
            err = KErrCANoPermission;
            }
        if ( err == KErrCANoRights || err == KErrCANoPermission )
            {
            // Rights expired or missing, show note or try get silent rights
            expired = ETrue;
            ERROR_LOG1( "CFileManagerViewBase::CheckFileRightsAndInformIfExpiredL-err=%d",
                err )
            HBufC8* previewUri = NULL;
            if ( drmHelper->HandleErrorOrPreviewL( err, aFullPath, previewUri ) == KErrNone )
                {
                expired = EFalse;
                }
            delete previewUri; // Not needed
            }
        CleanupStack::PopAndDestroy( drmHelper );
        }
    ERROR_LOG2(
        "CFileManagerViewBase::CheckFileRightsAndInformIfExpiredL-expired=%d,wmDrm=%d",
        expired, wmDrm )
    return !expired;
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::HandleFileNotFoundL
//
// ------------------------------------------------------------------------------
//
TBool CFileManagerViewBase::HandleFileNotFoundL( TInt aError )
    {
    if ( aError == KErrNotFound )
        {
        iEngine.SetObserver( this );
        iEngine.RefreshDirectory();
        return ETrue;
        }
    if ( aError == KErrPathNotFound )
        {
        TInt count( iEngine.FolderLevel() );
        TBool connectedRemoteDrive( EFalse );
        RefreshDriveInfoL();

        TFileManagerDriveInfo& drvInfo( DriveInfo() );
        TBool remoteDrive( EFalse );
        // Check if drive is remote drive and is it connected or not
        if ( drvInfo.iState & TFileManagerDriveInfo::EDriveRemote )
            {
            remoteDrive = ETrue;
            if ( drvInfo.iState & TFileManagerDriveInfo::EDriveConnected )
                {
                connectedRemoteDrive = ETrue;
                }
            }
        // Check if drive has been disconnected and reconnect canceled
        if ( remoteDrive && !connectedRemoteDrive )
            {
            // Do only root refresh if user is already got back in main view
            if ( Id() == CFileManagerAppUi::KFileManagerMainViewId && 
                iEngine.NavigationLevel() < 0 )
                {
                iEngine.SetObserver( this );
                iEngine.RefreshDirectory();
                }
            // Open memory store view to show not connected.
            else if ( Id() != CFileManagerAppUi::KFileManagerMemoryStoreViewId )
                {
                iIndex = 0;
                static_cast< CFileManagerAppUi* >( AppUi() )->ActivateMemoryStoreViewL();
                }
            else
                {
                iIndex = 0;
                if ( iContainer )
                    {
                    iContainer->SetListEmptyL();
                    }
                DirectoryChangedL();
                }
            }
        // Check if fetch was canceled in connected memory store view
        else if ( connectedRemoteDrive &&
                 Id() == CFileManagerAppUi::KFileManagerMemoryStoreViewId )
            {
            static_cast< CFileManagerAppUi* >( AppUi() )->ActivateMainViewL();
            }
        else if ( count > 0 &&
            ( connectedRemoteDrive ||
              !BaflUtils::PathExists(
                iEikonEnv->FsSession(), iEngine.CurrentDirectory() ) ) )
            {
            // Go back to last valid folder
            CFileManagerAppUi* appUi =
                static_cast< CFileManagerAppUi* >( AppUi() );
            TInt err( KErrNone );
            for ( TInt i( 0 ); i < count; i++ )
                {
                TRAP( err, iEngine.BackstepL() );
                if ( err == KErrNone )
                    {
                    break;
                    }
                }
            if ( iEngine.FolderLevel() ||
                 iEngine.State() == CFileManagerEngine::ESearch )
                {
                iEngine.SetObserver( this );
                iEngine.RefreshDirectory();
                }
            else
                {
                appUi->CloseFoldersViewL();
                }
            }
        else
            {
            // Refresh root folder
            iEngine.SetObserver( this );
            iEngine.RefreshDirectory();
            }
        return ETrue;
        }
    return EFalse;
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::ScreenDeviceChanged
//
// ------------------------------------------------------------------------------
//
void CFileManagerViewBase::ScreenDeviceChanged()
    {
    if ( iContainer )
        {
        iContainer->SetRect( ClientRect() );
        iContainer->DrawDeferred();
        }
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::NotifyL
//
// ------------------------------------------------------------------------------
//
TInt CFileManagerViewBase::NotifyL( TFileManagerNotify aType,
        TInt aData, const TDesC& aName )
    {
    TInt ret( KErrNone );
    switch ( aType )
        {
        case ENotifyDisksChanged:
            {
            if ( iDirectoryRefreshDenied )
                {
                iDirectoryRefreshPostponed = ETrue;
                }
            else if ( iActiveProcess == ENoProcess && !IsRefreshInProgress() )
                {
                StoreIndex();
                iEngine.SetObserver( this );
                iEngine.RefreshDirectory();
                }
#ifndef RD_MULTIPLE_DRIVE
            else if ( iActiveProcess == EEjectProcess )
                {
                // Memory card was put back, complete query
                if ( iEngine.AnyEjectableDrivePresent() )
                    {
                    delete iEjectQueryDialog;
                    iEjectQueryDialog = NULL;
                    }
                }
#endif // RD_MULTIPLE_DRIVE
            break;
            }
        case ENotifyBackupMemoryLow:
            {
            if( aData < KEstimateLowerLimit )
                {
                ret = KErrDiskFull;
                }
            else if( aData < KEstimateUpperLimit )
                {

#ifdef RD_FILE_MANAGER_BACKUP
                CFileManagerBackupSettings& settings( iEngine.BackupSettingsL() );
#endif // RD_FILE_MANAGER_BACKUP

                if ( !FileManagerDlgUtils::ShowConfirmQueryWithYesNoL(
#ifdef RD_FILE_MANAGER_BACKUP
                        R_QTN_FMGR_BACKUP_TIGHT_MEMORY,
                        iEngine.DriveName( settings.TargetDrive() )
#else // RD_FILE_MANAGER_BACKUP
                        R_QTN_CONFIRM_BACKUP_LEVEL1
#endif // RD_FILE_MANAGER_BACKUP
                     ) )
                    {
                    ret = KErrCancel;
                    }
                }
            break;
            }
        case ENotifyForcedFormat:
            {
            StopProgressDialogAndStoreValues();

            TInt textId( R_QTN_CONFIRM_FORMAT_TEXT2 );
#ifdef RD_MULTIPLE_DRIVE
            if ( DriveInfo().iState & TFileManagerDriveInfo::EDriveMassStorage )
                {
                textId = R_QTN_FMGR_FORMAT_MASS_QUERY2;
                }
#endif // RD_MULTIPLE_DRIVE

            TBool query( FileManagerDlgUtils::ShowConfirmQueryWithYesNoL( textId ) );
            LaunchProgressDialogL(
                iProgressFinalValue, iProgressCurrentValue, iActiveProcess );
            return query;
            }
        case ENotifyFileOpenDenied:
            {
            TBool launchProgress( StopProgressDialogAndStoreValues() );

            ret = !CheckFileRightsAndInformIfExpiredL( aName );
            if ( launchProgress )
                {
                LaunchProgressDialogL(
                    iProgressFinalValue, iProgressCurrentValue, iActiveProcess );
                }
            break;
            }
        default:
            {
            break;
            }
        }
    return ret;
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::MemoryStoreMenuFilteringL
//
// ------------------------------------------------------------------------------
//
void CFileManagerViewBase::MemoryStoreMenuFilteringL(
        CEikMenuPane& aMenuPane )
    {
    TBool isSearchOn( iEngine.State() == CFileManagerEngine::ESearch );
    TRAP_IGNORE ( RefreshDriveInfoL() );
    TFileManagerDriveInfo& drvInfo( DriveInfo() );   
    TInt driveNumber = drvInfo.iDrive;
    iEngine.GetDriveInfoL(driveNumber,drvInfo);

    // Common remote drive filtering
    RemoteDriveCommonFilteringL( aMenuPane );
    if( iContainer->IsSearchFieldVisible() && 
            !FilesCountInSearchField() )
        {
        aMenuPane.SetItemDimmed( EFileManagerMark, ETrue );
        }
    if ( iContainer->ListBoxNumberOfItems() )
        {
        if ( !iContainer->ListBoxSelectionIndexesCount() )
            {
            TUint32 fileType( iEngine.FileTypeL(
                iContainer->ListBoxCurrentItemIndex() ) );
            if ( ( fileType & KDefaultFolderMask ) == KDefaultFolderMask )
                {
                aMenuPane.SetItemDimmed( EFileManagerMoveToFolder, ETrue );
                }
            }
        }
    else
        {
        aMenuPane.SetItemDimmed( EFileManagerMoveToFolder, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerCopyToFolder, ETrue );
        }
    TInt index(iContainer->ListBoxCurrentItemIndex());
    TUint32 fileType(iEngine.FileTypeL(index));
    if (!(fileType & CFileManagerItemProperties::EFolder))
        {
        aMenuPane.SetItemDimmed(EFileManagerMoveToFolder, ETrue);
        }
    CFileManagerFeatureManager& featureManager( FeatureManager() );
    
#ifdef RD_MULTIPLE_DRIVE
    // No format item for mass storage in embedded mode dimming
    if ( !( drvInfo.iState & TFileManagerDriveInfo::EDriveFormattable ) )
        {
        aMenuPane.SetItemDimmed( EFileManagerFormatMassStorage, ETrue );
        }
#endif // RD_MULTIPLE_DRIVE

    // Memory store specific remote drive filtering
    if ( !featureManager.IsRemoteStorageFwSupported() ||
         !( drvInfo.iState & TFileManagerDriveInfo::EDriveRemote ) ||
         !( drvInfo.iState & TFileManagerDriveInfo::EDriveConnected ) )
        {
        // For disconnected or non remote drive
        aMenuPane.SetItemDimmed( EFileManagerRefreshRemoteDrive, ETrue );
        }

    if ( !featureManager.IsHelpSupported() )
        {
        // No help item dimming
        aMenuPane.SetItemDimmed( EAknCmdHelp, ETrue );
        }
    if ( !featureManager.IsIrdaSupported() )
        {
        // No infra red item dimming
        aMenuPane.SetItemDimmed( EFileManagerReceiveViaIR, ETrue );
        }
    if ( isSearchOn )
        {
        // Search view item dimming
        aMenuPane.SetItemDimmed( EFileManagerRename, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerReceiveViaIR, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerOrganise, ETrue );
//        aMenuPane.SetItemDimmed( EFileManagerMemoryCard, ETrue );
//        aMenuPane.SetItemDimmed( EFileManagerMemoryCardPassword, ETrue );
        //aMenuPane.SetItemDimmed( EFileManagerUnlockMemoryCard, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerRefreshRemoteDrive, ETrue );
#ifdef RD_MULTIPLE_DRIVE
        aMenuPane.SetItemDimmed( EFileManagerFormatMassStorage, ETrue );
#endif // RD_MULTIPLE_DRIVE
        aMenuPane.SetItemDimmed( EFileManagerSort, ETrue );
        }
    else
        {
        aMenuPane.SetItemDimmed( EFileManagerSearchSort, ETrue );
        }
       //dim the item unconditionally 
        aMenuPane.SetItemDimmed( EFileManagerFormatMassStorage, ETrue );
    if ( drvInfo.iState & TFileManagerDriveInfo::EDriveWriteProtected )
        {
        // Write protected item dimming
        aMenuPane.SetItemDimmed( EFileManagerRename, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerDelete, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerReceiveViaIR, ETrue );
//        aMenuPane.SetItemDimmed( EFileManagerMemoryCard, ETrue );
//        aMenuPane.SetItemDimmed( EFileManagerMemoryCardPassword, ETrue );
#ifdef RD_MULTIPLE_DRIVE
        aMenuPane.SetItemDimmed( EFileManagerFormatMassStorage, ETrue );
#endif // RD_MULTIPLE_DRIVE
        }

#ifdef RD_MULTIPLE_DRIVE
    if ( drvInfo.iState & TFileManagerDriveInfo::EDriveMassStorage )
        {
        // Mass storage item dimming
//        aMenuPane.SetItemDimmed( EFileManagerMemoryCard, ETrue );
//        aMenuPane.SetItemDimmed( EFileManagerMemoryCardPassword, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerUnlockMemoryCard, ETrue );

        if ( !( drvInfo.iState & TFileManagerDriveInfo::EDrivePresent ) )
            {
            aMenuPane.SetItemDimmed( EFileManagerFormatMassStorage, ETrue );
            }
        }
    else
#endif // RD_MULTIPLE_DRIVE
    if ( drvInfo.iState & TFileManagerDriveInfo::EDriveRemovable )
        {
        // Memory card item dimming
        if ( !( drvInfo.iState & TFileManagerDriveInfo::EDrivePresent ) ||
             ( drvInfo.iState & ( TFileManagerDriveInfo::EDriveCorrupted |
                                  TFileManagerDriveInfo::EDriveLocked ) ) )
            {
//            aMenuPane.SetItemDimmed( EFileManagerMemoryCardPassword, ETrue );
            aMenuPane.SetItemDimmed( EFileManagerDetails, ETrue );
            }
        if ( ( drvInfo.iState & TFileManagerDriveInfo::EDriveCorrupted ) ||
             !( drvInfo.iState & TFileManagerDriveInfo::EDriveLocked ) )
            {
            aMenuPane.SetItemDimmed( EFileManagerUnlockMemoryCard, ETrue );
            }
//        if ( !( drvInfo.iState & TFileManagerDriveInfo::EDriveLockable ) )
//            {
//            aMenuPane.SetItemDimmed( EFileManagerMemoryCardPassword, ETrue );
//            }
        if ( !featureManager.IsMmcPassWdSupported() )
            {
            //aMenuPane.SetItemDimmed( EFileManagerMemoryCardPassword, ETrue );
            aMenuPane.SetItemDimmed( EFileManagerUnlockMemoryCard, ETrue );
            }
#ifdef RD_MULTIPLE_DRIVE
        aMenuPane.SetItemDimmed( EFileManagerFormatMassStorage, ETrue );
#endif // RD_MULTIPLE_DRIVE
        }
    else
        {
        // No mass storage or memory card item dimming
//        aMenuPane.SetItemDimmed( EFileManagerMemoryCard, ETrue );
//        aMenuPane.SetItemDimmed( EFileManagerMemoryCardPassword, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerUnlockMemoryCard, ETrue );
#ifdef RD_MULTIPLE_DRIVE
        aMenuPane.SetItemDimmed( EFileManagerFormatMassStorage, ETrue );
#endif // RD_MULTIPLE_DRIVE
        }

//    CEikListBox& listBox = iContainer->ListBox();
    TBool dimSend( EFalse );

    if ( iContainer->ListBoxSelectionIndexesCount() )
        {
        // Selections in list
        aMenuPane.SetItemDimmed( EFileManagerOpen, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerRename, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerDetails, ETrue );

        if ( !featureManager.IsDrmFullSupported() &&
            AreChosenFilesProtectedL( ETrue ) )
            {
            dimSend = ETrue;
            }


        // Hide empty details if no item or memory specific details 
        // can be shown.
        if ( isSearchOn ||
            ( drvInfo.iState & TFileManagerDriveInfo::EDriveRemote ) )
            {
            aMenuPane.SetItemDimmed( EFileManagerDetails, ETrue );
            }
        
        if ( !( drvInfo.iState & TFileManagerDriveInfo::EDrivePresent ) || 
        	 ( ( drvInfo.iState & TFileManagerDriveInfo::EDriveRemote ) &&
             !( drvInfo.iState & TFileManagerDriveInfo::EDriveConnected ) ) )
           {
           // Handle unavailable drive OR  disconnected remote drive
           dimSend = ETrue;
           aMenuPane.SetItemDimmed( EFileManagerSort, ETrue );
           aMenuPane.SetItemDimmed( EFileManagerOrganise, ETrue );
           aMenuPane.SetItemDimmed( EFileManagerMark, ETrue );             
           }
        }
    else if ( iContainer->ListBoxNumberOfItems() )
        {
        // Check if there is files on the list
        TInt files( iEngine.FilesInFolderL() );
        if ( !files ) 
            {
            dimSend = ETrue;
            aMenuPane.SetItemDimmed( EFileManagerMark, ETrue );
            }
        
        // There is items in list, check selection type
        TUint32 fileType( iEngine.FileTypeL(
            iContainer->ListBoxCurrentItemIndex() ) );
        if ( ( fileType & KDefaultFolderMask ) == KDefaultFolderMask )
            {
            dimSend = ETrue;
            aMenuPane.SetItemDimmed( EFileManagerDelete, ETrue );
            aMenuPane.SetItemDimmed( EFileManagerRename, ETrue );
            }
        else if ( fileType & CFileManagerItemProperties::EFolder )
            {
            dimSend = ETrue;
            }

        if ( fileType & CFileManagerItemProperties::EPlaylist )
            {
            dimSend = ETrue;
            }

        // When full OMA DRM is in use, it is ok to show send option
        if( ( fileType & CFileManagerItemProperties::EForwardLocked ) &&
            !featureManager.IsDrmFullSupported() )
            {
            dimSend = ETrue;
            }
        if ( ( drvInfo.iState & TFileManagerDriveInfo::EDriveRemote ) &&
             !( drvInfo.iState & TFileManagerDriveInfo::EDriveConnected ) )
            {
            // Handle disconnected remote drive
            dimSend = ETrue;
            }
        }
    else 
        {
        // List is empty
        aMenuPane.SetItemDimmed( EFileManagerOpen, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerDelete, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerMark, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerRename, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerSort, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerSearchSort, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerDetails, ETrue );
        dimSend = ETrue;
        
        if ( drvInfo.iState & TFileManagerDriveInfo::EDriveWriteProtected )
            {
            // Handle write protected drive
            aMenuPane.SetItemDimmed( EFileManagerOrganise, ETrue );
            }
        if ( drvInfo.iState & TFileManagerDriveInfo::EDriveRemote )
            {
            // Handle empty remote folder
            aMenuPane.SetItemDimmed( EFileManagerDetails, ETrue );
            }

        if ( ( drvInfo.iState & TFileManagerDriveInfo::EDriveRemote ) &&
             !( drvInfo.iState & TFileManagerDriveInfo::EDriveConnected ) )
            {
            // Handle disconnected remote drive
            aMenuPane.SetItemDimmed( EFileManagerReceiveViaIR, ETrue );
            aMenuPane.SetItemDimmed( EFileManagerOrganise, ETrue );
//            aMenuPane.SetItemDimmed( EFileManagerDetails, ETrue );
            }
        else if ( !( drvInfo.iState & TFileManagerDriveInfo::EDrivePresent ) ||  
                 ( drvInfo.iState & (
                    TFileManagerDriveInfo::EDriveCorrupted |
                    TFileManagerDriveInfo::EDriveLocked ) ) )
            {
            // Handle unavailable drive
//            aMenuPane.SetItemDimmed( EFileManagerDetails, ETrue );
            aMenuPane.SetItemDimmed( EFileManagerReceiveViaIR, ETrue );
            aMenuPane.SetItemDimmed( EFileManagerOrganise, ETrue );
//            aMenuPane.SetItemDimmed( EFileManagerMemoryCardPassword, ETrue );
            
            if ( drvInfo.iState & TFileManagerDriveInfo::EDriveRemovable &&
                 !( drvInfo.iState & TFileManagerDriveInfo::EDriveLocked ) )
                {
                aMenuPane.SetItemDimmed( EFileManagerUnlockMemoryCard, ETrue );
                }
            }
        else if ( isSearchOn ||
                 !BaflUtils::PathExists(
                    iEikonEnv->FsSession(), iEngine.CurrentDirectory() ) )
            {
            // Handle empty search results and invalid path
            if ( isSearchOn || !iEngine.CurrentDirectory().Length() )
                {
                aMenuPane.SetItemDimmed( EFileManagerReceiveViaIR, ETrue );
                aMenuPane.SetItemDimmed( EFileManagerOrganise, ETrue );
//                aMenuPane.SetItemDimmed( EFileManagerMemoryCardPassword, ETrue );
//                aMenuPane.SetItemDimmed( EFileManagerMemoryCard, ETrue );
                aMenuPane.SetItemDimmed( EFileManagerUnlockMemoryCard, ETrue );
//                aMenuPane.SetItemDimmed( EFileManagerDetails, ETrue );
                }
            else
                {
                // BaflUtils::PathExists does not work for remote drive root dirs.
                if( drvInfo.iState & TFileManagerDriveInfo::EDriveRemote )
                    {
                    _LIT( KRootFolder, "?:\\" );
                    if ( iEngine.CurrentDirectory().MatchF( KRootFolder ) )
                        {
                        User::Leave( KErrPathNotFound );
                        }
                    }
                else
                    {
                    User::Leave( KErrPathNotFound );
                    }
                }
            }
        }

    if ( !dimSend )
        {
        AddSendOptionL( aMenuPane, EFileManagerDelete );
        }
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::OrganiseMenuFilteringL
//
// ------------------------------------------------------------------------------
//
void CFileManagerViewBase::OrganiseMenuFilteringL( CEikMenuPane& aMenuPane )
    {
//    CEikListBox& listBox( iContainer->ListBox() );


    // Search view item dimming
    if( iEngine.State() == CFileManagerEngine::ESearch )
        {
        aMenuPane.SetItemDimmed( EFileManagerNewFolder, ETrue );
        }

    TFileManagerDriveInfo& drvInfo( DriveInfo() );
    if ( drvInfo.iState & TFileManagerDriveInfo::EDriveWriteProtected )
        {
        // Write protected item dimming
        aMenuPane.SetItemDimmed( EFileManagerNewFolder, ETrue );

        }
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::DetailsMenuFilteringL
//
// ------------------------------------------------------------------------------
//
void CFileManagerViewBase::DetailsMenuFilteringL( CEikMenuPane& aMenuPane )
    {
    TInt index( iContainer->ListBoxCurrentItemIndex() );
    TUint32 fileType( iEngine.FileTypeL( index ) );
    if ( fileType & CFileManagerItemProperties::EFolder )
        {
        aMenuPane.SetItemDimmed( EFileManagerFileDetails, ETrue );
        }
    else
        {
        aMenuPane.SetItemDimmed( EFileManagerFolderDetails, ETrue );
        }
    if ( !FeatureManager().IsDrmFullSupported() ||
         !( fileType & CFileManagerItemProperties::EDrmProtected ) ||
        !HasInfoUrlL( index ) )
        {
        aMenuPane.SetItemDimmed( EFileManagerMoreInfoOnline, ETrue );
        }
    }

//// ------------------------------------------------------------------------------
//// CFileManagerViewBase::MemoryCardMenuFilteringL
////
//// ------------------------------------------------------------------------------
////
//void CFileManagerViewBase::MemoryCardMenuFilteringL( CEikMenuPane& aMenuPane )
//    {
//    TFileManagerDriveInfo& drvInfo( DriveInfo() );
//    
//    if ( drvInfo.iState & ( TFileManagerDriveInfo::EDriveCorrupted |
//                            TFileManagerDriveInfo::EDriveLocked |
//                            TFileManagerDriveInfo::EDriveMassStorage ) )
//        {
//        aMenuPane.SetItemDimmed( EFileManagerMemoryCardName, ETrue );
//        aMenuPane.SetItemDimmed( EFileManagerMemoryCardRename, ETrue );
//        }
//    else
//        {
//        if ( drvInfo.iName.Length() )
//            {
//            aMenuPane.SetItemDimmed( EFileManagerMemoryCardName, ETrue );
//            }
//        else
//            {
//            aMenuPane.SetItemDimmed( EFileManagerMemoryCardRename, ETrue );
//            }
//        }
//
//    if ( !( drvInfo.iState & TFileManagerDriveInfo::EDriveFormattable ) )
//        {
//        aMenuPane.SetItemDimmed( EFileManagerMemoryCardFormat, ETrue );
//        }
//    }
//
//// ------------------------------------------------------------------------------
//// CFileManagerViewBase::MemoryCardPasswordMenuFilteringL
////
//// ------------------------------------------------------------------------------
////
//void CFileManagerViewBase::MemoryCardPasswordMenuFilteringL( CEikMenuPane& aMenuPane )
//    {
//    TFileManagerDriveInfo& drvInfo( DriveInfo() );
//    
//    if ( drvInfo.iState & TFileManagerDriveInfo::EDrivePasswordProtected ) 
//        {
//        aMenuPane.SetItemDimmed( EFileManagerMemoryCardPasswordSet, ETrue );
//        }
//    else
//        {
//        aMenuPane.SetItemDimmed( EFileManagerMemoryCardPasswordChange, ETrue );
//        aMenuPane.SetItemDimmed( EFileManagerMemoryCardPasswordRemove, ETrue );
//        }
//    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::ContextSensitiveMenuFilteringL
//
// ------------------------------------------------------------------------------
//
void CFileManagerViewBase::ContextSensitiveMenuFilteringL( CEikMenuPane& aMenuPane )
    {
    TFileManagerDriveInfo& drvInfo( DriveInfo() );
    TInt driveNumber = drvInfo.iDrive;
    iEngine.GetDriveInfoL(driveNumber,drvInfo);

    // Check if there are files to send
    TInt dummy( 0 );
    CArrayFixFlat< TInt >* files = GetSendFilesLC( dummy );
    
    TBool dimSend( EFalse );
    if ( ( drvInfo.iState & TFileManagerDriveInfo::EDriveRemote ) &&
         !( drvInfo.iState & TFileManagerDriveInfo::EDriveConnected ) )
        {
        dimSend = ETrue;
        }
    
    if ( files->Count() && !dimSend )
        {
        AddSendOptionL( aMenuPane, EFileManagerOrganise );
        }
    CleanupStack::PopAndDestroy( files );

    if ( drvInfo.iState & TFileManagerDriveInfo::EDriveWriteProtected )
        {
        // Write protected item dimming
        aMenuPane.SetItemDimmed( EFileManagerMemoryStorageFormat, ETrue );
        }

    if ( iEngine.State() == CFileManagerEngine::ESearch ||
         !( drvInfo.iState & TFileManagerDriveInfo::EDriveRemovable ) )
        {
        // No memory card item dimming
        aMenuPane.SetItemDimmed( EFileManagerUnlockMemoryCard, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerMemoryStorageFormat, ETrue );
        }
    else
        {
        // Memory card item dimming
        if ( !( drvInfo.iState & TFileManagerDriveInfo::EDriveLocked ) )
            {
            aMenuPane.SetItemDimmed( EFileManagerUnlockMemoryCard, ETrue );
            }
        if ( !( drvInfo.iState & TFileManagerDriveInfo::EDriveCorrupted ) ||
             !( drvInfo.iState & TFileManagerDriveInfo::EDriveFormattable ) )
            {
            aMenuPane.SetItemDimmed( EFileManagerMemoryStorageFormat, ETrue );
            }
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::DriveInfo
// 
// -----------------------------------------------------------------------------
// 
TFileManagerDriveInfo& CFileManagerViewBase::DriveInfo() const
    {
    return static_cast< CFileManagerAppUi* >( AppUi() )->DriveInfo();
    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::RefreshDriveInfoL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerViewBase::RefreshDriveInfoL()
    {
    if ( !iEngine.CurrentDirectory().Length() )
        {
        return;
        }
    iEngine.GetDriveInfoL( DriveInfo() );
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::StartProcessL
//
// ------------------------------------------------------------------------------
//
void CFileManagerViewBase::StartProcessL(
        MFileManagerProcessObserver::TFileManagerProcess aProcess,
        TInt aValue )
    {
    if ( iActiveProcess != ENoProcess )
        {
        return;
        }
    LaunchProgressDialogL( KMaxTInt, 0, aProcess );
    iEngine.SetObserver( this );
    iActiveProcess = aProcess;
    TInt err( KErrNone );
    switch ( aProcess )
        {
        case EFormatProcess:
            {
            TRAP( err, iEngine.StartFormatProcessL( aValue ) );
            break;
            }
        case EBackupProcess:
        case ERestoreProcess: // FALLTHROUGH
            {
            CFileManagerAppUi* appUi = static_cast< CFileManagerAppUi* >( AppUi() );
            appUi->BackupOrRestoreStarted();
            
            TRAP( err, iEngine.StartBackupProcessL( aProcess ) );
            break;
            }
        case EEjectProcess:
            {
            TRAP( err, iEngine.StartEjectProcessL( aValue ) );
            break;
            }
        default:
            {
            TRAP( err, ClearProgressBarL() );
            iActiveProcess = ENoProcess;
            break;
            }
        }
    if ( err != KErrNone )
        {
        // Clean up the active process before forwarding leave
        ERROR_LOG2(
            "CFileManagerViewBase::StartProcessL-aProcess=%d,err=%d",
            aProcess, err )
        iActiveProcess = ENoProcess;
        User::Leave( err );
        }
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::CmdUnlockDriveL
//
// ------------------------------------------------------------------------------
//
void CFileManagerViewBase::CmdUnlockDriveL()
    {
    TFileManagerDriveInfo drvInfo;
    if ( DriveInfoAtCurrentPosL( drvInfo ) < 0 )
        {
        return; // No drive selected
        }

    if ( !UnlockRemovePasswordL( drvInfo.iDrive, EFalse ) ) // Unlock only
        {
        RefreshDriveInfoL();
        iEngine.SetObserver( this );
        iEngine.RefreshDirectory();
        }
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::CmdFormatDriveL
//
// ------------------------------------------------------------------------------
//
void CFileManagerViewBase::CmdFormatDriveL()
    {
    StoreIndex();
    TFileManagerDriveInfo drvInfo;
    if ( DriveInfoAtCurrentPosL( drvInfo ) < 0 )
        {
        return; // No drive selected
        }

    if ( !( drvInfo.iState & ( TFileManagerDriveInfo::EDriveRemovable |
                               TFileManagerDriveInfo::EDriveFormattable ) ) ||
        ( drvInfo.iState & TFileManagerDriveInfo::EDriveWriteProtected ) )
        {
        FileManagerDlgUtils::ShowErrorNoteL(
            R_QTN_MEMORYCARD_READONLY );
        return;
        }

    TBool query( EFalse );
#ifdef RD_MULTIPLE_DRIVE
    if ( drvInfo.iState & TFileManagerDriveInfo::EDriveUsbMemory )
        {
        HBufC* text = iEngine.GetFormattedDriveNameLC(
            drvInfo.iDrive, R_QTN_FMGR_USB_MEMORY_FORMAT_QUERY );
        query = FileManagerDlgUtils::ShowConfirmQueryWithYesNoL( *text );
        CleanupStack::PopAndDestroy( text );
        }
    else if ( drvInfo.iState & TFileManagerDriveInfo::EDriveMassStorage )
        {
        HBufC* text = iEngine.GetFormattedDriveNameLC(
            drvInfo.iDrive, R_QTN_FMGR_FORMAT_MASS_QUERY1 );
        query = FileManagerDlgUtils::ShowConfirmQueryWithYesNoL( *text );
        CleanupStack::PopAndDestroy( text );
        }
    else
        {
#endif // RD_MULTIPLE_DRIVE
        query = FileManagerDlgUtils::ShowConfirmQueryWithYesNoL(
            R_QTN_CONFIRM_FORMAT_TEXT );
#ifdef RD_MULTIPLE_DRIVE
        }
#endif // RD_MULTIPLE_DRIVE

    if ( query )
        {
        StartProcessL( EFormatProcess, drvInfo.iDrive );
        }
    }

//// ------------------------------------------------------------------------------
//// CFileManagerViewBase::CmdRenameDriveL
////
//// ------------------------------------------------------------------------------
////
//void CFileManagerViewBase::CmdRenameDriveL()
//    {
//    TFileManagerDriveInfo& drvInfo( DriveInfo() );
//    if ( drvInfo.iState & TFileManagerDriveInfo::EDriveWriteProtected )
//        {
//        FileManagerDlgUtils::ShowErrorNoteL( R_QTN_MEMORYCARD_READONLY );
//        return;
//        }
//    StoreIndex();
//    RenameDriveL( EFalse );
//    iEngine.SetObserver( this );
//    iEngine.RefreshDirectory();
//    }
//
//// ------------------------------------------------------------------------------
//// CFileManagerViewBase::CmdSetDrivePasswordL
////
//// ------------------------------------------------------------------------------
////
//void CFileManagerViewBase::CmdSetDrivePasswordL()
//    {
//    TBuf< KFmgrMaxMediaPassword > nullPwd;
//    TBuf< KFmgrMaxMediaPassword > pwd;
//    TInt ret( KErrNone );
//    if( FileManagerDlgUtils::ShowPasswordQueryL( pwd ) )
//        {
//        EmptyPwd( nullPwd );
//        ret = UpdatePasswordL( nullPwd, pwd );
//        if( ret == KErrNone )
//            {
//            FileManagerDlgUtils::ShowConfirmNoteL( R_QTN_PASSWORD_SET_TEXT );
//            RefreshDriveInfoL();
//            }
//        else
//            {
//            FileManagerDlgUtils::ShowErrorNoteL( R_QTN_CRITICAL_ERROR );
//            }
//        }
//    }
//
//// ------------------------------------------------------------------------------
//// CFileManagerViewBase::CmdChangeDrivePasswordL
////
//// ------------------------------------------------------------------------------
////
//void CFileManagerViewBase::CmdChangeDrivePasswordL()
//    {
//    TBuf< KFmgrMaxMediaPassword > pwd;
//    TBuf< KFmgrMaxMediaPassword > oldPwd;
//    TBool isDone( EFalse );
//    TBool isCanceled( EFalse );
//    TInt err( KErrNone );
//
//    // Ask for the old password until the correct one is given
//    while( !isDone )
//        {
//        EmptyPwd( oldPwd );
//        if( FileManagerDlgUtils::ShowSimplePasswordQueryL(
//                R_QTN_PASSWORD_OLD_TEXT, oldPwd ) )
//            {
//            err = UpdatePasswordL( oldPwd, oldPwd );
//            if( err == KErrNone )
//                {
//                isDone = ETrue;
//                }
//            else
//                {
//                FileManagerDlgUtils::ShowErrorNoteL(
//                    R_QTN_PASSWORDS_WRONG_TEXT );
//                }
//            }
//        else
//            {
//            isDone = ETrue;
//            isCanceled = ETrue;
//            }
//        }
//
//    // Then query for the new password
//    if( !isCanceled )
//        {
//        if( FileManagerDlgUtils::ShowPasswordQueryL( pwd ) )
//            {
//            err = UpdatePasswordL( oldPwd, pwd );
//            if( err == KErrNone )
//                {
//                FileManagerDlgUtils::ShowConfirmNoteL(
//                    R_QTN_PASSWORD_CHANGED_TEXT );
//                }
//            else
//                {
//                FileManagerDlgUtils::ShowErrorNoteL(
//                    R_QTN_CRITICAL_ERROR );
//                }
//            }
//        }
//    }
//
//// ------------------------------------------------------------------------------
//// CFileManagerViewBase::CmdRemoveDrivePasswordL
////
//// ------------------------------------------------------------------------------
////
//void CFileManagerViewBase::CmdRemoveDrivePasswordL()
//    {
//    if( !UnlockRemovePasswordL( ETrue ) )
//        {
//        FileManagerDlgUtils::ShowConfirmNoteL( R_QTN_PASSWORD_REMOVED_TEXT );
//        RefreshDriveInfoL();
//        }
//    }
//
//// ------------------------------------------------------------------------------
//// CFileManagerViewBase::CmdMemoryCardDetailsL
////
//// ------------------------------------------------------------------------------
////
//void CFileManagerViewBase::CmdMemoryCardDetailsL()
//    {
//    TFileManagerDriveInfo drvInfo;
//    iEngine.GetDriveInfoL( iEngine.CurrentDrive(), drvInfo );
//    FileManagerDlgUtils::ShowMemoryStoreInfoPopupL( drvInfo );
//    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::UpdatePasswordL
//
// ------------------------------------------------------------------------------
//
TInt CFileManagerViewBase::UpdatePassword(
        TInt aDrive, const TDesC& aOldPwd, const TDesC& aPwd )
    {
    TMediaPassword mPwdNew;
    TMediaPassword mPwdOld;

    ConvertCharsToPwd( aOldPwd, mPwdOld );
    ConvertCharsToPwd( aPwd, mPwdNew );

    return iEngine.SetDrivePassword( aDrive, mPwdOld, mPwdNew );
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::UnlockRemovePasswordL
//
// ------------------------------------------------------------------------------
//
TInt CFileManagerViewBase::UnlockRemovePasswordL(
        TInt aDrive, TBool aRemove )
    {
    TBuf< KFmgrMaxMediaPassword > oldPwd;
    TInt err( KErrNone );
    TMediaPassword pwd;
    TInt res( R_QTN_UNLOCK_PASSWORD_TEXT );
    TInt resWrong( R_QTN_UNLOCK_PWD_WRONG_TEXT );
    HBufC* text = NULL;

    if( aRemove )
        {
        // Confirm the action
        if( !FileManagerDlgUtils::ShowConfirmQueryWithYesNoL(
                R_QTN_PASSWORD_REMOVE_TEXT ) )
            {
            return KErrCancel; // Skip the rest if not accepted
            }
        res = R_QTN_PASSWORD_OLD_TEXT;
        resWrong = R_QTN_PASSWORDS_WRONG_TEXT;
        }
    else
        {
        // Just unlock
#ifdef RD_MULTIPLE_DRIVE
        text = iEngine.GetFormattedDriveNameLC(
            aDrive,
            R_QTN_MEMC_UNLOCK_PASSWORD_MULTIPLE_DEFAULTNAME,
            R_QTN_MEMC_UNLOCK_PASSWORD_MULTIPLE );
#else // RD_MULTIPLE_DRIVE
        text = StringLoader::LoadLC( R_QTN_UNLOCK_PASSWORD_TEXT );
#endif // RD_MULTIPLE_DRIVE
        }

    // Show until correct pwd is given or canceled
    TBool isDone( EFalse );
    while( !isDone )
        {
        // Empty first
        EmptyPwd( oldPwd );
        TBool pwdGiven( EFalse );
        if ( text )
            {
            pwdGiven = FileManagerDlgUtils::ShowSimplePasswordQueryL( *text, oldPwd );
            }
        else
            {
            pwdGiven = FileManagerDlgUtils::ShowSimplePasswordQueryL( res, oldPwd );
            }
        
        if( pwdGiven )
            {
            ConvertCharsToPwd( oldPwd, pwd );
            if( aRemove )
                {
                err = iEngine.RemoveDrivePassword(  aDrive, pwd );
                }
            else
                {
                err = iEngine.UnlockDrive( aDrive, pwd );
                }

            if ( err == KErrNone )
                {
                isDone = ETrue;
                }
            else if ( err == KErrAccessDenied )
                {
                FileManagerDlgUtils::ShowErrorNoteL( resWrong );
                }
            else if ( err == KErrNotReady )
                {
                isDone = ETrue;
                FileManagerDlgUtils::ShowErrorNoteL( R_QTN_MEMC_NOT_AVAILABLE );
                }
            else
                {
                isDone = ETrue;
                FileManagerDlgUtils::ShowErrorNoteL( R_QTN_CRITICAL_ERROR );
                }
            }
        else
            {
            err = KErrCancel;
            isDone = ETrue;
            }
        }

    if ( text )
        {
        CleanupStack::PopAndDestroy( text );
        }
    return err;
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::SetRemoteDriveConnectionStateL
//
// ------------------------------------------------------------------------------
//
void CFileManagerViewBase::SetRemoteDriveConnectionStateL( TBool aState )
    {
    TInt drv( 0 );

    StoreIndex();
    if ( !iEngine.CurrentDirectory().Length() )
        {
        TInt index( iContainer->ListBoxCurrentItemIndex() );
        CFileManagerItemProperties* prop = iEngine.GetItemInfoL( index );
        CleanupStack::PushL( prop );
        drv = TDriveUnit( prop->FullPath() );
        CleanupStack::PopAndDestroy( prop );
        }
    else
        {
        TFileManagerDriveInfo& drvInfo( DriveInfo() );
        drv = drvInfo.iDrive;
        }
    iEngine.SetRemoteDriveConnection( drv, aState );
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::OpenRemoteDriveSettingsL
//
// ------------------------------------------------------------------------------
//
void CFileManagerViewBase::OpenRemoteDriveSettingsL(
        const TDesC& aDriveName )
    {
    CFileManagerAppUi* appUi = static_cast< CFileManagerAppUi* >( AppUi() );
    appUi->ActivateRemoteDriveSettingsViewL( aDriveName );
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::IsDisconnectedRemoteDrive
//
// ------------------------------------------------------------------------------
//
TBool CFileManagerViewBase::IsDisconnectedRemoteDrive(
        CFileManagerItemProperties& aProp )
    {
    TUint32 drvState( 0 );
    if ( iEngine.DriveState( drvState, aProp.FullPath() ) == KErrNone )
        {
        if ( ( drvState & TFileManagerDriveInfo::EDriveRemote ) &&
            !( drvState & TFileManagerDriveInfo::EDriveConnected ) )
            {
            return ETrue;
            }
        }
    return EFalse;
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::RemoteDriveCommonFilteringL
//
// ------------------------------------------------------------------------------
//
void CFileManagerViewBase::RemoteDriveCommonFilteringL( CEikMenuPane& aMenuPane )
    {
    TBool dimAll( EFalse );
    if ( !FeatureManager().IsRemoteStorageFwSupported() )
        {
        dimAll = ETrue;
        }
    else
        {
//        CEikListBox& listBox = iContainer->ListBox();
        if ( iContainer->ListBoxNumberOfItems() )
            {    
            TInt index( iContainer->ListBoxCurrentItemIndex() );
            CFileManagerItemProperties* prop = iEngine.GetItemInfoL( index );

            TUint32 drvState( 0 );
            TInt err( iEngine.DriveState( drvState, prop->FullPath() ) );
            if ( err == KErrNone &&
                ( drvState & TFileManagerDriveInfo::EDriveRemote ) )
                {
                if ( drvState & TFileManagerDriveInfo::EDriveConnected )
                    {
                    aMenuPane.SetItemDimmed( EFileManagerConnectRemoveDrive, ETrue );
                    }
                else
                    {
                    aMenuPane.SetItemDimmed( EFileManagerDisconnectRemoveDrive, ETrue );
                    }
                }
            else
                {
                dimAll = ETrue;
                }
            delete prop;
            }
        else
            {
            // List is empty
            TFileManagerDriveInfo& drvInfo( DriveInfo() );
            
            if ( drvInfo.iState & TFileManagerDriveInfo::EDriveRemote )
                {
                if ( drvInfo.iState & TFileManagerDriveInfo::EDriveConnected )
                    {
                    aMenuPane.SetItemDimmed( EFileManagerConnectRemoveDrive, ETrue );
                    }
                else
                    {
                    aMenuPane.SetItemDimmed( EFileManagerDisconnectRemoveDrive, ETrue );
                    }
                }
            else
                {
                dimAll = ETrue;
                }
            }
        }
        
    if ( dimAll )
        {
        aMenuPane.SetItemDimmed( EFileManagerConnectRemoveDrive, ETrue );
        aMenuPane.SetItemDimmed( EFileManagerDisconnectRemoveDrive, ETrue );
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::LaunchProgressBarL  
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerViewBase::LaunchProgressBarL(
        TInt aDialogId,
        TInt aTextId,
        TInt64 aFinalValue,
        TInt64 aInitialValue,
        TBool aPeriodic,
        TBool aImmediatelyVisible )
    {
    ClearProgressBarL(); // Clear previous

    if ( aFinalValue )
        {
        iProgressDialog = new (ELeave) CAknProgressDialog(
            ( reinterpret_cast< CEikDialog** >( &iProgressDialog ) ), aImmediatelyVisible );
        iProgressDialog->PrepareLC( aDialogId );
        
        if ( aPeriodic )
            {
            iPeriodic = CPeriodic::NewL( CActive::EPriorityStandard );
            iPeriodic->Start(
                KProgressBarUpdateInterval, KProgressBarUpdateInterval,
                TCallBack( UpdateProgressBar, this ) );
            }
        }
    else
        {
        iProgressDialog = new (ELeave) CAknWaitDialog(
            ( reinterpret_cast< CEikDialog** >( &iProgressDialog ) ), aImmediatelyVisible );
        iProgressDialog->PrepareLC( aDialogId );
        }

    if ( aTextId )
        {
        HBufC* text = StringLoader::LoadLC( aTextId );
        iProgressDialog->SetTextL( *text );
        CleanupStack::PopAndDestroy( text );
        }

    iProgressDialog->SetCallback(this);
    iProgressInfo = iProgressDialog->GetProgressInfoL();
    if ( iProgressInfo )
        {
        iProgressInfo->SetFinalValue( static_cast<TInt>( aFinalValue / KMinificationFactor ) ); 
        iProgressInfo->SetAndDraw( static_cast<TInt>( aInitialValue / KMinificationFactor ) );
        }
    iProgressDialog->RunLD();
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::RefreshProgressDelayedStart
//
// ------------------------------------------------------------------------------
//
TInt CFileManagerViewBase::RefreshProgressDelayedStart( TAny* aPtr )
    {
    CFileManagerViewBase* view = static_cast< CFileManagerViewBase* > ( aPtr );
    TRAP_IGNORE( view->RefreshProgressDelayedStartL() );
    return KErrNone;
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::RefreshProgressDelayedStartL
//
// ------------------------------------------------------------------------------
//
void CFileManagerViewBase::RefreshProgressDelayedStartL()
    {
    CFileManagerAppUi* app = static_cast< CFileManagerAppUi* >( AppUi() );

    delete iRefreshProgressDelayedStart;
    iRefreshProgressDelayedStart = NULL;

    if( iProgressDialogRefresh )
        {
        iProgressDialogRefresh->ProcessFinishedL();
        iProgressDialogRefresh = NULL;
        }
    iProgressDialogRefresh = new( ELeave ) CAknProgressDialog(
        reinterpret_cast< CEikDialog** >( &iProgressDialogRefresh ),
        ETrue );
    iProgressDialogRefresh->SetCallback( this );

    if ( Id() == CFileManagerAppUi::KFileManagerSearchResultsViewId )
        {
        iProgressDialogRefresh->ExecuteLD( R_FILEMANAGER_FIND_WAIT_DIALOG );
        }
    else
        {
        iProgressDialogRefresh->ExecuteLD( R_FILEMANAGER_WAIT_NOTE_OPEN );
        }
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::IsRefreshInProgress
//
// ------------------------------------------------------------------------------
//
TBool CFileManagerViewBase::IsRefreshInProgress()
    {
    if ( iRefreshProgressDelayedStart || iProgressDialogRefresh )
        {
        return ETrue;
        }
    return EFalse;
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::ProcessCommandL
//
// ------------------------------------------------------------------------------
//
void CFileManagerViewBase::ProcessCommandL( TInt aCommand )
    {
    // Suppress commands during refresh
    if ( IsRefreshInProgress() )
        {
        switch ( aCommand )
            {
            case EAknSoftkeyOptions: // FALLTHROUGH
            case EAknSoftkeyBack:  // FALLTHROUGH
            case EAknSoftkeyContextOptions:  // FALLTHROUGH
            case EAknSoftkeyMark:  // FALLTHROUGH
            case EAknSoftkeyUnmark:  // FALLTHROUGH
            case EAknSoftkeySelect:
                {
                return;
                }
            default:
                {
                break;
                }
            }
        }

    // Handle commands directly
    switch ( aCommand )
        {
        case EAknSoftkeyContextOptions: // FALLTHROUGH
        case EAknSoftkeyMark:
            {
            HandleCommandL( aCommand );
            break;
            }
        default:
            {
            CAknView::ProcessCommandL( aCommand );
            break;
            }
        }
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::AskPathL
//
// ------------------------------------------------------------------------------
//
TBool CFileManagerViewBase::AskPathL( TDes& aPath, TInt aTextId )
    {
    TBool ret( EFalse );
    TInt memType(
        AknCommonDialogsDynMem::EMemoryTypePhone |
        AknCommonDialogsDynMem::EMemoryTypeMMC );

    if ( FeatureManager().IsRemoteStorageFwSupported() )
        {
        memType |= AknCommonDialogsDynMem::EMemoryTypeRemote;
        }

    HBufC* title = StringLoader::LoadLC( aTextId );
    CFileManagerFileSelectionFilter* filter =
        new( ELeave ) CFileManagerFileSelectionFilter( iEngine );
    CleanupStack::PushL( filter );

    ret = AknCommonDialogsDynMem::RunFolderSelectDlgLD(
        memType,
        aPath,
        KNullDesC,
        R_FILEMANAGER_FIND_MEMORY_SELECTIONDIALOG,
        R_FILEMANAGER_FIND_FOLDER_SELECTIONDIALOG,
        *title,
        filter );

    CleanupStack::PopAndDestroy( filter );
    CleanupStack::PopAndDestroy( title );
    return ret;
    }

#ifdef RD_FILE_MANAGER_BACKUP
// ------------------------------------------------------------------------------
// CFileManagerViewBase::StartSchBackupL
//
// ------------------------------------------------------------------------------
//
void CFileManagerViewBase::StartSchBackupL()
    {
    FUNC_LOG

    CFileManagerAppUi* appUi = static_cast< CFileManagerAppUi* >( AppUi() );
    CFileManagerSchBackupHandler& handler( appUi->SchBackupHandlerL() );

    if ( FeatureManager().IsFeatureSupported(
            EFileManagerFeatureScheduledBackupDisabled ) )
        {
        // Scheduled backup is disabled, disable scheduler and cancel backup
        INFO_LOG( "CFileManagerViewBase::StartSchBackupL-Backup disabled" )

        handler.CancelBackupStarter();
        CFileManagerTaskScheduler& scheduler( appUi->TaskSchedulerL() );
        scheduler.EnableBackupScheduleL( EFalse );
        appUi->SchBackupFinishedL( KErrCancel );
        return;
        }

    // Start scheduled backup if no process in progress
    // Otherwise wait process to finish
    if ( iActiveProcess == ENoProcess )
        {
        CFileManagerBackupSettings& settings( iEngine.BackupSettingsL() );
        TTime schTime( SetCurrentYearMonthAndDay( settings.Time() ) );
        
        TTime manualBackupOrRestoreStarted = appUi->BackupOrRestoreStartTime();
        TTime manualBackupOrRestoreEnded = appUi->BackupOrRestoreEndTime();
        
        if ( manualBackupOrRestoreStarted.Int64() > 0 &&
             schTime >= manualBackupOrRestoreStarted &&
             schTime <= manualBackupOrRestoreEnded )
            {
            INFO_LOG( "CFileManagerViewBase::StartSchBackupL-Backup canceled due to manual op" )

            handler.CancelBackupStarter();
            
            appUi->ResetBackupOrRestoreEndTime();// Cancel required only once
            }
        else
            {
            INFO_LOG( "CFileManagerViewBase::StartSchBackupL-Start backup" )

            iSchBackupPending = EFalse;
            iActiveProcess = ESchBackupProcess;
            iEngine.SetObserver( this );
            handler.StartBackupWithConfirm();
            }
        }
    // Ignore scheduled backup if backup or restore is in progress
    else if ( iActiveProcess == ESchBackupProcess ||
             iActiveProcess == EBackupProcess ||
             iActiveProcess == ERestoreProcess )
        {
        INFO_LOG( "CFileManagerViewBase::StartSchBackupL-Backup canceled" )

        handler.CancelBackupStarter();
        }
    else
        {
        INFO_LOG( "CFileManagerViewBase::StartSchBackupL-Backup pending" )

        iSchBackupPending = ETrue;
        }
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::SchBackupFinishedL
//
// ------------------------------------------------------------------------------
//
void CFileManagerViewBase::SchBackupFinishedL()
    {
    FUNC_LOG

    if ( iActiveProcess == ESchBackupProcess )
        {
        iActiveProcess = ENoProcess;
        iSchBackupPending = EFalse;
        }
    }

#endif // RD_FILE_MANAGER_BACKUP

// ------------------------------------------------------------------------------
// CFileManagerViewBase::SetCbaMskTextL
//
// ------------------------------------------------------------------------------
//
void CFileManagerViewBase::SetCbaMskTextL( const TInt aTextId )
    {
    HBufC* text = StringLoader::LoadLC( aTextId );
    CEikButtonGroupContainer* cba = Cba();
    if ( cba->ButtonCount() == KFmgrMSK )
        {
        TInt cmdId( cba->ButtonGroup()->CommandId( KFmgrMSK ) );
        cba->SetCommandL( KFmgrMSK, cmdId, *text );
        cba->DrawDeferred();
        }
    CleanupStack::PopAndDestroy( text );
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::UpdateCbaL
//
// ------------------------------------------------------------------------------
//
void CFileManagerViewBase::UpdateCbaL()
    {
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::UpdateCommonCbaL
//
// ------------------------------------------------------------------------------
//
void CFileManagerViewBase::UpdateCommonCbaL()
    {
    if ( !iContainer || IsRefreshInProgress() )
        {
        return;
        }

    CEikButtonGroupContainer* cba = Cba();

    if ( !iContainer->ListBoxNumberOfItems() )
        {
        cba->SetCommandSetL(
            R_FILEMANAGER_SOFTKEYS_OPTIONS_BACK__EMPTY );
        }
    else if ( iContainer->ListBoxSelectionIndexesCount() )
        {
        cba->SetCommandSetL(
            R_FILEMANAGER_SOFTKEYS_CONTEXT_OPTIONS_BACK__OPTIONS );
        }
    else
        {
        cba->SetCommandSetL(
            R_FILEMANAGER_SOFTKEYS_OPTIONS_BACK__OPEN );
        }

    // Restore right cancel softkey if it has been set by search field
    TBool restoreCancel( ETrue );
    if ( iContainer->IsSearchFieldVisible() &&
         cba->ButtonCount() >= CEikButtonGroupContainer::ERightSoftkeyPosition )
        {
        restoreCancel = ( cba->ButtonGroup()->CommandId(
            CEikButtonGroupContainer::ERightSoftkeyPosition ) == EAknSoftkeyCancel );
        }
    if ( !restoreCancel )
        {
        HBufC* cancelText = StringLoader::LoadLC( R_AVKON_SOFTKEY_CANCEL );
        cba->SetCommandL(
            CEikButtonGroupContainer::ERightSoftkeyPosition,
            EAknSoftkeyCancel,
            *cancelText );
        CleanupStack::PopAndDestroy( cancelText );
        }

    cba->DrawDeferred();
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::RenameDriveL
//
// ------------------------------------------------------------------------------
//
void CFileManagerViewBase::RenameDriveL( TBool aForceDefaultName )
    {
    TFileManagerDriveInfo drvInfo;
    if ( DriveInfoAtCurrentPosL( drvInfo ) < 0 )
        {
        return; // No drive selected
        }

    if ( drvInfo.iState & TFileManagerDriveInfo::EDriveMassStorage )
        {
        return; // Name not allowed
        }

    HBufC* drvName = HBufC::NewLC( KMaxVolumeName );
    TPtr name( drvName->Des() );
    // 16-bit chars are required for non western volume names
    const TInt KMaxNonWesternVolumeName( KMaxVolumeName / 2 );

    // Setup query according to variant type, western or non western
    TInt resId( R_FILEMANAGER_DRIVE_NAME_QUERY );
    TInt maxLen( KMaxVolumeName );
    if ( !FeatureManager().IsWesternVariant() )
        {
        resId = R_FILEMANAGER_DRIVE_NAME_QUERY_NON_WESTERN;
        maxLen = KMaxNonWesternVolumeName;
        }

    if ( aForceDefaultName || !drvInfo.iName.Length() )
        {
        HBufC* defaultName = NULL;
        if ( drvInfo.iState & TFileManagerDriveInfo::EDriveUsbMemory )
            {
            defaultName = StringLoader::LoadLC(
                R_QTN_FMGR_USB_MEMORY_DEFAULT_NAME );
            }
        else
            {
            defaultName = StringLoader::LoadLC( R_QTN_MMC_DEFAULT_NAME );
            }
        if ( defaultName->Length() > maxLen )
            {
            name.Copy( defaultName->Des().Left( maxLen ) );
            }
        else
            {
            name.Copy( *defaultName );
            }
        CleanupStack::PopAndDestroy( defaultName );
        }
    else
        {
        if ( drvInfo.iName.Length() > maxLen )
            {
            name.Copy( drvInfo.iName.Left( maxLen ) );
            }
        else
            {
            name.Copy( drvInfo.iName );
            }
        }

    // Loop until canceled, accepted or an error occurs
    TBool isDone( EFalse );
    while( !isDone )
        {
        CAknTextQueryDialog* renameDlg =
            CAknTextQueryDialog::NewL( name, CAknQueryDialog::ENoTone );
        renameDlg->SetMaxLength( maxLen );
        TBool ret( EFalse );
        if ( renameDlg->ExecuteLD( resId ) )
            {
            ret = ETrue;
            }
        if( ret && name.Compare( drvInfo.iName ) )
            {
            TInt err( iEngine.RenameDrive( drvInfo.iDrive, name ) );
            if( err == KErrNone )
                {
                FileManagerDlgUtils::ShowConfirmNoteL(
                    R_QTN_FMGR_CONFIRM_MEMORY_NAME_CHANGED );
                RefreshDriveInfoL();
                isDone = ETrue;
                }
            else if( err == KErrBadName )
                {
                FileManagerDlgUtils::ShowInfoNoteL(
                    R_QTN_INVALID_DRIVE_NAME );
                }
            else
                {
                FileManagerDlgUtils::ShowErrorNoteL(
                    R_QTN_CRITICAL_ERROR );
                isDone = ETrue;
                }
            }
        else
            {
            // Canceled
            isDone = ETrue;
            }
        }
    CleanupStack::PopAndDestroy( drvName );
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::CmdRefreshDirectoryL
//
// ------------------------------------------------------------------------------
//
void CFileManagerViewBase::CmdRefreshDirectoryL()
    {
    StoreIndex();
    iEngine.SetObserver( this );
    iEngine.ForcedRefreshDirectory();
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::ShowEjectQueryL
//
// ------------------------------------------------------------------------------
//
void CFileManagerViewBase::ShowEjectQueryL()
    {
    delete iEjectQueryDialog;
    iEjectQueryDialog = NULL;

    iEjectDone = EFalse;
    iEjectQueryDialog = CAknQueryDialog::NewL();
    HBufC* text = NULL;
    TInt index( iContainer->ListBoxCurrentItemIndex() );
    CFileManagerItemProperties* prop = iEngine.GetItemInfoLC( index );
#ifdef RD_MULTIPLE_DRIVE
    text = iEngine.GetFormattedDriveNameLC(
        prop->DriveId(),
        R_QTN_MEMC_INFO_EJECT_MULTIPLE_DEFAULTNAME,
        R_QTN_MEMC_INFO_EJECT_MULTIPLE );
#else // RD_MULTIPLE_DRIVE
    text = StringLoader::LoadLC( R_QTN_INFO_EJECT );
#endif // RD_MULTIPLE_DRIVE
    TRAP_IGNORE( iEjectQueryDialog->ExecuteLD(
        R_FILEMANAGER_EJECT_CONFIRM_QUERY, *text ) );
    CleanupStack::PopAndDestroy( text );
    CleanupStack::PopAndDestroy( prop );
    iEjectQueryDialog = NULL;
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::IsDriveAvailable
//
// ------------------------------------------------------------------------------
//
TBool CFileManagerViewBase::IsDriveAvailable( const TDesC& aPath ) const
    {
    TBool ret( EFalse );
    if ( aPath.Length() )
        {
        TInt drive = TDriveUnit( aPath );
        ret = IsDriveAvailable( drive );
        }
    return ret;
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::IsDriveAvailable
//
// ------------------------------------------------------------------------------
//
TBool CFileManagerViewBase::IsDriveAvailable( const TInt aDrive ) const
    {
    TUint32 drvState( 0 );
    if ( iEngine.DriveState( drvState, aDrive ) != KErrNone )
        {
        return EFalse;
        }
    if ( drvState & ( TFileManagerDriveInfo::EDriveLocked |
                      TFileManagerDriveInfo::EDriveCorrupted |
                      TFileManagerDriveInfo::EDriveInUse ) )
        {
        return EFalse; // Drive is unavailable
        }
    if ( !( drvState & TFileManagerDriveInfo::EDriveRemote ) &&
        !( drvState & TFileManagerDriveInfo::EDrivePresent ) )
        {
        return EFalse; // Drive is not present
        }
    return ETrue;
    }

// ----------------------------------------------------------------------------
// CFileManagerViewBase::CheckPhoneState
//
// ----------------------------------------------------------------------------
//
TBool CFileManagerViewBase::CheckPhoneState() const
    {
    // Check here all operations, which are supposed
    // to prevent manual backup or restore.
    TBool err( ETrue );
    TInt syncErr( 0 );
    TInt syncStat( 0 );
    TInt burErr( 0 );
    TInt burStat( 0 );
    
    // Check synchronization state
    syncErr = RProperty::Get(
        KPSUidDataSynchronizationInternalKeys, 
        KDataSyncStatus, syncStat );
        
    // Check backup/restore (e.g. PC Suite initiated) state
    burErr = RProperty::Get(
        KUidSystemCategory, 
        KUidBackupRestoreKey, burStat );
    const TBURPartType partType = static_cast< TBURPartType >
        ( burStat & KBURPartTypeMask );
    
    if ( (syncErr == KErrNone && syncStat > 0) 
        || (burErr == KErrNone && partType != EBURUnset && partType != EBURNormal ) )
        {
        err = EFalse;
        }
    
    return err;
    }

// ----------------------------------------------------------------------------
// CFileManagerViewBase::StopProgressDialogAndStoreValues
//
// ----------------------------------------------------------------------------
//
TBool CFileManagerViewBase::StopProgressDialogAndStoreValues()
    {
    TBool ret( EFalse );
    if ( iProgressDialog && iProgressInfo )
        {
        CEikProgressInfo::SInfo info( iProgressInfo->Info() );
        iProgressFinalValue = ( info.iFinalValue ) * KMinificationFactor;
        iProgressCurrentValue = ( iProgressInfo->CurrentValue() ) * KMinificationFactor;
        if ( !iProgressCurrentValue && iTotalTransferredBytes <= iProgressFinalValue )
            {
            iProgressCurrentValue = iTotalTransferredBytes;
            }
        TRAP_IGNORE( iProgressDialog->ProcessFinishedL() );
        iProgressDialog = NULL;
        iProgressInfo = NULL;
        ret = ETrue;
        }

    delete iRefreshProgressDelayedStart;
    iRefreshProgressDelayedStart = NULL;
    delete iPeriodic;
    iPeriodic = NULL;

    return ret;
    }


// ----------------------------------------------------------------------------
// CFileManagerViewBase::CheckPostponedDirectoryRefresh
//
// ----------------------------------------------------------------------------
//
void CFileManagerViewBase::CheckPostponedDirectoryRefresh()
    {
    if ( iDirectoryRefreshPostponed )
        {
        // Delete was canceled but directory was changed during query
        StoreIndex();
        iEngine.SetObserver( this );
        iEngine.RefreshDirectory();
        }
    iDirectoryRefreshPostponed = EFalse;
    }

// ----------------------------------------------------------------------------
// CFileManagerViewBase::DenyDirectoryRefresh
//
// ----------------------------------------------------------------------------
//
void CFileManagerViewBase::DenyDirectoryRefresh( TBool aDeny )
    {
    iDirectoryRefreshDenied = aDeny;
    if ( aDeny )
        {
        iDirectoryRefreshPostponed = EFalse; // Reset previous value
        }
    }

// ----------------------------------------------------------------------------
// CFileManagerViewBase::SortMenuFilteringL
//
// ----------------------------------------------------------------------------
//
void CFileManagerViewBase::SortMenuFilteringL( CEikMenuPane& aMenuPane )
    {
    TInt selected( EFileManagerSortByName );
    switch ( iEngine.SortMethod() )
        {
        case CFileManagerEngine::EByMatch:
            {
            if ( iEngine.State() == CFileManagerEngine::ESearch )
                {
                selected = EFileManagerSortByMatch;
                }
            break;
            }
        case CFileManagerEngine::EByName:
            {
            selected = EFileManagerSortByName;
            break;
            }
        case CFileManagerEngine::EByType:
            {
            selected = EFileManagerSortByType;
            break;
            }
        case CFileManagerEngine::EMostRecentFirst:
            {
            selected = EFileManagerSortMostRecentFirst;
            break;
            }
        case CFileManagerEngine::ELargestFirst:
            {
            selected = EFileManagerSortLargestFirst;
            break;
            }
        default:
            {
            break;
            }
        }
    TInt position = 0;
    if ( aMenuPane.MenuItemExists( selected, position ) )
    	{
    aMenuPane.SetItemButtonState( selected, EEikMenuItemSymbolOn );
    	}
    }

// ----------------------------------------------------------------------------
// CFileManagerViewBase::CmdSortL
//
// ----------------------------------------------------------------------------
//
void CFileManagerViewBase::CmdSortL( TInt aCommand )
    {
    CFileManagerEngine::TSortMethod sortMethod( CFileManagerEngine::EByName );
    switch ( aCommand )
        {
	    case EFileManagerSortByName:
	        {
	        sortMethod = CFileManagerEngine::EByName;
	        break;
	        }
	    case EFileManagerSortByType:
	        {
	        sortMethod = CFileManagerEngine::EByType;
	        break;
	        }
	    case EFileManagerSortMostRecentFirst:
	        {
	        sortMethod = CFileManagerEngine::EMostRecentFirst;
	        break;
	        }
	    case EFileManagerSortLargestFirst:
	        {
	        sortMethod = CFileManagerEngine::ELargestFirst;
	        break;
	        }
	    case EFileManagerSortByMatch:
	        {
	        sortMethod = CFileManagerEngine::EByMatch;
	        break;
	        }
	    default:
	        {
	        return;
	        }
        }
    if ( iEngine.SortMethod() != sortMethod )
        {
        iIndex = 0;
        if ( iContainer )
            {
            iContainer->SetCurrentItemIndexAfterSearch( 0 );
            }
        iEngine.SetCurrentIndex( 0 );
        iEngine.SetSortMethod( sortMethod );
        iEngine.RefreshSort();
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::FeatureManager
// 
// -----------------------------------------------------------------------------
// 
CFileManagerFeatureManager& CFileManagerViewBase::FeatureManager() const
    {
    return iEngine.FeatureManager();
    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::NotifyForegroundStatusChange
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerViewBase::NotifyForegroundStatusChange( TBool /*aForeground*/ )
    {
    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::ShowDiskSpaceErrorL
// 
// -----------------------------------------------------------------------------
// 
#ifdef RD_MULTIPLE_DRIVE

void CFileManagerViewBase::ShowDiskSpaceErrorL( const TDesC& aFolder )
    {
    TInt defaultNameResId( 0 );
    TInt namedResId( 0 );
    TInt drv( KErrNotFound );
    if ( aFolder.Length() )
        {
        TFileManagerDriveInfo drvInfo;
        drv = TDriveUnit( aFolder );
        iEngine.GetDriveInfoL( drv, drvInfo );
        if ( drvInfo.iState & TFileManagerDriveInfo::EDriveMassStorage )
            {
            defaultNameResId = R_QTN_MEMLO_NOT_ENOUGH_MASS_MEMORY;
            }
        else if ( drvInfo.iState & TFileManagerDriveInfo::EDriveRemovable )
            {
            defaultNameResId = R_QTN_MEMLO_NOT_ENOUGH_MEMORY_CARD_DEFAULTNAME;
            namedResId = R_QTN_MEMLO_NOT_ENOUGH_MEMORY_CARD_NAME;
            }
        else if ( !( drvInfo.iState & TFileManagerDriveInfo::EDriveRemote ) )
            {
            defaultNameResId = R_QTN_MEMLO_NOT_ENOUGH_DEVICE_MEMORY;
            }
        }
    if ( defaultNameResId )
        {
        HBufC* text = iEngine.GetFormattedDriveNameLC(
            drv, defaultNameResId, namedResId );
        FileManagerDlgUtils::ShowConfirmQueryWithOkL(
            FileManagerDlgUtils::EErrorIcons, *text );
        CleanupStack::PopAndDestroy( text );
        }
    else
        {
        Error( KErrDiskFull ); // Show general error
        }
    }

#else // RD_MULTIPLE_DRIVE

void CFileManagerViewBase::ShowDiskSpaceErrorL( const TDesC& /*aFolder*/ )
    {
    Error( KErrDiskFull ); // Show general error
    }

#endif // RD_MULTIPLE_DRIVE

// -----------------------------------------------------------------------------
// CFileManagerViewBase::DoLaunchProgressDialogAsync
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerViewBase::DoLaunchProgressDialogAsync()
    {
     // Store the bytes value to be restored after new launch
    TInt64 prevBytes = iTotalTransferredBytes;

    // Ensure that current progress value is up to date
    if ( iTotalTransferredBytes > iProgressCurrentValue &&
         iTotalTransferredBytes <= iProgressFinalValue )
        {
        iProgressCurrentValue = iTotalTransferredBytes;
        }

    TRAP_IGNORE( LaunchProgressDialogL(
        iProgressFinalValue, iProgressCurrentValue, iActiveProcess, ETrue ) );

    iTotalTransferredBytes = prevBytes;
    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::LaunchProgressDialogAsync
// 
// -----------------------------------------------------------------------------
// 
TInt CFileManagerViewBase::LaunchProgressDialogAsync( TAny* aPtr )
    {
    static_cast< CFileManagerViewBase* >( aPtr )->DoLaunchProgressDialogAsync();
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::FilesCountInSearchField()
// 
// -----------------------------------------------------------------------------
//
TInt CFileManagerViewBase::FilesCountInSearchField()
    {
    TInt count = 0;
    
    if( iContainer->IsSearchFieldVisible() )
        {
        TInt listBoxIndex =0;
        for(TInt i(0); i<iContainer->ListBoxNumberOfItems();i++ )
            {
            listBoxIndex = iContainer->SearchFieldToListBoxIndex( i );
            if( !iEngine.IsFolder( listBoxIndex ) )
                {
                count++;
                }
            }
        }
    return count;
    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::HandleServerAppExit
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerViewBase::HandleServerAppExit( TInt /*aReason*/ )
    {
    iEngine.SetAppExitOb( NULL );
    iEngine.SetObserver( this );
    iEngine.RefreshDirectory();
    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::EmptyPwd
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerViewBase::EmptyPwd( TDes& aPwd )
    {
    aPwd.FillZ( aPwd.MaxLength( ) );
    aPwd.Zero();
    }

// -----------------------------------------------------------------------------
// CFileManagerViewBase::ConvertCharsToPwd
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerViewBase::ConvertCharsToPwd(
        const TDesC& aWord, TDes8& aConverted )
    {
    // Make sure the target password is empty ( can't use the function here )
    aConverted.FillZ( aConverted.MaxLength() );
    aConverted.Zero();
    TInt size( aWord.Size() );
    if ( size )
        {
        if ( size > aConverted.MaxLength() )
            {
            size = aConverted.MaxLength();
            }
        aConverted.Copy( (TUint8*)aWord.Ptr(), size );
        }
    }

// ------------------------------------------------------------------------------
// CFileManagerViewBase::DriveInfoAtCurrentPosL
//
// ------------------------------------------------------------------------------
//
TInt CFileManagerViewBase::DriveInfoAtCurrentPosL(
        TFileManagerDriveInfo& aInfo )
    {
    TUid viewId( Id() );
    if ( viewId == CFileManagerAppUi::KFileManagerMemoryStoreViewId ||
         viewId == CFileManagerAppUi::KFileManagerFoldersViewId )
        {
        INFO_LOG1("CFileManagerViewBase::DriveInfoAtCurrentPosL viewId=%D", viewId.iUid)
        // Use cached info
        aInfo = DriveInfo();
        return aInfo.iDrive;
        }
    
    // Fetch info
    if ( !iContainer )
        {
        return KErrNotFound;
        }
    if ( !iContainer->ListBoxNumberOfItems() )
        {
        return KErrNotFound;
        }
    CFileManagerItemProperties* prop = iEngine.GetItemInfoLC(
        iContainer->ListBoxCurrentItemIndex() );
    TInt ret( KErrNotFound );
    TPtrC fullPath( prop->FullPath() );
    if ( fullPath.Length() )
        {
        ret = TDriveUnit( fullPath );
        iEngine.GetDriveInfoL( ret, aInfo );
        }
    CleanupStack::PopAndDestroy( prop );
    return ret;
    }

//  End of File  
