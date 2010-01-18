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
* Description:  Memory scan engine registry scanning
*
*/


#ifndef MSENGREGISTRYSCANNER_H
#define MSENGREGISTRYSCANNER_H

// USER INCLUDES
#include "msengscannerbase.h"


// CLASS DECLARATION

/**
* This class represents a registry scanner.
*
* @since 3.2
*/
class CMsengRegistryScanner : public CMsengScannerBase
    {
    public:  // Constructors and destructor
    
        /**
        * Constructor.
        * 
        * @param aDrive - Drive to be scanned.
        */
        static CMsengRegistryScanner* NewL( TDriveNumber aDrive, MMsengScannerObserver& aObserver, 
            CMsengInfoArray& aScanArray, RFs& aFsSession);
        
        /**
        * Destructor.
        */
        ~CMsengRegistryScanner();
        
    public: // New functions

        /**
        * Starts the scan operation.
        * @since S60 3.2
        */
        void Scan();

	protected: // Functions from base classes

        /**
         * From CMsengScannerBase
         * Do one incremental scanning step
         *
         * @since S60 3.2
         */
        virtual TStepResult PerformStepL();

        
    private:
    
        CMsengRegistryScanner(TDriveNumber aDrive, MMsengScannerObserver& aObserver, 
            CMsengInfoArray& aScanArray, RFs& aFsSession);

        void ScanSisRegistryL();
        
    private: // Internal enumerations

        /**
        *
        */
        enum TRegistryType
            {
            ERegistrySisx,
            ERegistryLast
            };

    private: // Data

        TRegistryType iType;
        TDriveNumber iDrive;
    };

#endif      // MSENGREGISTRYSCANNER
            
// End of File
