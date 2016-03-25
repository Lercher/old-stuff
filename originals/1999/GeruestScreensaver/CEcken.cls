VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "CEcken"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit

Private c As New Collection

Public Function Item(Index As Variant) As CEcke
Attribute Item.VB_UserMemId = 0
    On Error Resume Next
    Set Item = c(Index)
    On Error GoTo 0
End Function

Public Function NewEnum() As IUnknown
Attribute NewEnum.VB_UserMemId = -4
Attribute NewEnum.VB_MemberFlags = "40"
    Set NewEnum = c.[_NewEnum]
End Function

Public Function Count() As Long
    Count = c.Count
End Function

Public Function CountSeen() As Long
    Dim e As CEcke
    Dim l As Long
    
    l = 0
    For Each e In c
        If e.bSeen Then l = l + 1
    Next
    CountSeen = l
End Function

Public Function AllSeen() As Boolean
    Dim e As CEcke
    
    For Each e In c
        If Not e.bSeen Then
            AllSeen = False
            Exit Function
        End If
    Next
    AllSeen = True
End Function

Public Function Add(ByVal X As Single, ByVal Y As Single) As CEcke
    Dim e As CEcke
    
    Set e = New CEcke
    With e
        .X = X
        .Y = Y
    End With
    Set Add = AddEcke(e)
End Function

Public Function AddEcke(ByVal e As CEcke) As CEcke
    c.Add e, e.Key
    Set AddEcke = e
End Function

Public Sub ResetSeen()
    Dim e As CEcke
    For Each e In c
        e.bSeen = False
    Next
End Sub

Public Sub NewNachbarn()
    Dim e As CEcke
    For Each e In c
        Set e.Nachbarn = New CEcken
    Next
End Sub

Public Sub Remove(ByVal e As CEcke)
    c.Remove e.Key
End Sub
