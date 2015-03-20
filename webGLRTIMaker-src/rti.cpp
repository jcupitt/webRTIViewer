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

#include <QStringList>
#include <QDomDocument>
#include <QDomElement>

#include "rti.h"


QString getLine(FILE *file, bool *eof)
{
    char c;
    QString str = "";
    *eof = false;
    while(!feof(file) && fread(&c, sizeof(char), 1, file)!=0 && c!='\n')
        str.append(c);
    if (feof(file))
        *eof = true;
    return str;
};


bool Rti::loadRti(const QString& filename, MultiLayerImage** image)
{
#ifdef WIN32
#ifndef __MINGW32__
    FILE* file;
    if (fopen_s(&file, filename.toStdString().c_str(), "rb") != 0)
        return false;
#else
    FILE* file = fopen(filename.toStdString().c_str(), "rb");
    if (file == NULL)
        return false;
#endif
#else
    FILE* file = fopen(filename.toStdString().c_str(), "rb");
    if (file == NULL)
        return false;
#endif

	char s[1000];
	fscanf(file, "%s", s);
	std::string type(s);
	if (type.find("PTM") != std::string::npos)
	{
		fscanf(file, "%s", s);
		std::string format(s);
		if (format.find("PTM_FORMAT_LRGB") != std::string::npos)
			*image = new PtmLRGB();
		else if (format.find("PTM_FORMAT_RGB") != std::string::npos)
			*image = new PtmRGB();
  //    if (format.find("PTM_FORMAT_JPEG_LRGB")
	}
	else if (type.find("HSH") != std::string::npos)
		*image = new Hsh();
	else
		return false;
	(*image)->setFilename(filename);
    return true;
}


//Reflectance Transformation Imaging
Rti::Rti(): MultiLayerImage()
{
	headerOffset = 0;
}


Rti::~Rti()
{
	
}


bool Rti::setClipRect(const QRect& rect)
{
	clippingRect = rect;
	return true;
}


bool Rti::isNull()
{
	return (!valid || filename.isNull() || filename.isEmpty());
}


bool Rti::setFilename(const QString& filename)
{
	this->filename = filename;
	if (loadHeader())
	{
		this->valid = true;
		return true;
	}
	return false;
}


int Rti::getNumLayers()
{
	return coeffNumber;
}


//Hemispehrical Hamonics Map
Hsh::Hsh() : Rti()
{
    this->coeffNumber = 0;
	for (int i = 0; i < 9; i++)
		this->coeff[i] = NULL;
}


Hsh::~Hsh()
{
    releaseMemory();
}


