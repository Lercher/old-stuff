VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "CKanten"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit

Private c As Collection

Public Function Item(Index As Variant) As CEcke
Attribute Item.VB_UserMemId = 0
    Set Item = c(Index)
End Function

Public Function NewEnum() As IUnknown
Attribute NewEnum.VB_UserMemId = -4
Attribute NewEnum.VB_MemberFlags = "40"
    Set NewEnum = c.[_NewEnum]
End Function

Public Function Add(ByVal E1 As CEcke, ByVal E2 As CEcke) As CKante
    Dim K As CKante
    
again:
    On Error GoTo 0
    Set K = New CKante
    With K
        Set .E1 = E1
        Set .E2 = E2
        K.bActive = True
    End With
    On Error GoTo again
    c.Add K, K.Key
    On Error GoTo 0
    Set Add = K
End Function

Private Sub Class_Initialize()
    Clear
End Sub

Public Sub Clear()
    Set c = New Collection
End Sub

Public Sub MakeVollstaendig(ByVal Ecken As CEcken)
    Dim i As Long, j As Long, n As Long
    Dim e As CEcke
    
    Clear
    n = Ecken.Count
    j = 0
    For Each e In Ecken
        j = j + 1
        For i = n To j + 1 Step -1
            Add e, Ecken(i)
        Next
    Next
End Sub

Public Sub LookAtAll(ByVal Ecke As CEcke, ByRef ToFind As Long)
    Dim e As CEcke
    
    Ecke.bSeen = True
    ToFind = ToFind - 1
    If ToFind <= 0 Then Exit Sub
    For Each e In Ecke.Nachbarn
        'frmMain.lCount = frmMain.lCount + 1
        If Not e.bSeen Then LookAtAll e, ToFind
        If ToFind <= 0 Then Exit Sub
    Next
End Sub

Public Sub ResetTried()
    Dim K As CKante
    
    For Each K In c
        K.bTried = False
    Next
End Sub

Public Function GetNextMax() As CKante
    Dim K As CKante
    Dim kmax As CKante
    Dim max As Double
    
    max = -1
    Set kmax = Nothing
    For Each K In c
        If Not K.bTried Then
            If K.Distance > max Then
                Set kmax = K
                max = K.Distance
            End If
        End If
    Next
    If Not kmax Is Nothing Then kmax.bTried = True
    Set GetNextMax = kmax
End Function

Public Sub Remove(ByVal K As CKante)
    c.Remove K.Key
End Sub

Public Function Count() As Long
    Count = c.Count
End Function
