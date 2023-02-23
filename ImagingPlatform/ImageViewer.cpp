#include "ImageViewer.h"

#include <QPainter>

void ImageViewer::setPixmap(const QPixmap& rhs)
{
	std::lock_guard<std::mutex> lck(updateLock);
	m_pixmap = rhs;
}

QRectF ImageViewer::boundingRect() const
{
	return QRectF(-m_pixmap.width() / 2, -m_pixmap.height() / 2, m_pixmap.width(), m_pixmap.height());
}

void ImageViewer::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	std::lock_guard<std::mutex> lck(updateLock);
	painter->drawPixmap(-m_pixmap.width() / 2, -m_pixmap.height() / 2, m_pixmap);
}