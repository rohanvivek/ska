#ifndef ViewMain_h
#define ViewMain_h

#include <QtGui/QtGui>

#include <ViewPipeline.hh>

class viewmain_t : public QMainWindow
{
	Q_OBJECT

public:

	viewmain_t();
	~viewmain_t();

private slots:

	void open();

private:

	QAction * openAction_;
	QAction * quitAction_;

	viewpipeline_t * pipeline_;

	QMenu * fileMenu_;

	QToolBar * fileBar_;

	QLabel * filler_;
	QLineEdit * searchBox_;

}; // class viewmain_t

#endif // ViewMain_h
