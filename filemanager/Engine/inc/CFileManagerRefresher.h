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
* Description:  Asynchronous refresher for CGflmNavigatorModel
*
*/


#ifndef CFILEMANAGERREFRESHER_H
#define CFILEMANAGERREFRESHER_H

// INCLUDES
#include <e32base.h>
#include "GFLM.hrh"

// FORWARD DECLARATIONS
class CGflmNavigatorModel;
class MFileManagerProcessObserver;

// CLASS DECLARATION

/**
*  This class start asynchronous refreshing of given CGflmNavigatorModel.
*
*  @lib FileManagerEngine.lib
*  @since 2.0
*/
NONSHARABLE_CLASS(CFileManagerRefresher) : public CActive
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CFileManagerRefresher* NewL( CGflmNavigatorModel& aModel );

        /**
        * Destructor.
        */
        virtual ~CFileManagerRefresher();

    public: // New functions

        /**
        * Refreshes the model.
        * @since 2.0
        * @param aRefreshMode Given refresh mode.
        */
        void Refresh( TGflmRefreshMode aRefreshMode = ERefreshItems );

        /**
        * Sets observer
        * @since 2.0
        * @param aObserver Observer for the operation or NULL,
        *                  ownership is not transferred.
        */
        void SetObserver( MFileManagerProcessObserver* aObserver );

        /**
        * Cancels refresh.
        * @since 2.0
        */
        TBool CancelRefresh();

    protected:  // From CActive

        void RunL();

        TInt RunError( TInt aError );

        void DoCancel();

    private:

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        /**
        * C++ default constructor.
        */
        CFileManagerRefresher( CGflmNavigatorModel& aModel );

    private:    // Data

        // Ref: Reference to the model instance that will be refreshed
        CGflmNavigatorModel& iModel;

        // Ref: Pointer to the observer that will be notified
        MFileManagerProcessObserver* iObserver;

    };

#endif // CFILEMANAGERREFRESHER_H

// End of File
