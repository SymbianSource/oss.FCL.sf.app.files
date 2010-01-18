/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: CMMCScBkupOpParamsBase implementation
*
*
*/

#include "CMMCScBkupOperationParameters.h"

// System includes
#include <e32std.h>

// User includes
#include "MMCScBkupLogger.h"
#include "CMMCScBkupDriveAndOperationTypeManager.h"
#include "CMMCScBkupArchiveInfo.h"
#include "BkupEngine.hrh"

// ========================= MEMBER FUNCTIONS ================================

// ---------------------------------------------------------------------------
// CMMCScBkupOpParamsBase::CMMCScBkupOpParamsBase()
// 
// C++ constructor.
// ---------------------------------------------------------------------------
CMMCScBkupOpParamsBase::CMMCScBkupOpParamsBase()
    {
    }


// ---------------------------------------------------------------------------
// CMMCScBkupOpParamsBase::~CMMCScBkupOpParamsBase()
// 
// C++ destructor.
// ---------------------------------------------------------------------------
EXPORT_C CMMCScBkupOpParamsBase::~CMMCScBkupOpParamsBase()
    {
    delete iDriveAndOperations;
    iArchiveInfos.ResetAndDestroy();
    iArchiveInfos.Close();
   }


// ---------------------------------------------------------------------------
// CMMCScBkupOpParamsBase::ConstructL()
// 
// 
// ---------------------------------------------------------------------------
void CMMCScBkupOpParamsBase::ConstructL( TResourceReader& aDriveReader, TBitFlags aCategories )
    {
    iCategories = aCategories;
    iDriveAndOperations = CMMCScBkupDriveAndOperationTypeManager::NewL( aDriveReader );
    }


// ---------------------------------------------------------------------------
// CMMCScBkupOpParamsBase::DriveAndOperations()
// 
// 
// ---------------------------------------------------------------------------
EXPORT_C const CMMCScBkupDriveAndOperationTypeManager& CMMCScBkupOpParamsBase::DriveAndOperations() const
    {
    return *iDriveAndOperations;
    }

// ---------------------------------------------------------------------------
// CMMCScBkupOpParamsBase::SetArchiveInfosL()
// 
// 
// ---------------------------------------------------------------------------
EXPORT_C void CMMCScBkupOpParamsBase::SetArchiveInfosL(RPointerArray<CMMCScBkupArchiveInfo>& aInfos)
    {
    iArchiveInfos.ResetAndDestroy();

    for(TInt i = 0; i < aInfos.Count(); i++)
        {
        iArchiveInfos.AppendL(aInfos[i]);
        }
    }


// ---------------------------------------------------------------------------
// CMMCScBkupOpParamsBase::ArchiveInfos()
// 
// 
// ---------------------------------------------------------------------------
RMMCScBkupPointerArray<CMMCScBkupArchiveInfo>& CMMCScBkupOpParamsBase::ArchiveInfos()
    {
    return iArchiveInfos;
    }


// ---------------------------------------------------------------------------
// CMMCScBkupOpParamsBase::ArchiveInfo()
// 
// 
// ---------------------------------------------------------------------------
const CMMCScBkupArchiveInfo& CMMCScBkupOpParamsBase::ArchiveInfo(TBitFlags aCategory) const
    {
    CMMCScBkupArchiveInfo* archiveInfo = NULL;
    
    for(TInt i = 0; i < iArchiveInfos.Count(); i++)
        {
        archiveInfo = iArchiveInfos[i];
        if(archiveInfo->Category() == aCategory)
            {
            break;
            }
        }
    
    return *archiveInfo;
    }


// ---------------------------------------------------------------------------
// CMMCScBkupOpParamsBase::FileName()
// 
// 
// ---------------------------------------------------------------------------
const TDesC& CMMCScBkupOpParamsBase::FileName(TInt aIndex) const
    {
    const TDesC* ret = &KNullDesC;
    const CMMCScBkupArchiveInfo& archiveInfo = *iArchiveInfos[aIndex];

    if( archiveInfo.FileName().Length() )
        {
        ret = &archiveInfo.FileName();
        }

    return *ret;
    }

    
