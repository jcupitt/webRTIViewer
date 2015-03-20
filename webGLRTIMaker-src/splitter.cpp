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

#include <math.h>

#include <QPainter>
#include <QFile>
#include <QTextStream>
#include <QDomDocument>
#include <QDomElement>
#include <QImage>

#include "splitter.h"


void Node::saveTile(const QImage& image, const QString& destFolder, int tileSize, int layerIndex, int quality, const QString& format)
{
	if (valid)
	{
		QImage tile = image.copy(imgBox);
		QImage tile2 = tile.scaled(tileSize + 2, tileSize + 2, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		tile2.save(QString("%1/%2_%3.%4").arg(destFolder).arg(index + 1).arg(layerIndex).arg(format), format.toStdString().c_str(), quality);
		for (int i = 0; i < 4; i++)
			if (children[i] != NULL)
				children[i]->saveTile(image, destFolder, tileSize, layerIndex, quality, format);
	}
}


void Node::saveTile(const QImage& image, const QRect& rect, const QString& destFolder, int tileSize, int layerIndex, int quality, const QString& format)
{
	if (valid)
	{
		QRect tempRect(imgBox.x() - rect.x(), imgBox.y() - rect.y(), imgBox.width(), imgBox.height());
		QImage tile = image.copy(tempRect);
		QImage tile2 = tile.scaled(tileSize + 2, tileSize + 2, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		tile2.save(QString("%1/%2_%3.%4").arg(destFolder).arg(index + 1).arg(layerIndex).arg(format), format.toStdString().c_str(), quality);
		for (int i = 0; i < 4; i++)
			if (children[i] != NULL)
				children[i]->saveTile(image, rect, destFolder, tileSize, layerIndex, quality, format);
	}
}


Tree::Tree()
{

}


void Tree::build(const QRect& rect, int size)
{
	tileSize = 1 << (int)ceil(log10f(size) / log10f(2.0f));
	int maxImgSize = std::max(rect.height(), rect.width());
	maxSize = 1 << (int)ceil(log10f(maxImgSize) / log10f(2.0f));
	nLevels = (int)ceil(log10f(maxSize) / log10f(2.0f)) - (int)ceil(log10f(tileSize) / log10f(2.0f)) + 1;
		
	int woffset = (maxSize - rect.width()) / 2;
	int hoffset = (maxSize - rect.height()) / 2;

	int nNodes = (pow(4.0f, nLevels) - 1.0f) / 3.0f;
	nodes.resize(nNodes);
			
	imgRect = rect;
	imgRect.moveTo(woffset, hoffset);

	float pfp = 1 << (nLevels - 1);
	nodes[0].index = 0;
	nodes[0].parent =  -1;
	nodes[0].normalizeBox = QRectF(0, 0, 1.0 ,1.0);
	nodes[0].valid = true; 
	nodes[0].imgBox = QRect(0, 0, maxSize, maxSize);
	if (nNodes > 1)
	{
		for (int i = 0; i < 4; i++)
			nodes[0].children[i] = makeChildren(nodes[0], nodes[0].index + i + 1, 1);
	}
	else
	{
		for (int i = 0; i < 4; i++)
			nodes[0].children[i] = NULL;
	}
	nodes[0].imgBox = QRect(-pfp, -pfp, maxSize + 2*pfp, maxSize + 2*pfp);
}


void Tree::getTreeDescriptor(QString& output, bool compact)
{
	QTextStream streamOut(&output);
	streamOut << nodes.size() << " 0\n";
	streamOut << tileSize << "\n";
	streamOut << maxSize << " " << maxSize << " 255\n";
	streamOut << "0 0 0\n";
	if (!compact)
	{
		for (unsigned int i = 0; i < nodes.size(); i++)
		{
			streamOut << nodes[i].index + 1 << " " << nodes[i].parent << " ";
			for (int j = 0; j < 4; j++)
				if (nodes[i].children[(j + 2) % 4] != NULL)
					streamOut << nodes[i].children[(j + 2) % 4]->index << " ";
				else
					streamOut <<  "-1 ";
			streamOut << tileSize << " ";
			if (nodes[i].valid)
				streamOut << "1 ";
			else
				streamOut << "0 ";
			streamOut << nodes[i].normalizeBox.left() << " " << nodes[i].normalizeBox.top() << " 0 ";
			streamOut << nodes[i].normalizeBox.right() << " " << nodes[i].normalizeBox.bottom() << " 1\n";			
		}
	}
	else
	{
		/*for (int i = 0; i < nodes.size(); i++)
		{
			if (nodes[i].valid)
				streamOut << "1 ";
			else
				streamOut << "0 ";
		}*/
	}
}


Node* Tree::makeChildren(const Node& parent, int index, int level)
{
	Node& node = nodes[index];
	node.index = index;
	node.parent = parent.index;
	int t = index % 4;
	QSizeF sizef = parent.normalizeBox.size() / 2.0;
	node.normalizeBox = parent.normalizeBox;
	node.normalizeBox.setSize(sizef);
	QSize size = parent.imgBox.size() / 2.0;
	node.imgBox = parent.imgBox;
	node.imgBox.setSize(size);
	if (t == 1)
	{
		node.normalizeBox.moveTop(parent.normalizeBox.top() + sizef.height());
	}
	else if (t == 2)
	{
		node.normalizeBox.moveLeft(parent.normalizeBox.left() + sizef.width());
		node.normalizeBox.moveTop(parent.normalizeBox.top() + sizef.height());
		node.imgBox.moveLeft(parent.imgBox.left() + size.width());
	}
	else if (t == 3)
	{
		node.imgBox.moveTop(parent.imgBox.top() + size.height());
	}
	else if (t == 0)
	{
		node.normalizeBox.moveLeft(parent.normalizeBox.left() + sizef.width());
		node.imgBox.moveLeft(parent.imgBox.left() + size.width());
		node.imgBox.moveTop(parent.imgBox.top() + size.height());
	}
	node.valid = false;
	if (imgRect.intersects(node.imgBox))
		node.valid = true;
	if (level == nLevels - 1)
	{
		for (int i = 0; i < 4; i++)
			node.children[i] = NULL;
	}
	else
	{
		for (int i = 0; i < 4; i++)
			node.children[i] = makeChildren(node, 4*index + 1 + i, level + 1);
	}
	float pfp = 1 << (nLevels - 1 - level);
	node.imgBox.setTopLeft(node.imgBox.topLeft() - QPoint(pfp, pfp));
	node.imgBox.setBottomRight(node.imgBox.bottomRight() + QPoint(pfp, pfp));
	return &nodes[index];
}


Splitter::Splitter(MultiLayerImage* image, int tileSize, int maxRamMB)
{
	this->image = image;
	this->tileSize = tileSize;
	double temp = maxRamMB;
	temp /= this->image->getNumLayers() * 3;
	unsigned int side = sqrt(temp) * 1024;
	maxLoadingSize =  1 << std::min((int)floor(log10f(side) / log10f(2.0f)), 13);
	tree.build(QRect(0, 0, image->width(), image->height()), this->tileSize);
}


bool Splitter::split(const QString& destFolder, int quality, const QString& format)
{
	int maxSize = std::max(image->width(), image->height());
	if (maxSize <= maxLoadingSize)
	{
		if (image->loadData())
		{
			for (int i = 0; i < image->getNumLayers(); i++)
			{
				QImage layer;
				image->getLayer(i, &layer);
				QImage bigImage (tree.maxSize, tree.maxSize, QImage::Format_RGB888);
				bigImage.fill(Qt::black);
				QPainter painter;
				painter.begin(&bigImage);
				painter.drawImage(tree.imgRect, layer, QRect(0, 0, layer.width(), layer.height()));
				painter.end();
				tree.nodes[0].saveTile(bigImage, destFolder, tileSize, i + 1, quality, format);
			}
		}
		else
			return false;
	}
	else
	{
		int startLevel = (int)(log10f(tree.maxSize / maxLoadingSize) / log10f(2.0f));
		const int woffset = tree.imgRect.x();
		const int hoffset = tree.imgRect.y();
		int subImgSide = 1 << startLevel;
		int numSubImg = powf(4.0, startLevel);
		int startIndex = (numSubImg - 1.0f) / 3.0f;
		QRect bigRect (0, 0, tree.maxSize, tree.maxSize);
		std::vector<QImage> upLevelImg;
		int lenght = tree.maxSize / subImgSide;
		int pfp = 1 << (tree.nLevels - 1 - startLevel);
		for (int i = 0; i < image->getNumLayers(); i++)
		{
			upLevelImg.push_back(QImage(subImgSide * tileSize, subImgSide * tileSize,  QImage::Format_RGB888));
			upLevelImg.back().fill(Qt::black);
		}
		Tree upTree;
		upTree.build(QRect(0, 0, upLevelImg[0].width() / 2.0, upLevelImg[0].height() / 2.0), this->tileSize);
		for (int i = 0; i < startIndex; i++)
			upTree.nodes[i].valid = tree.nodes[i].valid;
		for (int i = startIndex; i < startIndex + numSubImg; i++)
		{
			if (tree.nodes[i].valid)
			{
				QRect subImgRect = bigRect.intersected(tree.nodes[i].imgBox);
				QRect clipRect = subImgRect.translated(-woffset, -hoffset);
				QRect imgRect = subImgRect.intersected(tree.imgRect);
				image->setClipRect(clipRect);
				if (image->loadData())
				{
					for (int j = 0; j < image->getNumLayers(); j++)
					{
						QImage layer;
						image->getLayer(j, &layer);
						QImage bigImage (subImgRect.width(), subImgRect.height(), QImage::Format_RGB888);
						bigImage.fill(Qt::black);
						QPainter painter;
						painter.begin(&bigImage);
						QRect drawRect(imgRect.x() - subImgRect.x(), imgRect.y() - subImgRect.y(), imgRect.width(), imgRect.height());
						painter.drawImage(drawRect, layer, QRect(0, 0, layer.width(), layer.height()));
						painter.end();
						tree.nodes[i].saveTile(bigImage, subImgRect, destFolder, tileSize, j + 1, quality, format);
						QRect tempRect(tree.nodes[i].imgBox.x() - subImgRect.x(), tree.nodes[i].imgBox.y() - subImgRect.y(), tree.nodes[i].imgBox.width(), tree.nodes[i].imgBox.height());
						QImage topTile = bigImage.copy(tempRect);
						topTile = topTile.scaled(tileSize + 2, tileSize + 2, Qt::KeepAspectRatio, Qt::SmoothTransformation);
						painter.begin(&upLevelImg[j]);
						QPoint left = (tree.nodes[i].imgBox.topLeft() + QPoint(pfp, pfp)) / lenght * tileSize - QPoint(1, 1);
						painter.drawImage(left, topTile);
						painter.end();
					}
				}
				else
					return false;
			}
		}
		for (int i = 0; i < image->getNumLayers(); i++)
		{
			upLevelImg[i] = upLevelImg[i].scaled(subImgSide * tileSize / 2.0, subImgSide * tileSize / 2.0, Qt::KeepAspectRatio, Qt::SmoothTransformation);
			upTree.nodes[0].saveTile(upLevelImg[i], destFolder, tileSize, i+1, quality, format);
		}
	}
	return true;
}


void Splitter::saveDescriptor(const QString& destFolder, const QString& format)
{
	QString fileDescr = destFolder + "/info.xml";
	QDomDocument doc("WebMultiRes");
	QDomElement root = doc.createElement("MultiRes");
	doc.appendChild(root);
	if (format == "jpg")
		root.setAttribute("format", 0);
	else
		root.setAttribute("format", 1);

	QDomElement content = doc.createElement("Content");
	image->getLayerInfo(doc, content);
			
	QString treeInfo; 
	tree.getTreeDescriptor(treeInfo, true);

	QDomElement treeElem = doc.createElement("Tree");
	treeElem.appendChild(doc.createTextNode(treeInfo));

	root.appendChild(content);
	root.appendChild(treeElem);

	QFile file(fileDescr);
	if (file.open(QIODevice::WriteOnly))
	{
		QTextStream stream(&file);
		doc.save(stream, 0);
		file.close();
	}
}
