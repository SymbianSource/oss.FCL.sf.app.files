/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
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
*				Utility class for Memory State Popup.
*
*
*/


#ifndef __CMSPUTIL_H__
#define __CMSPUTIL_H__

//  INCLUDES
#include <e32base.h>
#include <badesca.h>


//  CLASS DEFINITION

class CMSPUtil : public CBase
	{
	public:
		/**
		* Static constructor.
		* @param aArray Array of unit texts.
		* @return Pointer to an instance of CMSPUtil.
		*/
		static CMSPUtil* NewL( CDesCArrayFlat* aArray );

		/**
		* Virtual D'tor.
		*/
		virtual ~CMSPUtil( );

	protected:
		/**
		* Default c'tor, protected to prevent using directly.
		*/
		CMSPUtil( );

	public:
		/**
		* Type of unit.
		*/
		enum TUnitType
			{
			EByte = 0,
			EKiloByte,
			EMegaByte,
			EGigaByte,
			ETeraByte
			};

	public:
		/**
		* Solves the correct unit for given data size.
		* @param aNumber Amount of data to be solved, changed accordingly.
		* @return Unit text descriptor.
		*/
		IMPORT_C TPtrC SolveUnitAndSize( TInt64& aNumber );

	private:
		/**
		* Solves the correct unit for given data size, used internally.
		* @param aNumber Amount of data to be solved, changed accordingly.
		* @param aUnit Unit text index in unit array.
		*/
		void SolveUnitAndSize( TInt64& aNumber, TInt& aUnit );

	private:
		CDesCArrayFlat* iUnitArray;
	};

#endif      //  __CMSPUTIL_H__
