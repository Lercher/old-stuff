VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "CGraph"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit

Public Ecken As New CEcken
Public Kanten As New CKanten

Public Sub Vollstaendig()
    Kanten.MakeVollstaendig Ecken
    Debug.Print "Kantenzahl: "; Kanten.Count
End Sub

Public Sub Trivial()
    Kanten.Clear
End Sub

Public Function isZusammenhaengend() As Boolean
    Dim e As CEcke
        
    On Error GoTo leer
    Set e = Ecken(1)
    On Error GoTo 0
    Ecken.ResetSeen
    'frmMain.lCount = 0
    Kanten.LookAtAll Ecken(1), Ecken.Count
    'Debug.Print "Zus: "; frmMain.lCount
    isZusammenhaengend = Ecken.AllSeen
    Exit Function
leer:
    isZusammenhaengend = True
End Function

Public Sub Geruest(ByVal WithView As Boolean)
    Dim K As CKante
    
    If WithView Then frmMain.DrawGraph Me
    Kanten.ResetTried
    Do
        Set K = Kanten.GetNextMax
        If K Is Nothing Then Exit Do
        If WithView Then frmMain.DrawKante K, vbGreen
        If K.isPartOf3Zykel Then
            If WithView Then frmMain.DrawKante K, vbWhite
            Kanten.Remove K
            K.bActive = False
        Else
            K.bActive = False
            If Me.isZusammenhaengend Then
                If WithView Then frmMain.DrawKante K, vbWhite
                Kanten.Remove K
                'Kanten.ResetTried hier unnötig,
                'denn eine Kante, die den Graphen unzusammenhängend macht
                'wird das auch später, d.h. bei einem (echten) Teilgraphen wieder tun
            Else
                If WithView Then frmMain.DrawKante K, vbBlue
                K.bActive = True
            End If
        End If
    Loop
End Sub
