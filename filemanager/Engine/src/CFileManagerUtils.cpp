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
* Description:  Util methods for File Manager
*
*/



// INCLUDE FILES
#include <AknUtils.h>
#include <StringLoader.h>
#include <CDirectoryLocalizer.h>
#include <CommonContentPolicy.h>
#include <bautils.h>
#include <caf/caf.h>
#include <DcfCommon.h>
#include <FileManagerEngine.rsg>
#ifdef RD_MULTIPLE_DRIVE
#include <driveinfo.h>
#endif // RD_MULTIPLE_DRIVE
#if defined( __KEEP_DRM_CONTENT_ON_PHONE ) && defined ( FILE_MANAGER_USE_DISTRIBUTABLECHECKER )
 #include <distributablechecker.h>
#endif // __KEEP_DRM_CONTENT_ON_PHONE && FILE_MANAGER_USE_DISTRIBUTABLECHECKER
#include "GflmUtils.h"
#include <drmutility.h>
#include "CFileManagerUtils.h"
#include "CFilemanagerMimeIconArray.h"
#include "Cfilemanagerfolderarray.h"
#include "CGflmNavigatorModel.h"
#include "CFileManagerItemProperties.h"
#include "CFileManagerCommonDefinitions.h"
#include "CGflmGroupItem.h"
#include "CGflmFileSystemItem.h"
#include "CGflmDriveItem.h"
#include "CFileManagerFeatureManager.h"

// CONSTANTS
_LIT( KFlk, "flk:" );
_LIT( KLdf, "ldf:" );

#ifdef RD_MULTIPLE_DRIVE
const TInt KDriveNameArrayLen = 2;
const TInt KDriveLetterIndex = 0;
const TInt KDriveNameIndex = 1;
enum TFmgrDriveNameType
    {
    EFmgrDeviceMemory = 0,
    EFmgrMassStorage,
    EFmgrNamedMemoryCard,
    EFmgrDefaultMemoryCard,
    EFmgrDefaultUsbMemory,
    EFmgrDriveNameTypeCount // Must be the last
    };

typedef TInt TFmgrDriveNameLayoutText[ EFmgrDriveNameTypeCount ];
const TFmgrDriveNameLayoutText KFmgrDriveNames[] =
{
    { // CFileManagerUtils::EMainLayout
    R_QTN_FMGR_MAIN_DEVICE_MEMORY,      // EFmgrDeviceMemory
    R_QTN_FMGR_MAIN_MASS_STORAGE,       // EFmgrMassStorage
    R_QTN_FMGR_MAIN_NAMED_MEMORY_CARD,  // EFmgrNamedMemoryCard
    R_QTN_FMGR_MAIN_MEMORY_CARD_DEFAULT, // EFmgrDefaultMemoryCard
    R_QTN_FMGR_MAIN_USB_DEFAULT          // EFmgrDefaultUsbMemory
    },
    { // CFileManagerUtils::ETitleLayout
    R_QTN_FMGR_TITLE_DEVICE_MEMORY,      // EFmgrDeviceMemory
    R_QTN_FMGR_TITLE_MASS_STORAGE,       // EFmgrMassStorage
    R_QTN_FMGR_TITLE_NAMED_MEMORY_CARD,  // EFmgrNamedMemoryCard
    R_QTN_FMGR_TITLE_MEMORY_CARD_DEFAULT, // EFmgrDefaultMemoryCard
    R_QTN_FMGR_TITLE_USB_DEFAULT,         // EFmgrDefaultUsbMemory
    }
};
#endif // RD_MULTIPLE_DRIVE


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CFileManagerUtils::CFileManagerUtils
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CFileManagerUtils::CFileManagerUtils(
        RFs& aFs,
        CGflmNavigatorModel& aNavigator,
        CFileManagerFeatureManager& aFeatureManager ) :
    iFs( aFs ),
    iNavigator( aNavigator ),
    iFeatureManager( aFeatureManager )
    {
    }

// -----------------------------------------------------------------------------
// CFileManagerUtils::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CFileManagerUtils::ConstructL()
    {
    // Check and set value to indicate if the feature is supported
    iDrmFullSupported = iFeatureManager.IsDrmFullSupported();

    iMimeIconMapper = CFileManagerMimeIconArray::NewL(
        R_FILEMANAGER_MIME_ICON_MAPPER, iNavigator );

#ifdef RD_MULTIPLE_DRIVE
    iDefaultFolders = CFileManagerFolderArray::NewL( *this );
#else // RD_MULTIPLE_DRIVE
    iDefaultFolders = CFileManagerFolderArray::NewL(
        R_FILEMANAGER_DEFAULT_FOLDERS, *this );
    iDefaultFolders->CreateFolders();
#endif // RD_MULTIPLE_DRIVE

    }

