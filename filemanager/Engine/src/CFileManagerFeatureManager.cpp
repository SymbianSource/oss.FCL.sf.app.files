/*
* Copyright (c) 2007-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  This class manages the runtime variated features
*
*/


#include <e32std.h>
#include <centralrepository.h>
#include <featmgr.h>
#include <eikenv.h>
#include "CFileManagerFeatureManager.h"
#include "FileManagerDebug.h"
#include "FileManagerPrivateCRKeys.h"

// ======== MEMBER FUNCTIONS ========

// ----------------------------------------------------------------------------
// CFileManagerFeatureManager::CFileManagerFeatureManager
// ----------------------------------------------------------------------------
// 
CFileManagerFeatureManager::CFileManagerFeatureManager()
    {
    FUNC_LOG;
    }

// ----------------------------------------------------------------------------
// CFileManagerFeatureManager::ConstructL
// ----------------------------------------------------------------------------
// 
void CFileManagerFeatureManager::ConstructL()
    {
    FUNC_LOG;

    // Get the feature manager variations
    FeatureManager::InitializeLibL();
    iDrmFullSupported = FeatureManager::FeatureSupported( KFeatureIdDrmFull );
    iHelpSupported = FeatureManager::FeatureSupported( KFeatureIdHelp );
    iIrdaSupported = FeatureManager::FeatureSupported( KFeatureIdIrda );
    iRemoteStorageFwSupported =
        FeatureManager::FeatureSupported( KFeatureIdRemoteStorageFw );
    iMmcPassWdSupported = FeatureManager::FeatureSupported( KFeatureIdMmcLock );
    iWesternVariant = !( FeatureManager::FeatureSupported( KFeatureIdChinese ) );
    iMmcSwEjectSupported =
        ( FeatureManager::FeatureSupported( KFeatureIdMmcHotswap ) &&
          FeatureManager::FeatureSupported( KFeatureIdMmcEject ) );
    FeatureManager::UnInitializeLib();

    iEmbedded = CEikonEnv::Static()->StartedAsServerApp();

    // Get the local variations
    CRepository* cenRep = CRepository::NewLC( KCRUidFileManagerSettings );
    TInt err( cenRep->Get( KFileManagerFeatures, iFileManagerFeatures ) );
    if ( err != KErrNone )
        {
        ERROR_LOG1(
            "CFileManagerFeatureManager::ConstructL()-LocalVariationError=%d",
            err );
        iFileManagerFeatures = 0;
        }
    INFO_LOG1(
        "CFileManagerFeatureManager::ConstructL()-iFileManagerFeatures=0x%x",
        iFileManagerFeatures );
    CleanupStack::PopAndDestroy( cenRep );
    }

// ----------------------------------------------------------------------------
// CFileManagerFeatureManager::NewL
// ----------------------------------------------------------------------------
// 
CFileManagerFeatureManager* CFileManagerFeatureManager::NewL()
    {
    FUNC_LOG;

    CFileManagerFeatureManager* self =
        new ( ELeave ) CFileManagerFeatureManager();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ----------------------------------------------------------------------------
// CFileManagerFeatureManager::~CFileManagerFeatureManager
// ----------------------------------------------------------------------------
// 
CFileManagerFeatureManager::~CFileManagerFeatureManager()
    {
    FUNC_LOG;
    }

// ----------------------------------------------------------------------------
// CFileManagerFeatureManager::IsDrmFullSupported
// ----------------------------------------------------------------------------
// 
EXPORT_C TBool CFileManagerFeatureManager::IsDrmFullSupported() const
    {
    return iDrmFullSupported;
    }

// ----------------------------------------------------------------------------
// CFileManagerFeatureManager::IsHelpSupported
// ----------------------------------------------------------------------------
// 
EXPORT_C TBool CFileManagerFeatureManager::IsHelpSupported() const
    {
    return iHelpSupported;
    }

// ----------------------------------------------------------------------------
// CFileManagerFeatureManager::IsIrdaSupported
// ----------------------------------------------------------------------------
// 
EXPORT_C TBool CFileManagerFeatureManager::IsIrdaSupported() const
    {
    return iIrdaSupported;
    }

// ----------------------------------------------------------------------------
// CFileManagerFeatureManager::IsRemoteStorageFwSupported
// ----------------------------------------------------------------------------
// 
EXPORT_C TBool CFileManagerFeatureManager::IsRemoteStorageFwSupported() const
    {
    return iRemoteStorageFwSupported;
    }

// ----------------------------------------------------------------------------
// CFileManagerFeatureManager::IsMmcPassWdSupported
// ----------------------------------------------------------------------------
// 
EXPORT_C TBool CFileManagerFeatureManager::IsMmcPassWdSupported() const
    {
    return iMmcPassWdSupported;
    }

// ----------------------------------------------------------------------------
// CFileManagerFeatureManager::IsWesternVariant
// ----------------------------------------------------------------------------
// 
EXPORT_C TBool CFileManagerFeatureManager::IsWesternVariant() const
    {
    return iWesternVariant;
    }
// ----------------------------------------------------------------------------
// CFileManagerFeatureManager::IsMmcSwEjectSupported
// ----------------------------------------------------------------------------
// 
EXPORT_C TBool CFileManagerFeatureManager::IsMmcSwEjectSupported() const
    {
    return iMmcSwEjectSupported;
    }

// ----------------------------------------------------------------------------
// CFileManagerFeatureManager::IsFeatureSupported
// ----------------------------------------------------------------------------
// 
EXPORT_C TBool CFileManagerFeatureManager::IsFeatureSupported(
        TInt aFileManagerFeature ) const
    {
    return ( iFileManagerFeatures & aFileManagerFeature );
    }

// ----------------------------------------------------------------------------
// CFileManagerFeatureManager::IsEmbedded
// ----------------------------------------------------------------------------
// 
EXPORT_C TBool CFileManagerFeatureManager::IsEmbedded() const
    {
    return iEmbedded;
    }
