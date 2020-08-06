/**********************************************************************
**
** Copyright (C) 2018 Luxoft Sweden AB
**
** This file is part of the FaceLift project
**
** Permission is hereby granted, free of charge, to any person
** obtaining a copy of this software and associated documentation files
** (the "Software"), to deal in the Software without restriction,
** including without limitation the rights to use, copy, modify, merge,
** publish, distribute, sublicense, and/or sell copies of the Software,
** and to permit persons to whom the Software is furnished to do so,
** subject to the following conditions:
**
** The above copyright notice and this permission notice shall be
** included in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
** NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
** BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
** ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**
** SPDX-License-Identifier: MIT
**
**********************************************************************/

#pragma once

#include <QObject>

#include "ModelProperty.h"

class MediaIndexerService :
    public QObject
{

    Q_OBJECT

public:
    enum class MediaType {
        Audio,
        Video
    };

    struct MediaFile
    {
        QString title;
        QString artist;
        QString url;
        MediaType type;
    };

    MediaIndexerService()
    {
        addFile("J'attendrai", "Django Reinhardt", "http://www.youtube.com/embed/ANArGmr74u4?autoplay=1", MediaType::Video);
        addFile("West coast blues", "Wes Montgomery", "http://www.youtube.com/embed/TeMr4SQtWGo?autoplay=1", MediaType::Video);
        addFile("Bock to Bock", "Wes Montgomery", "http://www.youtube.com/embed/g3faH74Sba4?autoplay=1", MediaType::Video);
    }

    const QVector<MediaFile> &files() const
    {
        return m_files;
    }

    Q_SIGNAL void filesChanged();

private:
    void addFile(const char *title, const char *artist, const char *url, MediaType type)
    {
        static int i = 0;

        MediaFile s;
        s.title = QString::number(i++) + " - " + title;
        s.artist = artist;
        s.url = url;
        s.type = type;
        m_files.push_back(s);
        emit filesChanged();
    }

    QVector<MediaFile> m_files;

};
