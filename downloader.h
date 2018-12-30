#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QQueue>
#include <QMap>
#include <QTimer>
#include <QNetworkReply>
#include "output.h"

struct Download
{
  QUrl url;
  QString description;
  QNetworkReply* reply;
  bool operator ==(const Download& other);
};

class Downloader : public QObject
{
  Q_OBJECT
  int lastLength;
  QTimer timer;
  QList<Download> downloadQueue;
  QNetworkAccessManager* manager;
  QMap<QNetworkReply*,QUrl> replyUrlMap;
  QMap<QUrl,QUrl> redirectMapping;
  int maxConnections;
  int numConnections;
  QList<Download> currentDownloads;
  void continueQueue();
  void printProgress();
  QUrl getUnredirectedUrl(QUrl url);
  int getDownloadIndexByReply(QNetworkReply *reply);
public:
  explicit Downloader(QObject *parent = 0);
  void load(const QUrl& url, const QString& description);
  int getMaxConnections() const;
  void setMaxConnections(int value);
  void abort(const QUrl& url);
signals:
  void downloadSuccess(const QUrl& url, const QByteArray& data);
  void downloadFailed(const QUrl& url, QNetworkReply::NetworkError error);
  void queueEmpty();
private slots:
  void downloadFinished(QNetworkReply* reply);
};

#endif // DOWNLOADER_H
