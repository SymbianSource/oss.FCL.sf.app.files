/*
* Copyright (c) 2002-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Holds item information
*
*/


// INCLUDES
#include <e32base.h>
#include <f32file.h>
#include <tz.h>
#include "CFileManagerItemProperties.h"
#include "CFileManagerUtils.h"
#include "CFilemanagerMimeIconArray.h"
#include "Cfilemanagerfolderarray.h"
#include "Cfilemanageractivesize.h"
#include "CFilemanagerActiveCount.h"
#include "CFileManagerEngine.h" 
#include "CGflmGroupItem.h"
#include "CGflmFileSystemItem.h"
#include "CGflmDriveItem.h"
#include "CGflmGlobalActionItem.h"


// ============================ MEMBER FUNCTIONS ===============================
// -----------------------------------------------------------------------------
// CFileManagerItemProperties::CFileManagerItemProperties
// 
// -----------------------------------------------------------------------------
// 
CFileManagerItemProperties::CFileManagerItemProperties( 
        CFileManagerUtils& aUtils,
        CFileManagerEngine& aEngine ) :
            iSize( KErrNotFound ),
            iFilesContained( KErrNotFound ),
            iFoldersContained( KErrNotFound ),
            iOpenFiles( KErrNotFound ),
            iUtils( aUtils ),
            iEngine( aEngine )
    {
    }

