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
* Description:  File Manager application class
*
*/


#ifndef CFILEMANAGERAPP_H
#define CFILEMANAGERAPP_H

// INCLUDES
#include <aknapp.h> // CAknApplication

// CLASS DECLARATION

/**
 * FileManager application class.
 * This class is created by the Symbian OS framework by a call to 
 * NewApplication() function when the application is started. The 
 * main purpose of the application class is to create the 
 * application-specific document object (CFileManagerDocument in 
 * this case) via a call to virtual CreateDocumentL().
 */
class CFileManagerApplication : public CAknApplication
    {
    public:  // Constructors and destructor
        
        /**
         * Standard C++ constructor.
         */
        CFileManagerApplication();

        /**
         * Destructor.
         */
        ~CFileManagerApplication();

    public: // From CAknApplication
        /**
         * Creates the FileManager document object (CFileManagerDocument). 
         * Called by the application framework.
         * 
         * @return New FileManager document object (CFileManagerDocument).
         */
        CApaDocument* CreateDocumentL();

        /**
         * @return the UID of this application.
         */     
        TUid AppDllUid() const;

    };

#endif      // CFILEMANAGERAPP_H
            
// End of File
