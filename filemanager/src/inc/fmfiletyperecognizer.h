/*
 * Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * This component and the accompanying materials are made available
 * under the terms of "Eclipse Public License v1.0"
 * which accompanies this distribution, and is available
 * at the URL "http://www.eclipse.org/legal/epl-v10.html".
 *
 * Initial Contributors:
 * Nokia Corporation - initial contribution.
 * 
 * Contributors:
 *     
 * 
 * Description:
 *      The header file of the file type recognizer
 */

#ifndef FMFILERECOGNIZER_H
#define FMFILERECOGNIZER_H

#include <QString>
#include <QList>
#include <QStringList>
#include <QMap>

/*!
    \class FmFileTypeRecognizer
    \brief The class FmFileTypeRecognizer used to recognize file type by path.
 */
class FmFileTypeRecognizer
{
public:
    enum FileType{
        FileTypeDrive,
        FileTypeFolder,
        FileTypeImage,
        FileTypeVideo,
        FileTypeTone,
        FileTypePlaylist, 
        FileTypeText,
        FileTypeSisx,
        FileTypeJava, 
        FileTypeFlash, 
        FileTypeWidget, 
        FileTypeWebLink, 
        FileTypeUnKnown
    };


public:
    FmFileTypeRecognizer();
    virtual ~FmFileTypeRecognizer();
    
    /*!
     Recognize file type, currently only judge file by its extension name.
     */
    FmFileTypeRecognizer::FileType getType( const QString& path ) const ;
    
    /*!
     Profide a list of extension name by designate the FileType.
     */
    const QStringList getFileListFromFileType( const FmFileTypeRecognizer::FileType fileType ) const;
    
private:    
    // used to store FileType, extension name list pare.
    QMap<FileType, QStringList> mFileTypeMap;
    
    // used to store single extension name, FileType pare.
    // this map comes from mFileTypeMap
    QMap<QString, FileType>     mFileExtensionMap;
};

#endif /* FMDRIVEDETAILSTYPE_H */

