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
* Description:  This AO is signaled when directory entry is added or removed 
*                from file system
*
*/


#ifndef CFILEMANAGERFILESYSTEMEVENT_H
#define CFILEMANAGERFILESYSTEMEVENT_H


// INCLUDES
#include <e32base.h>
#include <f32file.h>


// FORWARD DECLARATIONS
class CFileManagerEngine;


// CLASS DECLARATION

/**
*  This class listens file system events
*
*  @lib FileManagerEngine.lib
*  @since 2.0
*/
NONSHARABLE_CLASS(CFileManagerFileSystemEvent) : public CActive
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CFileManagerFileSystemEvent* NewL( RFs& aFs, 
            CFileManagerEngine& aEngine,
            TNotifyType aNotifyType, 
            const TDesC& aFullPath = KNullDesC );

        /**
        * Destructor.
        */
        virtual ~CFileManagerFileSystemEvent();

    public:

        /**
        * Sets file system event
        * @since 2.0
        */
        void Setup();

        /**
        * This calls refresh if file system event has happened
        * @since 2.0
        */
        void CheckFileSystemEvent();


    protected:  // Functions from base classes

        /**
        * From CActive Called when asynchronous request has completed
        * @since 2.0
        */
        void RunL();

        /**
        * From CActive Called when asynchronous request has failed
        * @since 2.0
        */
        TInt RunError( TInt aError );

        /**
        * From CActive Called when asynchronous request was cancelled
        * @since 2.0
        */
        void DoCancel();

    private:
        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL( const TDesC& aFullPath );

        /**
        * C++ default constructor.
        */
        CFileManagerFileSystemEvent(
            RFs& aFs, 
            CFileManagerEngine& aEngine,
            TNotifyType aNotifyType );

    private:    // Data
        // Ref: Reference to opened File System session
        RFs& iFs;

        // Ref: This object is called when RunL occurs
        CFileManagerEngine& iEngine;

        // Type of the notify, entry or disk.
        TNotifyType iNotifyType;

        // Full path of folder from where notification is needed
        HBufC* iFullPath;

        // Indicates if the event has been received
        TBool iIsReceived;

    };

#endif // CFILEMANAGERFILESYSTEMEVENT_H

// End of File
