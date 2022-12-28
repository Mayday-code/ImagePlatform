#include "Previewer.h"

Previewer::Previewer(QWidget *parent)
	: QWidget(parent)
{
	layout = new QGridLayout;
	setLayout(layout);

	setWindowFlags(Qt::Window);
	setAttribute(Qt::WA_DeleteOnClose, true);  
}

void Previewer::addPreviewItem(PreviewItem* item, int r, int c)
{
	item->setScaledContents(true);
	layout->addWidget(item, r, c);
	connect(item, &PreviewItem::mouseClicked, this, &Previewer::moveTo);
}

Previewer::~Previewer()
{}
