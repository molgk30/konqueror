/**
 * kcookiespolicies.cpp - Cookies configuration
 *
 * Original Authors
 * Copyright (c) Waldo Bastian <bastian@kde.org>
 * Copyright (c) 1999 David Faure <faure@kde.org>
 *
 * Re-written by:
 * Copyright (c) 2000- Dawit Alemayehu <adawit@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <qvbox.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qwhatsthis.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qvbuttongroup.h>

#include <kidna.h>
#include <kmessagebox.h>
#include <klistview.h>
#include <klocale.h>
#include <kconfig.h>
#include <kdebug.h>
#include <dcopclient.h>

#include "ksaveioconfig.h"

#include "kcookiespolicies.h"
#include "kcookiespoliciesdlg_ui.h"

KCookiesPolicies::KCookiesPolicies(QWidget *parent)
                 :KCModule(parent, "kcmkio")
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this, 0, 0);

    dlg = new KCookiesPolicyDlgUI (this);
    mainLayout->addWidget(dlg);

    QWhatsThis::add( dlg->cbEnableCookies, i18n("Enable cookie support. Normally "
                                            "you will want to have cookie support "
                                            "enabled and customize it to suit your "
                                            "privacy needs.<p>Please note that "
                                            "disabling cookie support makes many "
                                            "of web today's web sites unbrowsable.") );

    QWhatsThis::add( dlg->cbRejectCrossDomainCookies,
                     i18n("Reject the so called third-party cookies. These "
                          "are cookies that originate from a site other than "
                          "the one you are currently browsing. For example, if "
                          "you visit www.foobar.com while this option is on, "
                          "only cookies that originate from www.foobar.com "
                          "will be processed per your settings. Cookies from "
                          "any other site will be rejected. This reduces the "
                          "chances of site operators compiling a profile about "
                          "your daily browsing habits.") );

    QWhatsThis::add( dlg->cbAutoAcceptSessionCookies,
                     i18n("Automatically accept temporary cookies mean to "
                          "expire at the end of the current session. Such "
                          "cookies will not be stored in your computer's "
                          "hard drive or storage device. Instead, they are "
                          "deleted when you close all applications (e.g. "
                          "your browser) that use them."
                          "<P>NOTE: Checking this option along with the next "
                          "one will override your default as well as site "
                          "specific cookie policies. However, doing so also "
                          "increases your privacy since all cookies will be "
                          "removed when the current session ends.") );

    QWhatsThis::add( dlg->cbIgnoreCookieExpirationDate,
                     i18n("Treat all cookies as session cookies. Session "
                          "cookies are small pieces of data that are temporarily "
                          "stored in your computer's memory until you quit or "
                          "close all applications (e.g. your browser) that "
                          "use use them. Unlike regular cookies, session "
                          "cookies are never stored on your hard drive or"
                          "other storage medium."
                          "<P>NOTE: checking this option along with the "
                          "previous one will override your default as well "
                          "as site specific cookie policies. However, doing "
                          "so also increases your privacy since all cookies "
                          "will be removed when the current session ends.") );

    QWhatsThis::add( dlg->bgDefault,
                     i18n("Determines how cookies received from a remote "
                          "machine will be handled: "
                          "<ul><li><b>Ask</b> will cause KDE to ask for your "
                          "confirmation whenever a server wants to set a cookie."
                          "</li><li><b>Accept</b> will cause cookies to be "
                          "accepted without prompting you.</li><li><b>Reject</b> "
                          "will cause the cookiejar to refuse all cookies it "
                          "receives.</li></ul>"
                          "<u>NOTE:<u>Domain specific policies, which can be set "
                          "below, always take precedence over the default policy.") );


    dlg->lvDomainPolicy->addColumn(i18n("Domain"));
    dlg->lvDomainPolicy->addColumn(i18n("Policy"), 100);

    QString wtstr = i18n("Domains for which you have set a specific cookie "
                         "policy. These specific policies override the default "
                         "policy setting for the given site(s).");

    QWhatsThis::add( dlg->lvDomainPolicy, wtstr );
    QWhatsThis::add( dlg->gbDomainSpecific, wtstr );

    connect( dlg->pbNew, SIGNAL(clicked()), SLOT( addPressed() ) );
    connect( dlg->pbChange, SIGNAL( clicked() ), SLOT( changePressed() ) );
    connect( dlg->pbDelete, SIGNAL( clicked() ), SLOT( deletePressed() ) );
    connect( dlg->pbDeleteAll, SIGNAL( clicked() ), SLOT( deleteAllPressed() ) );

    QWhatsThis::add( dlg->gbDomainSpecific, i18n("To add a new policy, simply click on the <i>Add...</i> "
                                             "button and supply the necessary information. To change an "
                                             "existing policy, click on the <i>Change...</i> button and "
                                             "choose the new policy from the policy dialog box. Clicking "
                                             "on the <i>Delete</i> button will remove the selected policy "
                                             "causing the default policy setting to be used for that "
                                             "domain.") );
    load();
}

KCookiesPolicies::~KCookiesPolicies()
{
}

void KCookiesPolicies::emitChanged ()
{
    emit changed ( true );
}

void KCookiesPolicies::cookiesEnabled( bool enable )
{
    dlg->bgDefault->setEnabled( enable );
    dlg->bgPreferences->setEnabled ( enable );
    dlg->gbDomainSpecific->setEnabled( enable );

    if (enable)
    {
      ignoreCookieExpirationDate ( enable );
      autoAcceptSessionCookies ( enable );
    }
}

void KCookiesPolicies::ignoreCookieExpirationDate ( bool enable )
{
    bool isAutoAcceptChecked = dlg->cbAutoAcceptSessionCookies->isChecked();
    enable = (enable && isAutoAcceptChecked);

    dlg->bgDefault->setEnabled( !enable );
    dlg->gbDomainSpecific->setEnabled( !enable );
}

void KCookiesPolicies::autoAcceptSessionCookies ( bool enable )
{
    bool isIgnoreExpirationChecked = dlg->cbIgnoreCookieExpirationDate->isChecked();
    enable = (enable && isIgnoreExpirationChecked);

    dlg->bgDefault->setEnabled( !enable );
    dlg->gbDomainSpecific->setEnabled( !enable );
}

void KCookiesPolicies::addNewPolicy(const QString& domain)
{
  PolicyDlg pdlg (i18n("New Cookie Policy"), this);
  pdlg.setEnableHostEdit (true, domain);

  if (dlg->rbPolicyAccept->isChecked())
    pdlg.setPolicy(KCookieAdvice::Reject);
  else
    pdlg.setPolicy(KCookieAdvice::Accept);

  if (pdlg.exec() && !pdlg.domain().isEmpty())
  {
    QString domain = KIDNA::toUnicode(pdlg.domain());
    int advice = pdlg.advice();

    if ( !handleDuplicate(domain, advice) )
    {
        const char* strAdvice = KCookieAdvice::adviceToStr(advice);
        QListViewItem* index = new QListViewItem (dlg->lvDomainPolicy,
                                                  domain, i18n(strAdvice));
        m_pDomainPolicy.insert (index, strAdvice);
        changed( true );
    }
  }
}


void KCookiesPolicies::addPressed()
{
  addNewPolicy (QString::null);
}

void KCookiesPolicies::changePressed()
{
  QListViewItem* index = dlg->lvDomainPolicy->currentItem();

  if (!index)
    return;

  QString oldDomain = index->text(0);

  PolicyDlg pdlg (i18n("Change Cookie Policy"), this);
  pdlg.setPolicy (KCookieAdvice::strToAdvice(m_pDomainPolicy[index]));
  pdlg.setEnableHostEdit (true, oldDomain);

  if( pdlg.exec() && !pdlg.domain().isEmpty())
  {
    QString newDomain = KIDNA::toUnicode(pdlg.domain());
    int advice = pdlg.advice();
    if (newDomain == oldDomain || !handleDuplicate(newDomain, advice))
    {
      m_pDomainPolicy[index] = KCookieAdvice::adviceToStr(advice);
      index->setText(0, newDomain);
      index->setText(1, i18n(m_pDomainPolicy[index]) );
      changed( true );
    }
  }
}

bool KCookiesPolicies::handleDuplicate( const QString& domain, int advice )
{
  QListViewItem* item = dlg->lvDomainPolicy->firstChild();
  while ( item != 0 )
  {
    if ( item->text(0) == domain )
    {
      QString msg = i18n("<qt>A policy already exists for"
                         "<center><b>%1</b></center>"
                         "Do you want to replace it?</qt>").arg(domain);
      int res = KMessageBox::warningYesNo(this, msg,
                                          i18n("Duplicate Policy"),
                                          QString::null);
      if ( res == KMessageBox::Yes )
      {
        m_pDomainPolicy[item]= KCookieAdvice::adviceToStr(advice);
        item->setText(0, domain);
        item->setText(1, i18n(m_pDomainPolicy[item]));
        changed( true );
        return true;
      }
      else
        return true;  // User Cancelled!!
    }
    item = item->nextSibling();
  }
  return false;
}

void KCookiesPolicies::deletePressed()
{
  QListViewItem* nextItem = 0L;
  QListViewItem* item = dlg->lvDomainPolicy->firstChild ();

  while (item != 0L)
  {
    if (dlg->lvDomainPolicy->isSelected (item))
    {
      nextItem = item->itemBelow();
      if ( !nextItem )
        nextItem = item->itemAbove();

      delete item;
      item = nextItem;
    }
    else
    {
      item = item->itemBelow();
    }
  }

  if (nextItem)
    dlg->lvDomainPolicy->setSelected (nextItem, true);

  updateButtons();
  changed( true );
}

void KCookiesPolicies::deleteAllPressed()
{
    m_pDomainPolicy.clear();
    dlg->lvDomainPolicy->clear();
    updateButtons();
    changed( true );
}

void KCookiesPolicies::updateButtons()
{
  bool hasItems = dlg->lvDomainPolicy->childCount() > 0;

  dlg->pbChange->setEnabled ((hasItems && d_itemsSelected == 1));
  dlg->pbDelete->setEnabled ((hasItems && d_itemsSelected > 0));
  dlg->pbDeleteAll->setEnabled ( hasItems );
}

void KCookiesPolicies::updateDomainList(const QStringList &domainConfig)
{
    dlg->lvDomainPolicy->clear();

    QStringList::ConstIterator it = domainConfig.begin();
    for (; it != domainConfig.end(); ++it)
    {
      QString domain;
      KCookieAdvice::Value advice;
      QListViewItem *index;

      splitDomainAdvice(*it, domain, advice);
      index = new QListViewItem( dlg->lvDomainPolicy, KIDNA::toUnicode(domain),
                                 i18n(KCookieAdvice::adviceToStr(advice)) );
      m_pDomainPolicy[index] = KCookieAdvice::adviceToStr(advice);
    }
}

void KCookiesPolicies::selectionChanged ()
{
  QListViewItem* item = dlg->lvDomainPolicy->firstChild ();

  d_itemsSelected = 0;

  while (item != 0L)
  {
    if (dlg->lvDomainPolicy->isSelected (item))
      d_itemsSelected++;
    item = item->nextSibling ();
  }

  updateButtons ();
}

void KCookiesPolicies::load()
{
  d_itemsSelected = 0;

  KConfig cfg ("kcookiejarrc");
  cfg.setGroup ("Cookie Policy");

  bool enableCookies = cfg.readBoolEntry("Cookies", true);
  dlg->cbEnableCookies->setChecked (enableCookies);
  cookiesEnabled( enableCookies );

  KCookieAdvice::Value advice = KCookieAdvice::strToAdvice (cfg.readEntry(
                                               "CookieGlobalAdvice", "Ask"));
  switch (advice)
  {
    case KCookieAdvice::Accept:
      dlg->rbPolicyAccept->setChecked (true);
      break;
    case KCookieAdvice::Reject:
      dlg->rbPolicyReject->setChecked (true);
      break;
    case KCookieAdvice::Ask:
    case KCookieAdvice::Dunno:
    default:
      dlg->rbPolicyAsk->setChecked (true);
  }

  bool enable = cfg.readBoolEntry("RejectCrossDomainCookies", true);
  dlg->cbRejectCrossDomainCookies->setChecked (enable);

  bool sessionCookies = cfg.readBoolEntry("AcceptSessionCookies", true);
  dlg->cbAutoAcceptSessionCookies->setChecked (sessionCookies);
  bool cookieExpiration = cfg.readBoolEntry("IgnoreExpirationDate", false);
  dlg->cbIgnoreCookieExpirationDate->setChecked (cookieExpiration);

  if (enableCookies)
  {
    ignoreCookieExpirationDate( cookieExpiration );
    autoAcceptSessionCookies( sessionCookies );
    updateDomainList(cfg.readListEntry("CookieDomainAdvice"));
    updateButtons();
  }

  // Connect the main swicth :) Enable/disable cookie support
  connect( dlg->cbEnableCookies, SIGNAL( toggled(bool) ),
           SLOT( cookiesEnabled(bool) ) );
  connect( dlg->cbEnableCookies, SIGNAL( clicked() ), SLOT( emitChanged() ) );

  // Connect the preference check boxes...
  connect( dlg->cbRejectCrossDomainCookies, SIGNAL( clicked() ),
           SLOT( emitChanged() ) );
  connect( dlg->cbAutoAcceptSessionCookies, SIGNAL( clicked() ),
           SLOT( emitChanged() ) );
  connect( dlg->cbIgnoreCookieExpirationDate, SIGNAL( clicked() ),
           SLOT( emitChanged() ) );
  connect ( dlg->cbAutoAcceptSessionCookies, SIGNAL(toggled(bool)),
            SLOT(autoAcceptSessionCookies(bool)));
  connect ( dlg->cbIgnoreCookieExpirationDate, SIGNAL(toggled(bool)),
            SLOT(ignoreCookieExpirationDate(bool)));

  // Connect the default cookie policy radio buttons...
  connect(dlg->bgDefault, SIGNAL(clicked(int)), SLOT(emitChanged()));

  // Connect signals from the domain specific policy listview.
  connect( dlg->lvDomainPolicy, SIGNAL(selectionChanged()),
           SLOT(selectionChanged()) );
  connect( dlg->lvDomainPolicy, SIGNAL(doubleClicked (QListViewItem *)),
           SLOT(changePressed() ) );
  connect( dlg->lvDomainPolicy, SIGNAL(returnPressed ( QListViewItem * )),
           SLOT(changePressed() ) );
}

void KCookiesPolicies::save()
{
  QString advice;
  QStringList domainConfig;

  KConfig cfg ( "kcookiejarrc" );
  cfg.setGroup( "Cookie Policy" );

  bool state = dlg->cbEnableCookies->isChecked();
  cfg.writeEntry( "Cookies", state );
  state = dlg->cbRejectCrossDomainCookies->isChecked();
  cfg.writeEntry( "RejectCrossDomainCookies", state );
  state = dlg->cbAutoAcceptSessionCookies->isChecked();
  cfg.writeEntry( "AcceptSessionCookies", state );
  state = dlg->cbIgnoreCookieExpirationDate->isChecked();
  cfg.writeEntry( "IgnoreExpirationDate", state );


  if (dlg->rbPolicyAccept->isChecked())
      advice = KCookieAdvice::adviceToStr(KCookieAdvice::Accept);
  else if (dlg->rbPolicyReject->isChecked())
      advice = KCookieAdvice::adviceToStr(KCookieAdvice::Reject);
  else
      advice = KCookieAdvice::adviceToStr(KCookieAdvice::Ask);

  cfg.writeEntry("CookieGlobalAdvice", advice);


  QListViewItem *at = dlg->lvDomainPolicy->firstChild();
  while( at )
  {
    domainConfig.append(QString::fromLatin1("%1:%2").arg(KIDNA::toAscii(at->text(0))).arg(m_pDomainPolicy[at]));
    at = at->nextSibling();
  }

  cfg.writeEntry("CookieDomainAdvice", domainConfig);
  cfg.sync();

  // Update the cookiejar...
  DCOPClient *m_dcopClient = new DCOPClient();
  if( !m_dcopClient->attach() )
    kdDebug(7104) << "Can't connect with the DCOP server." << endl;
  else
  {
    if( dlg->cbEnableCookies->isChecked() )
     {
        if ( !m_dcopClient->send ("kded", "kcookiejar", "reloadPolicy()",
                                  QString::null) )
           kdDebug(7104) << "Can't communicate with the cookiejar!" << endl;
     }
     else
     {
        if ( !m_dcopClient->send ("kded", "kcookiejar", "shutdown()",
                                  QString::null) )
           kdDebug(7104) << "Can't communicate with the cookiejar!" << endl;
     }
  }
  delete m_dcopClient;

  KSaveIOConfig::updateRunningIOSlaves (this);

  emit changed( false );
}


void KCookiesPolicies::defaults()
{
  dlg->cbEnableCookies->setChecked( true );
  dlg->rbPolicyAsk->setChecked( true );
  dlg->rbPolicyAccept->setChecked( false );
  dlg->rbPolicyReject->setChecked( false );
  dlg->cbRejectCrossDomainCookies->setChecked( true );
  dlg->cbAutoAcceptSessionCookies->setChecked( true );
  dlg->cbIgnoreCookieExpirationDate->setChecked( false );
  dlg->lvDomainPolicy->clear();

  cookiesEnabled( dlg->cbEnableCookies->isChecked() );
  updateButtons();
}

void KCookiesPolicies::splitDomainAdvice (const QString& cfg, QString &domain,
                                          KCookieAdvice::Value &advice)
{
  int pos;

  pos = cfg.find(':');

  if ( pos == -1)
  {
    domain = cfg;
    advice = KCookieAdvice::Dunno;
  }
  else
  {
    domain = cfg.left(pos);
    advice = KCookieAdvice::strToAdvice (cfg.mid (pos+1, cfg.length()));
  }
}

QString KCookiesPolicies::quickHelp() const
{
  return i18n("<h1>Cookies</h1> Cookies contain information that Konqueror"
              " (or any other KDE application using the HTTP protocol) stores"
              " on your computer from a remote Internet server. This means"
              " that a web server can store information about you and your"
              " browsing activities on your machine for later use. You might"
              " consider this an invasion of privacy.<p>However, cookies are"
              " useful in certain situations. For example, they are often used"
              " by Internet shops, so you can 'put things into a shopping"
              " basket'. Some sites require you have a browser that supports"
              " cookies.<p>Because most people want a compromise between privacy"
              " and the benefits cookies offer, KDE offers you the ability to"
              " customize the way it handles cookies. You might, for example"
              " want to set KDE's default policy to ask you whenever a server"
              " wants to set a cookie or simply reject or accept everything."
              " For example, you might choose to accept all cookies from your"
              " favorite shopping web site. For this all you have to do is"
              " either browse to that particular site and when you are presented"
              " with the cookie dialog box, click on <i> This domain </i> under"
              " the 'apply to' tab and choose accept or simply specify the name"
              " of the site in the <i> Domain Specific Policy </i> tab and set"
              " it to accept. This enables you to receive cookies from trusted"
              " web sites without being asked every time KDE receives a cookie."
             );
}

#include "kcookiespolicies.moc"
