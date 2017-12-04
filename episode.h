#ifndef EPISODE_H
#define EPISODE_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QByteArray>
#include <QUrl>
#include <QNetworkReply>
#include <QFile>
#include <QDir>
#include "output.h"

class Downloader;

class Episode : public QObject
{
  Q_OBJECT
  QUrl url;
  QString podcastName;
  QString episodeTitle;
  QString filename;
  QByteArray data;
  Downloader* downloader;
public:
  explicit Episode(const QString& podcastName, const QString& episodeTitle, const QUrl& url, Downloader* downloader, QObject *parent = 0);
  void load();
  bool save(QDir directory);
  QUrl getUrl() const;
  QString getEpisodeTitle() const;
  void setEpisodeTitle(const QString &value);
  void abort();

signals:
  void complete();
  void failed();
private slots:
  void downloadSuccess(const QUrl& url, const QByteArray& data);
  void downloadFailed(const QUrl& url, QNetworkReply::NetworkError error);
};

#endif // EPISODE_H
