//---------------------------------------------------------------------------

#ifndef mainFormH
#define mainFormH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.Dialogs.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.Mask.hpp>
#include <Vcl.CheckLst.hpp>
#include <Vcl.Menus.hpp>
#include <System.Actions.hpp>
#include <Vcl.ActnList.hpp>
#include "CryptoArchive.h"
#include "CryptoArchiveReader.h"
#include <Vcl.Samples.Gauges.hpp>
//---------------------------------------------------------------------------
class TformMain : public TForm
{
__published:	// IDE-managed Components
	TMemo *memStatus;
	TSaveDialog *saveDialog;
	TPageControl *pcMain;
	TTabSheet *TabSheet1;
	TButton *btnGenerateRsaKeys;
	TEdit *EditUserName;
	TButton *btnSaveOpenRsaKey;
	TMaskEdit *EditPassword;
	TTabSheet *TabSheet2;
	TListBox *lstOperators;
	TCheckListBox *chkListAbonents;
	TMemo *memText;
	TLabel *lbMessageHeader;
	TListBox *lstAttach;
	TButton *btnAddAtachments;
	TButton *btnEncrypt;
	TTabSheet *TabSheet3;
	TListBox *lstCrypts;
	TButton *btnAddCrypts;
	TButton *btnDecrypt;
	TLabel *lbAbonents;
	TPopupMenu *popClear;
	TMenuItem *N1;
	TLabel *lbAuto;
	TTimer *Timer2;
	TLabel *lbLogin;
	TLabel *lbPassword;
	TLabel *lbOperators;
	TOpenDialog *OpenDialog1;
	TActionList *actList;
	TAction *actGenKeys;
	TLabel *lbCryptoOutDir;
	TSaveDialog *SaveDialog1;
	TEdit *edtDirCrypts;
	TLabel *Label8;
	TEdit *edtSaveMessageDir;
	TLabel *lbMessagesOutDir;
	TLabel *lbAttachInfo;
	TGroupBox *grpStatus;
	TButton *btnLoadOpenRsaKey;
	TPopupMenu *popAbonents;
	TMenuItem *delAbonent;
	TPopupMenu *popOperators;
	TMenuItem *delOperator;
	TGauge *Gauge1;
	TTimer *Timer1;
	void __fastcall btnAddCryptsClick(TObject *Sender);
	void __fastcall btnAddAtachmentsClick(TObject *Sender);
	void __fastcall btnEncryptClick(TObject *Sender);
	void __fastcall btnDecryptClick(TObject *Sender);
	void __fastcall Button5Click(TObject *Sender);
	void __fastcall btnGenerateRsaKeysClick(TObject *Sender);
	void __fastcall btnSaveOpenRsaKeyClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall lstOperatorsClick(TObject *Sender);
	void __fastcall N1Click(TObject *Sender);
	void __fastcall Timer2Timer(TObject *Sender);
	void __fastcall actGenKeysExecute(TObject *Sender);
	void __fastcall edtDirCryptsDblClick(TObject *Sender);
	void __fastcall edtSaveMessageDirDblClick(TObject *Sender);
	void __fastcall pcMainChange(TObject *Sender);
	void __fastcall btnLoadOpenRsaKeyClick(TObject *Sender);
	void __fastcall delAbonentClick(TObject *Sender);
	void __fastcall delOperatorClick(TObject *Sender);
	void __fastcall memTextChange(TObject *Sender);
	void __fastcall Timer1Timer(TObject *Sender);
private:	// User declarations

	std::map<std::string, CryptoArchiveCreator> mapCryptoArchiveCreators;


public:		// User declarations
	__fastcall TformMain(TComponent* Owner);

	std:: string p123;
	bool autoriz = false;
	void UpdateLabelWithFileInfo();
	void copyOpenRsaKey(const String& rsa_key_path);
	void delAbonentFunc(int ind);
	void delOperatorFunc(int ind);

	void __fastcall UnlockForm(TWinControl* parentControl);
	void __fastcall LockFormExceptStopButton(TWinControl* parentControl);

	bool isProcessing = false;
};
//---------------------------------------------------------------------------
extern PACKAGE TformMain *formMain;
//---------------------------------------------------------------------------
#endif

