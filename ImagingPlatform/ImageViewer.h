#pragma once

#include <QGraphicsObject>
#include <mutex>
#include <QPixmap>
#include <QGraphicsSceneWheelEvent>

class ImageViewer : public QGraphicsObject {
	Q_OBJECT

public:
	void display(const QPixmap &pixmap);

private:
	QPixmap m_pixmap;
	std::mutex updateLock;

	qreal scale = 1;
	qreal prescale = 1;

private:
	QRectF boundingRect() const override;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
	void wheelEvent(QGraphicsSceneWheelEvent* e) override;
};