/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
 
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
 
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/     

#include <qdir.h>

#include "kfm_part.h"
#include "kfmview.h"
#include "kfmguiprops.h"

#include <qcolor.h>
#include <qmessagebox.h>
#include <qkeycode.h>

#include <opUIUtils.h>

#include <kpixmapcache.h>

KfmPart::KfmPart( QWidget *_parent ) : QWidget( _parent )
{
  cerr << "KfmPart::KfmPart( QWidget *_parent ) : QWidget( _parent )" << endl;
  setWidget( this );

  // Accept the focus
  OPPartIf::setFocusPolicy( OpenParts::Part::ClickFocus ); 

  setBackgroundColor( red );

  // Read application config file if not already done
  if (!KfmGuiProps::m_pDefaultProps)
  {
    debug("Reading global config");
    KConfig *config = kapp->getConfig();
    config->setGroup( "Settings" );
    KfmGuiProps::m_pDefaultProps = new KfmGuiProps(config);
  }
  
  // For the moment, no local properties
  // Copy the default properties
  m_Props = new KfmGuiProps( *KfmGuiProps::m_pDefaultProps );

  m_pView = new KfmView( this, this );
  m_pView->show();
  m_pView->setFocus();
}

void KfmPart::init()
{
  /******************************************************
   * Menu
   ******************************************************/

  // We want to display a menubar
  OpenParts::MenuBarManager_var menu_bar_manager = m_vMainWindow->menuBarManager();
  if ( !CORBA::is_nil( menu_bar_manager ) )
    menu_bar_manager->registerClient( id(), this );

  OpenParts::ToolBarManager_var tool_bar_manager = m_vMainWindow->toolBarManager();
  if ( !CORBA::is_nil( tool_bar_manager ) )
    tool_bar_manager->registerClient( id(), this );      
        
  OpenParts::StatusBarManager_var status_bar_manager = m_vMainWindow->statusBarManager();
  if ( !CORBA::is_nil( status_bar_manager ) )
    m_vStatusBar = status_bar_manager->registerClient( id() );

  m_vStatusBar->enable( OpenPartsUI::Show );        
  m_vStatusBar->insertItem( i18n("KFM"), 1 );

  m_pView->openURL( "file:/home/tronical/" );
}

KfmPart::~KfmPart()
{
  cout << "-KfmPart" << endl;
}
  
void KfmPart::cleanUp()
{
  // We must do that to avoid recursions.
  if ( m_bIsClean )
    return;

  // Say bye to our menu bar
  OpenParts::MenuBarManager_var menu_bar_manager = m_vMainWindow->menuBarManager();
  if ( !CORBA::is_nil( menu_bar_manager ) )
    menu_bar_manager->unregisterClient( id() );
  
  OpenParts::ToolBarManager_var tool_bar_manager = m_vMainWindow->toolBarManager();
  if ( !CORBA::is_nil( tool_bar_manager ) )
    tool_bar_manager->unregisterClient( id() );      
        
  OpenParts::StatusBarManager_var status_bar_manager = m_vMainWindow->statusBarManager();
  if ( !CORBA::is_nil( status_bar_manager ) )
    status_bar_manager->unregisterClient( id() );

  // Very important!!! The base classes must run
  // their cleanup function bevore! the constructor does it.
  // The reason is that CORBA reference counting is not very
  // happy about Qt, since Qt deletes its child widgets and does
  // not care about reference counters.
  OPPartIf::cleanUp();
}

void KfmPart::resizeEvent( QResizeEvent * )
{
  cerr << "Resizing" << endl;
  m_pView->setGeometry( 0, 0, width(), height() );
}

bool KfmPart::event( const char* _event, const CORBA::Any& _value )
{
  // Here we map events to function calls
  EVENT_MAPPER( _event, _value );

  MAPPING( OpenPartsUI::eventCreateMenuBar, OpenPartsUI::typeCreateMenuBar_var, mappingCreateMenubar );
  MAPPING( OpenPartsUI::eventCreateToolBar, OpenPartsUI::typeCreateToolBar_var, mappingCreateToolbar );

  END_EVENT_MAPPER;
  
  // No, we could not handle this event
  return false;
}

