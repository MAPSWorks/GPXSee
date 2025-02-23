#include <QJsonDocument>
#include <QJsonArray>
#include "geojsonparser.h"

#define MARKER_SIZE_MEDIUM 12
#define MARKER_SIZE_SMALL  8
#define MARKER_SIZE_LARGE  16

static int markerSize(const QString &str)
{
	if (str == "small")
		return MARKER_SIZE_SMALL;
	else if (str == "medium")
		return MARKER_SIZE_MEDIUM;
	else if (str == "large")
		return MARKER_SIZE_LARGE;
	else
		return -1;
}

static void setAreaProperties(Area &area, const QJsonValue &properties)
{
	QColor strokeColor(0x55, 0x55, 0x55);
	QColor fillColor(0x55, 0x55, 0x55, 0x99);
	double strokeWidth = 2;

	if (properties.isObject()) {
		QJsonObject o(properties.toObject());

		if (o["name"].isString())
			area.setName(o["name"].toString());
		if (o["title"].isString())
			area.setName(o["title"].toString());
		if (o["description"].isString())
			area.setDescription(o["description"].toString());

		if (o["stroke"].isString())
			strokeColor = QColor(o["stroke"].toString());
		if (o["stroke-opacity"].isDouble())
			strokeColor.setAlphaF(o["stroke-opacity"].toDouble());
		if (o["stroke-width"].isDouble())
			strokeWidth = o["stroke-width"].toDouble();
		if (o["fill"].isString())
			fillColor = QColor(o["fill"].toString());
		if (o["fill-opacity"].isDouble())
			fillColor.setAlphaF(o["fill-opacity"].toDouble());
		else
			fillColor.setAlphaF(0.6);
	}

	area.setStyle(PolygonStyle(fillColor, strokeColor, strokeWidth));
}

static void setTrackProperties(TrackData &track, const QJsonValue &properties)
{
	QColor color(0x55, 0x55, 0x55);
	double width = 2;

	if (properties.isObject()) {
		QJsonObject o(properties.toObject());

		if (o["name"].isString())
			track.setName(o["name"].toString());
		if (o["title"].isString())
			track.setName(o["title"].toString());
		if (o["description"].isString())
			track.setDescription(o["description"].toString());

		if (o["stroke"].isString())
			color = QColor(o["stroke"].toString());
		if (o["stroke-opacity"].isDouble())
			color.setAlphaF(o["stroke-opacity"].toDouble());
		if (o["stroke-width"].isDouble())
			width = o["stroke-width"].toDouble();
	}

	track.setStyle(LineStyle(color, width, Qt::SolidLine));
}

static void setWaypointProperties(Waypoint &waypoint,
  const QJsonValue &properties)
{
	QColor color(0x7e, 0x7e, 0x7e);
	int size = MARKER_SIZE_MEDIUM;

	if (!properties.isObject()) {
		QJsonObject o(properties.toObject());

		if (o["name"].isString())
			waypoint.setName(o["name"].toString());
		if (o["title"].isString())
			waypoint.setName(o["title"].toString());
		if (o["description"].isString())
			waypoint.setDescription(o["description"].toString());

		if (o["marker-color"].isString())
			color = QColor(o["marker-color"].toString());
		if (o["marker-symbol"].isString())
			waypoint.setSymbol(o["marker-symbol"].toString());
		if (o["marker-size"].isString())
			size = markerSize(o["marker-size"].toString());
	}

	waypoint.setStyle(PointStyle(color, size));
}


GeoJSONParser::Type GeoJSONParser::type(const QJsonObject &json)
{
	QString str(json["type"].toString());

	if (str == "Point")
		return Point;
	else if (str == "MultiPoint")
		return MultiPoint;
	else if (str == "LineString")
		return LineString;
	else if (str == "MultiLineString")
		return MultiLineString;
	else if (str == "Polygon")
		return Polygon;
	else if (str == "MultiPolygon")
		return MultiPolygon;
	else if (str == "GeometryCollection")
		return GeometryCollection;
	else if (str == "Feature")
		return Feature;
	else if (str == "FeatureCollection")
		return FeatureCollection;
	else
		return Unknown;
}

bool GeoJSONParser::point(const QJsonArray &coordinates, Waypoint &waypoint,
  const QJsonValue &properties)
{
	if (coordinates.count() < 2 || !coordinates.at(0).isDouble()
	  || !coordinates.at(1).isDouble()) {
		_errorString = "Invalid Point Coordinates";
		return false;
	}

	setWaypointProperties(waypoint, properties);

	waypoint.setCoordinates(Coordinates(coordinates.at(0).toDouble(),
	  coordinates.at(1).toDouble()));
	if (coordinates.count() == 3 && coordinates.at(2).isDouble())
		waypoint.setElevation(coordinates.at(2).toDouble());

	return true;
}

