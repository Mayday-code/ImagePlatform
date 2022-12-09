#pragma once

#include <QGraphicsObject>

class ImageViewer : public QGraphicsObject {
	Q_OBJECT

public:
	void setPixmap(QPixmap&& rhs);

private:
	QPixmap m_pixmap;

private:
	QRectF boundingRect() const override;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
};