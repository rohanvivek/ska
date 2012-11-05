/*----------------------------------------------------------------------------*
 * Copyright (c) 2012 Los Alamos National Security, LLC
 * All rights reserved
 *----------------------------------------------------------------------------*/

#include<iostream>

#include<QtGui/QFrame>
#include<qwt/qwt_plot.h>
#include<qwt/qwt_plot_canvas.h>
#include<qwt/qwt_plot_layout.h>
#include<qwt/qwt_plot_curve.h>
#include<qwt/qwt_scale_map.h>

#include<ViewSlope.hh>
#include<ViewHighlight.hh>

typedef viewhighlight_T<viewslope_t> viewhighlight_t;

viewslope_t::viewslope_t(QWidget * parent)
	: QwtPlot(parent), plot_(nullptr), highlightArea_(nullptr),
	highlight_(0, 0, 0, 0), points_(0), current_(0), ratio_(1),
	left_(50), right_(0), top_(2), bottom_(0)
{
	canvas()->setFrameStyle(QFrame::Box | QFrame::Plain);

	setAxisTitle(yLeft, "Instructions to Process");
	setAxisTitle(xBottom, "Issue Cycle");

	// create the curve plot and add it to parent
	plot_ = new QwtPlotCurve("");
	plot_->attach(this);

	// set plot style
	plot_->setStyle(QwtPlotCurve::Lines);
	plot_->setPen(QPen(Qt::red, 1));

	// create highlight area
	highlightArea_ = new viewhighlight_t(this);

	// set initial size
	resize(300, 300);
} // viewslope_t::viewslope_t

void viewslope_t::load(const QString & dataset,
	const QVector<double> & x_points, const QVector<double> & y_points)
{
	// set the window title
	QString title = "Slope view (" + dataset + ")";
	setWindowTitle(title);

	points_ = y_points.size();

	double xmin(std::numeric_limits<double>::max());
	double xmax(std::numeric_limits<double>::min());
	double ymin(std::numeric_limits<double>::max());
	double ymax(std::numeric_limits<double>::min());

	for(signed i(0); i<x_points.size(); ++i) {
		xmin = std::min(xmin, x_points[i]);
		xmax = std::max(xmax, x_points[i]);
	} // for

	for(signed i(0); i<y_points.size(); ++i) {
		ymin = std::min(ymin, y_points[i]);
		ymax = std::max(ymax, y_points[i]);
	} // for

	setAxisScale(yLeft, ymin, ymax);
	setAxisScale(xBottom, xmin, xmax);

	plot_->setRawSamples(x_points.data(), y_points.data(), x_points.size());
	updateOffsets();
	replot();
} // viewslope_t::load

void viewslope_t::highlightAreaPaintEvent(QPaintEvent * event)
{
	QPainter painter(highlightArea_);
	painter.fillRect(event->rect(), QColor(0,0,255, 63));
} // viewslope_t::highlightAreaPaintEvent

int viewslope_t::highlightAreaWidth()
{
	QRect cr = contentsRect();
	return cr.width();
} // viewslope_t::highlightAreaWidth

void viewslope_t::updateHighlightArea(const QRect & rect, int dy)
{
	if(dy) {
		moveHighlight(current_ - (dy/12.0));
	} // if
} // viewslope_t::updateHighlightArea

void viewslope_t::moveHighlight(int y) {
	current_ = y;

	highlight_.moveTo(left_, int(y*ratio_ - 0.5*ratio_ + top_ + pad_));

	highlightArea_->setGeometry(highlight_);
	highlightArea_->repaint();
} // viewslope_t::mouseMoveEvent

void viewslope_t::resizeEvent(QResizeEvent * event) {
	QwtPlot::resizeEvent(event);
	updateOffsets();
} // viewslope_t::resizeEvent

void viewslope_t::updateOffsets()
{
	QRect cr = contentsRect();
	QRect ccr = canvas()->contentsRect();

	left_ = cr.right() - (width_ + ccr.width());
	right_ = cr.right() - width_;
	top_ = cr.top() + width_;
	bottom_ = cr.top() + width_ + ccr.height();

	int yExtents = std::abs(bottom_ - top_) - (2*pad_);
	int xExtents = std::abs(right_ - left_) + 1;

	ratio_ = yExtents/double(points_);

	highlight_.setHeight(std::max(2, int(ratio_)));
	highlight_.setWidth(xExtents);
	highlightArea_->setGeometry(highlight_);
} // viewslope_t::updateOffsets
