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
*     CMsengScanner is the class that mostly controls the operation 
*     of Memory Scan Engine. It CMsengScannerBase-derived active objects  
*     to perform scanning operations.
*
*/


#ifndef CMSENGSCANNER_H
#define CMSENGSCANNER_H

// SYSTEM INCLUDES
#include <barsc2.h>             // CResourceFile
#include <barsread2.h>          // RResourceReader

// USER INCLUDES
#include "msengscanobserver.h"
#include "msenginfoarray.h"


// FORWARD DECLARATIONS
class MMsengUIHandler;
class CMsengFileExtScanner;
class CMsengFindAllScanner;
class CMsengRegistryScanner;


// CLASS DECLARATION

/**
*  CMsengScanner controls the scanning operation.
*/
class CMsengScanner : public CBase, public MMsengScannerObserver
    {
    public:  // Constructors and destructor
      
        /**
        * C++ default constructor.
        */
        CMsengScanner(MMsengUIHandler& aUIHandler, CResourceFile& aResFile);

        /**
        * Destructor.
        */
        virtual ~CMsengScanner();

    public: // New functions
        
        /**
        * 
        */
        void Cancel();
        
        /**
        * 
        */
        TBool HaveActiveScanners() const;

        /**
        * Start scanning (creates a new thread).
        * @param aDrive the drive which is scanned
        * @paran aNumberOfDataGroups 
        * @return KErrInUse if the scanning is already going on; 
        *         KErrNone otherwise
        */
        TInt ScanL(TDriveNumber aDrive, TInt aNumberOfDataGroups, RFs& FsSession);

        /**
        * Get scan results per UID.
        * @return Pointer to an array containing results per UID.
        */
        inline const CArrayFix<TInt64>* UidResults() const;

        /**
        * Get scan results per filename extension.
        * @return Pointer to an array containing results per ext.
        */
        inline const CArrayFix<TInt64>* ExtResults() const;

        /**
        * Get scan results per data group.
        * @return Pointer to an array containing results per group.
        */
        inline const CArrayFix<TInt64>* GroupResults() const;
        
        /**
        * 
        */
        inline const TDriveNumber CurrentDrive() const;

   public: // From MMsengScannerObserver

        /**
        *
        * @param aEvent event that is handled
        */
        void HandleScannerEventL(TScannerEvent aEvent, 
            const CMsengScannerBase& aScanner, TInt aError=KErrNone);

    private:

        /**
        * Do some simple scanning before actual work. 
        * @param aDrive the drive to be scanned
        * @param aFsSession reference to file server session
        */
        void PreScanL(TDriveNumber aDrive, RFs& aFsSession);
        
        // Prohibit copy constructor if not deriving from CBase.
        CMsengScanner( const CMsengScanner& );
        // Prohibit assigment operator if not deriving from CBase.
        CMsengScanner& operator= ( const CMsengScanner& );

    private:    // Data

        // External objects
        MMsengUIHandler& iUIHandler;
        CMsengInfoArray* iScanArray;
        CResourceFile& iResFile;

        // Owned objects
        CMsengFileExtScanner* iFileExtScanner;
        CMsengFindAllScanner* iFindAllScanner;
        CMsengRegistryScanner* iRegistryScanner;
    };

#include "msengscanner.inl"

#endif      // CMSENGSCANNER_H   
            
// End of File
