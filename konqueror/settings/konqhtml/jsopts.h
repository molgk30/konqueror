//-----------------------------------------------------------------------------
//
// HTML Options
//
// (c) Martin R. Jones 1996
//
// Port to KControl
// (c) Torben Weis 1998

#ifndef __JSOPTS_H__
#define __JSOPTS_H__

#include <qwidget.h>
#include <qmap.h>

class KColorButton;
class KConfig;
class KListView;
class KURLRequester;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QListViewItem;
class QRadioButton;
class QSpinBox;
class QButtonGroup;

class KJavaScriptOptions : public QWidget
{
  Q_OBJECT
public:
  KJavaScriptOptions( KConfig* config, QString group, QWidget* parent = 0, const char* name = 0 );

  virtual void load();
  virtual void save();
  virtual void defaults();

signals:
  void changed( bool state );

private slots:
  void changed();
  void importPressed();
  void exportPressed();
  void addPressed();
  void changePressed();
  void deletePressed();

private:
  void changeJavaScriptEnabled();
  void updateDomainList(const QStringList &domainConfig);

  KConfig *m_pConfig;
  QString m_groupname;
  QCheckBox *enableJavaScriptGloballyCB;
  QCheckBox *enableJavaScriptDebugCB;
  QButtonGroup *js_popup;
  KListView* domainSpecificLV;
  QMap<QListViewItem*, int> javaScriptDomainPolicy;
};


#endif		// __HTML_OPTIONS_H__

