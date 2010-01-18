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
*     Scan files.
*
*/


#ifndef MSENGFILESCANNER_H
#define MSENGFILESCANNER_H

// USER INCLUDES
#include "msengdirectoryscanner.h"

// CLASS DECLARATION

/**
* This class identifies all files in a given directory based upon a
* directory specification.
*/
class CMsengFileScanner : public CMsengDirectoryScanner
    {
    public:  // Constructors and destructor
        
        /**
        * C++ default constructor.
        */
        CMsengFileScanner(MMsengScannerObserver& aObserver, 
            CMsengInfoArray& aScanArray, RFs& aFsSession);
        
        
        /**
        * Destructor.
        */
        ~CMsengFileScanner();

    public:
    
        /**
        * 
        */
        enum TLocationResponse
            {
            EEntryWasProcessed = 0,
            EEntryWasDiscarded
            };

    protected: // From CMsengDirectoryScanner

        /**
        * Scan this directory for files matching a subclass' criteria
        */
        virtual TScanDirectoryResult ScanDirectoryL(const TDesC& aDirectory, RFs& aFsSession);

        /**
        * Ask the subclass to populate a CDir object with a list of files matching
        * chosen criteria
        */
        virtual CDir* FindFilesL(const TDesC& aDirectory, TBool& aMoveToNextDirectory) = 0;

        /**
        * Inform the subclass of a matching entry
        */
        virtual TLocationResponse HandleLocatedEntryL(const TDesC& aFullFileNameAndPath, 
            const TEntry& aEntry) = 0;
    };

#endif      // MSENGFILESCANNER_H   
            
// End of File
