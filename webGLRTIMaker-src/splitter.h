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

#ifndef SPLITTER_H
#define SPLITTER_H

#include <vector>

#include "rti.h"

class Node
{

public:

	int index;
	int parent;
	QRect imgBox;
	QRectF normalizeBox;
	bool valid;
	Node* children[4];

	void saveTile(const QImage& image, const QString& destFolder, int tileSize, int layerIndex, int quality, const QString& format);

	void saveTile(const QImage& image, const QRect& rect, const QString& destFolder, int tileSize, int layerIndex, int quality, const QString& format);

};

class Tree
{

public:

	std::vector<Node> nodes;
	QRect imgRect;
	int tileSize;
	int nLevels;
	int maxSize;

	Tree();

	void build(const QRect& rect, int size);
	
	void getTreeDescriptor(QString& output, bool compact = false);

private:

	Node* makeChildren(const Node& parent, int index, int level);

};


class Splitter
{

protected:

	MultiLayerImage* image;
	Tree tree;
	int tileSize;

	int maxLoadingSize;

public:

	Splitter(MultiLayerImage* image, int tileSize, int maxRamMB);

	bool split(const QString& destFolder, int quality, const QString& format);
	
	void saveDescriptor(const QString& destFolder, const QString& format);
	
};

#endif
