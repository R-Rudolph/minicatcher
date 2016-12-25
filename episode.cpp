#include "episode.h"
#include "downloader.h"

QUrl Episode::getUrl() const
{
  return url;
}

QString Episode::getEpisodeTitle() const
{
  return episodeTitle;
}

void Episode::setEpisodeTitle(const QString &value)
{
  episodeTitle = value;
}

Episode::Episode(const QString& podcastName, const QString &episodeTitle, const QUrl& url, Downloader *downloader, QObject *parent) : QObject(parent)
{
  this->episodeTitle = episodeTitle;
  this->downloader = downloader;
  this->url = url;
  this->filename = url.fileName();
  this->podcastName = podcastName;
  connect(downloader,&Downloader::downloadFailed,this,&Episode::downloadFailed);
  connect(downloader,&Downloader::downloadSuccess,this,&Episode::downloadSuccess);
}

void Episode::load()
{
  downloader->load(url);
}

bool Episode::save(QDir directory)
{
  if(!podcastName.isEmpty())
  {
    directory = QDir(QDir(directory).filePath(podcastName));
    directory.mkpath(".");
  }
  QFile file(directory.filePath(filename));
  if(file.open(QIODevice::WriteOnly))
  {
    file.write(data);
    file.close();
    return true;
  }
  return false;
}

void Episode::downloadSuccess(const QUrl &url, const QByteArray &data)
{
  if(url!=this->url)
    return;
  this->data = data;
  emit complete();
}

void Episode::downloadFailed(const QUrl &url, QNetworkReply::NetworkError)
{
  if(url!=this->url)
    return;
  emit failed();
}