bool Hsh::loadData()
{
	if (isNull())
		return false;

	releaseMemory();

#ifdef WIN32
#ifndef __MINGW32__
    FILE* file;
    if (fopen_s(&file, filename.toStdString().c_str(), "rb") != 0)
        return false;
#else
    FILE* file = fopen(filename.toStdString().c_str(), "rb");
    if (file == NULL)
        return false;
#endif
#else
    FILE* file = fopen(filename.toStdString().c_str(), "rb");
    if (file == NULL)
        return false;
#endif

	if (fseek(file, headerOffset, SEEK_CUR) != 0)
		return false;
	if (feof(file))
        return false;

	bool error = false;

	if (clippingRect.isNull())
		clippingRect = QRect(0, 0, w, h);
	else
		clippingRect = QRect(0, 0, w, h).intersected(clippingRect);
	int size = clippingRect.width() * clippingRect.height() * 3;
    for (int i = 0; i < coeffNumber; i++)
        coeff[i] = new unsigned char[size];

    size_t line_size = clippingRect.width() * coeffNumber * 3;
    unsigned char* line = new unsigned char[line_size];
	if (clippingRect.y() > 0)
	{
		long offset = w * coeffNumber * 3 * clippingRect.y();
		fseek(file, offset, SEEK_CUR);
	}
    long xOffset = 0;
	long xOffsetAfter = 0; 
	if (clippingRect.x() > 0)
		xOffset = clippingRect.x() * coeffNumber * 3;
	if (clippingRect.x() + clippingRect.width() < w)
		xOffsetAfter = (w - clippingRect.right() - 1) * coeffNumber * 3;
	
	//Get coefficients
	error = false;
	for (int y = clippingRect.y(); y < clippingRect.y() + clippingRect.height(); y++)
	{
		if (xOffset > 0)
			fseek(file, xOffset, SEEK_CUR);
		if (fread(line, sizeof(unsigned char), line_size, file) == line_size)
		{
			for (int k = 0; k < coeffNumber; k++)
			{
				unsigned char *C = coeff[k];
				int source_offset = k;
				int dest_offset = clippingRect.width() * (y - clippingRect.y()) * 3;
				for (int x = clippingRect.x(); x < clippingRect.x() + clippingRect.width(); x++)
				{
					C[dest_offset++] = line[source_offset]; source_offset += coeffNumber;
					C[dest_offset++] = line[source_offset]; source_offset += coeffNumber;
					C[dest_offset++] = line[source_offset]; source_offset += coeffNumber;
				}
			}
		}
		else
		{
			error = true;
			break;
		}
		if (xOffsetAfter > 0)
			fseek(file, xOffsetAfter, SEEK_CUR);
	}

	if (error || ferror(file) != 0)
	{
		delete[] line;
		releaseMemory();
		return false;
	}

	fclose(file);
	delete[] line;
	loaded = true;
    return true;
}


void Hsh::releaseMemory()
{
	for (int i = 0; i < coeffNumber; i++)
    {
		if (coeff[i])
		{
			delete[] coeff[i];
			coeff[i] = NULL;
		}
    }
}


bool Hsh::getLayer(int layerIndex, QImage* layer)
{
	if (!loaded || layerIndex < 0 || layerIndex > coeffNumber)
		return false;
	if (layer == NULL)
		return false;
	QImage tempImg(clippingRect.width(), clippingRect.height(), QImage::Format_RGB32);
	unsigned char *bits = tempImg.bits();
	for (int i = 0; i < clippingRect.width() * clippingRect.height(); i++)
	{
		bits[i*4] = coeff[layerIndex][i*3 + 2];
		bits[i*4 + 1] = coeff[layerIndex][i*3 + 1];
		bits[i*4 + 2] = coeff[layerIndex][i*3];
	}
	*layer = tempImg.convertToFormat(QImage::Format_RGB888);
	return true;
}


void Hsh::getLayerInfo(QDomDocument& doc, QDomElement& content)
{
	content.setAttribute("type", type);

    QDomElement sizeElem = doc.createElement("Size");
    sizeElem.setAttribute("width", this->width());
    sizeElem.setAttribute("height", this->height());
	sizeElem.setAttribute("coefficients", this->coeffNumber);

    QDomElement scaleElem = doc.createElement("Scale");
    QString scaleStr;
    for (int i = 0; i < this->coeffNumber; i++)
        scaleStr += QString("%1 ").arg(gmax[i]);
    scaleElem.appendChild(doc.createTextNode(scaleStr));
    QDomElement biasElem = doc.createElement("Bias");
    QString biasStr;
    for (int i = 0; i < this->coeffNumber; i++)
        biasStr += QString("%1 ").arg(gmin[i]);
    biasElem.appendChild(doc.createTextNode(biasStr));

    content.appendChild(sizeElem);
    content.appendChild(scaleElem);
    content.appendChild(biasElem);
}


