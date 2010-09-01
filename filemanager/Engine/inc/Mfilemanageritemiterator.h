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
* Description:  Interface to go through file(s) and folder(s) in copy and move
*
*/



#ifndef MFILEMANAGERITEMITERATOR_H
#define MFILEMANAGERITEMITERATOR_H

// INCLUDES
#include "FileManagerEngine.hrh"

// CLASS DECLARATION
/**
*  Defines an interface for iterating file(s) and folder(s).
*
*  @since 2.0
*/
class MFileManagerItemIterator 
	{
	public:

        /**
        * Gets the current processed item source and destination with full path
        * @since 2.0
        * @param aSrc Returns pointer to item source with full path
		*		      if this parameter is null and aDst contains destination folder
		*			  it means that destination is folder which should be created
        * @param aDst Returns pointer to item destination with full path
        */
        virtual void CurrentL(
            HBufC** aSrc,
            HBufC** aDst,
            TFileManagerTypeOfItem& aItemType ) = 0;
		
        /**
        * Goes to next item
        * @since 2.0
        * @return ETrue if next item found, EFalse if end reached
        */
		virtual TBool NextL() = 0;

        /**
        * Virtual desctructor
        * This is required because the ownership transfer does not work without
        */
		virtual ~MFileManagerItemIterator() {}
	};

#endif // MFILEMANAGERITEMITERATOR_H