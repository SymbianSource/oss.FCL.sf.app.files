/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:The class is used for the unit test cases.
* 
*/

#ifndef TESTCLASSDATAFMDRIVELISTPROVIDER_H
#define TESTCLASSDATAFMDRIVELISTPROVIDER_H
#include "fmdrivemodel.h"

class TestDataClassFmDriveListProvider: public FmDriveListProvider
{
public:
    TestDataClassFmDriveListProvider();
    ~TestDataClassFmDriveListProvider();
    virtual void getDriveList( QStringList &driveList );
    
};

#endif //TESTCLASSDATAFMDRIVELISTPROVIDER_H

TestDataClassFmDriveListProvider::TestDataClassFmDriveListProvider()
{
    
}

TestDataClassFmDriveListProvider::~TestDataClassFmDriveListProvider()
{
    
}

void TestDataClassFmDriveListProvider::getDriveList(QStringList &driveList)
{
    driveList<<"drive1:"
             <<"drive2:"
             <<"drive3:";             
}
