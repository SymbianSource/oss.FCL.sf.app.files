/*
* Copyright (c) 2005-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Filter GFLM items
*
*/



// INCLUDE FILES
#include "CFileManagerItemFilter.h"
#include "CFileManagerEngine.h"
#include "CGflmGroupItem.h"
#include "CGflmFileSystemItem.h"
#include "CGflmDriveItem.h"
#include "CGflmGlobalActionItem.h"
#include "CFileManagerCommonDefinitions.h"
#include "FileManagerDebug.h"
#include "TFileManagerDriveInfo.h"
#include "CFileManagerFeatureManager.h"
#include <FileManagerEngine.rsg>
#include <StringLoader.h>
#include <pathinfo.h>
#ifdef RD_MULTIPLE_DRIVE
#include <driveinfo.h>
#endif // RD_MULTIPLE_DRIVE


// ======== MEMBER FUNCTIONS ========

// ----------------------------------------------------------------------------
// CFileManagerItemFilter::CFileManagerItemFilter
// ----------------------------------------------------------------------------
//
CFileManagerItemFilter::CFileManagerItemFilter( CFileManagerEngine& aEngine )
    : iEngine( aEngine )
    {
    }

// ----------------------------------------------------------------------------
// CFileManagerItemFilter::ConstructL
// ----------------------------------------------------------------------------
//
void CFileManagerItemFilter::ConstructL()
    {
    // Phone memory root path
    iPhoneMemoryRootPath = PathInfo::PhoneMemoryRootPath().AllocL();
    iInternalDefaultName =
        StringLoader::LoadL( R_TEXT_PHONE_MEMORY );
    iRemovableDefaultName =
        StringLoader::LoadL( R_TEXT_MEMORY_CARD_DEFAULT );
    }

// ----------------------------------------------------------------------------
// CFileManagerItemFilter::NewL
// ----------------------------------------------------------------------------
//
CFileManagerItemFilter* CFileManagerItemFilter::NewL(
        CFileManagerEngine& aEngine )
    {
    CFileManagerItemFilter* self = new( ELeave ) CFileManagerItemFilter(
        aEngine );

    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    return self;
    }

// ----------------------------------------------------------------------------
// CFileManagerItemFilter::~CFileManagerItemFilter
// ----------------------------------------------------------------------------
// 
CFileManagerItemFilter::~CFileManagerItemFilter()
    {
    delete iPhoneMemoryRootPath;
    delete iInternalDefaultName;
    delete iRemovableDefaultName;
    }