bool KfmPart::mappingCreateMenubar( OpenPartsUI::MenuBar_ptr _menubar )
{
  // Do we loose control over the menubar `
  if ( CORBA::is_nil( _menubar ) )
  {
    m_vMenuEdit = 0L;
    m_vMenuView = 0L;
    m_vMenuOptions = 0L;
    return true;
  }

  _menubar->insertMenu( i18n( "&Edit" ), m_vMenuEdit, -1, -1 );

  m_vMenuEdit->insertItem( i18n("&Copy"), this, "slotCopy", CTRL+Key_C );  
  m_vMenuEdit->insertItem( i18n("&Paste"), this, "slotPaste", CTRL+Key_V );
  m_vMenuEdit->insertItem( i18n("&Move to Trash"), this, "slotTrash", 0 );  
  m_vMenuEdit->insertItem( i18n("&Delete"), this, "slotDelete", 0 );  
  m_vMenuEdit->insertSeparator( -1 );
  m_vMenuEdit->insertItem( i18n("&Select"), this, "slotSelect", CTRL+Key_S );
  m_vMenuEdit->insertSeparator( -1 );
  m_vMenuEdit->insertItem( i18n("Mime Types"), this, "slotEditMimeTypes", 0 );
  m_vMenuEdit->insertItem( i18n("Applications"), this, "slotEditApplications", 0 );
  m_vMenuEdit->insertSeparator( -1 );
  m_vMenuEdit->insertItem( i18n("Save Geometry"), this, "slotSaveGeometry", 0 );

  _menubar->insertMenu( i18n("&View"), m_vMenuView, -1, -1 );

  m_vMenuView->setCheckable( true );
  m_vMenuView->insertItem( i18n("Show Directory Tr&ee"), this, "slotShowTree" , 0 );
  m_vMenuView->insertItem( i18n("Split &window"), this, "slotSplitView" , 0 );
  m_vMenuView->insertItem( i18n("Show &Dot Files"), this, "slotShowDot" , 0 );
  m_vMenuView->insertItem( i18n("Image &Preview"), this, "slotShowSchnauzer" , 0 );
  m_vMenuView->insertItem( i18n("&Always Show index.html"), this, "slotShowHTML" , 0 );
  m_vMenuView->insertSeparator( -1 );  
  m_vMenuView->insertItem( i18n("&Large Icons"), this, "slotLargeIcons" , 0 );
  m_vMenuView->insertItem( i18n("&Small Icons"), this, "slotSmallIcons" , 0 );
  m_vMenuView->insertItem( i18n("&Tree View"), this, "slotTreeView" , 0 );
  m_vMenuView->insertSeparator( -1 );  
  m_vMenuView->insertItem( i18n("&Use HTML"), this, "slotHTMLView" , 0 );
  m_vMenuView->insertSeparator( -1 );  
  m_vMenuView->insertItem( i18n("Rel&oad Tree"), this, "slotReloadTree" , 0 );
  m_vMenuView->insertItem( i18n("&Reload Document"), this, "slotReload" , 0 );
 
  _menubar->insertMenu( i18n("&Options"), m_vMenuOptions, -1, -1 );
  
  m_vMenuOptions->insertItem( i18n("Configure &keys"), this, "slotConfigureKeys", 0 );

  // Yes, we handled this event
  return true;
}

