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
* Description:  Main class of the filemanagerengine
*
*/



// INCLUDE FILES
#include <StringLoader.h>
#include <barsread.h>
#include <sysutil.h>
#include <bautils.h>
//#include <cmgxfilemanager.h>
//#include <mgxfilemanagerfactory.h>
//#include <cmgalbummanager.h>
#include <pathinfo.h>
#include <AknServerApp.h>
#include <FileManagerEngine.rsg>
#ifdef RD_MULTIPLE_DRIVE
#include <driveinfo.h>
#endif // RD_MULTIPLE_DRIVE
#include "CFileManagerEngine.h"
#include "CGflmNavigatorModel.h"
#include "CGflmGroupItem.h"
#include "CGflmFileSystemItem.h"
#include "CGflmDriveItem.h"
#include "CGflmGlobalActionItem.h"
#include "MGflmItemGroup.h"
#include "Cfilemanagerfolderarray.h"
#include "CFilemanagerMimeIconArray.h"
#include "CFileManagerActiveExecute.h"
#include "CFileManagerRefresher.h"
#include "Cfilemanagerfilesystemevent.h"
#include "Cfilemanageractivesize.h"
#include "Cfilemanageractivedelete.h"
#include "FileManagerEngine.hrh"
#include "CFileManagerCommonDefinitions.h"
#include "CFileManagerUtils.h"
#include "CFileManagerItemFilter.h"
#include "CFileManagerDocHandler.h"
#include "FileManagerDebug.h"
#include "CFileManagerItemProperties.h"
#include "CFileManagerRemovableDriveHandler.h"
#include "CFileManagerRemoteDriveHandler.h"
#include "CFileManagerBackupSettings.h"
#include "CFileManagerRestoreSettings.h"
#include "CFileManagerActiveRename.h"
#include "CFileManagerFeatureManager.h"

// CONSTANTS
const TUint16 KIllegalChars[] = {
    '<', '>', '"', '/', '\\', '|', ':', '*', '?',
    0xFF02, // Full width quote
    0xFF0A, // Full width asterisk
    0xFF0F, // Full width slash
    0xFF1A, // Full width colon
    0xFF1C, // Full width left arrow
    0xFF1E, // Full width right arrow
    0xFF1F, // Full width question mark
    0xFF3C, // Full width back slash
    0xFF5C, // Full width pipe
    0x201C, // Left quote
    0x201D, // Right quote
    0x201F, // Reversed quote
    0x21B2, // Downwards arrow with tip leftwards
    0, // Array terminator
};
const TUint KDiskEventCheckInterval = 100000; // microseconds



