/*
* Copyright (c) 2002-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Receives obex object via InfraRed, used to receive files
*
*/



#ifndef CFILEMANAGERIRRECEIVER_H
#define CFILEMANAGERIRRECEIVER_H

// INCLUDES
#include <e32base.h>
#include <obex.h>

// FORWARD DECLARATIONS
class CObexServer;
class CObexBufObject;
class MFileManagerProcessObserver;
class CFileManagerEngine;

// CLASS DECLARATION
/**
*  Handles the file receiving via infrared
*
*  @lib FileManagerEngine.lib
*  @since 2.0
*/
NONSHARABLE_CLASS(CFileManagerIRReceiver) : public CBase,
                                            public MObexServerNotify
    {
    public:

        /**
        * Two-phased constructor.
        */
        IMPORT_C static CFileManagerIRReceiver* NewL(
            MFileManagerProcessObserver& aObserver,
            const TDesC& aPath,
            CFileManagerEngine& aEngine );

        /**
        * Destructor.
        */
        IMPORT_C ~CFileManagerIRReceiver();

        /**
         * Receives file through IR and stores it to
         * iPath location. Filename is get from 
         * sent file and not included in iPath.
         */
        IMPORT_C void ReceiveFileL();

        /**
         * Stops receive process and discards all received data.
         */
        IMPORT_C void StopReceiving();

    public: // From MObexServerNotify
        /**
         * @see MObexServerNotify
         */
        void ErrorIndication (TInt aError);
        /**
         * @see MObexServerNotify
         */
        void TransportUpIndication ();
        /**
         * @see MObexServerNotify
         */
        void TransportDownIndication ();  
        /**
         * @see MObexServerNotify
         */
        TBool TargetHeaderReceived(TDesC8& aTargetHeader);
        /**
         * @see MObexServerNotify
         */
        TInt ObexConnectIndication(
            const TObexConnectInfo& aRemoteInfo, const TDesC8& aInfo );
        /**
         * @see MObexServerNotify
         */
        void ObexDisconnectIndication (const TDesC8& aInfo);
        /**
         * @see MObexServerNotify
         */
        CObexBufObject* PutRequestIndication ();
        /**
         * @see MObexServerNotify
         */
        TInt PutPacketIndication ();  
        /**
         * @see MObexServerNotify
         */
        TInt PutCompleteIndication ();  
        /**
         * @see MObexServerNotify
         */
        CObexBufObject* GetRequestIndication(
            CObexBaseObject* aRequiredObject );
        /**
         * @see MObexServerNotify
         */
        TInt GetPacketIndication (); 
        /**
         * @see MObexServerNotify
         */
        TInt GetCompleteIndication ();  
        /**
         * @see MObexServerNotify
         */
        TInt SetPathIndication(
            const CObex::TSetPathInfo& aPathInfo, const TDesC8& aInfo );
        /**
         * @see MObexServerNotify
         */
        void AbortIndication ();  
    private:
        /**
        * C++ default constructor.
        */
        CFileManagerIRReceiver(
            MFileManagerProcessObserver& aObserver,
            CFileManagerEngine& aEngine,
            RFs& aFs );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL( const TDesC& aPath );

        /**
         * Resets inactivity timer.
         */
        void ResetInactivityTimer();

        /**
         * Stops inactivity timer.
         */
        void StopInactivityTimer();

        /**
         * Handles inactivity timeout.
         */
        static TInt InactivityTimeout( TAny* aPtr );

        /**
         * Closes connection on timeout.
         */
        void CloseConnection();

    private:    
        // Caller is informed about starting and stopping of the process. Also
        // the processed percentage is updated with this observer.
        MFileManagerProcessObserver& iObserver;

        // Own: Handles the obex object transfer
        CObexServer* iObexServer;

        // Own: Obex object of the received file
        CObexBufObject* iObexBufObject;

        // Own: Destination directory where the received file will be stored
        HBufC* iTempFile;

        // Ref. For checking file exists situation
        CFileManagerEngine& iEngine;

        // Error code
        TInt iError;
        
        // Is disk space already checked
        TBool iDiskSpaceChecked;

        // is there enought space
        TBool iEnoughSpace;

        // Ref: File server session
        RFs& iFs;

        // Own: temp buffer 
        CBufFlat* iBuffer;

        // Own: Inactivity timer
        CPeriodic* iInactivityTimer;

    };

#endif      // CFILEMANAGERIRRECEIVER_H
            
// End of File
