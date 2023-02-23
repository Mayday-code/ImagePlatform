#pragma once

#include <QGraphicsObject>
#include <mutex>

class ImageViewer : public QGraphicsObject {
	Q_OBJECT

public:
	void setPixmap(const QPixmap& rhs);

private:
	QPixmap m_pixmap;
	std::mutex updateLock;

private:
	QRectF boundingRect() const override;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
};