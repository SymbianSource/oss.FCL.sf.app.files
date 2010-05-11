/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: 
*     The actual "engine". 
*
*/


#ifndef CMSENG_H
#define CMSENG_H


// SYSTEM INCLUDES
#include <data_caging_path_literals.hrh>
#include <badesca.h>                        // descriptor arrays
#include <f32file.h>                        // enum TDriveNumber
#include <stringresourcereader.h>
#include <barsc2.h>                         // CResourceFile
#include <barsread2.h>                      // RResourceReader


// USER INCLUDES
#include "msenguihandler.h"


// FORWARD DECLARATIOS
class CMsengScanner;
class RFs;
class MMsengUIHandler;


// CONSTANTS

// Resource file path
_LIT(KMsengRscFilePath,"Z:mseng.rsc");


// DATA TYPES

/**
*  Type definition to handle arrays of integers 
*  more conveniently
*/
typedef CArrayFixFlat<TInt> CIntArray;


// CLASS DECLARATION

/**
*  The actual "engine".
*  This is the class which the UI instantiates.
*/
class CMseng :public CBase
    {
    public:  // Constructors and destructor
        /**
        * Two-phased constructor.
        * @param aUIHandler Reference to a class implementing MMsengUIHandler interface
        */
        IMPORT_C static CMseng* NewL(MMsengUIHandler& aUIHandler);
        /**
        * Destructor.
        */
        IMPORT_C ~CMseng();

    public: // New functions
        
        /**
        * Get the data groups.
        * @return CDesCArray* containing the names of the data groups.
        */
        IMPORT_C CDesCArray* DataGroupsL() const;

        /**
        * Get the scan result. This array contains exacly one
        * integer per data group. 
        * @return Array of integers. 
        */
        IMPORT_C CArrayFix<TInt64>* ScanResultL() const;

        /**
        * Is there scanning going on?
        * @return ETrue if there is scanning going on, otherwise EFalse.
        */
        IMPORT_C TBool ScanInProgress() const;

        /**
        * Get the amount of total and free space on a disk
        * @param aTotal Amount of total space in bytes in substituted here.
        * @param aFree Amount of free space in bytes in substituted here.
        * @param aVolume Disk identifier, e.g. 'C'
        */
        IMPORT_C void DiskInfoL(TInt64& aTotal, TInt64& aFree, const TDriveNumber aVolume) const;

        /**
        * Get the amount of total and free RAM.
        * @param aTotal Amount of total RAM in bytes in substituted here.
        * @param aTotal Amount of free RAM in bytes in substituted here.
        */
        IMPORT_C static void MemInfoL(TInt64& aTotal, TInt64& aFree);

        /**
        * Start scanning memory. 
        * This means starting the actual work of the engine. Starts scanning,
        * which runs until finished.
        *
        * The scanning is only supported for drives C and E, otherwise
        * leave occurs with error KErrNotSupported.
        *        
        * NOTE: the support for scanning E drive is not properly tested.
        *       It is assumed to have the same directory structure than in C.
        *
        * @param aDrive the drive to be scanned
        */
        IMPORT_C void ScanL(TDriveNumber aDrive);

        /**
        * Cancel ongoing scanning operation.
        */
        IMPORT_C void Cancel();
        
        /**
        * Check whether internal drive.
        * @param aDrv Drive to be checked.
        * @return ETrue if internal drive, otherwise EFalse.
        */
        static TBool IsInternalDrive( RFs& aFs, const TInt aDrv );

        /**
        * Check whether removable drive.
        * @param aDrv Drive to be checked.
        * @return ETrue if internal drive, otherwise EFalse.
        */
        static TBool IsRemovableDrive( RFs& aFs, const TInt aDrv );
       
        /**
        * Check whether mass storage drive.
        * @param aDrv Drive to be checked.
        * @return ETrue if mass storage drive, otherwise EFalse.
        */
        static TBool IsMassStorageDrive( RFs& aFs, const TInt aDrv );
        
    private:

        /**
        * C++ default constructor is prohibited.
        */
        CMseng(MMsengUIHandler& aUIHandler);

        /**
        * By default Symbian OS constructor is private.
        *
        * Initialize iResultArray by reading the the data group 
        *  names from resource file and setting all result values to zero.
        * Initialize iDataGroupUidArray and iDataGroupExtArray
        *  from resource file.
        */
        void ConstructL();
        

        // Prohibit copy constructor if not deriving from CBase.
        CMseng( const CMseng& );
        // Prohibit assigment operator if not deriving from CBase.
        CMseng& operator= ( const CMseng& );


    private:    // Data members
    
        MMsengUIHandler& iUIHandler;
        CMsengScanner* iScanner;

        // Number of data groups
        TInt iNumberOfDataGroups;

        // These arrays are needed to when calculating the result.
        // They are indexed using values from enum TDataGroups.
        // This creates the mapping between data groups and the
        // UIDs and extensions belonging to a particular data group
        CArrayPtrFlat<CIntArray>* iDataGroupUidArray;
        CArrayPtrFlat<CIntArray>* iDataGroupExtArray;        
        
        RFs iFsSession;
        CResourceFile* iResFile;
        mutable TInt64 iFreeMemory;        
        
    };

#endif      // CMSENG_H   
            
// End of File
