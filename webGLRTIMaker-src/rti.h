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


#ifndef RTI_H
#define RTI_H

#include "multilayer.h"


class Rti : public MultiLayerImage
{

protected:

	int coeffNumber;
	long headerOffset;
	
public:

	Rti();

	virtual ~Rti();

	virtual bool setClipRect(const QRect& rect);

	virtual bool isNull();

	virtual bool setFilename(const QString& filename);

	virtual int getNumLayers();

	virtual bool loadData() = 0;

	virtual void releaseMemory() = 0;

	virtual bool getLayer(int layerIndex, QImage* layer) = 0;

	virtual void getLayerInfo(QDomDocument& doc, QDomElement& content) = 0; 

	static bool loadRti(const QString& filename, MultiLayerImage** image);

private:

	virtual bool loadHeader() = 0;
};



class Hsh : public Rti
{

protected:

	float gmax[9];
	float gmin[9];

	unsigned char* coeff[9];

public:

	Hsh();

	virtual ~Hsh();

	virtual bool loadData();

	virtual void releaseMemory();

	virtual bool getLayer(int layerIndex, QImage* layer);

	virtual void getLayerInfo(QDomDocument& doc, QDomElement& content); 

private:

	virtual bool loadHeader();
};


class PtmLRGB : public Rti
{

protected:
	float scale[6];
	float bias[6];

	unsigned char* coeff[3];

public:

	PtmLRGB();

	virtual ~PtmLRGB();

	virtual bool loadData();

	virtual void releaseMemory();

	virtual bool getLayer(int layerIndex, QImage* layer);

	virtual void getLayerInfo(QDomDocument& doc, QDomElement& content);  

private:

	virtual bool loadHeader();
};


class PtmRGB : public Rti
{

protected:

	float scale[6];
	float bias[6];

	unsigned char* coeff[6];

public:

	PtmRGB();

	virtual ~PtmRGB();

	virtual bool loadData();

	virtual void releaseMemory();

	virtual bool getLayer(int layerIndex, QImage* layer);

	virtual void getLayerInfo(QDomDocument& doc, QDomElement& content);

private:

	virtual bool loadHeader();
	
};

#endif
