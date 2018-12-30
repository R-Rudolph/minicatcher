#include "podcast.h"
#include "episode.h"
#include "downloader.h"

void Podcast::setTargetFolder(const QDir &value)
{
  targetFolder = value;
}

QUrl Podcast::getUrl() const
{
  return url;
}

void Podcast::setUrl(const QUrl &value)
{
  url = value;
}

void Podcast::abort()
{
  foreach(Episode* episode, episodes)
  {
    disconnect(episode,&Episode::complete,this,&Podcast::episodeDownloadSuccess);
    disconnect(episode,&Episode::failed,this,&Podcast::episodeDownloadFailed);
    episode->abort();
    episode->deleteLater();
  }
  episodes.clear();
  emit done();
}

void Podcast::newEpisode(const QUrl &url, QString podcastTitle, const QString& episodeTitle, const QString& id)
{
  if(episodeTitlesKnown.contains(episodeTitle))
  {
    episodeTitlesKnown.removeAll(episodeTitle);
    episodeTitlesKnown.push_back(id);
    return;
  }
  else if(episodeTitlesKnown.contains(id))
  {
    return;
  }
  podcastTitle.replace(QRegExp("[\\/\\\\?%\\*:\\|<>]")," ");
  Episode* ep = new Episode(podcastTitle,episodeTitle,url,id,downloader,this);
  episodes.append(ep);
  connect(ep,&Episode::complete,this,&Podcast::episodeDownloadSuccess);
  connect(ep,&Episode::failed,this,&Podcast::episodeDownloadFailed);
}

void Podcast::parseRssLevel(QXmlStreamReader &xml)
{
  while(!xml.atEnd())
  {
    QXmlStreamReader::TokenType ttype = xml.readNext();
    switch(ttype)
    {
      case QXmlStreamReader::StartElement:
        if(xml.name()=="channel")
          parseChannelLevel(xml);
        else
          xml.skipCurrentElement();
        break;
      case QXmlStreamReader::EndElement:
        return;
      case QXmlStreamReader::Invalid:
        return;
      default:
        break;
    }
  }
}

void Podcast::parseChannelLevel(QXmlStreamReader &xml)
{
  QString channelName;
  while(!xml.atEnd())
  {
    QXmlStreamReader::TokenType ttype = xml.readNext();
    switch(ttype)
    {
      case QXmlStreamReader::StartElement:
        if(xml.name()=="item")
          parseItemLevel(xml,channelName);
        else if(xml.name()=="title")
          channelName = xml.readElementText();
        else
          xml.skipCurrentElement();
        break;
      case QXmlStreamReader::EndElement:
        return;
      case QXmlStreamReader::Invalid:
        return;
      default:
        break;
    }
  }
}

void Podcast::parseItemLevel(QXmlStreamReader &xml, const QString& podcastTitle)
{
  QString url;
  QString title;
  QString id;
  while(!xml.atEnd())
  {
    QXmlStreamReader::TokenType ttype = xml.readNext();
    switch(ttype)
    {
      case QXmlStreamReader::StartElement:
        if(xml.name()=="title")
          title = xml.readElementText();
        else if(xml.name()=="enclosure")
        {
          foreach(const QXmlStreamAttribute &attr, xml.attributes())
          {
            if(attr.name().toString() == QLatin1String("url"))
            {
              url = attr.value().toString();
            }
          }
          xml.skipCurrentElement();
        }
        else if(xml.name()=="guid")
        {
          id = xml.readElementText();
        }
        else
          xml.skipCurrentElement();
        break;
      case QXmlStreamReader::EndElement:
        if(!url.isNull() && !title.isNull())
        {
          if(id.isEmpty())
          {
            newEpisode(url,podcastTitle,title,url);
          }
          else
          {
            newEpisode(url,podcastTitle,title,id);
          }
        }
        return;
      case QXmlStreamReader::Invalid:
        return;
      default:
        break;
    }
  }
}

Podcast::Podcast(QUrl url, Downloader *downloader, QObject *parent) : QObject(parent)
{
  mode = LOAD;
  this->url = url;
  this->downloader = downloader;
  connect(downloader,&Downloader::downloadSuccess,this,&Podcast::feedDownloadSuccess);
  connect(downloader,&Downloader::downloadFailed,this,&Podcast::feedDownloadFailed);
  QSettings settings;
  QList<QVariant> knownList = settings.value("KnownEpisodes/"+url.toString(QUrl::EncodeReserved)).toList();
  foreach(QVariant entry, knownList)
  {
    episodeTitlesKnown.push_back(entry.toString());
  }
}

void Podcast::init(bool keepNewestEntry)
{
  if(keepNewestEntry)
    mode = INIT_LAST;
  else
    mode = INIT_NONE;
  update();
}

void Podcast::update()
{
  downloader->load(url,"Feed:"+getUrl().toString());
}

void Podcast::episodeDownloadSuccess()
{
  Episode* episode = (Episode*) QObject::sender();
  if(episode->save(targetFolder))
  {
    episodeTitlesKnown.push_back(episode->getEpisodeTitle());
  }
  else
  {
    out << "Could not save " << episode->getUrl().toString() << " to file." << endl;
    emit writingFailed();
  }
  episodes.removeAll(episode);
  episode->deleteLater();
  if(episodes.isEmpty())
  {
    QSettings settings;
    settings.setValue("KnownEpisodes/"+url.toString(QUrl::EncodeReserved),episodeTitlesKnown);
    emit done();
  }
}

void Podcast::episodeDownloadFailed()
{
  Episode* episode = (Episode*) QObject::sender();
  out << "Could not download " << episode->getUrl().toString() << endl;
  episodes.removeAll(episode);
  episode->deleteLater();
  if(episodes.isEmpty())
  {
    emit done();
  }
}

void Podcast::feedDownloadSuccess(const QUrl& url, const QByteArray& data)
{
  if(this->url!=url)
    return;
  QXmlStreamReader xml(data);
  if(xml.hasError())
  {
    return;
  }
  while(!xml.atEnd())
  {
    QXmlStreamReader::TokenType ttype = xml.readNext();
    switch(ttype)
    {
      case QXmlStreamReader::StartElement:
          if(xml.name()=="rss")
            parseRssLevel(xml);
          break;
      default:
        break;
    }
  }
  if(mode==LOAD)
    load();
  else if(mode==INIT_LAST)
  {
    episodes.removeFirst();
    foreach(Episode* ep, episodes)
    {
      episodeTitlesKnown.push_back(ep->getId());
    }
    QSettings settings;
    settings.setValue("KnownEpisodes/"+url.toString(QUrl::EncodeReserved),episodeTitlesKnown);
    emit done();
  }
  else if(mode==INIT_NONE)
  {
    foreach(Episode* ep, episodes)
    {
      episodeTitlesKnown.push_back(ep->getId());
    }
    QSettings settings;
    settings.setValue("KnownEpisodes/"+url.toString(QUrl::EncodeReserved),episodeTitlesKnown);
    emit done();
  }
}

void Podcast::feedDownloadFailed(const QUrl& url, QNetworkReply::NetworkError)
{
  if(this->url!=url)
    return;
  out << "Error: Could not load feed " << url.toString() << endl;
  emit done();
}

void Podcast::load()
{
  if(episodes.size()==0)
    emit done();
  foreach(Episode* ep, episodes)
    ep->load();
}
