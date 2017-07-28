#include "downloader.h"

int Downloader::getMaxConnections() const
{
  return maxConnections;
}

void Downloader::setMaxConnections(int value)
{
  maxConnections = value;
}

void Downloader::continueQueue()
{
  if(!downloadQueue.isEmpty())
  {
    while(currentDownloads.size()<maxConnections)
    {
      Download dl = downloadQueue.dequeue();
      load(dl.url,dl.description);
    }
  }
  else
  {
    emit queueEmpty();
  }
}

void Downloader::printProgress()
{
  if(currentDownloads.size()>0)
  {
    foreach(Download dl,currentDownloads)
    {
      if(!dl.reply->header(QNetworkRequest::ContentLengthHeader).isNull())
      {
        int part = dl.reply->bytesAvailable();
        int total = dl.reply->header(QNetworkRequest::ContentLengthHeader).toInt();
        int percent = 100*((float)part)/total;
        QString percentString = QString::number(percent) + "% ";
        out << percentString;
        for(int i=0;i<5-percentString.length();i++)
          out << " ";
        out << dl.description << endl;
      }
      else
      {
        int part = dl.reply->bytesAvailable();
        QString sizeString = QString::number(part/1000000) + "MB ";
        out << sizeString;
        for(int i=0;i<5-sizeString.length();i++)
          out << " ";
        out << dl.description << endl;
      }
    }
    out << downloadQueue.size() << " queued" << "\n\n";
    out.flush();
  }
}

QUrl Downloader::getUnredirectedUrl(QUrl url)
{
  while(redirectMapping.contains(url))
  {
    url = redirectMapping[url];
  }
  return url;
}

int Downloader::getDownloadIndexByReply(QNetworkReply *reply)
{
  for(int i=0;i<currentDownloads.size();i++)
  {
    if(currentDownloads[i].reply==reply)
      return i;
  }
  return -1;
}

Downloader::Downloader(QObject *parent) : QObject(parent)
{
  lastLength = 0;
  connect(&timer,&QTimer::timeout,this,&Downloader::printProgress);
  timer.setInterval(1000);
  timer.setSingleShot(false);
  manager = new QNetworkAccessManager(this);
  //manager->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
  connect(manager,&QNetworkAccessManager::finished,this,&Downloader::downloadFinished);
  maxConnections = 5;
  timer.start();
}

void Downloader::load(const QUrl &url, const QString& description)
{
  if(currentDownloads.size()>=maxConnections)
  {
    downloadQueue.enqueue(Download{url,description,nullptr});
    return;
  }
  QNetworkRequest request;
  request.setUrl(QUrl(url));
  QNetworkReply* reply = manager->get(request);
  currentDownloads.append(Download{url,description,reply});
}

void Downloader::downloadFinished(QNetworkReply *reply)
{
  QNetworkReply::NetworkError error = reply->error();
  int downloadIndex = getDownloadIndexByReply(reply);
  if(downloadIndex>=0)
  {
    Download& dl = currentDownloads[downloadIndex];
    if(!reply->attribute(QNetworkRequest::RedirectionTargetAttribute).isNull())
    {
      QUrl newUrl = QUrl(reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString());
      redirectMapping.insert(newUrl,reply->url());
      load(newUrl,dl.description);
    }
    else
    {
      QUrl url = reply->url();
      while(redirectMapping.contains(url))
      {
        url = redirectMapping[url];
      }
      if(error == QNetworkReply::NoError)
      {
        emit downloadSuccess(url,reply->readAll());
      }
      else
      {
        emit downloadFailed(url,error);
      }
    }
    currentDownloads.removeAll(dl);
  }
  reply->deleteLater();
  continueQueue();
}

bool Download::operator ==(const Download &other)
{
  return (url == other.url) &&
         (description == other.description) &&
         (reply == other.reply);
}
