#include <qdir.h>

#include <opMenu.h>

#include <kbookmark.h>
#include <kio_job.h>
#include <kio_openwith.h>
#include <kio_paste.h>
#include <kio_propsdlg.h>
#include <kpixmapcache.h>
#include <kprotocolmanager.h>
#include <krun.h>
#include <kservices.h>
#include <kurl.h>
#include <kuserprofile.h>
#include <userpaths.h>

#include "knewmenu.h"
#include "kpopupmenu.h"

KonqPopupMenu::KonqPopupMenu( QStringList urls,
                              mode_t mode,
                              QString viewURL, 
                              bool canGoBack, 
                              bool canGoForward )
  : m_pMenuNew(0L), m_sViewURL(viewURL), m_lstPopupURLs(urls), m_popupMode(mode)
{
  assert( m_lstPopupURLs.count() >= 1 );

  m_popupMenu = new OPMenu;
  bool bHttp          = true;
  bool isTrash        = true;
  bool currentDir     = false;
  bool isCurrentTrash = false;
  bool sReading       = true;
  bool sWriting       = true;
  bool sDeleting      = true;
  bool sMoving        = true;
  bool hasUpURL       = false;
  int id;

  KProtocolManager pManager = KProtocolManager::self();
  
  KURL url;
  QStringList::ConstIterator it = m_lstPopupURLs.begin();
  // Check whether all URLs are correct
  for ( ; it != m_lstPopupURLs.end(); it++ )
  {
    url = KURL( *it );

    if ( url.isMalformed() )
    {
//FIXME?
//      emit error( ERR_MALFORMED_URL, s );
      return;
    }
    QString protocol = url.protocol();

    if ( protocol != "http" ) bHttp = false; // not HTTP

    // check if all urls are in the trash
    if ( isTrash )
    {
      QString path = url.path();
      if ( path.right(1) != "/" )
	path += "/";
    
      if ( protocol != "file" ||
	   path != UserPaths::trashPath() )
	isTrash = false;
    }

    if ( sReading )
      sReading = pManager.supportsReading( protocol );

    if ( sWriting )
      sWriting = pManager.supportsWriting( protocol );
    
    if ( sDeleting )
      sDeleting = pManager.supportsDeleting( protocol );

    if ( sMoving )
      sMoving = pManager.supportsMoving( protocol );
  }

  //check if current url is trash
  url = KURL( m_sViewURL );
  url.cleanPath();
    
  if ( url.protocol() == "file" &&
       url.path(1) == UserPaths::trashPath() )
    isTrash = true;

  //check if url is current directory
  if ( m_lstPopupURLs.count() == 1 )
  {
    KURL firstPopupURL = KURL(m_lstPopupURLs.getFirst());
    firstPopupURL.cleanPath();
    kdebug(0, 1202, "View path is %s",url.path(1).data());
    kdebug(0, 1202, "First popup path is %s",firstPopupURL.path(1).data());
    if ( firstPopupURL.protocol() == url.protocol()
         && url.path(1) == firstPopupURL.path(1) )
    {
      currentDir = true;
      // ok, now check if we enable 'up'
      if ( url.hasPath() )
        hasUpURL = ( url.path(1) != "/");
    }
  }
  
  QObject::disconnect( m_popupMenu, SIGNAL( activated( int ) ), this, SLOT( slotPopup( int ) ) );

  m_popupMenu->clear();

//   //---------- Sven --------------
//   // check if menubar is hidden and if yes add "Show Menubar"
//   if (view->getGUI()->isMenuBarHidden())
//   {
    
//     popupMenu->insertItem(klocale->getAlias(ID_STRING_SHOW_MENUBAR),
// 			  view->getGUI(), SLOT(slotShowMenubar()));
//     popupMenu->insertSeparator();
//   }
//   //------------------------------

  if ( isTrash )
  {
    id = m_popupMenu->insertItem( i18n( "New view" ), 
				  this, SLOT( slotPopupNewView() ) );
    m_popupMenu->insertSeparator();    
    id = m_popupMenu->insertItem( i18n( "Empty Trash Bin" ), 
				  this, SLOT( slotPopupEmptyTrashBin() ) );
  } 
  else if ( S_ISDIR( (mode_t)m_popupMode ) )
  {
    //we don't want to use OpenParts here, because of "missing" interface 
    //methods for the popup menu (wouldn't make much sense imho) (Simon)    
    m_pMenuNew = new KNewMenu(); 
    id = m_popupMenu->insertItem( i18n("&New"), m_pMenuNew->popupMenu() );
    m_popupMenu->insertSeparator();

    if ( currentDir ) {
      id = m_popupMenu->insertItem( *KPixmapCache::toolbarPixmap( "up.xpm" ), i18n( "Up" ), KPOPUPMENU_UP_ID );
      m_popupMenu->setItemEnabled( id, hasUpURL );

      id = m_popupMenu->insertItem( *KPixmapCache::toolbarPixmap( "back.xpm" ), i18n( "Back" ), KPOPUPMENU_BACK_ID );
      m_popupMenu->setItemEnabled( id, canGoBack );

      id = m_popupMenu->insertItem( *KPixmapCache::toolbarPixmap( "forward.xpm" ), i18n( "Forward" ), KPOPUPMENU_FORWARD_ID );
      m_popupMenu->setItemEnabled( id, canGoForward );

      m_popupMenu->insertSeparator();  
    }

    id = m_popupMenu->insertItem( i18n( "New View"), this, SLOT( slotPopupNewView() ) );
    m_popupMenu->insertSeparator();    

    if ( sReading )
      id = m_popupMenu->insertItem( *KPixmapCache::toolbarPixmap( "editcopy.xpm" ), i18n( "Copy" ), this, SLOT( slotPopupCopy() ) );
    if ( sWriting )
      id = m_popupMenu->insertItem( *KPixmapCache::toolbarPixmap( "editpaste.xpm" ), i18n( "Paste" ), this, SLOT( slotPopupPaste() ) );
    if ( isClipboardEmpty() )
      m_popupMenu->setItemEnabled( id, false );
    if ( sMoving && !isCurrentTrash && !currentDir )
      id = m_popupMenu->insertItem( *KPixmapCache::pixmap( "kfm_trash.xpm", true ), i18n( "Move to trash" ), this, SLOT( slotPopupTrash() ) );
    if ( sDeleting && !currentDir )
      id = m_popupMenu->insertItem( i18n( "Delete" ), this, SLOT( slotPopupDelete() ) );
  }
  else
  {
    if ( bHttp )
    {
      /* Should be for http URLs (HTML pages) only ... */
      id = m_popupMenu->insertItem( i18n( "New View"), this, SLOT( slotPopupNewView() ) );
    }
    id = m_popupMenu->insertItem( i18n( "Open with" ), this, SLOT( slotPopupOpenWith() ) );
    m_popupMenu->insertSeparator();

    if ( sReading )
      id = m_popupMenu->insertItem( *KPixmapCache::toolbarPixmap( "editcopy.xpm" ), i18n( "Copy" ), this, SLOT( slotPopupCopy() ) );
    if ( sMoving && !isCurrentTrash && !currentDir )
      id = m_popupMenu->insertItem( *KPixmapCache::pixmap( "kfm_trash.xpm", true ), i18n( "Move to trash" ), this, SLOT( slotPopupTrash() ) );
    if ( sDeleting && !currentDir )
      id = m_popupMenu->insertItem( i18n( "Delete" ), this, SLOT( slotPopupDelete() ) );
  }

  id = m_popupMenu->insertItem( i18n( "Add To Bookmarks" ), this, SLOT( slotPopupAddToBookmark() ) );

  if ( m_pMenuNew ) m_pMenuNew->setPopupFiles( m_lstPopupURLs );

  // Do all URLs have the same mimetype ?
  url = KURL( m_lstPopupURLs.getFirst() );

  KMimeType* mime = KMimeType::findByURL( url, m_popupMode );
  ASSERT( mime );
  it = m_lstPopupURLs.begin();
  for( ++it /* skip first */; it != m_lstPopupURLs.end(); ++it )
  {
    KURL u( *it );  
    KMimeType* m = KMimeType::findByURL( u, m_popupMode );
    if ( m != mime )
      mime = 0L;
  }
  
  if ( mime )
  {
    KServiceTypeProfile::OfferList offers = KServiceTypeProfile::offers( mime->name() );

    QValueList<KDELnkMimeType::Service> builtin;
    QValueList<KDELnkMimeType::Service> user;
    if ( mime->name() == "application/x-kdelnk" )
    {
      builtin = KDELnkMimeType::builtinServices( url );
      user = KDELnkMimeType::userDefinedServices( url );
    }
  
    if ( !offers.isEmpty() || !user.isEmpty() || !builtin.isEmpty() )
      QObject::connect( m_popupMenu, SIGNAL( activated( int ) ), this, SLOT( slotPopup( int ) ) );

    if ( !offers.isEmpty() || !user.isEmpty() )
      m_popupMenu->insertSeparator();
  
    m_mapPopup.clear();
    m_mapPopup2.clear();
  
    KServiceTypeProfile::OfferList::Iterator it = offers.begin();
    for( ; it != offers.end(); it++ )
    {    
      id = m_popupMenu->insertItem( *(KPixmapCache::pixmap( it->service().icon(), true ) ),
				    it->service().name() );
      m_mapPopup[ id ] = &(it->service());
    }
    
    QValueList<KDELnkMimeType::Service>::Iterator it2 = user.begin();
    for( ; it2 != user.end(); ++it2 )
    {
      if ( !it2->m_strIcon.isEmpty() )
	id = m_popupMenu->insertItem( *(KPixmapCache::pixmap( it2->m_strIcon, true ) ), it2->m_strName );
      else
	id = m_popupMenu->insertItem( it2->m_strName );
      m_mapPopup2[ id ] = *it2;
    }
    
    if ( builtin.count() > 0 )
      m_popupMenu->insertSeparator();

    it2 = builtin.begin();
    for( ; it2 != builtin.end(); ++it2 )
    {
      if ( !it2->m_strIcon.isEmpty() )
	id = m_popupMenu->insertItem( *(KPixmapCache::pixmap( it2->m_strIcon, true ) ), it2->m_strName );
      else
	id = m_popupMenu->insertItem( it2->m_strName );
      m_mapPopup2[ id ] = *it2;
    }
  }
  
  if ( m_lstPopupURLs.count() == 1 )
  {
    m_popupMenu->insertSeparator();
    m_popupMenu->insertItem( i18n("Properties"), this, SLOT( slotPopupProperties() ) );
  }
}

