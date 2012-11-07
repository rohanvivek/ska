/*----------------------------------------------------------------------------*
 *
 *----------------------------------------------------------------------------*/

#include <QtGui/QScrollBar>
#include <graphviz/gvc.h>

#include <ViewGraph.hh>

viewgraph_t::viewgraph_t(QWidget * parent)
	: QMainWindow(parent), scale_(1.0)
{
	// create main image label
	imageLabel_ = new QLabel;
	imageLabel_->setBackgroundRole(QPalette::Base);
	imageLabel_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
		imageLabel_->setScaledContents(true);

	// create scroll area
	scrollArea_ = new QScrollArea;
	scrollArea_->setBackgroundRole(QPalette::Dark);
	scrollArea_->setWidget(imageLabel_);
	setCentralWidget(scrollArea_);

	// create zoom in
	zin_ = new QAction(QIcon(":/icons/zin.png"), tr("Zoom &In"), this);
	zin_->setShortcut(tr("Ctrl++"));
	connect(zin_, SIGNAL(triggered()), this , SLOT(zoom_in()));

	// create zoom out
	zout_ = new QAction(QIcon(":/icons/zout.png"), tr("Zoom &Out"), this);
	zout_->setShortcut(tr("Ctrl+-"));
	connect(zout_, SIGNAL(triggered()), this , SLOT(zoom_out()));

	// create zoom reset
	reset_ = new QAction(QIcon(":/icons/reset.png"), tr("Zoom &Reset"), this);
	reset_->setShortcut(tr("Ctrl+r"));
	connect(reset_, SIGNAL(triggered()), this , SLOT(zoom_reset()));

	// create toolbar
	fileBar_ = addToolBar(tr("File"));

	fileBar_->addAction(zin_);
	fileBar_->addAction(zout_);
	fileBar_->addAction(reset_);

	resize(300, 300);
} // viewgraph_t::viewgraph_t

void viewgraph_t::load(const QString & dataset, const QString & data)
{
	Agraph_t * graph;
	GVC_t * gvc;

	gvc = gvContext();

	// read the graph
	char * _data = strdup(data.toStdString().data());
	graph = agmemread(_data);
	free(_data);

	gvLayout (gvc, graph, "dot");

	// this is lame, for now we have to write to a temporary
	// file for the rendering and then read it back in.
	gvRenderFilename(gvc, graph, "png", "skaview_gv.png");
	QImage image("skaview_gv.png");
	int err = system("rm -rf skaview_gv.png");	
	if(err == -1 || err == 127) {
		// FIXME: error
	} // if

	imageLabel_->setPixmap(QPixmap::fromImage(image));
	imageLabel_->adjustSize();
} // viewgraph_t::load

void viewgraph_t::zoom_in()
{
	scaleImage(1.25);
} // viewgraph_t::zoom_in

void viewgraph_t::zoom_out()
{
	scaleImage(0.8);
} // viewgraph_t::zoom_out

void viewgraph_t::zoom_reset()
{
	imageLabel_->adjustSize();
	scale_ = 1.0;
} // viewgraph_t::zoom_reset

void viewgraph_t::scaleImage(double factor)
{
	Q_ASSERT(imageLabel_->pixmap());
	scale_ *= factor;

	imageLabel_->resize(scale_ * imageLabel_->pixmap()->size());

	adjustScrollBar(scrollArea_->horizontalScrollBar(), factor);
	adjustScrollBar(scrollArea_->verticalScrollBar(), factor);

	zin_->setEnabled(scale_ < 3.0);
	zout_->setEnabled(scale_ > 0.333);
} // viewgraph_t::scaleImage

void viewgraph_t::adjustScrollBar(QScrollBar * scrollBar, double factor)
{
	scrollBar->setValue(int(factor * scrollBar->value() +
		((factor - 1) * scrollBar->pageStep()/2)));
} // viewgraph_t::adjustScrollBar