bool Hsh::loadHeader()
{
#ifdef WIN32
#ifndef __MINGW32__
    FILE* file;
    if (fopen_s(&file, filename.toStdString().c_str(), "rb") != 0)
        return false;
#else
    FILE* file = fopen(filename.toStdString().c_str(), "rb");
    if (file == NULL)
        return false;
#endif
#else
    FILE* file = fopen(filename.toStdString().c_str(), "rb");
    if (file == NULL)
        return false;
#endif

    bool eof, error;
    QString str;
    do
    {
        str = getLine(file, &eof);
        if (eof) return false;
    } while (str.startsWith("#"));

    type = "HSH_RTI";
    format = str.toInt(&error);
    if (!error) return false;

    //Get width, height
    str = getLine(file, &eof);
	if (eof) return false;
    QStringList list = str.split(' ', QString::SkipEmptyParts);
    if (list.size() != 3)
        return false;
    w = list[0].toInt(&error);
    if (!error) return false;
    h = list[1].toInt(&error);
    if (!error) return false;

    //Get number of basis term
    str = getLine(file, &eof);
	if (eof) return false;
    list = str.split(' ',  QString::SkipEmptyParts);
    if (list.size() != 3)
        return false;
    coeffNumber = list[0].toInt(&error);
    if (!error) return false;

    //Get max and min scale value
    fread(gmin, sizeof(float), coeffNumber, file);
    fread(gmax, sizeof(float), coeffNumber, file);

    if (feof(file))
        return false;
	headerOffset = ftell(file);
	fclose(file);
	return true;
}


//LRGB Polynomial Texture Map
PtmLRGB::PtmLRGB() : Rti()
{
	coeffNumber = 3;
	for (int i = 0; i < 3; i++)
		this->coeff[i] = NULL;
}


PtmLRGB::~PtmLRGB()
{
	releaseMemory();
}


bool PtmLRGB::loadData()
{
	 if (isNull())
		return false;
	releaseMemory();
#ifdef WIN32
#ifndef __MINGW32__
    FILE* file;
    if (fopen_s(&file, filename.toStdString().c_str(), "rb") != 0)
        return false;
#else
    FILE* file = fopen(filename.toStdString().c_str(), "rb");
    if (file == NULL)
        return false;
#endif
#else
    FILE* file = fopen(filename.toStdString().c_str(), "rb");
    if (file == NULL)
        return false;
#endif

    bool error = false;
	if (fseek(file, headerOffset, SEEK_CUR) != 0)
		return false;
	if (feof(file))
        return false;

	if (clippingRect.isNull())
		clippingRect = QRect(0, 0, w, h);
	else
		clippingRect = QRect(0, 0, w, h).intersected(clippingRect);
	int size = clippingRect.width() * clippingRect.height() * 3;
	for (int i = 0; i < 3; i++)
        coeff[i] = new unsigned char[size];

    int multiplexed_channels = 3;
	if (format == "PTM_1.2")
		multiplexed_channels = 2;
    size_t line_size = clippingRect.width() * multiplexed_channels * 3;
    unsigned char* line = new unsigned char[line_size];
	if (h - clippingRect.bottom() - 1 > 0)
	{
		long offset = w * multiplexed_channels * 3 * (h - clippingRect.bottom() - 1);
		fseek(file, offset, SEEK_CUR);
	}
    long xOffset = clippingRect.x() * multiplexed_channels * 3;
	long xOffsetAfter = (w - clippingRect.right() - 1) * multiplexed_channels * 3;
	
	error = false;
	for (int y = clippingRect.y(); y < clippingRect.y() + clippingRect.height(); y++)
    {
		if (xOffset > 0)
			fseek(file, xOffset, SEEK_CUR);
        if (fread(line, sizeof(unsigned char), line_size, file) == line_size)
		{
			for(int c = 0; c < multiplexed_channels; c++)
			{
				int dest_offset = clippingRect.width() * (clippingRect.height() - (y - clippingRect.y()) - 1) * 3;
				int source_offset = c * 3;
				unsigned char *C = coeff[c];

				for (int x = clippingRect.x(); x < clippingRect.x() + clippingRect.width(); x++)
				{                
					memcpy(&C[dest_offset], &line[source_offset], sizeof(unsigned char) * 3); 
					source_offset += multiplexed_channels * 3;
					dest_offset += 3;
				}
			}
		}
		else
		{
			error = true;
			break;
		}
		if (xOffsetAfter > 0)
			fseek(file, xOffsetAfter, SEEK_CUR);
    }
	
	if (error || ferror(file) != 0)
	{
		delete[] line;
		releaseMemory();
		return false;
	}
    
	if(multiplexed_channels == 2)
	{
		error = false;
		if (clippingRect.y()> 0)
		{
			long offset = w * multiplexed_channels * 3 * (clippingRect.y());
			fseek(file, offset, SEEK_CUR);
		}
		xOffset = clippingRect.x() * 3;
		xOffsetAfter = (w - clippingRect.right() - 1) * 3;
		if (h - clippingRect.bottom() - 1 > 0)
		{
			long offset = w * 3 * (h - clippingRect.bottom() - 1);
			fseek(file, offset, SEEK_CUR);
		}
		int offset = clippingRect.width() * (clippingRect.height() - 1) * 3;
		line_size = clippingRect.width() * 3;
		for (int y = clippingRect.y(); y < clippingRect.y() + clippingRect.height(); y++)
		{
			if (xOffset > 0)
				fseek(file, xOffset, SEEK_CUR);
			if (fread(&coeff[2][offset], sizeof(unsigned char), line_size, file) < line_size)
			{
				error = true;
				break;
			}
			if (xOffsetAfter > 0)
				fseek(file, xOffsetAfter, SEEK_CUR);
			offset -= clippingRect.width() * 3;
		}
 	}
	
	if (error || ferror(file))
	{
		delete[] line;
		releaseMemory();
		return false;
	}

    fclose(file);
	type = "LRGB_PTM";
	delete[] line;
	loaded = true;
    return true;
}


