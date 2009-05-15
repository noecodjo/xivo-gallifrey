/* XiVO Client
 * Copyright (C) 2007-2009, Proformatique
 *
 * This file is part of XiVO Client.
 *
 * XiVO Client is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version, with a Section 7 Additional
 * Permission as follows:
 *   This notice constitutes a grant of such permission as is necessary
 *   to combine or link this software, or a modified version of it, with
 *   the OpenSSL project's "OpenSSL" library, or a derivative work of it,
 *   and to copy, modify, and distribute the resulting work. This is an
 *   extension of the special permission given by Trolltech to link the
 *   Qt code with the OpenSSL library (see
 *   <http://doc.trolltech.com/4.4/gpl.html>). The OpenSSL library is
 *   licensed under a dual license: the OpenSSL License and the original
 *   SSLeay license.
 *
 * XiVO Client is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with XiVO Client.  If not, see <http://www.gnu.org/licenses/>.
 */

/* $Revision$
 * $Date$
 */

#ifndef __PEERSLAYOUT_H__
#define __PEERSLAYOUT_H__
#include <QLayout>

/*! \brief Grid layout to organize the peers
 *
 * Layout to organize the peers on the screen
 * The peers are movable.
 */
class PeersLayout : public QLayout
{
    Q_OBJECT
public:
    //! constructor
    PeersLayout(QWidget * parent = 0);
    //! destructor
    virtual ~PeersLayout();
    //! set geometry
    void setGeometry( const QRect & );
    //! return size Hint (prefered size)
    QSize sizeHint() const;
    QSize minimumSize() const;
    QSize maximumSize() const;
    void addWidget(QWidget *, QPoint);
    //! add the Item at a specific emplacement
    void addItem(QLayoutItem *, QPoint);
    //! default addItem implementation
    void addItem(QLayoutItem *);
    //! return the number of items
    int count() const;
    QLayoutItem* itemAt(int) const;
    QLayoutItem* takeAt(int);
    //! get m_nb_rows
    int nbRows() const { return m_nb_rows; };
    //! set m_nb_rows
    void setNbRows(int rows) { m_nb_rows = rows; };
    //! get m_nb_columns
    int nbColumns() const { return m_nb_columns; };
    //! set m_nb_columns
    void setNbColumns(int cols) { m_nb_columns = cols; };
    QPoint getPosInGrid(const QPoint &) const;
    QPoint getPosFromGrid(const QPoint &) const;
    void setItemPosition(int i, QPoint pos);
    void setItemPosition(QWidget * widget, QPoint pos);
    QPoint getItemPosition(int i) const;
    QPoint getItemPosition(QWidget * widget) const;
    QRect getGridRect( const QRect & ) const;
private:
    int itemIndex(QWidget * widget) const;
    QPoint freePosition() const;
    QSize size() const;
    QSize maxItemSize() const;
    QList<QLayoutItem *> m_list;        //!< layout items list
    QList<QPoint> m_listPos;            //!< positions list
    int m_nb_rows;                      //!< height
    int m_nb_columns;                   //!< width
};

#endif

