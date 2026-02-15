object formMain: TformMain
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu, biMinimize]
  Caption = #1055#1088#1086#1075#1088#1072#1084#1084#1072'_3.0'
  ClientHeight = 1486
  ClientWidth = 1308
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -24
  Font.Name = 'Segoe UI'
  Font.Style = []
  Position = poDesktopCenter
  OnShow = FormShow
  PixelsPerInch = 192
  TextHeight = 32
  object lbAuto: TLabel
    AlignWithMargins = True
    Left = 6
    Top = 1435
    Width = 1296
    Height = 45
    Margins.Left = 6
    Margins.Top = 6
    Margins.Right = 6
    Margins.Bottom = 6
    Align = alBottom
    Caption = #1053#1077#1090' '#1072#1074#1090#1086#1088#1080#1079#1072#1094#1080#1080
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -32
    Font.Name = 'Segoe UI'
    Font.Style = []
    ParentFont = False
    Visible = False
    ExplicitWidth = 252
  end
  object lbLogin: TLabel
    AlignWithMargins = True
    Left = 6
    Top = 6
    Width = 1296
    Height = 32
    Margins.Left = 6
    Margins.Top = 6
    Margins.Right = 6
    Margins.Bottom = 6
    Align = alTop
    Caption = #1051#1086#1075#1080#1085':'
    ExplicitWidth = 72
  end
  object lbPassword: TLabel
    AlignWithMargins = True
    Left = 6
    Top = 102
    Width = 1296
    Height = 32
    Margins.Left = 6
    Margins.Top = 6
    Margins.Right = 6
    Margins.Bottom = 6
    Align = alTop
    Caption = #1055#1072#1088#1086#1083#1100':'
    ExplicitWidth = 87
  end
  object pcMain: TPageControl
    AlignWithMargins = True
    Left = 6
    Top = 202
    Width = 1296
    Height = 871
    Margins.Left = 6
    Margins.Top = 6
    Margins.Right = 6
    Margins.Bottom = 6
    ActivePage = TabSheet2
    Align = alTop
    TabOrder = 0
    Visible = False
    OnChange = pcMainChange
    ExplicitWidth = 1284
    object TabSheet2: TTabSheet
      Margins.Left = 6
      Margins.Top = 6
      Margins.Right = 6
      Margins.Bottom = 6
      Caption = #1047#1072#1096#1080#1092#1088#1086#1074#1072#1085#1080#1077
      ImageIndex = 1
      object lbMessageHeader: TLabel
        AlignWithMargins = True
        Left = 6
        Top = 253
        Width = 1268
        Height = 32
        Margins.Left = 6
        Margins.Top = 6
        Margins.Right = 6
        Margins.Bottom = 6
        Align = alTop
        Caption = #1047#1072#1075#1086#1083#1086#1074#1086#1082':'
        ExplicitWidth = 119
      end
      object lbAbonents: TLabel
        AlignWithMargins = True
        Left = 6
        Top = 6
        Width = 1268
        Height = 32
        Margins.Left = 6
        Margins.Top = 6
        Margins.Right = 6
        Margins.Bottom = 6
        Align = alTop
        Caption = #1040#1073#1086#1085#1077#1085#1090#1099':'
        ExplicitWidth = 116
      end
      object lbCryptoOutDir: TLabel
        AlignWithMargins = True
        Left = 6
        Top = 659
        Width = 1268
        Height = 32
        Margins.Left = 6
        Margins.Top = 6
        Margins.Right = 6
        Margins.Bottom = 6
        Align = alBottom
        Caption = #1055#1091#1090#1100' '#1089#1086#1093#1088#1072#1085#1077#1085#1080#1103' '#1082#1088#1080#1087#1090#1086#1075#1088#1072#1084#1084':'
        ExplicitWidth = 346
      end
      object Label8: TLabel
        AlignWithMargins = True
        Left = 6
        Top = 443
        Width = 1268
        Height = 32
        Margins.Left = 6
        Margins.Top = 6
        Margins.Right = 6
        Margins.Bottom = 6
        Align = alTop
        Caption = #1055#1088#1080#1083#1086#1078#1077#1085#1080#1103':'
        ExplicitWidth = 148
      end
      object lbAttachInfo: TLabel
        AlignWithMargins = True
        Left = 6
        Top = 615
        Width = 1268
        Height = 32
        Margins.Left = 6
        Margins.Top = 6
        Margins.Right = 6
        Margins.Bottom = 6
        Align = alBottom
        Caption = #1060#1072#1081#1083#1086#1074' '#1087#1088#1080#1083#1086#1078#1077#1085#1080#1081' - 0'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBlack
        Font.Height = -24
        Font.Name = 'Segoe UI'
        Font.Style = []
        ParentFont = False
        ExplicitWidth = 269
      end
      object chkListAbonents: TCheckListBox
        AlignWithMargins = True
        Left = 6
        Top = 50
        Width = 1268
        Height = 191
        Margins.Left = 6
        Margins.Top = 6
        Margins.Right = 6
        Margins.Bottom = 6
        Align = alTop
        ItemHeight = 32
        PopupMenu = popAbonents
        TabOrder = 0
        ExplicitWidth = 1256
      end
      object memText: TMemo
        AlignWithMargins = True
        Left = 6
        Top = 297
        Width = 1268
        Height = 72
        Margins.Left = 6
        Margins.Top = 6
        Margins.Right = 6
        Margins.Bottom = 6
        Align = alTop
        TabOrder = 1
        OnChange = memTextChange
        ExplicitWidth = 1256
      end
      object btnAddAtachments: TButton
        AlignWithMargins = True
        Left = 6
        Top = 381
        Width = 1268
        Height = 50
        Margins.Left = 6
        Margins.Top = 6
        Margins.Right = 6
        Margins.Bottom = 6
        Align = alTop
        Caption = #1044#1086#1073#1072#1074#1080#1090#1100' '#1092#1072#1081#1083#1099' '#1087#1088#1080#1083#1086#1078#1077#1085#1080#1103
        TabOrder = 2
        OnClick = btnAddAtachmentsClick
        ExplicitWidth = 1256
      end
      object lstAttach: TListBox
        AlignWithMargins = True
        Left = 6
        Top = 487
        Width = 1268
        Height = 116
        Margins.Left = 6
        Margins.Top = 6
        Margins.Right = 6
        Margins.Bottom = 6
        Align = alClient
        ImeName = 'lstAttach'
        ItemHeight = 32
        PopupMenu = popClear
        TabOrder = 3
        ExplicitWidth = 1256
      end
      object btnEncrypt: TButton
        AlignWithMargins = True
        Left = 6
        Top = 755
        Width = 1268
        Height = 50
        Margins.Left = 6
        Margins.Top = 6
        Margins.Right = 6
        Margins.Bottom = 6
        Align = alBottom
        Caption = #1047#1072#1096#1080#1092#1088#1086#1074#1072#1090#1100
        TabOrder = 4
        OnClick = btnEncryptClick
        ExplicitWidth = 1256
      end
      object edtDirCrypts: TEdit
        AlignWithMargins = True
        Left = 6
        Top = 703
        Width = 1268
        Height = 40
        Margins.Left = 6
        Margins.Top = 6
        Margins.Right = 6
        Margins.Bottom = 6
        Align = alBottom
        ImeName = 'edtDirCrypts'
        ParentShowHint = False
        ShowHint = True
        TabOrder = 5
        TextHint = ' ('#1076#1074#1086#1081#1085#1086#1081' '#1096#1077#1083#1095#1086#1082' '#1076#1083#1103' '#1074#1099#1073#1086#1088#1072')'
        OnDblClick = edtDirCryptsDblClick
        ExplicitWidth = 1256
      end
    end
    object TabSheet3: TTabSheet
      Margins.Left = 6
      Margins.Top = 6
      Margins.Right = 6
      Margins.Bottom = 6
      Caption = #1056#1072#1089#1096#1080#1092#1088#1086#1074#1072#1085#1080#1077
      ImageIndex = 2
      object lbMessagesOutDir: TLabel
        AlignWithMargins = True
        Left = 6
        Top = 659
        Width = 529
        Height = 32
        Margins.Left = 6
        Margins.Top = 6
        Margins.Right = 6
        Margins.Bottom = 6
        Align = alBottom
        Caption = #1055#1091#1090#1100' '#1089#1086#1093#1088#1072#1085#1077#1085#1080#1103' '#1088#1072#1089#1096#1080#1092#1088#1086#1074#1072#1085#1085#1099#1093' '#1089#1086#1086#1073#1097#1077#1085#1080#1081':'
      end
      object lstCrypts: TListBox
        AlignWithMargins = True
        Left = 6
        Top = 68
        Width = 1268
        Height = 579
        Margins.Left = 6
        Margins.Top = 6
        Margins.Right = 6
        Margins.Bottom = 6
        Align = alClient
        ItemHeight = 32
        PopupMenu = popClear
        TabOrder = 0
      end
      object btnAddCrypts: TButton
        AlignWithMargins = True
        Left = 6
        Top = 6
        Width = 1268
        Height = 50
        Margins.Left = 6
        Margins.Top = 6
        Margins.Right = 6
        Margins.Bottom = 6
        Align = alTop
        Caption = #1044#1086#1073#1072#1074#1080#1090#1100' '#1082#1088#1080#1087#1090#1086#1075#1088#1072#1084#1084#1099' '#1076#1083#1103' '#1088#1072#1089#1096#1080#1092#1088#1086#1074#1072#1085#1080#1103
        TabOrder = 1
        OnClick = btnAddCryptsClick
      end
      object btnDecrypt: TButton
        AlignWithMargins = True
        Left = 6
        Top = 755
        Width = 1268
        Height = 50
        Margins.Left = 6
        Margins.Top = 6
        Margins.Right = 6
        Margins.Bottom = 6
        Align = alBottom
        Caption = #1056#1072#1089#1096#1080#1092#1088#1086#1074#1072#1090#1100' '#1082#1088#1080#1087#1090#1086#1075#1088#1072#1084#1084#1099
        TabOrder = 2
        OnClick = btnDecryptClick
      end
      object edtSaveMessageDir: TEdit
        AlignWithMargins = True
        Left = 6
        Top = 703
        Width = 1268
        Height = 40
        Margins.Left = 6
        Margins.Top = 6
        Margins.Right = 6
        Margins.Bottom = 6
        Align = alBottom
        ParentShowHint = False
        ShowHint = True
        TabOrder = 3
        TextHint = ' ('#1076#1074#1086#1081#1085#1086#1081' '#1096#1077#1083#1095#1086#1082' '#1076#1083#1103' '#1074#1099#1073#1086#1088#1072')'
        OnDblClick = edtSaveMessageDirDblClick
      end
    end
    object TabSheet1: TTabSheet
      AlignWithMargins = True
      Margins.Left = 6
      Margins.Top = 6
      Margins.Right = 6
      Margins.Bottom = 6
      Caption = #1050#1083#1102#1095#1080
      object lbOperators: TLabel
        AlignWithMargins = True
        Left = 6
        Top = 201
        Width = 131
        Height = 32
        Margins.Left = 6
        Margins.Top = 6
        Margins.Right = 6
        Margins.Bottom = 6
        Align = alTop
        Caption = #1054#1087#1077#1088#1072#1090#1086#1088#1099':'
      end
      object btnGenerateRsaKeys: TButton
        AlignWithMargins = True
        Left = 6
        Top = 15
        Width = 1256
        Height = 50
        Margins.Left = 6
        Margins.Top = 15
        Margins.Right = 6
        Margins.Bottom = 6
        Align = alTop
        Caption = #1057#1075#1077#1085#1077#1088#1080#1088#1086#1074#1072#1090#1100' '#1085#1086#1074#1099#1077' '#1082#1083#1102#1095#1080
        TabOrder = 0
        OnClick = btnGenerateRsaKeysClick
      end
      object btnSaveOpenRsaKey: TButton
        AlignWithMargins = True
        Left = 6
        Top = 139
        Width = 1256
        Height = 50
        Margins.Left = 6
        Margins.Top = 6
        Margins.Right = 6
        Margins.Bottom = 6
        Align = alTop
        Caption = #1055#1086#1076#1077#1083#1080#1090#1100#1089#1103' '#1086#1090#1082#1088#1099#1090#1099#1084' '#1082#1083#1102#1095#1086#1084
        TabOrder = 1
        OnClick = btnSaveOpenRsaKeyClick
      end
      object lstOperators: TListBox
        AlignWithMargins = True
        Left = 6
        Top = 245
        Width = 1256
        Height = 548
        Margins.Left = 6
        Margins.Top = 6
        Margins.Right = 6
        Margins.Bottom = 6
        Align = alClient
        ImeName = 'lstYa'
        ItemHeight = 32
        PopupMenu = popOperators
        TabOrder = 2
        OnClick = lstOperatorsClick
      end
      object btnLoadOpenRsaKey: TButton
        AlignWithMargins = True
        Left = 6
        Top = 77
        Width = 1256
        Height = 50
        Margins.Left = 6
        Margins.Top = 6
        Margins.Right = 6
        Margins.Bottom = 6
        Align = alTop
        Caption = #1047#1072#1075#1088#1091#1079#1082#1072' '#1086#1090#1082#1088#1099#1090#1099#1093' '#1082#1083#1102#1095#1077#1081' '#1072#1073#1086#1085#1077#1085#1090#1086#1074
        TabOrder = 3
        OnClick = btnLoadOpenRsaKeyClick
      end
    end
  end
  object EditUserName: TEdit
    AlignWithMargins = True
    Left = 6
    Top = 50
    Width = 1296
    Height = 40
    Margins.Left = 6
    Margins.Top = 6
    Margins.Right = 6
    Margins.Bottom = 6
    Align = alTop
    AutoSize = False
    TabOrder = 1
    ExplicitWidth = 1284
  end
  object EditPassword: TMaskEdit
    AlignWithMargins = True
    Left = 6
    Top = 146
    Width = 1296
    Height = 40
    Margins.Left = 6
    Margins.Top = 6
    Margins.Right = 6
    Margins.Bottom = 10
    Align = alTop
    AutoSize = False
    TabOrder = 2
    Text = ''
    ExplicitWidth = 1284
  end
  object grpStatus: TGroupBox
    AlignWithMargins = True
    Left = 2
    Top = 1085
    Width = 1304
    Height = 338
    Margins.Left = 2
    Margins.Top = 6
    Margins.Right = 2
    Margins.Bottom = 6
    Align = alClient
    Caption = #1057#1090#1072#1090#1091#1089' '#1074#1099#1087#1086#1083#1085#1077#1085#1080#1103':'
    TabOrder = 3
    ExplicitWidth = 1292
    ExplicitHeight = 330
    object Gauge1: TGauge
      AlignWithMargins = True
      Left = 8
      Top = 283
      Width = 1288
      Height = 47
      Margins.Left = 6
      Margins.Top = 6
      Margins.Right = 6
      Margins.Bottom = 6
      Align = alBottom
      Progress = 0
      ExplicitLeft = 0
      ExplicitTop = 752
      ExplicitWidth = 1157
    end
    object memStatus: TMemo
      AlignWithMargins = True
      Left = 8
      Top = 40
      Width = 1288
      Height = 231
      Margins.Left = 6
      Margins.Top = 6
      Margins.Right = 6
      Margins.Bottom = 6
      Align = alClient
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -19
      Font.Name = 'Segoe UI'
      Font.Style = []
      ParentFont = False
      ReadOnly = True
      ScrollBars = ssVertical
      TabOrder = 0
      ExplicitWidth = 1276
      ExplicitHeight = 223
    end
  end
  object saveDialog: TSaveDialog
    Left = 672
    Top = 368
  end
  object popClear: TPopupMenu
    Left = 664
    Top = 388
    object N1: TMenuItem
      Caption = #1054#1095#1080#1089#1090#1080#1090#1100
      OnClick = N1Click
    end
  end
  object Timer2: TTimer
    Interval = 2000
    OnTimer = Timer2Timer
    Left = 860
    Top = 372
  end
  object OpenDialog1: TOpenDialog
    Left = 912
    Top = 544
  end
  object actList: TActionList
    Left = 690
    Top = 928
    object actGenKeys: TAction
      Caption = 'generateKeys'
      ShortCut = 49223
      OnExecute = actGenKeysExecute
    end
  end
  object SaveDialog1: TSaveDialog
    Left = 878
    Top = 846
  end
  object popAbonents: TPopupMenu
    Left = 1004
    Top = 348
    object delAbonent: TMenuItem
      Caption = #1059#1076#1072#1083#1080#1090#1100' '#1072#1073#1086#1085#1077#1085#1090#1072
      OnClick = delAbonentClick
    end
  end
  object popOperators: TPopupMenu
    Left = 452
    Top = 340
    object delOperator: TMenuItem
      Caption = #1059#1076#1072#1083#1080#1090#1100' '#1086#1087#1077#1088#1072#1090#1086#1088#1072
      OnClick = delOperatorClick
    end
  end
  object Timer1: TTimer
    Interval = 100
    OnTimer = Timer1Timer
    Left = 1010
    Top = 626
  end
end
