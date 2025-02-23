#ifndef IMG_GMAP_H
#define IMG_GMAP_H

#include "mapdata.h"

class QXmlStreamReader;
class QDir;

namespace IMG {

class GMAPData : public MapData
{
public:
	GMAPData(const QString &fileName);
	~GMAPData();

private:
	bool readXML(const QString &path, QString &dataDir, QString &typFile,
	  QString &baseMap);
	void mapProduct(QXmlStreamReader &reader, QString &dataDir,
	  QString &typFile, QString &baseMap);
	void subProduct(QXmlStreamReader &reader, QString &dataDir,
	  QString &baseMap);
	bool loadTile(const QDir &dir, bool baseMap);

	QList<const QString*> _files;
};

}

#endif // IMG_GMAP_H