void PtmLRGB::releaseMemory()
{
	for (int i = 0; i < 3; i++)
	{
		if (coeff[i])
		{
			delete[] coeff[i];
			coeff[i] = NULL;
		}
	}
}


bool PtmLRGB::getLayer(int layerIndex, QImage* layer)
{
	if (!loaded || layerIndex < 0 || layerIndex > coeffNumber)
		return false;
	if (layer == NULL)
		return false;
	QImage tempImg(clippingRect.width(), clippingRect.height(), QImage::Format_RGB32);
	unsigned char *bits = tempImg.bits();
	for (int i = 0; i < clippingRect.width() * clippingRect.height(); i++)
	{
		bits[i*4] = coeff[layerIndex][i*3 + 2];
		bits[i*4 + 1] = coeff[layerIndex][i*3 + 1];
		bits[i*4 + 2] = coeff[layerIndex][i*3];
	}
	*layer = tempImg.convertToFormat(QImage::Format_RGB888);
	return true;
}


void PtmLRGB::getLayerInfo(QDomDocument& doc, QDomElement& content)
{
	content.setAttribute("type", type);

    QDomElement sizeElem = doc.createElement("Size");
    sizeElem.setAttribute("width", this->width());
    sizeElem.setAttribute("height", this->height());
	sizeElem.setAttribute("coefficients", 6);

    QDomElement scaleElem = doc.createElement("Scale");
    QString scaleStr;
    for (int i = 0; i < 6; i++)
        scaleStr += QString("%1 ").arg(scale[i]);
    scaleElem.appendChild(doc.createTextNode(scaleStr));
    QDomElement biasElem = doc.createElement("Bias");
    QString biasStr;
    for (int i = 0; i < 6; i++)
        biasStr += QString("%1 ").arg(bias[i]);
    biasElem.appendChild(doc.createTextNode(biasStr));

    content.appendChild(sizeElem);
    content.appendChild(scaleElem);
    content.appendChild(biasElem);
}