// ----------------------------------------------------------------------------
// CFileManagerItemFilter::FilterItemL
// ----------------------------------------------------------------------------
// 
TBool CFileManagerItemFilter::FilterItemL(
        CGflmGroupItem* aItem,
        TInt /*aGroupId*/,
        CGflmDriveItem* /*aDrive*/ )
    {
    switch( aItem->Type() )
        {
        case CGflmGroupItem::EGlobalActionItem:
			{
            CGflmGlobalActionItem* actItem =
                static_cast< CGflmGlobalActionItem* >( aItem );
            switch( actItem->Id() )
                {
                case EFileManagerBackupAction:
                    {
                    // Disable backup in embedded mode, because it messes up 
                    // backup and restore operations since embedded apps are closed.
                    if ( iEngine.FeatureManager().IsEmbedded() )
                        {
                        return EFalse;
                        }
                    actItem->SetIconId( EFileManagerBackupMainIcon );
                    break;
                    }
                default:
                    {
                    break;
                    }
                }
            break;
			}
        case CGflmGroupItem::EDirectory:
            {
            CGflmFileSystemItem* fsItem =
                static_cast< CGflmFileSystemItem* >( aItem );
            fsItem->GetFullPath( iFileNameBuffer );
            return !iEngine.IsSystemFolder( iFileNameBuffer );
			}
        case CGflmGroupItem::EDrive:
            {
            CGflmDriveItem* drvItem = static_cast< CGflmDriveItem* >( aItem );

#ifndef RD_MULTIPLE_DRIVE
            const TVolumeInfo& vol( drvItem->VolumeInfo() );
            TInt drive( drvItem->Drive() );
#endif // RD_MULTIPLE_DRIVE

            INFO_LOG1( "CFileManagerItemFilter::FilterItemL()-drive=%d",
                drvItem->Drive() )

            INFO_LOG1( "CFileManagerItemFilter::FilterItemL()-driveAtt=%d",
                drvItem->VolumeInfo().iDrive.iDriveAtt )

            INFO_LOG1( "CFileManagerItemFilter::FilterItemL()-mediaAtt=%d",
                drvItem->VolumeInfo().iDrive.iMediaAtt )

            INFO_LOG1( "CFileManagerItemFilter::FilterItemL()-driveUniqueId=%u",
                drvItem->VolumeInfo().iUniqueID )

            INFO_LOG1( "CFileManagerItemFilter::FilterItemL()-driveSize=%lu",
                drvItem->VolumeInfo().iSize )

            INFO_LOG1( "CFileManagerItemFilter::FilterItemL()-VolumeStatus=%d",
                drvItem->VolumeStatus() )

            INFO_LOG1( "CFileManagerItemFilter::FilterItemL()-DriveStatus=%d",
                drvItem->DriveStatus() )

#ifdef RD_MULTIPLE_DRIVE
            // Setup drive icon
            TUint driveStatus( drvItem->DriveStatus() );
            if ( !( driveStatus & DriveInfo::EDriveUserVisible ) )
                {
                ERROR_LOG( "CFileManagerItemFilter::FilterItemL()-Hidden-1" )
                return EFalse;
                }
            TInt icon( 0 );
            if ( driveStatus & DriveInfo::EDriveInternal )
                {
                icon = EFileManagerPhoneMemoryMainIcon;
                if ( driveStatus & DriveInfo::EDriveExternallyMountable )
                    {
                    icon = EFileManagerMassStorageMainIcon;
                    }
                }
            else if ( driveStatus & DriveInfo::EDriveUsbMemory )
                {
                if ( !( driveStatus & DriveInfo::EDrivePresent ) )
                    {
                    ERROR_LOG( "CFileManagerItemFilter::FilterItemL()-NotPresentUsbMemory" )
                    return EFalse; // Not present USB drive is not shown
                    }
                icon = EFileManagerUsbMemoryMainIcon;
                }
            else if ( driveStatus & DriveInfo::EDriveRemovable )
                {
                icon = EFileManagerMemoryCardNotPresentMainIcon;
                if ( driveStatus & DriveInfo::EDrivePresent )
                    {
                    icon = EFileManagerMemoryCardPresentMainIcon;
                    }
                }
            else if ( driveStatus & DriveInfo::EDriveRemote )
                {
                icon = EFileManagerRemoteDriveMainIcon;
                }
            else
                {
                ERROR_LOG( "CFileManagerItemFilter::FilterItemL()-Hidden-2" )
                return EFalse;
                }
            drvItem->SetIconId( icon );
            // Setup root directory
            if ( PathInfo::GetRootPath( iFileNameBuffer, drvItem->Drive() ) != KErrNone )
                {
                ERROR_LOG( "CFileManagerItemFilter::FilterItemL()-Hidden-3" )
                return EFalse;
                }
            drvItem->SetRootDirectoryL( iFileNameBuffer );
            INFO_LOG( "CFileManagerItemFilter::FilterItemL()-Visible" )

#else // RD_MULTIPLE_DRIVE

            TBool isPhoneMem( EFalse );
            TPtrC root( drvItem->RootDirectory() );
            TPtrC phoneMemRoot( *iPhoneMemoryRootPath );
            TInt len( Min( root.Length(), phoneMemRoot.Length() ) );
            if ( !root.CompareF( phoneMemRoot.Left( len ) ) )
                {
                // Drive is defined to phone memory
                isPhoneMem = ETrue;
                drvItem->SetRootDirectoryL( *iPhoneMemoryRootPath );
                }
            else if ( vol.iDrive.iDriveAtt & KDriveAttInternal )
                {
                if ( drive == KFmgrRamDrive || drive == KFmgrRomDrive )
                    {
                    // Drive is not allowed internal drive
                    return EFalse;
                    }
                }
            if ( vol.iDrive.iDriveAtt & KDriveAttSubsted )
                {
                // Substed drives are not allowed
                return EFalse;
                }
            // Setup drive name
            if ( !drvItem->Name().Length() )
                {
                if ( isPhoneMem )
                    {
                    drvItem->SetLocalizedNameL( *iInternalDefaultName );
                    }
                else if ( ( vol.iDrive.iDriveAtt & KDriveAttRemovable ) ||
                        drive == KFmgrMemoryCardDrive )
                    {
                    drvItem->SetLocalizedNameL( *iRemovableDefaultName );
                    }
                else
                    {
                    drvItem->SetLocalizedNameL( drvItem->RootDirectory() );
                    }
                }
            // Setup drive icon
            if ( isPhoneMem )
                {
                drvItem->SetIconId( EFileManagerPhoneMemoryMainIcon );
                }
            else if ( ( vol.iDrive.iDriveAtt & KDriveAttRemovable ) ||
                drive == KFmgrMemoryCardDrive )
                {
                if ( vol.iDrive.iType == EMediaNotPresent )
                    {
                    drvItem->SetIconId( EFileManagerMemoryCardNotPresentMainIcon );
                    }
                else
                    {
                    drvItem->SetIconId( EFileManagerMemoryCardPresentMainIcon );
                    }
                }
            else if ( vol.iDrive.iDriveAtt & KDriveAttRemote )
                {
                drvItem->SetIconId( EFileManagerRemoteDriveMainIcon );
                }
            else
                {
                return EFalse;
                }
            INFO_LOG( "CFileManagerItemFilter::FilterItemL()-driveAllowed" )

#endif // RD_MULTIPLE_DRIVE
            break;
            }
        default:
            {
            break;
            }
        }
    return ETrue;
    }

//  End of File  
