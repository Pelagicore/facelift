#pragma once

#include "MediaIndexerService.h"
#include "mediaplayer/MediaIndexerModelPropertyAdapter.h"
#include <functional>

using namespace mediaplayer;

class MediaIndexerModelCpp :
    public MediaIndexerModelPropertyAdapter
{

    Q_OBJECT

public:
    MediaIndexerModelCpp(QObject *parent = nullptr) :
        MediaIndexerModelPropertyAdapter(parent)
    {
        m_files.setSize(m_service.files().size());
        m_files.setGetter([this](int index) {
                    const auto &files = m_service.files();
                    MediaFile file;
                    file.settitle(files[index].title);
                    file.setartist(files[index].artist);
                    file.seturl(files[index].url);
                    return file;
                });

        connect(&m_service, &MediaIndexerService::filesChanged, this, &MediaIndexerModelCpp::onFilesChanged);
    }

    void onFilesChanged()
    {
        m_files.setSize(m_service.files().size());
        m_files.notifyDataChanged();
    }

private:
    MediaIndexerService m_service;

};