bool PtmLRGB::loadHeader()
{
	#ifdef WIN32
#ifndef __MINGW32__
    FILE* file;
    if (fopen_s(&file, filename.toStdString().c_str(), "rb") != 0)
        return false;
#else
    FILE* file = fopen(filename.toStdString().c_str(), "rb");
    if (file == NULL)
        return false;
#endif
#else
    FILE* file = fopen(filename.toStdString().c_str(), "rb");
    if (file == NULL)
        return false;
#endif

    bool eof, error;
    //Get format
    format = getLine(file, &eof);
    if (eof) return false;

    //Get type
    type = getLine(file, &eof);
    if (eof) return false;
	
    //Get width and hieght
    QString str = getLine(file, &eof);
    if (eof) return false;
    w = str.toInt(&error);
    if (!error) return false;
    str = getLine(file, &eof);
    if (eof) return false;
    h = str.toInt(&error);
    if (!error) return false;

    //Get scale value
    str = getLine(file, &eof);
    if (eof) return false;
    QStringList list = str.split(' ', QString::SkipEmptyParts);
    if (list.size() != 6)
        return false;
    for (int i = 0; i < 6; i++)
    {
        scale[i] = list[i].toDouble(&error);
        if (!error) return false;
    }

    //Get bias value
    str = getLine(file, &eof);
    if (eof) return false;
    list = str.split(' ',  QString::SkipEmptyParts);
    if (list.size() != 6)
        return false;
    for (int i = 0; i < 6; i++)
    {
        bias[i] = list[i].toInt(&error);
        if (!error) return false;
    }

	 if (feof(file))
        return false;
	 headerOffset = ftell(file);
	 fclose(file);
	 return true;
}




//RGB Polynomial Texture Map
PtmRGB::PtmRGB() : Rti()
{
	coeffNumber = 6;
	for (int i = 0; i < 6; i++)
		this->coeff[i] = NULL;
}

	
PtmRGB::~PtmRGB()
{
	releaseMemory();
}


bool PtmRGB::loadData()
{
	if (isNull())
		return false;
	releaseMemory();
#ifdef WIN32
#ifndef __MINGW32__
    FILE* file;
    if (fopen_s(&file, filename.toStdString().c_str(), "rb") != 0)
        return false;
#else
    FILE* file = fopen(filename.toStdString().c_str(), "rb");
    if (file == NULL)
        return false;
#endif
#else
    FILE* file = fopen(filename.toStdString().c_str(), "rb");
    if (file == NULL)
        return false;
#endif

    bool error;
	if (fseek(file, headerOffset, SEEK_CUR) != 0)
		return false;
	if (feof(file))
        return false;   

	if (clippingRect.isNull())
		clippingRect = QRect(0, 0, w, h);
	else
		clippingRect = QRect(0, 0, w, h).intersected(clippingRect);
	int size = clippingRect.width() * clippingRect.height() * 3;
	for (int i = 0; i < 6; i++)
        coeff[i] = new unsigned char[size];

	size_t line_size = clippingRect.width() * 6;
    unsigned char* line = new unsigned char[line_size];
	
	long yOffset = w * 6 * (h - clippingRect.bottom() - 1);
	long yOffsetAfter =  w * 6 * (clippingRect.y());
    long xOffset = clippingRect.x() * 6;
	long xOffsetAfter = (w - clippingRect.right() - 1) * 6;
	
	error = false;
	for(int i =0 ; i < 3; i++)
    {
		if (yOffset > 0)
			fseek(file, yOffset, SEEK_CUR);
		for (int y = clippingRect.y(); y < clippingRect.y() + clippingRect.height(); y++)
		{
			if (xOffset > 0)
				fseek(file, xOffset, SEEK_CUR);
			if (fread(line, sizeof(unsigned char), line_size, file) == line_size)
			{
				for(int c = 0; c < 6; c++)
				{
					int dest_offset = clippingRect.width() * (clippingRect.height() - (y - clippingRect.y()) - 1) * 3 + i;
					int source_offset = c;
					unsigned char *C = coeff[c];
					for (int x = clippingRect.x(); x < clippingRect.x() + clippingRect.width(); x++)
					{
						C[dest_offset] = line[source_offset];
						dest_offset += 3;
						source_offset += 6;
					}
				}
			}
			else
			{
				error = true;
				break;
			}
			if (xOffsetAfter > 0)
				fseek(file, xOffsetAfter, SEEK_CUR);
		}
		if (yOffsetAfter > 0)
			fseek(file, yOffsetAfter, SEEK_CUR);
	}
	
	if (error || ferror(file))
	{
		delete[] line;
		releaseMemory();
		return false;
	}
    fclose(file);
	delete[] line;
	loaded = true;
    return true;
}


