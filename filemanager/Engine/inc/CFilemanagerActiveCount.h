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


#ifndef CFILEMANAGERACTIVECOUNT_H
#define CFILEMANAGERACTIVECOUNT_H

// INCLUDES
#include <e32base.h>
#include <AknWaitNoteWrapper.h>
#include "CFileManagerItemProperties.h"

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
NONSHARABLE_CLASS(CFileManagerActiveCount) : public CBase,
                                             public MAknBackgroundProcess
    {
    public:
        /**
        * Two-phased constructor.
        */
        static CFileManagerActiveCount* NewL(
            RFs& aFs,
            const TDesC& aRootFolder,
            const CFileManagerItemProperties::TFileManagerFileType aType );
        /**
        * Destructor.
        */
        ~CFileManagerActiveCount();

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
        * Get the result,
        * @since 2.0
        * @return count of
        */
        TInt FolderCount() const;

        /**
        * Get the result,
        * @since 2.0
        * @return count of
        */
        TInt FileCount() const;

        /**
        * Get the result,
        * @since 2.0
        * @return count of
        */
        TInt OpenFiles() const;

    private:
        /**
        * C++ default constructor.
        */
        CFileManagerActiveCount( 
            RFs& aFs );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL( 
            const TDesC& aRootFolder, 
            const CFileManagerItemProperties::TFileManagerFileType aType  );

    private: // Data

        /// Own: Used to go through all the folders in given directory
        CDirScan*   iDirScan;
    
        /// Own: CDirScan returns this
        CDir*       iDir;

        /// File Server session
        RFs&        iFs;
    
        /// When all folders are through, this is set
        TBool       iProcessDone;

        /// Index to currently checked CDir item
        TInt        iFileCounter;

        // Folder count
        TInt        iFolders;

        // Folder count
        TInt        iFiles;

        // Folder count
        TInt        iOpenFiles;

        // Processed entry fullpath
        TFileName   iFileName;

    };

#endif      // CFILEMANAGERACTIVECOUNT_H
            
// End of File

