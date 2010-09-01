/*
* Copyright (c) 2006-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Filter GFLM items
*
*/


#ifndef C_FILEMANAGERITEMFILTER_H
#define C_FILEMANAGERITEMFILTER_H


// INCLUDES
#include <e32base.h>
#include <f32file.h>
#include "MGflmItemFilter.h"


// FORWARD DECLARATIONS
class CFileManagerEngine;


// CLASS DECLARATION
/**
*  This class handles GFLM item filtering
*
*  @lib FileManagerEngine.lib
*  @since 3.1
*/
NONSHARABLE_CLASS(CFileManagerItemFilter) : public CBase,
                                            public MGflmItemFilter
    {

public:  // Constructors and destructor
    /**
    * Two-phased constructor.
    */
    static CFileManagerItemFilter* NewL( CFileManagerEngine& aEngine );

    /**
    * Destructor.
    */
    virtual ~CFileManagerItemFilter();

public: // From MGflmItemFilter
    /**
     * @see MGflmItemFilter
     */
    TBool FilterItemL(
        CGflmGroupItem* aItem, TInt aGroupId, CGflmDriveItem* aDrive );

private:
    /**
    * C++ default constructor.
    */
    CFileManagerItemFilter( CFileManagerEngine& aEngine );

    /**
    * By default Symbian 2nd phase constructor is private.
    */
    void ConstructL();

private:    // Data
    /**
     * Reference to file manager engine
     * Not own.
     */
    CFileManagerEngine& iEngine;

    /**
     * For file name handling
     */
    TFileName iFileNameBuffer;

    /**
     * Contains phone memory root path
     * Own.
     */
    HBufC* iPhoneMemoryRootPath;

    /**
     * Contains phone memory default name
     * Own.
     */
    HBufC* iInternalDefaultName;

    /**
     * Contains memory card default name
     * Own.
     */
    HBufC* iRemovableDefaultName;

    };

#endif  // C_FILEMANAGERITEMFILTER_H
            
// End of File
