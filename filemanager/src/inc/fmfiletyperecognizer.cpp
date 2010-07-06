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
 *      The source file of the file type recognizer
 */

#include "fmfiletyperecognizer.h"
#include "fmutils.h"
#include <QFileInfo>
#include <QStringList>

FmFileTypeRecognizer::FmFileTypeRecognizer()
{
    QStringList fileExtensionList;
    
    fileExtensionList.append( QString( "bmp" ) );
    fileExtensionList.append( QString( "gif" ) );
    fileExtensionList.append( QString( "jpe" ) );
    fileExtensionList.append( QString( "jpeg" ) );
    fileExtensionList.append( QString( "jpg" ) );
    fileExtensionList.append( QString( "ota" ) );
    fileExtensionList.append( QString( "png" ) );
    fileExtensionList.append( QString( "tif" ) );
    fileExtensionList.append( QString( "tiff" ) );
    fileExtensionList.append( QString( "wbmp" ) );
    fileExtensionList.append( QString( "wmf" ) );
    fileExtensionList.append( QString( "jp2" ) );
    fileExtensionList.append( QString( "jpg2" ) );
    fileExtensionList.append( QString( "jp3" ) );
    fileExtensionList.append( QString( "ico" ) );
    fileExtensionList.append( QString( "vcf" ) );
    mFileTypeMap.insert( FileTypeImage, fileExtensionList );
   
    fileExtensionList.clear();
    fileExtensionList.append( QString( "3gp" ) );
    fileExtensionList.append( QString( "mp4" ) );
    fileExtensionList.append( QString( "nim" ) );
    fileExtensionList.append( QString( "rm"  ) );
    fileExtensionList.append( QString( "rv"  ) );
    fileExtensionList.append( QString( "wmv" ) );
    fileExtensionList.append( QString( "3g2" ) );
    fileExtensionList.append( QString( "rmvb") );
    fileExtensionList.append( QString( "mkv" ) );
    mFileTypeMap.insert( FileTypeVideo, fileExtensionList );
    
    fileExtensionList.clear();
    fileExtensionList.append( QString( "aac" ) );
    fileExtensionList.append( QString( "amr" ) );
    fileExtensionList.append( QString( "au" ) );
    fileExtensionList.append( QString( "awb" ) );
    fileExtensionList.append( QString( "mid" ) );
    fileExtensionList.append( QString( "mp3" ) );
    fileExtensionList.append( QString( "ra" ) );
    fileExtensionList.append( QString( "rmf" ) );
    fileExtensionList.append( QString( "rng" ) );
    fileExtensionList.append( QString( "snd" ) );
    fileExtensionList.append( QString( "wav" ) );
    fileExtensionList.append( QString( "wve" ) );
    fileExtensionList.append( QString( "wma" ) );
    fileExtensionList.append( QString( "m4a" ) );
    fileExtensionList.append( QString( "ott" ) );
    fileExtensionList.append( QString( "mxmf" ) );
    mFileTypeMap.insert( FileTypeTone, fileExtensionList );

    fileExtensionList.clear();
    fileExtensionList.append( QString( "doc" ) );
    fileExtensionList.append( QString( "pdf" ) );
    fileExtensionList.append( QString( "pps" ) );
    fileExtensionList.append( QString( "ppt" ) );
    fileExtensionList.append( QString( "txt" ) );
    fileExtensionList.append( QString( "xls" ) );
    mFileTypeMap.insert( FileTypeText, fileExtensionList );
            
    fileExtensionList.clear();
    fileExtensionList.append( QString( "sis" ) );
    fileExtensionList.append( QString( "sisx" ) );
    mFileTypeMap.insert( FileTypeSisx, fileExtensionList );
    
    fileExtensionList.clear();
    fileExtensionList.append( QString( "jad" ) );
    fileExtensionList.append( QString( "jar" ) );
    mFileTypeMap.insert( FileTypeJava, fileExtensionList );
    
    fileExtensionList.clear();
    fileExtensionList.append( QString( "swf" ) );
    mFileTypeMap.insert( FileTypeFlash, fileExtensionList );

    // have not handle FileTypePlaylist
    // have not handle FileTypeWidget
    // have not handle FileTypeWebLink 
    
    // make mFileExtensionMap( data map for extenstion ) from mFileTypeMap
    // this map is used to speed up recognize    
    QMapIterator<FileType, QStringList> i( mFileTypeMap );
    while (i.hasNext()) {
        i.next();
        foreach( const QString extension, i.value() ) {
            mFileExtensionMap.insert( extension, i.key() );
        }
    }
    

}

FmFileTypeRecognizer::~FmFileTypeRecognizer()
{
    
}

FmFileTypeRecognizer::FileType FmFileTypeRecognizer::getType( const QString& path ) const
{
    if( FmUtils::isDrive( path ) ) {
        return FileTypeDrive;
    }
    
    QFileInfo fileInfo(path);
    if( fileInfo.isDir() ) {
        return FileTypeFolder;
    }
    
    //if can not find key, return default value: FileTypeUnKnown
    return mFileExtensionMap.value( fileInfo.suffix().toLower(), FileTypeUnKnown );
}

const QStringList FmFileTypeRecognizer::getFileListFromFileType( const FmFileTypeRecognizer::FileType fileType ) const
{
    return mFileTypeMap.value( fileType, QStringList() );
}