// -----------------------------------------------------------------------------
// CFileManagerItemProperties::NewL
// 
// -----------------------------------------------------------------------------
// 
CFileManagerItemProperties* CFileManagerItemProperties::NewL( 
        const TDesC& aFullPath,
        CFileManagerUtils& aUtils,
        CFileManagerEngine& aEngine )
    {
    CFileManagerItemProperties* self =
        new (ELeave) CFileManagerItemProperties( aUtils, aEngine );
    CleanupStack::PushL( self );
    self->ConstructL( aFullPath );
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CFileManagerItemProperties::NewL
// 
// -----------------------------------------------------------------------------
// 
CFileManagerItemProperties* CFileManagerItemProperties::NewL(
        const CGflmGroupItem& aItem,
        CFileManagerUtils& aUtils,
        CFileManagerEngine& aEngine )
    {
    CFileManagerItemProperties* self =
        new (ELeave) CFileManagerItemProperties( aUtils, aEngine );
    CleanupStack::PushL( self );
    self->ConstructL( aItem );
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CFileManagerItemProperties::ConstructL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerItemProperties::ConstructL( const TDesC& aFullPath )
    {
    iFullPath = aFullPath.AllocL();

    // Setup the rest of entry data when needed first time
    if ( CFileManagerUtils::HasFinalBackslash( aFullPath ) )
        {
        iState |= EItemDirectory;
        }
    else
        {
        iState |= EItemFile;
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerItemProperties::ConstructL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerItemProperties::ConstructL(
        const TDesC& aFullPath, const TEntry& aEntry )
    {
    iFullPath = aFullPath.AllocL();

    // Setup all entry data now
    SetEntryData( aEntry );
    }

// -----------------------------------------------------------------------------
// CFileManagerItemProperties::ConstructL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerItemProperties::ConstructL( const CGflmGroupItem& aItem )
    {
    switch ( aItem.Type() )
        {
        case CGflmGroupItem::EFile:
        case CGflmGroupItem::EDirectory:
            {
            const CGflmFileSystemItem& fsItem =
                static_cast< const CGflmFileSystemItem& >( aItem );
            HBufC* fullPath = fsItem.FullPathLC();
            ConstructL( *fullPath, fsItem.Entry() );
            if ( iUtils.IsRemoteDrive( *fullPath ) )
                {
                iState |= EItemRemoteDrive;
                }
            CleanupStack::PopAndDestroy( fullPath );
            break;
            }
        case CGflmGroupItem::EDrive:
            {
            const CGflmDriveItem& drvItem =
                static_cast< const CGflmDriveItem& >( aItem );
            iFullPath = drvItem.RootDirectory().AllocL();
#ifdef RD_MULTIPLE_DRIVE
            iName = iUtils.GetDriveNameL(
                drvItem.Drive(),
                CFileManagerUtils::EMainLayout );
#else // RD_MULTIPLE_DRIVE
            iName = drvItem.Name().AllocL();
#endif // RD_MULTIPLE_DRIVE
            iState |= EItemDrive;
            if ( iUtils.IsRemoteDrive( *iFullPath ) )
                {
                iState |= EItemRemoteDrive;
                }
            break;
            }
        case CGflmGroupItem::EGlobalActionItem:
            {
            const CGflmGlobalActionItem& actItem = 
                static_cast< const CGflmGlobalActionItem& >( aItem );
            if ( actItem.Id() == EFileManagerBackupAction )
                {
                // Ignore error
                iEngine.LatestBackupTime( iModified );
                }
            iName = aItem.Name().AllocL();
            // Action items do not have entry data, so mark it as fetched
            iState |= EItemAction | EItemEntryDataFetched;
            break;
            }
        default:
            {
            User::Leave( KErrNotFound );
            break;
            }
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerItemProperties::~CFileManagerItemProperties
// 
// -----------------------------------------------------------------------------
// 
CFileManagerItemProperties::~CFileManagerItemProperties()
    {
    delete iFullPath;
    delete iActiveSize;
    delete iActiveCount;
    delete iName;
    }

// -----------------------------------------------------------------------------
// CFileManagerItemProperties::Name() const
//
// -----------------------------------------------------------------------------
//
EXPORT_C TPtrC CFileManagerItemProperties::Name() const
    {
    if ( iState & EItemNotFileOrDir )
        {
        return iName->Des();
        }
    TParsePtrC parse( CFileManagerUtils::StripFinalBackslash( *iFullPath ) );
    return parse.Name();
    }

// -----------------------------------------------------------------------------
// CFileManagerItemProperties::ModifiedDate() const
//
// -----------------------------------------------------------------------------
//
EXPORT_C TTime CFileManagerItemProperties::ModifiedDate() const
    {
    EnsureEntryDataFetched();
    return iModified;
    }

// -----------------------------------------------------------------------------
// CFileManagerItemProperties::SizeL()
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt64 CFileManagerItemProperties::SizeL()
    {
    if ( iState & EItemNotFileOrDir )
        {
        return 0;
        }
    if( iSize == KErrNotFound )
        {
        if ( iState & EItemFile )
            {
            EnsureEntryDataFetched();
            }
        else
            {
            iSize = FolderSizeL( *iFullPath );
            }
        }
    return iSize;
    }

// -----------------------------------------------------------------------------
// CFileManagerItemProperties::FilesContainedL()
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CFileManagerItemProperties::FilesContainedL()
    {
    if( iFilesContained == KErrNotFound )
        {
        CountItemsL( CFileManagerItemProperties::EFile );
        }
    return iFilesContained;
    }

// -----------------------------------------------------------------------------
// CFileManagerItemProperties::FoldersContainedL()
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CFileManagerItemProperties::FoldersContainedL()
    {
    if( iFoldersContained == KErrNotFound )
        {
        CountItemsL( CFileManagerItemProperties::EFolder );
        }
    return iFoldersContained;
    }

// -----------------------------------------------------------------------------
// CFileManagerItemProperties::OpenFilesL()
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CFileManagerItemProperties::OpenFilesL()
    {
    if( iOpenFiles == KErrNotFound )
        {
        CountItemsL( CFileManagerItemProperties::EOpen );
        }
    return iOpenFiles;
    }

// -----------------------------------------------------------------------------
// CFileManagerItemProperties::TypeL()
//
// -----------------------------------------------------------------------------
//
EXPORT_C TUint32 CFileManagerItemProperties::TypeL()
    {
    if( iType == CFileManagerItemProperties::ENotDefined )
        {
        iType = iUtils.FileTypeL( *iFullPath );
        }
    return iType;
    }

// -----------------------------------------------------------------------------
// CFileManagerItemProperties::Ext() const
//
// -----------------------------------------------------------------------------
//
EXPORT_C TPtrC CFileManagerItemProperties::Ext() const
    {
    if ( iState & EItemNotFileOrDir )
        {
        return TPtrC( KNullDesC );
        }
    TParsePtrC parse( *iFullPath );
    TPtrC ext( parse.Ext() );
    return ext;
    }

// -----------------------------------------------------------------------------
// CFileManagerItemProperties::CountItemsL
// -----------------------------------------------------------------------------
//
TInt CFileManagerItemProperties::CountItemsL(
        const TFileManagerFileType& aType )
    {
    if ( !( iState & ( EItemDirectory | EItemDrive ) ) )
        {
        // It's not a folder
        return KErrNotFound;
        }

    delete iActiveCount;
    iActiveCount = NULL;

    iActiveCount = CFileManagerActiveCount::NewL(
        iEngine.Fs(), *iFullPath, aType );

    if ( !iActiveCount->IsProcessDone() )
        {
        iEngine.ShowWaitDialogL( *iActiveCount );
        }

    if ( iFilesContained == KErrNotFound )
        {
        iFilesContained = iActiveCount->FileCount();
        }

    if ( iFoldersContained == KErrNotFound )
        {
        iFoldersContained = iActiveCount->FolderCount();
        }

    if ( iOpenFiles == KErrNotFound )
        {
        iOpenFiles = iActiveCount->OpenFiles();
        }

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CFileManagerItemProperties::FolderSizeL
// -----------------------------------------------------------------------------
//
TInt64 CFileManagerItemProperties::FolderSizeL( const TDesC& aFullPath )
    {
    if ( !( iState & EItemDirectory ) )
        {
        // It's not a folder
        return KErrNotFound;
        }

    delete iActiveSize;
    iActiveSize = NULL;
    iActiveSize = CFileManagerActiveSize::NewL( iEngine.Fs(), aFullPath );
    iEngine.ShowWaitDialogL( *iActiveSize );
    if ( !iActiveSize->IsProcessDone() )
        {
        User::Leave( KErrCancel );
        }
    return iActiveSize->GetFolderSize();
    }

// -----------------------------------------------------------------------------
// CFileManagerItemProperties::FullPath() const
//
// -----------------------------------------------------------------------------
//
EXPORT_C TPtrC CFileManagerItemProperties::FullPath() const
    {
    if ( iFullPath )
        {
        return iFullPath->Des();
        }
    return TPtrC( KNullDesC );
    }

// -----------------------------------------------------------------------------
// CFileManagerItemProperties::NameAndExt() const
//
// -----------------------------------------------------------------------------
//
EXPORT_C TPtrC CFileManagerItemProperties::NameAndExt() const
    {
    if ( iState & EItemNotFileOrDir )
        {
        return iName->Des();
        }
    TParsePtrC parse( CFileManagerUtils::StripFinalBackslash( *iFullPath ) );
    return parse.NameAndExt();
    }

// -----------------------------------------------------------------------------
// CFileManagerItemProperties::LocalizedName() const
//
// -----------------------------------------------------------------------------
//
EXPORT_C TPtrC CFileManagerItemProperties::LocalizedName() const
    {
    if (iState & EItemDirectory )
        {
        TPtrC ptr( iUtils.LocalizedName( *iFullPath ) );
        if ( ptr.Length() )
            {
            return ptr;
            }
        }
    return NameAndExt();
    }

// -----------------------------------------------------------------------------
// CFileManagerItemProperties::ModifiedLocalDate() const
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CFileManagerItemProperties::ModifiedLocalDate(
        TTime& aTime ) const
    {
    aTime = ModifiedDate(); // convert from universal time
    RTz tz;
    TInt err( tz.Connect() );
    if ( err == KErrNone )
        {
        err = tz.ConvertToLocalTime( aTime );
        if ( err != KErrNone )
            {
            aTime = ModifiedDate(); // use universal time
            }
        tz.Close();
        }
    return err;
    }

// -----------------------------------------------------------------------------
// CFileManagerItemProperties::ContainsAnyFilesOrFolders()
//
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CFileManagerItemProperties::ContainsAnyFilesOrFolders()
    {
    if ( !( iState & EItemDirectory ) ||
         ( iState & EItemHasNoFilesOrFolders ) )
        {
        return EFalse;
        }
    if ( iState & EItemHasFilesOrFolders )
        {
        return ETrue;
        }
    TBool ret( CFileManagerUtils::HasAny(
        iEngine.Fs(),
        *iFullPath,
        KEntryAttNormal | KEntryAttDir | KEntryAttHidden | KEntryAttSystem ) );
    if ( ret )
        {
        iState |= EItemHasFilesOrFolders;
        }
    else
        {
        iState |= EItemHasNoFilesOrFolders;
        }
    return ret;
    }

// -----------------------------------------------------------------------------
// CFileManagerItemProperties::MimeTypeL()
//
// -----------------------------------------------------------------------------
// 
EXPORT_C TPtrC CFileManagerItemProperties::MimeTypeL()
    {
    if ( iState & EItemNotFileOrDir )
        {
        return TPtrC( KNullDesC );
        }
    return iUtils.MimeTypeL( *iFullPath );
    }

// -----------------------------------------------------------------------------
// CFileManagerItemProperties::IsDrive()
//
// -----------------------------------------------------------------------------
// 
EXPORT_C TBool CFileManagerItemProperties::IsDrive() const
    {
    if ( iState & EItemDrive )
        {
        return ETrue;
        }
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CFileManagerItemProperties::DriveName()
//
// -----------------------------------------------------------------------------
// 
EXPORT_C TPtrC CFileManagerItemProperties::DriveName() const
    {
    if ( iState & EItemDrive )
        {
        return Name();
        }
    return iEngine.CurrentDriveName();
    }

// -----------------------------------------------------------------------------
// CFileManagerItemProperties::IsRemoteDrive()
//
// -----------------------------------------------------------------------------
// 
EXPORT_C TBool CFileManagerItemProperties::IsRemoteDrive() const
    {
    if ( iState & EItemRemoteDrive )
        {
        return ETrue;
        }
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CFileManagerItemProperties::EnsureEntryDataFetched
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerItemProperties::EnsureEntryDataFetched() const
    {
    if ( iState & EItemEntryDataFetched )
        {
        return;
        }

    iState |= EItemEntryDataFetched; // Do not try fetch more than once

    TEntry entry;
    if ( iEngine.Fs().Entry( *iFullPath, entry ) != KErrNone )
        {
        return;
        }

    SetEntryData( entry );
    }

// -----------------------------------------------------------------------------
// CFileManagerItemProperties::SetEntryData
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerItemProperties::SetEntryData( const TEntry& aEntry ) const
    {
    iState |= EItemEntryDataFetched;

    iModified = aEntry.iModified;

    if ( aEntry.IsDir() )
        {
        iState |= EItemDirectory;
        }
    else
        {
        iState |= EItemFile;
        iSize = (TUint) aEntry.iSize;
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerItemProperties::DriveId
// 
// -----------------------------------------------------------------------------
// 
EXPORT_C TInt CFileManagerItemProperties::DriveId() const
    {
    TInt ret( KErrNotFound );
    if ( iFullPath )
        {
        ret = TDriveUnit( *iFullPath );
        }
    return ret;
    }

// End of File
