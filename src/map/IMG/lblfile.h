#ifndef IMG_LBLFILE_H
#define IMG_LBLFILE_H

#include <QPixmap>
#include "common/textcodec.h"
#include "section.h"
#include "subfile.h"
#include "label.h"

namespace IMG {

class HuffmanText;
class RGNFile;

class LBLFile : public SubFile
{
public:
	LBLFile(const IMGData *img)
	  : SubFile(img), _huffmanText(0), _table(0), _rasters(0), _imgIdSize(0),
	  _poiShift(0), _shift(0), _encoding(0) {}
	LBLFile(const QString *path)
	  : SubFile(path), _huffmanText(0), _table(0), _rasters(0), _imgIdSize(0),
	  _poiShift(0), _shift(0), _encoding(0) {}
	LBLFile(const SubFile *gmp, quint32 offset)
	  : SubFile(gmp, offset), _huffmanText(0), _table(0), _rasters(0),
	  _imgIdSize(0), _poiShift(0), _shift(0), _encoding(0) {}
	~LBLFile();

	bool load(Handle &hdl, const RGNFile *rgn, Handle &rgnHdl);
	void clear();

	Label label(Handle &hdl, quint32 offset, bool poi = false,
	  bool capitalize = true, bool convert = false) const;
	Label label(Handle &hdl, const SubFile *file, Handle &fileHdl,
	  quint32 size, bool capitalize = true, bool convert = false) const;

	quint8 imageIdSize() const {return _imgIdSize;}
	QPixmap image(Handle &hdl, quint32 id) const;

private:
	struct Image {
		quint32 offset;
		quint32 size;
	};

	Label str2label(const QVector<quint8> &str, bool capitalize,
	  bool convert) const;
	Label label6b(const SubFile *file, Handle &fileHdl, quint32 size,
	  bool capitalize, bool convert) const;
	Label label8b(const SubFile *file, Handle &fileHdl, quint32 size,
	  bool capitalize, bool convert) const;
	Label labelHuffman(Handle &hdl, const SubFile *file, Handle &fileHdl,
	  quint32 size, bool capitalize, bool convert) const;
	bool loadRasterTable(Handle &hdl, quint32 offset, quint32 size,
	  quint32 recordSize);

	HuffmanText *_huffmanText;
	quint32 *_table;
	Image *_rasters;
	TextCodec _codec;
	Section _base, _poi, _img;
	quint32 _imgCount;
	quint8 _imgIdSize;
	quint8 _poiShift;
	quint8 _shift;
	quint8 _encoding;
};

}

#endif // IMG_LBLFILE_H