// ============================ LOCAL FUNCTIONS ================================
// -----------------------------------------------------------------------------
// SearchMGAlbumIdL
// -----------------------------------------------------------------------------
//
/*static TInt SearchMGAlbumIdL(
        CMGAlbumManager& aAlbumManager, const TDesC& aFilename )
    {
    CDesCArray* fileArray =
        new ( ELeave ) CDesCArraySeg( KMGFileArrayGranularity );
    CleanupStack::PushL( fileArray );
    TInt albumCount( aAlbumManager.AlbumCount() );
    TInt ret( KErrNotFound );
    for( TInt i( 0 ); i < albumCount; ++i )
        {
        CMGAlbumInfo* albumInfo = aAlbumManager.AlbumInfoLC( i );
        TInt albumId( albumInfo->Id() );
        CleanupStack::PopAndDestroy( albumInfo );
        fileArray->Reset();
        aAlbumManager.GetAlbumFileArrayL( albumId, *fileArray );
        TInt pos( 0 );
        if( !fileArray->Find( aFilename, pos ) )
            {
            ret = albumId;
            break;
            }
        }
    CleanupStack::PopAndDestroy( fileArray );
    return ret;
    }
*/

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CFileManagerEngine::CFileManagerEngine
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CFileManagerEngine::CFileManagerEngine( RFs& aFs ) :
		iFs( aFs ), iObserver( NULL ), iSisFile( EFalse ),iDelayedDiskEventNotify( NULL )
    {
    FUNC_LOG
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CFileManagerEngine* CFileManagerEngine::NewL( RFs& aFs )
    {
    CFileManagerEngine* self = new ( ELeave ) CFileManagerEngine( aFs );

    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    return self;
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::ConstructL
// -----------------------------------------------------------------------------
//
void CFileManagerEngine::ConstructL()
    {
    FUNC_LOG

    INFO_LOG( "CFileManagerEngine::ConstructL()-Create Feature Manager" )
    iFeatureManager = CFileManagerFeatureManager::NewL();

    iState = ENavigation;

    INFO_LOG( "CFileManagerEngine::ConstructL()-Create navigator" )
    iNavigator = CGflmNavigatorModel::NewL( iFs );

    // Create drives group
    MGflmItemGroup* newGroup = iNavigator->CreateGroupL(
        EFileManagerDrivesGroup, EDrives );
#ifdef RD_FILE_MANAGER_BACKUP
    // Set backup action
    HBufC* actionCaption = StringLoader::LoadLC( R_QTN_FMGR_MAIN_BACKUP );
    newGroup->AddActionItemL( EFileManagerBackupAction, *actionCaption );
    CleanupStack::PopAndDestroy( actionCaption );
#endif // RD_FILE_MANAGER_BACKUP

    // Create files and folders groups
    iNavigator->CreateGroupL( EFileManagerFilesGroup, EDirectories | EFiles );

    INFO_LOG( "CFileManagerEngine::ConstructL()-Create refresher" )
    iRefresher = CFileManagerRefresher::NewL( *iNavigator );

    INFO_LOG( "CFileManagerEngine::ConstructL()-Create utils" )
    iUtils = CFileManagerUtils::NewL( iFs, *iNavigator, *iFeatureManager );

    INFO_LOG( "CFileManagerEngine::ConstructL()-Create disk event" )
    iDiskEvent = CFileManagerFileSystemEvent::NewL( iFs, *this, ENotifyDisk );

    INFO_LOG( "CFileManagerEngine::ConstructL()-Create removable drive handler" )
    iRemovableDrvHandler = CFileManagerRemovableDriveHandler::NewL(
        iFs, *iUtils, *this );
    INFO_LOG( "CFileManagerEngine::ConstructL()-Create remote drive handler" )
    iRemoteDrvHandler = CFileManagerRemoteDriveHandler::NewL(
        *this, *iUtils );

    INFO_LOG( "CFileManagerEngine::ConstructL()-Create item filter" )
    iItemFilter = CFileManagerItemFilter::NewL( *this );
    iNavigator->SetCustomFilter( iItemFilter );

    // Store new starting index
    iNavigationIndices.AppendL( KErrNotFound );
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::~CFileManagerEngine
// Destructor
// -----------------------------------------------------------------------------
// 
EXPORT_C CFileManagerEngine::~CFileManagerEngine()
    {
    delete iActiveRename;
    iNavigationIndices.Reset();
    delete iRemoteDrvHandler;
    delete iBackupSettings;
    delete iRestoreSettings;
    delete iRemovableDrvHandler;
    delete iDocHandler;
    //delete iMgxFileManager;
    delete iFileSystemEvent;
    delete iRefresher;
    delete iNavigator;
    delete iItemFilter;
    delete iSearchString;
    delete iSearchFolder;
    delete iActiveSize;
    delete iDiskEvent;
    delete iUtils;
    delete iDriveName;
    delete iFeatureManager;
    if( iDelayedDiskEventNotify != NULL )
        {
        iDelayedDiskEventNotify->Cancel();
        delete iDelayedDiskEventNotify;
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::SetFileSystemEventL
// -----------------------------------------------------------------------------
//
void CFileManagerEngine::SetFileSystemEventL( const TDesC& aFullPath )
    {
    delete iFileSystemEvent;
    iFileSystemEvent = NULL;
    if ( aFullPath != KNullDesC )
        {
        iFileSystemEvent = CFileManagerFileSystemEvent::NewL(
            iFs, *this, ENotifyEntry, aFullPath );
        }
    else
        {
        iFileSystemEvent = CFileManagerFileSystemEvent::NewL(
            iFs, *this, ENotifyEntry );
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::IndexToFullPathL
// -----------------------------------------------------------------------------
//
EXPORT_C HBufC* CFileManagerEngine::IndexToFullPathL(
        const TInt aIndex ) const
    {
    if ( aIndex < 0 || aIndex >= iNavigator->MdcaCount() )
        {
        User::Leave( KErrNotFound );
        }
    CGflmGroupItem* item = iNavigator->Item( aIndex );
    HBufC* fullPath = NULL;
    switch ( item->Type() )
        {
        case CGflmGroupItem::EFile: // Fall through
        case CGflmGroupItem::EDirectory:
            {
            CGflmFileSystemItem* fsItem =
                static_cast< CGflmFileSystemItem* >( item );
            fullPath = fsItem->FullPathL();
            break;
            }
        case CGflmGroupItem::EDrive:
            {
            CGflmDriveItem* drvItem =
                static_cast< CGflmDriveItem* >( item );
            fullPath = drvItem->RootDirectory().AllocL();
            break;
            }
        default:
            {
            User::Leave( KErrNotFound );
            break;
            }
        }
    return fullPath;
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::IndexToFullPathLC
// -----------------------------------------------------------------------------
//
EXPORT_C HBufC* CFileManagerEngine::IndexToFullPathLC(
        const TInt aIndex ) const
    {
    HBufC* fullPath = IndexToFullPathL( aIndex );
    CleanupStack::PushL( fullPath );
    return fullPath;
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::AddFullPathLC
// -----------------------------------------------------------------------------
//
HBufC* CFileManagerEngine::AddFullPathLC(
        const TDesC& aName, const TBool aIsFolder )
    {
    TPtrC dir( iNavigator->CurrentDirectory() );
    TInt len( dir.Length() + aName.Length() );
    if ( aIsFolder )
        {
        ++len; // Ensure space for backslash
        }
    HBufC* fullPath = HBufC::NewLC( len );
    TPtr ptr( fullPath->Des() );
    ptr.Append( dir );
    ptr.Append( aName );
    if ( aIsFolder )
        {
        CFileManagerUtils::EnsureFinalBackslash( ptr );
        }
    return fullPath;
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::SetMemoryL
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CFileManagerEngine::SetMemoryL( TFileManagerMemory /*aMemory*/ )
    {
    // Just check current drive avaibility, no matter what memory is set
    iLastDriveAvailable = CurrentDriveAvailable();
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::Memory
// -----------------------------------------------------------------------------
//
EXPORT_C TFileManagerMemory CFileManagerEngine::Memory() const
    {
#ifdef RD_MULTIPLE_DRIVE
    CGflmDriveItem* drvItem = iNavigator->CurrentDrive();
    if ( drvItem )
        {
        TUint driveStatus( drvItem->DriveStatus() );
        if ( driveStatus & DriveInfo::EDriveInternal )
            {
            if ( driveStatus & DriveInfo::EDriveExternallyMountable )
                {
                return EFmMassStorage;
                }
            return EFmPhoneMemory;
            }
        else if ( driveStatus & DriveInfo::EDriveUsbMemory )
            {
            return EFmUsbMemory;
            }
        else if ( driveStatus & DriveInfo::EDriveRemovable )
            {
            return EFmMemoryCard;
            }
        else if ( driveStatus & DriveInfo::EDriveRemote )
            {
            return EFmRemoteDrive;
            }
        }
    return EFmPhoneMemory;
#else // RD_MULTIPLE_DRIVE
    CGflmDriveItem* drvItem = iNavigator->CurrentDrive();
    if ( drvItem )
        {
        const TVolumeInfo vol( drvItem->VolumeInfo() );
        if ( vol.iDrive.iDriveAtt & KDriveAttInternal )
            {
            return EFmPhoneMemory;
            }
        else if ( vol.iDrive.iDriveAtt & KDriveAttRemovable )
            {
            return EFmMemoryCard;
            }
        else if ( vol.iDrive.iDriveAtt & KDriveAttRemote )
            {
            return EFmRemoteDrive;
            }
        }
    return EFmPhoneMemory;
#endif // RD_MULTIPLE_DRIVE
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::FileList
// -----------------------------------------------------------------------------
//
EXPORT_C MDesCArray* CFileManagerEngine::FileList() const
    {
    return iNavigator;
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::OpenL
// -----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerEngine::OpenL( const TInt aIndex )
    {
    // Store drive letter for current drive changed checking
    TChar prevDrive( 0 );
    TPtrC prevDir( iNavigator->CurrentDirectory() );
    if ( prevDir.Length() )
        {
        prevDrive = prevDir[ 0 ];
        }   

    if ( aIndex < 0 || aIndex >= iNavigator->MdcaCount() )
        {
        User::Leave( KErrNotFound );
        }
    CGflmGroupItem* item = iNavigator->Item( aIndex );
    switch ( item->Type() )
        {
        case CGflmGroupItem::EFile: // Fall through
        case CGflmGroupItem::EDirectory: // Fall through
        case CGflmGroupItem::EDrive:
            {

#ifdef RD_MULTIPLE_DRIVE
            // Ensure that default folders exist
            if ( item->Type() == CGflmGroupItem::EDrive )
                {
                CGflmDriveItem* drvItem =
                    static_cast< CGflmDriveItem* >( item );
                iUtils->CreateDefaultFolders(
                    drvItem->Drive(), EFalse );
                }
#endif // RD_MULTIPLE_DRIVE

            // Store navigation position
            SetCurrentIndex( aIndex );

            HBufC* fullPath = IndexToFullPathLC( aIndex );
            if ( item->Type() == CGflmGroupItem::EFile )
                {
                OpenFileL( *fullPath );
                }
            else
                {
                OpenDirectoryL( *fullPath );

                // Setup new navigation position
                iNavigationIndices.AppendL( 0 );
                }
            CleanupStack::PopAndDestroy( fullPath );
            break;
            }
        case CGflmGroupItem::EGlobalActionItem:
            {
            CGflmGlobalActionItem* actItem = 
                static_cast< CGflmGlobalActionItem* >( item );
            if ( iProcessObserver )
                {
                iProcessObserver->NotifyL(
                    MFileManagerProcessObserver::ENotifyActionSelected,
                    actItem->Id() );
                }
            break;
            }
        default:
            {
            break;
            }
        }
 
    // Ensure that cached current drive info gets updated if drive was changed
    TPtrC dir( iNavigator->CurrentDirectory() );
    if ( dir.Length() )
        {
        if ( dir[ 0 ] != prevDrive )
            {
            iCurrentDriveInfoRefreshed = EFalse;
            }
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::SetObserver
// -----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerEngine::SetObserver(
        MFileManagerProcessObserver* aObserver )
    {
    iProcessObserver = aObserver;
    iRefresher->SetObserver( aObserver );
    iRemovableDrvHandler->SetObserver( aObserver );
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::CurrentDirectory
// -----------------------------------------------------------------------------
//
EXPORT_C TPtrC CFileManagerEngine::CurrentDirectory() const
    {
    return iNavigator->CurrentDirectory();
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::CurrentDirectory
// -----------------------------------------------------------------------------
//
EXPORT_C TPtrC CFileManagerEngine::LocalizedNameOfCurrentDirectory() const
    {
    return iNavigator->LocalizedNameOfCurrentDirectory();
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::BackstepL
// -----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerEngine::BackstepL()
    {
    // Store drive letter for current drive changed checking
    TChar prevDrive( 0 );
    // Note that prevDir gets invalid after BackstepL
    TPtrC prevDir( iNavigator->CurrentDirectory() );
    if ( prevDir.Length() )
        {
        prevDrive = prevDir[ 0 ];
        }

    // Setup backstep navigation index
    if ( !iNavigator->NavigationLevel() )
        {
        // Always first for initial root level
        iCurrentItemName.Zero();
        }
    else if( !iCurrentItemName.Length() )
        {
        // Otherwise previously opened folder
        iCurrentItemName.Copy(
            iNavigator->LocalizedNameOfCurrentDirectory() );
        }
    TInt pos( iNavigationIndices.Count() - 1 );
    if ( pos >= 0 )
        {
        iNavigationIndices.Remove( pos );
        }

    iNavigator->BackstepL();

    // Ensure that cached current drive info gets updated if drive was changed
    TPtrC dir( iNavigator->CurrentDirectory() );
    if ( dir.Length() )
        {
        if ( dir[ 0 ] != prevDrive )
            {
            iCurrentDriveInfoRefreshed = EFalse;
            }
        }
    else
        {
        // Make sure that drive info is refreshed
        ClearDriveInfo();
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::FileTypeL
// -----------------------------------------------------------------------------
//
EXPORT_C TUint32 CFileManagerEngine::FileTypeL( const TInt aIndex ) const
    {
    TUint32 fileType( CFileManagerItemProperties::ENotDefined );
    if ( aIndex >= 0 )
        {
        HBufC* fullPath = IndexToFullPathLC( aIndex );
        fileType = FileTypeL( *fullPath );
        CleanupStack::PopAndDestroy( fullPath ); 
        }
    return fileType;
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::FileTypeL
// -----------------------------------------------------------------------------
//
EXPORT_C TUint32 CFileManagerEngine::FileTypeL( const TDesC& aFullPath ) const
    {
    return iUtils->FileTypeL( aFullPath );
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::DeleteL
// -----------------------------------------------------------------------------
//
EXPORT_C CFileManagerActiveDelete* CFileManagerEngine::CreateActiveDeleteL( 
                                              CArrayFixFlat< TInt >& aIndexList )
    {

    CFileManagerActiveDelete* activeDelete = 
        CFileManagerActiveDelete::NewL( aIndexList, *this, *iUtils );
    return activeDelete;
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::NewFolder
// -----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerEngine::NewFolderL( const TDesC& aFolderName )
    {   
    // Add backslash to end so that MkDir realizes that it is directory
    HBufC* fullPath = AddFullPathLC( aFolderName, ETrue );
    TPtr ptr( fullPath->Des() );

    if ( ptr.Length() <= KMaxFileName )
        {
        iCurrentItemName.Copy( aFolderName );

        // Remove white spaces from end, file server also ignores those
        iCurrentItemName.TrimRight();

        User::LeaveIfError( iFs.MkDir( ptr ) );
        }
    else
        {
        User::Leave( KErrBadName );
        }

    CleanupStack::PopAndDestroy( fullPath ); 
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::SetCurrentItemName
// -----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerEngine::SetCurrentItemName( const TDesC& aFileName )
    { 

    iCurrentItemName.Copy( aFileName );

    // Remove white spaces from end, file server also ignores those
    iCurrentItemName.TrimRight();

    }
// -----------------------------------------------------------------------------
// CFileManagerEngine::RenameL
// -----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerEngine::RenameL(
        const TInt aIndex,
        const TDesC& aNewFileName )
    {
    FUNC_LOG

    // Make those given names as full paths
    TBool isFolder( IsFolder( aIndex ) );
    HBufC* fullPathOld = IndexToFullPathLC( aIndex );
    HBufC* fullPathNew = AddFullPathLC( aNewFileName, isFolder );

    if ( !fullPathOld->Compare( *fullPathNew ) )
        {
        // The name has not changed, rename is not required
        CleanupStack::PopAndDestroy( fullPathNew );
        CleanupStack::PopAndDestroy( fullPathOld );
        return;
        }

    // Gallery notification is relevant only for local drives
    TBool isRemoteDrive( iUtils->IsRemoteDrive( *fullPathOld ) );

    // Media Gallery albums get easily messed up when files are renamed.
    // Direct album editing seems a trick but it is seems to only way 
    // to update Gallery properly.
    /*CMGAlbumManager* albumManager = MGAlbumManagerFactory::NewAlbumManagerL();
    CleanupStack:: PushL( albumManager );
    TInt albumId( KErrNotFound );
    if ( !isFolder && !isRemoteDrive )
        {
        // Do album search for files only
        TRAPD( err, albumId = SearchMGAlbumIdL( *albumManager, *fullPathOld ) );
        if( err != KErrNone )
            {
            albumId = KErrNotFound;
            }
        }
*/
    // Handle rename
    delete iActiveRename;
    iActiveRename = NULL;
    iActiveRename = CFileManagerActiveRename::NewL(
        *this, *iUtils, *fullPathOld, *fullPathNew );
    ShowWaitDialogL( *iActiveRename );
    User::LeaveIfError( iActiveRename->Result() );

    // Update Media Gallery album if file exist in album
    /*if( albumId != KErrNotFound )
        {
        TRAP_IGNORE( { albumManager->RemoveFromAlbumL( *fullPathOld, albumId );
            albumManager->AddToAlbumL( *fullPathNew, albumId ); } );
        }
    CleanupStack::PopAndDestroy( albumManager );*/

    iCurrentItemName.Copy( aNewFileName );

    /*if ( !isRemoteDrive )
        {
        // Notify Media Gallery again
        TRAP_IGNORE( MGXFileManagerL().UpdateL( *fullPathOld, *fullPathNew ) );
        }*/

    CleanupStack::PopAndDestroy( fullPathNew );
    CleanupStack::PopAndDestroy( fullPathOld );
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::FolderLevel
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CFileManagerEngine::FolderLevel()
    {
    CGflmDriveItem* drvItem = iNavigator->CurrentDrive();
    if ( !drvItem )
        {
        return KErrNotFound;  // Main level
        }
    TPtrC dir( iNavigator->CurrentDirectory() );
    TInt count( dir.Length() );
    if ( !count )
        {
        return KErrNotFound; // Main level
        }
    // Count how many backslashes is in string to figure out the deepness
    TInt min( drvItem->RootDirectory().Length() );
	TInt bsCount( 0 );
	while( count > min )
		{
        --count;
		if ( dir[ count ] == KFmgrBackslash()[ 0 ] )
			{
			++bsCount;
			}
		}
	return bsCount;
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::IsNameFoundL
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CFileManagerEngine::IsNameFoundL( const TDesC& aName )
    {
    return iUtils->IsNameFoundL( aName );
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::CurrentDrive
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CFileManagerEngine::CurrentDrive()
    {
    TInt ret( KErrNotFound );
    TPtrC ptr( iNavigator->CurrentDirectory() );
    if ( ptr.Length() )
        {
        if( RFs::CharToDrive( ptr[ 0 ], ret ) != KErrNone )
            {
            ret = KErrNotFound;
            }
        }
    return ret;
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::GetItemInfoL
// -----------------------------------------------------------------------------
//
EXPORT_C CFileManagerItemProperties* CFileManagerEngine::GetItemInfoL( 
                                                const TInt aIndex )
    {
    if ( aIndex < 0 || aIndex >= iNavigator->MdcaCount() )
        {
        User::Leave( KErrNotFound );
        }
    CGflmGroupItem* item = iNavigator->Item( aIndex );
    return CFileManagerItemProperties::NewL( *item, *iUtils, *this );
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::SetState
// -----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerEngine::SetState( TState aState )
    {
    iState = aState;
    iNavigator->EnableSearchMode( iState == ESearch );
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::SetState
// -----------------------------------------------------------------------------
//
EXPORT_C CFileManagerEngine::TState CFileManagerEngine::State() const
    {
    return iState;
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::RefreshDirectory
// -----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerEngine::RefreshDirectory()
    {
    FUNC_LOG

#ifdef FILE_MANAGER_INFO_LOG_ENABLED
    if ( iState == ENavigation )
        {
        TPtrC dir( iNavigator->CurrentDirectory() );
        INFO_LOG1( "CFileManagerEngine::RefreshDirectory-dir=%S", &dir );
        }
#endif // FILE_MANAGER_INFO_LOG_ENABLED

    iRefresher->Refresh();
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::GetMMCInfoL
// -----------------------------------------------------------------------------
//
EXPORT_C TFileManagerDriveInfo CFileManagerEngine::GetMMCInfoL() const
    {
    TFileManagerDriveInfo drvInfo;
    GetDriveInfoL(
        PathInfo::MemoryCardRootPath(), drvInfo );
    return drvInfo;
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::SearchString
// -----------------------------------------------------------------------------
//
EXPORT_C TPtrC CFileManagerEngine::SearchString() const
    {
    if ( iSearchString )
        {
        return iSearchString->Des();
        }
    return TPtrC( KNullDesC );
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::IsValidName
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CFileManagerEngine::IsValidName(
        const TDesC& aDriveAndPath, const TDesC& aName, TBool aIsFolder ) const
    {
    // Check name for bad chars
    const TUint16 KMinAllowedChar = 0x0020;
    const TUint16 KParagraphSeparator = 0x2029;
    const TUint16 KDot = '.';
    TFileName nameAfterTrim( aName );
    nameAfterTrim.TrimRight(); 
    TInt nameLen( nameAfterTrim.Length() );
    if ( !nameLen )
        {
        return EFalse;
        }
    for ( TInt i( 0 ); i < nameLen; i++ )
        {
        TUint16 ch( nameAfterTrim[ i ] );
        if ( ch < KMinAllowedChar || ch == KParagraphSeparator )
            {
            return EFalse;
            }
        }
    // File system ignores totally dot in the end of name, so 
    // we set here as not valid name, so that user gets correctly informed
    if ( nameAfterTrim[ nameLen - 1 ] == KDot || IllegalChars( nameAfterTrim ) )
        {
        return EFalse;
        }
    // Get full path length
    TPtrC pathPtr( aDriveAndPath.Length() ?
        TPtrC( aDriveAndPath ) : TPtrC( iNavigator->CurrentDirectory() ) );
    TInt pathLen( pathPtr.Length() );
    if ( !pathLen )
        {
        return EFalse;
        }
    TInt fullPathLen( pathLen );
    if ( !CFileManagerUtils::HasFinalBackslash( pathPtr ) )
        {
        ++fullPathLen; // Add backslash before name
        }
    fullPathLen += nameLen;
    if ( aIsFolder )
        {
        ++fullPathLen; // Add folder final backslash
        }

    TBool ret( EFalse );
    if ( fullPathLen <= KMaxFileName )
        {
        // Check full path
        HBufC* fullPath = HBufC::New( KMaxFileName );
        if ( fullPath )
            {
            TPtr ptr( fullPath->Des() );
            ptr.Copy( pathPtr );
            CFileManagerUtils::EnsureFinalBackslash( ptr ); // Add backslash before name
            ptr.Append( nameAfterTrim );
            ret = iFs.IsValidName( ptr );
            delete fullPath;
            }
        }
    return ret;
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::IllegalChars
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CFileManagerEngine::IllegalChars( const TDesC& aName ) const
    {
    for ( TInt i( 0 ); KIllegalChars[ i ]; i++ )
        {
        if ( aName.Locate( KIllegalChars[ i ] ) != KErrNotFound )
            {
            return ETrue;
            }
        }
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::EnoughSpaceL
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CFileManagerEngine::EnoughSpaceL(
        const TDesC& aToFolder, 
        TInt64 aSize, 
        MFileManagerProcessObserver::TFileManagerProcess aOperation ) const
    {
    TPtrC src( iNavigator->CurrentDirectory() );
    TInt srcDrv( KErrNotFound );
    if ( src.Length() )
        {
        User::LeaveIfError( RFs::CharToDrive( src[ 0 ], srcDrv ) );
        }
    TInt dstDrv( KErrNotFound );
    if ( aToFolder.Length() )
        {
        User::LeaveIfError( RFs::CharToDrive( aToFolder[ 0 ], dstDrv ) );
        }
    else
        {
        return ETrue; // Invalid destination
        }

    TBool ret( EFalse );

    // if operation is move and drive is same so there is always enough memory
    if ( srcDrv == dstDrv &&
        aOperation == MFileManagerProcessObserver::EMoveProcess )
        {
        ret = ETrue;
        }
    else
        {
        ret = !SysUtil::DiskSpaceBelowCriticalLevelL(
            &iFs,  aSize , dstDrv );
        }

    return ret;
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::FileSystemEvent
// -----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerEngine::FileSystemEvent( TBool aEventOn )
    {
    INFO_LOG2(
        "CFileManagerEngine::FileSystemEvent-aEventOn=%d,iFileSystemEventEnabled=%d",
        aEventOn, iFileSystemEventEnabled )

    INFO_LOG2(
        "CFileManagerEngine::FileSystemEvent-iDriveAddedOrChangedPostponed=%d,iEmbeddedApplicationOn=%d ",
        iDriveAddedOrChangedPostponed, iEmbeddedApplicationOn )

    if ( !iFileSystemEventEnabled && aEventOn )
        {
        iDriveAddedOrChangedPostponed = EFalse;
        iFileSystemEventEnabled = ETrue;
        }
    else if ( iFileSystemEventEnabled && !aEventOn )
        {
        iFileSystemEventEnabled = EFalse;
        if ( iDriveAddedOrChangedPostponed )
            {
            iDriveAddedOrChangedPostponed = EFalse;
            TRAP_IGNORE( DriveAddedOrChangedL() );
            }
        }

    if ( iUtils->IsRemoteDrive( iNavigator->CurrentDirectory() ) )
        {
         // At the moment no use to have event for remote drives
        delete iFileSystemEvent;
        iFileSystemEvent = NULL;
        return;
        }

    // Set event on when there is no embedded app with event
    if ( aEventOn &&
         !( iEmbeddedApplicationOn && iFileSystemEvent ) )
        {
        TInt err( KErrNone );
        switch ( iState )
            {
            case ENavigation:
                {
                TRAP( err, SetFileSystemEventL( iNavigator->CurrentDirectory() ) );
                break;
                }
            case ESearch:
                {
                TRAP( err, SetFileSystemEventL() );
                break;
                }
            default:
                {
                break;
                }
            }
        if ( err != KErrNone && iProcessObserver )
            {
            iProcessObserver->Error( err );
            }
        }
    // Check event and set it off when there is no embedded app
    else if( !iEmbeddedApplicationOn && iFileSystemEvent )
        {
        iFileSystemEvent->CheckFileSystemEvent();
        delete iFileSystemEvent;
        iFileSystemEvent = NULL;
        }
    }

// ------------------------------------------------------------------------------
// CFileManagerEngine::CurrentIndex
//
// ------------------------------------------------------------------------------
//
EXPORT_C TInt CFileManagerEngine::CurrentIndex()
    {
    if ( iCurrentItemName.Length() )
        {
        TInt count( iNavigator->MdcaCount() );
        for( TInt i( 0 ); i < count; ++i )
            {
            CGflmGroupItem* item = iNavigator->Item( i );
            if ( !item->Name().CompareF( iCurrentItemName ) )
                {
                iCurrentItemName.Zero();
                return i;
                }
            }
        iCurrentItemName.Zero();
        }

    // Top navigation index
    TInt pos( iNavigationIndices.Count() - 1 );
    if ( pos >= 0 )
        {
        return iNavigationIndices[ pos ];
        }
    return KErrNotFound;
    }

// ------------------------------------------------------------------------------
// CFileManagerEngine::CanDelete
//
// ------------------------------------------------------------------------------
//
TBool CFileManagerEngine::CanDelete( const TDesC& aFileName ) const
    {
    return iUtils->CanDelete( aFileName );
    }

// ------------------------------------------------------------------------------
// CFileManagerEngine::LocalizedName
//
// ------------------------------------------------------------------------------
//
EXPORT_C TPtrC CFileManagerEngine::LocalizedName(
        const TDesC& aFullPath ) const
    {
    return iUtils->LocalizedName( aFullPath );
    }

// ------------------------------------------------------------------------------
// CFileManagerEngine::FilesInFolderL
//
// ------------------------------------------------------------------------------
//
EXPORT_C TInt CFileManagerEngine::FilesInFolderL()
    {
    TInt ret( 0 );
    switch ( iState )
        {
        case ENavigation: // Fall through
        case ESearch:
            {
            TInt count( iNavigator->MdcaCount() );
            for( TInt i( 0 ); i < count; ++i )
                {
                CGflmGroupItem* item = iNavigator->Item( i );
                if ( item->Type() == CGflmGroupItem::EFile )
                    {
                    ++ret;
                    }
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
// CFileManagerEngine::CancelRefresh
//
// ------------------------------------------------------------------------------
//
EXPORT_C TBool CFileManagerEngine::CancelRefresh()
    {
    switch ( iState )
        {
        case ENavigation:
            {
            CancelTransfer( iNavigator->CurrentDirectory() );
            break;
            }
        case ESearch:
            {
            if ( iSearchFolder )
                {
                CancelTransfer( *iSearchFolder );
                }
            break;
            }
        default:
            {
            break;
            }
        }
    return iRefresher->CancelRefresh();
    }

// ------------------------------------------------------------------------------
// CFileManagerEngine::DriveAddedOrChangeAsyncL
//
// ------------------------------------------------------------------------------
//
TInt CFileManagerEngine::DriveAddedOrChangeAsyncL( TAny* aPtr )
    {
    static_cast<CFileManagerEngine*>( aPtr )->DriveAddedOrChangedL();

    //return value will be ignored by CPeriodic that calls this function
    //following line keeps the compiler happy
    return 0;
    }

// ------------------------------------------------------------------------------
// CFileManagerEngine::DriveAddedOrChangedL
//
// ------------------------------------------------------------------------------
//
void CFileManagerEngine::DriveAddedOrChangedL()
    {
    FUNC_LOG
    
    if( iDiskEvent )
        {
        iDiskEvent->Setup();
        }
        
    TBool processOngoing( iRemovableDrvHandler->IsProcessOngoing() );

    INFO_LOG3( "CFileManagerEngine::DriveAddedOrChangedL-event=%d,process=%d,postponed=%d ",
        iFileSystemEventEnabled, processOngoing, iDriveAddedOrChangedPostponed )

    if ( iFileSystemEventEnabled || processOngoing )
        {
        iDriveAddedOrChangedPostponed = ETrue;
        }
    else
        {
        iDriveAddedOrChangedPostponed = EFalse;
        ClearDriveInfo();
        if ( iProcessObserver &&
             !iEmbeddedApplicationOn &&
             !iWaitDialogOn &&
             !iRefresher->IsActive() )
            {
            StopDiskEventNotifyTimerAsync();
            TPtrC dir( iNavigator->CurrentDirectory() );
            if ( dir.Length() )
                {
                TBool isAvailable( CurrentDriveAvailable() );

                if ( iUtils->IsRemoteDrive( dir ) )
                    {
                    // Notify whether remote drive has become available
                    // So when receiving remote drive connection event,
                    // View will be refreshed
                    //if ( !iLastDriveAvailable && isAvailable )
                    //    {
                        iProcessObserver->NotifyL(
                            MFileManagerProcessObserver::ENotifyDisksChanged, 0 );
                    //    }
                    }
                else
                    {
                    // Notify if drive has become unavailable or
                    // avaibility has changed
                    if ( !isAvailable ||
                        isAvailable != iLastDriveAvailable )
                        {
                        iProcessObserver->NotifyL(
                            MFileManagerProcessObserver::ENotifyDisksChanged, 0 );
                        }
                    }
                }
            else
                {
                // Notify always when no folder is opened
                iProcessObserver->NotifyL(
                    MFileManagerProcessObserver::ENotifyDisksChanged, 0 );
                }
            }
        else
            {
            if( ( iProcessObserver == NULL ) || iEmbeddedApplicationOn )
                {
                //Do not refresh while embedded application is running or process observer is not set
                StopDiskEventNotifyTimerAsync();
                }
            else
                {
                if( iRefresher->IsActive() )
                   {
                   //start Timer and notify disk event until current disk refresh finishes
                   StartDiskEventNotifyTimerAsyncL();
                   }
                }
            }
        }
    }

// ------------------------------------------------------------------------------
// CFileManagerEngine::FolderContentChangedL
//
// ------------------------------------------------------------------------------
//
void CFileManagerEngine::FolderContentChangedL()
    {
    if( iProcessObserver )
        {
        iProcessObserver->NotifyL(
            MFileManagerProcessObserver::ENotifyDisksChanged, 0 );
        }
    else
        {
        RefreshDirectory();
        }
    }

// ------------------------------------------------------------------------------
// CFileManagerEngine::IsFolder
//
// ------------------------------------------------------------------------------
//
EXPORT_C TBool CFileManagerEngine::IsFolder( const TInt aIndex ) const
    {
    if ( aIndex < 0 || aIndex >= iNavigator->MdcaCount() )
        {
        return EFalse;
        }
    CGflmGroupItem* item = iNavigator->Item( aIndex );
    TBool ret( EFalse );
    switch ( item->Type() )
        {
        case CGflmGroupItem::EDirectory: // Fallthrough
        case CGflmGroupItem::EDrive: // Fallthrough
        case CGflmGroupItem::EGlobalActionItem:
            {
            ret = ETrue;
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
// CFileManagerEngine::IconIdL
//
// ------------------------------------------------------------------------------
//
EXPORT_C TInt CFileManagerEngine::IconIdL( const TInt aIndex ) const
    {
    if ( aIndex < 0 || aIndex >= iNavigator->MdcaCount() )
        {
        User::Leave( KErrNotFound );
        }
    CGflmGroupItem* item = iNavigator->Item( aIndex );
    return iUtils->ResolveIconL( *item );
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::ShowWaitDialogL( MAknBackgroundProcess& aProcess )
//
// -----------------------------------------------------------------------------
//
void CFileManagerEngine::ShowWaitDialogL( MAknBackgroundProcess& aProcess )
    {
    iWaitDialogOn = ETrue;
    TRAPD( err, iProcessObserver->ShowWaitDialogL( aProcess ) );
    iWaitDialogOn = EFalse;
    User::LeaveIfError( err );
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::EmbeddedAppStatus( TBool aStatus )
//
// -----------------------------------------------------------------------------
//
void CFileManagerEngine::EmbeddedAppStatus( TBool aStatus )
    {
    iEmbeddedApplicationOn = aStatus;
    FileSystemEvent( aStatus );
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::IsSystemFolder
//
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CFileManagerEngine::IsSystemFolder( const TDesC& aFullPath ) const
    {
    _LIT( KSystemFolder, "?:\\system\\*" );
    _LIT( KSysFolder, "?:\\sys\\*" );
    _LIT( KPrivateFolder, "?:\\private\\*" );
    _LIT( KResourceFolder, "?:\\resource\\*" );

    if ( aFullPath.MatchF( KSystemFolder ) != KErrNotFound ||
        aFullPath.MatchF( KSysFolder ) != KErrNotFound ||
        aFullPath.MatchF( KResourceFolder ) != KErrNotFound ||
        aFullPath.MatchF( KPrivateFolder ) != KErrNotFound )
        {
        return ETrue;
        }
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::CurrentDriveName()
//
// -----------------------------------------------------------------------------
//
EXPORT_C TPtrC CFileManagerEngine::CurrentDriveName()
    {
#ifdef RD_MULTIPLE_DRIVE
    TPtrC dir( iNavigator->CurrentDirectory() );
    if ( dir.Length() )
        {
        TInt drive = TDriveUnit( dir );
        return DriveName( drive );
        }
#else // RD_MULTIPLE_DRIVE
    CGflmDriveItem* drvItem = iNavigator->CurrentDrive();
    if ( drvItem )
        {
        return drvItem->Name();
        }
#endif // RD_MULTIPLE_DRIVE
    return TPtrC( KNullDesC );
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::GetDriveInfoL()
//
// -----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerEngine::GetDriveInfoL(
        TFileManagerDriveInfo& aInfo )
    {
    if ( iCurrentDriveInfoRefreshed )
        {
        // Use cached info
        aInfo = iCurrentDriveInfo;
        return;
        }

    CGflmDriveItem* drvItem = iNavigator->CurrentDrive();
    if ( drvItem )
        {
        GetDriveInfoL( drvItem->Drive(), aInfo );

        // Store info for later use
        iCurrentDriveInfo = aInfo;
        iCurrentDriveInfoRefreshed = ETrue;
        }
    else
        {
        User::Leave( KErrNotFound );
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::IsDistributableFile()
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CFileManagerEngine::IsDistributableFile(
        const TDesC& aFullPath, TBool& aIsProtected ) const
    {
    return iUtils->IsDistributableFile( aFullPath, aIsProtected );
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::RenameDrive()
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CFileManagerEngine::RenameDrive(
        const TInt aDrive,
        const TDesC& aName )
    {
    TInt len( aName.Length( ) );
    for( TInt i( 0 ); i < len; i++ )
        {
        TChar ch( aName[ i ] );
        // If not alphadigit or space, return error
        if( !ch.IsAlphaDigit() && !ch.IsSpace() )
            {
            return KErrBadName;
            }
        }
    TInt err( iFs.SetVolumeLabel( aName, aDrive ) );
    if ( err == KErrNone )
        {
        // Refresh drive info after rename
        ClearDriveInfo();
        }
    return err;
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::SetDrivePassword()
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CFileManagerEngine::SetDrivePassword(
        const TInt aDrive,
        const TMediaPassword& aOld,
        const TMediaPassword& aNew )
    {
    ClearDriveInfo();
    TInt err( iFs.LockDrive( aDrive, aOld, aNew, ETrue ) );
    INFO_LOG1( "CFileManagerEngine::SetDrivePassword err %d", err );
    return err;
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::RemoveDrivePassword()
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CFileManagerEngine::RemoveDrivePassword(
        const TInt aDrive,
        const TMediaPassword& aPwd )
    {
    ClearDriveInfo();
    TInt err( iFs.ClearPassword( aDrive, aPwd ) );
    INFO_LOG1( "CFileManagerEngine::RemoveDrivePassword err %d", err );
    return err;
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::UnlockDrive()
//
// -----------------------------------------------------------------------------
//    
EXPORT_C TInt CFileManagerEngine::UnlockDrive(
        const TInt aDrive,
        const TMediaPassword& aPwd )
    {
    ClearDriveInfo();
    TInt err( iFs.UnlockDrive( aDrive, aPwd, ETrue ) );
    INFO_LOG1( "CFileManagerEngine::UnlockDrive err %d", err );
    return err;
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::StartFormatProcessL()
//
// -----------------------------------------------------------------------------
//  
EXPORT_C void CFileManagerEngine::StartFormatProcessL(
        const TInt aDrive )
    {
    ClearDriveInfo();
    iRemovableDrvHandler->StartFormatL( aDrive );
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::StartEjectProcessL()
//
// -----------------------------------------------------------------------------
//  
EXPORT_C void CFileManagerEngine::StartEjectProcessL( const TInt /*aDrive*/ )
    {
    ClearDriveInfo();
    iRemovableDrvHandler->StartEjectL();
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::StartBackupProcessL()
//
// -----------------------------------------------------------------------------
//  
EXPORT_C void CFileManagerEngine::StartBackupProcessL(
        MFileManagerProcessObserver::TFileManagerProcess aProcess )
    {
    switch( aProcess )
        {
        case MFileManagerProcessObserver::EBackupProcess: // FALLTHROUGH
        case MFileManagerProcessObserver::ESchBackupProcess:
            {
            // Make sure that there is no embedded apps open
            delete iDocHandler;
            iDocHandler = NULL;

            ClearDriveInfo();
            iRemovableDrvHandler->StartBackupL( aProcess );
            break;
            }
        case MFileManagerProcessObserver::ERestoreProcess:
            {
            ClearDriveInfo();
            iRemovableDrvHandler->StartRestoreL();
            break;
            }
        default:
            {
            User::Leave( KErrNotSupported );
            break;
            }
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::CancelProcess()
//
// -----------------------------------------------------------------------------
//  
EXPORT_C void CFileManagerEngine::CancelProcess(
        MFileManagerProcessObserver::TFileManagerProcess aProcess )
    {
    switch( aProcess )
        {
        case MFileManagerProcessObserver::EFormatProcess:
        case MFileManagerProcessObserver::EBackupProcess: // FALLTHROUGH
        case MFileManagerProcessObserver::ERestoreProcess: // FALLTHROUGH
        case MFileManagerProcessObserver::ESchBackupProcess: // FALLTHROUGH
            {
            iRemovableDrvHandler->CancelProcess();
            break;
            }
        case MFileManagerProcessObserver::EFileOpenProcess:
            {
            if ( iDocHandler )
                {
                iDocHandler->CancelFileOpen();
                }
            break;
            }
        default:
            {
            break;
            }
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::GetDriveInfoL()
//
// -----------------------------------------------------------------------------
// 
void CFileManagerEngine::GetDriveInfoL(
        const TDesC& aPath,
        TFileManagerDriveInfo& aInfo ) const
    {
    if ( !aPath.Length() )
        {
        User::Leave( KErrNotFound );
        }
    TInt drv( 0 );
    User::LeaveIfError( RFs::CharToDrive( aPath[ 0 ], drv ) );
    GetDriveInfoL( drv, aInfo );
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::DriveState()
//
// -----------------------------------------------------------------------------
// 
EXPORT_C TInt CFileManagerEngine::DriveState(
        TUint32& aState, const TDesC& aPath ) const
    {
    TFileManagerDriveInfo info;
    TRAPD( err, GetDriveInfoL( aPath, info ) );
    if ( err == KErrNone )
        {
        aState = info.iState;
        }
    else
        {
        aState = 0;
        }
    return err;
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::DriveState()
//
// -----------------------------------------------------------------------------
// 
EXPORT_C TInt CFileManagerEngine::DriveState(
        TUint32& aState, const TInt aDrive ) const
    {
    TFileManagerDriveInfo info;
    TRAPD( err, GetDriveInfoL( aDrive, info ) );
    if ( err == KErrNone )
        {
        aState = info.iState;
        }
    else
        {
        aState = 0;
        }
    return err;
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::MGXFileManagerL()
//
// -----------------------------------------------------------------------------
// 
/*CMGXFileManager& CFileManagerEngine::MGXFileManagerL()
    {
    // MG2 notification object
    if ( !iMgxFileManager )
        {
        INFO_LOG( "CFileManagerEngine::MGXFileManagerL()-Create" )
        iMgxFileManager = MGXFileManagerFactory::NewFileManagerL( iFs );
        }
    return *iMgxFileManager;
    }*/

// -----------------------------------------------------------------------------
// CFileManagerEngine::Fs()
//
// -----------------------------------------------------------------------------
// 
RFs& CFileManagerEngine::Fs() const
    {
    return iFs;
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::IsRemoteDriveConnected()
//
// -----------------------------------------------------------------------------
// 
TBool CFileManagerEngine::IsRemoteDriveConnected( const TInt aDrive  ) const
    {
    return iRemoteDrvHandler->IsConnected( aDrive );
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::SetRemoteDriveConnection()
//
// -----------------------------------------------------------------------------
// 
EXPORT_C TInt CFileManagerEngine::SetRemoteDriveConnection(
        const TInt aDrive, TBool aConnect )
    {
    return iRemoteDrvHandler->SetConnection( aDrive, aConnect );
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::DeleteRemoteDrive()
//
// -----------------------------------------------------------------------------
// 
EXPORT_C TInt CFileManagerEngine::DeleteRemoteDrive( const TInt aDrive )
    {
    return iRemoteDrvHandler->DeleteSettings( aDrive );
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::BackupFileExistsL()
//
// -----------------------------------------------------------------------------
// 
#ifndef RD_FILE_MANAGER_BACKUP

TBool CFileManagerEngine::BackupFileExistsL( const TInt aDrive ) const
    {
    return iRemovableDrvHandler->BackupFileExistsL( aDrive );
    }

#endif // RD_FILE_MANAGER_BACKUP

// -----------------------------------------------------------------------------
// CFileManagerEngine::OpenFileL()
//
// -----------------------------------------------------------------------------
// 
void CFileManagerEngine::OpenFileL( const TDesC& aFullPath )
    {
    if ( !iDocHandler )
        {
        iDocHandler = CFileManagerDocHandler::NewL( *this, *iUtils );
        }
    iDocHandler->OpenFileL( aFullPath, iProcessObserver );
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::OpenDirectoryL()
//
// -----------------------------------------------------------------------------
// 
void CFileManagerEngine::OpenDirectoryL( const TDesC& aFullPath )
    {
    iNavigator->GoToDirectoryL( aFullPath, ETrue );
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::DriveRootDirectory()
//
// -----------------------------------------------------------------------------
//
EXPORT_C TPtrC CFileManagerEngine::DriveRootDirectory( TInt aDrive ) const
    {
    CGflmDriveItem* drvItem = iNavigator->DriveFromId( aDrive );
    if ( drvItem )
        {
        return drvItem->RootDirectory();
        }
    return TPtrC( KNullDesC );
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::BackupSettingsL()
//
// -----------------------------------------------------------------------------
//
EXPORT_C CFileManagerBackupSettings& CFileManagerEngine::BackupSettingsL()
    {
    if ( !iBackupSettings )
        {
        INFO_LOG( "CFileManagerEngine::BackupSettingsL()-Create" )
        iBackupSettings = CFileManagerBackupSettings::NewL( *this );
        }
    return *iBackupSettings;
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::RestoreSettingsL()
//
// -----------------------------------------------------------------------------
//
EXPORT_C CFileManagerRestoreSettings& CFileManagerEngine::RestoreSettingsL()
    {
    if ( !iRestoreSettings )
        {
        INFO_LOG( "CFileManagerEngine::RestoreSettingsL()-Create" )
        iRestoreSettings = CFileManagerRestoreSettings::NewL( *this );
        }
    return *iRestoreSettings;
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::GetRestoreInfoArrayL()
//
// -----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerEngine::GetRestoreInfoArrayL(
        RArray< CFileManagerRestoreSettings::TInfo >& aArray,
        const TInt aDrive ) const
    {
    return iRemovableDrvHandler->GetRestoreInfoArrayL( aArray, aDrive );
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::DriveName()
//
// -----------------------------------------------------------------------------
//
EXPORT_C TPtrC CFileManagerEngine::DriveName( const TInt aDrive )
    {
    CGflmDriveItem* drvItem = iNavigator->DriveFromId( aDrive );
    if ( drvItem )
        {
#ifdef RD_MULTIPLE_DRIVE
        delete iDriveName;
        iDriveName = NULL;

        TRAPD( err, iDriveName = iUtils->GetDriveNameL(
            drvItem->Drive(), CFileManagerUtils::ETitleLayout ) );
        if ( err == KErrNone )
            {
            return iDriveName->Des();
            }
#else // RD_MULTIPLE_DRIVE
        return drvItem->Name();
#endif // RD_MULTIPLE_DRIVE
        }
    return TPtrC( KNullDesC );
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::GetDriveInfoL()
//
// -----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerEngine::GetDriveInfoL(
            const TInt aDrive, TFileManagerDriveInfo& aInfo ) const
    {
    aInfo.GetInfoL( *this, aDrive );
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::CurrentDriveAvailable()
//
// -----------------------------------------------------------------------------
//
TBool CFileManagerEngine::CurrentDriveAvailable()
    {
    TFileManagerDriveInfo info;
    TRAPD( err, GetDriveInfoL( info ) );
    if ( err != KErrNone )
        {
        return EFalse;
        }

    TInt ret( ETrue );
    if ( info.iState & (
            TFileManagerDriveInfo::EDriveCorrupted |
            TFileManagerDriveInfo::EDriveLocked ) )
        {
        ret = EFalse;
        }
    else if ( !( info.iState & TFileManagerDriveInfo::EDrivePresent ) )
        {
        ret = EFalse;
        }

    if ( ( info.iState & TFileManagerDriveInfo::EDriveRemote ) &&
         !( info.iState & TFileManagerDriveInfo::EDriveConnected ) )
        {
        ret = EFalse;
        }
    return ret;
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::ClearDriveInfo()
//
// -----------------------------------------------------------------------------
//
void CFileManagerEngine::ClearDriveInfo()
    {
    iCurrentDriveInfoRefreshed = EFalse;
    iNavigator->ClearDriveInfo();
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::AnyEjectableDrivePresent()
//
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CFileManagerEngine::AnyEjectableDrivePresent() const
    {
    RArray< TInt > drives;
    iUtils->DriveList( drives, KDriveAttRemovable );
    TInt ret( EFalse );
    TInt count( drives.Count() );
    const TInt KPresentBits(
        TFileManagerDriveInfo::EDriveEjectable |
        TFileManagerDriveInfo::EDrivePresent );
    for( TInt i( 0 ); i < count; ++i )
        {
        TUint32 state( 0 );
        if ( DriveState( state, drives[ i ] ) == KErrNone )
            {
            if ( ( state & KPresentBits ) == KPresentBits )
                {
                ret = ETrue;
                break;
                }
            }
        }
    drives.Close();
    return ret;
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::ForcedRefreshDirectory()
//
// -----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerEngine::ForcedRefreshDirectory()
    {
    TPtrC ptr( iNavigator->CurrentDirectory() );
    if ( ptr.Length() && iUtils->IsRemoteDrive( ptr ) )
        {
        // Ensure that remote directory is reloaded
        iRemoteDrvHandler->RefreshDirectory( ptr );
        }
    RefreshDirectory();
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::HasAppRemoteDriveSupport()
//
// -----------------------------------------------------------------------------
//
TBool CFileManagerEngine::HasAppRemoteDriveSupport( TUid aUid )
    {
    return iRemoteDrvHandler->HasAppRemoteDriveSupport( aUid );
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::GetFileSizesL()
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt64 CFileManagerEngine::GetFileSizesL(
        const CArrayFixFlat<TInt>& aIndexList )
    {
    FUNC_LOG

    TBool isCanceled( EFalse );
    TInt64 size( 0 );
    TInt count( aIndexList.Count() );

    for( TInt i( 0 ); i < count; ++i )
        {
        CGflmGroupItem* item = iNavigator->Item( aIndexList.At( i ) );
        switch ( item->Type() )
            {
            case CGflmGroupItem::EFile:
                {
                CGflmFileSystemItem* fsItem =
                    static_cast< CGflmFileSystemItem* >( item );
                size += static_cast<TUint>(fsItem->Entry().iSize);
                break;
                }
            case CGflmGroupItem::EDirectory:
                {                        
                CGflmFileSystemItem* fsItem =
                    static_cast< CGflmFileSystemItem* >( item );
                HBufC* fullPath = fsItem->FullPathLC();

                delete iActiveSize;
                iActiveSize = NULL;

                iActiveSize = CFileManagerActiveSize::NewL(
                    iFs, *fullPath );

                ShowWaitDialogL( *iActiveSize );

                TInt64 folderSize( iActiveSize->GetFolderSize() );
                if ( folderSize == KErrNotFound )
                    {
                    // Calculation was canceled by the user
                    isCanceled = ETrue;
                    }
                else
                    {
                    size += folderSize;
                    }

                CleanupStack::PopAndDestroy( fullPath );
                break;
                }
            default:
                {
                break;
                }
            }

        if ( isCanceled )
            {
            break;
            }
        }

    if ( isCanceled )
        {
        return KErrNotFound;
        }
    return size;
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::CancelTransfer()
//
// -----------------------------------------------------------------------------
//
void CFileManagerEngine::CancelTransfer( const TDesC& aFullPath )
    {
    if ( iUtils->IsRemoteDrive( aFullPath ) )
        {
        iRemoteDrvHandler->CancelTransfer( aFullPath );
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::NavigationLevel
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CFileManagerEngine::NavigationLevel() const
    {
    return iNavigator->NavigationLevel();
    }

#ifdef RD_MULTIPLE_DRIVE
// -----------------------------------------------------------------------------
// CFileManagerEngine::GetFormattedDriveNameLC
// -----------------------------------------------------------------------------
//
EXPORT_C HBufC* CFileManagerEngine::GetFormattedDriveNameLC(
        const TInt aDrive,
        const TInt aTextIdForDefaultName,
        const TInt aTextIdForName ) const
    {
    return iUtils->GetFormattedDriveNameLC(
        aDrive, aTextIdForDefaultName, aTextIdForName );
    }
#else // RD_MULTIPLE_DRIVE
EXPORT_C HBufC* CFileManagerEngine::GetFormattedDriveNameLC(
        const TInt /*aDrive*/,
        const TInt /*aTextIdForDefaultName*/,
        const TInt /*aTextIdForName*/  ) const
    {
    User::Leave( KErrNotSupported );
    return NULL;
    }
#endif // RD_MULTIPLE_DRIVE

// -----------------------------------------------------------------------------
// CFileManagerEngine::GetItemInfoLC
// -----------------------------------------------------------------------------
//
EXPORT_C CFileManagerItemProperties* CFileManagerEngine::GetItemInfoLC(
        const TInt aIndex )
    {
    CFileManagerItemProperties* ret = GetItemInfoL( aIndex );
    CleanupStack::PushL( ret );
    return ret;
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::SetSearchStringL
// -----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerEngine::SetSearchStringL( const TDesC& aSearchString )
    {
    delete iSearchString;
    iSearchString = NULL;
    iSearchString = aSearchString.AllocL();
    iNavigator->SetSearchStringL( aSearchString );
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::SetSearchFolderL
// -----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerEngine::SetSearchFolderL( const TDesC& aSearchFolder )
    {
    delete iSearchFolder;
    iSearchFolder = NULL;
    iSearchFolder = aSearchFolder.AllocL();
    iNavigator->SetSearchFolderL( aSearchFolder );
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::SetDirectoryWithBackstepsL
// -----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerEngine::SetDirectoryWithBackstepsL(
        const TDesC& aDirectory )
    {
    CGflmDriveItem* drvItem = iNavigator->DriveFromPath( aDirectory );
    if ( !drvItem )
        {
        return;
        }
    // Build backsteps
    iNavigator->GoToDirectoryL( drvItem->RootDirectory(), aDirectory );

    // Build navigation indices
    iNavigationIndices.Reset();
    iNavigationIndices.AppendL( KErrNotFound ); // Set initial
    TInt count( iNavigator->NavigationLevel() ); // Set backsteps
    for( TInt i( 0 ); i < count; ++i )
        {
        iNavigationIndices.AppendL( KErrNotFound );
        }
    iNavigationIndices.AppendL( KErrNotFound ); // Set current
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::SetCurrentIndex
// -----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerEngine::SetCurrentIndex( const TInt aIndex )
    {
    TInt pos( iNavigationIndices.Count() - 1 );
    if ( pos >= 0 )
        {
        iNavigationIndices[ pos ] = aIndex;
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::SetSortMethod
// -----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerEngine::SetSortMethod( TSortMethod aSortMethod )
    {
    switch ( aSortMethod )
        {
        case EByName:          // Fallthrough
        case EByType:          // Fallthrough
        case EMostRecentFirst: // Fallthrough
        case ELargestFirst:    // Fallthrough
        case EByMatch:
            {
            // Make sure that the enums do match
            iNavigator->SetSortMethod(
                static_cast< TGflmSortMethod >( aSortMethod ) );
            break;
            }
        default:
            {
            ERROR_LOG1(
                "CFileManagerEngine::SetSortMethod-InvalidMethod=%d",
                aSortMethod )
            break;
            }
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::SortMethod
// -----------------------------------------------------------------------------
//
EXPORT_C CFileManagerEngine::TSortMethod CFileManagerEngine::SortMethod() const
    {
    // Make sure that the enums do match
    return static_cast< TSortMethod >( iNavigator->SortMethod() );
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::RefreshSort
// -----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerEngine::RefreshSort()
    {
    iRefresher->Refresh( ERefreshSort );
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::FeatureManager
// -----------------------------------------------------------------------------
//
EXPORT_C CFileManagerFeatureManager& CFileManagerEngine::FeatureManager() const
    {
    return *iFeatureManager;
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::SetAppExitOb
// -----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerEngine::SetAppExitOb( MAknServerAppExitObserver* aObserver )
    {
    iObserver = aObserver;
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::NoticeServerAppExit
// -----------------------------------------------------------------------------
//
void CFileManagerEngine::NoticeServerAppExit( TInt aReason )
    {
    if ( iObserver && iSisFile )
        {
        iUtils->FlushCache();
        iObserver->HandleServerAppExit( aReason );
        iSisFile = EFalse;
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::SetSisFile
// -----------------------------------------------------------------------------
//
void CFileManagerEngine::SetSisFile( TBool aSisFile )
    {
    iSisFile = aSisFile;
    }

// ---------------------------------------------------------------------------
// CFileManagerEngine::LatestBackupTimeL()
// ---------------------------------------------------------------------------
//
TInt CFileManagerEngine::LatestBackupTime( TTime& aBackupTime )
    {
    TRAPD( err, iRemovableDrvHandler->LatestBackupTimeL( aBackupTime ) );
    return err;
    }

// ---------------------------------------------------------------------------
// CFileManagerEngine::DeleteBackupsL()
// ---------------------------------------------------------------------------
//
EXPORT_C void CFileManagerEngine::DeleteBackupsL()
    {
    iRemovableDrvHandler->DeleteBackupsL();
    }

// ---------------------------------------------------------------------------
// CFileManagerEngine::StartDiskEventNotifyTimerAsyncL()
// ---------------------------------------------------------------------------
//
void CFileManagerEngine::StartDiskEventNotifyTimerAsyncL()
    {
    if ( iDelayedDiskEventNotify == NULL )
        {
        iDelayedDiskEventNotify = CPeriodic::NewL( CActive::EPriorityStandard );
        }
    if ( !iDelayedDiskEventNotify->IsActive() )
        {
       //ignore disk event notification while timer is already active
       iDelayedDiskEventNotify->Start( KDiskEventCheckInterval,
           KDiskEventCheckInterval,
           TCallBack( DriveAddedOrChangeAsyncL, this ) );
        }
    }

// ---------------------------------------------------------------------------
// CFileManagerEngine::StopDiskEventNotifyTimerAsync()
// ---------------------------------------------------------------------------
//
void CFileManagerEngine::StopDiskEventNotifyTimerAsync()
    {
    if ( iDelayedDiskEventNotify != NULL )
        {
        iDelayedDiskEventNotify->Cancel();
        delete iDelayedDiskEventNotify;
        iDelayedDiskEventNotify = NULL;
        }
    }


//  End of File
