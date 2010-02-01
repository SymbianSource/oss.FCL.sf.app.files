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
* Description: CMSPUtil Implementation
*
*
*/


//  INCLUDE FILES
#include <e32std.h>
#include "MSPPanic.hrh"
#include "msputil.h"

// ---------------------------------------------------------------------------

CMSPUtil* CMSPUtil::NewL( CDesCArrayFlat* aArray )
	{
	__ASSERT_DEBUG( aArray,
					User::Panic( KPanicMSP, EInvalidParameter ) );
	CMSPUtil* self = new( ELeave ) CMSPUtil;
	self->iUnitArray = aArray;
	return self;
	}

// ---------------------------------------------------------------------------

CMSPUtil::~CMSPUtil( )
	{
	if( iUnitArray )
	    {
	    iUnitArray->Delete( 0, iUnitArray->Count( ) );
	    }
	delete iUnitArray;
	}

// ---------------------------------------------------------------------------

CMSPUtil::CMSPUtil( ): iUnitArray( NULL )
	{
	}

// ---------------------------------------------------------------------------

void CMSPUtil::SolveUnitAndSize( TInt64& aNumber, TInt& aUnit )
	{
	TInt count( iUnitArray->Count( ) );
	const TInt KMaxDigits = 10000;
	const TInt KKiloByte = 1024;
	const TInt KRoundLimit = 512;
	const TInt KShiftValue = 10;
	aUnit = EByte;

	// If number is 1024 or more, it can be divided and unit changed
	while( aNumber >= 1024 && (aUnit + 1) < count )
		{
		// Show kilobytes and megabytes with 4 digits
		if( ( aUnit == EKiloByte || aUnit == EMegaByte ) && aNumber < KMaxDigits )
			{
			break;
			}
		// Check how the rounding should be done
		if( aNumber % KKiloByte < KRoundLimit )
			{
			// Round downwards
			aNumber >>= KShiftValue;	// 2^10 = 1024
			}
		else
			{
			aNumber = ( aNumber >> KShiftValue ) + 1;
			}
		aUnit++;		// Next enum
		}
	}


// ---------------------------------------------------------------------------

EXPORT_C TPtrC CMSPUtil::SolveUnitAndSize( TInt64& aNumber )
	{
	TInt unitType( EByte );
	SolveUnitAndSize( aNumber, unitType );
    if( unitType >= iUnitArray->Count() )
        {
        if( iUnitArray->Count() )
            {
            unitType = iUnitArray->Count() - 1;
            }
        else
            {
            unitType = 0;
            }
        }
        
	return ( *iUnitArray )[ unitType ];
	}

// End of File

