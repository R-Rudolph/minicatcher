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
    out << endl;
  }
}

void Downloader::printProgress()
{
  if(replies.size()>0)
  {
    int newLength = 0;
    foreach(QNetworkReply* reply,replies)
    {
      if(!reply->header(QNetworkRequest::ContentLengthHeader).isNull())
      {
        int part = reply->bytesAvailable();
        int total = reply->header(QNetworkRequest::ContentLengthHeader).toInt();
        int percent = 100*((float)part)/total;
        out << percent << "%|";
        newLength += 2 + QString::number(percent).length();
      }
      else
      {
        int part = reply->bytesAvailable();
        out << part/1000000 << "MB|";
        newLength += 3 + QString::number(part/1000000).length();
      }
    }
    out << downloadQueue.size() << " queued";
    newLength += 7 + QString::number(downloadQueue.size()).length();
    if(newLength<lastLength)
    {
      out << QString(lastLength-newLength,' ');
    }
    lastLength = newLength;
    out.flush();
    out << "\r";
  }
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