int KonqPopupMenu::exec( QPoint p )
{
  return m_popupMenu->exec( p );
}

KonqPopupMenu::~KonqPopupMenu()
{
  delete m_popupMenu;
  if ( m_pMenuNew ) delete m_pMenuNew;
}

void KonqPopupMenu::slotFileNewActivated( CORBA::Long id )
{
  if ( m_pMenuNew )
     {
       QStringList urls;
       urls.append( m_sViewURL );

       m_pMenuNew->setPopupFiles( urls );

       m_pMenuNew->slotNewFile( (int)id );
     }
}

void KonqPopupMenu::slotFileNewAboutToShow()
{
  if ( m_pMenuNew )
    m_pMenuNew->slotCheckUpToDate();
}


void KonqPopupMenu::slotPopupNewView()
{
  QStringList::ConstIterator it = m_lstPopupURLs.begin();
  for ( ; it != m_lstPopupURLs.end(); it++ )
    (void) new KRun(*it);
}

void KonqPopupMenu::slotPopupEmptyTrashBin()
{
  //TODO
}

void KonqPopupMenu::slotPopupCopy()
{
  // TODO (kclipboard.h will probably have to be ported to QStringList)
  //  KClipboard::self()->setURLList( m_lstPopupURLs ); 
}

void KonqPopupMenu::slotPopupPaste()
{
  assert( m_lstPopupURLs.count() == 1 );
  pasteClipboard( m_lstPopupURLs.getFirst() );
}

