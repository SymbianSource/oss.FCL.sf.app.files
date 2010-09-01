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
*     Scanner class used to scan file system by filename extension
*
*
*/


#ifndef MSENGFINDALLSCANNER_H
#define MSENGFINDALLSCANNER_H

//  INCLUDES
#include "msengfilescanner.h"

// CLASS DECLARATION

/**
*  Scanner class used to scan file system by filename extension.
*/
class CMsengFindAllScanner : public CMsengFileScanner
    {
    public:  // Constructors and destructor
        
        /**
        * constructor
        */
        static CMsengFindAllScanner* NewL(
            MMsengScannerObserver& aObserver, 
            CMsengInfoArray& aScanArray, 
            RFs& aFsSession);        


        /**
        * Destructor.
        */
        ~CMsengFindAllScanner();

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
        CMsengFindAllScanner(
            MMsengScannerObserver& aObserver, 
            CMsengInfoArray& aScanArray, 
            RFs& aFsSession);

    private:

        // Prohibit copy constructor if not deriving from CBase.
        CMsengFindAllScanner( const CMsengFindAllScanner& );
        // Prohibit assigment operator if not deriving from CBase.
        CMsengFindAllScanner& operator= ( const CMsengFindAllScanner& );
    };

#endif      // MSENGFINDALLSCANNER_H   
      
// End of File
