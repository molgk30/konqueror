/* This file is part of the KDE projects
   Copyright (C) 2000 David Faure <faure@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __konqdirpart_h
#define __konqdirpart_h

#include <qstring.h>
#include <kparts/part.h>
#include <kfileitem.h>

namespace KParts { class BrowserExtension; }
class KonqPropsView;
class QScrollView;
class KAction;
class KToggleAction;

class KonqDirPart: public KParts::ReadOnlyPart
{
  Q_OBJECT
public:
    KonqDirPart( QObject *parent, const char *name );
    virtual ~KonqDirPart();

    /**
     * The derived part should call this in its constructor
     */
    void setBrowserExtension( KParts::BrowserExtension * extension )
      { m_extension = extension; }
    KParts::BrowserExtension * extension()
      { return m_extension; }

    QScrollView * scrollWidget();

    virtual void saveState( QDataStream &stream );
    virtual void restoreState( QDataStream &stream );

    void mmbClicked( KFileItem * fileItem );

    void setNameFilter( const QString & nameFilter ) { m_nameFilter = nameFilter; }
    QString nameFilter() const { return m_nameFilter; }

    KonqPropsView * props() const { return m_pProps; }

    /**
     * "Cut" icons : disable those whose URL is in lst, enable the others
     */
    virtual void disableIcons( const KURL::List & lst ) = 0;

    /**
     * This class takes care of the counting of items, size etc. in the
     * current directory. Call this in openURL.
     */
    void resetCount()
    {
        m_lDirSize = 0;
        m_lFileCount = 0;
        m_lDirCount = 0;
    }
    /**
     * Update the counts for those new items
     */
    void newItems( const KFileItemList & entries );
    /**
     * Update the counts with this item being deleted
     */
    void deleteItem( KFileItem * fileItem );
    /**
     * Show the counts for the directory in the status bar
     */
    void emitTotalCount();
    /**
     * Show the counts for the selected items in the status bar, if any
     * otherwise show the info for the directory.
     * @param selectionChanged if true, we'll emit selectionInfo.
     */
    void emitCounts( const KFileItemList & lst, bool selectionChanged );

    /**
     * This is called by the actions that change the icon size. The view
     * should also call it initially, or any time it wants to change the size.
     * The view should also reimplement it, to update the view.
     */
    virtual void newIconSize( int size );

public slots:
    void slotBackgroundColor();
    void slotBackgroundImage();
    /**
     * Called when the clipboard's data changes, to update the 'cut' icons
     * Call this when the directory's listing is finished, to draw icons as cut.
     */
    void slotClipboardDataChanged();

    void slotIncIconSize();
    void slotDecIconSize();

    void slotIconSizeToggled( bool );

protected:
    QString m_nameFilter;

    KParts::BrowserExtension * m_extension;
    /**
     * View properties
     */
    KonqPropsView * m_pProps;

    KAction *m_paIncIconSize;
    KAction *m_paDecIconSize;
    KToggleAction *m_paDefaultIcons;
    KToggleAction *m_paLargeIcons;
    KToggleAction *m_paMediumIcons;
    KToggleAction *m_paSmallIcons;

    int m_iIconSize[4];

    unsigned long m_lDirSize;
    uint m_lFileCount;
    uint m_lDirCount;
};

#endif
