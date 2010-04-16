#
# Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
# All rights reserved.
# This component and the accompanying materials are made available
# under the terms of "Eclipse Public License v1.0"
# which accompanies this distribution, and is available
# at the URL "http://www.eclipse.org/legal/epl-v10.html".
#
# Initial Contributors:
# Nokia Corporation - initial contribution.
#
# Contributors:
#
# Description: 
#

INTERNAL_HEADERS += $$PWD/fmutils.h \
                    $$PWD/fmdrivemodel.h
                    
INTERNAL_SOURCES += $$PWD/fmdrivemodel.cpp 

symbian {
    INTERNAL_HEADERS += $$PWD/fms60utils.h

    INTERNAL_SOURCES += $$PWD/fmutils_s60.cpp \
                        $$PWD/fms60utils.cpp
}
win32 {
    INTERNAL_SOURCES += $$PWD/fmutils_win.cpp
}

HEADERS += $$INTERNAL_HEADERS
SOURCES += $$INTERNAL_SOURCES