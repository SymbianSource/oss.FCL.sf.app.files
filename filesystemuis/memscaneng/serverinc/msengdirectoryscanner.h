/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
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
*     Scan directories.
*
*/


#ifndef MSENGDIRECTORYSCANNER_H
#define MSENGDIRECTORYSCANNER_H

// USER INCLUDES
#include "msengscannerbase.h"


// CLASS DECLARATION

/**
*  This class identifies all subdirectories based upon a given root path
*/
class CMsengDirectoryScanner : public CMsengScannerBase
    {
    public:  // Constructors and destructor
        
        /**
        * C++ default constructor.
        */
        CMsengDirectoryScanner(MMsengScannerObserver& aObserver, 
            CMsengInfoArray& aScanArray, RFs& aFsSession);
        
        /**
        * Destructor.
        */
        ~CMsengDirectoryScanner();

		/**
		*
		*/
		enum TScanDirectoryResult
			{
			EContinueToNextDirectory = 0,
			EContinueProcessingCurrentDirectory
			};

    public: // New functions
        
        /**
        * Start the scan operation.
        */
        void ScanL(const CDesCArray& aRootDirs);

	protected: // From CMsengScannerBase

        /**
        * Do one incremental scanning step
        */
        virtual TStepResult PerformStepL();

	protected: // New framework

        /**
        * This is called when one directory needs scanning
        */
        virtual TScanDirectoryResult ScanDirectoryL(const TDesC& aDirectory, RFs& aFsSession) = 0;

        /**
        * Use this function to add a new directory to the list of
        * directories which require scanning
        */
        void AppendDirectoryL(const TDesC& aDirectory);

    private:

        /**
        *
        */
        enum TState
            {
            EExpandingRootDirs = 0,
            EParsingExpandedList,
            EScanningSubDirs
            };

    private:

        /**
        *
        */
        TState iState;

        /**
        *
        */
        TInt iCurrentIndex;

        /**
        *
        */
        CDirScan* iScanner;

        /**
        *
        */
        TInt iNumberOfRootDirectories;

        /**
        *
        */
        CDesCArray* iDirectoryList;
    };

#endif      // MSENGDIRECTORYSCANNER_H   
            
// End of File