// -----------------------------------------------------------------------------
// CFileManagerUtils::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CFileManagerUtils* CFileManagerUtils::NewL(
        RFs& aFs,
        CGflmNavigatorModel& aNavigator,
        CFileManagerFeatureManager& aFeatureManager )
    {
    CFileManagerUtils* self =
        new( ELeave ) CFileManagerUtils( aFs, aNavigator, aFeatureManager );

    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    return self;
    }

// -----------------------------------------------------------------------------
// CFileManagerUtils::~CFileManagerUtils
//
// -----------------------------------------------------------------------------
//
CFileManagerUtils::~CFileManagerUtils()
    {
#if defined( __KEEP_DRM_CONTENT_ON_PHONE ) && defined ( FILE_MANAGER_USE_DISTRIBUTABLECHECKER )
    delete iDistributableChecker;
#endif // __KEEP_DRM_CONTENT_ON_PHONE && FILE_MANAGER_USE_DISTRIBUTABLECHECKER
    delete iMimeIconMapper;
    delete iDefaultFolders;
    delete iCommonContentPolicy;
    }
    
// -----------------------------------------------------------------------------
// CFileManagerUtils::Fs() const
//
// -----------------------------------------------------------------------------
//
RFs& CFileManagerUtils::Fs() const
    {
    return iFs;
    }

// -----------------------------------------------------------------------------
// CFileManagerUtils::IsForwardLockedFile
// -----------------------------------------------------------------------------
//
TBool CFileManagerUtils::IsForwardLockedFile( const TDesC& aFullPath )
    {
    if ( iDrmFullSupported )
        {
        return EFalse;  // Full DRM is used
        }
    TBool ret( EFalse );

    if ( !iCommonContentPolicy )
        {
        TRAPD( err, iCommonContentPolicy = CCommonContentPolicy::NewL() );
        if ( err != KErrNone )
            {
            _LIT( KReason, "No Common Content Policy" );
            User::Panic( KReason, err );
            }
        }

    if( iCommonContentPolicy )
        {
        TRAPD( err, ret = iCommonContentPolicy->IsClosedFileL( aFullPath ) );
        if ( err != KErrNone )
            {
            ret = EFalse;
            }
        }

    return ret;
    }

// -----------------------------------------------------------------------------
// CFileManagerUtils::IsDrmProtectedFile
// -----------------------------------------------------------------------------
//
TBool CFileManagerUtils::IsDrmProtectedFile( const TDesC& aFullPath ) const
    {
    if ( !iDrmFullSupported )
        {
        return EFalse; // Partial DRM is used
        }
#if 0
    // Check from file extension, this is the fastest possible check
    _LIT( KOdfExt, ".odf" );
    _LIT( KDcfExt, ".dcf" );
    TParsePtrC parse( aFullPath );
    TPtrC ext( parse.Ext() );
    return ( !ext.CompareF( KOdfExt ) || !ext.CompareF( KDcfExt ) );
#else
    // Do similar quick check than in old DRMCommon
   
    RFile64 drmFile;
    
    User::LeaveIfError( drmFile.Open( 
        iFs, aFullPath, EFileRead | EFileShareReadersOrWriters ) );
    CleanupClosePushL( drmFile );
    DRM::CDrmUtility *drmCheck = DRM::CDrmUtility::NewLC();
       
    TBool isProtected( EFalse );        
    TRAP_IGNORE( isProtected = drmCheck->IsProtectedL( drmFile ) ); 
    
    CleanupStack::PopAndDestroy( drmCheck );
    CleanupStack::PopAndDestroy( &drmFile );
       
    return isProtected;
#endif
    }

// -----------------------------------------------------------------------------
// CFileManagerUtils::ResolveIconL
//
// -----------------------------------------------------------------------------
//
TInt CFileManagerUtils::ResolveIconL( const TDesC& aFullPath ) const
    {
    TInt iconId( EFileManagerOtherFileIcon );
    if ( HasFinalBackslash( aFullPath ) )
        {
        // Do not check for subfolders for remote drives
        iconId = EFileManagerFolderIcon;
        if ( !IsRemoteDrive( aFullPath ) )
            {
            if ( HasSubFolders( aFullPath ) )
                {
                iconId = EFileManagerFolderSubIcon;
                }
            else if ( !HasFiles( aFullPath ) )
                {
                iconId = EFileManagerFolderEmptyIcon;
                }
            }
        }
    else
        {
        iconId = iMimeIconMapper->ResolveIconL( aFullPath );
        }
    return iconId;
    }