void KonqPopupMenu::slotPopupTrash()
{
  //TODO
}

void KonqPopupMenu::slotPopupDelete()
{
  KIOJob *job = new KIOJob;
  list<string> lst;
  QStringList::Iterator it = m_lstPopupURLs.begin();
  for ( ; it != m_lstPopupURLs.end(); ++it )
    lst.push_back( it->data() );
  job->del( lst );
}

void KonqPopupMenu::slotPopupOpenWith()
{
  OpenWithDlg l( i18n("Open With:"), "", (QWidget*)0L, true );
  if ( l.exec() )
  {
    KService *service = l.service();
    if ( service )
    {
      KRun::run( *service, m_lstPopupURLs );
      return;
    }
    else
    {
      QString exec = l.text();
      exec += " %f";
      KRun::runOldApplication( exec, m_lstPopupURLs, false );
    }
  }
}

void KonqPopupMenu::slotPopupAddToBookmark()
{
  KBookmark *root = KBookmarkManager::self()->root();
  QStringList::ConstIterator it = m_lstPopupURLs.begin();
  for ( ; it != m_lstPopupURLs.end(); it++ )
    (void)new KBookmark( KBookmarkManager::self(), root, *it, *it );
}

void KonqPopupMenu::slotPopup( int id )
{
  // Is it a usual service
  QMap<int,const KService *>::Iterator it = m_mapPopup.find( id );
  if ( it != m_mapPopup.end() )
  {
    KRun::run( *(it.data()), m_lstPopupURLs );
    return;
  }
  
  // Is it a service specific to kdelnk files ?
  QMap<int,KDELnkMimeType::Service>::Iterator it2 = m_mapPopup2.find( id );
  if ( it2 == m_mapPopup2.end() )
    return;

  QStringList::Iterator it3 = m_lstPopupURLs.begin();
  for( ; it3 != m_lstPopupURLs.end(); ++it3 )
    KDELnkMimeType::executeService( *it3, it2.data() );

  return;
}

void KonqPopupMenu::slotPopupProperties()
{
  assert ( m_lstPopupURLs.count() == 1 );
  (void) new PropertiesDialog( m_lstPopupURLs.getFirst(), m_popupMode );
}

#include "kpopupmenu.moc"
