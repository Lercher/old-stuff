VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "CEcke"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit

Public X As Double
Public Y As Double
Public bSeen As Boolean
Public Nachbarn As CEcken
Public Key As String

Private Sub Class_Initialize()
    Key = "E" & Rnd()
    Set Nachbarn = New CEcken
End Sub

Public Function isNachbar(ByVal OfThisEcke As CEcke) As Boolean
    isNachbar = Not (Nachbarn.Item(OfThisEcke.Key) Is Nothing)
End Function