// -----------------------------------------------------------------------------
// CFileManagerUtils::DefaultFolder
//
// -----------------------------------------------------------------------------
//
TBool CFileManagerUtils::DefaultFolder( const TDesC& aFullPath ) const
    {
    return iDefaultFolders->Contains( aFullPath );
    }

// -----------------------------------------------------------------------------
// CFileManagerUtils::IsDrmLocalDataFile
//
// -----------------------------------------------------------------------------
//
TBool CFileManagerUtils::IsDrmLocalDataFile( const TDesC& aFullPath ) const
    {
    if ( !iDrmFullSupported )
        {
        return EFalse; // Partial DRM is used
        }

    TBool isFlk( EFalse );
    TBool isLdf( EFalse );
    TRAP_IGNORE( IsFlkOrLdfFileL( aFullPath, isFlk, isLdf ) );
    return isLdf;
    }

// -----------------------------------------------------------------------------
// CFileManagerUtils::CanDelete
//
// -----------------------------------------------------------------------------
//
TBool CFileManagerUtils::CanDelete( const TDesC& aFileName ) const
    {
    TEntry entry;
    TBool ret( EFalse );
    TBool fileOpen( EFalse );
    if ( KErrNone == iFs.Entry( aFileName, entry ) &&
        KErrNone == iFs.IsFileOpen( aFileName, fileOpen ) )
        {
        if ( !entry.IsReadOnly() && !entry.IsSystem() && !fileOpen )
            {
            ret = ETrue;
            }
        }
    return ret;
    }

// -----------------------------------------------------------------------------
// CFileManagerUtils::FileTypeL
//
// -----------------------------------------------------------------------------
//
TUint32 CFileManagerUtils::FileTypeL( const TDesC& aFullPath )
    {
    TUint32 fileType( CFileManagerItemProperties::ENotDefined );

    _LIT( KRootFolder, "?:\\" );
    if ( !aFullPath.MatchF( KRootFolder ) )
        {
        // RFs::Entry cannot be used for root folders
        fileType |= CFileManagerItemProperties::EFolder;
        }
    else if ( !IsRemoteDrive( aFullPath ) )
        {
        // Full check for local and removable drives
        TEntry entry;
        User::LeaveIfError( iFs.Entry( aFullPath, entry ) );

        if ( HasFinalBackslash( aFullPath ) || entry.IsDir() )
            {
            fileType |= CFileManagerItemProperties::EFolder;

            if ( DefaultFolder( aFullPath ) )
                {
                fileType |= CFileManagerItemProperties::EDefault;
                }
            }
        else
            {
            fileType |= CFileManagerItemProperties::EFile;

            if ( IsDrmProtectedFile( aFullPath) )
                {
                fileType |= CFileManagerItemProperties::EDrmProtected;
                if ( IsDrmLocalDataFile( aFullPath ) )
                    {
                    fileType |= CFileManagerItemProperties::EDrmLocalDataFile;
                    }
                }
            if ( IsForwardLockedFile( aFullPath ) )
                {
                fileType |= CFileManagerItemProperties::EForwardLocked;
                }
            // MimeIcon mapper knowledge is used to determine file type
            TInt id( iMimeIconMapper->ResolveIconL( aFullPath ) );
            if ( id == EFileManagerLinkFileIcon )
                {
                fileType |= CFileManagerItemProperties::ELink;
                }
            if ( id == EFileManagerPlaylistFileIcon )
                {
                fileType |= CFileManagerItemProperties::EPlaylist;
                }

            TBool fileOpen( EFalse );
            if ( iFs.IsFileOpen( aFullPath, fileOpen ) == KErrNone )
                {
                if ( fileOpen )
                    {
                    fileType |= CFileManagerItemProperties::EOpen;
                    }
                }
            if ( fileOpen || entry.IsReadOnly() || entry.IsSystem() )
                {
                fileType |= CFileManagerItemProperties::EReadOnly;
                }
            }
        }
    else
        {
        // Simple check for remote drives
        if ( HasFinalBackslash( aFullPath ) )
            {
            fileType |= CFileManagerItemProperties::EFolder;
            }
        else
            {
            fileType |= CFileManagerItemProperties::EFile;
            if ( IsDrmProtectedFile( aFullPath) )
                {
                fileType |= CFileManagerItemProperties::EDrmProtected;
                }
            }
        }
        
    return fileType;
    }

