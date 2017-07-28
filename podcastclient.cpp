#include "podcastclient.h"

QStringList PodcastClient::getFeedsFromSettings()
{
  QStringList resultList = settings.value("feeds").toStringList();
  if(resultList.isEmpty())
  {
    QString string = settings.value("feeds").toString();
    if(!string.isEmpty())
      resultList.push_back(string);
  }
  return resultList;
}

PodcastClient::PodcastClient(QObject *parent) : QObject(parent)
{
  if(settings.value("NumDownloads").isNull())
    settings.setValue("NumDownloads",10);
  if(settings.value("Dest").isNull())
    settings.setValue("Dest",".");
  else
  {
    if(!QDir(settings.value("Dest").toString()).exists())
    {
      settings.setValue("Dest",",");
    }
  }
  downloader.setMaxConnections(settings.value("NumDownloads").toInt());
  foreach(QString url, getFeedsFromSettings())
  {
    Podcast* podcast = new Podcast(QUrl(url), &downloader, this);
    podcast->setTargetFolder(QDir(settings.value("Dest").toString()));
    podcasts.push_back(podcast);
    connect(podcast,&Podcast::done,this,&PodcastClient::podcastDone);
  }
}

bool PodcastClient::downloadAll()
{
  if(podcasts.isEmpty())
  {
    out << "No podcasts in list. Done." << endl;
    return true;
  }
  finishedCtr = 0;
  foreach(Podcast* podcast, podcasts)
  {
    podcast->update();
  }
  return false;
}

bool PodcastClient::addPodcast(const QUrl &url, const QString &mode)
{
  podcasts.clear();
  finishedCtr = 0;
  if(!url.isValid())
  {
    out << "Invalid URL." << endl;
    return true;
  }
  if(mode=="last" || mode.isEmpty())
  {
    Podcast* podcast = new Podcast(url, &downloader, this);
    podcasts.push_back(podcast);
    connect(podcast,&Podcast::done,this,&PodcastClient::podcastDone);
    podcast->init(true);
    QStringList feeds;
    foreach(QString url, getFeedsFromSettings())
    {
      feeds.push_back(url);
    }
    feeds.push_back(url.toString());
    settings.setValue("feeds",feeds);
    return false;
  }
  else if(mode=="all")
  {
    QStringList feeds;
    foreach(QString url, getFeedsFromSettings())
    {
      feeds.push_back(url);
    }
    feeds.push_back(url.toString());
    settings.setValue("feeds",feeds);
    return true;
  }
  else if(mode=="none")
  {
    Podcast* podcast = new Podcast(url, &downloader, this);
    podcasts.push_back(podcast);
    connect(podcast,&Podcast::done,this,&PodcastClient::podcastDone);
    podcast->init(false);
    QStringList feeds;
    foreach(QString url, getFeedsFromSettings())
    {
      feeds.push_back(url);
    }
    feeds.push_back(url.toString());
    settings.setValue("feeds",feeds);
    return false;
  }
  else
  {
    out << "Invalid adding mode: " << mode << endl;
    out << "Modes are: last, all, none" << endl;
    return true;
  }
}

void PodcastClient::removePodcast(const QUrl &url)
{
  QStringList feeds;
  foreach(QString url, getFeedsFromSettings())
  {
    feeds.push_back(url);
  }
  feeds.removeAll(url.toString());
  if(feeds.isEmpty())
    settings.remove("feeds");
  else
    settings.setValue("feeds",feeds);
  QCoreApplication::exit(0);
}

void PodcastClient::setDest(const QString &dest)
{
  QDir dir(dest);
  if(dir.exists())
  {
    settings.setValue("Dest",dest);
    settings.sync();
  }
  else
    out << "Target folder does not exist." << endl;
}

QString PodcastClient::getDest()
{
  return settings.value("Dest").toString();
}

void PodcastClient::list()
{
  foreach(QString url, getFeedsFromSettings())
  {
    out << url << endl;
  }
}

void PodcastClient::setMaxDownloads(int num)
{
  downloader.setMaxConnections(num);
  settings.setValue("NumDownloads",num);
  settings.sync();
}

int PodcastClient::getMaxDownloads()
{
  return downloader.getMaxConnections();
}

void PodcastClient::podcastDone()
{
  finishedCtr++;
  if(finishedCtr==podcasts.size())
  {
    settings.sync();
    QCoreApplication::exit(0);
  }
}

