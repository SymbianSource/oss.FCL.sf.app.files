/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  AppUi class of the application.
*
*/

#ifndef __DEVENCUIAPPUI_H__
#define __DEVENCUIAPPUI_H__

// INCLUDES
#include <aknViewAppUi.h>
#include <AknQueryDialog.h>
#include <DevEncMmcObserver.h>
#include "DevEncUiMemInfoObserver.h"
#include "DevEncUiTimer.h"

// FORWARD DECLARATIONS
class CDevEncUiEncrView;
class CDevEncUiEncryptionOperator;
class CDevEncUiDecrView;
class CDevEncUiMainView;
class CDevEncUiMemoryEntity;
class CRepository;

// CLASS DECLARATION
/**
* CDevEncUiAppUi application UI class.
* Interacts with the user through the UI and request message processing
* from the handler class
*/
class CDevEncUiAppUi : public CAknViewAppUi,
                       public MMemoryCardObserver,
                       public MDevEncUiMemInfoObserver,
                       public MDevEncUiTimerCallback
    {
    public: // Constructors and destructor

        /**
        * ConstructL.
        * 2nd phase constructor.
        */
        void ConstructL();

        /**
        * CDevEncUiAppUi.
        * C++ default constructor. This needs to be public due to
        * the way the framework constructs the AppUi
        */
        CDevEncUiAppUi();

        /**
        * Virtual Destructor.
        */
        virtual ~CDevEncUiAppUi();

        /**
        * Dynamically initialises a menu pane.
        * The Uikon framework calls this function, if it is implemented in a
        * menu's observer, immediately before the menu pane is activated.
        */
        void DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane );

        /**
        * From MMemoryCardObserver.
        */
        void MMCStatusChangedL();

        /**
        * From MDevEncUiMemInfoObserver.
        */
        void UpdateInfo( TDevEncUiMemoryType aType,
                         TUint aState,
                         TUint aProgress );
        
        /**
         * From MDevEncUiTimerCallback
         */
        void Timeout();
        
    protected:
        /**
        * From CAknViewAppUi. Handles changes in keyboard focus when an
        * application switches to foreground.
        */
        void HandleForegroundEventL( TBool aForeground );

    private:  // Functions from base classes

        /**
        * From CEikAppUi, HandleCommandL.
        * Takes care of command handling.
        * @param aCommand Command to be handled.
        */
        void HandleCommandL( TInt aCommand );

        /**
        * Displays the context-sensitive help.
        */
        void DisplayHelpL();

        void DoUpdateInfoL( TDevEncUiMemoryType aType,
                            TUint aState );

        /**
         * From CEikAppUi.
         * Command line processing.
         *
         * When DocumentHandler wants to launch NpdViewer as a standalone
         * application, it use RApaLsSession::StartDocument(filename, ...).
         *
         * CEikonEnv::ConstructAppFromCommandLineL(...), at first, pass 
         * the filename information to ProcessCommandParametersL 
         * correctly. But default CEikAppUi::ProcessCommandParametersL 
         * overwrite it's reference parameter (TFileName& aDocumentName) to 
         * application's defualt document name such as "NpdViewer.ini".
         * (In EikAppUi.cpp, 
         * Application()->GetDefaultDocumentFileName(aDocumentName);
         * do this). 
         * So,  when CEikonEnv::ConstructAppFromCommandLineL(...) calls
         * CApaDocument::OpenFileL(...), the filename information from 
         * Document Handler had been lost.
         * 
         * On the other hand, when DocumentHandler wants to launch NpdViewer 
         * as a embeded application, it use CApaDocument::OpenFileL() directly 
         * and never call ProcessCommandParametersL.
         *
         * So, in order to pass a correct filename information to OpenFileL 
         * in both case, we decide to override this function.
         *
         * @param aCommand command. (ignored)
         * @param aDocumentName Filename.
         * @param aTail optional command line parampeter. (ignored)
         * @return ETrue if aDocumentName file exists.
         */
         TBool ProcessCommandParametersL(
             TApaCommand aCommand,
             TFileName& aDocumentName,
             const TDesC8& aTail);

         /**
         * From CEikAppUi.
         * Start fileviewer with specified filename.
         * Both standalone and embeded case, this function is really called 
         *
         * @param aFilename Filename to view.
         */
         void OpenFileL(const TDesC& aFilename);
         
         /** 
          * Copy a file to a new path (Others folder)
          **/
         void CopyL(const TDesC &anOld, const TDesC &aNew);

 	public:
 	
         /**
         * Start fileviewer with specified file handle.
         * Both standalone and embeded case, this function is really called 
         *
         * @param aFile File handle.
         */
         void OpenFileL(RFile& aFile);

    private: // Data

        /** Created by this class, ownership transferred to CAknAppUi */
        CDevEncUiMainView* iMainView;

        /** Created by this class, ownership transferred to CAknAppUi */
        CDevEncUiEncrView* iEncryptionView;

        /** Created by this class, ownership transferred to CAknAppUi */
        CDevEncUiDecrView* iDecryptionView;

        /** Owned */
        CDevEncUiEncryptionOperator* iEncOperator;

        /** Owned */
        RArray<CDevEncUiMemoryEntity*> iMemEntities;
        
        /** Owned */
        CRepository* iCrSettings;

        /** Owned */
        CMmcObserver* iObserver;
        
        /** Owned */
        RFs iFs;
        
        /** Holds the current MMC mount status (from file server) */
        TInt iMmcStatus;
        
        /** Hold the current MMC encryption status (from NFE) */
        TInt iMmcEncState;
        
        /** ETrue if a Mmc status update is ongoing (to prevent re-entry) */
        TBool iStatusUpdateOngoing;
        
        /** Owned */
        CDevEncUiTimer* iTimer;
        
        /** ETrue if the application is foreground */
        TBool iForeground;
        
        /** ETrue if the application hasn't completed yet the construction phase */
        TBool iConstructionOnGoing;
    };

#endif // __DEVENCUIAPPUI_H__

// End of File
