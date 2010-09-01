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
* Description:  Application entry point.
*
*/

#ifndef __DEVENCUIAPPLICATION_H__
#define __DEVENCUIAPPLICATION_H__

// INCLUDES
#include <aknapp.h>

// CLASS DECLARATION

/**
* CDevEncUiApplication application class.
* Provides factory to create concrete document object.
* An instance of CDevEncUiApplication is the application part of the
* AVKON application framework for the DEVENCUI example application.
*/
class CDevEncUiApplication : public CAknApplication
    {
    public: // Functions from base classes

        /**
        * From CApaApplication, AppDllUid.
        * @return Application's UID (KUidDEVENCUIApp).
        */
        TUid AppDllUid() const;

    protected: // Functions from base classes

        /**
        * From CApaApplication, CreateDocumentL.
        * Creates CDevEncUiDocument document object. The returned
        * pointer in not owned by the CDevEncUiApplication object.
        * @return A pointer to the created document object.
        */
        CApaDocument* CreateDocumentL();
    };

#endif // __DEVENCUIAPPLICATION_H__

// End of File
