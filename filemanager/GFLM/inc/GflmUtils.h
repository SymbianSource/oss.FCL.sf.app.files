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
* Description:  Static utility class for GFLM
*
*/



#ifndef GFLMUTILS_H
#define GFLMUTILS_H


//  INCLUDES
#include <e32def.h>
#include <e32std.h>
#include "FileManagerDebug.h"

// FORWARD DECLARATIONS
class TEntry;

// CLASS DECLARATION
/**
*  GflmUtils is a static utility class
*
*  @lib GFLM.lib
*  @since 2.0
*/
NONSHARABLE_CLASS(GflmUtils)
    {
    public:

        /**
        * Returns the index of a Nth character in a descriptor. The search
        * is started from the end of the descriptor
        * @since 2.0
        * @param aDescriptor The descriptor upon which the search is made
        * @param aChar The character that is located
        * @param aNth Tells how many matches are made
        * @return Index to the located character. KErrNotFound if the character
        *         was not found. KErrArgument if aNth was <= 0
        */
        static TInt LocateReverseNth(
            const TDesC& aDescriptor, TChar aChar, const TInt aNth);

        /**
        * Ensures that final backslash is set
        * @since 3.1
        * @param aPath
        */
        IMPORT_C static void EnsureFinalBackslash( TDes& aPath );

        /**
        * Strips final backslash if it exists
        * @since 3.1
        * @param aPath
        * @return Pointer to stripped path
        */
        IMPORT_C static TPtrC StripFinalBackslash( const TDesC& aPath );

        /**
        * Checks if final backslash exist
        * @since 3.2
        * @param aPath Directory path
        */
        IMPORT_C static TBool HasFinalBackslash( const TDesC& aPath );

        /**
        * Gets item full path
        * @since 3.2
        * @param aDir Parent directory path
        * @param aEntry File system entry
        * @param aFullPath Full path to item
        */
        IMPORT_C static void GetFullPath(
            const TDesC& aDir, const TEntry& aEntry, TDes& aFullPath );

        /**
        * Gets item full path
        * @since 3.2
        * @param aDir Parent directory path
        * @param aEntry File system entry
        * @return Pointer to full path
        */
        IMPORT_C static HBufC* FullPathLC(
            const TDesC& aDir, const TEntry& aEntry );

        /**
        * Gets item full path
        * @since 3.2
        * @param aDir Parent directory path
        * @param aEntry File system entry
        * @return Pointer to full path
        */
        IMPORT_C static HBufC* FullPathL(
            const TDesC& aDir, const TEntry& aEntry );

        /**
        * Strips final dot if it exists
        * @since 3.1
        * @param aPath
        * @return Pointer to stripped path
        */
        static TPtrC StripFinalDot( const TDesC& aPath );

        /**
        * Strips given string from end if it exists
        * @since 3.2
        * @param aString
        * @param aStringToStrip
        * @return Pointer to stripped string
        */
        static TPtrC StripFinal( const TDesC& aString, const TDesC& aStringToStrip );

        /**
        * Checks if string contains any wild cards
        * @since 3.2
        * @param aString
        * @return ETrue if string contains a wild card. Otherwise EFalse.
        */
        static TBool HasWildCard( const TDesC& aString );

    };

#endif // GFLMUTILS_H

// End of File
