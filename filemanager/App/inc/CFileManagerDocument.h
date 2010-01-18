/*
* Copyright (c) 2002-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Document class of the file manager
*
*/



#ifndef CFILEMANAGERDOCUMENT_H
#define CFILEMANAGERDOCUMENT_H

//  INCLUDES
#include <GenericParamConsumer.h>
#include <ConeResLoader.h>  // RConeResourceLoader
#include <badesca.h>


// FORWARD DECLARATIONS
class CFileManagerIconArray;
class CFileManagerEngine;
class CFileManagerStringCache;


// CLASS DECLARATION
/**
 * FileManager application document class. 
 * An object of this class is created by the Symbian OS framework by a call to 
 * CFileManagerApplication::CreateDocumentL().
 * The application framework creates the FileManager application UI object 
 * (CFileManagerAppUi) by a call to the virtual function CreateAppUiL().
 */
class CFileManagerDocument : public CAiwGenericParamConsumer
    {

    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CFileManagerDocument* NewL( CEikApplication& aApp );
        
        /**
        * Destructor.
        */
        ~CFileManagerDocument();

    public: // From CAiwGenericParamConsumer
        CEikAppUi* CreateAppUiL();

        void UpdateTaskNameL( CApaWindowGroupName* aWgName );

        CFileStore* OpenFileL( TBool aDoOpen, const TDesC& aFilename, RFs& aFs );

    public: // New methods
        /**
         * Returns the icon array of this application.
         *
         * @return The icon array of this application.
         */
        CFileManagerIconArray* IconArray() const;

        /**
         * Returns reference to engine of this application
         *
         * @return Reference to engine of this application
         */
        CFileManagerEngine& Engine() const;

        /**
         * Returns the model for listbox
         * @return Return MDesCArray to current filelist
         */
        MDesCArray* FileList() const;

        /**
         * Clears the string cache
         */
        void ClearStringCache();

        /**
         * Gets last error
         */
        TInt LastError() const;

        /**
         * Checks is app was launched for scheduled backup
         */
        TBool IsScheduledBackup();

        /**
         * Stores ECOM plugins to be deleted with document
         */
        void StorePluginL( const TUid& aUid );

    private:

        /**
         * Standard C++ constructor.
         * 
         * @param aApp Application which this document belongs to.
         */
        CFileManagerDocument( CEikApplication& aApp );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        /**
         * Deletes stored ECOM plugins
         */
        void DeletePlugins();

    private:    // Data
        /// Own: Engine of the application
        CFileManagerEngine* iEngine;
        /// Own: Icon array that contains all the icons used in listboxes.
        CFileManagerIconArray* iIconArray;
        /// Resource loader for view resources
        RConeResourceLoader iViewResourceLoader;
        /// Resource loader for engine resources
        RConeResourceLoader iEngineResourceLoader;
        // Own: For caching listbox strings
        CFileManagerStringCache* iStringCache;
        // Own: Stores loaded ECom plugin ids
        RArray< TUid > iPluginArray;

    };

#endif      // CFILEMANAGERDOCUMENT_H
            
// End of File