void PtmRGB::releaseMemory()
{
	for (int i = 0; i < 6; i++)
	{
		if (coeff[i])
		{
			delete[] coeff[i];
			coeff[i] = NULL;
		}
	}
}


bool PtmRGB::getLayer(int layerIndex, QImage* layer)
{
	if (!loaded || layerIndex < 0 || layerIndex > coeffNumber)
		return false;
	if (layer == NULL)
		return false;
	QImage tempImg(clippingRect.width(), clippingRect.height(), QImage::Format_RGB32);
	unsigned char *bits = tempImg.bits();
	for (int i = 0; i < clippingRect.width() * clippingRect.height(); i++)
	{
		bits[i*4] = coeff[layerIndex][i*3 + 2];
		bits[i*4 + 1] = coeff[layerIndex][i*3 + 1];
		bits[i*4 + 2] = coeff[layerIndex][i*3];
	}
	*layer = tempImg.convertToFormat(QImage::Format_RGB888);
	return true;
}


void PtmRGB::getLayerInfo(QDomDocument& doc, QDomElement& content)
{
	content.setAttribute("type", type);

    QDomElement sizeElem = doc.createElement("Size");
    sizeElem.setAttribute("width", this->width());
    sizeElem.setAttribute("height", this->height());
	sizeElem.setAttribute("coefficients", 6);

    QDomElement scaleElem = doc.createElement("Scale");
    QString scaleStr;
    for (int i = 0; i < 6; i++)
        scaleStr += QString("%1 ").arg(scale[i]);
    scaleElem.appendChild(doc.createTextNode(scaleStr));
    QDomElement biasElem = doc.createElement("Bias");
    QString biasStr;
    for (int i = 0; i < 6; i++)
        biasStr += QString("%1 ").arg(bias[i]);
    biasElem.appendChild(doc.createTextNode(biasStr));

    content.appendChild(sizeElem);
    content.appendChild(scaleElem);
    content.appendChild(biasElem);
}


bool PtmRGB::loadHeader()
{
#ifdef WIN32
#ifndef __MINGW32__
    FILE* file;
    if (fopen_s(&file, filename.toStdString().c_str(), "rb") != 0)
        return false;
#else
    FILE* file = fopen(filename.toStdString().c_str(), "rb");
    if (file == NULL)
        return false;
#endif
#else
    FILE* file = fopen(filename.toStdString().c_str(), "rb");
    if (file == NULL)
        return false;
#endif

    bool eof, error;

    //Get version
    getLine(file, &eof);
    if (eof) return false;

    //Get type
    getLine(file, &eof);
    if (eof) return false;
	type = "RGB_PTM";

    //Get width and height
    QString str = getLine(file, &eof);
    if (eof) return false;
    w = str.toInt(&error);
    if (!error) return false;
    str = getLine(file, &eof);
    if (eof) return false;
    h = str.toInt(&error);
    if (!error) return false;

    //Get scale value
    str = getLine(file, &eof);
    if (eof) return false;
    QStringList list = str.split(' ',  QString::SkipEmptyParts);
    if (list.size() != 6)
        return false;
    for (int i = 0; i < 6; i++)
    {
        scale[i] = list[i].toDouble(&error);
        if (!error) return false;
    }

    //Get bias value
    str = getLine(file, &eof);
    if (eof) return false;
    list = str.split(' ',  QString::SkipEmptyParts);
    if (list.size() != 6)
        return false;
    for (int i = 0; i < 6; i++)
    {
        bias[i] = list[i].toDouble(&error);
        if (!error) return false;
    }

	 if (feof(file))
        return false;
	 headerOffset = ftell(file);
	 fclose(file);
	 return true;
}
