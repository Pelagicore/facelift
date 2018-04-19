#pragma once

#include <QObject>

#include "FaceliftProperty.h"

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
        filesChanged();
    }

    QVector<MediaFile> m_files;

};