// ---------------------------------------------------------------------------
// CMMCScBkupOpParamsBackupFull::ReadFromResourceL()
// 
// Read categories with associated data from resource
// ---------------------------------------------------------------------------
void CMMCScBkupOpParamsBackupFull::ReadFromResourceL( TResourceReader& aReader )
    {
    TInt count( aReader.ReadInt16() );
    
    for ( TInt i( 0 ); i < count; i++ )
        {
        // Read category
        TBitFlags category = static_cast< TBitFlags >(aReader.ReadUint32());
        
        // Read archive name
        const TDriveUnit driveUnit(iDrive);
        const TDriveName driveName(driveUnit.Name());
        TFileName name;
        TPtrC archiveName( aReader.ReadTPtrC() );
        
        name.Append(driveName);
        name.Append(KBackUpFolder());
        name.Append(archiveName);
        
        // Read special flags
        TBitFlags flags = static_cast< TBitFlags >(aReader.ReadUint32());
        TBitFlags excludedFlags = static_cast< TBitFlags >(aReader.ReadUint32());
        
        // Read SIDs belonging to category
        TInt16 subCount = aReader.ReadInt16();
        RArray<TSecureId> sids;
        CleanupClosePushL( sids );
        
        for(TInt j = 0; j < subCount; j++)
            {
            sids.AppendL(static_cast<TSecureId>(aReader.ReadUint32()));
            }            
        
        // Read excluded SIDs for category
        subCount = aReader.ReadInt16();
        RArray<TSecureId> excludeSids;
        CleanupClosePushL( excludeSids );
        
        for(TInt j = 0; j < subCount; j++)
            {
            excludeSids.AppendL(static_cast<TSecureId>(aReader.ReadUint32()));
            }            
        
        TEntry entry;
        entry.iName = name;
        CMMCScBkupArchiveInfo* archiveInfo = CMMCScBkupArchiveInfo::NewLC( entry );
        archiveInfo->SetCategory( category );
        archiveInfo->SetSpecialFlags( flags );
        archiveInfo->SetExcludedSpecialFlags( excludedFlags );
        archiveInfo->SetSIDs( sids );
        archiveInfo->SetExcludedSIDs( excludeSids );
        
        // If the category is specified, then add it in list of categories to be archived
#ifdef RD_FILE_MANAGER_BACKUP
        if( Categories().Value() & category.Value() )
#else
        if( category.Value() == EBUCatAllInOne )
#endif
            {
            __LOG1("CMMCScBkupOpParamsBase::ReadFromResourceL() - adding category 0x%x", category.Value());
            RMMCScBkupPointerArray<CMMCScBkupArchiveInfo>& archives = ArchiveInfos();
            archives.AppendL( archiveInfo );
            CleanupStack::Pop( archiveInfo );
            }
        else
            {
            __LOG1("CMMCScBkupOpParamsBase::ReadFromResourceL() - category 0x%x not included", category.Value());
            CleanupStack::PopAndDestroy( archiveInfo );
            }
        
        CleanupStack::PopAndDestroy( &excludeSids );
        CleanupStack::PopAndDestroy( &sids );
        }
    }




















// ========================= MEMBER FUNCTIONS ================================

// ---------------------------------------------------------------------------
// CMMCScBkupOpParamsBackupFull::CMMCScBkupOpParamsBackupFull()
// 
// C++ constructor.
// ---------------------------------------------------------------------------
CMMCScBkupOpParamsBackupFull::CMMCScBkupOpParamsBackupFull( TDriveNumber aDrive )
    : iDrive( aDrive )
    {
    }


// ---------------------------------------------------------------------------
// CMMCScBkupOpParamsBackupFull::~CMMCScBkupOpParamsBackupFull()
// 
// C++ destructor.
// ---------------------------------------------------------------------------
EXPORT_C CMMCScBkupOpParamsBackupFull::~CMMCScBkupOpParamsBackupFull()
    {
    }

// ---------------------------------------------------------------------------
// CMMCScBkupOpParamsBackupFull::ConstructL()
// 
// 
// ---------------------------------------------------------------------------
void CMMCScBkupOpParamsBackupFull::ConstructL( TResourceReader& aDriveReader, TResourceReader& aCategoryReader,
    TBitFlags aCategories )
    {
    CMMCScBkupOpParamsBase::ConstructL( aDriveReader, aCategories );
    ReadFromResourceL( aCategoryReader );
    }