bool GeoJSONParser::multiPoint(const QJsonArray &coordinates,
  QVector<Waypoint> &waypoints, const QJsonValue &properties)
{
	for (int i = 0; i < coordinates.size(); i++) {
		if (!coordinates.at(i).isArray()) {
			_errorString = "Invalid MultiPoint coordinates";
			return false;
		} else {
			waypoints.resize(waypoints.size() + 1);
			if (!point(coordinates.at(i).toArray(), waypoints.last(), properties))
				return false;
		}
	}

	return true;
}

bool GeoJSONParser::lineString(const QJsonArray &coordinates,
  SegmentData &segment)
{
	for (int i = 0; i < coordinates.size(); i++) {
		QJsonArray point(coordinates.at(i).toArray());
		if (point.count() < 2 || !point.at(0).isDouble()
		  || !point.at(1).isDouble()) {
			_errorString = "Invalid LineString coordinates";
			return false;
		}

		Trackpoint t(Coordinates(point.at(0).toDouble(),
		  point.at(1).toDouble()));
		if (point.count() == 3 && point.at(2).isDouble())
			t.setElevation(point.at(2).toDouble());
		segment.append(t);
	}

	return true;
}

bool GeoJSONParser::lineString(const QJsonArray &coordinates, TrackData &track,
  const QJsonValue &properties)
{
	setTrackProperties(track, properties);

	track.append(SegmentData());

	lineString(coordinates, track.last());

	return true;
}

bool GeoJSONParser::multiLineString(const QJsonArray &coordinates,
  TrackData &track, const QJsonValue &properties)
{
	setTrackProperties(track, properties);

	for (int i = 0; i < coordinates.size(); i++) {
		if (!coordinates.at(i).isArray()) {
			_errorString = "Invalid MultiLineString coordinates";
			return false;
		} else {
			track.append(SegmentData());
			if (!lineString(coordinates.at(i).toArray(), track.last()))
				return false;
		}
	}

	return true;
}

bool GeoJSONParser::polygon(const QJsonArray &coordinates, ::Polygon &pg)
{
	for (int i = 0; i < coordinates.size(); i++) {
		if (!coordinates.at(i).isArray()) {
			_errorString = "Invalid Polygon linear ring";
			return false;
		}

		const QJsonArray lr(coordinates.at(i).toArray());
		QVector<Coordinates> data;

		for (int j = 0; j < lr.size(); j++) {
			QJsonArray point(lr.at(j).toArray());
			if (point.count() < 2 || !point.at(0).isDouble()
			  || !point.at(1).isDouble()) {
				_errorString = "Invalid Polygon linear ring coordinates";
				return false;
			}
			data.append(Coordinates(point.at(0).toDouble(),
			  point.at(1).toDouble()));
		}

		pg.append(data);
	}

	return true;
}

bool GeoJSONParser::polygon(const QJsonArray &coordinates, Area &area,
  const QJsonValue &properties)
{
	setAreaProperties(area, properties);

	::Polygon p;
	if (!polygon(coordinates, p))
		return false;
	area.append(p);

	return true;
}

bool GeoJSONParser::multiPolygon(const QJsonArray &coordinates,
  Area &area, const QJsonValue &properties)
{
	setAreaProperties(area, properties);

	for (int i = 0; i < coordinates.size(); i++) {
		if (!coordinates.at(i).isArray()) {
			_errorString = "Invalid MultiPolygon coordinates";
			return false;
		} else {
			::Polygon p;
			if (!polygon(coordinates.at(i).toArray(), p))
				return false;
			area.append(p);
		}
	}

	return true;
}

