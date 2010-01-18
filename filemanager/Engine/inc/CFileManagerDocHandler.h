/*
* Copyright (c) 2004-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Wraps document handler functions
*
*/



#ifndef CFILEMANAGERDOCHANDLER_H
#define CFILEMANAGERDOCHANDLER_H

// INCLUDES
#include <apparc.h>
#include <AknServerApp.h>
#include <apgcli.h>
#include "MFileManagerThreadFunction.h"

// FORWARD DECLARATIONS
class CDocumentHandler;
class CFileManagerEngine;
class CFileManagerUtils;
class CEikProcess;
class CFileManagerThreadWrapper;
class MFileManagerProcessObserver;


// CLASS DECLARATION
/**
*  This class wraps dochandler functions
*
*  @lib FileManagerEngine.lib
*  @since 2.7
*/
NONSHARABLE_CLASS(CFileManagerDocHandler) : public CBase,
                                            public MAknServerAppExitObserver,
                                            public MFileManagerThreadFunction
    {
    public:

        /**
        * Two-phased constructor.
        */
        static CFileManagerDocHandler* NewL(
            CFileManagerEngine& aEngine,
             CFileManagerUtils& aUtils );
            
        /**
        * Destructor.
        */
        ~CFileManagerDocHandler();

    private: // From MAknServerAppExitObserver
        void HandleServerAppExit( TInt aReason );

    private: // From MFileManagerThreadFunction

        void ThreadStepL();

        TBool IsThreadDone();

        void NotifyThreadClientL( TNotifyType aType, TInt aValue );

    public: // New functions
        void OpenFileL(
            const TDesC& aFullPath, MFileManagerProcessObserver* aObserver );

        void CancelFileOpen();

    private:
        /**
        * C++ default constructor.
        */
        CFileManagerDocHandler(
            CFileManagerEngine& aEngine,
            CFileManagerUtils& aUtils );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        void OpenShareableFileL( RFile64& aShareableFile, const TDesC8& aMime );

    private:

        // Ref: To notify engine of embedded application
        CFileManagerEngine& iEngine;
        
        // Ref: For using utilities
        CFileManagerUtils& iUtils;

        // Ref: Shareable file server session
        RFs& iFs;

        // Own: For launching files
        CDocumentHandler* iDocHandler;

        // Own: Shareable file
        RFile64 iFile;

        // Own: File ready indicator
        TBool iFileReady;

        // Own: File fullpath
        HBufC* iFileFullPath;

        // Own: File MIME type
        HBufC8* iFileMime;

        // Ref: Observer for document open status
        MFileManagerProcessObserver* iObserver;

        // Own: For async file open for downloading file to cache
        CFileManagerThreadWrapper* iThreadWrapper;

        // Own: For getting app uids
        RApaLsSession iApaSession;

        // Own: Indicates the file open status
        TBool iEmbeddedAppOpen;

    };

#endif // CFILEMANAGERDOCHANDLER_H
