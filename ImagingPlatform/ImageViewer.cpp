#include "ImageViewer.h"

#include <QPainter>
#include <iostream>

void ImageViewer::display(const QPixmap &pixmap)
{
	std::lock_guard<std::mutex> g(updateLock);
	m_pixmap = pixmap;
	this->update();
}

QRectF ImageViewer::boundingRect() const
{
	return QRectF(-m_pixmap.width() / 2, -m_pixmap.height() / 2, m_pixmap.width(), m_pixmap.height());
}

void ImageViewer::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	std::lock_guard<std::mutex> lck(updateLock);
	painter->drawPixmap(-m_pixmap.width() / 2, -m_pixmap.height() / 2, m_pixmap);

	if (scale != prescale) {
		setScale(scale);
		prescale = scale;
	}
}

void ImageViewer::wheelEvent(QGraphicsSceneWheelEvent* e)
{
	if (e->delta() > 0) {
		if (scale >= 10) return;
		scale *= 1.1;
		return;
	}

	if (e->delta() < 0) {
		if (scale <= 0.1) return;
		scale *= 0.9;
		return;
	}
}