bool GeoJSONParser::geometryCollection(const QJsonObject &json,
  QList<TrackData> &tracks, QList<Area> &areas,
  QVector<Waypoint> &waypoints, const QJsonValue &properties)
{
	if (!json.contains("geometries") || !json["geometries"].isArray()) {
		_errorString = "Invalid/missing GeometryCollection geometries array";
		return false;
	}

	QJsonArray geometries(json["geometries"].toArray());
	for (int i = 0; i < geometries.size(); i++) {
		QJsonObject geometry(geometries.at(i).toObject());
		switch (type(geometry)) {
			case Point:
				waypoints.resize(waypoints.size() + 1);
				if (!point(geometry["coordinates"].toArray(), waypoints.last(),
				  properties))
					return false;
				break;
			case MultiPoint:
				if (!multiPoint(geometry["coordinates"].toArray(), waypoints,
				  properties))
					return false;
				break;
			case LineString:
				tracks.append(TrackData());
				if (!lineString(geometry["coordinates"].toArray(),
				  tracks.last(), properties))
					return false;
				break;
			case MultiLineString:
				tracks.append(TrackData());
				if (!multiLineString(geometry["coordinates"].toArray(),
				  tracks.last(), properties))
					return false;
				break;
			case Polygon:
				areas.append(Area());
				if (!polygon(geometry["coordinates"].toArray(), areas.last(),
				  properties))
					return false;
				break;
			case MultiPolygon:
				areas.append(Area());
				if (!multiPolygon(geometry["coordinates"].toArray(),
				  areas.last(), properties))
					return false;
				break;
			case GeometryCollection:
				if (!geometryCollection(geometry, tracks, areas, waypoints,
				  properties))
					return false;
				break;
			default:
				_errorString = geometry["type"].toString()
				  + ": invalid/missing geometry type";
				return false;
		}
	}

	return true;
}

bool GeoJSONParser::feature(const QJsonObject &json, QList<TrackData> &tracks,
  QList<Area> &areas, QVector<Waypoint> &waypoints)
{
	QJsonValue properties(json["properties"]);
	QJsonObject geometry(json["geometry"].toObject());

	switch (type(geometry)) {
		case Point:
			waypoints.resize(waypoints.size() + 1);
			return point(geometry["coordinates"].toArray(), waypoints.last(),
			  properties);
		case MultiPoint:
			return multiPoint(geometry["coordinates"].toArray(), waypoints,
			  properties);
		case LineString:
			tracks.append(TrackData());
			return lineString(geometry["coordinates"].toArray(), tracks.last(),
			  properties);
		case MultiLineString:
			tracks.append(TrackData());
			return multiLineString(geometry["coordinates"].toArray(),
			  tracks.last(), properties);
		case GeometryCollection:
			return geometryCollection(geometry, tracks, areas, waypoints);
		case Polygon:
			areas.append(Area());
			return polygon(geometry["coordinates"].toArray(), areas.last(),
			  properties);
		case MultiPolygon:
			areas.append(Area());
			return multiPolygon(geometry["coordinates"].toArray(), areas.last(),
			  properties);
		default:
			_errorString = geometry["type"].toString()
			  + ": invalid/missing Feature geometry";
			return false;
	}
}

bool GeoJSONParser::featureCollection(const QJsonObject &json,
  QList<TrackData> &tracks, QList<Area> &areas,
  QVector<Waypoint> &waypoints)
{
	if (!json.contains("features") || !json["features"].isArray()) {
		_errorString = "Invalid/missing FeatureCollection features array";
		return false;
	}

	QJsonArray features(json["features"].toArray());
	for (int i = 0; i < features.size(); i++)
		if (!feature(features.at(i).toObject(), tracks, areas, waypoints))
			return false;

	return true;
}


bool GeoJSONParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Area> &areas, QVector<Waypoint> &waypoints)
{
	Q_UNUSED(routes);
	QJsonParseError error;
	QJsonDocument doc(QJsonDocument::fromJson(file->readAll(), &error));

	if (doc.isNull()) {
		_errorString = "JSON parse error: " + error.errorString() + " ["
		  + QString::number(error.offset) + "]";
		return false;
	}

	QJsonObject json(doc.object());

	switch (type(json)) {
		case Point:
			waypoints.resize(waypoints.size() + 1);
			return point(json["coordinates"].toArray(), waypoints.last());
		case MultiPoint:
			return multiPoint(json["coordinates"].toArray(), waypoints);
		case LineString:
			tracks.append(TrackData());
			return lineString(json["coordinates"].toArray(), tracks.last());
		case MultiLineString:
			tracks.append(TrackData());
			return multiLineString(json["coordinates"].toArray(), tracks.last());
		case GeometryCollection:
			return geometryCollection(json, tracks, areas, waypoints);
		case Feature:
			return feature(json, tracks, areas, waypoints);
		case FeatureCollection:
			return featureCollection(json, tracks, areas, waypoints);
		case Polygon:
			areas.append(Area());
			return polygon(json["coordinates"].toArray(), areas.last());
		case MultiPolygon:
			areas.append(Area());
			return multiPolygon(json["coordinates"].toArray(), areas.last());
		case Unknown:
			if (json["type"].toString().isNull())
				_errorString = "Not a GeoJSON file";
			else
				_errorString = json["type"].toString()
				  + ": unknown GeoJSON object";
			return false;
	}

	return true;
}
