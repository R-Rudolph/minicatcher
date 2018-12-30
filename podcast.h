#ifndef PODCAST_H
#define PODCAST_H

#include <QObject>
#include <QString>
#include <QSet>
#include <QUrl>
#include <QDir>
#include <QJsonObject>
#include <QDebug>
#include <QSettings>
#include <QCoreApplication>
#include <QNetworkReply>
#include <QXmlStreamReader>
#include "output.h"

class Episode;
class Downloader;

class Podcast : public QObject
{
  enum Mode
  {
    LOAD,
    INIT_LAST,
    INIT_NONE
  };
  Q_OBJECT
  Mode mode;
  Downloader* downloader;
  QString title;
  QUrl url;
  QDir targetFolder;
  QStringList episodeTitlesKnown;
  QList<Episode*> episodes;

  void newEpisode(const QUrl &url, QString podcastTitle, const QString& episodeTitle, const QString& id);
  void parseRssLevel(QXmlStreamReader& xml);
  void parseChannelLevel(QXmlStreamReader& xml);
  void parseItemLevel(QXmlStreamReader &xml, const QString& podcastTitle);
  void irrelevantElement(QXmlStreamReader& xml);
public:
  explicit Podcast(QUrl url, Downloader* downloader, QObject *parent = 0);
  void init(bool keepNewestEntry);
  QUrl getUrl() const;
  void setUrl(const QUrl &value);
  void abort();

signals:
  void done();
  void writingFailed();
public slots:
  void update();
  void setTargetFolder(const QDir &value);
private slots:
  void episodeDownloadSuccess();
  void episodeDownloadFailed();
  void feedDownloadSuccess(const QUrl& url, const QByteArray& data);
  void feedDownloadFailed(const QUrl& url, QNetworkReply::NetworkError error);
  void load();
};

#endif // PODCAST_H
