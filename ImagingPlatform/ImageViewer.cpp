#include "ImageViewer.h"

#include <QPainter>

void ImageViewer::setPixmap(QPixmap&& rhs)
{
	m_pixmap = std::move(rhs);
}

QRectF ImageViewer::boundingRect() const
{
	return QRectF(-m_pixmap.width() / 2, -m_pixmap.height() / 2, m_pixmap.width(), m_pixmap.height());
}

void ImageViewer::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	painter->drawPixmap(-m_pixmap.width() / 2, -m_pixmap.height() / 2, m_pixmap);
}