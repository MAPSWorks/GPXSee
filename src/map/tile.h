#ifndef TILE_H
#define TILE_H

#include <QVariant>
#include <QPixmap>
#include <QPoint>
#include <QDebug>
#include "rectd.h"

class FetchTile
{
public:
	FetchTile() {}
	FetchTile(const QPoint &xy, const QVariant &zoom, const RectD &bbox = RectD())
	  : _xy(xy), _zoom(zoom), _bbox(bbox) {}

	const QVariant &zoom() const {return _zoom;}
	const QPoint &xy() const {return _xy;}
	const RectD &bbox() const {return _bbox;}
	QPixmap& pixmap() {return _pixmap;}

private:
	QPoint _xy;
	QVariant _zoom;
	RectD _bbox;
	QPixmap _pixmap;
};

class RenderTile
{
public:
	RenderTile(const QPoint &xy, const QByteArray &data, const QString &key)
	  : _xy(xy), _data(data), _key(key) {}

	const QPoint &xy() const {return _xy;}
	const QString &key() const {return _key;}
	const QPixmap &pixmap() const {return _pixmap;}

	void load() {_pixmap.loadFromData(_data);}

private:
	QPoint _xy;
	QByteArray _data;
	QString _key;
	QPixmap _pixmap;
};

#ifndef QT_NO_DEBUG
inline QDebug operator<<(QDebug dbg, const FetchTile &tile)
{
	dbg.nospace() << "Tile(" << tile.zoom() << ", " << tile.xy() << ", "
	  << tile.bbox() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG

#endif // TILE_H
