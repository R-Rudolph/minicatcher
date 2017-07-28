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
    while(replies.size()<maxConnections)
    {
      load(downloadQueue.dequeue());
    }
  }
  else
  {
    emit queueEmpty();
  }
}

void Downloader::printProgress()
{
  if(replies.size()>0)
  {
    foreach(QNetworkReply* reply,replies)
    {
      if(!reply->header(QNetworkRequest::ContentLengthHeader).isNull())
      {
        int part = reply->bytesAvailable();
        int total = reply->header(QNetworkRequest::ContentLengthHeader).toInt();
        int percent = 100*((float)part)/total;
        QString percentString = QString::number(percent) + "% ";
        out << percentString;
        for(int i=0;i<5-percentString.length();i++)
          out << " ";
        out << getUnredirectedUrl(reply->url()).toString() << endl;
      }
      else
      {
        int part = reply->bytesAvailable();
        QString sizeString = QString::number(part/1000000) + "MB ";
        out << sizeString;
        for(int i=0;i<5-sizeString.length();i++)
          out << " ";
        out << getUnredirectedUrl(reply->url()).toString() << endl;
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

Downloader::Downloader(QObject *parent) : QObject(parent)
{
  lastLength = 0;
  connect(&timer,&QTimer::timeout,this,&Downloader::printProgress);
  timer.setInterval(1000);
  timer.setSingleShot(false);
  manager = new QNetworkAccessManager(this);
  connect(manager,&QNetworkAccessManager::finished,this,&Downloader::downloadFinished);
  maxConnections = 5;
  timer.start();
}

void Downloader::load(const QUrl &url)
{
  if(replies.size()>=maxConnections)
  {
    downloadQueue.enqueue(url);
    return;
  }
  QNetworkRequest request;
  request.setUrl(QUrl(url));
  QNetworkReply* reply = manager->get(request);
  replies.insert(reply);
}

void Downloader::downloadFinished(QNetworkReply *reply)
{
  QNetworkReply::NetworkError error = reply->error();
  if(!reply->attribute(QNetworkRequest::RedirectionTargetAttribute).isNull())
  {
    QUrl newUrl = QUrl(reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString());
    redirectMapping.insert(newUrl,reply->url());
    load(newUrl);
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
  replies.remove(reply);
  reply->deleteLater();
  continueQueue();
}
