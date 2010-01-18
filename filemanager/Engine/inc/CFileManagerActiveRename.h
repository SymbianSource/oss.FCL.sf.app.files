/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Class wraps rename operation
*
*/



#ifndef CFILEMANAGERACTIVERENAME_H
#define CFILEMANAGERACTIVERENAME_H

// INCLUDES
#include <e32std.h>
#include "CFileManagerActiveBase.h"

// FORWARD DECLARATIONS
class RFs;
class CFileManagerEngine;
class CFileManagerUtils;

// CLASS DECLARATION
/**
* Class wraps rename operation.
*
*  @lib FileManagerEngine.lib
*  @since 3.2
*/
NONSHARABLE_CLASS( CFileManagerActiveRename ) : public CFileManagerActiveBase
    {
    public:
        /**
        * Constructor.
        */
        static CFileManagerActiveRename* NewL(
            CFileManagerEngine& aEngine,
            CFileManagerUtils& aUtils,
            const TDesC& aName,
            const TDesC& aNewName );

        /**
        * Destructor.
        */
        ~CFileManagerActiveRename();

    private: // From CFileManagerActiveBase
        void ThreadFunctionL( const TBool& aCanceled );
        
        void CancelThreadFunction();

    private:
        /**
        * C++ default constructor.
        */
        CFileManagerActiveRename(
            CFileManagerEngine& aEngine,
            CFileManagerUtils& aUtils );

        void ConstructL( const TDesC& aName, const TDesC& aNewName );

        TInt ThreadGetMaxSubfolderPathL( const TBool& aCanceled );

    private: // Data
        // Ref: File Manager engine
        CFileManagerEngine& iEngine;

        // Ref: File Manager utils
        CFileManagerUtils& iUtils;

        // Ref: Shareable file server session
        RFs& iFs;

        // Own: Stores name.
        HBufC* iName;

        // Own: Stores new name.
        HBufC* iNewName;

        // Indicates if operation is done in remote drive
        TBool iIsRemoteDrive;
    };

#endif      // CFILEMANAGERACTIVERENAME_H
            
// End of File