bool KfmPart::mappingCreateToolbar( OpenPartsUI::ToolBarFactory_ptr factory )
{
  OpenPartsUI::Pixmap_var pix;

  if (CORBA::is_nil(factory))
     {
       m_vMainToolBar = 0L;
       m_vLocationToolBar = 0L;
       
       return true;
     }

  m_vMainToolBar = factory->create( OpenPartsUI::ToolBarFactory::Transient );
  
  m_vMainToolBar->setFullWidth( false );
  
  pix = OPUIUtils::convertPixmap( *KPixmapCache::toolbarPixmap( "up.xpm" ) );
  m_vMainToolBar->insertButton2( pix, 0, SIGNAL(clicked()), 
                                 this, "slotUp", true, i18n("Up"), -1);

  pix = OPUIUtils::convertPixmap( *KPixmapCache::toolbarPixmap( "back.xpm" ) );
  m_vMainToolBar->insertButton2( pix, 1, SIGNAL(clicked()), 
                                 this, "slotBack", true, i18n("Back"), -1);

  pix = OPUIUtils::convertPixmap( *KPixmapCache::toolbarPixmap( "forward.xpm" ) );
  m_vMainToolBar->insertButton2( pix, 2, SIGNAL(clicked()), 
                                 this, "slotForward", true, i18n("Forward"), -1);

  pix = OPUIUtils::convertPixmap( *KPixmapCache::toolbarPixmap( "home.xpm" ) );
//  m_vMainToolBar->insertButton2( pix, 3, SIGNAL(clicked()), 
//                                 this, "slotHome", true, i18n("Home"), -1);

  m_vMainToolBar->insertSeparator( -1 );
  
  pix = OPUIUtils::convertPixmap( *KPixmapCache::toolbarPixmap( "editcopy.xpm" ) );
//  m_vMainToolBar->insertButton2( pix, ID_COPY, SIGNAL(clicked()), 
//                                 this, "slotCopy", true, i18n("Copy"), -1);

  pix = OPUIUtils::convertPixmap( *KPixmapCache::toolbarPixmap( "editpaste.xpm" ) );
//  m_vMainToolBar->insertButton2( pix, ID_PASTE, SIGNAL(clicked()), 
//                                 this, "slotPaste", true, i18n("Paste"), -1);
 				 
  m_vMainToolBar->insertSeparator( -1 );				 

  pix = OPUIUtils::convertPixmap( *KPixmapCache::toolbarPixmap( "help.xpm" ) );
//  m_vMainToolBar->insertButton2( pix, ID_HELP, SIGNAL(clicked()), 
//                                 this, "slotHelp", true, i18n("Stop"), -1);
				 
  m_vMainToolBar->insertSeparator( -1 );				 

  pix = OPUIUtils::convertPixmap( *KPixmapCache::toolbarPixmap( "stop.xpm" ) );
//  m_vMainToolBar->insertButton2( pix, ID_STOP, SIGNAL(clicked()), 
//                                 this, "slotStop", true, i18n("Stop"), -1);
				 
  m_vLocationToolBar = factory->create( OpenPartsUI::ToolBarFactory::Transient );
  
  m_vLocationToolBar->setFullWidth( true );

  m_vLocationToolBar->insertTextLabel( i18n("Location : "), -1, -1 );
  
  m_vLocationToolBar->insertLined("", ID_LOCATION, SIGNAL(returnPressed()), this, "slotURLEntered", true, i18n("Current Location"), 70, -1 );
  m_vLocationToolBar->setItemAutoSized( ID_LOCATION, true );
    
  m_vMainToolBar->enable( OpenPartsUI::Show );
  m_vLocationToolBar->enable( OpenPartsUI::Show );

  m_vLocationToolBar->setLinedText( ID_LOCATION, m_pView->workingURL() );
  
  return true;
}

void KfmPart::slotURLEntered()
{
  string url = m_vLocationToolBar->linedText( ID_LOCATION );
  
  // Exit if the user did not enter an URL
  if ( url.empty() )
    return;

  // Root directory?
  if ( url[0] == '/' )
  {
    K2URL::encode( url );
  }
  // Home directory?
  else if ( url[0] == '~' )
  {
    QString tmp( QDir::homeDirPath().data() );
    tmp += m_vLocationToolBar->linedText( ID_LOCATION ) + 1;
    K2URL u( tmp );
    url = u.url();
  }
  else if ( strncmp( url.c_str(), "www.", 4 ) == 0 )
  {
    string tmp = "http://";
    K2URL::encode( url );
    tmp += url;
    url = tmp;
  }
  else if ( strncmp( url.c_str(), "ftp.", 4 ) == 0 )
  {
    string tmp = "ftp://";
    K2URL::encode( url );
    tmp += url;
    url = tmp;
  }
  
  K2URL u( url.c_str() );
  if ( u.isMalformed() )
  {
    string tmp = i18n("Malformed URL\n");
    tmp += m_vLocationToolBar->linedText( ID_LOCATION );
    QMessageBox::critical( (QWidget*)0L, i18n( "KFM Error" ), tmp.c_str(), i18n( "OK" ) );
    return;
  }
	
//  m_bBack = false;
//  m_bForward = false;

  m_pView->openURL( url.c_str() );
}

void KfmPart::bookmarkSelected( CORBA::Long id )
{
  // TODO
}

void KfmPart::setStatusBarText( const char *_text )
{
  // TODO
}

void KfmPart::setLocationBarURL( const char *_url )
{
  // TODO
}

void KfmPart::setUpURL( const char *_url )
{
  // TODO
}
  
void KfmPart::addHistory( const char *_url, int _xoffset, int _yoffset )
{
  // TODO
}

void KfmPart::createGUI( const char *_url )
{
  // TODO
}

bool KfmPart::hasUpURL()
{
  return false;
}

bool KfmPart::hasBackHistory()
{
  return false;
}

bool KfmPart::hasForwardHistory()
{
  return false;
}

#include "kfm_part.moc"


