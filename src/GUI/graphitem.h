#ifndef GRAPHITEM_H
#define GRAPHITEM_H

#include <QGraphicsObject>
#include <QPen>
#include "data/graph.h"
#include "units.h"
#include "graphicsscene.h"

class GraphItem : public QObject, public GraphicsItem
{
	Q_OBJECT

public:
	GraphItem(const Graph &graph, GraphType type, int width,
	  const QColor &color, Qt::PenStyle style, QGraphicsItem *parent = 0);
	virtual ~GraphItem() {}

	virtual ToolTip info() const = 0;

	QPainterPath shape() const {return _shape;}
	QRectF boundingRect() const {return _shape.boundingRect();}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	GraphType graphType() const {return _type;}
	const QRectF &bounds() const {return _bounds;}

	qreal max() const;
	qreal min() const;
	qreal avg() const;

	void setScale(qreal sx, qreal sy);
	void setGraphType(GraphType type);
	void setColor(const QColor &color);
	void setWidth(int width);
	void setUnits(Units units) {_units = units;}
	void updateStyle();

	GraphItem *secondaryGraph() const {return _secondaryGraph;}
	void setSecondaryGraph(GraphItem *graph) {_secondaryGraph = graph;}

	qreal yAtX(qreal x) const;
	qreal distanceAtTime(qreal time) const;
	qreal timeAtDistance(qreal distance) const;

	void redraw();

signals:
	void selected(bool);

public slots:
	void hover(bool hover);

protected:
	void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
	void mousePressEvent(QGraphicsSceneMouseEvent *event);

	Units _units;

private:
	const GraphSegment *segment(qreal x, GraphType type) const;
	void updatePath();
	void updateShape();
	void updateBounds();
	void updateColor();
	const QColor &color() const;

	Graph _graph;

	QColor _color;
	GraphType _type;
	qreal _sx, _sy;

	QPainterPath _path;
	QPainterPath _shape;
	QRectF _bounds;
	QPen _pen;
	bool _time;

	GraphItem *_secondaryGraph;
};

#endif // GRAPHITEM_H
