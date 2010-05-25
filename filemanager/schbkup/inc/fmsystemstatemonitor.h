/*
* Copyright (c) 2002-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  system State monitor
*
*/

#ifndef __FM_SYSTEMSTATE_MONITOR__
#define __FM_SYSTEMSTATE_MONITOR__

#include <e32base.h>
#include <ssm/ssmstateawaresession.h>

/**
 * MFmSystemStateMonitorObserver
 * 
 * Monitor component observer.
 * 
 * @since S60 5.2
 */
class MFmSystemStateMonitorObserver
    {    
    public:
        /**
         * Called when the system State changed  
         */
        virtual void SystemStateChangedEvent() = 0;
    };

/**
 * CFmSystemStateMonitor
 * 
 * Component for monitoring system state
 * 
 * @since S60 5.2
 */  
class CFmSystemStateMonitor : public CActive
    {
    public:
        /**
         * Create new monitor
         * 
         * @param aObserver monitor observer
         * @since S60 5.2     
         */    
        static CFmSystemStateMonitor* NewL(
            MFmSystemStateMonitorObserver& aObserver );   
        
        /**
         * Destructor
         * 
         * @since S60 5.2     
         */    
        ~CFmSystemStateMonitor();
    
        /**
         * Start monitor
         * 
         * @param aStateToMonitor the state needs monitored
         * @since S60 5.2
         */        
        void StartMonitor( TUint aStateToMonitor );    
        
        /**
         * Indicate system state is normal or not
		 *       
         * @since S60 5.2
		 * @return ETrue if system state is normal, EFalse if not
         */ 
        TBool IsSystemStateNormal();

    private:
        /** 
         * C++ constructor
         *
         * @param aObserver monitor observer.   
         * @since S60 5.2      
         */        
        CFmSystemStateMonitor( MFmSystemStateMonitorObserver& aObserver );    

        /**
         * Symbian 2nd phase constructor
         * 
         * @since S60 5.2
         */        
        void ConstructL();

    private: // From CActive
        /**
         * From CActive
         */        
        void RunL();

        /**
         * From CActive         
         * @since S60 5.2
         */        
        void DoCancel(); 

        /**
         * From CActive         
         * @since S60 5.2
         */        
        TInt RunError( TInt aError );

    private:	
        // observer used to inform try schedule backup again when system state become normal
        MFmSystemStateMonitorObserver& iObserver;

        // State aware session.
        RSsmStateAwareSession iSAS;
        
        // State to monitor
        TUint iStateToMonitor;
    };

#endif // __FM_SYSTEMSTATE_MONITOR__
