//
//
// "Misc Options" Tab for KFM configuration
//
// (c) Sven Radej 1998
// (c) David Faure 1998

#include <qlabel.h>
#include <qgroupbox.h>
#include <qlayout.h>//CT - 12Nov1998
#include <qwhatsthis.h>
#include <kapp.h>

#include "miscopts.h"

#include <konqdefaults.h> // include default values directly from konqueror
#include <mousedefaults.h> // get default for DEFAULT_CHANGECURSOR
#include <klocale.h>
#include <kconfig.h>

extern KConfig *g_pConfig;
extern QString g_groupname;


//-----------------------------------------------------------------------------

KMiscHTMLOptions::KMiscHTMLOptions(KConfig *config, QString group, QWidget *parent, const char *name )
    : KCModule( parent, name ), g_pConfig(config), groupname(group)
{
    QVBoxLayout *lay = new QVBoxLayout(this, 10, 5);

    cbCursor = new QCheckBox(i18n("&Change cursor over links"), this);
    lay->addWidget(cbCursor);

    QWhatsThis::add( cbCursor, i18n("If this option is set, the shape of the cursor will change "
       "(usually to a hand) if it is moved over a hyperlink.") );

    connect(cbCursor, SIGNAL(clicked()), this, SLOT(changed()));

    cbUnderline = new QCheckBox(i18n("&Underline links"), this);
    lay->addWidget(cbUnderline);

    QWhatsThis::add( cbUnderline, i18n("Tells konqueror to underline hyperlinks.") );

    connect(cbUnderline, SIGNAL(clicked()), this, SLOT(changed()));

    m_pAutoLoadImagesCheckBox = new QCheckBox( i18n( ""
     "Automatically load images\n"
     "(Otherwise, click the Images button to load when needed)" ), this );

    QWhatsThis::add( m_pAutoLoadImagesCheckBox, "FIXME" );

    connect(m_pAutoLoadImagesCheckBox, SIGNAL(clicked()), this, SLOT(changed()));

    lay->addWidget( m_pAutoLoadImagesCheckBox, 1 );

    lay->addStretch(10);
    lay->activate();

    load();
}

void KMiscHTMLOptions::load()
{
    // *** load ***
    g_pConfig->setGroup( "HTML Settings" );
    bool changeCursor = g_pConfig->readBoolEntry("ChangeCursor", DEFAULT_CHANGECURSOR);
    bool underlineLinks = g_pConfig->readBoolEntry("UnderlineLinks", DEFAULT_UNDERLINELINKS);
    bool bAutoLoadImages = g_pConfig->readBoolEntry( "AutoLoadImages", true );

    // *** apply to GUI ***

    cbCursor->setChecked( changeCursor );
    cbUnderline->setChecked( underlineLinks );
    m_pAutoLoadImagesCheckBox->setChecked( bAutoLoadImages );
}

void KMiscHTMLOptions::defaults()
{
    cbCursor->setChecked( false );
    cbUnderline->setChecked( true );
    m_pAutoLoadImagesCheckBox->setChecked( false );
}

void KMiscHTMLOptions::save()
{
    g_pConfig->setGroup( "HTML Settings" );
    g_pConfig->writeEntry( "ChangeCursor", cbCursor->isChecked() );
    g_pConfig->writeEntry( "UnderlineLinks", cbUnderline->isChecked() );
    g_pConfig->writeEntry( "AutoLoadImages", m_pAutoLoadImagesCheckBox->isChecked() );
    g_pConfig->sync();
}

void KMiscHTMLOptions::changed()
{
    emit KCModule::changed(true);
}

#include "miscopts.moc"
