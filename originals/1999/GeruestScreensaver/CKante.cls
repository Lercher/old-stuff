VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "CKante"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit

Private theE1 As CEcke
Private theE2 As CEcke
Private theDistance As Double
Private theActive As Boolean
Public bTried As Boolean
Public Key As String

Public Property Get bActive() As Boolean
    bActive = theActive
End Property

Public Property Let bActive(ByVal rhs As Boolean)
    If theActive <> rhs Then
        theActive = rhs
        If theActive Then
            E1.Nachbarn.AddEcke E2
            E2.Nachbarn.AddEcke E1
        Else
            E1.Nachbarn.Remove E2
            E2.Nachbarn.Remove E1
        End If
    End If
End Property

Public Property Get Distance() As Double
    Distance = theDistance
End Property

Public Property Get E1() As CEcke
    Set E1 = theE1
End Property

Public Property Set E1(ByVal vNewValue As CEcke)
    Set theE1 = vNewValue
    UpdateDistance
End Property

Public Property Get E2() As CEcke
    Set E2 = theE2
End Property

Public Property Set E2(ByVal vNewValue As CEcke)
    Set theE2 = vNewValue
    UpdateDistance
End Property

Private Sub UpdateDistance()
    Dim dx As Double, dy As Double
    
    If theE1 Is Nothing Or theE2 Is Nothing Then Exit Sub
    Debug.Assert Not theE1 Is theE2
    dx = theE1.X - theE2.X
    dy = theE1.Y - theE2.Y
    theDistance = Sqr(dx * dx + dy * dy)
End Sub

Public Function Nachbar(ByVal OfThisEcke As CEcke) As CEcke
    If bActive Then
        If theE1 Is OfThisEcke Then
            Set Nachbar = theE2
        ElseIf theE2 Is OfThisEcke Then
            Set Nachbar = theE1
        Else
            Set Nachbar = Nothing
        End If
    Else
        Set Nachbar = Nothing
    End If
End Function

Public Function isPartOf3Zykel() As Boolean
    Dim e As CEcke
    Dim Testecke As CEcke
    Dim n As CEcken
    
    If E1.Nachbarn.Count < E2.Nachbarn.Count Then
        Set n = E1.Nachbarn
        Set Testecke = E2
    Else
        Set n = E2.Nachbarn
        Set Testecke = E1
    End If
    'frmMain.lCount = 0
    For Each e In n
        'frmMain.lCount = frmMain.lCount + 1
        If e.isNachbar(Testecke) Then
            isPartOf3Zykel = True
            'Debug.Print "Zyc: "; frmMain.lCount
            Exit Function
        End If
    Next
    isPartOf3Zykel = False
    'Debug.Print "Zyc bad: "; frmMain.lCount
End Function

Private Sub Class_Initialize()
    theActive = False
    bTried = False
    Key = "K" & Rnd()
End Sub

Private Sub Class_Terminate()
    bActive = False
End Sub
