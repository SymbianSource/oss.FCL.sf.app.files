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
* Description:  Implementation of application entry point functions.
*
*/

// INCLUDE FILES
#include <eikstart.h>
#include "DevEncUiApplication.h"


// -----------------------------------------------------------------------------
// NewApplication()
//
// -----------------------------------------------------------------------------
//
LOCAL_C CApaApplication* NewApplication()
	{
	return new CDevEncUiApplication;
	}

// -----------------------------------------------------------------------------
// E32Main()
//
// -----------------------------------------------------------------------------
//
GLDEF_C TInt E32Main()
	{
	return EikStart::RunApplication( NewApplication );
	}