// ---------------------------------------------------------------------------
// CMMCScBkupOpParamsBackupFull::NewL()
// 
// 
// ---------------------------------------------------------------------------
EXPORT_C CMMCScBkupOpParamsBackupFull* CMMCScBkupOpParamsBackupFull::NewL( TResourceReader& aDriveReader, TResourceReader& aCategoryReader,
    TDriveNumber aDrive, TBitFlags aCategories )
    {
    CMMCScBkupOpParamsBackupFull* self = new(ELeave) CMMCScBkupOpParamsBackupFull( aDrive );
    CleanupStack::PushL( self );
    self->ConstructL( aDriveReader, aCategoryReader, aCategories );
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// CMMCScBkupOpParamsBackupFull::AssociatedOpType()
// 
// 
// ---------------------------------------------------------------------------
EXPORT_C TMMCScBkupOperationType CMMCScBkupOpParamsBackupFull::AssociatedOpType() const
    {
    return EMMCScBkupOperationTypeFullBackup;
    }


// ---------------------------------------------------------------------------
// CMMCScBkupOpParamsBackupFull::PartType()
// 
// 
// ---------------------------------------------------------------------------
EXPORT_C TBURPartType CMMCScBkupOpParamsBackupFull::PartType() const
    {
    return EBURBackupFull;
    }


// ---------------------------------------------------------------------------
// CMMCScBkupOpParamsBackupFull::IncrementType()
// 
// 
// ---------------------------------------------------------------------------
EXPORT_C TBackupIncType CMMCScBkupOpParamsBackupFull::IncrementType() const
    {
    return EBackupBase;
    }


// ---------------------------------------------------------------------------
// CMMCScBkupOpParamsBackupFull::PassiveTransferType()
// 
// 
// ---------------------------------------------------------------------------
EXPORT_C TTransferDataType CMMCScBkupOpParamsBackupFull::PassiveTransferType() const
    {
    return EPassiveBaseData;
    }


// ---------------------------------------------------------------------------
// CMMCScBkupOpParamsBackupFull::ActiveTransferType()
// 
// 
// ---------------------------------------------------------------------------
EXPORT_C TTransferDataType CMMCScBkupOpParamsBackupFull::ActiveTransferType() const
    {
    return EActiveBaseData;
    }


// ---------------------------------------------------------------------------
// CMMCScBkupOpParamsBackupFull::PackageTransferType()
// 
// 
// ---------------------------------------------------------------------------
EXPORT_C TPackageDataType CMMCScBkupOpParamsBackupFull::PackageTransferType() const
    {
    return ESystemData;
    }





























// ========================= MEMBER FUNCTIONS ================================

// ---------------------------------------------------------------------------
// CMMCScBkupOpParamsRestoreFull::CMMCScBkupOpParamsRestoreFull()
// 
// C++ constructor.
// ---------------------------------------------------------------------------
CMMCScBkupOpParamsRestoreFull::CMMCScBkupOpParamsRestoreFull()
    {
    }


// ---------------------------------------------------------------------------
// CMMCScBkupOpParamsRestoreFull::~CMMCScBkupOpParamsRestoreFull()
// 
// C++ destructor.
// ---------------------------------------------------------------------------
EXPORT_C CMMCScBkupOpParamsRestoreFull::~CMMCScBkupOpParamsRestoreFull()
    {
    }


// ---------------------------------------------------------------------------
// CMMCScBkupOpParamsRestoreFull::NewL()
// 
// 
// ---------------------------------------------------------------------------
EXPORT_C CMMCScBkupOpParamsRestoreFull* CMMCScBkupOpParamsRestoreFull::NewL( TResourceReader& aDriveReader, TBitFlags aCategories )
    {
    CMMCScBkupOpParamsRestoreFull* self = new(ELeave) CMMCScBkupOpParamsRestoreFull( );
    CleanupStack::PushL( self );
    self->ConstructL( aDriveReader, aCategories );
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// CMMCScBkupOpParamsRestoreFull::AssociatedOpType()
// 
// 
// ---------------------------------------------------------------------------
EXPORT_C TMMCScBkupOperationType CMMCScBkupOpParamsRestoreFull::AssociatedOpType() const
    {
    return EMMCScBkupOperationTypeFullRestore;
    }


// ---------------------------------------------------------------------------
// CMMCScBkupOpParamsRestoreFull::PartType()
// 
// 
// ---------------------------------------------------------------------------
EXPORT_C TBURPartType CMMCScBkupOpParamsRestoreFull::PartType() const
    {
    return EBURRestoreFull;
    }


// ---------------------------------------------------------------------------
// CMMCScBkupOpParamsRestoreFull::IncrementType()
// 
// 
// ---------------------------------------------------------------------------
EXPORT_C TBackupIncType CMMCScBkupOpParamsRestoreFull::IncrementType() const
    {
    return EBackupBase;
    }


// ---------------------------------------------------------------------------
// CMMCScBkupOpParamsRestoreFull::PassiveTransferType()
// 
// 
// ---------------------------------------------------------------------------
EXPORT_C TTransferDataType CMMCScBkupOpParamsRestoreFull::PassiveTransferType() const
    {
    return EPassiveBaseData;
    }


// ---------------------------------------------------------------------------
// CMMCScBkupOpParamsRestoreFull::ActiveTransferType()
// 
// 
// ---------------------------------------------------------------------------
EXPORT_C TTransferDataType CMMCScBkupOpParamsRestoreFull::ActiveTransferType() const
    {
    return EActiveBaseData;
    }


// ---------------------------------------------------------------------------
// CMMCScBkupOpParamsRestoreFull::PackageTransferType()
// 
// 
// ---------------------------------------------------------------------------
EXPORT_C TPackageDataType CMMCScBkupOpParamsRestoreFull::PackageTransferType() const
    {
    return ESystemData;
    }


