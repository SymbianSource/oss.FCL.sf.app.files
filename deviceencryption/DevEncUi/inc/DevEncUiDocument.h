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
* Description:  Document class of the application.
*
*/

#ifndef __DEVENCUIDOCUMENT_H__
#define __DEVENCUIDOCUMENT_H__

// INCLUDES
#include <AknDoc.h>

// FORWARD DECLARATIONS
class CDevEncUiAppUi;
class CEikApplication;


// CLASS DECLARATION

/**
* CDevEncUiDocument application class.
* An instance of class CDevEncUiDocument is the Document part of the
* AVKON application framework for the DEVENCUI example application.
*/
class CDevEncUiDocument : public CAknDocument
    {
    public: // Constructors and destructor

        /**
        * NewL.
        * Two-phased constructor.
        * Construct a CDevEncUiDocument for the AVKON application aApp
        * using two phase construction, and return a pointer
        * to the created object.
        * @param aApp Application creating this document.
        * @return A pointer to the created instance of CDevEncUiDocument.
        */
        static CDevEncUiDocument* NewL( CEikApplication& aApp );

        /**
        * NewLC.
        * Two-phased constructor.
        * Construct a CDevEncUiDocument for the AVKON application aApp
        * using two phase construction, and return a pointer
        * to the created object.
        * @param aApp Application creating this document.
        * @return A pointer to the created instance of CDevEncUiDocument.
        */
        static CDevEncUiDocument* NewLC( CEikApplication& aApp );

        /**
        * ~CDevEncUiDocument
        * Virtual Destructor.
        */
        virtual ~CDevEncUiDocument();

    public: // Functions from base classes

        /**
        * CreateAppUiL
        * From CEikDocument, CreateAppUiL.
        * Create a CDevEncUiAppUi object and return a pointer to it.
        * The object returned is owned by the Uikon framework.
        * @return Pointer to created instance of AppUi.
        */
        CEikAppUi* CreateAppUiL();

    private: // Constructors

        /**
        * ConstructL
        * 2nd phase constructor.
        */
        void ConstructL();

        /**
        * CDevEncUiDocument.
        * C++ default constructor.
        * @param aApp Application creating this document.
        */
        CDevEncUiDocument( CEikApplication& aApp );
        
        /**
         * From CEikDocument.
         * Always return NULL, so as not to create a INI file.
         *
         * @param aDoOpen ETrue if file exists. (ignored)
         * @param aFilename file to view. 
         * @param aFs FileServerSession. (ignored)
         * @return file store. (always NULL)
         */
        CFileStore* OpenFileL(
            TBool /*aDoOpen*/, 
            const TDesC& aFilename,
            RFs& /*aFs*/);
                        
        /**
         * From CEikDocument.
         *
         * @param aFileStore. (ignored)
         * @param aFile. 
         */
        void OpenFileL( 
            CFileStore*& /*aFileStore*/,
            RFile& aFile);
        	
    };

#endif // __DEVENCUIDOCUMENT_H__

// End of File