// -----------------------------------------------------------------------------
// CFileManagerUtils::ResolveIconL
//
// -----------------------------------------------------------------------------
//
TInt CFileManagerUtils::ResolveIconL( CGflmGroupItem& aItem ) const
    {
    TInt iconId( EFileManagerOtherFileIcon );
    if ( aItem.GetIconId( iconId ) == KErrNone )
        {
        // Use already mapped icon
        return iconId;
        }

    // Map icon
    switch ( aItem.Type() )
        {
        case CGflmGroupItem::EFile:
            {
            CGflmFileSystemItem& fsItem =
                static_cast< CGflmFileSystemItem& >( aItem );
            HBufC* fullPath = fsItem.FullPathLC();
            iconId = iMimeIconMapper->ResolveIconL( *fullPath );
            fsItem.SetIconId( iconId );
            CleanupStack::PopAndDestroy( fullPath );
            break;
            }
        case CGflmGroupItem::EDirectory:
            {
            CGflmFileSystemItem& fsItem =
                static_cast< CGflmFileSystemItem& >( aItem );
            HBufC* fullPath = fsItem.FullPathLC();

            // Do not check for subfolders for remote drives
            iconId = EFileManagerFolderIcon;
            if ( !IsRemoteDrive( *fullPath ) )
                {
                if ( HasSubFolders( *fullPath ) )
                    {
                    iconId = EFileManagerFolderSubIcon;
                    }
                else if ( !HasFiles( *fullPath ) )
                    {
                    iconId = EFileManagerFolderEmptyIcon;
                    }
                }
            fsItem.SetIconId( iconId );
            CleanupStack::PopAndDestroy( fullPath );
            break;
            }
        case CGflmGroupItem::EDrive: // Fall through
        case CGflmGroupItem::EGlobalActionItem:
            {
            break;
            }
        default:
            {
            User::Leave( KErrNotFound );
            break;
            }
        }

    return iconId;
    }

// -----------------------------------------------------------------------------
// CFileManagerUtils::HasAny
// 
// -----------------------------------------------------------------------------
//
TBool CFileManagerUtils::HasAny(
        RFs& aFs, const TDesC& aFullPath,  TUint aAttMask )
    {
    RDir dir;
    TBool ret( EFalse );
    if ( dir.Open( aFs, aFullPath, aAttMask ) != KErrNone )
        {
        return EFalse;
        }
    TEntry entry;
    if ( dir.Read( entry ) == KErrNone )
        {
        ret = ETrue;
        }
    dir.Close();
    return ret;
    }

// -----------------------------------------------------------------------------
// CFileManagerUtils::HasSubFolders
// 
// -----------------------------------------------------------------------------
//
TBool CFileManagerUtils::HasSubFolders( const TDesC& aFullPath ) const
    {
    return HasAny( iFs, aFullPath, KEntryAttDir | KEntryAttMatchExclusive );
    }

// -----------------------------------------------------------------------------
// CFileManagerUtils::HasFiles
// 
// -----------------------------------------------------------------------------
//
TBool CFileManagerUtils::HasFiles( const TDesC& aFullPath ) const
    {
    return HasAny( iFs, aFullPath, KEntryAttNormal );
    }

// -----------------------------------------------------------------------------
// CFileManagerUtils::IsRemoteDrive
// 
// -----------------------------------------------------------------------------
//
TBool CFileManagerUtils::IsRemoteDrive( const TDesC& aPath ) const
    {
    TBool ret( EFalse );
    
    if ( aPath.Length() )
        {
        CGflmDriveItem* drvItem = iNavigator.DriveFromPath( aPath );
        if ( drvItem )
            {
            const TVolumeInfo& vol( drvItem->VolumeInfo() );
            if ( vol.iDrive.iDriveAtt & KDriveAttRemote )
                {
                ret = ETrue;
                }
            }
        }
    return ret;
    }

// -----------------------------------------------------------------------------
// CFileManagerUtils::CreateDefaultFolders
// 
// -----------------------------------------------------------------------------
//
void CFileManagerUtils::CreateDefaultFolders(
        const TInt aDrive, const TBool aForced ) const
    {
    iDefaultFolders->CreateFolders( aDrive, aForced );
    }

// -----------------------------------------------------------------------------
// CFileManagerUtils::IsDistributableFile
// 
// -----------------------------------------------------------------------------
//
TInt CFileManagerUtils::IsDistributableFile(
        const TDesC& aFullPath, TBool& aIsProtected )
    {
#if defined( __KEEP_DRM_CONTENT_ON_PHONE ) && defined ( FILE_MANAGER_USE_DISTRIBUTABLECHECKER )
    if ( !iDistributableChecker )
        {
        TRAPD( err, iDistributableChecker = CDistributableChecker::NewL() );        
        if ( err != KErrNone )
            {
            _LIT( KReason, "No Distributablechecker" );
            User::Panic( KReason, err );
            }
        }
    return iDistributableChecker->Check( aFullPath, aIsProtected );
#else // __KEEP_DRM_CONTENT_ON_PHONE && FILE_MANAGER_USE_DISTRIBUTABLECHECKER
    aIsProtected = EFalse;
    TBool isFlk( EFalse );
    TBool isLdf( EFalse );
    TRAPD( err, IsFlkOrLdfFileL( aFullPath, isFlk, isLdf ) );
    // Do similar check than in old DRMCommon
    aIsProtected = ( isFlk || isLdf );
    return err;
#endif // __KEEP_DRM_CONTENT_ON_PHONE && FILE_MANAGER_USE_DISTRIBUTABLECHECKER
    }

