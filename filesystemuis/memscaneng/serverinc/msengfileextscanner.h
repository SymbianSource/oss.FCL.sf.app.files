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
*     Scanner class used to scan file system by filename extensions.
*
*/


#ifndef MSENGFILEEXTSCANNER_H
#define MSENGFILEEXTSCANNER_H

//  INCLUDES
#include "msengfilescanner.h"

// CLASS DECLARATION

/**
*  Scanner class used to scan file system by filename extension.
*/
class CMsengFileExtScanner : public CMsengFileScanner
    {
    public:  // Constructors and destructor
        
        /**
        * constructor
        */
        static CMsengFileExtScanner* NewL(
            MMsengScannerObserver& aObserver, 
            CMsengInfoArray& aScanArray, 
            RFs& aFsSession);            

        /**
        * Destructor.
        */
        ~CMsengFileExtScanner();

    public: // New functions
        
        /**
        *
        */
        virtual CDir* FindFilesL(const TDesC& aDirectory, TBool& aMoveToNextDirectory);

        /**
        *
        */
        virtual TLocationResponse HandleLocatedEntryL(const TDesC& aFullFileNameAndPath, 
            const TEntry& aEntry);

    private:

        /**
        * C++ default constructor.
        */
        CMsengFileExtScanner(
            MMsengScannerObserver& aObserver, 
            CMsengInfoArray& aScanArray, 
            RFs& aFsSession);            

    private:

        // Prohibit copy constructor if not deriving from CBase.
        CMsengFileExtScanner( const CMsengFileExtScanner& );
        // Prohibit assigment operator if not deriving from CBase.
        CMsengFileExtScanner& operator= ( const CMsengFileExtScanner& );

		/**
		*
		*/
		TInt iCurrentExtensionIndex;

		/**
		*
		*/
        TBool iMoveToNextDirectory;
    };

#endif      // MSENGFILEEXTSCANNER_H   
            
// End of File
