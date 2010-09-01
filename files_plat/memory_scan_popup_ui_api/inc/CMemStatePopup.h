/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: 
*				Memory State Popup.
*
*
*/


#ifndef __CMEMORYSTATEPOPUP_H__
#define __CMEMORYSTATEPOPUP_H__

// SYSTEM INCLUDES
#include <e32base.h>

// USER INCLUDES
#include "msputil.h"

//  CONSTANTS
const TText KRightToLeftMark = 0x200F;

// CLASS DEFINITION
class CMemStatePopup :	public CBase
	{
	public:
		//   Destructor
		virtual ~CMemStatePopup( );

	private:
		//   C++ constructors
		CMemStatePopup( );

	public:
		/**
		* Launches the memory scan popup
		* @param aDrive The drive to be scanned
		* @param aTitle Title of the popup
		*/
		IMPORT_C static void RunLD( TDriveNumber aDrive, TDesC& aTitle );

		/**
		* Get an instance of CMSPUtil class, containing the unit text array
		* @param aUtil Reference to a CMSPUtil pointer,
		*		 updated to point to the instance created
		*/
		IMPORT_C static void GetUtilL( CMSPUtil*& aUtil );

	private:
		// Not implemented
		CMemStatePopup( const CMemStatePopup& );
		CMemStatePopup& operator=( CMemStatePopup& );

	private:
	};

#endif      //  __CMEMORYSTATEPOPUP_H__

// End of File
