/*
* Copyright (c) 2002-2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Calculates directory size
*
*/



#ifndef CFILEMANAGERACTIVESIZE_H
#define CFILEMANAGERACTIVESIZE_H

// INCLUDES
#include <e32base.h>
#include <AknWaitNoteWrapper.h>

// FORWARD DECLARATIONS
class CDirScan;

// CLASS DECLARATION
/**
*  
*  Calculates folder size, one folder at time.
*
*  @lib FileManagerEngine.lib
*  @since 2.0
*/
NONSHARABLE_CLASS(CFileManagerActiveSize) : public CBase, 
                                            public MAknBackgroundProcess
    {
    public:
        /**
        * Two-phased constructor.
        */
        static CFileManagerActiveSize* NewL(
            RFs& aFs, const TDesC& aRootFolder );
        /**
        * Destructor.
        */
        ~CFileManagerActiveSize();

    public: // From MAknBackgroundProcess
        /**
         * Completes one cycle of the process.
         */
        void StepL();

        /**
         * Return true when the process is done.
         */
        TBool IsProcessDone() const;

    public:

        /**
        * Get the result, size of the folder
        * @since 2.0
        * @return Size of the folder in bytes
        */
        TInt64 GetFolderSize() const;

    private:
        /**
        * C++ default constructor.
        */
        CFileManagerActiveSize( RFs& aFs );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL( const TDesC& aRootFolder );

    private: // Data

        /// Own: Used to go through all the folders in given directory
        CDirScan* iDirScan;
    
        /// Own: CDirScan returns this
        CDir* iDir;

        /// File Server session
        RFs& iFs;

        /// When all folders are through, this is set
        TBool iProcessDone;

        /// Index to currently checked CDir item
        TInt iFileCounter;

        // Folder Size
        TInt64 iSize;
    };

#endif      // CFILEMANAGERACTIVESIZE_H
            
// End of File

