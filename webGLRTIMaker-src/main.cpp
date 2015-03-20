/*************************************************************************/
/*                                                                       */
/*  WebGLRtiMaker                                                        */
/*  Command line tool to pre-process a RTI image (or a high              */
/*  resolution standard image) for the pubblication on the web           */
/*                                                                       */
/*  Copyright (C) 2015                                                   */
/*  Gianpaolo Palma					                                     */
/*  Visual Computing Laboratory                                          */
/*  ISTI - Italian National Research Council (CNR)                       */
/*  http://vcg.isti.cnr.it/rti/webviewer.php                             */
/*  mailto: gianpaolo[DOT]palma[AT]isti[DOT]cnr[DOT]it                   */
/*          marco[DOT]dibenedetto[AT]isti[DOT]cnr[DOT]it                 */
/*                                                                       */
/*                                                                       */
/*  This program is free software: you can redistribute it and/or modify */
/*  it under the terms of the GNU General Public License as published by */
/*  the Free Software Foundation, either version 3 of the License, or    */
/*  (at your option) any later version.                                  */
/*                                                                       */
/*  This program is distributed in the hope that it will be useful,      */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/*  GNU General Public License for more details.                         */
/*                                                                       */ 
/*  You should have received a copy of the GNU General Public License    */
/*  along with this program.  If not, see <http://www.gnu.org/licenses/> */
/*************************************************************************/


#include "image.h"
#include "rti.h"
#include "splitter.h"
#include "qgetopt.h"

#include <QFileInfo>
#include <QDir>
#include <QImage>
#include <QImageReader>

#include <iostream>

using std::cout;
using std::endl;


void cleanFolder(const QString& folderName, QDir& dir)
{
	if (dir.exists(folderName))
	{
		dir.cd(folderName);

		dir.setFilter(QDir::Files);
		QFileInfoList filelist = dir.entryInfoList();
		for (int i = 0; i < filelist.size(); ++i) 
		{
			QFileInfo fileInfo = filelist.at(i);
			QFile f(fileInfo.absoluteFilePath());
			f.remove();
		}

		dir.cdUp();
		dir.rmdir(folderName);
	}
}

int main( int argc, char ** argv )
{
	QString filename;
	int quality = 90;
	int ramLimit = 1024;
	int tileSize = 256;
	bool pngFormat = false;
	GetOpt opt(argc, argv);
	opt.addArgument("input", "input image - Supported Format: RTI (LRGB-PTM, RGB-PTM, HSH), JPEG, PNG, TIFF. ", &filename);
	opt.addOption('q', "quality", "Quality of save tiles (default: 90)", &quality);
	opt.addOption('r', "ram", "max ram used (in Megabytes default 1024)", &ramLimit);
	opt.addOption('t', "tileSize", "size of the tile (in pixel default 256)", &ramLimit);
	opt.addSwitch('p', "png", "save output tiles as png", &pngFormat);
	
	opt.parse(); 
	
	QString format("jpg");
	if (pngFormat)
		format = "png";

	QFileInfo fi(filename);
	if (!fi.exists())
	{
		cout << "The file does not exist." << endl;
        exit(-1);
	}
	
	if (fi.suffix() != "ptm" && fi.suffix() != "rti" && fi.suffix() != "jpg" && fi.suffix() != "png" && fi.suffix() != "tif" && fi.suffix() != "tiff" )
	{
		cout << "Unsupported file format. The tool accepts a RTI file (PTM or RTI) or a simple image (JPG, PNG, TIF)." << endl;
		exit(-1);
	}

	MultiLayerImage* image;
	if (fi.suffix() == "jpg" || fi.suffix() == "png" || fi.suffix() == "tif" || fi.suffix() == "tiff")
	{
		image = new Image();
		image->setFilename(filename);
	}
	else if (fi.suffix() == "ptm" || fi.suffix() == "rti")
		Rti::loadRti(filename, &image);
	if (!image->isNull())
	{
		QString pathname = fi.absoluteFilePath();
		QString folderName = fi.baseName();
		QDir dir(fi.absolutePath());

		cleanFolder(folderName, dir);

		dir.mkdir(folderName);
		QString destFolder = fi.absolutePath() + "/" + folderName; 

		Splitter splitter(image, tileSize, ramLimit);
		if (!splitter.split(destFolder, quality, format))
		{
			cleanFolder(folderName, dir);
			exit(-1);
		}
		splitter.saveDescriptor(destFolder, format);
		return 0;
	}
	exit(-1);
}
