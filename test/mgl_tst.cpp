// gcc -o mgl_tst mgl_tst.cpp  -lstdc++ -lQtCore -lQtGui -lmgl -lmgl-qt

#include <QtGui/QApplication>
#include <mgl2/base_cf.h>
#include <mgl2/qt.h>
#include <mgl2/qmathgl.h>

class Plot : public mglDraw
{
	protected:

	public:
		int Draw(mglGraph *pg)
		{
			pg->LoadFont("heros");
			pg->SetFontSize(3.2);

			double _x[] = {1., 2., 3., 4.};
			double _y[] = {3., 1., 2.2, 2.};
			double _ex[] = {0., 0., 0., 0.};
			double _ey[] = {0.5, 0.4, 0.7, 0.5};


			mglData x, y, ex, ey;
			x.Link(_x, 4);
			y.Link(_y, 4);
			ex.Link(_ex, 4);
			ey.Link(_ey, 4);
			//x.Set(_x, 4);
			//y.Set(_y, 4);

			pg->SetRanges(0., 5., 0., 5.);
			pg->Grid();

			pg->Plot(x, y, "-r2", "legend Line");
			pg->Error(x, y, ex, ey, "#sr2", "legend Points");

			pg->Axis();
			pg->Legend();

			pg->Title("Test");
			pg->Label('x', "x", 0);
			pg->Label('y', "y", 0);
			return 0;
		}
};



int main(int argc, char** argv)
{
	QApplication app(argc, argv);
	//mgl_def_font("heros", "");

	QMathGL *pMgl = new QMathGL();
	Plot plt;
	pMgl->setDraw(&plt);

	pMgl->setZoom(0);
	pMgl->autoResize = 1;
	//pMgl->adjust();	// needs parent window
	pMgl->show();

	return app.exec();
}
