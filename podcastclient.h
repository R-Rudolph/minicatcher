#ifndef PODCASTCLIENT_H
#define PODCASTCLIENT_H

#include <QObject>
#include <QSettings>
#include <QDir>
#include <QList>
#include <QApplication>

#include "podcast.h"
#include "downloader.h"
#include "output.h"

class PodcastClient : public QObject
{
  Q_OBJECT
  QSettings settings;
  Downloader downloader;
  QList<Podcast*> podcasts;

  int finishedCtr;
  QStringList getFeedsFromSettings();
public:
  explicit PodcastClient(QObject *parent = 0);
  bool downloadAll();
  bool addPodcast(const QUrl& url, const QString& mode);
  void removePodcast(const QUrl& url);
  void setDest(const QString& dest);
  QString getDest();
  void list();
  void setMaxDownloads(int num);
  int getMaxDownloads();
signals:

private slots:
  void podcastDone();
};

#endif // PODCASTCLIENT_H
