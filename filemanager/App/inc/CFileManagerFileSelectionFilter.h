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
* Description:  Filters PlatSec dirs from file selection
*
*/



#ifndef CFILEMANAGERFILESELECTIONFILTER_H
#define CFILEMANAGERFILESELECTIONFILTER_H

//  INCLUDES
#include <e32base.h>
#include <MAknFileFilter.h>

// FORWARD DECLARATIONS
class CFileManagerEngine;

// CLASS DECLARATION
/**
 *
 */
class CFileManagerFileSelectionFilter : public CBase,
                                        public MAknFileFilter
    {
    public:
        CFileManagerFileSelectionFilter( CFileManagerEngine& aEngine );

    public: // From MAknFileFilter
        TBool Accept( const TDesC& aDriveAndPath, const TEntry& aEntry ) const;

    private: // Data
        /// Ref: Reference to engine of the application.
        CFileManagerEngine& iEngine;
    };

#endif // CFILEMANAGERFILESELECTIONFILTER_H

// End of File
