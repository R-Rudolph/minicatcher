#include <QCoreApplication>
#include <QStringList>
#include <QString>
#include "podcastclient.h"

void printHelp()
{
  out << "Usage:" << endl;
  out << " minicatcher [Action]" << endl << endl;
  out << "Actions:" << endl;
  out << "  -f, --fetch            Downloads new episodes. Giving no action also does this" << endl;
  out << "  -l, --list             Lists current sources" << endl;
  out << "  -a, --add <URL> [MODE] Adds URL to sources" << endl;
  out << "  -r, --rem <URL>        Removes URL from sources" << endl;
  out << "  -m, --max-dl [NUM]     Displays (NUM not given) or sets number of parallel downloads" << endl;
  out << "  -d, --dest [DEST]      Displays (DEST not given) or sets download location" << endl << endl;
  out << "Modes:" << endl;
  out << " last: Only most recent episode is considered new" << endl;
  out << " none: None of the episodes are considered new" << endl;
  out << " all: All episodes are consisdered new" << endl;
}

int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);
  QCoreApplication::setOrganizationName("Ro_bat");
  QCoreApplication::setApplicationName("MiniCatcher");
  QStringList args = app.arguments();

  PodcastClient client;
  if(args.count() < 2 || args[1] == "--fetch" || args[1]=="-f")
  {
    if(client.downloadAll())
      return 0;
  }
  else if(args[1] == "--list" || args[1] == "-l")
  {
    client.list();
    return 0;
  }
  else if(args[1] == "--add" || args[1] == "-a")
  {
    if(args.count()<3)
    {
      out << "Missing URL argument." << endl;
      out << "Usage: minicatcher --add <URL> [MODE]" << endl;
      out << "Modes:" << endl;
      out << " last: Only most recent episode is considered new" << endl;
      out << " none: None of the episodes are considered new" << endl;
      out << " all: All episodes are consisdered new" << endl;
      return 1;
    }
    QString mode = "last";
    if(args.count()>3)
      mode = args[3];
    if(client.addPodcast(args[2],mode))
      return 0;
  }
  else if(args[1]=="--rem" || args[1]=="-r")
  {
    if(args.count()<3)
    {
      out << "Missing URL argument." << endl;
      out << "Usage: minicatcher --rem <URL>" << endl;
      return 1;
    }
    client.removePodcast(args[2]);
    return 0;
  }
  else if(args[1]=="--max-dl" || args[1]=="-m")
  {
    if(args.count()<3)
    {
      out << "Max. Connections: "<<client.getMaxDownloads() << endl;
      return 0;
    }
    else
    {
      bool ok;
      int number = args[2].toInt(&ok);
      if(ok && number>0)
      {
        client.setMaxDownloads(number);
        out << "Number of parallel downloads set to " << number << "." << endl;
        return 0;
      }
      else
      {
        out << "Invalid number given." << endl;
        return 1;
      }
    }
  }
  else if(args[1]=="--dest" || args[1]=="-d")
  {
    if(args.count()<3)
    {
      out << "Download Destination: " << client.getDest() << endl;
      return 0;
    }
    else
    {
      client.setDest(args[2]);
      out << "Download Destionation set to: " << args[2] << endl;
      return 0;
    }
  }
  else if(args[1] == "--help" || args[1] == "-h")
  {
    printHelp();
    return 0;
  }
  else
  {
    printHelp();
    return 1;
  }
  return app.exec();
}
