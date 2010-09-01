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
*     Base class for scanner classes.
*
*
*/


#ifndef MSENGSCANNERBASE_H
#define MSENGSCANNERBASE_H




// USER INCLUDES
#include "mseng.hrh"
#include "msenginfoarray.h"
#include "msengscanobserver.h" 



// CONSTANTS
const TInt KDirectoryListGranularity = 10;

// CLASS DECLARATION

/**
*  Base class for scanner classes.
*  
*/
class CMsengScannerBase : public CActive
    {
    public:  // Constructors and destructor
 
        /**
        *
        */
        enum TStepResult
            {
            ECompleteRequest = 0,
            ERequestIssuedInternally,
            EScanComplete
            };
        
        /**
        * C++ default constructor.
        */
        CMsengScannerBase(MMsengScannerObserver& aObserver, 
            CMsengInfoArray& aInfoArray, RFs& aFsSession);
        
        /**
        * Destructor.
        */
        virtual ~CMsengScannerBase();

	protected: // Internal functions

        /**
        * Do one incremental scanning step
        */
        virtual TStepResult PerformStepL() = 0;

        /**
        * Complete this active object's request and set it active again
        */
        void CompleteRequest(TInt aCode = KErrNone);

        /**
        * Add new value in bytes to the amount of data found with this UID.
        * @param aUid The UID type.
        * @param aSize The size of the file with this UID.
        */
        inline void AddSizeByUidL(TUidTypes aUid, TInt64 aSize);

        /**
        * Add new value in bytes to the amount of data found with this UID.
        * @param aUid The UID type.
        * @param aSize The size of the file with this UID.
        */
        inline void AddSizeByExtL(TExtTypes aExt, TInt64 aSize);

        /**
        * Query if the directory is in the list of directories to be 
        * excluded from scanning, or is a subdirectory of one.
        * @param aDirectory Path of the directory
        */
        inline TBool IsExcludedDir(const TDesC& aDirectory);

        /**
        * Query if the directory is in the list of directories to be 
        * excluded from scanning, or is a subdirectory of one.
        * @param aDirectory Path of the directory
        */
        inline TBool IsSpecialDir(const TDesC& aDirectory);

        /**
        * Access the file server session
        */
        inline RFs& FsSession();

        /**
        * Access the information array
        */
        inline CMsengInfoArray& InfoArray();

        /**
        * Access the observer of this scanner
        */
        inline MMsengScannerObserver& ScannerObserver();

    private: // from CActive

        /**
        *
        */
        virtual void RunL();

        /**
        *
		*/
        virtual void DoCancel();

        /**
        *
        */
        virtual TInt RunError(TInt aError);

    private:

        // Prohibit copy constructor if not deriving from CBase.
        CMsengScannerBase( const CMsengScannerBase& );
        // Prohibit assigment operator if not deriving from CBase.
        CMsengScannerBase& operator= ( const CMsengScannerBase& );


    private:

        MMsengScannerObserver& iObserver;
        CMsengInfoArray& iInfoArray;
        RFs& iFsSession;
    };

#include "msengscannerbase.inl"

#endif      // CMSENGSCANNERBASE_H   
            
// End of File
