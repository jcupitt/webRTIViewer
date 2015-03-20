/*************************************************************************/
/*                                                                       */
/*  WebGLRtiMaker                                                        */
/*  Command line tool to pre-process a RTI image (or a high              */
/*  resolution standard image) for the pubblication on the web           */
/*                                                                       */
/*  Copyright (C) 2015                                                   */
/*  Gianpaolo Palma                                                      */
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

#include <stdio.h>

#include "image.h"

Image::Image():MultiLayerImage()
{
	this->type = QString("IMAGE");
}

Image::~Image()
{

}

bool Image::checkError()
{
	if (this->reader.error() != 0)
	{
		printf("%s\n", this->reader.errorString().toStdString().c_str());
		return true;
	}
	return false;
}

bool Image::isNull()
{
	return !valid || this->checkError();
}

bool Image::setClipRect(const QRect& rect)
{
	this->reader.setFileName(this->filename);
	if (this->checkError())
		return false;
	this->clippingRect = rect;
	this->reader.setClipRect(rect);
	if (this->checkError())
		return false;
	return true;
}

bool Image::loadData()
{
	this->reader.read(&this->image);
	if (!this->checkError())
	{
		loaded = true;
		return true;
	}
	return false;
}

void Image::releaseMemory()
{

}

int Image::getNumLayers()
{
	return 1;
}

bool Image::getLayer(int layerIndex, QImage* layer)
{
	if (!loaded || layerIndex > 1)
		return false;
	*layer = image;
	return true;
}

bool Image::setFilename(const QString& filename)
{
	this->filename = filename;
	this->reader.setFileName(filename);
	if (this->reader.error() == 0)
	{
		this->w = this->reader.size().width();
		this->h = this->reader.size().height();
		valid = true;
		return true;
	}
	return false;
}


void Image:: getLayerInfo(QDomDocument& doc, QDomElement& content)
{
	content.setAttribute("type", type);
	QDomElement sizeElem = doc.createElement("Size");
	sizeElem.setAttribute("width", w);
	sizeElem.setAttribute("height", h);
	sizeElem.setAttribute("coefficients", 1);
	content.appendChild(sizeElem);
}