// -----------------------------------------------------------------------------
// CFileManagerUtils::MimeTypeL
// 
// -----------------------------------------------------------------------------
//
TPtrC CFileManagerUtils::MimeTypeL( const TDesC& aFullPath ) const
    {
    return iNavigator.ResolveMimeTypeL( aFullPath );
    }

// -----------------------------------------------------------------------------
// CFileManagerUtils::DriveList
// 
// -----------------------------------------------------------------------------
//
void CFileManagerUtils::DriveList(
        RArray< TInt >& aDrives, const TInt aAttMask ) const
    {
    aDrives.Reset();
    TDriveList drivesList;
    if ( iFs.DriveList( drivesList ) != KErrNone )
        {
        return;
        }
    for ( TInt i( 0 ); i < KMaxDrives; i++ )
        {
        if ( drivesList[ i ] )
            {
            TBool append( EFalse );
            TDriveInfo drvInfo;
            if ( iFs.Drive( drvInfo, i ) == KErrNone )
                {
                if ( drvInfo.iDriveAtt & aAttMask )
                    {
                    append = ETrue;
                    }
                }
            if ( append )
                {
                aDrives.Append( i );
                }
            }
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerUtils::IsRemovableDrive
// 
// -----------------------------------------------------------------------------
//
TBool CFileManagerUtils::IsRemovableDrive( const TDesC& aPath ) const
    {
    return IsDrive( iFs, aPath, KDriveAttRemovable );
    }

// -----------------------------------------------------------------------------
// CFileManagerUtils::GetFullPath
// -----------------------------------------------------------------------------
//
void CFileManagerUtils::GetFullPath(
        const TDesC& aDir, const TEntry& aEntry, TDes& aFullPath )
    {
    GflmUtils::GetFullPath( aDir, aEntry, aFullPath );
    }

// -----------------------------------------------------------------------------
// CFileManagerUtils::StripFinalBackslash
// -----------------------------------------------------------------------------
//
EXPORT_C TPtrC CFileManagerUtils::StripFinalBackslash( const TDesC& aPath )
    {
    return GflmUtils::StripFinalBackslash( aPath );
    }

// -----------------------------------------------------------------------------
// CFileManagerUtils::EnsureFinalBackslash
// -----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerUtils::EnsureFinalBackslash( TDes& aPath )
    {
    GflmUtils::EnsureFinalBackslash( aPath );
    }

// -----------------------------------------------------------------------------
// CFileManagerUtils::StripDirectionalCharsLC
// -----------------------------------------------------------------------------
//
HBufC* CFileManagerUtils::StripDirectionalCharsLC( const TDesC& aText )
    {
    HBufC* text = aText.AllocLC();
    TPtr ptr( text->Des() );
    AknTextUtils::StripCharacters( ptr, KFmgrDirectionalChars );
    return text;
    }

// -----------------------------------------------------------------------------
// CFileManagerUtils::IsFlkOrLdfFileL
// -----------------------------------------------------------------------------
//
void CFileManagerUtils::IsFlkOrLdfFileL(
        const TDesC& aFullPath, TBool& aIsFlk, TBool& aIsLdf )
    {
    aIsFlk = EFalse;
    aIsLdf = EFalse;
    ContentAccess::CContent* content =
        ContentAccess::CContent::CContent::NewLC(
            aFullPath, EContentShareReadOnly );
    HBufC* contentId = HBufC::NewLC( KMaxFileName );
    TPtr ptrContentId( contentId->Des() );
    TInt err( content->GetStringAttribute(
        ContentAccess::EContentID, ptrContentId ) );
    if ( err != KErrNone && err != KErrCANotSupported )
        {
        User::Leave( err );
        }
    if ( !ptrContentId.Left( KFlk().Length() ).Compare( KFlk ) )
        {
        aIsFlk = ETrue;
        }
    else if ( !ptrContentId.Left( KLdf().Length() ).Compare( KLdf ) )
        {
        aIsLdf = ETrue;
        }
    CleanupStack::PopAndDestroy( contentId );
    CleanupStack::PopAndDestroy( content );
    }

// -----------------------------------------------------------------------------
// CFileManagerUtils::HasFinalBackslash
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CFileManagerUtils::HasFinalBackslash( const TDesC& aPath )
    {
    return GflmUtils::HasFinalBackslash( aPath );
    }

// -----------------------------------------------------------------------------
// CFileManagerUtils::IsInternalDrive
// -----------------------------------------------------------------------------
//
TBool CFileManagerUtils::IsInternalDrive( RFs& aFs, const TInt aDrive )
    {
    return IsDrive( aFs, aDrive, KDriveAttInternal );
    }

// -----------------------------------------------------------------------------
// CFileManagerUtils::IsFromInternalToRemovableDrive
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CFileManagerUtils::IsFromInternalToRemovableDrive(
        RFs& aFs, const TDesC& aSrcPath, const TDesC& aDstPath )
    {
    TInt srcDrv = TDriveUnit( aSrcPath );
    TInt dstDrv = TDriveUnit( aDstPath );
    if ( srcDrv == dstDrv )
        {
        return EFalse;
        }
    return ( IsInternalDrive( aFs, srcDrv ) &&
             !IsInternalDrive( aFs, dstDrv ) );
    }

// -----------------------------------------------------------------------------
// CFileManagerUtils::RemoveReadOnlyAttribute
// -----------------------------------------------------------------------------
//
TInt CFileManagerUtils::RemoveReadOnlyAttribute(
        RFs& aFs, const TDesC& aFullPath, const TEntry& aEntry )
    {
    TInt ret( KErrNone );
    if( aEntry.IsReadOnly() )
        {
        ret = aFs.SetEntry(
        aFullPath, aEntry.iModified, 0, KEntryAttReadOnly );
        }
    return ret;
    }

// -----------------------------------------------------------------------------
// CFileManagerUtils::RemoveReadOnlyAttribute
// -----------------------------------------------------------------------------
//
TInt CFileManagerUtils::RemoveReadOnlyAttribute(
        RFs& aFs, const TDesC& aFullPath )
    {
    TEntry entry;
    TInt ret( aFs.Entry( aFullPath, entry ) );
    if ( ret == KErrNone )
        {
        ret = RemoveReadOnlyAttribute( aFs, aFullPath, entry );
        }
    return ret;
    }

#ifdef RD_MULTIPLE_DRIVE
// -----------------------------------------------------------------------------
// CFileManagerUtils::GetDriveNameArrayLC
// -----------------------------------------------------------------------------
//
CDesCArray* CFileManagerUtils::GetDriveNameArrayLC(
        const TInt aDrive, TUint& aDriveStatus ) const
    {
    CGflmDriveItem* drvItem = iNavigator.DriveFromId( aDrive );
    if ( !drvItem )
        {
        User::Leave( KErrNotFound );
        }
    aDriveStatus = drvItem->DriveStatus();
    CDesCArray* ret = new( ELeave ) CDesCArrayFlat( KDriveNameArrayLen );
    CleanupStack::PushL( ret );
    ret->AppendL( TDriveUnit( aDrive ).Name() );

    if ( aDriveStatus & DriveInfo::EDriveInternal )
        {
        // Device memory and mass storage drives
        return ret;
        }
    // Other drives
    TPtrC drvName( drvItem->Name() );
    if ( drvName.Length() )
        {
        ret->AppendL( drvName );
        }    
    return ret;
    }

// -----------------------------------------------------------------------------
// CFileManagerUtils::GetDriveNameL
// -----------------------------------------------------------------------------
//
HBufC* CFileManagerUtils::GetDriveNameL(
        const TInt aDrive, const TDriveNameLayout aLayout ) const
    {
    if ( aLayout < 0 || aLayout >= ELayoutCount )
        {
        User::Leave( KErrArgument );
        }
    TUint driveStatus( 0 );
    CDesCArray* array = GetDriveNameArrayLC( aDrive, driveStatus );
    HBufC* ret = NULL;
    if ( driveStatus & DriveInfo::EDriveInternal )
        {
        // Device memory and mass storage drives
        if ( driveStatus & DriveInfo::EDriveExternallyMountable )
            {
            ret = StringLoader::LoadL(
                KFmgrDriveNames[ aLayout ][ EFmgrMassStorage ],
                array->MdcaPoint( KDriveLetterIndex ) );
            }
        else
            {
            ret = StringLoader::LoadL(
                KFmgrDriveNames[ aLayout ][ EFmgrDeviceMemory ],
                array->MdcaPoint( KDriveLetterIndex ) );
            }
        }
    else if ( driveStatus & DriveInfo::EDriveRemovable )
        {
        // Physically removable drives
        if ( array->MdcaCount() > 1 )
            {
            ret = StringLoader::LoadL(
                KFmgrDriveNames[ aLayout ][ EFmgrNamedMemoryCard ],
                *array );
            }
        else
            {
            TInt driveType( EFmgrDefaultMemoryCard );
            if ( driveStatus & DriveInfo::EDriveUsbMemory )
                {
                driveType = EFmgrDefaultUsbMemory;
                }
            ret = StringLoader::LoadL(
                    KFmgrDriveNames[ aLayout ][ driveType ],
                array->MdcaPoint( KDriveLetterIndex ) );
            }
        }
    else if ( array->MdcaCount() > 1 )
        {
        // Other drives
        ret = array->MdcaPoint( KDriveNameIndex ).AllocL();
        }
    else
        {
        User::Leave( KErrNotFound );
        }
    CleanupStack::PopAndDestroy( array );
    return ret;
    }

// -----------------------------------------------------------------------------
// CFileManagerUtils::GetFormattedDriveNameLC
// -----------------------------------------------------------------------------
//
HBufC* CFileManagerUtils::GetFormattedDriveNameLC(
        const TInt aDrive,
        const TInt aTextIdForDefaultName,
        const TInt aTextIdForName ) const
    {
    TUint driveStatus( 0 );
    HBufC* ret = NULL;
    CDesCArray* array = GetDriveNameArrayLC( aDrive, driveStatus );
    if ( aTextIdForName && array->MdcaCount() > 1 )
        {
        ret = StringLoader::LoadL(
            aTextIdForName,
            *array );
        }
    else if ( aTextIdForDefaultName )
        {
			if( driveStatus & DriveInfo::EDriveUsbMemory )
				{
				CDesCArray* usbLetterName = new ( ELeave ) CDesCArrayFlat( KDriveNameArrayLen );
				CleanupStack::PushL( usbLetterName );
				
				TPtrC16 driveletter=array->MdcaPoint( KDriveLetterIndex );
				usbLetterName->AppendL( driveletter );
				
				if(array->MdcaCount() > 1 )
					{
					TPtrC16 drivename=array->MdcaPoint( KDriveNameIndex );
					usbLetterName->AppendL( drivename );
					}
				else
					{
					TPtrC16 drivename=_L("");
					usbLetterName->AppendL( drivename );
					}
				
				
				ret = StringLoader::LoadL(
				aTextIdForDefaultName,*usbLetterName );
	
				CleanupStack::PopAndDestroy( usbLetterName );
			}
		else
			{
			ret = StringLoader::LoadL(aTextIdForDefaultName, array->MdcaPoint(
					KDriveLetterIndex));
            	}
        }
    else if (  array->MdcaCount() > 1 )
        {
        ret = array->MdcaPoint( KDriveNameIndex ).AllocL();
        }
    else
        {
        User::Leave( KErrNotFound );
        }
    CleanupStack::PopAndDestroy( array );
    CleanupStack::PushL( ret );
    return ret;
    }

#endif // RD_MULTIPLE_DRIVE

// -----------------------------------------------------------------------------
// CFileManagerUtils::IsRemoteDrive
// -----------------------------------------------------------------------------
EXPORT_C TBool CFileManagerUtils::IsRemoteDrive(
        RFs& aFs, const TDesC& aPath )
    {
    return IsDrive( aFs, aPath, KDriveAttRemote );
    }

// -----------------------------------------------------------------------------
// CFileManagerUtils::IsDrive
// -----------------------------------------------------------------------------
TBool CFileManagerUtils::IsDrive(
        RFs& aFs, const TDesC& aPath, const TUint aAttMask )
    {
    TInt drv( 0 );
    if ( aPath.Length() &&
        RFs::CharToDrive( aPath[ 0 ], drv ) == KErrNone )
        {
        return IsDrive( aFs, drv, aAttMask );
        }
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CFileManagerUtils::IsDrive
// -----------------------------------------------------------------------------
TBool CFileManagerUtils::IsDrive(
        RFs& aFs, const TInt aDrive, const TUint aAttMask )
    {
    TDriveInfo drvInfo;
    if ( aFs.Drive( drvInfo, aDrive ) == KErrNone )
        {
        if ( drvInfo.iDriveAtt & aAttMask )
            {
            return ETrue;
            }
        }
    return EFalse;
    }

// ------------------------------------------------------------------------------
// CFileManagerUtils::LocalizedName
//
// ------------------------------------------------------------------------------
//
TPtrC CFileManagerUtils::LocalizedName( const TDesC& aFullPath ) const
    {
    return iNavigator.LocalizedName( aFullPath );
    }

// -----------------------------------------------------------------------------
// CFileManagerUtils::IsNameFoundL
// -----------------------------------------------------------------------------
//
TBool CFileManagerUtils::IsNameFoundL( const TDesC& aFullPath ) const
    {
    TParsePtrC parse( CFileManagerUtils::StripFinalBackslash( aFullPath ) );
    TPtrC baseDir( parse.DriveAndPath() );
    TPtrC currentDir( iNavigator.CurrentDirectory() );
    TBool ret( EFalse );
    HBufC* name1 = CFileManagerUtils::StripDirectionalCharsLC( parse.NameAndExt() );
    // Remove white spaces from end, file server also ignores those
    name1->Des().TrimRight();
    if ( !parse.PathPresent() ||
         ( currentDir.Length() && !currentDir.CompareF( baseDir ) ) )
        {
        TBool searchmode  = iNavigator.SearchMode();
        iNavigator.EnableSearchMode( EFalse );
        // Do check from current directory, all items are cached
        TInt count( iNavigator.MdcaCount() );
        for( TInt i( 0 ); i < count; ++i )
            {
            CGflmGroupItem* item = iNavigator.Item( i );
            TInt itemType( item->Type() );
            switch (itemType )
                {
                case CGflmGroupItem::EFile: // FALLTHROUGH
                case CGflmGroupItem::EDirectory:
                    {
                    // Compare with file system name
                    CGflmFileSystemItem* fsItem =
                        static_cast< CGflmFileSystemItem* >( item );
                    HBufC* name2 = CFileManagerUtils::StripDirectionalCharsLC(
                        fsItem->Entry().iName );
                    ret = !( name1->CompareF( *name2 ) );
                    CleanupStack::PopAndDestroy( name2 );
                    if ( !ret && itemType == CGflmGroupItem::EDirectory )
                        {
                        // Compare with localized name
                        name2 = CFileManagerUtils::StripDirectionalCharsLC(
                            fsItem->Name() );
                        if ( name2->Length() )
                            {
                            ret = !( name1->CompareF( *name2 ) );
                            }
                        CleanupStack::PopAndDestroy( name2 );
                        }
                    break;
                    }                    
                default:
                    {
                    break;
                    }
                }
            if ( ret )
                {
                break;
                }
            }
        iNavigator.EnableSearchMode( searchmode );
        }
    else
        {
        // Do check from any directory
        HBufC* buffer = HBufC::NewLC(
            Max( KMaxFileName, aFullPath.Length() + KFmgrBackslashSpace ) );
        TPtr ptr( buffer->Des() );
        ptr.Copy( aFullPath );
        CFileManagerUtils::EnsureFinalBackslash( ptr );
        ret = ( BaflUtils::FolderExists( iFs, ptr ) ||
                BaflUtils::FileExists( iFs,
                    CFileManagerUtils::StripFinalBackslash( aFullPath ) ) );
        if ( !ret &&
             !IsRemoteDrive( aFullPath ) &&
             BaflUtils::PathExists( iFs, baseDir ) )
            {
            // Check directory for localized items on local drives
            CDir* dir = NULL;
            User::LeaveIfError( iFs.GetDir(
                baseDir, KEntryAttMatchMask | KEntryAttNormal, ESortNone, dir ) );
            CleanupStack::PushL( dir );
            ptr.Zero();
            TInt count( dir->Count() );
            for ( TInt i ( 0 ); i < count; ++i )
                {
                HBufC* name2 = NULL;
                const TEntry& entry( ( *dir )[ i ] );
                if( entry.IsDir() )
                    {
                    // Compare with localized name
                    CFileManagerUtils::GetFullPath( baseDir, entry, ptr );
                    TPtrC ptr2( iNavigator.LocalizedName( ptr ) );
                    if ( ptr2.Length() )
                        {
                        name2 = CFileManagerUtils::StripDirectionalCharsLC( ptr2 );
                        }
                    }
                if ( !name2 )
                    {
                    // Compare with file system name
                    name2 = CFileManagerUtils::StripDirectionalCharsLC( entry.iName );
                    }
                ret = !( name1->CompareF( *name2 ) );
                CleanupStack::PopAndDestroy( name2 );
                if ( ret )
                    {
                    break;
                    }
                }
            CleanupStack::PopAndDestroy( dir );
            }
        CleanupStack::PopAndDestroy( buffer );
        }
    CleanupStack::PopAndDestroy( name1 );
    return ret;
    }
// -----------------------------------------------------------------------------
// CFileManagerUtils::FlushCache
// -----------------------------------------------------------------------------
//
void CFileManagerUtils::FlushCache() const
    {
    iNavigator.FlushCache();
    }

//  End of File
