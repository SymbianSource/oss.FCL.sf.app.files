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
* Description: Declaration for CMMCScBkupOpParamsBase
*     
*
*/

#ifndef __CMMCSCBKUPOPERATIONPARAMETERS_H__
#define __CMMCSCBKUPOPERATIONPARAMETERS_H__

// System includes
#include <connect/sbdefs.h>
#include <connect/sbtypes.h>
#include <barsread.h>
#include <babitflags.h>

// User includes
#include "MMCScBkupOperations.h"
#include "CMMCScBkupDataOwnerInfo.h"
#include "RMMCScBkupPointerArray.h"

// Namespaces
using namespace conn;

// Classes referenced
class CMMCScBkupDriveAndOperationTypeManager;
class CMMCScBkupArchiveInfo;

_LIT( KBackUpFolder, "\\Backup\\" );
_LIT( KBackUpFiles, "*.arc" );

/**
*
*
* @since 3.0
*/
NONSHARABLE_CLASS(CMMCScBkupOpParamsBase) : public CBase
    {
    public:

        /**
        *
        */
        IMPORT_C ~CMMCScBkupOpParamsBase();

    protected:

        /**
        *
        */
        CMMCScBkupOpParamsBase( );

        /**
        *
        */
        void ConstructL( TResourceReader& aDriveReader, TBitFlags aCategories );

    public: // Common mandatory framework methods

        /**
        *
        */
        virtual TMMCScBkupOperationType AssociatedOpType() const = 0;

        /**
        *
        */
        virtual TBURPartType PartType() const = 0;

        /**
        *
        */
        virtual TBackupIncType IncrementType() const = 0;

        /**
        *
        */
        virtual TTransferDataType PassiveTransferType() const = 0;

        /**
        *
        */
        virtual TTransferDataType ActiveTransferType() const = 0;

        /**
        *
        */
        virtual TPackageDataType PackageTransferType() const = 0;

    public: // Common attributes

        /**
        *
        */
        IMPORT_C const CMMCScBkupDriveAndOperationTypeManager& DriveAndOperations() const;
        
        /**
        *
        */
        IMPORT_C void SetArchiveInfosL(RPointerArray<CMMCScBkupArchiveInfo>& aInfos);
        
        /**
        *
        */
        RMMCScBkupPointerArray<CMMCScBkupArchiveInfo>& ArchiveInfos();
        
        /**
        *
        */
        const CMMCScBkupArchiveInfo& ArchiveInfo(TBitFlags aCategory) const;

        /**
        *
        */
        const TDesC& FileName(TInt aIndex) const;

        /**
        *
        */
        TBitFlags Categories() { return iCategories; }
        
    private: // Data members

        //
        CMMCScBkupDriveAndOperationTypeManager* iDriveAndOperations;
        //
        RMMCScBkupPointerArray< CMMCScBkupArchiveInfo > iArchiveInfos;
        //
        TBitFlags iCategories;
    };



/**
*
*
* @since 3.0
*/
NONSHARABLE_CLASS(CMMCScBkupOpParamsBackupFull) : public CMMCScBkupOpParamsBase
    {
    public:

        /**
        *
        */
        IMPORT_C static CMMCScBkupOpParamsBackupFull* NewL( TResourceReader& aDriveReader, TResourceReader& aCategoryReader,
            TDriveNumber aDrive, TBitFlags aCategories );

        /**
        *
        */
        IMPORT_C ~CMMCScBkupOpParamsBackupFull();

    private:

        /**
        *
        */
        CMMCScBkupOpParamsBackupFull( TDriveNumber aDrive );

        /**
        *
        */
        void ConstructL( TResourceReader& aDriveReader, TResourceReader& aCategoryReader, TBitFlags aCategories );

        /**
        *
        */
        void ReadFromResourceL( TResourceReader& aReader );
        
    public: // From CMMCScBkupOpParamsBase
        IMPORT_C TMMCScBkupOperationType AssociatedOpType() const;
        IMPORT_C TBURPartType PartType() const;
        IMPORT_C TBackupIncType IncrementType() const;
        IMPORT_C TTransferDataType PassiveTransferType() const;
        IMPORT_C TTransferDataType ActiveTransferType() const;
        IMPORT_C TPackageDataType PackageTransferType() const;

    private: // Data members

        //
        TDriveNumber iDrive;
    };




/**
*
*
* @since 3.0
*/
NONSHARABLE_CLASS(CMMCScBkupOpParamsRestoreFull) : public CMMCScBkupOpParamsBase
    {
    public:

        /**
        *
        */
        IMPORT_C static CMMCScBkupOpParamsRestoreFull* NewL( TResourceReader& aDriveReader, TBitFlags aCategories );

        /**
        *
        */
        IMPORT_C ~CMMCScBkupOpParamsRestoreFull();

    private:

        /**
        *
        */
        CMMCScBkupOpParamsRestoreFull();

    public: // From CMMCScBkupOpParamsBase
        IMPORT_C TMMCScBkupOperationType AssociatedOpType() const;
        IMPORT_C TBURPartType PartType() const;
        IMPORT_C TBackupIncType IncrementType() const;
        IMPORT_C TTransferDataType PassiveTransferType() const;
        IMPORT_C TTransferDataType ActiveTransferType() const;
        IMPORT_C TPackageDataType PackageTransferType() const;
    };





#endif // __CMMCSCBKUPOPERATIONPARAMETERS_H__

//End of File
